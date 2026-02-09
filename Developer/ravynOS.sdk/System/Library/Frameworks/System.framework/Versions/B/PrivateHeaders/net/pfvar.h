/*
 * Copyright (c) 2007-2019 Apple Inc. All rights reserved.
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

/*	$apfw: git commit b6bf13f8321283cd7ee82b1795e86506084b1b95 $ */
/*	$OpenBSD: pfvar.h,v 1.259 2007/12/02 12:08:04 pascoe Exp $ */

/*
 * Copyright (c) 2001 Daniel Hartmeier
 * NAT64 - Copyright (c) 2010 Viagenie Inc. (http://www.viagenie.ca)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _NET_PFVAR_H_
#define _NET_PFVAR_H_

/*
 * XXX
 * XXX Private interfaces.  Do not include this file; use pfctl(8) instead.
 * XXX
 */

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <libkern/tree.h>

#include <net/radix.h>
#include <netinet/in.h>
#include <net/if_var.h>

union sockaddr_union {
	struct sockaddr         sa;
	struct sockaddr_in      sin;
	struct sockaddr_in6     sin6;
};

#define PF_TCPS_PROXY_SRC       ((TCP_NSTATES)+0)
#define PF_TCPS_PROXY_DST       ((TCP_NSTATES)+1)

#define PF_MD5_DIGEST_LENGTH    16
#ifdef MD5_DIGEST_LENGTH
#if PF_MD5_DIGEST_LENGTH != MD5_DIGEST_LENGTH
#error
#endif /* PF_MD5_DIGEST_LENGTH != MD5_DIGEST_LENGTH */
#endif /* MD5_DIGEST_LENGTH */


#define PF_GRE_PPTP_VARIANT     0x01

enum    { PF_INOUT, PF_IN, PF_OUT };
enum    { PF_PASS, PF_DROP, PF_SCRUB, PF_NOSCRUB, PF_NAT, PF_NONAT,
	  PF_BINAT, PF_NOBINAT, PF_RDR, PF_NORDR, PF_SYNPROXY_DROP,
	  PF_DUMMYNET, PF_NODUMMYNET, PF_NAT64, PF_NONAT64 };
enum    { PF_RULESET_SCRUB, PF_RULESET_FILTER, PF_RULESET_NAT,
	  PF_RULESET_BINAT, PF_RULESET_RDR, PF_RULESET_DUMMYNET,
	  PF_RULESET_MAX };
enum    { PF_OP_NONE, PF_OP_IRG, PF_OP_EQ, PF_OP_NE, PF_OP_LT,
	  PF_OP_LE, PF_OP_GT, PF_OP_GE, PF_OP_XRG, PF_OP_RRG };
enum    { PF_DEBUG_NONE, PF_DEBUG_URGENT, PF_DEBUG_MISC, PF_DEBUG_NOISY };
enum    { PF_CHANGE_NONE, PF_CHANGE_ADD_HEAD, PF_CHANGE_ADD_TAIL,
	  PF_CHANGE_ADD_BEFORE, PF_CHANGE_ADD_AFTER,
	  PF_CHANGE_REMOVE, PF_CHANGE_GET_TICKET };
enum    { PF_GET_NONE, PF_GET_CLR_CNTR };

/*
 * Note about PFTM_*: real indices into pf_rule.timeout[] come before
 * PFTM_MAX, special cases afterwards. See pf_state_expires().
 */
enum    { PFTM_TCP_FIRST_PACKET, PFTM_TCP_OPENING, PFTM_TCP_ESTABLISHED,
	  PFTM_TCP_CLOSING, PFTM_TCP_FIN_WAIT, PFTM_TCP_CLOSED,
	  PFTM_UDP_FIRST_PACKET, PFTM_UDP_SINGLE, PFTM_UDP_MULTIPLE,
	  PFTM_ICMP_FIRST_PACKET, PFTM_ICMP_ERROR_REPLY,
	  PFTM_GREv1_FIRST_PACKET, PFTM_GREv1_INITIATING,
	  PFTM_GREv1_ESTABLISHED, PFTM_ESP_FIRST_PACKET, PFTM_ESP_INITIATING,
	  PFTM_ESP_ESTABLISHED, PFTM_OTHER_FIRST_PACKET, PFTM_OTHER_SINGLE,
	  PFTM_OTHER_MULTIPLE, PFTM_FRAG, PFTM_INTERVAL,
	  PFTM_ADAPTIVE_START, PFTM_ADAPTIVE_END, PFTM_SRC_NODE,
	  PFTM_TS_DIFF, PFTM_MAX, PFTM_PURGE, PFTM_UNLINKED };

/* PFTM default values */
#define PFTM_TCP_FIRST_PACKET_VAL       120     /* First TCP packet */
#define PFTM_TCP_OPENING_VAL            30      /* No response yet */
#define PFTM_TCP_ESTABLISHED_VAL        (24 * 60 * 60)  /* Established */
#define PFTM_TCP_CLOSING_VAL            (15 * 60)       /* Half closed */
#define PFTM_TCP_FIN_WAIT_VAL           45      /* Got both FINs */
#define PFTM_TCP_CLOSED_VAL             90      /* Got a RST */
#define PFTM_UDP_FIRST_PACKET_VAL       60      /* First UDP packet */
#define PFTM_UDP_SINGLE_VAL             30      /* Unidirectional */
#define PFTM_UDP_MULTIPLE_VAL           60      /* Bidirectional */
#define PFTM_ICMP_FIRST_PACKET_VAL      20      /* First ICMP packet */
#define PFTM_ICMP_ERROR_REPLY_VAL       10      /* Got error response */
#define PFTM_GREv1_FIRST_PACKET_VAL     120
#define PFTM_GREv1_INITIATING_VAL       30
#define PFTM_GREv1_ESTABLISHED_VAL      1800
#define PFTM_ESP_FIRST_PACKET_VAL       120
#define PFTM_ESP_INITIATING_VAL         30
#define PFTM_ESP_ESTABLISHED_VAL        900
#define PFTM_OTHER_FIRST_PACKET_VAL     60      /* First packet */
#define PFTM_OTHER_SINGLE_VAL           30      /* Unidirectional */
#define PFTM_OTHER_MULTIPLE_VAL         60      /* Bidirectional */
#define PFTM_FRAG_VAL                   30      /* Fragment expire */
#define PFTM_INTERVAL_VAL               10      /* Expire interval */
#define PFTM_SRC_NODE_VAL               0       /* Source tracking */
#define PFTM_TS_DIFF_VAL                30      /* Allowed TS diff */

enum    { PF_NOPFROUTE, PF_FASTROUTE, PF_ROUTETO, PF_DUPTO, PF_REPLYTO };
enum    { PF_LIMIT_STATES,
	  PF_LIMIT_APP_STATES,
	  PF_LIMIT_SRC_NODES, PF_LIMIT_FRAGS,
	  PF_LIMIT_TABLES, PF_LIMIT_TABLE_ENTRIES, PF_LIMIT_MAX };
#define PF_POOL_IDMASK          0x0f
enum    { PF_POOL_NONE, PF_POOL_BITMASK, PF_POOL_RANDOM,
	  PF_POOL_SRCHASH, PF_POOL_ROUNDROBIN };
enum    { PF_ADDR_ADDRMASK, PF_ADDR_NOROUTE, PF_ADDR_DYNIFTL,
	  PF_ADDR_TABLE, PF_ADDR_RTLABEL, PF_ADDR_URPFFAILED,
	  PF_ADDR_RANGE };
#define PF_POOL_TYPEMASK        0x0f
#define PF_POOL_STICKYADDR      0x20
#define PF_WSCALE_FLAG          0x80
#define PF_WSCALE_MASK          0x0f

#define PF_LOG                  0x01
#define PF_LOG_ALL              0x02
#define PF_LOG_SOCKET_LOOKUP    0x04

