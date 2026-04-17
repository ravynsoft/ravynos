/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  FileRecord_M.c
 *  msdosfs.kext
 *
 *  Created by Susanne Romano on 04/10/2017.
 */

#include "FileRecord_M.h"
#include "FAT_Access_M.h"
#include "DirOPS_Handler.h"
#include "Conv.h"
#include "FileOPS_Handler.h"
#include "diagnostic.h"
#include "Logger.h"
#include "Naming_Hash.h"

/* ------------------------------------------------------------------------------------------
 *                                  File Record operations
 * ------------------------------------------------------------------------------------------ */
int
FILERECORD_AllocateRecord(NodeRecord_s** ppvNode, FileSystemRecord_s *psFSRecord, uint32_t uFirstCluster,
                              RecordIdentifier_e eRecordID, NodeDirEntriesData_s* psNodeDirEntriesData, const char *pcUTF8Name )
{
    int iError      = 0;
    void* pvBuffer  = NULL;
    size_t uRecordSize = sizeof(NodeRecord_s);

    // We allocate some extra data for directory nodes.
    if ( eRecordID == RECORD_IDENTIFIER_DIR || eRecordID == RECORD_IDENTIFIER_ROOT )
    {
        uRecordSize += sizeof(ClusterData_s);
    }
    *ppvNode = malloc(uRecordSize);

    if (*ppvNode!= NULL)
    {
        memset( *ppvNode, 0, uRecordSize );
        SET_NODE_AS_VALID((*ppvNode));

        (*ppvNode)->sRecordData.psFSRecord      = psFSRecord;
        (*ppvNode)->sRecordData.eRecordID       = eRecordID;
        (*ppvNode)->sRecordData.uFirstCluster   = uFirstCluster;
        (*ppvNode)->sRecordData.psClusterChain  = NULL;
        
        if ( eRecordID == RECORD_IDENTIFIER_DIR || eRecordID == RECORD_IDENTIFIER_ROOT )
        {
            (*ppvNode)->sExtraData.sDirData.uDirVersion     = 1;
            (*ppvNode)->sExtraData.sDirData.psDirClusterData = (ClusterData_s *)((*ppvNode) + 1);
            (*ppvNode)->sExtraData.sDirData.sHT = NULL;

#ifdef MSDOS_NLINK_IS_CHILD_COUNT
            (*ppvNode)->sExtraData.sDirData.uChildCount     = 0;
#endif
        }
        else if ( eRecordID == RECORD_IDENTIFIER_LINK )
        {
            uint32_t uBytesPerSector    = psFSRecord->sFSInfo.uBytesPerSector;
            pvBuffer                    = malloc(uBytesPerSector);
            if ( NULL == pvBuffer )
            {
                iError = ENOMEM;
                goto exit_with_error;
            }

            uint32_t uStartCluster  = DIROPS_GetStartCluster(psFSRecord, &psNodeDirEntriesData->sDosDirEntry);
            uint64_t uReadSize      = pread(psFSRecord->iFD, pvBuffer, uBytesPerSector, DIROPS_VolumeOffsetForCluster(psFSRecord,uStartCluster));
            if ( uBytesPerSector != uReadSize )
            {
                iError = errno;
                MSDOS_LOG(LEVEL_ERROR, "FILERECORD_AllocateRecord failed to read errno = %d\n", iError);
                goto exit_with_error;
            }

            struct symlink* psLink = (struct symlink*)pvBuffer;
            for (uint8_t uLengthCounter = 0; uLengthCounter < SYMLINK_LENGTH_LENGTH ; ++uLengthCounter)
            {
                char cLengthChar = psLink->length[uLengthCounter];
                (*ppvNode)->sExtraData.sSymLinkData.uSymLinkLength = 10 * ((*ppvNode)->sExtraData.sSymLinkData.uSymLinkLength) + cLengthChar - '0';
            }

            free(pvBuffer);
        }
        else
        {
            (*ppvNode)->sExtraData.sFileData.bIsPreAllocated = false;
        }
        
        if ( psNodeDirEntriesData )
        {
            (*ppvNode)->sRecordData.bIsNDEValid = true;
            (*ppvNode)->sRecordData.sNDE = *psNodeDirEntriesData;
        }
        
        if ( uFirstCluster == 0 )
        {
            (*ppvNode)->sRecordData.uLastAllocatedCluster   = 0;
            //WA for root in case of fat12/16. cluster chain length must be "1".
            if ( IS_FAT_12_16_ROOT_DIR((*ppvNode)) )
            {
                (*ppvNode)->sRecordData.uClusterChainLength = 1;
            }
            else
            {
                (*ppvNode)->sRecordData.uClusterChainLength = 0;
            }
        }
        else
        {
            iError = FAT_Access_M_ChainLength( psFSRecord, uFirstCluster, &(*ppvNode)->sRecordData.uClusterChainLength, NULL, &(*ppvNode)->sRecordData.uLastAllocatedCluster );
            if ( iError != 0 )
            {
                goto exit_with_error;
            }
        }

        //Save the name for quick FSAtter
        CONV_DuplicateName(&(*ppvNode)->sRecordData.pcUtf8Name, pcUTF8Name);

        //Initialize locks
        MultiReadSingleWrite_Init(&(*ppvNode)->sRecordData.sRecordLck);
        pthread_mutex_init(&(*ppvNode)->sRecordData.sUnAlignedWriteLck, NULL);
        
        for (int iCond = 0; iCond < NUM_OF_COND; iCond++) {
            pthread_cond_init( &(*ppvNode)->sRecordData.sCondTable[iCond].sCond, NULL );
            (*ppvNode)->sRecordData.sCondTable[iCond].uSectorNum = 0;
        }
    }
    else
    {
        MSDOS_LOG(LEVEL_ERROR, "FILERECORD_AllocateRecord: fail to allocate record.\n");
        iError = ENOMEM;
        goto exit_with_error;
    }

#ifdef MSDOS_NLINK_IS_CHILD_COUNT
    //If its a dir calculate num of children
    if ( eRecordID == RECORD_IDENTIFIER_DIR )
    {
        iError = DIROPS_CountChildrenInADirectory(*ppvNode);
        if ( iError != 0 )
        {
            goto exit_with_error;
        }
    }
#endif

    return iError;

exit_with_error:
    if ( NULL != *ppvNode )
    {
        free(*ppvNode);
    }
    *ppvNode = NULL;

    if ( NULL != pvBuffer )
    {
        free(pvBuffer);
    }

    return iError;
}

