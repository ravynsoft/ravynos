//
//  lf_hfs_generic_buf.c
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#include "lf_hfs_generic_buf.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_rangelist.h"
#include "lf_hfs_locks.h"
#include "lf_hfs_logger.h"
#include <sys/queue.h>
#include <assert.h>

#define GEN_BUF_ALLOC_DEBUG 0

TAILQ_HEAD(buf_cache_head, buf_cache_entry);

struct buf_cache_entry {
    TAILQ_ENTRY(buf_cache_entry) buf_cache_link;
    GenericLFBuf sBuf;
};

boolean_t buf_cache_state = false;
struct buf_cache_head buf_cache_list;
pthread_mutex_t buf_cache_mutex;      /* protects access to buffer cache data */


#define BUF_CACHE_MAX_ENTRIES_UPPER_LIMIT   (140)
#define BUF_CACHE_MAX_ENTRIES_LOWER_LIMIT   (128)
#define BUF_CACHE_MAX_DATA_UPPER_LIMIT (1536*1024)
#define BUF_CACHE_MAX_DATA_LOWER_LIMIT (1024*1024)

CacheStats_S gCacheStat = {0};

#define IGNORE_MOUNT_FD         (INT_MAX)

void lf_hfs_generic_buf_cache_init( void );
void lf_hfs_generic_buf_cache_deinit( void );
struct buf_cache_entry *lf_hfs_generic_buf_cache_find( GenericLFBufPtr psBuf );
struct buf_cache_entry *lf_hfs_generic_buf_cache_find_by_phy_cluster(int iFD, uint64_t uPhyCluster, uint64_t uBlockSize);
struct buf_cache_entry *lf_hfs_generic_buf_cache_find_gen_buf(GenericLFBufPtr psBuf);
GenericLFBuf           *lf_hfs_generic_buf_cache_add( GenericLFBuf *psBuf );
void lf_hfs_generic_buf_cache_update( GenericLFBufPtr psBuf );
void lf_hfs_generic_buf_cache_copy( struct buf_cache_entry *entry, GenericLFBufPtr psBuf );
void lf_hfs_generic_buf_cache_remove( struct buf_cache_entry *entry );
void lf_hfs_generic_buf_cache_remove_all( int iFD );
void lf_hfs_generic_buf_ref(GenericLFBuf *psBuf);
void lf_hfs_generic_buf_rele(GenericLFBuf *psBuf);

// lf_hfs_generic_buf_take_ownership
// Take ownership on this buff.
// When the function returns zero, we own the buffer it is locked by our thread.
// When EAGAIN is returned, another thread raced us to own this buffer. Try again.
// ETIMEDOUT indicates that we timeout waiting for the buffer owner to release it
int lf_hfs_generic_buf_take_ownership(GenericLFBuf *psBuf, pthread_mutex_t *pSem) {
    lf_lck_mtx_lock(&psBuf->sLock);

    if ((psBuf->uUseCnt) && (psBuf->sOwnerThread != pthread_self())) {
        
        // Someone else owns the buffer
        if (pSem) {
            lf_lck_mtx_unlock(pSem);
        }

        // Wait for the buffer to get released
        struct timespec sWaitTime = {.tv_sec = 3, .tv_nsec = 0};
        
        int iWaitErr = lf_cond_wait_relative(&psBuf->sOwnerCond, &psBuf->sLock, &sWaitTime);
        if (iWaitErr == ETIMEDOUT) {
            LFHFS_LOG(LEVEL_ERROR, "lf_hfs_generic_buf_take_ownership_retry: ETIMEDOUT on %p", psBuf);
            return(ETIMEDOUT);
        } else if (iWaitErr) {
            LFHFS_LOG(LEVEL_ERROR, "lf_hfs_generic_buf_take_ownership_retry: lf_cond_wait_relative returned %d on %p", iWaitErr, psBuf);
            return(EINVAL);
        }
        
        // Buffer owner change, Retry.
        lf_lck_mtx_unlock(&psBuf->sLock);
        return(EAGAIN);
    }
    
    // We own the buffer
    assert(psBuf->uLockCnt == 0);
    assert(psBuf->uUseCnt == 0);
    psBuf->pLockingThread = pthread_self();
    psBuf->sOwnerThread   = pthread_self();
    psBuf->uUseCnt++;
    psBuf->uLockCnt++;
    return(0);
}

