/*
 * Copyright (c) 2007, 2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __PTHREAD_WORKQUEUE_H__
#define __PTHREAD_WORKQUEUE_H__

#include <stdbool.h>
#include <sys/cdefs.h>
#include <sys/event.h>
#include <Availability.h>
#include <pthread/pthread.h>
#include <pthread/qos.h>
#ifndef _PTHREAD_BUILDING_PTHREAD_
#include <pthread/qos_private.h>
#endif

#define PTHREAD_WORKQUEUE_SPI_VERSION 20170201

/* Feature checking flags, returned by _pthread_workqueue_supported()
 *
 * Note: These bits should match the definition of PTHREAD_FEATURE_*
 * bits defined in libpthread/kern/kern_internal.h */

#define WORKQ_FEATURE_DISPATCHFUNC	0x01	// pthread_workqueue_setdispatch_np is supported (or not)
#define WORKQ_FEATURE_FINEPRIO		0x02	// fine grained pthread workq priorities
#define WORKQ_FEATURE_MAINTENANCE	0x10	// QOS class maintenance
#define WORKQ_FEATURE_KEVENT        0x40    // Support for direct kevent delivery
#define WORKQ_FEATURE_WORKLOOP      0x80    // Support for direct workloop requests

/* Legacy dispatch priority bands */

#define WORKQ_NUM_PRIOQUEUE	4

#define WORKQ_HIGH_PRIOQUEUE	0	// high priority queue
#define WORKQ_DEFAULT_PRIOQUEUE	1	// default priority queue
#define WORKQ_LOW_PRIOQUEUE	2	// low priority queue
#define WORKQ_BG_PRIOQUEUE	3	// background priority queue
#define WORKQ_NON_INTERACTIVE_PRIOQUEUE 128 // libdispatch SPI level

/* Legacy dispatch workqueue function flags */
#define WORKQ_ADDTHREADS_OPTION_OVERCOMMIT 0x00000001

__BEGIN_DECLS

// Legacy callback prototype, used with pthread_workqueue_setdispatch_np
typedef void (*pthread_workqueue_function_t)(int queue_priority, int options, void *ctxt);
// New callback prototype, used with pthread_workqueue_init
typedef void (*pthread_workqueue_function2_t)(pthread_priority_t priority);

// Newer callback prototype, used in conjection with function2 when there are kevents to deliver
// both parameters are in/out parameters
#define WORKQ_KEVENT_EVENT_BUFFER_LEN 16
typedef void (*pthread_workqueue_function_kevent_t)(void **events, int *nevents);

typedef void (*pthread_workqueue_function_workloop_t)(uint64_t *workloop_id, void **events, int *nevents);

#define PTHREAD_WORKQUEUE_CONFIG_VERSION               2
#define PTHREAD_WORKQUEUE_CONFIG_MIN_SUPPORTED_VERSION 1
#define PTHREAD_WORKQUEUE_CONFIG_SUPPORTED_FLAGS       0
struct pthread_workqueue_config {
	uint32_t flags;
	uint32_t version;
	pthread_workqueue_function_kevent_t kevent_cb;
	pthread_workqueue_function_workloop_t workloop_cb;
	pthread_workqueue_function2_t workq_cb;
	uint64_t queue_serialno_offs;
	uint64_t queue_label_offs;
};

__API_AVAILABLE(macos(10.15), ios(13.0))
int
pthread_workqueue_setup(struct pthread_workqueue_config *cfg, size_t cfg_size);

// Initialises the pthread workqueue subsystem, passing the new-style callback prototype,
// the dispatchoffset and an unused flags field.
__API_AVAILABLE(macos(10.10), ios(8.0))
int
_pthread_workqueue_init(pthread_workqueue_function2_t func, int offset, int flags);

__API_AVAILABLE(macos(10.11), ios(9.0))
int
_pthread_workqueue_init_with_kevent(pthread_workqueue_function2_t queue_func, pthread_workqueue_function_kevent_t kevent_func, int offset, int flags);