void FILERECORD_FreeRecord(NodeRecord_s* psNode)
{
    FILERECORD_EvictAllFileChainCacheEntriesFromGivenOffset(psNode,0);

    psNode->sRecordData.psClusterChain  = NULL;

    if (IS_DIR(psNode))
    {
        // Free any cached directory data (cached cluster, if any, and
        // hash table).
        DIROPS_DestroyHTForDirectory(psNode);
    }

    if (psNode->sRecordData.pcUtf8Name != NULL)
        free(psNode->sRecordData.pcUtf8Name);
    
    //Deinit locks
    MultiReadSingleWrite_DeInit(&psNode->sRecordData.sRecordLck);
    pthread_mutex_destroy(&psNode->sRecordData.sUnAlignedWriteLck);
    
    for (int iCond = 0; iCond < NUM_OF_COND; iCond++) {
        pthread_cond_destroy( &psNode->sRecordData.sCondTable[iCond].sCond);
    }
    
    // Invalidate magic
    INVALIDATE_NODE(psNode);

    //free File Record
    free(psNode);

    psNode = NULL;

}

/* ------------------------------------------------------------------------------------------
 *                              Cluster chain cache SPI
 * ------------------------------------------------------------------------------------------ */


static void     FILERECORD_EvictLRUChainCacheEntry(FileSystemRecord_s* psFSRecord);
static void     FILERECORD_RemoveChainCacheEntry(ClusterChainCacheEntry_s* psEntryToRemove);
static uint8_t  FILERECORD_GetLastElementNumInCacheEntry(ClusterChainCacheEntry_s* psEntry);
static int      FILERECORD_UpdateLastElementNumInCacheEntry(ClusterChainCacheEntry_s* psEntry);

static int      FILERECORD_CreateChainCacheEntry(NodeRecord_s* psNodeRecord,uint32_t uFirstCluster,uint64_t uOffsetInFile);
static void     FILERECORD_AddChainCacheEntryToLRU(FileSystemRecord_s* psFSRecord,ClusterChainCacheEntry_s* psNewClusterChainEntry);
static void     FILERECORD_SetChainCacheEntryInFileLocations(ClusterChainCacheEntry_s* psNewClusterChainEntry,NodeRecord_s* psNodeRecord);
static int      FILERECORD_FillChainCacheEntryWithData(NodeRecord_s* psNodeRecord, uint32_t uFirstCluster,uint64_t uOffsetInFile,ClusterChainCacheEntry_s* psClusterChainEntry);
static void     FILERECORD_GetClusterFromChainArray(NodeRecord_s* psNodeRecord,ClusterChainCacheEntry_s* psLookupEntry, uint32_t* puWantedCluster, uint32_t* puContiguousClusterLength,
                                                             uint64_t uWantedOffsetInFile);
static int      FILERECORD_LookForOffsetInFileChainCache(NodeRecord_s* psNodeRecord,ClusterChainCacheEntry_s** psLookupEntry,uint64_t uWantedOffsetInFile, uint32_t* puWantedCluster,
                                                             uint32_t* puContiguousClusterLength, bool* bFoundLocation);
static int FILERECORD_FindClusterToCreateChainCacheEntry(bool* pbFoundLocation, NodeRecord_s* psNodeRecord, uint32_t uOffsetLocationInClusterChain, uint64_t uWantedOffsetInFile,
                                                         uint32_t* puWantedCluster, uint32_t* puContiguousClusterLength);
/* ------------------------------------------------------------------------------------------ */

/* Assumption: Under write lock */
static void FILERECORD_FillConsecutiveClusterData(FileSystemRecord_s* psFSRecord, ClusterChainCacheEntry_s* psClusterChainEntry, uint32_t uCluster, uint8_t uConClusterCounter, uint32_t uAmountOfConsecutiveCluster, uint64_t* puOffsetInFile)
{
    psClusterChainEntry->psConsecutiveCluster[uConClusterCounter].uActualStartCluster = uCluster;
    psClusterChainEntry->psConsecutiveCluster[uConClusterCounter].uAmountOfConsecutiveClusters = uAmountOfConsecutiveCluster;
    psClusterChainEntry->psConsecutiveCluster[uConClusterCounter].uClusterOffsetFromFileStart = (uint32_t) (*puOffsetInFile / CLUSTER_SIZE(psFSRecord));
    psClusterChainEntry->uAmountOfClusters += uAmountOfConsecutiveCluster;
    *puOffsetInFile += uAmountOfConsecutiveCluster * CLUSTER_SIZE(psFSRecord);
}

