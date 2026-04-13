/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  DirOPS_Handler.c
 *  msdosfs
 *
 *  Created by Or Haimovich on 15/10/17.
 */

#include "DirOPS_Handler.h"
#include "FAT_Access_M.h"
#include "FileRecord_M.h"
#include "FSOPS_Handler.h"
#include "Conv.h"
#include "Logger.h"
#include <CommonCrypto/CommonDigest.h>

#include <sys/time.h>
#include <sys/stat.h>
#include "FileOPS_Handler.h"
#include "Naming_Hash.h"

#include "diagnostic.h"

#define DOT_DIR_COOKIE              ( 0 )
#define DOT_DIR_SIZE                ( UVFS_DIRENTRY_RECLEN(1) )
#define DOT_X2_COOKIE               ( DOT_DIR_SIZE )
#define DOT_X2_DIR_SIZE             ( UVFS_DIRENTRY_RECLEN(2) )
#define SYNTHESIZE_ROOT_DOTS_SIZE   ( DOT_DIR_SIZE + DOT_X2_DIR_SIZE )
#define DOT_COOKIE_TO_SIZE(uCookie) (uCookie==DOT_DIR_COOKIE? DOT_DIR_SIZE : DOT_X2_DIR_SIZE)

#define DIR_BAD_CLUSTER (-17)
#define MAX_AMOUNT_OF_SHORT_GENERATION_NUM (1000000)

typedef enum
{
    DIR_ENTRY_DELETED = 0x10,
    DIR_ENTRY_EMPTY,
    DIR_ENTRY_FOUND,
    DIR_ENTRY_VOL_NAME,
    DIR_ENTRY_UNKNOWN,
    DIR_ENTRY_START,
    DIR_ENTRY_ERROR,
    DIR_ENTRY_EOD
    
} DirEntryStatus_e;

uint8_t puRecordId2FaType [RECORD_IDENTIFIER_AMOUNT] = {
    [RECORD_IDENTIFIER_DIR]     = UVFS_FA_TYPE_DIR,
    [RECORD_IDENTIFIER_ROOT]    = UVFS_FA_TYPE_DIR,
    [RECORD_IDENTIFIER_FILE]    = UVFS_FA_TYPE_FILE,
    [RECORD_IDENTIFIER_LINK]    = UVFS_FA_TYPE_SYMLINK,
};


//---------------------------------- Functions Decleration ---------------------------------------

static bool         DIROPS_IsEntryASymLink(struct dosdirentry* psEntry, FileSystemRecord_s *psFSRecord, int* piError);
static DirEntryStatus_e DIROPS_GetDirEntryByOffset( NodeRecord_s* psFolderNode, ClusterData_s* psClusterData, uint64_t uEntryOffset,
                                                   uint64_t* puDosEntryOffset, uint64_t* puNextEnteryOffset, NodeDirEntriesData_s* psNodeDirEntriesData, struct unistr255* psName, uint32_t *uError);
static void DIROPS_SynthesizeDotAndDotX2( NodeRecord_s* psFolderNode, uint8_t* puBuf, uint32_t* uBytesWrite, uint64_t uCookie );
static int  DIROPS_VerifyCookieAndVerifier( NodeRecord_s* psFolderNode, uint64_t uCookie, ClusterData_s* psClusterData, bool bIsReadAttr, uint64_t uVerifier );
static int  DIROPS_LookForFreeEntriesInDirectory(NodeRecord_s* psFolderNode,uint32_t uAmountOfNewEntries,uint32_t* puNewStartEntryOffset);
static int  DIROPS_CreateShortNameEntry(NodeRecord_s* psFolderNode, struct unistr255* psName, uint8_t puShortName[SHORT_NAME_LEN], int iShortNameKind, struct dosdirentry* psDosDirEntry, const UVFSFileAttributes *psAttrs, uint8_t uIsLowerCase, uint32_t uStartCluster, int uEntryType);
static int  DIROPS_SaveNewEntriesIntoDevice(NodeRecord_s* psFolderNode,struct dosdirentry* psShortNameDosDirEntry, uint32_t uNewStartEntryOffset,struct unistr255* psName,uint32_t uAmountOfNewEntries);
static int  DIROPS_ReadDirInternal(UVFSFileNode dirNode, void* pvBuf, size_t uBufLen, uint64_t uCookie, size_t *bytesRead, uint64_t *puVerifier, bool bIsReadAttr);
static bool DIROPS_DirScanIsMatch(NodeRecord_s* psFolderNode, struct unistr255* psName, ScanDirRequest_s* psScanDirRequest, NodeDirEntriesData_s* psNodeDirEntriesData, uint32_t* piErr);
static bool DIROPS_CompareTimes(const struct timespec* psTimeA, const struct timespec* psTimeB);
static int  DIROPS_PopulateHT(NodeRecord_s* psFolderNode, LF_HashTable_t* psHT, bool bAllowExistence);
static void DIROPS_ReleaseHTLRUSlot(NodeRecord_s* psFolderNode);
static void DIROPS_CleanRefereaceForHTForDirectory(NodeRecord_s* psFolderNode, LF_HashTable_t** ppsHT, uint8_t** ppuClusterData);
//---------------------------------- Functions Implementation ------------------------------------

/*
 * Assumptions: psFolderNode lock for write, global sDirEntryAccess lock for write.
 *
 * Update the last modified fields.
 *
 * Windows never updates the times in a directory's directory entry.
 * Since we wish to update them persistently, we use the times from
 * the "." entry in the directory. Since the root doesn't have a "."
 * entry, we use the times in the volume label entry (if there is one).
 */
int
DIROPS_UpdateDirLastModifiedTime( NodeRecord_s* psFolderNode )
{
    int iErr = 0;
    struct timespec sCurrentTS;
    uint16_t uTime, uDate;

    if ( !IS_DIR( psFolderNode) )
    {
        // Bug!
        iErr =  ENOTDIR;
        goto exit;
    }

    CONV_GetCurrentTime( &sCurrentTS );
    msdosfs_unix2dostime( &sCurrentTS, &uDate, &uTime, NULL);

    // Root directory behave like a regular file entry.
    if ( IS_ROOT( psFolderNode ) )
    {
        if ( psFolderNode->sRecordData.bIsNDEValid )
        {
            struct dosdirentry sDosDirEntry = psFolderNode->sRecordData.sNDE.sDosDirEntry;
            putuint16(sDosDirEntry.deMDate, uDate);
            putuint16(sDosDirEntry.deMTime, uTime);

            iErr = DIROPS_UpdateDirectoryEntry( psFolderNode, &psFolderNode->sRecordData.sNDE, &sDosDirEntry, false );
            if ( iErr )
            {
                MSDOS_LOG( LEVEL_ERROR, "Failed to update directory entry err = %d\n", iErr );
            }
        }

        goto exit;
    }
    else
    {
        uint32_t uSectorSize = psFolderNode->sRecordData.psFSRecord->sFSInfo.uBytesPerSector;
        iErr = DIROPS_GetDirCluster(psFolderNode, 0, psFolderNode->sExtraData.sDirData.psDirClusterData);
        if ( iErr != 0 )
        {
            goto exit;
        }

        struct dosdirentry* psDirEntryStartPtr = (struct dosdirentry*)psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData;

        // Update only if there is a diff in the modified time.
        if ( (uTime != getuint16(psDirEntryStartPtr->deMTime)) ||
             (uDate != getuint16(psDirEntryStartPtr->deMDate)) )
        {

            psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = false;
            
            // Update modified time in '.' entry...
            putuint16(psDirEntryStartPtr->deMDate, uDate);
            putuint16(psDirEntryStartPtr->deMTime, uTime);

            size_t uBytesWrite = pwrite( psFolderNode->sRecordData.psFSRecord->iFD, psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData, uSectorSize,
                                        DIROPS_VolumeOffsetForCluster(psFolderNode->sRecordData.psFSRecord, psFolderNode->sRecordData.uFirstCluster) );
            if ( uBytesWrite != uSectorSize )
            {
                iErr = errno;
                goto exit;
            }
            
            psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = true;
        }
    }

exit:
    return iErr;
}

uint32_t
DIROPS_GetStartCluster( FileSystemRecord_s* psFSRecord,  struct dosdirentry* psEntry )
{
    uint32_t uStartCluster = getuint16(psEntry->deStartCluster);
    if ( psFSRecord->sFatInfo.uFatMask == FAT32_MASK )
    {
        uStartCluster |= ( getuint16(psEntry->deHighClust) << 16 );
    }
    
    return uStartCluster;
}

void
DIROPS_SetStartCluster( FileSystemRecord_s* psFSRecord,  struct dosdirentry* psEntry, uint32_t uNewStartCluster )
{
    putuint16( psEntry->deStartCluster, (uNewStartCluster & 0xFFFF) );
    if ( psFSRecord->sFatInfo.uFatMask == FAT32_MASK )
    {
        putuint16( psEntry->deHighClust, ((uNewStartCluster >> 16) & 0xFFFF));
    }
}

uint64_t
DIROPS_VolumeOffsetForCluster(FileSystemRecord_s *psFSRecord, uint32_t uCluster)
{
    return (((uint64_t)uCluster - CLUST_FIRST) * CLUSTER_SIZE(psFSRecord)) + ((uint64_t)(psFSRecord->sFSInfo.uClusterOffset) * psFSRecord->sFSInfo.uBytesPerSector);
}

/* This is part of an on-disk format, silence the warning, won't be able to get away from this */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

/*
 * Given a chunk of memory, compute the md5 digest as a string of 32
 * hex digits followed by a NUL character.
 */
void DIROPS_GetMD5Digest(void * pvText, size_t uLength, char pcDigest[33])
{    
    unsigned char pcDigestRaw[16];
    memset(pcDigestRaw,0,sizeof(pcDigestRaw));
    
    CC_MD5_CTX context;
    CC_MD5_Init(&context);
    CC_MD5_Update(&context, pvText, (unsigned)uLength);
    CC_MD5_Final(pcDigestRaw, &context);
    
    for (uint8_t uDigestoffset =0; uDigestoffset < 16; ++uDigestoffset)
    {
        /*
         * The "3" below is for the two hex digits plus trailing '\0'.
         * Note that the trailing '\0' from byte N is overwritten
         * by the first character of byte N+1, and that digest[] has
         * a length of 33 == 2 * 16 + 1 in order to have room for a
         * trailing '\0' after the last byte's characters.
         */
        (void) snprintf(pcDigest, 3, "%02x", pcDigestRaw[uDigestoffset]);
        pcDigest += 2;
    }
}
#pragma clang diagnostic pop

bool
DIROPS_VerifyIfLinkAndGetLinkLength(struct symlink* psLink, uint32_t* puLinkLength)
{
    // Verify the magic
    if (strncmp(psLink->magic, symlink_magic, SYMLINK_MAGIC_LENGTH) != 0)
    {
        return false;
    }
    
    *puLinkLength = 0;
    // Parse length field
    for (uint8_t uLengthCounter =0; uLengthCounter < SYMLINK_LENGTH_LENGTH ; ++uLengthCounter)
    {
        char cLengthChar = psLink->length[uLengthCounter];
        // Check if length is decimal
        if (cLengthChar < '0' || cLengthChar > '9')
        {
            // Length is non-decimal
            return false;
        }
        *puLinkLength = 10 * (*puLinkLength) + cLengthChar - '0';
    }
    
    // Verify unLength is a valid length
    if (*puLinkLength > SYMLINK_LINK_MAX)
    {
        return false;
    }
    
    
    char digest[33];
    // Verify the MD5 digest
    DIROPS_GetMD5Digest(psLink->link, *puLinkLength, digest);
    if (strncmp(digest, psLink->md5, 32) != 0)
        return false;
    
    // It passed all the checks;
    // must be a symlink
    return true;
}

bool DIROPS_IsDotOrDotDotName(const char* pcUTF8Name)
{
    size_t uNameLen = strlen(pcUTF8Name);
    if ( (  uNameLen == 1 && pcUTF8Name[0] == '.' ) ||
        ( uNameLen == 2 && pcUTF8Name[0] == '.' && pcUTF8Name[1] == '.' ) )
    {
        return true;
    }
    return false;
}

static bool
DIROPS_IsEntryASymLink(struct dosdirentry* psEntry, FileSystemRecord_s *psFSRecord, int* piError)
{
    if ( getuint32(psEntry->deFileSize) != sizeof(struct symlink) )
    {
        return false;
    }

    struct symlink* psLink;
    bool bIsLink = false;
    void* pvBuffer = NULL;
    
    // If Cluster == 0 it must be a file.
    uint32_t uStartCluster = DIROPS_GetStartCluster(psFSRecord, psEntry);
    
    if (!uStartCluster)
        goto exit;

    //Need to read the file to check if its a link or not
    uint32_t uLength = 0;
    uint32_t uLastCluster = 0;
    *piError = FAT_Access_M_ChainLength(psFSRecord, uStartCluster, &uLength, NULL, &uLastCluster);
    if (*piError != 0)
    {
        goto exit;
    }

    // Since link size is const, every file, that its cluster length not equals to link cluster length,
    // is automaticly not a link
    if (uLength * CLUSTER_SIZE(psFSRecord) != ROUND_UP(sizeof(struct symlink), CLUSTER_SIZE(psFSRecord)))
    {
        goto exit;
    }

    pvBuffer = malloc((uLength * CLUSTER_SIZE(psFSRecord)));
    
    if (pvBuffer == NULL)
    {
        *piError = ENOMEM;
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_IsEntryASymLink failed to allocate memory\n");
        return bIsLink;
    }
    
    uint32_t uNextCluster = 0;
    uint32_t uReadClusters = 0;
    uint32_t uReadAccClusters = 0;
    size_t uReadSize = 0;
    
    while (CLUSTER_IS_VALID(uStartCluster, psFSRecord))
    {
        uReadClusters = FAT_Access_M_ContiguousClustersInChain(psFSRecord, uStartCluster, &uNextCluster, piError);
        uReadAccClusters += uReadClusters;
        if (*piError)
        {
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_IsEntryASymLink failed to get next cont clusers = %d\n", *piError);
            break;
        }
        uReadSize +=  pread(psFSRecord->iFD, pvBuffer+uReadSize, uReadClusters*CLUSTER_SIZE(psFSRecord), DIROPS_VolumeOffsetForCluster(psFSRecord,uStartCluster));
        if ( (uReadAccClusters * CLUSTER_SIZE(psFSRecord)) != uReadSize  )
        {
            *piError = errno;
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_IsEntryASymLink failed to read errno = %d\n", *piError);
            break;
        }
        uStartCluster = uNextCluster;
    }
    
    if ( *piError )
    {
        goto exit;
    }
    
    psLink = (struct symlink*) pvBuffer;
    uint32_t uLinkLength;
    
    bIsLink = DIROPS_VerifyIfLinkAndGetLinkLength(psLink, &uLinkLength);
    
exit:
    if (pvBuffer)
        free(pvBuffer);
    return bIsLink;
    
}

static void
DIROPS_SynthesizeDotAndDotX2( NodeRecord_s* psFolderNode, uint8_t* puBuf, uint32_t* puBytesWrite, uint64_t uCookie )
{
    UVFSDirEntry* psDotEntry = (UVFSDirEntry*)puBuf;
    memset( psDotEntry, 0, DOT_COOKIE_TO_SIZE(uCookie) );

    psDotEntry->de_fileid       = MSDOS_GetFileID(psFolderNode);
    psDotEntry->de_filetype     = UVFS_FA_TYPE_DIR;
    psDotEntry->de_reclen       = DOT_COOKIE_TO_SIZE(uCookie);
    psDotEntry->de_nextcookie   = uCookie + DOT_COOKIE_TO_SIZE(uCookie);
    uint8_t* puNameBuf          = (uint8_t*)psDotEntry->de_name;
    puNameBuf[0]                = '.';
    if ( uCookie == DOT_DIR_COOKIE )
    {
        puNameBuf[1] = '\0';
        psDotEntry->de_namelen  = 1;
    }
    else
    {
        puNameBuf[1] = '.';
        puNameBuf[2] = '\0';
        psDotEntry->de_namelen  = 2;
    }
    
    *puBytesWrite = DOT_COOKIE_TO_SIZE(uCookie);
}

