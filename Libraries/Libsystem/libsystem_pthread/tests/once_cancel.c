#include <pthread.h>

#include "darwintest_defaults.h"

static volatile int once_invoked = 0;

static void
cancelation_handler(void * __unused arg)
{
	T_LOG("cancelled");
}

__attribute__((noreturn))
static void
await_cancelation(void)
{
	pthread_cleanup_push(cancelation_handler, NULL);
	T_LOG("waiting for cancellation");

	// can't use darwintest once cancellation is enabled
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	while (true) {
		pthread_testcancel();
		sched_yield();
	}

	pthread_cleanup_pop(0);
}

static void *
await_cancelation_in_once(void *arg)
{
	// disable cancellation until pthread_once to protect darwintest
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	T_LOG("starting the thread");
	pthread_once_t *once = (pthread_once_t *)arg;
	pthread_once(once, await_cancelation);
	return NULL;
}

static void
oncef(void)
{
	T_LOG("once invoked");
	once_invoked++;
}

T_DECL(once_cancel, "pthread_once is re-executed if cancelled")
{
	pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_t t;
	void *join_result = NULL;

	T_ASSERT_POSIX_ZERO(
			pthread_create(&t, NULL, await_cancelation_in_once, &once), NULL);
	T_ASSERT_POSIX_ZERO(pthread_cancel(t), NULL);
	T_ASSERT_POSIX_ZERO(pthread_join(t, &join_result), NULL);
	T_ASSERT_EQ(join_result, PTHREAD_CANCELED, NULL);

	T_ASSERT_POSIX_ZERO(pthread_once(&once, oncef), NULL);
	T_ASSERT_EQ(once_invoked, 1, NULL);
}
