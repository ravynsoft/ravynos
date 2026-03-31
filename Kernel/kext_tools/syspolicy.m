/*
 *  syspolicy.m
 *  kext_tools
 *
 *  Copyright 2017 Apple Inc. All rights reserved.
 *
 */
#import <Foundation/Foundation.h>

#import <SystemPolicy/SystemPolicy.h>
#import <bootstrap.h>
#import "syspolicy.h"
#import "kext_tools_util.h"

// Basic global state - perform initialization once and rely on that for future calls
static BOOL gInitialized = NO;
static SPKernelExtensionPolicy *gSystemPolicy = nil;

static BOOL
isSystemPolicyLinked() {
    return NSClassFromString(@"SPKernelExtensionPolicy") ? YES : NO;
}

static BOOL
isSystemPolicyServiceAvailable() {
    BOOL serviceIsAvailable = NO;
    mach_port_t syspolicy_port = MACH_PORT_NULL;
    kern_return_t kern_result = 0;
    kern_result = bootstrap_look_up(bootstrap_port,
                                    "com.apple.security.syspolicy.kext",
                                    &syspolicy_port);
    serviceIsAvailable = (kern_result == 0 && syspolicy_port != 0);
    mach_port_deallocate(mach_task_self(), syspolicy_port);
    return serviceIsAvailable;
}

static void
initializeGlobalState() {
    BOOL useSystemPolicy = isSystemPolicyLinked() && isSystemPolicyServiceAvailable();
    if (useSystemPolicy) {
        gSystemPolicy = [[SPKernelExtensionPolicy alloc] init];
    }
    gInitialized = YES;
}

Boolean
SPAllowKextLoad(OSKextRef kext) {
    Boolean allowed = true;

    if (!gInitialized) {
        initializeGlobalState();
    }

    if (gSystemPolicy) {
        NSString *kextPath = (__bridge_transfer NSString*)copyKextPath(kext);
        allowed = [gSystemPolicy canLoadKernelExtension:kextPath error:nil] ? true : false;
    }

    return allowed;
}

Boolean
SPAllowKextLoadCache(OSKextRef kext) {
    Boolean allowed = true;

    if (!gInitialized) {
        initializeGlobalState();
    }

    if (gSystemPolicy) {
        NSString *kextPath = (__bridge_transfer NSString*)copyKextPath(kext);
        allowed = [gSystemPolicy canLoadKernelExtensionInCache:kextPath error:nil] ? true : false;
    }

    return allowed;
}
