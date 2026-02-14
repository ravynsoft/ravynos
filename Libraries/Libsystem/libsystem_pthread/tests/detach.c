#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dispatch/dispatch.h>
#include <sys/mman.h>

#include "darwintest_defaults.h"

static void*
thread_routine(void *loc)
{
	uintptr_t i = (uintptr_t)loc;

	switch (i % 3) {
	case 0:
		usleep(1000);
		break;
	case 1:
		pthread_exit(pthread_self());
		__builtin_unreachable();
	case 2:
		break;
	}
	return NULL;
}

T_DECL(pthread_detach, "Test creating and detaching threads in a loop",
		T_META_CHECK_LEAKS(NO), T_META_ALL_VALID_ARCHS(YES))
{
	const size_t count = 32;
	pthread_t ths[count];

	for (size_t i = 0; i < 100; i++) {
		for (size_t j = 0; j < count; j++) {
			T_ASSERT_POSIX_ZERO(pthread_create(&ths[j], NULL,
							thread_routine, (void *)j), "thread creation");
			T_ASSERT_POSIX_ZERO(pthread_detach(ths[j]), "thread detach");
		}
		usleep(50000);
	}
}
