/*
 * Copyright (c) 2012 Apple Inc. All Rights Reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *  CCCryptorReset_internal.h
 */

#ifndef CCCryptorReset_internal_h
#define CCCryptorReset_internal_h

#if !defined (_MSC_VER) && !defined(__ANDROID__)
#include <os/variant_private.h>
#include <mach-o/dyld_priv.h>
#endif

#include <TargetConditionals.h>

// refer to TargetConditionals.h for the target macro hierarchy
#if   TARGET_OS_OSX
#define CC_DYLOAD_PROGRAM_SDK_VERSION       DYLD_MACOSX_VERSION_10_13
#elif TARGET_OS_IPHONE
#define CC_DYLOAD_PROGRAM_SDK_VERSION       DYLD_IOS_VERSION_11_0
#elif defined (_MSC_VER) || defined (__ANDROID__)
#define CC_DYLOAD_PROGRAM_SDK_VERSION       0x01
#else
#error CC_DYLOAD_PROGRAM_SDK_VERSION undefined
#endif

#if defined (_MSC_VER) || defined(__ANDROID__)
#define ProgramLinkedOnOrAfter_macOS1013_iOS11() true
#else
#define ProgramLinkedOnOrAfter_macOS1013_iOS11()  (dyld_get_program_sdk_version() >= CC_DYLOAD_PROGRAM_SDK_VERSION)
#endif
#endif /* CCCryptorReset_internal_h */
