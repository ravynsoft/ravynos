/*
 * Copyright (c) 2017-2018 Apple Inc. All rights reserved.
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

// This DNS message was gleaned from a uDNS query request packet that was captured with Wireshark.
uint8_t udns_query_request_message[28] = {            // contains 1 question for www.f5.com
    0x31, 0xca, // transaction id
    0x01, 0x00,    // flags
    0x00, 0x01,    // 1 question
    0x00, 0x00,    // no anwsers
    0x00, 0x00,    // no authoritative answers
    0x00, 0x00, // no additionals
    0x03, 0x77, 0x77, 0x77, 0x02, 0x66, 0x35, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01
};

// This DNS message was gleaned from a uDNS query request packet that was captured with Wireshark.
// Then the header id (more specifically, the msg->h.id) was deliberately cleared to force code
// path to traverse regression case, <rdar://problem/28556513>.
uint8_t udns_query_request_message_with_invalid_id[28] = {  // contains 1 question for www.f5.com, msg->h.id = 0
    0x00, 0x00,    // transaction id
    0x01, 0x00, // flags
    0x00, 0x01, // 1 question
    0x00, 0x00, // no anwsers
    0x00, 0x00, // no authoritative answers
    0x00, 0x00, // no additionals
    0x03, 0x77, 0x77, 0x77, 0x02, 0x66, 0x35, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01
};

uint8_t arp_request_packet[28] = {  // contains 1 question for www.f5.com, msg->h.id = 0
    0x00, 0x01, // hardware type: enet
    0x08, 0x00, // protocol type: IP
    0x06, // hardware size
    0x04, // Protcol size
    0x00, 0x01, // opcode request
    0x24, 0x01, 0xc7, 0x24, 0x35, 0x00, // Sender mac addr
    0x11, 0xe2, 0x14, 0x01,    // Sender ip addr
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // target mac addr
    0x11, 0xe2, 0x17, 0xbe    // target ip addr
};

mDNSlocal void InitmDNSStorage(mDNS *const m)
{
    memset(m, 0, sizeof(mDNS));
}

@interface mDNSCoreReceiveTest : XCTestCase
{
}
@end

@implementation mDNSCoreReceiveTest

- (void)setUp
{
    mDNSPlatformTimeInit();
    mDNS_LoggingEnabled = 0;
    mDNS_PacketLoggingEnabled = 0;
}

- (void)tearDown {
}

- (void)testValidQueryReq
{
    mDNS *const m = &mDNSStorage;
    mDNSAddr srcaddr, dstaddr;
    mDNSIPPort srcport, dstport;
    DNSMessage * msg;
    const mDNSu8 * end;
    
    // Init unit test environment and verify no error occurred.
    mStatus result = init_mdns_environment(mDNStrue);
    XCTAssertEqual(result, mStatus_NoError);
    
    // Used random values for srcaddr and srcport
    srcaddr.type       = mDNSAddrType_IPv4;
    srcaddr.ip.v4.b[0] = 192;
    srcaddr.ip.v4.b[1] = 168;
    srcaddr.ip.v4.b[2] = 1;
    srcaddr.ip.v4.b[3] = 10;
    srcport.NotAnInteger = swap16((mDNSu16)53);
    
    // Used random values for dstaddr and dstport
    dstaddr.type       = mDNSAddrType_IPv4;
    dstaddr.ip.v4.b[0] = 192;
    dstaddr.ip.v4.b[1] = 168;
    dstaddr.ip.v4.b[2] = 1;
    dstaddr.ip.v4.b[3] = 20;
    dstport.NotAnInteger = swap16((mDNSu16)49339);
    
    // Set message to a DNS message (copied from a WireShark packet)
    msg = (DNSMessage *)udns_query_request_message;
    end = udns_query_request_message + sizeof(udns_query_request_message);
    
    // Execute mDNSCoreReceive using a valid DNS message
    mDNSCoreReceive(m, msg, end, &srcaddr, srcport, &dstaddr, dstport, if_nametoindex("en0"));
    
    // Verify that mDNSCoreReceiveQuery traversed the normal code path
    XCTAssertEqual(m->mDNSStats.NormalQueries,  1);
}

- (void)testNullDstQueryReqTest
{
    mDNS *const m = &mDNSStorage;
    mDNSAddr srcaddr;
    mDNSIPPort srcport, dstport;
    DNSMessage * msg;
    const mDNSu8 * end;

    // This test case does not require setup of interfaces, the record's cache, or pending questions
    // so m is initialized to all zeros.
    InitmDNSStorage(m);

    // Used random values for srcaddr and srcport
    srcaddr.type       = mDNSAddrType_IPv4;
    srcaddr.ip.v4.b[0] = 192;
    srcaddr.ip.v4.b[1] = 168;
    srcaddr.ip.v4.b[2] = 1;
    srcaddr.ip.v4.b[3] = 10;
    srcport.NotAnInteger = swap16((mDNSu16)53);

    // Used random value for dstport
    dstport.NotAnInteger = swap16((mDNSu16)49339);

    // Set message to a DNS message (copied from a WireShark packet)
    msg = (DNSMessage *)udns_query_request_message_with_invalid_id;
    end = udns_query_request_message_with_invalid_id + sizeof(udns_query_request_message_with_invalid_id);

    // Execute mDNSCoreReceive to regress <rdar://problem/28556513>
    mDNSCoreReceive(m, msg, end, &srcaddr, srcport, NULL, dstport, if_nametoindex("en0"));

    // Verify that mDNSCoreReceiveQuery was NOT traversed through the normal code path
    XCTAssertEqual(m->mDNSStats.NormalQueries,  0);

    // Verify code path that previously crashed, in <rdar://problem/28556513>, now traverses successfully
    // by checking a counter that was incremented on code path that crashed.
    XCTAssertEqual(m->MPktNum,                  1);
}

#if 0
- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}
#endif

@end