static int
DIROPS_LookForFreeEntriesInDirectory(NodeRecord_s* psFolderNode, uint32_t uAmountOfNewEntries, uint32_t* puNewStartEntryOffset)
{
    uint32_t iError =0 ;
    uint32_t uAmountOfFreeEntriesFound = 0;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psFolderNode);
    *puNewStartEntryOffset = 0;
    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psFolderNode) )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForFreeEntriesInDirectory: Folder node is not a directory.\n");
        return ENOTDIR;
    }
    
    uint32_t uLastCluster =  psFolderNode->sRecordData.uLastAllocatedCluster;
    
    NodeDirEntriesData_s sNodeDirEntriesData;
    memset( &sNodeDirEntriesData,  0, sizeof(NodeDirEntriesData_s) );
    
    uint32_t uEntryOffset = 0;
    uint64_t uDosEntryOffset= 0;
    uint64_t uNextEnteryOffset;
    
    struct unistr255 *psName = malloc(sizeof(struct unistr255));
    if (psName == NULL)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForFreeEntriesInDirectory: Failed to allocate memory.\n");
        return ENOMEM;
    }
    DirEntryStatus_e eStatus = DIR_ENTRY_START;
    
    do {
        eStatus = DIROPS_GetDirEntryByOffset( psFolderNode, psFolderNode->sExtraData.sDirData.psDirClusterData, uEntryOffset, &uDosEntryOffset, &uNextEnteryOffset, &sNodeDirEntriesData, psName, &iError);
        //Look for series of deleted;
        if ( eStatus == DIR_ENTRY_DELETED )
        {
            //Update start entry offset if needed
            if (!uAmountOfFreeEntriesFound)
                *puNewStartEntryOffset = uEntryOffset;

            uAmountOfFreeEntriesFound++;
        }
        // If found non deleted/ non empty dir entry reset counters
        else if ( DIR_ENTRY_FOUND == eStatus || DIR_ENTRY_VOL_NAME == eStatus)
        {
            uAmountOfFreeEntriesFound = 0;
            *puNewStartEntryOffset = 0;
        }
        
        // If found empty - see if there is enough space to
        // insert the wanted amount of entries, else need to allocate new buffer
        // fill it with 0's. If root 12/16 can't add new file.
        else if (eStatus == DIR_ENTRY_EMPTY || eStatus == DIR_ENTRY_EOD)
        {
            uint32_t uDirSize = psFolderNode->sRecordData.uClusterChainLength * CLUSTER_SIZE(psFSRecord);
            if (IS_FAT_12_16_ROOT_DIR(psFolderNode))
            {
                uDirSize = psFSRecord->sRootInfo.uRootLength;
            }
            
            uint32_t uFreeEntriesToAllocate = (uDirSize - uEntryOffset) / sizeof(struct dosdirentry);

            //Update start entry offset if needed
            if (!uAmountOfFreeEntriesFound)
                *puNewStartEntryOffset = uEntryOffset;
            
            // If need to allocate new clusters
            if (uAmountOfFreeEntriesFound + uFreeEntriesToAllocate < uAmountOfNewEntries)
            {
                if (IS_FAT_12_16_ROOT_DIR(psFolderNode))
                {
                    MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForFreeEntriesInDirectory: Can't extend root folder.\n");
                    uLastCluster = 0;
                    iError = ENOSPC;
                }
                else
                {
                    //Clculate how many clusters are needed to be allocated
                    uint32_t uAmountOfNewFreeSpaceToExtend = (uAmountOfNewEntries - uAmountOfFreeEntriesFound) * sizeof(struct dosdirentry);
                    uint32_t uAmountOfClustersToExtendTheDir = ((ROUND_UP(uAmountOfNewFreeSpaceToExtend, CLUSTER_SIZE(psFSRecord))) / CLUSTER_SIZE(psFSRecord));

                    uint32_t uAmountOfAllocatedClusters = 0;
                    uint32_t uNewFirstAllocatedCluster = 0;
                    iError = FAT_Access_M_AllocateClusters(psFSRecord, uAmountOfClustersToExtendTheDir, uLastCluster, &uNewFirstAllocatedCluster, &uLastCluster, &uAmountOfAllocatedClusters, false, true, NULL, false);
                    if (iError != 0)
                    {
                        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForFreeEntriesInDirectory: Failed to allocate new cluster for dir expand [%d].\n",iError);
                        uLastCluster = 0;
                    }
                    else
                    {
                        //Update the extended dir params
                        psFolderNode->sRecordData.uClusterChainLength += uAmountOfAllocatedClusters;
                        psFolderNode->sRecordData.uLastAllocatedCluster = uLastCluster;
                        
                        uFreeEntriesToAllocate += (uAmountOfAllocatedClusters * CLUSTER_SIZE(psFSRecord)) / sizeof(struct dosdirentry);
                    }
                }
            }
            
            //Did not mannaged to allocate -> exit from the while loop
            if (uAmountOfFreeEntriesFound + uFreeEntriesToAllocate < uAmountOfNewEntries)
            {
                MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForFreeEntriesInDirectory: Failed to allocate new cluster for dir expand [%d].\n",iError);
                break;
            }
            
            // Found enough
            break;
        }
        // If error
        else if  ( eStatus == DIR_ENTRY_ERROR )
        {
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForFreeEntriesInDirectory:DIROPS_GetDirEntryByOffset error: %d.\n", iError);
            break;
        }
        
        uEntryOffset = (uint32_t) uNextEnteryOffset;
        
    } while ( uAmountOfFreeEntriesFound < uAmountOfNewEntries );

    free(psName);
    return iError;
}

/*
 * Determine a DOS (8.3-style) short name that is unique within the directory given
 * by "dep".  The short_name parameter is both input and output; on input, it is
 * the short name derived from the Unicode name (as produced by msdosfs_unicode_to_dos_name);
 * on output, it is the unique short name (which may contain a generation number).
 * The dir_offset parameter is the offset (in bytes, a multiple of 32) from the start
 * of the directory to the first long name entry (used as a hint to derive a
 * likely unique generation number), or 1 if no generation number should be used.
 */
static int
DIROPS_CreateShortNameEntry(NodeRecord_s* psFolderNode, struct unistr255* psName, uint8_t puShortName[SHORT_NAME_LEN], int iShortNameKind, struct dosdirentry* psDosDirEntry, const UVFSFileAttributes * psAttrs, uint8_t uIsLowerCase, uint32_t uStartCluster, int uEntryType)
{
    uint32_t uGenerationNum = 0;
    bool bNeedsGeneration = (iShortNameKind == 3);
    int iError = 0;
    bool bIsExist = false;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psFolderNode);
    
    LookForDirEntryArgs_s sArgs;
    memcpy(sArgs.sData.pcShortName,puShortName,SHORT_NAME_LEN);
    sArgs.eMethod = LU_BY_SHORT_NAME;
    NodeDirEntriesData_s sNodeDirEntriesData;
    RecordIdentifier_e eRecoredId;
    
    if (!bNeedsGeneration)
    {
        /*
         * uNeedsGeneration == 0 means the short name is case-insensitively equal to
         * the long name, so don't use a generation number.
         */
        iError = DIROPS_LookForDirEntry( psFolderNode, &sArgs, &eRecoredId, &sNodeDirEntriesData);
        if (iError == ENOENT)
            iError = 0;
        else
            if (!iError) MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateShortNameEntry: short name already exist.\n");
            else MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateShortNameEntry: error raised while looking for short dir entry [%d].\n",iError);
    }
    else
    {
        for ( uGenerationNum = 1; uGenerationNum < MAX_AMOUNT_OF_SHORT_GENERATION_NUM; ++uGenerationNum)
        {
            iError = msdosfs_apply_generation_to_short_name(puShortName, uGenerationNum);
            memcpy( sArgs.sData.pcShortName, puShortName, SHORT_NAME_LEN );
            if ( !iError )
                bIsExist = (DIROPS_LookForDirEntry( psFolderNode, &sArgs, &eRecoredId, &sNodeDirEntriesData) != ENOENT);
            
            if (!bIsExist)
                break;
        }
    }
    
    memset(psDosDirEntry,0,sizeof(struct dosdirentry));
     psDosDirEntry->deAttributes |= ATTR_ARCHIVE;
    // If the file name starts with ".", make it invisible on Windows.
    if (puShortName[0] == '.')
        psDosDirEntry->deAttributes |= ATTR_HIDDEN;
 
    // Update given start cluster
    DIROPS_SetStartCluster(psFSRecord, psDosDirEntry, uStartCluster);
    
    //Update lower case flag
    psDosDirEntry->deLowerCase = uIsLowerCase;
    
    //Update name
    memcpy(psDosDirEntry->deName,puShortName,SHORT_NAME_LEN);
    
    if (uEntryType == UVFS_FA_TYPE_DIR)
    {
        psDosDirEntry->deAttributes |= ATTR_DIRECTORY;
        //Only dir shoudln't have this attr
        psDosDirEntry->deAttributes &= ~ATTR_ARCHIVE;
        putuint32(psDosDirEntry->deFileSize,0);
    }
    else if (uEntryType == UVFS_FA_TYPE_SYMLINK)
    {
        putuint32(psDosDirEntry->deFileSize,sizeof(struct symlink));
    }
    else
    {
        //Size has already been checked against the maximum.
        uint64_t uSize = (psAttrs->fa_validmask & UVFS_FA_VALID_SIZE)
            ? psAttrs->fa_size : 0;
        putuint32(psDosDirEntry->deFileSize,(uint32_t) uSize);
    }
    
    //Update times
    struct timespec spec;
    CONV_GetCurrentTime(&spec);
    
    uint16_t uDate,uTime;
    uint8_t uSeconds;
    struct timespec sattrTimeVal;
    uint16_t uattrDate,uattrTime;
    uint8_t uattrSeconds;
    
    msdosfs_unix2dostime(&spec, &uDate, &uTime, &uSeconds);

    if (psAttrs->fa_validmask & UVFS_FA_VALID_BIRTHTIME)
    {
        sattrTimeVal = psAttrs->fa_birthtime;
        msdosfs_unix2dostime(&sattrTimeVal, &uattrDate, &uattrTime, &uattrSeconds);
        putuint16(psDosDirEntry->deCDate,uattrDate);
        putuint16(psDosDirEntry->deCTime,uattrTime);
    }
    else
    {
        putuint16(psDosDirEntry->deCDate,uDate);
        putuint16(psDosDirEntry->deCTime,uTime);
        psDosDirEntry->deCHundredth = uSeconds;
    }

    if (psAttrs->fa_validmask & UVFS_FA_VALID_MTIME)
    {
        sattrTimeVal = psAttrs->fa_mtime;
        msdosfs_unix2dostime(&sattrTimeVal, &uattrDate, &uattrTime, &uattrSeconds);
        putuint16(psDosDirEntry->deMDate,uattrDate);
        putuint16(psDosDirEntry->deMTime,uattrTime);
    }
    else
    {
        putuint16(psDosDirEntry->deMDate,uDate);
        putuint16(psDosDirEntry->deMTime,uTime);
    }

    if (psAttrs->fa_validmask & UVFS_FA_VALID_ATIME)
    {
        sattrTimeVal = psAttrs->fa_atime;
        msdosfs_unix2dostime(&sattrTimeVal, &uattrDate, &uattrTime, &uattrSeconds);
        putuint16(psDosDirEntry->deADate,uattrDate);
    }
    else
    {
        putuint16(psDosDirEntry->deADate,uDate);
    }
    
    return iError;
}

static int DIROPS_SaveNewEntriesIntoDevice(NodeRecord_s* psFolderNode, struct dosdirentry* psShortNameDosDirEntry, uint32_t uNewStartEntryOffset, struct unistr255* psName, uint32_t uAmountOfNewEntries)
{
    int iError = 0;

    switch (psName->length) {
        case 1:
        case 2:
            if (psName->chars[0] == '.' &&
                (psName->length == 1 || psName->chars[1] == '.'))
            {
                // Don't apply the no-trailing-dot rule to "." and "..".
                // Note: We will always write it out correctly to disk
                // (because the short name entry has already been created),
                // but we don't want to screw up the hash calculation for
                // later.
                break;
            }
            // otherwise, fall-through...

        default:
            CONV_convert_to_fsm(psName);
            break;
    }

    //Need to update modification time
    struct timespec spec;
    CONV_GetCurrentTime(&spec);
    uint16_t uDate,uTime;

    msdosfs_unix2dostime(&spec, &uDate, &uTime, NULL);
    putuint16(psShortNameDosDirEntry->deMDate,uDate);
    putuint16(psShortNameDosDirEntry->deMTime,uTime);

    u_int8_t chksum = msdosfs_winChksum(psShortNameDosDirEntry->deName);
    
    uint32_t uClusterSize = CLUSTER_SIZE(GET_FSRECORD(psFolderNode));
    if ( IS_FAT_12_16_ROOT_DIR(psFolderNode) )
    {
        uClusterSize = GET_FSRECORD(psFolderNode)->sRootInfo.uRootLength;
    }

    uint32_t uWantedClusterInChain = uNewStartEntryOffset / uClusterSize;
    iError = DIROPS_GetDirCluster( psFolderNode, uWantedClusterInChain , psFolderNode->sExtraData.sDirData.psDirClusterData );
    if ( iError != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_SaveNewEntriesIntoDevice: Get Dir Cluster error [%d].\n",iError);
        goto exit;
    }
    
    psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = false;
    
    //For each new dir entry
    for (uint32_t uLongNameCounter =0; uLongNameCounter < uAmountOfNewEntries; ++uLongNameCounter)
    {
        uint32_t uEntryNum = (uNewStartEntryOffset % uClusterSize)/sizeof(struct winentry);
        
        if ( uLongNameCounter < uAmountOfNewEntries -1)
        {
            //Insert long name entry
            struct winentry psClusterStart;
            msdosfs_unicode2winfn(psName->chars, psName->length, &psClusterStart, uAmountOfNewEntries - uLongNameCounter - 1, chksum);
            memcpy((uint8_t*) &((struct winentry*) psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData)[uEntryNum],(uint8_t*) &psClusterStart,sizeof(struct winentry));
        }
        else
        {
            // Insert short name entry
            struct dosdirentry* psClusterStart =  &((struct dosdirentry*) psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData)[uEntryNum];
            memcpy((uint8_t*) psClusterStart,(uint8_t*) psShortNameDosDirEntry,sizeof(struct dosdirentry));
        }
        
        uNewStartEntryOffset += sizeof(struct winentry);
        
        //If we need to replace a cluster, or we finished updating all entries - preform a write
        if ( (( uNewStartEntryOffset % uClusterSize ) == 0) || (uLongNameCounter == uAmountOfNewEntries - 1 ) )
        {
            // Flush the cluster data.
            ssize_t uSize = pwrite( GET_FSRECORD(psFolderNode)->iFD, psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData, uClusterSize, psFolderNode->sExtraData.sDirData.psDirClusterData->uAbsoluteClusterOffset );
            if ( uSize != uClusterSize )
            {
                iError = errno;
                MSDOS_LOG(LEVEL_ERROR, "DIROPS_SaveNewEntriesIntoDevice: Failed to write to the device error [%d].\n",iError);
                goto exit;
            }
            
            //After writing all is good
            psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = true;
            
            //Check if we need to replace a cluster
            if ( (( uNewStartEntryOffset % uClusterSize ) == 0) && (uLongNameCounter != uAmountOfNewEntries - 1) )
            {
                uWantedClusterInChain++;
                iError = DIROPS_GetDirCluster( psFolderNode, uWantedClusterInChain, psFolderNode->sExtraData.sDirData.psDirClusterData );
                if ( iError != 0 )
                {
                    MSDOS_LOG(LEVEL_ERROR, "DIROPS_SaveNewEntriesIntoDevice: Get Dir Cluster error [%d].\n",iError);
                    goto exit;
                }
                
                //We changed cluster, so we are not valid again
                psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = false;
            }
        }
    }
    
exit:
    return iError;
}

