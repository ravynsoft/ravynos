#include <time.h>
#include <sys/time.h>
#include <mach/mach_time.h>
#include <stdlib.h>
#include <unistd.h>

#include <darwintest.h>

static void burn_cpu(void){
	static char *dummy_text = "Four score and seven years ago our fathers brought forth on this continent a new nation, conceived in liberty, and dedicated to the";

	for (int i = 0; i < 100; i++){
		char key[64]; char txt[64];
		strncpy(txt, dummy_text, 64);
		for (int j = 0; i < 64; i++){
			key[j] = rand() % 1;
		}
		setkey(key);
		encrypt(txt, 0);
		encrypt(txt, 1);
	}
}

T_DECL(clock_gettime_realtime, "clock_gettime(CLOCK_REALTIME, tp)")
{
	struct timespec ts;
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_REALTIME, &ts), NULL);

	struct timeval tv;
	T_ASSERT_POSIX_ZERO(gettimeofday(&tv, NULL), NULL);

	T_EXPECT_LE((unsigned long)tv.tv_sec - (unsigned long)ts.tv_sec, (unsigned long)1, 
				"gettimeofday() should return same as clock_gettime(CLOCK_REALTIME)");
}

T_DECL(clock_gettime_monotonic, "clock_gettime(CLOCK_MONOTONIC, tp)")
{
	struct timespec ts1, ts2;
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC, &ts1), NULL);

	sleep(1);

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC, &ts2), NULL);

	uint64_t nsec1 = (uint64_t)ts1.tv_sec * NSEC_PER_SEC + (uint64_t)ts1.tv_nsec;
	uint64_t nsec2 = (uint64_t)ts2.tv_sec * NSEC_PER_SEC + (uint64_t)ts2.tv_nsec;
	uint64_t nsec_diff = (uint64_t)llabs((int64_t)nsec2 - (int64_t)nsec1);

	T_EXPECT_GE(nsec_diff, 100 * NSEC_PER_MSEC, "clock_gettime(CLOCK_MONOTONIC) should advance at least 100ms");
	T_EXPECT_LE(nsec_diff, 10 * NSEC_PER_SEC, "clock_gettime(CLOCK_MONOTONIC) should advance no more than 10s");
}
T_DECL(clock_gettime_monotonic_raw, "clock_gettime(CLOCK_MONOTONIC_RAW, tp)")
{
	struct timespec ts1, ts2;
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC_RAW, &ts1), NULL);

	sleep(1);

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC_RAW, &ts2), NULL);

	uint64_t nsec1 = (uint64_t)ts1.tv_sec * NSEC_PER_SEC + (uint64_t)ts1.tv_nsec;
	uint64_t nsec2 = (uint64_t)ts2.tv_sec * NSEC_PER_SEC + (uint64_t)ts2.tv_nsec;
	uint64_t nsec_diff = (uint64_t)llabs((int64_t)nsec2 - (int64_t)nsec1);

	T_EXPECT_GE(nsec_diff, 100 * NSEC_PER_MSEC, "clock_gettime(CLOCK_MONOTONIC_RAW) should advance at least 100ms");
	T_EXPECT_LE(nsec_diff, 10 * NSEC_PER_SEC, "clock_gettime(CLOCK_MONOTONIC_RAW) should advance no more than 10s");
}

T_DECL(clock_gettime_uptime_raw, "clock_gettime(CLOCK_UPTIME_RAW, tp)")
{
	struct timespec ts1, ts2;
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_UPTIME_RAW, &ts1), NULL);

	sleep(1);

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_UPTIME_RAW, &ts2), NULL);

	uint64_t nsec1 = (uint64_t)ts1.tv_sec * NSEC_PER_SEC + (uint64_t)ts1.tv_nsec;
	uint64_t nsec2 = (uint64_t)ts2.tv_sec * NSEC_PER_SEC + (uint64_t)ts2.tv_nsec;
	uint64_t nsec_diff = (uint64_t)llabs((int64_t)nsec2 - (int64_t)nsec1);

	T_EXPECT_GE(nsec_diff, 100 * NSEC_PER_MSEC, "clock_gettime(CLOCK_UPTIME_RAW) should advance at least 100ms");
	T_EXPECT_LE(nsec_diff, 10 * NSEC_PER_SEC, "clock_gettime(CLOCK_UPTIME_RAW) should advance no more than 10s");
}

