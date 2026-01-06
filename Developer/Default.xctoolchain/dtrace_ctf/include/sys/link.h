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
 *	Copyright (c) 1988 AT&T
 *	  All Rights Reserved
 *
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _SYS_LINK_H
#define	_SYS_LINK_H

#ifndef	_ASM
#include <sys/types.h>

#include <sys/elftypes.h>

#endif

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Public structure defined and maintained within the runtime linker
 */
#ifndef	_ASM
/*
 * Debugging events enabled inside of the runtime linker.  To
 * access these events see the librtld_db interface.
 */
typedef enum {
	RD_NONE = 0,		/* no event */
	RD_PREINIT,		/* the Initial rendezvous before .init */
	RD_POSTINIT,		/* the Second rendezvous after .init */
	RD_DLACTIVITY,		/* a dlopen or dlclose has happened */
	RD_DYLD_LOST,		/* communication with target dyld was lost */
	RD_DYLD_EXIT		/* target exited. */
} rd_event_e;

#endif	/* _ASM */

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_LINK_H */
