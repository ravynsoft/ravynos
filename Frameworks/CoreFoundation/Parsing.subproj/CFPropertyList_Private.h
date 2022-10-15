/*    CFPropertyList.h
 Copyright (c) 2017, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2017, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#include <CoreFoundation/CFPropertyList.h>

// this is only allowed the first 8 bits
typedef CF_OPTIONS(CFOptionFlags, CFPropertyListSupportedFormats) {
    kCFPropertyListSupportedFormatOpenStepFormat = 1 << 8,
    kCFPropertyListSupportedFormatBinary_v1_0 = 1 << 9,
    kCFPropertyListSupportedFormatXML_v1_0 = 1 << 10,
};

#define kCFPropertyListMutabilityMask 0xFF  // first 8 bits
