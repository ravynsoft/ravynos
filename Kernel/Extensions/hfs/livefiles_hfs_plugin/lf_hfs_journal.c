/*
 * Copyright (c) 2002-2015 Apple Inc. All rights reserved.
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
//
// This file implements a simple write-ahead journaling layer.  
// In theory any file system can make use of it by calling these 
// functions when the fs wants to modify meta-data blocks.  See
// hfs_journal.h for a more detailed description of the api and
// data structures.
//
// Dominic Giampaolo (dbg@apple.com)
// Port to Live-Files: Oded Shoshani (oshoshani@apple.com)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <sys/disk.h>
#include <sys/kdebug.h>
#include "lf_hfs_locks.h"
#include "lf_hfs_journal.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_generic_buf.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_vfsops.h"

// ************************** Function Definitions ***********************
// number of bytes to checksum in a block_list_header
// NOTE: this should be enough to clear out the header
//       fields as well as the first entry of binfo[]

#define CHECK_JOURNAL(jnl)                                                   \
    do {                                                                     \
        if (jnl == NULL) {                                                   \
            printf("%s:%d: null journal ptr?\n", __FILE__, __LINE__);        \
            panic("%s:%d: null journal ptr?\n", __FILE__, __LINE__);         \
        }                                                                    \
        if (jnl->jdev == NULL) {                                             \
            printf("%s:%d: jdev is null!\n", __FILE__, __LINE__);            \
            panic("%s:%d: jdev is null!\n", __FILE__, __LINE__);             \
        }                                                                    \
        if (jnl->fsdev == NULL) {                                            \
            printf("%s:%d: fsdev is null!\n", __FILE__, __LINE__);           \
            panic("%s:%d: fsdev is null!\n", __FILE__, __LINE__);            \
        }                                                                    \
        if (jnl->jhdr->magic != JOURNAL_HEADER_MAGIC) {                      \
            printf("%s:%d: jhdr magic corrupted (0x%x != 0x%x)\n",           \
                __FILE__, __LINE__, jnl->jhdr->magic, JOURNAL_HEADER_MAGIC); \
            panic("%s:%d: jhdr magic corrupted (0x%x != 0x%x)\n",            \
                __FILE__, __LINE__, jnl->jhdr->magic, JOURNAL_HEADER_MAGIC); \
        }                                                                    \
        if (jnl->jhdr->start <= 0 || jnl->jhdr->start > jnl->jhdr->size) {   \
            printf("%s:%d: jhdr start looks bad (0x%llx max size 0x%llx)\n", \
                __FILE__, __LINE__, jnl->jhdr->start, jnl->jhdr->size);      \
            panic("%s:%d: jhdr start looks bad (0x%llx max size 0x%llx)\n",  \
                __FILE__, __LINE__, jnl->jhdr->start, jnl->jhdr->size);      \
        }                                                                    \
        if (jnl->jhdr->end <= 0 || jnl->jhdr->end > jnl->jhdr->size) {       \
            printf("%s:%d: jhdr end looks bad (0x%llx max size 0x%llx)\n",   \
                __FILE__, __LINE__, jnl->jhdr->end, jnl->jhdr->size);        \
            panic("%s:%d: jhdr end looks bad (0x%llx max size 0x%llx)\n",    \
                __FILE__, __LINE__, jnl->jhdr->end, jnl->jhdr->size);        \
        }                                                                    \
    } while(0)

#define CHECK_TRANSACTION(tr)        \
    do {                             \
        if (tr == NULL) {            \
            printf("%s:%d: null transaction ptr?\n", __FILE__, __LINE__); \
            panic("%s:%d: null transaction ptr?\n", __FILE__, __LINE__);  \
        }                            \
        if (tr->jnl == NULL) {       \
            printf("%s:%d: null tr->jnl ptr?\n", __FILE__, __LINE__); \
            panic("%s:%d: null tr->jnl ptr?\n", __FILE__, __LINE__);  \
        }                            \
        if (tr->blhdr != (block_list_header *)tr->tbuffer) {        \
            printf("%s:%d: blhdr (%p) != tbuffer (%p)\n", __FILE__, __LINE__, tr->blhdr, tr->tbuffer); \
            panic("%s:%d: blhdr (%p) != tbuffer (%p)\n", __FILE__, __LINE__, tr->blhdr, tr->tbuffer);  \
        }                            \
        if (tr->total_bytes < 0) {   \
            printf("%s:%d: tr total_bytes looks bad: %d\n", __FILE__, __LINE__, tr->total_bytes); \
            panic("%s:%d: tr total_bytes looks bad: %d\n", __FILE__, __LINE__, tr->total_bytes);  \
        }                            \
        if (tr->journal_start < 0) { \
            printf("%s:%d: tr journal start looks bad: 0x%llx\n", __FILE__, __LINE__, tr->journal_start); \
            panic("%s:%d: tr journal start looks bad: 0x%llx\n", __FILE__, __LINE__, tr->journal_start);  \
        }                            \
        if (tr->journal_end < 0) {   \
            printf("%s:%d: tr journal end looks bad: 0x%llx\n", __FILE__, __LINE__, tr->journal_end); \
            panic("%s:%d: tr journal end looks bad: 0x%llx\n", __FILE__, __LINE__, tr->journal_end);  \
        }                            \
        if (tr->blhdr && (tr->blhdr->max_blocks <= 0 || tr->blhdr->max_blocks > (tr->jnl->jhdr->size/tr->jnl->jhdr->jhdr_size))) {          \
            printf("%s:%d: tr blhdr max_blocks looks bad: %d\n", __FILE__, __LINE__, tr->blhdr->max_blocks);    \
            panic("%s:%d: tr blhdr max_blocks looks bad: %d\n", __FILE__, __LINE__, tr->blhdr->max_blocks);     \
        }                            \
    } while(0)

#define SWAP16(x) OSSwapInt16(x)
#define SWAP32(x) OSSwapInt32(x)
#define SWAP64(x) OSSwapInt64(x)

#define JNL_WRITE    0x0001
#define JNL_READ     0x0002
#define JNL_HEADER   0x8000

#define BLHDR_CHECKSUM_SIZE 32
#define MAX_JOURNAL_SIZE 0x80000000U

#define STARTING_BUCKETS 256
typedef struct bucket {
    off_t     block_num;
    uint32_t  jnl_offset;
    uint32_t  block_size;
    int32_t   cksum;
} bucket;

static int     replay_journal(journal *jnl);
static void    free_old_stuff(journal *jnl);
static errno_t journal_allocate_transaction(journal *jnl);
static void    get_io_info(struct vnode *devvp, size_t phys_blksz, journal *jnl);
static size_t  read_journal_header(journal *jnl, void *data, size_t len);
static size_t  do_journal_io(journal *jnl, off_t *offset, void *data, size_t len, int direction);
static unsigned int calc_checksum(const char *ptr, int len);
static void    swap_journal_header(journal *jnl);
static int     end_transaction(transaction *tr,
                           int force_it,
                           errno_t (*callback)(void*),
                           void *callback_arg,
                           boolean_t drop_lock);
static void   abort_transaction(journal *jnl, transaction *tr);
static void   size_up_tbuffer(journal *jnl, uint32_t tbuffer_size, uint32_t phys_blksz);
static void   lock_condition(journal *jnl, ConditionalFlag_S *psCondFlag, __unused const char *condition_name);
static void   wait_condition(journal *jnl, ConditionalFlag_S *psCondFlag, __unused const char *condition_name);
static void   unlock_condition(journal *jnl, ConditionalFlag_S *psCondFlag);
static int    write_journal_header(journal *jnl, int updating_start, uint32_t sequence_num);
static size_t read_journal_data(journal *jnl, off_t *offset, void *data, size_t len);
static size_t write_journal_data(journal *jnl, off_t *offset, void *data, size_t len);
        

static __inline__ void lock_oldstart(journal *jnl) {
    lf_lck_mtx_lock(&jnl->old_start_lock);
}
    
static __inline__ void unlock_oldstart(journal *jnl) {
    lf_lck_mtx_unlock(&jnl->old_start_lock);
}
    
__inline__ void journal_lock(journal *jnl) {
    lf_lck_mtx_lock(&jnl->jlock);
    if (jnl->owner) {
        panic ("jnl: owner is %p, expected NULL\n", jnl->owner);
    }
    jnl->owner = pthread_self();
}

__inline__ void journal_unlock(journal *jnl) {
    jnl->owner = NULL;
    lf_lck_mtx_unlock(&jnl->jlock);
}

static __inline__ void lock_flush(journal *jnl) {
    lf_lck_mtx_lock(&jnl->flock);
}

static __inline__ void unlock_flush(journal *jnl) {
    lf_lck_mtx_unlock(&jnl->flock);
}

// ************************** Global Variables ***********************
// Journal Locking
lck_grp_attr_t *jnl_group_attr  = NULL;
lck_attr_t     *jnl_lock_attr   = NULL;
lck_grp_t      *jnl_mutex_group = NULL;

// By default, we grow the list of extents to trim by 4K at a time.
// We'll opt to flush a transaction if it contains at least
// JOURNAL_FLUSH_TRIM_EXTENTS extents to be trimmed (even if the number
// of modified blocks is small).
enum {
    JOURNAL_DEFAULT_TRIM_BYTES   = 4096,
    JOURNAL_DEFAULT_TRIM_EXTENTS = JOURNAL_DEFAULT_TRIM_BYTES / sizeof(dk_extent_t),
    JOURNAL_FLUSH_TRIM_EXTENTS   = JOURNAL_DEFAULT_TRIM_EXTENTS * 15 / 16
};

unsigned int jnl_trim_flush_limit = JOURNAL_FLUSH_TRIM_EXTENTS;

// tbuffer
#define DEFAULT_TRANSACTION_BUFFER_SIZE  (128*1024)
#define MAX_TRANSACTION_BUFFER_SIZE      (3072*1024)
uint32_t def_tbuffer_size = 0; // XXXdbg - so I can change it in the debugger

// ************************** Global Functions ***********************
void journal_init(void) {
    
    jnl_lock_attr    = lf_lck_attr_alloc_init();
    jnl_group_attr   = lf_lck_grp_attr_alloc_init();
    jnl_mutex_group  = lf_lck_grp_alloc_init();
}

journal *journal_open(struct vnode *jvp,
             off_t         offset,
             off_t         journal_size,
             struct vnode *fsvp,
             size_t        min_fs_blksz,
             int32_t       flags,
             int32_t       tbuffer_size,
             void        (*flush)(void *arg),
             void         *arg,
             struct mount *fsmount) {
    journal        *jnl;
    uint32_t     orig_blksz=0;
    uint32_t     phys_blksz;
    u_int32_t    min_size = 0;
    int          orig_checksum, checksum;
    
    /* Get the real physical block size. */
    if (ioctl(jvp->psFSRecord->iFD, DKIOCGETBLOCKSIZE, (caddr_t)&phys_blksz)) {
        goto cleanup_jdev_name;
    }
    
    if (phys_blksz > min_fs_blksz) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: error: phys blksize %u bigger than min fs blksize %zd\n",
               phys_blksz, min_fs_blksz);
        goto cleanup_jdev_name;
    }
    
    if (journal_size < (256*1024) || journal_size > (1024*1024*1024)) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: journal size %lld looks bogus.\n", journal_size);
        goto cleanup_jdev_name;
    }
    
    min_size = phys_blksz * (phys_blksz / sizeof(block_info));
    /* Reject journals that are too small given the sector size of the device */
    if (journal_size < min_size) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: journal size (%lld) too small given sector size of (%u)\n",
               journal_size, phys_blksz);
        goto cleanup_jdev_name;
    }
    
    if ((journal_size % phys_blksz) != 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: journal size 0x%llx is not an even multiple of block size 0x%x\n",
               journal_size, phys_blksz);
        goto cleanup_jdev_name;
    }
    
    jnl = hfs_mallocz(sizeof(struct journal));
    
    jnl->jdev         = jvp;
    jnl->jdev_offset  = offset;
    jnl->jdev_blknum  = (uint32_t)(offset / min_fs_blksz);
    jnl->fsdev        = fsvp;
    jnl->flush        = flush;
    jnl->flush_arg    = arg;
    jnl->flags        = (flags & JOURNAL_OPTION_FLAGS_MASK);
    lf_lck_mtx_init(&jnl->old_start_lock);
    lf_cond_init(&jnl->flushing.sCond);
    lf_cond_init(&jnl->asyncIO.sCond);
    lf_cond_init(&jnl->writing_header.sCond);
    
    /* We hold the mount to later pass to the throttling code for IO
     * accounting.
     */
    jnl->fsmount      = fsmount;
    
    get_io_info(jvp, phys_blksz, jnl);
    
    jnl->header_buf = hfs_malloc(phys_blksz);
    jnl->header_buf_size = phys_blksz;
    
    jnl->jhdr = (journal_header *)jnl->header_buf;
    memset(jnl->jhdr, 0, sizeof(journal_header));
    
    // we have to set this up here so that do_journal_io() will work
    jnl->jhdr->jhdr_size = phys_blksz;
    
    if (read_journal_header(jnl, jnl->jhdr, phys_blksz) != phys_blksz) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: could not read %u bytes for the journal header.\n",
               phys_blksz);
        goto bad_journal;
    }
    
    /*
     * Check for a bad jhdr size after reading in the journal header.
     * The journal header length cannot be zero
     */
    if (jnl->jhdr->jhdr_size == 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: bad jhdr size (%d) \n", jnl->jhdr->jhdr_size);
        goto bad_journal;
    }
    
    orig_checksum = jnl->jhdr->checksum;
    jnl->jhdr->checksum = 0;
    
    if (jnl->jhdr->magic == SWAP32(JOURNAL_HEADER_MAGIC)) {
        
        // do this before the swap since it's done byte-at-a-time
        orig_checksum = SWAP32(orig_checksum);
        checksum = calc_checksum((char *)jnl->jhdr, JOURNAL_HEADER_CKSUM_SIZE);
        swap_journal_header(jnl);
        jnl->flags |= JOURNAL_NEED_SWAP;
        
    } else {
        
        checksum = calc_checksum((char *)jnl->jhdr, JOURNAL_HEADER_CKSUM_SIZE);
    }
    
    if (jnl->jhdr->magic != JOURNAL_HEADER_MAGIC && jnl->jhdr->magic != OLD_JOURNAL_HEADER_MAGIC) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: journal magic is bad (0x%x != 0x%x)\n",
               jnl->jhdr->magic, JOURNAL_HEADER_MAGIC);
        goto bad_journal;
    }
    
    // only check if we're the current journal header magic value
    if (jnl->jhdr->magic == JOURNAL_HEADER_MAGIC) {
        
        if (orig_checksum != checksum) {
            LFHFS_LOG(LEVEL_ERROR, "jnl: open: journal checksum is bad (0x%x != 0x%x)\n",
                   orig_checksum, checksum);
            
            //goto bad_journal;
        }
    }
    
    // XXXdbg - convert old style magic numbers to the new one
    if (jnl->jhdr->magic == OLD_JOURNAL_HEADER_MAGIC) {
        jnl->jhdr->magic = JOURNAL_HEADER_MAGIC;
    }

    if (phys_blksz != (size_t)jnl->jhdr->jhdr_size && jnl->jhdr->jhdr_size != 0) {
        /*
         * The volume has probably been resized (such that we had to adjust the
         * logical sector size), or copied to media with a different logical
         * sector size.
         *
         * For us, though, no big deal because we are giving byte offsets to
         * pread() and pwrite() to do our I/O, and as long as we use self-
         * consistent units, we are all good.
         */
        LFHFS_LOG(LEVEL_ERROR,
                  "jnl: block size mismatch: phys_blksz=%llu, jhdr->jhdr_size=%llu -- COMPENSATING\n",
                  (unsigned long long)phys_blksz, (unsigned long long)jnl->jhdr->jhdr_size);
        orig_blksz = phys_blksz;
    }
    
    if (   jnl->jhdr->start <= 0
        || jnl->jhdr->start > jnl->jhdr->size
        || jnl->jhdr->start > 1024*1024*1024) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: jhdr start looks bad (0x%llx max size 0x%llx)\n",
               jnl->jhdr->start, jnl->jhdr->size);
        goto bad_journal;
    }
    
    if (   jnl->jhdr->end <= 0
        || jnl->jhdr->end > jnl->jhdr->size
        || jnl->jhdr->end > 1024*1024*1024) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: jhdr end looks bad (0x%llx max size 0x%llx)\n",
               jnl->jhdr->end, jnl->jhdr->size);
        goto bad_journal;
    }
    
    if (jnl->jhdr->size < (256*1024) || jnl->jhdr->size > 1024*1024*1024) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: jhdr size looks bad (0x%llx)\n", jnl->jhdr->size);
        goto bad_journal;
    }
    
    // XXXdbg - can't do these checks because hfs writes all kinds of
    //          non-uniform sized blocks even on devices that have a block size
    //          that is larger than 512 bytes (i.e. optical media w/2k blocks).
    //          therefore these checks will fail and so we just have to punt and
    //          do more relaxed checking...
    // XXXdbg    if ((jnl->jhdr->start % jnl->jhdr->jhdr_size) != 0) {
    if ((jnl->jhdr->start % 512) != 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: journal start (0x%llx) not a multiple of 512?\n",
               jnl->jhdr->start);
        goto bad_journal;
    }
    
    //XXXdbg    if ((jnl->jhdr->end % jnl->jhdr->jhdr_size) != 0) {
    if ((jnl->jhdr->end % 512) != 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: journal end (0x%llx) not a multiple of block size (0x%x)?\n",
               jnl->jhdr->end, jnl->jhdr->jhdr_size);
        goto bad_journal;
    }
    
    if (jnl->jhdr->blhdr_size < 0) {
        //throw out invalid sizes
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: blhdr size looks bogus! (%d) \n",
               jnl->jhdr->blhdr_size);
        goto bad_journal;
    }
    
    // take care of replaying the journal if necessary
    if (flags & JOURNAL_RESET) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal start/end pointers reset! (s 0x%llx e 0x%llx)\n",
               jnl->jhdr->start, jnl->jhdr->end);
        jnl->jhdr->start = jnl->jhdr->end;
    } else if (replay_journal(jnl) != 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_open: Error replaying the journal!\n");
        goto bad_journal;
    }

    /*
     * When we get here, we know that the journal is empty (jnl->jhdr->start ==
     * jnl->jhdr->end).  If the device's logical block size was different from
     * the journal's header size, then we can now restore the device's logical
     * block size and update the journal's header size to match.
     *
     * Note that we also adjust the journal's start and end so that they will
     * be aligned on the new block size.  We pick a new sequence number to
     * avoid any problems if a replay found previous transactions using the old
     * journal header size.  (See the comments in journal_create(), above.)
     */

    if (orig_blksz != 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: updating journal header with block size %llu\n",
                  (unsigned long long)phys_blksz);

        jnl->jhdr->jhdr_size = phys_blksz;
        jnl->jhdr->start = phys_blksz;
        jnl->jhdr->end = phys_blksz;
        jnl->jhdr->sequence_num = (jnl->jhdr->sequence_num +
                                   (journal_size / phys_blksz) +
                                   (random() % 16384)) & 0x00ffffff;

        if (write_journal_header(jnl, 1, jnl->jhdr->sequence_num)) {
            LFHFS_LOG(LEVEL_ERROR, "jnl: open: failed to update journal header size\n");
            goto bad_journal;
        }
    }

    // make sure this is in sync!
    jnl->active_start = jnl->jhdr->start;
    jnl->sequence_num = jnl->jhdr->sequence_num;
    
    // set this now, after we've replayed the journal
    size_up_tbuffer(jnl, tbuffer_size, phys_blksz);
    
    // TODO: Does this need to change if the device's logical block size changed?
    if ((off_t)(jnl->jhdr->blhdr_size/sizeof(block_info)-1) > (jnl->jhdr->size/jnl->jhdr->jhdr_size)) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: open: jhdr size and blhdr size are not compatible (0x%llx, %d, %d)\n", jnl->jhdr->size,
               jnl->jhdr->blhdr_size, jnl->jhdr->jhdr_size);
        goto bad_journal;
    }
    
    lf_lck_mtx_init(&jnl->jlock);
    lf_lck_mtx_init(&jnl->flock);
    lf_lck_rw_init(&jnl->trim_lock);
    
    goto journal_open_complete;
    
