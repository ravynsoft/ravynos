/*  CFPropertyList_Internal.h
    Copyright (c) 2020, Apple Inc. and the Swift project authors
 
    Portions Copyright (c) 2020, Apple Inc. and the Swift project authors
    Licensed under Apache License v2.0 with Runtime Library Exception
    See http://swift.org/LICENSE.txt for license information
    See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFPROPERTYLIST_INTERNAL__)
#define __COREFOUNDATION_CFPROPERTYLIST_INTERNAL 1

#include "CFBase.h"

/// Limit for the max recursion depth to avoid unbounded stack explosion when
/// parsing a crafted plist during validation of an object graph and during reading.
/// For macOS use maximum recusrion limit of `512` is used.
/// For iPhone, tvOS, and Watches use `128` due to the memory limitations.
///
/// rdar://61207578 ([Ward CFPropertyList audit, Low] unbounded recursion (binary and plain plists))
/// rdar://61529878 ([Ward CFPropertyList audit, Medium] Plist exponential growth DoS)
CF_INLINE size_t _CFPropertyListMaxRecursionDepth() {
    /// Depth that won't hit stack limits on any platform
#if !TARGET_OS_IOS && !TARGET_OS_ANDROID
    return 512;
#else 
    return 128;
#endif
}

/// Limit for the width of collection references when the format we're writing out doesn't natively support references like bplists do
///
/// rdar://61529878 ([Ward CFPropertyList audit, Medium] Plist exponential growth DoS)
CF_INLINE size_t _CFPropertyListMaxRecursionWidth() {
    // For now let's start with a resonable value that during testing allows many common cases but prevents very "wide" references to the same collections
    return _CFPropertyListMaxRecursionDepth() * 3;
}

#endif /*! __COREFOUNDATION_CFPROPERTYLIST_INTERNAL__ */
