// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//


/*	CFRunArray.h
	Copyright (c) 2004-2019, Apple Inc. All rights reserved.
 
        Contains CFRunArray
*/

#if !defined(__COREFOUNDATION_CFRUNARRAY__)
#define __COREFOUNDATION_CFRUNARRAY__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>
#include <stdarg.h>

CF_EXTERN_C_BEGIN

/* CFRunArray is mutable only.  It does manage it's internal data structure with copy-on-write behavior though.
*/
typedef struct __CFRunArray *CFRunArrayRef;

/*! @function CFRunArrayGetTypeID
Returns the type identifier of all CFAttributedString instances.
*/
CF_EXPORT CFTypeID CFRunArrayGetTypeID(void);

CF_EXPORT CFRunArrayRef CFRunArrayCreate(CFAllocatorRef allocator);

CF_EXPORT CFIndex CFRunArrayGetCount(CFRunArrayRef array);
CF_EXPORT CFTypeRef CFRunArrayGetValueAtIndex(CFRunArrayRef array, CFIndex loc, CFRange *effectiveRange, CFIndex *runArrayIndexPtr);
CF_EXPORT CFTypeRef CFRunArrayGetValueAtRunArrayIndex(CFRunArrayRef array, CFIndex runArrayIndex, CFIndex *countPtr);

/* Inserts range.length instances of obj at location range.location
*/
CF_EXPORT void CFRunArrayInsert(CFRunArrayRef array, CFRange range, CFTypeRef obj);

/* Delete values specified by the range
*/
CF_EXPORT void CFRunArrayDelete(CFRunArrayRef array, CFRange range);

/* Replaces values in the specified range with count instances of obj
*/
CF_EXPORT void CFRunArrayReplace(CFRunArrayRef array, CFRange range, CFTypeRef obj, CFIndex count);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFRUNARRAY__ */