int
DIROPS_CreateNewEntry(NodeRecord_s* psFolderNode, const char *pcUTF8Name, const UVFSFileAttributes *attrs, uint32_t uNodeStartCluster, int uEntryType)
{
    int iError = 0;

    // For now.. catch some internal bugs..
    assert( (uEntryType == UVFS_FA_TYPE_DIR) || (attrs->fa_validmask & UVFS_FA_VALID_MODE ) );
    if ( !(attrs->fa_validmask & UVFS_FA_VALID_MODE) && (uEntryType != UVFS_FA_TYPE_DIR) )
    {
        return EINVAL;
    }

    if ( ( attrs->fa_validmask & READ_ONLY_FA_FIELDS ) /*|| ( attrs->fa_validmask & ~VALID_IN_ATTR_MASK )*/ )
    {
        return EINVAL;
    }

    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psFolderNode) )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateNewEntry: FolderNode is not a directory.\n");
        return ENOTDIR;
    }
    
    struct unistr255* psName = (struct unistr255*) malloc(sizeof(struct unistr255));
    if ( psName == NULL )
    {
        iError = ENOMEM;
        goto exit;
    }
    memset( psName, 0, sizeof(struct unistr255));

    // Convert the search name to UTF-16
    iError = CONV_UTF8ToUnistr255((const uint8_t *)pcUTF8Name, strlen(pcUTF8Name), psName, 0);
    if ( iError != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateNewEntry: fail to convert utf8 -> utf16.\n");
        goto exit;
    }
    
    uint32_t uAmountOfNewEntries = 1;
    uint8_t uIsLowerCase;
    uint8_t puShortName[SHORT_NAME_LEN];
    memset(puShortName,0x20,SHORT_NAME_LEN);
    
    // Determine the number of consecutive directory entries we'll need.
    int iShortNameKind = msdosfs_unicode_to_dos_name(psName->chars, psName->length, puShortName, &uIsLowerCase);
    switch (iShortNameKind) {
        case 0:
            /*
             * The name is syntactically invalid.  Normally, we'd return EINVAL,
             * but ENAMETOOLONG makes it clear that the name is the problem (and
             * allows Carbon to return a more meaningful error).
             */
            iError = ENAMETOOLONG;
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateNewEntry: Short name type invalid [%d].\n",iError);
            goto exit;
        case 1:
            // The name is already a short, DOS name, so no long name entries needed.
            uAmountOfNewEntries = 1;
            break;
        case 2:
        case 3:
            // The name needs long name entries.  The +1 is for the short name entry.
            uAmountOfNewEntries = msdosfs_winSlotCnt(psName->chars, (int)psName->length) + 1;
            break;
    }

    MultiReadSingleWrite_LockWrite( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );
    
    // Look for free slots in the directory
    uint32_t uNewStartEntryOffset;
    iError = DIROPS_LookForFreeEntriesInDirectory(psFolderNode, uAmountOfNewEntries, &uNewStartEntryOffset);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateNewEntry: Lookup for free directory entries failed with error [%d].\n",iError);
        goto free_lock;
    }
    
    // Generate short name entry
    struct dosdirentry  sShortNameDosDirEntry;
    iError = DIROPS_CreateShortNameEntry(psFolderNode, psName, puShortName, iShortNameKind, &sShortNameDosDirEntry, attrs,uIsLowerCase, uNodeStartCluster, uEntryType);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateNewEntry: Generate short name entry failed with error [%d].\n",iError);
        goto free_lock;
    }
    
    // Save all new entries into the device
    iError = DIROPS_SaveNewEntriesIntoDevice(psFolderNode, &sShortNameDosDirEntry, uNewStartEntryOffset, psName, uAmountOfNewEntries);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateNewEntry: Save new entries to device failed with error [%d].\n",iError);
        goto free_lock;
    }

    //
    // WE ARE PAST THE POINT OF NO RETURN.
    //

    //Update Directory version
    psFolderNode->sExtraData.sDirData.uDirVersion++;

#ifdef MSDOS_NLINK_IS_CHILD_COUNT
    //Update the parent child dir count
    psFolderNode->sExtraData.sDirData.uChildCount++;
#endif

    iError = ht_insert(psFolderNode->sExtraData.sDirData.sHT, psName, uNewStartEntryOffset, false);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateNewEntry: ht_insert failed with [%d].\n",iError);
    }

    DIROPS_UpdateDirLastModifiedTime( psFolderNode );

free_lock:
    MultiReadSingleWrite_FreeWrite( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );
exit:
    if (psName)
        free(psName);
    return iError;
}

//---------------------------------- SPI Implementation ------------------------------------------

int
MSDOS_MkDir (UVFSFileNode dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_MkDir\n");
    VERIFY_NODE_IS_VALID(dirNode);

    NodeRecord_s* psParentRecord = GET_RECORD(dirNode);
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psParentRecord);
    int iError = 0;

    // Lookup if file name exist
    RecordIdentifier_e eRecId;
    NodeDirEntriesData_s sNodeDirEntriesData;

    iError = DIROPS_CreateHTForDirectory(psParentRecord);
    if (iError)
        return iError;

    // Lock the parent node
    FSOPS_SetDirtyBitAndAcquireLck(psFSRecord);
    MultiReadSingleWrite_LockWrite( &psParentRecord->sRecordData.sRecordLck );

    //Lock DirEntry lock
    MultiReadSingleWrite_LockRead( &psFSRecord->sDirEntryAccessLock );
    
    // Lookup for node in the directory
    int iLookupError = DIROPS_LookForDirEntryByName (psParentRecord, name, &eRecId, &sNodeDirEntriesData );
    
    //Unlok DirEntry lock
    MultiReadSingleWrite_FreeRead( &psFSRecord->sDirEntryAccessLock );
    if ( iLookupError != ENOENT )
    {
        if (!iLookupError)
            iError = EEXIST;
        else
            iError = iLookupError;
        
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_MkDir: Look for same name dir entry failure [%d].\n", iError);
        goto exit;
    }
    
    // Allocate cluster for the new dir
    uint32_t uAllocatedCluster;
    uint32_t uLastAllocatedCluster;
    uint32_t uAmountOfAllocatedClusters =0;
    iError = FAT_Access_M_AllocateClusters(psParentRecord->sRecordData.psFSRecord, 1,0, &uAllocatedCluster, &uLastAllocatedCluster, &uAmountOfAllocatedClusters, false, true, NULL, false);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_MkDir: New cluster allocation failure [%d].\n", iError);
        goto exit;
    }
    
    // Create direntry in the parent directory
    iError = DIROPS_CreateNewEntry(psParentRecord, name, attrs, uAllocatedCluster, UVFS_FA_TYPE_DIR);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_MkDir: Create new dir entry failure [%d].\n", iError);
        goto free_allocated_clusters;
    }
    
    UVFSFileAttributes sParentAttributes;
    MSDOS_GetAtrrFromDirEntry(psParentRecord, &sParentAttributes);
    
    // Lookup for the new dir
    iError = DIROPS_LookupInternal(dirNode, name, outNode, true);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_MkDir: Lookup for new created file failure [%d].\n", iError);
        goto free_allocated_clusters;
    }
    
    NodeRecord_s* psNewFolderRecord = GET_RECORD(*outNode);
    
    // Create '.' and '..' entries
    iError = DIROPS_CreateNewEntry(psNewFolderRecord, ".",attrs,uAllocatedCluster,UVFS_FA_TYPE_DIR);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_MkDir: Create '.' entry failure [%d].\n", iError);
        goto free_allocated_clusters;
    }

    uint32_t uParentCluster = DIROPS_GetStartCluster(psFSRecord, &psParentRecord->sRecordData.sNDE.sDosDirEntry);
    sParentAttributes.fa_validmask &= ~READ_ONLY_FA_FIELDS;
    iError = DIROPS_CreateNewEntry(psNewFolderRecord, "..",&sParentAttributes,uParentCluster,UVFS_FA_TYPE_DIR);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_MkDir: Create '..' entry failure [%d].\n", iError);
        goto free_allocated_clusters;
    }

    DIAGNOSTIC_INSERT(GET_RECORD(*outNode),psParentRecord->sRecordData.uFirstCluster,name);

    if (!iError)
    {
        goto exit;
    }
    
free_allocated_clusters:
    FAT_Access_M_FATChainFree( psFSRecord,uAllocatedCluster,false);
exit:
    MultiReadSingleWrite_FreeWrite( &psParentRecord->sRecordData.sRecordLck );
    //Unlock dirtybit
    FSOPS_FlushCacheAndFreeLck(psFSRecord);
    //Getting EEXIST here, doesn't require force evict
    bool bReleaseHT = ((iError != 0) && (iError != EEXIST));
    DIROPS_ReleaseHTForDirectory(psParentRecord, bReleaseHT);
    return iError;
}

RecordIdentifier_e
DIROPS_GetRecordId( struct dosdirentry* psDirEntry, NodeRecord_s* psFolderNode )
{
    // Found record id.
    RecordIdentifier_e eRecId = RECORD_IDENTIFIER_ROOT;
    if ( psDirEntry->deAttributes & ATTR_DIRECTORY )
    {
        eRecId =  RECORD_IDENTIFIER_DIR;
    }
    else
    {
        //Need to check if symlink or regular file
        int iError = 0;
        
        if ( DIROPS_IsEntryASymLink( psDirEntry, GET_FSRECORD(psFolderNode), &iError) )
        {
            eRecId =  RECORD_IDENTIFIER_LINK;
        }
        else
        {
            if ( iError == 0 )
            {
                eRecId = RECORD_IDENTIFIER_FILE;
            }
            else
            {
                eRecId = RECORD_IDENTIFIER_UNKNOWN;
            }
        }
    }
    
    return eRecId;
}

static int
DIROPS_ReadDirInternal (UVFSFileNode dirNode, void* pvBuf, size_t uBufLen, uint64_t uCookie, size_t *bytesRead, uint64_t *puVerifier, bool bIsReadAttr)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_ReadDirInternal\n");
    
    NodeRecord_s* psFolderNode = GET_RECORD(dirNode);
    uint32_t uTotalDirEntries  = 0;
    uint64_t uPrevEntrySize    = 0;
    uint32_t uNextCookieOffset = 0;
    uint32_t iErr              = 0;
    struct unistr255* psName   = NULL;
    char* puUTF8               = NULL;
    bool bAddAtLeastOneEntry   = false;
    bool bSynthDots            = false;

    NodeDirEntriesData_s    sDirEntryEntriesData;
    memset( &sDirEntryEntriesData, 0, sizeof(sDirEntryEntriesData) );

    if (bytesRead == NULL || puVerifier == NULL)
    {
        return EINVAL;
    }
    *bytesRead = 0;

    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psFolderNode) )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ReadDirInternal node is not a directory.\n");
        return ENOTDIR;
    }
    
    // Make sure there is a place for at least one entry with maximal allowed name
    uint64_t uMaxRecLen = (!bIsReadAttr)? UVFS_DIRENTRY_RECLEN(FAT_MAX_FILENAME_UTF8) : _UVFS_DIRENTRYATTR_RECLEN(UVFS_DIRENTRYATTR_NAMEOFF, FAT_MAX_FILENAME_UTF8);
    if ( uBufLen < uMaxRecLen )
    {
        return EINVAL;
    }
    
    // Lock Directory for read.
    MultiReadSingleWrite_LockRead( &psFolderNode->sRecordData.sRecordLck );
    // Lock directories entries for read.
    MultiReadSingleWrite_LockRead( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );

    iErr = DIROPS_VerifyCookieAndVerifier( psFolderNode, uCookie, psFolderNode->sExtraData.sDirData.psDirClusterData, bIsReadAttr, *puVerifier);
    if ( iErr != 0 )
    {
        goto exit;
    }

    *puVerifier = psFolderNode->sExtraData.sDirData.uDirVersion;
    uint8_t* puBuf              = (uint8_t*)pvBuf;
    uint64_t uDosEntryOffset    = 0;
    uint64_t uNextEnteryOffset  = 0;

    psName  = (struct unistr255*)malloc(sizeof(struct unistr255));
    puUTF8  = (char*)malloc(FAT_MAX_FILENAME_UTF8);
    if ( psName == NULL || puUTF8 == NULL )
    {
        MSDOS_LOG(LEVEL_ERROR, "Failed to allocate memory");
        iErr = ENOMEM;
        goto exit;
    }
    memset( psName, 0, sizeof(struct unistr255) );
    memset( puUTF8, 0, FAT_MAX_FILENAME_UTF8    );

    /* At this point cookie and verifier is good, and we have space in the buffer to put at least one record */
    uint64_t uLastEntryOffset   = uCookie;
    DirEntryStatus_e eStatus    = DIR_ENTRY_START;
    
    if ( IS_ROOT( psFolderNode ) && !bIsReadAttr )
    {
        if (( uCookie == DOT_DIR_COOKIE ) || ( uCookie == DOT_X2_COOKIE ))
        {
            uint32_t uBytesWrite = 0;
            
            // Assuming uBufLen >= UVFS_DIRENTRY_RECLEN(FAT_MAX_FILENAME_UTF8), so there is no need to check buffer size.
            while ( uCookie < SYNTHESIZE_ROOT_DOTS_SIZE )
            {
                DIROPS_SynthesizeDotAndDotX2( psFolderNode, puBuf, &uBytesWrite, uCookie );
                uCookie         += uBytesWrite;
                puBuf           += uBytesWrite;
                *bytesRead      += uBytesWrite;
                uPrevEntrySize   = uBytesWrite;
                uTotalDirEntries++;
                bSynthDots = true;
            }
            
            uLastEntryOffset  = 0;
            uNextCookieOffset = SYNTHESIZE_ROOT_DOTS_SIZE;
        }
        else
        {
            uLastEntryOffset -= SYNTHESIZE_ROOT_DOTS_SIZE;
            uNextCookieOffset = SYNTHESIZE_ROOT_DOTS_SIZE;
        }
    }
    
    void* pvEntry = puBuf;
    
    do {
        eStatus = DIROPS_GetDirEntryByOffset( psFolderNode, psFolderNode->sExtraData.sDirData.psDirClusterData, uLastEntryOffset, &uDosEntryOffset, &uNextEnteryOffset, &sDirEntryEntriesData, psName, &iErr);
        if ( DIR_ENTRY_FOUND == eStatus )
        {
            // Calculate the new entry size.
            CONV_Unistr255ToUTF8(psName, puUTF8);

            // Ignore '.' and '..' for read attr.
            if ( !(bIsReadAttr && ( (strcmp(puUTF8, ".")==0) || (strcmp(puUTF8, "..")==0))) )
            {
                uint64_t uEntrySize = (!bIsReadAttr)? UVFS_DIRENTRY_RECLEN(strlen(puUTF8)) : _UVFS_DIRENTRYATTR_RECLEN(UVFS_DIRENTRYATTR_NAMEOFF, strlen(puUTF8));

                // Check if there is a place to add a new entry
                if ( uEntrySize + *bytesRead > uBufLen )
                {
                    if ( !bIsReadAttr )
                    {
                        UVFSDirEntry* psDirEntry    = (UVFSDirEntry*)(puBuf-uPrevEntrySize);
                        psDirEntry->de_nextrec      = 0;
                        psDirEntry->de_nextcookie   = uLastEntryOffset + uNextCookieOffset;
                    }
                    else
                    {
                        UVFSDirEntryAttr* psDirEntry = (UVFSDirEntryAttr*)(puBuf-uPrevEntrySize);
                        psDirEntry->dea_nextrec      = 0;
                        psDirEntry->dea_nextcookie   = uLastEntryOffset + uNextCookieOffset;
                    }

                    break;
                }

                uPrevEntrySize  = uEntrySize;
                *bytesRead      += uEntrySize;
                pvEntry         = puBuf;
                puBuf           += uEntrySize;

                RecordIdentifier_e eRecId = DIROPS_GetRecordId( &sDirEntryEntriesData.sDosDirEntry, psFolderNode );
                if ( eRecId == RECORD_IDENTIFIER_UNKNOWN )
                {
                    MSDOS_LOG(LEVEL_ERROR, "MSDOS_ReadDirInternal RECORD_IDENTIFIER_UNKNOWN.\n");
                    iErr = EIO;
                    goto exit;
                }

                if ( !bIsReadAttr )
                {
                    UVFSDirEntry* psDirEntry    = (UVFSDirEntry*)pvEntry;
                    psDirEntry->de_filetype     = puRecordId2FaType[eRecId];
                    psDirEntry->de_fileid       = DIROPS_GetStartCluster( GET_FSRECORD(psFolderNode), &sDirEntryEntriesData.sDosDirEntry );
                    if (psDirEntry->de_fileid == 0) {
                        /* Special place-holder ID for empty files. */
                        psDirEntry->de_fileid   = FILENO_EMPTY;
                    }
                    psDirEntry->de_namelen      = strlen( puUTF8 );
                    psDirEntry->de_nextrec      = uEntrySize;
                    psDirEntry->de_nextcookie   = uNextEnteryOffset + uNextCookieOffset;
                    memcpy( psDirEntry->de_name, puUTF8, psDirEntry->de_namelen );
                    psDirEntry->de_name[psDirEntry->de_namelen] = 0;
                }
                else
                {
                    UVFSDirEntryAttr* psDirEntry = (UVFSDirEntryAttr*)pvEntry;
                    NodeRecord_s* psTmpNodeRecord;
                    // Allocate record - can't  be '.' or '..' since we are skipping them
                    iErr = FILERECORD_AllocateRecord( &psTmpNodeRecord, GET_FSRECORD(psFolderNode),
                                                     DIROPS_GetStartCluster( GET_FSRECORD(psFolderNode), &sDirEntryEntriesData.sDosDirEntry ), eRecId, &sDirEntryEntriesData, NULL);

                    if ( iErr != 0 )
                    {
                        goto exit;
                    }

                    MSDOS_GetAtrrFromDirEntry( psTmpNodeRecord, &psDirEntry->dea_attrs );
                    FILERECORD_FreeRecord(psTmpNodeRecord);

                    psDirEntry->dea_nextcookie   = uNextEnteryOffset + uNextCookieOffset;
                    psDirEntry->dea_nextrec      = uEntrySize;
                    psDirEntry->dea_namelen      = strlen( puUTF8 );
                    psDirEntry->dea_spare0       = 0;
                    psDirEntry->dea_nameoff      = UVFS_DIRENTRYATTR_NAMEOFF;
                    memcpy( UVFS_DIRENTRYATTR_NAMEPTR(psDirEntry), puUTF8, psDirEntry->dea_namelen );
                    UVFS_DIRENTRYATTR_NAMEPTR(psDirEntry)[psDirEntry->dea_namelen] = 0;
                }

                uTotalDirEntries++;
                bAddAtLeastOneEntry = true;
            }
        }
        else if ( eStatus == DIR_ENTRY_EMPTY || eStatus == DIR_ENTRY_EOD )
        {
            if ( !bIsReadAttr )
            {
                UVFSDirEntry* psDirEntry;
                // In case of empty directory.. need to point back to the last valid entry...
                if ( bSynthDots && !bAddAtLeastOneEntry )
                {
                    psDirEntry = (UVFSDirEntry*)((uint8_t*)pvEntry - uPrevEntrySize);
                }
                else
                {
                    psDirEntry = (UVFSDirEntry*)pvEntry;
                }

                psDirEntry->de_nextrec      = 0;
                psDirEntry->de_nextcookie   = UVFS_DIRCOOKIE_EOF;
            }
            else
            {
                UVFSDirEntryAttr* psDirEntry = (UVFSDirEntryAttr*)pvEntry;
                psDirEntry->dea_nextrec      = 0;
                psDirEntry->dea_nextcookie   = UVFS_DIRCOOKIE_EOF;
            }

            //If the dir is empty, need to return UVFS_READDIR_EOF_REACHED;
            if (uTotalDirEntries == 0)
                iErr = UVFS_READDIR_EOF_REACHED;

            break;
        }
        else if  ( eStatus == DIR_ENTRY_ERROR )
        {
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_ReadDirInternal:DIROPS_GetDirEntryByOffset error: %d.\n", iErr);
            break;
        }

        uLastEntryOffset = uNextEnteryOffset;

    } while ( eStatus != DIR_ENTRY_EMPTY && eStatus != DIR_ENTRY_ERROR );

    /* if we have no entries and it's not eof, something is wrong */
    if ( (uTotalDirEntries == 0) && (iErr != UVFS_READDIR_EOF_REACHED) )
    {
        iErr = EINVAL;
        goto exit;
    }
    if ( eStatus == DIR_ENTRY_ERROR )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ReadDirInternal DIR_ENTRY_ERROR.\n");
        iErr = EIO;
        goto exit;
    }
    
