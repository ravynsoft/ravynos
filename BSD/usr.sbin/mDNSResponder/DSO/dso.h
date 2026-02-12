/* dso.h
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

#ifndef __DSO_H
#define __DSO_H

#include <stdbool.h>
#include <stdint.h>

// Maximum number of additional TLVs we support in a DSO message.
#define MAX_ADDITLS           10

typedef enum {
    kDSOType_Keepalive = 1,
    kDSOType_RetryDelay = 2,
    kDSOType_EncryptionPadding = 3,
    kDSOType_DNSPushSubscribe = 0x40,
    kDSOType_DNSPushUpdate = 0x41,
    kDSOType_DNSPushUnsubscribe = 0x42,
    kDSOType_DNSPushReconfirm = 0x43,
    kDSOType_mDNSLinkRequest = 0xF901,
    kDSOType_mDNSLinkDiscontinue = 0xF902,
    kDSOType_mDNSMessage = 0xF903,
    kDSOType_LinkIdentifier = 0xF904,
    kDSOType_L2SourceAddress = 0xF905,
    kDSOType_IPSourceAddress = 0xF906,
    kDSOType_mDNSReportLinkChanges = 0xF907,
    kDSOType_mDNSStopLinkChanges = 0xF908,
    kDSOType_mDNSLinkAvailable = 0xF900,
    kDSOType_mDNSLinkUnavailable = 0xF90a,
    kDSOType_LinkPrefix = 0xf90b
} dso_message_types_t;

// When a DSO message arrives, or one that was sent is acknowledged, or the state of the DSO connection
// changes, we need to call the user of the DSO connection.
typedef enum {
    kDSOEventType_DNSMessage,      // A DNS message that is not a DSO message
    kDSOEventType_DNSResponse,     // A DNS response that is not a DSO response
    kDSOEventType_DSOMessage,      // DSOState.primary and DSOState.additl will contain the message TLVs;
                                   // header will contain the DNS header
    kDSOEventType_Finalize,        // The DSO connection to the other DSO endpoint has terminated and we are
                                   // in the idle loop.
    kDSOEventType_DSOResponse,     // DSOState.primary and DSOState.additl contain any TLVs in the response;
                                   // header contains the DNS header
    kDSOEventType_Connected,       // We succeeded in making a connection
    kDSOEventType_ConnectFailed,   // We failed to get a connection
    kDSOEventType_Disconnected,    // We were connected, but have disconnected or been disconnected
    kDSOEventType_ShouldReconnect, // We are disconnected, and a scheduled reconnect timer has gone off.
    							   // Recipient is responsible for reconnecting, or deciding not to.
    kDSOEventType_Inactive,		   // We went inactive and the inactivity timeout expired, so it's time to drop the connection.
    kDSOEventType_Keepalive,       // It's time to send a keepalive message, here are the values to send
    kDSOEventType_KeepaliveRcvd,   // We just received a keepalive from a client, here are the values.
    kDSOEventType_RetryDelay       // We got a RetryDelay from the server.   Have to shut down.
} dso_event_type_t;

typedef struct dso_outstanding_query {
    uint16_t id;
    void *context;
} dso_outstanding_query_t;

typedef struct dso_outstanding_query_state {
    int outstanding_query_count;
    int max_outstanding_queries;
    dso_outstanding_query_t queries[0];
} dso_outstanding_query_state_t;

typedef struct dso_query_receive_context {
    void *query_context;
    uint16_t rcode;
} dso_query_receive_context_t;

typedef struct dso_disconnect_context {
    uint32_t reconnect_delay;
} dso_disconnect_context_t;

typedef struct dso_keepalive_context {
    uint32_t inactivity_timeout;
    uint32_t keepalive_interval;
} dso_keepalive_context_t;

// Structure to represent received DSO TLVs
typedef struct dsotlv {
    uint16_t opcode;
    uint16_t length;
    const uint8_t *payload;
} dso_tlv_t;

// DSO message under construction
typedef struct dso_message {
    uint8_t *buf;                 // The buffer in which we are constructing the message
    size_t max;                   // Size of the buffer
    size_t cur;                   // Current position in the buffer
    bool building_tlv;            // True if we have started and not finished building a TLV
    int outstanding_query_number; // Number of the outstanding query state entry for this message, or -1
    size_t tlv_len;               // Current length of the TLV we are building.
    size_t tlv_len_offset;        // Where to store the length of the current TLV when finished.
    const uint8_t *no_copy_bytes; // One TLV can have data that isn't copied into the buffer
    size_t no_copy_bytes_len;     // Length of that data, if any.
    size_t no_copy_bytes_offset;  // Where in the buffer the data should be interposed.
} dso_message_t;

// Record of ongoing activity
typedef struct dso_activity dso_activity_t;
struct dso_activity {
    dso_activity_t *next;
    void (*finalize)(dso_activity_t *activity);
    const char *activity_type;  // Name of the activity type, must be the same pointer for all activities of a type.
    void *context;              // Activity implementation's context (if any).
    char *name;                 // Name of the individual activity
};

typedef struct dso_transport dso_transport_t;
typedef struct dso_state dso_state_t;
typedef int64_t event_time_t;

typedef void (*dso_event_callback_t)(void *context, const void *header,
                                     dso_state_t *dso, dso_event_type_t eventType);
typedef void (*dso_transport_finalize_t)(dso_transport_t *transport);

// DNS Stateless Operations state
struct dso_state {
    dso_state_t *next;
    void *context;                   // The context of the next layer up (e.g., a Discovery Proxy)
    dso_event_callback_t cb;         // Called when an event happens

    // Transport state; handled separately for reusability
    dso_transport_t *transport;		 // The transport (e.g., dso-transport.c or other).
    dso_transport_finalize_t transport_finalize;

    uint32_t serial;                 // Unique serial number which can be used after the DSO has been dropped.
    bool is_server;                  // True if the endpoint represented by this DSO state is a server
                                     // (according to the DSO spec)
    bool has_session;                // True if DSO session establishment has happened for this DSO endpoint
    event_time_t response_awaited;   // If we are waiting for a session-establishing response, when it's
                                     // expected; otherwise zero.
    uint32_t keepalive_interval;     // Time between keepalives (to be sent, on client, expected, on server)
    uint32_t inactivity_timeout;     // Session can't be inactive more than this amount of time.
    event_time_t keepalive_due;      // When the next keepalive is due (to be received or sent)
    event_time_t inactivity_due;     // When next activity has to happen for connection to remain active
    dso_activity_t *activities;      // Outstanding DSO activities.

    dso_tlv_t primary;               // Primary TLV for current message
    dso_tlv_t additl[MAX_ADDITLS];   // Additional TLVs
    int num_additls;                 // Number of additional TLVs in this message

    char *remote_name;

    dso_outstanding_query_state_t *outstanding_queries;
};

// Provided by dso.c
dso_state_t *dso_create(bool is_server, int max_outstanding_queries, const char *remote_name,
                        dso_event_callback_t callback, void *context, dso_transport_t *transport);
dso_state_t *dso_find_by_serial(uint32_t serial);
void dso_drop(dso_state_t *dso);
int64_t dso_idle(void *context, int64_t now, int64_t next_timer_event);
void dso_release(dso_state_t **dsop);
void dso_start_tlv(dso_message_t *state, int opcode);
void dso_add_tlv_bytes(dso_message_t *state, const uint8_t *bytes, size_t len);
void dso_add_tlv_bytes_no_copy(dso_message_t *state, const uint8_t *bytes, size_t len);
void dso_add_tlv_byte(dso_message_t *state, uint8_t byte);
void dso_add_tlv_u16(dso_message_t *state, uint16_t u16);
void dso_add_tlv_u32(dso_message_t *state, uint32_t u32);
void dso_finish_tlv(dso_message_t *state);
dso_activity_t *dso_find_activity(dso_state_t *dso, const char *name, const char *activity_type, void *context);
dso_activity_t *dso_add_activity(dso_state_t *dso, const char *name, const char *activity_type,
                                            void *context, void (*finalize)(dso_activity_t *));
void dso_drop_activity(dso_state_t *dso, dso_activity_t *activity);
void dso_ignore_response(dso_state_t *dso, void *context);
bool dso_make_message(dso_message_t *state, uint8_t *outbuf, size_t outbuf_size,
                      dso_state_t *dso, bool unidirectional, void *callback_state);
size_t dso_message_length(dso_message_t *state);
void dso_retry_delay(dso_state_t *dso, const DNSMessageHeader *header);
void dso_keepalive(dso_state_t *dso, const DNSMessageHeader *header);
void dso_message_received(dso_state_t *dso, const uint8_t *message, size_t message_length);
void dns_message_received(dso_state_t *dso, const uint8_t *message, size_t message_length);

// Provided by DSO transport implementation for use by dso.c:
int64_t dso_transport_idle(void *context, int64_t now, int64_t next_timer_event);
bool dso_send_simple_response(dso_state_t *dso, int rcode, const DNSMessageHeader *header, const char *pres);
bool dso_send_not_implemented(dso_state_t *dso, const DNSMessageHeader *header);
bool dso_send_refused(dso_state_t *dso, const DNSMessageHeader *header);
bool dso_send_formerr(dso_state_t *dso, const DNSMessageHeader *header);
bool dso_send_servfail(dso_state_t *dso, const DNSMessageHeader *header);
bool dso_send_name_error(dso_state_t *dso, const DNSMessageHeader *header);
bool dso_send_no_error(dso_state_t *dso, const DNSMessageHeader *header);
#endif // !defined(__DSO_H)

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
