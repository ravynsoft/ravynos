/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
 *	@(#)protosw.h	8.1 (Berkeley) 6/2/93
 * $FreeBSD: src/sys/sys/protosw.h,v 1.28.2.2 2001/07/03 11:02:01 ume Exp $
 */

#ifndef _SYS_PROTOSW_H_
#define _SYS_PROTOSW_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

/* XXX: this will go away */
#define PR_SLOWHZ       2               /* 2 slow timeouts per second */

/*
 * The arguments to the ctlinput routine are
 *      (*protosw[].pr_ctlinput)(cmd, sa, arg);
 * where cmd is one of the commands below, sa is a pointer to a sockaddr,
 * and arg is a `void *' argument used within a protocol family.
 */
#define PRC_IFDOWN              0       /* interface transition */
#define PRC_ROUTEDEAD           1       /* select new route if possible ??? */
#define PRC_IFUP                2       /* interface has come back up */
#define PRC_QUENCH2             3       /* DEC congestion bit says slow down */
#define PRC_QUENCH              4       /* some one said to slow down */
#define PRC_MSGSIZE             5       /* message size forced drop */
#define PRC_HOSTDEAD            6       /* host appears to be down */
#define PRC_HOSTUNREACH         7       /* deprecated (use PRC_UNREACH_HOST) */
#define PRC_UNREACH_NET         8       /* no route to network */
#define PRC_UNREACH_HOST        9       /* no route to host */
#define PRC_UNREACH_PROTOCOL    10      /* dst says bad protocol */
#define PRC_UNREACH_PORT        11      /* bad port # */
/* was PRC_UNREACH_NEEDFRAG	12         (use PRC_MSGSIZE) */
#define PRC_UNREACH_SRCFAIL     13      /* source route failed */
#define PRC_REDIRECT_NET        14      /* net routing redirect */
#define PRC_REDIRECT_HOST       15      /* host routing redirect */
#define PRC_REDIRECT_TOSNET     16      /* redirect for type of service & net */
#define PRC_REDIRECT_TOSHOST    17      /* redirect for tos & host */
#define PRC_TIMXCEED_INTRANS    18      /* packet lifetime expired in transit */
#define PRC_TIMXCEED_REASS      19      /* lifetime expired on reass q */
#define PRC_PARAMPROB           20      /* header incorrect */
#define PRC_UNREACH_ADMIN_PROHIB        21     /* packet administrativly prohibited */

#define PRC_NCMDS               22

#define PRC_IS_REDIRECT(cmd)    \
	((cmd) >= PRC_REDIRECT_NET && (cmd) <= PRC_REDIRECT_TOSHOST)


#endif  /* !_SYS_PROTOSW_H_ */
