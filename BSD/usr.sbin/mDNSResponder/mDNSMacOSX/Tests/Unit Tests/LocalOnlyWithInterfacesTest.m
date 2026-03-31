/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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
char test_query_any_msgbuf[35] = {
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x73, 0x65, 0x72, 0x76, 0x65,
    0x72, 0x2e, 0x64, 0x6f, 0x74, 0x62, 0x65, 0x6e, 0x6e, 0x75, 0x2e, 0x63, 0x6f, 0x6d, 0x00, 0x00,
    0x01, 0x00, 0x01
};

// Modified for different scopes
char test_query_local_msgbuf[35] = {
    0x00, 0x00, 0x10, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x31, 0x32, 0x33, 0x73, 0x65, 0x72, 0x76, 0x65,
    0x72, 0x2e, 0x64, 0x6f, 0x74, 0x62, 0x65, 0x6e, 0x6e, 0x75, 0x2e, 0x63, 0x6f, 0x6d, 0x00, 0x00,
    0x01, 0x00, 0x01
};

char test_query_interface_msgbuf[35] = {
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x73, 0x65, 0x72, 0x76, 0x65,
    0x72, 0x2e, 0x64, 0x6f, 0x74, 0x62, 0x65, 0x6e, 0x6e, 0x75, 0x2e, 0x63, 0x6f, 0x6d, 0x00, 0x00,
    0x01, 0x00, 0x01
};

// Variables associated with contents of the above uDNS message
mDNSlocal char test_domainname_cstr[] = "123server.dotbennu.com.";

mDNSlocal mDNSBool _TestCreateEtcHostsEntryWithInterfaceID(const domainname *domain, const struct sockaddr *sa, const domainname *cname, mDNSInterfaceID interfaceID, AuthHash *auth)
{   // Copied from mDNSMacOSXCreateEtcHostsEntry
    AuthRecord *rr;
    mDNSu32 namehash;
    AuthGroup *ag;
    mDNSInterfaceID InterfaceID = mDNSInterface_LocalOnly;
    mDNSu16 rrtype;

    if (!domain)
    {
        LogMsg("_TestCreateEtcHostsEntryWithInterfaceID: ERROR!! name NULL");
        return mDNSfalse;
    }
    if (!sa && !cname)
    {
        LogMsg("_TestCreateEtcHostsEntryWithInterfaceID: ERROR!! sa and cname both NULL");
        return mDNSfalse;
    }

    if (sa && sa->sa_family != AF_INET && sa->sa_family != AF_INET6)
    {
        LogMsg("_TestCreateEtcHostsEntryWithInterfaceID: ERROR!! sa with bad family %d", sa->sa_family);
        return mDNSfalse;
    }


    if (interfaceID)
    {
        InterfaceID = interfaceID;
    }

    if (sa)
        rrtype = (sa->sa_family == AF_INET ? kDNSType_A : kDNSType_AAAA);
    else
        rrtype = kDNSType_CNAME;

    // Check for duplicates. See whether we parsed an entry before like this ?
    namehash = DomainNameHashValue(domain);
    ag = AuthGroupForName(auth, namehash, domain);
    if (ag)
    {
        rr = ag->members;
        while (rr)
        {
            if (rr->resrec.rrtype == rrtype)
            {
                if (rrtype == kDNSType_A)
                {
                    mDNSv4Addr ip;
                    ip.NotAnInteger = ((struct sockaddr_in*)sa)->sin_addr.s_addr;
                    if (mDNSSameIPv4Address(rr->resrec.rdata->u.ipv4, ip) && InterfaceID == rr->resrec.InterfaceID)
                    {
                        LogInfo("_TestCreateEtcHostsEntryWithInterfaceID: Same IPv4 address and InterfaceID for name %##s ID %d", domain->c, IIDPrintable(InterfaceID));
                        return mDNSfalse;
                    }
                }
                else if (rrtype == kDNSType_AAAA)
                {
                    mDNSv6Addr ip6;
                    ip6.l[0] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[0];
                    ip6.l[1] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[1];
                    ip6.l[2] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[2];
                    ip6.l[3] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[3];
                    if (mDNSSameIPv6Address(rr->resrec.rdata->u.ipv6, ip6) && InterfaceID == rr->resrec.InterfaceID)
                    {
                        LogInfo("_TestCreateEtcHostsEntryWithInterfaceID: Same IPv6 address and InterfaceID for name %##s ID %d", domain->c, IIDPrintable(InterfaceID));
                        return mDNSfalse;
                    }
                }
                else if (rrtype == kDNSType_CNAME)
                {
                    if (SameDomainName(&rr->resrec.rdata->u.name, cname))
                    {
                        LogInfo("_TestCreateEtcHostsEntryWithInterfaceID: Same cname %##s for name %##s", cname->c, domain->c);
                        return mDNSfalse;
                    }
                }
            }
            rr = rr->next;
        }
    }
    rr = (AuthRecord *) callocL("etchosts", sizeof(*rr));
    if (rr == NULL) return mDNSfalse;
    mDNS_SetupResourceRecord(rr, NULL, InterfaceID, rrtype, 1, kDNSRecordTypeKnownUnique, AuthRecordLocalOnly, FreeEtcHosts, NULL);
    AssignDomainName(&rr->namestorage, domain);

    if (sa)
    {
        rr->resrec.rdlength = sa->sa_family == AF_INET ? sizeof(mDNSv4Addr) : sizeof(mDNSv6Addr);
        if (sa->sa_family == AF_INET)
            rr->resrec.rdata->u.ipv4.NotAnInteger = ((struct sockaddr_in*)sa)->sin_addr.s_addr;
        else
        {
            rr->resrec.rdata->u.ipv6.l[0] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[0];
            rr->resrec.rdata->u.ipv6.l[1] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[1];
            rr->resrec.rdata->u.ipv6.l[2] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[2];
            rr->resrec.rdata->u.ipv6.l[3] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[3];
        }
    }
    else
    {
        rr->resrec.rdlength = DomainNameLength(cname);
        rr->resrec.rdata->u.name.c[0] = 0;
        AssignDomainName(&rr->resrec.rdata->u.name, cname);
    }
    rr->resrec.namehash = DomainNameHashValue(rr->resrec.name);
    SetNewRData(&rr->resrec, mDNSNULL, 0);  // Sets rr->rdatahash for us
    LogInfo("_TestCreateEtcHostsEntryWithInterfaceID: Adding resource record %s ID %d", ARDisplayString(&mDNSStorage, rr), IIDPrintable(rr->resrec.InterfaceID));
    InsertAuthRecord(&mDNSStorage, auth, rr);
    return mDNStrue;
}

