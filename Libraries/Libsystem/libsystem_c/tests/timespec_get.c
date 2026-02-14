#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#include <darwintest.h>

T_DECL(timespec_get, "timespec_get")
{
	struct timespec ts;
	T_ASSERT_EQ(timespec_get(&ts, TIME_UTC), TIME_UTC, NULL);

	struct timeval tv;
	T_ASSERT_POSIX_ZERO(gettimeofday(&tv, NULL), NULL);

	T_EXPECT_LE((unsigned long)tv.tv_sec - (unsigned long)ts.tv_sec, (unsigned long)1,
				"gettimeofday() should return same as timespec_get(TIME_UTC)");
}

