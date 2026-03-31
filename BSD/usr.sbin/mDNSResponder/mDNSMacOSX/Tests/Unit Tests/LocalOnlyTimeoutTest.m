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
#include "mDNSMacOSX.h"
#import <XCTest/XCTest.h>

// This query request message was generated from the following command: "dns-sd -lo -timeout -Q cardinal2.apple.com. A"
char query_req_msgbuf[33]= {
	0x00, 0x01, 0x90, 0x00,
	// DNSServiceFlags.L = (kDNSServiceFlagsReturnIntermediates |kDNSServiceFlagsSuppressUnusable | kDNSServiceFlagsTimeout)
	0xff, 0xff, 0xff, 0xff,
	// interfaceIndex = mDNSInterface_LocalOnly
	0x63, 0x61, 0x72, 0x64, 0x69, 0x6e, 0x61, 0x6c,
	0x32, 0x2e, 0x61, 0x70, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x2e, 0x00, 0x00, 0x01, 0x00,
	0x01
};

mDNSlocal mStatus InitEtcHostsRecords(void)
{
    mDNS *m = &mDNSStorage;
    struct sockaddr_storage hostaddr;
    
    AuthHash newhosts;
    mDNSPlatformMemZero(&newhosts, sizeof(AuthHash));
    
    memset(&hostaddr, 0, sizeof(hostaddr));
    get_ip("127.0.0.1", &hostaddr);
    
    domainname domain;
    MakeDomainNameFromDNSNameString(&domain, "localhost");
    
    mDNSMacOSXCreateEtcHostsEntry_ut(&domain, (struct sockaddr *) &hostaddr, mDNSNULL, mDNSNULL, &newhosts);
    
    memset(&hostaddr, 0, sizeof(hostaddr));
    get_ip("0000:0000:0000:0000:0000:0000:0000:0001", &hostaddr);
    
    MakeDomainNameFromDNSNameString(&domain, "localhost");
    
    mDNSMacOSXCreateEtcHostsEntry_ut(&domain, (struct sockaddr *) &hostaddr, mDNSNULL, mDNSNULL, &newhosts);
    
    memset(&hostaddr, 0, sizeof(hostaddr));
    get_ip("255.255.255.255", &hostaddr);
    
    MakeDomainNameFromDNSNameString(&domain, "broadcasthost");
    
    mDNSMacOSXCreateEtcHostsEntry_ut(&domain, (struct sockaddr *) &hostaddr, mDNSNULL, mDNSNULL, &newhosts);
    
    memset(&hostaddr, 0, sizeof(hostaddr));
    get_ip("17.226.40.200", &hostaddr);
    
    MakeDomainNameFromDNSNameString(&domain, "cardinal2.apple.com");
    
    mDNSMacOSXCreateEtcHostsEntry_ut(&domain, (struct sockaddr *) &hostaddr, mDNSNULL, mDNSNULL, &newhosts);
    UpdateEtcHosts_ut(&newhosts);
    
    m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
    mDNS_Execute(m);
    
    return mStatus_NoError;
}

@interface LocalOnlyATimeoutTest : XCTestCase
{
    request_state* client_request_message;
    UDPSocket* local_socket;
    char domainname_cstr[MAX_ESCAPED_DOMAIN_NAME];
}
@end

@implementation LocalOnlyATimeoutTest

// The InitUnitTest() initializes a minimal mDNSResponder environment as
// well as allocates memory for a local_socket and client request.
// It also sets the domainname_cstr specified in the client's query request.
// Note: This unit test does not send packets on the wire and it does not open sockets.
- (void)setUp
{
	// Init mDNSStorage
	mStatus result = init_mdns_storage();
    XCTAssertEqual(result, mStatus_NoError);

	// Allocate a client request
	local_socket = (UDPSocket *)calloc(1, sizeof(*local_socket));

	// Allocate memory for a request that is used to make client requests.
	client_request_message = calloc(1, sizeof(request_state));

	// Init domainname that is used by unit tests
	strlcpy(domainname_cstr, "cardinal2.apple.com.", sizeof(domainname_cstr));
}

// This function does memory cleanup and no verification.
- (void)tearDown
{
    mDNSPlatformMemFree(local_socket);
}