// Function: lf_hfs_generic_buf_allocate
// Allocate GenericBuff structure and if exists, attach to a previously allocated buffer of the same physical block.
GenericLFBufPtr lf_hfs_generic_buf_allocate( vnode_t psVnode, daddr64_t uBlockN, uint32_t uBlockSize, uint64_t uFlags ) {

    uint64_t uPhyCluster   = 0;
    uint64_t uInClusterOffset = 0;
    GenericLFBufPtr psBuf  = NULL;
    GenericLFBuf     sBuf  = {0};
    struct buf_cache_entry *psCacheEntry = NULL;

    assert(psVnode);
    
    if (uFlags & GEN_BUF_PHY_BLOCK) {
        uPhyCluster   = uBlockN;
    } else {
        // Determine PHY block number
        uint64_t uStartCluster = 0;
        int iError = raw_readwrite_get_cluster_from_offset(psVnode,
                                                           uBlockSize*uBlockN,
                                                           &uStartCluster,
                                                           &uInClusterOffset,
                                                           NULL );
        if (iError != 0) {
            panic("Error calculating uPhyCluster");
        }
        
        uint64_t uReadOffset = (HFSTOVCB(psVnode->sFSParams.vnfs_mp->psHfsmount)->hfsPlusIOPosOffset +
                                uStartCluster * HFSTOVCB(psVnode->sFSParams.vnfs_mp->psHfsmount)->blockSize) + uInClusterOffset;
        
        uPhyCluster = uReadOffset / HFSTOVCB(psVnode->sFSParams.vnfs_mp->psHfsmount)->hfs_physical_block_size;
    }

    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_allocate: psVnode %p, uBlockN %llu, uBlockSize %u, uFlags 0x%llx, uPhyCluster %llu: ",
               psVnode, uBlockN, uBlockSize, uFlags, uPhyCluster);
    #endif

    // Check buffer cache, if a memory buffer already allocated for this physical block
    if ( buf_cache_state && !(uFlags & GEN_BUF_NON_CACHED)) {
    retry:
        lf_lck_mtx_lock(&buf_cache_mutex);

        psCacheEntry = lf_hfs_generic_buf_cache_find_by_phy_cluster(VNODE_TO_IFD(psVnode), uPhyCluster, uBlockSize);
        if (psCacheEntry) {
            // buffer exists, share.
            TAILQ_REMOVE(&buf_cache_list, psCacheEntry, buf_cache_link);
            TAILQ_INSERT_HEAD(&buf_cache_list, psCacheEntry, buf_cache_link);

            psBuf = &psCacheEntry->sBuf;
            #if GEN_BUF_ALLOC_DEBUG
                printf("Already in cache: %p (UseCnt %u uCacheFlags 0x%llx)\n", psBuf, psBuf->uUseCnt, psBuf->uCacheFlags);
            #endif
            int iRet = lf_hfs_generic_buf_take_ownership(psBuf, &buf_cache_mutex);
            if (iRet == EAGAIN) {
                goto retry;
            } else if (iRet) {
                LFHFS_LOG(LEVEL_ERROR, "lf_hfs_generic_buf_allocate: lf_hfs_generic_buf_take_ownership returned %d.\n", iRet);
                return(NULL);
            } 
            
            lf_hfs_generic_buf_unlock(psBuf);
            lf_lck_mtx_unlock(&buf_cache_mutex);
            return(psBuf);
        }

        lf_lck_mtx_unlock(&buf_cache_mutex);
    }

    // Not found in cache, need to create a GenBuf
    sBuf.uBlockN       = uBlockN;
    sBuf.uDataSize     = uBlockSize;
    sBuf.psVnode       = psVnode;
    sBuf.uPhyCluster   = uPhyCluster;
    sBuf.uCacheFlags   = uFlags;
    sBuf.uUseCnt       = 1;
    sBuf.sOwnerThread = pthread_self();

    if ( buf_cache_state && !(uFlags & GEN_BUF_NON_CACHED)) {
        
        // Add to cache
        lf_lck_mtx_lock(&buf_cache_mutex);
        
        GenericLFBufPtr psCachedBuf = lf_hfs_generic_buf_cache_add(&sBuf);
        
        lf_cond_init(&psCachedBuf->sOwnerCond);
        lf_lck_mtx_init(&psCachedBuf->sLock);

        if (psCachedBuf) {
            if (uFlags & (GEN_BUF_IS_UPTODATE | GEN_BUF_LITTLE_ENDIAN)) {
                lf_hfs_generic_buf_lock(psCachedBuf);
                lf_hfs_generic_buf_set_cache_flag(psCachedBuf, uFlags & (GEN_BUF_IS_UPTODATE | GEN_BUF_LITTLE_ENDIAN));
                lf_hfs_generic_buf_unlock(psCachedBuf);
            }
        }
        
        lf_lck_mtx_unlock(&buf_cache_mutex);
        #if GEN_BUF_ALLOC_DEBUG
            printf("Added to cache %p\n", psCachedBuf);
        #endif
        return psCachedBuf;
        
    } else {
        // Alloc memomry for a non-cached buffer
        psBuf  = hfs_mallocz(sizeof(GenericLFBuf));
        if (!psBuf) {
            goto error;
        }
        memcpy(psBuf, &sBuf, sizeof(*psBuf));
        psBuf->pvData = hfs_mallocz(psBuf->uDataSize);
        if (!psBuf->pvData) {
            goto error;
        }

        lf_cond_init(&psBuf->sOwnerCond);
        lf_lck_mtx_init(&psBuf->sLock);

        gCacheStat.gen_buf_uncached++;
        if (gCacheStat.gen_buf_uncached > gCacheStat.max_gen_buf_uncached) {
            gCacheStat.max_gen_buf_uncached = gCacheStat.gen_buf_uncached;
        }
        if (uFlags & (GEN_BUF_IS_UPTODATE | GEN_BUF_LITTLE_ENDIAN)) {
            lf_hfs_generic_buf_lock(psBuf);
            lf_hfs_generic_buf_set_cache_flag(psBuf, uFlags & (GEN_BUF_IS_UPTODATE | GEN_BUF_LITTLE_ENDIAN));
            lf_hfs_generic_buf_unlock(psBuf);
        }

        #if GEN_BUF_ALLOC_DEBUG
            printf("Provided uncached %p\n", psBuf);
        #endif
        
        return psBuf;
    }
