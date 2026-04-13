/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_chash.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 20/3/18.
 */

#ifndef lf_hfs_chash_h
#define lf_hfs_chash_h
#include "lf_hfs_common.h"
#include "lf_hfs.h"

struct cnode* hfs_chash_getcnode(struct hfsmount *hfsmp, ino_t inum, struct vnode **vpp, int wantrsrc, int skiplock, int *out_flags, int *hflags);
void hfs_chash_lock(struct hfsmount *hfsmp);
void hfs_chash_lock_spin(struct hfsmount *hfsmp);
void hfs_chash_lock_convert(struct hfsmount *hfsmp);
void hfs_chash_unlock(struct hfsmount *hfsmp);
void hfs_chashwakeup(struct hfsmount *hfsmp, struct cnode *cp, int hflags);
void hfs_chash_abort(struct hfsmount *hfsmp, struct cnode *cp);
struct vnode* hfs_chash_getvnode(struct hfsmount *hfsmp, ino_t inum, int wantrsrc, int skiplock, int allow_deleted);
int hfs_chash_snoop(struct hfsmount *hfsmp, ino_t inum, int existence_only, int (*callout)(const cnode_t *cp, void *), void * arg);
int hfs_chash_set_childlinkbit(struct hfsmount *hfsmp, cnid_t cnid);
int hfs_chashremove(struct hfsmount *hfsmp, struct cnode *cp);
void hfs_chash_mark_in_transit(struct hfsmount *hfsmp, struct cnode *cp);
void hfs_chash_lower_OpenLookupCounter(struct cnode *cp);
void hfs_chash_raise_OpenLookupCounter(struct cnode *cp);
void hfs_chash_wait(struct hfsmount *hfsmp, struct cnode  *cp);

#endif /* lf_hfs_chash_h */
