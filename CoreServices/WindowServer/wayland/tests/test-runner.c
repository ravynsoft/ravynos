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
#include "../config.h"

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sys/ptrace.h>
#ifdef HAVE_SYS_PROCCTL_H
#include <sys/procctl.h>
#elif defined(HAVE_SYS_PRCTL_H)
#include <sys/prctl.h>
#ifndef PR_SET_PTRACER
# define PR_SET_PTRACER 0x59616d61
#endif
#endif

#include "test-runner.h"

/* when set to 1, check if tests are not leaking opened files.
 * It is turned on by default. It can be turned off by
 * WAYLAND_TEST_NO_LEAK_CHECK environment variable. */
int fd_leak_check_enabled;

/* when this var is set to 0, every call to test_set_timeout() is
 * suppressed - handy when debugging the test. Can be set by
 * WAYLAND_TEST_NO_TIMEOUTS environment variable. */
static int timeouts_enabled = 1;

/* set to one if the output goes to the terminal */
static int is_atty = 0;

extern const struct test __start_test_section, __stop_test_section;

static const struct test *
find_test(const char *name)
{
	const struct test *t;

	for (t = &__start_test_section; t < &__stop_test_section; t++)
		if (strcmp(t->name, name) == 0)
			return t;

	return NULL;
}

static void
usage(const char *name, int status)
{
	const struct test *t;

	fprintf(stderr, "Usage: %s [TEST]\n\n"
		"With no arguments, run all test.  Specify test case to run\n"
		"only that test without forking.  Available tests:\n\n",
		name);

	for (t = &__start_test_section; t < &__stop_test_section; t++)
		fprintf(stderr, "  %s\n", t->name);

	fprintf(stderr, "\n");

	exit(status);
}

void
test_set_timeout(unsigned int to)
{
	int re;

	if (!timeouts_enabled) {
		fprintf(stderr, "Timeouts suppressed.\n");
		return;
	}

	re = alarm(to);
	fprintf(stderr, "Timeout was %sset", re ? "re-" : "");

	if (to != 0)
		fprintf(stderr, " to %d second%s from now.\n",
			to, to > 1 ? "s" : "");
	else
		fprintf(stderr, " off.\n");
}

static void
sigalrm_handler(int signum)
{
	fprintf(stderr, "Test timed out.\n");
	abort();
}

void
check_fd_leaks(int supposed_fds)
{
	int num_fds;

	if (fd_leak_check_enabled) {
		num_fds = count_open_fds();
		if (supposed_fds != num_fds) {
			fprintf(stderr, "fd leak detected in test. "
				"Opened %d files, unclosed %d\n", num_fds,
				num_fds - supposed_fds);
			abort();
		}
	} else {
		fprintf(stderr, "FD leak checks disabled\n");
	}
}

static void
run_test(const struct test *t)
{
	int cur_fds;
	struct sigaction sa;

	if (timeouts_enabled) {
		sa.sa_handler = sigalrm_handler;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		assert(sigaction(SIGALRM, &sa, NULL) == 0);
	}

	//cur_alloc = get_current_alloc_num();
	cur_fds = count_open_fds();

	t->run();

	/* turn off timeout (if any) after test completion */
	if (timeouts_enabled)
		alarm(0);

	check_fd_leaks(cur_fds);

	exit(EXIT_SUCCESS);
}

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

static void
set_xdg_runtime_dir(void)
{
	char xdg_runtime_dir[PATH_MAX];
	const char *xrd_env;

	xrd_env = getenv("XDG_RUNTIME_DIR");
	/* if XDG_RUNTIME_DIR is not set in environ, fallback to /tmp */
	assert((snprintf(xdg_runtime_dir, PATH_MAX, "%s/wayland-tests-XXXXXX",
			 (xrd_env && xrd_env[0] == '/') ? xrd_env : "/tmp") < PATH_MAX)
		&& "test error: XDG_RUNTIME_DIR too long");

	assert(mkdtemp(xdg_runtime_dir) && "test error: mkdtemp failed");
	if (mkdir(xdg_runtime_dir, 0700) == -1)
		if (errno != EEXIST) {
			perror("Creating XDG_RUNTIME_DIR");
			abort();
		}

	if (setenv("XDG_RUNTIME_DIR", xdg_runtime_dir, 1) == -1) {
		perror("Setting XDG_RUNTIME_DIR");
		abort();
	}
}

static void
rmdir_xdg_runtime_dir(void)
{
	const char *xrd_env = getenv("XDG_RUNTIME_DIR");
	assert(xrd_env && xrd_env[0] == '/' && "No XDG_RUNTIME_DIR set");

	/* rmdir may fail if some test didn't do clean up */
	if (rmdir(xrd_env) == -1)
		perror("Cleaning XDG_RUNTIME_DIR");
}

#define RED	"\033[31m"
#define GREEN	"\033[32m"

static void
stderr_set_color(const char *color)
{
	/* use colors only when the output is connected to
	 * the terminal */
	if (is_atty)
		fprintf(stderr, "%s", color);
}

static void
stderr_reset_color(void)
{
	if (is_atty)
		fprintf(stderr, "\033[0m");
}

