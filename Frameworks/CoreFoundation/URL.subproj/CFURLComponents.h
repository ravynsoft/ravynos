/*	CFURLComponents.h
	Copyright (c) 2015-2019, Apple Inc. All rights reserved.
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#ifndef __COREFOUNDATION_CFURLCOMPONENTS__
#define __COREFOUNDATION_CFURLCOMPONENTS__

// This file is for the use of NSURLComponents only.

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>

// For swift-corelibs-foundation:
// When SCF is compiled under Darwin, these are IPI, not SPI.
// Do not associate availability with them.
#if DEPLOYMENT_RUNTIME_SWIFT
#define _CF_URL_COMPONENTS_API_AVAILABLE(...)
#else
#define _CF_URL_COMPONENTS_API_AVAILABLE(...) API_AVAILABLE(__VA_ARGS__)
#endif

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

typedef struct CF_BRIDGED_TYPE(id) __CFURLComponents *CFURLComponentsRef;

CF_EXPORT CFTypeID _CFURLComponentsGetTypeID(void) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

// URLComponents are always mutable.
CF_EXPORT _Nullable CFURLComponentsRef _CFURLComponentsCreate(CFAllocatorRef alloc) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFURLComponentsRef _CFURLComponentsCreateWithURL(CFAllocatorRef alloc, CFURLRef url, Boolean resolveAgainstBaseURL) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFURLComponentsRef _CFURLComponentsCreateWithString(CFAllocatorRef alloc, CFStringRef string) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFURLComponentsRef _CFURLComponentsCreateCopy(CFAllocatorRef alloc, CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFURLRef _CFURLComponentsCopyURL(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFURLRef _CFURLComponentsCopyURLRelativeToURL(CFURLComponentsRef components, _Nullable CFURLRef relativeToURL) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyString(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyScheme(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyUser(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyPassword(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyHost(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFNumberRef _CFURLComponentsCopyPort(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyPath(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyQuery(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyFragment(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

// Returns true if the scheme argument can be passed to _CFURLComponentsSetScheme. A valid scheme string is an ALPHA character followed by 0 or more ALPHA, DIGIT, "+", "-", or "." characters. Because NULL can be passed to _CFURLComponentsSetScheme to clear the scheme component, passing NULL to this function also returns true.
CF_EXPORT Boolean _CFURLComponentsSchemeIsValid(_Nullable CFStringRef scheme) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));

// These return false if the conversion fails
CF_EXPORT Boolean _CFURLComponentsSetScheme(CFURLComponentsRef components, _Nullable CFStringRef scheme) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetUser(CFURLComponentsRef components, _Nullable CFStringRef user) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetPassword(CFURLComponentsRef components, _Nullable CFStringRef password) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetHost(CFURLComponentsRef components, _Nullable CFStringRef host) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetPort(CFURLComponentsRef components, _Nullable CFNumberRef port) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetPath(CFURLComponentsRef components, _Nullable CFStringRef path) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetQuery(CFURLComponentsRef components, _Nullable CFStringRef query) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetFragment(CFURLComponentsRef components, _Nullable CFStringRef fragment) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyPercentEncodedUser(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyPercentEncodedPassword(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyPercentEncodedHost(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyPercentEncodedPath(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyPercentEncodedQuery(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT _Nullable CFStringRef _CFURLComponentsCopyPercentEncodedFragment(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

// These return false if the conversion fails
CF_EXPORT Boolean _CFURLComponentsSetPercentEncodedUser(CFURLComponentsRef components, _Nullable CFStringRef user) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetPercentEncodedPassword(CFURLComponentsRef components, _Nullable CFStringRef password) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetPercentEncodedHost(CFURLComponentsRef components, _Nullable CFStringRef host) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetPercentEncodedPath(CFURLComponentsRef components, _Nullable CFStringRef path) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetPercentEncodedQuery(CFURLComponentsRef components, _Nullable CFStringRef query) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFURLComponentsSetPercentEncodedFragment(CFURLComponentsRef components, _Nullable CFStringRef fragment) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT CFRange _CFURLComponentsGetRangeOfScheme(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFRange _CFURLComponentsGetRangeOfUser(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFRange _CFURLComponentsGetRangeOfPassword(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFRange _CFURLComponentsGetRangeOfHost(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFRange _CFURLComponentsGetRangeOfPort(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFRange _CFURLComponentsGetRangeOfPath(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFRange _CFURLComponentsGetRangeOfQuery(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFRange _CFURLComponentsGetRangeOfFragment(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT CFStringRef _CFStringCreateByAddingPercentEncodingWithAllowedCharacters(CFAllocatorRef alloc, CFStringRef string, CFCharacterSetRef allowedCharacters) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFStringRef _Nullable _CFStringCreateByRemovingPercentEncoding(CFAllocatorRef alloc, CFStringRef string) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

// These return singletons
CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLUserAllowedCharacterSet(void) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLPasswordAllowedCharacterSet(void) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLHostAllowedCharacterSet(void) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLPathAllowedCharacterSet(void) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLQueryAllowedCharacterSet(void) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLFragmentAllowedCharacterSet(void) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

// keys for dictionaries returned by _CFURLComponentsCopyQueryItems
CF_EXPORT const CFStringRef _kCFURLComponentsNameKey _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));
CF_EXPORT const CFStringRef _kCFURLComponentsValueKey _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));

CF_EXPORT _Nullable CFArrayRef _CFURLComponentsCopyQueryItems(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT void _CFURLComponentsSetQueryItems(CFURLComponentsRef components, CFArrayRef names, CFArrayRef values) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _Nullable CFArrayRef _CFURLComponentsCopyPercentEncodedQueryItems(CFURLComponentsRef components) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));
CF_EXPORT Boolean _CFURLComponentsSetPercentEncodedQueryItems(CFURLComponentsRef components, CFArrayRef names, CFArrayRef values) _CF_URL_COMPONENTS_API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));

CF_ASSUME_NONNULL_END
CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif // __COREFOUNDATION_CFURLCOMPONENTS__
