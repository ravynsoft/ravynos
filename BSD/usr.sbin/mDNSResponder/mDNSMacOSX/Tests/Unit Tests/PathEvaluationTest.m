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

#import <NetworkExtension/NEPolicySession.h>

@interface PathEvaluationTest : XCTestCase
{
}
@end

@implementation PathEvaluationTest

- (void)setUp
{
    mDNSPlatformMemZero(&mDNSStorage, sizeof(mDNS));
    init_mdns_environment(mDNStrue);
}

- (void)tearDown
{
}

- (void)testPathDeny
{
    if(!getenv("DNSSDUTIL_XCTEST")) return;   //   Don't run without this environment variable
    mDNSBool        isBlocked;
    DNSQuestion     q;
    mDNSInterfaceID routableIndex;

    mDNSPlatformMemZero(&q, sizeof(DNSQuestion));
    q.TargetQID.NotAnInteger = 1;
    q.pid = getpid();
    q.InterfaceID = if_nametoindex( "pdp_ip0" );
    fprintf(stdout, "%s %s with cellular index %d named pdp_ip0\n", q.InterfaceID ? "Starting" : "Exiting (no cellular interface)", __FUNCTION__, q.InterfaceID);
    if (!q.InterfaceID) return;

    routableIndex = IndexForInterfaceByName_ut( "pdp_ip0" );
    fprintf(stdout, "Testing blocked by (%s)\n", routableIndex ? "policy" : "no route");

    mDNSPlatformGetDNSRoutePolicy(&q, &isBlocked);
    XCTAssertFalse(isBlocked);

    // Now block it
    NSMutableArray *routeRules = [NSMutableArray array];
    NEPolicyRouteRule *routeRule = [NEPolicyRouteRule routeRuleWithAction:NEPolicyRouteRuleActionDeny forType:NEPolicyRouteRuleTypeCellular];
    [routeRules addObject:routeRule];
    routeRule = [NEPolicyRouteRule routeRuleWithAction:NEPolicyRouteRuleActionDeny forType:NEPolicyRouteRuleTypeWiFi];
    [routeRules addObject:routeRule];
    routeRule = [NEPolicyRouteRule routeRuleWithAction:NEPolicyRouteRuleActionDeny forType:NEPolicyRouteRuleTypeWired];
    [routeRules addObject:routeRule];

    NEPolicyResult *result = [NEPolicyResult routeRules:routeRules];
    NEPolicy *policy = [[NEPolicy alloc] initWithOrder:1 result:result  conditions:@[ [NEPolicyCondition effectivePID:q.pid], [NEPolicyCondition allInterfaces] ]];

    NEPolicySession  *policySession = [[NEPolicySession alloc] init];
    XCTAssertNotNil(policySession, "Check entitlemnts");
    [policySession addPolicy:policy];
    [policySession apply];

    mDNSPlatformGetDNSRoutePolicy(&q, &isBlocked);
    //  Either if these asserts indicate a regression in mDNSPlatformGetDNSRoutePolicy
    if (routableIndex)  XCTAssertTrue(isBlocked, "blocked by (policy) test failure");
    else                XCTAssertFalse(isBlocked, "blocked by (no route) test failure");

    [policySession removeAllPolicies];
    [policySession apply];
    fprintf(stdout, "Completed %s\n", __FUNCTION__);
}

@end
