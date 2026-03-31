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
uint8_t test_query_client_msgbuf[35] = {
	0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x73, 0x65, 0x72, 0x76, 0x65,
	0x72, 0x2e, 0x64, 0x6f, 0x74, 0x62, 0x65, 0x6e, 0x6e, 0x75, 0x2e, 0x63, 0x6f, 0x6d, 0x00, 0x00,
	0x01, 0x00, 0x01
};

// This uDNS message is a canned response that was originally captured by wireshark.
uint8_t test_query_response_msgbuf[108] = {
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
char test_original_domainname_cstr[] = "123server.dotbennu.com.";
char test_cname_domainname_cstr[] = "test212.dotbennu.com.";

@interface SuspiciousReplyTest : XCTestCase
{
    UDPSocket* local_socket;
    request_state* client_request_message;}
@end

@implementation SuspiciousReplyTest

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
    [self _verifySuspiciousResponseBehavior];
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
    char *msgptr = (char *)test_query_client_msgbuf;
    size_t msgsz = sizeof(test_query_client_msgbuf);
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
    XCTAssertFalse(strcmp(qname_cstr, test_original_domainname_cstr));
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

// This unit test tries to receive a response but changes the QID so it is ignored and can trigger suspicious mode
// 1) Test a suspicious response is ignored, but if it was previously requested, then don't go into suspicious mode
// 2) Test a suspicious response is ignored, and it does trigger suspicious mode
// 3) Test a configuration change event will reset suspicious mode
- (void)_verifySuspiciousResponseBehavior
{
    mDNS *const m = &mDNSStorage;
    DNSMessage *msgptr = (DNSMessage *)test_query_response_msgbuf;
    size_t msgsz = sizeof(test_query_response_msgbuf);
    request_state* req = client_request_message;
    mDNSOpaque16    suspiciousQID;

    // 1)
    // Receive and verify it is suspicious  (ignored response)
    // But not too suspicious               (did NOT go into suspicious mode)

    suspiciousQID.NotAnInteger = 0xDEAD;
    receive_suspicious_response_ut(req, msgptr, msgsz, suspiciousQID, true);
    
    // Verify 0 records recevied
    mDNSu32 CacheUsed =0, notUsed =0;
    LogCacheRecords_ut(mDNS_TimeNow(m), &CacheUsed, &notUsed);
    XCTAssertEqual(CacheUsed, 0);               // Verify 0 records recevied
    XCTAssertFalse(m->NextSuspiciousTimeout);   // And NOT in suspicious mode

    // 2)
    // Receive and verify it is suspicious  (ignored response)
    // And put itself in suspicious mode    (did go into suspicious mode)

    receive_suspicious_response_ut(req, msgptr, msgsz, suspiciousQID, false);
    LogCacheRecords_ut(mDNS_TimeNow(m), &CacheUsed, &notUsed);
    XCTAssertEqual(CacheUsed, 0);               // Verify 0 records recevied
    XCTAssertTrue(m->NextSuspiciousTimeout);    // And IS in suspicious mode

    // 3)
    // Verify suspicious mode is stopped when a configuration change occurs.

    force_uDNS_SetupDNSConfig_ut(m);
    XCTAssertFalse(m->NextSuspiciousTimeout);
}


@end
