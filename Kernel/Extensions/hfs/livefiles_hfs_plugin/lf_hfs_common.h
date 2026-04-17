/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_common.h
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 31/12/2017.
 */

#ifndef lf_hfs_common_h
#define lf_hfs_common_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#import  <os/log.h>
#include <err.h>
#include <sys/errno.h>
#include <UserFS/UserVFS.h>

#define LF_HFS_CHECK_UNMAPPED           0
#define LF_HFS_QOUTA_SUPPORT            0
#define LF_HFS_FULL_VNODE_SUPPORT       0
#define LF_HFS_NATIVE_SEARCHFS_SUPPORT  0

#define min MIN
#define max MAX

/* pseudo-errors returned inside kernel to modify return to process */
#define ERESTART        (-1)        /* restart syscall */
#define EJUSTRETURN     (-2)        /* don't modify regs, just return */
#define ERECYCLE        (-5)        /* restart lookup under heavy vnode pressure/recycling */
#define EREDRIVEOPEN    (-6)
#define EKEEPLOOKING    (-7)

typedef struct
{
    int      iFD;         // File descriptor as received from usbstoraged
    unsigned uUnmountHint; // Unmount hint (passed on in LFHFS_UNMOUNT, cleared on LFHFS_MOUNT)
} FileSystemRecord_s;

#define    VPTOFSRECORD(vp) (vp->sFSParams.vnfs_mp->psHfsmount->hfs_devvp->psFSRecord)

#define    VNODE_TO_IFD(vp)          ((vp->bIsMountVnode)? (vp->psFSRecord->iFD)          : ((VPTOFSRECORD(vp))->iFD))
#define    VNODE_TO_UNMOUNT_HINT(vp) ((vp->bIsMountVnode)? (vp->psFSRecord->uUnmountHint) : ((VPTOFSRECORD(vp))->uUnmountHint))

/* Macros to clear/set/test flags. */
#define    SET(t, f)    (t) |= (f)
#define    CLR(t, f)    (t) &= ~(f)
#define    ISSET(t, f)    ((t) & (f))

#undef ROUND_DOWN
#define ROUND_DOWN(_x, _m)    (((_x) / (_m)) * (_m))

#undef ROUND_UP
#define ROUND_UP(_x, _m)      ROUND_DOWN((_x) + (_m) - 1, (_m))

struct HFSUniStr255 {
    u_int16_t    length;        /* number of unicode characters */
    u_int16_t    unicode[255];    /* unicode characters */
} __attribute__((aligned(2), packed));
typedef struct HFSUniStr255 HFSUniStr255;
typedef const HFSUniStr255 *ConstHFSUniStr255Param;

struct hfsmount;

#define    B_LOCKED      0x00000010

#define    RDONLY        0x00000200 /* lookup with read-only semantics */
#define    HASBUF        0x00000400 /* has allocated pathname buffer */
#define    SAVENAME      0x00000800 /* save pathname buffer */
#define    SAVESTART     0x00001000 /* save starting directory */
#define    ISDOTDOT      0x00002000 /* current component name is .. */
#define    MAKEENTRY     0x00004000 /* entry is to be added to name cache */
#define    ISLASTCN      0x00008000 /* this is last component of pathname */
#define    ISSYMLINK     0x00010000 /* symlink needs interpretation */
#define    ISWHITEOUT    0x00020000 /* found whiteout */
#define    DOWHITEOUT    0x00040000 /* do whiteouts */
#define    WILLBEDIR     0x00080000 /* new files will be dirs; allow trailing / */
#define    ISUNICODE     0x00100000 /* current component name is unicode*/
#define    ISOPEN        0x00200000 /* caller is opening; return a real vnode. */
#define    NOCROSSMOUNT  0x00400000 /* do not cross mount points */
#define    NOMACCHECK    0x00800000 /* do not perform MAC checks */
#define    AUDITVNODE1   0x04000000 /* audit the looked up vnode information */
#define    AUDITVNODE2   0x08000000 /* audit the looked up vnode information */
#define    TRAILINGSLASH 0x10000000 /* path ended in a slash */
#define    NOCAPCHECK    0x20000000 /* do not perform capability checks */
#define    PARAMASK      0x3ffffe00 /* mask of parameter descriptors */

