/*
* Copyright (c) 2019 Apple Inc. All rights reserved.
*
* @APPLE_LICENSE_HEADER_START@
*
* "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
* Reserved.  This file contains Original Code and/or Modifications of
* Original Code as defined in and that are subject to the Apple Public
* Source License Version 1.0 (the 'License').  You may not use this file
* except in compliance with the License.  Please obtain a copy of the
* License at http://www.apple.com/publicsource and read it before using
* this file.
*
* The Original Code and all software distributed under the License are
* distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
* INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
* License for the specific language governing rights and limitations
* under the License."
*
* @APPLE_LICENSE_HEADER_END@
*/

#import <XCTest/XCTest.h>
#include <CommonCrypto/CommonDigest.h>
#include <objc/runtime.h>

#include "test_support.h"

struct TestInfo
{
    const char*         testName;
    const char*         runLine;
};

#include "XCTestGenerated.h"

@interface ContainerizedTestRunner : XCTestCase
- (void)launchTest:(const char *)test withRunLine:(const char *)runLine;
@end

@implementation ContainerizedTestRunner
#if 1
+ (void)registerTest:(const TestInfo &)info withEnv:(const char *)env andExtensions:(const char *)extension {
    char *fullRunline = NULL;
    asprintf(&fullRunline, "TMPDIR=/tmp/ TEST_OUTPUT=XCTest  %s %s", env, info.runLine);
    unsigned char hash[CC_SHA1_DIGEST_LENGTH];
    char hashStr[CC_SHA1_DIGEST_LENGTH*2+1];
    CC_SHA1(fullRunline, (CC_LONG)strlen(fullRunline), &hash[0]);
    snprintf(&hashStr[0], CC_SHA1_DIGEST_LENGTH*2+1, "%x%x%x%x", hash[0], hash[1], hash[2], hash[3]);
    char buffer[4096];
    snprintf(&buffer[0], 4096, "test_%s_%s_%s", info.testName, extension, hashStr);
    SEL newSel = sel_registerName(buffer);
    IMP newIMP = imp_implementationWithBlock(^(id self) {
        [self launchTest:info.testName withRunLine:fullRunline];
    });
    class_addMethod([self class], newSel, newIMP, "v@:");
}

+ (void)load {
    for (const TestInfo& info : sTests) {
        [self registerTest:info withEnv:"TEST_DYLD_MODE=2 DYLD_USE_CLOSURES=0" andExtensions:"dyld2"];
        [self registerTest:info withEnv:"TEST_DYLD_MODE=2 DYLD_USE_CLOSURES=0 DYLD_SHARED_REGION=avoid" andExtensions:"dyld2_nocache"];
        [self registerTest:info withEnv:"TEST_DYLD_MODE=3 DYLD_USE_CLOSURES=1" andExtensions:"dyld3"];
        [self registerTest:info withEnv:"TEST_DYLD_MODE=3 DYLD_USE_CLOSURES=1 DYLD_SHARED_REGION=avoid" andExtensions:"dyld3_nocache"];
#if 0
        [self registerTest:info withEnv:"TEST_DYLD_MODE=2 DYLD_USE_CLOSURES=0 MallocStackLogging=1 MallocDebugReport=none" andExtensions:"dyld2_leaks"];
        [self registerTest:info withEnv:"TEST_DYLD_MODE=2 DYLD_USE_CLOSURES=0 DYLD_SHARED_REGION=avoid MallocStackLogging=1 MallocDebugReport=none" andExtensions:"dyld2_nocache_leaks"];
        [self registerTest:info withEnv:"TEST_DYLD_MODE=3 DYLD_USE_CLOSURES=1 MallocStackLogging=1 MallocDebugReport=none" andExtensions:"dyld3_leaks"];
        [self registerTest:info withEnv:"TEST_DYLD_MODE=3 DYLD_USE_CLOSURES=1 DYLD_SHARED_REGION=avoid MallocStackLogging=1 MallocDebugReport=none" andExtensions:"dyld3_nocache_leaks"];
#endif
    };
}