static int
FILERECORD_FillChainCacheEntryWithData(NodeRecord_s* psNodeRecord, uint32_t uFirstCluster, uint64_t uOffsetInFile, ClusterChainCacheEntry_s* psClusterChainEntry)
{
    errno_t Error = 0;
    uint32_t uCluster = uFirstCluster;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);
    psClusterChainEntry->uFileOffset = uOffsetInFile;
    psClusterChainEntry->uAmountOfClusters = 0;
    psClusterChainEntry->pvFileOwner = psNodeRecord;

    // look for consecutive clusters from the given first cluster
    for (uint8_t uConClusterCounter = 0; uConClusterCounter < MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY; uConClusterCounter++)
    {
        uint32_t uNextCluster;
        uint32_t uAmountOfConsecutiveCluster = FAT_Access_M_ContiguousClustersInChain(psFSRecord, uCluster, &uNextCluster, &Error);
        if (Error != 0)
            return Error;

        FILERECORD_FillConsecutiveClusterData(psFSRecord, psClusterChainEntry, uCluster, uConClusterCounter, uAmountOfConsecutiveCluster, &uOffsetInFile);
        
        uCluster = uNextCluster;
        // Check if reached to the end of the file
        if (!CLUSTER_IS_VALID(uCluster, psFSRecord))
        {
            break;
        }
    }
    return Error;
}

/* Assumption: Under write lock */
static void
FILERECORD_SetChainCacheEntryInFileLocations(ClusterChainCacheEntry_s* psNewClusterChainEntry,NodeRecord_s* psNodeRecord)
{
    if (psNodeRecord->sRecordData.psClusterChain == NULL)
    {// If this is the first cluster chain entry for this file
        psNewClusterChainEntry->psNextClusterChainCacheEntry = NULL;
        psNewClusterChainEntry->psPrevClusterChainCacheEntry = NULL;
        
        psNodeRecord->sRecordData.psClusterChain = psNewClusterChainEntry;
        
        return;
    }
    
    // Need to look for the correct location to insedrt the record
    ClusterChainCacheEntry_s* psLookupEntry = psNodeRecord->sRecordData.psClusterChain;
    bool bFoundLocation = false;
    while (!bFoundLocation && psLookupEntry->psNextClusterChainCacheEntry != NULL)
    {
        // If current entry offset is larger then the new entry offset,
        // new entry should come before the current entry
        if (psLookupEntry->uFileOffset > psNewClusterChainEntry->uFileOffset)
        {
            psNewClusterChainEntry->psPrevClusterChainCacheEntry = psLookupEntry->psPrevClusterChainCacheEntry;
            if (psLookupEntry->psPrevClusterChainCacheEntry != NULL)
            {
                // If current entry was not the first entry
                psNewClusterChainEntry->psPrevClusterChainCacheEntry->psNextClusterChainCacheEntry = psNewClusterChainEntry;
            }
            else
            {
                // If current entry was the first entry
                // Need to update the file node
                psNodeRecord->sRecordData.psClusterChain = psNewClusterChainEntry;
            }

            psNewClusterChainEntry->psNextClusterChainCacheEntry = psLookupEntry;
            psLookupEntry->psPrevClusterChainCacheEntry = psNewClusterChainEntry;

            bFoundLocation = true;
        }
        else
        {
            psLookupEntry = psLookupEntry->psNextClusterChainCacheEntry;
        }
    }

    if (!bFoundLocation)
    {
        //We reached to the end of the entries list
        psNewClusterChainEntry->psPrevClusterChainCacheEntry = psLookupEntry;
        psLookupEntry->psNextClusterChainCacheEntry = psNewClusterChainEntry;
        psNewClusterChainEntry->psNextClusterChainCacheEntry = NULL;
    }
}

/* Assumption: Under write lock */
static void
FILERECORD_RemoveChainCacheEntry(ClusterChainCacheEntry_s* psEntryToRemove)
{
    NodeRecord_s* psFileOwner = GET_RECORD(psEntryToRemove->pvFileOwner);
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psFileOwner);
    
    // Handle LRU pointers
    psFSRecord->psClusterChainCache->uAmountOfEntries--;
    bool bSetLRULocation = false;

    // In case this is the LRU item
    if (psFSRecord->psClusterChainCache->psChainCacheLRU == psEntryToRemove)
    {
        psFSRecord->psClusterChainCache->psChainCacheLRU = psEntryToRemove->psLRUNext;
        if ( psEntryToRemove->psLRUNext!= NULL )
            psEntryToRemove->psLRUNext->psLRUPrev = NULL;

        bSetLRULocation = true;
    }

    // In case this is the MRU item
    if (psFSRecord->psClusterChainCache->psChainCacheMRU == psEntryToRemove)
    {
        psFSRecord->psClusterChainCache->psChainCacheMRU = psEntryToRemove->psLRUPrev;
        if ( psEntryToRemove->psLRUPrev!= NULL )
            psEntryToRemove->psLRUPrev->psLRUNext = NULL;

        bSetLRULocation = true;
    }

    if (!bSetLRULocation)
    {
        psEntryToRemove->psLRUNext->psLRUPrev =  psEntryToRemove->psLRUPrev;
        psEntryToRemove->psLRUPrev->psLRUNext = psEntryToRemove->psLRUNext;
    }

    // If this is the first entry in the file
    if (psEntryToRemove == psFileOwner->sRecordData.psClusterChain)
    {
        psFileOwner->sRecordData.psClusterChain = psEntryToRemove->psNextClusterChainCacheEntry;
        if (psEntryToRemove->psNextClusterChainCacheEntry != NULL)
            psEntryToRemove->psNextClusterChainCacheEntry->psPrevClusterChainCacheEntry = NULL;
    }
    else
    {
        psEntryToRemove->psPrevClusterChainCacheEntry->psNextClusterChainCacheEntry = psEntryToRemove->psNextClusterChainCacheEntry;
        if ( psEntryToRemove->psNextClusterChainCacheEntry != NULL )
            psEntryToRemove->psNextClusterChainCacheEntry->psPrevClusterChainCacheEntry = psEntryToRemove->psPrevClusterChainCacheEntry;
    }
    
    free(psEntryToRemove);
    psEntryToRemove = NULL;
}

