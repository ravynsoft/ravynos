/*
 * Copyright © 2012 Collabora, Ltd.
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

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

#include "test-runner.h"

#if defined(__FreeBSD__)
#include <sys/sysctl.h>

/*
 * On FreeBSD, get file descriptor information using sysctl() since that does
 * not depend on a mounted fdescfs (which provides /dev/fd/N for N > 2).
 */
int
count_open_fds(void)
{
	int error;
	int nfds;
	int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_NFDS, 0 };
	size_t len;

	len = sizeof(nfds);
	error = sysctl(mib, 4, &nfds, &len, NULL, 0);
	assert(error == 0 && "sysctl KERN_PROC_NFDS failed.");
	return nfds;
}
#else
int
count_open_fds(void)
{
	DIR *dir;
	struct dirent *ent;
	int count = 0;

	/*
	 * Using /dev/fd instead of /proc/self/fd should allow this code to
	 * work on non-Linux operating systems.
	 */
	dir = opendir("/dev/fd");
	assert(dir && "opening /dev/fd failed.");

	errno = 0;
	while ((ent = readdir(dir))) {
		const char *s = ent->d_name;
		if (s[0] == '.' && (s[1] == 0 || (s[1] == '.' && s[2] == 0)))
			continue;
		count++;
	}
	assert(errno == 0 && "reading /dev/fd failed.");

	closedir(dir);

	return count;
}
#endif

void
exec_fd_leak_check(int nr_expected_fds)
{
	const char *exe = "exec-fd-leak-checker";
	char number[16] = { 0 };
	const char *test_build_dir = getenv("TEST_BUILD_DIR");
	char exe_path[256] = { 0 };

	if (test_build_dir == NULL || test_build_dir[0] == 0) {
	        test_build_dir = ".";
	}

	snprintf(exe_path, sizeof exe_path - 1, "%s/%s", test_build_dir, exe);

	snprintf(number, sizeof number - 1, "%d", nr_expected_fds);
	execl(exe_path, exe, number, (char *)NULL);
	assert(0 && "execing fd leak checker failed");
}

#define USEC_TO_NSEC(n) (1000 * (n))

/* our implementation of usleep and sleep functions that are safe to use with
 * timeouts (timeouts are implemented using alarm(), so it is not safe use
 * usleep and sleep. See man pages of these functions)
 */
void
test_usleep(useconds_t usec)
{
	struct timespec ts = {
		.tv_sec = 0,
		.tv_nsec = USEC_TO_NSEC(usec)
	};

	assert(nanosleep(&ts, NULL) == 0);
}

/* we must write the whole function instead of
 * wrapping test_usleep, because useconds_t may not
 * be able to contain such a big number of microseconds */
void
test_sleep(unsigned int sec)
{
	struct timespec ts = {
		.tv_sec = sec,
		.tv_nsec = 0
	};

	assert(nanosleep(&ts, NULL) == 0);
}

/** Try to disable coredumps
 *
 * Useful for tests that crash on purpose, to avoid creating a core file
 * or launching an application crash handler service or cluttering coredumpctl.
 *
 * NOTE: Calling this may make the process undebuggable.
 */
void
test_disable_coredumps(void)
{
	struct rlimit r;

	if (getrlimit(RLIMIT_CORE, &r) == 0) {
		r.rlim_cur = 0;
		setrlimit(RLIMIT_CORE, &r);
	}

#if defined(HAVE_PRCTL) && defined(PR_SET_DUMPABLE)
	prctl(PR_SET_DUMPABLE, 0, 0, 0, 0);
#endif
}
