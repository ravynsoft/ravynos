/*
 * Copyright (c) 2000-2017 Apple Inc. All rights reserved.
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

/*	$FreeBSD: src/sys/netinet6/mld6_var.h,v 1.1.2.1 2000/07/15 07:14:36 kris Exp $	*/
/*	$KAME: mld6_var.h,v 1.4 2000/03/25 07:23:54 sumikawa Exp $	*/

/*
 * Copyright (C) 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NETINET6_MLD6_VAR_H_
#define _NETINET6_MLD6_VAR_H_
#include <sys/appleapiopts.h>

/*
 * Multicast Listener Discovery (MLD)
 * implementation-specific definitions.
 */

/*
 * Per-link MLD state.
 */
struct mld_ifinfo {
	uint32_t mli_ifindex;   /* interface this instance belongs to */
	uint32_t mli_version;   /* MLDv1 Host Compatibility Mode */
	uint32_t mli_v1_timer;  /* MLDv1 Querier Present timer (s) */
	uint32_t mli_v2_timer;  /* MLDv2 General Query (interface) timer (s)*/
	uint32_t mli_flags;     /* MLD per-interface flags */
	uint32_t mli_rv;        /* MLDv2 Robustness Variable */
	uint32_t mli_qi;        /* MLDv2 Query Interval (s) */
	uint32_t mli_qri;       /* MLDv2 Query Response Interval (s) */
	uint32_t mli_uri;       /* MLDv2 Unsolicited Report Interval (s) */
	uint32_t _pad;
};

#define MLIF_SILENT     0x00000001      /* Do not use MLD on this ifp */
#define MLIF_USEALLOW   0x00000002      /* Use ALLOW/BLOCK for joins/leaves */
#define MLIF_PROCESSED  0x00000004      /* Entry has been processed and can be skipped */

/*
 * MLD version tag.
 */
#define MLD_VERSION_NONE                0 /* Invalid */
#define MLD_VERSION_1                   1
#define MLD_VERSION_2                   2 /* Default */

#endif /* _NETINET6_MLD6_VAR_H_ */