/* Assumption: Under write lock */
static void
FILERECORD_EvictLRUChainCacheEntry(FileSystemRecord_s* psFSRecord)
{
    ClusterChainCacheEntry_s* psLRUEntryToEvict = psFSRecord->psClusterChainCache->psChainCacheLRU;
    ClusterChainCacheEntry_s* psLRUEntryIter = psLRUEntryToEvict->psLRUNext;

    while (psLRUEntryIter)
    {
        if (psLRUEntryIter->uLRUCounter < psLRUEntryToEvict->uLRUCounter)
        {
            psLRUEntryToEvict = psLRUEntryIter;
        }
        psLRUEntryIter = psLRUEntryIter->psLRUNext;
    }

    FILERECORD_RemoveChainCacheEntry(psLRUEntryToEvict);
}

/* Assumption: Under write lock */
static void
FILERECORD_AddChainCacheEntryToLRU(FileSystemRecord_s* psFSRecord,ClusterChainCacheEntry_s* psNewClusterChainEntry)
{
    psNewClusterChainEntry->uLRUCounter = atomic_fetch_add(&psFSRecord->psClusterChainCache->uMaxLRUCounter,1);

    psFSRecord->psClusterChainCache->uAmountOfEntries++;
    if (psFSRecord->psClusterChainCache->uAmountOfEntries > MAX_CHAIN_CACHE_ENTRIES)
    {// Need to evict the LRU item before adding a new one
        FILERECORD_EvictLRUChainCacheEntry( psFSRecord);
    }

    if (psFSRecord->psClusterChainCache->psChainCacheLRU == NULL)
    {
        psFSRecord->psClusterChainCache->psChainCacheLRU = psNewClusterChainEntry;
    }

    psNewClusterChainEntry->psLRUPrev = psFSRecord->psClusterChainCache->psChainCacheMRU;
    if (psFSRecord->psClusterChainCache->psChainCacheMRU != NULL)
    {
        psFSRecord->psClusterChainCache->psChainCacheMRU->psLRUNext = psNewClusterChainEntry;
    }
    psFSRecord->psClusterChainCache->psChainCacheMRU = psNewClusterChainEntry;
    psNewClusterChainEntry->psLRUNext = NULL;
}

int FILERECORD_UpdateNewAllocatedClustersInChain(NodeRecord_s* psNodeRecord, uint32_t uFirstCluster, uint32_t uChainLength, uint64_t uOffsetInFile)
{
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);
    ClusterChainCacheEntry_s* psLookupEntry = psNodeRecord->sRecordData.psClusterChain;
    
new_one:
    //If file node doesn't have cache
    if (psLookupEntry == NULL) {
        ClusterChainCacheEntry_s* psNewClusterChainEntry = malloc(sizeof(ClusterChainCacheEntry_s));
        if (psNewClusterChainEntry == NULL)
        {
            MSDOS_LOG(LEVEL_ERROR, "FILERECORD_UpdateNewAllocatedClustersInChain failed to allocate memory\n");
            return ENOMEM;
        }

        memset(psNewClusterChainEntry,0,sizeof(ClusterChainCacheEntry_s));

        psNewClusterChainEntry->uFileOffset = uOffsetInFile;
        psNewClusterChainEntry->uAmountOfClusters = 0;
        psNewClusterChainEntry->pvFileOwner = psNodeRecord;

        FILERECORD_FillConsecutiveClusterData(psFSRecord, psNewClusterChainEntry, uFirstCluster, 0, uChainLength, &uOffsetInFile);

        // Set In File location
        FILERECORD_SetChainCacheEntryInFileLocations(psNewClusterChainEntry, psNodeRecord);

        // Set LRU location
        FILERECORD_AddChainCacheEntryToLRU(psFSRecord, psNewClusterChainEntry);
        
        return 0;
    }
    
    //Look for last allocated cluster chain
    do
    {
        if (psLookupEntry->psNextClusterChainCacheEntry != NULL)
        {
            psLookupEntry = psLookupEntry->psNextClusterChainCacheEntry;
        }
        else
        {
            //This Entry is Full, need new one
            if (psLookupEntry->psConsecutiveCluster[MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY-1].uAmountOfConsecutiveClusters != 0)
            {
                psLookupEntry = NULL;
                goto new_one;
            }
            else
            {
                //Find an empty spot and fill it with the new data
                uint8_t uArrayCounter;
                for (uArrayCounter = 0; uArrayCounter < MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY ; uArrayCounter++)
                {
                    if (psLookupEntry->psConsecutiveCluster[uArrayCounter].uActualStartCluster == 0)
                    {
                        FILERECORD_FillConsecutiveClusterData(psFSRecord, psLookupEntry, uFirstCluster, uArrayCounter, uChainLength, &uOffsetInFile);
                        psLookupEntry->uLRUCounter = atomic_fetch_add(&psFSRecord->psClusterChainCache->uMaxLRUCounter,1);
                        break;
                    }
                }
                
                assert(uArrayCounter != MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY);
            }
            break;
        }
    }while (psLookupEntry != NULL);
    
    return 0;
}

