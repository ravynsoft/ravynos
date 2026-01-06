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

struct context {
	union {
		pthread_mutex_t mutex;
		pthread_cond_t cond;
	};
	pthread_mutex_t mutex2;
	long value;
	long count;
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
	
		res = pthread_mutex_unlock(&context->mutex);
		if (res) {
			T_ASSERT_POSIX_ZERO(res, "[%ld] pthread_mutex_lock", context->count);
		}
	} while (__sync_fetch_and_sub(&context->count, 1) > 0);
	return NULL;
}


static void
_test_condvar_prepost_race(void)
{
	struct context context = {
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.mutex2 = PTHREAD_MUTEX_INITIALIZER,
		.value = 0,
		.count = 1000,
	};
	int i;
	int res;
	int threads = 8;
	pthread_t p[threads];
	for (i = 0; i < threads; ++i) {
		res = pthread_create(&p[i], NULL, test_thread, &context);
		T_ASSERT_POSIX_ZERO(res, "pthread_create()");
	}
	for (i = 0; i < threads; ++i) {
		res = pthread_join(p[i], NULL);
		T_ASSERT_POSIX_ZERO(res, "pthread_join()");
	}

	T_PASS("initial pthread mutex storm completed");

	pthread_mutex_destroy(&context.mutex);

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

T_DECL(mutex_prepost_fairshare, "pthread_mutex_prepost (fairshare)",
	T_META_ALL_VALID_ARCHS(YES),
	T_META_ENVVAR("PTHREAD_MUTEX_DEFAULT_POLICY=1"))
{
	int i;
	int count = 100;
	for (i=0; i < count; i++) {
		_test_condvar_prepost_race();
	}
}

T_DECL(mutex_prepost_firstfit, "pthread_mutex_prepost (firstfit)",
	T_META_ALL_VALID_ARCHS(YES),
	T_META_ENVVAR("PTHREAD_MUTEX_DEFAULT_POLICY=3"))
{
	int i;
	int count = 100;
	for (i=0; i < count; i++) {
		_test_condvar_prepost_race();
	}
}
