/*% clang -o # -Wall -Wextra -I/System/Library/Frameworks/System.framework/PrivateHeaders %
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/qos.h>
#include <sys/resource.h>
#include <pthread.h>

#include <mach/mach.h>
#include <mach/host_info.h>
#include <mach/mach_error.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/mach_syscalls.h>
#include <mach/policy.h>
#include <mach/task_policy.h>

#define QOS_TIER(i) \
	(((i) == THREAD_QOS_UNSPECIFIED)        ?  QOS_CLASS_UNSPECIFIED     :       \
	 ((i)  == THREAD_QOS_USER_INTERACTIVE)   ? QOS_CLASS_USER_INTERACTIVE :       \
	 ((i)  == THREAD_QOS_USER_INITIATED)     ? QOS_CLASS_USER_INITIATED   :       \
	 ((i)  == THREAD_QOS_LEGACY)             ? QOS_CLASS_DEFAULT          :       \
	 ((i)  == THREAD_QOS_UTILITY)            ? QOS_CLASS_UTILITY          :       \
	 ((i)  == THREAD_QOS_BACKGROUND)         ? QOS_CLASS_BACKGROUND       :       \
	 ((i)  == THREAD_QOS_MAINTENANCE)        ? QOS_CLASS_MAINTENANCE      :       \
	 -1)

struct kern_qos {
	long requested;
	long override;
};

void get_kern_qos(struct kern_qos *kern_qos){
	kern_return_t kr;
	boolean_t get_default = false;
	mach_msg_type_number_t count;

	struct thread_policy_state thread_policy;
	count = THREAD_POLICY_STATE_COUNT;
	kr = thread_policy_get(mach_thread_self(), THREAD_POLICY_STATE, (thread_policy_t)&thread_policy, &count, &get_default);
	if (kr != KERN_SUCCESS) { mach_error("thread_policy_get(... THREAD_POLICY_STATE ...)", kr); }

	kern_qos->requested = QOS_TIER((thread_policy.requested & POLICY_REQ_TH_QOS_MASK) >> POLICY_REQ_TH_QOS_SHIFT);
	kern_qos->override = QOS_TIER((thread_policy.requested & POLICY_REQ_TH_QOS_OVER_MASK) >> POLICY_REQ_TH_QOS_OVER_SHIFT);
}

void assert_fixedpri(bool fixedpri){
	kern_return_t kr;
	boolean_t get_default = false;
	mach_msg_type_number_t count;

	thread_extended_policy_data_t extpol;
	count = THREAD_EXTENDED_POLICY_COUNT;
	kr = thread_policy_get(mach_thread_self(), THREAD_EXTENDED_POLICY, (thread_policy_t)&extpol, &count, &get_default);
	if (kr != KERN_SUCCESS) { mach_error("thread_policy_get(... THREAD_EXTENDED_POLICY ...)", kr); }

	assert(extpol.timeshare == !fixedpri);
}

void *assert_thread_qos(void *arg){
	struct kern_qos *correct_kern_qos = (struct kern_qos *)arg;
	struct kern_qos actual_kern_qos;

	get_kern_qos(&actual_kern_qos);

	assert(actual_kern_qos.requested == qos_class_self());
	assert(actual_kern_qos.requested == correct_kern_qos->requested);
	assert(actual_kern_qos.override == correct_kern_qos->override);

	return NULL;
}

void *fixedpri_test(void *arg){
	struct kern_qos *correct_kern_qos = (struct kern_qos *)arg;

	assert_thread_qos(correct_kern_qos);
	assert_fixedpri(false);

	pthread_set_fixedpriority_self();

	assert_thread_qos(correct_kern_qos);
	assert_fixedpri(true);

	pthread_set_timeshare_self();

	assert_thread_qos(correct_kern_qos);
	assert_fixedpri(false);

	return NULL;
}

int main(){
	if (geteuid() != 0){
		printf("Must be run as root\n");
		return 1;
	}

	struct kern_qos kern_qos;

	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	// Main thread QoS

	kern_qos.requested = qos_class_self();
	kern_qos.override = QOS_CLASS_UNSPECIFIED;

	assert_thread_qos(&kern_qos);
	assert(qos_class_self() == qos_class_main());

	// Created pthread

	kern_qos.requested = QOS_CLASS_UTILITY;
	kern_qos.override = QOS_CLASS_UNSPECIFIED;

	pthread_attr_set_qos_class_np(&attr, QOS_CLASS_UTILITY, 0);
	pthread_create(&thread, &attr, assert_thread_qos, &kern_qos);
	pthread_join(thread, NULL);

	// pthread_set_fixedpriority_self()

	kern_qos.requested = QOS_CLASS_USER_INITIATED;
	kern_qos.override = QOS_CLASS_UNSPECIFIED;

	pthread_attr_set_qos_class_np(&attr, QOS_CLASS_USER_INITIATED, 0);
	pthread_create(&thread, &attr, fixedpri_test, &kern_qos);
	pthread_join(thread, NULL);

	return 0;
}
