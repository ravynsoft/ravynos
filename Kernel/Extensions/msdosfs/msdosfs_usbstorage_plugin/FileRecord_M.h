/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  FileRecord_M.h
 *  msdosfs.kext
 *
 *  Created by Susanne Romano on 04/10/2017.
 */

#ifndef FileRecord_Access_M_h
#define FileRecord_Access_M_h

#include "Common.h"

// --------------------------- File Record perations -----------------------------------

int     FILERECORD_AllocateRecord(NodeRecord_s** ppvNode, FileSystemRecord_s *psFSRecord, uint32_t uFirstCluster, RecordIdentifier_e eRecordID, NodeDirEntriesData_s* psNodeDirEntries, const char *pcUTF8Name);
void    FILERECORD_FreeRecord(NodeRecord_s* ppvNode);

// ------------------- Cluster Chain Cache Operations -----------------------------------

int     FILERECORD_InitChainCache(FileSystemRecord_s *psFSRecord);
void    FILERECORD_DeInitChainCache(FileSystemRecord_s *psFSRecord);
void    FILERECORD_EvictAllFileChainCacheEntriesFromGivenOffset(NodeRecord_s* psNodeRecord,uint64_t uOffsetToEvictFrom);
void    FILERECORD_GetChainFromCache(NodeRecord_s* psNodeRecord,uint64_t uWantedOffsetInFile,uint32_t* puWantedCluster, uint32_t* puContiguousClusterLength, int* piError);
int     FILERECORD_UpdateNewAllocatedClustersInChain(NodeRecord_s* psNodeRecord, uint32_t uFirstCluster, uint32_t uChainLength, uint64_t uOffsetInFile);
// ----------------- Locks -------------------
void FILERECORD_MultiLock( NodeRecord_s** ppsNRs, int iNumOfElem, bool bIsWrite, bool bLock );


#endif /* FileRecord_M_h */
