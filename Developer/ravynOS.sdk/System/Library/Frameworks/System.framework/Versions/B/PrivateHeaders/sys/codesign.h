/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_CODESIGN_H_
#define _SYS_CODESIGN_H_

#include <System/kern/cs_blobs.h>

/* MAC flags used by F_ADDFILESIGS_* */
#define MAC_VNODE_CHECK_DYLD_SIM 0x1   /* tells the MAC framework that dyld-sim is being loaded */

#define CLEAR_LV_ENTITLEMENT "com.apple.private.security.clear-library-validation"

/* csops  operations */
#define CS_OPS_STATUS           0       /* return status */
#define CS_OPS_MARKINVALID      1       /* invalidate process */
#define CS_OPS_MARKHARD         2       /* set HARD flag */
#define CS_OPS_MARKKILL         3       /* set KILL flag (sticky) */
#define CS_OPS_CDHASH           5       /* get code directory hash */
#define CS_OPS_PIDOFFSET        6       /* get offset of active Mach-o slice */
#define CS_OPS_ENTITLEMENTS_BLOB 7      /* get entitlements blob */
#define CS_OPS_MARKRESTRICT     8       /* set RESTRICT flag (sticky) */
#define CS_OPS_SET_STATUS       9       /* set codesign flags */
#define CS_OPS_BLOB             10      /* get codesign blob */
#define CS_OPS_IDENTITY         11      /* get codesign identity */
#define CS_OPS_CLEARINSTALLER   12      /* clear INSTALLER flag */
#define CS_OPS_CLEARPLATFORM 13 /* clear platform binary status (DEVELOPMENT-only) */
#define CS_OPS_TEAMID       14  /* get team id */
#define CS_OPS_CLEAR_LV     15  /* clear the library validation flag */

#define CS_MAX_TEAMID_LEN       64


#include <sys/types.h>
#include <mach/message.h>

__BEGIN_DECLS
/* code sign operations */
int csops(pid_t pid, unsigned int  ops, void * useraddr, size_t usersize);
int csops_audittoken(pid_t pid, unsigned int  ops, void * useraddr, size_t usersize, audit_token_t * token);
__END_DECLS


#endif /* _SYS_CODESIGN_H_ */
