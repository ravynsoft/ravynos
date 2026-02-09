/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#ifndef _NET_PKTSCHED_FQ_CODEL_H_
#define _NET_PKTSCHED_FQ_CODEL_H_

#include <sys/types.h>
#include <sys/param.h>


#ifdef __cplusplus
extern "C" {
#endif


struct fq_codel_flowstats {
	u_int32_t       fqst_min_qdelay;
#define FQ_FLOWSTATS_OLD_FLOW   0x1
#define FQ_FLOWSTATS_NEW_FLOW   0x2
#define FQ_FLOWSTATS_LARGE_FLOW 0x4
#define FQ_FLOWSTATS_DELAY_HIGH 0x8
#define FQ_FLOWSTATS_FLOWCTL_ON 0x10
	u_int32_t       fqst_flags;
	u_int32_t       fqst_bytes;
	u_int32_t       fqst_flowhash;
};

#define FQ_IF_MAX_FLOWSTATS     20

struct fq_codel_classstats {
	u_int32_t       fcls_pri;
	u_int32_t       fcls_service_class;
	u_int32_t       fcls_quantum;
	u_int32_t       fcls_drr_max;
	int64_t         fcls_budget;
	u_int64_t       fcls_target_qdelay;
	u_int64_t       fcls_update_interval;
	u_int32_t       fcls_flow_control;
	u_int32_t       fcls_flow_feedback;
	u_int32_t       fcls_dequeue_stall;
	u_int32_t       fcls_flow_control_fail;
	u_int64_t       fcls_drop_overflow;
	u_int64_t       fcls_drop_early;
	u_int32_t       fcls_drop_memfailure;
	u_int32_t       fcls_flows_cnt;
	u_int32_t       fcls_newflows_cnt;
	u_int32_t       fcls_oldflows_cnt;
	u_int64_t       fcls_pkt_cnt;
	u_int64_t       fcls_dequeue;
	u_int64_t       fcls_dequeue_bytes;
	u_int64_t       fcls_byte_cnt;
	u_int32_t       fcls_throttle_on;
	u_int32_t       fcls_throttle_off;
	u_int32_t       fcls_throttle_drops;
	u_int32_t       fcls_dup_rexmts;
	u_int32_t       fcls_flowstats_cnt;
	struct fq_codel_flowstats fcls_flowstats[FQ_IF_MAX_FLOWSTATS];
};


#ifdef __cplusplus
}
#endif

#endif /* _NET_PKTSCHED_PKTSCHED_FQ_CODEL_H_ */
