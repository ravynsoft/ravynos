//
//  lf_hfs_generic_buf.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_generic_buf_h
#define lf_hfs_generic_buf_h

#include "lf_hfs.h"

#define BUF_SKIP_NONLOCKED      0x01
#define BUF_SKIP_LOCKED         0x02
#define BUF_SCAN_CLEAN          0x04    /* scan the clean buffers */
#define BUF_SCAN_DIRTY          0x08    /* scan the dirty buffers */
#define BUF_NOTIFY_BUSY         0x10    /* notify the caller about the busy pages during the scan */

// uCacheFlags:
#define    GEN_BUF_WRITE_LOCK      0x00001000 // When set, the buffer does not get written to media. It will be written as part of a journal transaction.
#define    GEN_BUF_NON_CACHED      0x00002000 // If set, the buffer is not cached.
#define    GEN_BUF_IS_UPTODATE     0x00004000 // Set if memory content is equal or newer than media content
#define    GEN_BUF_PHY_BLOCK       0x00008000 // Indicates that the uBlockN field contains a physical block number
#define    GEN_BUF_LITTLE_ENDIAN   0x00010000 // When set, the data in the buffer contains small-endian data and should not be written to media

typedef struct GenericBuffer {
    
    uint64_t        uCacheFlags;
    pthread_mutex_t sLock;          // Sync access to buffer arguments + data
    pthread_t       pLockingThread; // Allows recursive lock by the same thread
    uint32_t        uLockCnt;       // Allows recursive lock by the same thread
    pthread_t       sOwnerThread;   // Current owner of buffer.
    pthread_cond_t  sOwnerCond;     // Clicked everytime a buffer owner is released.
    uint32_t        uUseCnt;        // Counts the number of buffer allocations
    void*           pvData;
    uint32_t        uDataSize;
    uint32_t        uValidBytes;
    daddr64_t       uBlockN;
    vnode_t         psVnode;
    uint64_t        uFlags;
    uint64_t        uPhyCluster;
    void           (*pfFunc)(struct GenericBuffer *psBuf, void *pvArg); // A function to be called at the last minute before disk-write
    void            *pvCallbackArgs;                                    // pfFunc args
} GenericLFBuf, *GenericLFBufPtr;

typedef struct {
    uint32_t buf_cache_size;
    uint32_t max_buf_cache_size;
    uint32_t max_gen_buf_uncached;
    uint32_t gen_buf_uncached;
    uint32_t buf_cache_remove;
    uint32_t buf_cache_cleanup;

    uint64_t buf_total_allocated_size;
} CacheStats_S;

extern CacheStats_S gCacheStat;


GenericLFBufPtr     lf_hfs_generic_buf_allocate( vnode_t psVnode, daddr64_t uBlockN, uint32_t uBlockSize, uint64_t uFlags );
int                 lf_hfs_generic_buf_take_ownership(GenericLFBuf *psBuf, pthread_mutex_t *pSem);
int                 lf_hfs_generic_buf_take_ownership_retry(GenericLFBuf *psBuf);
int                 lf_hfs_generic_buf_validate_owner(GenericLFBuf *psBuf);
GenericLFBufPtr     lf_hfs_generic_buf_duplicate(GenericLFBufPtr pBuff, uint32_t uExtraCacheFlags);
errno_t             lf_hfs_generic_buf_read( GenericLFBufPtr psBuf );
errno_t             lf_hfs_generic_buf_write( GenericLFBufPtr psBuf );
void                lf_hfs_generic_buf_invalidate( GenericLFBufPtr psBuf );
void                lf_hfs_generic_buf_release( GenericLFBufPtr psBuf );
void                lf_hfs_generic_buf_clear( GenericLFBufPtr psBuf );
void                lf_hfs_generic_buf_set_cache_flag(GenericLFBufPtr psBuf, uint64_t uCacheFlags);
void                lf_hfs_generic_buf_clear_cache_flag(GenericLFBufPtr psBuf, uint64_t uCacheFlags);
void                lf_hfs_generic_buf_override_owner(GenericLFBufPtr psBuf);
void                lf_hfs_generic_buf_lock(GenericLFBufPtr psBuf);
void                lf_hfs_generic_buf_unlock(GenericLFBufPtr psBuf);
void                lf_hfs_generic_buf_cache_init( void );
void                lf_hfs_generic_buf_cache_deinit( void );
void                lf_hfs_generic_buf_cache_clear_by_iFD( int iFD );
void                lf_hfs_generic_buf_cache_update( GenericLFBufPtr psBuf );
void                lf_hfs_generic_buf_cache_remove_vnode(vnode_t vp);
void                lf_hfs_generic_buf_cache_UnLockBufCache(void);
void                lf_hfs_generic_buf_cache_LockBufCache(void);

typedef int (*IterateCallback)(GenericLFBuf *psBuff, void *pvArgs);
int lf_hfs_generic_buf_write_iterate(vnode_t psVnode, IterateCallback pfCallback, uint32_t uFlags, void *pvArgs);


#endif /* lf_hfs_generic_buf_h */
