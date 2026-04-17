 /* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  FAT_Access_M.c
 *  usbstorage_plugin
 */

#include "FAT_Access_M.h"
#include "DirOPS_Handler.h"
#include <unistd.h>
#include "Logger.h"
#include "RawFile_Access_M.h"
#include "ZeroFill.h"

//---------------------------------- Globals Decleration -----------------------------------------

#define FAT_ACCESS_LOCK(psFSRecord)      (pthread_mutex_lock(&psFSRecord->sFATCache.sFatMutex))
#define FAT_ACCESS_FREE(psFSRecord)      (pthread_mutex_unlock(&psFSRecord->sFATCache.sFatMutex))

#define INVALID_CACHE_OFFSET(psFSRecord) (psFSRecord->sFatInfo.uFatOffset + psFSRecord->sFatInfo.uFatSize)

#define LRU_COUNTER_SIZE    (63)
#define FAT_EOF(psFSRecord) (psFSRecord->sFatInfo.uFatMask & CLUST_EOFE)

#define FAT_DIRTY_BIT_CLUSTER   (1)
#define FAT_16_DIRTY_BIT_IDX    (15)
#define FAT_32_DIRTY_BIT_IDX    (27)

const uint64_t guMaxLRU = (1LLU << LRU_COUNTER_SIZE);

//---------------------------------- Functions Decleration ---------------------------------------

static int          FAT_Access_M_GetFATCluster( FileSystemRecord_s *psFSRecord, FatCacheEntry_s* psCacheEntry, uint32_t uBlockSize, uint64_t uBlockOffset );
static void         FAT_Access_M_UpdateLRU( uint8_t uFATEntryToUpdate,FileSystemRecord_s *psFSRecord );
static uint8_t      FAT_Access_M_GetLRUIndex(FileSystemRecord_s *psFSRecord);
static uint32_t     FAT_Access_M_FreeChainLength(FileSystemRecord_s *psFSRecord, uint32_t uStart, uint32_t uCount, int* piError);
static errno_t      FAT_Access_M_FATChainAlloc(FileSystemRecord_s *psFSRecord, uint32_t uStart, uint32_t uClusterCount, uint32_t uFillwith, uint32_t *uStartAllocated, uint32_t *uCountAllocated);
static int          FAT_Access_M_FlushCacheEntry(FileSystemRecord_s *psFSRecord, FatCacheEntry_s* psCacheEntry);
static void         FAT_Access_M_SetCacheEntryAsDirty(FileSystemRecord_s *psFSRecord, uint32_t uCluster);
static uint8_t      FAT_Access_M_GetCacheEntryAccordingToCluster(FileSystemRecord_s *psFSRecord, uint32_t uCluster);
static int          FAT_Access_M_GetFatEntry(FileSystemRecord_s *psFSRecord,uint32_t uClusterNum, uint8_t** ppuEntry);
static uint32_t     FAT_Access_M_FatBlockSize( FileSystemRecord_s *psFSRecord, uint64_t uBlockOffset );
static int          FAT_Access_M_SetClustersFatEntryContent( FileSystemRecord_s *psFSRecord, uint32_t uCluster, uint32_t uValue );

//---------------------------------- Functions Implementation ------------------------------------

static int
FAT_Access_M_FlushCacheEntry( FileSystemRecord_s *psFSRecord, FatCacheEntry_s* psCacheEntry )
{
    int iErr = 0;

    if ( !psCacheEntry->bIsDirty )
    {
        goto exit;
    }

    uint64_t uBlockOffset = psCacheEntry->uFatCacheEntryOffset;
    uint32_t uBlockSize   = FAT_Access_M_FatBlockSize(psFSRecord,uBlockOffset);
    
    if ( uBlockOffset >= INVALID_CACHE_OFFSET(psFSRecord) )
    {
        iErr = EFAULT;
        goto exit;
    }

    // Save new data to both of the fat tables.
    uint8_t uFatIdx;
    uint64_t uOffset;
    for ( uFatIdx=0, uOffset=uBlockOffset; uFatIdx < psFSRecord->sFatInfo.uNumOfFATs; uFatIdx++, uOffset+=psFSRecord->sFatInfo.uFatSize )
    {
        size_t uActualBytesWritten = pwrite( psFSRecord->iFD, psCacheEntry->pvFatEntryCache, uBlockSize, psFSRecord->sFatInfo.uFatOffset+uOffset );
        if ( uActualBytesWritten != uBlockSize )
        {
            iErr = errno;
            MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_FlushCacheEntry failed to write errno = [%d]\n", iErr);
            goto exit;
        }
    }
    
    // Only when finished writing to both copies
    // the cache is not dirty
    psCacheEntry->bIsDirty = false;
    
exit:
    return iErr;
}

static void FATMOD_CopyFATCacheToAlter(FileSystemRecord_s *psFSRecord)
{
    for ( uint8_t FATCacheCounter = 0; FATCacheCounter < FAT_CACHE_SIZE; FATCacheCounter++ )
    {
        psFSRecord->sFATCache.psAlterFATCacheEntries[FATCacheCounter].bIsDirty = psFSRecord->sFATCache.psFATCacheEntries[FATCacheCounter].bIsDirty;
        if (psFSRecord->sFATCache.psAlterFATCacheEntries[FATCacheCounter].bIsDirty) {
            memcpy(psFSRecord->sFATCache.psAlterFATCacheEntries[FATCacheCounter].pvFatEntryCache, psFSRecord->sFATCache.psFATCacheEntries[FATCacheCounter].pvFatEntryCache, FAT_BLOCK_SIZE);
            psFSRecord->sFATCache.psAlterFATCacheEntries[FATCacheCounter].uFatCacheEntryOffset = psFSRecord->sFATCache.psFATCacheEntries[FATCacheCounter].uFatCacheEntryOffset;
            
            psFSRecord->sFATCache.psFATCacheEntries[FATCacheCounter].bIsDirty = false;
        } else {
            psFSRecord->sFATCache.psAlterFATCacheEntries[FATCacheCounter].uFatCacheEntryOffset = INVALID_CACHE_OFFSET(psFSRecord);
        }
    }
}

int
FATMOD_FlushAllCacheEntries(FileSystemRecord_s *psFSRecord)
{
    int iErr = 0;

    pthread_mutex_lock(&psFSRecord->sFATCache.sAlterFatMutex);
    FAT_ACCESS_LOCK(psFSRecord);
    FATMOD_CopyFATCacheToAlter(psFSRecord);
    FAT_ACCESS_FREE(psFSRecord);
    
    // Flush all dirty entries that are in the cache
    for ( uint8_t FATCacheCounter = 0; FATCacheCounter < FAT_CACHE_SIZE; FATCacheCounter++ )
    {
        // Even if failed in writing one of the cache parts, continue try to flush all the rest in order to save as much as possible
        iErr |= FAT_Access_M_FlushCacheEntry( psFSRecord, &psFSRecord->sFATCache.psAlterFATCacheEntries[FATCacheCounter] );
    }

    pthread_mutex_unlock(&psFSRecord->sFATCache.sAlterFatMutex);
    return iErr;
}

