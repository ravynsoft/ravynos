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
/* Copyright (c) 1998, 1999 Apple Computer, Inc. All Rights Reserved */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1982, 1986, 1990, 1993
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
 *	@(#)socketvar.h	8.3 (Berkeley) 2/19/95
 * $FreeBSD: src/sys/sys/socketvar.h,v 1.46.2.6 2001/08/31 13:45:49 jlemon Exp $
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _SYS_SOCKETVAR_H_
#define _SYS_SOCKETVAR_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/types.h> /* u_quad_t */

typedef u_quad_t so_gen_t;


#define SO_TC_STATS_MAX 4

struct data_stats {
	u_int64_t       rxpackets;
	u_int64_t       rxbytes;
	u_int64_t       txpackets;
	u_int64_t       txbytes;
};

#define MSG_PRI_0 0     /* TCP message priority, lowest */
#define MSG_PRI_1 1
#define MSG_PRI_2 2
#define MSG_PRI_3 3     /* TCP message priority, highest */
#define MSG_PRI_MAX MSG_PRI_3
#define MSG_PRI_MIN MSG_PRI_0
#define MSG_PRI_COUNT 4
#define MSG_PRI_DEFAULT MSG_PRI_1


#if defined(__LP64__)
#define _XSOCKET_PTR(x)         u_int32_t
#else
#define _XSOCKET_PTR(x)         x
#endif

/* Flags returned in data field for EVFILT_SOCK events. */
#define SOCKEV_CONNECTED        0x00000001 /* connected */
#define SOCKEV_DISCONNECTED     0x00000002 /* disconnected */

#pragma pack(4)

struct xsockbuf {
	u_int32_t       sb_cc;
	u_int32_t       sb_hiwat;
	u_int32_t       sb_mbcnt;
	u_int32_t       sb_mbmax;
	int32_t         sb_lowat;
	short           sb_flags;
	short           sb_timeo;
};

/*
 * Externalized form of struct socket used by the sysctl(3) interface.
 */
struct  xsocket {
	u_int32_t               xso_len;        /* length of this structure */
	_XSOCKET_PTR(struct socket *) xso_so;   /* makes a convenient handle */
	short                   so_type;
	short                   so_options;
	short                   so_linger;
	short                   so_state;
	_XSOCKET_PTR(caddr_t)   so_pcb;         /* another convenient handle */
	int                     xso_protocol;
	int                     xso_family;
	short                   so_qlen;
	short                   so_incqlen;
	short                   so_qlimit;
	short                   so_timeo;
	u_short                 so_error;
	pid_t                   so_pgid;
	u_int32_t               so_oobmark;
	struct xsockbuf         so_rcv;
	struct xsockbuf         so_snd;
	uid_t                   so_uid;         /* XXX */
};

#if !CONFIG_EMBEDDED
struct  xsocket64 {
	u_int32_t               xso_len;        /* length of this structure */
	u_int64_t               xso_so;         /* makes a convenient handle */
	short                   so_type;
	short                   so_options;
	short                   so_linger;
	short                   so_state;
	u_int64_t               so_pcb;         /* another convenient handle */
	int                     xso_protocol;
	int                     xso_family;
	short                   so_qlen;
	short                   so_incqlen;
	short                   so_qlimit;
	short                   so_timeo;
	u_short                 so_error;
	pid_t                   so_pgid;
	u_int32_t               so_oobmark;
	struct xsockbuf         so_rcv;
	struct xsockbuf         so_snd;
	uid_t                   so_uid;         /* XXX */
};
#endif /* !CONFIG_EMBEDDED */

#define XSO_SOCKET      0x001
#define XSO_RCVBUF      0x002
#define XSO_SNDBUF      0x004
#define XSO_STATS       0x008
#define XSO_INPCB       0x010
#define XSO_TCPCB       0x020
#define XSO_KCREG       0x040
#define XSO_KCB         0x080
#define XSO_EVT         0x100

struct  xsocket_n {
	u_int32_t               xso_len;        /* length of this structure */
	u_int32_t               xso_kind;       /* XSO_SOCKET */
	u_int64_t               xso_so;         /* makes a convenient handle */
	short                   so_type;
	u_int32_t               so_options;
	short                   so_linger;
	short                   so_state;
	u_int64_t               so_pcb;         /* another convenient handle */
	int                     xso_protocol;
	int                     xso_family;
	short                   so_qlen;
	short                   so_incqlen;
	short                   so_qlimit;
	short                   so_timeo;
	u_short                 so_error;
	pid_t                   so_pgid;
	u_int32_t               so_oobmark;
	uid_t                   so_uid;         /* XXX */
	pid_t                   so_last_pid;
	pid_t                   so_e_pid;
};

struct xsockbuf_n {
	u_int32_t               xsb_len;        /* length of this structure */
	u_int32_t               xsb_kind;       /* XSO_RCVBUF or XSO_SNDBUF */
	u_int32_t               sb_cc;
	u_int32_t               sb_hiwat;
	u_int32_t               sb_mbcnt;
	u_int32_t               sb_mbmax;
	int32_t                 sb_lowat;
	short                   sb_flags;
	short                   sb_timeo;
};

struct xsockstat_n {
	u_int32_t               xst_len;        /* length of this structure */
	u_int32_t               xst_kind;       /* XSO_STATS */
	struct data_stats       xst_tc_stats[SO_TC_STATS_MAX];
};

/*
 * Global socket statistics
 */
struct soextbkidlestat {
	u_int32_t       so_xbkidle_maxperproc;
	u_int32_t       so_xbkidle_time;
	u_int32_t       so_xbkidle_rcvhiwat;
	int32_t         so_xbkidle_notsupp;
	int32_t         so_xbkidle_toomany;
	int32_t         so_xbkidle_wantok;
	int32_t         so_xbkidle_active;
	int32_t         so_xbkidle_nocell;
	int32_t         so_xbkidle_notime;
	int32_t         so_xbkidle_forced;
	int32_t         so_xbkidle_resumed;
	int32_t         so_xbkidle_expired;
	int32_t         so_xbkidle_resched;
	int32_t         so_xbkidle_nodlgtd;
	int32_t         so_xbkidle_drained;
};

#pragma pack()

#endif /* !_SYS_SOCKETVAR_H_ */
