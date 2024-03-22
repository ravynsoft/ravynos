/*
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _TEST_RUNNER_H_
#define _TEST_RUNNER_H_

#ifdef NDEBUG
#error "Tests must not be built with NDEBUG defined, they rely on assert()."
#endif

#include <unistd.h>

struct test {
	const char *name;
	void (*run)(void);
	int must_fail;
} __attribute__ ((aligned (16)));

#define TEST(name)							\
	static void name(void);						\
									\
	const struct test test##name					\
		 __attribute__ ((used, section ("test_section"))) = {	\
		#name, name, 0						\
	};								\
									\
	static void name(void)

#define FAIL_TEST(name)							\
	static void name(void);						\
									\
	const struct test test##name					\
		 __attribute__ ((used, section ("test_section"))) = {	\
		#name, name, 1						\
	};								\
									\
	static void name(void)

int
count_open_fds(void);

void
exec_fd_leak_check(int nr_expected_fds); /* never returns */

void
check_fd_leaks(int supposed_fds);

/*
 * set/reset the timeout in seconds. The timeout starts
 * at the point of invoking this function
 */
void
test_set_timeout(unsigned int);

/* test-runner uses alarm() and SIGALRM, so we can not
 * use usleep and sleep functions in tests (see 'man usleep'
 * or 'man sleep', respectively). Following functions are safe
 * to use in tests */
void
test_usleep(useconds_t);

void
test_sleep(unsigned int);

void
test_disable_coredumps(void);

#define DISABLE_LEAK_CHECKS				\
	do {						\
		extern int fd_leak_check_enabled;	\
		fd_leak_check_enabled = 0;		\
	} while (0);

#endif
