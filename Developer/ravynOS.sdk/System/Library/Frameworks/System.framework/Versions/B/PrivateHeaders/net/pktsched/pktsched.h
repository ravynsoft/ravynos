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

#ifndef _PKTSCHED_PKTSCHED_H_
#define _PKTSCHED_PKTSCHED_H_

#ifdef __cplusplus
extern "C" {
#endif

/* packet scheduler type */
#define PKTSCHEDT_NONE          0       /* reserved */
#define PKTSCHEDT_CBQ           1       /* cbq */
#define PKTSCHEDT_HFSC          2       /* hfsc */
#define PKTSCHEDT_PRIQ          3       /* priority queue */
#define PKTSCHEDT_FAIRQ         4       /* fairq */
#define PKTSCHEDT_TCQ           5       /* traffic class queue */
#define PKTSCHEDT_QFQ           6       /* quick fair queueing */
#define PKTSCHEDT_FQ_CODEL      7       /* Flow queues with CoDel */
#define PKTSCHEDT_MAX           8       /* should be max sched type + 1 */


#ifdef __cplusplus
}
#endif
#endif /* _PKTSCHED_PKTSCHED_H_ */
