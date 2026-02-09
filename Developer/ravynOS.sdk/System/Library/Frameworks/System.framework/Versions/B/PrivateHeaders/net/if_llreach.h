/*
 * Copyright (c) 2011-2014 Apple Inc. All rights reserved.
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

#ifndef _NET_IF_LLREACH_H_
#define _NET_IF_LLREACH_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <sys/types.h>

/*
 * Per-interface link-layer reachability information (private).
 */
#define IF_LLREACHINFO_ADDRLEN          64      /* max ll addr len */
#define IF_LLREACHINFO_RESERVED2        16      /* more reserved bits */

struct if_llreach_info {
	u_int32_t               lri_refcnt;     /* reference count */
	u_int32_t               lri_ifindex;    /* interface index */
	u_int64_t               lri_expire;     /* expiration (calendar) time */
	u_int32_t               lri_probes;     /* total # of probes */
	u_int16_t               lri_reserved;   /* for future use */
	u_int16_t               lri_proto;      /* ll proto */
	u_int8_t                lri_addr[IF_LLREACHINFO_ADDRLEN]; /* ll addr */
	int32_t                 lri_rssi;       /* received signal strength */
	int32_t                 lri_lqm;        /* link quality metric */
	int32_t                 lri_npm;        /* node proximity metric */
	u_int8_t                lri_reserved2[IF_LLREACHINFO_RESERVED2];
};


#ifdef  __cplusplus
}
#endif
#endif /* !_NET_IF_LLREACH_H_ */