mDNSlocal mStatus InitEtcHostsRecords(void)
{
    mDNS *m = &mDNSStorage;
    struct sockaddr_storage hostaddr;
    domainname domain;

    AuthHash newhosts;
    mDNSPlatformMemZero(&newhosts, sizeof(AuthHash));

    memset(&hostaddr, 0, sizeof(hostaddr));
    get_ip("10.0.0.201", &hostaddr);
    MakeDomainNameFromDNSNameString(&domain, "123server.dotbennu.com");
    _TestCreateEtcHostsEntryWithInterfaceID(&domain, (struct sockaddr *) &hostaddr, mDNSNULL, mDNSInterface_P2P, &newhosts);

    memset(&hostaddr, 0, sizeof(hostaddr));
    get_ip("10.0.0.202", &hostaddr);
    MakeDomainNameFromDNSNameString(&domain, "123server.dotbennu.com");
    mDNSMacOSXCreateEtcHostsEntry_ut(&domain, (struct sockaddr *) &hostaddr, mDNSNULL, mDNSNULL, &newhosts);

    memset(&hostaddr, 0, sizeof(hostaddr));
    get_ip("10.0.0.203", &hostaddr);
    MakeDomainNameFromDNSNameString(&domain, "123server.dotbennu.com");
    _TestCreateEtcHostsEntryWithInterfaceID(&domain, (struct sockaddr *) &hostaddr, mDNSNULL, primary_interfaceID, &newhosts);

    UpdateEtcHosts_ut(&newhosts);
    m->NextScheduledEvent = mDNS_TimeNow_NoLock(m);
    mDNS_Execute(m);

    return mStatus_NoError;
}

mDNSlocal mDNSs32 NumReplies(reply_state * reply)
{
    mDNSs32 result = 0;
    reply_state * nextreply = reply;
    while(nextreply) { result++; nextreply = nextreply->next;}
    return result;
}

mDNSlocal mDNSBool HasReplyWithInterfaceIndex(reply_state * reply, mDNSu32 interfaceIndex)
{
    mDNSBool result = mDNSfalse;
    reply_state * nextreply = reply;
    while(nextreply)
    {
        result = (ntohl(nextreply->rhdr[0].ifi) == interfaceIndex);
        if (result) break;
        nextreply = nextreply->next;
    }
    return result;
}