static int
FAT_Access_M_GetFATCluster( FileSystemRecord_s *psFSRecord, FatCacheEntry_s* psCacheEntry, uint32_t uBlockSize, uint64_t uBlockOffset )
{
    int iErr = 0;

    iErr = FAT_Access_M_FlushCacheEntry( psFSRecord, psCacheEntry );
    if ( iErr != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_GetFATCluster failed to flush cache entry\n");
        goto exit;
    }
    
    if ( pread( psFSRecord->iFD, psCacheEntry->pvFatEntryCache, uBlockSize, psFSRecord->sFatInfo.uFatOffset+uBlockOffset) != uBlockSize )
    {
        iErr = errno;
        MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_GetFATCluster failed to read. errno = [%d]\n", iErr);
        psCacheEntry->uFatCacheEntryOffset = INVALID_CACHE_OFFSET(psFSRecord);
        goto exit;
    }

    psCacheEntry->uFatCacheEntryOffset = uBlockOffset;
    
exit:
    return iErr;
}

// Find first free cluster in FAT
int
FAT_Access_M_FindFirstFreeClusterFromGivenCluster( FileSystemRecord_s *psFSRecord, uint32_t uCluster )
{
    int iErr                        = 0;
    uCluster                        = MAX( uCluster, CLUST_FIRST );
    uint32_t uSartCluster           = uCluster;
    uint32_t uNextCluster           = 0;

    FAT_ACCESS_LOCK(psFSRecord);

    iErr = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, uCluster, &uNextCluster );
    if ( iErr != 0 )
    {
        goto exit;
    }

    // Look for contiguous clusters
    while (uNextCluster != 0 && (uCluster < psFSRecord->sFSInfo.uMaxCluster))
    {
        uCluster++;
        // We reached the end of the FAT,
        // need to return to the start of the FAT
        if (uCluster == psFSRecord->sFSInfo.uMaxCluster)
        {
            uCluster = CLUST_FIRST;
        }

        // We finished looking for free cluster in all the FAT
        // and found no free cluster
        if (uCluster == uSartCluster)
        {
            psFSRecord->sFSInfo.uFirstFreeCluster = 0;
            MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_FindFirstFreeClusterFromGivenCluster: Device is full! No free cluster to allocate");
            goto exit;
        }
        
         iErr = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, uCluster, &uNextCluster );
        if ( iErr != 0 )
        {
            goto exit;
        }
    }
    
    psFSRecord->sFSInfo.uFirstFreeCluster = uCluster;
//    MSDOS_LOG(LEVEL_DEBUG, "First free cluster: %d\n", psFSRecord->sFSInfo.uFirstFreeCluster);

exit:
    FAT_ACCESS_FREE(psFSRecord);
    return iErr;
}

uint32_t
FAT_Access_M_ContiguousClustersInChain(FileSystemRecord_s *psFSRecord, uint32_t uCluster, uint32_t* puNextCluster, errno_t* pError)
{
    uint32_t uResults = 1;

    if (!CLUSTER_IS_VALID(uCluster, psFSRecord))
    {
        MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_ContiguousClustersInChain got bad cluster [%u], MaxCluster [%u] .\n", uCluster, psFSRecord->sFSInfo.uMaxCluster);
        assert(0);
        *pError = EIO;
        return 0;
    }

    FAT_ACCESS_LOCK(psFSRecord);

    // Figure out which block of the FAT contains the entry for the given cluster.
    //
    // NOTE: FAT_BLOCK_SIZE is at least as large as the used portion of a FAT12 FAT.
    // So, for FAT12, we will end up reading the entire FAT at one time.
    *pError = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, uCluster, puNextCluster );
    if ( *pError != 0 )
    {
        uResults = 0;
        goto exit;
    }

    // Look for contiguous clusters
    while ((*puNextCluster) == uCluster+1)
    {
        uResults++;
        uCluster++;
        *pError = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, uCluster, puNextCluster );
        if ( *pError != 0 )
        {
            uResults = 0;
            goto exit;
        }
    }

exit:
    FAT_ACCESS_FREE(psFSRecord);

    return uResults;
}

static void
FAT_Access_M_UpdateLRU( uint8_t uFATEntryToUpdate, FileSystemRecord_s *psFSRecord )
{
    //In case the counter is 'close' to the max number possible, need to reset the counter
    VolumeFatCache_s* psFATCache = &psFSRecord->sFATCache;
    
    if ( ++psFATCache->uLRUCounter > guMaxLRU )
    {
        for ( uint8_t uFATCacheCounter = 0; uFATCacheCounter < FAT_CACHE_SIZE; uFATCacheCounter++ )
        {
            psFATCache->psFATCacheEntries[uFATCacheCounter].uLRUCounter = 0;
        }
        psFATCache->uLRUCounter = 1;
    }
    
    psFATCache->psFATCacheEntries[uFATEntryToUpdate].uLRUCounter = psFATCache->uLRUCounter;
}

static uint8_t
FAT_Access_M_GetLRUIndex( FileSystemRecord_s *psFSRecord )
{
    uint8_t  uLRUIndex      = 0;
    uint64_t uMinLRUCounter = guMaxLRU;

    for (uint8_t FATCacheCounter = 0; FATCacheCounter < FAT_CACHE_SIZE; FATCacheCounter++)
    {
        if ( psFSRecord->sFATCache.psFATCacheEntries[FATCacheCounter].uLRUCounter < uMinLRUCounter )
        {
            uLRUIndex = FATCacheCounter;
            uMinLRUCounter = psFSRecord->sFATCache.psFATCacheEntries[FATCacheCounter].uLRUCounter;
        }
    }
    
    return uLRUIndex;
}

//---------------------------------- SPI Implementation ------------------------------------

uint32_t
FAT_Access_M_Fat32EntryOffset(uint32_t uCluster)
{
    // FAT32 uses 32 bits per FAT entry, but the upper 4 bits are reserved
    return uCluster * sizeof(uint32_t);
}

uint32_t
FAT_Access_M_Fat32EntryGet(uint32_t uCluster, uint8_t *puEntry)
{
    uint32_t result = getuint32(puEntry);
    return result & 0x0FFFFFFFU;
}

