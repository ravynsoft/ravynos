/*
 * Copyright (c) 2014 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _WORKQUEUE_INTERNAL_H_
#define _WORKQUEUE_INTERNAL_H_

/* These definitions are shared between the kext and userspace inside the pthread project. Consolidating
 * duplicate definitions that used to exist in both projects, when separate.
 */

// Sometimes something gets passed a bucket number and we need a way to express
// that it's actually the event manager.  Use the (0)th bucket for that.
#define WORKQ_THREAD_QOS_MIN        (THREAD_QOS_MAINTENANCE)
#define WORKQ_THREAD_QOS_MAX        (THREAD_QOS_LAST - 1)
#define WORKQ_THREAD_QOS_CLEANUP    (THREAD_QOS_LEGACY)
#define WORKQ_THREAD_QOS_MANAGER    (THREAD_QOS_LAST) // outside of MIN/MAX

#define WORKQ_NUM_QOS_BUCKETS       (WORKQ_THREAD_QOS_MAX)
#define WORKQ_NUM_BUCKETS           (WORKQ_THREAD_QOS_MAX + 1)
#define WORKQ_IDX(qos)              ((qos) - 1) // 0 based index

// magical `nkevents` values for _pthread_wqthread
#define WORKQ_EXIT_THREAD_NKEVENT   (-1)

#endif // _WORKQUEUE_INTERNAL_H_
