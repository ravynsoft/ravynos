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

/*	CFAvailability.h
	Copyright (c) 2013-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFAVAILABILITY__)
#define __COREFOUNDATION_CFAVAILABILITY__ 1

#include <TargetConditionals.h>
#include <Availability.h>

#if (TARGET_OS_MAC || TARGET_OS_EMBEDDED || TARGET_OS_IPHONE || TARGET_OS_WIN32)
// Even if unused, these must remain here for compatibility, because projects rely on them being included.
#include <AvailabilityMacros.h>
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif
#ifndef __has_extension
#define __has_extension(x) 0
#endif

// The arguments to these availability macros is a version number, e.g. 10_6, 3_0 or 'NA'
// To use a deprecation message with the macro, add a string as the last argument.
#if __has_feature(attribute_availability_with_message)

#define __NSi_2_0 introduced=2.0
#define __NSi_2_1 introduced=2.1
#define __NSi_2_2 introduced=2.2
#define __NSi_3_0 introduced=3.0
#define __NSi_3_1 introduced=3.1
#define __NSi_3_2 introduced=3.2
#define __NSi_4_0 introduced=4.0
#define __NSi_4_1 introduced=4.1
#define __NSi_4_2 introduced=4.2
#define __NSi_4_3 introduced=4.3
#define __NSi_5_0 introduced=5.0
#define __NSi_5_1 introduced=5.1
#define __NSi_6_0 introduced=6.0
#define __NSi_6_1 introduced=6.1
#define __NSi_7_0 introduced=7.0
#define __NSi_7_1 introduced=7.1
#define __NSi_8_0 introduced=8.0
#define __NSi_8_1 introduced=8.1
#define __NSi_9_0 introduced=9.0
#define __NSi_9_1 introduced=9.1
#define __NSi_10_0 introduced=10.0
#define __NSi_10_1 introduced=10.1
#define __NSi_10_2 introduced=10.2
#define __NSi_10_3 introduced=10.3
#define __NSi_10_4 introduced=10.4
#define __NSi_10_5 introduced=10.5
#define __NSi_10_6 introduced=10.6
#define __NSi_10_7 introduced=10.7
#define __NSi_10_8 introduced=10.8
#define __NSi_10_9 introduced=10.9
#define __NSi_10_10 introduced=10.10
#define __NSi_10_10_2 introduced=10.10.2
#define __NSi_10_10_3 introduced=10.10.3

#define __NSd_2_0 ,deprecated=2.0
#define __NSd_2_1 ,deprecated=2.1
#define __NSd_2_2 ,deprecated=2.2
#define __NSd_3_0 ,deprecated=3.0
#define __NSd_3_1 ,deprecated=3.1
#define __NSd_3_2 ,deprecated=3.2
#define __NSd_4_0 ,deprecated=4.0
#define __NSd_4_1 ,deprecated=4.1
#define __NSd_4_2 ,deprecated=4.2
#define __NSd_4_3 ,deprecated=4.3
#define __NSd_5_0 ,deprecated=5.0
#define __NSd_5_1 ,deprecated=5.1
#define __NSd_6_0 ,deprecated=6.0
#define __NSd_6_1 ,deprecated=6.1
#define __NSd_7_0 ,deprecated=7.0
#define __NSd_7_1 ,deprecated=7.1
#define __NSd_8_0 ,deprecated=8.0
#define __NSd_8_1 ,deprecated=8.1
#define __NSd_9_0 ,deprecated=9.0
#define __NSd_9_1 ,deprecated=9.1
#define __NSd_10_0 ,deprecated=10.0
#define __NSd_10_1 ,deprecated=10.1
#define __NSd_10_2 ,deprecated=10.2
#define __NSd_10_3 ,deprecated=10.3
#define __NSd_10_4 ,deprecated=10.4
#define __NSd_10_5 ,deprecated=10.5
#define __NSd_10_6 ,deprecated=10.6
#define __NSd_10_7 ,deprecated=10.7
#define __NSd_10_8 ,deprecated=10.8
#define __NSd_10_9 ,deprecated=10.9
#define __NSd_10_10 ,deprecated=10.10
#define __NSd_10_10_2 ,deprecated=10.10.2
#define __NSd_10_10_3 ,deprecated=10.10.3

#define __NSi_NA unavailable
#define __NSd_NA

// Do not use TBD as an argument to NS_AVAILABLE
#define __NSi_TBD introduced=9876.5

#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE))
// This section is for compilers targeting OS X which support attribute_availability_with_message

#define CF_AVAILABLE(_mac, _ios) __attribute__((availability(macosx,__NSi_##_mac)))
#define CF_AVAILABLE_MAC(_mac) __attribute__((availability(macosx,__NSi_##_mac)))
#define CF_AVAILABLE_IOS(_ios) __attribute__((availability(macosx,unavailable)))
#define CF_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, ...) __attribute__((availability(macosx,__NSi_##_macIntro __NSd_##_macDep,message="" __VA_ARGS__)))
#define CF_DEPRECATED_MAC(_macIntro, _macDep, ...) __attribute__((availability(macosx,__NSi_##_macIntro __NSd_##_macDep,message="" __VA_ARGS__)))
#define CF_DEPRECATED_IOS(_iosIntro, _iosDep, ...) __attribute__((availability(macosx,unavailable)))

#elif (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
// This section is for compilers targeting iOS which support attribute_availability_with_message

#define CF_AVAILABLE(_mac, _ios) __attribute__((availability(ios,__NSi_##_ios)))
#define CF_AVAILABLE_MAC(_mac) __attribute__((availability(ios,unavailable)))
#define CF_AVAILABLE_IOS(_ios) __attribute__((availability(ios,__NSi_##_ios)))
#define CF_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, ...) __attribute__((availability(ios,__NSi_##_iosIntro __NSd_##_iosDep,message="" __VA_ARGS__)))
#define CF_DEPRECATED_MAC(_macIntro, _macDep, ...) __attribute__((availability(ios,unavailable)))
#define CF_DEPRECATED_IOS(_iosIntro, _iosDep, ...) __attribute__((availability(ios,__NSi_##_iosIntro __NSd_##_iosDep,message="" __VA_ARGS__)))

#endif

#elif (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
// This section is for OS X or iOS, and compilers without support for attribute_availability_with_message. We fall back to Availability.h.

#ifndef __AVAILABILITY_INTERNAL__MAC_10_0_DEP__MAC_10_0
#define __AVAILABILITY_INTERNAL__MAC_10_0_DEP__MAC_10_0 __AVAILABILITY_INTERNAL_DEPRECATED
#endif

#define CF_AVAILABLE(_mac, _ios) __OSX_AVAILABLE_STARTING(__MAC_##_mac, __IPHONE_##_ios)
#define CF_AVAILABLE_MAC(_mac) __OSX_AVAILABLE_STARTING(__MAC_##_mac, __IPHONE_NA)
#define CF_AVAILABLE_IOS(_ios) __OSX_AVAILABLE_STARTING(__MAC_NA, __IPHONE_##_ios)
#define CF_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, ...) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_##_macIntro, __MAC_##_macDep, __IPHONE_##_iosIntro, __IPHONE_##_iosDep)
#define CF_DEPRECATED_MAC(_macIntro, _macDep, ...) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_##_macIntro, __MAC_##_macDep, __IPHONE_NA, __IPHONE_NA)
#define CF_DEPRECATED_IOS(_iosIntro, _iosDep, ...) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_NA, __MAC_NA, __IPHONE_##_iosIntro, __IPHONE_##_iosDep)

#endif // __has_feature(attribute_availability_with_message)

#ifndef CF_AVAILABLE
// This section is for platforms which do not support availability
#define CF_AVAILABLE(_mac, _ios)
#define CF_AVAILABLE_MAC(_mac)
#define CF_AVAILABLE_IOS(_ios)
#define CF_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, ...)
#define CF_DEPRECATED_MAC(_macIntro, _macDep, ...)
#define CF_DEPRECATED_IOS(_iosIntro, _iosDep, ...)
#endif

// Older versions of these macros; use iOS versions instead
#define CF_AVAILABLE_IPHONE(_ios) CF_AVAILABLE_IOS(_ios)
#define CF_DEPRECATED_IPHONE(_iosIntro, _iosDep) CF_DEPRECATED_IOS(_iosIntro, _iosDep)

// Enum availability macros
#if __has_feature(enumerator_attributes) && __has_attribute(availability)
#define CF_ENUM_AVAILABLE(_mac, _ios) CF_AVAILABLE(_mac, _ios)
#define CF_ENUM_AVAILABLE_MAC(_mac) CF_AVAILABLE_MAC(_mac)
#define CF_ENUM_AVAILABLE_IOS(_ios) CF_AVAILABLE_IOS(_ios)
#define CF_ENUM_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, ...) CF_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, __VA_ARGS__)
#define CF_ENUM_DEPRECATED_MAC(_macIntro, _macDep, ...) CF_DEPRECATED_MAC(_macIntro, _macDep, __VA_ARGS__)
#define CF_ENUM_DEPRECATED_IOS(_iosIntro, _iosDep, ...) CF_DEPRECATED_IOS(_iosIntro, _iosDep, __VA_ARGS__)
#else
#define CF_ENUM_AVAILABLE(_mac, _ios)
#define CF_ENUM_AVAILABLE_MAC(_mac)
#define CF_ENUM_AVAILABLE_IOS(_ios)
#define CF_ENUM_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, ...)
#define CF_ENUM_DEPRECATED_MAC(_macIntro, _macDep, ...)
#define CF_ENUM_DEPRECATED_IOS(_iosIntro, _iosDep, ...)
#endif

// Enums and Options
#if (__cplusplus && __cplusplus >= 201103L && (__has_extension(cxx_strong_enums) || __has_feature(objc_fixed_enum))) || (!__cplusplus && __has_feature(objc_fixed_enum))
#define CF_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#if (__cplusplus)
#define CF_OPTIONS(_type, _name) _type _name; enum : _type
#else
#define CF_OPTIONS(_type, _name) enum _name : _type _name; enum _name : _type
#endif
#else
#define CF_ENUM(_type, _name) _type _name; enum
#define CF_OPTIONS(_type, _name) _type _name; enum
#endif

// Extension availability macros
#define CF_EXTENSION_UNAVAILABLE(_msg)      __OS_EXTENSION_UNAVAILABLE(_msg)
#define CF_EXTENSION_UNAVAILABLE_MAC(_msg)  __OSX_EXTENSION_UNAVAILABLE(_msg)
#define CF_EXTENSION_UNAVAILABLE_IOS(_msg)  __IOS_EXTENSION_UNAVAILABLE(_msg)

#endif // __COREFOUNDATION_CFAVAILABILITY__