uint32_t
FAT_Access_M_Fat32EntrySet(uint32_t uCluster, uint8_t *puEntry, uint32_t uValue)
{
    uint32_t uOldValue = getuint32(puEntry);
    putuint32(puEntry, (uValue & 0x0FFFFFFFU) | (uOldValue & 0xF0000000U));
    return uOldValue & 0x0FFFFFFFU;
}

uint32_t
FAT_Access_M_Fat16EntryOffset(uint32_t uCluster)
{
    return uCluster * sizeof(uint16_t);
}

uint32_t
FAT_Access_M_Fat16EntryGet(uint32_t uCluster, uint8_t *puEntry)
{
    uint32_t uResult = getuint16(puEntry);
    return uResult;
}

uint32_t
FAT_Access_M_Fat16EntrySet(uint32_t uCluster, uint8_t *puEntry, uint32_t uValue)
{
    uint32_t oldValue = getuint16(puEntry);
    putuint16(puEntry, uValue);
    return oldValue;
}

uint32_t
FAT_Access_M_Fat12EntryOffset(uint32_t uCluster)
{
    // FAT12 tightly packs 12 bits per cluster; 3 bytes for every 2 clusters.
    // I find it easiest to think in terms of a pair of FAT entries (cluster
    // numbers 2*N and 2*N+1) occupying 3 bytes (at offset 3*N).
    return (uCluster * 3) / 2;
}

uint32_t
FAT_Access_M_Fat12EntryGet(uint32_t uCluster, uint8_t *puEntry)
{
    // entry may not be 2-byte aligned, so fetch the value a byte at a time
    uint32_t uResult = (puEntry)[0] | ((puEntry)[1] << 8);
    
    // Extract the upper or lower 12 bits.  If the cluster number is even, we
    // want the low 12 bits.  If the cluster number is odd, we want the high
    // 12 bits.
    
    if (uCluster & 1)
    {
        uResult >>= 4;
    }
    else
    {
        uResult &= 0x0FFF;
    }
    
    return uResult;
}

uint32_t
FAT_Access_M_Fat12EntrySet(uint32_t uCluster, uint8_t *puEntry, uint32_t uValue)
{
    uint32_t uOldValue = (puEntry)[0] | ((puEntry)[1] << 8);
    uint32_t uNewValue;
    
    // Replace the upper or lower 12 bits.  If the cluster number is even, we
    // change the low 12 bits.  If the cluster number is odd, we change the high
    // 12 bits.
    
    if (uCluster & 1)
    {
        uNewValue = (uOldValue & 0x000F) | (uValue << 4);
        (puEntry)[0] = uNewValue;
        (puEntry)[1] = uNewValue >> 8;
        uOldValue >>= 4;
    }
    else
    {
        uNewValue = (uOldValue & 0xF000) | (uValue & 0x0FFF);
        (puEntry)[0] = uNewValue;
        (puEntry)[1] = uNewValue >> 8;
        uOldValue &= 0x0FFF;
    }
    
    return uOldValue;
}

// Pin the size of a FAT cache block to the end of the FAT
int
FAT_Access_M_GetClustersFatEntryContent( FileSystemRecord_s *psFSRecord, uint32_t uCluster, uint32_t* puNextCluser )
{
    uint32_t uNextCluster = 0;
    uint8_t* puEntry      = NULL;
    int iErr              = 0;

    FAT_ACCESS_LOCK(psFSRecord);

    iErr = FAT_Access_M_GetFatEntry( psFSRecord, uCluster, &puEntry );
    if ( iErr != 0 )
    {
        goto exit;
    }
    
    uNextCluster = psFSRecord->sFSOperations.uFatEntryGet( uCluster, puEntry );
    
exit:
    FAT_ACCESS_FREE(psFSRecord);
    *puNextCluser = uNextCluster;

    return iErr;
}

static int
FAT_Access_M_SetClustersFatEntryContent( FileSystemRecord_s *psFSRecord, uint32_t uCluster, uint32_t uValue )
{
    int iErr            = 0;
    uint8_t* puEntry    = NULL;

    iErr = FAT_Access_M_GetFatEntry( psFSRecord, uCluster, &puEntry );
    if ( iErr != 0 )
    {
        goto exit;
    }

    psFSRecord->sFSOperations.uFatEntrySet( uCluster, puEntry, uValue & psFSRecord->sFatInfo.uFatMask );
    //Set cache entry as dirty
    FAT_Access_M_SetCacheEntryAsDirty( psFSRecord, uCluster );

exit:
    return iErr;
}

// Pin the size of a FAT cache block to the end of the FAT
static uint32_t
FAT_Access_M_FatBlockSize( FileSystemRecord_s *psFSRecord, uint64_t uBlockOffset )
{
    uint32_t uBlockSize = FAT_BLOCK_SIZE;
    
    if (uBlockOffset + uBlockSize > psFSRecord->sFatInfo.uFatSize)
        uBlockSize = (uint32_t)(psFSRecord->sFatInfo.uFatSize - uBlockOffset);
    
    return uBlockSize;
}

int
FAT_Access_M_ChainLength(FileSystemRecord_s *psFSRecord, uint32_t uCluster, uint32_t* puLength, NodeRecord_s *psNodeRecord, uint32_t* puLastCluster)
{
    uint32_t uNumClusters   = 0;
    uint32_t uNextCluster   = 0;
    uint32_t uExtentLength  = 0;
    int iErr                = 0;

    FAT_ACCESS_LOCK(psFSRecord);

    // In case of FAT12/16 Root Dir can't be extended
    if (    ( psNodeRecord != NULL ) &&
            ( IS_FAT_12_16_ROOT_DIR(psNodeRecord) ) )
    {
        *puLength = 1;
        goto exit;
    }
    
    while (CLUSTER_IS_VALID(uCluster, psFSRecord))
    {
        uExtentLength = FAT_Access_M_ContiguousClustersInChain(psFSRecord, uCluster, &uNextCluster, &iErr);
        if ( iErr != 0 )
        {
            // There was an error reading from the FAT
            MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_ChainLength failed to read from fat iErr = %d\n", iErr);
            goto exit;
        }
        uNumClusters += uExtentLength;
        if (uNumClusters >= psFSRecord->sFSInfo.uMaxCluster)
        {
            MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_ChainLength found circle cycle.\n");
            iErr = EIO;
            goto exit;
        }
        
        *puLastCluster = uCluster + uExtentLength - 1;
        uCluster = uNextCluster;
    }
    
    if (uCluster < (CLUST_EOFS & psFSRecord->sFatInfo.uFatMask))
    {
        // There was a bad cluster number in the chain.
        MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_ChainLength found bad cluster in the chain.\n");
        iErr = EIO;
        goto exit;
    }
    
    *puLength = uNumClusters;

exit:
    FAT_ACCESS_FREE(psFSRecord);
    return iErr;
}

