/*
 * Copyright (c) 2013-2014 Apple Inc. All rights reserved.
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

#ifndef _QOS_PRIVATE_H
#define _QOS_PRIVATE_H

#include <pthread/qos.h>
#include <pthread/priority_private.h>
#include <sys/qos.h> /* qos_class_t */
#include <sys/qos_private.h>

#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
// allow __DARWIN_C_LEVEL to turn off the use of mach_port_t
#include <mach/port.h>
#endif

// redeffed here to avoid leaving __QOS_ENUM defined in the public header
#define __QOS_ENUM(name, type, ...) enum { __VA_ARGS__ }; typedef type name##_t
#define __QOS_AVAILABLE_10_10
#define __QOS_AVAILABLE_10_11
#define __QOS_AVAILABLE_10_12

#if defined(__has_feature) && defined(__has_extension)
#if __has_feature(objc_fixed_enum) || __has_extension(cxx_strong_enums)
#undef __QOS_ENUM
#define __QOS_ENUM(name, type, ...) typedef enum : type { __VA_ARGS__ } name##_t
#endif
#if __has_feature(enumerator_attributes)
#undef __QOS_AVAILABLE_10_10
#define __QOS_AVAILABLE_10_10 __API_AVAILABLE(macos(10.10), ios(8.0))
#undef __QOS_AVAILABLE_10_11
#define __QOS_AVAILABLE_10_11 __API_AVAILABLE(macos(10.11), ios(9.0), tvos(9.0), watchos(2.0))
#undef __QOS_AVAILABLE_10_12
#define __QOS_AVAILABLE_10_12 __API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
#undef __QOS_AVAILABLE_10_15_1
#define __QOS_AVAILABLE_10_15_1 __API_AVAILABLE(macos(10.15.1), ios(13.2), tvos(13.2), watchos(6.2))
#endif
#endif

// This enum matches workq_set_self_flags in
// xnu's workqueue_internal.h.
__QOS_ENUM(_pthread_set_flags, unsigned int,
   _PTHREAD_SET_SELF_QOS_FLAG __QOS_AVAILABLE_10_10 = 0x1,
   _PTHREAD_SET_SELF_VOUCHER_FLAG __QOS_AVAILABLE_10_10 = 0x2,
   _PTHREAD_SET_SELF_FIXEDPRIORITY_FLAG __QOS_AVAILABLE_10_11 = 0x4,
   _PTHREAD_SET_SELF_TIMESHARE_FLAG __QOS_AVAILABLE_10_11 = 0x8,
   _PTHREAD_SET_SELF_WQ_KEVENT_UNBIND __QOS_AVAILABLE_10_12 = 0x10,
   _PTHREAD_SET_SELF_ALTERNATE_AMX __QOS_AVAILABLE_10_15_1 = 0x20,
);

#undef __QOS_ENUM
#undef __QOS_AVAILABLE_10_10
#undef __QOS_AVAILABLE_10_11
#undef __QOS_AVAILABLE_10_12

#ifndef KERNEL

__BEGIN_DECLS

/*!
 * @function pthread_set_qos_class_np
 *
 * @abstract
 * Sets the requested QOS class and relative priority of the current thread.
 *
 * @discussion
 * The QOS class and relative priority represent an overall combination of
 * system quality of service attributes on a thread.
 *
 * Subsequent calls to interfaces such as pthread_setschedparam() that are
 * incompatible or in conflict with the QOS class system will unset the QOS
 * class requested with this interface and pthread_get_qos_class_np() will
 * return QOS_CLASS_UNSPECIFIED thereafter. A thread so modified is permanently
 * opted-out of the QOS class system and calls to this function to request a QOS
 * class for such a thread will fail and return EPERM.
 *
 * @param __pthread
 * The current thread as returned by pthread_self().
 * EINVAL will be returned if any other thread is provided.
 *
 * @param __qos_class
 * A QOS class value:
 *	- QOS_CLASS_USER_INTERACTIVE
 *	- QOS_CLASS_USER_INITIATED
 *	- QOS_CLASS_DEFAULT
 *	- QOS_CLASS_UTILITY
 *	- QOS_CLASS_BACKGROUND
 *	- QOS_CLASS_MAINTENANCE
 * EINVAL will be returned if any other value is provided.
 *
 * @param __relative_priority
 * A relative priority within the QOS class. This value is a negative offset
 * from the maximum supported scheduler priority for the given class.
 * EINVAL will be returned if the value is greater than zero or less than
 * QOS_MIN_RELATIVE_PRIORITY.
 *
 * @return
 * Zero if successful, othwerise an errno value.
 */