T_DECL(clock_gettime_cputime, "clock_gettime(CLOCK_*_CPUTIME_ID, tp)")
{
	struct timespec thread_ts1, thread_ts2;
	struct timespec process_ts1, process_ts2;
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &process_ts1), NULL);
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &thread_ts1), NULL);

	burn_cpu();

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &process_ts2), NULL);
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &thread_ts1), NULL);

	uint64_t nsec1, nsec2, nsec_diff;

	// CLOCK_PROCESS_CPUTIME_ID
	nsec1 = (uint64_t)process_ts1.tv_sec * NSEC_PER_SEC + (uint64_t)process_ts1.tv_nsec;
	nsec2 = (uint64_t)process_ts2.tv_sec * NSEC_PER_SEC + (uint64_t)process_ts2.tv_nsec;
	nsec_diff = (uint64_t)llabs((int64_t)nsec2 - (int64_t)nsec1);
	T_EXPECT_GE(nsec_diff, NSEC_PER_USEC, "clock_gettime(CLOCK_PROCESS_CPUTIME_ID) should advance at least 1us");
	T_EXPECT_LE(nsec_diff, 10 * NSEC_PER_SEC, "clock_gettime(CLOCK_PROCESS_CPUTIME_ID) should advance no more than 10s");

	// CLOCK_THREAD_CPUTIME_ID
	nsec1 = (uint64_t)thread_ts1.tv_sec * NSEC_PER_SEC + (uint64_t)thread_ts1.tv_nsec;
	nsec2 = (uint64_t)thread_ts2.tv_sec * NSEC_PER_SEC + (uint64_t)thread_ts2.tv_nsec;
	nsec_diff = (uint64_t)llabs((int64_t)nsec2 - (int64_t)nsec1);
	T_EXPECT_GE(nsec_diff, NSEC_PER_USEC, "clock_gettime(CLOCK_THREAD_CPUTIME_ID) should advance at least 1us");
	T_EXPECT_LE(nsec_diff, 10 * NSEC_PER_SEC, "clock_gettime(CLOCK_THREAD_CPUTIME_ID) should advance no more than 10s");
}

T_DECL(clock_gettime_monotonic_comparison, "compare CLOCK_MONOTONIC to CLOCK_MONOTONIC_RAW")
{
	struct timespec ts1, ts2;
	bool should_retry = true;

retry:
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC, &ts1), NULL);
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC_RAW, &ts2), NULL);

	uint64_t nsec1 = (uint64_t)ts1.tv_sec * NSEC_PER_SEC + (uint64_t)ts1.tv_nsec;
	uint64_t nsec2 = (uint64_t)ts2.tv_sec * NSEC_PER_SEC + (uint64_t)ts2.tv_nsec;
	uint64_t nsec_diff = (uint64_t)llabs((int64_t)nsec2 - (int64_t)nsec1);

	if (should_retry && nsec_diff > nsec2/20){
		should_retry = false;
		goto retry;
	}

	T_EXPECT_LE(nsec_diff, nsec2/20, "CLOCK_MONOTONIC and CLOCK_MONOTONIC_RAW should be within 5%%");
}