struct pf_addr {
	union {
		struct in_addr          _v4addr;
		struct in6_addr         _v6addr;
		u_int8_t                _addr8[16];
		u_int16_t               _addr16[8];
		u_int32_t               _addr32[4];
	} pfa;              /* 128-bit address */
#define v4addr  pfa._v4addr
#define v6addr  pfa._v6addr
#define addr8   pfa._addr8
#define addr16  pfa._addr16
#define addr32  pfa._addr32
};

#define PF_TABLE_NAME_SIZE       32

#define PFI_AFLAG_NETWORK       0x01
#define PFI_AFLAG_BROADCAST     0x02
#define PFI_AFLAG_PEER          0x04
#define PFI_AFLAG_MODEMASK      0x07
#define PFI_AFLAG_NOALIAS       0x08

#ifndef RTLABEL_LEN
#define RTLABEL_LEN 32
#endif

struct pf_addr_wrap {
	union {
		struct {
			struct pf_addr           addr;
			struct pf_addr           mask;
		}                        a;
		char                     ifname[IFNAMSIZ];
		char                     tblname[PF_TABLE_NAME_SIZE];
		char                     rtlabelname[RTLABEL_LEN];
		u_int32_t                rtlabel;
	}                        v;
	union {
		void                    *dyn    __attribute__((aligned(8)));
		void                    *tbl    __attribute__((aligned(8)));
		int                      dyncnt __attribute__((aligned(8)));
		int                      tblcnt __attribute__((aligned(8)));
	}                        p __attribute__((aligned(8)));
	u_int8_t                 type;          /* PF_ADDR_* */
	u_int8_t                 iflags;        /* PFI_AFLAG_* */
};

struct pf_port_range {
	u_int16_t                       port[2];
	u_int8_t                        op;
};

union pf_rule_xport {
	struct pf_port_range    range;
	u_int16_t               call_id;
	u_int32_t               spi;
};


#define PF_INET_INET6


/* Both IPv4 and IPv6 */
#ifdef PF_INET_INET6

#define PF_AEQ(a, b, c) \
	((c == AF_INET && (a)->addr32[0] == (b)->addr32[0]) || \
	((a)->addr32[3] == (b)->addr32[3] && \
	(a)->addr32[2] == (b)->addr32[2] && \
	(a)->addr32[1] == (b)->addr32[1] && \
	(a)->addr32[0] == (b)->addr32[0])) \

#define PF_ANEQ(a, b, c) \
	((c == AF_INET && (a)->addr32[0] != (b)->addr32[0]) || \
	((a)->addr32[3] != (b)->addr32[3] || \
	(a)->addr32[2] != (b)->addr32[2] || \
	(a)->addr32[1] != (b)->addr32[1] || \
	(a)->addr32[0] != (b)->addr32[0])) \

#define PF_ALEQ(a, b, c) \
	((c == AF_INET && (a)->addr32[0] <= (b)->addr32[0]) || \
	((a)->addr32[3] <= (b)->addr32[3] && \
	(a)->addr32[2] <= (b)->addr32[2] && \
	(a)->addr32[1] <= (b)->addr32[1] && \
	(a)->addr32[0] <= (b)->addr32[0])) \

#define PF_AZERO(a, c) \
	((c == AF_INET && !(a)->addr32[0]) || \
	(!(a)->addr32[0] && !(a)->addr32[1] && \
	!(a)->addr32[2] && !(a)->addr32[3])) \

#define PF_MATCHA(n, a, m, b, f) \
	pf_match_addr(n, a, m, b, f)

#define PF_ACPY(a, b, f) \
	pf_addrcpy(a, b, f)

#define PF_AINC(a, f) \
	pf_addr_inc(a, f)

#define PF_POOLMASK(a, b, c, d, f) \
	pf_poolmask(a, b, c, d, f)

#else

/* Just IPv6 */

#ifdef PF_INET6_ONLY

#define PF_AEQ(a, b, c) \
	((a)->addr32[3] == (b)->addr32[3] && \
	(a)->addr32[2] == (b)->addr32[2] && \
	(a)->addr32[1] == (b)->addr32[1] && \
	(a)->addr32[0] == (b)->addr32[0]) \

#define PF_ANEQ(a, b, c) \
	((a)->addr32[3] != (b)->addr32[3] || \
	(a)->addr32[2] != (b)->addr32[2] || \
	(a)->addr32[1] != (b)->addr32[1] || \
	(a)->addr32[0] != (b)->addr32[0]) \

#define PF_ALEQ(a, b, c) \
	((a)->addr32[3] <= (b)->addr32[3] && \
	(a)->addr32[2] <= (b)->addr32[2] && \
	(a)->addr32[1] <= (b)->addr32[1] && \
	(a)->addr32[0] <= (b)->addr32[0]) \

#define PF_AZERO(a, c) \
	(!(a)->addr32[0] && \
	!(a)->addr32[1] && \
	!(a)->addr32[2] && \
	!(a)->addr32[3]) \

#define PF_MATCHA(n, a, m, b, f) \
	pf_match_addr(n, a, m, b, f)

#define PF_ACPY(a, b, f) \
	pf_addrcpy(a, b, f)

#define PF_AINC(a, f) \
	pf_addr_inc(a, f)

#define PF_POOLMASK(a, b, c, d, f) \
	pf_poolmask(a, b, c, d, f)

#else

/* Just IPv4 */
#ifdef PF_INET_ONLY

#define PF_AEQ(a, b, c) \
	((a)->addr32[0] == (b)->addr32[0])

#define PF_ANEQ(a, b, c) \
	((a)->addr32[0] != (b)->addr32[0])

#define PF_ALEQ(a, b, c) \
	((a)->addr32[0] <= (b)->addr32[0])

#define PF_AZERO(a, c) \
	(!(a)->addr32[0])

#define PF_MATCHA(n, a, m, b, f) \
	pf_match_addr(n, a, m, b, f)

#define PF_ACPY(a, b, f) \
	(a)->v4.s_addr = (b)->v4.s_addr

#define PF_AINC(a, f) \
	do { \
	        (a)->addr32[0] = htonl(ntohl((a)->addr32[0]) + 1); \
	} while (0)

#define PF_POOLMASK(a, b, c, d, f) \
	do { \
	        (a)->addr32[0] = ((b)->addr32[0] & (c)->addr32[0]) | \
	        (((c)->addr32[0] ^ 0xffffffff) & (d)->addr32[0]); \
	} while (0)

#endif /* PF_INET_ONLY */
#endif /* PF_INET6_ONLY */
#endif /* PF_INET_INET6 */


struct pf_rule_uid {
	uid_t            uid[2];
	u_int8_t         op;
	u_int8_t         _pad[3];
};

struct pf_rule_gid {
	uid_t            gid[2];
	u_int8_t         op;
	u_int8_t         _pad[3];
};

struct pf_rule_addr {
	struct pf_addr_wrap      addr;
	union pf_rule_xport      xport;
	u_int8_t                 neg;
};

struct pf_pooladdr {
	struct pf_addr_wrap              addr;
	TAILQ_ENTRY(pf_pooladdr)         entries;
#if !defined(__LP64__)
	u_int32_t                        _pad[2];
#endif /* !__LP64__ */
	char                             ifname[IFNAMSIZ];
	void                            *kif    __attribute__((aligned(8)));
};

TAILQ_HEAD(pf_palist, pf_pooladdr);

struct pf_poolhashkey {
	union {
		u_int8_t                key8[16];
		u_int16_t               key16[8];
		u_int32_t               key32[4];
	} pfk;              /* 128-bit hash key */
#define key8    pfk.key8
#define key16   pfk.key16
#define key32   pfk.key32
};

struct pf_pool {
	struct pf_palist         list;
#if !defined(__LP64__)
	u_int32_t                _pad[2];
#endif /* !__LP64__ */
	void                    *cur            __attribute__((aligned(8)));
	struct pf_poolhashkey    key            __attribute__((aligned(8)));
	struct pf_addr           counter;
	int                      tblidx;
	u_int16_t                proxy_port[2];
	u_int8_t                 port_op;
	u_int8_t                 opts;
	sa_family_t              af;
};


