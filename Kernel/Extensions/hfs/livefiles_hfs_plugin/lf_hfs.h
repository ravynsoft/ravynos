/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#ifndef lf_hfs_h
#define lf_hfs_h

#include <sys/kernel_types.h>
#include "lf_hfs_locks.h"
#include "lf_hfs_format.h"
#include "lf_hfs_catalog.h"
#include "lf_hfs_rangelist.h"
#include "lf_hfs_cnode.h"
#include "lf_hfs_defs.h"


#define HFS_MAX_DEFERED_ALLOC    (1024*1024)

#define HFS_MAX_FILES    (UINT32_MAX - kHFSFirstUserCatalogNodeID)

// 400 megs is a "big" file (i.e. one that when deleted
// would touch enough data that we should break it into
// multiple separate transactions)
#define HFS_BIGFILE_SIZE (400LL * 1024LL * 1024LL)

enum { kMDBSize = 512 };                                /* Size of I/O transfer to read entire MDB */

enum { kMasterDirectoryBlock = 2 };                     /* MDB offset on disk in 512-byte blocks */
enum { kMDBOffset = kMasterDirectoryBlock * 512 };      /* MDB offset on disk in bytes */

#define kRootDirID kHFSRootFolderID

/* How many free extents to cache per volume */
#define kMaxFreeExtents        10

/* Maximum file size that we're willing to defrag on open */
#define HFS_MAX_DEFRAG_SIZE         (104857600)     // 100 * 1024 * 1024 (100MB)
#define HFS_INITIAL_DEFRAG_SIZE     (20971520)      // 20 * 1024 * 1024 (20MB)

#define HFS_AVERAGE_NAME_SIZE    22
#define AVERAGE_HFSDIRENTRY_SIZE  (8+HFS_AVERAGE_NAME_SIZE+4)

/*
 * HFS_MINFREE gives the minimum acceptable percentage
 * of file system blocks which may be free (but this
 * minimum will never exceed HFS_MAXRESERVE bytes). If
 * the free block count drops below this level only the
 * superuser may continue to allocate blocks.
 */
#define HFS_MINFREE             (1)
#define HFS_MAXRESERVE          ((u_int64_t)(250*1024*1024))
#define HFS_BT_MAXRESERVE       ((u_int64_t)(10*1024*1024))

/*
 * HFS_META_DELAY is a duration (in usecs) used for triggering the
 * hfs_syncer() routine. We will back off if writes are in
 * progress, but...
 * HFS_MAX_META_DELAY is the maximum time we will allow the
 * syncer to be delayed.
 */
enum {
    HFS_META_DELAY     = 100  * 1000,   // 0.1 secs
    //HFS_META_DELAY     = 10  * 1000,   // 0.01 secs
    HFS_MAX_META_DELAY = 5000 * 1000    //   5 secs
    //HFS_MAX_META_DELAY = 1000 * 1000    //   1 secs
};

#define HFS_META_DELAY_TS ((struct timespec){ 0, HFS_META_DELAY * NSEC_PER_USEC })


