/*    CFNumber_Private.h
 Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#if !defined(__COREFOUNDATION_CFNUMBER_PRIVATE__)
#define __COREFOUNDATION_CFNUMBER_PRIVATE__ 1

#include <CoreFoundation/CFNumber.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct {
    int64_t high;
    uint64_t low;
} CFSInt128Struct;

enum {
    kCFNumberSInt128Type = 17
};

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFNUMBER_PRIVATE__ */
