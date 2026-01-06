#include <pthread.h>

#include "darwintest_defaults.h"

#define MAX_THREADS 512
#define THREAD_DEPTH 32

static void *
thread(void * arg)
{
	T_LOG("thread %lx here: %d", (uintptr_t)pthread_self(), (int)arg);
	return (arg);
}

T_DECL(pthread_bulk_create, "pthread_bulk_create")
{
	void *thread_res;
	pthread_t t[THREAD_DEPTH];

	for (int i = 0; i < MAX_THREADS; i += THREAD_DEPTH) {
		T_LOG("Creating threads %d..%d\n", i, i + THREAD_DEPTH - 1);
		for (int j = 0; j < THREAD_DEPTH; j++) {
			void *arg = (void *)(intptr_t)(i + j);
			T_QUIET; T_ASSERT_POSIX_ZERO(
					pthread_create(&t[j], NULL, thread, arg), NULL);
		}
		T_LOG("Waiting for threads");
		for (int j = 0; j < THREAD_DEPTH; j++) {
			T_QUIET; T_ASSERT_POSIX_ZERO(pthread_join(t[j], &thread_res), NULL);
			T_QUIET; T_ASSERT_EQ(i + j, (int)thread_res, "thread return value");
		}
	}
}