/* This structure describes the HFS specific mount structure data. */
typedef struct hfsmount {
    u_int32_t     hfs_flags;              /* see below */

    /* Physical Description */
    u_int32_t     hfs_logical_block_size;    /* Logical block size of the disk as reported by ioctl(DKIOCGETBLOCKSIZE), always a multiple of 512 */
    daddr64_t     hfs_logical_block_count;  /* Number of logical blocks on the disk, as reported by ioctl(DKIOCGETBLOCKCOUNT) */
    u_int64_t     hfs_logical_bytes;    /* Number of bytes on the disk device this HFS is mounted on (blockcount * blocksize) */
    /*
     * Regarding the two AVH sector fields below:
     * Under normal circumstances, the filesystem's notion of the "right" location for the AVH is such that
     * the partition and filesystem's are in sync.  However, during a filesystem resize, HFS proactively
     * writes a new AVH at the end of the filesystem, assuming that the partition will be resized accordingly.
     *
     * However, it is not technically a corruption if the partition size is never modified.  As a result, we need
     * to keep two copies of the AVH around "just in case" the partition size is not modified.
     */
    daddr64_t    hfs_partition_avh_sector;    /* location of Alt VH w.r.t partition size */
    daddr64_t    hfs_fs_avh_sector;           /* location of Alt VH w.r.t filesystem size */

    u_int32_t     hfs_physical_block_size;    /* Physical block size of the disk as reported by ioctl(DKIOCGETPHYSICALBLOCKSIZE) */
    u_int32_t     hfs_log_per_phys;           /* Number of logical blocks per physical block size */

    /* Access to VFS and devices */
    struct mount *   hfs_mp;                  /* filesystem vfs structure */
    struct vnode *   hfs_devvp;               /* block device mounted vnode */
    struct vnode *   hfs_extents_vp;
    struct vnode *   hfs_catalog_vp;
    struct vnode *   hfs_allocation_vp;
    struct vnode *   hfs_attribute_vp;
    struct vnode *   hfs_startup_vp;
    struct vnode *   hfs_attrdata_vp;         /* pseudo file */
    struct cnode *   hfs_extents_cp;
    struct cnode *   hfs_catalog_cp;
    struct cnode *   hfs_allocation_cp;
    struct cnode *   hfs_attribute_cp;
    struct cnode *   hfs_startup_cp;
    dev_t            hfs_raw_dev;           /* device mounted */
    u_int32_t        hfs_logBlockSize;      /* Size of buffer cache buffer for I/O */

    /* Default values for HFS standard and non-init access */
    uid_t         hfs_uid;            /* uid to set as owner of the files */
    gid_t         hfs_gid;            /* gid to set as owner of the files */
    mode_t        hfs_dir_mask;       /* mask to and with directory protection bits */
    mode_t        hfs_file_mask;      /* mask to and with file protection bits */
    u_int32_t     hfs_encoding;       /* Default encoding for non hfs+ volumes */

    /* Persistent fields (on disk, dynamic) */
    time_t        hfs_mtime;          /* file system last modification time */
    u_int32_t     hfs_filecount;      /* number of files in file system */
    u_int32_t     hfs_dircount;       /* number of directories in file system */
    u_int32_t     freeBlocks;            /* free allocation blocks */
    u_int32_t     reclaimBlocks;      /* number of blocks we are reclaiming during resize */
    u_int32_t     tentativeBlocks;      /* tentative allocation blocks -- see note below */
    u_int32_t     nextAllocation;      /* start of next allocation search */
    u_int32_t     sparseAllocation;   /* start of allocations for sparse devices */
    u_int32_t     vcbNxtCNID;         /* next unused catalog node ID - protected by catalog lock */
    u_int32_t     vcbWrCnt;           /* file system write count */
    u_int64_t     encodingsBitmap;    /* in-use encodings */
    u_int16_t     vcbNmFls;           /* HFS Only - root dir file count */
    u_int16_t     vcbNmRtDirs;        /* HFS Only - root dir directory count */

    /* Persistent fields (on disk, static) */
    u_int16_t             vcbSigWord;

    // Volume will be inconsistent if header is not flushed
    bool                hfs_header_dirty;

    // Volume header is dirty, but won't be inconsistent if not flushed
    bool                hfs_header_minor_change;

    u_int32_t             vcbAtrb;
    u_int32_t             vcbJinfoBlock;
    u_int32_t             localCreateDate;/* volume create time from volume header (For HFS+, value is in local time) */
    time_t                hfs_itime;    /* file system creation time (creation date of the root folder) */
    time_t                hfs_btime;    /* file system last backup time */
    u_int32_t             blockSize;    /* size of allocation blocks */
    u_int32_t             totalBlocks;    /* total allocation blocks */
    u_int32_t            allocLimit;    /* Do not allocate this block or beyond */
    /*
     * NOTE: When resizing a volume to make it smaller, allocLimit is set to the allocation
     * block number which will contain the new alternate volume header.  At all other times,
     * allocLimit is set to totalBlocks.  The allocation code uses allocLimit instead of
     * totalBlocks to limit which blocks may be allocated, so that during a resize, we don't
     * put new content into the blocks we're trying to truncate away.
     */
    int32_t             vcbClpSiz;
    u_int32_t     vcbFndrInfo[8];
    int16_t             vcbVBMSt;        /* HFS only */
    int16_t             vcbAlBlSt;        /* HFS only */

    /* vcb stuff */
    u_int8_t             vcbVN[256];        /* volume name in UTF-8 */
    u_int32_t             volumeNameEncodingHint;
    u_int32_t             hfsPlusIOPosOffset;    /* Disk block where HFS+ starts */
    u_int32_t             vcbVBMIOSize;        /* volume bitmap I/O size */

    /* cache of largest known free extents */
    u_int32_t            vcbFreeExtCnt;
    HFSPlusExtentDescriptor vcbFreeExt[kMaxFreeExtents];
    pthread_mutex_t            vcbFreeExtLock;

    /* Summary Table */
    u_int8_t            *hfs_summary_table; /* Each bit is 1 vcbVBMIOSize of bitmap, byte indexed */
    u_int32_t            hfs_summary_size;    /* number of BITS in summary table defined above (not bytes!) */
    u_int32_t            hfs_summary_bytes;    /* number of BYTES in summary table */

    u_int32_t             scan_var;            /* For initializing the summary table */


    u_int32_t        reserveBlocks;        /* free block reserve */
    u_int32_t        loanedBlocks;        /* blocks on loan for delayed allocations */
    u_int32_t        lockedBlocks;        /* blocks reserved and locked */

    /*
     * HFS+ Private system directories (two). Any access
     * (besides looking at the cd_cnid) requires holding
     * the Catalog File lock.
     */
    struct cat_desc     hfs_private_desc[2];
    struct cat_attr     hfs_private_attr[2];

    u_int32_t        hfs_metadata_createdate;

    /* Journaling variables: */
    struct journal      *jnl;           // the journal for this volume (if one exists)
    struct vnode        *jvp;           // device where the journal lives (may be equal to devvp)
    u_int64_t            jnl_start;     // start block of the journal file (so we don't delete it)
    u_int64_t            jnl_size;
    u_int64_t            hfs_jnlfileid;
    u_int64_t            hfs_jnlinfoblkid;
    pthread_rwlock_t     hfs_global_lock;
    pthread_t             hfs_global_lockowner;
    u_int32_t            hfs_transaction_nesting;

    /*
     * Notification variables
     * See comments in hfs mount code for what the
     * default levels are set to.
     */
    u_int32_t        hfs_notification_conditions;
    u_int32_t        hfs_freespace_notify_dangerlimit;
    u_int32_t        hfs_freespace_notify_warninglimit;
    u_int32_t        hfs_freespace_notify_nearwarninglimit;
    u_int32_t        hfs_freespace_notify_desiredlevel;

    /* time mounted and last mounted mod time "snapshot" */
    time_t        hfs_mount_time;
    time_t        hfs_last_mounted_mtime;

    /* Metadata allocation zone variables: */
    u_int32_t    hfs_metazone_start;
    u_int32_t    hfs_metazone_end;
    u_int32_t       hfs_min_alloc_start;
    u_int32_t       hfs_freed_block_count;
    int        hfs_overflow_maxblks;
    int        hfs_catalog_maxblks;
    
    /* defrag-on-open variables */
    int        hfs_defrag_nowait;  //issue defrags now, regardless of whether or not we've gone past 3 min.
    uint64_t        hfs_defrag_max;    //maximum file size we'll defragment on this mount

#if HFS_SPARSE_DEV
    /* Sparse device variables: */
    struct vnode * hfs_backingvp;
    u_int32_t      hfs_last_backingstatfs;
    u_int32_t      hfs_sparsebandblks;
    u_int64_t      hfs_backingfs_maxblocks;
#endif
    size_t         hfs_max_inline_attrsize;

    pthread_mutex_t      hfs_mutex;      /* protects access to hfsmount data */
    pthread_mutex_t      sync_mutex;     
    
    enum {
        HFS_THAWED,
        HFS_WANT_TO_FREEZE,    // This state stops hfs_sync from starting
        HFS_FREEZING,        // We're in this state whilst we're flushing
        HFS_FROZEN            // Everything gets blocked in hfs_lock_global
    } hfs_freeze_state;
    union {
        /*
         * When we're freezing (HFS_FREEZING) but not yet
         * frozen (HFS_FROZEN), we record the freezing thread
         * so that we stop other threads from taking locks,
         * but allow the freezing thread.
         */
        pthread_t hfs_freezing_thread;
        /*
         * Once we have frozen (HFS_FROZEN), we record the
         * process so that if it dies, we can automatically
         * unfreeze.
         */
        proc_t hfs_freezing_proc;
    };

    pthread_t        hfs_downgrading_thread; /* thread who's downgrading to rdonly */

    /* Resize variables: */
    u_int32_t        hfs_resize_blocksmoved;
    u_int32_t        hfs_resize_totalblocks;
    u_int32_t        hfs_resize_progress;

    /* the full UUID of the volume, not the one stored in finderinfo */
    uuid_t         hfs_full_uuid;

    /* Per mount cnode hash variables: */
    pthread_mutex_t      hfs_chash_mutex;  /* protects access to cnode hash table */
    u_long               hfs_cnodehash;    /* size of cnode hash table - 1 */
    LIST_HEAD(cnodehashhead, cnode) *hfs_cnodehashtbl;    /* base of cnode hash */

    /* Per mount fileid hash variables  (protected by catalog lock!) */
    u_long hfs_idhash; /* size of cnid/fileid hash table -1 */
    LIST_HEAD(idhashhead, cat_preflightid) *hfs_idhashtbl; /* base of ID hash */

    // Records the oldest outstanding sync request
    struct timeval    hfs_sync_req_oldest;

    /* Records the syncer thread so that we can avoid the syncer
     queing more syncs. */
    pthread_t        hfs_syncer_thread;

    // Not currently used except for debugging purposes
    // Since we pass this to OSAddAtomic, this needs to be 4-byte aligned.
    uint32_t        hfs_active_threads;

    enum {
        // These are indices into the array below

        // Tentative ranges can be claimed back at any time
        HFS_TENTATIVE_BLOCKS    = 0,

        // Locked ranges cannot be claimed back, but the allocation
        // won't have been written to disk yet
        HFS_LOCKED_BLOCKS        = 1,
    };
    // These lists are not sorted like a range list usually is
    struct rl_head hfs_reserved_ranges[2];

    //General counter of link id
    int cur_link_id;

} hfsmount_t;