bad_journal:
    hfs_free(jnl->header_buf);
    hfs_free(jnl);
cleanup_jdev_name:
    jnl = NULL;
journal_open_complete:
    return jnl;
}

journal *journal_create(struct vnode *jvp,
               off_t         offset,
               off_t         journal_size,
               struct vnode *fsvp,
               size_t        min_fs_blksz,
               int32_t       flags,
               int32_t       tbuffer_size,
               void          (*flush)(void *arg),
               void          *arg,
               struct mount  *fsmount) {
    
    journal     *jnl;
    uint32_t    phys_blksz, new_txn_base;
    u_int32_t   min_size;

    /*
     * Cap the journal max size to 2GB.  On HFS, it will attempt to occupy
     * a full allocation block if the current size is smaller than the allocation
     * block on which it resides.  Once we hit the exabyte filesystem range, then
     * it will use 2GB allocation blocks.  As a result, make the cap 2GB.
     */
    
    /* Get the real physical block size. */
    if (ioctl(jvp->psFSRecord->iFD, DKIOCGETBLOCKSIZE, (caddr_t)&phys_blksz)) {
        goto cleanup_jdev_name;
    }
    
    if (journal_size < (256*1024) || journal_size > (MAX_JOURNAL_SIZE)) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: create: journal size %lld looks bogus.\n", journal_size);
        goto cleanup_jdev_name;
    }
    
    min_size = phys_blksz * (phys_blksz / sizeof(block_info));
    /* Reject journals that are too small given the sector size of the device */
    if (journal_size < min_size) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: create: journal size (%lld) too small given sector size of (%u)\n",
               journal_size, phys_blksz);
        goto cleanup_jdev_name;
    }
    
    if (phys_blksz > min_fs_blksz) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: create: error: phys blksize %u bigger than min fs blksize %zd\n",
                phys_blksz, min_fs_blksz);
        goto cleanup_jdev_name;
    }
    
    if ((journal_size % phys_blksz) != 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: create: journal size 0x%llx is not an even multiple of block size 0x%ux\n",
               journal_size, phys_blksz);
        goto cleanup_jdev_name;
    }
    
    
    jnl = hfs_mallocz(sizeof(struct journal));
    
    jnl->jdev         = jvp;
    jnl->jdev_offset  = offset;
    jnl->jdev_blknum  = (uint32_t)(offset / min_fs_blksz);
    jnl->fsdev        = fsvp;
    jnl->flush        = flush;
    jnl->flush_arg    = arg;
    jnl->flags        = (flags & JOURNAL_OPTION_FLAGS_MASK);
    lf_lck_mtx_init(&jnl->old_start_lock);
    
    // Keep a point to the mount around for use in IO throttling.
    jnl->fsmount      = fsmount;
    
    get_io_info(jvp, phys_blksz, jnl);
    
    jnl->header_buf = hfs_malloc(phys_blksz);
    jnl->header_buf_size = phys_blksz;
    
    jnl->jhdr = (journal_header *)jnl->header_buf;
    memset(jnl->jhdr, 0, sizeof(journal_header));
    
    // we have to set this up here so that do_journal_io() will work
    jnl->jhdr->jhdr_size = phys_blksz;
    
    //
    // We try and read the journal header to see if there is already one
    // out there.  If there is, it's possible that it has transactions
    // in it that we might replay if we happen to pick a sequence number
    // that is a little less than the old one, there is a crash and the
    // last txn written ends right at the start of a txn from the previous
    // incarnation of this file system.  If all that happens we would
    // replay the transactions from the old file system and that would
    // destroy your disk.  Although it is extremely unlikely for all those
    // conditions to happen, the probability is non-zero and the result is
    // severe - you lose your file system.  Therefore if we find a valid
    // journal header and the sequence number is non-zero we write junk
    // over the entire journal so that there is no way we will encounter
    // any old transactions.  This is slow but should be a rare event
    // since most tools erase the journal.
    //
    if (   read_journal_header(jnl, jnl->jhdr, phys_blksz) == phys_blksz
        && jnl->jhdr->magic == JOURNAL_HEADER_MAGIC
        && jnl->jhdr->sequence_num != 0) {
        
        new_txn_base = (jnl->jhdr->sequence_num + (journal_size / phys_blksz) + (random() % 16384)) & 0x00ffffff;
        LFHFS_LOG(LEVEL_ERROR, "jnl: create: avoiding old sequence number 0x%x (0x%x)\n", jnl->jhdr->sequence_num, new_txn_base);
        
    } else {
        new_txn_base = random() & 0x00ffffff;
    }
    
    memset(jnl->header_buf, 0, phys_blksz);
    
    jnl->jhdr->magic      = JOURNAL_HEADER_MAGIC;
    jnl->jhdr->endian     = ENDIAN_MAGIC;
    jnl->jhdr->start      = phys_blksz;    // start at block #1, block #0 is for the jhdr itself
    jnl->jhdr->end        = phys_blksz;
    jnl->jhdr->size       = journal_size;
    jnl->jhdr->jhdr_size  = phys_blksz;
    size_up_tbuffer(jnl, tbuffer_size, phys_blksz);
    
    jnl->active_start     = jnl->jhdr->start;
    
    jnl->jhdr->sequence_num = new_txn_base;
    
    lf_lck_mtx_init(&jnl->jlock);
    lf_lck_mtx_init(&jnl->flock);
    lf_lck_rw_init(&jnl->trim_lock);
    
    lf_cond_init(&jnl->flushing.sCond);
    lf_cond_init(&jnl->asyncIO.sCond);
    lf_cond_init(&jnl->writing_header.sCond);
    jnl->flush_aborted = FALSE;
    jnl->async_trim = NULL;
    jnl->sequence_num = jnl->jhdr->sequence_num;
    
    if (write_journal_header(jnl, 1, jnl->jhdr->sequence_num) != 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_create: failed to write journal header.\n");
        goto bad_write;
    }
    
    goto journal_create_complete;
    
    
bad_write:
    hfs_free(jnl->header_buf);
    jnl->jhdr = NULL;
    hfs_free(jnl);
cleanup_jdev_name:
    jnl = NULL;
journal_create_complete:
    return jnl;
}



void *journal_owner(journal *jnl) {
    return jnl->owner;
}

/* Is the given cnode either the .journal or .journal_info_block file on
 * a volume with an active journal?  Many VNOPs use this to deny access
 * to those files.
 *
 * Note: the .journal file on a volume with an external journal still
 * returns true here, even though it does not actually hold the contents
 * of the volume's journal.
 */
_Bool hfs_is_journal_file(struct hfsmount *hfsmp, struct cnode *cp) {
    if (hfsmp->jnl != NULL &&
        (cp->c_fileid == hfsmp->hfs_jnlinfoblkid ||
         cp->c_fileid == hfsmp->hfs_jnlfileid)) {
        return true;
    } else {
        return false;
    }
}

bool is_journaled(UVFSFileNode *psRootNode) {
    
    struct vnode *psRootVnode = *psRootNode;
    
    if (!psRootNode) {
        LFHFS_LOG(LEVEL_DEBUG, "is_journaled: psRootNode is NULL");
        return false;
    }

    if (!psRootVnode->sFSParams.vnfs_mp) {
        LFHFS_LOG(LEVEL_DEBUG, "is_journaled: psRootVnode->sFSParams.vnfs_mp is NULL");
        return false;
    }

    if (psRootVnode->sFSParams.vnfs_mp->psHfsmount->jnl)
        return true;
        
    return false;
}


// Media no longer available, clear all memory occupied by the journal
void journal_release(journal *jnl) {
    if (jnl->owner != pthread_self()) {
        journal_lock(jnl);
    }

    if (jnl->active_tr) {
        abort_transaction(jnl, jnl->active_tr);
    }
        
    if (jnl->cur_tr) {
        abort_transaction(jnl, jnl->cur_tr);
    }

    free_old_stuff(jnl);
    
    hfs_free(jnl->header_buf);
    jnl->jhdr = (void *)0xbeefbabe;
    
    journal_unlock(jnl);
    lf_lck_mtx_destroy(&jnl->old_start_lock);
    lf_lck_mtx_destroy(&jnl->jlock);
    lf_lck_mtx_destroy(&jnl->flock);
    hfs_free(jnl);
}


void journal_close(journal *jnl) {
    volatile off_t *start, *end;
    int             counter=0;
    
    CHECK_JOURNAL(jnl);
    
    // set this before doing anything that would block so that
    // we start tearing things down properly.
    //
    jnl->flags |= JOURNAL_CLOSE_PENDING;
    
    if (jnl->owner != pthread_self()) {
        journal_lock(jnl);
    }
    
    wait_condition(jnl, &jnl->flushing, "journal_close");
    
    //
    // only write stuff to disk if the journal is still valid
    //
    if ((jnl->flags & JOURNAL_INVALID) == 0) {
        
        if (jnl->active_tr) {
            /*
             * "journal_end_transaction" will fire the flush asynchronously
             */
            journal_end_transaction(jnl);
        }
        
        // flush any buffered transactions
        if (jnl->cur_tr) {
            transaction *tr = jnl->cur_tr;
            
            jnl->cur_tr = NULL;
            /*
             * "end_transaction" will wait for any in-progress flush to complete
             * before flushing "cur_tr" synchronously("must_wait" == TRUE)
             */
            end_transaction(tr, 1, NULL, NULL, FALSE);
        }
        /*
         * if there was an "active_tr", make sure we wait for
         * it to flush if there was no "cur_tr" to process
         */
        wait_condition(jnl, &jnl->flushing, "journal_close");
        
        //start = &jnl->jhdr->start;
        start = &jnl->active_start;
        end   = &jnl->jhdr->end;
        
        while (*start != *end && counter++ < 5000) {
            //printf("jnl: close: flushing the buffer cache (start 0x%llx end 0x%llx)\n", *start, *end);
            if (jnl->flush) {
                jnl->flush(jnl->flush_arg);
            }
            usleep(10000);
        }
        
        if (*start != *end) {
            LFHFS_LOG(LEVEL_ERROR, "jnl: close: buffer flushing didn't seem to flush out all the transactions! (0x%llx - 0x%llx)\n",
                   *start, *end);
        }
        
        // make sure this is in sync when we close the journal
        jnl->jhdr->start = jnl->active_start;
        
        // if this fails there's not much we can do at this point...
        write_journal_header(jnl, 1, jnl->sequence_num);
    } else {
        // if we're here the journal isn't valid any more.
        // so make sure we don't leave any locked blocks lying around
        LFHFS_LOG(LEVEL_ERROR, "jnl: close: journal is invalid.  aborting outstanding transactions\n");
        if (jnl->active_tr || jnl->cur_tr) {
            transaction *tr;
            
            if (jnl->active_tr) {
                tr = jnl->active_tr;
                jnl->active_tr = NULL;
            } else {
                tr = jnl->cur_tr;
                jnl->cur_tr = NULL;
            }
            abort_transaction(jnl, tr);
            
            if (jnl->active_tr || jnl->cur_tr) {
                panic("jnl: close: jnl @ %p had both an active and cur tr\n", jnl);
            }
        }
    }
    wait_condition(jnl, &jnl->asyncIO, "journal_close");
    
    free_old_stuff(jnl);
    
    hfs_free(jnl->header_buf);
    jnl->jhdr = (void *)0xbeefbabe;
    
    journal_unlock(jnl);
    lf_lck_mtx_destroy(&jnl->old_start_lock);
    lf_lck_mtx_destroy(&jnl->jlock);
    lf_lck_mtx_destroy(&jnl->flock);
    hfs_free(jnl);
}

// This function performs the following:
// 1) Checks that we have a valid journal
// 2) locks the journal
// 3) Allocates roon in the journal
int journal_start_transaction(journal *jnl) {
    
    int ret;
    
    #if JOURNAL_DEBUG
        printf("journal_start_transaction (%u).\n", jnl->nested_count);
    #endif

    CHECK_JOURNAL(jnl);
    
    free_old_stuff(jnl);
    
    if (jnl->flags & JOURNAL_INVALID) {
        return EINVAL;
    }
    
    if (jnl->owner == pthread_self()) {
        if (jnl->active_tr == NULL) {
            panic("jnl: start_tr: active_tr is NULL (jnl @ %p, owner %p, current_thread %p\n",
                  jnl, jnl->owner, pthread_self());
        }
        jnl->nested_count++;
        return 0;
    }
    
    journal_lock(jnl);
    
    if (jnl->nested_count != 0 || jnl->active_tr != NULL) {
        panic("jnl: start_tr: owner %p, nested count %d, active_tr %p jnl @ %p\n",
              jnl->owner, jnl->nested_count, jnl->active_tr, jnl);
    }
    
    jnl->nested_count = 1;
    
    // if there's a buffered transaction, use it.
    if (jnl->cur_tr) {
        jnl->active_tr = jnl->cur_tr;
        jnl->cur_tr    = NULL;
        
        return 0;
    }
    
    ret = journal_allocate_transaction(jnl);
    if (ret) {
        goto bad_start;
    }
    
    // printf("jnl: start_tr: owner 0x%x new tr @ 0x%x\n", jnl->owner, jnl->active_tr);
    
    return 0;
    
bad_start:
    jnl->nested_count = 0;
    journal_unlock(jnl);
    
    return ret;
}
// journal_end_transaction
// This function does the following:
// 1) Validates journal status/state
// 2)
int journal_end_transaction(journal *jnl) {
    int ret;
    transaction *tr;
    
#if JOURNAL_DEBUG
    printf("journal_end_transaction   (%u).\n", jnl->nested_count-1);
#endif
    
    CHECK_JOURNAL(jnl);
    
    free_old_stuff(jnl);
    
    if ((jnl->flags & JOURNAL_INVALID) && jnl->owner == NULL) {
        return 0;
    }
    
    if (jnl->owner != pthread_self()) {
        panic("jnl: end_tr: I'm not the owner! jnl %p, owner %p, curact %p\n",
              jnl, jnl->owner, pthread_self());
    }
    jnl->nested_count--;
    
    if (jnl->nested_count > 0) {
        return 0;
    } else if (jnl->nested_count < 0) {
        panic("jnl: jnl @ %p has negative nested count (%d). bad boy.\n", jnl, jnl->nested_count);
    }
    
    if (jnl->flags & JOURNAL_INVALID) {
        if (jnl->active_tr) {
            if (jnl->cur_tr != NULL) {
                panic("jnl: journal @ %p has active tr (%p) and cur tr (%p)\n",
                      jnl, jnl->active_tr, jnl->cur_tr);
            }
            tr             = jnl->active_tr;
            jnl->active_tr = NULL;
            
            abort_transaction(jnl, tr);
        }
        journal_unlock(jnl);
        
        return EINVAL;
    }
    
    tr = jnl->active_tr;
    CHECK_TRANSACTION(tr);
    
    // clear this out here so that when check_free_space() calls
    // the FS flush function, we don't panic in journal_flush()
    // if the FS were to call that.  note: check_free_space() is
    // called from end_transaction().
    jnl->active_tr = NULL;
    
    /* Examine the force-journal-flush state in the active txn */
    if (tr->flush_on_completion == TRUE) {
        /*
         * If the FS requested it, disallow group commit and force the
         * transaction out to disk immediately.
         */
        ret = end_transaction(tr, 1, NULL, NULL, TRUE);
    }
    else {
        /* in the common path we can simply use the double-buffered journal */
        ret = end_transaction(tr, 0, NULL, NULL, TRUE);
    }
    
    return ret;
}