// This unit test starts a local only request for "cardinal2.apple.com.".  It first
// calls start_client_request to start a query, it then verifies the
// req and query data structures are set as expected. Next, the cache is verified to
// be empty by AnswerNewLocalOnlyQuestion() and so results in GenerateNegativeResponse()
// getting called which sets up a reply with a negative answer in it for the client.
// On return from mDNS_Execute, the client's reply structure is verified to be set as
// expected. Lastly the timeout is simulated and mDNS_Execute is called. This results
// in a call to TimeoutQuestions(). And again, the GenerateNegativeResponse() is called
// which returns a negative response to the client.  This time the client reply is verified
// to be setup with a timeout result.
- (void)testStartLocalOnlyClientQueryRequest
{
	mDNS *const m = &mDNSStorage;
    request_state* req = client_request_message;
	char *msgptr = (char *)query_req_msgbuf;
	size_t msgsz = sizeof(query_req_msgbuf);
	DNSQuestion *q;
	mDNSs32 min_size = sizeof(DNSServiceFlags) + sizeof(mDNSu32) + 4;
	mStatus err = mStatus_NoError;
	char qname_cstr[MAX_ESCAPED_DOMAIN_NAME];
	struct reply_state *reply;
	size_t len;

	// Process the unit test's client request
	start_client_request(req, msgptr, msgsz, query_request, local_socket);
	XCTAssertEqual(err, mStatus_NoError);

	// Verify the query initialized and request fields were set as expected
	XCTAssertEqual(req->hdr.version, VERSION);
	XCTAssertGreaterThan((mDNSs32)req->data_bytes, min_size);
	XCTAssertEqual(req->flags, (kDNSServiceFlagsSuppressUnusable | kDNSServiceFlagsReturnIntermediates | kDNSServiceFlagsTimeout));
	XCTAssertEqual(req->interfaceIndex, kDNSServiceInterfaceIndexLocalOnly);
    XCTAssertNotEqual(req->terminate, (req_termination_fn)0);

	q = &req->u.queryrecord.op.q;
	XCTAssertEqual(q, m->NewLocalOnlyQuestions);
	XCTAssertNil((__bridge id)m->Questions);
	XCTAssertNil((__bridge id)m->NewQuestions);
	XCTAssertEqual(q->SuppressUnusable, 1);
	XCTAssertEqual(q->ReturnIntermed, 1);
	XCTAssertEqual(q->Suppressed, mDNSfalse);									// Regress <rdar://problem/27571734>

	ConvertDomainNameToCString(&q->qname, qname_cstr);
	XCTAssertFalse(strcmp(qname_cstr, domainname_cstr));
	XCTAssertEqual(q->qnamehash, DomainNameHashValue(&q->qname));

	XCTAssertEqual(q->InterfaceID, mDNSInterface_LocalOnly);
	XCTAssertEqual(q->flags, req->flags);
	XCTAssertEqual(q->qtype, 1);
	XCTAssertEqual(q->qclass, 1);
	XCTAssertEqual(q->LongLived, 0);
	XCTAssertEqual(q->ExpectUnique, mDNSfalse);
	XCTAssertEqual(q->ForceMCast, 0);
	XCTAssertEqual(q->TimeoutQuestion, 1);
	XCTAssertEqual(q->WakeOnResolve, 0);
	XCTAssertEqual(q->UseBackgroundTraffic, 0);
	XCTAssertEqual(q->ValidationRequired, 0);
	XCTAssertEqual(q->ValidatingResponse, 0);
	XCTAssertEqual(q->ProxyQuestion, 0);
    XCTAssertNotEqual((void*)q->QuestionCallback, (void*)mDNSNULL);
	XCTAssertNil((__bridge id)q->DNSSECAuthInfo);
    XCTAssertNil((__bridge id)(void*)q->DAIFreeCallback);
	XCTAssertNotEqual(q->StopTime, 0);
	XCTAssertEqual(q->AppendSearchDomains, 0);
    XCTAssertNil((__bridge id)q->DuplicateOf);

	// At this point the the cache is empty. Calling mDNS_Execute will answer the local-only
	// question with a negative response.
	m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
	mDNS_Execute(m);  // Regress <rdar://problem/28721294>

	// Verify reply is a negative response and error code is set to kDNSServiceErr_NoSuchRecord error.
	reply = req->replies;
    XCTAssertNotEqual(reply, (reply_state*)mDNSNULL);

	XCTAssertNil((__bridge id)m->NewLocalOnlyQuestions);
	XCTAssertEqual(q->LOAddressAnswers, 0);

	len = get_reply_len(qname_cstr, 0);

	XCTAssertNil((__bridge id)reply->next);
	XCTAssertEqual(reply->totallen, reply->mhdr->datalen + sizeof(ipc_msg_hdr));
	XCTAssertEqual(reply->mhdr->version, VERSION);
	XCTAssertEqual(reply->mhdr->datalen, len);
	XCTAssertEqual(reply->mhdr->ipc_flags, 0);
	XCTAssertEqual(reply->mhdr->op, query_reply_op);

    XCTAssertTrue((reply->rhdr->flags & htonl(kDNSServiceFlagsAdd)));
	XCTAssertEqual(reply->rhdr->ifi, kDNSServiceInterfaceIndexLocalOnly);	    // Regress <rdar://problem/27340874>
	XCTAssertEqual(reply->rhdr->error,
					(DNSServiceErrorType)htonl(kDNSServiceErr_NoSuchRecord));	// Regress <rdar://problem/24827555>

	// Simulate what udsserver_idle normally does for clean up
	freeL("StartLocalOnlyClientQueryRequest:reply", reply);
	req->replies = NULL;

	// Simulate the query time out of the local-only question.
	// The expected behavior is a negative answer with time out error
	m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
	q->StopTime = mDNS_TimeNow_NoLock(m);
	m->NextScheduledStopTime -= mDNSPlatformOneSecond*5;
	mDNS_Execute(m);

	// Verify the reply is a negative response with timeout error.
	reply = req->replies;
    XCTAssertNotEqual(reply, (reply_state*)mDNSNULL);
    XCTAssertNil((__bridge id)m->NewLocalOnlyQuestions);
    XCTAssertEqual(q->LOAddressAnswers, 0);

	len = get_reply_len(qname_cstr, 0);

    XCTAssertNil((__bridge id)reply->next);
	XCTAssertEqual(reply->totallen, len + sizeof(ipc_msg_hdr));
	XCTAssertEqual(reply->mhdr->version, VERSION);
	XCTAssertEqual(reply->mhdr->datalen, len);
	XCTAssertEqual(reply->mhdr->ipc_flags, 0);
	XCTAssertEqual(reply->mhdr->op, query_reply_op);
    XCTAssertTrue((reply->rhdr->flags & htonl(kDNSServiceFlagsAdd)));
	XCTAssertEqual(reply->rhdr->ifi, kDNSServiceInterfaceIndexLocalOnly);	    // Regress <rdar://problem/27340874>
	XCTAssertEqual(reply->rhdr->error,
					(DNSServiceErrorType)htonl(kDNSServiceErr_Timeout));		// Regress <rdar://problem/27562965>

	// Free request and reallocate to use when query is restarted
	free_req(req);
	client_request_message = calloc(1, sizeof(request_state));
}