/* A packed Operating System description for fingerprinting */
typedef u_int32_t pf_osfp_t;
#define PF_OSFP_ANY     ((pf_osfp_t)0)
#define PF_OSFP_UNKNOWN ((pf_osfp_t)-1)
#define PF_OSFP_NOMATCH ((pf_osfp_t)-2)

struct pf_osfp_entry {
	SLIST_ENTRY(pf_osfp_entry) fp_entry;
#if !defined(__LP64__)
	u_int32_t               _pad;
#endif /* !__LP64__ */
	pf_osfp_t               fp_os;
	int                     fp_enflags;
#define PF_OSFP_EXPANDED        0x001           /* expanded entry */
#define PF_OSFP_GENERIC         0x002           /* generic signature */
#define PF_OSFP_NODETAIL        0x004           /* no p0f details */
#define PF_OSFP_LEN     32
	char                    fp_class_nm[PF_OSFP_LEN];
	char                    fp_version_nm[PF_OSFP_LEN];
	char                    fp_subtype_nm[PF_OSFP_LEN];
};
#define PF_OSFP_ENTRY_EQ(a, b) \
    ((a)->fp_os == (b)->fp_os && \
    memcmp((a)->fp_class_nm, (b)->fp_class_nm, PF_OSFP_LEN) == 0 && \
    memcmp((a)->fp_version_nm, (b)->fp_version_nm, PF_OSFP_LEN) == 0 && \
    memcmp((a)->fp_subtype_nm, (b)->fp_subtype_nm, PF_OSFP_LEN) == 0)

/* handle pf_osfp_t packing */
#define _FP_RESERVED_BIT        1  /* For the special negative #defines */
#define _FP_UNUSED_BITS         1
#define _FP_CLASS_BITS          10 /* OS Class (Windows, Linux) */
#define _FP_VERSION_BITS        10 /* OS version (95, 98, NT, 2.4.54, 3.2) */
#define _FP_SUBTYPE_BITS        10 /* patch level (NT SP4, SP3, ECN patch) */
#define PF_OSFP_UNPACK(osfp, class, version, subtype) do { \
	(class) = ((osfp) >> (_FP_VERSION_BITS+_FP_SUBTYPE_BITS)) & \
	    ((1 << _FP_CLASS_BITS) - 1); \
	(version) = ((osfp) >> _FP_SUBTYPE_BITS) & \
	    ((1 << _FP_VERSION_BITS) - 1);\
	(subtype) = (osfp) & ((1 << _FP_SUBTYPE_BITS) - 1); \
} while (0)
#define PF_OSFP_PACK(osfp, class, version, subtype) do { \
	(osfp) = ((class) & ((1 << _FP_CLASS_BITS) - 1)) << (_FP_VERSION_BITS \
	    + _FP_SUBTYPE_BITS); \
	(osfp) |= ((version) & ((1 << _FP_VERSION_BITS) - 1)) << \
	    _FP_SUBTYPE_BITS; \
	(osfp) |= (subtype) & ((1 << _FP_SUBTYPE_BITS) - 1); \
} while (0)

/* the fingerprint of an OSes TCP SYN packet */
typedef u_int64_t       pf_tcpopts_t;
struct pf_os_fingerprint {
	SLIST_HEAD(pf_osfp_enlist, pf_osfp_entry) fp_oses; /* list of matches */
	pf_tcpopts_t            fp_tcpopts;     /* packed TCP options */
	u_int16_t               fp_wsize;       /* TCP window size */
	u_int16_t               fp_psize;       /* ip->ip_len */
	u_int16_t               fp_mss;         /* TCP MSS */
	u_int16_t               fp_flags;
#define PF_OSFP_WSIZE_MOD       0x0001          /* Window modulus */
#define PF_OSFP_WSIZE_DC        0x0002          /* Window don't care */
#define PF_OSFP_WSIZE_MSS       0x0004          /* Window multiple of MSS */
#define PF_OSFP_WSIZE_MTU       0x0008          /* Window multiple of MTU */
#define PF_OSFP_PSIZE_MOD       0x0010          /* packet size modulus */
#define PF_OSFP_PSIZE_DC        0x0020          /* packet size don't care */
#define PF_OSFP_WSCALE          0x0040          /* TCP window scaling */
#define PF_OSFP_WSCALE_MOD      0x0080          /* TCP window scale modulus */
#define PF_OSFP_WSCALE_DC       0x0100          /* TCP window scale dont-care */
#define PF_OSFP_MSS             0x0200          /* TCP MSS */
#define PF_OSFP_MSS_MOD         0x0400          /* TCP MSS modulus */
#define PF_OSFP_MSS_DC          0x0800          /* TCP MSS dont-care */
#define PF_OSFP_DF              0x1000          /* IPv4 don't fragment bit */
#define PF_OSFP_TS0             0x2000          /* Zero timestamp */
#define PF_OSFP_INET6           0x4000          /* IPv6 */
	u_int8_t                fp_optcnt;      /* TCP option count */
	u_int8_t                fp_wscale;      /* TCP window scaling */
	u_int8_t                fp_ttl;         /* IPv4 TTL */
#define PF_OSFP_MAXTTL_OFFSET   40
/* TCP options packing */
#define PF_OSFP_TCPOPT_NOP      0x0             /* TCP NOP option */
#define PF_OSFP_TCPOPT_WSCALE   0x1             /* TCP window scaling option */
#define PF_OSFP_TCPOPT_MSS      0x2             /* TCP max segment size opt */
#define PF_OSFP_TCPOPT_SACK     0x3             /* TCP SACK OK option */
#define PF_OSFP_TCPOPT_TS       0x4             /* TCP timestamp option */
#define PF_OSFP_TCPOPT_BITS     3               /* bits used by each option */
#define PF_OSFP_MAX_OPTS \
    ((sizeof(pf_tcpopts_t) * 8) \
    / PF_OSFP_TCPOPT_BITS)

	SLIST_ENTRY(pf_os_fingerprint)  fp_next;
};

struct pf_osfp_ioctl {
	struct pf_osfp_entry    fp_os;
	pf_tcpopts_t            fp_tcpopts;     /* packed TCP options */
	u_int16_t               fp_wsize;       /* TCP window size */
	u_int16_t               fp_psize;       /* ip->ip_len */
	u_int16_t               fp_mss;         /* TCP MSS */
	u_int16_t               fp_flags;
	u_int8_t                fp_optcnt;      /* TCP option count */
	u_int8_t                fp_wscale;      /* TCP window scaling */
	u_int8_t                fp_ttl;         /* IPv4 TTL */

	int                     fp_getnum;      /* DIOCOSFPGET number */
};


union pf_rule_ptr {
	struct pf_rule          *ptr            __attribute__((aligned(8)));
	u_int32_t                nr             __attribute__((aligned(8)));
} __attribute__((aligned(8)));

#define PF_ANCHOR_NAME_SIZE      64

struct pf_rule {
	struct pf_rule_addr      src;
	struct pf_rule_addr      dst;
#define PF_SKIP_IFP             0
#define PF_SKIP_DIR             1
#define PF_SKIP_AF              2
#define PF_SKIP_PROTO           3
#define PF_SKIP_SRC_ADDR        4
#define PF_SKIP_SRC_PORT        5
#define PF_SKIP_DST_ADDR        6
#define PF_SKIP_DST_PORT        7
#define PF_SKIP_COUNT           8
	union pf_rule_ptr        skip[PF_SKIP_COUNT];
#define PF_RULE_LABEL_SIZE       64
	char                     label[PF_RULE_LABEL_SIZE];
#define PF_QNAME_SIZE            64
	char                     ifname[IFNAMSIZ];
	char                     qname[PF_QNAME_SIZE];
	char                     pqname[PF_QNAME_SIZE];
#define PF_TAG_NAME_SIZE         64
	char                     tagname[PF_TAG_NAME_SIZE];
	char                     match_tagname[PF_TAG_NAME_SIZE];