typedef hfsmount_t  ExtendedVCB;


/* Aliases for legacy (Mac OS 9) field names */
#define vcbLsMod           hfs_mtime
#define vcbVolBkUp         hfs_btime
#define extentsRefNum      hfs_extents_vp
#define catalogRefNum      hfs_catalog_vp
#define allocationsRefNum  hfs_allocation_vp
#define vcbFilCnt          hfs_filecount
#define vcbDirCnt          hfs_dircount

static inline void MarkVCBDirty(hfsmount_t *hfsmp)
{
    hfsmp->hfs_header_dirty = true;
}

static inline void MarkVCBClean(hfsmount_t *hfsmp)
{
    hfsmp->hfs_header_dirty = false;
    hfsmp->hfs_header_minor_change = false;
}

static inline bool IsVCBDirty(ExtendedVCB *vcb)
{
    return vcb->hfs_header_minor_change || vcb->hfs_header_dirty;
}

// Header is changed but won't be inconsistent if we don't write it
static inline void hfs_note_header_minor_change(hfsmount_t *hfsmp)
{
    hfsmp->hfs_header_minor_change = true;
}

// Must header be flushed for volume to be consistent?
static inline bool hfs_header_needs_flushing(hfsmount_t *hfsmp)
{
    return (hfsmp->hfs_header_dirty
            || ISSET(hfsmp->hfs_catalog_cp->c_flag, C_MODIFIED)
            || ISSET(hfsmp->hfs_extents_cp->c_flag, C_MODIFIED)
            || (hfsmp->hfs_attribute_cp
                && ISSET(hfsmp->hfs_attribute_cp->c_flag, C_MODIFIED))
            || (hfsmp->hfs_allocation_cp
                && ISSET(hfsmp->hfs_allocation_cp->c_flag, C_MODIFIED))
            || (hfsmp->hfs_startup_cp
                && ISSET(hfsmp->hfs_startup_cp->c_flag, C_MODIFIED)));
}