// journal_modify_block_start
// This function does the following:
// 1) Makes sure the journal file is on and valid
// 2) Clean up (free previous transactions)
// 3) Validate that the phy-block-size has not changed.
// 4) Locks the buffer.
// Buffer life cycle with journal:
// 1) Client code (ie btrees_io.c) allocates a buffer (ie gains ownership). Other threads will pend on using this buffer until it is released.
// 2) Client code calls journal_modify_block_start which sets the GEN_BUF_WRITE_LOCK uCacheFlag.
// 3) Client code modifies the buffer.
// 4) Client code calls journal_modify_block_end which released the buffer. The GEN_BUF_WRITE_LOCK flag remains set.
//  It this point other threads are welcomed to modify the buffer (after executing steps 1 and 2 above). The buffer content will not be written to media before transaction_end, thus only the accumulative change of both threads after transaction_end will be committed.
// 5) transaction-end (called from within client-code or async Sync) obtains ownership on in transaction buffers. By doing that it makes sure no buffer is currently being modified by any Client code. It then prepares the buffer for commiting (ie realigns endianizm), and commits (writes to the t-buffer, write the t-buffer to media, updates journal-info, clears the GEN_BUF_WRITE_LOCK flags and writes the buffers to media).
int journal_modify_block_start(journal *jnl, GenericLFBuf *psGenBuf) {

    transaction *tr;
    
#if JOURNAL_DEBUG
    printf("journal_modify_block_start: psGenBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uCacheFlags 0x%llx, uPhyCluster %llu, uLockCnt %u\n",
           psGenBuf, psGenBuf->psVnode, psGenBuf->uBlockN, psGenBuf->uDataSize, psGenBuf->uCacheFlags ,psGenBuf->uPhyCluster, psGenBuf->uLockCnt);
#endif
    
    CHECK_JOURNAL(jnl);
    
    free_old_stuff(jnl);
    
    if (jnl->flags & JOURNAL_INVALID) {
        return EINVAL;
    }
    
    tr = jnl->active_tr;
    CHECK_TRANSACTION(tr);
    
    if (jnl->owner != pthread_self()) {
        panic("jnl: modify_block_start: called w/out a transaction! jnl %p, owner %p, curact %p\n",
              jnl, jnl->owner, pthread_self());
    }
    
    //printf("jnl: mod block start (bp 0x%x vp 0x%x l/blkno %qd/%qd bsz %d; total bytes %d)\n",
    //   bp, buf_vnode(bp), buf_lblkno(bp), buf_blkno(bp), buf_size(bp), tr->total_bytes);
    
    // can't allow blocks that aren't an even multiple of the
    // underlying block size.
    if ((psGenBuf->uDataSize % jnl->jhdr->jhdr_size) != 0) {
        uint32_t bad=0;
        uint32_t phys_blksz;

        if (ioctl(jnl->jdev->psFSRecord->iFD, DKIOCGETBLOCKSIZE, (caddr_t)&phys_blksz)) {
            bad = 1;
        } else if (phys_blksz != (uint32_t)jnl->jhdr->jhdr_size) {
            if (phys_blksz < 512) {
                panic("jnl: mod block start: phys blksz %d is too small (%d, %d)\n",
                      phys_blksz, psGenBuf->uDataSize, jnl->jhdr->jhdr_size);
            }
            
            if ((psGenBuf->uDataSize % phys_blksz) != 0) {
                bad = 1;
            } else if (phys_blksz < (uint32_t)jnl->jhdr->jhdr_size) {
                jnl->jhdr->jhdr_size = phys_blksz;
            } else {
                // the phys_blksz is now larger... need to realloc the jhdr
                char *new_header_buf;
                
                LFHFS_LOG(LEVEL_ERROR, "jnl: phys blksz got bigger (was: %d/%d now %d)\n",
                       jnl->header_buf_size, jnl->jhdr->jhdr_size, phys_blksz);
                new_header_buf = hfs_malloc(phys_blksz);
                memcpy(new_header_buf, jnl->header_buf, jnl->header_buf_size);
                memset(&new_header_buf[jnl->header_buf_size], 0x18, (phys_blksz - jnl->header_buf_size));
                hfs_free(jnl->header_buf);
                jnl->header_buf = new_header_buf;
                jnl->header_buf_size = phys_blksz;
                
                jnl->jhdr = (journal_header *)jnl->header_buf;
                jnl->jhdr->jhdr_size = phys_blksz;
            }
        } else {
            bad = 1;
        }

        if (bad) {
            panic("jnl: mod block start: bufsize %d not a multiple of block size %d\n",
                  psGenBuf->uDataSize, jnl->jhdr->jhdr_size);
            
            return -1;
        }
    }
    
    // make sure that this transaction isn't bigger than the whole journal
    if ((tr->total_bytes+psGenBuf->uDataSize) >= (size_t)(jnl->jhdr->size - jnl->jhdr->jhdr_size)) {
        panic("jnl: transaction too big (%d >= %lld bytes, bufsize %d, tr %p bp %p)\n",
              tr->total_bytes, (tr->jnl->jhdr->size - jnl->jhdr->jhdr_size), psGenBuf->uDataSize, tr, psGenBuf->pvData);
        
        return -1;
    }
    
    lf_hfs_generic_buf_set_cache_flag(psGenBuf, GEN_BUF_WRITE_LOCK);
    
    return 0;
}
// journal_modify_block_end
// This function does the following:
// 1) Makes sure the journal file is on and valid
// 2) Clean up (free previous transactions)
// 3) Check if this block already exists in transaction
// 4) Add block number to transcation. We dont add the block data, nor we release the buffer at this point.
//    This will be done later on, at the transaction-end.
int journal_modify_block_end(journal *jnl, GenericLFBuf *psGenBuf,
                            void (*func)(GenericLFBuf *bp, void *arg), void *arg) {
    int                i = 1;
    size_t             tbuffer_offset=0;
    block_list_header *blhdr, *prev=NULL;
    transaction       *tr = NULL;
    
    #if JOURNAL_DEBUG
        printf("journal_modify_block_end:   psGenBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uPhyCluster %llu uLockCnt %u\n",
               psGenBuf, psGenBuf->psVnode, psGenBuf->uBlockN, psGenBuf->uDataSize, psGenBuf->uPhyCluster, psGenBuf->uLockCnt);
    #endif
    
    CHECK_JOURNAL(jnl);
    
    free_old_stuff(jnl);

    if (func) {
        psGenBuf->pfFunc         = func;
        psGenBuf->pvCallbackArgs = arg;
    }
    
    if (jnl->flags & JOURNAL_INVALID) {
        /* Still need to buf_brelse(). Callers assume we consume the bp. */
        lf_hfs_generic_buf_clear_cache_flag(psGenBuf, GEN_BUF_WRITE_LOCK);
        lf_hfs_generic_buf_release(psGenBuf);
        return EINVAL;
    }
    
    tr = jnl->active_tr;
    CHECK_TRANSACTION(tr);
    
    if (jnl->owner != pthread_self()) {
        panic("jnl: modify_block_end: called w/out a transaction! jnl %p, owner %p, curact %p\n",
              jnl, jnl->owner, pthread_self());
    }
    
    if ((psGenBuf->uCacheFlags & GEN_BUF_WRITE_LOCK) == 0) {
        panic("jnl: modify_block_end: bp %p not locked! jnl @ %p\n", psGenBuf, jnl);
    }
    
    // first check if this block is already part of this transaction
    for (blhdr = tr->blhdr; blhdr; prev = blhdr, blhdr = (block_list_header *)((long)blhdr->binfo[0].bnum)) {
        tbuffer_offset = jnl->jhdr->blhdr_size;
        
        for (i = 1; i < blhdr->num_blocks; i++) {
            GenericLFBuf *bp = (void*)blhdr->binfo[i].u.bp;
            if (psGenBuf == bp) {
                // Block found in transaction
                #if JOURNAL_DEBUG
                    printf("block_end, already in journal:   psGenBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uPhyCluster %llu uLockCnt %u\n",
                       psGenBuf, psGenBuf->psVnode, psGenBuf->uBlockN, psGenBuf->uDataSize, psGenBuf->uPhyCluster, psGenBuf->uLockCnt);
                #endif
                break;
            }
            if (blhdr->binfo[i].bnum != (off_t)-1) {
                off_t uSizeOfBuf = ((GenericLFBuf*)(blhdr->binfo[i].u.bp))->uDataSize;
                tbuffer_offset  += uSizeOfBuf;
            } else {
                tbuffer_offset  += blhdr->binfo[i].u.bi.bsize;
            }
        }
        
        if (i < blhdr->num_blocks) {
            break;
        }
    }
    
    if (blhdr == NULL
        && prev
        && (prev->num_blocks+1) <= prev->max_blocks
        && (prev->bytes_used+psGenBuf->uDataSize) <= (uint32_t)tr->tbuffer_size) {
        // Block not found, add to last list
        blhdr = prev;
        
    } else if (blhdr == NULL) {
        block_list_header *nblhdr;
        if (prev == NULL) {
            panic("jnl: modify block end: no way man, prev == NULL?!?, jnl %p, psGenBuf %p\n", jnl, psGenBuf);
        }
        // Add another tbuffer:
        
        // we got to the end of the list, didn't find the block and there's
        // no room in the block_list_header pointed to by prev
        
        // we allocate another tbuffer and link it in at the end of the list
        // through prev->binfo[0].bnum.  that's a skanky way to do things but
        // avoids having yet another linked list of small data structures to manage.
        
        nblhdr = hfs_malloc(tr->tbuffer_size);
        
        // journal replay code checksum check depends on this.
        memset(nblhdr, 0, BLHDR_CHECKSUM_SIZE);
        // Fill up the rest of the block with unimportant bytes
        memset(nblhdr + BLHDR_CHECKSUM_SIZE, 0x5a, jnl->jhdr->blhdr_size - BLHDR_CHECKSUM_SIZE);
        
        // initialize the new guy
        nblhdr->max_blocks = (jnl->jhdr->blhdr_size / sizeof(block_info)) - 1;
        nblhdr->num_blocks = 1;      // accounts for this header block
        nblhdr->bytes_used = (uint32_t)jnl->jhdr->blhdr_size;
        nblhdr->flags = BLHDR_CHECK_CHECKSUMS;
        
        tr->num_blhdrs++;
        tr->total_bytes += jnl->jhdr->blhdr_size;
        
        // then link him in at the end
        prev->binfo[0].bnum = (off_t)((long)nblhdr);
        
        // and finally switch to using the new guy
        blhdr          = nblhdr;
        i              = 1;
    }
    
    if ((i+1) > blhdr->max_blocks) {
        panic("jnl: modify_block_end: i = %d, max_blocks %d\n", i, blhdr->max_blocks);
    }
    
    // if this is true then this is a new block we haven't seen before
    if (i >= blhdr->num_blocks) {
        off_t    bsize;
        bsize = psGenBuf->uDataSize;
        
        // Add block to list
        blhdr->binfo[i].bnum = (off_t)(psGenBuf->uBlockN);
        blhdr->binfo[i].u.bp = (void*)psGenBuf;
       
        blhdr->bytes_used += bsize;
        tr->total_bytes   += bsize;
        
        blhdr->num_blocks++;
    }

    // We can release the block here to allow other threads to perform operations on it until the next transaction-end.
    // The buffer will not be removed from cache since it is write-locked.
    lf_hfs_generic_buf_release(psGenBuf);

    return 0;
}

// This function validates if a block is already registered to a transaction
/*
 * Flush the contents of the journal to the disk.
 *
 *  Input:
 *      wait_for_IO -
 *      If TRUE, wait to write in-memory journal to the disk
 *      consistently, and also wait to write all asynchronous
 *      metadata blocks to its corresponding locations
 *      consistently on the disk.  This means that the journal
 *      is empty at this point and does not contain any
 *      transactions.  This is overkill in normal scenarios
 *      but is useful whenever the metadata blocks are required
 *      to be consistent on-disk instead of just the journal
 *      being consistent; like before live verification
 *      and live volume resizing.
 *
 *      If FALSE, only wait to write in-memory journal to the
 *      disk consistently.  This means that the journal still
 *      contains uncommitted transactions and the file system
 *      metadata blocks in the journal transactions might be
 *      written asynchronously to the disk.  But there is no
 *      guarantee that they are written to the disk before
 *      returning to the caller.  Note that this option is
 *      sufficient for file system data integrity as it
 *      guarantees consistent journal content on the disk.
 */
int journal_flush(journal *jnl, journal_flush_options_t options) {
    boolean_t drop_lock   = FALSE;
    errno_t   error       = 0;
    uint32_t  flush_count = 0;
    
    CHECK_JOURNAL(jnl);
    
    free_old_stuff(jnl);
    
    if (jnl->flags & JOURNAL_INVALID) {
        return EINVAL;
    }
    
    if (jnl->owner != pthread_self()) {
        journal_lock(jnl);
        drop_lock = TRUE;
    }
    
    if (ISSET(options, JOURNAL_FLUSH_FULL))
        flush_count = jnl->flush_counter;
    
    // if we're not active, flush any buffered transactions
    if (jnl->active_tr == NULL && jnl->cur_tr) {
        transaction *tr = jnl->cur_tr;
        
        jnl->cur_tr = NULL;
        
        if (ISSET(options, JOURNAL_WAIT_FOR_IO)) {
            wait_condition(jnl, &jnl->flushing, "journal_flush");
            wait_condition(jnl, &jnl->asyncIO,  "journal_flush");
        }
        
        // As the journal flush changes the MetaData content (update Endianizm), we need to lock the system times.
        int lockflags = hfs_systemfile_lock(jnl->fsmount->psHfsmount, SFL_CATALOG | SFL_ATTRIBUTE | SFL_EXTENTS | SFL_BITMAP, HFS_EXCLUSIVE_LOCK);

        /*
         * "end_transction" will wait for any current async flush
         * to complete, before flushing "cur_tr"... because we've
         * specified the 'must_wait' arg as TRUE, it will then
         * synchronously flush the "cur_tr"
         */
        end_transaction(tr, 1, NULL, NULL, drop_lock);   // force it to get flushed
        
        hfs_systemfile_unlock(jnl->fsmount->psHfsmount, lockflags);
        
    } else  {
        if (drop_lock == TRUE) {
            journal_unlock(jnl);
        }
        
        /* Because of pipelined journal, the journal transactions
         * might be in process of being flushed on another thread.
         * If there is nothing to flush currently, we should
         * synchronize ourselves with the pipelined journal thread
         * to ensure that all inflight transactions, if any, are
         * flushed before we return success to caller.
         */
        wait_condition(jnl, &jnl->flushing, "journal_flush");
    }
    if (ISSET(options, JOURNAL_WAIT_FOR_IO)) {
        wait_condition(jnl, &jnl->asyncIO, "journal_flush");
    }
    
    if (ISSET(options, JOURNAL_FLUSH_FULL)) {
        
        dk_synchronize_t sync_request = {
            .options                        = 0,
        };
        
        // We need a full cache flush. If it has not been done, do it here.
        if (flush_count == jnl->flush_counter)
            error = ioctl(jnl->jdev->psFSRecord->iFD, DKIOCSYNCHRONIZE, (caddr_t)&sync_request);
        
        // If external journal partition is enabled, flush filesystem data partition.
        if (jnl->jdev != jnl->fsdev)
            error = ioctl(jnl->jdev->psFSRecord->iFD, DKIOCSYNCHRONIZE, (caddr_t)&sync_request);
        
    }
    
    return error;
}


