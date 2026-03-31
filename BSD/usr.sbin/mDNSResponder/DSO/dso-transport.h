/* dso-transport.h
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

#ifndef __DSO_TRANSPORT_H
#define __DSO_TRANSPORT_H

#ifdef DSO_USES_NETWORK_FRAMEWORK
#include <Network/Network.h>
#endif

// Maximum number of IP addresses that we'll deal with as a result of looking up a name
// to which to connect.
#define MAX_DSO_CONNECT_ADDRS 16

// Threshold above which we indicate that a DSO connection isn't writable.   This is advisory,
// but e.g. for a Discovery Relay, if the remote proxy isn't consuming what we are sending, we
// should start dropping packets on the floor rather than just queueing more and more packets.
// 60k may actually be too much.   This is used when we're using NW Framework, because it doesn't
// allow us to use TCP_NOTSENT_LOWAT directly.
#define MAX_UNSENT_BYTES 60000

struct dso_transport {
    dso_state_t *dso;			   // DSO state for which this is the transport 
    struct dso_transport *next;    // Transport is on list of transports.
    void *event_context;           // I/O event context
    mDNSAddr remote_addr;          // The IP address to which we have connected
    int remote_port;               // The port to which we have connected

#ifdef DSO_USES_NETWORK_FRAMEWORK
    nw_connection_t connection;
    dispatch_data_t to_write;
    size_t bytes_to_write;
    size_t unsent_bytes;
    uint32_t serial;               // Serial number for locating possibly freed dso_transport_t structs
    bool write_failed;             // This is set if any of the parts of the dso_write process fail
#else
    TCPSocket *connection;         // Socket connected to Discovery Proxy
    size_t bytes_needed;
    size_t message_length;         // Length of message we are currently accumulating, if known
    uint8_t *inbuf;                // Buffer for incoming messages.
    size_t inbuf_size;
    uint8_t *inbufp;               // Current read pointer (may not be in inbuf)
    bool need_length;              // True if we need a 2-byte length

    uint8_t lenbuf[2];             // Buffer for storing the length in a DNS TCP message

#define MAX_WRITE_HUNKS 4          // When writing a DSO message, we need this many separate hunks.
    const uint8_t *to_write[MAX_WRITE_HUNKS];
    ssize_t write_lengths[MAX_WRITE_HUNKS];
    int num_to_write;
#endif // DSO_USES_NETWORK_FRAMEWORK

    uint8_t *outbuf;               // Output buffer for building and sending DSO messages
    size_t outbuf_size;
};

typedef struct dso_lookup dso_lookup_t;
struct dso_lookup {
    dso_lookup_t *next;
    DNSServiceRef sdref;
};

typedef struct dso_connect_state dso_connect_state_t;
struct dso_connect_state {
    dso_connect_state_t *next;
    dso_event_callback_t callback;
    dso_state_t *dso;
    char *detail;
    void *context;
    TCPListener *listener;

    char *hostname;
    int num_addrs;
    int cur_addr;
    mDNSAddr addresses[MAX_DSO_CONNECT_ADDRS];
    mDNSIPPort ports[MAX_DSO_CONNECT_ADDRS];
    DNSServiceRef lookup;

    mDNSBool connecting;
    mDNSIPPort config_port, connect_port;
#ifdef DSO_USES_NETWORK_FRAMEWORK
    uint32_t serial;
    nw_connection_t connection;
    bool tls_enabled;
#else
    size_t inbuf_size;
#endif
    size_t outbuf_size;
    int max_outstanding_queries;
    mDNSs32 last_event;
    mDNSs32 reconnect_time;
};

typedef struct {
	TCPListener *listener;
    dso_event_callback_t callback;
	void *context;
} dso_listen_context_t;

void dso_transport_init(void);
mStatus dso_set_connection(dso_state_t *dso, TCPSocket *socket);
void dso_schedule_reconnect(mDNS *m, dso_connect_state_t *cs, mDNSs32 when);
void dso_set_callback(dso_state_t *dso, void *context, dso_event_callback_t cb);
mStatus dso_message_write(dso_state_t *dso, dso_message_t *msg, bool disregard_low_water);
dso_connect_state_t *dso_connect_state_create(const char *host, mDNSAddr *addr, mDNSIPPort port,
                                              int num_outstanding_queries,
                                              size_t inbuf_size, size_t outbuf_size,
                                              dso_event_callback_t callback,
                                              dso_state_t *dso, void *context, const char *detail);
#ifdef DSO_USES_NETWORK_FRAMEWORK
void dso_connect_state_use_tls(dso_connect_state_t *cs);
#endif
void dso_connect_state_drop(dso_connect_state_t *cs);
bool dso_connect(dso_connect_state_t *connect_state);
mStatus dso_listen(dso_connect_state_t *listen_context);
bool dso_write_start(dso_transport_t *transport, size_t length);
bool dso_write_finish(dso_transport_t *transport);
void dso_write(dso_transport_t *transport, const uint8_t *buf, size_t length);
#endif // __DSO_TRANSPORT_H

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
