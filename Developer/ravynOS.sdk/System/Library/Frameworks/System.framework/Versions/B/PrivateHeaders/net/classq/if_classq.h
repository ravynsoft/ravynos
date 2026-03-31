/*
 * Copyright (c) 2011-2018 Apple Inc. All rights reserved.
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

#ifndef _NET_CLASSQ_IF_CLASSQ_H_
#define _NET_CLASSQ_IF_CLASSQ_H_

#define IFCQ_SC_MAX             10              /* max number of queues */


#include <net/pktsched/pktsched_tcq.h>
#include <net/pktsched/pktsched_qfq.h>
#include <net/pktsched/pktsched_fq_codel.h>

#ifdef __cplusplus
extern "C" {
#endif
struct if_ifclassq_stats {
	u_int32_t       ifqs_len;
	u_int32_t       ifqs_maxlen;
	struct pktcntr  ifqs_xmitcnt;
	struct pktcntr  ifqs_dropcnt;
	u_int32_t       ifqs_scheduler;
	union {
		struct tcq_classstats   ifqs_tcq_stats;
		struct qfq_classstats   ifqs_qfq_stats;
		struct fq_codel_classstats      ifqs_fq_codel_stats;
	};
} __attribute__((aligned(8)));

#ifdef __cplusplus
}
#endif

#endif /* _NET_CLASSQ_IF_CLASSQ_H_ */
