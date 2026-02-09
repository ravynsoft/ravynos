/*
 * Copyright (c) 2012-2017 Apple Inc. All rights reserved.
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

#ifndef _NETINET_MPTCP_VAR_H_
#define _NETINET_MPTCP_VAR_H_

#include <netinet/in.h>
#include <netinet/tcp.h>


typedef struct mptcp_flow {
	size_t                  flow_len;
	size_t                  flow_tcpci_offset;
	uint32_t                flow_flags;
	sae_connid_t            flow_cid;
	struct sockaddr_storage flow_src;
	struct sockaddr_storage flow_dst;
	uint32_t                flow_relseq;    /* last subflow rel seq# */
	int32_t                 flow_soerror;   /* subflow level error */
	uint32_t                flow_probecnt;  /* number of probes sent */
	conninfo_tcp_t          flow_ci;        /* must be the last field */
} mptcp_flow_t;

typedef struct conninfo_mptcp {
	size_t          mptcpci_len;
	size_t          mptcpci_flow_offset;    /* offsetof first flow */
	size_t          mptcpci_nflows;         /* number of subflows */
	uint32_t        mptcpci_state;          /* MPTCP level state */
	uint32_t        mptcpci_mpte_flags;     /* Session flags */
	uint32_t        mptcpci_flags;          /* MPTCB flags */
	uint32_t        mptcpci_ltoken;         /* local token */
	uint32_t        mptcpci_rtoken;         /* remote token */
	uint32_t        mptcpci_notsent_lowat;  /* NOTSENT_LOWAT */

	/* Send side */
	uint64_t        mptcpci_snduna;         /* DSN of last unacked byte */
	uint64_t        mptcpci_sndnxt;         /* DSN of next byte to send */
	uint64_t        mptcpci_sndmax;         /* DSN of max byte sent */
	uint64_t        mptcpci_lidsn;          /* Local IDSN */
	uint32_t        mptcpci_sndwnd;         /* Send window snapshot */

	/* Receive side */
	uint64_t        mptcpci_rcvnxt;         /* Next expected DSN */
	uint64_t        mptcpci_rcvatmark;      /* Session level rcvnxt */
	uint64_t        mptcpci_ridsn;          /* Peer's IDSN */
	uint32_t        mptcpci_rcvwnd;         /* Receive window */

	uint8_t         mptcpci_mpte_addrid;    /* last addr id */

	mptcp_flow_t    mptcpci_flows[1];
} conninfo_mptcp_t;

/* Use SymptomsD notifications of wifi and cell status in subflow selection */
#define MPTCP_KERN_CTL_NAME    "com.apple.network.advisory"
typedef struct symptoms_advisory {
	union {
		uint32_t        sa_nwk_status_int;
		struct {
			union {
#define SYMPTOMS_ADVISORY_NOCOMMENT     0x0000
#define SYMPTOMS_ADVISORY_USEAPP        0xFFFF /* Very ugly workaround to avoid breaking backwards compatibility - ToDo: Fix it in +1 */
				uint16_t        sa_nwk_status;
				struct {
#define SYMPTOMS_ADVISORY_WIFI_BAD     0x01
#define SYMPTOMS_ADVISORY_WIFI_OK      0x02
					uint8_t sa_wifi_status;
#define SYMPTOMS_ADVISORY_CELL_BAD     0x01
#define SYMPTOMS_ADVISORY_CELL_OK      0x02
					uint8_t sa_cell_status;
				};
			};
			uint16_t        sa_unused;
		};
	};
} symptoms_advisory_t;

#define MPTCP_TARGET_BASED_RSSI_THRESHOLD -75
struct mptcp_symptoms_answer {
	struct symptoms_advisory advisory;
	uuid_t  uuid;
	int32_t rssi;
};

struct mptcp_symptoms_ask_uuid {
	uint32_t        cmd;
#define MPTCP_SYMPTOMS_ASK_UUID         1
	uuid_t          uuid;
	uint32_t        priority;
#define MPTCP_SYMPTOMS_UNKNOWN          0
#define MPTCP_SYMPTOMS_BACKGROUND       1
#define MPTCP_SYMPTOMS_FOREGROUND       2
};

struct kev_mptcp_data {
	int value;
};

#endif /* _NETINET_MPTCP_VAR_H_ */
