/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_vfsops.h
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 20/03/2018.
 */

#ifndef lf_hfs_vfsops_h
#define lf_hfs_vfsops_h

#include "lf_hfs.h"
#include "lf_hfs_dirops_handler.h"

typedef enum {
//    HFS_FVH_WAIT                        = 0x0001, // Livefiles always waits for non-journal volume header writes.
    HFS_FVH_WRITE_ALT                   = 0x0002,
    HFS_FVH_FLUSH_IF_DIRTY              = 0x0004,
    HFS_FVH_MARK_UNMOUNT                = 0x0008,
    HFS_FVH_SKIP_TRANSACTION            = 0x0010, // This volume flush is called from within an hfs-transaction
} hfs_flush_volume_header_options_t;

enum volop{
    VOL_UPDATE,
    VOL_MKDIR,
    VOL_RMDIR,
    VOL_MKFILE,
    VOL_RMFILE
};

void    hfs_mark_inconsistent(struct hfsmount *hfsmp, hfs_inconsistency_reason_t reason);
int     hfs_flushvolumeheader(struct hfsmount *hfsmp, hfs_flush_volume_header_options_t options);
int     hfs_mount(struct mount *mp, vnode_t devvp, user_addr_t data);
int     hfs_ScanVolGetVolName(int iFd, char* pcVolumeName);
void    hfs_getvoluuid(struct hfsmount *hfsmp, uuid_t result_uuid);
void    hfs_scan_blocks (struct hfsmount *hfsmp);
int     hfs_vfs_root(struct mount *mp, struct vnode **vpp);
int     hfs_unmount(struct mount *mp);
void    hfs_setencodingbits(struct hfsmount *hfsmp, u_int32_t encoding);
int     hfs_volupdate(struct hfsmount *hfsmp, enum volop op, int inroot);
int     hfs_vget(struct hfsmount *hfsmp, cnid_t cnid, struct vnode **vpp, int skiplock, int allow_deleted);
int     hfs_GetInfoByID(struct hfsmount *hfsmp, cnid_t cnid, UVFSFileAttributes *file_attrs, char pcName[MAX_UTF8_NAME_LENGTH]);
int     fsck_hfs(int fd, check_flags_t how);
#endif /* lf_hfs_vfsops_h */
