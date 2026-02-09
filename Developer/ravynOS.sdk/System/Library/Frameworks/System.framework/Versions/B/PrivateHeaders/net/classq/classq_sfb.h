/*
 * Copyright (c) 2011-2012 Apple Inc. All rights reserved.
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

#ifndef _NET_CLASSQ_CLASSQ_SFB_H_
#define _NET_CLASSQ_CLASSQ_SFB_H_


#ifdef __cplusplus
extern "C" {
#endif

#define SFB_FP_SHIFT    14                      /* fixed-point shift (Q14) */
#define SFB_LEVELS      2                       /* L */
#define SFB_BINS_SHIFT  5
#define SFB_BINS        (1 << SFB_BINS_SHIFT)   /* N */

struct sfbstats {
	u_int64_t               drop_early;
	u_int64_t               drop_pbox;
	u_int64_t               drop_queue;
	u_int64_t               marked_packets;
	u_int64_t               pbox_packets;
	u_int64_t               pbox_time;
	u_int64_t               hold_time;
	u_int64_t               dequeue_avg;
	u_int64_t               rehash_intval;
	u_int64_t               num_rehash;
	u_int64_t               null_flowid;
	u_int64_t               flow_controlled;
	u_int64_t               flow_feedback;
	u_int64_t               dequeue_stall;
};

struct sfbbinstats {
	int16_t         pmark;          /* marking probability in Q format */
	u_int16_t       pkts;           /* number of packets */
	u_int32_t       bytes;          /* number of bytes */
};

struct sfb_stats {
	u_int32_t               allocation;
	u_int32_t               dropthresh;
	u_int32_t               clearpkts;
	u_int32_t               current;
	u_int64_t               target_qdelay;
	u_int64_t               update_interval;
	u_int64_t               min_estdelay;
	u_int32_t               delay_fcthreshold;
	u_int32_t               flags;
	struct sfbstats         sfbstats;
	struct sfbbins {
		struct sfbbinstats stats[SFB_LEVELS][SFB_BINS];
	} binstats[2] __attribute__((aligned(8)));
};


#ifdef __cplusplus
}
#endif
#endif /* _NET_CLASSQ_CLASSQ_SFB_H_ */
