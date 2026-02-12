/*
 * Copyright (c) 2017-2019 Apple Inc. All rights reserved.
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

#include "unittest_common.h"
#import <XCTest/XCTest.h>

struct UDPSocket_struct
{
	mDNSIPPort port; // MUST BE FIRST FIELD -- mDNSCoreReceive expects every UDPSocket_struct to begin with mDNSIPPort port
};
typedef struct UDPSocket_struct UDPSocket;

// This client request was generated using the following command: "dns-sd -Q 123server.dotbennu.com. A".
uint8_t query_client_msgbuf[35] = {
	0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x73, 0x65, 0x72, 0x76, 0x65,
	0x72, 0x2e, 0x64, 0x6f, 0x74, 0x62, 0x65, 0x6e, 0x6e, 0x75, 0x2e, 0x63, 0x6f, 0x6d, 0x00, 0x00,
	0x01, 0x00, 0x01
};

// This uDNS message is a canned response that was originally captured by wireshark.
uint8_t query_response_msgbuf[108] = {
    0x69, 0x41, // transaction id
	0x85, 0x80, // flags
	0x00, 0x01, // 1 question for 123server.dotbennu.com. Addr
	0x00, 0x02,	// 2 anwsers: 123server.dotbennu.com. CNAME test212.dotbennu.com., test212.dotbennu.com. Addr 10.100.0.1,
	0x00, 0x01,	// 1 authorities anwser: dotbennu.com. NS cardinal2.apple.com.
	0x00, 0x00, 0x09, 0x31, 0x32, 0x33,
    0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x08, 0x64, 0x6f, 0x74, 0x62, 0x65, 0x6e, 0x6e, 0x75, 0x03,
    0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01, 0xc0, 0x0c, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00,
    0x02, 0x56, 0x00, 0x0a, 0x07, 0x74, 0x65, 0x73, 0x74, 0x32, 0x31, 0x32, 0xc0, 0x16, 0xc0, 0x34,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x04, 0x0a, 0x64, 0x00, 0x01, 0xc0, 0x16,
    0x00, 0x02, 0x00, 0x01, 0x00, 0x01, 0x51, 0x80, 0x00, 0x12, 0x09, 0x63, 0x61, 0x72, 0x64, 0x69,
    0x6e, 0x61, 0x6c, 0x32, 0x05, 0x61, 0x70, 0x70, 0x6c, 0x65, 0xc0, 0x1f
};

// Variables associated with contents of the above uDNS message
#define uDNS_TargetQID 16745
char udns_original_domainname_cstr[] = "123server.dotbennu.com.";
char udns_cname_domainname_cstr[] = "test212.dotbennu.com.";
static const mDNSv4Addr dns_response_ipv4 = {{ 10, 100, 0, 1 }};

@interface CNameRecordTest : XCTestCase
{
    UDPSocket* local_socket;
    request_state* client_request_message;}
@end

@implementation CNameRecordTest

// The InitThisUnitTest() initializes the mDNSResponder environment as well as
// a DNSServer. It also allocates memory for a local_socket and client request.
// Note: This unit test does not send packets on the wire and it does not open sockets.
- (void)setUp
{
    // Init unit test environment and verify no error occurred.
    mStatus result = init_mdns_environment(mDNStrue);
    XCTAssertEqual(result, mStatus_NoError);
    
    // Add one DNS server and verify it was added.
    AddDNSServer_ut();
    XCTAssertEqual(CountOfUnicastDNSServers(&mDNSStorage), 1);
    
    // Create memory for a socket that is never used or opened.
    local_socket = (UDPSocket *) mDNSPlatformMemAllocateClear(sizeof(*local_socket));
    
    // Create memory for a request that is used to make this unit test's client request.
    client_request_message = calloc(1, sizeof(request_state));
}

- (void)tearDown
{
    mDNS *m = &mDNSStorage;
    request_state* req = client_request_message;
    DNSServer   *ptr, **p = &m->DNSServers;
    
    while (req->replies)
    {
        reply_state *reply = req->replies;
        req->replies = req->replies->next;
        mDNSPlatformMemFree(reply);
    }
    mDNSPlatformMemFree(req);
    
    mDNSPlatformMemFree(local_socket);
    
    while (*p)
    {
        ptr = *p;
        *p = (*p)->next;
        LogInfo("FinalizeUnitTest: Deleting server %p %#a:%d (%##s)", ptr, &ptr->addr, mDNSVal16(ptr->port), ptr->domain.c);
        mDNSPlatformMemFree(ptr);
    }
}

- (void)testCNameRecordTestSeries
{
    [self _startClientQueryRequest];
    [self _populateCacheWithClientResponseRecords];
    [self _simulateNetworkChangeAndVerify];
}

// This test simulates a uds client request by setting up a client request and then
// calling mDNSResponder's handle_client_request.  The handle_client_request function
// processes the request and starts a query.  This unit test verifies
// the client request and query were setup as expected.  This unit test also calls
// mDNS_execute which determines the cache does not contain the new question's
// answer.
- (void)_startClientQueryRequest
{
    mDNS *const m = &mDNSStorage;
    request_state* req = client_request_message;
    char *msgptr = (char *)query_client_msgbuf;
    size_t msgsz = sizeof(query_client_msgbuf);
    mDNSs32 min_size = sizeof(DNSServiceFlags) + sizeof(mDNSu32) + 4;
    DNSQuestion *q;
    mStatus err = mStatus_NoError;
    char qname_cstr[MAX_ESCAPED_DOMAIN_NAME];
    
    // Process the unit test's client request
    start_client_request(req, msgptr, msgsz, query_request, local_socket);
    XCTAssertEqual(err, mStatus_NoError);
    
    // Verify the request fields were set as expected
    XCTAssertNil((__bridge id)req->next);
    XCTAssertNil((__bridge id)req->primary);
    XCTAssertEqual(req->sd, client_req_sd);
    XCTAssertEqual(req->process_id, client_req_process_id);
    XCTAssertFalse(strcmp(req->pid_name, client_req_pid_name));
    XCTAssertEqual(req->validUUID, mDNSfalse);
    XCTAssertEqual(req->errsd, 0);
    XCTAssertEqual(req->uid, client_req_uid);
    XCTAssertEqual(req->ts, t_complete);
    XCTAssertGreaterThan((mDNSs32)req->data_bytes, min_size);
    XCTAssertEqual(req->msgend, msgptr+msgsz);
    XCTAssertNil((__bridge id)(void*)req->msgbuf);
    XCTAssertEqual(req->hdr.version, VERSION);
    XCTAssertNil((__bridge id)req->replies);
    XCTAssertNotEqual(req->terminate, (req_termination_fn)0);
    XCTAssertEqual(req->flags, kDNSServiceFlagsReturnIntermediates);
    XCTAssertEqual(req->interfaceIndex, kDNSServiceInterfaceIndexAny);
    
    // Verify the query fields were set as expected
    q = &req->u.queryrecord.op.q;
    XCTAssertNotEqual(q, (DNSQuestion *)mDNSNULL);
    XCTAssertEqual(q, m->Questions);
    XCTAssertEqual(q, m->NewQuestions);
    XCTAssertEqual(q->SuppressUnusable, mDNSfalse);
    XCTAssertEqual(q->ReturnIntermed, mDNStrue);
    XCTAssertEqual(q->Suppressed, mDNSfalse);
    
    ConvertDomainNameToCString(&q->qname, qname_cstr);
    XCTAssertFalse(strcmp(qname_cstr, udns_original_domainname_cstr));
    XCTAssertEqual(q->qnamehash, DomainNameHashValue(&q->qname));
    
    XCTAssertEqual(q->InterfaceID, mDNSInterface_Any);
    XCTAssertEqual(q->flags, req->flags);
    XCTAssertEqual(q->qtype, 1);
    XCTAssertEqual(q->qclass, 1);
    XCTAssertEqual(q->LongLived, 0);
    XCTAssertEqual(q->ExpectUnique, mDNSfalse);
    XCTAssertEqual(q->ForceMCast, 0);
    XCTAssertEqual(q->TimeoutQuestion, 0);
    XCTAssertEqual(q->WakeOnResolve, 0);
    XCTAssertEqual(q->UseBackgroundTraffic, 0);
    XCTAssertEqual(q->ValidationRequired, 0);
    XCTAssertEqual(q->ValidatingResponse, 0);
    XCTAssertEqual(q->ProxyQuestion, 0);
    XCTAssertNotEqual((void*)q->QuestionCallback, (void*)mDNSNULL);
    XCTAssertNil((__bridge id)q->DNSSECAuthInfo);
    XCTAssertNil((__bridge id)(void*)q->DAIFreeCallback);
    XCTAssertEqual(q->AppendSearchDomains, 0);
    XCTAssertNil((__bridge id)q->DuplicateOf);
    
    // Call mDNS_Execute to see if the new question, q, has an answer in the cache.
    // It won't be yet because the cache is empty.
    m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
    mDNS_Execute(m);
    
    // Verify mDNS_Execute processed the new question.
    XCTAssertNil((__bridge id)m->NewQuestions);
    
    // Verify the cache is empty and the request got no reply.
    XCTAssertEqual(m->rrcache_totalused, 0);
    XCTAssertNil((__bridge id)req->replies);
}

// This unit test receives a canned uDNS response message by calling the mDNSCoreReceive() function.
// It then verifies cache entries were added for the CNAME and A records that were contained in the
// answers of the canned response, query_response_msgbuf.  This unit test also verifies that
// 2 add events were generated for the client.
- (void)_populateCacheWithClientResponseRecords
{
    mDNS *const m = &mDNSStorage;
    DNSMessage *msgptr = (DNSMessage *)query_response_msgbuf;
    size_t msgsz = sizeof(query_response_msgbuf);
    struct reply_state *reply;
    request_state* req = client_request_message;
    DNSQuestion *q = &req->u.queryrecord.op.q;
    const char *data;
    const char *end;
    char name[kDNSServiceMaxDomainName];
    uint16_t rrtype, rrclass, rdlen;
    const char *rdata;
    size_t len;
    char domainname_cstr[MAX_ESCAPED_DOMAIN_NAME];
    
    // Receive and populate the cache with canned response
    receive_response(req, msgptr, msgsz);
    
    // Verify 2 cache entries for CName and A record are present
    mDNSu32 CacheUsed =0, notUsed =0;
    LogCacheRecords_ut(mDNS_TimeNow(m), &CacheUsed, &notUsed);
    XCTAssertEqual(CacheUsed, m->rrcache_totalused);
    XCTAssertEqual(CacheUsed, 4); // 2 for the CacheGroup object plus 2 for the A and CNAME records
    XCTAssertEqual(m->PktNum, 1); // one packet was received
    
    // Verify question's qname is now set with the A record's domainname
    ConvertDomainNameToCString(&q->qname, domainname_cstr);
    XCTAssertEqual(q->qnamehash, DomainNameHashValue(&q->qname));
    XCTAssertFalse(strcmp(domainname_cstr, udns_cname_domainname_cstr));
    
    // Verify client's add event for CNAME is properly formed
    reply = req->replies;
    XCTAssertNotEqual(reply, (reply_state*)mDNSNULL);
    XCTAssertNil((__bridge id)reply->next);
    
    data    = (char *)&reply->rhdr[1];
    end     = data+reply->totallen;
    get_string(&data, data+reply->totallen, name, kDNSServiceMaxDomainName);
    rrtype  = get_uint16(&data, end);
    rrclass = get_uint16(&data, end);
    rdlen   = get_uint16(&data, end);
    rdata   = get_rdata(&data, end, rdlen);
    len     = get_reply_len(name, rdlen);
    
    XCTAssertEqual(reply->totallen, len + sizeof(ipc_msg_hdr));
    XCTAssertEqual(reply->mhdr->version, VERSION);
    XCTAssertEqual(reply->mhdr->datalen, len);
    XCTAssertEqual(reply->mhdr->ipc_flags, 0);
    XCTAssertEqual(reply->mhdr->op, query_reply_op);
    
    XCTAssertEqual(reply->rhdr->flags, htonl(kDNSServiceFlagsAdd));
    XCTAssertEqual(reply->rhdr->ifi, kDNSServiceInterfaceIndexAny);
    XCTAssertEqual(reply->rhdr->error, kDNSServiceErr_NoError);
    
    XCTAssertEqual(rrtype, kDNSType_CNAME);
    XCTAssertEqual(rrclass, kDNSClass_IN);
    ConvertDomainNameToCString((const domainname *const)rdata, domainname_cstr);
    XCTAssertFalse(strcmp(domainname_cstr, "test212.dotbennu.com."));
    
    // The mDNS_Execute call generates an add event for the A record
    m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
    mDNS_Execute(m);
    
    // Verify the client's reply contains a properly formed add event for the A record.
    reply = req->replies;
    XCTAssertNotEqual(reply, (reply_state*)mDNSNULL);
    XCTAssertNotEqual(reply->next, (reply_state*)mDNSNULL);
    reply = reply->next;
    
    data    = (char *)&reply->rhdr[1];
    end     = data+reply->totallen;
    get_string(&data, data+reply->totallen, name, kDNSServiceMaxDomainName);
    rrtype  = get_uint16(&data, end);
    rrclass = get_uint16(&data, end);
    rdlen   = get_uint16(&data, end);
    rdata   = get_rdata(&data, end, rdlen);
    len     = get_reply_len(name, rdlen);
    
    XCTAssertEqual(reply->totallen, len + sizeof(ipc_msg_hdr));
    XCTAssertEqual(reply->mhdr->version, VERSION);
    XCTAssertEqual(reply->mhdr->datalen, len);
    
    XCTAssertEqual(reply->mhdr->ipc_flags, 0);
    XCTAssertEqual(reply->mhdr->op, query_reply_op);
    
    XCTAssertEqual(reply->rhdr->flags, htonl(kDNSServiceFlagsAdd));
    XCTAssertEqual(reply->rhdr->ifi, kDNSServiceInterfaceIndexAny);
    XCTAssertEqual(reply->rhdr->error, kDNSServiceErr_NoError);
    
    XCTAssertEqual(rrtype, kDNSType_A);
    XCTAssertEqual(rrclass, kDNSClass_IN);
    XCTAssertEqual(rdata[0], dns_response_ipv4.b[0]);
    XCTAssertEqual(rdata[1], dns_response_ipv4.b[1]);
    XCTAssertEqual(rdata[2], dns_response_ipv4.b[2]);
    XCTAssertEqual(rdata[3], dns_response_ipv4.b[3]);
}

// This function verifies the cache and event handling occurred as expected when a network change happened.
// The uDNS_SetupDNSConfig is called to simulate a network change and two outcomes occur. First the A record
// query is restarted and sent to a new DNS server. Second the cache records are purged. Then mDNS_Execute
// is called and it removes the purged cache records and generates a remove event for the A record.
// The following are verified:
//      1.) The restart of query for A record.
//      2.) The cache is empty after mDNS_Execute removes the cache entres.
//      3.) The remove event is verified by examining the request's reply data.
- (void)_simulateNetworkChangeAndVerify
{
    mDNS *const m = &mDNSStorage;
    request_state*  req = client_request_message;
    DNSQuestion*    q = &req->u.queryrecord.op.q;
    mDNSu32 CacheUsed =0, notUsed =0;
    const char *data;    const char *end;
    char name[kDNSServiceMaxDomainName];
    uint16_t rrtype, rrclass, rdlen;
    const char *rdata;
    size_t len;
    
    // The uDNS_SetupDNSConfig reconfigures the resolvers so the A record query is restarted and
    // both the CNAME and A record are purged.
    force_uDNS_SetupDNSConfig_ut(m);

    // Verify the A record query was restarted.  This is done indirectly by noticing the transaction id and interval have changed.
    XCTAssertEqual(q->ThisQInterval, InitialQuestionInterval);
    XCTAssertNotEqual(q->TargetQID.NotAnInteger, uDNS_TargetQID);
    
    // Then mDNS_Execute removes both records from the cache and calls the client back with a remove event for A record.
    m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
    mDNS_Execute(m);
    
    // Verify the cache entries are removed
    LogCacheRecords_ut(mDNS_TimeNow(m), &CacheUsed, &notUsed);
    XCTAssertEqual(CacheUsed, m->rrcache_totalused);
    XCTAssertEqual(CacheUsed, 0);
    
    // Verify the A record's remove event is setup as expected in the reply data
    struct reply_state *reply;
    reply = req->replies;
    XCTAssertNotEqual(reply, (reply_state*)mDNSNULL);
    XCTAssertNotEqual(reply->next, (reply_state*)mDNSNULL);
    XCTAssertNotEqual(reply->next->next, (reply_state*)mDNSNULL);

    reply = reply->next->next; // Get to last event to verify remove event
    data    = (char *)&reply->rhdr[1];
    end     = data+reply->totallen;
    get_string(&data, data+reply->totallen, name, kDNSServiceMaxDomainName);
    rrtype  = get_uint16(&data, end);
    rrclass = get_uint16(&data, end);
    rdlen   = get_uint16(&data, end);
    rdata   = get_rdata(&data, end, rdlen);
    len     = get_reply_len(name, rdlen);
    
    XCTAssertEqual(reply->totallen, reply->mhdr->datalen + sizeof(ipc_msg_hdr));
    XCTAssertEqual(reply->mhdr->version, VERSION);
    XCTAssertEqual(reply->mhdr->datalen, len);
    XCTAssertEqual(reply->mhdr->ipc_flags, 0);
    XCTAssertEqual(reply->mhdr->op, query_reply_op);
    
    XCTAssertNotEqual(reply->rhdr->flags, htonl(kDNSServiceFlagsAdd));
    XCTAssertEqual(reply->rhdr->ifi, kDNSServiceInterfaceIndexAny);
    XCTAssertEqual(reply->rhdr->error, kDNSServiceErr_NoError);
    
    XCTAssertEqual(rrtype, kDNSType_A);
    XCTAssertEqual(rrclass, kDNSClass_IN);
    XCTAssertEqual(rdata[0], dns_response_ipv4.b[0]);
    XCTAssertEqual(rdata[1], dns_response_ipv4.b[1]);
    XCTAssertEqual(rdata[2], dns_response_ipv4.b[2]);
    XCTAssertEqual(rdata[3], dns_response_ipv4.b[3]);
}

@end