static int
FILERECORD_UpdateLastElementNumInCacheEntry(ClusterChainCacheEntry_s* psEntry )
{
    int iError = 0;
    uint8_t uArrayCounter;
    for ( uArrayCounter = 0; uArrayCounter <= MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY ; uArrayCounter++)
    {
        // Reached to an empty element (or last element),
        // try to update the previous element, with new data.
        if (uArrayCounter == MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY || psEntry->psConsecutiveCluster[uArrayCounter].uActualStartCluster == 0)
        {
            if (uArrayCounter != 0) uArrayCounter --;
            NodeRecord_s* psNodeRecord = (NodeRecord_s*) psEntry->pvFileOwner;
            uint32_t uLastKnownCluster = psEntry->psConsecutiveCluster[uArrayCounter].uActualStartCluster + psEntry->psConsecutiveCluster[uArrayCounter].uAmountOfConsecutiveClusters -1;

            if (CLUSTER_IS_VALID(uLastKnownCluster, GET_FSRECORD(psNodeRecord)))
            {
                break;
            }

            //Check if last known element can be extended
            uint32_t NextCluster;
            uint32_t uNewChainLength = 0;
            iError = FAT_Access_M_GetClustersFatEntryContent(GET_FSRECORD(psNodeRecord), uLastKnownCluster, &NextCluster);
            if (iError)
            {
                break;
            }

            if (NextCluster == uLastKnownCluster + 1)
            {
                uNewChainLength = FAT_Access_M_ContiguousClustersInChain(GET_FSRECORD(psNodeRecord), NextCluster, &NextCluster, &iError);
                if (iError)
                {
                    break;
                }

                psEntry->psConsecutiveCluster[uArrayCounter].uAmountOfConsecutiveClusters += uNewChainLength ;
                psEntry->uAmountOfClusters += uNewChainLength ;

            }
            uArrayCounter++;

            //While we still can populate
            while (uArrayCounter < MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY)
            {
                // If still not the end of the file -> try to populate more
                if (CLUSTER_IS_VALID(NextCluster, GET_FSRECORD(psNodeRecord)))
                {
                    uLastKnownCluster = NextCluster;
                    uNewChainLength = FAT_Access_M_ContiguousClustersInChain(GET_FSRECORD(psNodeRecord), uLastKnownCluster, &NextCluster, &iError);
                    if (iError)
                    {
                        break;
                    }

                    psEntry->psConsecutiveCluster[uArrayCounter].uActualStartCluster = uLastKnownCluster;
                    psEntry->psConsecutiveCluster[uArrayCounter].uAmountOfConsecutiveClusters = uNewChainLength;
                    psEntry->psConsecutiveCluster[uArrayCounter].uClusterOffsetFromFileStart = psEntry->psConsecutiveCluster[uArrayCounter-1].uClusterOffsetFromFileStart+psEntry->psConsecutiveCluster[uArrayCounter-1].uAmountOfConsecutiveClusters;
                    psEntry->uAmountOfClusters += uNewChainLength;
                }
                else
                {
                    break;
                }

                uArrayCounter++;
            }

            break;
        }
    }

    return iError;
}

/*
 * Since the array containing the contiguous clusters, dosn't have to be always fully occupied,
 * in order to get the last known cluster we need to look for an element with cluster == 0
 * (since cluster 0 can't be a valid cluster).
 *
 * Assumption: Under write lock
 */
static uint8_t
FILERECORD_GetLastElementNumInCacheEntry(ClusterChainCacheEntry_s* psEntry)
{
    uint8_t uArrayCounter;
    for ( uArrayCounter = 0; uArrayCounter < MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY; uArrayCounter++)
    {
        if ( psEntry->psConsecutiveCluster[uArrayCounter].uActualStartCluster == 0)
        {
            break;
        }
    }
    
    return --uArrayCounter;
}