// This unit test populates the cache with four /etc/hosts records and then
// verifies there are four entries in the cache.
- (void)testPopulateCacheWithClientLOResponseRecords
{
	mDNS *const m = &mDNSStorage;

	// Verify cache is empty
	int count = LogEtcHosts_ut(m);
	XCTAssertEqual(count, 0);

	// Populate /etc/hosts
	mStatus result = InitEtcHostsRecords();
	XCTAssertEqual(result, mStatus_NoError);

	// mDNS_Execute is called to populate the /etc/hosts cache.
	m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
	mDNS_Execute(m);

	count = LogEtcHosts_ut(m);
	XCTAssertEqual(count, 4);
    
    [self _testRestartLocalOnlyClientQueryRequest];   //  Continuation of this test
}

// This unit test starts a local only request for "cardinal2.apple.com.".  It first
// calls start_client_request to start a query, it then verifies the
// req and query data structures are set as expected. Next, the cache is verified to
// contain the answer by AnswerNewLocalOnlyQuestion() and so results in setting up an
// answer reply to the client. On return from mDNS_Execute, the client's reply structure
// is verified to be set as expected. Lastly the timeout is simulated and mDNS_Execute is
// called. This results in a call to TimeoutQuestions(). And this time, the
// GenerateNegativeResponse() is called which returns a negative response to the client
// which specifies the timeout occurred. Again, the answer reply is verified to
// to specify a timeout.
- (void)_testRestartLocalOnlyClientQueryRequest
{
	mDNS *const m = &mDNSStorage;
	request_state* req = client_request_message;
	char *msgptr = (char *)query_req_msgbuf;
	size_t msgsz = sizeof(query_req_msgbuf);	DNSQuestion *q;
	mDNSs32 min_size = sizeof(DNSServiceFlags) + sizeof(mDNSu32) + 4;
	mStatus err = mStatus_NoError;
	char qname_cstr[MAX_ESCAPED_DOMAIN_NAME];
	struct reply_state *reply;
	size_t len;

	// Process the unit test's client request
	start_client_request(req, msgptr, msgsz, query_request, local_socket);
    XCTAssertEqual(err, mStatus_NoError);

	XCTAssertEqual(req->hdr.version, VERSION);
    XCTAssertGreaterThan((mDNSs32)req->data_bytes, min_size);
	XCTAssertEqual(req->flags, (kDNSServiceFlagsSuppressUnusable | kDNSServiceFlagsReturnIntermediates | kDNSServiceFlagsTimeout));
	XCTAssertEqual(req->interfaceIndex, kDNSServiceInterfaceIndexLocalOnly);
    XCTAssertNotEqual(req->terminate, (req_termination_fn)0);
    XCTAssertNil((__bridge id)m->Questions);

	q = &req->u.queryrecord.op.q;
	XCTAssertEqual(q, m->NewLocalOnlyQuestions);
	XCTAssertEqual(q->SuppressUnusable, 1);
	XCTAssertEqual(q->ReturnIntermed, 1);
	XCTAssertEqual(q->Suppressed, mDNSfalse);										// Regress <rdar://problem/27571734>
	XCTAssertEqual(q->qnamehash, DomainNameHashValue(&q->qname));
	XCTAssertEqual(q->InterfaceID, mDNSInterface_LocalOnly);
	XCTAssertEqual(q->flags, req->flags);
	XCTAssertEqual(q->qtype, 1);
	XCTAssertEqual(q->qclass, 1);
	XCTAssertEqual(q->LongLived, 0);
	XCTAssertEqual(q->ExpectUnique, mDNSfalse);
	XCTAssertEqual(q->ForceMCast, 0);
	XCTAssertEqual(q->TimeoutQuestion, 1);
	XCTAssertEqual(q->WakeOnResolve, 0);
	XCTAssertEqual(q->UseBackgroundTraffic, 0);
	XCTAssertEqual(q->ValidationRequired, 0);
	XCTAssertEqual(q->ValidatingResponse, 0);
	XCTAssertEqual(q->ProxyQuestion, 0);
    XCTAssertNotEqual((void*)q->QuestionCallback, (void*)mDNSNULL);
    XCTAssertNil((__bridge id)q->DNSSECAuthInfo);
    XCTAssertNil((__bridge id)(void*)q->DAIFreeCallback);
    XCTAssertNotEqual(q->StopTime, 0);
    XCTAssertEqual(q->AppendSearchDomains, 0);
    XCTAssertNil((__bridge id)q->DuplicateOf);
	ConvertDomainNameToCString(&q->qname, qname_cstr);
    XCTAssertFalse(strcmp(qname_cstr, domainname_cstr));

	// Answer local-only question with found cache entry
	m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
	mDNS_Execute(m);															// Regress <rdar://problem/28721294>
    XCTAssertNil((__bridge id)m->NewLocalOnlyQuestions);
	XCTAssertEqual(req->u.queryrecord.op.answered, 1);
	XCTAssertEqual(q->LOAddressAnswers, 1);
	XCTAssertEqual(q, m->LocalOnlyQuestions);

	reply = req->replies;
	len = get_reply_len(qname_cstr, 4);

    XCTAssertNil((__bridge id)reply->next);
	XCTAssertEqual(reply->totallen, len + sizeof(ipc_msg_hdr));
	XCTAssertEqual(reply->mhdr->version, VERSION);
	XCTAssertEqual(reply->mhdr->datalen, len);
	XCTAssertEqual(reply->mhdr->ipc_flags, 0);
	XCTAssertEqual(reply->mhdr->op, query_reply_op);
	XCTAssertTrue((reply->rhdr->flags & htonl(kDNSServiceFlagsAdd)));
	XCTAssertEqual(reply->rhdr->ifi, kDNSServiceInterfaceIndexLocalOnly);	    // Regress <rdar://problem/27340874>
	XCTAssertEqual(reply->rhdr->error, kDNSServiceErr_NoError);

	// Simulate the query time out of the local-only question.
	// The expected behavior is a negative answer with time out error
	m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
	q->StopTime = mDNS_TimeNow_NoLock(m);
	m->NextScheduledStopTime -= mDNSPlatformOneSecond*5;
	mDNS_Execute(m);

	reply = req->replies->next;
	XCTAssertNotEqual(reply, (reply_state*)mDNSNULL);
    XCTAssertNil((__bridge id)reply->next);
    XCTAssertNil((__bridge id)m->NewLocalOnlyQuestions);
	XCTAssertEqual(q->LOAddressAnswers, 0);
	len = get_reply_len(qname_cstr, 0);

    XCTAssertNil((__bridge id)reply->next);
	XCTAssertEqual(reply->totallen, len + + sizeof(ipc_msg_hdr));
	XCTAssertEqual(reply->mhdr->version, VERSION);
	XCTAssertEqual(reply->mhdr->datalen, len);
	XCTAssertEqual(reply->mhdr->ipc_flags, 0);
	XCTAssertEqual(reply->mhdr->op, query_reply_op);
    XCTAssertTrue((reply->rhdr->flags & htonl(kDNSServiceFlagsAdd)));
	XCTAssertEqual(reply->rhdr->ifi, kDNSServiceInterfaceIndexLocalOnly);	    // Regress <rdar://problem/27340874>
	XCTAssertEqual(reply->rhdr->error,
					(DNSServiceErrorType)htonl(kDNSServiceErr_Timeout));		// Regress <rdar://problem/27562965>

	free_req(req);
}

@end
