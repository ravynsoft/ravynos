#include <stdatomic.h>
#include <pthread.h>
#include <pthread/introspection_private.h>
#include <dispatch/dispatch.h>

#include "darwintest_defaults.h"

static pthread_introspection_hook_t prev_pthread_introspection_hook;

#define THREAD_COUNT 3

static void my_pthread_introspection_hook(unsigned int event, pthread_t thread,
		void *addr, size_t size)
{
	static atomic_int create_count;
	static atomic_int terminate_count;

	uint64_t tid;
	pthread_threadid_np(NULL, &tid);

	if (event == PTHREAD_INTROSPECTION_THREAD_CREATE) {
		T_LOG("event = PTHREAD_INTROSPECTION_THREAD_CREATE, thread = %p:%lld, addr = %p, size = 0x%zx", thread, tid, addr, size);
		create_count++;
	} else if (event == PTHREAD_INTROSPECTION_THREAD_TERMINATE) {
		T_LOG("event = PTHREAD_INTROSPECTION_THREAD_TERMINATE, thread = %p:%lld, addr = %p, size = 0x%zx", thread, tid, addr, size);
		terminate_count++;
		T_ASSERT_GE(create_count, THREAD_COUNT, NULL);
		T_PASS("Got termination events");
		T_END;
	}

	if (prev_pthread_introspection_hook != NULL){
		prev_pthread_introspection_hook(event, thread, addr, size);
	}
}

T_DECL(pthread_introspection, "PR-25679871",
       T_META_TIMEOUT(30), T_META_ALL_VALID_ARCHS(YES))
{
	prev_pthread_introspection_hook = pthread_introspection_hook_install(&my_pthread_introspection_hook);

	// minus one that comes after this block
	for (int i = 0; i < THREAD_COUNT - 1; i++) {
		T_LOG("Creating dispatch_async thread %d.", i);
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			T_LOG("Started dispatch_async thread %d.", i);
			sleep(3);
		});
	}
	dispatch_queue_t serial_queue = dispatch_queue_create("test queue", NULL);
	__block dispatch_block_t looping_block = ^{
		static int count;
		if (count++ < 20) {
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC), serial_queue, looping_block);
		} else {
			T_LOG("Looping block complete");
		}
	};
	T_LOG("Starting looping block");
	dispatch_async(serial_queue, looping_block);

	sleep(30);

	T_FAIL("Why are we still alive?");
}