/* this function is taken from libinput/test/litest.c
 * (rev 028513a0a723e97941c39)
 *
 * Returns: 1 if a debugger is confirmed present; 0 if no debugger is
 * present or if it can't be determined.
 */
#if defined(HAVE_SYS_PROCCTL_H) && defined(PROC_TRACE_STATUS)
static int
is_debugger_attached(void)
{
	int rc;
	int status;
	rc = procctl(P_PID, getpid(), PROC_TRACE_STATUS, &status);
	if (rc == -1) {
		perror("procctl");
		return 0;
	}
	/* -1=tracing disabled, 0=no debugger attached, >0=pid of debugger. */
	return status > 0;
}
#elif defined(HAVE_SYS_PRCTL_H)
static int
is_debugger_attached(void)
{
	int status;
	int rc;
	pid_t pid;
	int pipefd[2];

	if (pipe(pipefd) == -1) {
		perror("pipe");
		return 0;
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		close(pipefd[0]);
		close(pipefd[1]);
		return 0;
	} else if (pid == 0) {
		char buf;
		pid_t ppid = getppid();

		/* Wait until parent is ready */
		close(pipefd[1]);  /* Close unused write end */
		read(pipefd[0], &buf, 1);
		close(pipefd[0]);
		if (buf == '-')
			_exit(1);
		if (ptrace(PTRACE_ATTACH, ppid, NULL, NULL) != 0)
			_exit(1);
		if (!waitpid(-1, NULL, 0))
			_exit(1);
		ptrace(PTRACE_CONT, NULL, NULL);
		ptrace(PTRACE_DETACH, ppid, NULL, NULL);
		_exit(0);
	} else {
		close(pipefd[0]);

		/* Enable child to ptrace the parent process */
		rc = prctl(PR_SET_PTRACER, pid);
		if (rc != 0 && errno != EINVAL) {
			/* An error prevents us from telling if a debugger is attached.
			 * Instead of propagating the error, assume no debugger present.
			 * But note the error to the log as a clue for troubleshooting.
			 * Then flag the error state to the client by sending '-'.
			 */
			perror("prctl");
			write(pipefd[1], "-", 1);
		} else {
			/* Signal to client that parent is ready by passing '+' */
			write(pipefd[1], "+", 1);
		}
		close(pipefd[1]);

		waitpid(pid, &status, 0);
		rc = WEXITSTATUS(status);
	}

	return rc;
}
#endif

int main(int argc, char *argv[])
{
	const struct test *t;
	pid_t pid;
	int total, pass;
	siginfo_t info;

	if (isatty(fileno(stderr)))
		is_atty = 1;

	if (is_debugger_attached()) {
		fd_leak_check_enabled = 0;
		timeouts_enabled = 0;
	} else {
		fd_leak_check_enabled = !getenv("WAYLAND_TEST_NO_LEAK_CHECK");
		timeouts_enabled = !getenv("WAYLAND_TEST_NO_TIMEOUTS");
	}

	if (argc == 2 && strcmp(argv[1], "--help") == 0)
		usage(argv[0], EXIT_SUCCESS);

	if (argc == 2) {
		t = find_test(argv[1]);
		if (t == NULL) {
			fprintf(stderr, "unknown test: \"%s\"\n", argv[1]);
			usage(argv[0], EXIT_FAILURE);
		}

		set_xdg_runtime_dir();
		/* run_test calls exit() */
		assert(atexit(rmdir_xdg_runtime_dir) == 0);

		run_test(t);
	}

	/* set our own XDG_RUNTIME_DIR */
	set_xdg_runtime_dir();

	pass = 0;
	for (t = &__start_test_section; t < &__stop_test_section; t++) {
		int success = 0;

		pid = fork();
		assert(pid >= 0);

		if (pid == 0)
			run_test(t); /* never returns */

		if (waitid(P_PID, pid, &info, WEXITED)) {
			stderr_set_color(RED);
			fprintf(stderr, "waitid failed: %s\n",
				strerror(errno));
			stderr_reset_color();

			abort();
		}

		switch (info.si_code) {
		case CLD_EXITED:
			if (info.si_status == EXIT_SUCCESS)
				success = !t->must_fail;
			else
				success = t->must_fail;

			stderr_set_color(success ? GREEN : RED);
			fprintf(stderr, "test \"%s\":\texit status %d",
				t->name, info.si_status);

			break;
		case CLD_KILLED:
		case CLD_DUMPED:
			if (t->must_fail)
				success = 1;

			stderr_set_color(success ? GREEN : RED);
			fprintf(stderr, "test \"%s\":\tsignal %d",
				t->name, info.si_status);

			break;
		}

		if (success) {
			pass++;
			fprintf(stderr, ", pass.\n");
		} else
			fprintf(stderr, ", fail.\n");

		stderr_reset_color();

		/* print separator line */
		fprintf(stderr, "----------------------------------------\n");
	}

	total = &__stop_test_section - &__start_test_section;
	fprintf(stderr, "%d tests, %d pass, %d fail\n",
		total, pass, total - pass);

	/* cleaning */
	rmdir_xdg_runtime_dir();

	return pass == total ? EXIT_SUCCESS : EXIT_FAILURE;
}