static uint8_t
FAT_Access_M_GetCacheEntryAccordingToCluster( FileSystemRecord_s *psFSRecord, uint32_t uCluster )
{
    uint32_t uFoundCluster  = FAT_CACHE_SIZE;
    uint64_t uEntryOffset   = psFSRecord->sFSOperations.uFatEntryOffset( uCluster );
    uint64_t uBlockOffset   = uEntryOffset & ~FAT_BLOCK_MASK; // round down to start of block

    VolumeFatCache_s* psFATCache = &psFSRecord->sFATCache;
    
    // Go over all FAT cache entries
    for ( uint8_t uFATCacheCounter = 0; uFATCacheCounter < FAT_CACHE_SIZE; uFATCacheCounter++ )
    {
        if ( uBlockOffset == psFATCache->psFATCacheEntries[uFATCacheCounter].uFatCacheEntryOffset )
        {
            uFoundCluster = uFATCacheCounter;
            break;
        }
    }
    
    return uFoundCluster;
}

static void
FAT_Access_M_SetCacheEntryAsDirty( FileSystemRecord_s *psFSRecord, uint32_t uCluster )
{
    uint32_t uCacheEntry = FAT_Access_M_GetCacheEntryAccordingToCluster( psFSRecord, uCluster );
    if ( uCacheEntry < FAT_CACHE_SIZE )
    {
        psFSRecord->sFATCache.psFATCacheEntries[uCacheEntry].bIsDirty = true;
    }
}

/*
 * Check the length of a free cluster chain starting at start.
 *
 * uStart - start of chain
 * uCount - number of clusters
 */
static uint32_t
FAT_Access_M_FreeChainLength(FileSystemRecord_s *psFSRecord, uint32_t uStart, uint32_t uCount, int* piError)
{
    uint32_t uCluster       = uStart;
    uint32_t uFreeCount     = 0;
    uint32_t uNextCluster   = 0;
    
    *piError = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, uCluster, &uNextCluster );
    if ( *piError != 0 )
    {
        uFreeCount = 0;
        goto exit;
    }

    while ( (uNextCluster == 0) && (uCluster <= psFSRecord->sFSInfo.uMaxCluster) && (uCount > uFreeCount) )
    {
        uFreeCount++;
        uCluster++;
         *piError = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, uCluster, &uNextCluster );
        if ( *piError != 0 )
        {
            uFreeCount = 0;
            goto exit;
        }
    }

exit:
    return uFreeCount;
}


/*
 * Allocate a contiguous cluster chain
 *
 * uStart    - first cluster of chain
 * uClusterCount    - number of clusters in chain
 * fillwith - what to write into fat entry of last cluster
 * uStartAllocated - first allocated cluster
 * uCountAllocated - number of clusters allocated
 */
static int
FAT_Access_M_FATChainAlloc( FileSystemRecord_s *psFSRecord, uint32_t uStart, uint32_t uClusterCount, uint32_t uFillwith, uint32_t *uStartAllocated, uint32_t *uCountAllocated )
{
    int iErr                    = 0;
    uint32_t uCount             = uClusterCount;
    uint32_t uCurrentCluster    = uStart;
    uint32_t uNextCluster       = 0;

    /*
     * Be sure the clusters are in the filesystem.
     */
    if ( (uStart < CLUST_FIRST) || ((uStart + uCount - 1) > psFSRecord->sFSInfo.uMaxCluster) )
    {
        MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_FATChainAlloc: start=%u, count=%u, fill=%u\n", uStart, uCount, uFillwith);
        iErr = EFAULT;
        goto exit;
    }

    /* Loop over all clusters in the chain */
    while (uCount > 0)
    {
	
        uNextCluster = (--uCount > 0)? (uCurrentCluster + 1) : uFillwith;
        iErr = FAT_Access_M_SetClustersFatEntryContent(psFSRecord, uCurrentCluster, uNextCluster);

        if ( iErr != 0 )
        {
            goto exit;
        }
        
        uCurrentCluster++;
    }

    // First allocated cluster
    if (uStartAllocated) 
    {
        *uStartAllocated = uStart;
    }
    // Number of allocated clusters
    if (uCountAllocated)
    {
        *uCountAllocated = uClusterCount;
    }

exit:
    return iErr;
}

static int
FAT_Access_M_GetFatEntry( FileSystemRecord_s *psFSRecord,uint32_t uClusterNum, uint8_t** puEntry )
{
    int iErr = 0;
    // Check if Current FAT cache contains the wanted cluster
    uint64_t uEntryOffset           = psFSRecord->sFSOperations.uFatEntryOffset( uClusterNum );
    uint64_t uBlockOffset           = uEntryOffset & ~FAT_BLOCK_MASK;          // round down to start of block
    uint32_t uBlockSize             = FAT_Access_M_FatBlockSize( psFSRecord, uBlockOffset );
    VolumeFatCache_s* psFATCache    = &psFSRecord->sFATCache;
    
    // Look for the FAT cache entry
    uint8_t uFATCacheCounter = FAT_Access_M_GetCacheEntryAccordingToCluster( psFSRecord, uClusterNum );

    // If we found the entry in the cache.
    if ( uFATCacheCounter < FAT_CACHE_SIZE )
    {
        *puEntry = ((uint8_t *)psFATCache->psFATCacheEntries[uFATCacheCounter].pvFatEntryCache) + (uEntryOffset - uBlockOffset);
        FAT_Access_M_UpdateLRU( uFATCacheCounter, psFSRecord );
        goto exit;
    }

    // In case non of the cache entries is the one needed -> find the LRU one and replace
    uFATCacheCounter = FAT_Access_M_GetLRUIndex( psFSRecord );

    iErr = FAT_Access_M_GetFATCluster( psFSRecord, &psFATCache->psFATCacheEntries[uFATCacheCounter], uBlockSize, uBlockOffset );
    if (iErr)
    {
        MSDOS_LOG(LEVEL_ERROR, "FAT_Access_M_GetFatEntry: fail to get fat cluster\n");
        *puEntry = NULL;
        goto exit;
    }

    *puEntry = (uint8_t *) psFATCache->psFATCacheEntries[uFATCacheCounter].pvFatEntryCache + (uEntryOffset - uBlockOffset);
    FAT_Access_M_UpdateLRU( uFATCacheCounter, psFSRecord );

exit:
    return iErr;
}

