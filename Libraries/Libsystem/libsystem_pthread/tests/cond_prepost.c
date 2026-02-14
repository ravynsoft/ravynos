#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <TargetConditionals.h>

#include <pthread/pthread_spis.h>

#include <sys/sysctl.h>

#include "darwintest_defaults.h"
#include <darwintest_multiprocess.h>

// <rdar://problem/38810583> this test case is intended to test for the
// specific issue found in this radar. That is, if:
//
//     1. A mutex is in first-fit policy mode, and
//     2. is used as the mutex in a pthread_cond_wait (or timedwait), and
//     3. the mutex has the K-bit set but has no kernel waiters, and
//     4. the cvwait call preposts an unlock to the mutex
//
//  Under these conditions, the fact that the cvwait preposted an unlock to
//  the paired mutex is lost during the call. The P-bit was never returned to
//  userspace and the kwq in the kernel would continue to exist. If the same
//  uaddr is then reused as another synchroniser type then we would often
//  return EINVAL from the wait/lock function.
//
//  So this test is attempting to:
//
//     1. Repeatedly bang on a mutex+cvar for a number of iterations in the
//        hope of triggering a cvwait prepost situation.
//     2. Then destroy both the mutex and cvar, and reinitialise each memory
//        location as the opposite type of synchroniser. Then cvwait once to
//        trigger the failure condition.

struct context {
	union {
		pthread_mutex_t mutex;
		pthread_cond_t cond;
	};
	union {
		pthread_mutex_t mutex2;
		pthread_cond_t cond2;
	};
	long value;
	long count;
	long waiter;
};

static void *test_cond(void *ptr) {
	struct context *context = ptr;
	int res;

	res = pthread_cond_wait(&context->cond, &context->mutex2);
	T_ASSERT_POSIX_ZERO(res, "condition wait on condvar completed");
	res = pthread_mutex_unlock(&context->mutex2);
	T_ASSERT_POSIX_ZERO(res, "unlock condvar mutex");
	return NULL;
}

static void *test_cond_wake(void *ptr) {
	struct context *context = ptr;
	int res;

	res = pthread_mutex_lock(&context->mutex2);
	T_ASSERT_POSIX_ZERO(res, "locked condvar mutex");
	res = pthread_cond_signal(&context->cond);
	T_ASSERT_POSIX_ZERO(res, "condvar signalled");
	res = pthread_mutex_unlock(&context->mutex2);
	T_ASSERT_POSIX_ZERO(res, "dropped condvar mutex");

	return NULL;
}

static void *test_thread(void *ptr) {
	int res;
	long old;
	struct context *context = ptr;

	int i = 0;
	char *str;

	do {
		bool try = i++ & 1;
		bool cond = i & 16;

		if (!try){
			str = "pthread_mutex_lock";
			res = pthread_mutex_lock(&context->mutex);
		} else {
			str = "pthread_mutex_trylock";
			res = pthread_mutex_trylock(&context->mutex);
		}
		if (res != 0) {
			if (try && res == EBUSY) {
				continue;
			}
			T_ASSERT_POSIX_ZERO(res, "[%ld] %s", context->count, str);
		}

		old = __sync_fetch_and_or(&context->value, 1);
		if ((old & 1) != 0) {
			T_FAIL("[%ld] OR %lx\n", context->count, old);
		}

		old = __sync_fetch_and_and(&context->value, 0);
		if ((old & 1) == 0) {
			T_FAIL("[%ld] AND %lx\n", context->count, old);
		}

		if (cond && !context->waiter) {
			context->waiter = 1;
			struct timespec ts = {
				.tv_sec = 0,
				.tv_nsec = 10ull * NSEC_PER_MSEC,
			};

			res = pthread_cond_timedwait_relative_np(&context->cond2, &context->mutex, &ts);
			if (res == ETIMEDOUT) {
				// ignore, should be the last thread out
			} else if (res) {
				T_ASSERT_POSIX_ZERO(res, "[%ld] pthread_cond_wait",
						context->count);
			}
			context->waiter = 0;
			res = pthread_mutex_unlock(&context->mutex);
			if (res) {
				T_ASSERT_POSIX_ZERO(res, "[%ld] pthread_mutex_unlock",
						context->count);
			}
		} else {
			if (context->waiter) {
				res = pthread_cond_broadcast(&context->cond2);
				if (res) {
					T_ASSERT_POSIX_ZERO(res, "[%ld] pthread_cond_broadcast",
							context->count);
				}
			}
			res = pthread_mutex_unlock(&context->mutex);
			if (res) {
				T_ASSERT_POSIX_ZERO(res, "[%ld] pthread_mutex_unlock",
						context->count);
			}
		}
	} while (__sync_fetch_and_sub(&context->count, 1) > 0);
	return NULL;
}


static void
_test_condvar_prepost_race(void)
{
	struct context context = {
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.cond2 = PTHREAD_COND_INITIALIZER,
		.value = 0,
		.count = 10000,
		.waiter = false,
	};
	int i;
	int res;
	int threads = 8;
	pthread_t p[threads];
	for (i = 0; i < threads; ++i) {
		res = pthread_create(&p[i], NULL, test_thread, &context);
		T_QUIET; T_ASSERT_POSIX_ZERO(res, "pthread_create()");
	}
	for (i = 0; i < threads; ++i) {
		res = pthread_join(p[i], NULL);
		T_QUIET; T_ASSERT_POSIX_ZERO(res, "pthread_join()");
	}

	T_PASS("initial pthread mutex storm completed");

	pthread_mutex_destroy(&context.mutex);
	pthread_cond_destroy(&context.cond2);

	pthread_mutex_init(&context.mutex2, NULL);
	pthread_cond_init(&context.cond, NULL);
	res = pthread_mutex_lock(&context.mutex2);
	T_ASSERT_POSIX_ZERO(res, "mutex lock for condition wait");
	res = pthread_create(&p[0], NULL, test_cond, &context);
	T_QUIET; T_ASSERT_POSIX_ZERO(res, "pthread_create()");
	res = pthread_create(&p[1], NULL, test_cond_wake, &context);
	T_QUIET; T_ASSERT_POSIX_ZERO(res, "pthread_create()");

	res = pthread_join(p[0], NULL);
	T_QUIET; T_ASSERT_POSIX_ZERO(res, "pthread_join()");
	res = pthread_join(p[1], NULL);
	T_QUIET; T_ASSERT_POSIX_ZERO(res, "pthread_join()");

	pthread_cond_destroy(&context.cond);
}

T_DECL(cond_prepost_fairshare, "cond_prepost_fairshare (fairshare)",
	T_META_ALL_VALID_ARCHS(YES),
	T_META_ENVVAR("PTHREAD_MUTEX_DEFAULT_POLICY=1"))
{
	int i;
	int count = 100;
	for (i=0; i < count; i++) {
		_test_condvar_prepost_race();
	}
}

T_DECL(cond_prepost_firstfit, "cond_prepost_firstfit (firstfit)",
	T_META_ALL_VALID_ARCHS(YES),
	T_META_ENVVAR("PTHREAD_MUTEX_DEFAULT_POLICY=3"))
{
	int i;
	int count = 100;
	for (i=0; i < count; i++) {
		_test_condvar_prepost_race();
	}
}
