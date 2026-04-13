/*
 * Copyright (c) 1999-2000, 2002, 2007 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "../fsck_messages.h"

/* External API to CheckHFS */

enum {
	kNeverCheck = 0,	/* never check (clean/dirty status only) */
	kDirtyCheck = 1,	/* only check if dirty */
	kAlwaysCheck = 2,	/* always check */
	kPartialCheck = 3,	/* used with kForceRepairs in order to set up environment */
	kForceCheck = 4,
	kMajorCheck = 5,	/* Check for major vs. minor errors */

	kNeverRepair = 0,	/* never repair */
	kMinorRepairs = 1,	/* only do minor repairs (fsck preen) */
	kMajorRepairs = 2,	/* do all possible repairs */
	kForceRepairs = 3,	/* force a repair of catalog B-Tree */
	
	kNeverLog = 0,
	kFatalLog = 1,		/* (fsck preen) */
	kVerboseLog = 2,	/* (Disk First Aid) */
	kDebugLog = 3
};

enum {
	R_NoMem			= 1,	/* not enough memory to do scavenge */
	R_IntErr		= 2,	/* internal Scavenger error */
	R_NoVol			= 3,	/* no volume in drive */
	R_RdErr			= 4,	/* unable to read from disk */
	R_WrErr			= 5,	/* unable to write to disk */
	R_BadSig		= 6,	/* not HFS/HFS+ signature */
	R_VFail			= 7,	/* verify failed */
	R_RFail			= 8,	/* repair failed */
	R_UInt			= 9,	/* user interrupt */
	R_Modified		= 10,	/* volume modifed by another app */
	R_BadVolumeHeader	= 11,	/* Invalid VolumeHeader */
	R_FileSharingIsON	= 12,	/* File Sharing is on */
	R_Dirty			= 13,	/* Dirty, but no checks were done */

	Max_RCode		= 13	/* maximum result code */
};

/* Option bits to indicate which type of btree to rebuild */
#define REBUILD_CATALOG		0x1
#define REBUILD_EXTENTS		0x2
#define REBUILD_ATTRIBUTE	0x4

extern int gGUIControl;

extern int CheckHFS(	const char *rdevnode, int fsReadRef, int fsWriteRef, 
						int checkLevel, int repairLevel, 
						fsck_ctx_t fsckContext,
						int lostAndFoundMode, int canWrite,
						int *modified, int liveMode, int rebuildOptions );

extern int journal_replay(const char *);

