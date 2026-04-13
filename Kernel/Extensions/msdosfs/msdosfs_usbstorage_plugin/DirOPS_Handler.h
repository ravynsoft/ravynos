/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  DirOPS_Handler.h
 *  msdosfs
 *
 *  Created by Or Haimovich on 15/10/17.
 */

#ifndef DirOPS_Handler_h
#define DirOPS_Handler_h

#include <stdio.h>
#include "Common.h"

#define IS_ROOT(psNodeRecord) ( (psNodeRecord->sRecordData.eRecordID == RECORD_IDENTIFIER_ROOT) ? (1) : (0) )

#define IS_FAT_12_16_ROOT_DIR(psNodeRecord) ( ( (psNodeRecord->sRecordData.eRecordID == RECORD_IDENTIFIER_ROOT) && \
                                                !(GET_FSRECORD(psNodeRecord)->sFatInfo.uFatMask == FAT32_MASK) ) ? (1) : (0) )


typedef enum
{
    LU_BY_NAME,
    LU_BY_FIRST_CLUSTER_NUM,
    LU_BY_SHORT_NAME,
    LU_BY_VOLUME_ENTRY,
    LU_BY_SEARCH_CRITERIA,
    
} LookForDirEntryMethod_e;

typedef struct
{
    scandir_matching_request_t* psMatchingCriteria;
    scandir_matching_reply_t*   psMatchingResult;

    uint64_t uStartOffset;
} ScanDirRequest_s;

typedef union
{
    struct unistr255* psSearchName;
    uint32_t          uFirstClusterIdx;
    char              pcShortName[SHORT_NAME_LEN];
    ScanDirRequest_s  sScanDirRequest;
} LookForDirEntryExtraData_s;

typedef struct
{
    LookForDirEntryMethod_e     eMethod;
    LookForDirEntryExtraData_s  sData;
    
} LookForDirEntryArgs_s;

int MSDOS_ReadDir(UVFSFileNode dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytesRead, uint64_t *verifier);
int MSDOS_ReadDirAttr(UVFSFileNode dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytesRead, uint64_t *verifier);
int MSDOS_MkDir(UVFSFileNode dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);
int MSDOS_RmDir(UVFSFileNode dirNode, const char *name);
int MSDOS_Remove(UVFSFileNode dirNode, const char *name, UVFSFileNode victimNode);
int MSDOS_Lookup(UVFSFileNode dirNode, const char *name, UVFSFileNode *outNode);
int MSDOS_ScanDir (UVFSFileNode psDirNode, scandir_matching_request_t* psMatchingCriteria, scandir_matching_reply_t* psMatchingResult);

extern uint8_t puRecordId2FaType [RECORD_IDENTIFIER_AMOUNT];

#ifdef MSDOS_NLINK_IS_CHILD_COUNT
int         DIROPS_CountChildrenInADirectory( NodeRecord_s* psFolderNode);
#endif
bool        DIROPS_IsDotOrDotDotName(const char* pcUTF8Name);
uint64_t    DIROPS_VolumeOffsetForCluster(FileSystemRecord_s *psFSRecord, uint32_t uCluster);
bool        DIROPS_VerifyIfLinkAndGetLinkLength(struct symlink* psLink, uint32_t* puLinkLength);
int         DIROPS_GetDirCluster(NodeRecord_s* psFolderNode, uint32_t uWantedClusterOffsetInChain, ClusterData_s* psClusterData);
int         DIROPS_UpdateDirectoryEntry(NodeRecord_s* psFolderNode, NodeDirEntriesData_s* psNodeDirEntriesData, struct dosdirentry* psDosDirEntry, bool bLockDirEntryAccess);
int         DIROPS_UpdateDirLastModifiedTime( NodeRecord_s* psFolderNode );
int         DIROPS_CreateNewEntry(NodeRecord_s* psFolderNode, const char *pcUTF8Name,const UVFSFileAttributes *attrs,uint32_t uNodeStartCluster,int uEntryType);
void        DIROPS_GetMD5Digest(void * pvText, size_t uLength, char pcDigest[33]);
int         DIROPS_LookForDirEntry( NodeRecord_s* psFolderNode, LookForDirEntryArgs_s* psArgs, RecordIdentifier_e* peRecoredId, NodeDirEntriesData_s* psNodeDirEntriesData );
void        DIROPS_SetStartCluster( FileSystemRecord_s* psFSRecord,  struct dosdirentry* psEntry, uint32_t uNewStartCluster );
int         DIROPS_isDirEmpty( NodeRecord_s* psFolderNode );
int         DIROPS_MarkNodeDirEntriesAsDeleted( NodeRecord_s* psFolderNode, NodeDirEntriesData_s* psNodeDirEntriesData, const char *pcUTF8Name);
uint32_t    DIROPS_GetStartCluster( FileSystemRecord_s* psFSRecord,  struct dosdirentry* psEntry );
RecordIdentifier_e DIROPS_GetRecordId( struct dosdirentry* psDirEntry, NodeRecord_s* psFolderNode );
int         DIROPS_LookupInternal( UVFSFileNode dirNode, const char *pcUTF8Name, UVFSFileNode *outNode, bool bLockDirEntryAccess );
int         DIROPS_LookForDirEntryByName (NodeRecord_s* psFolderNode, const char *pcUTF8Name, RecordIdentifier_e* peRecoredId, NodeDirEntriesData_s* psNodeDirEntriesData);
int         DIROPS_CreateHTForDirectory( NodeRecord_s* psFolderNode);
void        DIROPS_ReleaseHTForDirectory(NodeRecord_s* psFolderNode, bool bForceEvict);
void        DIROPS_DestroyHTForDirectory(NodeRecord_s* psFolderNode);
#endif /* DirOPS_Handler_h */