error:
    if (psBuf && psBuf->pvData) {
        hfs_free(psBuf->pvData);
    }
    if (psBuf) {
        hfs_free(psBuf);
    }
    return(NULL);
}

errno_t lf_hfs_generic_buf_read( GenericLFBufPtr psBuf )
{
    errno_t iErr                    = 0;
    uint64_t uActuallyRead          = 0;
    uint64_t uReadStartCluster      = 0;
    
    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_read: psBuf %p, psVnode %p, uBlockN %llu, uBlockSize %u, uFlags 0x%llx, uPhyCluster %llu: ",
           psBuf, psBuf->psVnode, psBuf->uBlockN, psBuf->uDataSize, psBuf->uCacheFlags, psBuf->uPhyCluster);
    #endif
    
    if (!psBuf) {
        return(EINVAL);
    }
    
    if ( buf_cache_state && !(psBuf->uCacheFlags & GEN_BUF_NON_CACHED))
    {
        lf_lck_mtx_lock(&buf_cache_mutex);
        lf_hfs_generic_buf_cache_update(psBuf);
        lf_lck_mtx_unlock(&buf_cache_mutex);
    }

    lf_hfs_generic_buf_lock(psBuf);
    
    assert(psBuf->uUseCnt != 0);
    assert(psBuf->sOwnerThread == pthread_self());
    
    if (psBuf->uCacheFlags & GEN_BUF_IS_UPTODATE) {
    
        // The buffer already contains data equals or newer than media.
        #if GEN_BUF_ALLOC_DEBUG
            printf("already up-to-date.\n");
        #endif
        goto exit;
    }
    
    // Cache is disabled or buffer wasn't found, read data from media
    iErr = raw_readwrite_read_mount(psBuf->psVnode,
                                    psBuf->uPhyCluster,
                                    HFSTOVCB(psBuf->psVnode->sFSParams.vnfs_mp->psHfsmount)->hfs_physical_block_size,
                                    psBuf->pvData,
                                    psBuf->uDataSize,
                                    &uActuallyRead,
                                    &uReadStartCluster);

    if ( iErr == 0 ) {
        psBuf->uValidBytes  = (uint32_t)uActuallyRead;
        lf_hfs_generic_buf_set_cache_flag(psBuf, GEN_BUF_IS_UPTODATE);

        #if GEN_BUF_ALLOC_DEBUG
            uint32_t *puData = psBuf->pvData;
            printf("Success. uPhyCluster %llu, Data: 0x%x, 0x%x, 0x%x, 0x%x\n", psBuf->uPhyCluster, puData[0], puData[2], puData[2], puData[3]);
        #endif

    } else {
        
        #if GEN_BUF_ALLOC_DEBUG
            printf("Error. uPhyCluster %llu, iErr %d.\n", psBuf->uPhyCluster, iErr);
        #endif
    }
