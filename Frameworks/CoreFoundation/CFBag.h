/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFBag.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFBAG__)
#define __COREFOUNDATION_CFBAG__ 1

#include <CoreFoundation/CFBase.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef const void *	(*CFBagRetainCallBack)(CFAllocatorRef allocator, const void *value);
typedef void		(*CFBagReleaseCallBack)(CFAllocatorRef allocator, const void *value);
typedef CFStringRef	(*CFBagCopyDescriptionCallBack)(const void *value);
typedef Boolean		(*CFBagEqualCallBack)(const void *value1, const void *value2);
typedef CFHashCode	(*CFBagHashCallBack)(const void *value);
typedef struct {
    CFIndex				version;
    CFBagRetainCallBack			retain;
    CFBagReleaseCallBack		release;
    CFBagCopyDescriptionCallBack	copyDescription;
    CFBagEqualCallBack			equal;
    CFBagHashCallBack			hash;
} CFBagCallBacks;

CF_EXPORT
const CFBagCallBacks kCFTypeBagCallBacks;
CF_EXPORT
const CFBagCallBacks kCFCopyStringBagCallBacks;

typedef void (*CFBagApplierFunction)(const void *value, void *context);

typedef const struct __CFBag * CFBagRef;
typedef struct __CFBag * CFMutableBagRef;

CF_EXPORT
CFTypeID CFBagGetTypeID(void);

CF_EXPORT
CFBagRef CFBagCreate(CFAllocatorRef allocator, const void **values, CFIndex numValues, const CFBagCallBacks *callBacks);

CF_EXPORT
CFBagRef CFBagCreateCopy(CFAllocatorRef allocator, CFBagRef theBag);

CF_EXPORT
CFMutableBagRef CFBagCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFBagCallBacks *callBacks);

CF_EXPORT
CFMutableBagRef CFBagCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFBagRef theBag);

CF_EXPORT
CFIndex CFBagGetCount(CFBagRef theBag);

CF_EXPORT
CFIndex CFBagGetCountOfValue(CFBagRef theBag, const void *value);

CF_EXPORT
Boolean CFBagContainsValue(CFBagRef theBag, const void *value);

CF_EXPORT
const void *CFBagGetValue(CFBagRef theBag, const void *value);

CF_EXPORT
Boolean CFBagGetValueIfPresent(CFBagRef theBag, const void *candidate, const void **value);

CF_EXPORT
void CFBagGetValues(CFBagRef theBag, const void **values);

CF_EXPORT
void CFBagApplyFunction(CFBagRef theBag, CFBagApplierFunction applier, void *context);

CF_EXPORT
void CFBagAddValue(CFMutableBagRef theBag, const void *value);

CF_EXPORT
void CFBagReplaceValue(CFMutableBagRef theBag, const void *value);

CF_EXPORT
void CFBagSetValue(CFMutableBagRef theBag, const void *value);

CF_EXPORT
void CFBagRemoveValue(CFMutableBagRef theBag, const void *value);

CF_EXPORT
void CFBagRemoveAllValues(CFMutableBagRef theBag);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFBAG__ */