	char                     overload_tblname[PF_TABLE_NAME_SIZE];

	TAILQ_ENTRY(pf_rule)     entries;
#if !defined(__LP64__)
	u_int32_t                _pad[2];
#endif /* !__LP64__ */
	struct pf_pool           rpool;

	u_int64_t                evaluations;
	u_int64_t                packets[2];
	u_int64_t                bytes[2];

	u_int64_t                ticket;
#define PF_OWNER_NAME_SIZE       64
	char                     owner[PF_OWNER_NAME_SIZE];
	u_int32_t                priority;

	void                    *kif            __attribute__((aligned(8)));
	struct pf_anchor        *anchor         __attribute__((aligned(8)));
	void                    *overload_tbl   __attribute__((aligned(8)));

	pf_osfp_t                os_fingerprint __attribute__((aligned(8)));

	unsigned int             rtableid;
	u_int32_t                timeout[PFTM_MAX];
	u_int32_t                states;
	u_int32_t                max_states;
	u_int32_t                src_nodes;
	u_int32_t                max_src_nodes;
	u_int32_t                max_src_states;
	u_int32_t                max_src_conn;
	struct {
		u_int32_t               limit;
		u_int32_t               seconds;
	}                        max_src_conn_rate;
	u_int32_t                qid;
	u_int32_t                pqid;
	u_int32_t                rt_listid;
	u_int32_t                nr;
	u_int32_t                prob;
	uid_t                    cuid;
	pid_t                    cpid;

	u_int16_t                return_icmp;
	u_int16_t                return_icmp6;
	u_int16_t                max_mss;
	u_int16_t                tag;
	u_int16_t                match_tag;

	struct pf_rule_uid       uid;
	struct pf_rule_gid       gid;

	u_int32_t                rule_flag;
	u_int8_t                 action;
	u_int8_t                 direction;
	u_int8_t                 log;
	u_int8_t                 logif;
	u_int8_t                 quick;
	u_int8_t                 ifnot;
	u_int8_t                 match_tag_not;
	u_int8_t                 natpass;

#define PF_STATE_NORMAL         0x1
#define PF_STATE_MODULATE       0x2
#define PF_STATE_SYNPROXY       0x3
	u_int8_t                 keep_state;
	sa_family_t              af;
	u_int8_t                 proto;
	u_int8_t                 type;
	u_int8_t                 code;
	u_int8_t                 flags;
	u_int8_t                 flagset;
	u_int8_t                 min_ttl;
	u_int8_t                 allow_opts;
	u_int8_t                 rt;
	u_int8_t                 return_ttl;

/* service class categories */
#define SCIDX_MASK              0x0f
#define SC_BE                   0x10
#define SC_BK_SYS               0x11
#define SC_BK                   0x12
#define SC_RD                   0x13
#define SC_OAM                  0x14
#define SC_AV                   0x15
#define SC_RV                   0x16
#define SC_VI                   0x17
#define SC_SIG                  0x17
#define SC_VO                   0x18
#define SC_CTL                  0x19

/* diffserve code points */
#define DSCP_MASK               0xfc
#define DSCP_CUMASK             0x03
#define DSCP_EF                 0xb8
#define DSCP_AF11               0x28
#define DSCP_AF12               0x30
#define DSCP_AF13               0x38
#define DSCP_AF21               0x48
#define DSCP_AF22               0x50
#define DSCP_AF23               0x58
#define DSCP_AF31               0x68
#define DSCP_AF32               0x70
#define DSCP_AF33               0x78
#define DSCP_AF41               0x88
#define DSCP_AF42               0x90
#define DSCP_AF43               0x98
#define AF_CLASSMASK            0xe0
#define AF_DROPPRECMASK         0x18
	u_int8_t                 tos;
	u_int8_t                 anchor_relative;
	u_int8_t                 anchor_wildcard;

#define PF_FLUSH                0x01
#define PF_FLUSH_GLOBAL         0x02
	u_int8_t                 flush;

	u_int8_t                proto_variant;
	u_int8_t                extfilter; /* Filter mode [PF_EXTFILTER_xxx] */
	u_int8_t                extmap;    /* Mapping mode [PF_EXTMAP_xxx] */
	u_int32_t               dnpipe;
	u_int32_t               dntype;
};

/* pf device identifiers */
#define PFDEV_PF                0
#define PFDEV_PFM               1
#define PFDEV_MAX               2

/* rule flags */
#define PFRULE_DROP             0x0000
#define PFRULE_RETURNRST        0x0001
#define PFRULE_FRAGMENT         0x0002
#define PFRULE_RETURNICMP       0x0004
#define PFRULE_RETURN           0x0008
#define PFRULE_NOSYNC           0x0010
#define PFRULE_SRCTRACK         0x0020  /* track source states */
#define PFRULE_RULESRCTRACK     0x0040  /* per rule */

/* scrub flags */
#define PFRULE_NODF             0x0100
#define PFRULE_FRAGCROP         0x0200  /* non-buffering frag cache */
#define PFRULE_FRAGDROP         0x0400  /* drop funny fragments */
#define PFRULE_RANDOMID         0x0800
#define PFRULE_REASSEMBLE_TCP   0x1000

/* rule flags for TOS/DSCP/service class differentiation */
#define PFRULE_TOS              0x2000
#define PFRULE_DSCP             0x4000
#define PFRULE_SC               0x8000

/* rule flags again */
#define PFRULE_IFBOUND          0x00010000      /* if-bound */
#define PFRULE_PFM              0x00020000      /* created by pfm device */

#define PFSTATE_HIWAT           10000   /* default state table size */
#define PFSTATE_ADAPT_START     6000    /* default adaptive timeout start */
#define PFSTATE_ADAPT_END       12000   /* default adaptive timeout end */

#define PFAPPSTATE_HIWAT        10000   /* default same as state table */

enum pf_extmap {
	PF_EXTMAP_APD   = 1,    /* Address-port-dependent mapping */
	PF_EXTMAP_AD,           /* Address-dependent mapping */
	PF_EXTMAP_EI            /* Endpoint-independent mapping */
};

enum pf_extfilter {
	PF_EXTFILTER_APD = 1,   /* Address-port-dependent filtering */
	PF_EXTFILTER_AD,        /* Address-dependent filtering */
	PF_EXTFILTER_EI         /* Endpoint-independent filtering */
};

struct pf_threshold {
	u_int32_t       limit;
#define PF_THRESHOLD_MULT       1000
#define PF_THRESHOLD_MAX        0xffffffff / PF_THRESHOLD_MULT
	u_int32_t       seconds;
	u_int32_t       count;
	u_int32_t       last;
};

struct pf_src_node {
	RB_ENTRY(pf_src_node) entry;
	struct pf_addr   addr;
	struct pf_addr   raddr;
	union pf_rule_ptr rule;
	void            *kif;
	u_int64_t        bytes[2];
	u_int64_t        packets[2];
	u_int32_t        states;
	u_int32_t        conn;
	struct pf_threshold     conn_rate;
	u_int64_t        creation;
	u_int64_t        expire;
	sa_family_t      af;
	u_int8_t         ruletype;
};

#define PFSNODE_HIWAT           10000   /* default source node table size */


union pf_state_xport {
	u_int16_t       port;
	u_int16_t       call_id;
	u_int32_t       spi;
};

struct pf_state_host {
	struct pf_addr          addr;
	union pf_state_xport    xport;
};


struct hook_desc;
TAILQ_HEAD(hook_desc_head, hook_desc);


#define PFSTATE_NOSYNC   0x01
#define PFSTATE_FROMSYNC 0x02
#define PFSTATE_STALE    0x04

#define __packed        __attribute__((__packed__))

/*
 * Unified state structures for pulling states out of the kernel
 * used by pfsync(4) and the pf(4) ioctl.
 */
