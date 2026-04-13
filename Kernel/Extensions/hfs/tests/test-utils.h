/*
 * Copyright (c) 2014-2015 Apple Inc. All rights reserved.
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

#ifndef TEST_UTILS_H_
#define TEST_UTILS_H_

#include <mach-o/dyld.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/syslimits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <libgen.h>

__BEGIN_DECLS

#if HAVE_ASSERT_FAIL

void  __attribute__((format (printf, 3, 4), noreturn))
	assert_fail_(const char *file, int line, const char *assertion, ...);

#else // !HAVE_ASSERT_FAIL

static inline void  __attribute__((format (printf, 3, 4), noreturn))
assert_fail_(const char *file, int line, const char *assertion, ...)
{
	va_list args;
	va_start(args, assertion);
	char *msg;
	vasprintf(&msg, assertion, args);
	va_end(args);
	printf("\n%s:%u: error: %s\n", file, line, msg);
	exit(1);
}

#endif // !HAVE_ASSERT_FAIL

int test_cleanup(bool (^)(void));
void do_cleanups(void);

#define assert_fail(str, ...)						\
	assert_fail_(__FILE__, __LINE__, str, ## __VA_ARGS__)

#undef assert
#define assert(condition)											\
	do {															\
		if (!(condition))											\
			assert_fail_(__FILE__, __LINE__,						\
						 "assertion failed: %s", #condition);		\
	} while (0)

#define assert_with_errno_(condition, condition_str)					\
	do {																\
		if (!(condition))												\
			assert_fail_(__FILE__, __LINE__, "%s failed: %s",			\
						 condition_str, strerror(errno));				\
	} while (0)

#define assert_with_errno(condition)			\
	assert_with_errno_((condition), #condition)

#define assert_no_err(condition) \
	assert_with_errno_(!(condition), #condition)

#define assert_equal(lhs, rhs, fmt)										\
	do {																\
		typeof (lhs) lhs_ = (lhs);										\
		typeof (lhs) rhs_ = (rhs);										\
		if (lhs_ != rhs_)												\
			assert_fail(#lhs " (" fmt ") != " #rhs " (" fmt ")",		\
						lhs_, rhs_);									\
	} while (0)

#define assert_equal_int(lhs, rhs)	assert_equal(lhs, rhs, "%d")
#define assert_equal_ll(lhs, rhs)	assert_equal(lhs, rhs, "%lld");

#define check_io(fn, len) check_io_((fn), len, __FILE__, __LINE__, #fn)

static inline ssize_t check_io_(ssize_t res, ssize_t len, const char *file,
								int line, const char *fn_str)
{
	if (res < 0)
		assert_fail_(file, line, "%s failed: %s", fn_str, strerror(errno));
	else if (len != -1 && res != len)
		assert_fail_(file, line, "%s != %ld", fn_str, len);
	return res;
}

#define ignore_eintr(x, error_val)								\
	({															\
		typeof(x) eintr_ret_;									\
		do {													\
			eintr_ret_ = (x);									\
		} while (eintr_ret_ == (error_val) && errno == EINTR);	\
		eintr_ret_;												\
	})

#define lengthof(x)		(sizeof(x) / sizeof(*x))

__END_DECLS

#endif // TEST_UTILS_H_