T_DECL(clock_gettime_nsec_np, "clock_gettime_nsec_np()")
{
	struct timespec ts;
	uint64_t nsec, ts_nsec, diff;

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_REALTIME, &ts), NULL);
	T_WITH_ERRNO; T_ASSERT_NE((nsec = clock_gettime_nsec_np(CLOCK_REALTIME)), (uint64_t)0, NULL);
	ts_nsec = (uint64_t)ts.tv_sec * NSEC_PER_SEC + (uint64_t)ts.tv_nsec;
	diff = (uint64_t)llabs((int64_t)nsec - (int64_t)ts_nsec);
	T_EXPECT_LE(diff, 100 * NSEC_PER_MSEC, "CLOCK_REALTIME: clock_gettime() and clock_gettime_nsec_np() should be within 100ms");

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC, &ts), NULL);
	T_WITH_ERRNO; T_ASSERT_NE((nsec = clock_gettime_nsec_np(CLOCK_MONOTONIC)), (uint64_t)0, NULL);
	ts_nsec = (uint64_t)ts.tv_sec * NSEC_PER_SEC + (uint64_t)ts.tv_nsec;
	diff = (uint64_t)llabs((int64_t)nsec - (int64_t)ts_nsec);
	T_EXPECT_LE(diff, 100 * NSEC_PER_MSEC, "CLOCK_MONOTONIC: clock_gettime() and clock_gettime_nsec_np() should be within 100ms");

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC_RAW, &ts), NULL);
	T_WITH_ERRNO; T_ASSERT_NE((nsec = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW)), (uint64_t)0, NULL);
	ts_nsec = (uint64_t)ts.tv_sec * NSEC_PER_SEC + (uint64_t)ts.tv_nsec;
	diff = (uint64_t)llabs((int64_t)nsec - (int64_t)ts_nsec);
	T_EXPECT_LE(diff, 100 * NSEC_PER_MSEC, "CLOCK_MONOTONIC_RAW: clock_gettime() and clock_gettime_nsec_np() should be within 100ms");

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_UPTIME_RAW, &ts), NULL);
	T_WITH_ERRNO; T_ASSERT_NE((nsec = clock_gettime_nsec_np(CLOCK_UPTIME_RAW)), (uint64_t)0, NULL);
	ts_nsec = (uint64_t)ts.tv_sec * NSEC_PER_SEC + (uint64_t)ts.tv_nsec;
	diff = (uint64_t)llabs((int64_t)nsec - (int64_t)ts_nsec);
	T_EXPECT_LE(diff, 100 * NSEC_PER_MSEC, "CLOCK_UPTIME_RAW: clock_gettime() and clock_gettime_nsec_np() should be within 100ms");

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts), NULL);
	T_WITH_ERRNO; T_ASSERT_NE((nsec = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID)), (uint64_t)0, NULL);
	ts_nsec = (uint64_t)ts.tv_sec * NSEC_PER_SEC + (uint64_t)ts.tv_nsec;
	diff = (uint64_t)llabs((int64_t)nsec - (int64_t)ts_nsec);
	T_EXPECT_LE(diff, 100 * NSEC_PER_MSEC, "CLOCK_PROCESS_CPUTIME_ID: clock_gettime() and clock_gettime_nsec_np() should be within 100ms");

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts), NULL);
	T_WITH_ERRNO; T_ASSERT_NE((nsec = clock_gettime_nsec_np(CLOCK_THREAD_CPUTIME_ID)), (uint64_t)0, NULL);
	ts_nsec = (uint64_t)ts.tv_sec * NSEC_PER_SEC + (uint64_t)ts.tv_nsec;
	diff = (uint64_t)llabs((int64_t)nsec - (int64_t)ts_nsec);
	T_EXPECT_LE(diff, 100 * NSEC_PER_MSEC, "CLOCK_THREAD_CPUTIME_ID: clock_gettime() and clock_gettime_nsec_np() should be within 100ms");
}

