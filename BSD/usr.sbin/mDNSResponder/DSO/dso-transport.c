/* dso-transport.c
 *
 * Copyright (c) 2018-2019 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//*************************************************************************************************************
// Headers

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <netdb.h>           // For gethostbyname()
#include <sys/socket.h>      // For AF_INET, AF_INET6, etc.
#include <net/if.h>          // For IF_NAMESIZE
#include <netinet/in.h>      // For INADDR_NONE
#include <netinet/tcp.h>     // For SOL_TCP, TCP_NOTSENT_LOWAT
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "dns_sd.h"
#include "DNSCommon.h"
#include "mDNSEmbeddedAPI.h"
#include "dso.h"
#include "dso-transport.h"

#ifdef DSO_USES_NETWORK_FRAMEWORK
// Network Framework only works on MacOS X at the moment, and we need the locking primitives for
// MacOSX.
#include "mDNSMacOSX.h"
#endif

extern mDNS mDNSStorage;

static dso_connect_state_t *dso_connect_states; // DSO connect states that exist.
static dso_transport_t *dso_transport_states; // DSO transport states that exist.
#ifdef DSO_USES_NETWORK_FRAMEWORK
static uint32_t dso_transport_serial; // Serial number of next dso_transport_state_t or dso_connect_state_t.
static dispatch_queue_t dso_dispatch_queue;
#else
static void dso_read_callback(TCPSocket *sock, void *context, mDNSBool connection_established,
                       mStatus err);
#endif

void
dso_transport_init(void)
{
#ifdef DSO_USES_NETWORK_FRAMEWORK
    // It's conceivable that we might want a separate queue, but we don't know yet, so for
    // now we just use the main dispatch queue, which should be on the main dispatch thread,
    // which is _NOT_ the kevent thread.   So whenever we are doing anything on the dispatch
    // queue (any completion functions for NW framework) we need to acquire the lock before
    // we even look at any variables that could be changed by the other thread.
    dso_dispatch_queue = dispatch_get_main_queue();
#endif
}

#ifdef DSO_USES_NETWORK_FRAMEWORK
static dso_connect_state_t *
dso_connect_state_find(uint32_t serial)
{
    dso_connect_state_t *csp;
    for (csp = dso_connect_states; csp; csp = csp->next) {
        if (csp->serial ==  serial) {
            return csp;
        }
    }
    return NULL;
}
#endif

static void
dso_transport_finalize(dso_transport_t *transport)
{
    dso_transport_t **tp = &dso_transport_states;
    if (transport->connection != NULL) {
#ifdef DSO_USES_NETWORK_FRAMEWORK
        nw_connection_cancel(transport->connection);
        nw_release(transport->connection);
#else
        mDNSPlatformTCPCloseConnection(transport->connection);
#endif
        transport->connection = NULL;
    }
    while (*tp) {
        if (*tp == transport) {
            *tp = transport->next;
        } else {
            tp = &transport->next;
        }
    }
    free(transport);
}    

// We do all of the finalization for the dso state object and any objects it depends on here in the
// dso_idle function because it avoids the possibility that some code on the way out to the event loop
// _after_ the DSO connection has been dropped might still write to the DSO structure or one of the
// dependent structures and corrupt the heap, or indeed in the unlikely event that this memory was
// freed and then reallocated before the exit to the event loop, there could be a bad pointer
// dereference.
//
// If there is a finalize function, that function MUST either free its own state that references the
// DSO state, or else must NULL out the pointer to the DSO state.
int64_t dso_transport_idle(void *context, int64_t now_in, int64_t next_timer_event)
{
    dso_connect_state_t *cs, *cnext;
    mDNS *m = context;
    mDNSs32 now = (mDNSs32)now_in;
    mDNSs32 next_event = (mDNSs32)next_timer_event;

    // Notice if a DSO connection state is active but hasn't seen activity in a while.
    for (cs = dso_connect_states; cs != NULL; cs = cnext) {
        cnext = cs->next;
        if (!cs->connecting && cs->last_event != 0) {
            mDNSs32 expiry = cs->last_event + 90 * mDNSPlatformOneSecond;
            if (now - expiry > 0) {
                cs->last_event = 0;
                cs->callback(cs->context, NULL, NULL, kDSOEventType_ConnectFailed);
                if (cs->lookup != NULL) {
                    DNSServiceRef ref = cs->lookup;
                    cs->lookup = NULL;
                    mDNS_DropLockBeforeCallback();
                    DNSServiceRefDeallocate(ref);
                    mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
                }
            } else {
                if (next_timer_event - expiry > 0) {
                    next_timer_event = expiry;
                }
            }
        } else if (!cs->connecting && cs->reconnect_time && now - cs->reconnect_time > 0) {
            cs->reconnect_time = 0; // Don't try to immediately reconnect if it fails.
            // If cs->dso->transport is non-null, we're already connected.
            if (cs->dso && cs->dso->transport == NULL) {
                cs->callback(cs->context, NULL, NULL, kDSOEventType_ShouldReconnect);
            }
        }
        if (cs->reconnect_time != 0 && next_event - cs->reconnect_time > 0) {
            next_event = cs->reconnect_time;
        }
    }
            
    return next_event;
}

// Call to schedule a reconnect at a later time.
void dso_schedule_reconnect(mDNS *m, dso_connect_state_t *cs, mDNSs32 when)
{
    cs->reconnect_time = when * mDNSPlatformOneSecond + m->timenow;
}

// If a DSO was created by an incoming connection, the creator of the listener can use this function
// to supply context and a callback for future events.
void dso_set_callback(dso_state_t *dso, void *context, dso_event_callback_t cb)
{
    dso->cb = cb;
    dso->context = context;
}

// This is called before writing a DSO message to the output buffer.  length is the length of the message.
// Returns true if we have successfully selected for write (which means that we're under TCP_NOTSENT_LOWAT).
// Otherwise returns false.   It is valid to write even if it returns false, but there is a risk that
// the write will return EWOULDBLOCK, at which point we'd have to blow away the connection.   It is also
// valid to give up at this point and not write a message; as long as dso_write_finish isn't called, a later
// call to dso_write_start will overwrite the length that was stored by the previous invocation.
//
// The circumstance in which this would occur is that we have filled the kernel's TCP output buffer for this
// connection all the way up to TCP_NOTSENT_LOWAT, and then we get a query from the Discovery Proxy to which we
// need to respond.  Because TCP_NOTSENT_LOWAT is fairly low, there should be a lot of room in the TCP output
// buffer for small responses; it would need to be the case that we are getting requests from the proxy at a
// high rate for us to fill the output buffer to the point where a write of a 12-byte response returns
// EWOULDBLOCK; in that case, things are so dysfunctional that killing the connection isn't any worse than
// allowing it to continue.

// An additional note about the motivation for this code: the idea originally was that we'd do scatter/gather
// I/O here: this lets us write everything out in a single sendmsg() call.   This isn't used with the mDNSPlatformTCP
// code because it doesn't support scatter/gather.   Network Framework does, however, and in principle we could
// write to the descriptor directly if that were really needed.

bool dso_write_start(dso_transport_t *transport, size_t length)
{
    // The transport doesn't support messages outside of this range.
    if (length < 12 || length > 65535) {
        return false;
    }

#ifdef DSO_USES_NETWORK_FRAMEWORK
    uint8_t lenbuf[2];

    if (transport->to_write != NULL) {
        nw_release(transport->to_write);
        transport->to_write = NULL;
    }
    lenbuf[0] = length >> 8;
    lenbuf[1] = length & 255;
    transport->to_write = dispatch_data_create(lenbuf, 2, dso_dispatch_queue,
                                               DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    if (transport->to_write == NULL) {
        transport->write_failed = true;
        return false;
    }
    transport->bytes_to_write = length + 2;

    // We don't have access to TCP_NOTSENT_LOWAT, so for now we track how many bytes we've written
    // versus how many bytes that we've written have completed, and if that creeps above MAX_UNSENT_BYTES,
    // we return false here to indicate that there is congestion.
    if (transport->unsent_bytes > MAX_UNSENT_BYTES) {
        return false;
    } else {
        return true;
    }
#else
    transport->lenbuf[0] = length >> 8;
    transport->lenbuf[1] = length & 255;

    transport->to_write[0] = transport->lenbuf;
    transport->write_lengths[0] = 2;
    transport->num_to_write = 1;

    return mDNSPlatformTCPWritable(transport->connection);
#endif // DSO_USES_NETWORK_FRAMEWORK
}

// Called to finish a write (dso_write_start .. dso_write .. [ dso_write ... ] dso_write_finish).  The
// write must completely finish--if we get a partial write, this means that the connection is stalled, and
// so we drop it.  Since this can call dso_drop, the caller must not reference the DSO state object
// after this call if the return value is false.
bool dso_write_finish(dso_transport_t *transport)
{
#ifdef DSO_USES_NETWORK_FRAMEWORK
    uint32_t serial = transport->dso->serial;
    int bytes_to_write = transport->bytes_to_write;
    transport->bytes_to_write = 0;
    if (transport->write_failed) {
        dso_drop(transport->dso);
        return false;
    }
    transport->unsent_bytes += bytes_to_write;
    nw_connection_send(transport->connection, transport->to_write, NW_CONNECTION_DEFAULT_STREAM_CONTEXT, true,
                       ^(nw_error_t  _Nullable error) {
                           dso_state_t *dso;
                           KQueueLock();
                           dso = dso_find_by_serial(serial);
                           if (error != NULL) {
                               LogMsg("dso_write_finish: write failed: %s", strerror(nw_error_get_error_code(error)));
                               if (dso != NULL) {
                                   dso_drop(dso);
                               }
                           } else {
                               dso->transport->unsent_bytes -= bytes_to_write;
                               LogMsg("dso_write_finish completion routine: %d bytes written, %d bytes outstanding",
                                      bytes_to_write, dso->transport->unsent_bytes);
                           }
                           KQueueUnlock("dso_write_finish completion routine");
                       });
    nw_release(transport->to_write);
    transport->to_write = NULL;
    return true;
#else
    ssize_t result, total = 0;
    int i;

   if (transport->num_to_write > MAX_WRITE_HUNKS) {
        LogMsg("dso_write_finish: fatal internal programming error: called %d times (more than limit of %d)", 
               transport->num_to_write, MAX_WRITE_HUNKS);
        dso_drop(transport->dso);
        return false;
    }

    // This is our ersatz scatter/gather I/O.
    for (i = 0; i < transport->num_to_write; i++) {
        result = mDNSPlatformWriteTCP(transport->connection, (const char *)transport->to_write[i], transport->write_lengths[i]);
        if (result != transport->write_lengths[i]) {
            if (result < 0) {
                LogMsg("dso_write_finish: fatal: mDNSPlatformWrite on %s returned %d", transport->dso->remote_name, errno);
            } else {
                LogMsg("dso_write_finish: fatal: mDNSPlatformWrite: short write on %s: %ld < %ld",
                       transport->dso->remote_name, (long)result, (long)total);
            }
            dso_drop(transport->dso);
            return false;
        }
    }
#endif
    return true;
}

// This function may only be called after a previous call to dso_write_start; it records the length of and
// pointer to the write buffer.  These buffers must remain valid until dso_write_finish() is called.  The
// caller is responsible for managing the memory they contain.  The expected control flow for writing is:
// dso_write_start(); dso_write(); dso_write(); dso_write(); dso_write_finished(); There should be one or
// more calls to dso_write; these will ideally be translated into a single scatter/gather sendmsg call (or
// equivalent) to the kernel.
void dso_write(dso_transport_t *transport, const uint8_t *buf, size_t length)
{
    if (length == 0) {
        return;
    }

#ifdef DSO_USES_NETWORK_FRAMEWORK
    if (transport->write_failed) {
        return;
    }
    dispatch_data_t dpd = dispatch_data_create(buf, length, dso_dispatch_queue,
                                               DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    if (dpd == NULL) {
        transport->write_failed = true;
        return;
    }
    if (transport->to_write != NULL) {
        dispatch_data_t dpc = dispatch_data_create_concat(transport->to_write, dpd);
        dispatch_release(dpd);
        dispatch_release(transport->to_write);
        if (dpc == NULL) {
            transport->to_write = NULL;
            transport->write_failed = true;
            return;
        }
        transport->to_write = dpc;
    }
#else
    // We'll report this in dso_write_finish();
    if (transport->num_to_write >= MAX_WRITE_HUNKS) {
        transport->num_to_write++;
        return;
    }

    transport->to_write[transport->num_to_write] = buf;
    transport->write_lengths[transport->num_to_write] = length;
    transport->num_to_write++;
#endif
}

// Write a DSO message
int dso_message_write(dso_state_t *dso, dso_message_t *msg, bool disregard_low_water)
{
    dso_transport_t *transport = dso->transport;
    if (transport->connection != NULL) {
        if (dso_write_start(transport, dso_message_length(msg)) || disregard_low_water) {
            dso_write(transport, msg->buf, msg->no_copy_bytes_offset);
            dso_write(transport, msg->no_copy_bytes, msg->no_copy_bytes_len);
            dso_write(transport, &msg->buf[msg->no_copy_bytes_offset], msg->cur - msg->no_copy_bytes_offset);
            return dso_write_finish(transport);
        }
    }
    return mStatus_NoMemoryErr;
}

// Replies to some message we were sent with a response code and no data.
// This is a convenience function for replies that do not require that a new
// packet be constructed.   It takes advantage of the fact that the message
// to which this is a reply is still in the input buffer, and modifies that
// message in place to turn it into a response.

bool dso_send_simple_response(dso_state_t *dso, int rcode, const DNSMessageHeader *header, const char *pres)
{
    dso_transport_t *transport = dso->transport;
    (void)pres; // might want this later.
    DNSMessageHeader response = *header;
    
    // Just return the message, with no questions, answers, etc.
    response.flags.b[1] = (response.flags.b[1] & ~kDNSFlag1_RC_Mask) | rcode;
    response.flags.b[0] |= kDNSFlag0_QR_Response;
    response.numQuestions = 0;
    response.numAnswers = 0;
    response.numAuthorities = 0;
    response.numAdditionals = 0;

    // Buffered write back to discovery proxy
    (void)dso_write_start(transport, 12);
    dso_write(transport, (uint8_t *)&response, 12);
    if (!dso_write_finish(transport)) {
        return false;
    }
    return true;
}

// DSO Message we received has a primary TLV that's not implemented.
// XXX is this what we're supposed to do here? check draft.
bool dso_send_not_implemented(dso_state_t *dso, const DNSMessageHeader *header)
{
    return dso_send_simple_response(dso, kDNSFlag1_RC_DSOTypeNI, header, "DSOTYPENI");
}

// Non-DSO message we received is refused.
bool dso_send_refused(dso_state_t *dso, const DNSMessageHeader *header)
{
    return dso_send_simple_response(dso, kDNSFlag1_RC_Refused, header, "REFUSED");
}

bool dso_send_formerr(dso_state_t *dso, const DNSMessageHeader *header)
{
    return dso_send_simple_response(dso, kDNSFlag1_RC_FormErr, header, "FORMERR");
}

bool dso_send_servfail(dso_state_t *dso, const DNSMessageHeader *header)
{
    return dso_send_simple_response(dso, kDNSFlag1_RC_ServFail, header, "SERVFAIL");
}

bool dso_send_name_error(dso_state_t *dso, const DNSMessageHeader *header)
{
    return dso_send_simple_response(dso, kDNSFlag1_RC_NXDomain, header, "NXDOMAIN");
}

bool dso_send_no_error(dso_state_t *dso, const DNSMessageHeader *header)
{
    return dso_send_simple_response(dso, kDNSFlag1_RC_NoErr, header, "NOERROR");
}

#ifdef DSO_USES_NETWORK_FRAMEWORK
static void dso_read_message(dso_transport_t *transport, size_t length);

static void dso_read_message_length(dso_transport_t *transport)
{
    const uint32_t serial = transport->dso->serial;
    if (transport->connection == NULL) {
        LogMsg("dso_read_message_length called with null connection.");
        return;
    }
    nw_connection_receive(transport->connection, 2, 2,
                          ^(dispatch_data_t content, nw_content_context_t __unused context,
                            bool __unused is_complete, nw_error_t error) {
                              dso_state_t *dso;
                              // Don't touch anything or look at anything until we have the lock.
                              KQueueLock();
                              dso = dso_find_by_serial(serial);
                              if (error != NULL) {
                                  LogMsg("dso_read_message_length: read failed: %s",
                                         strerror(nw_error_get_error_code(error)));
                              fail:
                                  if (dso != NULL) {
                                      mDNS_Lock(&mDNSStorage);
                                      dso_drop(dso);
                                      mDNS_Unlock(&mDNSStorage);
                                  }
                              } else if (content == NULL) {
                                  LogMsg("dso_read_message_length: remote end closed connection.");
                                  goto fail;
                              } else {
                                  size_t length;
                                  const uint8_t *lenbuf;
                                  dispatch_data_t map = dispatch_data_create_map(content, (const void **)&lenbuf, &length);
                                  if (map == NULL) {
                                      LogMsg("dso_read_message_length: map create failed");
                                      goto fail;
                                  } else if (length != 2) {
                                      LogMsg("dso_read_message_length: invalid length = %d", length);
                                      goto fail;
                                  }
                                  length = ((unsigned)(lenbuf[0]) << 8) | ((unsigned)lenbuf[1]);
                                  dso_read_message(transport, length);
                              }
                              KQueueUnlock("dso_read_message_length completion routine");
                          });
}

void dso_read_message(dso_transport_t *transport, size_t length)
{
    const uint32_t serial = transport->dso->serial;
    if (transport->connection == NULL) {
        LogMsg("dso_read_message called with null connection.");
        return;
    }
    nw_connection_receive(transport->connection, length, length,
                          ^(dispatch_data_t content, nw_content_context_t __unused context,
                            bool __unused is_complete, nw_error_t error) {
                              dso_state_t *dso;
                              // Don't touch anything or look at anything until we have the lock.
                              KQueueLock();
                              dso = dso_find_by_serial(serial);
                              if (error != NULL) {
                                  LogMsg("dso_read_message: read failed: %s", strerror(nw_error_get_error_code(error)));
                              fail:
                                  if (dso != NULL) {
                                      mDNS_Lock(&mDNSStorage);
                                      dso_drop(dso);
                                      mDNS_Unlock(&mDNSStorage);
                                  }
                              } else if (content == NULL) {
                                  LogMsg("dso_read_message: remote end closed connection");
                                  goto fail;
                              } else {
                                  size_t bytes_read;
                                  const uint8_t *message;
                                  dispatch_data_t map = dispatch_data_create_map(content, (const void **)&message, &bytes_read);
                                  if (map == NULL) {
                                      LogMsg("dso_read_message_length: map create failed");
                                      goto fail;
                                  } else if (bytes_read != length) {
                                      LogMsg("dso_read_message_length: only %d of %d bytes read", bytes_read, length);
                                      goto fail;
                                  }
                                  // Process the message.
                                  mDNS_Lock(&mDNSStorage);
                                  dns_message_received(dso, message, length);
                                  mDNS_Unlock(&mDNSStorage);

                                  // Now read the next message length.
                                  dso_read_message_length(transport);
                              }
                              KQueueUnlock("dso_read_message completion routine");
                          });
}
#else
// Called whenever there's data available on a DSO connection
void dso_read_callback(TCPSocket *sock, void *context, mDNSBool connection_established, int err)
{
    dso_transport_t *transport = context;
    dso_state_t *dso;
    mDNSBool closed = mDNSfalse;

    mDNS_Lock(&mDNSStorage);
    dso = transport->dso;

    // This shouldn't ever happen.
    if (err) {
        LogMsg("dso_read_callback: error %d", err);
        dso_drop(dso);
        goto out;
    }

    // Connection is already established by the time we set this up.
    if (connection_established) {
        goto out;
    }
    
    // This will be true either if we have never read a message or
    // if the last thing we did was to finish reading a message and
    // process it.
    if (transport->message_length == 0) {
        transport->need_length = true;
        transport->inbufp = transport->inbuf;
        transport->bytes_needed = 2;
    }
    
    // Read up to bytes_needed bytes.
    ssize_t count = mDNSPlatformReadTCP(sock, transport->inbufp, transport->bytes_needed, &closed);
    // LogMsg("read(%d, %p:%p, %d) -> %d", fd, dso->inbuf, dso->inbufp, dso->bytes_needed, count);
    if (count < 0) {
        LogMsg("dso_read_callback: read from %s returned %d", dso->remote_name, errno);
        dso_drop(dso);
        goto out;
    }

    // If we get selected for read and there's nothing to read, the remote end has closed the
    // connection.
    if (closed) {
        LogMsg("dso_read_callback: remote %s closed", dso->remote_name);
        dso_drop(dso);
        goto out;
    }
    
    transport->inbufp += count;
    transport->bytes_needed -= count;

    // If we read all the bytes we wanted, do what's next.
    if (transport->bytes_needed == 0) {
        // We just finished reading the complete length of a DNS-over-TCP message.
        if (transport->need_length) {
            // Get the number of bytes in this DNS message
            transport->bytes_needed = (((int)transport->inbuf[0]) << 8) | transport->inbuf[1];

            // Under no circumstances can length be zero.
            if (transport->bytes_needed == 0) {
                LogMsg("dso_read_callback: %s sent zero-length message.", dso->remote_name);
                dso_drop(dso);
                goto out;
            }

            // The input buffer size is AbsoluteMaxDNSMessageData, which is around 9000 bytes on
            // big platforms and around 1500 bytes on smaller ones.   If the remote end has sent
            // something larger than that, it's an error from which we can't recover.
            if (transport->bytes_needed > transport->inbuf_size - 2) {
                LogMsg("dso_read_callback: fatal: Proxy at %s sent a too-long (%ld bytes) message",
                       dso->remote_name, (long)transport->bytes_needed);
                dso_drop(dso);
                goto out;
            }

            transport->message_length = transport->bytes_needed;
            transport->inbufp = transport->inbuf + 2;
            transport->need_length = false;

        // We just finished reading a complete DNS-over-TCP message.
        } else {
            dns_message_received(dso, &transport->inbuf[2], transport->message_length);
            transport->message_length = 0;
        }
    }
out:
    mDNS_Unlock(&mDNSStorage);
}
#endif // DSO_USES_NETWORK_FRAMEWORK

#ifdef DSO_USES_NETWORK_FRAMEWORK
static dso_transport_t *dso_transport_create(nw_connection_t connection, bool is_server, void *context,
                                             int max_outstanding_queries, size_t outbuf_size_in, const char *remote_name,
                                             dso_event_callback_t cb, dso_state_t *dso)
{
    dso_transport_t *transport;
    uint8_t *transp;
    const size_t outbuf_size = outbuf_size_in + 256; // Space for additional TLVs

    // We allocate everything in a single hunk so that we can free it together as well.
    transp = mallocL("dso_transport_create", (sizeof *transport) + outbuf_size);
    if (transp == NULL) {
        transport = NULL;
        goto out;
    }
    // Don't clear the buffers.
    mDNSPlatformMemZero(transp, sizeof (*transport));

    transport = (dso_transport_t *)transp;
    transp += sizeof *transport;

    transport->outbuf = transp;
    transport->outbuf_size = outbuf_size;

    if (dso == NULL) {
        transport->dso = dso_create(is_server, max_outstanding_queries, remote_name, cb, context, transport);
        if (transport->dso == NULL) {
            mDNSPlatformMemFree(transport);
            transport = NULL;
            goto out;
        }
    } else {
        transport->dso = dso;
    }
    transport->connection = connection;
    nw_retain(transport->connection);
    transport->serial = dso_transport_serial++;

    transport->dso->transport = transport;
    transport->dso->transport_finalize = dso_transport_finalize;
    transport->next = dso_transport_states;
    dso_transport_states = transport;

    // Start looking for messages...
    dso_read_message_length(transport);
out:
    return transport;
}
#else
// Create a dso_transport_t structure
static dso_transport_t *dso_transport_create(TCPSocket *sock, bool is_server, void *context, int max_outstanding_queries,
                                             size_t inbuf_size_in, size_t outbuf_size_in, const char *remote_name,
                                             dso_event_callback_t cb, dso_state_t *dso)
{
    dso_transport_t *transport;
    size_t outbuf_size;
    size_t inbuf_size;
    uint8_t *transp;
    int status;

    // There's no point in a DSO that doesn't have a callback.
    if (!cb) {
        return NULL;
    }

    outbuf_size = outbuf_size_in + 256; // Space for additional TLVs
    inbuf_size = inbuf_size_in + 2;   // Space for length

    // We allocate everything in a single hunk so that we can free it together as well.
    transp = mallocL("dso_transport_create", (sizeof *transport) + inbuf_size + outbuf_size);
    if (transp == NULL) {
        transport = NULL;
        goto out;
    }
    // Don't clear the buffers.
    mDNSPlatformMemZero(transp, sizeof (*transport));

    transport = (dso_transport_t *)transp;
    transp += sizeof *transport;

    transport->inbuf = transp;
    transport->inbuf_size = inbuf_size;
    transp += inbuf_size;

    transport->outbuf = transp;
    transport->outbuf_size = outbuf_size;

    if (dso == NULL) {
        transport->dso = dso_create(is_server, max_outstanding_queries, remote_name, cb, context, transport);
        if (transport->dso == NULL) {
            mDNSPlatformMemFree(transport);
            transport = NULL;
            goto out;
        }
    } else {
        transport->dso = dso;
    }
    transport->connection = sock;

    status = mDNSPlatformTCPSocketSetCallback(sock, dso_read_callback, transport);
    if (status != mStatus_NoError) {
        LogMsg("dso_create: unable to set callback: %d", status);
        dso_drop(transport->dso);
        goto out;
    }

    transport->dso->transport = transport;
    transport->dso->transport_finalize = dso_transport_finalize;
    transport->next = dso_transport_states;
    dso_transport_states = transport;
out:
    return transport;
}
#endif // DSO_USES_NETWORK_FRAMEWORK

// This should all be replaced with Network Framework connection setup.
dso_connect_state_t *dso_connect_state_create(const char *hostname, mDNSAddr *addr, mDNSIPPort port,
                                              int max_outstanding_queries, size_t inbuf_size, size_t outbuf_size,
                                              dso_event_callback_t callback, dso_state_t *dso, void *context, const char *detail)
{
    int detlen = strlen(detail) + 1;
    int hostlen = hostname == NULL ? 0 : strlen(hostname) + 1;
    int len;
    dso_connect_state_t *cs;
    char *csp;
    char nbuf[INET6_ADDRSTRLEN + 1];
    dso_connect_state_t **states;

    // Enforce Some Minimums (Xxx these are a bit arbitrary, maybe not worth doing?)
    if (inbuf_size < MaximumRDSize || outbuf_size < 128 || max_outstanding_queries < 1) {
        return 0;
    }

    // If we didn't get a hostname, make a presentation form of the IP address to use instead.
    if (!hostlen) {
        if (addr != NULL) {
            if (addr->type == mDNSAddrType_IPv4) {
                hostname = inet_ntop(AF_INET, &addr->ip.v4, nbuf, sizeof nbuf);
            } else {
                hostname = inet_ntop(AF_INET6, &addr->ip.v6, nbuf, sizeof nbuf);
            }
            if (hostname != NULL) {
                hostlen = strlen(nbuf);
            }
        }
    }
    // If we don't have a printable name, we won't proceed, because this means we don't know
    // what to connect to.
    if (!hostlen) {
        return 0;
    }

    len = (sizeof *cs) + detlen + hostlen;
    csp = malloc(len);
    if (!csp) {
        return NULL;
    }
    cs = (dso_connect_state_t *)csp;
    memset(cs, 0, sizeof *cs);
    csp += sizeof *cs;

    cs->detail = csp;
    memcpy(cs->detail, detail, detlen);
    csp += detlen;
    cs->hostname = csp;
    memcpy(cs->hostname, hostname, hostlen);

    cs->config_port = port;
    cs->max_outstanding_queries = max_outstanding_queries;
    cs->outbuf_size = outbuf_size;
    if (context) {
        cs->context = context;
    } // else cs->context = NULL because of memset call above.
    cs->callback = callback;
    cs->connect_port.NotAnInteger = 0;
    cs->dso = dso;
#ifdef DSO_USES_NETWORK_FRAMEWORK
    cs->serial = dso_transport_serial++;
#else
    cs->inbuf_size = inbuf_size;
#endif

    if (addr) {
        cs->num_addrs = 1;
        cs->addresses[0] = *addr;
        cs->ports[0] = port;
    }
    for (states = &dso_connect_states; *states != NULL; states = &(*states)->next)
        ;
    *states = cs;
    return cs;
}

#ifdef DSO_USES_NETWORK_FRAMEWORK
void dso_connect_state_use_tls(dso_connect_state_t *cs)
{
    cs->tls_enabled = true;
}
#endif

void dso_connect_state_drop(dso_connect_state_t *cs)
{
    dso_connect_state_t **states;

    for (states = &dso_connect_states; *states != NULL && *states != cs; states = &(*states)->next)
        ;
    if (*states) {
        *states = cs->next;;
    } else {
        LogMsg("dso_connect_state_drop: dropping a connect state that isn't recognized.");
    }
#ifdef DSO_USES_NETWORK_FRAMEWORK
    if (cs->connection != NULL) {
        nw_connection_cancel(cs->connection);
        nw_release(cs->connection);
        cs->connection = NULL;
    }
#endif
    mDNSPlatformMemFree(cs);
}

#ifdef DSO_USES_NETWORK_FRAMEWORK
static void
dso_connection_succeeded(dso_connect_state_t *cs)
{
    // We got a connection.
    dso_transport_t *transport =
        dso_transport_create(cs->connection, false, cs->context, cs->max_outstanding_queries,
                             cs->outbuf_size, cs->hostname, cs->callback, cs->dso);
    nw_release(cs->connection);
    cs->connection = NULL;
    if (transport == NULL) {
        // If dso_transport_create fails, there's no point in continuing to try to connect to new
        // addresses
        LogMsg("dso_connection_succeeded: dso_create failed");
        // XXX we didn't retain the connection, so we're done when it goes out of scope, right?
    } else {
        // Call the "we're connected" callback, which will start things up.
        transport->dso->cb(cs->context, NULL, transport->dso, kDSOEventType_Connected);
    }
    
    cs->last_event = 0;

    // When the connection has succeeded, stop asking questions.
    if (cs->lookup != NULL) {
        mDNS *m = &mDNSStorage;
        DNSServiceRef ref = cs->lookup;
        cs->lookup = NULL;
        mDNS_DropLockBeforeCallback();
        DNSServiceRefDeallocate(ref);
        mDNS_ReclaimLockAfterCallback();
    }
    return;
}


static void dso_connect_internal(dso_connect_state_t *cs)
{
    uint32_t serial = cs->serial;

    cs->last_event = mDNSStorage.timenow;

    if (cs->num_addrs <= cs->cur_addr) {
        if (cs->lookup == NULL) {
            LogMsg("dso_connect_internal: %s: no more addresses to try", cs->hostname);
            cs->last_event = 0;
            cs->callback(cs->context, NULL, NULL, kDSOEventType_ConnectFailed);
        }
        // Otherwise, we will get more callbacks when outstanding queries either fail or succeed.
        return;
    }            
        
    char addrbuf[INET6_ADDRSTRLEN + 1];
    char portbuf[6];

    inet_ntop(cs->addresses[cs->cur_addr].type == mDNSAddrType_IPv4 ? AF_INET : AF_INET6,
              cs->addresses[cs->cur_addr].type == mDNSAddrType_IPv4
              ? (void *)cs->addresses[cs->cur_addr].ip.v4.b
              : (void *)cs->addresses[cs->cur_addr].ip.v6.b, addrbuf, sizeof addrbuf);
    snprintf(portbuf, sizeof portbuf, "%u", ntohs(cs->ports[cs->cur_addr].NotAnInteger));
    cs->cur_addr++;

    nw_endpoint_t endpoint = nw_endpoint_create_host(addrbuf, portbuf);
    if (endpoint == NULL) {
    nomem:
        LogMsg("dso_connect_internal: no memory creating connection.");
        return;
    }
    nw_parameters_t parameters = NULL;
    nw_parameters_configure_protocol_block_t configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
    if (cs->tls_enabled) {
        // This sets up a block that's called when we get a TLS connection and want to verify
        // the cert.   Right now we only support opportunistic security, which means we have
        // no way to validate the cert.   Future work: add support for validating the cert
        // using a TLSA record if one is present.
        configure_tls = ^(nw_protocol_options_t tls_options) {
            sec_protocol_options_t sec_options = nw_tls_copy_sec_protocol_options(tls_options);
            sec_protocol_options_set_verify_block(sec_options, 
                                                  ^(sec_protocol_metadata_t __unused metadata,
                                                    sec_trust_t __unused trust_ref,
                                                    sec_protocol_verify_complete_t complete) {
                                                      complete(true);
                                                  }, dso_dispatch_queue);
        };
    }
    parameters = nw_parameters_create_secure_tcp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);
    if (parameters == NULL) {
        goto nomem;
    }
    nw_connection_t connection = nw_connection_create(endpoint, parameters);
    if (connection == NULL) {
        goto nomem;
    }
    cs->connection = connection;

    LogMsg("dso_connect_internal: Attempting to connect to %s%%%s", addrbuf, portbuf);
    nw_connection_set_queue(connection, dso_dispatch_queue);
    nw_connection_set_state_changed_handler(
        connection, ^(nw_connection_state_t state, nw_error_t error) {
            dso_connect_state_t *ncs;
            KQueueLock();
            ncs = dso_connect_state_find(serial); // Might have been freed.
            if (ncs == NULL) {
                LogMsg("forgotten connection is %s.",
                       state == nw_connection_state_cancelled ? "canceled" :
                       state == nw_connection_state_failed ? "failed" :
                       state == nw_connection_state_waiting ? "canceled" :
                       state == nw_connection_state_ready ? "ready" : "unknown");
                if (state != nw_connection_state_cancelled) {
                    nw_connection_cancel(connection);
                    // Don't need to release it because only NW framework is holding a reference (XXX right?)
                }
            } else {
                if (state == nw_connection_state_waiting) {
                    LogMsg("connection to %#a%%%d is waiting", &ncs->addresses[ncs->cur_addr], ncs->ports[ncs->cur_addr]);

                    // XXX the right way to do this is to just let NW Framework wait until we get a connection,
                    // but there are a bunch of problems with that right now.   First, will we get "waiting" on
                    // every connection we try?   We aren't relying on NW Framework for DNS lookups, so we are
                    // connecting to an IP address, not a host, which means in principle that a later IP address
                    // might be reachable.   So we have to stop trying on this one to try that one.   Oops.
                    // Once we get NW Framework to use internal calls to resolve names, we can fix this.
                    // Second, maybe we want to switch to polling if this happens.   Probably not, but we need
                    // to think this through.   So right now we're just using the semantics of regular sockets,
                    // which we /have/ thought through.   So in the future we should do this think-through and
                    // try to use NW Framework as it's intended to work rather than as if it were just sockets.
                    ncs->connecting = mDNSfalse;
                    nw_connection_cancel(connection);
                } else if (state == nw_connection_state_failed) {
                    // We tried to connect, but didn't succeed.
                    LogMsg("dso_connect_internal: failed to connect to %s on %#a%%%d: %s%s",
                           ncs->hostname, &ncs->addresses[ncs->cur_addr], ncs->ports[ncs->cur_addr],
                           strerror(nw_error_get_error_code(error)), ncs->detail);
                    nw_release(ncs->connection);
                    ncs->connection = NULL;
                    ncs->connecting = mDNSfalse;
                    // This will do the work of figuring out if there are more addresses to try.
                    mDNS_Lock(&mDNSStorage);
                    dso_connect_internal(ncs);
                    mDNS_Unlock(&mDNSStorage);
                } else if (state == nw_connection_state_ready) {
                    ncs->connecting = mDNSfalse;
                    mDNS_Lock(&mDNSStorage);
                    dso_connection_succeeded(ncs);
                    mDNS_Unlock(&mDNSStorage);
                } else if (state == nw_connection_state_cancelled) {
                    if (ncs->connection) {
                        nw_release(ncs->connection);
                    }
                    ncs->connection = NULL;
                    ncs->connecting = mDNSfalse;
                    // If we get here and cs exists, we are still trying to connect.   So do the next step.
                    mDNS_Lock(&mDNSStorage);
                    dso_connect_internal(ncs);
                    mDNS_Unlock(&mDNSStorage);
                }
            }                
            KQueueUnlock("dso_connect_internal state change handler");
        });
    nw_connection_start(connection);
    cs->connecting = mDNStrue;
}

#else
static void dso_connect_callback(TCPSocket *sock, void *context, mDNSBool connected, int err)
{
    dso_connect_state_t *cs = context;
    char *detail;
    int status;
    dso_transport_t *transport;
    mDNS *m = &mDNSStorage;

    (void)connected;
    mDNS_Lock(m);
    detail = cs->detail;
    
    // If we had a socket open but the connect failed, close it and try the next address, if we have
    // a next address.
    if (sock != NULL) {
        cs->last_event = m->timenow;

        cs->connecting = mDNSfalse;
        if (err != mStatus_NoError) {
            mDNSPlatformTCPCloseConnection(sock);
            LogMsg("dso_connect_callback: connect %p failed (%d)", cs, err);
        } else {
        success:
            // We got a connection.
            transport = dso_transport_create(sock, false, cs->context, cs->max_outstanding_queries,
                                             cs->inbuf_size, cs->outbuf_size, cs->hostname, cs->callback, cs->dso);
            if (transport == NULL) {
                // If dso_create fails, there's no point in continuing to try to connect to new
                // addresses
            fail:
                LogMsg("dso_connect_callback: dso_create failed");
                mDNSPlatformTCPCloseConnection(sock);
            } else {
                // Call the "we're connected" callback, which will start things up.
                transport->dso->cb(cs->context, NULL, transport->dso, kDSOEventType_Connected);
            }

            cs->last_event = 0;

            // When the connection has succeeded, stop asking questions.
            if (cs->lookup != NULL) {
                DNSServiceRef ref = cs->lookup;
                cs->lookup = NULL;
                mDNS_DropLockBeforeCallback();
                DNSServiceRefDeallocate(ref);
                mDNS_ReclaimLockAfterCallback();
            }
            mDNS_Unlock(m);
            return;
        }
    }

    // If there are no addresses to connect to, and there are no queries running, then we can give
    // up.  Otherwise, we wait for one of the queries to deliver an answer.
    if (cs->num_addrs <= cs->cur_addr) {
        if (cs->lookup == NULL) {
            LogMsg("dso_connect_callback: %s: no more addresses to try", cs->hostname);
            cs->last_event = 0;
            cs->callback(cs->context, NULL, NULL, kDSOEventType_ConnectFailed);
        }
        // Otherwise, we will get more callbacks when outstanding queries either fail or succeed.
        mDNS_Unlock(m);
        return;
    }            
        
    sock = mDNSPlatformTCPSocket(kTCPSocketFlags_Zero, cs->addresses[cs->cur_addr].type, NULL, NULL, mDNSfalse);
    if (sock == NULL) {
        LogMsg("drConnectCallback: couldn't get a socket for %s: %s%s",
               cs->hostname, strerror(errno), detail);
        goto fail;
    }

    LogMsg("dso_connect_callback: Attempting to connect to %#a%%%d",
           &cs->addresses[cs->cur_addr], ntohs(cs->ports[cs->cur_addr].NotAnInteger));

    status = mDNSPlatformTCPConnect(sock, &cs->addresses[cs->cur_addr], cs->ports[cs->cur_addr], NULL,
                                    dso_connect_callback, cs);
    cs->cur_addr++;
    if (status == mStatus_NoError || status == mStatus_ConnEstablished) {
        // This can't happen in practice on MacOS; we don't know about all other operating systems,
        // so we handle it just in case.
        LogMsg("dso_connect_callback: synchronous connect to %s", cs->hostname);
        goto success;
    } else if (status == mStatus_ConnPending) {
        LogMsg("dso_connect_callback: asynchronous connect to %s", cs->hostname);
        cs->connecting = mDNStrue;
        // We should get called back when the connection succeeds or fails.
        mDNS_Unlock(m);
        return;
    }
    LogMsg("dso_connect_callback: failed to connect to %s on %#a%d: %s%s",
           cs->hostname, &cs->addresses[cs->cur_addr],
           ntohs(cs->ports[cs->cur_addr].NotAnInteger), strerror(errno), detail);
    mDNS_Unlock(m);
}

static void dso_connect_internal(dso_connect_state_t *cs)
{
    dso_connect_callback(NULL, cs, false, mStatus_NoError);
}
#endif // DSO_USES_NETWORK_FRAMEWORK

static void dso_inaddr_callback(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
                                DNSServiceErrorType errorCode, const char *fullname, const struct sockaddr *sa,
                                uint32_t ttl, void *context)
{
    dso_connect_state_t *cs = context;
    char addrbuf[INET6_ADDRSTRLEN + 1];
    mDNS *m = &mDNSStorage;
    (void)sdRef;

    cs->last_event = m->timenow;
    inet_ntop(sa->sa_family, (sa->sa_family == AF_INET
                              ? (void *)&((struct sockaddr_in *)sa)->sin_addr
                              : (void *)&((struct sockaddr_in6 *)sa)->sin6_addr), addrbuf, sizeof addrbuf);
    LogMsg("dso_inaddr_callback: %s: flags %x index %d error %d fullname %s addr %s ttl %lu",
           cs->hostname, flags, interfaceIndex, errorCode, fullname, addrbuf, (unsigned long)ttl);
    
    if (errorCode != mStatus_NoError) {
        return;
    }

    if (cs->num_addrs == MAX_DSO_CONNECT_ADDRS) {
        if (cs->cur_addr > 1) {
            memmove(&cs->addresses, &cs->addresses[cs->cur_addr],
                    (MAX_DSO_CONNECT_ADDRS - cs->cur_addr) * sizeof cs->addresses[0]);
            cs->num_addrs -= cs->cur_addr;
            cs->cur_addr = 0;
        } else {
            LogMsg("dso_inaddr_callback: ran out of room for addresses.");
            return;
        }
    }

    if (sa->sa_family == AF_INET) {
        cs->addresses[cs->num_addrs].type = mDNSAddrType_IPv4;
        mDNSPlatformMemCopy(&cs->addresses[cs->num_addrs].ip.v4,
                            &((struct sockaddr_in *)sa)->sin_addr, sizeof cs->addresses[cs->num_addrs].ip.v4);
    } else {
        cs->addresses[cs->num_addrs].type = mDNSAddrType_IPv6;
        mDNSPlatformMemCopy(&cs->addresses[cs->num_addrs].ip.v6,
                            &((struct sockaddr_in *)sa)->sin_addr, sizeof cs->addresses[cs->num_addrs].ip.v6);
    }

    cs->ports[cs->num_addrs] = cs->config_port;
    cs->num_addrs++;
    if (!cs->connecting) {
        LogMsg("dso_inaddr_callback: starting a new connection.");
        dso_connect_internal(cs);
    } else {
        LogMsg("dso_inaddr_callback: connection in progress, deferring new connect until it fails.");
    }
}

bool dso_connect(dso_connect_state_t *cs)
{
    struct in_addr in;
    struct in6_addr in6;

    // If the connection state was created with an address, use that rather than hostname.
    if (cs->num_addrs > 0) {
        dso_connect_internal(cs);
    }
    // Else allow an IPv4 address literal string
    else if (inet_pton(AF_INET, cs->hostname, &in)) {
        cs->num_addrs = 1;
        cs->addresses[0].type = mDNSAddrType_IPv4;
        cs->addresses[0].ip.v4.NotAnInteger = in.s_addr;
        cs->ports[0] = cs->config_port;
        dso_connect_internal(cs);
    }
    // ...or an IPv6 address literal string
    else if (inet_pton(AF_INET6, cs->hostname, &in6)) {
        cs->num_addrs = 1;
        cs->addresses[0].type = mDNSAddrType_IPv6;
        memcpy(&cs->addresses[0].ip.v6, &in6, sizeof in6);
        cs->ports[0] = cs->config_port;
        dso_connect_internal(cs);
    }
    // ...or else look it up.
    else {
        mDNS *m = &mDNSStorage;
        int err;
        mDNS_DropLockBeforeCallback();
        err = DNSServiceGetAddrInfo(&cs->lookup, kDNSServiceFlagsReturnIntermediates,
                                    kDNSServiceInterfaceIndexAny, 0, cs->hostname, dso_inaddr_callback, cs);

        mDNS_ReclaimLockAfterCallback();
        if (err != mStatus_NoError) {
            LogMsg("dso_connect: inaddr lookup query allocate failed for '%s': %d", cs->hostname, err);
            return false;
        }
    }
    return true;
}

#ifdef DSO_USES_NETWORK_FRAMEWORK
// We don't need this for DNS Push, so it is being left as future work.
int dso_listen(dso_connect_state_t * __unused listen_context)
{
    return mStatus_UnsupportedErr;
}

#else

// Called whenever we get a connection on the DNS TCP socket
static void dso_listen_callback(TCPSocket *sock, mDNSAddr *addr, mDNSIPPort *port,
                                const char *remote_name, void *context)
{
    dso_connect_state_t *lc = context;
    dso_transport_t *transport;

    mDNS_Lock(&mDNSStorage);
    transport = dso_transport_create(sock, mDNStrue, lc->context, lc->max_outstanding_queries,
                                     lc->inbuf_size, lc->outbuf_size, remote_name, lc->callback, NULL);
    if (transport == NULL) {
        mDNSPlatformTCPCloseConnection(sock);
        LogMsg("No memory for new DSO connection from %s", remote_name);
        goto out;
    }

    transport->remote_addr = *addr;
    transport->remote_port = ntohs(port->NotAnInteger);
    if (transport->dso->cb) {
        transport->dso->cb(lc->context, 0, transport->dso, kDSOEventType_Connected);
    }
    LogMsg("DSO connection from %s", remote_name);
out:
    mDNS_Unlock(&mDNSStorage);
}

// Listen for connections; each time we get a connection, make a new dso_state_t object with the specified
// parameters and call the callback.   Port can be zero to leave it unspecified.

int dso_listen(dso_connect_state_t *listen_context)
{
    char addrbuf[INET6_ADDRSTRLEN + 1];
    mDNSIPPort port;
    mDNSBool reuseAddr = mDNSfalse;
    
    if (listen_context->config_port.NotAnInteger) {
        port = listen_context->config_port;
        reuseAddr = mDNStrue;
    }
    listen_context->listener = mDNSPlatformTCPListen(mDNSAddrType_None, &port, NULL, kTCPSocketFlags_Zero,
                                                     reuseAddr, 5, dso_listen_callback, listen_context);
    if (!listen_context->listener) {
        return mStatus_UnknownErr;
    }
    listen_context->connect_port = port;
    if (listen_context->addresses[0].type == mDNSAddrType_IPv4) {
        inet_ntop(AF_INET, &listen_context->addresses[0].ip.v4, addrbuf, sizeof addrbuf);
    } else {
        inet_ntop(AF_INET6, &listen_context->addresses[0].ip.v6, addrbuf, sizeof addrbuf);
    }
    
    LogMsg("DSOListen: Listening on %s%%%d", addrbuf, ntohs(listen_context->connect_port.NotAnInteger));
    return mStatus_NoError;
}
#endif // DSO_USES_NETWORK_FRAMEWORK

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