struct pfsync_state_scrub {
	u_int16_t       pfss_flags;
	u_int8_t        pfss_ttl;       /* stashed TTL		*/
#define PFSYNC_SCRUB_FLAG_VALID         0x01
	u_int8_t        scrub_flag;
	u_int32_t       pfss_ts_mod;    /* timestamp modulation	*/
} __packed;

struct pfsync_state_host {
	struct pf_addr          addr;
	union pf_state_xport    xport;
	u_int16_t               pad[2];
} __packed;

struct pfsync_state_peer {
	struct pfsync_state_scrub scrub;        /* state is scrubbed	*/
	u_int32_t       seqlo;          /* Max sequence number sent	*/
	u_int32_t       seqhi;          /* Max the other end ACKd + win	*/
	u_int32_t       seqdiff;        /* Sequence number modulator	*/
	u_int16_t       max_win;        /* largest window (pre scaling)	*/
	u_int16_t       mss;            /* Maximum segment size option	*/
	u_int8_t        state;          /* active state level		*/
	u_int8_t        wscale;         /* window scaling factor	*/
	u_int8_t        pad[6];
} __packed;

struct pfsync_state {
	u_int32_t        id[2];
	char             ifname[IFNAMSIZ];
	struct pfsync_state_host lan;
	struct pfsync_state_host gwy;
	struct pfsync_state_host ext_lan;
	struct pfsync_state_host ext_gwy;
	struct pfsync_state_peer src;
	struct pfsync_state_peer dst;
	struct pf_addr   rt_addr;
	struct hook_desc_head unlink_hooks;
#if !defined(__LP64__)
	u_int32_t       _pad[2];
#endif /* !__LP64__ */
	u_int32_t        rule;
	u_int32_t        anchor;
	u_int32_t        nat_rule;
	u_int64_t        creation;
	u_int64_t        expire;
	u_int32_t        packets[2][2];
	u_int32_t        bytes[2][2];
	u_int32_t        creatorid;
	u_int16_t        tag;
	sa_family_t      af_lan;
	sa_family_t      af_gwy;
	u_int8_t         proto;
	u_int8_t         direction;
	u_int8_t         log;
	u_int8_t         allow_opts;
	u_int8_t         timeout;
	u_int8_t         sync_flags;
	u_int8_t         updates;
	u_int8_t         proto_variant;
	u_int8_t         __pad;
	u_int32_t        flowhash;
} __packed;

#define PFSYNC_FLAG_COMPRESS    0x01
#define PFSYNC_FLAG_STALE       0x02
#define PFSYNC_FLAG_SRCNODE     0x04
#define PFSYNC_FLAG_NATSRCNODE  0x08


#define pf_state_counter_to_pfsync(s, d) do {                   \
	d[0] = (s>>32)&0xffffffff;                              \
	d[1] = s&0xffffffff;                                    \
} while (0)

#define pf_state_counter_from_pfsync(s)         \
	(((u_int64_t)(s[0])<<32) | (u_int64_t)(s[1]))



TAILQ_HEAD(pf_rulequeue, pf_rule);

struct pf_anchor;

struct pf_ruleset {
	struct {
		struct pf_rulequeue      queues[2];
		struct {
			struct pf_rulequeue     *ptr;
			struct pf_rule          **ptr_array;
			u_int32_t                rcount;
			u_int32_t                ticket;
			int                      open;
		}                        active, inactive;
	}                        rules[PF_RULESET_MAX];
	struct pf_anchor        *anchor;
	u_int32_t                tticket;
	int                      tables;
	int                      topen;
};

RB_HEAD(pf_anchor_global, pf_anchor);
RB_HEAD(pf_anchor_node, pf_anchor);
struct pf_anchor {
	RB_ENTRY(pf_anchor)      entry_global;
	RB_ENTRY(pf_anchor)      entry_node;
	struct pf_anchor        *parent;
	struct pf_anchor_node    children;
	char                     name[PF_ANCHOR_NAME_SIZE];
	char                     path[MAXPATHLEN];
	struct pf_ruleset        ruleset;
	int                      refcnt;        /* anchor rules */
	int                      match;
	char                     owner[PF_OWNER_NAME_SIZE];
};
RB_PROTOTYPE(pf_anchor_global, pf_anchor, entry_global, pf_anchor_compare);
RB_PROTOTYPE(pf_anchor_node, pf_anchor, entry_node, pf_anchor_compare);

#define PF_RESERVED_ANCHOR      "_pf"

#define PFR_TFLAG_PERSIST       0x00000001
#define PFR_TFLAG_CONST         0x00000002
#define PFR_TFLAG_ACTIVE        0x00000004
#define PFR_TFLAG_INACTIVE      0x00000008
#define PFR_TFLAG_REFERENCED    0x00000010
#define PFR_TFLAG_REFDANCHOR    0x00000020
#define PFR_TFLAG_USRMASK       0x00000003
#define PFR_TFLAG_SETMASK       0x0000003C
#define PFR_TFLAG_ALLMASK       0x0000003F

struct pfr_table {
	char                     pfrt_anchor[MAXPATHLEN];
	char                     pfrt_name[PF_TABLE_NAME_SIZE];
	u_int32_t                pfrt_flags;
	u_int8_t                 pfrt_fback;
};

enum { PFR_FB_NONE, PFR_FB_MATCH, PFR_FB_ADDED, PFR_FB_DELETED,
       PFR_FB_CHANGED, PFR_FB_CLEARED, PFR_FB_DUPLICATE,
       PFR_FB_NOTMATCH, PFR_FB_CONFLICT, PFR_FB_MAX };

struct pfr_addr {
	union {
		struct in_addr   _pfra_ip4addr;
		struct in6_addr  _pfra_ip6addr;
	}                pfra_u;
	u_int8_t         pfra_af;
	u_int8_t         pfra_net;
	u_int8_t         pfra_not;
	u_int8_t         pfra_fback;
};
#define pfra_ip4addr    pfra_u._pfra_ip4addr
#define pfra_ip6addr    pfra_u._pfra_ip6addr

enum { PFR_DIR_IN, PFR_DIR_OUT, PFR_DIR_MAX };
enum { PFR_OP_BLOCK, PFR_OP_PASS, PFR_OP_ADDR_MAX, PFR_OP_TABLE_MAX };
#define PFR_OP_XPASS    PFR_OP_ADDR_MAX

struct pfr_astats {
	struct pfr_addr  pfras_a;
#if !defined(__LP64__)
	u_int32_t        _pad;
#endif /* !__LP64__ */
	u_int64_t        pfras_packets[PFR_DIR_MAX][PFR_OP_ADDR_MAX];
	u_int64_t        pfras_bytes[PFR_DIR_MAX][PFR_OP_ADDR_MAX];
	u_int64_t        pfras_tzero;
};

enum { PFR_REFCNT_RULE, PFR_REFCNT_ANCHOR, PFR_REFCNT_MAX };

struct pfr_tstats {
	struct pfr_table pfrts_t;
	u_int64_t        pfrts_packets[PFR_DIR_MAX][PFR_OP_TABLE_MAX];
	u_int64_t        pfrts_bytes[PFR_DIR_MAX][PFR_OP_TABLE_MAX];
	u_int64_t        pfrts_match;
	u_int64_t        pfrts_nomatch;
	u_int64_t        pfrts_tzero;
	int              pfrts_cnt;
	int              pfrts_refcnt[PFR_REFCNT_MAX];
#if !defined(__LP64__)
	u_int32_t        _pad;
#endif /* !__LP64__ */
};
#define pfrts_name      pfrts_t.pfrt_name
#define pfrts_flags     pfrts_t.pfrt_flags

struct pfi_kif {
	char                             pfik_name[IFNAMSIZ];
	u_int64_t                        pfik_packets[2][2][2];
	u_int64_t                        pfik_bytes[2][2][2];
	u_int64_t                        pfik_tzero;
	int                              pfik_flags;
	int                              pfik_states;
	int                              pfik_rules;
#if !defined(__LP64__)
	u_int32_t                        _pad;
#endif /* !__LP64__ */
};

#define PFI_IFLAG_SKIP          0x0100  /* skip filtering on interface */


