#include <atf-c.h>

#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include <dirent.h>

ATF_TC_WITHOUT_HEAD(atf__environment);
ATF_TC_BODY(atf__environment, tc)
{
	DIR *cwd = opendir(".");

	ATF_REQUIRE(cwd != NULL);

	system("pwd");

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
	ATF_REQUIRE(strcmp(getenv("HOME"), getenv("TMPDIR")) == 0);
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

ATF_TC_WITHOUT_HEAD(atf__signal);
ATF_TC_BODY(atf__signal, tc)
{
	atf_tc_expect_signal(SIGHUP, "should exit by SIGHUP");
	kill(getpid(), SIGHUP);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, atf__environment);
	ATF_TP_ADD_TC(tp, atf__timeout);
	ATF_TP_ADD_TC(tp, atf__checkfail);
	ATF_TP_ADD_TC(tp, atf__exit_code);
	ATF_TP_ADD_TC(tp, atf__signal);

	return atf_no_error();
}
