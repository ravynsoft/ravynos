/*  CFListFormatter.h
    Copyright (c) 2018-2019, Apple Inc. and the Swift project authors

    Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
    Licensed under Apache License v2.0 with Runtime Library Exception
    See http://swift.org/LICENSE.txt for license information
    See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#ifndef __COREFOUNDATION_CFLISTFORMATTER_h
#define __COREFOUNDATION_CFLISTFORMATTER_h

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFLocale.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

typedef struct CF_BRIDGED_TYPE(id) __CFListFormatter *CFListFormatterRef;

CF_EXPORT
CFTypeID _CFListFormatterGetTypeID(void);

CFListFormatterRef _Nullable _CFListFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale);
CFStringRef _Nullable _CFListFormatterCreateStringByJoiningStrings(CFAllocatorRef allocator, CFListFormatterRef formatter, const CFArrayRef strings);

CF_ASSUME_NONNULL_END
CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif // __COREFOUNDATION_CFLISTFORMATTER_h