enum privdirtype {
    FILE_HARDLINKS,
    DIR_HARDLINKS
};

#define HFS_ALLOCATOR_SCAN_INFLIGHT     0x0001      /* scan started */
#define HFS_ALLOCATOR_SCAN_COMPLETED    0x0002      /* initial scan was completed */

/* HFS mount point flags */
#define HFS_READ_ONLY             0x00001
#define HFS_UNKNOWN_PERMS         0x00002
#define HFS_WRITEABLE_MEDIA       0x00004
#define HFS_CLEANED_ORPHANS       0x00008
#define HFS_X                     0x00010
#define HFS_CASE_SENSITIVE        0x00020
//#define HFS_STANDARD              0x00040
#define HFS_METADATA_ZONE         0x00080
#define HFS_FRAGMENTED_FREESPACE  0x00100
#define HFS_NEED_JNL_RESET        0x00200
//#define HFS_HAS_SPARSE_DEVICE     0x00400
#define HFS_RESIZE_IN_PROGRESS    0x00800
#define HFS_QUOTAS                0x01000
#define HFS_CREATING_BTREE        0x02000
/* When set, do not update nextAllocation in the mount structure */
#define HFS_SKIP_UPDATE_NEXT_ALLOCATION 0x04000
/* When set, the file system supports extent-based extended attributes */
#define HFS_XATTR_EXTENTS         0x08000
#define    HFS_FOLDERCOUNT           0x10000
/* When set, the file system exists on a virtual device, like disk image */
//#define HFS_VIRTUAL_DEVICE        0x20000
/* When set, we're in hfs_changefs, so hfs_sync should do nothing. */
#define HFS_IN_CHANGEFS           0x40000
/* When set, we are in process of downgrading or have downgraded to read-only,
 * so hfs_start_transaction should return EROFS.
 */
