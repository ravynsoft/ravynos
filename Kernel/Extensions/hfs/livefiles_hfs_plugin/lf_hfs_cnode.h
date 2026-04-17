/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_cnode.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#ifndef lf_hfs_cnode_h
#define lf_hfs_cnode_h

#include "lf_hfs_locks.h"
#include "lf_hfs_catalog.h"
#include "lf_hfs_rangelist.h"
#include "lf_hfs_vnode.h"
#include <sys/stat.h>

enum hfs_locktype {
    HFS_SHARED_LOCK = 1,
    HFS_EXCLUSIVE_LOCK = 2,
    HFS_TRY_EXCLUSIVE_LOCK = 3
};

/* Option flags for cnode and truncate lock functions */
enum hfs_lockflags {
    HFS_LOCK_DEFAULT           = 0x0,    /* Default flag, no options provided */
    HFS_LOCK_ALLOW_NOEXISTS    = 0x1,    /* Allow locking of all cnodes, including cnode marked deleted with no catalog entry */
    HFS_LOCK_SKIP_IF_EXCLUSIVE = 0x2,    /* Skip locking if the current thread already holds the lock exclusive */

    // Used when you do not want to check return from hfs_lock
    HFS_LOCK_ALWAYS            = HFS_LOCK_ALLOW_NOEXISTS,
};
#define HFS_SHARED_OWNER  (void *)0xffffffff

#define ZFTIMELIMIT    (5 * 60)

/* Zero-fill file and push regions out to disk */
enum {
    // Use this flag if you're going to sync later
    HFS_FILE_DONE_NO_SYNC     = 1,
};

/*
 * The filefork is used to represent an HFS file fork (data or resource).
 * Reading or writing any of these fields requires holding cnode lock.
 */
struct filefork {
    struct cnode    *ff_cp;                 /* cnode associated with this fork */
    struct rl_head  ff_invalidranges;       /* Areas of disk that should read back as zeroes */
    union {
        void        *ffu_sysfileinfo;       /* additional info for system files */
        char        *ffu_symlinkptr;        /* symbolic link pathname */
    } ff_union;
    struct cat_fork ff_data;                /* fork data (size, extents) */
};
typedef struct filefork filefork_t;

/* Aliases for common fields */
#define ff_size          ff_data.cf_size
#define ff_new_size      ff_data.cf_new_size
#define ff_clumpsize     ff_data.cf_clump
#define ff_bytesread     ff_data.cf_bytesread
#define ff_extents       ff_data.cf_extents

/*
 * Note that the blocks fields are protected by the cnode lock, *not*
 * the truncate lock.
 */
#define ff_blocks        ff_data.cf_blocks
#define ff_unallocblocks ff_data.cf_vblocks

#define ff_symlinkptr    ff_union.ffu_symlinkptr
#define ff_sysfileinfo   ff_union.ffu_sysfileinfo

/* The btree code still needs these... */
#define fcbEOF           ff_size
#define fcbExtents       ff_extents
#define fcbBTCBPtr       ff_sysfileinfo

typedef u_int8_t atomicflag_t;

/*
 * Hardlink Origin (for hardlinked directories).
 */
struct linkorigin {
    TAILQ_ENTRY(linkorigin)  lo_link;  /* chain */
    void *  lo_thread;      /* thread that performed the lookup */
    cnid_t  lo_cnid;        /* hardlink's cnid */
    cnid_t  lo_parentcnid;  /* hardlink's parent cnid */
};
typedef struct linkorigin linkorigin_t;

#define MAX_CACHED_ORIGINS  10
#define MAX_CACHED_FILE_ORIGINS 8

/*
 * The cnode is used to represent each active (or recently active)
 * file or directory in the HFS filesystem.
 *
 * Reading or writing any of these fields requires holding c_lock.
 */
struct cnode {
    pthread_rwlock_t                c_rwlock;                   /* cnode's lock */
    pthread_t                       c_lockowner;                /* cnode's lock owner (exclusive case only) */
    pthread_rwlock_t                c_truncatelock;             /* protects file from truncation during read/write */
    pthread_t                       c_truncatelockowner;        /* truncate lock owner (exclusive case only) */
    pthread_cond_t                  c_cacsh_cond;               /* cond for cnode cacsh*/
    
