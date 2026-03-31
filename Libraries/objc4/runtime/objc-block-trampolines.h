/*
 * Copyright (c) 2018 Apple Inc.  All Rights Reserved.
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
 */


#ifndef _OBJC_TRAMPOLINES_H
#define _OBJC_TRAMPOLINES_H

/* 
 * WARNING  DANGER  HAZARD  BEWARE  EEK
 * 
 * Everything in this file is for Apple Internal use only.
 * These will change in arbitrary OS updates and in unpredictable ways.
 * When your program breaks, you get to keep both pieces.
 */

/*
 * objc-block-trampolines.h: Symbols for IMP block trampolines
 */

// WARNING: remapped code and dtrace do not play well together. Dtrace
// will place trap instructions to instrument the code, which then get
// remapped along with everything else. The remapped traps are not
// recognized by dtrace and the process crashes. To avoid this, dtrace
// blacklists this library by name. Do not change the name of this
// library. rdar://problem/42627391

#include <objc/objc-api.h>

OBJC_EXPORT const char _objc_blockTrampolineImpl
    OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);

OBJC_EXPORT const char _objc_blockTrampolineStart
    OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);

OBJC_EXPORT const char _objc_blockTrampolineLast
    OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);


OBJC_EXPORT const char _objc_blockTrampolineImpl_stret
OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0)
    OBJC_ARM64_UNAVAILABLE;

OBJC_EXPORT const char _objc_blockTrampolineStart_stret
OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0)
    OBJC_ARM64_UNAVAILABLE;

OBJC_EXPORT const char _objc_blockTrampolineLast_stret
OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0)
    OBJC_ARM64_UNAVAILABLE;

#endif