exit:
    MultiReadSingleWrite_FreeRead( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );
    MultiReadSingleWrite_FreeRead( &psFolderNode->sRecordData.sRecordLck );
    if ( psName != NULL )
        free( psName );
    if ( puUTF8 != NULL )
        free(puUTF8);

    //Getting UVFS_READDIR_EOF_REACHED here, doesn't require force evict
    bool bReleaseHT = ((iErr != 0) && (iErr != UVFS_READDIR_EOF_REACHED));
    DIROPS_ReleaseHTForDirectory(psFolderNode, bReleaseHT);

    return iErr;
}

int
MSDOS_ReadDir(UVFSFileNode dirNode, void* pvBuf, size_t uBufLen, uint64_t uCookie, size_t *bytesRead, uint64_t *puVerifier)
{
    VERIFY_NODE_IS_VALID(dirNode);
    return DIROPS_ReadDirInternal(dirNode, pvBuf, uBufLen, uCookie, bytesRead, puVerifier, false);
}

int
MSDOS_ReadDirAttr(UVFSFileNode dirNode, void* pvBuf, size_t uBufLen, uint64_t uCookie, size_t *bytesRead, uint64_t *puVerifier)
{
    VERIFY_NODE_IS_VALID(dirNode);
    return DIROPS_ReadDirInternal(dirNode, pvBuf, uBufLen, uCookie, bytesRead, puVerifier, true);
}

int
DIROPS_GetDirCluster(NodeRecord_s* psFolderNode, uint32_t uWantedClusterOffsetInChain, ClusterData_s* psClusterData)
{
    FileSystemRecord_s *psFSRecord = GET_FSRECORD(psFolderNode);
    uint32_t uClusterSize  = 0;
    uint64_t uOffsetInFile = 0;
    bool bIsFat12Or16Root = false;
    int iError = 0;
    
    //Already allocated and valid - no need to reread again
    if ( !((!psClusterData->bIsAllocated) || (!psClusterData->bIsValid) ||
        (psClusterData->uClusterNum != uWantedClusterOffsetInChain ) ||
        (psClusterData->uLocalDirHTGeneration < psFSRecord->uDirHTGeneration) ))
    {
        return 0;
    }
    
    // In case of FAT12/16 Root Dir can't be extended
    if ( IS_FAT_12_16_ROOT_DIR(psFolderNode) )
    {
        if ( uWantedClusterOffsetInChain != 0 )
        {
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_GetDirCluster fat12/16 root dir can't be extended. uWantedClusterOffsetInChain = [%u]\n", uWantedClusterOffsetInChain);
            return DIR_BAD_CLUSTER;
        }
        uClusterSize = psFSRecord->sRootInfo.uRootLength;
        uOffsetInFile = 0;
        bIsFat12Or16Root = true;
    }
    else
    {
        if ( uWantedClusterOffsetInChain > psFolderNode->sRecordData.uClusterChainLength )
        {
            return DIR_BAD_CLUSTER;
        }
        uClusterSize = CLUSTER_SIZE(psFSRecord);
        uOffsetInFile = uClusterSize*uWantedClusterOffsetInChain;
    }
    
    psClusterData->bIsValid = false;
    
    if ( psClusterData->bIsAllocated )
    {
        free( psClusterData->puClusterData );
        psClusterData->bIsAllocated = false;
    }
    
    psClusterData->puClusterData = malloc(uClusterSize);
    if ( psClusterData->puClusterData == NULL )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_GetDirCluster failed to allocate memory.\n");
        return ENOMEM;
    }
    
    psClusterData->bIsAllocated = true;
    psClusterData->uClusterNum  = uWantedClusterOffsetInChain;
    psClusterData->uLength      = uClusterSize;

    //Update uDirHTGeneration
    psClusterData->uLocalDirHTGeneration = psFSRecord->uDirHTGeneration;
    
    uint32_t uWantedCluster, uContiguousClusterLength;
    FILERECORD_GetChainFromCache( psFolderNode, uOffsetInFile, &uWantedCluster, &uContiguousClusterLength,&iError);
    if (iError)
        return iError;

    if (uContiguousClusterLength == 0)
        return EINVAL;

    psClusterData->uAbsoluteClusterNum      = uWantedCluster;
    psClusterData->uAbsoluteClusterOffset   = bIsFat12Or16Root?
                                                psFSRecord->sRootInfo.uRootSector*psFSRecord->sFSInfo.uBytesPerSector :
                                                DIROPS_VolumeOffsetForCluster(psFSRecord,uWantedCluster);
    
    //  Read the Directory cluster from the memory.
    ssize_t readBytes = pread(psFSRecord->iFD, psClusterData->puClusterData, uClusterSize, psClusterData->uAbsoluteClusterOffset);
    if ( readBytes != uClusterSize )
    {
        iError = errno;
        free(psClusterData->puClusterData);
        psClusterData->puClusterData = NULL;
        psClusterData->bIsAllocated = false;
        psClusterData->uLength      = 0;
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_GetDirCluster failed to read errno = [%d].\n", iError);
    }
    
    if (!iError) psClusterData->bIsValid = true;
    
    return iError;
}

static int
DIROPS_VerifyCookieAndVerifier( NodeRecord_s* psFolderNode, uint64_t uCookie, ClusterData_s* psClusterData, bool bIsReadAttr, uint64_t uVerifier )
{
    FileSystemRecord_s *psFSRecord  = GET_FSRECORD(psFolderNode);
    uint32_t uClusterSize           = 0;
    uint64_t uCurrentEntryOffset    = 0;
    uint32_t uRelCurrentEntryOffset = 0;
    uint32_t uCurrentCluster        = 0;
    struct dosdirentry *psEntry     = NULL;
    struct winentry *psLongEntry    = NULL;
    int iErr                        = 0;

    if ( uCookie == 0 )
    {
        if ( uVerifier != UVFS_DIRCOOKIE_VERIFIER_INITIAL )
        {
            iErr =  UVFS_READDIR_VERIFIER_MISMATCHED;
            goto exit;
        }
    }
    else if ( uCookie == UVFS_DIRCOOKIE_EOF )
    {
        iErr = UVFS_READDIR_EOF_REACHED;
        goto exit;
    }
    else if ( uVerifier != psFolderNode->sExtraData.sDirData.uDirVersion )
    {
        iErr = UVFS_READDIR_VERIFIER_MISMATCHED;
        goto exit;
    }
    
    if ( IS_ROOT(psFolderNode) && !bIsReadAttr )
    {
        if (( uCookie == DOT_DIR_COOKIE ) || ( uCookie == DOT_X2_COOKIE  ))
        {
            return iErr;
        }
        else
        {
            uCookie -= SYNTHESIZE_ROOT_DOTS_SIZE;
        }
    }
    
    if ( (uCookie % sizeof(struct dosdirentry)) != 0 )
    {
        return UVFS_READDIR_BAD_COOKIE;
    }
    
    // In case of FAT12/16 Root Dir can't be extended
    if ( IS_FAT_12_16_ROOT_DIR( psFolderNode ) )
    {
        uClusterSize = psFSRecord->sRootInfo.uRootLength;
    }
    else
    {
        uClusterSize = CLUSTER_SIZE(psFSRecord);
    }
    
    uCurrentEntryOffset     = uCookie;
    uRelCurrentEntryOffset  = uCookie % uClusterSize;
    uCurrentCluster         = (uint32_t)(uCurrentEntryOffset / uClusterSize);
    
    iErr = DIROPS_GetDirCluster( psFolderNode, uCurrentCluster, psClusterData );
    if ( iErr != 0 )
    {
        if ( iErr == DIR_BAD_CLUSTER )
        {
            return UVFS_READDIR_BAD_COOKIE;
        }
        else
        {
            return iErr;
        }
    }
    
    psEntry = (struct dosdirentry *)(&psClusterData->puClusterData[uRelCurrentEntryOffset]);
    if ( psEntry->deName[0] == SLOT_EMPTY )
    {
        return UVFS_READDIR_BAD_COOKIE;
    }

    // Long name attr.
    psLongEntry = (struct winentry*)psEntry;
    if ( (psEntry->deName[0] != SLOT_DELETED) && ((psEntry->deAttributes & ATTR_WIN95_MASK) == ATTR_WIN95) && (!(psLongEntry->weCnt & WIN_LAST)) )
    {
        return UVFS_READDIR_BAD_COOKIE;
    }

exit:
    return iErr;
}