/* flags for RDR options */
#define PF_DPORT_RANGE  0x01            /* Dest port uses range */
#define PF_RPORT_RANGE  0x02            /* RDR'ed port uses range */

/* Reasons code for passing/dropping a packet */
#define PFRES_MATCH     0               /* Explicit match of a rule */
#define PFRES_BADOFF    1               /* Bad offset for pull_hdr */
#define PFRES_FRAG      2               /* Dropping following fragment */
#define PFRES_SHORT     3               /* Dropping short packet */
#define PFRES_NORM      4               /* Dropping by normalizer */
#define PFRES_MEMORY    5               /* Dropped due to lacking mem */
#define PFRES_TS        6               /* Bad TCP Timestamp (RFC1323) */
#define PFRES_CONGEST   7               /* Congestion (of ipintrq) */
#define PFRES_IPOPTIONS 8               /* IP option */
#define PFRES_PROTCKSUM 9               /* Protocol checksum invalid */
#define PFRES_BADSTATE  10              /* State mismatch */
#define PFRES_STATEINS  11              /* State insertion failure */
#define PFRES_MAXSTATES 12              /* State limit */
#define PFRES_SRCLIMIT  13              /* Source node/conn limit */
#define PFRES_SYNPROXY  14              /* SYN proxy */
#define PFRES_DUMMYNET  15              /* Dummynet */
#define PFRES_MAX       16              /* total+1 */

#define PFRES_NAMES { \
	"match", \
	"bad-offset", \
	"fragment", \
	"short", \
	"normalize", \
	"memory", \
	"bad-timestamp", \
	"congestion", \
	"ip-option", \
	"proto-cksum", \
	"state-mismatch", \
	"state-insert", \
	"state-limit", \
	"src-limit", \
	"synproxy", \
	"dummynet", \
	NULL \
}

/* Counters for other things we want to keep track of */
#define LCNT_STATES             0       /* states */
#define LCNT_SRCSTATES          1       /* max-src-states */
#define LCNT_SRCNODES           2       /* max-src-nodes */
#define LCNT_SRCCONN            3       /* max-src-conn */
#define LCNT_SRCCONNRATE        4       /* max-src-conn-rate */
#define LCNT_OVERLOAD_TABLE     5       /* entry added to overload table */
#define LCNT_OVERLOAD_FLUSH     6       /* state entries flushed */
#define LCNT_MAX                7       /* total+1 */

#define LCNT_NAMES { \
	"max states per rule", \
	"max-src-states", \
	"max-src-nodes", \
	"max-src-conn", \
	"max-src-conn-rate", \
	"overload table insertion", \
	"overload flush states", \
	NULL \
}

/* UDP state enumeration */
#define PFUDPS_NO_TRAFFIC       0
#define PFUDPS_SINGLE           1
#define PFUDPS_MULTIPLE         2

#define PFUDPS_NSTATES          3       /* number of state levels */

#define PFUDPS_NAMES { \
	"NO_TRAFFIC", \
	"SINGLE", \
	"MULTIPLE", \
	NULL \
}

/* GREv1 protocol state enumeration */
#define PFGRE1S_NO_TRAFFIC              0
#define PFGRE1S_INITIATING              1
#define PFGRE1S_ESTABLISHED             2

#define PFGRE1S_NSTATES                 3       /* number of state levels */

#define PFGRE1S_NAMES { \
	"NO_TRAFFIC", \
	"INITIATING", \
	"ESTABLISHED", \
	NULL \
}

#define PFESPS_NO_TRAFFIC       0
#define PFESPS_INITIATING       1
#define PFESPS_ESTABLISHED      2

#define PFESPS_NSTATES          3       /* number of state levels */

#define PFESPS_NAMES { "NO_TRAFFIC", "INITIATING", "ESTABLISHED", NULL }

/* Other protocol state enumeration */
#define PFOTHERS_NO_TRAFFIC     0
#define PFOTHERS_SINGLE         1
#define PFOTHERS_MULTIPLE       2

#define PFOTHERS_NSTATES        3       /* number of state levels */

#define PFOTHERS_NAMES { \
	"NO_TRAFFIC", \
	"SINGLE", \
	"MULTIPLE", \
	NULL \
}

#define FCNT_STATE_SEARCH       0
#define FCNT_STATE_INSERT       1
#define FCNT_STATE_REMOVALS     2
#define FCNT_MAX                3

#define SCNT_SRC_NODE_SEARCH    0
#define SCNT_SRC_NODE_INSERT    1
#define SCNT_SRC_NODE_REMOVALS  2
#define SCNT_MAX                3


struct pf_status {
	u_int64_t       counters[PFRES_MAX];
	u_int64_t       lcounters[LCNT_MAX];    /* limit counters */
	u_int64_t       fcounters[FCNT_MAX];
	u_int64_t       scounters[SCNT_MAX];
	u_int64_t       pcounters[2][2][3];
	u_int64_t       bcounters[2][2];
	u_int64_t       stateid;
	u_int32_t       running;
	u_int32_t       states;
	u_int32_t       src_nodes;
	u_int64_t       since                   __attribute__((aligned(8)));
	u_int32_t       debug;
	u_int32_t       hostid;
	char            ifname[IFNAMSIZ];
	u_int8_t        pf_chksum[PF_MD5_DIGEST_LENGTH];
};

struct cbq_opts {
	u_int32_t       minburst;
	u_int32_t       maxburst;
	u_int32_t       pktsize;
	u_int32_t       maxpktsize;
	u_int32_t       ns_per_byte;
	u_int32_t       maxidle;
	int32_t         minidle;
	u_int32_t       offtime;
	u_int32_t       flags;
};

struct priq_opts {
	u_int32_t       flags;
};

struct qfq_opts {
	u_int32_t       flags;
	u_int32_t       lmax;
};

struct hfsc_opts {
	/* real-time service curve */
	u_int64_t       rtsc_m1;        /* slope of the 1st segment in bps */
	u_int64_t       rtsc_d;         /* the x-projection of m1 in msec */
	u_int64_t       rtsc_m2;        /* slope of the 2nd segment in bps */
	u_int32_t       rtsc_fl;        /* service curve flags */
#if !defined(__LP64__)
	u_int32_t       _pad;
#endif /* !__LP64__ */
	/* link-sharing service curve */
	u_int64_t       lssc_m1;
	u_int64_t       lssc_d;
	u_int64_t       lssc_m2;
	u_int32_t       lssc_fl;
#if !defined(__LP64__)
	u_int32_t       __pad;
#endif /* !__LP64__ */
	/* upper-limit service curve */
	u_int64_t       ulsc_m1;
	u_int64_t       ulsc_d;
	u_int64_t       ulsc_m2;
	u_int32_t       ulsc_fl;
	u_int32_t       flags;          /* scheduler flags */
};

struct fairq_opts {
	u_int32_t       nbuckets;       /* hash buckets */
	u_int32_t       flags;
	u_int64_t       hogs_m1;        /* hog detection bandwidth */

	/* link-sharing service curve */
	u_int64_t       lssc_m1;
	u_int64_t       lssc_d;
	u_int64_t       lssc_m2;
};

/* bandwidth types */
#define PF_ALTQ_BW_ABSOLUTE     1       /* bw in absolute value (bps) */
#define PF_ALTQ_BW_PERCENT      2       /* bandwidth in percentage */

/* ALTQ rule flags */
#define PF_ALTQF_TBR            0x1     /* enable Token Bucket Regulator */

/* queue rule flags */
#define PF_ALTQ_QRF_WEIGHT      0x1     /* weight instead of priority */

struct pf_altq {
	char                     ifname[IFNAMSIZ];

	/* discipline-specific state */
	void                    *altq_disc __attribute__((aligned(8)));
	TAILQ_ENTRY(pf_altq)     entries __attribute__((aligned(8)));
#if !defined(__LP64__)
	u_int32_t               _pad[2];
#endif /* !__LP64__ */

