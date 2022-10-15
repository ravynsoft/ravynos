/*	CFAvailability.h
	Copyright (c) 2013-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFAVAILABILITY__)
#define __COREFOUNDATION_CFAVAILABILITY__ 1

#if __has_include(<CoreFoundation/TargetConditionals.h>)
#include <CoreFoundation/TargetConditionals.h>
#else
#include <TargetConditionals.h>
#endif

#if __has_include(<Availability.h>) && __has_include(<os/availability.h>) && __has_include(<AvailabilityMacros.h>)
#include <Availability.h>
#include <os/availability.h>
// Even if unused, these must remain here for compatibility, because projects rely on them being included.
#include <AvailabilityMacros.h>
#else
#define API_AVAILABLE(...)
#define API_DEPRECATED(...)
#define API_UNAVAILABLE(...)
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
#if __has_feature(attribute_availability_with_version_underscores) || (__has_feature(attribute_availability_with_message) && __clang__ && __clang_major__ >= 7)
#if TARGET_OS_OSX
// This section is for compilers targeting OS X which support attribute_availability_with_message

#define CF_AVAILABLE(_mac, _ios) __attribute__((availability(macosx,introduced=_mac)))
#define CF_AVAILABLE_MAC(_mac) __attribute__((availability(macosx,introduced=_mac)))
#define CF_AVAILABLE_IOS(_ios) __attribute__((availability(macosx,unavailable)))
#define CF_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, ...) __attribute__((availability(macosx,introduced=_macIntro,deprecated=_macDep,message="" __VA_ARGS__)))
#define CF_DEPRECATED_MAC(_macIntro, _macDep, ...) __attribute__((availability(macosx,introduced=_macIntro,deprecated=_macDep,message="" __VA_ARGS__)))
#define CF_DEPRECATED_IOS(_iosIntro, _iosDep, ...) __attribute__((availability(macosx,unavailable)))

#elif TARGET_OS_IPHONE
// This section is for compilers targeting iOS which support attribute_availability_with_message

#define CF_AVAILABLE(_mac, _ios) __attribute__((availability(ios,introduced=_ios)))
#define CF_AVAILABLE_MAC(_mac) __attribute__((availability(ios,unavailable)))
#define CF_AVAILABLE_IOS(_ios) __attribute__((availability(ios,introduced=_ios)))
#define CF_DEPRECATED(_macIntro, _macDep, _iosIntro, _iosDep, ...) __attribute__((availability(ios,introduced=_iosIntro,deprecated=_iosDep,message="" __VA_ARGS__)))
#define CF_DEPRECATED_MAC(_macIntro, _macDep, ...) __attribute__((availability(ios,unavailable)))
#define CF_DEPRECATED_IOS(_iosIntro, _iosDep, ...) __attribute__((availability(ios,introduced=_iosIntro,deprecated=_iosDep,message="" __VA_ARGS__)))

#endif

#elif TARGET_OS_OSX || TARGET_OS_IPHONE
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

// "Soft" deprecation.
#ifndef API_TO_BE_DEPRECATED
/// This macro is used as a version number in API that will be deprecated in an upcoming release. We call this API "soft deprecated". Soft deprecation is an intermediate step before formal deprecation, used as a way to give you a heads-up about the API before you start getting a compiler warning.
/// You can find all places in your code that use soft deprecated API by redefining the value of this macro to your current minimum deployment target, for example:
///   (macOS)
///    clang -DAPI_TO_BE_DEPRECATED=10.12 other compiler flags
///   (iOS)
///    clang -DAPI_TO_BE_DEPRECATED=11.0 other compiler flags
#define API_TO_BE_DEPRECATED 100000
#endif

// Enums and Options
#if __has_attribute(enum_extensibility)
#define __CF_ENUM_ATTRIBUTES __attribute__((enum_extensibility(open)))
#define __CF_CLOSED_ENUM_ATTRIBUTES __attribute__((enum_extensibility(closed)))
#define __CF_OPTIONS_ATTRIBUTES __attribute__((flag_enum,enum_extensibility(open)))
#else
#define __CF_ENUM_ATTRIBUTES
#define __CF_CLOSED_ENUM_ATTRIBUTES
#define __CF_OPTIONS_ATTRIBUTES
#endif

#define __CF_ENUM_GET_MACRO(_1, _2, NAME, ...) NAME
#define __CF_ENUM_FIXED_IS_AVAILABLE (__cplusplus && __cplusplus >= 201103L && (__has_extension(cxx_strong_enums) || __has_feature(objc_fixed_enum))) || (!__cplusplus && (__has_feature(objc_fixed_enum) || __has_extension(cxx_fixed_enum)))

#if __CF_ENUM_FIXED_IS_AVAILABLE
#define __CF_NAMED_ENUM(_type, _name)     int __CF_ENUM_ ## _name; enum __CF_ENUM_ATTRIBUTES _name : _type; typedef enum _name _name; enum _name : _type
#define __CF_ANON_ENUM(_type)             enum __CF_ENUM_ATTRIBUTES : _type
#define CF_CLOSED_ENUM(_type, _name)      int __CF_ENUM_ ## _name; enum __CF_CLOSED_ENUM_ATTRIBUTES _name : _type; typedef enum _name _name; enum _name : _type
#if (__cplusplus)
#define CF_OPTIONS(_type, _name) _type _name; enum __CF_OPTIONS_ATTRIBUTES : _type
#else
#define CF_OPTIONS(_type, _name) int __CF_OPTIONS_ ## _name; enum __CF_OPTIONS_ATTRIBUTES _name : _type; typedef enum _name _name; enum _name : _type
#endif
#else
#define __CF_NAMED_ENUM(_type, _name) _type _name; enum
#define __CF_ANON_ENUM(_type) enum
#define CF_CLOSED_ENUM(_type, _name) _type _name; enum
#define CF_OPTIONS(_type, _name) _type _name; enum
#endif

/* CF_ENUM supports the use of one or two arguments. The first argument is always the integer type used for the values of the enum. The second argument is an optional type name for the macro. When specifying a type name, you must precede the macro with 'typedef' like so:

typedef CF_ENUM(CFIndex, CFComparisonResult) {
    ...
};

If you do not specify a type name, do not use 'typdef', like so:

CF_ENUM(CFIndex) {
    ...
};
*/
#define CF_ENUM(...) __CF_ENUM_GET_MACRO(__VA_ARGS__, __CF_NAMED_ENUM, __CF_ANON_ENUM, )(__VA_ARGS__)

