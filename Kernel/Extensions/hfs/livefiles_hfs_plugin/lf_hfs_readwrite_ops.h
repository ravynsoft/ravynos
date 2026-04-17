//
//  lf_hfs_readwrite_ops.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_readwrite_ops_h
#define lf_hfs_readwrite_ops_h

#include <stdio.h>
#include "lf_hfs.h"

struct vnop_blockmap_args {
    struct vnodeop_desc     *a_desc;
    vnode_t                 a_vp;
    off_t                   a_foffset;
    size_t                  a_size;
    daddr64_t               *a_bpn;
    size_t                  *a_run;
    void                    *a_poff;
    int                     a_flags;
};

#define HFS_TRUNCATE_SKIPTIMES      0x00000002 /* implied by skipupdate; it is a subset */

int hfs_vnop_blockmap(struct vnop_blockmap_args *ap);
int hfs_prepare_release_storage (struct hfsmount *hfsmp, struct vnode *vp);
int hfs_release_storage (struct hfsmount *hfsmp, struct filefork *datafork, struct filefork *rsrcfork, u_int32_t fileid);
int hfs_truncate(struct vnode *vp, off_t length, int flags, int truncateflags);
int hfs_vnop_preallocate(struct vnode * vp, LIFilePreallocateArgs_t* psPreAllocReq, LIFilePreallocateArgs_t* psPreAllocRes);

#endif /* lf_hfs_readwrite_ops_h */