// ************************** Local Functions ***********************
static int update_fs_block(journal *jnl, void *block_ptr, off_t fs_block, size_t bsize) {

    int            iRet    = 0;
    GenericLFBuf *psGenBuf = NULL;
    
    // first read the block we want.
    psGenBuf = lf_hfs_generic_buf_allocate(jnl->fsmount->psHfsmount->hfs_devvp,
                                           fs_block,
                                           (uint32_t)bsize,
                                           GEN_BUF_PHY_BLOCK | GEN_BUF_NON_CACHED);
    if (!psGenBuf) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: update_fs_block: error allocating fs block # %lld!\n", fs_block);
        iRet = -1;
        goto exit;
    }
    
    iRet = lf_hfs_generic_buf_read(psGenBuf);
    if (iRet) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: update_fs_block: error reading fs block # %lld!\n", fs_block);
        goto exit;
    }
    
    // copy the journal data over top of it
    memcpy(psGenBuf->pvData, block_ptr, bsize);
    
    iRet = lf_hfs_generic_buf_write(psGenBuf);
    if (iRet) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: update_fs_block: failed to write block %lld (ret %d)\n", fs_block, iRet);
        goto exit;
    }

exit:
    if (psGenBuf) {
       lf_hfs_generic_buf_release(psGenBuf);
    }
    
    return iRet;
}


static int grow_table(struct bucket **buf_ptr, int num_buckets, int new_size) {
    struct bucket *newBuf;
    int current_size = num_buckets, i;
    
    // return if newsize is less than the current size
    if (new_size < num_buckets) {
        return current_size;
    }
    
    newBuf = hfs_malloc(new_size*sizeof(struct bucket));
    
    //  printf("jnl: lookup_bucket: expanded co_buf to %d elems\n", new_size);
    
    // copy existing elements
    bcopy(*buf_ptr, newBuf, num_buckets*sizeof(struct bucket));
    
    // initialize the new ones
    for(i = num_buckets; i < new_size; i++) {
        newBuf[i].block_num = (off_t)-1;
    }
    
    // free the old container
    hfs_free(*buf_ptr);
    
    // reset the buf_ptr
    *buf_ptr = newBuf;
    
    return new_size;
}


static int insert_block(journal *jnl, struct bucket **buf_ptr, int blk_index, off_t num, size_t size, size_t offset, int32_t cksum, int *num_buckets_ptr, int *num_full_ptr, int overwriting) {
    
    if (!overwriting) {
        // grow the table if we're out of space - we may index the table
        // with *num_full_ptr (lookup_bucket() can return a maximum value ==
        // *num_full_ptr), so we need to grow when we hit (*num_buckets_ptr - 1)
        // to prevent out-of-bounds indexing
        if (*num_full_ptr >= (*num_buckets_ptr - 1)) {
            int new_size = *num_buckets_ptr * 2;
            int grow_size = grow_table(buf_ptr, *num_buckets_ptr, new_size);
            
            if (grow_size < new_size) {
                LFHFS_LOG(LEVEL_ERROR, "jnl: add_block: grow_table returned an error!\n");
                return -1;
            }
            
            *num_buckets_ptr = grow_size; //update num_buckets to reflect the new size
        }
        
        // if we're not inserting at the end, we need to bcopy
        if (blk_index != *num_full_ptr) {
            bcopy( (*buf_ptr)+(blk_index), (*buf_ptr)+(blk_index+1), (*num_full_ptr-blk_index)*sizeof(struct bucket) );
        }
        
        (*num_full_ptr)++; // increment only if we're not overwriting
    }
    
    // sanity check the values we're about to add
    if ((off_t)offset >= jnl->jhdr->size) {
        offset = jnl->jhdr->jhdr_size + (offset - jnl->jhdr->size);
    }
    if (size <= 0) {
        panic("jnl: insert_block: bad size in insert_block (%zd)\n", size);
    }
    
    (*buf_ptr)[blk_index].block_num = num;
    (*buf_ptr)[blk_index].block_size = (uint32_t)size;
    (*buf_ptr)[blk_index].jnl_offset = (uint32_t)offset;
    (*buf_ptr)[blk_index].cksum = cksum;
    
    return blk_index;
}

static int do_overlap(journal *jnl, struct bucket **buf_ptr, int blk_index, off_t block_num, size_t size, __unused size_t offset, int32_t cksum, int *num_buckets_ptr, int *num_full_ptr) {
    
    int     num_to_remove, index, i, overwrite, err;
    size_t  jhdr_size = jnl->jhdr->jhdr_size, new_offset;
    off_t   overlap, block_start, block_end;
    
    block_start = block_num*jhdr_size;
    block_end = block_start + size;
    overwrite = (block_num == (*buf_ptr)[blk_index].block_num && size >= (*buf_ptr)[blk_index].block_size);
    
    // first, eliminate any overlap with the previous entry
    if (blk_index != 0 && !overwrite) {
        off_t prev_block_start = (*buf_ptr)[blk_index-1].block_num*jhdr_size;
        off_t prev_block_end = prev_block_start + (*buf_ptr)[blk_index-1].block_size;
        overlap = prev_block_end - block_start;
        if (overlap > 0) {
            if (overlap % jhdr_size != 0) {
                panic("jnl: do_overlap: overlap with previous entry not a multiple of %zd\n", jhdr_size);
            }
            
            // if the previous entry completely overlaps this one, we need to break it into two pieces.
            if (prev_block_end > block_end) {
                off_t new_num = block_end / jhdr_size;
                size_t new_size = prev_block_end - block_end;
                
                new_offset = (*buf_ptr)[blk_index-1].jnl_offset + (block_end - prev_block_start);
                
                err = insert_block(jnl, buf_ptr, blk_index, new_num, new_size, new_offset, cksum, num_buckets_ptr, num_full_ptr, 0);
                if (err < 0) {
                    panic("jnl: do_overlap: error inserting during pre-overlap\n");
                }
            }
            
            // Regardless, we need to truncate the previous entry to the beginning of the overlap
            (*buf_ptr)[blk_index-1].block_size = (uint32_t)(block_start - prev_block_start);
            (*buf_ptr)[blk_index-1].cksum = 0;   // have to blow it away because there's no way to check it
        }
    }
    
    // then, bail out fast if there's no overlap with the entries that follow
    if (!overwrite && block_end <= (off_t)((*buf_ptr)[blk_index].block_num*jhdr_size)) {
        return 0; // no overlap, no overwrite
    } else if (overwrite && (blk_index + 1 >= *num_full_ptr || block_end <= (off_t)((*buf_ptr)[blk_index+1].block_num*jhdr_size))) {
        
        (*buf_ptr)[blk_index].cksum = cksum;   // update this
        return 1; // simple overwrite
    }
    
    // Otherwise, find all cases of total and partial overlap. We use the special
    // block_num of -2 to designate entries that are completely overlapped and must
    // be eliminated. The block_num, size, and jnl_offset of partially overlapped
    // entries must be adjusted to keep the array consistent.
    index = blk_index;
    num_to_remove = 0;
    while (index < *num_full_ptr && block_end > (off_t)((*buf_ptr)[index].block_num*jhdr_size)) {
        if (block_end >= (off_t)(((*buf_ptr)[index].block_num*jhdr_size + (*buf_ptr)[index].block_size))) {
            (*buf_ptr)[index].block_num = -2; // mark this for deletion
            num_to_remove++;
        } else {
            overlap = block_end - (*buf_ptr)[index].block_num*jhdr_size;
            if (overlap > 0) {
                if (overlap % jhdr_size != 0) {
                    panic("jnl: do_overlap: overlap of %lld is not multiple of %zd\n", overlap, jhdr_size);
                }
                
                // if we partially overlap this entry, adjust its block number, jnl offset, and size
                (*buf_ptr)[index].block_num += (overlap / jhdr_size); // make sure overlap is multiple of jhdr_size, or round up
                (*buf_ptr)[index].cksum = 0;
                
                new_offset = (*buf_ptr)[index].jnl_offset + overlap; // check for wrap-around
                if ((off_t)new_offset >= jnl->jhdr->size) {
                    new_offset = jhdr_size + (new_offset - jnl->jhdr->size);
                }
                (*buf_ptr)[index].jnl_offset = (uint32_t)new_offset;
                
                (*buf_ptr)[index].block_size -= overlap; // sanity check for negative value
                if ((*buf_ptr)[index].block_size <= 0) {
                    panic("jnl: do_overlap: after overlap, new block size is invalid (%u)\n", (*buf_ptr)[index].block_size);
                    // return -1; // if above panic is removed, return -1 for error
                }
            }
            
        }
        
        index++;
    }
    
    // bcopy over any completely overlapped entries, starting at the right (where the above loop broke out)
    index--; // start with the last index used within the above loop
    while (index >= blk_index) {
        if ((*buf_ptr)[index].block_num == -2) {
            if (index == *num_full_ptr-1) {
                (*buf_ptr)[index].block_num = -1; // it's the last item in the table... just mark as free
            } else {
                bcopy( (*buf_ptr)+(index+1), (*buf_ptr)+(index), (*num_full_ptr - (index + 1)) * sizeof(struct bucket) );
            }
            (*num_full_ptr)--;
        }
        index--;
    }
    
    // eliminate any stale entries at the end of the table
    for(i = *num_full_ptr; i < (*num_full_ptr + num_to_remove); i++) {
        (*buf_ptr)[i].block_num = -1;
    }
    
    return 0; // if we got this far, we need to insert the entry into the table (rather than overwrite)
}


static int lookup_bucket(struct bucket **buf_ptr, off_t block_num, int num_full) {
    int lo, hi, index, matches, i;
    
    if (num_full == 0) {
        return 0; // table is empty, so insert at index=0
    }
    
    lo = 0;
    hi = num_full - 1;
    index = -1;
    
    // perform binary search for block_num
    do {
        int mid = (hi - lo)/2 + lo;
        off_t this_num = (*buf_ptr)[mid].block_num;
        
        if (block_num == this_num) {
            index = mid;
            break;
        }
        
        if (block_num < this_num) {
            hi = mid;
            continue;
        }
        
        if (block_num > this_num) {
            lo = mid + 1;
            continue;
        }
    } while (lo < hi);
    
    // check if lo and hi converged on the match
    if (block_num == (*buf_ptr)[hi].block_num) {
        index = hi;
    }
    
    // if no existing entry found, find index for new one
    if (index == -1) {
        index = (block_num < (*buf_ptr)[hi].block_num) ? hi : hi + 1;
    } else {
        // make sure that we return the right-most index in the case of multiple matches
        matches = 0;
        i = index + 1;
        while (i < num_full && block_num == (*buf_ptr)[i].block_num) {
            matches++;
            i++;
        }
        
        index += matches;
    }
    
    return index;
}

// PR-3105942: Coalesce writes to the same block in journal replay
// We coalesce writes by maintaining a dynamic sorted array of physical disk blocks
// to be replayed and the corresponding location in the journal which contains
// the most recent data for those blocks. The array is "played" once the all the
// blocks in the journal have been coalesced. The code for the case of conflicting/
// overlapping writes to a single block is the most dense. Because coalescing can
// disrupt the existing time-ordering of blocks in the journal playback, care
// is taken to catch any overlaps and keep the array consistent.
static int add_block(journal *jnl, struct bucket **buf_ptr, off_t block_num, size_t size, size_t offset, int32_t cksum, int *num_buckets_ptr, int *num_full_ptr) {
    int    blk_index, overwriting;
    
    // on return from lookup_bucket(), blk_index is the index into the table where block_num should be
    // inserted (or the index of the elem to overwrite).
    blk_index = lookup_bucket( buf_ptr, block_num, *num_full_ptr);
    
    // check if the index is within bounds (if we're adding this block to the end of
    // the table, blk_index will be equal to num_full)
    if (blk_index < 0 || blk_index > *num_full_ptr) {
        //printf("jnl: add_block: trouble adding block to co_buf\n");
        return -1;
    } // else printf("jnl: add_block: adding block 0x%llx at i=%d\n", block_num, blk_index);
    
    // Determine whether we're overwriting an existing entry by checking for overlap
    overwriting = do_overlap(jnl, buf_ptr, blk_index, block_num, size, offset, cksum, num_buckets_ptr, num_full_ptr);
    if (overwriting < 0) {
        return -1; // if we got an error, pass it along
    }
    
    // returns the index, or -1 on error
    blk_index = insert_block(jnl, buf_ptr, blk_index, block_num, size, offset, cksum, num_buckets_ptr, num_full_ptr, overwriting);
    
    return blk_index;
}

static void swap_block_list_header(journal *jnl, block_list_header *blhdr) {
    int i;
    
    blhdr->max_blocks = SWAP16(blhdr->max_blocks);
    blhdr->num_blocks = SWAP16(blhdr->num_blocks);
    blhdr->bytes_used = SWAP32(blhdr->bytes_used);
    blhdr->checksum   = SWAP32(blhdr->checksum);
    blhdr->flags      = SWAP32(blhdr->flags);
    
    if (blhdr->num_blocks >= ((jnl->jhdr->blhdr_size / sizeof(block_info)) - 1)) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: blhdr num blocks looks suspicious (%d / blhdr size %d).  not swapping.\n", blhdr->num_blocks, jnl->jhdr->blhdr_size);
        return;
    }
    
    for(i = 0; i < blhdr->num_blocks; i++) {
        blhdr->binfo[i].bnum    = SWAP64(blhdr->binfo[i].bnum);
        blhdr->binfo[i].u.bi.bsize   = SWAP32(blhdr->binfo[i].u.bi.bsize);
        blhdr->binfo[i].u.bi.b.cksum = SWAP32(blhdr->binfo[i].u.bi.b.cksum);
    }
}