T_DECL(clock_getres, "clock_getres()")
{
	struct timespec ts;

	T_ASSERT_POSIX_ZERO(clock_getres(CLOCK_REALTIME, &ts), NULL);
	T_LOG("Resolution of CLOCK_REALTIME is %ld ns", ts.tv_nsec);
	T_EXPECT_EQ(ts.tv_sec, (long)0, NULL);
	T_EXPECT_GT(ts.tv_nsec, (long)0, NULL);

	T_ASSERT_POSIX_ZERO(clock_getres(CLOCK_MONOTONIC, &ts), NULL);
	T_LOG("Resolution of CLOCK_MONOTONIC is %ld ns", ts.tv_nsec);
	T_EXPECT_EQ(ts.tv_sec, (long)0, NULL);
	T_EXPECT_GT(ts.tv_nsec, (long)0, NULL);

	T_ASSERT_POSIX_ZERO(clock_getres(CLOCK_MONOTONIC_RAW, &ts), NULL);
	T_LOG("Resolution of CLOCK_MONOTONIC_RAW is %ld ns", ts.tv_nsec);
	T_EXPECT_EQ(ts.tv_sec, (long)0, NULL);
	T_EXPECT_GT(ts.tv_nsec, (long)0, NULL);

	T_ASSERT_POSIX_ZERO(clock_getres(CLOCK_UPTIME_RAW, &ts), NULL);
	T_LOG("Resolution of CLOCK_UPTIME_RAW is %ld ns", ts.tv_nsec);
	T_EXPECT_EQ(ts.tv_sec, (long)0, NULL);
	T_EXPECT_GT(ts.tv_nsec, (long)0, NULL);

	T_ASSERT_POSIX_ZERO(clock_getres(CLOCK_PROCESS_CPUTIME_ID, &ts), NULL);
	T_LOG("Resolution of CLOCK_MONOTONIC_RAW is %ld ns", ts.tv_nsec);
	T_EXPECT_EQ(ts.tv_sec, (long)0, NULL);
	T_EXPECT_GT(ts.tv_nsec, (long)0, NULL);

	T_ASSERT_POSIX_ZERO(clock_getres(CLOCK_THREAD_CPUTIME_ID, &ts), NULL);
	T_LOG("Resolution of CLOCK_MONOTONIC_RAW is %ld ns", ts.tv_nsec);
	T_EXPECT_EQ(ts.tv_sec, (long)0, NULL);
	T_EXPECT_GT(ts.tv_nsec, (long)0, NULL);
}

T_DECL(clock_settime_realtime, "clock_settime(CLOCK_REALTIME, tp)",
	   T_META("as_root", "true"))
{
	struct timespec ts;
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_REALTIME, &ts), NULL);

	time_t initial_time = ts.tv_sec;

	ts.tv_nsec += 1 * NSEC_PER_SEC;
	T_ASSERT_POSIX_ZERO(clock_settime(CLOCK_REALTIME, &ts), NULL);

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_REALTIME, &ts), NULL);
	T_EXPECT_GT(ts.tv_sec - initial_time, (time_t)0, "time should move forward at least one second");
	T_EXPECT_LE(ts.tv_sec - initial_time, (time_t)2, "time should move forward less than two seconds");
}
T_DECL(clock_settime_other, "clock_settime(CLOCK_*, tp)",
	   T_META("as_root", "true"))
{
	struct timespec ts;;
	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_MONOTONIC_RAW, &ts), NULL);
	T_EXPECT_EQ(clock_settime(CLOCK_MONOTONIC_RAW, &ts), -1, NULL);
	T_EXPECT_EQ(errno, EINVAL, NULL);

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts), NULL);
	T_EXPECT_EQ(clock_settime(CLOCK_PROCESS_CPUTIME_ID, &ts), -1, NULL);
	T_EXPECT_EQ(errno, EINVAL, NULL);

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_UPTIME_RAW, &ts), NULL);
	T_EXPECT_EQ(clock_settime(CLOCK_UPTIME_RAW, &ts), -1, NULL);
	T_EXPECT_EQ(errno, EINVAL, NULL);

	T_ASSERT_POSIX_ZERO(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts), NULL);
	T_EXPECT_EQ(clock_settime(CLOCK_THREAD_CPUTIME_ID, &ts), -1, NULL);
	T_EXPECT_EQ(errno, EINVAL, NULL);
}