@interface LocalOnlyWithInterfacesTest : XCTestCase
{
    UDPSocket* local_socket;
    request_state* client_request_message;}
@end

@implementation LocalOnlyWithInterfacesTest

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

    AddDNSServerScoped_ut(primary_interfaceID, kScopeInterfaceID);
    XCTAssertEqual(CountOfUnicastDNSServers(&mDNSStorage), 2);

    // Populate /etc/hosts
    result = InitEtcHostsRecords();
    XCTAssertEqual(result, mStatus_NoError);
    
    int count = LogEtcHosts_ut(&mDNSStorage);
    XCTAssertEqual(count, 3);

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

// This unit test tries 3 different local cache queries
// 1) Test the Any query does not receive the entry scoped to the primary interface, but does received the local and P2P entries
// 2) Test the LocalOnly query receives all the entries
// 3) Test the interface scoped query receives the interface scoped entry
- (void)testLocalOnlyWithInterfacesTestSeries
{
    request_state* req = client_request_message;

    // Verify Any index returns 2 results.
    [self _executeClientQueryRequest: req andMsgBuf: test_query_any_msgbuf];
    XCTAssertEqual(NumReplies(req->replies), 2);
    XCTAssertTrue(HasReplyWithInterfaceIndex(req->replies, kDNSServiceInterfaceIndexP2P));
    XCTAssertTrue(HasReplyWithInterfaceIndex(req->replies, kDNSServiceInterfaceIndexLocalOnly));

    // Verify LocalOnly index returns 3 results.
    [self _executeClientQueryRequest: req andMsgBuf: test_query_local_msgbuf];
    XCTAssertEqual(NumReplies(req->replies), 3);
    XCTAssertTrue(HasReplyWithInterfaceIndex(req->replies, kDNSServiceInterfaceIndexP2P));
    XCTAssertTrue(HasReplyWithInterfaceIndex(req->replies, kDNSServiceInterfaceIndexLocalOnly));
    XCTAssertTrue(HasReplyWithInterfaceIndex(req->replies, primary_interfaceID));

    // Verify en0 index returns 1 result.
    test_query_interface_msgbuf[7] = primary_interfaceID;
    [self _executeClientQueryRequest: req andMsgBuf: test_query_interface_msgbuf];
    XCTAssertEqual(NumReplies(req->replies), 1);
    XCTAssertTrue(HasReplyWithInterfaceIndex(req->replies, primary_interfaceID));

}

// Simulate a uds client request by setting up a client request and then
// calling mDNSResponder's handle_client_request.  The handle_client_request function
// processes the request and starts a query.  This unit test verifies
// the client request and query were setup as expected.  This unit test also calls
// mDNS_execute which determines the cache does not contain the new question's
// answer.
- (void)_executeClientQueryRequest: (request_state*)req andMsgBuf: (char*)msgbuf
{
    mDNS *const m = &mDNSStorage;
    char *msgptr = msgbuf;
    size_t msgsz = sizeof(test_query_local_msgbuf);
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
    
    // Verify the query fields were set as expected
    q = &req->u.queryrecord.op.q;
    XCTAssertNotEqual(q, (DNSQuestion *)mDNSNULL);
    if (m->Questions)
    {
        XCTAssertEqual(q, m->Questions);
        XCTAssertEqual(q, m->NewQuestions);
        XCTAssertTrue(q->InterfaceID == mDNSInterface_Any || q->InterfaceID == primary_interfaceID);
    }
    else
    {
        XCTAssertEqual(q, m->LocalOnlyQuestions);
        XCTAssertEqual(q, m->NewLocalOnlyQuestions);
        XCTAssertEqual(q->InterfaceID, mDNSInterface_LocalOnly);
    }
    XCTAssertEqual(q->SuppressUnusable, mDNSfalse);
    XCTAssertEqual(q->ReturnIntermed, mDNStrue);
    XCTAssertEqual(q->Suppressed, mDNSfalse);
    
    ConvertDomainNameToCString(&q->qname, qname_cstr);
    XCTAssertFalse(strcmp(qname_cstr, test_domainname_cstr));
    XCTAssertEqual(q->qnamehash, DomainNameHashValue(&q->qname));
    
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
    XCTAssertNil((__bridge id)m->NewLocalOnlyQuestions);
    XCTAssertEqual(m->rrcache_totalused, 0);
    m->Questions = nil;                 //  Reset
}


@end
