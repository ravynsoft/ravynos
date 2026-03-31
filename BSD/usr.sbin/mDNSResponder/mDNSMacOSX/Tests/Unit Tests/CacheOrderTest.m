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

// This client request was generated using the following command: "dns-sd -Q web.mydomain.test".
uint8_t test_order_query_msgbuf[30] = {
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x65, 0x62, 0x2e, 0x6d, 0x79, 0x64,
    0x6f, 0x6d, 0x61, 0x69, 0x6e, 0x2e, 0x74, 0x65, 0x73, 0x74, 0x00, 0x00, 0x01, 0x00, 0x01
};
#if 0
0000   10 c1 01 00 00 01 00 00 00 00 00 00 03 77 65 62
0010   08 6d 79 64 6f 6d 61 69 6e 04 74 65 73 74 00 00
0020   01 00 01

0000   ef 53 01 00 00 01 00 00 00 00 00 00 03 77 65 62
0010   08 6d 79 64 6f 6d 61 69 6e 04 74 65 73 74 00 00
0020   01 00 01

uint8_t test_query_client_msgbuf[35] = {
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x73, 0x65, 0x72, 0x76, 0x65,
    0x72, 0x2e, 0x64, 0x6f, 0x74, 0x62, 0x65, 0x6e, 0x6e, 0x75, 0x2e, 0x63, 0x6f, 0x6d, 0x00, 0x00,
    0x01, 0x00, 0x01
};
#endif
// This uDNS message is a canned response that was originally captured by wireshark.
uint8_t test_order_response1_msgbuf[228] = {
    0x0f, 0x98, // transaction id
    0x85, 0x80, // flags
    0x00, 0x01, // 1 query: web.mydomain.test: type A, class IN
    0x00, 0x04, // 4 anwsers: Addr 10.0.0.101, Addr 10.0.0.105, Addr 10.0.0.104, Addr 10.0.0.102
    0x00, 0x01, // 1 authoritative nameservers: mydomain.test: type NS, class IN, ns ns.mydomain.test
    0x00, 0x01, // 1 additional: ns.mydomain.test: type A, class IN, addr 192.168.0.23
    0x03, 0x77, 0x65, 0x62,
    0x08, 0x6d, 0x79, 0x64, 0x6f, 0x6d, 0x61, 0x69, 0x6e, 0x04, 0x74, 0x65, 0x73, 0x74, 0x00, 0x00,
    0x01, 0x00, 0x01, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x0a,
    0x00, 0x00, 0x65, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x0a,
    0x00, 0x00, 0x69, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x0a,
    0x00, 0x00, 0x68, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x0a,
    0x00, 0x00, 0x66, 0xc0, 0x10, 0x00, 0x02, 0x00, 0x01, 0x00, 0x01, 0x51, 0x80, 0x00, 0x05, 0x02,
    0x6e, 0x73, 0xc0, 0x10, 0xc0, 0x6f, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x51, 0x80, 0x00, 0x04,
    0xc0, 0xa8, 0x00, 0x17
};

// This uDNS message is a canned response that was originally captured by wireshark, then modified to match above (other than Addr order).
uint8_t test_order_response2_msgbuf[228] = {
    0x0f, 0x98, // transaction id
    0x85, 0x80, // flags
    0x00, 0x01, // 1 query: web.mydomain.test: type A, class IN
    0x00, 0x04, // 4 anwsers: Addr 10.0.0.102, Addr 10.0.0.101, Addr 10.0.0.104, Addr 10.0.0.105
    0x00, 0x01, // 1 authoritative nameservers: mydomain.test: type NS, class IN, ns ns.mydomain.test
    0x00, 0x01, // 1 additional: ns.mydomain.test: type A, class IN, addr 192.168.0.23
    0x03, 0x77, 0x65, 0x62,
    0x08, 0x6d, 0x79, 0x64, 0x6f, 0x6d, 0x61, 0x69, 0x6e, 0x04, 0x74, 0x65, 0x73, 0x74, 0x00, 0x00,
    0x01, 0x00, 0x01, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x0a,
    0x00, 0x00, 0x66, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x0a,
    0x00, 0x00, 0x65, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x0a,
    0x00, 0x00, 0x68, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x0a,
    0x00, 0x00, 0x69, 0xc0, 0x10, 0x00, 0x02, 0x00, 0x01, 0x00, 0x01, 0x51, 0x80, 0x00, 0x05, 0x02,
    0x6e, 0x73, 0xc0, 0x10, 0xc0, 0x6f, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x51, 0x80, 0x00, 0x04,
    0xc0, 0xa8, 0x00, 0x17
};

