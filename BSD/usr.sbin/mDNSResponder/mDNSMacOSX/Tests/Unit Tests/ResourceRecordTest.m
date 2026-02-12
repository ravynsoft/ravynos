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

@interface ResourceRecordTest : XCTestCase
{
}
@end

@implementation ResourceRecordTest

- (void)setUp
{
}

- (void)tearDown
{
}

- (void)testTXTSetup
{
    AuthRecord authRec;
    mDNS_SetupResourceRecord(&authRec, mDNSNULL, mDNSInterface_Any, kDNSType_TXT, kStandardTTL, kDNSRecordTypeShared, AuthRecordAny,mDNSNULL, mDNSNULL);
    XCTAssertEqual(authRec.resrec.rrtype,               kDNSType_TXT);
    XCTAssertEqual(authRec.resrec.RecordType,           kDNSRecordTypeShared);
    XCTAssertEqual(authRec.resrec.rdata->MaxRDLength,   sizeof(RDataBody));
}

- (void)testASetup
{
    AuthRecord authRec;
    mDNS_SetupResourceRecord(&authRec, mDNSNULL, mDNSInterface_Any, kDNSType_A, kHostNameTTL, kDNSRecordTypeUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
    
    XCTAssertEqual(authRec.resrec.rrtype,           kDNSType_A);
    XCTAssertEqual(authRec.resrec.RecordType,       kDNSRecordTypeUnique);
    // Add more verifications
}

- (void)testOPTSetup
{
    AuthRecord opt;
    mDNSu32    updatelease = 7200;

    // Setup the OPT Record
    mDNS_SetupResourceRecord(&opt, mDNSNULL, mDNSInterface_Any, kDNSType_OPT, kStandardTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);

    // Verify the basic initialization is all ok

    opt.resrec.rrclass    = NormalMaxDNSMessageData;
    opt.resrec.rdlength   = sizeof(rdataOPT);   // One option in this OPT record
    opt.resrec.rdestimate = sizeof(rdataOPT);
    opt.resrec.rdata->u.opt[0].opt           = kDNSOpt_Lease;
    opt.resrec.rdata->u.opt[0].u.updatelease = updatelease;

    // Put the resource record in and verify everything is fine
#if 0
    mDNSu8     data[AbsoluteMaxDNSMessageData];
    mDNSu8     *p = data;
    mDNSu16    numAdditionals;
    
    p = PutResourceRecordTTLWithLimit((DNSMessage*)&data, p, &numAdditionals, &opt.resrec, opt.resrec.rroriginalttl, data + AbsoluteMaxDNSMessageData);
#endif
}

// Repeat with bad data to make sure it bails out cleanly

#if 0
- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}
#endif

@end
