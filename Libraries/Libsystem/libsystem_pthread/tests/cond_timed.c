#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/time.h>
#include <libkern/OSAtomic.h>
#include <dispatch/dispatch.h>

#include "darwintest_defaults.h"

#define NUM_THREADS 8
#define RDAR_38144536 1

struct context {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	long udelay;
	long count;
};

static void *wait_thread(void *ptr) {
	int res;
	struct context *context = ptr;

	bool loop = true;
	while (loop) {
		struct timespec ts;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		tv.tv_sec += (tv.tv_usec + context->udelay) / (__typeof(tv.tv_sec)) USEC_PER_SEC;
		tv.tv_usec = (tv.tv_usec + context->udelay) % (__typeof(tv.tv_usec)) USEC_PER_SEC;
		TIMEVAL_TO_TIMESPEC(&tv, &ts);

		res = pthread_mutex_lock(&context->mutex);
		if (res) {
			fprintf(stderr, "[%ld] pthread_mutex_lock: %s\n", context->count, strerror(res));
			abort();
		}

		if (context->count > 0) {
			res = pthread_cond_timedwait(&context->cond, &context->mutex, &ts);
			if (res != ETIMEDOUT) {
				fprintf(stderr, "[%ld] pthread_cond_timedwait: %s\n", context->count, strerror(res));
				abort();
			}
			--context->count;
		} else {
			loop = false;
		}

		res = pthread_mutex_unlock(&context->mutex);
		if (res) {
			fprintf(stderr, "[%ld] pthread_mutex_unlock: %s\n", context->count, strerror(res));
			abort();
		}
	}

	return NULL;
}

T_DECL(cond_timedwait_timeout, "pthread_cond_timedwait() timeout")
{
	// This testcase launches 8 threads that all perform timed wait on the same
	// conditional variable that is not being signaled in a loop. Ater the total
	// of 8000 timeouts all threads finish and the testcase prints out the
	// expected time (5[ms]*8000[timeouts]/8[threads]=5s) vs elapsed time.
	struct context context = {
		.cond = PTHREAD_COND_INITIALIZER,
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.udelay = 5000,
		.count = 8000,
	};

	long uexpected = (context.udelay * context.count) / NUM_THREADS;
	T_LOG("waittime expected: %ld us", uexpected);
	struct timeval start, end;
	gettimeofday(&start, NULL);

	pthread_t p[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; ++i) {
		T_ASSERT_POSIX_ZERO(pthread_create(&p[i], NULL, wait_thread, &context),
							"pthread_create");
	}

	usleep((useconds_t) uexpected);
	bool loop = true;
	while (loop) {
		T_QUIET; T_ASSERT_POSIX_ZERO(pthread_mutex_lock(&context.mutex),
							"pthread_mutex_lock");
		if (context.count <= 0) {
			loop = false;
		}
		T_QUIET; T_ASSERT_POSIX_ZERO(pthread_mutex_unlock(&context.mutex),
							"pthread_mutex_unlock");
	}

	for (int i = 0; i < NUM_THREADS; ++i) {
		T_ASSERT_POSIX_ZERO(pthread_join(p[i], NULL), "pthread_join");
	}

	gettimeofday(&end, NULL);
	uint64_t uelapsed =
			((uint64_t) end.tv_sec * USEC_PER_SEC + (uint64_t) end.tv_usec) -
			((uint64_t) start.tv_sec * USEC_PER_SEC + (uint64_t) start.tv_usec);
	T_LOG("waittime actual:   %llu us", uelapsed);
}

struct prodcons_context {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	bool consumer_ready;
	bool workitem_available;
	bool padding[6];
};

static void *consumer_thread(void *ptr) {
	struct prodcons_context *context = ptr;

	// tell producer thread that we are ready
	T_ASSERT_POSIX_ZERO(pthread_mutex_lock(&context->mutex), "pthread_mutex_lock");

	context->consumer_ready = true;
	T_ASSERT_POSIX_ZERO(pthread_cond_signal(&context->cond), "pthread_cond_signal");

	// wait for a work item to become available
	do {
		// mutex will be dropped and allow producer thread to acquire
		T_ASSERT_POSIX_ZERO(pthread_cond_wait(&context->cond, &context->mutex), "pthread_cond_wait");

		// loop in case of spurious wakeups
	} while (context->workitem_available == false);

	// work item has been sent, so dequeue it and tell producer
	context->workitem_available = false;
	T_ASSERT_POSIX_ZERO(pthread_cond_signal(&context->cond), "pthread_cond_signal");

	// unlock mutex, we are done here
	T_ASSERT_POSIX_ZERO(pthread_mutex_unlock(&context->mutex), "pthread_mutex_unlock");

	T_PASS("Consumer thread exiting");

	return NULL;
}

#define TESTCASE_TIMEOUT (10) /* seconds */
typedef enum {
	eNullTimeout,
	eZeroTimeout,
	eBeforeEpochTimeout,
	eRecentPastTimeout
} TimeOutType;

static DT_TEST_RETURN cond_timedwait_timeouts_internal(TimeOutType timeout, bool relative);

T_DECL(cond_timedwait_nulltimeout, "pthread_cond_timedwait() with NULL timeout, ensure mutex is unlocked")
{
	cond_timedwait_timeouts_internal(eNullTimeout, false);
}

