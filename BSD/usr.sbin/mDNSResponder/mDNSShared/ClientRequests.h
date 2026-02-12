/*
 * Copyright (c) 2018-2019 Apple Inc. All rights reserved.
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

#ifndef __ClientRequests_h
#define __ClientRequests_h

#include "mDNSEmbeddedAPI.h"
#include "dns_sd_internal.h"

typedef void (*QueryRecordResultHandler)(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord,
    DNSServiceErrorType error, void *context);

typedef struct
{
    DNSQuestion                 q;                      // DNSQuestion for record query.
    domainname *                qname;                  // Name of the original record.
    mDNSInterfaceID             interfaceID;            // Interface over which to perform query.
    QueryRecordResultHandler    resultHandler;          // Handler for query record operation results.
    void *                      resultContext;          // Context to pass to result handler.
    mDNSu32                     reqID;                  // 
    int                         searchListIndex;        // Index that indicates the next search domain to try.
#if MDNSRESPONDER_SUPPORTS(APPLE, UNICAST_DOTLOCAL)
    DNSQuestion *               q2;                     // DNSQuestion for unicast version of a record with a dot-local name.
    mDNSu16                     q2Type;                 // q2's original qtype value.
    mDNSBool                    q2LongLived;            // q2's original LongLived value.
    mDNSBool                    q2ReturnIntermed;       // q2's original ReturnIntermed value.
    mDNSBool                    q2TimeoutQuestion;      // q2's original TimeoutQuestion value.
    mDNSBool                    q2AppendSearchDomains;  // q2's original AppendSearchDomains value.
#endif
#if MDNSRESPONDER_SUPPORTS(APPLE, REACHABILITY_TRIGGER)
    mDNSBool                    answered;               // True if the query was answered.
#endif

}   QueryRecordOp;

typedef struct
{
    mDNSInterfaceID     interfaceID;    // InterfaceID being used for query record operations.
    mDNSu32             protocols;      // Protocols (IPv4, IPv6) specified by client.
    QueryRecordOp *     op4;            // Query record operation object for A record.
    QueryRecordOp *     op6;            // Query record operation object for AAAA record.

}   GetAddrInfoClientRequest;

typedef struct
{
    QueryRecordOp       op; // Query record operation object.

}   QueryRecordClientRequest;

#ifdef __cplusplus
extern "C" {
#endif

mDNSexport mStatus GetAddrInfoClientRequestStart(GetAddrInfoClientRequest *inRequest, mDNSu32 inReqID,
    const char *inHostnameStr, mDNSu32 inInterfaceIndex, DNSServiceFlags inFlags, mDNSu32 inProtocols, mDNSs32 inPID,
    const mDNSu8 inUUID[UUID_SIZE], mDNSu32 inUID, QueryRecordResultHandler inResultHandler, void *inResultContext);
mDNSexport void GetAddrInfoClientRequestStop(GetAddrInfoClientRequest *inRequest);
mDNSexport const domainname * GetAddrInfoClientRequestGetQName(const GetAddrInfoClientRequest *inRequest);
mDNSexport mDNSBool GetAddrInfoClientRequestIsMulticast(const GetAddrInfoClientRequest *inRequest);

mDNSexport mStatus QueryRecordClientRequestStart(QueryRecordClientRequest *inRequest, mDNSu32 inReqID,
    const char *inQNameStr, mDNSu32 inInterfaceIndex, DNSServiceFlags inFlags, mDNSu16 inQType, mDNSu16 inQClass,
    mDNSs32 inPID, mDNSu8 inUUID[UUID_SIZE], mDNSu32 inUID, QueryRecordResultHandler inResultHandler, void *inResultContext);
mDNSexport void QueryRecordClientRequestStop(QueryRecordClientRequest *inRequest);
mDNSexport const domainname * QueryRecordClientRequestGetQName(const QueryRecordClientRequest *inRequest);
mDNSexport mDNSu16 QueryRecordClientRequestGetType(const QueryRecordClientRequest *inRequest);
mDNSexport mDNSBool QueryRecordClientRequestIsMulticast(QueryRecordClientRequest *inRequest);

#ifdef __cplusplus
}
#endif

#endif  // __ClientRequests_h