exit:
    lf_hfs_generic_buf_unlock(psBuf);
    return iErr;
}

errno_t lf_hfs_generic_buf_write( GenericLFBufPtr psBuf ) {
    errno_t iErr = 0;

    lf_hfs_generic_buf_lock(psBuf);

    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_write: psBuf %p psVnode %p, uBlockN %llu, uDataSize %u, uFlags 0x%llx, uPhyCluster %llu, uUseCnt %u\n", psBuf, psBuf->psVnode, psBuf->uBlockN, psBuf->uDataSize, psBuf->uFlags, psBuf->uPhyCluster, psBuf->uUseCnt);
        uint32_t *puData = psBuf->pvData;
        printf("psBuf uPhyCluster %llu, Data: 0x%x, 0x%x, 0x%x, 0x%x\n", psBuf->uPhyCluster, puData[0], puData[2], puData[2], puData[3]);
    #endif

    assert(psBuf->uUseCnt != 0);
    assert(!(psBuf->uCacheFlags & GEN_BUF_WRITE_LOCK));
    assert(psBuf->sOwnerThread == pthread_self());
    
    iErr = raw_readwrite_write_mount(psBuf->psVnode,
                                     psBuf->uPhyCluster,
                                     HFSTOVCB(psBuf->psVnode->sFSParams.vnfs_mp->psHfsmount)->hfs_physical_block_size,
                                     psBuf->pvData,
                                     psBuf->uDataSize,
                                     NULL, NULL);

    lf_hfs_generic_buf_unlock(psBuf);
    return iErr;
}

void lf_hfs_generic_buf_clear( GenericLFBufPtr psBuf ) {
    memset(psBuf->pvData,0,sizeof(psBuf->uDataSize));
}

void lf_hfs_generic_buf_invalidate( GenericLFBuf *psBuf ) {
    struct buf_cache_entry *psCacheEntry;

    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_invalidate: psBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uFlags 0x%llx, uPhyCluster %llu, uUseCnt %u\n",
               psBuf, psBuf->psVnode, psBuf->uBlockN, psBuf->uDataSize, psBuf->uCacheFlags, psBuf->uPhyCluster, psBuf->uUseCnt);
    #endif

    lf_hfs_generic_buf_lock(psBuf);
    lf_hfs_generic_buf_rele(psBuf);
    
    assert(psBuf->uUseCnt == 0);
    assert(psBuf->sOwnerThread == NULL);

    // Check buffer cache, if a memory buffer already allocated for this physical block
    if ( buf_cache_state && !(psBuf->uCacheFlags & GEN_BUF_NON_CACHED)) {
        
        lf_lck_mtx_lock(&buf_cache_mutex);
        psCacheEntry = lf_hfs_generic_buf_cache_find_gen_buf(psBuf);

        if (psCacheEntry) {
            lf_hfs_generic_buf_cache_remove(psCacheEntry);
        } else {
            panic("A buffer is marked Cached, but was not found in Cache");
        }
        
        lf_lck_mtx_unlock(&buf_cache_mutex);

    } else {
        // This is a non-cached buffer
        gCacheStat.gen_buf_uncached--;
        lf_hfs_generic_buf_unlock(psBuf);
        lf_cond_destroy(&psBuf->sOwnerCond);
        lf_lck_mtx_destroy(&psBuf->sLock);
        hfs_free(psBuf->pvData);
        hfs_free(psBuf);
    }
}

void  lf_hfs_generic_buf_ref(GenericLFBuf *psBuf) {
    lf_hfs_generic_buf_lock(psBuf);
    assert(psBuf->sOwnerThread == pthread_self());
    psBuf->uUseCnt++;
    lf_hfs_generic_buf_unlock(psBuf);
}

