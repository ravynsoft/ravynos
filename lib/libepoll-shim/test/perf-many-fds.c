#include <atf-c.h>

#include <sys/eventfd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NR_EVENTFDS (20000)

/*
 * Those eventfd operations must not take longer than 15s! This was previously
 * the case with the old, naive list data structure used to manage eventfds.
 */

ATF_TC(perf_many_fds__perf);
ATF_TC_HEAD(perf_many_fds__perf, tc)
{
	atf_tc_set_md_var(tc, "timeout", "15");
}
ATF_TC_BODY(perf_many_fds__perf, tc)
{
	int *eventfds = malloc(NR_EVENTFDS * sizeof(int));
	ATF_REQUIRE(eventfds);

	for (long i = 0; i < NR_EVENTFDS; ++i) {
		eventfds[i] = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
		if (eventfds[i] < 0) {
			atf_tc_skip("could not create eventfd: %d", errno);
		}
	}

	for (long i = 0; i < 2000000; ++i) {
		ATF_REQUIRE(eventfd_write(eventfds[0], 1) == 0);
		if (i % 10000 == 0) {
			fprintf(stderr, ".");
		}
	}
	free(eventfds);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, perf_many_fds__perf);

	return atf_no_error();
}
