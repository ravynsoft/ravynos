#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "darwintest_defaults.h"

#define STACK_SIZE      32768
#define THREAD_DEPTH    2000

static unsigned int glob = 0;
static unsigned int i;

static pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

static void *
thread_exit(__unused void *arg)
{
	unsigned int count;

	sleep(5);
	pthread_mutex_lock(&count_lock);
	count = ++glob;
	pthread_mutex_unlock(&count_lock);

	T_QUIET; T_EXPECT_NE(pthread_mach_thread_np(pthread_self()), (mach_port_t)0, NULL);

	if (count == THREAD_DEPTH){
		T_PASS("all the threads survived main thread exit");
		T_END;
	}
	return NULL;
}

T_DECL(pthread_exit, "pthread_exit", T_META_LTEPHASE(LTE_INSTALLEDUSEROS))
{
	int j;
	pthread_t th[THREAD_DEPTH];

	T_LOG("Creating threads %d..%d", i, i+THREAD_DEPTH-1);
	for (j = 0;  j < THREAD_DEPTH;  j++) {
		pthread_attr_t  attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		T_QUIET; T_ASSERT_POSIX_SUCCESS(pthread_create(&th[j], &attr, thread_exit, (void *)&glob), NULL);
		pthread_attr_destroy(&attr);
	}
	pthread_exit(pthread_self());
	T_FAIL("Zombie walks");
}

static void *
thread_stub(__unused void *arg)
{
	return NULL;
}

T_DECL(pthread_exit_private_stacks, "pthread_exit with private stacks",
       T_META_CHECK_LEAKS(NO))
{
	int j;
	pthread_t th[THREAD_DEPTH];
	void *stacks[THREAD_DEPTH];

	for (j = 0; j < THREAD_DEPTH; j++) {
		T_QUIET; T_ASSERT_NOTNULL((stacks[j] = malloc(STACK_SIZE)), NULL);
	}

	for (i=0;i < 20; i++) {
		for (j = 0;  j < THREAD_DEPTH;  j++) {
			pthread_attr_t  attr;
			pthread_attr_init(&attr);
			pthread_attr_setstack(&attr, stacks[j], STACK_SIZE);
			T_QUIET; T_ASSERT_POSIX_SUCCESS(pthread_create(&th[j], &attr, thread_stub, (void *)&glob), NULL);
			pthread_attr_destroy(&attr);
		}
		for (j = 0;  j < THREAD_DEPTH;  j++) {
			T_QUIET; T_ASSERT_POSIX_SUCCESS(pthread_join(th[j], NULL), NULL);
		}
		T_PASS("Created threads %d..%d", i*THREAD_DEPTH, (i+1)*THREAD_DEPTH-1);
	}

}

T_DECL(pthread_exit_detached, "pthread_exit with detached threads")
{
	int j;
	pthread_t th[THREAD_DEPTH];

	for (i=0;i < 20; i++) {
		for (j = 0;  j < THREAD_DEPTH;  j++) {
			pthread_attr_t  attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			T_QUIET; T_ASSERT_POSIX_SUCCESS(pthread_create(&th[j], &attr, thread_stub, (void *)&glob), NULL);
			pthread_attr_destroy(&attr);
		}
		sleep(1);
		T_PASS("Created threads %d..%d", i*THREAD_DEPTH, (i+1)*THREAD_DEPTH-1);
	}
	T_PASS("Success!");
}