int lf_hfs_generic_buf_validate_owner(GenericLFBuf *psBuf) {
    
    return(psBuf->sOwnerThread == pthread_self());
}

void  lf_hfs_generic_buf_rele(GenericLFBuf *psBuf) {
    lf_hfs_generic_buf_lock(psBuf);
    assert(psBuf->uUseCnt != 0);
    assert(psBuf->sOwnerThread == pthread_self());
    psBuf->uUseCnt--;
    if (psBuf->uUseCnt == 0) {
        psBuf->sOwnerThread = NULL;
        lf_cond_wakeup(&psBuf->sOwnerCond);
    }
    lf_hfs_generic_buf_unlock(psBuf);
}

void lf_hfs_generic_buf_lock(GenericLFBufPtr psBuf) {
    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_lock: psBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uFlags 0x%llx, uPhyCluster %llu, uUseCnt %u\n",
               psBuf, psBuf->psVnode, psBuf->uBlockN, psBuf->uDataSize, psBuf->uCacheFlags, psBuf->uPhyCluster, psBuf->uUseCnt);
    #endif

    if (psBuf->pLockingThread == pthread_self()) {
        psBuf->uLockCnt++;
    } else {
        lf_lck_mtx_lock(&psBuf->sLock);
        assert(psBuf->uLockCnt == 0);
        psBuf->uLockCnt = 1;
        psBuf->pLockingThread = pthread_self();
    }
}

void lf_hfs_generic_buf_unlock(GenericLFBufPtr psBuf) {
    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_unlock: psBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uFlags 0x%llx, uPhyCluster %llu, uUseCnt %u\n",
               psBuf, psBuf->psVnode, psBuf->uBlockN, psBuf->uDataSize, psBuf->uCacheFlags, psBuf->uPhyCluster, psBuf->uUseCnt);
    #endif
    
    assert(psBuf->pLockingThread == pthread_self());
    assert(psBuf->uLockCnt);
    
    psBuf->uLockCnt--;
    if (!psBuf->uLockCnt) {
        psBuf->pLockingThread = NULL;
        lf_lck_mtx_unlock(&psBuf->sLock);
    }
}

void lf_hfs_generic_buf_set_cache_flag(GenericLFBufPtr psBuf, uint64_t uCacheFlags) {
    lf_hfs_generic_buf_lock(psBuf);
    psBuf->uCacheFlags |= uCacheFlags;
    lf_hfs_generic_buf_unlock(psBuf);
}

void lf_hfs_generic_buf_clear_cache_flag(GenericLFBufPtr psBuf, uint64_t uCacheFlags) {
    lf_hfs_generic_buf_lock(psBuf);
    psBuf->uCacheFlags &= ~uCacheFlags;
    lf_hfs_generic_buf_unlock(psBuf);
}

static void lf_hfs_buf_free_unused()
{
    //We want to free more then we actually need, so that we won't have to come here every new buf that we allocate
    while ( gCacheStat.buf_cache_size > BUF_CACHE_MAX_ENTRIES_LOWER_LIMIT ||
           gCacheStat.buf_total_allocated_size > BUF_CACHE_MAX_DATA_LOWER_LIMIT)
    {
        struct buf_cache_entry *last;
        
        last = TAILQ_LAST(&buf_cache_list, buf_cache_head);
        
        if (!last) {
            break;
        }
        
        lf_hfs_generic_buf_lock(&last->sBuf);
        
        if ((last->sBuf.uUseCnt) || (last->sBuf.uCacheFlags & GEN_BUF_WRITE_LOCK)) {
            // Last buffer in buffer cache is in use.
            // Nothing more to free
            lf_hfs_generic_buf_unlock(&last->sBuf);
            break;
        }
        
        ++gCacheStat.buf_cache_cleanup;
        lf_hfs_generic_buf_cache_remove(last);
    }
}