static DirEntryStatus_e
DIROPS_GetDirEntryByOffset( NodeRecord_s* psFolderNode, ClusterData_s* psClusterData, uint64_t uEntryOffset,
                           uint64_t* puDosEntryOffset, uint64_t* puNextEnteryOffset, NodeDirEntriesData_s* psNodeDirEntriesData, struct unistr255* psName, uint32_t *uError)
{
    FileSystemRecord_s *psFSRecord = GET_FSRECORD(psFolderNode);
    uint32_t uClusterSize;
    
    // In case of FAT12/16 Root Dir can't be extended
    if ( IS_FAT_12_16_ROOT_DIR( psFolderNode ) )
    {
        uClusterSize = psFSRecord->sRootInfo.uRootLength;
    }
    else
    {
        uClusterSize = CLUSTER_SIZE(psFSRecord);
    }

    uint64_t uCurrentEntryOffset = uEntryOffset;
    psNodeDirEntriesData->uFirstEntryOffset = uEntryOffset; //Save entry offset for hash table
    uint32_t uRelCurrentEntryOffset = (uint32_t) (uEntryOffset % uClusterSize);
    uint32_t uCurrentCluster = (uint32_t)(uCurrentEntryOffset / uClusterSize);
    
    //In case that the last directory cluster was full, and the last one.
    if ( psFolderNode->sRecordData.uClusterChainLength == uCurrentCluster )
    {
        return DIR_ENTRY_EOD;
    }
    
    errno_t err = DIROPS_GetDirCluster( psFolderNode, uCurrentCluster, psClusterData );
    if ( err != 0 )
    {
        if (uError) *uError = err;
        return DIR_ENTRY_ERROR;
    }
    
    struct dosdirentry *psEntry = (struct dosdirentry *) &(psClusterData->puClusterData[uRelCurrentEntryOffset]);
    
    if ( psEntry->deName[0] == SLOT_EMPTY )
    {
        *puNextEnteryOffset = -1;
        return DIR_ENTRY_EMPTY;
    }
    
    if ( psEntry->deName[0] == SLOT_DELETED )
    {
        *puNextEnteryOffset = uCurrentEntryOffset + sizeof(struct dosdirentry);
        return DIR_ENTRY_DELETED;
    }
    
    // If it's not a long name entry
    if ( (psEntry->deAttributes & ATTR_WIN95_MASK) != ATTR_WIN95 )
    {
        // if we get here, we must have found a short name entry or volume name entries.
        {
            *puNextEnteryOffset = uCurrentEntryOffset + sizeof(struct dosdirentry);
            *puDosEntryOffset = uCurrentEntryOffset;
            psName->length = msdosfs_dos2unicodefn((u_char*)psEntry->deName, psName->chars, psEntry->deLowerCase);
            
            psNodeDirEntriesData->sDosDirEntry                  = *psEntry;
            psNodeDirEntriesData->uClusterSize                  = uClusterSize;
            psNodeDirEntriesData->uFirstClusterIdxInChain       = uCurrentCluster;
            psNodeDirEntriesData->uRelFirstEntryOffset          = uRelCurrentEntryOffset;
            psNodeDirEntriesData->uLastClusterAbsoluteOffset    = psClusterData->uAbsoluteClusterOffset;
            psNodeDirEntriesData->uLastEntryOffsetInCluster     = uRelCurrentEntryOffset;
            psNodeDirEntriesData->uNumberOfDirEntriesForNode    = 1;
            
            return (psEntry->deAttributes & ATTR_VOLUME)? DIR_ENTRY_VOL_NAME : DIR_ENTRY_FOUND;
        }
    }
    
    // if we get here, we must have found a long name entry
    {
        psNodeDirEntriesData->uClusterSize                  = uClusterSize;
        psNodeDirEntriesData->uFirstClusterIdxInChain       = uCurrentCluster;
        psNodeDirEntriesData->uRelFirstEntryOffset          = uRelCurrentEntryOffset;
        
        struct winentry* psLongEntry = (struct winentry*)psEntry;
        if ( !(psLongEntry->weCnt & WIN_LAST) )
        {
            *puNextEnteryOffset = uCurrentEntryOffset + sizeof(struct dosdirentry);
            return DIR_ENTRY_UNKNOWN;
        }
        
        int iLongNameChecksum   = psLongEntry->weChksum;
        uint32_t uLongNameIndex = psLongEntry->weCnt & WIN_CNT;
        
        //Include the short name enrty
        psNodeDirEntriesData->uNumberOfDirEntriesForNode = uLongNameIndex + 1;

        psName->length = uLongNameIndex * WIN_CHARS;
        for (uint32_t uCurNameIdx = uLongNameIndex; uCurNameIdx > 0; uCurNameIdx--)
        {
            psLongEntry = (struct winentry *)(&psClusterData->puClusterData[uRelCurrentEntryOffset]);
            if ( iLongNameChecksum != psLongEntry->weChksum )
            {
                *puNextEnteryOffset = uCurrentEntryOffset + sizeof(struct dosdirentry);
                return DIR_ENTRY_UNKNOWN;
            }
            
            uint32_t uUnicodeIndex = (uCurNameIdx - 1) * WIN_CHARS;
            for ( uint32_t uUTF16Counter = 0; uUTF16Counter<WIN_CHARS; ++uUTF16Counter )
            {
                uint8_t* puCp = ((uint8_t*)psLongEntry) + puLongNameOffset[uUTF16Counter];
                uint16_t uCh = getuint16(puCp);
                
                if ( uCh == 0 )
                {
                    if ( uCurNameIdx != uLongNameIndex )
                    {
                        *puNextEnteryOffset = uCurrentEntryOffset + sizeof(struct dosdirentry);
                        return DIR_ENTRY_UNKNOWN;
                    }
                    
                    psName->length = uUTF16Counter + uUnicodeIndex;
                    break;
                }
                else
                {
                    if ( uUnicodeIndex + uUTF16Counter > WIN_MAXLEN )
                    {
                        *puNextEnteryOffset = uCurrentEntryOffset + sizeof(struct dosdirentry);
                        return DIR_ENTRY_UNKNOWN;
                    }
                    
                    psName->chars[uUnicodeIndex + uUTF16Counter] = uCh;
                }
            }
            
            uCurrentEntryOffset     += sizeof(struct dosdirentry);
            uRelCurrentEntryOffset  += sizeof(struct dosdirentry);
            
            // Check if we need the next cluster
            if ( uRelCurrentEntryOffset % uClusterSize == 0 )
            {
                uRelCurrentEntryOffset = 0;
                uCurrentCluster++;

                //In case that the last directory cluster was full, and the last one.
                if ( psFolderNode->sRecordData.uClusterChainLength == uCurrentCluster )
                {
                    return DIR_ENTRY_EOD;
                }

                errno_t err = DIROPS_GetDirCluster( psFolderNode, uCurrentCluster, psClusterData );
                if ( err != 0 )
                {
                    if (uError) *uError = err;
                    return DIR_ENTRY_ERROR;
                }
            }
        }
        
        psEntry = (struct dosdirentry *)(&psClusterData->puClusterData[uRelCurrentEntryOffset]);
        
        *puNextEnteryOffset = uCurrentEntryOffset + sizeof(struct dosdirentry);
        *puDosEntryOffset = uCurrentEntryOffset;
        
        psNodeDirEntriesData->sDosDirEntry                  = *psEntry;
        psNodeDirEntriesData->uLastClusterAbsoluteOffset    = psClusterData->uAbsoluteClusterOffset;
        psNodeDirEntriesData->uLastEntryOffsetInCluster     = uRelCurrentEntryOffset;
        
        return DIR_ENTRY_FOUND;
    }

    assert(0);
    return DIR_ENTRY_UNKNOWN;
}

int
DIROPS_MarkNodeDirEntriesAsDeleted( NodeRecord_s* psFolderNode, NodeDirEntriesData_s* psNodeDirEntriesData, const char *pcUTF8Name)
{
    errno_t err = 0;
    uint32_t uClusterSize = psNodeDirEntriesData->uClusterSize;
    
    uint32_t uCurCacheCluster = psNodeDirEntriesData->uFirstClusterIdxInChain;
    err = DIROPS_GetDirCluster( psFolderNode, uCurCacheCluster, psFolderNode->sExtraData.sDirData.psDirClusterData );
    if ( err != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_MarkNodeDirEntriesAsDeleted fail to get dir cluster\n");
        goto exit;
    }
    
    psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = false;
    
    for ( uint32_t uIdx=0, uCurOffset = psNodeDirEntriesData->uRelFirstEntryOffset;
         uIdx<psNodeDirEntriesData->uNumberOfDirEntriesForNode;
         uIdx++ )
    {
        
        psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData[ uCurOffset % uClusterSize ] = SLOT_DELETED;
        
        uCurOffset += sizeof(struct dosdirentry);
        if ( (( uCurOffset % uClusterSize ) == 0) || (uIdx == psNodeDirEntriesData->uNumberOfDirEntriesForNode-1) )
        {
            // Flush the cluster data.
            ssize_t uSize = pwrite( GET_FSRECORD(psFolderNode)->iFD, psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData, uClusterSize, psFolderNode->sExtraData.sDirData.psDirClusterData->uAbsoluteClusterOffset );
            if ( uSize != uClusterSize )
            {
                err = errno;
                MSDOS_LOG(LEVEL_ERROR, "DIROPS_MarkNodeDirEntriesAsDeleted fail to write errno = [%d]\n", err);
                goto exit;
            }
            
            //After writing all is good
            psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = true;

            if ( (( uCurOffset % uClusterSize ) == 0) && (uIdx != psNodeDirEntriesData->uNumberOfDirEntriesForNode-1) )
            {
                uCurCacheCluster++;
                err = DIROPS_GetDirCluster( psFolderNode, uCurCacheCluster, psFolderNode->sExtraData.sDirData.psDirClusterData );
                if ( err != 0 )
                {
                    MSDOS_LOG(LEVEL_ERROR, "DIROPS_MarkNodeDirEntriesAsDeleted fail to get dir cluster\n");
                    goto exit;
                }
                
                //We changed cluster, so we are not valid again
                psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = false;
            }
        }
    }

#ifdef MSDOS_NLINK_IS_CHILD_COUNT
    //Update the parent child dir count
    psFolderNode->sExtraData.sDirData.uChildCount--;
#endif

    err = ht_remove(psFolderNode->sExtraData.sDirData.sHT, pcUTF8Name, psNodeDirEntriesData->uFirstEntryOffset);
    //check if we reached to lower bound
    if (ht_reached_low_bound(psFolderNode->sExtraData.sDirData.sHT))
    {
        //reset Incmoplete and populate the HT
        psFolderNode->sExtraData.sDirData.sHT->bIncomplete = false;
        err = DIROPS_PopulateHT(psFolderNode, psFolderNode->sExtraData.sDirData.sHT, true);
    }
    
    DIROPS_UpdateDirLastModifiedTime( psFolderNode );

exit:
    return err;
}

int
DIROPS_isDirEmpty( NodeRecord_s* psFolderNode )
{
    uint32_t err                 = 0;
    uint64_t uEntryOffset       = 0;
    uint64_t uDosEntryOffset    = 0;
    uint64_t uNextEnteryOffset  = 0;
    
    NodeDirEntriesData_s sNodeDirEntriesData;
    memset( &sNodeDirEntriesData,   0, sizeof(NodeDirEntriesData_s) );

    struct unistr255* psName = (struct unistr255*)malloc(sizeof(struct unistr255));
    if ( psName == NULL )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_isDirEmpty fail to allocate memory.\n");
        err = ENOMEM;
        goto exit;
    }
    memset( psName,                 0, sizeof(struct unistr255)     );
    
    if ( !IS_DIR(psFolderNode) )
    {
        err = ENOTDIR;
        goto exit;
    }
    
    DirEntryStatus_e eStatus = DIR_ENTRY_START;
    do {
        eStatus = DIROPS_GetDirEntryByOffset( psFolderNode, psFolderNode->sExtraData.sDirData.psDirClusterData, uEntryOffset, &uDosEntryOffset, &uNextEnteryOffset, &sNodeDirEntriesData, psName, &err );
        if ( DIR_ENTRY_FOUND == eStatus )
        {
            if ( !((( psName->length == 1 ) && ( psName->chars[0] == 46 )) ||
                   (( psName->length == 2 ) && ( psName->chars[0] == 46 ) && (psName->chars[1] == 46) )) )
            {
                err = ENOTEMPTY;
                goto exit;
            }
        }
        else if (eStatus == DIR_ENTRY_ERROR)
        {
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_isDirEmpty:DIROPS_GetDirEntryByOffset error: %d.\n", err);
            goto exit;
        }
        uEntryOffset = uNextEnteryOffset;
        
    } while ( eStatus != DIR_ENTRY_EMPTY && eStatus != DIR_ENTRY_ERROR && eStatus != DIR_ENTRY_EOD );
    
    
    if ( eStatus != DIR_ENTRY_EMPTY && eStatus != DIR_ENTRY_EOD )
    {
        err = 1;
        goto exit;
    }

exit:
    if ( psName != NULL )
        free( psName );

    return err;
}

#ifdef MSDOS_NLINK_IS_CHILD_COUNT
int
DIROPS_CountChildrenInADirectory( NodeRecord_s* psFolderNode)
{
    int iError = 0;
    uint64_t uEntryOffset       = 0;
    uint64_t uDosEntryOffset    = 0;
    uint64_t uNextEnteryOffset  = 0;
    NodeDirEntriesData_s sNodeDirEntriesData;

    struct unistr255* psName   = (struct unistr255*)malloc(sizeof(struct unistr255));

    //Initialize counter
    psFolderNode->sExtraData.sDirData.uChildCount = 0;

    DirEntryStatus_e eStatus = DIR_ENTRY_START;
    do {
        eStatus = DIROPS_GetDirEntryByOffset( psFolderNode, psFolderNode->sExtraData.sDirData.psDirClusterData, uEntryOffset, &uDosEntryOffset, &uNextEnteryOffset, &sNodeDirEntriesData, psName, &iError );
        if ( DIR_ENTRY_FOUND == eStatus )
        {
            (psFolderNode->sExtraData.sDirData.uChildCount)++;
        }
        else if ( eStatus == DIR_ENTRY_ERROR )
        {
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_CountChildrenInADirectory:DIROPS_GetDirEntryByOffset error %d.\n", iError);
            goto exit;
        }

        uEntryOffset = uNextEnteryOffset;

    } while ( eStatus != DIR_ENTRY_EMPTY && eStatus != DIR_ENTRY_ERROR && eStatus != DIR_ENTRY_EOD );

    if (IS_ROOT( psFolderNode ))
    {
        (psFolderNode->sExtraData.sDirData.uChildCount)+=2;
    }

exit:
    if ( psName != NULL )
        free( psName );

    return iError;
}
#endif /* MSDOS_NLINK_IS_CHILD_COUNT */

// N.B. psFolderNode must be write-locked.
// UPON RETURN, psFolderNode WILL BE UNLOCKED.
// Returns the node evicted from the cache, if it was not busy and we were able
// to acquire the write-lock.
static NodeRecord_s *
DIROPS_AcquireHTLRUSlotAndUnlockNode(NodeRecord_s* psFolderNode, const struct timespec *psTimeStamp)
{
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psFolderNode);
    NodeRecord_s* psEvictedFolderNode = NULL;
    bool bLocked = true;

    MultiReadSingleWrite_LockWrite(&psFSRecord->sDirHTLRUTableLock);

    // Check to see if we still own a slot.  If we do, just refresh the LRU
    // time.
    if (psFolderNode->sExtraData.sDirData.psDirHTLRUSlot &&
        psFolderNode->sExtraData.sDirData.psDirHTLRUSlot->psOwningNode == (void *)psFolderNode)
    {
        psFolderNode->sExtraData.sDirData.psDirHTLRUSlot->sLastUsed = *psTimeStamp;
        psFolderNode->sExtraData.sDirData.psDirHTLRUSlot->bBusy = true;
        goto out;
    }

    // Need to find one.  Scan the slots, and evict the least recently used
    // directory.
    DirHTState_s *psDirHTState, *psDirHTStateLRU = NULL;
    int i;
    for (i = 0; i < DIR_LRU_TABLE_MAX; i++)
    {
        psDirHTState = &psFSRecord->sDirHTLRUTable[i];

        // If this slot is not owned, take it!
        if (psDirHTState->psOwningNode == NULL)
        {
            psDirHTStateLRU = psDirHTState;
            break;
        }

        // First time through?  Nothing to compare.
        if (psDirHTStateLRU == NULL)
        {
            psDirHTStateLRU = psDirHTState;
            continue;
        }

        if (psDirHTState->sLastUsed.tv_sec < psDirHTStateLRU->sLastUsed.tv_sec)
        {
            psDirHTStateLRU = psDirHTState;
        }
        else if (psDirHTState->sLastUsed.tv_sec == psDirHTStateLRU->sLastUsed.tv_sec &&
                 psDirHTState->sLastUsed.tv_nsec < psDirHTStateLRU->sLastUsed.tv_nsec)
        {
            psDirHTStateLRU = psDirHTState;
        }
    }

    // We now own the slot.  Any previous user will notice when they are done
    // using their cached directory information, and release it at that time.
    //
    // If the slot had an owner that is not currently busy, return it to the
    // caller so that it's cache resources can be freed.  If it is busy, it
    // will notice that it's been evicted and clean up on its own.
    if (psDirHTStateLRU->bBusy == false) {
        psEvictedFolderNode = psDirHTStateLRU->psOwningNode;
    }
    psDirHTStateLRU->psOwningNode = psFolderNode;
    psDirHTStateLRU->sLastUsed = *psTimeStamp;
    psDirHTStateLRU->bBusy = true;
    psFolderNode->sExtraData.sDirData.psDirHTLRUSlot = psDirHTStateLRU;

    // We are done manipulating psFolderNode, so we can unlock it now.
    // This is also important for the code block below.
    MultiReadSingleWrite_FreeWrite(&psFolderNode->sRecordData.sRecordLck);
    bLocked = false;

    //
    // N.B. WE HAVE TO DO A DELICATE LITTLE LOCKING DANCE HERE.
    // In order to safely return the evicted node, we have to lock it,
    // otherwise it could be freed out from under us.  But in order to safely
    // lock it, we have to hold the sDirHTLRUTableLock.  THIS VIOLATES THE
    // LOCKING ORDER.
    //
    // We are forced to use a try-lock here to avoid a deadlock situation.
    // However, all is not lost!  If we fail to lock the evicted folder,
    // chances are that either the directory is going to be used (which would
    // necessitate re-creating the HT anyway), or is being reclaimed (which
    // will free it anyway).
    //
    // In any case, we will log a message so we can see if this ever happens.
    //
    if (psEvictedFolderNode != NULL &&
        MultiReadSingleWrite_TryLockWrite(&psEvictedFolderNode->sRecordData.sRecordLck) == false)
    {
        MSDOS_LOG(LEVEL_ERROR, "Failed to lock LRU-Evicted Directory Node - hope it cleans up!");
        psEvictedFolderNode = NULL;
    }