    LIST_ENTRY(cnode)               c_hash;                     /* cnode's hash chain */
    u_int32_t                       c_flag;                     /* cnode's runtime flags */
    u_int32_t                       c_hflag;                    /* cnode's flags for maintaining hash - protected by global hash lock */
    struct vnode                    *c_vp;                      /* vnode for data fork or dir */
    struct vnode                    *c_rsrc_vp;                 /* vnode for resource fork */
    u_int32_t                       c_childhint;                /* catalog hint for children (small dirs only) */
    u_int32_t                       c_dirthreadhint;            /* catalog hint for directory's thread rec */
    struct cat_desc                 c_desc;                     /* cnode's descriptor */
    struct cat_attr                 c_attr;                     /* cnode's attributes */
    TAILQ_HEAD(hfs_originhead, linkorigin)  c_originlist;       /* hardlink origin cache */
    TAILQ_HEAD(hfs_hinthead, directoryhint) c_hintlist;         /* readdir directory hint list */
    int16_t                         c_dirhinttag;               /* directory hint tag */
    union {
        int16_t                     cu_dirhintcnt;              /* directory hint count */
        int16_t                     cu_syslockcount;            /* system file use only */
    } c_union;
    u_int32_t                       c_dirchangecnt;             /* changes each insert/delete (in-core only) */
    struct filefork                 *c_datafork;                /* cnode's data fork */
    struct filefork                 *c_rsrcfork;                /* cnode's rsrc fork */
    atomicflag_t                    c_touch_acctime;
    atomicflag_t                    c_touch_chgtime;
    atomicflag_t                    c_touch_modtime;

    // The following flags are protected by the truncate lock
    union {
        struct {
            bool                    c_need_dvnode_put_after_truncate_unlock : 1;
            bool                    c_need_rvnode_put_after_truncate_unlock : 1;
        };
        uint8_t                     c_tflags;
    };

    /*
     * Where we're using a journal, we keep track of the last
     * transaction that we did an update in.  If a minor modification
     * is made, we'll still push it if we're still on the same
     * transaction.
     */
    uint32_t c_update_txn;

    volatile uint32_t  uOpenLookupRefCount;

};
typedef struct cnode cnode_t;

/* Aliases for common cnode fields */
#define c_cnid        c_desc.cd_cnid
#define c_hint        c_desc.cd_hint
#define c_parentcnid    c_desc.cd_parentcnid
#define c_encoding    c_desc.cd_encoding

#define c_fileid    c_attr.ca_fileid
#define c_mode        c_attr.ca_mode
#define c_linkcount    c_attr.ca_linkcount
#define c_uid        c_attr.ca_uid
#define c_gid        c_attr.ca_gid
#define c_rdev        c_attr.ca_union1.cau_rdev
#define c_atime        c_attr.ca_atime
#define c_mtime        c_attr.ca_mtime
#define c_ctime        c_attr.ca_ctime
#define c_itime        c_attr.ca_itime
#define c_btime        c_attr.ca_btime
#define c_bsdflags        c_attr.ca_bsdflags
#define c_finderinfo    c_attr.ca_finderinfo
#define c_blocks    c_attr.ca_union2.cau_blocks
#define c_entries    c_attr.ca_union2.cau_entries
#define c_zftimeout    c_childhint

#define c_dirhintcnt    c_union.cu_dirhintcnt
#define c_syslockcount  c_union.cu_syslockcount

/* hash maintenance flags kept in c_hflag and protected by hfs_chash_mutex */
#define H_ALLOC      0x00001    /* CNode is being allocated */
#define H_ATTACH     0x00002    /* CNode is being attached to by another vnode */
#define H_TRANSIT    0x00004    /* CNode is getting recycled  */
#define H_WAITING    0x00008    /* CNode is being waited for */

/*
 * Runtime cnode flags (kept in c_flag)
 */
#define C_NEED_RVNODE_PUT   0x0000001  /* Need to do a vnode_put on c_rsrc_vp after the unlock */
#define C_NEED_DVNODE_PUT   0x0000002  /* Need to do a vnode_put on c_vp after the unlock */
#define C_ZFWANTSYNC        0x0000004  /* fsync requested and file has holes */
#define C_FROMSYNC          0x0000008  /* fsync was called from sync */