static int replay_journal(journal *jnl) {
    int          i, bad_blocks=0;
    unsigned int   orig_checksum, checksum, check_block_checksums = 0;
    size_t         ret;
    size_t         max_bsize = 0;        /* protected by block_ptr */
    block_list_header *blhdr;
    off_t          offset, txn_start_offset=0, blhdr_offset, orig_jnl_start;
    char          *buff, *block_ptr=NULL;
    struct bucket *co_buf;
    int           num_buckets = STARTING_BUCKETS, num_full, check_past_jnl_end = 1, in_uncharted_territory = 0;
    uint32_t      last_sequence_num = 0;
    int           replay_retry_count = 0;
    
    LFHFS_LOG(LEVEL_DEFAULT, "replay_journal: start.\n");

    
    // wrap the start ptr if it points to the very end of the journal
    if (jnl->jhdr->start == jnl->jhdr->size) {
        jnl->jhdr->start = jnl->jhdr->jhdr_size;
    }
    if (jnl->jhdr->end == jnl->jhdr->size) {
        jnl->jhdr->end = jnl->jhdr->jhdr_size;
    }
    
    if (jnl->jhdr->start == jnl->jhdr->end) {
        LFHFS_LOG(LEVEL_DEFAULT, "replay_journal: journal empty.\n");
        goto success;
    }
    
    orig_jnl_start = jnl->jhdr->start;
    
    // allocate memory for the header_block.  we'll read each blhdr into this
    buff = hfs_malloc(jnl->jhdr->blhdr_size);
    
    // allocate memory for the coalesce buffer
    co_buf = hfs_malloc(num_buckets*sizeof(struct bucket));
    
restart_replay:
    
    // initialize entries
    for(i = 0; i < num_buckets; i++) {
        co_buf[i].block_num = -1;
    }
    num_full = 0; // empty at first
    
    
    while (check_past_jnl_end || jnl->jhdr->start != jnl->jhdr->end) {
        offset = blhdr_offset = jnl->jhdr->start;
        ret = read_journal_data(jnl, &offset, buff, jnl->jhdr->blhdr_size);
        if (ret != (size_t)jnl->jhdr->blhdr_size) {
            LFHFS_LOG(LEVEL_ERROR, "jnl: replay_journal: Could not read block list header block @ 0x%llx!\n", offset);
            goto bad_txn_handling;
        }
        
        blhdr = (block_list_header *)buff;
        
        orig_checksum = blhdr->checksum;
        blhdr->checksum = 0;
        if (jnl->flags & JOURNAL_NEED_SWAP) {
            // calculate the checksum based on the unswapped data
            // because it is done byte-at-a-time.
            orig_checksum = (unsigned int)SWAP32(orig_checksum);
            checksum = calc_checksum((char *)blhdr, BLHDR_CHECKSUM_SIZE);
            swap_block_list_header(jnl, blhdr);
        } else {
            checksum = calc_checksum((char *)blhdr, BLHDR_CHECKSUM_SIZE);
        }
        
        
        //
        // XXXdbg - if these checks fail, we should replay as much
        //          we can in the hopes that it will still leave the
        //          drive in a better state than if we didn't replay
        //          anything
        //
        if (checksum != orig_checksum) {
            if (check_past_jnl_end && in_uncharted_territory) {
                
                if (blhdr_offset != jnl->jhdr->end) {
                    LFHFS_LOG(LEVEL_ERROR, "jnl: Extra txn replay stopped @ %lld / 0x%llx\n", blhdr_offset, blhdr_offset);
                }
                
                check_past_jnl_end = 0;
                jnl->jhdr->end = blhdr_offset;
                continue;
            }
            
            LFHFS_LOG(LEVEL_ERROR, "jnl: replay_journal: bad block list header @ 0x%llx (checksum 0x%x != 0x%x)\n",
                   blhdr_offset, orig_checksum, checksum);
            
            if (blhdr_offset == orig_jnl_start) {
                // if there's nothing in the journal at all, just bail out altogether.
                goto bad_replay;
            }
            
            goto bad_txn_handling;
        }
        
        if (   (last_sequence_num != 0)
            && (blhdr->binfo[0].u.bi.b.sequence_num != 0)
            && (blhdr->binfo[0].u.bi.b.sequence_num != last_sequence_num)
            && (blhdr->binfo[0].u.bi.b.sequence_num != last_sequence_num+1)) {
            
            txn_start_offset = jnl->jhdr->end = blhdr_offset;
            
            if (check_past_jnl_end) {
                check_past_jnl_end = 0;
                LFHFS_LOG(LEVEL_ERROR, "jnl: 2: extra replay stopped @ %lld / 0x%llx (seq %d < %d)\n",
                       blhdr_offset, blhdr_offset, blhdr->binfo[0].u.bi.b.sequence_num, last_sequence_num);
                continue;
            }
            
            LFHFS_LOG(LEVEL_ERROR, "jnl: txn sequence numbers out of order in txn @ %lld / %llx! (%d < %d)\n",
                   blhdr_offset, blhdr_offset, blhdr->binfo[0].u.bi.b.sequence_num, last_sequence_num);
            goto bad_txn_handling;
        }
        last_sequence_num = blhdr->binfo[0].u.bi.b.sequence_num;
        
        if (blhdr_offset >= jnl->jhdr->end && jnl->jhdr->start <= jnl->jhdr->end) {
            if (last_sequence_num == 0) {
                check_past_jnl_end = 0;
                LFHFS_LOG(LEVEL_ERROR, "jnl: pre-sequence-num-enabled txn's - can not go further than end (%lld %lld).\n",
                       jnl->jhdr->start, jnl->jhdr->end);
                if (jnl->jhdr->start != jnl->jhdr->end) {
                    jnl->jhdr->start = jnl->jhdr->end;
                }
                continue;
            }
            LFHFS_LOG(LEVEL_ERROR, "jnl: examining extra transactions starting @ %lld / 0x%llx\n", blhdr_offset, blhdr_offset);
        }
        
        if (   blhdr->max_blocks <= 0 || blhdr->max_blocks > (jnl->jhdr->size/jnl->jhdr->jhdr_size)
            || blhdr->num_blocks <= 0 || blhdr->num_blocks > blhdr->max_blocks) {
            LFHFS_LOG(LEVEL_ERROR, "jnl: replay_journal: bad looking journal entry: max: %d num: %d\n",
                   blhdr->max_blocks, blhdr->num_blocks);
            goto bad_txn_handling;
        }
        
        max_bsize = 0;
        for (i = 1; i < blhdr->num_blocks; i++) {
            if (blhdr->binfo[i].bnum < 0 && blhdr->binfo[i].bnum != (off_t)-1) {
                LFHFS_LOG(LEVEL_ERROR, "jnl: replay_journal: bogus block number 0x%llx\n", blhdr->binfo[i].bnum);
                goto bad_txn_handling;
            }
            
            if ((size_t)blhdr->binfo[i].u.bi.bsize > max_bsize) {
                max_bsize = blhdr->binfo[i].u.bi.bsize;
            }
        }
        
        if (blhdr->flags & BLHDR_CHECK_CHECKSUMS) {
            check_block_checksums = 1;
            block_ptr = hfs_malloc(max_bsize);
        } else {
            block_ptr = NULL;
        }
        
        if (blhdr->flags & BLHDR_FIRST_HEADER) {
            txn_start_offset = blhdr_offset;
        }
        
        //printf("jnl: replay_journal: adding %d blocks in journal entry @ 0x%llx to co_buf\n",
        //       blhdr->num_blocks-1, jnl->jhdr->start);
        bad_blocks = 0;
        for (i = 1; i < blhdr->num_blocks; i++) {
            int size, ret_val;
            off_t number;
            
            size = blhdr->binfo[i].u.bi.bsize;
            number = blhdr->binfo[i].bnum;
            
            // don't add "killed" blocks
            if (number == (off_t)-1) {
                //printf("jnl: replay_journal: skipping killed fs block (index %d)\n", i);
            } else {
                
                if (check_block_checksums) {
                    int32_t disk_cksum;
                    off_t block_offset;
                    
                    block_offset = offset;
                    
                    // read the block so we can check the checksum
                    ret = read_journal_data(jnl, &block_offset, block_ptr, size);
                    if (ret != (size_t)size) {
                        LFHFS_LOG(LEVEL_ERROR, "jnl: replay_journal: Could not read journal entry data @ offset 0x%llx!\n", offset);
                        goto bad_txn_handling;
                    }
                    
                    disk_cksum = calc_checksum(block_ptr, size);
                    
                    // there is no need to swap the checksum from disk because
                    // it got swapped when the blhdr was read in.
                    if (blhdr->binfo[i].u.bi.b.cksum != 0 && disk_cksum != blhdr->binfo[i].u.bi.b.cksum) {
                        LFHFS_LOG(LEVEL_ERROR, "jnl: txn starting at %lld (%lld) @ index %3d bnum %lld (%d) with disk cksum != blhdr cksum (0x%.8x 0x%.8x)\n",
                               txn_start_offset, blhdr_offset, i, number, size, disk_cksum, blhdr->binfo[i].u.bi.b.cksum);
                        LFHFS_LOG(LEVEL_ERROR, "jnl: 0x%.8x 0x%.8x 0x%.8x 0x%.8x  0x%.8x 0x%.8x 0x%.8x 0x%.8x\n",
                               *(int *)&block_ptr[0*sizeof(int)], *(int *)&block_ptr[1*sizeof(int)], *(int *)&block_ptr[2*sizeof(int)], *(int *)&block_ptr[3*sizeof(int)],
                               *(int *)&block_ptr[4*sizeof(int)], *(int *)&block_ptr[5*sizeof(int)], *(int *)&block_ptr[6*sizeof(int)], *(int *)&block_ptr[7*sizeof(int)]);
                        
                        goto bad_txn_handling;
                    }
                }
                
                
                // add this bucket to co_buf, coalescing where possible
                // printf("jnl: replay_journal: adding block 0x%llx\n", number);
                ret_val = add_block(jnl, &co_buf, number, size, (size_t) offset, blhdr->binfo[i].u.bi.b.cksum, &num_buckets, &num_full);
                
                if (ret_val == -1) {
                    LFHFS_LOG(LEVEL_ERROR, "jnl: replay_journal: trouble adding block to co_buf\n");
                    goto bad_replay;
                } // else printf("jnl: replay_journal: added block 0x%llx at i=%d\n", number);
            }
            
            // increment offset
            offset += size;
            
            // check if the last block added puts us off the end of the jnl.
            // if so, we need to wrap to the beginning and take any remainder
            // into account
            //
            if (offset >= jnl->jhdr->size) {
                offset = jnl->jhdr->jhdr_size + (offset - jnl->jhdr->size);
            }
        }
        
        if (block_ptr) {
            hfs_free(block_ptr);
            block_ptr = NULL;
        }
        
        if (bad_blocks) {
        bad_txn_handling:
            /* Journal replay got error before it found any valid
             *  transations, abort replay */
            if (txn_start_offset == 0) {
                LFHFS_LOG(LEVEL_ERROR, "jnl: no known good txn start offset! aborting journal replay.\n");
                goto bad_replay;
            }
            
            /* Repeated error during journal replay, abort replay */
            if (replay_retry_count == 3) {
                LFHFS_LOG(LEVEL_ERROR, "jnl: repeated errors replaying journal! aborting journal replay.\n");
                goto bad_replay;
            }
            replay_retry_count++;
            
            /* There was an error replaying the journal (possibly
             * EIO/ENXIO from the device).  So retry replaying all
             * the good transactions that we found before getting
             * the error.
             */
            jnl->jhdr->start = orig_jnl_start;
            jnl->jhdr->end = txn_start_offset;
            check_past_jnl_end = 0;
            last_sequence_num = 0;
            LFHFS_LOG(LEVEL_ERROR, "jnl: restarting journal replay (%lld - %lld)!\n", jnl->jhdr->start, jnl->jhdr->end);
            goto restart_replay;
        }
        
        jnl->jhdr->start += blhdr->bytes_used;
        if (jnl->jhdr->start >= jnl->jhdr->size) {
            // wrap around and skip the journal header block
            jnl->jhdr->start = (jnl->jhdr->start % jnl->jhdr->size) + jnl->jhdr->jhdr_size;
        }
        
        if (jnl->jhdr->start == jnl->jhdr->end) {
            in_uncharted_territory = 1;
        }
    }
    
    if (jnl->jhdr->start != jnl->jhdr->end) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: start %lld != end %lld.  resetting end.\n", jnl->jhdr->start, jnl->jhdr->end);
        jnl->jhdr->end = jnl->jhdr->start;
    }
    
    //printf("jnl: replay_journal: replaying %d blocks\n", num_full);
    
    /*
     * make sure it's at least one page in size, so
     * start max_bsize at PAGE_SIZE
     */
    for (i = 0, max_bsize = PAGE_SIZE; i < num_full; i++) {
        
        if (co_buf[i].block_num == (off_t)-1)
            continue;
        
        if (co_buf[i].block_size > max_bsize)
            max_bsize = co_buf[i].block_size;
    }
    /*
     * round max_bsize up to the nearest PAGE_SIZE multiple
     */
    if (max_bsize & (PAGE_SIZE - 1)) {
        max_bsize = (max_bsize + PAGE_SIZE) & ~(PAGE_SIZE - 1);
    }
    
    block_ptr = hfs_malloc(max_bsize);
    
    // Replay the coalesced entries in the co-buf
    for(i = 0; i < num_full; i++) {
        size_t size = co_buf[i].block_size;
        off_t jnl_offset = (off_t) co_buf[i].jnl_offset;
        off_t number = co_buf[i].block_num;
        
        
        // printf("replaying co_buf[%d]: block 0x%llx, size 0x%x, jnl_offset 0x%llx\n", i, co_buf[i].block_num,
        //      co_buf[i].block_size, co_buf[i].jnl_offset);
        
        if (number == (off_t)-1) {
            // printf("jnl: replay_journal: skipping killed fs block\n");
        } else {
            
            // do journal read, and set the phys. block
            ret = read_journal_data(jnl, &jnl_offset, block_ptr, size);
            if (ret != size) {
                LFHFS_LOG(LEVEL_ERROR, "jnl: replay_journal: Could not read journal entry data @ offset 0x%llx!\n", jnl_offset);
                goto bad_replay;
            }
            
            if (update_fs_block(jnl, block_ptr, number, size) != 0) {
                goto bad_replay;
            }
        }
    }
    
    
    // done replaying; update jnl header
    if (write_journal_header(jnl, 1, jnl->jhdr->sequence_num) != 0) {
        goto bad_replay;
    }
    
    // free block_ptr
    if (block_ptr) {
        hfs_free(block_ptr);
        block_ptr = NULL;
    }
    
    // free the coalesce buffer
    hfs_free(co_buf);
    co_buf = NULL;
    
    hfs_free(buff);
    
success:
    LFHFS_LOG(LEVEL_DEFAULT, "replay_journal: success.\n");
    return 0;
    
bad_replay:
    hfs_free(block_ptr);
    hfs_free(co_buf);
    hfs_free(buff);
    
    LFHFS_LOG(LEVEL_ERROR, "replay_journal: error.\n");
    return -1;
}

// buffer_written:
// This function get executed after a buffer has been written to its
// final destination.
// This function lets us know when a buffer has been
// flushed to disk.  Originally (kext), it was called from deep
// within the driver stack and thus is quite limited in what it could do.
// Notably, it could not initiate any new i/o's or allocate/free memory.
static void buffer_written(transaction *tr, GenericLFBuf *bp) {

    journal      *jnl;
    transaction  *ctr, *prev=NULL, *next;
    size_t        i;
    size_t        bufsize, amt_flushed, total_bytes;
    
    
    // snarf out the bits we want
    bufsize = bp->uDataSize;
    
    // then we've already seen it
    if (tr == NULL) {
        return;
    }
    
    CHECK_TRANSACTION(tr);
    
    jnl = tr->jnl;
    
    CHECK_JOURNAL(jnl);
    
    amt_flushed = tr->num_killed;
    total_bytes = tr->total_bytes;
    
    // update the number of blocks that have been flushed.
    // this buf may represent more than one block so take
    // that into account.
    amt_flushed     += tr->num_flushed;
    tr->num_flushed += bufsize;
    
    // if this transaction isn't done yet, just return as
    // there is nothing to do.
    //
    // NOTE: we are careful to not reference anything through
    //       the tr pointer after doing the OSAddAtomic().  if
    //       this if statement fails then we are the last one
    //       and then it's ok to dereference "tr".
    //
    if ((amt_flushed + bufsize) < total_bytes) {
        return;
    }
    
    // this will single thread checking the transaction
    lock_oldstart(jnl);
    
    if (tr->total_bytes == (int)0xfbadc0de) {
        // then someone beat us to it...
        unlock_oldstart(jnl);
        return;
    }
    
    // mark this so that we're the owner of dealing with the
    // cleanup for this transaction
    tr->total_bytes = 0xfbadc0de;
    
    if (jnl->flags & JOURNAL_INVALID)
        goto transaction_done;
    
    //printf("jnl: tr 0x%x (0x%llx 0x%llx) in jnl 0x%x completed.\n",
    //   tr, tr->journal_start, tr->journal_end, jnl);
    
    // find this entry in the old_start[] index and mark it completed
    for(i = 0; i < sizeof(jnl->old_start)/sizeof(jnl->old_start[0]); i++) {
        
        if ((off_t)(jnl->old_start[i] & ~(0x8000000000000000ULL)) == tr->journal_start) {
            jnl->old_start[i] &= ~(0x8000000000000000ULL);
            break;
        }
    }
    
    if (i >= sizeof(jnl->old_start)/sizeof(jnl->old_start[0])) {
        panic("jnl: buffer_flushed: did not find tr w/start @ %lld (tr %p, jnl %p)\n",
              tr->journal_start, tr, jnl);
    }
    
    
    // if we are here then we need to update the journal header
    // to reflect that this transaction is complete
    if (tr->journal_start == jnl->active_start) {
        jnl->active_start = tr->journal_end;
        tr->journal_start = tr->journal_end = (off_t)0;
    }
    
    // go through the completed_trs list and try to coalesce
    // entries, restarting back at the beginning if we have to.
    for (ctr = jnl->completed_trs; ctr; prev=ctr, ctr=next) {
        if (ctr->journal_start == jnl->active_start) {
            jnl->active_start = ctr->journal_end;
            if (prev) {
                prev->next = ctr->next;
            }
            if (ctr == jnl->completed_trs) {
                jnl->completed_trs = ctr->next;
            }
            
            next           = jnl->completed_trs;   // this starts us over again
            ctr->next      = jnl->tr_freeme;
            jnl->tr_freeme = ctr;
            ctr            = NULL;
            
        } else if (tr->journal_end == ctr->journal_start) {
            ctr->journal_start = tr->journal_start;
            next               = jnl->completed_trs;  // this starts us over again
            ctr                = NULL;
            tr->journal_start  = tr->journal_end = (off_t)0;
            
        } else if (tr->journal_start == ctr->journal_end) {
            ctr->journal_end  = tr->journal_end;
            next              = ctr->next;
            tr->journal_start = tr->journal_end = (off_t)0;
        } else if (ctr->next && ctr->journal_end == ctr->next->journal_start) {
            // coalesce the next entry with this one and link the next
            // entry in at the head of the tr_freeme list
            next              = ctr->next;           // temporarily use the "next" variable
            ctr->journal_end  = next->journal_end;
            ctr->next         = next->next;
            next->next        = jnl->tr_freeme;      // link in the next guy at the head of the tr_freeme list
            jnl->tr_freeme    = next;
            
            next              = jnl->completed_trs;  // this starts us over again
            ctr               = NULL;
            
        } else {
            next = ctr->next;
        }
    }
    
    // if this is true then we didn't merge with anyone
    // so link ourselves in at the head of the completed
    // transaction list.
    if (tr->journal_start != 0) {
        // put this entry into the correct sorted place
        // in the list instead of just at the head.
        
        prev = NULL;
        for (ctr = jnl->completed_trs; ctr && tr->journal_start > ctr->journal_start; prev=ctr, ctr=ctr->next) {
            // just keep looping
        }
        
        if (ctr == NULL && prev == NULL) {
            jnl->completed_trs = tr;
            tr->next = NULL;
            
        } else if (ctr == jnl->completed_trs) {
            tr->next = jnl->completed_trs;
            jnl->completed_trs = tr;
            
        } else {
            tr->next = prev->next;
            prev->next = tr;
        }
        
    } else {
        // if we're here this tr got merged with someone else so
        // put it on the list to be free'd
        tr->next       = jnl->tr_freeme;
        jnl->tr_freeme = tr;
    }
transaction_done:
    unlock_oldstart(jnl);
    
    unlock_condition(jnl, &jnl->asyncIO);
}

