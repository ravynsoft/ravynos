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

#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"
#import <XCTest/XCTest.h>

@interface DNSMessageTest : XCTestCase
{
    DNSMessage *msg;
}
@end

@implementation DNSMessageTest

- (void)setUp
{
    msg = (DNSMessage *)malloc (sizeof(DNSMessage));
    XCTAssert(msg != NULL);
    
    // message header should be 12 bytes
    XCTAssertEqual(sizeof(msg->h),        12);
}

- (void)tearDown
{
    XCTAssert(msg != NULL);
    free(msg);
}

- (void)testMessageInitialization
{
    // Initialize the message
    InitializeDNSMessage(&msg->h, onesID, QueryFlags);
    
    // Check that the message is initialized properly
    XCTAssertEqual(msg->h.numAdditionals, 0);
    XCTAssertEqual(msg->h.numAnswers,     0);
    XCTAssertEqual(msg->h.numQuestions,   0);
    XCTAssertEqual(msg->h.numAuthorities, 0);
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