void lf_hfs_generic_buf_release( GenericLFBufPtr psBuf )
{
    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_release: psBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uFlags 0x%llx, uPhyCluster %llu, uUseCnt %u\n",
               psBuf, psBuf->psVnode, psBuf->uBlockN, psBuf->uDataSize, psBuf->uCacheFlags, psBuf->uPhyCluster, psBuf->uUseCnt);
    #endif

    if (!psBuf) {
        return;
    }
    
    lf_hfs_generic_buf_rele(psBuf);

    // If Unused and UnCached, free.
    if ((psBuf->uCacheFlags & GEN_BUF_NON_CACHED) && (psBuf->uUseCnt == 0)) {
        // Buffer not in cache - free it
        gCacheStat.gen_buf_uncached--;
        lf_cond_destroy(&psBuf->sOwnerCond);
        lf_lck_mtx_destroy(&psBuf->sLock);
        hfs_free(psBuf->pvData);
        hfs_free(psBuf);
        return;
    }

    // Cleanup unused entries in the cache
    int iTry = lf_lck_mtx_try_lock(&buf_cache_mutex);
    if (iTry) {
        return;
    }

    //We want to free more then we actually need, so that we won't have to come here every new buf that we allocate
    lf_hfs_buf_free_unused();
    lf_lck_mtx_unlock(&buf_cache_mutex);
}

//  Buffer Cache functions

void lf_hfs_generic_buf_cache_init( void ) {
    gCacheStat.buf_cache_size       = 0;
    gCacheStat.max_gen_buf_uncached = 0;
    gCacheStat.gen_buf_uncached     = 0;
    lf_lck_mtx_init(&buf_cache_mutex);
    TAILQ_INIT(&buf_cache_list);
    buf_cache_state = true;
}

void lf_hfs_generic_buf_cache_deinit( void )
{
    lf_hfs_generic_buf_cache_remove_all(IGNORE_MOUNT_FD);

    assert(gCacheStat.buf_cache_size   == 0);
    assert(gCacheStat.gen_buf_uncached == 0);

    buf_cache_state = false;
    lf_lck_mtx_destroy(&buf_cache_mutex);
}

void lf_hfs_generic_buf_cache_clear_by_iFD( int iFD )
{
    lf_hfs_generic_buf_cache_remove_all(iFD);
}

boolean_t lf_hfs_generic_buf_match_range( struct buf_cache_entry *entry, GenericLFBufPtr psBuf )
{
    if ( VTOF(entry->sBuf.psVnode) != VTOF(psBuf->psVnode) )
    {
        return false;
    }

    uint64_t size_1     = entry->sBuf.uDataSize;
    uint64_t start_1    = entry->sBuf.uBlockN * size_1;
    uint64_t end_1      = start_1 + size_1 - 1;
    uint64_t size_2     = psBuf->uDataSize;
    uint64_t start_2    = psBuf->uBlockN * size_2;
    uint64_t end_2      = start_2 + size_2 - 1;

    enum rl_overlaptype overlap;
    struct rl_entry entry_range = {.rl_start = start_1, .rl_end = end_1};

    overlap = rl_overlap(&entry_range, start_2, end_2);

    switch (overlap)
    {
        case RL_MATCHINGOVERLAP:
            return true;
        case RL_OVERLAPCONTAINSRANGE:
            // Make sure we have same start though
            assert(start_1 == start_2);
            return true;
        case RL_NOOVERLAP:
        case RL_OVERLAPISCONTAINED:
            return false;
        case RL_OVERLAPSTARTSBEFORE:
        case RL_OVERLAPENDSAFTER:
            LFHFS_LOG(LEVEL_ERROR, " lf_hfs_generic_buf_match_range : cache overlap [%d]", overlap);
            assert(0);
    }
}

struct buf_cache_entry * lf_hfs_generic_buf_cache_find( GenericLFBufPtr psBuf )
{
    struct buf_cache_entry *entry, *entry_next;

    TAILQ_FOREACH_SAFE(entry, &buf_cache_list, buf_cache_link, entry_next)
    {
        if ( lf_hfs_generic_buf_match_range(entry, psBuf) )
        {
            break;
        }
    }

    return entry;
}

// Run the function pfCallback on all buffers that belongs to node psVnode.
int lf_hfs_generic_buf_write_iterate(vnode_t psVnode, IterateCallback pfCallback, uint32_t uFlags, void *pvArgs) {
   
    struct buf_cache_entry *psCacheEntry, *psNextCacheEntry;
    int iFD = VNODE_TO_IFD(psVnode);

    TAILQ_FOREACH_SAFE(psCacheEntry, &buf_cache_list, buf_cache_link, psNextCacheEntry) {
        int iEntryFD = VNODE_TO_IFD(psCacheEntry->sBuf.psVnode);
        
        if ( (iFD == iEntryFD) && (psCacheEntry->sBuf.psVnode == psVnode)) {
            if ((uFlags & BUF_SKIP_LOCKED) && (psCacheEntry->sBuf.uCacheFlags & GEN_BUF_WRITE_LOCK)) {
                continue;
            }
            if ((uFlags & BUF_SKIP_NONLOCKED) && !(psCacheEntry->sBuf.uCacheFlags & GEN_BUF_WRITE_LOCK)) {
                continue;
            }
            pfCallback(&psCacheEntry->sBuf, pvArgs);
        }
    }
    return(0);
}