__API_DEPRECATED_WITH_REPLACEMENT("pthread_set_qos_class_self_np", macos(10.10, 10.10), ios(8.0, 8.0))
int
pthread_set_qos_class_np(pthread_t __pthread,
						 qos_class_t __qos_class,
						 int __relative_priority);

/* Private interfaces for libdispatch to encode/decode specific values of pthread_priority_t. */

// Encode a class+priority pair into a pthread_priority_t,
__API_AVAILABLE(macos(10.10), ios(8.0))
pthread_priority_t
_pthread_qos_class_encode(qos_class_t qos_class, int relative_priority, unsigned long flags);

// Decode a pthread_priority_t into a class+priority pair.
__API_AVAILABLE(macos(10.10), ios(8.0))
qos_class_t
_pthread_qos_class_decode(pthread_priority_t priority, int *relative_priority, unsigned long *flags);

// Encode a legacy workqueue API priority into a pthread_priority_t. This API
// is deprecated and can be removed when the simulator no longer uses it.
__API_DEPRECATED("no longer used", macos(10.10, 10.13), ios(8.0, 11.0))
pthread_priority_t
_pthread_qos_class_encode_workqueue(int queue_priority, unsigned long flags);

#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL

// Set QoS or voucher, or both, on pthread_self()
__API_AVAILABLE(macos(10.10), ios(8.0))
int
_pthread_set_properties_self(_pthread_set_flags_t flags, pthread_priority_t priority, mach_port_t voucher);

// Set self to fixed priority without disturbing QoS or priority
__API_AVAILABLE(macos(10.10), ios(8.0))
int
pthread_set_fixedpriority_self(void);

// Inverse of pthread_set_fixedpriority_self()
__API_AVAILABLE(macos(10.10), ios(8.0))
int
pthread_set_timeshare_self(void);

// Set self to avoid running on the same AMX as
// other work in this group.
// Only allowed on non-workqueue pthreads
__API_AVAILABLE(macos(10.15.1), ios(13.2), tvos(13.2), watchos(6.2))
int
pthread_prefer_alternate_amx_self(void);

/*!
 * @const PTHREAD_MAX_PARALLELISM_PHYSICAL
 * Flag that can be used with pthread_qos_max_parallelism() and
 * pthread_time_constraint_max_parallelism() to ask for a count of physical
 * compute units available for parallelism (default is logical).
 */
#define PTHREAD_MAX_PARALLELISM_PHYSICAL 0x1

/*!
 * @function pthread_qos_max_parallelism
 *
 * @abstract
 * Returns the number of compute units available for parallel computation at
 * a specified QoS class.
 *
 * @param qos
 * The specified QoS class.
 *
 * @param flags
 * 0 or PTHREAD_MAX_PARALLELISM_PHYSICAL.
 *
 * @return
 * The number of compute units available for parallel computation for the
 * specified QoS, or -1 on failure (with errno set accordingly).
 */
__API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
int
pthread_qos_max_parallelism(qos_class_t qos, unsigned long flags);

/*!
 * @function pthread_time_constraint_max_parallelism()
 *
 * @abstract
 * Returns the number of compute units available for parallel computation on
 * realtime threads.
 *
 * @param flags
 * 0 or PTHREAD_MAX_PARALLELISM_PHYSICAL.
 *
 * @return
 * The number of compute units available for parallel computation on realtime
 * threads, or -1 on failure (with errno set accordingly).
 */
__API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
int
pthread_time_constraint_max_parallelism(unsigned long flags);

#endif // __DARWIN_C_LEVEL >= __DARWIN_C_FULL

__END_DECLS

#endif // KERNEL

#endif //_QOS_PRIVATE_H
