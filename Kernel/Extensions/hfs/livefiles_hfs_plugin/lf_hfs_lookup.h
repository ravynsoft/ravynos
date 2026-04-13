//
//  lf_hfs_lookup.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 25/03/2018.
//

#ifndef lf_hfs_lookup_h
#define lf_hfs_lookup_h

#include "lf_hfs_vnode.h"
#include "lf_hfs_vnops.h"

int hfs_vnop_lookup(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp);

#endif /* lf_hfs_lookup_h */