struct buf_cache_entry *lf_hfs_generic_buf_cache_find_by_phy_cluster(int iFD, uint64_t uPhyCluster, uint64_t uBlockSize) {

    struct buf_cache_entry *psCacheEntry, *psNextCacheEntry;
    
    TAILQ_FOREACH_SAFE(psCacheEntry, &buf_cache_list, buf_cache_link, psNextCacheEntry) {
        if (psCacheEntry->sBuf.psVnode)
        {
            int iEntryFD = VNODE_TO_IFD(psCacheEntry->sBuf.psVnode);
            if ( (psCacheEntry->sBuf.uPhyCluster == uPhyCluster) &&
                 (iEntryFD                       == iFD        ) &&
                 (psCacheEntry->sBuf.uDataSize   >= uBlockSize )  ) {
                break;
            }
        }
        else
        {
            LFHFS_LOG(LEVEL_ERROR, "lf_hfs_generic_buf_cache_find_by_phy_cluster: got buf with vnode == NULL, cache_flags: 0x%llx, uUseCnt %d", psCacheEntry->sBuf.uCacheFlags, psCacheEntry->sBuf.uUseCnt);
            assert(0);
        }

    }
    return psCacheEntry;
}

struct buf_cache_entry *lf_hfs_generic_buf_cache_find_gen_buf(GenericLFBufPtr psBuf) {
    
    struct buf_cache_entry *psCacheEntry, *psNextCacheEntry;
    
    TAILQ_FOREACH_SAFE(psCacheEntry, &buf_cache_list, buf_cache_link, psNextCacheEntry) {
        if ( &psCacheEntry->sBuf == psBuf ) {
            break;
        }
    }
    return psCacheEntry;
}

GenericLFBufPtr lf_hfs_generic_buf_cache_add( GenericLFBufPtr psBuf )
{
    struct buf_cache_entry *entry;

    //Check if we have enough space to alloc this buffer, unless need to evict something
    if (gCacheStat.buf_total_allocated_size + psBuf->uDataSize > BUF_CACHE_MAX_DATA_UPPER_LIMIT ||
        gCacheStat.buf_cache_size + 1 == BUF_CACHE_MAX_ENTRIES_UPPER_LIMIT)
    {
        lf_hfs_buf_free_unused();
    }

    entry = hfs_mallocz(sizeof(*entry));
    if (!entry) {
        goto error;
    }

    memcpy(&entry->sBuf, (void*)psBuf, sizeof(*psBuf));
    entry->sBuf.uCacheFlags &= ~GEN_BUF_NON_CACHED;
    
    entry->sBuf.pvData = hfs_mallocz(psBuf->uDataSize);
    if (!entry->sBuf.pvData) {
        goto error;
    }

    TAILQ_INSERT_HEAD(&buf_cache_list, entry, buf_cache_link);

    gCacheStat.buf_cache_size++;
    gCacheStat.buf_total_allocated_size+=psBuf->uDataSize;
    
    if (gCacheStat.buf_cache_size > gCacheStat.max_buf_cache_size) {
        gCacheStat.max_buf_cache_size = gCacheStat.buf_cache_size;
    }

    return(&entry->sBuf);
    
error:
    if (entry) {
        if (entry->sBuf.pvData) {
            hfs_free(entry->sBuf.pvData);
        }
        hfs_free(entry);
    }
    return(NULL);
}

void lf_hfs_generic_buf_cache_update( GenericLFBufPtr psBuf )
{
    struct buf_cache_entry *entry;

    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_cache_update: psBuf %p\n", psBuf);
    #endif

    // Check that cache entry still exists and hasn't thrown away
    entry = lf_hfs_generic_buf_cache_find(psBuf);
    if (!entry) {
        return;
    }

    TAILQ_REMOVE(&buf_cache_list, entry, buf_cache_link);
    TAILQ_INSERT_HEAD(&buf_cache_list, entry, buf_cache_link);
}