#define HFS_RDONLY_DOWNGRADE      0x80000
#define HFS_DID_CONTIG_SCAN      0x100000
#define HFS_UNMAP                0x200000
//#define HFS_SSD                  0x400000
#define HFS_SUMMARY_TABLE        0x800000
//#define HFS_CS                  0x1000000
//#define HFS_CS_METADATA_PIN     0x2000000
#define HFS_FEATURE_BARRIER     0x8000000    /* device supports barrier-only flush */
//#define HFS_CS_SWAPFILE_PIN    0x10000000

/* Macro to update next allocation block in the HFS mount structure.  If
 * the HFS_SKIP_UPDATE_NEXT_ALLOCATION is set, do not update
 * nextAllocation block.
 */
#define HFS_UPDATE_NEXT_ALLOCATION(hfsmp, new_nextAllocation)       \
{                                                                   \
    if ((hfsmp->hfs_flags & HFS_SKIP_UPDATE_NEXT_ALLOCATION) == 0)  \
        hfsmp->nextAllocation = new_nextAllocation;                 \
}

/* Macro for incrementing and decrementing the folder count in a cnode
 * attribute only if the HFS_FOLDERCOUNT bit is set in the mount flags
 * and kHFSHasFolderCount bit is set in the cnode flags.  Currently these
 * bits are only set for case sensitive HFS+ volumes.
 */