	u_int32_t                aflags;        /* ALTQ rule flags */
	u_int32_t                bwtype;        /* bandwidth type */

	/* scheduler spec */
	u_int32_t                scheduler;     /* scheduler type */
	u_int32_t                tbrsize;       /* tokenbucket regulator size */
	u_int64_t                ifbandwidth;   /* interface bandwidth */

	/* queue spec */
	char                     qname[PF_QNAME_SIZE];  /* queue name */
	char                     parent[PF_QNAME_SIZE]; /* parent name */
	u_int32_t                parent_qid;    /* parent queue id */
	u_int32_t                qrflags;       /* queue rule flags */
	union {
		u_int32_t        priority;      /* priority */
		u_int32_t        weight;        /* weight */
	};
	u_int32_t                qlimit;        /* queue size limit */
	u_int32_t                flags;         /* misc flags */
#if !defined(__LP64__)
	u_int32_t               __pad;
#endif /* !__LP64__ */
	u_int64_t                bandwidth;     /* queue bandwidth */
	union {
		struct cbq_opts          cbq_opts;
		struct priq_opts         priq_opts;
		struct hfsc_opts         hfsc_opts;
		struct fairq_opts        fairq_opts;
		struct qfq_opts          qfq_opts;
	} pq_u;

	u_int32_t                qid;           /* return value */
};

struct pf_tagname {
	TAILQ_ENTRY(pf_tagname) entries;
	char                    name[PF_TAG_NAME_SIZE];
	u_int16_t               tag;
	int                     ref;
};

#define PFFRAG_FRENT_HIWAT      5000    /* Number of fragment entries */
#define PFFRAG_FRAG_HIWAT       1000    /* Number of fragmented packets */
#define PFFRAG_FRCENT_HIWAT     50000   /* Number of fragment cache entries */
#define PFFRAG_FRCACHE_HIWAT    10000   /* Number of fragment descriptors */

#define PFR_KTABLE_HIWAT        1000    /* Number of tables */
#define PFR_KENTRY_HIWAT        200000  /* Number of table entries */
#define PFR_KENTRY_HIWAT_SMALL  100000  /* Number of table entries (tiny hosts) */

/*
 * ioctl parameter structures
 */

struct pfioc_pooladdr {
	u_int32_t                action;
	u_int32_t                ticket;
	u_int32_t                nr;
	u_int32_t                r_num;
	u_int8_t                 r_action;
	u_int8_t                 r_last;
	u_int8_t                 af;
	char                     anchor[MAXPATHLEN];
	struct pf_pooladdr       addr;
};

struct pfioc_rule {
	u_int32_t        action;
	u_int32_t        ticket;
	u_int32_t        pool_ticket;
	u_int32_t        nr;
	char             anchor[MAXPATHLEN];
	char             anchor_call[MAXPATHLEN];
	struct pf_rule   rule;
};

struct pfioc_natlook {
	struct pf_addr   saddr;
	struct pf_addr   daddr;
	struct pf_addr   rsaddr;
	struct pf_addr   rdaddr;
	union pf_state_xport    sxport;
	union pf_state_xport    dxport;
	union pf_state_xport    rsxport;
	union pf_state_xport    rdxport;
	sa_family_t      af;
	u_int8_t         proto;
	u_int8_t         proto_variant;
	u_int8_t         direction;
};

struct pfioc_state {
	struct pfsync_state     state;
};

struct pfioc_src_node_kill {
	/* XXX returns the number of src nodes killed in psnk_af */
	sa_family_t psnk_af;
	struct pf_rule_addr psnk_src;
	struct pf_rule_addr psnk_dst;
};

struct pfioc_state_addr_kill {
	struct pf_addr_wrap             addr;
	u_int8_t                        reserved_[3];
	u_int8_t                        neg;
	union pf_rule_xport             xport;
};

struct pfioc_state_kill {
	/* XXX returns the number of states killed in psk_af */
	sa_family_t             psk_af;
	u_int8_t                psk_proto;
	u_int8_t                psk_proto_variant;
	u_int8_t                _pad;
	struct pfioc_state_addr_kill    psk_src;
	struct pfioc_state_addr_kill    psk_dst;
	char                    psk_ifname[IFNAMSIZ];
	char                    psk_ownername[PF_OWNER_NAME_SIZE];
};

struct pfioc_states {
	int     ps_len;
	union {
		caddr_t                  psu_buf;
		struct pfsync_state     *psu_states;
	} ps_u __attribute__((aligned(8)));
#define ps_buf          ps_u.psu_buf
#define ps_states       ps_u.psu_states
};


#define PFTOK_PROCNAME_LEN    64
#pragma pack(1)
struct pfioc_token {
	u_int64_t                       token_value;
	u_int64_t                       timestamp;
	pid_t                           pid;
	char                            proc_name[PFTOK_PROCNAME_LEN];
};
#pragma pack()

struct pfioc_kernel_token {
	SLIST_ENTRY(pfioc_kernel_token) next;
	struct pfioc_token              token;
};

struct pfioc_remove_token {
	u_int64_t                token_value;
	u_int64_t                refcount;
};

struct pfioc_tokens {
	int     size;
	union {
		caddr_t                         pgtu_buf;
		struct pfioc_token              *pgtu_tokens;
	} pgt_u __attribute__((aligned(8)));
#define pgt_buf         pgt_u.pgtu_buf
#define pgt_tokens      pgt_u.pgtu_tokens
};



struct pfioc_src_nodes {
	int     psn_len;
	union {
		caddr_t                 psu_buf;
		struct pf_src_node      *psu_src_nodes;
	} psn_u __attribute__((aligned(8)));
#define psn_buf         psn_u.psu_buf
#define psn_src_nodes   psn_u.psu_src_nodes
};


struct pfioc_if {
	char             ifname[IFNAMSIZ];
};

struct pfioc_tm {
	int              timeout;
	int              seconds;
};

struct pfioc_limit {
	int              index;
	unsigned         limit;
};

struct pfioc_altq {
	u_int32_t        action;
	u_int32_t        ticket;
	u_int32_t        nr;
	struct pf_altq   altq                   __attribute__((aligned(8)));
};

struct pfioc_qstats {
	u_int32_t        ticket;
	u_int32_t        nr;
	void            *buf                    __attribute__((aligned(8)));
	int              nbytes                 __attribute__((aligned(8)));
	u_int8_t         scheduler;
};

struct pfioc_ruleset {
	u_int32_t        nr;
	char             path[MAXPATHLEN];
	char             name[PF_ANCHOR_NAME_SIZE];
};

#define PF_RULESET_ALTQ         (PF_RULESET_MAX)
#define PF_RULESET_TABLE        (PF_RULESET_MAX+1)
struct pfioc_trans {
	int              size;  /* number of elements */
	int              esize; /* size of each element in bytes */
	struct pfioc_trans_e {
		int             rs_num;
		char            anchor[MAXPATHLEN];
		u_int32_t       ticket;
	} *array __attribute__((aligned(8)));
};



#define PFR_FLAG_ATOMIC         0x00000001
#define PFR_FLAG_DUMMY          0x00000002
#define PFR_FLAG_FEEDBACK       0x00000004
#define PFR_FLAG_CLSTATS        0x00000008
#define PFR_FLAG_ADDRSTOO       0x00000010
#define PFR_FLAG_REPLACE        0x00000020
#define PFR_FLAG_ALLRSETS       0x00000040
#define PFR_FLAG_ALLMASK        0x0000007F

struct pfioc_table {
	struct pfr_table         pfrio_table;
	void                    *pfrio_buffer   __attribute__((aligned(8)));
	int                      pfrio_esize    __attribute__((aligned(8)));
	int                      pfrio_size;
	int                      pfrio_size2;
	int                      pfrio_nadd;
	int                      pfrio_ndel;
	int                      pfrio_nchange;
	int                      pfrio_flags;
	u_int32_t                pfrio_ticket;
};
#define pfrio_exists    pfrio_nadd
#define pfrio_nzero     pfrio_nadd
#define pfrio_nmatch    pfrio_nadd
#define pfrio_naddr     pfrio_size2
#define pfrio_setflag   pfrio_size2
#define pfrio_clrflag   pfrio_nadd