out:
    MultiReadSingleWrite_FreeWrite(&psFSRecord->sDirHTLRUTableLock);
    if (bLocked)
        MultiReadSingleWrite_FreeWrite(&psFolderNode->sRecordData.sRecordLck);
    return psEvictedFolderNode;
}

// N.B. psFolderNode must be write-locked.
static void
DIROPS_ReleaseHTLRUSlot(NodeRecord_s* psFolderNode)
{
    if (psFolderNode->sExtraData.sDirData.psDirHTLRUSlot == NULL)
        return;

    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psFolderNode);
    MultiReadSingleWrite_LockWrite(&psFSRecord->sDirHTLRUTableLock);
    if (psFolderNode->sExtraData.sDirData.psDirHTLRUSlot->psOwningNode == (void *)psFolderNode)
        psFolderNode->sExtraData.sDirData.psDirHTLRUSlot->psOwningNode = NULL;
    MultiReadSingleWrite_FreeWrite(&psFSRecord->sDirHTLRUTableLock);
    psFolderNode->sExtraData.sDirData.psDirHTLRUSlot = NULL;
}

// N.B. psFolderNode must be write-locked.
static bool
DIROPS_WasEvictedFromHTLRUSlot(NodeRecord_s* psFolderNode, unsigned int uBusyCount)
{
    if (psFolderNode->sExtraData.sDirData.psDirHTLRUSlot == NULL)
        return true;

    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psFolderNode);
    MultiReadSingleWrite_LockWrite(&psFSRecord->sDirHTLRUTableLock);
    bool rv = psFolderNode->sExtraData.sDirData.psDirHTLRUSlot->psOwningNode != (void *)psFolderNode;
    if (rv == false && uBusyCount == 0)
        psFolderNode->sExtraData.sDirData.psDirHTLRUSlot->bBusy = false;
    MultiReadSingleWrite_FreeWrite(&psFSRecord->sDirHTLRUTableLock);

    return rv;
}

static int
DIROPS_PopulateHT(NodeRecord_s* psFolderNode, LF_HashTable_t* psHT, bool bAllowExistence)
{
    uint32_t iError = 0;
    uint64_t uEntryOffset       = 0;
    uint64_t uDosEntryOffset    = 0;
    uint64_t uNextEnteryOffset  = 0;
    NodeDirEntriesData_s sNodeDirEntriesData;
    struct unistr255* psName   = (struct unistr255*)malloc(sizeof(struct unistr255));
    if ( psName == NULL )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_PopulateHT: Failed to allocate memory");
        iError = ENOMEM;
        goto exit;
    }
    memset( psName, 0, sizeof(struct unistr255) );

    //Initialize counter
    DirEntryStatus_e eStatus = DIR_ENTRY_START;
    do {
        eStatus = DIROPS_GetDirEntryByOffset( psFolderNode, psFolderNode->sExtraData.sDirData.psDirClusterData, uEntryOffset, &uDosEntryOffset, &uNextEnteryOffset, &sNodeDirEntriesData, psName, &iError );
        if ( DIR_ENTRY_FOUND == eStatus )
        {
            iError = ht_insert(psHT, psName, uEntryOffset, false);
            if (bAllowExistence && iError == EEXIST) iError = 0;
            if (iError)
            {
                goto exit;
            }
        }
        else if ( eStatus == DIR_ENTRY_ERROR )
        {
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_PopulateHT:DIROPS_GetDirEntryByOffset fail: %d .\n", iError);
            goto exit;
        }

        uEntryOffset = uNextEnteryOffset;

    } while ( eStatus != DIR_ENTRY_EMPTY && eStatus != DIR_ENTRY_ERROR && eStatus != DIR_ENTRY_EOD );



exit:
    if ( psName != NULL )
        free( psName );

    return iError;
}

// N.B. psFolderNode must be write-locked.
// UPON RETURN, THE NODE WILL BE UNLOCKED.
static void
DIROPS_MaybeFreeEvictedHTAndUnlockNode(NodeRecord_s* psFolderNode)
{
    LF_HashTable_t* psHT = NULL;
    uint8_t* puClusterData = NULL;

    if (DIROPS_WasEvictedFromHTLRUSlot(psFolderNode, psFolderNode->sExtraData.sDirData.uHTBusyCount) &&
        psFolderNode->sExtraData.sDirData.uHTBusyCount == 0)
    {
        psFolderNode->sExtraData.sDirData.psDirHTLRUSlot = NULL;

        psHT = psFolderNode->sExtraData.sDirData.sHT;
        psFolderNode->sExtraData.sDirData.sHT = NULL;

        puClusterData = psFolderNode->sExtraData.sDirData.psDirClusterData->bIsAllocated ?
        psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData :
        NULL;
        psFolderNode->sExtraData.sDirData.psDirClusterData->bIsAllocated = false;
        psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData = NULL;
        psFolderNode->sExtraData.sDirData.psDirClusterData->uLocalDirHTGeneration = 0;
        
        psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = false;
    }

    MultiReadSingleWrite_FreeWrite(&psFolderNode->sRecordData.sRecordLck);

    if (psHT != NULL)
        ht_DeAllocateHashTable(psHT);
    if (puClusterData != NULL)
        free(puClusterData);
}

int
DIROPS_CreateHTForDirectory( NodeRecord_s* psFolderNode)
{
    int iError                 = 0;
    bool bLocked               = false;
    struct timespec sTimeStamp = { 0 };
    NodeRecord_s* psEvictedFolderNode = NULL;

    if (!IS_DIR(psFolderNode))
    {
        return (ENOTDIR);
    }

    clock_gettime(CLOCK_MONOTONIC, &sTimeStamp);

    MultiReadSingleWrite_LockWrite(&psFolderNode->sRecordData.sRecordLck);

    // If the table exists, get out now.
    if (psFolderNode->sExtraData.sDirData.sHT != NULL)
    {
        psFolderNode->sExtraData.sDirData.uHTBusyCount++;
        psEvictedFolderNode = DIROPS_AcquireHTLRUSlotAndUnlockNode(psFolderNode, &sTimeStamp);
        if (psEvictedFolderNode)
            DIROPS_MaybeFreeEvictedHTAndUnlockNode(psEvictedFolderNode);
        return 0;
    }

    MultiReadSingleWrite_FreeWrite(&psFolderNode->sRecordData.sRecordLck);

    LF_HashTable_t* psHT = NULL;
    iError = ht_AllocateHashTable(&psHT);
    
    if (iError != 0)
        goto exit;

    MultiReadSingleWrite_LockWrite(&psFolderNode->sRecordData.sRecordLck);
    bLocked = true;

    if (psFolderNode->sExtraData.sDirData.sHT != NULL)
    {
        // We raced with somebody else while allocating the table and lost.
        goto exit;
    }

    iError = DIROPS_PopulateHT(psFolderNode, psHT, false);

    psFolderNode->sExtraData.sDirData.sHT = psHT;
    psHT = NULL;

    psFolderNode->sExtraData.sDirData.uHTBusyCount++;
    psEvictedFolderNode = DIROPS_AcquireHTLRUSlotAndUnlockNode(psFolderNode, &sTimeStamp);
    bLocked = false;

exit:
    if (bLocked)
        MultiReadSingleWrite_FreeWrite(&psFolderNode->sRecordData.sRecordLck);
    if (psEvictedFolderNode)
        DIROPS_MaybeFreeEvictedHTAndUnlockNode(psEvictedFolderNode);
    if ( psHT != NULL )
        ht_DeAllocateHashTable( psHT );

    return iError;
}

static void
DIROPS_CleanRefereaceForHTForDirectory(NodeRecord_s* psFolderNode, LF_HashTable_t** ppsHT, uint8_t** ppuClusterData)
{
    psFolderNode->sExtraData.sDirData.psDirHTLRUSlot = NULL;

    *ppsHT = psFolderNode->sExtraData.sDirData.sHT;
    psFolderNode->sExtraData.sDirData.sHT = NULL;

    *ppuClusterData = psFolderNode->sExtraData.sDirData.psDirClusterData->bIsAllocated ?
    psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData :
    NULL;
    psFolderNode->sExtraData.sDirData.psDirClusterData->bIsAllocated = false;
    psFolderNode->sExtraData.sDirData.psDirClusterData->puClusterData = NULL;
    psFolderNode->sExtraData.sDirData.psDirClusterData->uLocalDirHTGeneration = 0;
    
    psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid = false;
}

void
DIROPS_ReleaseHTForDirectory(NodeRecord_s* psFolderNode, bool bForceEvict)
{
    // Caller is done using the directory -- if we have been evicted, relase
    // our cached data.
    MultiReadSingleWrite_LockWrite(&psFolderNode->sRecordData.sRecordLck);

    LF_HashTable_t* psHT = NULL;
    uint8_t* puClusterData = NULL;

    psFolderNode->sExtraData.sDirData.uHTBusyCount--;

    if (DIROPS_WasEvictedFromHTLRUSlot(psFolderNode, psFolderNode->sExtraData.sDirData.uHTBusyCount) || bForceEvict)
    {
        DIROPS_CleanRefereaceForHTForDirectory(psFolderNode, &psHT, &puClusterData);
    }

    MultiReadSingleWrite_FreeWrite(&psFolderNode->sRecordData.sRecordLck);

    if (psHT != NULL)
        ht_DeAllocateHashTable(psHT);
    if (puClusterData != NULL)
        free(puClusterData);
}

void
DIROPS_DestroyHTForDirectory(NodeRecord_s* psFolderNode)
{
    MultiReadSingleWrite_LockWrite(&psFolderNode->sRecordData.sRecordLck);

    DIROPS_ReleaseHTLRUSlot(psFolderNode);

    LF_HashTable_t* psHT = NULL;
    uint8_t* puClusterData = NULL;

    DIROPS_CleanRefereaceForHTForDirectory(psFolderNode, &psHT, &puClusterData);

    MultiReadSingleWrite_FreeWrite(&psFolderNode->sRecordData.sRecordLck);

    if (psHT != NULL)
        ht_DeAllocateHashTable(psHT);
    if (puClusterData != NULL)
        free(puClusterData);
}



static bool DIROPS_CompareTimes(const struct timespec* psTimeA, const struct timespec* psTimeB)
{
    //Returns true if a happened at or after b.
    if (psTimeA->tv_sec == psTimeB->tv_sec)
        return psTimeA->tv_nsec >= psTimeB->tv_nsec;
    else
        return psTimeA->tv_sec > psTimeB->tv_sec;
}

static bool DIROPS_DirScanIsMatch(NodeRecord_s* psFolderNode, struct unistr255* psName, ScanDirRequest_s* psScanDirRequest, NodeDirEntriesData_s* psNodeDirEntriesData, uint32_t* piErr)
{
    bool bIsMatch = true;
    NodeRecord_s* psNodeRecord = NULL;
    FileSystemRecord_s *psFSRecord  = GET_FSRECORD(psFolderNode);
    UVFSDirEntryAttr* psDirEntry = psScanDirRequest->psMatchingResult->smr_entry;

    //Handle Names
    char* pcOriginalFileNameUTF8 = (char*)malloc(FAT_MAX_FILENAME_UTF8);
    char* pcFileNameUTF8 = (char*)malloc(FAT_MAX_FILENAME_UTF8);
    char* pcSearchRequestLowerCaseUTF8 = (char*)malloc(FAT_MAX_FILENAME_UTF8);
    if (pcFileNameUTF8 == NULL || pcSearchRequestLowerCaseUTF8 == NULL || pcOriginalFileNameUTF8 == NULL)
    {
        MSDOS_LOG(LEVEL_ERROR, "Failed to allocate memory");
        *piErr = ENOMEM;
        bIsMatch = false;
        goto fail_exit;
    }

    CONV_Unistr255ToUTF8(psName, pcOriginalFileNameUTF8);
    CONV_Unistr255ToLowerCase( psName );
    CONV_Unistr255ToUTF8(psName, pcFileNameUTF8);
    size_t uNameLen = strlen(pcFileNameUTF8);

    //Need to skip on . and .. entries
    if (DIROPS_IsDotOrDotDotName(pcFileNameUTF8))
    {
        bIsMatch = false;
        goto exit;
    }

    //Need to get files params: need to get the file node
    uint32_t eRecId = DIROPS_GetRecordId( &psNodeDirEntriesData->sDosDirEntry, psFolderNode );
    *piErr = FILERECORD_AllocateRecord( &psNodeRecord, GET_FSRECORD(psFolderNode),
                                     DIROPS_GetStartCluster( psFSRecord, &psNodeDirEntriesData->sDosDirEntry),
                                     eRecId, psNodeDirEntriesData, pcFileNameUTF8 );
    if (*piErr)
    {
        bIsMatch = false;
        goto exit;
    }
    MSDOS_GetAtrrFromDirEntry( psNodeRecord, &psDirEntry->dea_attrs );

    // filter out hidden files
    bool bAllowHiddenFiles = (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_validmask & UVFS_FA_VALID_BSD_FLAGS) &&
                             (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_bsd_flags & UF_HIDDEN);
    if (bAllowHiddenFiles == false) {
        
        // Filter out files with filename that begins with .
        if ( pcFileNameUTF8[0] == '.' ) {
            bIsMatch = false;
            goto exit;
        }
        
        // Filter out files with ATTR_HIDDEN flag
        struct dosdirentry *psDosDirEntry  = &psNodeRecord->sRecordData.sNDE.sDosDirEntry;
        if ( psDosDirEntry && (psDosDirEntry->deAttributes & ATTR_HIDDEN) ) {
            bIsMatch = false;
            goto exit;
        }
    }
    
    // If need to verify name contains
    if (psScanDirRequest->psMatchingCriteria->smr_filename_contains != NULL)
    {
        //For each name in smr_filename_contains
        bool bAtLeastOneNameContainsMatched = false;
        char** ppcNameContainsStr = psScanDirRequest->psMatchingCriteria->smr_filename_contains;
        while ( (*ppcNameContainsStr) && (strlen(*ppcNameContainsStr) != 0) && !bAtLeastOneNameContainsMatched)
        {
            uint64_t uNameContainsLength = strlen(*ppcNameContainsStr);
            *piErr = CONV_UTF8ToLowerCase(*ppcNameContainsStr,pcSearchRequestLowerCaseUTF8);
            if (*piErr)
            {
                bIsMatch = false;
                goto exit;
            }
            
            if (uNameContainsLength <= uNameLen)
            {
                if(strstr(pcFileNameUTF8, pcSearchRequestLowerCaseUTF8) != NULL)
                {
                    bAtLeastOneNameContainsMatched |= true;
                }
            }
            ppcNameContainsStr++;
        }
        bIsMatch = bAtLeastOneNameContainsMatched;
    }

    if (!bIsMatch) goto check_if_directory;

    // If need to verify name appendix
    if (psScanDirRequest->psMatchingCriteria->smr_filename_ends_with != NULL)
    {
        //For each name in smr_filename_contains
        bool bAtLeastOneNameEndWithMatched = false;
        char** ppcNameEndsWithStr = psScanDirRequest->psMatchingCriteria->smr_filename_ends_with;
        while ( (*ppcNameEndsWithStr) && (strlen(*ppcNameEndsWithStr) != 0) && !bAtLeastOneNameEndWithMatched)
        {
            uint64_t uNameEndsWithLength = strlen(*ppcNameEndsWithStr);
            *piErr = CONV_UTF8ToLowerCase(*ppcNameEndsWithStr,pcSearchRequestLowerCaseUTF8);
            if (*piErr)
            {
                bIsMatch = false;
                goto exit;
            }
            if (uNameEndsWithLength <= uNameLen)
            {
                char* pcFileAppendix = pcFileNameUTF8 + uNameLen - uNameEndsWithLength;

                if ( !memcmp(pcFileAppendix, pcSearchRequestLowerCaseUTF8, uNameEndsWithLength) )
                {
                    bAtLeastOneNameEndWithMatched |= true;
                }
            }
            ppcNameEndsWithStr++;
        }
        bIsMatch = bAtLeastOneNameEndWithMatched;
    }

    if (!bIsMatch) goto check_if_directory;

    //If need to validate any other param
    if (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_validmask != 0)
    {
        // If need to verify the file type
        if (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_validmask & UVFS_FA_VALID_TYPE)
        {
            uint32_t uEntryType = 0x1 << (psDirEntry->dea_attrs.fa_type - 1);
            if ((psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_type & uEntryType) != uEntryType)
            {
                bIsMatch = false;
            }
        }

        if (!bIsMatch) goto check_if_directory;

        // If need to verify the file mTime
        if (psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_validmask & UVFS_FA_VALID_MTIME)
        {
            //Comapre if the Mtime of the found entry is after the search Mtime
            if (!DIROPS_CompareTimes(&psDirEntry->dea_attrs.fa_mtime, &psScanDirRequest->psMatchingCriteria->smr_attribute_filter->fa_mtime))
            {
                bIsMatch = false;
            }
        }
    }

    if (bIsMatch)
        psScanDirRequest->psMatchingResult->smr_result_type = SEARCH_RESULT_MATCH;

check_if_directory:
    //In case that one of the requested creteria wasn't fullfiled we need to check if this is a folder and return
    if (psDirEntry->dea_attrs.fa_type == UVFS_FA_TYPE_DIR)
    {
        psScanDirRequest->psMatchingResult->smr_result_type |= SEARCH_RESULT_PUSH;
        bIsMatch = true;
    }
exit:
    if (bIsMatch)
    {
        psDirEntry->dea_namelen      = strlen( pcOriginalFileNameUTF8 );
        psDirEntry->dea_spare0       = 0;
        psDirEntry->dea_nameoff      = UVFS_DIRENTRYATTR_NAMEOFF;
        memcpy( UVFS_DIRENTRYATTR_NAMEPTR((UVFSDirEntryAttr*) psDirEntry), pcOriginalFileNameUTF8, psDirEntry->dea_namelen );
        UVFS_DIRENTRYATTR_NAMEPTR(psDirEntry)[psDirEntry->dea_namelen] = 0;
    }
fail_exit:
    if (psNodeRecord != NULL)
        FILERECORD_FreeRecord(psNodeRecord);
    if (pcFileNameUTF8 != NULL)
        free(pcFileNameUTF8);
    if (pcSearchRequestLowerCaseUTF8 != NULL)
        free(pcSearchRequestLowerCaseUTF8);
    if (pcOriginalFileNameUTF8 != NULL)
        free(pcOriginalFileNameUTF8);
    return bIsMatch;
}