#if __has_attribute(swift_wrapper)
#define _CF_TYPED_ENUM __attribute__((swift_wrapper(enum)))
#else
#define _CF_TYPED_ENUM
#endif

#if __has_attribute(swift_wrapper)
#define _CF_TYPED_EXTENSIBLE_ENUM __attribute__((swift_wrapper(struct)))
#else
#define _CF_TYPED_EXTENSIBLE_ENUM
#endif

#if DEPLOYMENT_RUNTIME_SWIFT
#define CF_STRING_ENUM
#define CF_EXTENSIBLE_STRING_ENUM

#define CF_TYPED_ENUM
#define CF_TYPED_EXTENSIBLE_ENUM
#else
#define CF_STRING_ENUM _CF_TYPED_ENUM
#define CF_EXTENSIBLE_STRING_ENUM _CF_TYPED_EXTENSIBLE_ENUM

#define CF_TYPED_ENUM _CF_TYPED_ENUM
#define CF_TYPED_EXTENSIBLE_ENUM _CF_TYPED_EXTENSIBLE_ENUM
#endif

#define __CF_ERROR_ENUM_GET_MACRO(_1, _2, NAME, ...) NAME
#if ((__cplusplus && __cplusplus >= 201103L && (__has_extension(cxx_strong_enums) || __has_feature(objc_fixed_enum))) || (!__cplusplus && __has_feature(objc_fixed_enum))) && __has_attribute(ns_error_domain)
#define __CF_NAMED_ERROR_ENUM(_domain, _name)     enum __attribute__((ns_error_domain(_domain))) _name : CFIndex _name; enum _name : CFIndex
#define __CF_ANON_ERROR_ENUM(_domain)             enum __attribute__((ns_error_domain(_domain))) : CFIndex
#else
#define __CF_NAMED_ERROR_ENUM(_domain, _name) __CF_NAMED_ENUM(CFIndex, _name)
#define __CF_ANON_ERROR_ENUM(_domain) __CF_ANON_ENUM(CFIndex)
#endif

/* CF_ERROR_ENUM supports the use of one or two arguments. The first argument is always the domain specifier for the enum. The second argument is an optional name of the typedef for the macro. When specifying a name for of the typedef, you must precede the macro with 'typedef' like so:
 
 typedef CF_ERROR_ENUM(kCFSomeErrorDomain, SomeErrorCodes) {
 ...
 };
 
 If you do not specify a typedef name, do not use 'typedef', like so:
 
 CF_ERROR_ENUM(kCFSomeErrorDomain) {
 ...
 };
 */
#define CF_ERROR_ENUM(...) __CF_ERROR_ENUM_GET_MACRO(__VA_ARGS__, __CF_NAMED_ERROR_ENUM, __CF_ANON_ERROR_ENUM)(__VA_ARGS__)

#ifndef CF_SWIFT_BRIDGED_TYPEDEF
#if __has_attribute(swift_bridged_typedef)
#define CF_SWIFT_BRIDGED_TYPEDEF __attribute__((swift_bridged_typedef))
#else
#define CF_SWIFT_BRIDGED_TYPEDEF
#endif
#endif

// Extension availability macros
#define CF_EXTENSION_UNAVAILABLE(_msg)      __OS_EXTENSION_UNAVAILABLE(_msg)
#define CF_EXTENSION_UNAVAILABLE_MAC(_msg)  __OSX_EXTENSION_UNAVAILABLE(_msg)
#define CF_EXTENSION_UNAVAILABLE_IOS(_msg)  __IOS_EXTENSION_UNAVAILABLE(_msg)

// Swift availability macro
#if __has_feature(attribute_availability_swift)
#define CF_SWIFT_UNAVAILABLE(_msg) __attribute__((availability(swift, unavailable, message=_msg)))
#else
#define CF_SWIFT_UNAVAILABLE(_msg)
#endif

#endif // __COREFOUNDATION_CFAVAILABILITY__
