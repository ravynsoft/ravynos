#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#include "darwintest_defaults.h"

struct ctx {
	volatile int last_holder;
	volatile int quit;
	int iterations[2];
	pthread_mutex_t l;
};

static void *
job(struct ctx *ctx, int idx)
{
	int ret;
	while (!ctx->quit) {
		ret = pthread_mutex_trylock(&ctx->l);
		T_QUIET; T_ASSERT_TRUE(ret == EBUSY || ret == 0, "trylock");

		if (ret == EBUSY) {
			T_QUIET; T_ASSERT_POSIX_ZERO(pthread_mutex_lock(&ctx->l),
					"pthread_mutex_lock");
			// we know that the other thread was just holding the lock
			T_QUIET; T_ASSERT_EQ(ctx->last_holder, !idx,
					"expecting oppsosite last holder after failed trylock");
		}

		ctx->last_holder = idx;
		ctx->iterations[idx]++;

		T_QUIET; T_ASSERT_POSIX_ZERO(pthread_mutex_unlock(&ctx->l),
									 "pthread_mutex_unlock");
	}
	return NULL;
}

static void *
job1(void *ctx)
{
	return job((struct ctx *)ctx, 0);
}

static void *
job2(void *ctx)
{
	return job((struct ctx *)ctx, 1);
}

T_DECL(mutex_trylock, "pthread_mutex_trylock",
		T_META_ALL_VALID_ARCHS(YES))
{
	// This testcase spins up two threads with identical jobs. They try-lock
	// the same mutex. If that fails, they check that the last holder of the
	// lock is the other thread.
	const int test_duration = 10; // sec
	struct ctx ctx = {0};

	pthread_t t1, t2;

	T_ASSERT_POSIX_ZERO(pthread_mutex_init(&ctx.l, NULL),
						"pthread_mutex_init");
	T_ASSERT_POSIX_ZERO(pthread_create(&t1, NULL, job1, &ctx),
						"pthread_create 1");
	T_ASSERT_POSIX_ZERO(pthread_create(&t2, NULL, job2, &ctx),
						"pthread_create 2");

	sleep(test_duration);

	ctx.quit = 1;
	T_ASSERT_POSIX_ZERO(pthread_join(t1, NULL), "pthread join 1");
	T_ASSERT_POSIX_ZERO(pthread_join(t2, NULL), "pthread join 2");

	T_LOG("after %d seconds iterations 0: %d, 1: %d. Exiting\n",
		  test_duration, ctx.iterations[0], ctx.iterations[1]);
}
