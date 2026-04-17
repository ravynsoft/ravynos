/*
 * Copyright (c) 2000, 2005 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef HFS_DBG_H_
#define HFS_DBG_H_

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdbool.h>

// So that the analyzer acknowledges assertions...
#if defined(__clang_analyzer__) || DEBUG
#define panic_on_assert true
#else
extern bool panic_on_assert;
#endif

#if DEBUG
extern bool hfs_corruption_panics;
#else
#define hfs_corruption_panics false
#endif

__attribute__((noreturn))
void hfs_assert_fail(const char *file, unsigned line, const char *expr);

#define hfs_assert(expr)										\
	do {														\
		if (__builtin_expect(panic_on_assert, false)			\
			&& __builtin_expect(!(expr), false)) {				\
			hfs_assert_fail(__FILE__, __LINE__, #expr);			\
		}														\
	} while (0)

// On production, will printf rather than assert
#define hfs_warn(format, ...)									\
	do {														\
		if (__builtin_expect(panic_on_assert, false)) {			\
			panic(format, ## __VA_ARGS__);						\
			__builtin_unreachable();							\
		} else													\
			printf(format, ## __VA_ARGS__);						\
	} while (0)

// Quiet on production
#define hfs_debug(format, ...)									\
	do {														\
		if (__builtin_expect(panic_on_assert, false))			\
			printf(format, ## __VA_ARGS__);						\
	} while (0)

// Panic on debug unless boot-arg tells us not to
#define hfs_corruption_debug(format, ...)						\
	do {														\
		if (__builtin_expect(hfs_corruption_panics, false)) {	\
			panic(format, ## __VA_ARGS__);						\
			__builtin_unreachable();							\
		}														\
		else													\
			printf(format, ## __VA_ARGS__);						\
	} while (0)

__END_DECLS

#endif // HFS_DBG_H_