int
FAT_Access_M_FATInit(FileSystemRecord_s *psFSRecord)
{
    int iErr = 0;

    VolumeFatCache_s* psFATCache = &psFSRecord->sFATCache;
    memset( psFATCache, 0, sizeof(VolumeFatCache_s) );

    psFATCache->uAmountOfAllocatedCacheEntries = 0;

    // Init fat mutex.
    pthread_mutexattr_t sAttr;
    pthread_mutexattr_init( &sAttr );
    // Set attr to mutex_recursive.. as the lock can be call on the same thread recursively.
    pthread_mutexattr_settype( &sAttr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &psFATCache->sFatMutex, &sAttr );
    pthread_mutex_init( &psFATCache->sAlterFatMutex, &sAttr );
    
    pthread_mutexattr_destroy( &sAttr );
    
    for ( uint8_t uIdx = 0; uIdx < FAT_CACHE_SIZE; uIdx++ )
    {
        FatCacheEntry_s* psCacheEntry = &psFATCache->psFATCacheEntries[uIdx];

        psCacheEntry->pvFatEntryCache       = NULL;
        psCacheEntry->uLRUCounter           = 0;
        psCacheEntry->uFatCacheEntryOffset  = INVALID_CACHE_OFFSET(psFSRecord);
        psCacheEntry->bIsDirty              = false;
        psCacheEntry->pvFatEntryCache       = malloc(FAT_BLOCK_SIZE);
        if ( psCacheEntry->pvFatEntryCache == NULL )
        {
            iErr = ENOMEM;
            goto exit;
        }
        
        psCacheEntry = &psFATCache->psAlterFATCacheEntries[uIdx];

        psCacheEntry->pvFatEntryCache       = NULL;
        psCacheEntry->uLRUCounter           = 0;
        psCacheEntry->uFatCacheEntryOffset  = INVALID_CACHE_OFFSET(psFSRecord);
        psCacheEntry->bIsDirty              = false;
        psCacheEntry->pvFatEntryCache       = malloc(FAT_BLOCK_SIZE);
        if ( psCacheEntry->pvFatEntryCache == NULL )
        {
            iErr = ENOMEM;
            goto exit;
        }
    }

    // Locate first free cluster
    iErr = FAT_Access_M_FindFirstFreeClusterFromGivenCluster( psFSRecord, CLUST_FIRST );
    if ( iErr != 0 )
    {
        goto exit;
    }

exit:
    return iErr;
}

void
FAT_Access_M_FATFini(FileSystemRecord_s *psFSRecord)
{
    VolumeFatCache_s* psFATCache = &psFSRecord->sFATCache;
    psFATCache->uAmountOfAllocatedCacheEntries = 0;

    // Free FAT Cache
    for ( uint8_t uIdx = 0; uIdx < FAT_CACHE_SIZE; uIdx++ )
    {
        FatCacheEntry_s* psCacheEntry = &psFATCache->psFATCacheEntries[uIdx];
        if (psCacheEntry->pvFatEntryCache) free(psCacheEntry->pvFatEntryCache);
        psCacheEntry->pvFatEntryCache       = NULL;
        psCacheEntry->uLRUCounter           = 0;
        psCacheEntry->uFatCacheEntryOffset  = INVALID_CACHE_OFFSET(psFSRecord);
        
        psCacheEntry = &psFATCache->psAlterFATCacheEntries[uIdx];
        if (psCacheEntry->pvFatEntryCache) free(psCacheEntry->pvFatEntryCache);
        psCacheEntry->pvFatEntryCache       = NULL;
        psCacheEntry->uLRUCounter           = 0;
        psCacheEntry->uFatCacheEntryOffset  = INVALID_CACHE_OFFSET(psFSRecord);
    }
    
    //Destroy FAT mutex
    pthread_mutex_destroy( &psFATCache->sFatMutex );
    pthread_mutex_destroy( &psFATCache->sAlterFatMutex );
}

/*
 * Allocate contiguous free clusters.
 * 
 * uStart                 - preferred start of cluster chain.
 * uCount                 - number of clusters requested.
 * uFillWith              - put this value into the fat entry for the
 *		                    last allocated cluster.
 * puRetCluster           - put the first allocated cluster's number here.
 * puNumAllocatedClusters - how many clusters were actually allocated.
 * bMustBeConting         - the allocated range should be contiguous (if
 *                          we can extend the current EOF, we will,
 *                          and the rest will be allocated contiguous)
 */