static size_t write_journal_data(journal *jnl, off_t *offset, void *data, size_t len) {
    return do_journal_io(jnl, offset, data, len, JNL_WRITE);
}

static size_t read_journal_data(journal *jnl, off_t *offset, void *data, size_t len) {
    return do_journal_io(jnl, offset, data, len, JNL_READ);
}


// This function sets the size of the tbuffer and the
// size of the blhdr.  It assumes that jnl->jhdr->size
// and jnl->jhdr->jhdr_size are already valid.
static void size_up_tbuffer(journal *jnl, uint32_t tbuffer_size, uint32_t phys_blksz) {
    //
    // one-time initialization based on how much memory
    // there is in the machine.
    //
    if (def_tbuffer_size == 0) {
        uint64_t memsize = 0;
        size_t l = sizeof(memsize);
        sysctlbyname("hw.memsize", &memsize, &l, NULL, 0);
        
        if (memsize < (256*1024*1024)) {
            def_tbuffer_size = DEFAULT_TRANSACTION_BUFFER_SIZE;
        } else if (memsize < (512*1024*1024)) {
            def_tbuffer_size = DEFAULT_TRANSACTION_BUFFER_SIZE * 2;
        } else if (memsize < (1024*1024*1024)) {
            def_tbuffer_size = DEFAULT_TRANSACTION_BUFFER_SIZE * 3;
        } else {
            def_tbuffer_size = (uint32_t)(DEFAULT_TRANSACTION_BUFFER_SIZE * (memsize / (256*1024*1024)));
        }
    }
    
    // For analyzer
    if (!(jnl->jhdr->jhdr_size > 0)) {
        panic("jnl->jhdr->jhdr_size is %d", jnl->jhdr->jhdr_size);
    }
    
    // size up the transaction buffer... can't be larger than the number
    // of blocks that can fit in a block_list_header block.
    if (tbuffer_size == 0) {
        jnl->tbuffer_size = def_tbuffer_size;
    } else {
        // make sure that the specified tbuffer_size isn't too small
        if (tbuffer_size < jnl->jhdr->blhdr_size * 2) {
            tbuffer_size = jnl->jhdr->blhdr_size * 2;
        }
        // and make sure it's an even multiple of the block size
        if ((tbuffer_size % jnl->jhdr->jhdr_size) != 0) {
            tbuffer_size -= (tbuffer_size % jnl->jhdr->jhdr_size);
        }
        
        jnl->tbuffer_size = tbuffer_size;
    }
    
    if (jnl->tbuffer_size > (jnl->jhdr->size / 2)) {
        jnl->tbuffer_size = (uint32_t)(jnl->jhdr->size / 2);
    }
    
    if (jnl->tbuffer_size > MAX_TRANSACTION_BUFFER_SIZE) {
        jnl->tbuffer_size = MAX_TRANSACTION_BUFFER_SIZE;
    }
    
    jnl->jhdr->blhdr_size = (jnl->tbuffer_size / jnl->jhdr->jhdr_size) * sizeof(block_info);
    if (jnl->jhdr->blhdr_size < phys_blksz) {
        jnl->jhdr->blhdr_size = phys_blksz;
    } else if ((jnl->jhdr->blhdr_size % phys_blksz) != 0) {
        // have to round up so we're an even multiple of the physical block size
        jnl->jhdr->blhdr_size = (jnl->jhdr->blhdr_size + (phys_blksz - 1)) & ~(phys_blksz - 1);
    }
}


static int write_journal_header(journal *jnl, int updating_start, uint32_t sequence_num) {
    static int num_err_prints = 0;
    int ret=0;
    off_t jhdr_offset = 0;

    // Flush the track cache if we're not doing force-unit-access
    // writes.
    if (!updating_start && (jnl->flags & JOURNAL_DO_FUA_WRITES) == 0) {
        
        dk_synchronize_t sync_request = {
            .options            = DK_SYNCHRONIZE_OPTION_BARRIER,
        };
        
        /*
         * If device doesn't support barrier-only flush, or
         * the journal is on a different device, use full flush.
         */
        if (!(jnl->flags & JOURNAL_FEATURE_BARRIER) || (jnl->jdev != jnl->fsdev)) {
            sync_request.options = 0;
            jnl->flush_counter++;
        }
        
        ret = ioctl(jnl->jdev->psFSRecord->iFD, DKIOCSYNCHRONIZE, (caddr_t)&sync_request);
    }
    if (ret != 0) {
        //
        // Only print this error if it's a different error than the
        // previous one, or if it's the first time for this device
        // or if the total number of printfs is less than 25.  We
        // allow for up to 25 printfs to insure that some make it
        // into the on-disk syslog.  Otherwise if we only printed
        // one, it's possible it would never make it to the syslog
        // for the root volume and that makes debugging hard.
        //
        if (   ret != jnl->last_flush_err
            || (jnl->flags & JOURNAL_FLUSHCACHE_ERR) == 0
            || num_err_prints++ < 25) {
            
            LFHFS_LOG(LEVEL_ERROR, "jnl: flushing fs disk buffer returned 0x%x\n", ret);
            
            jnl->flags |= JOURNAL_FLUSHCACHE_ERR;
            jnl->last_flush_err = ret;
        }
    }
    
    jnl->jhdr->sequence_num = sequence_num;
    jnl->jhdr->checksum = 0;
    jnl->jhdr->checksum = calc_checksum((char *)jnl->jhdr, JOURNAL_HEADER_CKSUM_SIZE);
    
    if (do_journal_io(jnl, &jhdr_offset, jnl->header_buf, jnl->jhdr->jhdr_size, JNL_WRITE|JNL_HEADER) != (size_t)jnl->jhdr->jhdr_size) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: write_journal_header: error writing the journal header!\n");
        jnl->flags |= JOURNAL_INVALID;
        return -1;
    }
    
    // If we're not doing force-unit-access writes, then we
    // have to flush after writing the journal header so that
    // a future transaction doesn't sneak out to disk before
    // the header does and thus overwrite data that the old
    // journal header refers to.  Saw this exact case happen
    // on an IDE bus analyzer with Larry Barras so while it
    // may seem obscure, it's not.
    //
    if (updating_start && (jnl->flags & JOURNAL_DO_FUA_WRITES) == 0) {
        
        dk_synchronize_t sync_request = {
            .options            = DK_SYNCHRONIZE_OPTION_BARRIER,
        };
        
        /*
         * If device doesn't support barrier-only flush, or
         * the journal is on a different device, use full flush.
         */
        if (!(jnl->flags & JOURNAL_FEATURE_BARRIER) || (jnl->jdev != jnl->fsdev)) {
            sync_request.options = 0;
            jnl->flush_counter++;
        }
        
        ioctl(jnl->jdev->psFSRecord->iFD, DKIOCSYNCHRONIZE, (caddr_t)&sync_request);
    }
    return 0;
}

static int journal_binfo_cmp(const void *a, const void *b) {
    
    const block_info *bi_a = (const struct block_info *)a;
    const block_info *bi_b = (const struct block_info *)b;
    daddr64_t res;
    
    if (bi_a->bnum == (off_t)-1) {
        return 1;
    }
    if (bi_b->bnum == (off_t)-1) {
        return -1;
    }
    
    // don't have to worry about negative block
    // numbers so this is ok to do.
    GenericLFBuf *psGenBufA, *psGenBufB;
    psGenBufA = (void*)bi_a->u.bp;
    psGenBufB = (void*)bi_b->u.bp;
    res = psGenBufA->uBlockN - psGenBufB->uBlockN;
    
    return (int)res;
}

// finish_end_transaction:

static int finish_end_transaction(transaction *tr, errno_t (*callback)(void*), void *callback_arg) {
    int                i;
    size_t             amt;
    size_t             ret = 0;
    off_t              end;
    journal           *jnl = tr->jnl;
    GenericLFBuf       *bp = NULL, **bparray = NULL;
    block_list_header *blhdr=NULL, *next=NULL;
    size_t             tbuffer_offset;
    int                bufs_written = 0;
    int                ret_val = 0;
    
    end  = jnl->jhdr->end;
    
    for (blhdr = tr->blhdr; blhdr; blhdr = (block_list_header *)((long)blhdr->binfo[0].bnum)) {
        
        amt = blhdr->bytes_used;
        
        blhdr->binfo[0].u.bi.b.sequence_num = tr->sequence_num;
        
        blhdr->checksum = 0;
        blhdr->checksum = calc_checksum((char *)blhdr, BLHDR_CHECKSUM_SIZE);
        
        bparray = hfs_malloc(blhdr->num_blocks * sizeof(buf_t));
        tbuffer_offset = jnl->jhdr->blhdr_size;
        
        // for each block in the block-header,
        for (i = 1; i < blhdr->num_blocks; i++) {
            size_t   bsize;
            
            /*
             * finish preparing the shadow buf_t before
             * calculating the individual block checksums
             */
            if (blhdr->binfo[i].bnum != (off_t)-1) {
                daddr64_t blkno;
                
                bp = (void*)blhdr->binfo[i].u.bp;
                blkno  = bp->uPhyCluster;
                // update this so we write out the correct physical block number!
                blhdr->binfo[i].bnum = (off_t)(blkno);

                bparray[i] = bp;
                bsize = bp->uDataSize;
                blhdr->binfo[i].u.bi.bsize   = (uint32_t)bsize;
                blhdr->binfo[i].u.bi.b.cksum = calc_checksum(&((char *)blhdr)[tbuffer_offset], (uint32_t)bsize);
            } else {
                bparray[i] = NULL;
                bsize = blhdr->binfo[i].u.bi.bsize;
                blhdr->binfo[i].u.bi.b.cksum = 0;
            }
            tbuffer_offset += bsize;
        }

        /*
         * if we fired off the journal_write_header asynchronously in
         * 'end_transaction', we need to wait for its completion
         * before writing the actual journal data
         */
        wait_condition(jnl, &jnl->writing_header, "finish_end_transaction");
        
        if (jnl->write_header_failed == FALSE)
            ret = write_journal_data(jnl, &end, blhdr, amt);
        else
            ret_val = -1;

        #if HFS_CRASH_TEST
            CRASH_ABORT(CRASH_ABORT_JOURNAL_AFTER_JOURNAL_DATA, jnl->fsmount->psHfsmount, NULL);
        #endif

        /*
         * put the bp pointers back so that we can
         * make the final pass on them
         */
        for (i = 1; i < blhdr->num_blocks; i++)
            blhdr->binfo[i].u.bp = (void*)bparray[i];
        
        hfs_free(bparray);
        
        if (ret_val == -1)
            goto bad_journal;
        
        if (ret != amt) {
            LFHFS_LOG(LEVEL_ERROR, "jnl: end_transaction: only wrote %zu of %zu bytes to the journal!\n",
                   ret, amt);
            
            ret_val = -1;
            goto bad_journal;
        }
    }
    jnl->jhdr->end  = end;    // update where the journal now ends
    tr->journal_end = end;    // the transaction ends here too
    
    if (tr->journal_start == 0 || tr->journal_end == 0) {
        panic("jnl: end_transaction: bad tr journal start/end: 0x%llx 0x%llx\n",
              tr->journal_start, tr->journal_end);
    }
    
    if (write_journal_header(jnl, 0, jnl->saved_sequence_num) != 0) {
        ret_val = -1;
        goto bad_journal;
    }
    
    #if HFS_CRASH_TEST
        CRASH_ABORT(CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, jnl->fsmount->psHfsmount, NULL);
    #endif
    
    /*
     * If the caller supplied a callback, call it now that the blocks have been
     * written to the journal.  This is used by journal_relocate so, for example,
     * the file system can change its pointer to the new journal.
     */
    if (callback != NULL && callback(callback_arg) != 0) {
        ret_val = -1;
        goto bad_journal;
    }
    
    // the buffer_flushed_callback will only be called for the
    // real blocks that get flushed so we have to account for
    // the block_list_headers here.
    //
    tr->num_flushed = tr->num_blhdrs * jnl->jhdr->blhdr_size;
    
    lock_condition(jnl, &jnl->asyncIO, "finish_end_transaction");
    
    //
    // setup for looping through all the blhdr's.
    //
    for (blhdr = tr->blhdr; blhdr; blhdr = next) {
        uint16_t    num_blocks;
        
        /*
         * grab this info ahead of issuing the buf_bawrites...
         * once the last one goes out, its possible for blhdr
         * to be freed (especially if we get preempted) before
         * we do the last check of num_blocks or
         * grab the next blhdr pointer...
         */
        next = (block_list_header *)((long)blhdr->binfo[0].bnum);
        num_blocks = blhdr->num_blocks;
        
        /*
         * we can re-order the buf ptrs because everything is written out already
         */
        qsort(&blhdr->binfo[1], num_blocks-1, sizeof(block_info), journal_binfo_cmp);
        
        /*
         * need to make sure that the loop issuing the buf_bawrite's
         * does not touch blhdr once the last buf_bawrite has been
         * issued... at that point, we no longer have a legitmate
         * reference on the associated storage since it will be
         * released upon the completion of that last buf_bawrite
         */
        for (i = num_blocks-1; i >= 1; i--) {
            if (blhdr->binfo[i].bnum != (off_t)-1)
                break;
            num_blocks--;
        }
        for (i = 1; i < num_blocks; i++) {
            
            if ((bp = (void*)blhdr->binfo[i].u.bp)) {

                errno_t ret_val = 0;

                #if JOURNAL_DEBUG
                    printf("journal write physical: bp %p, psVnode %p, uBlockN %llu, uPhyCluster %llu uLockCnt %u\n",
                           bp, bp->psVnode, bp->uBlockN, bp->uPhyCluster, bp->uLockCnt);
                #endif
                
                lf_hfs_generic_buf_clear_cache_flag(bp, GEN_BUF_WRITE_LOCK);
                ret_val = lf_hfs_generic_buf_write(bp);

                #if HFS_CRASH_TEST
                    CRASH_ABORT(CRASH_ABORT_JOURNAL_IN_BLOCK_DATA, jnl->fsmount->psHfsmount, NULL);
                #endif

                if (ret_val) {
                    LFHFS_LOG(LEVEL_ERROR, "jnl: raw_readwrite_write_mount inside finish_end_transaction returned %d.\n", ret_val);
                }

                buffer_written(tr, bp);
                
                lf_hfs_generic_buf_unlock(bp);
                lf_hfs_generic_buf_release(bp);
                
                bufs_written++;
            }
        }
    }
    #if HFS_CRASH_TEST
        CRASH_ABORT(CRASH_ABORT_JOURNAL_AFTER_BLOCK_DATA, jnl->fsmount->psHfsmount, NULL);
    #endif
    if (bufs_written == 0) {
        /*
         * since we didn't issue any buf_bawrite's, there is no
         * async trigger to cause the memory associated with this
         * transaction to be freed... so, move it to the garbage
         * list now
         */
        lock_oldstart(jnl);
        
        tr->next       = jnl->tr_freeme;
        jnl->tr_freeme = tr;
        
        unlock_oldstart(jnl);
        
        unlock_condition(jnl, &jnl->asyncIO);
    }
    
    //printf("jnl: end_tr: tr @ 0x%x, jnl-blocks: 0x%llx - 0x%llx. exit!\n",
    //   tr, tr->journal_start, tr->journal_end);
    
bad_journal:
    if (ret_val == -1) {
        abort_transaction(jnl, tr);        // cleans up list of extents to be trimmed
        
        /*
         * 'flush_aborted' is protected by the flushing condition... we need to
         * set it before dropping the condition so that it will be
         * noticed in 'end_transaction'... we add this additional
         * aborted condition so that we can drop the 'flushing' condition
         * before grabbing the journal lock... this avoids a deadlock
         * in 'end_transaction' which is holding the journal lock while
         * waiting for the 'flushing' condition to clear...
         * everyone else will notice the JOURNAL_INVALID flag
         */
        jnl->flush_aborted = TRUE;
        
        unlock_condition(jnl, &jnl->flushing);
        journal_lock(jnl);
        
        jnl->flags |= JOURNAL_INVALID;
        jnl->old_start[sizeof(jnl->old_start)/sizeof(jnl->old_start[0]) - 1] &= ~0x8000000000000000LL;
        
        journal_unlock(jnl);
    } else
        unlock_condition(jnl, &jnl->flushing);
    
    return (ret_val);
}
static off_t free_space(journal *jnl) {
    off_t free_space_offset;
    
    if (jnl->jhdr->start < jnl->jhdr->end) {
        free_space_offset = jnl->jhdr->size - (jnl->jhdr->end - jnl->jhdr->start) - jnl->jhdr->jhdr_size;
    } else if (jnl->jhdr->start > jnl->jhdr->end) {
        free_space_offset = jnl->jhdr->start - jnl->jhdr->end;
    } else {
        // journal is completely empty
        free_space_offset = jnl->jhdr->size - jnl->jhdr->jhdr_size;
    }
    
    return free_space_offset;
}

