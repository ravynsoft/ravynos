/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_xattr.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 28/3/18.
 */
#ifndef lf_hfs_xattr_h
#define lf_hfs_xattr_h

#include "lf_hfs_vnode.h"
#include "lf_hfs_format.h"
#include <UserFS/UserVFS.h>

int hfs_attrkeycompare(HFSPlusAttrKey *searchKey, HFSPlusAttrKey *trialKey);
int init_attrdata_vnode(struct hfsmount *hfsmp);
int file_attribute_exist(struct hfsmount *hfsmp, uint32_t fileID);
int hfs_buildattrkey(u_int32_t fileID, const char *attrname, HFSPlusAttrKey *key);
int hfs_removeallattr(struct hfsmount *hfsmp, u_int32_t fileid, bool *open_transaction);
int hfs_vnop_getxattr(vnode_t vp, const char *attr_name, void *buf, size_t bufsize, size_t *actual_size);
int hfs_vnop_setxattr(vnode_t vp, const char *attr_name, const void *buf, size_t bufsize, UVFSXattrHow How);
int hfs_vnop_removexattr(vnode_t vp, const char *attr_name);
int hfs_vnop_listxattr(vnode_t vp, void *buf, size_t bufsize, size_t *actual_size);

#endif /* lf_hfs_xattr_h */
