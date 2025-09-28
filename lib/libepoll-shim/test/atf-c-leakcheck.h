#ifndef ATF_C_LEAKCHECK_H_
#define ATF_C_LEAKCHECK_H_

#include <atf-c.h>

#ifndef _GNU_SOURCE
#error "Need to define _GNU_SOURCE"
#endif

#include <fcntl.h>
#include <unistd.h>

static int fd_leak_test_a;
static int fd_leak_test_b;
static int fd_leak_test_c;
static int fd_leak_test_d;

static void
init_fd_checking(void)
{
	/* We check for fd leaks after each test. Remember fd numbers for
	 * checking here. */
	int fds_1[2];
	ATF_REQUIRE(pipe2(fds_1, O_CLOEXEC) == 0);

	fd_leak_test_a = fds_1[0];
	fd_leak_test_b = fds_1[1];

	int fds_2[2];
	ATF_REQUIRE(pipe2(fds_2, O_CLOEXEC) == 0);
	fd_leak_test_c = fds_2[0];
	fd_leak_test_d = fds_2[1];

	ATF_REQUIRE(close(fds_1[0]) == 0);
	ATF_REQUIRE(close(fds_1[1]) == 0);
	ATF_REQUIRE(close(fds_2[0]) == 0);
	ATF_REQUIRE(close(fds_2[1]) == 0);
}

static void
check_for_fd_leaks(void)
{
	/* Test that all fds of previous tests
	 * have been closed successfully. */

	int fds_1[2];
	ATF_REQUIRE(pipe2(fds_1, O_CLOEXEC) == 0);

	int fds_2[2];
	ATF_REQUIRE(pipe2(fds_2, O_CLOEXEC) == 0);

	ATF_REQUIRE(fds_1[0] == fd_leak_test_a);
	ATF_REQUIRE(fds_1[1] == fd_leak_test_b);
	ATF_REQUIRE(fds_2[0] == fd_leak_test_c);
	ATF_REQUIRE(fds_2[1] == fd_leak_test_d);

	ATF_REQUIRE(close(fds_1[0]) == 0);
	ATF_REQUIRE(close(fds_1[1]) == 0);
	ATF_REQUIRE(close(fds_2[0]) == 0);
	ATF_REQUIRE(close(fds_2[1]) == 0);
}

#define ATF_TC_BODY_FD_LEAKCHECK(tc, tcptr)                     \
	static void fd_leakcheck_##tc##_body(                   \
	    atf_tc_t const *tcptr __attribute__((__unused__))); \
	ATF_TC_BODY(tc, tcptr)                                  \
	{                                                       \
		init_fd_checking();                             \
		fd_leakcheck_##tc##_body(tcptr);                \
		check_for_fd_leaks();                           \
	}                                                       \
	static void fd_leakcheck_##tc##_body(                   \
	    atf_tc_t const *tcptr __attribute__((__unused__)))

#endif
