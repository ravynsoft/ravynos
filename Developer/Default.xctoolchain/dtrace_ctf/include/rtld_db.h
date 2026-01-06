/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef	_RTLD_DB_H
#define	_RTLD_DB_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/link.h>

typedef uint64_t Lmid_t;

typedef unsigned long	psaddr_t;

struct ps_prochandle;


typedef enum {
	RD_OK,		/* generic "call" succeeded */
} rd_err_e;

/*
 * ways that the event notification can take place:
 */
typedef enum {
	RD_NOTIFY_BPT,		/* set break-point at address */
} rd_notify_e;

/*
 * information on ways that the event notification can take place:
 */
typedef struct rd_notify {
	rd_notify_e	type;
	union {
		psaddr_t	bptaddr;	/* break point address */
		long		syscallno;	/* system call id */
	} u;
} rd_notify_t;

/*
 * information about event instance:
 */
typedef enum {
	RD_NOSTATE = 0,		/* no state information */
	RD_CONSISTENT,		/* link-maps are stable */
	RD_ADD,			/* currently adding object to link-maps */
} rd_state_e;

typedef struct rd_event_msg {
	rd_event_e	type;
	union {
		rd_state_e	state;	/* for DLACTIVITY */
	} u;
} rd_event_msg_t;

typedef struct rd_agent rd_agent_t;

extern char		*rd_errstr(rd_err_e rderr);
extern rd_err_e		rd_event_addr(rd_agent_t *, rd_event_e, rd_notify_t *);
extern rd_err_e		rd_event_enable(rd_agent_t *, int);
extern rd_err_e		rd_event_getmsg(rd_agent_t *, rd_event_msg_t *);

#ifdef	__cplusplus
}
#endif

#endif	/* _RTLD_DB_H */