#define C_MODIFIED          0x0000010  /* CNode has been modified */
#define C_NOEXISTS          0x0000020  /* CNode has been deleted, catalog entry is gone */
#define C_DELETED           0x0000040  /* CNode has been marked to be deleted */
#define C_HARDLINK          0x0000080  /* CNode is a hard link (file or dir) */

/*
 * A minor modification is one where the volume would not be inconsistent if
 * the change was not pushed to disk.  For example, changes to times.
 */
#define C_MINOR_MOD            0x0000100  /* CNode has a minor modification */

#define C_HASXATTRS         0x0000200  /* cnode has extended attributes */
/*
 * For C_SSD_STATIC: SSDs may want to deal with the file payload data in a
 * different manner knowing that the content is not likely to be modified. This is
 * purely advisory at the HFS level, and is not maintained after the cnode goes out of core.
 */
#define C_SSD_STATIC        0x0000800  /* Assume future writes contain static content */

#define C_NEED_DATA_SETSIZE 0x0001000  /* Do a ubc_setsize(0) on c_rsrc_vp after the unlock */
#define C_NEED_RSRC_SETSIZE 0x0002000  /* Do a ubc_setsize(0) on c_vp after the unlock */
#define C_DIR_MODIFICATION  0x0004000  /* Directory is being modified, wait for lookups */
#define C_ALWAYS_ZEROFILL   0x0008000  /* Always zero-fill the file on an fsync */

#define C_RENAMED           0x0010000  /* cnode was deleted as part of rename; C_DELETED should also be set */
#define C_NEEDS_DATEADDED   0x0020000  /* cnode needs date-added written to the finderinfo bit */
#define C_BACKINGSTORE      0x0040000  /* cnode is a backing store for an existing or currently-mounting filesystem */

/*
 * This flag indicates the cnode might be dirty because it
 * was mapped writable so if we get any page-outs, update
 * the modification and change times.
 */
#define C_MIGHT_BE_DIRTY_FROM_MAPPING   0x0080000

/*
 * Convert between cnode pointers and vnode pointers
 */
#define VTOC(vp)    ((struct cnode *) (vp)->sFSParams.vnfs_fsnode)

#define CTOV(cp,rsrc)    (((rsrc) && S_ISREG((cp)->c_mode)) ? \
                            (cp)->c_rsrc_vp : (cp)->c_vp)

/*
 * Convert between vnode pointers and file forks
 *
 * Note: no CTOF since that is ambiguous
 */

#define FTOC(fp)    ((fp)->ff_cp)

#define VTOF(vp)    ((vp) == VTOC((vp))->c_rsrc_vp ?    \
                        VTOC((vp))->c_rsrcfork :        \
                        VTOC((vp))->c_datafork)

#define VCTOF(vp, cp)       ((vp) == (cp)->c_rsrc_vp ?  \
                            (cp)->c_rsrcfork :          \
                            (cp)->c_datafork)

#define FTOV(fp)    ((fp) == FTOC(fp)->c_rsrcfork ?     \
                        FTOC(fp)->c_rsrc_vp :            \
                        FTOC(fp)->c_vp)
/*
 * Test for a resource fork
 */
#define FORK_IS_RSRC(fp)    ((fp) == FTOC(fp)->c_rsrcfork)

#define VNODE_IS_RSRC(vp)    ((vp) == VTOC((vp))->c_rsrc_vp)

/*
 * The following is the "invisible" bit from the fdFlags field
 * in the FndrFileInfo.
 */
enum { kFinderInvisibleMask = 1 << 14 };

/*
 * HFS cnode hash functions.
 */
void  hfs_chashinit(void);
void  hfs_chashinit_finish(struct hfsmount *hfsmp);
void  hfs_delete_chash(struct hfsmount *hfsmp);

/* Get new default vnode */
int hfs_getnewvnode(struct hfsmount *hfsmp, struct vnode *dvp, struct componentname *cnp, struct cat_desc *descp, int flags, struct cat_attr *attrp, struct cat_fork *forkp, struct vnode **vpp, int *out_flags);

#define ATIME_ONDISK_ACCURACY    300

static inline bool hfs_should_save_atime(cnode_t *cp)
{
    /*
     * We only write atime updates to disk if the delta is greater
     * than ATIME_ONDISK_ACCURACY.
     */
    return (cp->c_atime < cp->c_attr.ca_atimeondisk || cp->c_atime - cp->c_attr.ca_atimeondisk > ATIME_ONDISK_ACCURACY);
}