/* Assumption: Under write lock */
static int
FILERECORD_FindClusterToCreateChainCacheEntry(bool* pbFoundLocation, NodeRecord_s* psNodeRecord, uint32_t uOffsetLocationInClusterChain, uint64_t uWantedOffsetInFile,
                                                             uint32_t* puWantedCluster, uint32_t* puContiguousClusterLength)
{
    int iError = 0;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);
    while (!(*pbFoundLocation))
    {
        uint32_t NextCluster;
        uint32_t uNewChainLength = FAT_Access_M_ContiguousClustersInChain(psFSRecord, *puWantedCluster, &NextCluster, &iError);
        if (iError)
        {
            MSDOS_LOG(LEVEL_ERROR, "FILERECORD_FindClusterToCreateChainCacheEntry failed allocateerror [%d].\n", iError);
            *puContiguousClusterLength = 0;
            break;
        }

        if (uOffsetLocationInClusterChain < uNewChainLength)
        {
            *puWantedCluster += uOffsetLocationInClusterChain;
            *puContiguousClusterLength = uNewChainLength - uOffsetLocationInClusterChain;
            //If we reached the end of the chain
            // Create new cache entry after the last one exsiting
            iError = FILERECORD_CreateChainCacheEntry(psNodeRecord, *puWantedCluster, ROUND_DOWN( uWantedOffsetInFile, CLUSTER_SIZE(psFSRecord)));
            if (iError)
            {
                *puContiguousClusterLength = 0;
                MSDOS_LOG(LEVEL_ERROR, "FILERECORD_FindClusterToCreateChainCacheEntry  fail to create cache entry [%d].\n", iError);
                break;
            }

            *pbFoundLocation = true;
        }
        else if ((uOffsetLocationInClusterChain == uNewChainLength) && !CLUSTER_IS_VALID(NextCluster, psFSRecord))
        {
            //If we got to the end of the cluster chain
            *puContiguousClusterLength = 0;
            break;
        }
        else
        {
            *puWantedCluster = NextCluster;
            uOffsetLocationInClusterChain -= uNewChainLength;
        }
    }

    return iError;
}

/* Assumption: Under write lock */
static int
FILERECORD_CreateChainCacheEntry(NodeRecord_s* psNodeRecord, uint32_t uFirstCluster, uint64_t uOffsetInFile)
{
    int iError = 0;
    ClusterChainCacheEntry_s* psNewClusterChainEntry = malloc(sizeof(ClusterChainCacheEntry_s));
    if (psNewClusterChainEntry == NULL)
    {
        MSDOS_LOG(LEVEL_ERROR, "FILERECORD_CreateChainCacheEntry failed to allocate memory\n");
        return ENOMEM;
    }

    memset(psNewClusterChainEntry,0,sizeof(ClusterChainCacheEntry_s));

    iError = FILERECORD_FillChainCacheEntryWithData(psNodeRecord, uFirstCluster, uOffsetInFile, psNewClusterChainEntry);
    if (iError)
    {
        free(psNewClusterChainEntry);
        return iError;
    }

    // Set In File location
    FILERECORD_SetChainCacheEntryInFileLocations(psNewClusterChainEntry,psNodeRecord);

    // Set LRU location
    FILERECORD_AddChainCacheEntryToLRU(GET_FSRECORD(psNodeRecord), psNewClusterChainEntry);
    
    return 0;
}

/* Assumption: Under read lock */
static void
FILERECORD_GetClusterFromChainArray(NodeRecord_s* psNodeRecord,ClusterChainCacheEntry_s* psLookupEntry, uint32_t* puWantedCluster, uint32_t* puContiguousClusterLength, uint64_t uWantedOffsetInFile)
{
    *puContiguousClusterLength = 0;
    uint32_t uClusterDistanceFromFileStart = (uint32_t) uWantedOffsetInFile/CLUSTER_SIZE(GET_FSRECORD(psNodeRecord));
    
    //Go over the elements and look for the wanted cluster
    for (uint8_t uArrayCounter =0; uArrayCounter < MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY; uArrayCounter++)
    {
        uint32_t clusterFromEntryStart = uClusterDistanceFromFileStart - psLookupEntry->psConsecutiveCluster[uArrayCounter].uClusterOffsetFromFileStart;
        if ((psLookupEntry->psConsecutiveCluster[uArrayCounter].uAmountOfConsecutiveClusters == 0) ||
            (uClusterDistanceFromFileStart < psLookupEntry->psConsecutiveCluster[uArrayCounter].uClusterOffsetFromFileStart))
        {
            break;
        }
        
        if (clusterFromEntryStart < psLookupEntry->psConsecutiveCluster[uArrayCounter].uAmountOfConsecutiveClusters)
        {
            *puWantedCluster = psLookupEntry->psConsecutiveCluster[uArrayCounter].uActualStartCluster + clusterFromEntryStart;
            *puContiguousClusterLength = psLookupEntry->psConsecutiveCluster[uArrayCounter].uAmountOfConsecutiveClusters - clusterFromEntryStart;
            break;
        }
    }
}

/* Assumption: Under read lock
 * Returns the last known psLookupEntry in case uWantedOffsetInFile is not in the cache*/
static int
FILERECORD_LookForOffsetInFileChainCache(NodeRecord_s* psNodeRecord,ClusterChainCacheEntry_s** psLookupEntry,uint64_t uWantedOffsetInFile, uint32_t* puWantedCluster,
                        uint32_t* puContiguousClusterLength, bool* bFoundLocation)
{
    int iError =0;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);

    do
    {
        // If LookupEntryOffset is larger then wnated offset we know that there is no cache for the wanted offset
        if ((*psLookupEntry)->uFileOffset > uWantedOffsetInFile)
        {
            // Need to allocate new cache entry with the wanted offset
            // The new location is before psLookupEntry
            *psLookupEntry = (*psLookupEntry)->psPrevClusterChainCacheEntry;
            break;
        }

        uint64_t uSizeOfCurrentEntry = (*psLookupEntry)->uAmountOfClusters*CLUSTER_SIZE(psFSRecord);
        uint64_t uEndOfEntryOffset   = (*psLookupEntry)->uFileOffset+ uSizeOfCurrentEntry;
        //If the wanted offset is within the current entry range
        if (((*psLookupEntry)->uFileOffset <= uWantedOffsetInFile) && (uWantedOffsetInFile < uEndOfEntryOffset))
        {
            // Need to find the wanted cluster according to the distance between
            // the wanted offset to the current entry offset

            FILERECORD_GetClusterFromChainArray(psNodeRecord,*psLookupEntry, puWantedCluster, puContiguousClusterLength, uWantedOffsetInFile);
            if (*puContiguousClusterLength != 0) {
                *bFoundLocation = true;
                break;
            }
        }

        if ((*psLookupEntry)->psNextClusterChainCacheEntry != NULL)
        {
            *psLookupEntry = (*psLookupEntry)->psNextClusterChainCacheEntry;
        }
        else
        {
            break;
        }

    }while (*psLookupEntry != NULL);

    return iError;
}