T_DECL(cond_timedwait_zerotimeout, "pthread_cond_timedwait() with zero timeout, ensure mutex is unlocked")
{
#if RDAR_38144536
	T_SKIP("skipped <rdar://38144536>");
#else // RDAR_38144536
	cond_timedwait_timeouts_internal(eZeroTimeout, false);
#endif // RDAR_38144536
}

T_DECL(cond_timedwait_beforeepochtimeout, "pthread_cond_timedwait() with timeout before the epoch, ensure mutex is unlocked")
{
#if RDAR_38144536
	T_SKIP("skipped <rdar://38144536>");
#else // RDAR_38144536
	cond_timedwait_timeouts_internal(eBeforeEpochTimeout, false);
#endif // RDAR_38144536
}

T_DECL(cond_timedwait_pasttimeout, "pthread_cond_timedwait() with timeout in the past, ensure mutex is unlocked")
{
#if RDAR_38144536
	T_SKIP("skipped <rdar://38144536>");
#else // RDAR_38144536
	cond_timedwait_timeouts_internal(eRecentPastTimeout, false);
#endif // RDAR_38144536
}

T_DECL(cond_timedwait_relative_nulltimeout, "pthread_cond_timedwait_relative_np() with relative NULL timeout, ensure mutex is unlocked")
{
	cond_timedwait_timeouts_internal(eNullTimeout, true);
}

T_DECL(cond_timedwait_relative_pasttimeout, "pthread_cond_timedwait_relative_np() with relative timeout in the past, ensure mutex is unlocked")
{
	cond_timedwait_timeouts_internal(eRecentPastTimeout, true);
}

static DT_TEST_RETURN cond_timedwait_timeouts_internal(TimeOutType timeout, bool relative)
{
	// This testcase mimics a producer-consumer model where the consumer checks
	// in and waits until work becomes available. The producer then waits until
	// the work has been consumed and the consumer quiesces. Since condition
	// variables may have spurious wakeups, the timeout should not matter,
	// but there have been functional issues where the mutex would not be unlocked
	// for a timeout in the past.
	struct prodcons_context context = {
		.cond = PTHREAD_COND_INITIALIZER,
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.consumer_ready = false,
		.workitem_available = false
	};

	struct timeval test_timeout;
	gettimeofday(&test_timeout, NULL);
	test_timeout.tv_sec += TESTCASE_TIMEOUT;

	T_ASSERT_POSIX_ZERO(pthread_mutex_lock(&context.mutex), "pthread_mutex_lock");

	pthread_t p;
	T_ASSERT_POSIX_ZERO(pthread_create(&p, NULL, consumer_thread, &context),
							"pthread_create");

	// Wait until consumer thread is able to acquire the mutex, check in, and block
	// in its own condition variable. We do not want to start generating work before
	// the consumer thread is available
	do {
		// mutex will be dropped and allow consumer thread to acquire
		T_ASSERT_POSIX_ZERO(pthread_cond_wait(&context.cond, &context.mutex), "pthread_cond_wait");

		// loop in case of spurious wakeups
	} while (context.consumer_ready == false);

	// consumer is ready and blocked in its own condition variable, and
	// producer has mutex acquired. Send a work item and wait for it
	// to be dequeued

	context.workitem_available = true;
	T_ASSERT_POSIX_ZERO(pthread_cond_signal(&context.cond), "pthread_cond_signal");

	do {
		struct timeval now;

		gettimeofday(&now, NULL);
		T_QUIET; T_ASSERT_TRUE(timercmp(&now, &test_timeout, <), "timeout reached waiting for consumer thread to consume");

		struct timespec ts;

		if (relative) {
			switch (timeout) {
				case eNullTimeout:
					break;
				case eRecentPastTimeout:
					ts.tv_sec = -1;
					ts.tv_nsec = 0;
					break;
				case eZeroTimeout:
				case eBeforeEpochTimeout:
					break;
			}
		} else {
			switch (timeout) {
				case eNullTimeout:
					break;
				case eZeroTimeout:
					ts.tv_sec = 0;
					ts.tv_nsec = 0;
					break;
				case eBeforeEpochTimeout:
					ts.tv_sec = -1;
					ts.tv_nsec = 0;
					break;
				case eRecentPastTimeout:
					ts.tv_sec = now.tv_sec - 1;
					ts.tv_nsec = now.tv_usec / 1000;
					break;
			}
		}

		int ret;
		if (relative) {
			ret = pthread_cond_timedwait_relative_np(&context.cond, &context.mutex, timeout == eNullTimeout ? NULL : &ts);
		} else {
			ret = pthread_cond_timedwait(&context.cond, &context.mutex, timeout == eNullTimeout ? NULL : &ts);
		}
		if (ret != 0 && ret != EINTR && ret != ETIMEDOUT) T_ASSERT_POSIX_ZERO(ret, "timedwait returned error");

		usleep(10*1000); // avoid spinning in a CPU-bound loop

		// loop in case of spurious wakeups
	} while (context.workitem_available == true);

	T_ASSERT_POSIX_ZERO(pthread_mutex_unlock(&context.mutex), "pthread_mutex_unlock");

	T_ASSERT_POSIX_ZERO(pthread_join(p, NULL), "pthread_join");

	T_PASS("Consumer completed work");
}