typedef enum {
    HFS_NOT_DIRTY   = 0,
    HFS_DIRTY       = 1,
    HFS_DIRTY_ATIME = 2
} hfs_dirty_t;


static inline hfs_dirty_t hfs_is_dirty(cnode_t *cp)
{
    if (ISSET(cp->c_flag, C_NOEXISTS))
        return HFS_NOT_DIRTY;

    if (ISSET(cp->c_flag, C_MODIFIED | C_MINOR_MOD | C_NEEDS_DATEADDED)
        || cp->c_touch_chgtime || cp->c_touch_modtime) {
        return HFS_DIRTY;
    }

    if (cp->c_touch_acctime || hfs_should_save_atime(cp))
        return HFS_DIRTY_ATIME;

    return HFS_NOT_DIRTY;
}

/*
 * Catalog Lookup struct (runtime)
 *
 * This is used so that when we need to malloc a container for a catalog
 * lookup operation, we can acquire memory for everything in one fell swoop
 * as opposed to putting many of these objects on the stack.  The cat_fork
 * data structure can take up 100+bytes easily, and that can add to stack
 * overhead.
 *
 * As a result, we use this to easily pass around the memory needed for a
 * lookup operation.
 */
#define HFS_TEMPLOOKUP_NAMELEN 32

struct cat_lookup_buffer {
    struct cat_desc lookup_desc;
    struct cat_attr lookup_attr;
    struct filefork lookup_fork;
    struct componentname lookup_cn;
    char lookup_name[HFS_TEMPLOOKUP_NAMELEN]; /* for open-unlinked paths only */
};

/* Input flags for hfs_getnewvnode */

#define GNV_WANTRSRC   0x01  /* Request the resource fork vnode. */
#define GNV_SKIPLOCK   0x02  /* Skip taking the cnode lock (when getting resource fork). */
#define GNV_CREATE     0x04  /* The vnode is for a newly created item. */
#define GNV_NOCACHE       0x08  /* Delay entering this item in the name cache */
#define GNV_USE_VP     0x10  /* Use the vnode provided in *vpp instead of creating a new one */

/* Output flags for hfs_getnewvnode */

#define GNV_CHASH_RENAMED    0x01    /* The cnode was renamed in-flight */
#define GNV_CAT_DELETED        0x02    /* The cnode was deleted from the catalog */
#define GNV_NEW_CNODE        0x04    /* We are vending out a newly initialized cnode */
#define GNV_CAT_ATTRCHANGED    0x08    /* Something in struct cat_attr changed in between cat_lookups */


int hfs_valid_cnode(struct hfsmount *hfsmp, struct vnode *dvp, struct componentname *cnp, cnid_t cnid, struct cat_attr *cattr, int *error);
int hfs_lock(struct cnode *cp, enum hfs_locktype locktype, enum hfs_lockflags flags);
void hfs_unlock(struct cnode *cp);
void hfs_lock_truncate(struct cnode *cp, enum hfs_locktype locktype, enum hfs_lockflags flags);
void hfs_unlock_truncate(struct cnode *cp, enum hfs_lockflags flags);
int hfs_lockpair(struct cnode *cp1, struct cnode *cp2, enum hfs_locktype locktype);
void hfs_unlockpair(struct cnode *cp1, struct cnode *cp2);
int  hfs_lockfour(struct cnode *cp1, struct cnode *cp2, struct cnode *cp3, struct cnode *cp4, enum hfs_locktype locktype, struct cnode **error_cnode);
void hfs_unlockfour(struct cnode *cp1, struct cnode *cp2, struct cnode *cp3, struct cnode *cp4);
uint32_t hfs_incr_gencount (struct cnode *cp);
void hfs_clear_might_be_dirty_flag(cnode_t *cp);
void hfs_write_dateadded (struct cat_attr *attrp, uint64_t dateadded);
u_int32_t hfs_get_dateadded(struct cnode *cp);
void hfs_touchtimes(struct hfsmount *hfsmp, struct cnode* cp);
void hfs_write_gencount (struct cat_attr *attrp, uint32_t gencount);
int  hfs_vnop_reclaim(struct vnode *vp);
#endif /* lf_hfs_cnode_h */