static void dump_journal(journal *jnl) {
    transaction *ctr;
    
    printf("  jdev_offset %.8llx\n", jnl->jdev_offset);
    printf("  magic: 0x%.8x\n", jnl->jhdr->magic);
    printf("  start: 0x%.8llx\n", jnl->jhdr->start);
    printf("  end:   0x%.8llx\n", jnl->jhdr->end);
    printf("  size:  0x%.8llx\n", jnl->jhdr->size);
    printf("  blhdr size: %d\n", jnl->jhdr->blhdr_size);
    printf("  jhdr size: %d\n", jnl->jhdr->jhdr_size);
    printf("  chksum: 0x%.8x\n", jnl->jhdr->checksum);
    
    printf("  completed transactions:\n");
    for (ctr = jnl->completed_trs; ctr; ctr = ctr->next) {
        printf("    0x%.8llx - 0x%.8llx\n", ctr->journal_start, ctr->journal_end);
    }
}

// The journal must be locked on entry to this function.
// The "desired_size" is in bytes.
static int check_free_space( journal *jnl,
                             int desired_size,
                             boolean_t *delayed_header_write,
                             uint32_t sequence_num) {

    size_t    i;
    int    counter=0;
    
    //printf("jnl: check free space (desired 0x%x, avail 0x%Lx)\n",
    //       desired_size, free_space(jnl));
    
    if (delayed_header_write)
        *delayed_header_write = FALSE;
    
    while (1) {
        int old_start_empty;
        
        // make sure there's space in the journal to hold this transaction
        if (free_space(jnl) > desired_size && jnl->old_start[0] == 0) {
            break;
        }
        if (counter++ == 5000) {
            dump_journal(jnl);
            panic("jnl: check_free_space: buffer flushing isn't working "
                  "(jnl @ %p s %lld e %lld f %lld [active start %lld]).\n", jnl,
                  jnl->jhdr->start, jnl->jhdr->end, free_space(jnl), jnl->active_start);
        }
        if (counter > 7500) {
            return ENOSPC;
        }
        
        // here's where we lazily bump up jnl->jhdr->start.  we'll consume
        // entries until there is enough space for the next transaction.
        old_start_empty = 1;
        lock_oldstart(jnl);
        
        for (i = 0; i < sizeof(jnl->old_start)/sizeof(jnl->old_start[0]); i++) {
            int   lcl_counter;
            
            lcl_counter = 0;
            while (jnl->old_start[i] & 0x8000000000000000LL) {
                if (lcl_counter++ > 10000) {
                    panic("jnl: check_free_space: tr starting @ 0x%llx not flushing (jnl %p).\n",
                          jnl->old_start[i], jnl);
                }
                
                unlock_oldstart(jnl);
                if (jnl->flush) {
                    jnl->flush(jnl->flush_arg);
                }
                usleep(10000);
                lock_oldstart(jnl);
            }
            
            if (jnl->old_start[i] == 0) {
                continue;
            }
            
            old_start_empty   = 0;
            jnl->jhdr->start  = jnl->old_start[i];
            jnl->old_start[i] = 0;
            
            if (free_space(jnl) > desired_size) {
                
                if (delayed_header_write)
                    *delayed_header_write = TRUE;
                else {
                    unlock_oldstart(jnl);
                    write_journal_header(jnl, 1, sequence_num);
                    lock_oldstart(jnl);
                }
                break;
            }
        }
        unlock_oldstart(jnl);
        
        // if we bumped the start, loop and try again
        if (i < sizeof(jnl->old_start)/sizeof(jnl->old_start[0])) {
            continue;
        } else if (old_start_empty) {
            //
            // if there is nothing in old_start anymore then we can
            // bump the jhdr->start to be the same as active_start
            // since it is possible there was only one very large
            // transaction in the old_start array.  if we didn't do
            // this then jhdr->start would never get updated and we
            // would wind up looping until we hit the panic at the
            // start of the loop.
            //
            jnl->jhdr->start = jnl->active_start;
            
            if (delayed_header_write)
                *delayed_header_write = TRUE;
            else
                write_journal_header(jnl, 1, sequence_num);
            continue;
        }
        
        
        // if the file system gave us a flush function, call it to so that
        // it can flush some blocks which hopefully will cause some transactions
        // to complete and thus free up space in the journal.
        if (jnl->flush) {
            jnl->flush(jnl->flush_arg);
        }
        
        // wait for a while to avoid being cpu-bound (this will
        // put us to sleep for 10 milliseconds)
        usleep(10000);
    }

    return 0;
}

static void lock_condition(journal *jnl, ConditionalFlag_S *psCondFlag, __unused const char *condition_name) {
    
    lock_flush(jnl);
    
    while (psCondFlag->uFlag) {
        pthread_cond_wait(&psCondFlag->sCond, &jnl->flock);
    }
    
    psCondFlag->uFlag = TRUE;
    unlock_flush(jnl);
}

static void wait_condition(journal *jnl, ConditionalFlag_S *psCondFlag, __unused const char *condition_name) {
    
    if (!psCondFlag->uFlag)
        return;
    
    lock_flush(jnl);
    
    while (psCondFlag->uFlag) {
        pthread_cond_wait(&psCondFlag->sCond, &jnl->flock);
    }
    
    unlock_flush(jnl);
}

static void unlock_condition(journal *jnl, ConditionalFlag_S *psCondFlag) {
    lock_flush(jnl);
    
    psCondFlag->uFlag = FALSE;
    pthread_cond_broadcast(&psCondFlag->sCond);
    
    unlock_flush(jnl);
}

/*
 * End a transaction:
 * 1) Determine if it is time to commit the transaction or not:
 * If the transaction is small enough, and we're not forcing
 * a write to disk, the "active" transaction becomes the "current" transaction,
 * and will be reused for the next transaction that is started (group commit).
 *
 * 2) Commit:
 * If the transaction gets written to disk (because force_it is true, or no
 * group commit, or the transaction is sufficiently full), the blocks get
 * written into the journal first, then they are written to their final location
 * asynchronously. When those async writes complete, the transaction can be freed
 * and removed from the journal.
 *
 * 3) Callback:
 * An optional callback can be supplied.  If given, it is called after the
 * the blocks have been written to the journal, but before the async writes
 * of those blocks to their normal on-disk locations.  This is used by
 * journal_relocate so that the location of the journal can be changed and
 * flushed to disk before the blocks get written to their normal locations.
 * Note that the callback is only called if the transaction gets written to
 * the journal during this end_transaction call; you probably want to set the
 * force_it flag.
 *
 * 4) Free blocks' Generic Buff.
 *
 * Inputs:
 *    tr           Transaction to add to the journal
 *    force_it     If true, force this transaction to the on-disk journal immediately.
 *    callback     See description above.  Pass NULL for no callback.
 *    callback_arg Argument passed to callback routine.
 *
 * Result
 *         0        No errors
 *        -1        An error occurred.  The journal is marked invalid.
 */
static int end_transaction(transaction *tr, int force_it, errno_t (*callback)(void*), void *callback_arg, boolean_t drop_lock) {

    block_list_header  *blhdr=NULL, *next=NULL;
    int           i, ret_val = 0;
    journal      *jnl = tr->jnl;
    GenericLFBuf *bp;
    size_t        tbuffer_offset;
    
    if (jnl->cur_tr) {
        panic("jnl: jnl @ %p already has cur_tr %p, new tr: %p\n",
              jnl, jnl->cur_tr, tr);
    }
    
    // if there weren't any modified blocks in the transaction
    // just save off the transaction pointer and return.
    if (tr->total_bytes == (int)jnl->jhdr->blhdr_size) {
        jnl->cur_tr = tr;
        goto done;
    }
    
    // if our transaction buffer isn't very full, just hang
    // on to it and don't actually flush anything.  this is
    // what is known as "group commit".  we will flush the
    // transaction buffer if it's full or if we have more than
    // one of them so we don't start hogging too much memory.
    //
    // We also check the device supports UNMAP/TRIM, and if so,
    // the number of extents waiting to be trimmed.  If it is
    // small enough, then keep accumulating more (so we can
    // reduce the overhead of trimming).  If there was a prior
    // trim error, then we stop issuing trims for this
    // volume, so we can also coalesce transactions.
    //
    if (   force_it == 0
        && (jnl->flags & JOURNAL_NO_GROUP_COMMIT) == 0
        && tr->num_blhdrs < 3
        && (tr->total_bytes <= ((tr->tbuffer_size*tr->num_blhdrs) - tr->tbuffer_size/8))
        && (!(jnl->flags & JOURNAL_USE_UNMAP) || (tr->trim.extent_count < jnl_trim_flush_limit))) {

        jnl->cur_tr = tr;
        goto done;
    }
    
    lock_condition(jnl, &jnl->flushing, "end_transaction");
    
    /*
     * if the previous 'finish_end_transaction' was being run
     * asynchronously, it could have encountered a condition
     * that caused it to mark the journal invalid... if that
     * occurred while we were waiting for it to finish, we
     * need to notice and abort the current transaction
     */
    if ((jnl->flags & JOURNAL_INVALID) || jnl->flush_aborted == TRUE) {
        unlock_condition(jnl, &jnl->flushing);
        
        abort_transaction(jnl, tr);
        ret_val = -1;
        goto done;
    }
    
    /*
     * Store a pointer to this transaction's trim list so that
     * future transactions can find it.
     *
     * Note: if there are no extents in the trim list, then don't
     * bother saving the pointer since nothing can add new extents
     * to the list (and other threads/transactions only care if
     * there is a trim pending).
     */
    lf_lck_rw_lock_exclusive(&jnl->trim_lock);
    if (jnl->async_trim != NULL)
        panic("jnl: end_transaction: async_trim already non-NULL!");
    if (tr->trim.extent_count > 0)
        jnl->async_trim = &tr->trim;
    lf_lck_rw_unlock_exclusive(&jnl->trim_lock);
    
    /*
     * snapshot the transaction sequence number while we are still behind
     * the journal lock since it will be bumped upon the start of the
     * next transaction group which may overlap the current journal flush...
     * we pass the snapshot into write_journal_header during the journal
     * flush so that it can write the correct version in the header...
     * because we hold the 'flushing' condition variable for the duration
     * of the journal flush, 'saved_sequence_num' remains stable
     */
    jnl->saved_sequence_num = jnl->sequence_num;
    
    /*
     * if we're here we're going to flush the transaction buffer to disk.
     * 'check_free_space' will not return untl there is enough free
     * space for this transaction in the journal and jnl->old_start[0]
     * is avaiable for use
     */
    check_free_space(jnl, tr->total_bytes, &tr->delayed_header_write, jnl->saved_sequence_num);
    
    // range check the end index
    if (jnl->jhdr->end <= 0 || jnl->jhdr->end > jnl->jhdr->size) {
        panic("jnl: end_transaction: end is bogus 0x%llx (sz 0x%llx)\n",
              jnl->jhdr->end, jnl->jhdr->size);
    }

    // this transaction starts where the current journal ends
    tr->journal_start = jnl->jhdr->end;
    
    lock_oldstart(jnl);
    /*
     * Because old_start is locked above, we can cast away the volatile qualifier before passing it to memmove.
     * slide everyone else down and put our latest guy in the last
     * entry in the old_start array
     */
    memmove(__CAST_AWAY_QUALIFIER(&jnl->old_start[0], volatile, void *), __CAST_AWAY_QUALIFIER(&jnl->old_start[1], volatile, void *), sizeof(jnl->old_start)-sizeof(jnl->old_start[0]));
    jnl->old_start[sizeof(jnl->old_start)/sizeof(jnl->old_start[0]) - 1] = tr->journal_start | 0x8000000000000000LL;
    
    unlock_oldstart(jnl);
    
    // go over the blocks in the transaction.
    // for each block, call the fpCallback and copy the content into the journal buffer
    for (blhdr = tr->blhdr; blhdr; blhdr = next) {
        char         *blkptr;
        size_t       bsize;
        
        tbuffer_offset = jnl->jhdr->blhdr_size;
        
        for (i = 1; i < blhdr->num_blocks; i++) {
            
            if (blhdr->binfo[i].bnum != (off_t)-1) {

                bp = (GenericLFBuf*)blhdr->binfo[i].u.bp;
                
                if (bp == NULL) {
                    panic("jnl: inconsistent binfo (NULL bp w/bnum %lld; jnl @ %p, tr %p)\n",
                          blhdr->binfo[i].bnum, jnl, tr);
                }
                
                bsize = bp->uDataSize;
                
                blkptr = (char *)&((char *)blhdr)[tbuffer_offset];
                
                int iRet;
            retry:
                iRet = lf_hfs_generic_buf_take_ownership(bp, NULL);
                if (iRet == EAGAIN) {
                    goto retry;
                } else if (iRet) {
                    LFHFS_LOG(LEVEL_ERROR, "jnl: end_transaction: lf_hfs_generic_buf_take_ownership returned %d.\n", iRet);
                    ret_val = -1;
                    goto done;
                }
                
                if (!(bp->uCacheFlags & GEN_BUF_WRITE_LOCK)) {
                    panic("GEN_BUF_WRITE_LOCK should be set!");
                }
                
                // Call the buffer callback
                if (bp->pfFunc) {
                    bp->pfFunc(bp, bp->pvCallbackArgs);
                    bp->pfFunc = NULL;
                }
                
                if (bp->uCacheFlags & GEN_BUF_LITTLE_ENDIAN) {
                    panic("We do not want to write a GEN_BUF_LITTLE_ENDIAN buffer to media!");
                }
                
                // copy the data into the transaction buffer...
                memcpy(blkptr, bp->pvData, bsize);

                blhdr->binfo[i].u.bp = (void*)bp;
                
            } else {
                // bnum == -1, only true if a block was "killed"
                bsize = blhdr->binfo[i].u.bi.bsize;
            }
            tbuffer_offset += bsize;
        }
        next = (block_list_header *)((long)blhdr->binfo[0].bnum);
    }

    #if HFS_CRASH_TEST
        CRASH_ABORT(CRASH_ABORT_JOURNAL_BEFORE_FINISH, jnl->fsmount->psHfsmount, NULL);
    #endif

    ret_val = finish_end_transaction(tr, callback, callback_arg);

done:
    if (drop_lock == TRUE) {
        journal_unlock(jnl);
    }
    return (ret_val);
}

static void abort_transaction(journal *jnl, transaction *tr) {

    block_list_header *blhdr, *next;
    // for each block list header, iterate over the blocks then
    // free up the memory associated with the block list.
    for (blhdr = tr->blhdr; blhdr; blhdr = next) {
        int    i;
        
        for (i = 1; i < blhdr->num_blocks; i++) {
            GenericLFBufPtr bp;
            
            if (blhdr->binfo[i].bnum == (off_t)-1)
                continue;
            
            bp = (void*)blhdr->binfo[i].u.bp;

            // Release the buffers
            lf_hfs_generic_buf_clear_cache_flag(bp, GEN_BUF_WRITE_LOCK);
            if (lf_hfs_generic_buf_validate_owner(bp)) { // abort_transaction can be called before or after we take ownership
                lf_hfs_generic_buf_release(bp);
            }
            
        }
        next = (block_list_header *)((long)blhdr->binfo[0].bnum);
        
        // we can free blhdr here since we won't need it any more
        blhdr->binfo[0].bnum = 0xdeadc0de;
        hfs_free(blhdr);
    }
    
    /*
     * If the transaction we're aborting was the async transaction, then
     * tell the current transaction that there is no pending trim
     * any more.
     */
    lf_lck_rw_lock_exclusive(&jnl->trim_lock);
    if (jnl->async_trim == &tr->trim)
        jnl->async_trim = NULL;
    lf_lck_rw_unlock_exclusive(&jnl->trim_lock);
    
    
    if (tr->trim.extents) {
        hfs_free(tr->trim.extents);
    }
    tr->trim.allocated_count = 0;
    tr->trim.extent_count = 0;
    tr->trim.extents = NULL;
    tr->tbuffer     = NULL;
    tr->blhdr       = NULL;
    tr->total_bytes = 0xdbadc0de;
    hfs_free(tr);
}

