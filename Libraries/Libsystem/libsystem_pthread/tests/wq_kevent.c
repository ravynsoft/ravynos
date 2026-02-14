#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/qos.h>

#include <dispatch/dispatch.h>

#include "../private/workqueue_private.h"
#include "../private/qos_private.h"

#include "wq_kevent.h"

static dispatch_semaphore_t sema;
static dispatch_time_t timeout;

static int do_wait(int threads){
	for (int i = 0; i < threads; i++){
		int ret = dispatch_semaphore_wait(sema, timeout);
		if (ret){
			fprintf(stderr, "timout waiting for thread %d.\n", i);
			return 1;
		}
	}
	fprintf(stderr, "\tsuccessfully signaled by %d threads.\n\n", threads);
	return 0;
}

static void workqueue_func(pthread_priority_t priority){
	fprintf(stderr, "WARNING: workqueue_func called.\n");
	dispatch_semaphore_signal(sema);
}

void (^cb)(pthread_priority_t p) = NULL;
static void workqueue_func_kevent(void **buf, int *count){
	pthread_priority_t p = (pthread_priority_t)pthread_getspecific(4);
	fprintf(stderr, "\tthread with qos %s spawned. (buf: %p, count: %d)\n", describe_pri(p), buf ? *buf : NULL, count ? *count : 0);

	if (cb){
		cb(p);
	}

	dispatch_semaphore_signal(sema);
}

int main(int argc, char *argv[]){
	int ret = 0;
	int exit_status = 0;

	ret = _pthread_workqueue_init_with_kevent(workqueue_func, workqueue_func_kevent, 0, 0);
	assert(ret == 0);

	sema = dispatch_semaphore_create(0);
	assert(sema != NULL);
	timeout = dispatch_time(DISPATCH_TIME_NOW, 5LL * NSEC_PER_SEC);

	_pthread_workqueue_set_event_manager_priority(_pthread_qos_class_encode(QOS_CLASS_UTILITY,0,0));

	// one constrained thread
	requests[0].priority = _pthread_qos_class_encode(QOS_CLASS_USER_INTERACTIVE, 0, 0);
	requests[0].count = 1;

	if ((ret = do_req()) < 0) return ret;
	if ((ret = do_wait(1)) < 0) return ret;

	// one overcommit thread
	requests[0].priority = _pthread_qos_class_encode(QOS_CLASS_USER_INTERACTIVE, 0, _PTHREAD_PRIORITY_OVERCOMMIT_FLAG);
	requests[0].count = 1;

	if ((ret = do_req()) < 0) return ret;
	if ((ret = do_wait(1)) < 0) return ret;

	// one event manager
	requests[0].priority = _pthread_qos_class_encode(QOS_CLASS_USER_INTERACTIVE, 0, _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG);
	requests[0].count = 1;

	if ((ret = do_req()) < 0) return ret;
	if ((ret = do_wait(1)) < 0) return ret;

	// one constrained thread
	requests[0].priority = _pthread_qos_class_encode(QOS_CLASS_USER_INTERACTIVE, 0, 0);
	requests[0].count = 1;

	if ((ret = do_req()) < 0) return ret;
	if ((ret = do_wait(1)) < 0) return ret;

	// whole bunch of constrained threads (must be last)

	dispatch_semaphore_t mgr_sema = dispatch_semaphore_create(0);
	assert(mgr_sema != NULL);

	requests[0].priority = _pthread_qos_class_encode(QOS_CLASS_USER_INTERACTIVE, 0, 0);
	requests[0].count = 1;

	cb = ^(pthread_priority_t p){ 
		if (p & _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG){
			dispatch_semaphore_signal(mgr_sema);
		}

		// burn some CPU
		for (int i = 0; i < 1000000; i++){
			char c[32];
			sprintf(c, "%d", i);
		}
	};
	for (int i = 0; i < 8; i++)
		if ((ret = do_req()) < 0) return ret;
	ret = dispatch_semaphore_wait(mgr_sema, timeout);
	if (ret) {
		fprintf(stderr, "timeout waiting for a manager thread");
		return 1;
	}
	fprintf(stderr, "\tsucessfully signaled by a manager thread.\n");
	
	return 0;
}