__API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
int
_pthread_workqueue_init_with_workloop(pthread_workqueue_function2_t queue_func, pthread_workqueue_function_kevent_t kevent_func, pthread_workqueue_function_workloop_t workloop_func, int offset, int flags);

// Non-zero enables kill on current thread, zero disables it.
__API_AVAILABLE(macos(10.6), ios(3.2))
int
__pthread_workqueue_setkill(int);

// Dispatch function to be called when new worker threads are created.
__API_AVAILABLE(macos(10.8), ios(6.0))
int
pthread_workqueue_setdispatch_np(pthread_workqueue_function_t worker_func);

// Dispatch offset to be set in the kernel.
__API_AVAILABLE(macos(10.9), ios(7.0))
void
pthread_workqueue_setdispatchoffset_np(int offset);

// Request additional worker threads.
__API_AVAILABLE(macos(10.8), ios(6.0))
int
pthread_workqueue_addthreads_np(int queue_priority, int options, int numthreads);

// Retrieve the supported pthread feature set
__API_AVAILABLE(macos(10.10), ios(8.0))
int
_pthread_workqueue_supported(void);

// Request worker threads (fine grained priority)
__API_AVAILABLE(macos(10.10), ios(8.0))
int
_pthread_workqueue_addthreads(int numthreads, pthread_priority_t priority);

// Should this thread return to the kernel?
__API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
bool
_pthread_workqueue_should_narrow(pthread_priority_t priority);

__API_AVAILABLE(macos(10.11), ios(9.0))
int
_pthread_workqueue_set_event_manager_priority(pthread_priority_t priority);

// Apply a QoS override without allocating userspace memory
__API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
int
_pthread_qos_override_start_direct(mach_port_t thread, pthread_priority_t priority, void *resource);

// Drop a corresponding QoS override made above, if the resource matches
__API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
int
_pthread_qos_override_end_direct(mach_port_t thread, void *resource);

// Apply a QoS override without allocating userspace memory
__API_DEPRECATED_WITH_REPLACEMENT("_pthread_qos_override_start_direct",
		macos(10.10, 10.12), ios(8.0, 10.0), tvos(8.0, 10.0), watchos(1.0, 3.0))
int
_pthread_override_qos_class_start_direct(mach_port_t thread, pthread_priority_t priority);

// Drop a corresponding QoS override made above.
__API_DEPRECATED_WITH_REPLACEMENT("_pthread_qos_override_end_direct",
		macos(10.10, 10.12), ios(8.0, 10.0), tvos(8.0, 10.0), watchos(1.0, 3.0))
int
_pthread_override_qos_class_end_direct(mach_port_t thread);

// Apply a QoS override on a given workqueue thread.
__API_AVAILABLE(macos(10.10), ios(8.0))
int
_pthread_workqueue_override_start_direct(mach_port_t thread, pthread_priority_t priority);

// Apply a QoS override on a given workqueue thread.
__API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
int
_pthread_workqueue_override_start_direct_check_owner(mach_port_t thread, pthread_priority_t priority, mach_port_t *ulock_addr);

// Drop all QoS overrides on the current workqueue thread.
__API_AVAILABLE(macos(10.10), ios(8.0))
int
_pthread_workqueue_override_reset(void);

// Apply a QoS override on a given thread (can be non-workqueue as well) with a resource/queue token
__API_AVAILABLE(macos(10.10.2))
int
_pthread_workqueue_asynchronous_override_add(mach_port_t thread, pthread_priority_t priority, void *resource);

// Reset overrides for the given resource for the current thread
__API_AVAILABLE(macos(10.10.2))
int
_pthread_workqueue_asynchronous_override_reset_self(void *resource);

// Reset overrides for all resources for the current thread
__API_AVAILABLE(macos(10.10.2))
int
_pthread_workqueue_asynchronous_override_reset_all_self(void);

__API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
int
_pthread_workloop_create(uint64_t workloop_id, uint64_t options, pthread_attr_t *attr);

__API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
int
_pthread_workloop_destroy(uint64_t workloop_id);

__END_DECLS

#endif // __PTHREAD_WORKQUEUE_H__