static errno_t
FAT_Access_M_ContClusterAllocate(FileSystemRecord_s *psFSRecord, uint32_t uStart, uint32_t uCount, uint32_t uFillWith, uint32_t* puFirstAllocatedCluster, uint32_t* puNumAllocatedClusters,
                                 bool bMustBeConting)
{
    errno_t error = 0;
    uint32_t uClusterNum;	/* A cluster number (loop index) */
    uint32_t uFreeClusterLen = 0;		/* The number of contiguous free clusters at "cn" */
    uint32_t uLargestContiguousLenFound;	/* The largest contiguous run of free clusters found so far */
    uint32_t uFirstClusterForLargestContiguous;	/* The starting cluster of largest run found */
    uint32_t uClustersAllocated =0;
    uint32_t uFirstAllocated =0;
    uint32_t uNewEOF = 0;
    
    *puNumAllocatedClusters = 0;
    *puFirstAllocatedCluster = 0;
    
    MSDOS_LOG(LEVEL_DEBUG, "FAT_Access_M_ContClusterAllocate: requested uStart %d uCount %d, bMustBeConting %d\n", uStart, uCount, bMustBeConting);
    
    if (uStart) 
    {
        /* Try to allocate from the EOF */
        uint32_t uLen = FAT_Access_M_FreeChainLength(psFSRecord, uStart + 1, uCount, &error);
        if (error)
            goto done;
        
        if (uLen != 0 ) {
            /* Use the min between what we found and what we need. */
            uLen = MIN(uLen, uCount);
            error = FAT_Access_M_FATChainAlloc(psFSRecord, uStart + 1, uLen, uFillWith, &uFirstAllocated, &uClustersAllocated);
            if (error)
                goto done;
            
            *puFirstAllocatedCluster = uFirstAllocated;
            *puNumAllocatedClusters += uClustersAllocated;
            uNewEOF = uFirstAllocated + uClustersAllocated - 1;
            /* Update the allocated space */
            uCount -= uLen;
            
            /* if we done allocating, go out */
            if (uCount == 0)
                goto done;
        }
    }
    else 
    {
        /* No specific starting point requested, so use the first free cluster */
        uStart = psFSRecord->sFSInfo.uFirstFreeCluster; 
    }

    uLargestContiguousLenFound = 0;
    uFirstClusterForLargestContiguous = 0;
    
    /* Scan through the FAT for contiguous free clusters. */
    for (uClusterNum = uStart; uClusterNum <= psFSRecord->sFSInfo.uMaxCluster; uClusterNum += uFreeClusterLen+1)
    {
        uFreeClusterLen = FAT_Access_M_FreeChainLength(psFSRecord, uClusterNum, uCount, &error);
        if (error)
            goto done;
        
        if (uFreeClusterLen == 0) continue;
        
        /*
         * If we found enough clusters, or if we don't have to allocate the whole range contiguously
         * we can stop the allocation here
         */
        if (uFreeClusterLen >= uCount || !bMustBeConting)
        {
            /* Use the min between what we found and what we need. */
            uFreeClusterLen = MIN(uCount, uFreeClusterLen);
            
            /* Set next free cluster for next time */
            error = FAT_Access_M_FindFirstFreeClusterFromGivenCluster(psFSRecord, uClusterNum + uFreeClusterLen);
            if (error)
            {
                goto done;
            }
            
            error = FAT_Access_M_FATChainAlloc(psFSRecord, uClusterNum, uFreeClusterLen, uFillWith, &uFirstAllocated, &uClustersAllocated);
            if (!error) {
                *puNumAllocatedClusters += uClustersAllocated;

                if (*puFirstAllocatedCluster == 0)
                    *puFirstAllocatedCluster = uFirstAllocated;
                /* Check if we need to update the new EOF */
                if (uNewEOF != 0) {
                    FAT_Access_M_SetClustersFatEntryContent( psFSRecord, uNewEOF, uFirstAllocated);
                }
            }
            
            goto done;
        }
        
        /* Keep track of longest free extent found */
        if (uFreeClusterLen > uLargestContiguousLenFound)
        {
            uFirstClusterForLargestContiguous = uClusterNum;
            uLargestContiguousLenFound = uFreeClusterLen;
        }
    }

    /* We ended the loop, Let's start over */
    for (uClusterNum = CLUST_FIRST; uClusterNum < uStart; uClusterNum += uFreeClusterLen+1)
    {
        uFreeClusterLen = FAT_Access_M_FreeChainLength(psFSRecord, uClusterNum, uCount, &error);
        if (error)
            goto done;
        
        if (uFreeClusterLen == 0) continue;
        
        if (uFreeClusterLen >= uCount || !bMustBeConting)
        {
            /* Use the min between what we found and what we need. */
            uFreeClusterLen = MIN(uCount, uFreeClusterLen);
            
            /* Set next free cluster for next time */
            error = FAT_Access_M_FindFirstFreeClusterFromGivenCluster(psFSRecord,uClusterNum + uFreeClusterLen);
            if (error)
            {
                goto done;
            }
            
            error = FAT_Access_M_FATChainAlloc(psFSRecord, uClusterNum, uFreeClusterLen, uFillWith, &uFirstAllocated, &uClustersAllocated);
            if (!error) {
                if (*puFirstAllocatedCluster == 0)
                    *puFirstAllocatedCluster = uFirstAllocated;
                *puNumAllocatedClusters += uClustersAllocated;

                /* Check if we need to update the new EOF */
                if (uNewEOF != 0) {
                    FAT_Access_M_SetClustersFatEntryContent( psFSRecord, uNewEOF, uFirstAllocated);
                }
            }
            
            goto done;
        }
        
        /* Keep track of longest free extent found */
        if (uFreeClusterLen > uLargestContiguousLenFound)
        {
            uFirstClusterForLargestContiguous = uClusterNum;
            uLargestContiguousLenFound = uFreeClusterLen;
        }
    }

    /*
     * If we get here, there was no single contiguous chain as long as we
     * wanted.  Check to see if we found *any* free chains.
     */
    if (!uLargestContiguousLenFound)
    {
        error = ENOSPC;
        goto done;
    }

    /*
     * Just allocate the largest free chain we found.
     */
    if (uLargestContiguousLenFound)
    {
        psFSRecord->sFSInfo.uFirstFreeCluster = uFirstClusterForLargestContiguous + uLargestContiguousLenFound;
        error = FAT_Access_M_FATChainAlloc(psFSRecord, uFirstClusterForLargestContiguous, uLargestContiguousLenFound, uFillWith, &uFirstAllocated, &uClustersAllocated);
        if (!error) {
            if (*puFirstAllocatedCluster == 0)
                *puFirstAllocatedCluster = uFirstAllocated;
            *puNumAllocatedClusters += uClustersAllocated;
            
            /* Check if we need to update the new EOF */
            if (uNewEOF != 0) {
                FAT_Access_M_SetClustersFatEntryContent( psFSRecord, uNewEOF, uFirstAllocated);
            }
        }
    }

done:
    if ( error == 0 )
    {
        // Update number of free clusters
        psFSRecord->sFSInfo.uFreeClusters -= *puNumAllocatedClusters;
    }
    return error;
}

/*
* Free cluster chain starting at given cluster 
*
* bIsPartialRelease : when not the whole file is beeing released, uStart will mark as EOF.
*/
int
FAT_Access_M_FATChainFree( FileSystemRecord_s *psFSRecord, uint32_t uStart, bool bIsPartialRelease )
{
    int iErr                = 0;
    uint32_t uCluster       = uStart;
    uint32_t uNextCluster   = 0;
    uint32_t uCount         = 0;
    uint32_t uReleaseCount  = 0;
    
    MSDOS_LOG(LEVEL_DEBUG, "FAT_Access_M_FATChainFree uStart = %u, bIsPartialRelease = %d\n", uStart, bIsPartialRelease);

    if ( uStart < CLUST_FIRST )
    {
        MSDOS_LOG( LEVEL_ERROR, "FAT_Access_M_FATChainFree Get bad cluster  = %u", uStart );
        iErr = EINVAL;
        // Bug! assert for now..
        assert( uStart >= CLUST_FIRST );
        return iErr;
    }

    FAT_ACCESS_LOCK(psFSRecord);

    do {
        iErr = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, uCluster, &uNextCluster );
        if ( iErr != 0 )
        {
            goto exit;
        }

        //In case trying to free a freed cluster
        if ( uNextCluster == CLUST_FREE )
            break;

        // In case of partial release, mark the first cluster as EOF.
        if ( ( uCount == 0 ) && bIsPartialRelease )
        {
            FAT_Access_M_SetClustersFatEntryContent(psFSRecord, uCluster, FAT_EOF(psFSRecord));
//            MSDOS_LOG(LEVEL_DEBUG, "Marking %u as EOF\n", uCluster);
        }
        else
        {
            FAT_Access_M_SetClustersFatEntryContent(psFSRecord, uCluster, 0);
//            MSDOS_LOG(LEVEL_DEBUG, "Marking %u as deleted\n", uCluster);
            ++uReleaseCount;
        }
        uCluster = uNextCluster;
        ++uCount;
        
    } while ( uNextCluster <= psFSRecord->sFSInfo.uMaxCluster );
    
    MSDOS_LOG(LEVEL_DEBUG, "FAT_Access_M_FATChainFree: %d clusters freed\n", uReleaseCount);
    
    // Update number of free clusters
    psFSRecord->sFSInfo.uFreeClusters += uReleaseCount;

