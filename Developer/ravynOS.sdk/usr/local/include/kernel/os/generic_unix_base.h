/*
 * This source file is part of the Swift.org open source project
 *
 * Copyright (c) 2015 Apple Inc. and the Swift project authors
 *
 * Licensed under Apache License v2.0 with Runtime Library Exception
 *
 * See https://swift.org/LICENSE.txt for license information
 * See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 *
 */

#ifndef __OS_GENERIC_UNIX_BASE__
#define __OS_GENERIC_UNIX_BASE__

#if __has_include(<sys/sysmacros.h>)
#include <sys/sysmacros.h>
#endif

#if defined(__FreeBSD__)
#include <libutil.h>
#include <fcntl.h>
#endif
#include <sys/param.h>

#if __has_include(<sys/cdefs.h>) || defined(__linux__)
#include <sys/cdefs.h>
#endif

#ifndef API_AVAILABLE
#define API_AVAILABLE(...)
#endif
#ifndef API_DEPRECATED
#define API_DEPRECATED(...)
#endif
#ifndef API_UNAVAILABLE
#define API_UNAVAILABLE(...)
#endif
#ifndef API_DEPRECATED_WITH_REPLACEMENT
#define API_DEPRECATED_WITH_REPLACEMENT(...)
#endif

#if __GNUC__
#define OS_EXPECT(x, v) __builtin_expect((x), (v))
#define OS_UNUSED __attribute__((__unused__))
#else
#define OS_EXPECT(x, v) (x)
#define OS_UNUSED
#endif

#ifndef os_likely
#define os_likely(x) OS_EXPECT(!!(x), 1)
#endif
#ifndef os_unlikely
#define os_unlikely(x) OS_EXPECT(!!(x), 0)
#endif

#if __has_feature(assume_nonnull)
#define OS_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define OS_ASSUME_NONNULL_END   _Pragma("clang assume_nonnull end")
#else
#define OS_ASSUME_NONNULL_BEGIN
#define OS_ASSUME_NONNULL_END
#endif

#if __has_builtin(__builtin_assume)
#define OS_COMPILER_CAN_ASSUME(expr) __builtin_assume(expr)
#else
#define OS_COMPILER_CAN_ASSUME(expr) ((void)(expr))
#endif

#if __has_feature(attribute_availability_swift)
// equivalent to __SWIFT_UNAVAILABLE from Availability.h
#define OS_SWIFT_UNAVAILABLE(_msg) \
		__attribute__((__availability__(swift, unavailable, message=_msg)))
#else
#define OS_SWIFT_UNAVAILABLE(_msg)
#endif

#if __has_attribute(swift_private)
# define OS_REFINED_FOR_SWIFT __attribute__((__swift_private__))
#else
# define OS_REFINED_FOR_SWIFT
#endif

#if __has_attribute(swift_name)
# define OS_SWIFT_NAME(_name) __attribute__((__swift_name__(#_name)))
#else
# define OS_SWIFT_NAME(_name)
#endif

#define __OS_STRINGIFY(s) #s
#define OS_STRINGIFY(s) __OS_STRINGIFY(s)
#define __OS_CONCAT(x, y) x ## y
#define OS_CONCAT(x, y) __OS_CONCAT(x, y)

#if __has_feature(objc_fixed_enum) || __has_extension(cxx_strong_enums)
#define OS_ENUM(_name, _type, ...) \
typedef enum : _type { __VA_ARGS__ } _name##_t
#else
#define OS_ENUM(_name, _type, ...) \
enum { __VA_ARGS__ }; typedef _type _name##_t
#endif

/*
 * Stub out misc linking and compilation attributes
 */

#ifdef OS_EXPORT
#undef OS_EXPORT
#endif
#define OS_EXPORT

#ifdef OS_WARN_RESULT_NEEDS_RELEASE
#undef OS_WARN_RESULT_NEEDS_RELEASE
#endif

#ifdef OS_WARN_RESULT
#undef OS_WARN_RESULT
#endif
#define OS_WARN_RESULT

#ifdef OS_NOTHROW
#undef OS_NOTHROW
#endif
#define OS_NOTHROW

#endif /* __OS_GENERIC_UNIX_BASE__ */