static bool DIROPS_IsLastEntryInDir(NodeRecord_s* psFolderNode, uint64_t uEntryOffset, uint32_t* piErr, ClusterData_s* psClusterData)
{
    DirEntryStatus_e eStatus = DIR_ENTRY_START;
    uint64_t uDosEntryOffset    = 0;
    uint64_t uNextEnteryOffset  = 0;
    struct unistr255* psName        = (struct unistr255*)malloc(sizeof(struct unistr255));
    if (psName == NULL)
    {
        *piErr = ENOMEM;
        return false;
    }

    NodeDirEntriesData_s sNodeDirEntriesData;
    bool bIsEOF = false;

    eStatus = DIROPS_GetDirEntryByOffset( psFolderNode, psClusterData, uEntryOffset, &uDosEntryOffset, &uNextEnteryOffset, &sNodeDirEntriesData, psName, piErr );
    if (eStatus == DIR_ENTRY_EOD || eStatus == DIR_ENTRY_EMPTY)
    {
        bIsEOF = true;
    }

    free(psName);
    return bIsEOF;
}

int
DIROPS_LookForDirEntry( NodeRecord_s* psFolderNode, LookForDirEntryArgs_s* psArgs, RecordIdentifier_e* peRecoredId, NodeDirEntriesData_s* psNodeDirEntriesData )
{
    uint32_t err                = 0;
    uint64_t uEntryOffset       = (psArgs->eMethod == LU_BY_SEARCH_CRITERIA)? psArgs->sData.sScanDirRequest.uStartOffset : 0;
    uint64_t uDosEntryOffset    = 0;
    uint64_t uNextEnteryOffset  = 0;

    struct unistr255* psName        = (struct unistr255*)malloc(sizeof(struct unistr255));

    if ( psName == NULL )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForDirEntry fail to allocate memory.\n");
        err = ENOMEM;
        goto exit;
    }

    memset( psNodeDirEntriesData,   0, sizeof(NodeDirEntriesData_s)  );
    memset( psName,                 0, sizeof(struct unistr255)     );

    if (( psArgs->eMethod == LU_BY_SHORT_NAME ) && ( psArgs->sData.pcShortName[0] == 0 ))
    {
        err = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForDirEntry: LU_BY_SHORT_NAME name is empty.\n", EINVAL);
        goto exit;
    }
    else if ( psArgs->eMethod == LU_BY_NAME && psArgs->sData.psSearchName == NULL)
    {
        err = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForDirEntry: LU_BY_NAME but psSearchName == NULL \n");
        goto exit;
    }

    DirEntryStatus_e eStatus = DIR_ENTRY_START;
    bool bFoundMatch = false;
    do {
        eStatus = DIROPS_GetDirEntryByOffset( psFolderNode, psFolderNode->sExtraData.sDirData.psDirClusterData, uEntryOffset, &uDosEntryOffset, &uNextEnteryOffset, psNodeDirEntriesData, psName, &err );
        if ( DIR_ENTRY_FOUND == eStatus )
        {
            switch( psArgs->eMethod )
            {
                case LU_BY_NAME:
                {
                    if ( psArgs->sData.psSearchName->length == psName->length )
                    {
                        /* Convert to lower case */
                        CONV_Unistr255ToLowerCase( psName );

                        if ( !memcmp(psName->chars, psArgs->sData.psSearchName->chars, psArgs->sData.psSearchName->length*sizeof(psArgs->sData.psSearchName->chars[0])) )
                        {
                            bFoundMatch = true;
                        }
                    }

                }   break;

                case LU_BY_FIRST_CLUSTER_NUM:
                {
                    bFoundMatch = ( psArgs->sData.uFirstClusterIdx == DIROPS_GetStartCluster( GET_FSRECORD(psFolderNode), &psNodeDirEntriesData->sDosDirEntry ) );
                    
                }   break;

                case LU_BY_SEARCH_CRITERIA:
                {
                    bFoundMatch =  DIROPS_DirScanIsMatch(psFolderNode, psName, &psArgs->sData.sScanDirRequest, psNodeDirEntriesData, &err);
                    if (err) goto exit;
                    if (bFoundMatch)
                    {
                        //Check if last entry in directory
                        psArgs->sData.sScanDirRequest.psMatchingResult->smr_entry->dea_nextcookie   = DIROPS_IsLastEntryInDir(psFolderNode, uNextEnteryOffset, &err, psFolderNode->sExtraData.sDirData.psDirClusterData)? UVFS_DIRCOOKIE_EOF : uNextEnteryOffset;
                        psArgs->sData.sScanDirRequest.psMatchingResult->smr_entry->dea_nextrec      = _UVFS_DIRENTRYATTR_RECLEN(UVFS_DIRENTRYATTR_NAMEOFF, psArgs->sData.sScanDirRequest.psMatchingResult->smr_entry->dea_namelen);

                        if (err) goto exit;
                    }
                }   break;

                case LU_BY_SHORT_NAME:
                {
                    bFoundMatch = !memcmp( psArgs->sData.pcShortName, psNodeDirEntriesData->sDosDirEntry.deName, SHORT_NAME_LEN );
                    
                }   break;
                    
                default:
                {
                    //looking for LU_BY_VOLUME_ENTRY and couldn't find this entry
                    break;
                }
            }
        }
        else if ( ( psArgs->eMethod == LU_BY_VOLUME_ENTRY ) && ( eStatus == DIR_ENTRY_VOL_NAME ) )
        {
            bFoundMatch = true;
        }
        uEntryOffset = uNextEnteryOffset;
        
    } while ( eStatus != DIR_ENTRY_EMPTY && eStatus != DIR_ENTRY_ERROR && !bFoundMatch && eStatus != DIR_ENTRY_EOD );

    if (eStatus == DIR_ENTRY_ERROR)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForDirEntry: DIROPS_GetDirEntryByOffset fail: %d.\n", err);
        goto exit;
    }

    if ( !bFoundMatch )
    {
        if ((eStatus == DIR_ENTRY_EMPTY || eStatus == DIR_ENTRY_EOD) && psArgs->eMethod == LU_BY_SEARCH_CRITERIA)
        {
            psArgs->sData.sScanDirRequest.psMatchingResult->smr_result_type = 0;
            psArgs->sData.sScanDirRequest.psMatchingResult->smr_entry->dea_nextcookie = UVFS_DIRCOOKIE_EOF;
        }
        else
        {
            err = ENOENT;
        }
        goto exit;
    }

    if ( peRecoredId != NULL )
    {
        // Found record id.
        *peRecoredId = DIROPS_GetRecordId( &psNodeDirEntriesData->sDosDirEntry, psFolderNode );
        if ( *peRecoredId == RECORD_IDENTIFIER_UNKNOWN )
        {
            err = EBADF;
            goto exit;
        }
    }
    
    err = 0;
    goto exit;
    
    
exit:
    if ( psName != NULL )
        free( psName );

    return err;
}

int
MSDOS_Lookup (UVFSFileNode dirNode, const char *pcUTF8Name, UVFSFileNode *outNode)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Lookup\n");
    VERIFY_NODE_IS_VALID(dirNode);

    int iErr = 0;
    NodeRecord_s* psFolderNode = GET_RECORD(dirNode);

    iErr = DIROPS_CreateHTForDirectory(psFolderNode);
    if (iErr)
        return iErr;

    MultiReadSingleWrite_LockRead( &psFolderNode->sRecordData.sRecordLck );

    iErr = DIROPS_LookupInternal( dirNode, pcUTF8Name, outNode, true );

    MultiReadSingleWrite_FreeRead( &psFolderNode->sRecordData.sRecordLck );

    if (!iErr)
        DIAGNOSTIC_INSERT(GET_RECORD(*outNode),psFolderNode->sRecordData.uFirstCluster,pcUTF8Name);

    //Getting ENOENT here, doesn't require force evict
    bool bReleaseHT = ((iErr != 0) && (iErr != ENOENT));
    DIROPS_ReleaseHTForDirectory(psFolderNode, bReleaseHT);

    return iErr;
}

int
DIROPS_LookForDirEntryByName (NodeRecord_s* psFolderNode, const char *pcUTF8Name, RecordIdentifier_e* peRecoredId, NodeDirEntriesData_s* psNodeDirEntriesData)
{
    if ( !IS_DIR(psFolderNode) )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForDirEntryByName: Given node is not a dir [%d]\n",ENOTDIR);
        return ENOTDIR;
    }

    //Lock the hash table while looking for the entry
    MultiReadSingleWrite_LockRead(&psFolderNode->sExtraData.sDirData.sHT->sHTLck);
        
    uint32_t err = 0;
    uint64_t uDosEntryOffset    = 0;
    uint64_t uNextEnteryOffset  = 0;
    bool bFoundMatch = false;
    bool bNeedToForceInsert = false;

    struct unistr255* psName        = (struct unistr255*)malloc(sizeof(struct unistr255));
    struct unistr255* psSearchName  = (struct unistr255*)malloc(sizeof(struct unistr255));
    memset( psName,                 0, sizeof(struct unistr255)     );
    memset( psSearchName,           0, sizeof(struct unistr255)     );
    memset( psNodeDirEntriesData,   0, sizeof(NodeDirEntriesData_s)  );

    /* Convert the search name to UTF-16 */
    uint32_t uFlags = (DIROPS_IsDotOrDotDotName(pcUTF8Name))? 0 : UTF_SFM_CONVERSIONS;
    err = CONV_UTF8ToUnistr255((const uint8_t *)pcUTF8Name, strlen(pcUTF8Name), psSearchName, uFlags);
    if ( err != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForDirEntryByName: failed to convert utf8 -> utf16.\n");
        goto exit;
    }
    
    CONV_Unistr255ToLowerCase( psSearchName );
    LF_TableEntry_t* psTableEntry = ht_LookupByName(psFolderNode->sExtraData.sDirData.sHT, psSearchName, NULL);
    if (psTableEntry == NULL)
    {
        if (psFolderNode->sExtraData.sDirData.sHT->bIncomplete)
        {
            goto lookup_OldSchool;
        }
        else
        {
            err= ENOENT;
            goto exit;
        }
    }

    LF_TableEntry_t* psFirstTableEntry = psTableEntry;
    FileSystemRecord_s *psFSRecord = psFolderNode->sRecordData.psFSRecord;
    uint32_t uClusterSize = CLUSTER_SIZE(psFSRecord);
    bool bCheckCurrentClusterOnly = true;
    uint32_t uAlreadyCheckedCluster = 0;
    bool bAlreadyCheckedClusterIsValid = false;

    DirEntryStatus_e eStatus = DIR_ENTRY_START;
    do {
        // First time through the loop, don't check entries that aren't in our
        // current cluster.  If we don't find it, then we go ahead and do an
        // exhaustive search.
        if (bCheckCurrentClusterOnly &&
            psFolderNode->sExtraData.sDirData.psDirClusterData != NULL &&
            (psTableEntry->uEntryNum / uClusterSize) != psFolderNode->sExtraData.sDirData.psDirClusterData->uClusterNum)
        {
            goto loop;
        }
        // If this is our second pass through the loop, we can skip entries
        // in the cluster that we've already searched through.  This saves
        // us redundant work if e.g. we had cluster 0 cached and got a lookup
        // request for an entry that is in cluster 1.
        if (bAlreadyCheckedClusterIsValid &&
            (psTableEntry->uEntryNum / uClusterSize) == uAlreadyCheckedCluster)
        {
            goto loop;
        }
        eStatus = DIROPS_GetDirEntryByOffset( psFolderNode, psFolderNode->sExtraData.sDirData.psDirClusterData, psTableEntry->uEntryNum, &uDosEntryOffset, &uNextEnteryOffset, psNodeDirEntriesData, psName, &err );

        if (eStatus == DIR_ENTRY_ERROR)
        {//Clean exit in case of a real error during DIROPS_GetDirEntryByOffset
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookForDirEntryByName: DIROPS_GetDirEntryByOffset fail: %d.\n", err);
            goto exit;
        }
        else if ( DIR_ENTRY_FOUND == eStatus )
        {
            if ( psSearchName->length == psName->length )
            {
                /* Convert to lower case */
                CONV_Unistr255ToLowerCase( psName );

                if ( !memcmp(psName->chars, psSearchName->chars, psSearchName->length*sizeof(psSearchName->chars[0])) )
                {
                    bFoundMatch = true;
                }
            }
        }
        else
        {
            //Should never get here - all HT entries should be valid
            MSDOS_LOG(LEVEL_ERROR, "psTableEntry->uEntryNum %llu, status: %d. is valid %d\n", psTableEntry->uEntryNum, eStatus, psFolderNode->sExtraData.sDirData.psDirClusterData->bIsValid);
            assert(0);
        }

    loop:
        psTableEntry = psTableEntry->psNextEntry;
        if (psTableEntry == NULL && bCheckCurrentClusterOnly)
        {
            psTableEntry = psFirstTableEntry;
            bCheckCurrentClusterOnly = false;
            if (psFolderNode->sExtraData.sDirData.psDirClusterData != NULL)
            {
                // We can skip the cluster we've already searched on our next
                // time through the loop.
                uAlreadyCheckedCluster = psFolderNode->sExtraData.sDirData.psDirClusterData->uClusterNum;
                bAlreadyCheckedClusterIsValid = true;
            }
        }
    } while ((psTableEntry != NULL) && !bFoundMatch);