static void swap_journal_header(journal *jnl) {
    jnl->jhdr->magic      = SWAP32(jnl->jhdr->magic);
    jnl->jhdr->endian     = SWAP32(jnl->jhdr->endian);
    jnl->jhdr->start      = SWAP64(jnl->jhdr->start);
    jnl->jhdr->end        = SWAP64(jnl->jhdr->end);
    jnl->jhdr->size       = SWAP64(jnl->jhdr->size);
    jnl->jhdr->blhdr_size = SWAP32(jnl->jhdr->blhdr_size);
    jnl->jhdr->checksum   = SWAP32(jnl->jhdr->checksum);
    jnl->jhdr->jhdr_size  = SWAP32(jnl->jhdr->jhdr_size);
    jnl->jhdr->sequence_num  = SWAP32(jnl->jhdr->sequence_num);
}

// this isn't a great checksum routine but it will do for now.
// we use it to checksum the journal header and the block list
// headers that are at the start of each transaction.
static unsigned int calc_checksum(const char *ptr, int len) {
    int i;
    unsigned int cksum=0;
    
    // this is a lame checksum but for now it'll do
    for(i = 0; i < len; i++, ptr++) {
        cksum = (cksum << 8) ^ (cksum + *(unsigned char *)ptr);
    }
    
    return (~cksum);
}


static size_t do_journal_io(journal *jnl, off_t *offset, void *data, size_t len, int direction) {
    off_t     curlen = len;
    size_t    io_sz = 0;
    off_t     max_iosize;
#if 0 // TBD
    int       err;
    buf_t     bp;
    off_t     accumulated_offset = 0;
    ExtendedVCB *vcb = HFSTOVCB(jnl->fsmount->psHfsmount);
#endif
    
    if (*offset < 0 || *offset > jnl->jhdr->size) {
        panic("jnl: do_jnl_io: bad offset 0x%llx (max 0x%llx)\n", *offset, jnl->jhdr->size);
    }
    
    if (direction & JNL_WRITE)
        max_iosize = jnl->max_write_size;
    else if (direction & JNL_READ)
        max_iosize = jnl->max_read_size;
    else
        max_iosize = 128 * 1024;
    
again:
    
    // Determine the Current R/W Length, taking cyclic wrap around into account
    if (*offset + curlen > jnl->jhdr->size && *offset != 0 && jnl->jhdr->size != 0) {
        if (*offset == jnl->jhdr->size) {
            *offset = jnl->jhdr->jhdr_size;
        } else {
            curlen = jnl->jhdr->size - *offset;
        }
    }
    
    if (curlen > max_iosize) {
        curlen = max_iosize;
    }
    
    if (curlen <= 0) {
        panic("jnl: do_jnl_io: curlen == %lld, offset 0x%llx len %zd\n", curlen, *offset, len);
    }
    
    if (*offset == 0 && (direction & JNL_HEADER) == 0) {
        panic("jnl: request for i/o to jnl-header without JNL_HEADER flag set! (len %lld, data %p)\n", curlen, data);
    }
    

    // Perform the I/O
    uint64_t phyblksize = jnl->fsmount->psHfsmount->hfs_physical_block_size; 
    uint64_t uBlkNum    = jnl->jdev_blknum+(*offset)/phyblksize;
    
    if (direction & JNL_READ) {
        raw_readwrite_read_mount(jnl->jdev, uBlkNum, phyblksize, data, curlen, NULL, NULL);

    } else if (direction & JNL_WRITE) {
        raw_readwrite_write_mount(jnl->jdev, uBlkNum, phyblksize, data, curlen, NULL, NULL);
    }

    // Move to the next section
    *offset += curlen;
    io_sz   += curlen;
    
    if (io_sz != len) {
        // handle wrap-around
        data    = (char *)data + curlen;
        curlen  = len - io_sz;
        if (*offset >= jnl->jhdr->size) {
            *offset = jnl->jhdr->jhdr_size;
        }
        goto again;
    }
    
    return io_sz;
}

static size_t read_journal_header(journal *jnl, void *data, size_t len) {
    off_t hdr_offset = 0;
    
    return do_journal_io(jnl, &hdr_offset, data, len, JNL_READ|JNL_HEADER);
}

static void get_io_info(struct vnode *devvp, size_t phys_blksz, journal *jnl) {
    off_t    readblockcnt;
    off_t    writeblockcnt;
    off_t    readmaxcnt=0, tmp_readmaxcnt;
    off_t    writemaxcnt=0, tmp_writemaxcnt;
    off_t    readsegcnt, writesegcnt;
    
    // First check the max read size via several different mechanisms...
    ioctl(devvp->psFSRecord->iFD, DKIOCGETMAXBYTECOUNTREAD, (caddr_t)&readmaxcnt);
    
    if (ioctl(devvp->psFSRecord->iFD, DKIOCGETMAXBLOCKCOUNTREAD, (caddr_t)&readblockcnt) == 0) {
        tmp_readmaxcnt = readblockcnt * phys_blksz;
        if (readmaxcnt == 0 || (readblockcnt > 0 && tmp_readmaxcnt < readmaxcnt)) {
            readmaxcnt = tmp_readmaxcnt;
        }
    }
    
    if (ioctl(devvp->psFSRecord->iFD, DKIOCGETMAXSEGMENTCOUNTREAD, (caddr_t)&readsegcnt)) {
        readsegcnt = 0;
    }
    
    if (readsegcnt > 0 && (readsegcnt * PAGE_SIZE) < readmaxcnt) {
        readmaxcnt = readsegcnt * PAGE_SIZE;
    }
    
    if (readmaxcnt == 0) {
        readmaxcnt = 128 * 1024;
    } else if (readmaxcnt > UINT32_MAX) {
        readmaxcnt = UINT32_MAX;
    }
    
    
    // Now check the max writes size via several different mechanisms...
    ioctl(devvp->psFSRecord->iFD, DKIOCGETMAXBYTECOUNTWRITE, (caddr_t)&writemaxcnt);
    
    if (ioctl(devvp->psFSRecord->iFD, DKIOCGETMAXBLOCKCOUNTWRITE, (caddr_t)&writeblockcnt) == 0) {
        tmp_writemaxcnt = writeblockcnt * phys_blksz;
        if (writemaxcnt == 0 || (writeblockcnt > 0 && tmp_writemaxcnt < writemaxcnt)) {
            writemaxcnt = tmp_writemaxcnt;
        }
    }
    
    if (ioctl(devvp->psFSRecord->iFD, DKIOCGETMAXSEGMENTCOUNTWRITE, (caddr_t)&writesegcnt)) {
        writesegcnt = 0;
    }
    
    if (writesegcnt > 0 && (writesegcnt * PAGE_SIZE) < writemaxcnt) {
        writemaxcnt = writesegcnt * PAGE_SIZE;
    }
    
    if (writemaxcnt == 0) {
        writemaxcnt = 128 * 1024;
    } else if (writemaxcnt > UINT32_MAX) {
        writemaxcnt = UINT32_MAX;
    }
    
    jnl->max_read_size  = readmaxcnt;
    jnl->max_write_size = writemaxcnt;
}

// this is a work function used to free up transactions that
// completed. they can't be free'd from buffer_flushed_callback
// because it is called from deep with the disk driver stack
// and thus can't do something that would potentially cause
// paging.  it gets called by each of the journal api entry
// points so stuff shouldn't hang around for too long.
static void free_old_stuff(journal *jnl) {
    transaction *tr, *next;
    block_list_header  *blhdr=NULL, *next_blhdr=NULL;
    
    if (jnl->tr_freeme == NULL)
        return;
    
    lock_oldstart(jnl);
    tr = jnl->tr_freeme;
    jnl->tr_freeme = NULL;
    unlock_oldstart(jnl);
    
    for(; tr; tr=next) {
        for (blhdr = tr->blhdr; blhdr; blhdr = next_blhdr) {
            next_blhdr = (block_list_header *)((long)blhdr->binfo[0].bnum);
            blhdr->binfo[0].bnum = 0xdeadc0de;
            
            hfs_free(blhdr);
            
            KERNEL_DEBUG(0xbbbbc01c, jnl, tr, tr->tbuffer_size, 0, 0);
        }
        next = tr->next;
        hfs_free(tr);
    }
}

// Allocate a new active transaction.
// The function does the following:
// 1) mallocs memory for a transaction structure and a buffer
// 2) initializes the transaction structure and the buffer (invalid CRC + 0x5a)
static errno_t journal_allocate_transaction(journal *jnl) {
    transaction *tr;
    
    tr = hfs_mallocz(sizeof(transaction));
    
    tr->tbuffer_size = jnl->tbuffer_size;
    
    tr->tbuffer = hfs_malloc(tr->tbuffer_size);
    
    // journal replay code checksum check depends on this.
    memset(tr->tbuffer, 0, BLHDR_CHECKSUM_SIZE);
    // Fill up the rest of the block with unimportant bytes (0x5a 'Z' chosen for visibility)
    memset(tr->tbuffer + BLHDR_CHECKSUM_SIZE, 0x5a, jnl->jhdr->blhdr_size - BLHDR_CHECKSUM_SIZE);
    
    tr->blhdr = (block_list_header *)tr->tbuffer;
    tr->blhdr->max_blocks = (jnl->jhdr->blhdr_size / sizeof(block_info)) - 1;
    tr->blhdr->num_blocks = 1;      // accounts for this header block
    tr->blhdr->bytes_used = jnl->jhdr->blhdr_size;
    tr->blhdr->flags = BLHDR_CHECK_CHECKSUMS | BLHDR_FIRST_HEADER;
    
    tr->sequence_num = ++jnl->sequence_num;
    tr->num_blhdrs  = 1;
    tr->total_bytes = jnl->jhdr->blhdr_size;
    tr->jnl         = jnl;
    
    jnl->active_tr  = tr;
    
    return 0;
}

int journal_kill_block(journal *jnl, GenericLFBuf *psGenBuf) {
    int                i;
    uint64_t           uflags;
    block_list_header *blhdr;
    transaction       *tr;

    #if JOURNAL_DEBUG
        printf("journal_kill_block: psGenBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uPhyCluster %llu uLockCnt %u\n",
           psGenBuf, psGenBuf->psVnode, psGenBuf->uBlockN, psGenBuf->uDataSize ,psGenBuf->uPhyCluster, psGenBuf->uLockCnt);
    #endif
    
    CHECK_JOURNAL(jnl);
    free_old_stuff(jnl);
    
    if (jnl->flags & JOURNAL_INVALID) {
        lf_hfs_generic_buf_clear_cache_flag(psGenBuf, GEN_BUF_WRITE_LOCK);
        lf_hfs_generic_buf_release(psGenBuf);
        return 0;
    }
    
    tr = jnl->active_tr;
    CHECK_TRANSACTION(tr);
    
    if (jnl->owner != pthread_self()) {
        panic("jnl: journal_kill_block: called w/out a transaction! jnl %p, owner %p, curact %p\n",
              jnl, jnl->owner, pthread_self());
    }
    
    uflags = psGenBuf->uCacheFlags;
    
    if ( !(uflags & GEN_BUF_WRITE_LOCK))
        panic("jnl: journal_kill_block: called with bp not B_LOCKED");
    
    /*
     * bp must be BL_BUSY and B_LOCKED
     * first check if it's already part of this transaction
     */
    for (blhdr = tr->blhdr; blhdr; blhdr = (block_list_header *)((long)blhdr->binfo[0].bnum)) {
        
        for (i = 1; i < blhdr->num_blocks; i++) {
            if (psGenBuf == (void*)blhdr->binfo[i].u.bp) {
                
                // if the block has the DELWRI and FILTER bits sets, then
                // things are seriously weird.  if it was part of another
                // transaction then journal_modify_block_start() should
                // have force it to be written.
                //
                //if ((bflags & B_DELWRI) && (bflags & B_FILTER)) {
                //    panic("jnl: kill block: this defies all logic! bp 0x%x\n", bp);
                //} else {
                tr->num_killed += psGenBuf->uDataSize;
                //}
                blhdr->binfo[i].bnum = (off_t)-1;
                blhdr->binfo[i].u.bp = NULL;
                blhdr->binfo[i].u.bi.bsize = psGenBuf->uDataSize;
                
                lf_hfs_generic_buf_clear_cache_flag(psGenBuf, GEN_BUF_WRITE_LOCK);
                lf_hfs_generic_buf_release(psGenBuf);
                
                return 0;
            }
        }
    }
    
    /*
     * We did not find the block in any transaction buffer but we still
     * need to release it or else it will be left locked forever.
     */
    lf_hfs_generic_buf_clear_cache_flag(psGenBuf, GEN_BUF_WRITE_LOCK);
    lf_hfs_generic_buf_release(psGenBuf);

    return 0;
}

int journal_is_clean(struct vnode *jvp,
                     off_t         offset,
                     off_t         journal_size,
                     struct vnode *fsvp,
                     size_t        min_fs_block_size,
                     struct mount  *fsmount) {
    
    journal        jnl;
    uint32_t    phys_blksz;
    int        ret;
    int        orig_checksum, checksum;
    
    /* Get the real physical block size. */
    if (ioctl(jvp->psFSRecord->iFD, DKIOCGETBLOCKSIZE, (caddr_t)&phys_blksz)) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_is_clean: failed to get device block size.\n");
        ret = EINVAL;
        goto cleanup_jdev_name;
    }
    
    if (phys_blksz > (uint32_t)min_fs_block_size) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_is_clean: error: phys blksize %d bigger than min fs blksize %zd\n",
               phys_blksz, min_fs_block_size);
        ret = EINVAL;
        goto cleanup_jdev_name;
    }
    
    if (journal_size < (256*1024) || journal_size > (MAX_JOURNAL_SIZE)) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_is_clean: journal size %lld looks bogus.\n", journal_size);
        ret = EINVAL;
        goto cleanup_jdev_name;
    }
    
    if ((journal_size % phys_blksz) != 0) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_is_clean: journal size 0x%llx is not an even multiple of block size 0x%x\n",
               journal_size, phys_blksz);
        ret = EINVAL;
        goto cleanup_jdev_name;
    }
    
    memset(&jnl, 0, sizeof(jnl));
    
    jnl.header_buf = hfs_malloc(phys_blksz);
    jnl.header_buf_size = phys_blksz;

    // Keep a point to the mount around for use in IO throttling.
    jnl.fsmount = fsmount;

    get_io_info(jvp, phys_blksz, &jnl);
    
    jnl.jhdr = (journal_header *)jnl.header_buf;
    memset(jnl.jhdr, 0, sizeof(journal_header));
    
    jnl.jdev        = jvp;
    jnl.jdev_offset = offset;
    jnl.jdev_blknum = (uint32_t)(offset / phys_blksz);
    jnl.fsdev       = fsvp;
    
    // we have to set this up here so that do_journal_io() will work
    jnl.jhdr->jhdr_size = phys_blksz;
    
    if (read_journal_header(&jnl, jnl.jhdr, phys_blksz) != (unsigned)phys_blksz) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_is_clean: could not read %d bytes for the journal header.\n",
               phys_blksz);
        ret = EINVAL;
        goto get_out;
    }
    
    orig_checksum = jnl.jhdr->checksum;
    jnl.jhdr->checksum = 0;
    
    if (jnl.jhdr->magic == SWAP32(JOURNAL_HEADER_MAGIC)) {
        // do this before the swap since it's done byte-at-a-time
        orig_checksum = SWAP32(orig_checksum);
        checksum = calc_checksum((char *)jnl.jhdr, JOURNAL_HEADER_CKSUM_SIZE);
        swap_journal_header(&jnl);
        jnl.flags |= JOURNAL_NEED_SWAP;
    } else {
        checksum = calc_checksum((char *)jnl.jhdr, JOURNAL_HEADER_CKSUM_SIZE);
    }
    
    if (jnl.jhdr->magic != JOURNAL_HEADER_MAGIC && jnl.jhdr->magic != OLD_JOURNAL_HEADER_MAGIC) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_is_clean: journal magic is bad (0x%x != 0x%x)\n",
               jnl.jhdr->magic, JOURNAL_HEADER_MAGIC);
        ret = EINVAL;
        goto get_out;
    }
    
    if (orig_checksum != checksum) {
        LFHFS_LOG(LEVEL_ERROR, "jnl: journal_is_clean: journal checksum is bad (0x%x != 0x%x)\n", orig_checksum, checksum);
        ret = EINVAL;
        goto get_out;
    }
    
    //
    // if the start and end are equal then the journal is clean.
    // otherwise it's not clean and therefore an error.
    //
    if (jnl.jhdr->start == jnl.jhdr->end) {
        ret = 0;
    } else {
        ret = EBUSY;    // so the caller can differentiate an invalid journal from a "busy" one
    }
    
get_out:
    hfs_free(jnl.header_buf);
cleanup_jdev_name:
    return ret;
}

uint32_t journal_current_txn(journal *jnl) {
    return jnl->sequence_num + (jnl->active_tr || jnl->cur_tr ? 0 : 1);
}