/*
 * component name operations (for VNOP_LOOKUP)
 */
#define    LOOKUP        0    /* perform name lookup only */
#define    CREATE        1    /* setup for file creation */
#define    DELETE        2    /* setup for file deletion */
#define    RENAME        3    /* setup for file renaming */
#define    OPMASK        3    /* mask for operation */

#define ALL_UVFS_MODES (UVFS_FA_MODE_OTH(UVFS_FA_MODE_RWX) | UVFS_FA_MODE_GRP(UVFS_FA_MODE_RWX) | UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX))

#define    UF_NODUMP    0x00000001    /* do not dump file */

#define VALID_NODE_MAGIC        (0xC0BEC0BE)
#define VALID_NODE_BADMAGIC     (0xDEADDABA)
#define INVALIDATE_NODE(psNodeRecord)                                                   \
    do {                                                                                \
        if ( psNodeRecord != NULL ) {                                                   \
            ((vnode_t)psNodeRecord)->uValidNodeMagic1 = VALID_NODE_BADMAGIC;      \
            ((vnode_t)psNodeRecord)->uValidNodeMagic2 = VALID_NODE_BADMAGIC;      \
        }                                                                               \
    } while(0)
#define SET_NODE_AS_VALID(psNodeRecord)                                             \
    do {                                                                            \
        if ( psNodeRecord != NULL ) {                                               \
            ((vnode_t)psNodeRecord)->uValidNodeMagic1 = VALID_NODE_MAGIC;     \
            ((vnode_t)psNodeRecord)->uValidNodeMagic2 = VALID_NODE_MAGIC;     \
        }                                                                           \
    } while(0)
#define VERIFY_NODE_IS_VALID(psNodeRecord)                                              \
    do {                                                                                \
        if ((psNodeRecord) &&                                                           \
            ((vnode_t)psNodeRecord)->uValidNodeMagic1 == VALID_NODE_BADMAGIC &&   \
            ((vnode_t)psNodeRecord)->uValidNodeMagic2 == VALID_NODE_BADMAGIC ) {  \
            LFHFS_LOG( LEVEL_ERROR, "[%s] Got stale node", __FUNCTION__ );                                 \
            return ESTALE;                                                              \
        }                                                                               \
        if ((psNodeRecord == NULL) ||                                                   \
            ((vnode_t)psNodeRecord)->uValidNodeMagic1 != VALID_NODE_MAGIC ||      \
            ((vnode_t)psNodeRecord)->uValidNodeMagic2 != VALID_NODE_MAGIC ) {     \
            LFHFS_LOG( LEVEL_ERROR, "[%s] Got invalid node", __FUNCTION__ );                               \
            return EINVAL;                                                              \
        }                                                                               \
    } while(0)
#define VERIFY_NODE_IS_VALID_FOR_RECLAIM(psNodeRecord)                                                  \
    do {                                                                                                \
        if ((psNodeRecord == NULL)                                                          ||          \
            (   ((vnode_t)psNodeRecord)->uValidNodeMagic1 != VALID_NODE_MAGIC &&                  \
                ((vnode_t)psNodeRecord)->uValidNodeMagic1 != VALID_NODE_BADMAGIC )    ||          \
            (   ((vnode_t)psNodeRecord)->uValidNodeMagic2 != VALID_NODE_MAGIC &&                  \
                ((vnode_t)psNodeRecord)->uValidNodeMagic2 != VALID_NODE_BADMAGIC )) {             \
            LFHFS_LOG( LEVEL_ERROR, "Got invalid node for reclaim" );                                   \
            return EINVAL;                                                                              \
        }                                                                                               \
    } while(0)

#define ALL_UVFS_MODES (UVFS_FA_MODE_OTH(UVFS_FA_MODE_RWX) | UVFS_FA_MODE_GRP(UVFS_FA_MODE_RWX) | UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX))
#define UVFS_READ_ONLY (UVFS_FA_MODE_OTH(UVFS_FA_MODE_R | UVFS_FA_MODE_X) | UVFS_FA_MODE_GRP(UVFS_FA_MODE_R | UVFS_FA_MODE_X) | UVFS_FA_MODE_USR(UVFS_FA_MODE_R | UVFS_FA_MODE_X))


#endif /* lf_hfs_common_h */