lookup_OldSchool:
    if ( !bFoundMatch )
    {
        //If Incomplit, search old school
        if (psFolderNode->sExtraData.sDirData.sHT->bIncomplete)
        {
            LookForDirEntryArgs_s sArgs;
            sArgs.sData.psSearchName = psSearchName;
            sArgs.eMethod = LU_BY_NAME;

            err = DIROPS_LookForDirEntry( psFolderNode, &sArgs, peRecoredId, psNodeDirEntriesData);
            if (!err)
            {
                bNeedToForceInsert = true;
            }
        }
        else
        {
            err = ENOENT;
        }
    }
    else
    {
        if ( peRecoredId != NULL )
        {
            // Found record id.
            *peRecoredId = DIROPS_GetRecordId( &psNodeDirEntriesData->sDosDirEntry, psFolderNode );
            if ( *peRecoredId == RECORD_IDENTIFIER_UNKNOWN )
            {
                err = EBADF;
                goto exit;
            }
        }
    }

exit:

    MultiReadSingleWrite_FreeRead(&psFolderNode->sExtraData.sDirData.sHT->sHTLck);

    //Need to insert the new entry into the cache
    if (bNeedToForceInsert) ht_insert(psFolderNode->sExtraData.sDirData.sHT, psSearchName, psNodeDirEntriesData->uFirstEntryOffset, true);

    if ( psName != NULL )
        free( psName );
    if ( psSearchName != NULL )
        free( psSearchName );

    return err;
}

/*
 Assumption : dirNode lock for read/write.
 */
int
DIROPS_LookupInternal (UVFSFileNode dirNode, const char *pcUTF8Name, UVFSFileNode *outNode, bool bLockDirEntryAccess )
{
    MSDOS_LOG(LEVEL_DEBUG, "DIROPS_LookupInternal\n");

    NodeRecord_s* psFolderNode = GET_RECORD(dirNode);
    errno_t err = 0;

    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psFolderNode) )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_LookupInternal: Given node is not a dir [%d]\n",ENOTDIR);
        return ENOTDIR;
    }
    // In case of '.' or '..' return error...
    else if ( DIROPS_IsDotOrDotDotName(pcUTF8Name) )
    {
        return 1;
    }

    // Lock Directory entry for read.
    if ( bLockDirEntryAccess )
        MultiReadSingleWrite_LockRead( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );

    RecordIdentifier_e eRecId;
    NodeDirEntriesData_s sNodeDirEntriesData;
    // Look for node in directory
    err = DIROPS_LookForDirEntryByName (psFolderNode, pcUTF8Name, &eRecId, &sNodeDirEntriesData );
    if ( err != 0 )
    {
        MSDOS_LOG((err == ENOENT) ? LEVEL_DEBUG : LEVEL_ERROR, "DIROPS_LookupInternal fail to lookup for dir entry [%d].\n",err);
        goto func_fail;
    }

    NodeRecord_s* psNodeRecord;
    // Allocate record
    err = FILERECORD_AllocateRecord( &psNodeRecord, GET_FSRECORD(psFolderNode),
                                    DIROPS_GetStartCluster( GET_FSRECORD(psFolderNode), &sNodeDirEntriesData.sDosDirEntry ), eRecId, &sNodeDirEntriesData, pcUTF8Name );
    if ( err != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "FILERECORD_AllocateRecord fail to allocate record (%d).\n", err);
        goto func_fail;
    }

    //If its a dir calculate hash table
    if ( eRecId == RECORD_IDENTIFIER_DIR )
    {
        err = DIROPS_CreateHTForDirectory(psNodeRecord);
        if ( err != 0 )
        {
            goto func_fail;
        }
    }

    *outNode = psNodeRecord;

    err = 0;
    goto exit;

func_fail:
    *outNode = NULL;
exit:
    if ( bLockDirEntryAccess )
        MultiReadSingleWrite_FreeRead( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );

    return err;
}

int
MSDOS_Remove (UVFSFileNode dirNode, const char *pcUTF8Name, __unused UVFSFileNode victimNode)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Remove\n");
    VERIFY_NODE_IS_VALID(dirNode);

    NodeRecord_s* psFolderNode = GET_RECORD(dirNode);
    int iErr = 0;

    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psFolderNode) )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Remove: Given node is not a dir [%d]\n",ENOTDIR);
        return ENOTDIR;
    }
    
    // '.' and '..' can't be removed.
    if ( DIROPS_IsDotOrDotDotName(pcUTF8Name) )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Remove '.' && '..' can't be removed\n");
        return EPERM;
    }

    iErr = DIROPS_CreateHTForDirectory(psFolderNode);
    if (iErr)
        return iErr;

    FSOPS_SetDirtyBitAndAcquireLck(GET_FSRECORD(psFolderNode));
    MultiReadSingleWrite_LockWrite( &psFolderNode->sRecordData.sRecordLck );
    MultiReadSingleWrite_LockWrite( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );

    RecordIdentifier_e eRecId;
    NodeDirEntriesData_s sNodeDirEntriesData;
    // Look for node in directory
    iErr = DIROPS_LookForDirEntryByName (psFolderNode, pcUTF8Name, &eRecId, &sNodeDirEntriesData );
    if ( iErr != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Remove fail to look for dir entry\n");
        goto exit;
    }
     
    // Make sure the entry is not a directory.
    if ( eRecId == RECORD_IDENTIFIER_DIR )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Remove entry is a directory.\n");
        iErr = EISDIR;
        goto exit;
    }
    
    // Update directory version.
    psFolderNode->sExtraData.sDirData.uDirVersion++;

    // Mark all node directory entries as deleted
    iErr = DIROPS_MarkNodeDirEntriesAsDeleted( psFolderNode, &sNodeDirEntriesData ,pcUTF8Name);
    if ( iErr != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Remove fail to mark enries as deleted.\n");
        goto exit;
    }
    
    // Release clusters (if any allocated..).
    if ( DIROPS_GetStartCluster( GET_FSRECORD(psFolderNode), &sNodeDirEntriesData.sDosDirEntry ) != 0 )
    {
        iErr = FAT_Access_M_FATChainFree( GET_FSRECORD(psFolderNode),
                                  DIROPS_GetStartCluster( GET_FSRECORD(psFolderNode), &sNodeDirEntriesData.sDosDirEntry ), false );
        if ( iErr != 0 )
        {
            goto exit;
        }
    }

exit:
    MultiReadSingleWrite_FreeWrite( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );
    MultiReadSingleWrite_FreeWrite( &psFolderNode->sRecordData.sRecordLck );
    //Unlock dirtybit
    FSOPS_FlushCacheAndFreeLck(GET_FSRECORD(psFolderNode));
    //Getting EISDIR here, doesn't require force evict
    bool bReleaseHT = ((iErr != 0) && (iErr != EISDIR));
    DIROPS_ReleaseHTForDirectory(psFolderNode, bReleaseHT);
    return iErr;
}

int
MSDOS_RmDir (UVFSFileNode dirNode, const char *pcUTF8Name)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_RmDir\n");
    VERIFY_NODE_IS_VALID(dirNode);
    
    NodeRecord_s* psFolderNode = GET_RECORD(dirNode);
    int iErr = 0;
    
    // Make sure the UVFSFileNode is a directory.
    if ( !IS_DIR(psFolderNode) )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_RmDir: Given node is not a dir [%d]\n",ENOTDIR);
        return ENOTDIR;
    }
    
    // '.' and '..' can't be removed.
    if ( DIROPS_IsDotOrDotDotName(pcUTF8Name) )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_RmDir '.' && '..' can't be removed\n");
        return EPERM;
    }

    iErr = DIROPS_CreateHTForDirectory(psFolderNode);
    if (iErr)
        return iErr;

    FSOPS_SetDirtyBitAndAcquireLck(GET_FSRECORD(psFolderNode));
    MultiReadSingleWrite_LockWrite( &psFolderNode->sRecordData.sRecordLck );
    MultiReadSingleWrite_LockWrite( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );

    RecordIdentifier_e eRecId = RECORD_IDENTIFIER_UNKNOWN;
    NodeDirEntriesData_s sNodeDirEntriesData;
    
    // Look for node in directory
    iErr = DIROPS_LookForDirEntryByName (psFolderNode, pcUTF8Name, &eRecId, &sNodeDirEntriesData );
    if ( iErr != 0 )
    {
        goto exit;
    }
    
    // Make sure the entry a directory.
    if ( eRecId != RECORD_IDENTIFIER_DIR )
    {
        iErr = ENOTDIR;
        goto exit;
    }
    
    // Verify the directory is empty
    NodeRecord_s* psTempNodeRecord;
    // Allocate record
    iErr = FILERECORD_AllocateRecord( &psTempNodeRecord, GET_FSRECORD(psFolderNode),
                                        DIROPS_GetStartCluster( GET_FSRECORD(psFolderNode), &sNodeDirEntriesData.sDosDirEntry ), eRecId, &sNodeDirEntriesData, pcUTF8Name );
    if ( iErr != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_RmDir failed to allocate record.\n");
        goto exit;
    }

    iErr = DIROPS_isDirEmpty( psTempNodeRecord );
    
    // Free record
    FILERECORD_FreeRecord( psTempNodeRecord );
    if ( iErr != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_RmDir dir is not empty.\n");
        goto exit;
    }
    
    // Update directory version.
    psFolderNode->sExtraData.sDirData.uDirVersion++;

    // Mark all node directory entries as deleted
    iErr = DIROPS_MarkNodeDirEntriesAsDeleted( psFolderNode, &sNodeDirEntriesData, pcUTF8Name);
    if ( iErr != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_RmDir fail to mark enries as deleted.\n");
        goto exit;
    }
    
    // Release clusters.
    iErr = FAT_Access_M_FATChainFree( GET_FSRECORD(psFolderNode),
                               DIROPS_GetStartCluster( GET_FSRECORD(psFolderNode), &sNodeDirEntriesData.sDosDirEntry ), false );
    if ( iErr != 0 )
    {
        goto exit;
    }

exit:
    MultiReadSingleWrite_FreeWrite( &GET_FSRECORD(psFolderNode)->sDirEntryAccessLock );
    MultiReadSingleWrite_FreeWrite( &psFolderNode->sRecordData.sRecordLck );
    //Unlock dirtybit
    FSOPS_FlushCacheAndFreeLck(GET_FSRECORD(psFolderNode));
    //Getting ENOTDIR here, doesn't require force evict
    bool bReleaseHT = ((iErr != 0) && (iErr != ENOTDIR));
    DIROPS_ReleaseHTForDirectory(psFolderNode, bReleaseHT);
    return iErr;
}

/*
     Update directory entry and flush to device (Read-Modify-Write).
     Assumption: psNodeRecord lock for write.
 */
int
DIROPS_UpdateDirectoryEntry( NodeRecord_s* psNodeRecord, NodeDirEntriesData_s* psNodeDirEntriesData, struct dosdirentry* psDosDirEntry, bool bLockDirEntryAccess )
{
    int iErr = 0;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);

    // Lock dir entry access.
    if ( bLockDirEntryAccess )
        MultiReadSingleWrite_LockWrite( &psNodeRecord->sRecordData.psFSRecord->sDirEntryAccessLock );

    uint32_t uSectorSize = psNodeRecord->sRecordData.psFSRecord->sFSInfo.uBytesPerSector;
    uint8_t* puSectorData = malloc( uSectorSize );
    if ( puSectorData == NULL )
    {
        iErr = ENOMEM;
        goto exit;
    }

    uint64_t uLastEntrySectorInCluster  = psNodeDirEntriesData->uLastEntryOffsetInCluster / uSectorSize;
    uint64_t uLastSectorOffset          = psNodeDirEntriesData->uLastClusterAbsoluteOffset + uLastEntrySectorInCluster*uSectorSize;
    uint64_t uLastEntryOffsetInSector   = psNodeDirEntriesData->uLastEntryOffsetInCluster % uSectorSize;

    size_t uBytesRead = pread(GET_FSRECORD(psNodeRecord)->iFD, puSectorData, uSectorSize, uLastSectorOffset);
    if ( uBytesRead != uSectorSize )
    {
        iErr = errno;
        goto exit;
    }

    //Increment global uDirHTGeneration
    atomic_fetch_add(&psFSRecord->uDirHTGeneration, 1);

    struct dosdirentry* psDirEntryStartPtr = (struct dosdirentry*)&puSectorData[uLastEntryOffsetInSector];
    *psDirEntryStartPtr = *psDosDirEntry;
    
    size_t uBytesWrite = pwrite(GET_FSRECORD(psNodeRecord)->iFD, puSectorData, uSectorSize, uLastSectorOffset);
    if ( uBytesWrite != uSectorSize )
    {
        iErr = errno;
        goto exit;
    }
    
    psNodeRecord->sRecordData.sNDE.sDosDirEntry = *psDosDirEntry;
    
exit:
    if ( bLockDirEntryAccess )
        MultiReadSingleWrite_FreeWrite( &psNodeRecord->sRecordData.psFSRecord->sDirEntryAccessLock );

    if ( puSectorData )
        free( puSectorData );
    
    return iErr;
}

int
MSDOS_ScanDir (UVFSFileNode psDirNode, scandir_matching_request_t* psMatchingCriteria, scandir_matching_reply_t* psMatchingResult)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_ScanDir\n");
    int iErr = 0;
    NodeRecord_s* psFolderNode = GET_RECORD(psDirNode);
    LookForDirEntryArgs_s sArgs = {0};
    RecordIdentifier_e eRecoredId;
    NodeDirEntriesData_s sNDE;

    VERIFY_NODE_IS_VALID(psDirNode);

    // Make sure the psFolderNode is a directory.
    if ( !IS_DIR(psFolderNode) )
    {
        return ENOTDIR;
    }

    if (((psMatchingCriteria->smr_filename_contains != NULL) && (strlen(*psMatchingCriteria->smr_filename_contains) == 0)) ||
        ((psMatchingCriteria->smr_filename_ends_with != NULL) && (strlen(*psMatchingCriteria->smr_filename_ends_with) == 0)))
    {
        iErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ScanDir: one of the given smr_filename_contains or smr_filename_ends_with, is empty.\n", EINVAL);
        goto exit;
    }

    MultiReadSingleWrite_LockWrite( &psFolderNode->sRecordData.sRecordLck );

    iErr = DIROPS_VerifyCookieAndVerifier( psFolderNode, psMatchingCriteria->smr_start_cookie, psFolderNode->sExtraData.sDirData.psDirClusterData, true, psMatchingCriteria->smr_verifier);
    if ( iErr != 0 )
    {
        goto exit;
    }

    psMatchingResult->smr_verifier = psFolderNode->sExtraData.sDirData.uDirVersion;
    psMatchingResult->smr_result_type = 0;
    sArgs.eMethod = LU_BY_SEARCH_CRITERIA;
    sArgs.sData.sScanDirRequest.psMatchingCriteria = psMatchingCriteria;
    sArgs.sData.sScanDirRequest.psMatchingResult = psMatchingResult;
    sArgs.sData.sScanDirRequest.uStartOffset = psMatchingCriteria->smr_start_cookie;

    iErr = DIROPS_LookForDirEntry(psFolderNode, &sArgs, &eRecoredId, &sNDE);

exit:
    MultiReadSingleWrite_FreeWrite( &psFolderNode->sRecordData.sRecordLck );
    return iErr;
}
