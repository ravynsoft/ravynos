#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <sys/qos.h>

#include <dispatch/dispatch.h>

#include "../private/workqueue_private.h"
#include "../private/qos_private.h"

#include "wq_kevent.h"

static int rand_qos(){
	switch (rand() % 5){
		case 0: return QOS_CLASS_USER_INTERACTIVE;
		case 1: return QOS_CLASS_USER_INITIATED;
		case 2: return QOS_CLASS_DEFAULT;
		case 3: return QOS_CLASS_UTILITY;
		case 4: return QOS_CLASS_BACKGROUND;
	}
	return QOS_CLASS_UNSPECIFIED;
}

static void workqueue_func(pthread_priority_t priority){
	fprintf(stderr, "WARNING: workqueue_func called.\n");
}

static void workqueue_func_kevent(void **buf, int *count){
	pthread_priority_t p = (pthread_priority_t)pthread_getspecific(4);
	fprintf(stderr, "\tthread with qos %s spawned (count: %d).\n", describe_pri(p), *count);

	//struct timeval start, stop;
	//gettimeofday(&start, NULL);

	for (int i = 0; i < (rand() % 10000) * 1000 + 50000; i++){
		burn_cpu();
	}

	//gettimeofday(&stop, NULL);
	//fprintf(stderr, "\tthread exited %ld usec later\n", stop.tv_usec - start.tv_usec + (stop.tv_sec - start.tv_sec) * 1000000);
}

int main(int argc, char *argv[]){
	int ret = 0;

	ret = _pthread_workqueue_init_with_kevent(workqueue_func, workqueue_func_kevent, 0, 0);
	assert(ret == 0);

	int iteration = 0;
	while (iteration++ < 1000){
		switch (iteration % 5){
			case 0:
				// one event manager
				bzero(requests, sizeof(requests));
				requests[0].priority = _pthread_qos_class_encode(rand_qos(), 0, _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG);
				requests[0].count = 1;

				if ((ret = do_req()) < 0) return ret;
				break;

			case 1:
				// one constrained thread
				bzero(requests, sizeof(requests));
				requests[0].priority = _pthread_qos_class_encode(rand_qos(), 0, 0);
				requests[0].count = rand() % 2;
				if (requests[0].count > 0 && (ret = do_req()) < 0) return ret;
				break;

			case 2:
				// one event manager
				bzero(requests, sizeof(requests));
				requests[0].priority = _pthread_qos_class_encode(rand_qos(), 0, _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG);
				requests[0].count = 1;

				if ((ret = do_req()) < 0) return ret;
				break;

			case 3:
				// one overcommit thread
				bzero(requests, sizeof(requests));
				requests[0].priority = _pthread_qos_class_encode(rand_qos(), 0, _PTHREAD_PRIORITY_OVERCOMMIT_FLAG);
				requests[0].count = rand() % 2;

				if (requests[0].count > 0 && (ret = do_req()) < 0) return ret;
				break;

			case 4:
				// varied constrained threads
				bzero(requests, sizeof(requests));
				requests[0].priority = _pthread_qos_class_encode(QOS_CLASS_USER_INTERACTIVE, 0, 0);
				requests[0].count = rand() % 4;
				requests[1].priority = _pthread_qos_class_encode(QOS_CLASS_USER_INITIATED, 0, 0);
				requests[1].count = rand() % 4;
				requests[2].priority = _pthread_qos_class_encode(QOS_CLASS_UTILITY, 0, 0);
				requests[2].count = rand() % 4;
				requests[3].priority = _pthread_qos_class_encode(QOS_CLASS_BACKGROUND, 0, 0);
				requests[3].count = rand() % 4;
				if ((requests[0].count + requests[1].count + requests[2].count + requests[3].count) > 0 && (ret = do_req()) < 0) return ret;
				break;
		}
		usleep(rand() % 100000);
	}

	return 0;
}