exit:
    FAT_ACCESS_FREE(psFSRecord);
    return iErr;
}

/*
     Allocate X number of clusters and fill them with zeros.
     return 0 if succeed, else return the appropiate error#.
 */
int
FAT_Access_M_AllocateClusters( FileSystemRecord_s *psFSRecord, uint32_t uClustToAlloc, uint32_t uLastKnownCluster, uint32_t* puFirstAllocatedCluster, uint32_t* puLastAllocatedCluster, uint32_t* puAmountOfAllocatedClusters, bool bPartialAllocationAllowed, bool bFillWithZeros, NewAllocatedClusterInfo_s** ppsNewAllocatedClusterInfoToReturn , bool bMustBeContiguousAllocation)
{
    int iErr                                = 0;
    uint32_t uNumOfNewAllocatedClusters     = 0;
    uint32_t uSucNum                        = 0;
    uint32_t uFirstNewCluster               = 0;
    uint32_t uLastAllocatedCluster          = 0;
    uint32_t uCounter                       = 0;
    uint32_t uStartCluster                  = uLastKnownCluster;

    FAT_ACCESS_LOCK(psFSRecord);
    NewAllocatedClusterInfo_s* psNewAllocatedClusterInfo = NULL;
    bool bShouldStopAllocation = false;
    
	do
    {
        iErr = FAT_Access_M_ContClusterAllocate( psFSRecord, uStartCluster, uClustToAlloc, FAT_EOF(psFSRecord), &uFirstNewCluster, &uSucNum, bMustBeContiguousAllocation);
        
        if ( iErr != 0 )
        {
            if ( uCounter > 0 )
            {
                if (iErr == ENOSPC && bPartialAllocationAllowed)
                {
                    *puLastAllocatedCluster = uLastAllocatedCluster;
                    *puAmountOfAllocatedClusters = uNumOfNewAllocatedClusters;
                }
                else
                {
                    goto fail;
                }
            }
            goto exit;
        }
        
        // if we have to contiguous allocate the clusters,
        // but we didn't allocate all of the clusters at once
        // we need to fail
        // if bPartialAllocationAllowed - ENOSPC, else leave it as is
        // and return what me mannaged to allocated contiguously
        if (bMustBeContiguousAllocation && uSucNum < uClustToAlloc) {
            if (!bPartialAllocationAllowed) {
                iErr = ENOSPC;
                
                // Need to free what we already allocated
                FAT_Access_M_FATChainFree( psFSRecord, *puFirstAllocatedCluster, false );
                if ( uLastKnownCluster != 0 )
                {
                    FAT_Access_M_SetClustersFatEntryContent( psFSRecord, uLastKnownCluster, FAT_EOF(psFSRecord));
                }
                goto exit;
            } else {
                bShouldStopAllocation = true;
            }
        }
        
        if ( uCounter == 0 )
        {
            *puFirstAllocatedCluster = uFirstNewCluster;
        }
        
        if ( uStartCluster != 0 )
        {
            FAT_Access_M_SetClustersFatEntryContent( psFSRecord, uStartCluster, uFirstNewCluster);
        }

        if (ppsNewAllocatedClusterInfoToReturn != NULL)
        {
            if (psNewAllocatedClusterInfo == NULL)
            {
                psNewAllocatedClusterInfo = (NewAllocatedClusterInfo_s*) malloc(sizeof(NewAllocatedClusterInfo_s));
                if (psNewAllocatedClusterInfo == NULL)
                {
                    iErr = ENOMEM;
                    goto fail;
                }
                
                *ppsNewAllocatedClusterInfoToReturn = psNewAllocatedClusterInfo;
            } else {
                psNewAllocatedClusterInfo->psNext = (NewAllocatedClusterInfo_s*) malloc(sizeof(NewAllocatedClusterInfo_s));
                if (psNewAllocatedClusterInfo->psNext == NULL)
                {
                    iErr = ENOMEM;
                    goto fail;
                }
                psNewAllocatedClusterInfo = psNewAllocatedClusterInfo->psNext;
            }
            
            psNewAllocatedClusterInfo->uNewAlloctedStartCluster = uFirstNewCluster;
            psNewAllocatedClusterInfo->uAmountOfConsecutiveClusters = uSucNum;
            psNewAllocatedClusterInfo->psNext = NULL;
        }

        uClustToAlloc               -= uSucNum;
        uNumOfNewAllocatedClusters  += uSucNum;
        uLastAllocatedCluster       = uFirstNewCluster + uSucNum - 1;

        if ( bFillWithZeros )
        {
            uint32_t uNumOfClustersToZeroFill = uSucNum;
            uint32_t uStartClusterToZeroFill = uFirstNewCluster;
            uint32_t NextCluster;
            while( uNumOfClustersToZeroFill > 0 && CLUSTER_IS_VALID(uStartClusterToZeroFill, psFSRecord))
            {
                uint32_t uChainLen = FAT_Access_M_ContiguousClustersInChain(psFSRecord, uStartClusterToZeroFill, &NextCluster, &iErr);
                if ( iErr != 0 )
                {
                    MSDOS_LOG( LEVEL_ERROR, "FAT_Access_M_AllocateClusters: Failed to write zero buffer into the device\n");
                    FAT_Access_M_FATChainFree( psFSRecord, *puFirstAllocatedCluster, false );
                    if ( uLastKnownCluster != 0 )
                    {
                        FAT_Access_M_SetClustersFatEntryContent( psFSRecord, uLastKnownCluster, FAT_EOF(psFSRecord));
                    }
                    goto exit;
                }

                if (uChainLen == 0) break;
                
                iErr = ZeroFill_Fill( psFSRecord->iFD, DIROPS_VolumeOffsetForCluster( psFSRecord, uStartClusterToZeroFill ), ( CLUSTER_SIZE(psFSRecord) * uChainLen ) );
                if ( iErr != 0 )
                {
                    MSDOS_LOG( LEVEL_ERROR, "FAT_Access_M_AllocateClusters: Failed to write zero buffer into the device\n");
                    FAT_Access_M_FATChainFree( psFSRecord, *puFirstAllocatedCluster, false );
                    if ( uLastKnownCluster != 0 )
                    {
                        FAT_Access_M_SetClustersFatEntryContent( psFSRecord, uLastKnownCluster, FAT_EOF(psFSRecord));
                    }
                    goto exit;
                }

                uStartClusterToZeroFill = NextCluster;
                uNumOfClustersToZeroFill -= uChainLen;
            }
        }
        
        uStartCluster = uLastAllocatedCluster;
        ++uCounter;
        
    } while ( uClustToAlloc > 0 && !bShouldStopAllocation);
    
    *puLastAllocatedCluster = uLastAllocatedCluster;
    *puAmountOfAllocatedClusters = uNumOfNewAllocatedClusters;
    
    goto exit;
    
fail:
    FAT_Access_M_FATChainFree( psFSRecord, *puFirstAllocatedCluster, false );

    if ( uLastKnownCluster != 0 )
    {
        FAT_Access_M_SetClustersFatEntryContent( psFSRecord, uLastKnownCluster, FAT_EOF(psFSRecord));
    }
    
exit:
    FAT_ACCESS_FREE(psFSRecord);
        
    return iErr;
}