#define INC_FOLDERCOUNT(hfsmp, cattr)                           \
            if ((hfsmp->hfs_flags & HFS_FOLDERCOUNT) &&         \
            (cattr.ca_recflags & kHFSHasFolderCountMask))       \
            {                                                   \
                cattr.ca_dircount++;                            \
            }                                                   \

#define DEC_FOLDERCOUNT(hfsmp, cattr)                           \
            if ((hfsmp->hfs_flags & HFS_FOLDERCOUNT) &&         \
            (cattr.ca_recflags & kHFSHasFolderCountMask) &&     \
            (cattr.ca_dircount > 0))                            \
            {                                                   \
                cattr.ca_dircount--;                            \
            }                                                   \

typedef struct filefork FCB;

/*
 * Macros for creating item names for our special/private directories.
 */
#define MAKE_INODE_NAME(name, size, linkno) \
(void) snprintf((name), size, "%s%d", HFS_INODE_PREFIX, (linkno))
#define HFS_INODE_PREFIX_LEN    5

#define MAKE_DIRINODE_NAME(name, size, linkno) \
(void) snprintf((name), size, "%s%d", HFS_DIRINODE_PREFIX, (linkno))
#define HFS_DIRINODE_PREFIX_LEN   4

#define MAKE_DELETED_NAME(NAME, size, FID) \
(void) snprintf((NAME), size, "%s%d", HFS_DELETE_PREFIX, (FID))
#define HFS_DELETE_PREFIX_LEN    4

enum { kHFSPlusMaxFileNameBytes = kHFSPlusMaxFileNameChars * 3 };


/* macro to determine if hfs or hfsplus */
#define ISHFSPLUS(VCB) ((VCB)->vcbSigWord == kHFSPlusSigWord)

/*
 * Various ways to acquire a VFS mount point pointer:
 */
#define VTOVFS(VP)          (vp->sFSParams.vnfs_mp)
#define HFSTOVFS(HFSMP)     ((HFSMP)->hfs_mp)
#define VCBTOVFS(VCB)       (HFSTOVFS(VCB))

/*
 * Various ways to acquire an HFS mount point pointer:
 */
#define VTOHFS(vp)      ((struct hfsmount *)(vp->sFSParams.vnfs_mp->psHfsmount))
#define VFSTOHFS(mp)    ((struct hfsmount *)(mp->psHfsmount))
#define VCBTOHFS(vcb)   (vcb)
#define FCBTOHFS(fcb)   ((struct hfsmount *)((vnode_mount((fcb)->ff_cp->c_vp))->psHfsmount))

/*
 * Various ways to acquire a VCB (legacy) pointer:
 */
#define VTOVCB(VP)       (VTOHFS(VP))
#define VFSTOVCB(MP)     (VFSTOHFS(MP))
#define HFSTOVCB(HFSMP)  (HFSMP)
#define FCBTOVCB(FCB)    (FCBTOHFS(FCB))

#define E_NONE          (0)
#define kHFSBlockSize   (512)

/*
 * Macros for getting the MDB/VH sector and offset
 */
#define HFS_PRI_SECTOR(blksize)          (1024 / (blksize))
#define HFS_PRI_OFFSET(blksize)          ((blksize) > 1024 ? 1024 : 0)

#define HFS_ALT_SECTOR(blksize, blkcnt)  (((blkcnt) - 1) - (512 / (blksize)))
#define HFS_ALT_OFFSET(blksize)          ((blksize) > 1024 ? (blksize) - 1024 : 0)

#define HFS_PHYSBLK_ROUNDDOWN(sector_num, log_per_phys)    ((sector_num / log_per_phys) * log_per_phys)

/* HFS System file locking */
#define SFL_CATALOG     0x0001
#define SFL_EXTENTS     0x0002
#define SFL_BITMAP      0x0004
#define SFL_ATTRIBUTE   0x0008
#define SFL_STARTUP    0x0010
#define SFL_VM_PRIV    0x0020
#define SFL_VALIDMASK   (SFL_CATALOG | SFL_EXTENTS | SFL_BITMAP | SFL_ATTRIBUTE | SFL_STARTUP | SFL_VM_PRIV)

