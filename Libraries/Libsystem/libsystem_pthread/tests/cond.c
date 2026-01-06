#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <libkern/OSAtomic.h>

#include "darwintest_defaults.h"
#include <darwintest_utils.h>

struct context {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	pthread_cond_t ready_cond;
	long waiters;
	long count;
	bool ready;
	char _padding[7];
};


static void *wait_thread(void *ptr) {
	struct context *context = ptr;

	// tell producer thread that we are ready
	T_QUIET;
	T_ASSERT_POSIX_ZERO(pthread_mutex_lock(&context->mutex), "pthread_mutex_lock");
	context->ready = true;
	T_QUIET;
	T_ASSERT_POSIX_ZERO(pthread_cond_signal(&context->ready_cond), "pthread_cond_signal");

	bool loop = true;
	while (loop) {

		if (context->count > 0) {
			++context->waiters;
			T_QUIET;
			T_ASSERT_POSIX_ZERO(pthread_cond_wait(&context->cond, &context->mutex), "[%ld] pthread_rwlock_unlock", context->count);
			--context->waiters;
			--context->count;
		} else {
			loop = false;
		}

	}

	T_QUIET;
	T_ASSERT_POSIX_ZERO(pthread_mutex_unlock(&context->mutex), "[%ld] pthread_mutex_unlock", context->count);

	return NULL;
}

T_DECL(cond, "pthread_cond",
		T_META_ALL_VALID_ARCHS(YES), T_META_TIMEOUT(120), T_META_CHECK_LEAKS(NO))
{
	struct context context = {
		.cond = PTHREAD_COND_INITIALIZER,
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.ready_cond = PTHREAD_COND_INITIALIZER,
		.waiters = 0,
		.count = 50000 * dt_ncpu(),
		.ready = false,
	};
	int i;
	int res;
	int threads = 2;
	pthread_t p[threads];
	for (i = 0; i < threads; ++i) {
		T_QUIET;
		T_ASSERT_POSIX_ZERO(pthread_mutex_lock(&context.mutex), "pthread_mutex_lock");

		context.ready = false;

		T_QUIET;
		T_ASSERT_POSIX_ZERO(pthread_create(&p[i], NULL, wait_thread, &context), "pthread_create");

		do {
			// mutex will be dropped and allow consumer thread to acquire
			T_QUIET;
			T_ASSERT_POSIX_ZERO(pthread_cond_wait(&context.ready_cond, &context.mutex), "pthread_cond_wait");
		} while (context.ready == false);

		T_QUIET;
		T_ASSERT_POSIX_ZERO(pthread_mutex_unlock(&context.mutex), "pthread_mutex_lock");

		T_LOG("Thread %d ready.", i);
	}

	T_LOG("All threads ready.");

	long half = context.count / 2;

	bool loop = true;
	while (loop) {
		T_QUIET;
		T_ASSERT_POSIX_ZERO(pthread_mutex_lock(&context.mutex), "[%ld] pthread_mutex_lock", context.count);
		if (context.waiters) {
			char *str;
			if (context.count > half) {
				str = "pthread_cond_broadcast";
				res = pthread_cond_broadcast(&context.cond);
			} else {
				str = "pthread_cond_signal";
				res = pthread_cond_signal(&context.cond);
			}
			T_QUIET;
			T_ASSERT_POSIX_ZERO(res, "[%ld] %s", context.count, str);
		}
		if (context.count <= 0) {
			loop = false;
			T_PASS("Completed stres test successfully.");
		}

		T_QUIET;
		T_ASSERT_POSIX_ZERO(pthread_mutex_unlock(&context.mutex),
				"[%ld] pthread_mutex_unlock", context.count);
	}

	for (i = 0; i < threads; ++i) {
		T_ASSERT_POSIX_ZERO(pthread_join(p[i], NULL), NULL);
	}
}