struct pfioc_iface {
	char     pfiio_name[IFNAMSIZ];
	void    *pfiio_buffer                   __attribute__((aligned(8)));
	int      pfiio_esize                    __attribute__((aligned(8)));
	int      pfiio_size;
	int      pfiio_nzero;
	int      pfiio_flags;
};


struct pf_ifspeed {
	char                    ifname[IFNAMSIZ];
	u_int64_t               baudrate;
};

/*
 * ioctl operations
 */

#define DIOCSTART       _IO  ('D',  1)
#define DIOCSTOP        _IO  ('D',  2)
#define DIOCADDRULE     _IOWR('D',  4, struct pfioc_rule)
#define DIOCGETSTARTERS _IOWR('D',  5, struct pfioc_tokens)
#define DIOCGETRULES    _IOWR('D',  6, struct pfioc_rule)
#define DIOCGETRULE     _IOWR('D',  7, struct pfioc_rule)
#define DIOCSTARTREF    _IOR ('D',  8, u_int64_t)
#define DIOCSTOPREF     _IOWR('D',  9, struct pfioc_remove_token)
/* XXX cut 10 - 17 */
#define DIOCCLRSTATES   _IOWR('D', 18, struct pfioc_state_kill)
#define DIOCGETSTATE    _IOWR('D', 19, struct pfioc_state)
#define DIOCSETSTATUSIF _IOWR('D', 20, struct pfioc_if)
#define DIOCGETSTATUS   _IOWR('D', 21, struct pf_status)
#define DIOCCLRSTATUS   _IO  ('D', 22)
#define DIOCNATLOOK     _IOWR('D', 23, struct pfioc_natlook)
#define DIOCSETDEBUG    _IOWR('D', 24, u_int32_t)
#define DIOCGETSTATES   _IOWR('D', 25, struct pfioc_states)
#define DIOCCHANGERULE  _IOWR('D', 26, struct pfioc_rule)
#define DIOCINSERTRULE  _IOWR('D',  27, struct pfioc_rule)
#define DIOCDELETERULE  _IOWR('D',  28, struct pfioc_rule)
#define DIOCSETTIMEOUT  _IOWR('D', 29, struct pfioc_tm)
#define DIOCGETTIMEOUT  _IOWR('D', 30, struct pfioc_tm)
#define DIOCADDSTATE    _IOWR('D', 37, struct pfioc_state)
#define DIOCCLRRULECTRS _IO  ('D', 38)
#define DIOCGETLIMIT    _IOWR('D', 39, struct pfioc_limit)
#define DIOCSETLIMIT    _IOWR('D', 40, struct pfioc_limit)
#define DIOCKILLSTATES  _IOWR('D', 41, struct pfioc_state_kill)
#define DIOCSTARTALTQ   _IO  ('D', 42)
#define DIOCSTOPALTQ    _IO  ('D', 43)
#define DIOCADDALTQ     _IOWR('D', 45, struct pfioc_altq)
#define DIOCGETALTQS    _IOWR('D', 47, struct pfioc_altq)
#define DIOCGETALTQ     _IOWR('D', 48, struct pfioc_altq)
#define DIOCCHANGEALTQ  _IOWR('D', 49, struct pfioc_altq)
#define DIOCGETQSTATS   _IOWR('D', 50, struct pfioc_qstats)
#define DIOCBEGINADDRS  _IOWR('D', 51, struct pfioc_pooladdr)
#define DIOCADDADDR     _IOWR('D', 52, struct pfioc_pooladdr)
#define DIOCGETADDRS    _IOWR('D', 53, struct pfioc_pooladdr)
#define DIOCGETADDR     _IOWR('D', 54, struct pfioc_pooladdr)
#define DIOCCHANGEADDR  _IOWR('D', 55, struct pfioc_pooladdr)
/* XXX cut 55 - 57 */
#define DIOCGETRULESETS _IOWR('D', 58, struct pfioc_ruleset)
#define DIOCGETRULESET  _IOWR('D', 59, struct pfioc_ruleset)
#define DIOCRCLRTABLES  _IOWR('D', 60, struct pfioc_table)
#define DIOCRADDTABLES  _IOWR('D', 61, struct pfioc_table)
#define DIOCRDELTABLES  _IOWR('D', 62, struct pfioc_table)
#define DIOCRGETTABLES  _IOWR('D', 63, struct pfioc_table)
#define DIOCRGETTSTATS  _IOWR('D', 64, struct pfioc_table)
#define DIOCRCLRTSTATS  _IOWR('D', 65, struct pfioc_table)
#define DIOCRCLRADDRS   _IOWR('D', 66, struct pfioc_table)
#define DIOCRADDADDRS   _IOWR('D', 67, struct pfioc_table)
#define DIOCRDELADDRS   _IOWR('D', 68, struct pfioc_table)
#define DIOCRSETADDRS   _IOWR('D', 69, struct pfioc_table)
#define DIOCRGETADDRS   _IOWR('D', 70, struct pfioc_table)
#define DIOCRGETASTATS  _IOWR('D', 71, struct pfioc_table)
#define DIOCRCLRASTATS  _IOWR('D', 72, struct pfioc_table)
#define DIOCRTSTADDRS   _IOWR('D', 73, struct pfioc_table)
#define DIOCRSETTFLAGS  _IOWR('D', 74, struct pfioc_table)
#define DIOCRINADEFINE  _IOWR('D', 77, struct pfioc_table)
#define DIOCOSFPFLUSH   _IO('D', 78)
#define DIOCOSFPADD     _IOWR('D', 79, struct pf_osfp_ioctl)
#define DIOCOSFPGET     _IOWR('D', 80, struct pf_osfp_ioctl)
#define DIOCXBEGIN      _IOWR('D', 81, struct pfioc_trans)
#define DIOCXCOMMIT     _IOWR('D', 82, struct pfioc_trans)
#define DIOCXROLLBACK   _IOWR('D', 83, struct pfioc_trans)
#define DIOCGETSRCNODES _IOWR('D', 84, struct pfioc_src_nodes)
#define DIOCCLRSRCNODES _IO('D', 85)
#define DIOCSETHOSTID   _IOWR('D', 86, u_int32_t)
#define DIOCIGETIFACES  _IOWR('D', 87, struct pfioc_iface)
#define DIOCSETIFFLAG   _IOWR('D', 89, struct pfioc_iface)
#define DIOCCLRIFFLAG   _IOWR('D', 90, struct pfioc_iface)
#define DIOCKILLSRCNODES _IOWR('D', 91, struct pfioc_src_node_kill)
#define DIOCGIFSPEED    _IOWR('D', 92, struct pf_ifspeed)

extern struct pf_anchor_global pf_anchors;
extern struct pf_anchor pf_main_anchor;
#define pf_main_ruleset pf_main_anchor.ruleset

/* these ruleset functions can be linked into userland programs (pfctl) */
extern int pf_get_ruleset_number(u_int8_t);
extern void pf_init_ruleset(struct pf_ruleset *);
extern int pf_anchor_setup(struct pf_rule *, const struct pf_ruleset *,
    const char *);
extern int pf_anchor_copyout(const struct pf_ruleset *, const struct pf_rule *,
    struct pfioc_rule *);
extern void pf_anchor_remove(struct pf_rule *);
extern void pf_remove_if_empty_ruleset(struct pf_ruleset *);
extern struct pf_anchor *pf_find_anchor(const char *);
extern struct pf_ruleset *pf_find_ruleset(const char *);
extern struct pf_ruleset *pf_find_ruleset_with_owner(const char *,
    const char *, int, int *);
extern struct pf_ruleset *pf_find_or_create_ruleset(const char *);
extern void pf_rs_initialize(void);

#ifdef  __cplusplus
}
#endif
#endif /* _NET_PFVAR_H_ */