// Variables associated with contents of the above uDNS message
char test_order_domainname_cstr[] = "web.mydomain.test.";

@interface CacheOrderTest : XCTestCase
{
    UDPSocket* local_socket;
    request_state* client_request_message;}
@end

@implementation CacheOrderTest

// The InitThisUnitTest() initializes the mDNSResponder environment as well as
// a DNSServer. It also allocates memory for a local_socket and client request.
// Note: This unit test does not send packets on the wire and it does not open sockets.
- (void)setUp
{
    mDNSPlatformMemZero(&mDNSStorage, sizeof(mDNS));

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

- (void)testSuspiciousReplyTestSeries
{
    [self _clientQueryRequest];
    [self _verifyCacheOrderBehavior];
}

// Simulate a uds client request by setting up a client request and then
// calling mDNSResponder's handle_client_request.  The handle_client_request function
// processes the request and starts a query.  This unit test verifies
// the client request and query were setup as expected.  This unit test also calls
// mDNS_execute which determines the cache does not contain the new question's
// answer.
- (void)_clientQueryRequest
{
    mDNS *const m = &mDNSStorage;
    request_state* req = client_request_message;
    char *msgptr = (char *)test_order_query_msgbuf;
    size_t msgsz = sizeof(test_order_query_msgbuf);
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
    XCTAssertFalse(strcmp(qname_cstr, test_order_domainname_cstr));
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

// This unit test performs two queries and verifies the cache oredr is updated on a new response.
// 1) Verify response is ordered in the cache as expected
// 2) Test again with new response, and verify cache order is updated
- (void)_verifyCacheOrderBehavior
{
    mDNS *const m = &mDNSStorage;
    DNSMessage *msgptr;
    size_t msgsz;
    request_state* req = client_request_message;
    DNSQuestion *q = &req->u.queryrecord.op.q;
    mStatus status;

    // 1)
    // Process first response
    // Verify response cache count & order

    msgptr = (DNSMessage *)test_order_response1_msgbuf;
    msgsz = sizeof(test_order_response1_msgbuf);
    receive_response(req, msgptr, msgsz);
    
    // Verify records received
    mDNSu32 CacheUsed =0, notUsed =0;
    LogCacheRecords_ut(mDNS_TimeNow(m), &CacheUsed, &notUsed);
    XCTAssertEqual(CacheUsed, 5);               // Verify 4 records received + Cache Group

    // Verify record order
    mDNSu8  lastoctet1[4] = {101, 105, 104, 102};
    status = verify_cache_addr_order_for_domain_ut(m, lastoctet1, 4, &q->qname);
    XCTAssertEqual(status, mStatus_NoError, @"Cache order test 1 failed");

    // 2)
    // Process second response
    // Verify response cache count & order

    msgptr = (DNSMessage *)test_order_response2_msgbuf;
    msgsz = sizeof(test_order_response2_msgbuf);
    receive_response(req, msgptr, msgsz);

    // Verify records received
    LogCacheRecords_ut(mDNS_TimeNow(m), &CacheUsed, &notUsed);
    XCTAssertEqual(CacheUsed, 5);               // Verify 4 records received + Cache Group

    // Verify record order
    mDNSu8  lastoctet2[4] = {102, 101, 104, 105};
    status = verify_cache_addr_order_for_domain_ut(m, lastoctet2, 4, &q->qname);
    XCTAssertEqual(status, mStatus_NoError, @"Cache order test 2 failed");
}


@end