void lf_hfs_generic_buf_cache_copy( struct buf_cache_entry *entry, __unused GenericLFBufPtr psBuf )
{
    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_cache_copy: psBuf %p\n", psBuf);
    #endif

    TAILQ_REMOVE(&buf_cache_list, entry, buf_cache_link);
    TAILQ_INSERT_HEAD(&buf_cache_list, entry, buf_cache_link);
}

void lf_hfs_generic_buf_cache_remove( struct buf_cache_entry *entry ) {
    
    if (entry->sBuf.uUseCnt != 0) {
        LFHFS_LOG(LEVEL_ERROR, "lf_hfs_generic_buf_cache_remove: remove buffer %p with uUseCnt %u", &entry->sBuf, entry->sBuf.uUseCnt);
    }

    #if GEN_BUF_ALLOC_DEBUG
        GenericLFBuf *psBuf = &entry->sBuf;
        printf("lf_hfs_generic_buf_cache_remove: psBuf %p, psVnode %p, uBlockN %llu, uDataSize %u, uFlags 0x%llx, uPhyCluster %llu, uUseCnt %u\n",
               psBuf, psBuf->psVnode, psBuf->uBlockN, psBuf->uDataSize, psBuf->uCacheFlags, psBuf->uPhyCluster, psBuf->uUseCnt);
    #endif
    
    TAILQ_REMOVE(&buf_cache_list, entry, buf_cache_link);
    --gCacheStat.buf_cache_size;
    ++gCacheStat.buf_cache_remove;
    gCacheStat.buf_total_allocated_size -= entry->sBuf.uDataSize;

    assert(entry->sBuf.uLockCnt == 1);
    
    lf_lck_mtx_unlock(&entry->sBuf.sLock);
    lf_cond_destroy(&entry->sBuf.sOwnerCond);
    lf_lck_mtx_destroy(&entry->sBuf.sLock);
    
    hfs_free(entry->sBuf.pvData);
    hfs_free(entry);
}

void lf_hfs_generic_buf_cache_remove_all( int iFD ) {
    struct buf_cache_entry *entry, *entry_next;

    lf_lck_mtx_lock(&buf_cache_mutex);

    TAILQ_FOREACH_SAFE(entry, &buf_cache_list, buf_cache_link, entry_next)
    {
        if ( (iFD == IGNORE_MOUNT_FD) || ( VNODE_TO_IFD(entry->sBuf.psVnode) == iFD ) )
        {
            if (iFD == IGNORE_MOUNT_FD) {
                // Media no longer available, force remove all
                TAILQ_REMOVE(&buf_cache_list, entry, buf_cache_link);
                --gCacheStat.buf_cache_size;
                ++gCacheStat.buf_cache_remove;
                gCacheStat.buf_total_allocated_size -= entry->sBuf.uDataSize;
            } else {
                lf_hfs_generic_buf_lock(&entry->sBuf);
                lf_hfs_generic_buf_cache_remove(entry);
            }
        }
    }

    lf_lck_mtx_unlock(&buf_cache_mutex);
}

/* buf_cache_mutex Should get locked from the caller using lf_hfs_generic_buf_cache_LockBufCache*/
void lf_hfs_generic_buf_cache_remove_vnode(vnode_t vp) {

    struct buf_cache_entry *entry, *entry_next;

    #if GEN_BUF_ALLOC_DEBUG
        printf("lf_hfs_generic_buf_cache_remove_vnode: vp %p: ", vp);
    #endif
    
    TAILQ_FOREACH_SAFE(entry, &buf_cache_list, buf_cache_link, entry_next) {
        
        if ( entry->sBuf.psVnode == vp ) {
            
            #if GEN_BUF_ALLOC_DEBUG
                printf("&sBuf %p, ", &entry->sBuf);
            #endif
            
            lf_hfs_generic_buf_lock(&entry->sBuf);
            lf_hfs_generic_buf_cache_remove(entry);
        }
    }

    #if GEN_BUF_ALLOC_DEBUG
        printf("Done.\n");
    #endif
}

void lf_hfs_generic_buf_cache_LockBufCache(void)
{
    lf_lck_mtx_lock(&buf_cache_mutex);
}

void lf_hfs_generic_buf_cache_UnLockBufCache(void)
{
    lf_lck_mtx_unlock(&buf_cache_mutex);
}
