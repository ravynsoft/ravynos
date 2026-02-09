/*
 * Copyright (c) 2000-2020 Apple Inc. All rights reserved.
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

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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

/*
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ip_var.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _NETINET6_IP6_VAR_H_
#define _NETINET6_IP6_VAR_H_
#include <sys/appleapiopts.h>


#define IP6S_SRCRULE_COUNT 16
#include <netinet6/scope6_var.h>

struct  ip6stat {
	u_quad_t ip6s_total;            /* total packets received */
	u_quad_t ip6s_tooshort;         /* packet too short */
	u_quad_t ip6s_toosmall;         /* not enough data */
	u_quad_t ip6s_fragments;        /* fragments received */
	u_quad_t ip6s_fragdropped;      /* frags dropped(dups, out of space) */
	u_quad_t ip6s_fragtimeout;      /* fragments timed out */
	u_quad_t ip6s_fragoverflow;     /* fragments that exceeded limit */
	u_quad_t ip6s_forward;          /* packets forwarded */
	u_quad_t ip6s_cantforward;      /* packets rcvd for unreachable dest */
	u_quad_t ip6s_redirectsent;     /* packets forwarded on same net */
	u_quad_t ip6s_delivered;        /* datagrams delivered to upper level */
	u_quad_t ip6s_localout;         /* total ip packets generated here */
	u_quad_t ip6s_odropped;         /* lost packets due to nobufs, etc. */
	u_quad_t ip6s_reassembled;      /* total packets reassembled ok */
	u_quad_t ip6s_atmfrag_rcvd;     /* atomic fragments received */
	u_quad_t ip6s_fragmented;       /* datagrams successfully fragmented */
	u_quad_t ip6s_ofragments;       /* output fragments created */
	u_quad_t ip6s_cantfrag;         /* don't fragment flag was set, etc. */
	u_quad_t ip6s_badoptions;       /* error in option processing */
	u_quad_t ip6s_noroute;          /* packets discarded due to no route */
	u_quad_t ip6s_badvers;          /* ip6 version != 6 */
	u_quad_t ip6s_rawout;           /* total raw ip packets generated */
	u_quad_t ip6s_badscope;         /* scope error */
	u_quad_t ip6s_notmember;        /* don't join this multicast group */
	u_quad_t ip6s_nxthist[256];     /* next header history */
	u_quad_t ip6s_m1;               /* one mbuf */
	u_quad_t ip6s_m2m[32];          /* two or more mbuf */
	u_quad_t ip6s_mext1;            /* one ext mbuf */
	u_quad_t ip6s_mext2m;           /* two or more ext mbuf */
	u_quad_t ip6s_exthdrtoolong;    /* ext hdr are not continuous */
	u_quad_t ip6s_nogif;            /* no match gif found */
	u_quad_t ip6s_toomanyhdr;       /* discarded due to too many headers */

	/*
	 * statistics for improvement of the source address selection
	 * algorithm:
	 */
	/* number of times that address selection fails */
	u_quad_t ip6s_sources_none;
	/* number of times that an address on the outgoing I/F is chosen */
	u_quad_t ip6s_sources_sameif[SCOPE6_ID_MAX];
	/* number of times that an address on a non-outgoing I/F is chosen */
	u_quad_t ip6s_sources_otherif[SCOPE6_ID_MAX];
	/*
	 * number of times that an address that has the same scope
	 * from the destination is chosen.
	 */
	u_quad_t ip6s_sources_samescope[SCOPE6_ID_MAX];
	/*
	 * number of times that an address that has a different scope
	 * from the destination is chosen.
	 */
	u_quad_t ip6s_sources_otherscope[SCOPE6_ID_MAX];
	/* number of times that a deprecated address is chosen */
	u_quad_t ip6s_sources_deprecated[SCOPE6_ID_MAX];

	u_quad_t ip6s_forward_cachehit;
	u_quad_t ip6s_forward_cachemiss;

	/* number of times that each rule of source selection is applied. */
	u_quad_t ip6s_sources_rule[IP6S_SRCRULE_COUNT];

	/* number of times we ignored address on expensive secondary interfaces */
	u_quad_t ip6s_sources_skip_expensive_secondary_if;

	/* pkt dropped, no mbufs for control data */
	u_quad_t ip6s_pktdropcntrl;

	/* total packets trimmed/adjusted  */
	u_quad_t ip6s_adj;
	/* hwcksum info discarded during adjustment */
	u_quad_t ip6s_adj_hwcsum_clr;

	/* duplicate address detection collisions */
	u_quad_t ip6s_dad_collide;

	/* DAD NS looped back */
	u_quad_t ip6s_dad_loopcount;

	/* NECP policy related drop */
	u_quad_t ip6s_necp_policy_drop;

	/* CLAT46 stats */
	u_quad_t ip6s_clat464_in_tooshort_drop;
	u_quad_t ip6s_clat464_in_nov6addr_drop;
	u_quad_t ip6s_clat464_in_nov4addr_drop;
	u_quad_t ip6s_clat464_in_v4synthfail_drop;
	u_quad_t ip6s_clat464_in_64transfail_drop;
	u_quad_t ip6s_clat464_in_64proto_transfail_drop;
	u_quad_t ip6s_clat464_in_64frag_transfail_drop;
	u_quad_t ip6s_clat464_in_invalpbuf_drop;
	u_quad_t ip6s_clat464_in_success;
	u_quad_t ip6s_clat464_in_drop;
	u_quad_t ip6s_clat464_in_v4_drop;

	u_quad_t ip6s_clat464_out_nov6addr_drop;
	u_quad_t ip6s_clat464_out_v6synthfail_drop;
	u_quad_t ip6s_clat464_out_46transfail_drop;
	u_quad_t ip6s_clat464_out_46proto_transfail_drop;
	u_quad_t ip6s_clat464_out_46frag_transfail_drop;
	u_quad_t ip6s_clat464_out_invalpbuf_drop;
	u_quad_t ip6s_clat464_out_success;
	u_quad_t ip6s_clat464_out_drop;

	u_quad_t ip6s_clat464_v6addr_conffail;
	u_quad_t ip6s_clat464_plat64_pfx_setfail;
	u_quad_t ip6s_clat464_plat64_pfx_getfail;

	u_quad_t ip6s_rcv_if_weak_match;
	u_quad_t ip6s_rcv_if_no_match;
};

enum ip6s_sources_rule_index {
	IP6S_SRCRULE_0, IP6S_SRCRULE_1, IP6S_SRCRULE_2, IP6S_SRCRULE_3, IP6S_SRCRULE_4,
	IP6S_SRCRULE_5, IP6S_SRCRULE_6, IP6S_SRCRULE_7,
	IP6S_SRCRULE_7x, IP6S_SRCRULE_8
};

#endif /* !_NETINET6_IP6_VAR_H_ */