#else
// This would be the way to do it if XCTest did not insist on using the selector name for differentiating results in the Xcode UI
+ (NSArray<NSInvocation *> *)testInvocations {
    static NSArray<NSInvocation *> *invocations = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSMutableArray<NSInvocation *> *invocationArray = [[NSMutableArray alloc] init];
        for (const TestInfo& info : sTests) {
            NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:[[ContainerizedTestRunner class]
                                                instanceMethodSignatureForSelector:@selector(launchTest:withRunLine:)]];
            invocation.selector = @selector(launchTest:withRunLine:);
            [invocation setArgument:(void*)&info.testName atIndex:2];
            [invocation setArgument:(void*)&info.runLine atIndex:3];
            [invocationArray addObject:invocation];
        }
        invocations = [NSArray arrayWithArray:invocationArray];
    });
    return [invocations copy];
}

- (NSString *) name {
    const char *testName = NULL;
    const char *runLine = NULL;
    [self.invocation getArgument:(void*)&testName atIndex:2];
    [self.invocation getArgument:(void*)&runLine atIndex:3];
    return [NSString stringWithFormat:@"%s: %s", testName, runLine];
}

- (NSString *)nameForLegacyLogging
{
    return self.name;
}
#endif

- (void)setUp {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _process process;
        fprintf(stderr, "CHROOT: %s\n", CHROOT_PATH);
        process.set_executable_path("/usr/sbin/chroot");
        const char *args[] = {CHROOT_PATH, "/bin/sh", "-c", "/sbin/mount -t devfs devfs /dev", NULL};
        process.set_args(args);
        process.launch();
    });
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void) executeCommandInContainer:(const char *)command {
    _process process;
    process.set_executable_path("/usr/sbin/chroot");
    const char *args[] = {CHROOT_PATH, "/bin/sh", "-c", command, NULL};
    process.set_args(args);
    __block dispatch_data_t output = NULL;
    process.set_stdout_handler(^(int fd) {
        ssize_t size = 0;
        do {
            char buffer[16384];
            size = read(fd, &buffer[0], 16384);
            if (size == -1) { break; }
            dispatch_data_t data = dispatch_data_create(&buffer[0], size, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
            if (!output) {
                output = data;
            } else {
                output = dispatch_data_create_concat(output, data);
            }
        } while (size > 0);
    });
    process.set_exit_handler(^(pid_t pid) {
        int status = 0;
        (void)waitpid(pid, &status, 0);

        int exitStatus = WEXITSTATUS(status);
        if (exitStatus != 0) {
            NSString *failure = [NSString stringWithFormat:@"Test exited with return code %d\n%s", exitStatus, command];
            [self recordFailureWithDescription:failure inFile:@__FILE__ atLine:__LINE__ expected:NO];
            return;
        }
        if (!output) {
            NSString *failure = [NSString stringWithFormat:@"Test did not write any data to stdout\n%s", command];
            [self recordFailureWithDescription:failure inFile:@__FILE__ atLine:__LINE__ expected:NO];
            return;
        }
        NSError *error = nil;
        NSDictionary *dict = [NSPropertyListSerialization propertyListWithData:(NSData *)output options:NSPropertyListImmutable format:nil error:&error];
        if (!dict) {
            NSString *failure = [NSString stringWithFormat:@"Could not convert stdout \"%@\" to property list. Got Error %@\n%s", output, error, command];
            [self recordFailureWithDescription:failure inFile:@__FILE__ atLine:__LINE__ expected:NO];
            return;
        }
        if (![dict[@"PASS"] boolValue]) {
            NSString *failure = [NSString stringWithFormat:@"%@\n%s", dict[@"INFO"], command];
            [self recordFailureWithDescription:failure inFile:dict[@"FILE"] atLine:[dict[@"LINE"] unsignedIntegerValue] expected:NO];
            return;
        }
    });
    process.launch();
}


- (void) launchTest:(const char *)test withRunLine:(const char *)runLine {
    char command[4096];
    snprintf(&command[0], 4096, "cd /AppleInternal/CoreOS/tests/dyld/%s; %s", test, runLine);
    [self executeCommandInContainer:command];
// sudo chroot . /bin/sh -c 'TEST_OUTPUT=BATS /AppleInternal/CoreOS/tests/dyld/dyld_get_sdk_version/sdk-check.exe'

}

//
//- (void)testExample {
//    // This is an example of a functional test case.
//    // Use XCTAssert and related functions to verify your tests produce the correct results.
//}
//
//- (void)testPerformanceExample {
//    // This is an example of a performance test case.
//    [self measureBlock:^{
//        // Put the code you want to measure the time of here.
//    }];
//}

@end