/* ------------------------------------------------------------------------------------------
 *                              Cluster chain cache API
 * ------------------------------------------------------------------------------------------ */

int
FILERECORD_InitChainCache(FileSystemRecord_s *psFSRecord)
{
    psFSRecord->psClusterChainCache = malloc(sizeof(ClusterChainCache_s));
    if (psFSRecord->psClusterChainCache != NULL)
    {
        psFSRecord->psClusterChainCache->psChainCacheLRU = NULL;
        psFSRecord->psClusterChainCache->psChainCacheMRU = NULL;
        psFSRecord->psClusterChainCache->uAmountOfEntries = 0;
        psFSRecord->psClusterChainCache->uMaxLRUCounter = 0;

        //Init Locks
        MultiReadSingleWrite_Init(&psFSRecord->psClusterChainCache->sClusterChainLck);
    }
    else
    {
        MSDOS_LOG(LEVEL_ERROR, "FILERECORD_InitChainCache: fail to allocate record.\n");
        return ENOMEM;
    }

    return 0;
}

void
FILERECORD_DeInitChainCache(FileSystemRecord_s *psFSRecord)
{
    // Deinit Locks
    MultiReadSingleWrite_DeInit(&psFSRecord->psClusterChainCache->sClusterChainLck);
    if (psFSRecord->psClusterChainCache) free(psFSRecord->psClusterChainCache);
    psFSRecord->psClusterChainCache = NULL;
}

void
FILERECORD_EvictAllFileChainCacheEntriesFromGivenOffset(NodeRecord_s* psNodeRecord,uint64_t uOffsetToEvictFrom)
{
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);
    // Lock Cache for write
    MultiReadSingleWrite_LockWrite(&psFSRecord->psClusterChainCache->sClusterChainLck);

    ClusterChainCacheEntry_s* psClusterChainToEvict = psNodeRecord->sRecordData.psClusterChain;
    ClusterChainCacheEntry_s* psClusterChainNext;

    while (psClusterChainToEvict != NULL)
    {
        //Calculate entry end offset
        uint64_t uEntryEndOffset = psClusterChainToEvict->uFileOffset +  psClusterChainToEvict->uAmountOfClusters * psNodeRecord->sRecordData.psFSRecord->sFSInfo.uBytesPerCluster;
        psClusterChainNext = psClusterChainToEvict->psNextClusterChainCacheEntry;

        //Evict only if given offset is before entry end offset
        if (uOffsetToEvictFrom <= uEntryEndOffset )
        {
            FILERECORD_RemoveChainCacheEntry(psClusterChainToEvict);
        }

        psClusterChainToEvict = psClusterChainNext;
    }

    // Unlock Cache
    MultiReadSingleWrite_FreeWrite(&psFSRecord->psClusterChainCache->sClusterChainLck);
}

/*
 * Returns a set of contiguous clusters according to a given offset in a file
 * The function will look if the given offset is already inside the cache.
 * In case not, will lookup the wanted offset from the FAT, and will update the cache.
 */
