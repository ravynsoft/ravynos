#include <atf-c.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>

ATF_TC_WITHOUT_HEAD(atf__environment);
ATF_TC_BODY(atf__environment, tc)
{
	DIR *cwd = opendir(".");

	ATF_REQUIRE(cwd != NULL);

	int number_ents = 0;
	struct dirent *de;
	while ((de = readdir(cwd)) != NULL) {
		ATF_REQUIRE(strcmp(de->d_name, ".") == 0 ||
		    strcmp(de->d_name, "..") == 0);
		++number_ents;
	}

	ATF_REQUIRE(number_ents == 2);

	ATF_REQUIRE(getenv("LANG") == NULL);
	ATF_REQUIRE(getenv("LC_ALL") == NULL);
	ATF_REQUIRE(getenv("LC_COLLATE") == NULL);
	ATF_REQUIRE(getenv("LC_CTYPE") == NULL);
	ATF_REQUIRE(getenv("LC_MESSAGES") == NULL);
	ATF_REQUIRE(getenv("LC_MONETARY") == NULL);
	ATF_REQUIRE(getenv("LC_NUMERIC") == NULL);
	ATF_REQUIRE(getenv("LC_TIME") == NULL);

#ifdef _WIN32
	atf_tc_skip("not applicable under Windows");
#endif

	ATF_REQUIRE(getenv("TMPDIR") != NULL);
	ATF_REQUIRE(getenv("HOME") != NULL);
	ATF_REQUIRE(strcmp(getenv("HOME"), getenv("TMPDIR")) == 0);

	ATF_REQUIRE(getenv("TZ") != NULL);
	ATF_REQUIRE(strcmp(getenv("TZ"), "UTC") == 0);
}

ATF_TC(atf__timeout);
ATF_TC_HEAD(atf__timeout, tc)
{
	atf_tc_set_md_var(tc, "timeout", "3");
}
ATF_TC_BODY(atf__timeout, tc)
{
	atf_tc_expect_timeout("sleep should take longer than 3s");
	sleep(5);
}

ATF_TC_WITHOUT_HEAD(atf__checkfail);
ATF_TC_BODY(atf__checkfail, tc)
{
	atf_tc_expect_fail("this should fail");
	ATF_CHECK(4 == 5);
}

ATF_TC_WITHOUT_HEAD(atf__exit_code);
ATF_TC_BODY(atf__exit_code, tc)
{
	atf_tc_expect_exit(42, "should exit with code 42");
	sleep(1);
	exit(42);
}

ATF_TC_WITHOUT_HEAD(atf__signal_sighup);
ATF_TC_BODY(atf__signal_sighup, tc)
{
#ifdef _WIN32
	atf_tc_skip("not applicable under Windows");
#else
	atf_tc_expect_signal(SIGHUP, "should exit by SIGHUP");
	fprintf(stderr, "hello atf__signal_sighup test!\n");
	kill(getpid(), SIGHUP);
#endif
}

ATF_TC_WITHOUT_HEAD(atf__signal_sigsegv);
ATF_TC_BODY(atf__signal_sigsegv, tc)
{
#ifdef _WIN32
	atf_tc_skip("not applicable under Windows");
#else
	atf_tc_expect_signal(SIGSEGV, "should exit by SIGSEGV");
	kill(getpid(), SIGSEGV);
#endif
}

ATF_TC_WITHOUT_HEAD(atf__signal_sigint);
ATF_TC_BODY(atf__signal_sigint, tc)
{
#ifdef _WIN32
	atf_tc_skip("not applicable under Windows");
#else
	atf_tc_expect_signal(SIGINT, "should exit by SIGINT");
	kill(getpid(), SIGINT);
#endif
}

ATF_TC_WITHOUT_HEAD(atf__signal_sigkill);
ATF_TC_BODY(atf__signal_sigkill, tc)
{
#ifdef _WIN32
	atf_tc_skip("not applicable under Windows");
#else
	atf_tc_expect_signal(SIGKILL, "should exit by SIGKILL");
	kill(getpid(), SIGKILL);
#endif
}

ATF_TC_WITHOUT_HEAD(atf__signal_sigabrt);
ATF_TC_BODY(atf__signal_sigabrt, tc)
{
#ifdef _WIN32
	atf_tc_skip("not applicable under Windows");
#else
	atf_tc_expect_signal(SIGABRT, "should exit by SIGABRT");
	kill(getpid(), SIGABRT);
#endif
}

ATF_TC_WITHOUT_HEAD(atf__signal_sigterm);
ATF_TC_BODY(atf__signal_sigterm, tc)
{
#ifdef _WIN32
	atf_tc_skip("not applicable under Windows");
#else
	atf_tc_expect_signal(SIGTERM, "should exit by SIGTERM");
	kill(getpid(), SIGTERM);
#endif
}

ATF_TC_WITHOUT_HEAD(atf__signal_sigfpe);
ATF_TC_BODY(atf__signal_sigfpe, tc)
{
#ifdef _WIN32
	atf_tc_skip("not applicable under Windows");
#else
	atf_tc_expect_signal(SIGFPE, "should exit by SIGFPE");

	volatile int d = 0;
	exit(100 / d);
#endif
}

ATF_TC_WITHOUT_HEAD(atf__skip);
ATF_TC_BODY(atf__skip, tc)
{
	atf_tc_skip("this test should be skipped");
}

ATF_TC_WITHOUT_HEAD(atf__stderr);
ATF_TC_BODY(atf__stderr, tc)
{
	fprintf(stderr, "line 1\n");
	fprintf(stderr, "line 2\n");
	fprintf(stderr, "line 3\n");
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, atf__environment);
	ATF_TP_ADD_TC(tp, atf__timeout);
	ATF_TP_ADD_TC(tp, atf__checkfail);
	ATF_TP_ADD_TC(tp, atf__exit_code);
	ATF_TP_ADD_TC(tp, atf__signal_sighup);
	ATF_TP_ADD_TC(tp, atf__signal_sigsegv);
	ATF_TP_ADD_TC(tp, atf__signal_sigint);
	ATF_TP_ADD_TC(tp, atf__signal_sigkill);
	ATF_TP_ADD_TC(tp, atf__signal_sigabrt);
	ATF_TP_ADD_TC(tp, atf__signal_sigterm);
	ATF_TP_ADD_TC(tp, atf__signal_sigfpe);
	ATF_TP_ADD_TC(tp, atf__skip);
	ATF_TP_ADD_TC(tp, atf__stderr);

	return atf_no_error();
}