/*
     Trunacte X last clusters of node record.
     Assumption : psFolderNode locked for write.
     return 0 if succeed, else return the appropiate error#.
 */
int
FAT_Access_M_TruncateLastClusters( NodeRecord_s* psNodeRecord, uint32_t uClusToTrunc )
{
    if ( psNodeRecord == NULL )
    {
        return EINVAL;
    }
    // In case of FAT12/16 Root Dir can't be truncated
    if (( psNodeRecord != NULL ) &&
        ( IS_FAT_12_16_ROOT_DIR(psNodeRecord) ) )
    {
        return EINVAL;
    }
    // Bug! assert for now..
    assert( uClusToTrunc <= psNodeRecord->sRecordData.uClusterChainLength );
    assert( uClusToTrunc > 0 );
    
    int iErr                        = 0;
    uint32_t uNextCluster           = 0;
    uint32_t uExtentLength          = 0;
    FileSystemRecord_s *psFSRecord  = GET_FSRECORD(psNodeRecord);
    uint32_t uNewClusterChainLen    = psNodeRecord->sRecordData.uClusterChainLength - uClusToTrunc;
    uint32_t uCluster               = psNodeRecord->sRecordData.uFirstCluster;

    FAT_ACCESS_LOCK(psFSRecord);

    if ( uNewClusterChainLen == 0 )
    {
        iErr = FAT_Access_M_FATChainFree( psFSRecord, psNodeRecord->sRecordData.uFirstCluster, false );
        if ( iErr != 0 )
        {
            goto exit;
        }
        psNodeRecord->sRecordData.uFirstCluster         = 0;
        psNodeRecord->sRecordData.uLastAllocatedCluster = 0;
        psNodeRecord->sRecordData.uClusterChainLength   = 0;
    }
    else
    {
        while (CLUSTER_IS_VALID(uCluster, psFSRecord))
        {
            uExtentLength = FAT_Access_M_ContiguousClustersInChain(psFSRecord, uCluster, &uNextCluster, &iErr);
            if ( iErr != 0 )
            {
                goto exit;
            }
            
            if ( uExtentLength >= uNewClusterChainLen )
            {
                psNodeRecord->sRecordData.uLastAllocatedCluster = uCluster + uNewClusterChainLen - 1;
                psNodeRecord->sRecordData.uClusterChainLength -= uClusToTrunc;
                iErr = FAT_Access_M_FATChainFree( psFSRecord, psNodeRecord->sRecordData.uLastAllocatedCluster, true );
                if ( iErr != 0 )
                {
                    goto exit;
                }
                uNewClusterChainLen = 0;
                break;
            }
            
            uNewClusterChainLen -= uExtentLength;
            uCluster            = uNextCluster;
        }
        
        if ( uNewClusterChainLen != 0 )
        {
            iErr = 1;
            goto exit;
        }
    }
    
exit:
    FAT_ACCESS_FREE(psFSRecord);
    return iErr;
}

int
FATMOD_SetDriveDirtyBit( FileSystemRecord_s *psFSRecord, bool bBitToDirty )
{
    int iErr                        = 0;
    uint32_t uFatVal                = 0;
    uint32_t uMask                  = 0;

    FAT_ACCESS_LOCK(psFSRecord);

    // In FAT12 we don't have a dirty bit so we don't have to check if device is dirty
    if (!(psFSRecord->sFatInfo.uFatMask == FAT12_MASK))
    {
        VolumeFatCache_s* psFATCache    = &psFSRecord->sFATCache;
        if ( psFATCache->bDriveDirtyBit != bBitToDirty )
        {
            MSDOS_LOG( LEVEL_DEBUG, "FATMOD_SetDriveDirtyBit: bBitToDirty = [%d]\n", bBitToDirty );

            // Get FAT entry for cluster #1.  Get the "clean shut down" bit.
            iErr = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, FAT_DIRTY_BIT_CLUSTER, &uFatVal );
            if ( iErr != 0 )
            {
                goto exit;
            }

            /* Figure out which bit in the FAT entry needs to change */
            if (psFSRecord->sFatInfo.uFatMask == FAT32_MASK)
                uMask = 1<<FAT_32_DIRTY_BIT_IDX;
            else
                uMask = 1<<FAT_16_DIRTY_BIT_IDX;

            /* Update the "clean" bit */
            if ( bBitToDirty )
                uFatVal &= ~uMask;
            else
                uFatVal |= uMask;

            FAT_Access_M_SetClustersFatEntryContent( psFSRecord, FAT_DIRTY_BIT_CLUSTER, uFatVal);

            // Look for the FAT cache entry
            uint8_t uIdx = FAT_Access_M_GetCacheEntryAccordingToCluster( psFSRecord, FAT_DIRTY_BIT_CLUSTER );
            if ( uIdx < FAT_CACHE_SIZE )
            {
                iErr = FAT_Access_M_FlushCacheEntry( psFSRecord, &psFATCache->psFATCacheEntries[uIdx] );
                if ( iErr != 0 )
                {
                    goto exit;
                }

                psFATCache->bDriveDirtyBit = bBitToDirty;
            }
        }
    }

exit:
    FAT_ACCESS_FREE(psFSRecord);
    return iErr;
}

int
FAT_Access_M_GetTotalFreeClusters( FileSystemRecord_s *psFSRecord, uint32_t* puNumOfFreeClusters )
{
    int iErr            = 0;
    uint32_t uCluster   = CLUST_FIRST;
    uint32_t uCount     = 0;
    uint32_t uVal       = 0;

    FAT_ACCESS_LOCK(psFSRecord);

    while ( uCluster <= psFSRecord->sFSInfo.uMaxCluster )
    {
        iErr = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, uCluster, &uVal );
        if ( iErr != 0 )
        {
            goto exit;
        }
        else if ( uVal == CLUST_FREE )
        {
            uCount++;
        }

        uCluster++;
    }

    MSDOS_LOG( LEVEL_DEFAULT, "Number of free clusters = [%u]\n", uCount );
    *puNumOfFreeClusters = uCount;

exit:
    FAT_ACCESS_FREE(psFSRecord);
    return iErr;
}
