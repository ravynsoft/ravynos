/* dso.c
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

#include "DNSCommon.h"
#include "mDNSEmbeddedAPI.h"

#include "dso.h"

#ifdef STANDALONE
#undef LogMsg
#define LogMsg(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define mDNSRandom(x) arc4random_uniform(x)
#define mDNSPlatformMemAllocateClear(length) calloc(1, length)
#endif // STANDALONE

//*************************************************************************************************************
// Remaining work TODO

// - Add keepalive/inactivity timeout support
// - Notice if it takes a long time to get a response when establishing a session, and treat that
//   as "DSO not supported."
// - TLS support
// - Actually use Network Framework


//*************************************************************************************************************
// Globals

static dso_state_t *dso_connections;
static dso_state_t *dso_connections_needing_cleanup; // DSO connections that have been shut down but aren't yet freed.
static uint32_t dso_serial; // Used to uniquely mark DSO objects, incremented once for each dso_state_t created.

dso_state_t *dso_find_by_serial(uint32_t serial)
{
    dso_state_t *dsop;

    for (dsop = dso_connections; dsop; dsop = dsop->next) {
        if (dsop->serial == serial) {
            return dsop;
        }
    }
    return NULL;
}

// This function is called either when an error has occurred requiring the a DSO connection be
// dropped, or else when a connection to a DSO endpoint has been cleanly closed and is ready to be
// dropped for that reason.

void dso_drop(dso_state_t *dso)
{
    dso_state_t *dsop;
    
    if (dso_connections == dso) {
        dso_connections = dso->next;
    } else {
        for (dsop = dso_connections; dsop != NULL && dsop->next != dso; dsop = dsop->next) {
            LogMsg("dsop = %p dsop->next = %p dso = %p", dsop, dsop->next, dso);
        }
        if (dsop) {
            dsop->next = dso->next;
        // If we get to the end of the list without finding dso, it means that it's already
        // been dropped.
        } else {
            return;
        }
    }
    dso->next = dso_connections_needing_cleanup;
    dso_connections_needing_cleanup = dso;
}

int64_t dso_idle(void *context, int64_t now, int64_t next_timer_event)
{
    dso_state_t *dso, *dnext;
    dso_activity_t *ap, *anext;
    
    for (dso = dso_connections_needing_cleanup; dso; dso = dnext) {
        dnext = dso->next;
        // Finalize and then free any activities.
        for (ap = dso->activities; ap; ap = anext) {
            anext = ap->next;
            if (ap->finalize) {
                ap->finalize(ap);
            }
            free(ap);
        }
        if (dso->transport != NULL && dso->transport_finalize != NULL) {
            dso->transport_finalize(dso->transport);
            dso->transport = NULL;
        }
        if (dso->cb) {
            dso_disconnect_context_t disconnect_context;
            memset(&disconnect_context, 0, sizeof disconnect_context);
            dso->cb(dso->context, &disconnect_context, dso, kDSOEventType_Disconnected);
            dso->cb(dso->context, NULL, dso, kDSOEventType_Finalize);
        } else {
            free(dso);
        }
    }
    dso_connections_needing_cleanup = NULL;

    // Do keepalives.
    for (dso = dso_connections; dso; dso = dso->next) {
        if (dso->inactivity_due == 0) {
            if (dso->inactivity_timeout != 0) {
                dso->inactivity_due = now + dso->inactivity_timeout;
                if (next_timer_event - dso->keepalive_due > 0) {
                    next_timer_event = dso->keepalive_due;
                }
            }
        } else if (now - dso->inactivity_due > 0 && dso->cb != NULL) {
            dso->cb(dso->context, 0, dso, kDSOEventType_Inactive);
        }
        if (dso->keepalive_due != 0 && dso->keepalive_due < now && dso->cb != NULL) {
            dso_keepalive_context_t kc;
            memset(&kc, 0, sizeof kc);
            dso->cb(dso->context, &kc, dso, kDSOEventType_Keepalive);
            dso->keepalive_due = now + dso->keepalive_interval;
            if (next_timer_event - dso->keepalive_due > 0) {
                next_timer_event = dso->keepalive_due;
            }
        }
    }
    return dso_transport_idle(context, now, next_timer_event);
}

// Called when something happens that establishes a DSO session.
static void dso_session_established(dso_state_t *dso)
{
    dso->has_session = true;
    // Set up inactivity timer and keepalive timer...
}

// Create a dso_state_t structure
dso_state_t *dso_create(bool is_server, int max_outstanding_queries, const char *remote_name,
                        dso_event_callback_t callback, void *context, dso_transport_t *transport)
{
    dso_state_t *dso;
    int namelen = strlen(remote_name) + 1;
    int outsize = (sizeof (dso_outstanding_query_state_t)) + max_outstanding_queries * sizeof (dso_outstanding_query_t);

    // We allocate everything in a single hunk so that we can free it together as well.
    dso = (dso_state_t *) mDNSPlatformMemAllocateClear((sizeof *dso) + outsize + namelen);
    if (dso == NULL) {
        goto out;
    }
    dso->outstanding_queries = (dso_outstanding_query_state_t *)(dso + 1);
    dso->outstanding_queries->max_outstanding_queries = max_outstanding_queries;

    dso->remote_name = ((char *)dso->outstanding_queries) + outsize;
    memcpy(dso->remote_name, remote_name, namelen);
    dso->remote_name[namelen] = 0;

    dso->cb = callback;
    dso->context = context;
    dso->transport = transport;
    dso->is_server = is_server;
    dso->serial = dso_serial++;

    dso->next = dso_connections;
    dso_connections = dso;
out:
    return dso;
}

// Start building a TLV in an outgoing dso message.
void dso_start_tlv(dso_message_t *state, int opcode)
{
    // Make sure there's room for the length and the TLV opcode.
    if (state->cur + 4 >= state->max) {
        LogMsg("dso_start_tlv called when no space in output buffer!");
        assert(0);
    }

    // We need to not yet have a TLV.
    if (state->building_tlv) {
        LogMsg("dso_start_tlv called while already building a TLV!");
        assert(0);
    }
    state->building_tlv = true;
    state->tlv_len = 0;
    
    // Set up the TLV header.
    state->buf[state->cur] = opcode >> 8;
    state->buf[state->cur + 1] = opcode & 255;
    state->tlv_len_offset = state->cur + 2;
    state->cur += 4;
}

// Add some bytes to a TLV that's being built, but don't copy them--just remember the
// pointer to the buffer.   This is used so that when we have a message to forward, we
// don't copy it into the output buffer--we just use scatter/gather I/O.
void dso_add_tlv_bytes_no_copy(dso_message_t *state, const uint8_t *bytes, size_t len)
{
    if (!state->building_tlv) {
        LogMsg("add_tlv_bytes called when not building a TLV!");
        assert(0);
    }
    if (state->no_copy_bytes_len) {
        LogMsg("add_tlv_bytesNoCopy called twice on the same DSO message.");
        assert(0);
    }
    state->no_copy_bytes_len = len;
    state->no_copy_bytes = bytes;
    state->no_copy_bytes_offset = state->cur;
    state->tlv_len += len;
}

// Add some bytes to a TLV that's being built.
void dso_add_tlv_bytes(dso_message_t *state, const uint8_t *bytes, size_t len)
{
    if (!state->building_tlv) {
        LogMsg("add_tlv_bytes called when not building a TLV!");
        assert(0);
    }
    if (state->cur + len > state->max) {
        LogMsg("add_tlv_bytes called with no room in output buffer.");
        assert(0);
    }
    memcpy(&state->buf[state->cur], bytes, len);
    state->cur += len;
    state->tlv_len += len;
}

// Add a single byte to a TLV that's being built.
void dso_add_tlv_byte(dso_message_t *state, uint8_t byte)
{
    if (!state->building_tlv) {
        LogMsg("dso_add_tlv_byte called when not building a TLV!");
        assert(0);
    }
    if (state->cur + 1 > state->max) {
        LogMsg("dso_add_tlv_byte called with no room in output buffer.");
        assert(0);
    }
    state->buf[state->cur++] = byte;
    state->tlv_len++;
}

// Add an uint16_t to a TLV that's being built.
void dso_add_tlv_u16(dso_message_t *state, uint16_t u16)
{
    if (!state->building_tlv) {
        LogMsg("dso_add_tlv_u16 called when not building a TLV!");
        assert(0);
    }
    if ((state->cur + sizeof u16) > state->max) {
        LogMsg("dso_add_tlv_u16 called with no room in output buffer.");
        assert(0);
    }
    state->buf[state->cur++] = u16 >> 8;
    state->buf[state->cur++] = u16 & 255;
    state->tlv_len += 2;
}

// Add an uint32_t to a TLV that's being built.
void dso_add_tlv_u32(dso_message_t *state, uint32_t u32)
{
    if (!state->building_tlv) {
        LogMsg("dso_add_tlv_u32 called when not building a TLV!");
        assert(0);
    }
    if ((state->cur + sizeof u32) > state->max) {
        LogMsg("dso_add_tlv_u32 called with no room in output buffer.");
        assert(0);
    }
    state->buf[state->cur++] = u32 >> 24;
    state->buf[state->cur++] = (u32 >> 16) & 255;
    state->buf[state->cur++] = (u32 >> 8) & 255;
    state->buf[state->cur++] = u32 & 255;
    state->tlv_len += 4;
}

// Finish building a TLV.
void dso_finish_tlv(dso_message_t *state)
{
    if (!state->building_tlv) {
        LogMsg("dso_finish_tlv called when not building a TLV!");
        assert(0);
    }

    // A TLV can't be longer than this.
    if (state->tlv_len > 65535) {
        LogMsg("dso_finish_tlv was given more than 65535 bytes of TLV payload!");
        assert(0);
    }
    state->buf[state->tlv_len_offset] = state->tlv_len >> 8;
    state->buf[state->tlv_len_offset + 1] = state->tlv_len & 255;
    state->tlv_len = 0;
    state->building_tlv = false;
}

dso_activity_t *dso_find_activity(dso_state_t *dso, const char *name, const char *activity_type, void *context)
{
    dso_activity_t *activity;

    // If we haven't been given something to search for, don't search.
    if (name == NULL && context == NULL) {
        return NULL;
    }
        
    // An activity can be identified by name or context, but if name is present, that's what identifies it.
    for (activity = dso->activities; activity; activity = activity->next) {
        if (activity->activity_type == activity_type && ((activity->name == NULL || name == NULL|| !strcmp(activity->name, name)) &&
                                                         (context == NULL && context == activity->context))) {
            return activity;
        }
    }
    return NULL;
}

// Make an activity structure to hang off the DSO.
dso_activity_t *dso_add_activity(dso_state_t *dso, const char *name, const char *activity_type,
                                 void *context, void (*finalize)(dso_activity_t *))
{
    size_t namelen = name ? strlen(name) + 1 : 0;
    size_t len;
    dso_activity_t *activity;
    void *ap;

    // Shouldn't add an activity that's already been added.
    activity = dso_find_activity(dso, name, activity_type, context);
    if (activity != NULL) {
        LogMsg("dso_add_activity: activity %s%s%p added twice.", name ? name : "", name ? " " : "", context);
        return NULL;
    }

    len = namelen + sizeof *activity;
    ap = mDNSPlatformMemAllocateClear(len);
    if (ap == NULL) {
        return NULL;
    }
    activity = (dso_activity_t *)ap;
    ap = (char *)ap + sizeof *activity;

    // Activities can be identified either by name or by context
    if (namelen) {
        activity->name = ap;
        memcpy(activity->name, name, namelen);
    } else {
        activity->name = NULL;
    }
    activity->context = context;

    // Activity type is expected to be a string constant; all activities of the same type must
    // reference the same constant, not different constants with the same contents.
    activity->activity_type = activity_type;
    activity->finalize = finalize;

    // Retain this activity on the list.
    activity->next = dso->activities;
    dso->activities = activity;
    return activity;
}

void dso_drop_activity(dso_state_t *dso, dso_activity_t *activity)
{
    dso_activity_t **app = &dso->activities;
    bool matched = false;

    // Remove this activity from the list.
    while (*app) {
        if (*app == activity) {
            *app = activity->next;
            matched = true;
        } else {
            app = &((*app)->next);
        }
    }

    // If an activity that's not on the DSO list is passed here, it's an internal consistency
    // error that probably indicates something is corrupted.
    if (!matched) {
        LogMsg("dso_drop_activity: FATAL: activity that's not on the list has been dropped!");
        assert(0);
    }

    activity->finalize(activity);
    free(activity);
}

void dso_ignore_response(dso_state_t *dso, void *context)
{
    dso_outstanding_query_state_t *midState = dso->outstanding_queries;
    int i;
    for (i = 0; i < midState->max_outstanding_queries; i++) {
        // The query is still be outstanding, and we want to know it when it comes back, but we forget the context,
        // which presumably is a reference to something that's going away.
        if (midState->queries[i].context == context) {
            midState->queries[i].context = NULL;
        }
    }
}

bool dso_make_message(dso_message_t *state, uint8_t *outbuf, size_t outbuf_size,
                      dso_state_t *dso, bool unidirectional, void *callback_state)
{
    DNSMessageHeader *msg_header;
    dso_outstanding_query_state_t *midState = dso->outstanding_queries;

    memset(state, 0, sizeof *state);
    state->buf = outbuf;
    state->max = outbuf_size;

    // We need space for the TCP message length plus the DNS header.
    if (state->max < sizeof *msg_header) {
        LogMsg("dso_make_message: called without enough buffer space to store a DNS header!");
        assert(0);
    }

    // This buffer should be 16-bit aligned.
    msg_header = (DNSMessageHeader *)state->buf;
    
    // The DNS header for a DSO message is mostly zeroes
    memset(msg_header, 0, sizeof *msg_header);
    msg_header->flags.b[0] = kDNSFlag0_QR_Query | kDNSFlag0_OP_DSO;

    // Servers can't send DSO messages until there's a DSO session.
    if (dso->is_server && !dso->has_session) {
        LogMsg("dso_make_message: FATAL: server attempting to make a DSO message with no session!");
        assert(0);
    }

    // Response-requiring messages need to have a message ID.
    if (!unidirectional) {
        bool msg_id_ok = true;
        uint16_t message_id;
        int looping = 0;
        int i, avail = -1;

        // If we don't have room for another outstanding message, the caller should try
        // again later.
        if (midState->outstanding_query_count == midState->max_outstanding_queries) {
            return false;
        }
        // Generate a random message ID.   This doesn't really need to be cryptographically sound
        // (right?) because we're encrypting the whole data stream in TLS.
        do {
            // This would be a surprising fluke, but let's not get killed by it.
            if (looping++ > 1000) {
                return false;
            }
            message_id = mDNSRandom(65536);
            msg_id_ok = true;
            if (message_id == 0) {
                msg_id_ok = false;
            } else {
                for (i = 0; i < midState->max_outstanding_queries; i++) {
                    if (midState->queries[i].id == 0 && avail == -1) {
                        avail = i;
                    } else if (midState->queries[i].id == message_id) {
                        msg_id_ok = false;
                    }
                }
            }
        } while (!msg_id_ok);
        midState->queries[avail].id = message_id;
        midState->queries[avail].context = callback_state;
        midState->outstanding_query_count++;
        msg_header->id.NotAnInteger = message_id;
        state->outstanding_query_number = avail;
    } else {
        // Clients aren't allowed to send unidirectional messages until there's a session.
        if (!dso->has_session) {
            LogMsg("dso_make_message: FATAL: client making a DSO unidirectional message with no session!");
            assert(0);
        }
        state->outstanding_query_number = -1;
    }

    state->cur = sizeof *msg_header;
    return true;
}

size_t dso_message_length(dso_message_t *state)
{
    return state->cur + state->no_copy_bytes_len;
}

void dso_retry_delay(dso_state_t *dso, const DNSMessageHeader *header)
{
    dso_disconnect_context_t context;
    if (dso->cb) {
        memset(&context, 0, sizeof context);
        if (dso->primary.length != 4) {
            LogMsg("Invalid DSO Retry Delay length %d from %s", dso->primary.length, dso->remote_name);
            dso_send_formerr(dso, header);
            return;
        }
        memcpy(&context, dso->primary.payload, dso->primary.length);
        context.reconnect_delay = ntohl(context.reconnect_delay);
        dso->cb(dso->context, &context, dso, kDSOEventType_RetryDelay);
    }
}

void dso_keepalive(dso_state_t *dso, const DNSMessageHeader *header)
{
    dso_keepalive_context_t context;
    memset(&context, 0, sizeof context);
    if (dso->primary.length != 8) {
        LogMsg("Invalid DSO Keepalive length %d from %s", dso->primary.length, dso->remote_name);
        dso_send_formerr(dso, header);
        return;
    }
    memcpy(&context, dso->primary.payload, dso->primary.length);
    context.inactivity_timeout = ntohl(context.inactivity_timeout);
    context.keepalive_interval = ntohl(context.keepalive_interval);
    if (dso->is_server) {
        if (dso->cb) {
            if (dso->keepalive_interval < context.keepalive_interval) {
                context.keepalive_interval = dso->keepalive_interval;
            }
            if (dso->inactivity_timeout < context.inactivity_timeout) {
                context.inactivity_timeout = dso->inactivity_timeout;
            }
            dso->cb(dso->context, &context, dso, kDSOEventType_KeepaliveRcvd);
        }
    } else {
        if (dso->keepalive_interval > context.keepalive_interval) {
            dso->keepalive_interval = context.keepalive_interval;
        }
        if (dso->inactivity_timeout > context.inactivity_timeout) {
            dso->inactivity_timeout = context.inactivity_timeout;
        }
    }    
}

// We received a DSO message; validate it, parse it and, if implemented, dispatch it.
void dso_message_received(dso_state_t *dso, const uint8_t *message, size_t message_length)
{
    int i;
    size_t offset;
    const DNSMessageHeader *header = (const DNSMessageHeader *)message;
    int response = (header->flags.b[0] & kDNSFlag0_QR_Mask) == kDNSFlag0_QR_Response;
    dso_query_receive_context_t qcontext;

    if (message_length < 12) {
        LogMsg("dso_message_received: response too short: %ld bytes", (long)message_length);
        dso_drop(dso);
        goto out;
    }

    // See if we have sent a message for which a response is expected.
    if (response) {
        bool expected = false;
        
        // A zero ID on a response is not permitted.
        if (header->id.NotAnInteger == 0) {
            LogMsg("dso_message_received: response with id==0 received from %s", dso->remote_name);
            dso_drop(dso);
            goto out;
        }
        // It's possible for a DSO response to contain no TLVs, but if that's the case, the length
        // should always be twelve.
        if (message_length < 16 && message_length != 12) {
            LogMsg("dso_message_received: response with bogus length==%ld received from %s", (long)message_length, dso->remote_name);
            dso_drop(dso);
            goto out;
        }
        for (i = 0; i < dso->outstanding_queries->max_outstanding_queries; i++) {
            if (dso->outstanding_queries->queries[i].id == header->id.NotAnInteger) {
                qcontext.query_context = dso->outstanding_queries->queries[i].context;
                qcontext.rcode = header->flags.b[1] & kDNSFlag1_RC_Mask;
                
                // If we are a client, and we just got an acknowledgment, a session has been established.
                if (!dso->is_server && !dso->has_session && (header->flags.b[1] & kDNSFlag1_RC_Mask) == kDNSFlag1_RC_NoErr) {
                    dso_session_established(dso);
                }
                dso->outstanding_queries->queries[i].id = 0;
                dso->outstanding_queries->queries[i].context = 0;
                dso->outstanding_queries->outstanding_query_count--;
                if (dso->outstanding_queries->outstanding_query_count < 0) {
                    LogMsg("dso_message_receive: programming error: outstanding_query_count went negative.");
                    assert(0);
                }
                // If there were no TLVs, we don't need to parse them.
                expected = true;
                if (message_length == 12) {
                    dso->primary.opcode = 0;
                    dso->primary.length = 0;
                    dso->num_additls = 0;
                }
                break;
            }
        }

        // This is fatal because we've received a response to a message we didn't send, so
        // it's not just that we don't understand what was sent.
        if (!expected) {
            LogMsg("dso_message_received: fatal: %s sent %ld byte message, QR=1", dso->remote_name, (long)message_length);
            dso_drop(dso);
            goto out;
        }
    }

    // Make sure that the DNS header is okay (QDCOUNT, ANCOUNT, NSCOUNT and ARCOUNT are all zero)
    for (i = 0; i < 4; i++) {
        if (message[4 + i * 2] != 0 || message[4 + i * 2 + 1] != 0) {
            LogMsg("dso_message_received: fatal: %s sent %ld byte DSO message, %s is nonzero",
                   dso->remote_name, (long)message_length,
                   (i == 0 ? "QDCOUNT" : (i == 1 ? "ANCOUNT" : ( i == 2 ? "NSCOUNT" : "ARCOUNT"))));
            dso_drop(dso);
            goto out;
        }
    }

    // Check that there is space for there to be a primary TLV
    if (message_length < 16 && message_length != 12) {
        LogMsg("dso_message_received: fatal: %s sent short (%ld byte) DSO message",
               dso->remote_name, (long)message_length);

        // Short messages are a fatal error. XXX check DSO document
        dso_drop(dso);
        goto out;
    }
    
    // If we are a server, and we don't have a session, and this is a message, then we have now established a session.
    if (!dso->has_session && dso->is_server && !response) {
        dso_session_established(dso);
    }

    // If a DSO session isn't yet established, make sure the message is a request (if is_server) or a
    // response (if not).
    if (!dso->has_session && ((dso->is_server && response) || (!dso->is_server && !response))) {
        LogMsg("dso_message_received: received a %s with no established session from %s",
               response ? "response" : "request", dso->remote_name);
        dso_drop(dso);
    }

    // Get the primary TLV and count how many TLVs there are in total
    offset = 12;
    while (offset < message_length) {
        // Get the TLV opcode
        int opcode = (((unsigned)message[offset]) << 8) + message[offset + 1];
        // And the length
        size_t length = (((unsigned)message[offset + 2]) << 8) + message[offset + 3];

        // Is there room for the contents of this TLV?
        if (length + offset > message_length) {
            LogMsg("dso_message_received: fatal: %s: TLV (%d %ld) extends past end (%ld)",
                   dso->remote_name, opcode, (long)length, (long)message_length);

            // Short messages are a fatal error. XXX check DSO document
            dso_drop(dso);
            goto out;
        }

        // Is this the primary TLV?
        if (offset == 12) {
            dso->primary.opcode = opcode;
            dso->primary.length = length;
            dso->primary.payload = &message[offset + 4];
            dso->num_additls = 0;
        } else {
            if (dso->num_additls < MAX_ADDITLS) {
                dso->additl[dso->num_additls].opcode = opcode;
                dso->additl[dso->num_additls].length = length;
                dso->additl[dso->num_additls].payload = &message[offset + 4];
                dso->num_additls++;
            } else {
                // XXX MAX_ADDITLS should be enough for all possible additional TLVs, so this
                // XXX should never happen; if it does, maybe it's a fatal error.
                LogMsg("dso_message_received: %s: ignoring additional TLV (%d %ld) in excess of %d",
                       dso->remote_name, opcode, (long)length, MAX_ADDITLS);
            }
        }
        offset += 4 + length;
    }

    // Call the callback with the message or response
    if (dso->cb) {
        if (message_length != 12 && dso->primary.opcode == kDSOType_Keepalive) {
            dso_keepalive(dso, header);
        } else if (message_length != 12 && dso->primary.opcode == kDSOType_RetryDelay) {
            dso_retry_delay(dso, header);
        } else {
            if (response) {
                dso->cb(dso->context, &qcontext, dso, kDSOEventType_DSOResponse);
            } else {
                dso->cb(dso->context, header, dso, kDSOEventType_DSOMessage);
            }
        }
    }
out:
    ;
}

// This code is currently assuming that we won't get a DNS message, but that's not true.   Fix.
void dns_message_received(dso_state_t *dso, const uint8_t *message, size_t message_length)
{
    DNSMessageHeader *header;
    int opcode, response;

    // We can safely assume that the header is 16-bit aligned.
    header = (DNSMessageHeader *)message;
    opcode = header->flags.b[0] & kDNSFlag0_OP_Mask;
    response = (header->flags.b[0] & kDNSFlag0_QR_Mask) == kDNSFlag0_QR_Response;

    // Validate the length of the DNS message.
    if (message_length < 12) {
        LogMsg("dns_message_received: fatal: %s sent short (%ld byte) message",
               dso->remote_name, (long)message_length);

        // Short messages are a fatal error.
        dso_drop(dso);
        return;
    }
    
    // This is not correct for the general case.
    if (opcode != kDNSFlag0_OP_DSO) {
        LogMsg("dns_message_received: %s sent %ld byte %s, QTYPE=%d",
               dso->remote_name, (long)message_length, (response ? "response" : "request"), opcode);
        if (dso->cb) {
            dso->cb(dso->context, header, dso,
                    response ? kDSOEventType_DNSMessage : kDSOEventType_DNSResponse);
        }
    } else {
        dso_message_received(dso, message, message_length);
    }
}

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
