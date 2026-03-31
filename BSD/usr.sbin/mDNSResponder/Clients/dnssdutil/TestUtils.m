//
//  TestUtils.m
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#import "TestUtils.h"

#import <Foundation/Foundation.h>
#import <CoreUtils/CoreUtils.h>
#import <XCTest/XCTest.h>

#if TARGET_OS_OSX
#define XCTest_Framework_Runtime_Path       "/AppleInternal/Developer/Library/Frameworks/XCTest.framework"
#else
#define XCTest_Framework_Runtime_Path       "/Developer/Library/Frameworks/XCTest.framework"
#endif

//===========================================================================================================================
//    XCTest Utils
//===========================================================================================================================
static NSBundle * LoadXCTestFramework()
{
    NSBundle * result = nil;
    Boolean loaded = (NSClassFromString(@"XCTestSuite") != nil);

    if(!result) {
        result = [NSBundle bundleWithPath: @ XCTest_Framework_Runtime_Path];
        [result load];
        loaded = (NSClassFromString(@"XCTestSuite") != nil);
        if( !loaded ) {
            FPrintF( stdout, "Failed to load XCTest framework from: %s\n", XCTest_Framework_Runtime_Path );
        }
    }

    return( result );
}

//===========================================================================================================================
//    Main Test Running
//===========================================================================================================================

Boolean TestUtilsRunXCTestNamed(const char * classname)
{
    Boolean     result = false;
    NSBundle *  xctestFramework = LoadXCTestFramework();

    if(xctestFramework) {
        NSString *  name = [NSString stringWithUTF8String: classname];
        NSBundle *  testBundle = [NSBundle bundleWithPath: @"/AppleInternal/XCTests/com.apple.mDNSResponder/Tests.xctest"];
        [testBundle load];

        XCTestSuite *   compiledSuite = [NSClassFromString(@"XCTestSuite") testSuiteForTestCaseWithName: name];
        if(compiledSuite.tests.count) {
            [compiledSuite runTest];
            XCTestRun *testRun = compiledSuite.testRun;
            result = (testRun.hasSucceeded != NO);
        } else {
            FPrintF( stdout, "Test class %s not found\n", classname );
        }
    }

    return( result );
}
