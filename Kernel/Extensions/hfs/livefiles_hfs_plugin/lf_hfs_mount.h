/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_mount.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#ifndef lf_hfs_mount_h
#define lf_hfs_mount_h

#include <sys/time.h>

/*
 * Arguments to mount HFS-based filesystems
 */

#define OVERRIDE_UNKNOWN_PERMISSIONS 0

#define UNKNOWNUID ((uid_t)99)
#define UNKNOWNGID ((gid_t)99)
#define UNKNOWNPERMISSIONS (S_IRWXU | S_IROTH | S_IXOTH)        /* 705 */

struct hfs_mount_args {
    char            *fspec;                 /* block special device to mount */
    uid_t           hfs_uid;                /* uid that owns hfs files (standard HFS only) */
    gid_t           hfs_gid;                /* gid that owns hfs files (standard HFS only) */
    mode_t          hfs_mask;               /* mask to be applied for hfs perms  (standard HFS only) */
    u_int32_t       hfs_encoding;           /* encoding for this volume (standard HFS only) */
    struct timezone hfs_timezone;           /* user time zone info (standard HFS only) */
    int             flags;                  /* mounting flags, see below */
    int             journal_tbuffer_size;   /* size in bytes of the journal transaction buffer */
    int             journal_flags;          /* flags to pass to journal_open/create */
    int             journal_disable;        /* don't use journaling (potentially dangerous) */
};

#define HFSFSMNT_NOXONFILES     0x1    /* disable execute permissions for files */
#define HFSFSMNT_WRAPPER        0x2    /* mount HFS wrapper (if it exists) */
#define HFSFSMNT_EXTENDED_ARGS  0x4    /* indicates new fields after "flags" are valid */

/*
 * User specifiable flags.
 *
 * Unmount uses MNT_FORCE flag.
 */
#define    MNT_RDONLY           0x00000001      /* read only filesystem */
#define    MNT_SYNCHRONOUS      0x00000002      /* file system written synchronously */
#define    MNT_NOEXEC           0x00000004      /* can't exec from filesystem */
#define    MNT_NOSUID           0x00000008      /* don't honor setuid bits on fs */
#define    MNT_NODEV            0x00000010      /* don't interpret special files */
#define    MNT_UNION            0x00000020      /* union with underlying filesystem */
#define    MNT_ASYNC            0x00000040      /* file system written asynchronously */
#define    MNT_CPROTECT         0x00000080      /* file system supports content protection */

#define    MNT_LOCAL            0x00001000      /* filesystem is stored locally */
#define    MNT_QUOTA            0x00002000      /* quotas are enabled on filesystem */
#define    MNT_ROOTFS           0x00004000      /* identifies the root filesystem */
#define    MNT_DOVOLFS          0x00008000      /* FS supports volfs (deprecated flag in Mac OS X 10.5) */

#define MNT_DONTBROWSE          0x00100000      /* file system is not appropriate path to user data */
#define MNT_IGNORE_OWNERSHIP    0x00200000      /* VFS will ignore ownership information on filesystem objects */
#define MNT_AUTOMOUNTED         0x00400000      /* filesystem was mounted by automounter */
#define MNT_JOURNALED           0x00800000      /* filesystem is journaled */
#define MNT_NOUSERXATTR         0x01000000      /* Don't allow user extended attributes */
#define MNT_DEFWRITE            0x02000000      /* filesystem should defer writes */
#define MNT_MULTILABEL          0x04000000      /* MAC support for individual labels */
#define MNT_NOATIME             0x10000000      /* disable update of file access time */
#define MNT_SNAPSHOT            0x40000000      /* The mount is a snapshot */


#endif /* lf_hfs_mount_h */
