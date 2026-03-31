/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2006-2018 Apple Inc. All rights reserved.
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
 */


#ifndef _dnsextd_h
#define _dnsextd_h


#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"
#include "GenLinkedList.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define LLQ_TABLESIZE   1024    // !!!KRS make this dynamically growable


typedef enum DNSZoneSpecType
{
    kDNSZonePublic,
    kDNSZonePrivate
} DNSZoneSpecType;


typedef struct DNSZone
{
    domainname name;
    DNSZoneSpecType type;
    DomainAuthInfo      *   updateKeys; // linked list of keys for signing deletion updates
    DomainAuthInfo      *   queryKeys;  // linked list of keys for queries
    struct DNSZone      *   next;
} DNSZone;


typedef struct
{
    struct sockaddr_in src;
    size_t len;
    DNSZone * zone;
    mDNSBool isZonePublic;
    DNSMessage msg;
    // Note: extra storage for oversized (TCP) messages goes here
} PktMsg;

// lease table entry
typedef struct RRTableElem
{
    struct RRTableElem *next;
    struct sockaddr_in cli;   // client's source address
    long expire;              // expiration time, in seconds since epoch
    domainname zone;          // from zone field of update message
    domainname name;          // name of the record
    CacheRecord rr;           // last field in struct allows for allocation of oversized RRs
} RRTableElem;

typedef enum
{
    RequestReceived = 0,
    ChallengeSent   = 1,
    Established     = 2
} LLQState;

typedef struct AnswerListElem
{
    struct AnswerListElem *next;
    domainname name;
    mDNSu16 type;
    CacheRecord *KnownAnswers;  // All valid answers delivered to client
    CacheRecord *EventList;     // New answers (adds/removes) to be sent to client
    int refcount;
    mDNSBool UseTCP;            // Use TCP if UDP would cause truncation
    pthread_t tid;              // Allow parallel list updates
} AnswerListElem;

// llq table entry
typedef struct LLQEntry
{
    struct LLQEntry *next;
    struct sockaddr_in cli;   // clien'ts source address
    domainname qname;
    mDNSu16 qtype;
    mDNSOpaque64 id;
    LLQState state;
    mDNSu32 lease;            // original lease, in seconds
    mDNSs32 expire;           // expiration, absolute, in seconds since epoch
    AnswerListElem *AnswerList;
} LLQEntry;


typedef void (*EventCallback)( void * context );

typedef struct EventSource
{
    EventCallback callback;
    void                *   context;
    TCPSocket *         sock;
    int fd;
    mDNSBool markedForDeletion;
    struct  EventSource *   next;
} EventSource;


// daemon-wide information
typedef struct
{
    // server variables - read only after initialization (no locking)
    struct sockaddr_in addr;            // the address we will bind to
    struct sockaddr_in llq_addr;        // the address we will receive llq requests on.
    struct sockaddr_in ns_addr;         // the real ns server address
    int tcpsd;                          // listening TCP socket for dns requests
    int udpsd;                          // listening UDP socket for dns requests
    int tlssd;                          // listening TCP socket for private browsing
    int llq_tcpsd;                      // listening TCP socket for llq service
    int llq_udpsd;                      // listening UDP socket for llq service
    DNameListElem   *   public_names;   // list of public SRV names
    DNSZone         *   zones;

    // daemon variables - read only after initialization (no locking)
    mDNSIPPort private_port;           // listening port for private messages
    mDNSIPPort llq_port;           // listening port for llq

    // lease table variables (locked via mutex after initialization)
    RRTableElem **table;       // hashtable for records with leases
    pthread_mutex_t tablelock; // mutex for lease table
    mDNSs32 nbuckets;          // buckets allocated
    mDNSs32 nelems;            // elements in table

    // LLQ table variables
    LLQEntry *LLQTable[LLQ_TABLESIZE];  // !!!KRS change this and RRTable to use a common data structure
    AnswerListElem *AnswerTable[LLQ_TABLESIZE];
    int AnswerTableCount;
    int LLQEventNotifySock;          // Unix domain socket pair - update handling thread writes to EventNotifySock, which wakes
    int LLQEventListenSock;          // the main thread listening on EventListenSock, indicating that the zone has changed

    GenLinkedList eventSources;     // linked list of EventSource's
} DaemonInfo;


int
ParseConfig
(
    DaemonInfo  *   d,
    const char  *   file
);


#endif