/* If a runtime corruption is detected, mark the volume inconsistent
 * bit in the volume attributes.
 */
typedef enum {
    HFS_INCONSISTENCY_DETECTED,
    // Used when unable to rollback an operation that failed
    HFS_ROLLBACK_FAILED,
    // Used when the latter part of an operation failed, but we chose not to roll back
    HFS_OP_INCOMPLETE,
    // Used when someone told us to force an fsck on next mount
    HFS_FSCK_FORCED,
} hfs_inconsistency_reason_t;

#define HFS_ERESERVEDNAME        (-8)

typedef enum hfs_sync_mode {
    HFS_FSYNC,
    HFS_FSYNC_FULL,
    HFS_FSYNC_BARRIER
} hfs_fsync_mode_t;

typedef enum hfs_flush_mode {
    HFS_FLUSH_JOURNAL,              // Flush journal
    HFS_FLUSH_JOURNAL_META,         // Flush journal and metadata blocks
    HFS_FLUSH_FULL,                 // Flush journal and does a cache flush
    HFS_FLUSH_CACHE,                // Flush track cache to media
    HFS_FLUSH_BARRIER,              // Barrier-only flush to ensure write order
    HFS_FLUSH_JOURNAL_BARRIER       // Flush journal with barrier
} hfs_flush_mode_t;

/* Number of bits used to represent maximum extended attribute size */
#define HFS_XATTR_SIZE_BITS    31

#define HFS_LINK_MAX    32767

typedef enum {
    // Push all modifications to disk (including minor ones)
    HFS_UPDATE_FORCE = 0x01,
} hfs_update_options_t;

/*
 * Maximum extended attribute size supported for all extended attributes except
 * resource fork and finder info.
 */
#define HFS_XATTR_MAXSIZE    INT32_MAX

#if DEBUG
    #define HFS_CRASH_TEST 1
#else
    #define HFS_CRASH_TEST 0
#endif

#if HFS_CRASH_TEST
typedef enum {
    CRASH_ABORT_NONE,
    CRASH_ABORT_MAKE_DIR,
    CRASH_ABORT_JOURNAL_BEFORE_FINISH,        // Crash driver before journal update starts
    CRASH_ABORT_JOURNAL_AFTER_JOURNAL_DATA,   // Crash driver after the journal data has been written but before the journal header has been updated
    CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, // Crash driver after the journal header has been updated but before blocks were written to destination
    CRASH_ABORT_JOURNAL_IN_BLOCK_DATA,        // Crash driver while writing data blocks
    CRASH_ABORT_JOURNAL_AFTER_BLOCK_DATA,     // Crash the driver after the data blocks were written
    CRASH_ABORT_ON_UNMOUNT,                   // Crash on unmount
    CRASH_ABORT_RANDOM,                       // Crach at random time (introduced by tester)
    CRASH_ABORT_LAST
} CrashAbort_E;

typedef int (*CrashAbortFunction_FP)(CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, pthread_t pSyncerThread);

extern CrashAbortFunction_FP gpsCrashAbortFunctionArray[];

#define CRASH_ABORT(CrashAbortCondition, psHfsmount, Vnode)                                         \
    {                                                                                               \
        if (gpsCrashAbortFunctionArray[(CrashAbortCondition)]) {                                    \
                                                                                                    \
            pthread_t pSyncerThread = 0;                                                            \
            if ( ((psHfsmount)->hfs_syncer_thread) &&                                               \
                 ((psHfsmount)->hfs_syncer_thread != (void*)1) ) {                                  \
                pSyncerThread = (psHfsmount)->hfs_syncer_thread;                                    \
            }                                                                                       \
            gpsCrashAbortFunctionArray[(CrashAbortCondition)](                                      \
                (CrashAbortCondition),                                                              \
                (psHfsmount)->hfs_devvp->psFSRecord->iFD,                                           \
                (Vnode), pSyncerThread );                                                           \
        }                                                                                           \
    }

#endif // HFS_CRASH_TEST


#endif /* lf_hfs_h */