void
FILERECORD_GetChainFromCache(NodeRecord_s* psNodeRecord, uint64_t uWantedOffsetInFile, uint32_t* puWantedCluster, uint32_t* puContiguousClusterLength, int* piError)
{
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);

    //Lock the cache for write
    MultiReadSingleWrite_LockWrite(&psFSRecord->psClusterChainCache->sClusterChainLck);

    *puWantedCluster = psNodeRecord->sRecordData.uFirstCluster;
    uint32_t uOffsetLocationInClusterChain  = (uint32_t) (uWantedOffsetInFile/CLUSTER_SIZE(psFSRecord));
    bool bFoundLocation = false;
    //In case of FAT12/16 Root Dir can't be extended
    if ( IS_FAT_12_16_ROOT_DIR(psNodeRecord) )
    {
        *puContiguousClusterLength =  1;
        goto exit;
    }

    //In case this file was created empy
    if (psNodeRecord->sRecordData.uFirstCluster == 0)
    {
        *puContiguousClusterLength =  0;
        goto exit;
    }

    ClusterChainCacheEntry_s* psLookupEntry = psNodeRecord->sRecordData.psClusterChain;

    if (psLookupEntry == NULL)
    {
        //This file node has no cache entry
        //Lookup will start from the begining of the file
        //Get cluster according to offset
        //Create the new cache entry
        *piError = FILERECORD_FindClusterToCreateChainCacheEntry(&bFoundLocation, psNodeRecord, uOffsetLocationInClusterChain, uWantedOffsetInFile, puWantedCluster, puContiguousClusterLength);

        goto exit;
    }

    //Look for wanted offset In file chain
    FILERECORD_LookForOffsetInFileChainCache(psNodeRecord, &psLookupEntry, uWantedOffsetInFile, puWantedCluster, puContiguousClusterLength, &bFoundLocation);

    //Try to update the last entry and check if wanted offset inside
    if (psLookupEntry != NULL && !bFoundLocation)
    {
        *piError = FILERECORD_UpdateLastElementNumInCacheEntry( psLookupEntry );
        if ( *piError != 0 )
        {
            *puContiguousClusterLength = 0;
            goto exit;
        }

        uOffsetLocationInClusterChain = (uint32_t) ((uWantedOffsetInFile - psLookupEntry->uFileOffset)/CLUSTER_SIZE(psFSRecord));
        if (uOffsetLocationInClusterChain < psLookupEntry->uAmountOfClusters)
        {
            FILERECORD_GetClusterFromChainArray(psNodeRecord, psLookupEntry, puWantedCluster, puContiguousClusterLength, uWantedOffsetInFile);
            if (*puContiguousClusterLength == 0) {
                MSDOS_LOG(LEVEL_DEBUG, "Could not find cluster in the cache.\n");
            }
            else
            {
                bFoundLocation = true;
            }
        }
    }

    if (bFoundLocation && psLookupEntry!= NULL)
    {
        psLookupEntry->uLRUCounter = atomic_fetch_add(&psFSRecord->psClusterChainCache->uMaxLRUCounter,1);
    }
    //If we reached to the last file cache entry or we found a hole
    //in the cache and we need to create a new cache entry
    else
    {
        // Lookup will start from the last relevant cluster
        // Get cluster according to offset
        if (psLookupEntry != NULL)
        {
            //Look for the last element with data
            uint8_t uLastElementWithRelevantCluster = FILERECORD_GetLastElementNumInCacheEntry(psLookupEntry);
            uint64_t uLastKnownOffset = psLookupEntry->uFileOffset + (psLookupEntry->uAmountOfClusters * CLUSTER_SIZE(psFSRecord));
            uOffsetLocationInClusterChain = (uint32_t) ((uWantedOffsetInFile - uLastKnownOffset)/CLUSTER_SIZE(psFSRecord));
            ConsecutiveClusterChacheEntry_s* psElement = &psLookupEntry->psConsecutiveCluster[uLastElementWithRelevantCluster];
            *puWantedCluster = psElement->uActualStartCluster + psElement->uAmountOfConsecutiveClusters - 1;

            //Get the next cluster according to last known cluster
            *piError = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, *puWantedCluster, puWantedCluster);
            //If we reached the end of the chain
            if ( *piError != 0 || !CLUSTER_IS_VALID(*puWantedCluster, psFSRecord))
            {
                MSDOS_LOG(LEVEL_DEBUG, "Next Cluster is invalid, end of chain.\n");
                *puContiguousClusterLength = 0;
                goto exit;
            }
        }
        else
        {
            // No referance entry-> Start from the begining
            *puWantedCluster = psNodeRecord->sRecordData.uFirstCluster;
            uOffsetLocationInClusterChain = (uint32_t) (uWantedOffsetInFile)/CLUSTER_SIZE(psFSRecord);
        }
        
        *piError = FILERECORD_FindClusterToCreateChainCacheEntry(&bFoundLocation, psNodeRecord, uOffsetLocationInClusterChain, uWantedOffsetInFile, puWantedCluster, puContiguousClusterLength);
    } //if (!bFoundLocation)

exit:
    //Unlock the cache
    MultiReadSingleWrite_FreeWrite(&psFSRecord->psClusterChainCache->sClusterChainLck);
}

static int
FILERECORD_CompareRecordsLock( const void *pvNR1, const void *pvNR2 )
{
    int64_t iRes= *(NodeRecord_s**)pvNR1 - *(NodeRecord_s**)pvNR2;
    return iRes>0? 1 : iRes==0? 0 : -1;
}

static int
FILERECORD_CompareRecordsRelease( const void *pvNR1, const void *pvNR2 )
{
    int64_t iRes= *(NodeRecord_s**)pvNR1 - *(NodeRecord_s**)pvNR2;
    return iRes>0? -1 : iRes==0? 0 : 1;
}

/*
 * Lock/Release multiple NodeReords by order.
 */
void
FILERECORD_MultiLock( NodeRecord_s** ppsNRs, int iNumOfElem, bool bIsWrite, bool bLock )
{
    qsort(ppsNRs, iNumOfElem, sizeof(NodeRecord_s*), bLock? FILERECORD_CompareRecordsLock : FILERECORD_CompareRecordsRelease);

    for ( int iIdx=0; iIdx<iNumOfElem; iIdx++ )
    {
        if ( (ppsNRs[iIdx] != NULL)  )
        {
            if ( (iIdx == 0) || ppsNRs[iIdx] != ppsNRs[iIdx-1])
            {
                if ( bLock )
                {
                    if ( bIsWrite )
                    {
                        MultiReadSingleWrite_LockWrite(&ppsNRs[iIdx]->sRecordData.sRecordLck);
                    }
                    else
                    {
                        MultiReadSingleWrite_LockRead(&ppsNRs[iIdx]->sRecordData.sRecordLck);
                    }
                }
                else
                {
                    if ( bIsWrite )
                    {
                        MultiReadSingleWrite_FreeWrite(&ppsNRs[iIdx]->sRecordData.sRecordLck);
                    }
                    else
                    {
                        MultiReadSingleWrite_FreeRead(&ppsNRs[iIdx]->sRecordData.sRecordLck);
                    }
                }
            }
        }
    }
}
