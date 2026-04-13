/*
 * Copyright (C) 2003-2015 Apple Inc. All rights reserved.
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

#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/malloc.h>
#include <sys/ubc.h>
#include <sys/quota.h>

#include <sys/kdebug.h>

#include "hfs.h"
#include "hfs_catalog.h"
#include "hfs_cnode.h"
#include "hfs_dbg.h"
#include "hfs_mount.h"
#include "hfs_quota.h"
#include "hfs_endian.h"

#include "BTreesInternal.h"
#include "FileMgrInternal.h"



void hfs_generate_volume_notifications(struct hfsmount *hfsmp) 
{
	fsid_t fsid;
	u_int32_t freeblks, state=999;

	/* Do not generate low disk notifications for read-only volumes */
	if (hfsmp->hfs_flags & HFS_READ_ONLY) {
		return;
	}

	fsid.val[0] = hfsmp->hfs_raw_dev;
	fsid.val[1] = vfs_typenum(HFSTOVFS(hfsmp));
	
	freeblks = hfs_freeblks(hfsmp, 1);

	/*
	 * Find the theshold the number of free blocks fits into.
	 * We fire upon reaching a level below desired only once,
	 * except for when we reach the low disk or near low disk levels
	 * from below, in which case we do not fire unless we have also
	 * reached the desired disk level (hysteresis).
	 * This is illustrated in the following diagram:
	 *
	 * fire ^
	 * --------- desired level
	 *      |
	 *
	 *
	 *      |
	 * --------- near low disk level
	 * fire v
	 *
	 *
	 *      |
	 * --------- low disk level
	 * fire v
	 *
	 *
	 *      | ^ fire
	 * --------- very low disk level
	 * fire v |
	 *
	 */
	if (freeblks < hfsmp->hfs_freespace_notify_dangerlimit) {
		state = 4;
	} else if (freeblks < hfsmp->hfs_freespace_notify_warninglimit) {
		state = 3;
	} else if (freeblks < hfsmp->hfs_freespace_notify_nearwarninglimit) {
		state = 2;
	} else if (freeblks < hfsmp->hfs_freespace_notify_desiredlevel) {
		/* We are between the near low disk and desired levels */
		state = 1;
	} else if (freeblks >= hfsmp->hfs_freespace_notify_desiredlevel) {
		state = 0;
	}

	/* Free blocks are less than dangerlimit for the first time */
	if (state == 4 && !(hfsmp->hfs_notification_conditions & VQ_VERYLOWDISK)) {
		/* Dump some logging to track down intermittent issues */
		printf("hfs: set VeryLowDisk: vol:%s, freeblks:%d, dangerlimit:%d\n", hfsmp->vcbVN, freeblks, hfsmp->hfs_freespace_notify_dangerlimit);

#if HFS_SPARSE_DEV
		// If we're a sparse device, dump some info about the backing store..
		hfs_lock_mount(hfsmp);
		vnode_t backing_vp = hfsmp->hfs_backingvp;
		if (backing_vp && vnode_get(backing_vp) != 0)
			backing_vp = NULL;
		hfs_unlock_mount(hfsmp);

		if (backing_vp) {
			struct vfsstatfs *sfs = vfs_statfs(vnode_mount(backing_vp));
			printf("hfs: set VeryLowDisk: vol:%s, backingstore b_avail:%lld, tag:%d\n",
				   hfsmp->vcbVN, sfs->f_bavail, vnode_tag(backing_vp));
			vnode_put(backing_vp);
		}
#endif

		hfsmp->hfs_notification_conditions |= (VQ_VERYLOWDISK|VQ_LOWDISK|VQ_NEARLOWDISK);
		vfs_event_signal(&fsid, hfsmp->hfs_notification_conditions, (intptr_t)NULL);
	} else if (state == 3) {
		/* Free blocks are less than warning limit for the first time */
		if (!(hfsmp->hfs_notification_conditions & VQ_LOWDISK)) {
			printf("hfs: set LowDisk: vol:%s, freeblks:%d, warninglimit:%d\n", hfsmp->vcbVN, freeblks, hfsmp->hfs_freespace_notify_warninglimit);
			hfsmp->hfs_notification_conditions |= (VQ_LOWDISK|VQ_NEARLOWDISK);
			vfs_event_signal(&fsid, hfsmp->hfs_notification_conditions, (intptr_t)NULL);
		} else if (hfsmp->hfs_notification_conditions & VQ_VERYLOWDISK) {
			/* Free blocks count has increased from danger limit to warning limit, so just clear VERYLOWDISK warning */
			printf("hfs: clear VeryLowDisk: vol:%s, freeblks:%d, dangerlimit:%d\n", hfsmp->vcbVN, freeblks, hfsmp->hfs_freespace_notify_dangerlimit);
			hfsmp->hfs_notification_conditions &= ~VQ_VERYLOWDISK;
			vfs_event_signal(&fsid, hfsmp->hfs_notification_conditions, (intptr_t)NULL);
		}
	} else if (state == 2) {
		/* Free blocks are less than the near warning limit for the first time */
		if (!(hfsmp->hfs_notification_conditions & VQ_NEARLOWDISK)) {
			printf("hfs: set NearLowDisk: vol:%s, freeblks:%d, nearwarninglimit:%d\n", hfsmp->vcbVN, freeblks,
				   hfsmp->hfs_freespace_notify_nearwarninglimit);

			hfsmp->hfs_notification_conditions |= VQ_NEARLOWDISK;
			vfs_event_signal(&fsid, hfsmp->hfs_notification_conditions, (intptr_t)NULL);
		} else {
			/* Free blocks count has increased from warning/danger limit to near warning limit,
			 * so clear VERYLOWDISK / LOWDISK warnings, and signal if we clear VERYLOWDISK */
			hfsmp->hfs_notification_conditions &= ~VQ_LOWDISK;
			if (hfsmp->hfs_notification_conditions & VQ_VERYLOWDISK) {
				printf("hfs: clear VeryLowDisk: vol:%s, freeblks:%d, dangerlimit:%d\n", hfsmp->vcbVN, freeblks,
					   hfsmp->hfs_freespace_notify_dangerlimit);

				hfsmp->hfs_notification_conditions &= ~VQ_VERYLOWDISK;
				vfs_event_signal(&fsid, hfsmp->hfs_notification_conditions, (intptr_t)NULL);
			}
		}
	} else if (state == 1) {
		/* Free blocks are less than the desireable level, but more than the near warning level
		 * In this case, we may have to notify if we were previously underneath the danger limit */
		if (hfsmp->hfs_notification_conditions & VQ_VERYLOWDISK) {
			printf("hfs: clear VeryLowDisk: vol:%s, freeblks:%d, dangerlimit:%d\n", hfsmp->vcbVN, freeblks,
				   hfsmp->hfs_freespace_notify_dangerlimit);

			hfsmp->hfs_notification_conditions &= ~VQ_VERYLOWDISK;
			vfs_event_signal(&fsid, hfsmp->hfs_notification_conditions, (intptr_t)NULL);
		}
	} else if (state == 0) {
		/* Free blocks count has increased to desirable level, so clear all conditions */
		if (hfsmp->hfs_notification_conditions & (VQ_NEARLOWDISK|VQ_LOWDISK|VQ_VERYLOWDISK)) {
			if (hfsmp->hfs_notification_conditions & VQ_NEARLOWDISK) {
				printf("hfs: clear NearLowDisk: vol:%s, freeblks:%d, nearwarninglimit:%d, desiredlevel:%d\n", hfsmp->vcbVN,
					   freeblks, hfsmp->hfs_freespace_notify_nearwarninglimit, hfsmp->hfs_freespace_notify_desiredlevel);
			}
			if (hfsmp->hfs_notification_conditions & VQ_LOWDISK) {
				printf("hfs: clear LowDisk: vol:%s, freeblks:%d, warninglimit:%d, desiredlevel:%d\n", hfsmp->vcbVN, freeblks,
					   hfsmp->hfs_freespace_notify_warninglimit, hfsmp->hfs_freespace_notify_desiredlevel);
			}
			if (hfsmp->hfs_notification_conditions & VQ_VERYLOWDISK) {
				printf("hfs: clear VeryLowDisk: vol:%s, freeblks:%d, dangerlimit:%d\n", hfsmp->vcbVN, freeblks, hfsmp->hfs_freespace_notify_warninglimit);
			} 
			hfsmp->hfs_notification_conditions &= ~(VQ_VERYLOWDISK|VQ_LOWDISK|VQ_NEARLOWDISK);
			if (hfsmp->hfs_notification_conditions == 0) {
				vfs_event_signal(&fsid, VQ_UPDATE|VQ_DESIRED_DISK, (intptr_t)NULL);
			} else {
				vfs_event_signal(&fsid, hfsmp->hfs_notification_conditions, (intptr_t)NULL);
			}
		}
	}
}
