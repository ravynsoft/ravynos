/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  FAT_Access_M.h
 *  usbstorage_plugin
 */

#ifndef FAT_Access_M_h
#define FAT_Access_M_h

#include <stdio.h>
#include "Common.h"

// (Maximum) size of a FAT buffer.
//
// This is assumed to be a power of two.  It must be at least as large as
// the sector size (which could be as large as 4096).  Since FAT12 entries
// cross sector boundaries, it needs to be a multiple of two sectors.
// The maximum size of a FAT12 FAT is 6KiB, so using 8KiB here guarantees
// the entire FAT12 FAT will be read/written at one time, which simplifies
// things.
#define   FAT_BLOCK_SIZE (8192U)
#define   FAT_BLOCK_MASK (FAT_BLOCK_SIZE -1)
#define   MAX_BLOCK_SIZE (4096U)

typedef struct NewAllocatedClusterInfo{
    uint32_t uNewAlloctedStartCluster;
    uint32_t uAmountOfConsecutiveClusters;
    struct NewAllocatedClusterInfo* psNext;
} NewAllocatedClusterInfo_s;

uint32_t     FAT_Access_M_Fat32EntryOffset(uint32_t uCluster);
uint32_t     FAT_Access_M_Fat32EntryGet(uint32_t uCluster, uint8_t* ppuEntry);
uint32_t     FAT_Access_M_Fat32EntrySet(uint32_t uCluster, uint8_t* ppuEntry, uint32_t uValue);
uint32_t     FAT_Access_M_Fat16EntryOffset(uint32_t uCluster);
uint32_t     FAT_Access_M_Fat16EntryGet(uint32_t uCluster, uint8_t* ppuEntry);
uint32_t     FAT_Access_M_Fat16EntrySet(uint32_t uCluster, uint8_t* ppuEntry, uint32_t uValue);
uint32_t     FAT_Access_M_Fat12EntryOffset(uint32_t uCluster);
uint32_t     FAT_Access_M_Fat12EntryGet(uint32_t uCluster, uint8_t* ppuEntry);
uint32_t     FAT_Access_M_Fat12EntrySet(uint32_t uCluster, uint8_t* ppuEntry, uint32_t uValue);

errno_t      FAT_Access_M_ChainLength(FileSystemRecord_s *psFSRecord, uint32_t uCluster, uint32_t* puLength, NodeRecord_s *psNodeRecord, uint32_t* puLastCluster);
int          FAT_Access_M_FATInit(FileSystemRecord_s *psFSRecord);
void         FAT_Access_M_FATFini(FileSystemRecord_s *psFSRecord);
uint32_t     FAT_Access_M_ContiguousClustersInChain( FileSystemRecord_s *psFSRecord, uint32_t uCluster, uint32_t* uNextCluster, errno_t* pError );
int          FAT_Access_M_FATChainFree( FileSystemRecord_s *psFSRecord, uint32_t uStart, bool bIsPartialRelease );
int          FAT_Access_M_AllocateClusters( FileSystemRecord_s *psFSRecord, uint32_t uClustToAlloc, uint32_t uLastKnownCluste, uint32_t* puFirstAllocatedCluster, uint32_t* puLastAllocatedCluster, uint32_t* puAmountOfAllocatedClusters, bool bPartialAllocationAllowed, bool bFillWithZeros, NewAllocatedClusterInfo_s** ppsNewAllocatedClusterInfoToReturn, bool bMustBeContiguousAllocation );
int          FAT_Access_M_TruncateLastClusters( NodeRecord_s* psFolderNode, uint32_t uClusToTrunc );
int          FAT_Access_M_GetTotalFreeClusters( FileSystemRecord_s *psFSRecord, uint32_t* puNumOfFreeClusters );
int          FAT_Access_M_FindFirstFreeClusterFromGivenCluster(FileSystemRecord_s *psFSRecord, uint32_t uCluster);
int          FAT_Access_M_GetClustersFatEntryContent( FileSystemRecord_s *psFSRecord, uint32_t uCluster, uint32_t* puNextCluser );

int          FATMOD_SetDriveDirtyBit( FileSystemRecord_s *psFSRecord,bool bBitToDirty );
int          FATMOD_FlushAllCacheEntries(FileSystemRecord_s *psFSRecord);

#endif /* FAT_Access_M_h */
