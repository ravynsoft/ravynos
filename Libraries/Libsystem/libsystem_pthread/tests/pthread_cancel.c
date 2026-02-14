#include <pthread.h>

#include "darwintest_defaults.h"

static void *
run(void * __unused arg)
{
	while (true) {
		pthread_testcancel();
		sched_yield();
	}
}

T_DECL(pthread_cancel, "pthread_cancel",
		T_META_ALL_VALID_ARCHS(YES))
{
	pthread_t thread;
	void *join_result = NULL;
	T_ASSERT_POSIX_ZERO(pthread_create(&thread, NULL, run, NULL), NULL);
	T_ASSERT_POSIX_ZERO(pthread_cancel(thread), NULL);
	T_ASSERT_POSIX_ZERO(pthread_join(thread, &join_result), NULL);
	T_ASSERT_EQ(join_result, PTHREAD_CANCELED, NULL);
}
