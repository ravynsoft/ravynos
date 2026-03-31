/*
 *  syspolicy.h
 *  kext_tools
 *
 *  Copyright 2017 Apple Inc. All rights reserved.
 *
 */
#pragma once

#import <IOKit/kext/OSKext.h>

#if !TARGET_OS_EMBEDDED
#define HAVE_SYSTEM_POLICY 1
#else // !TARGET_OS_EMBEDDED
#define HAVE_SYSTEM_POLICY 0
#endif // !TARGET_OS_EMBEDDED

Boolean SPAllowKextLoad(OSKextRef kext);
Boolean SPAllowKextLoadCache(OSKextRef kext);
