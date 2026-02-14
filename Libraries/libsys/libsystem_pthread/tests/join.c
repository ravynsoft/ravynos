#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <libkern/OSAtomicQueue.h>

#include "darwintest_defaults.h"

#define WAITTIME (100 * 1000)

static inline void*
test(void)
{
	static uintptr_t idx;
	return (void*)idx;
}

static void *
thread(void *param)
{
	usleep(WAITTIME);
	return param;
}

/*
static void *
thread1(void *param)
{
	int res;
	pthread_t p = param;

	usleep(WAITTIME);
	res = pthread_join(p, NULL);
	assert(res == 0);
	return 0;
}
*/

T_DECL(join, "pthread_join",
		T_META_ALL_VALID_ARCHS(YES))
{
	int res;
	kern_return_t kr;
	pthread_t p = NULL;
	void *param, *value;

	param = test();
	res = pthread_create(&p, NULL, thread, param);
	T_ASSERT_POSIX_ZERO(res, "pthread_create");
	value = NULL;
	res = pthread_join(p, &value);
	T_ASSERT_POSIX_ZERO(res, "pthread_join");
	T_ASSERT_EQ_PTR(param, value, "early join value");

	param = test();
	res = pthread_create(&p, NULL, thread, param);
	T_ASSERT_POSIX_ZERO(res, "pthread_create");
	usleep(3 * WAITTIME);
	value = NULL;
	res = pthread_join(p, &value);
	T_ASSERT_POSIX_ZERO(res, "pthread_join");
	T_ASSERT_EQ_PTR(param, value, "late join value");

	param = test();
	res = pthread_create_suspended_np(&p, NULL, thread, param);
	T_ASSERT_POSIX_ZERO(res, "pthread_create_suspended_np");
	kr = thread_resume(pthread_mach_thread_np(p));
	T_ASSERT_EQ_INT(kr, 0, "thread_resume");
	value = NULL;
	res = pthread_join(p, &value);
	T_ASSERT_POSIX_ZERO(res, "pthread_join");
	T_ASSERT_EQ_PTR(param, value, "suspended early join value");

	param = test();
	res = pthread_create_suspended_np(&p, NULL, thread, param);
	T_ASSERT_POSIX_ZERO(res, "pthread_create_suspended_np");
	kr = thread_resume(pthread_mach_thread_np(p));
	T_ASSERT_EQ_INT(kr, 0, "thread_resume");
	usleep(3 * WAITTIME);
	value = NULL;
	res = pthread_join(p, &value);
	T_ASSERT_POSIX_ZERO(res, "pthread_join");
	T_ASSERT_EQ_PTR(param, value, "suspended late join value");

	// This test is supposed to test joining on the main thread.  It's not
	// clear how to express this with libdarwintest for now.
	/*
	test();
	param = pthread_self();
	res = pthread_create_suspended_np(&p, NULL, thread1, param);
	T_ASSERT_POSIX_ZERO(res, "pthread_create_suspended_np");
	res = pthread_detach(p);
	T_ASSERT_POSIX_ZERO(res, "pthread_detach");
	kr = thread_resume(pthread_mach_thread_np(p));
	T_ASSERT_EQ_INT(kr, 0, "thread_resume");
	pthread_exit(0);
	*/
}

static void *
thread_stub(__unused void *arg)
{
	return NULL;
}

T_DECL(pthread_join_stress, "pthread_join in a loop")
{
	for (int i = 0; i < 1000; i++) {
		pthread_t th[16];
		for (int j = 0; j < i%16; j++){
			T_QUIET; T_ASSERT_POSIX_SUCCESS(pthread_create(&th[j], NULL, thread_stub, NULL), NULL);
		}
		for (int j = i%16; j >= 0; j--){
			T_QUIET; T_ASSERT_POSIX_SUCCESS(pthread_join(th[j], NULL), NULL);
		}
	}
	T_PASS("Success!");
}

static pthread_mutex_t join3way_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t join3way_cond = PTHREAD_COND_INITIALIZER;
static OSQueueHead join3way_queue = OS_ATOMIC_QUEUE_INIT;

struct join3way_item {
    pthread_t th;
    struct join3way_item *next;
};

static void *
join3way_joiner(__unused void *arg)
{
    pthread_mutex_lock(&join3way_mutex);
    while (1) {
        pthread_cond_wait(&join3way_cond, &join3way_mutex);
        struct join3way_item *item = OSAtomicDequeue(&join3way_queue,
                offsetof(struct join3way_item, next));
        pthread_join(item->th, NULL);
        free(item);
    }
    return NULL;
}

static void *
join3way_child(__unused void *arg)
{
    struct join3way_item *item = malloc(sizeof(struct join3way_item));
    item->th = pthread_self();
    item->next = NULL;
    OSAtomicEnqueue(&join3way_queue, item,
            offsetof(struct join3way_item, next));
    pthread_cond_signal(&join3way_cond);
    return NULL;
}

static void *
join3way_creator(__unused void *arg)
{
    pthread_attr_t attr;
    T_QUIET; T_ASSERT_POSIX_ZERO(pthread_attr_init(&attr), "pthread_attr_init");
    T_ASSERT_POSIX_ZERO(pthread_attr_set_qos_class_np(&attr,
            QOS_CLASS_USER_INTERACTIVE, 0), "pthread_attr_set_qos_class_np (child)");

    int n = 1000;
    while (--n > 0) {
        pthread_t t;
        T_QUIET; T_ASSERT_POSIX_SUCCESS(pthread_create(&t, &attr,
                join3way_child, NULL), "create thread");
    }
    T_ASSERT_EQ_INT(n, 0, "created all child threads");
    return NULL;
}

T_DECL(pthread_join_3way, "pthread_join from non-parent with priority inversion")
{
    pthread_attr_t joinerattr;
    T_QUIET; T_ASSERT_POSIX_ZERO(pthread_attr_init(&joinerattr),
            "pthread_attr_init");
    T_ASSERT_POSIX_ZERO(pthread_attr_set_qos_class_np(&joinerattr,
            QOS_CLASS_USER_INTERACTIVE, 0), "pthread_attr_set_qos_class_np");

    pthread_t joiner;
    T_ASSERT_POSIX_SUCCESS(pthread_create(&joiner, &joinerattr, join3way_joiner,
            NULL), "create joiner thread");

    pthread_attr_t creatorattr;
    T_QUIET; T_ASSERT_POSIX_ZERO(pthread_attr_init(&creatorattr),
            "pthread_attr_init");
    T_ASSERT_POSIX_ZERO(pthread_attr_set_qos_class_np(&joinerattr,
            QOS_CLASS_BACKGROUND, 0), "pthread_attr_set_qos_class_np (creator)");

    pthread_t creator;
    T_ASSERT_POSIX_SUCCESS(pthread_create(&creator, &creatorattr,
            join3way_creator, NULL), "create creator thread");

    pthread_join(creator, NULL);
}
