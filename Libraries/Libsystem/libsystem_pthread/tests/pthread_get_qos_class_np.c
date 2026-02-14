#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/qos.h>
#include <sys/resource.h>
#include <pthread.h>

#include "darwintest_defaults.h"

static void *sleep_thread(void __unused *arg){
	sleep(1);
	return NULL;
}

/* Regression test for <rdar://problem/29209770> */
T_DECL(test_pthread_get_qos_class_np, "Test for pthread_get_qos_class_np()", T_META_CHECK_LEAKS(NO)) {
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_set_qos_class_np(&attr, QOS_CLASS_BACKGROUND, 0);
	pthread_create(&thread, &attr, sleep_thread, NULL);

	qos_class_t qos;
	pthread_get_qos_class_np(thread, &qos, NULL);

	T_EXPECT_EQ(qos, (qos_class_t)QOS_CLASS_BACKGROUND, NULL);
}
