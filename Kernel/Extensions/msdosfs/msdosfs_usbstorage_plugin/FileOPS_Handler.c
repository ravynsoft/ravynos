/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  FileOPS_Handler.c
 *  usbstorage_plugin
 *
 *  Created by Or Haimovich on 15/10/17.
 */

#include <sys/stat.h>

#include "FileOPS_Handler.h"
#include "DirOPS_Handler.h"
#include "FAT_Access_M.h"
#include "Conv.h"
#include "FileRecord_M.h"
#include "RawFile_Access_M.h"
#include "Logger.h"
#include "ZeroFill.h"
#include "diagnostic.h"
#include "FSOPS_Handler.h"

#define MSDOS_VALID_BSD_FLAGS_MASK (SF_ARCHIVED | SF_IMMUTABLE | UF_IMMUTABLE | UF_HIDDEN)

////// Functions prototypes ////////

static int DIROPS_CreateLinkAccordingToContent(FileSystemRecord_s* psFSRecord,uint32_t uCluster, const char *pcContents, uint32_t uAmountOfAllocatedClusters);
static int MSDOS_SetAtrrToDirEntry (NodeRecord_s* psNodeRecord, const UVFSFileAttributes *setAttrs, UVFSFileAttributes *outAttrs, bool bPartialAllocationAllowed, bool bNeedToUpdateDirEntry, bool bFillNewClustersWithZeros, bool bFillLastClusterWithZeros );
static int FileOPS_FillClusterSuffixWithZeros( NodeRecord_s* psNodeRecord, uint32_t uFillFromOffset, uint32_t uCluster );

static uint32_t    MSDOS_GetFileBSDFlags (u_int8_t deAttributes);
static int         MSDOS_SetFileBSDFlags (uint32_t uFileBSDFlags, u_int8_t *puDeAttributes, bool bIsADir);
/*
 Assumption : psNodeRecord lock for write.
 */
int
FILEOPS_UpdateLastModifiedTime( NodeRecord_s* psNodeRecord )
{
    int iErr = 0;
    uint16_t uTime, uDate;
    bool bNeedToUpdate = false;
    struct timespec sCurrentTS;

    // Root directory is special case of updating dircetory modification time..
    if ( psNodeRecord->sRecordData.eRecordID == RECORD_IDENTIFIER_DIR )
    {
        return EISDIR;
    }

    CONV_GetCurrentTime( &sCurrentTS );

    // If ATTR_ARCHIVE bit not set... always update.
    if ( !(psNodeRecord->sRecordData.sNDE.sDosDirEntry.deAttributes & ATTR_ARCHIVE) && !IS_ROOT(psNodeRecord) )
    {
        bNeedToUpdate = true;
    }
    else
    {
        // Update only if there is a diff in the modified time.
        msdosfs_unix2dostime(&sCurrentTS, &uDate, &uTime, NULL);
        if ( (uTime != getuint16(psNodeRecord->sRecordData.sNDE.sDosDirEntry.deMTime)) ||
             (uDate != getuint16(psNodeRecord->sRecordData.sNDE.sDosDirEntry.deMDate)) )
        {
            bNeedToUpdate = true;
        }
    }

    if ( bNeedToUpdate )
    {
        UVFSFileAttributes sAttr, sDontCare;
        memset( &sAttr, 0, sizeof(UVFSFileAttributes) );

        sAttr.fa_validmask  |= UVFS_FA_VALID_MTIME;
        sAttr.fa_mtime      = sCurrentTS;

        iErr = MSDOS_SetAtrrToDirEntry( psNodeRecord, (const UVFSFileAttributes*) &sAttr, &sDontCare, false, true, true, true );
    }

    return iErr;
}

static int
DIROPS_CreateLinkAccordingToContent(FileSystemRecord_s* psFSRecord,uint32_t uCluster, const char * pcContents, uint32_t uAmountOfAllocatedClusters)
{
    int iError =0;
    size_t uLinkLength = strlen(pcContents);
    
    // Verify unLength is a valid length
    if (uLinkLength> SYMLINK_LINK_MAX)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateLinkAccordingToContent: Link Length > SYMLINK_LINK_MAX. Error [%d].\n",ENAMETOOLONG);
        iError = ENAMETOOLONG;
        goto exit;
    }

    if (uLinkLength == 0)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateLinkAccordingToContent: Link content is empty. Error [%d].\n",EINVAL);
        iError = EINVAL;
        goto exit;
    }

    //Allocate buffer
    uint32_t uBufferSize = sizeof(struct symlink);
    void* pvLinkContentBuffer = malloc(uBufferSize);
    if (pvLinkContentBuffer == NULL)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateLinkAccordingToContent: Failed to allocate buffer for symlink. Error [%d]\n",ENOMEM);
        iError = ENOMEM;
        goto exit;
    }

    memset(pvLinkContentBuffer,0,uBufferSize);
    
    struct symlink* psLink = (struct symlink*) pvLinkContentBuffer;
    
    // Set link magic
    memcpy(psLink->magic, symlink_magic, SYMLINK_MAGIC_LENGTH);
    
    // Set Link Length
    char* pcLinkLength = psLink->length;
    snprintf(pcLinkLength, 6, "%04u\n", (unsigned)uLinkLength);
    
    // Set the MD5 digest
    DIROPS_GetMD5Digest((void*) pcContents, uLinkLength, psLink->md5);
    psLink->newline2 = '\n';
    
    // Set the data into the link
    memcpy(psLink->link,pcContents,uLinkLength);
    
    /* Pad with newline if there is room */
    if (uLinkLength < SYMLINK_LINK_MAX)
        psLink->link[uLinkLength++] = '\n';
    
    /* Pad with spaces if there is room */
    if (uLinkLength < SYMLINK_LINK_MAX)
        memset(&psLink->link[uLinkLength], ' ', SYMLINK_LINK_MAX-uLinkLength);

    uint32_t uAmountOfWrittenClusters = 0;
    uint32_t uLinkOffset =0 ;
    while (uAmountOfWrittenClusters < uAmountOfAllocatedClusters)
    {
        uint32_t uNextCluster = 0;
        uint32_t uExtentLength = FAT_Access_M_ContiguousClustersInChain(psFSRecord, uCluster, &uNextCluster, &iError);
        uint32_t uClusterBufferSize = uExtentLength*CLUSTER_SIZE(psFSRecord);
        void* pvClusterBuffer = malloc(uClusterBufferSize);
        if (pvClusterBuffer == NULL)
        {
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateLinkAccordingToContent: Failed to allocate Cluster buffer. Error [%d]\n",ENOMEM);
            iError = ENOMEM;
            break;
        }

        memset(pvClusterBuffer,0,uClusterBufferSize);
        uint32_t uBytesToCopy = MIN(uClusterBufferSize,uBufferSize - uLinkOffset);
        memcpy(pvClusterBuffer,pvLinkContentBuffer + uLinkOffset,uBytesToCopy);

        if (pwrite(psFSRecord->iFD, pvClusterBuffer, uClusterBufferSize, DIROPS_VolumeOffsetForCluster(psFSRecord,uCluster)) != uClusterBufferSize)
        {
            iError = errno;
            MSDOS_LOG(LEVEL_ERROR, "DIROPS_CreateLinkAccordingToContent: Failed to write link content into the device. Error [%d]\n",iError);
            free(pvClusterBuffer);
            break;
        }

        uCluster = uNextCluster;
        uAmountOfWrittenClusters += uExtentLength;
        uLinkOffset += uBytesToCopy;
        free(pvClusterBuffer);
    }

    free(pvLinkContentBuffer);
    
exit:
    return iError;
}

uint64_t
MSDOS_GetFileID (NodeRecord_s* psNodeRecord)
{
    uint64_t uFileID = psNodeRecord->sRecordData.uFirstCluster;
    
    if ( IS_FAT_12_16_ROOT_DIR(psNodeRecord) )
    {
        /* FAT12 and FAT16 have a special hard-coded file ID. */
        uFileID = FILENO_ROOT;
    }
    
    /* Back-stop the file ID in case of something bogus. */
    if ( uFileID == 0 )
    {
        uFileID = FILENO_EMPTY;
    }
    
    return uFileID;
}

static uint32_t
MSDOS_GetFileBSDFlags (u_int8_t deAttributes)
{
    uint32_t uFileBSDFlags = 0;

    if ((deAttributes & (ATTR_ARCHIVE | ATTR_DIRECTORY)) == 0)    // DOS: flag set means "needs to be archived"
        uFileBSDFlags |= SF_ARCHIVED;                // BSD: flag set means "has been archived"
    if ((deAttributes & (ATTR_READONLY | ATTR_DIRECTORY)) == ATTR_READONLY)
        uFileBSDFlags |= UF_IMMUTABLE;                // DOS read-only becomes BSD user immutable
    if (deAttributes & ATTR_HIDDEN)
        uFileBSDFlags |= UF_HIDDEN;
    return uFileBSDFlags;
}

static int
MSDOS_SetFileBSDFlags (uint32_t uFileBSDFlags, u_int8_t *puDeAttributes, bool bIsADir)
{
    
    if ( (uFileBSDFlags & ~MSDOS_VALID_BSD_FLAGS_MASK) != 0)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_SetFileBSDFlags: Recieved invalid BSD Flag to set [0x%x]\n",uFileBSDFlags);
        return EINVAL;
    }
    
    if (uFileBSDFlags & UF_HIDDEN)
    {
        *puDeAttributes |= ATTR_HIDDEN;
    }
    else
    {
        *puDeAttributes &= ~ATTR_HIDDEN;
    }
    
    if (!bIsADir)
    {
        if (uFileBSDFlags & (SF_IMMUTABLE | UF_IMMUTABLE) )
        {
            *puDeAttributes |= ATTR_READONLY;
        }
        else
        {
            *puDeAttributes &= ~ATTR_READONLY;
        }
        
        if (uFileBSDFlags & SF_ARCHIVED)
        {
            *puDeAttributes &= ~ATTR_ARCHIVE;
        }
        else
        {
            *puDeAttributes |= ATTR_ARCHIVE;
        }
    }
    
    return 0;
}

/*
 Assumption : psNodeRecord lock for read / write.
 */
void
MSDOS_GetAtrrFromDirEntry (NodeRecord_s* psNodeRecord, UVFSFileAttributes *outAttrs)
{
    memset( outAttrs, 0, sizeof(UVFSFileAttributes) );

    // Need to synthesize root attributes?..
    if ( IS_ROOT(psNodeRecord) && !psNodeRecord->sRecordData.bIsNDEValid )
    {
        outAttrs->fa_validmask  = SYNTH_ROOT_VALID_ATTR_MASK;
        outAttrs->fa_mode       = UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX);
        outAttrs->fa_type       = UVFS_FA_TYPE_DIR;
#ifdef MSDOS_NLINK_IS_CHILD_COUNT
        outAttrs->fa_nlink      = psNodeRecord->sExtraData.sDirData.uChildCount;
#else
        outAttrs->fa_nlink      = 1;
#endif
        outAttrs->fa_allocsize  = IS_FAT_12_16_ROOT_DIR(psNodeRecord)?
                                  GET_FSRECORD(psNodeRecord)->sRootInfo.uRootLength :
                                  (uint64_t)psNodeRecord->sRecordData.uClusterChainLength * (uint64_t)CLUSTER_SIZE(GET_FSRECORD(psNodeRecord));
        outAttrs->fa_size       = outAttrs->fa_allocsize;
        outAttrs->fa_fileid     = MSDOS_GetFileID(psNodeRecord);

        return;
    }
    
    struct dosdirentry *psDirEntry  = &psNodeRecord->sRecordData.sNDE.sDosDirEntry;
    
    outAttrs->fa_validmask = VALID_OUT_ATTR_MASK;
    
    outAttrs->fa_type = puRecordId2FaType[psNodeRecord->sRecordData.eRecordID];
    outAttrs->fa_mode = UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX) ;
    outAttrs->fa_nlink =
#ifdef MSDOS_NLINK_IS_CHILD_COUNT
        IS_DIR(psNodeRecord) ? psNodeRecord->sExtraData.sDirData.uChildCount :
#endif
        1;

    outAttrs->fa_allocsize = IS_FAT_12_16_ROOT_DIR(psNodeRecord)?
                             GET_FSRECORD(psNodeRecord)->sRootInfo.uRootLength :
                             (uint64_t)psNodeRecord->sRecordData.uClusterChainLength * (uint64_t)CLUSTER_SIZE(GET_FSRECORD(psNodeRecord));
    outAttrs->fa_size = IS_DIR(psNodeRecord)        ? outAttrs->fa_allocsize :
                        IS_SYMLINK(psNodeRecord)    ? psNodeRecord->sExtraData.sSymLinkData.uSymLinkLength :
                        getuint32(psDirEntry->deFileSize);
    
    outAttrs->fa_fileid = MSDOS_GetFileID(psNodeRecord);
    outAttrs->fa_bsd_flags  = MSDOS_GetFileBSDFlags(psDirEntry->deAttributes);

    // A/M/C Times..
    {
        // In case of directories.. take the times from '.' entry..
        if ( IS_DIR(psNodeRecord) && !IS_ROOT(psNodeRecord) )
        {
            int iErr = DIROPS_GetDirCluster(psNodeRecord, 0, psNodeRecord->sExtraData.sDirData.psDirClusterData);
            if ( iErr == 0 )
            {
                psDirEntry = (struct dosdirentry*) psNodeRecord->sExtraData.sDirData.psDirClusterData->puClusterData;
            }
        }

        struct timespec sTimespec;
        msdosfs_dos2unixtime(getuint16(psDirEntry->deADate), 0, 0, &sTimespec);
        outAttrs->fa_atime = sTimespec;

        msdosfs_dos2unixtime(getuint16(psDirEntry->deMDate), getuint16(psDirEntry->deMTime), 0, &sTimespec);
        outAttrs->fa_mtime = sTimespec;

        msdosfs_dos2unixtime(getuint16(psDirEntry->deMDate), getuint16(psDirEntry->deMTime), 0, &sTimespec);
        outAttrs->fa_ctime = sTimespec;

        msdosfs_dos2unixtime(getuint16(psDirEntry->deCDate), getuint16(psDirEntry->deCTime), 0, &sTimespec);
        outAttrs->fa_birthtime = sTimespec;
    }
}

/*
 Assumption : psNodeRecord lock for write.
 */
static int
MSDOS_SetAtrrToDirEntry (NodeRecord_s* psNodeRecord, const UVFSFileAttributes *setAttrs, UVFSFileAttributes *outAttrs, bool bPartialAllocationAllowed, bool bNeedToUpdateDirEntry, bool bFillNewClustersWithZeros, bool bFillLastClusterWithZeros )
{
    int iErr = 0;
    struct dosdirentry sDirEntry  = psNodeRecord->sRecordData.sNDE.sDosDirEntry;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);

    if ( setAttrs->fa_validmask == 0 )
    {
        goto exit;
    }

    // Didn't found volume entry.. ignoring..
    if ( IS_ROOT(psNodeRecord) && !psNodeRecord->sRecordData.bIsNDEValid )
    {
        goto exit;
    }

    if ( ( setAttrs->fa_validmask & READ_ONLY_FA_FIELDS )
         /*|| ( setAttrs->fa_validmask & ~VALID_IN_ATTR_MASK )*/    )
    {
        iErr = EINVAL;
        goto exit;
    }

    if ( !IS_DIR( psNodeRecord ) )
    {
        sDirEntry.deAttributes |= ATTR_ARCHIVE;
    }

    if ( setAttrs->fa_validmask & UVFS_FA_VALID_SIZE )
    {
        /* Cannot change size of a directory or symlink! */
        if ( IS_DIR(psNodeRecord) || IS_SYMLINK(psNodeRecord) )
        {
            iErr = EPERM;
            goto exit;
        }
        /* Check file size not too big */
        if ( setAttrs->fa_size > DOS_FILESIZE_MAX )
        {
            iErr = EFBIG;
            goto exit;
        }

        /* Update file size */
        uint64_t uNewFileSize = setAttrs->fa_size;
        uint64_t uCurAllocatSize = (uint64_t)psNodeRecord->sRecordData.uClusterChainLength * (uint64_t)CLUSTER_SIZE(GET_FSRECORD(psNodeRecord));

        // Read-Modify-Write the last cluster in order to fill is suffix with zeros.
        if ( bFillLastClusterWithZeros && ( setAttrs->fa_size > getuint32( psNodeRecord->sRecordData.sNDE.sDosDirEntry.deFileSize )) )
        {
            uint32_t uClusterSize = CLUSTER_SIZE(psFSRecord);
            uint32_t uBytesToKeep = ( getuint32( psNodeRecord->sRecordData.sNDE.sDosDirEntry.deFileSize ) % uClusterSize );
            if ( uBytesToKeep > 0 )
            {
                iErr = FileOPS_FillClusterSuffixWithZeros( psNodeRecord, uBytesToKeep, psNodeRecord->sRecordData.uLastAllocatedCluster );
                if ( iErr != 0 )
                {
                    goto exit;
                }
            }
        }

        if ( setAttrs->fa_size > uCurAllocatSize )
        {
            uint32_t uFirstNewClusAlloc = 0;
            uint32_t uLastClusAlloc     = 0;
            uint32_t uAmountOfAllocatedClusters = 0;
            uint64_t uNeedToAllocSize = setAttrs->fa_size - uCurAllocatSize;
            uNeedToAllocSize = ROUND_UP( uNeedToAllocSize, (uint64_t)CLUSTER_SIZE(GET_FSRECORD(psNodeRecord)) );
            uint64_t uNeedToAllocClusters = uNeedToAllocSize / CLUSTER_SIZE(GET_FSRECORD(psNodeRecord));
            
            if ( uNeedToAllocClusters > 0 )
            {
                NewAllocatedClusterInfo_s* psNewAllocatedClusterInfoToReturn = NULL;
                iErr = FAT_Access_M_AllocateClusters( psFSRecord, (uint32_t)uNeedToAllocClusters, psNodeRecord->sRecordData.uLastAllocatedCluster, &uFirstNewClusAlloc, &uLastClusAlloc, &uAmountOfAllocatedClusters, bPartialAllocationAllowed, bFillNewClustersWithZeros, &psNewAllocatedClusterInfoToReturn, false );
                if ( iErr != 0 && !(bPartialAllocationAllowed && iErr == ENOSPC) )
                {
                    goto exit;
                }

                assert(!((uAmountOfAllocatedClusters* CLUSTER_SIZE(psFSRecord) < uNeedToAllocSize) && (bPartialAllocationAllowed && iErr != ENOSPC)));

                if ((uAmountOfAllocatedClusters* CLUSTER_SIZE(psFSRecord) < uNeedToAllocSize) && (bPartialAllocationAllowed && iErr == ENOSPC))
                {
                    uNewFileSize = uCurAllocatSize + uAmountOfAllocatedClusters* CLUSTER_SIZE(psFSRecord);

                    //If didn't allocate any cluster
                    if (uAmountOfAllocatedClusters == 0)
                    {
                        goto exit;
                    }
                    else
                    {
                        //Cleaning error
                        iErr = 0;
                    }
                }

                MultiReadSingleWrite_LockWrite(&psFSRecord->psClusterChainCache->sClusterChainLck);
                uint64_t uStartAllocatedOffset = uCurAllocatSize;
                while (psNewAllocatedClusterInfoToReturn != NULL)
                {
                    FILERECORD_UpdateNewAllocatedClustersInChain(psNodeRecord, psNewAllocatedClusterInfoToReturn->uNewAlloctedStartCluster, psNewAllocatedClusterInfoToReturn->uAmountOfConsecutiveClusters, uStartAllocatedOffset);
                    
                    uStartAllocatedOffset += psNewAllocatedClusterInfoToReturn->uAmountOfConsecutiveClusters * CLUSTER_SIZE(psFSRecord);
                    NewAllocatedClusterInfo_s* psClusterInfoToFree = psNewAllocatedClusterInfoToReturn;
                    psNewAllocatedClusterInfoToReturn = psNewAllocatedClusterInfoToReturn->psNext;
                    
                    free(psClusterInfoToFree);
                }
                MultiReadSingleWrite_FreeWrite(&psFSRecord->psClusterChainCache->sClusterChainLck);
                
                if ( psNodeRecord->sRecordData.uFirstCluster == 0 )
                {
                    psNodeRecord->sRecordData.uFirstCluster = uFirstNewClusAlloc;
                    DIROPS_SetStartCluster( GET_FSRECORD(psNodeRecord), &sDirEntry, uFirstNewClusAlloc );
                }
                psNodeRecord->sRecordData.uClusterChainLength   += uAmountOfAllocatedClusters;
                psNodeRecord->sRecordData.uLastAllocatedCluster = uLastClusAlloc;
            }
        }
        else if ( setAttrs->fa_size < uCurAllocatSize )
        {
            uint64_t uNeedToTruncSize = uCurAllocatSize - setAttrs->fa_size;
            uNeedToTruncSize = ROUND_DOWN( uNeedToTruncSize, (uint64_t)CLUSTER_SIZE(GET_FSRECORD(psNodeRecord)) );
            uint64_t uNeedToTruncClusters = uNeedToTruncSize / CLUSTER_SIZE(GET_FSRECORD(psNodeRecord));

            if ( uNeedToTruncClusters > 0 )
            {
                FAT_Access_M_TruncateLastClusters( psNodeRecord, (uint32_t)uNeedToTruncClusters );
                if ( psNodeRecord->sRecordData.uFirstCluster == 0 )
                {
                    DIROPS_SetStartCluster( GET_FSRECORD(psNodeRecord), &sDirEntry, 0 );
                }
            }
            
            FILERECORD_EvictAllFileChainCacheEntriesFromGivenOffset(psNodeRecord,ROUND_UP(setAttrs->fa_size,(uint64_t)CLUSTER_SIZE(GET_FSRECORD(psNodeRecord))));
        }

        putuint32( sDirEntry.deFileSize, (uNewFileSize & 0xffffffff) );
    }
        
    if ( setAttrs->fa_validmask & UVFS_FA_VALID_ATIME )
    {
        msdosfs_unix2dostime((struct timespec*)&setAttrs->fa_atime, (uint16_t*)sDirEntry.deADate, NULL, NULL);
    }
    
    if ( setAttrs->fa_validmask & UVFS_FA_VALID_MTIME )
    {
        msdosfs_unix2dostime((struct timespec*)&setAttrs->fa_mtime, (uint16_t*)sDirEntry.deMDate, (uint16_t*)sDirEntry.deMTime, NULL);
    }

    if ( setAttrs->fa_validmask & UVFS_FA_VALID_BIRTHTIME )
    {
        msdosfs_unix2dostime((struct timespec*)&setAttrs->fa_birthtime, (uint16_t*)sDirEntry.deCDate, (uint16_t*)sDirEntry.deCTime, NULL);
    }

    if (setAttrs->fa_validmask & UVFS_FA_VALID_BSD_FLAGS)
    {
        iErr = MSDOS_SetFileBSDFlags(setAttrs->fa_bsd_flags, &sDirEntry.deAttributes, IS_DIR( psNodeRecord ));
        if ( iErr != 0 )
        {
            goto exit;
        }
    }
    
    // Read-Modify-Write update dir entry.
    if ( bNeedToUpdateDirEntry )
    {
        iErr = DIROPS_UpdateDirectoryEntry( psNodeRecord, &psNodeRecord->sRecordData.sNDE, &sDirEntry, true );
        if ( iErr != 0 )
        {
            goto exit;
        }
    }
    else
    {
        psNodeRecord->sRecordData.sNDE.sDosDirEntry = sDirEntry;
    }

exit:
    if (iErr == 0)
    {
        MSDOS_GetAtrrFromDirEntry( psNodeRecord, outAttrs );
    }

    return iErr;
}

static int
FileOPS_FillClusterSuffixWithZeros( NodeRecord_s* psNodeRecord, uint32_t uFillFromOffset, uint32_t uCluster )
{

    MSDOS_LOG( LEVEL_DEBUG, "FileOPS_FillClusterSuffixWithZeros = %u %u\n", uFillFromOffset, uCluster);

    int iErr = 0;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psNodeRecord);
    // Read-Modify-Write the last cluster in order to fill is suffix with zeros.
    uint32_t uClusterSize = CLUSTER_SIZE(psFSRecord);
    uint32_t uBytesToKeep = uFillFromOffset;

    // Allocate buffer for cluster.
    uint8_t*  puClusterData = malloc( uClusterSize );
    if ( puClusterData == NULL )
    {
        iErr = ENOMEM;
        goto exit;
    }

    // Read the last cluster.
    size_t uBytesRead = pread( psFSRecord->iFD, puClusterData, uClusterSize, DIROPS_VolumeOffsetForCluster( psFSRecord, uCluster ) );
    if ( uBytesRead != uClusterSize )
    {
        iErr = errno;
        goto exit;
    }

    memset( puClusterData+uBytesToKeep, 0, uClusterSize-uBytesToKeep );

    // Write the last cluster.
    size_t uBytesWrite = pwrite( psFSRecord->iFD, puClusterData, uClusterSize, DIROPS_VolumeOffsetForCluster( psFSRecord, uCluster ) );
    if ( uBytesWrite != uClusterSize )
    {
        iErr = errno;
        goto exit;
    }

exit:
    if ( puClusterData )
        free( puClusterData );

    return iErr;
}

int
MSDOS_GetAttr (UVFSFileNode Node, UVFSFileAttributes *outAttrs)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_GetAttr\n");
    VERIFY_NODE_IS_VALID(Node);

    NodeRecord_s* psNodeRecord      = GET_RECORD(Node);
    
    MultiReadSingleWrite_LockRead( &psNodeRecord->sRecordData.sRecordLck );

    MSDOS_GetAtrrFromDirEntry (psNodeRecord, outAttrs);
    
    MultiReadSingleWrite_FreeRead( &psNodeRecord->sRecordData.sRecordLck );

    return 0;
}

int
MSDOS_SetAttr (UVFSFileNode Node, const UVFSFileAttributes *setAttrs, UVFSFileAttributes *outAttrs)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_SetAttr\n");
    VERIFY_NODE_IS_VALID(Node);
    
    NodeRecord_s* psNodeRecord      = GET_RECORD(Node);

    FSOPS_SetDirtyBitAndAcquireLck(GET_FSRECORD(psNodeRecord));
    MultiReadSingleWrite_LockWrite( &psNodeRecord->sRecordData.sRecordLck );
    
    int iErr = MSDOS_SetAtrrToDirEntry( psNodeRecord, setAttrs, outAttrs, false, true, true, true );
    
    MultiReadSingleWrite_FreeWrite( &psNodeRecord->sRecordData.sRecordLck );
    FSOPS_FlushCacheAndFreeLck(GET_FSRECORD(psNodeRecord));
    return iErr;
}

int
MSDOS_Reclaim (UVFSFileNode Node)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Reclaim\n");
    
    if (Node != NULL)
    {
        VERIFY_NODE_IS_VALID_FOR_RECLAIM(Node);
#ifdef DIAGNOSTIC
        if ( !GET_RECORD(Node)->sRecordData.bRemovedFromDiag )
        {
            DIAGNOSTIC_REMOVE(GET_RECORD(Node));
        }
#endif
        
        FILEOPS_FreeUnusedPreAllocatedClusters(GET_RECORD(Node));
        
        FILERECORD_FreeRecord(GET_RECORD(Node));
    }
    
    return 0;
}

int
MSDOS_ReadLink (UVFSFileNode Node, void *outBuf, size_t bufsize, size_t *actuallyRead, UVFSFileAttributes *outAttrs)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_ReadLink\n");
    VERIFY_NODE_IS_VALID(Node);

    NodeRecord_s* psLinkRecord = GET_RECORD(Node);
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psLinkRecord);
    int iError = 0;
    
    MultiReadSingleWrite_LockRead( &psLinkRecord->sRecordData.sRecordLck );

    // Verify node represent a link
    if (psLinkRecord->sRecordData.eRecordID != RECORD_IDENTIFIER_LINK)
    {
        iError = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ReadLink: fileNode is not a link. Exiting with error [%d]\n",iError);
        goto exit;
    }

    uint32_t uBufferSize = psLinkRecord->sRecordData.uClusterChainLength * CLUSTER_SIZE(psFSRecord);
    void* pvBuffer = malloc(uBufferSize);
    if (pvBuffer == NULL)
    {
        iError = ENOMEM;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ReadLink: Failed to allocate buffer [%d]\n",iError);
        goto exit;
    }
    
    //Need to read the link cluster
    memset(pvBuffer,0,uBufferSize);
    uint32_t uLinkSize = sizeof(struct symlink);
    *actuallyRead = RAWFILE_read(psLinkRecord, 0, uLinkSize, pvBuffer,&iError);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ReadLink: Failed to read link content [%d]\n",iError);
        goto free_buffer;
    }
    
    struct symlink* psLink = (struct symlink*) pvBuffer;
    uint32_t uLinkLength = 0;
    
    if (!DIROPS_VerifyIfLinkAndGetLinkLength(psLink, &uLinkLength))
    {
        iError = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ReadLink: Failed to read link length [%d]\n",iError);
        goto free_buffer;
    }
    
    // Validate that the link length fits to the given buffer length - 1
    // (since we need to set the last char as NUL-terminate)
    if (uLinkLength > bufsize - 1)
    {
        iError = ENOBUFS;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ReadLink: link length larger then given buffer size [%d]\n",iError);
        goto free_buffer;
    }

    *actuallyRead = uLinkLength;
    memcpy(outBuf,psLink->link,*actuallyRead);

    //Set last char as NUL-terminate
    ((char*) outBuf)[(*actuallyRead)] = '\0';
    (*actuallyRead)++;

    //Get attributes
    MSDOS_GetAtrrFromDirEntry(psLinkRecord, outAttrs);

free_buffer:
    free(pvBuffer);
exit:
    MultiReadSingleWrite_FreeRead( &psLinkRecord->sRecordData.sRecordLck );

    return iError;
}

int
MSDOS_Read(UVFSFileNode Node, uint64_t offset, size_t length, void *outBuf, size_t *actuallyRead)
{
    VERIFY_NODE_IS_VALID(Node);
    NodeRecord_s* psFileRecord = GET_RECORD(Node);
    int iError = 0;
    bool locked = true;
    MultiReadSingleWrite_LockRead( &psFileRecord->sRecordData.sRecordLck );

    // Verify node represent a file
    if (psFileRecord->sRecordData.eRecordID != RECORD_IDENTIFIER_FILE)
    {
        if (IS_DIR(psFileRecord))
            iError =  EISDIR;
        else
            iError = EINVAL;

        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Read: read on non-regular file. [%d]", iError);
        goto exit;
    }

    // Asking to read after file size -> return actuallyRead = 0
    if (offset >= getuint32(psFileRecord->sRecordData.sNDE.sDosDirEntry.deFileSize))
    {
        MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Read: Asked to read after file size. Exiting. offset: %llu, file size %u\n",offset, getuint32(psFileRecord->sRecordData.sNDE.sDosDirEntry.deFileSize));

        *actuallyRead = 0;
        goto exit;
    }

    locked = false;
    MultiReadSingleWrite_FreeRead( &psFileRecord->sRecordData.sRecordLck);

    //Read data
    *actuallyRead = RAWFILE_read(psFileRecord, offset, length, outBuf,&iError);

exit:
    if (locked) MultiReadSingleWrite_FreeRead( &psFileRecord->sRecordData.sRecordLck);
    return iError;
}

int
MSDOS_Write (UVFSFileNode Node, uint64_t offset, size_t length, const void *buf, size_t *actuallyWritten)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Write\n");
    VERIFY_NODE_IS_VALID(Node);

    NodeRecord_s* psFileRecord = GET_RECORD(Node);
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psFileRecord);
    uint64_t uLengthToBeWrriten = length;
    int iError = 0;
    UVFSFileAttributes sFileAttrs;


    //Check that the given node is a file
    if (psFileRecord->sRecordData.eRecordID == RECORD_IDENTIFIER_ROOT || psFileRecord->sRecordData.eRecordID == RECORD_IDENTIFIER_DIR)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Given node is a directory. \n");
        return EISDIR;
    }
    else if (psFileRecord->sRecordData.eRecordID != RECORD_IDENTIFIER_FILE)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Given node is not a file. Record ID [%d]\n",psFileRecord->sRecordData.eRecordID);
        return EINVAL;
    }

    if (length == 0)
    {
        MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Write: length == 0. Exiting\n");
        *actuallyWritten  = 0;
        return 0;
    }

    // Validate given arguments
    if (offset + length > DOS_FILESIZE_MAX)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Given length + offset is too big\n");
        return EFBIG;
    }

    bool bLockForWrite = true;
    bool locked = true;
    // Lock the File Node
    FSOPS_SetDirtyBitAndAcquireLck(psFSRecord);
    MultiReadSingleWrite_LockWrite( &psFileRecord->sRecordData.sRecordLck );

    // Remember some values in case the write fails.
    uint64_t uOriginalAllocatedSize = (uint64_t)psFileRecord->sRecordData.uClusterChainLength * (uint64_t)CLUSTER_SIZE(psFSRecord);
    uint32_t uOriginalSize = getuint32(psFileRecord->sRecordData.sNDE.sDosDirEntry.deFileSize);
    uint32_t uOriginalLastCluster = psFileRecord->sRecordData.uLastAllocatedCluster;

    /*
     * If the end of the write will be beyond the current allocated size,
     * then grow the file to accommodate the bytes we want to write.
     * using SetAttr with the new wanted size, the file will be extended
     */
    if (offset + uLengthToBeWrriten > uOriginalAllocatedSize)
    {
        memset(&sFileAttrs,0,sizeof(UVFSFileAttributes));
        sFileAttrs.fa_validmask |= UVFS_FA_VALID_SIZE;
        sFileAttrs.fa_size = offset + uLengthToBeWrriten;

        //SetAttr with size changing and without writing the data into the dir entry
        iError = MSDOS_SetAtrrToDirEntry( psFileRecord, &sFileAttrs, &sFileAttrs, true, false, false, (offset>uOriginalSize) );
        uint64_t uNewAllocatedSize = psFileRecord->sRecordData.uClusterChainLength* (uint64_t)CLUSTER_SIZE(psFSRecord);

        if ( iError )
        {
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Failed to allocate enough clusters for wanted length. Error [%d]\n",iError);
            goto exit;
        }

        // Not even single cluster was allocated or not enougth to even get to the offset
        if ( (uNewAllocatedSize == uOriginalAllocatedSize) || (uNewAllocatedSize <= offset) )
        {
            iError = ENOSPC;
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Failed to allocate enough clusters for wanted length. Error [%d]\n",iError);
            goto exit;
        }

        if (uNewAllocatedSize < offset + uLengthToBeWrriten)
        {
            // Couldn't allocate as much as needed
            // Adjusting the length to write
            MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Write: not enough clusters for wanted length, bytes to write %llu out of %lu. Error\n", uLengthToBeWrriten, length);
            uLengthToBeWrriten = uNewAllocatedSize - offset;
        }

        // Need to zero fill the data between the original file size to the new offset to write from.
        if ( offset > uOriginalSize )
        {
            uint32_t uClusterSize = CLUSTER_SIZE(psFSRecord);
            uint32_t u32Offset = (uint32_t)offset; // Max file size is 4GB-1.
            uint64_t uFromOffset = ROUND_UP( uOriginalSize, uClusterSize );
            uint32_t uNumOfClustersToZeroFill = ((uint32_t) ROUND_UP((u32Offset-uFromOffset),uClusterSize)) / uClusterSize;
            uint32_t uContiguousClusterLength, uWantedCluster;
            while( uNumOfClustersToZeroFill > 0 )
            {
                FILERECORD_GetChainFromCache( psFileRecord, uFromOffset, &uWantedCluster, &uContiguousClusterLength ,&iError );
                if ( iError != 0 )
                {
                    MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Failed to get chain from cache. Error [%d]\n",iError);
                    goto revert_file_size;
                }

                if (uContiguousClusterLength == 0) break;
                
                uint32_t uCurClusNum = MIN( uNumOfClustersToZeroFill, uContiguousClusterLength );

                iError = ZeroFill_Fill( psFSRecord->iFD, DIROPS_VolumeOffsetForCluster( psFSRecord, uWantedCluster ), uCurClusNum*uClusterSize );
                if ( iError != 0 )
                {
                    MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Failed to zero fill. Error [%d]\n",iError);
                    goto revert_file_size;
                }

                uFromOffset += uCurClusNum*uClusterSize;
                uNumOfClustersToZeroFill -= uCurClusNum;
            }
        }
    }
    else if ( offset + uLengthToBeWrriten > uOriginalSize )
    {
        uint32_t u32NewFileSize = (uint32_t)offset + (uint32_t)uLengthToBeWrriten;
        putuint32( psFileRecord->sRecordData.sNDE.sDosDirEntry.deFileSize, u32NewFileSize );

        if ( ( offset > uOriginalSize) && (uOriginalSize % CLUSTER_SIZE(psFSRecord) != 0) )
        {
            uint32_t uBytesToKeep = uOriginalSize % CLUSTER_SIZE(psFSRecord);
            iError = FileOPS_FillClusterSuffixWithZeros( psFileRecord, uBytesToKeep, uOriginalLastCluster );
            if ( iError != 0 )
            {
                MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Failed to Fill Cluster Suffix With Zeros. Error [%d]\n",iError);
                goto revert_file_size;
            }
        }
    }

    locked = false;
    MultiReadSingleWrite_FreeWrite( &psFileRecord->sRecordData.sRecordLck );

    *actuallyWritten = RAWFILE_write(psFileRecord, offset, uLengthToBeWrriten, (void*) buf, &iError);

    // In case the write failed -> return the file to it's original size
    if ( iError || (*actuallyWritten == 0) )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Write: Failed in writing to device. Error [%d], written %lu bytes, wanted %llu\n bytes",iError, *actuallyWritten, uLengthToBeWrriten);
        if ( (!iError) && (length > 0) ) {
            // Probably a bug so we want our layer to report an error
            iError = EIO;
        }
        goto revert_file_size;
    }
    else
    {
        /*
         * If Write passed, Update dir entry if needed
         */
        bLockForWrite = false;
        locked = true;
        //Change Lock to Read
        MultiReadSingleWrite_LockRead( &psFileRecord->sRecordData.sRecordLck );

        if (offset + uLengthToBeWrriten > uOriginalSize)
        {
            iError = DIROPS_UpdateDirectoryEntry( psFileRecord, &psFileRecord->sRecordData.sNDE, &psFileRecord->sRecordData.sNDE.sDosDirEntry, true );
            if (iError)
            {
                MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Write: Failed to update Dir Entry with new size. Error [%d]\n",iError);
                goto revert_file_size;
            }
        }

        //Update last modified time
        iError = FILEOPS_UpdateLastModifiedTime( psFileRecord );
        if (iError)
        {
            MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Write: Failed to update last modified time. Error [%d]\n",iError);
        }
    }

    //Check if all the pre-allocated cluster are used
    if (psFileRecord->sExtraData.sFileData.bIsPreAllocated)
    {
        if (ROUND_UP(getuint32(psFileRecord->sRecordData.sNDE.sDosDirEntry.deFileSize), CLUSTER_SIZE(psFSRecord))/CLUSTER_SIZE(psFSRecord) == psFileRecord->sRecordData.uClusterChainLength)
        {
            psFileRecord->sExtraData.sFileData.bIsPreAllocated = false;
            psFSRecord->uPreAllocatedOpenFiles--;
        }
    }
    
    goto exit;

revert_file_size:

    if (locked) {
        if (!bLockForWrite)
        {
            // Need to revert file size -> Change lock back to write
            MultiReadSingleWrite_FreeRead( &psFileRecord->sRecordData.sRecordLck );
            MultiReadSingleWrite_LockWrite( &psFileRecord->sRecordData.sRecordLck );
            bLockForWrite = true;
        }
    } else {
        MultiReadSingleWrite_LockWrite( &psFileRecord->sRecordData.sRecordLck );
        bLockForWrite = true;
        locked = true;
    }
    
    // Need to check if someone else already wrote after this write operation,
    // if does we can't revert and truncate the file. The file will be filled with 0's
    uint64_t uCurrentSize = getuint32(psFileRecord->sRecordData.sNDE.sDosDirEntry.deFileSize);

    if ( uCurrentSize == offset + uLengthToBeWrriten )
    {
        memset( &sFileAttrs, 0, sizeof(UVFSFileAttributes) );
        sFileAttrs.fa_validmask |= UVFS_FA_VALID_SIZE;
        sFileAttrs.fa_size = uOriginalSize;

        //SetAttr with size changing and without writing the data into the dir entry
        int iReturnToOldSizeError = MSDOS_SetAtrrToDirEntry( psFileRecord, &sFileAttrs, &sFileAttrs, true, false, true, true );
        if (iReturnToOldSizeError)
        {
            MSDOS_LOG( LEVEL_ERROR, "MSDOS_Write: Failed in updating original size. Error [%d]\n", iReturnToOldSizeError );
        }
    }
    
exit:
    if (locked) {
        if (bLockForWrite)
        {
            MultiReadSingleWrite_FreeWrite( &psFileRecord->sRecordData.sRecordLck );
        }
        else
        {
            // Free the file lock
            MultiReadSingleWrite_FreeRead( &psFileRecord->sRecordData.sRecordLck );
        }
    }

    //Unlock dirtybit
    FSOPS_FlushCacheAndFreeLck(psFSRecord);
    return iError;
}

int
MSDOS_Create (UVFSFileNode dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode)
{
    VERIFY_NODE_IS_VALID(dirNode);

    if ( !(attrs->fa_validmask & UVFS_FA_VALID_MODE) )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Create: fa_validmask dosen't have UVFS_FA_VALID_MODE [%d].\n",EINVAL);
        return EINVAL;
    }

    uint64_t uSize = (attrs->fa_validmask & UVFS_FA_VALID_SIZE)
        ? attrs->fa_size : 0;
    
    NodeRecord_s* psParentRecord = GET_RECORD(dirNode);
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psParentRecord);
    int iError = 0;

    iError = DIROPS_CreateHTForDirectory(psParentRecord);
    if (iError)
        return iError;

    // Lookup if file name exist
    RecordIdentifier_e eRecId;
    NodeDirEntriesData_s sNodeDirEntriesData;
    
    // Lock the parent node
    FSOPS_SetDirtyBitAndAcquireLck(psFSRecord);
    MultiReadSingleWrite_LockWrite( &psParentRecord->sRecordData.sRecordLck );

    //Lock DirEntry lock
    MultiReadSingleWrite_LockRead( &GET_FSRECORD(psParentRecord)->sDirEntryAccessLock );
    
    // Lookup for node in the directory
    int iLookupError = DIROPS_LookForDirEntryByName (psParentRecord, name, &eRecId, &sNodeDirEntriesData );
    
    //Unlock DirEntry lock
    MultiReadSingleWrite_FreeRead( &GET_FSRECORD(psParentRecord)->sDirEntryAccessLock );
    
    if ( iLookupError != ENOENT )
    {
        iError = iLookupError ? iLookupError : EEXIST;       
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Create: Lookup For Dir Entry ended with error [%d].\n",iError);
        goto exit;
    }

    /* Check file size not too big */
    if ( uSize > DOS_FILESIZE_MAX )
    {
        iError = EFBIG;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Create: file size  too big [%d].\n",iError);
        goto exit;
    }

    // Allocate clusters for the new file
    uint32_t uAmountOfClustersToAllocate = (uint32_t) (ROUND_UP(uSize, CLUSTER_SIZE(psFSRecord))/CLUSTER_SIZE(psFSRecord)) ;
    uint32_t uAllocatedCluster = 0;
    uint32_t uLastAllocatedCluster = 0;
    uint32_t uAmountOfAllocatedClusters = 0;
    NewAllocatedClusterInfo_s* psNewAllocatedClusterInfoToReturn = NULL;
    if (uAmountOfClustersToAllocate != 0)
    {
        //Allocate only if needed
        iError = FAT_Access_M_AllocateClusters(psFSRecord, uAmountOfClustersToAllocate, 0, &uAllocatedCluster, &uLastAllocatedCluster, &uAmountOfAllocatedClusters, false, true, &psNewAllocatedClusterInfoToReturn, false );
        if (iError)
        {
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Create: Allocate new file clusters ended with error [%d].\n",iError);
            goto exit;
        }
    }
    
    // Create direntry in the parent directory
    iError = DIROPS_CreateNewEntry(psParentRecord, name, attrs, uAllocatedCluster, UVFS_FA_TYPE_FILE);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Create: Create new dir entry for the file ended with error [%d].\n",iError);
        goto free_allocated_clusters;
    }
    
    // Lookup for the new file
    iError = DIROPS_LookupInternal (dirNode, name, outNode, true);
    if (iError)
    {
        MSDOS_LOG((iError == ENOENT) ? LEVEL_DEBUG : LEVEL_ERROR, "MSDOS_Create:New file lookup ended with error [%d].\n",iError);
        goto free_allocated_clusters;
    }

    uint64_t uStartAllocatedOffset = 0;
    NodeRecord_s* psNodeRecord = GET_RECORD(*outNode);
    MultiReadSingleWrite_LockWrite(&psFSRecord->psClusterChainCache->sClusterChainLck);
    while (psNewAllocatedClusterInfoToReturn)
    {
        FILERECORD_UpdateNewAllocatedClustersInChain(psNodeRecord, psNewAllocatedClusterInfoToReturn->uNewAlloctedStartCluster, psNewAllocatedClusterInfoToReturn->uAmountOfConsecutiveClusters, uStartAllocatedOffset);
        
        uStartAllocatedOffset += psNewAllocatedClusterInfoToReturn->uAmountOfConsecutiveClusters * CLUSTER_SIZE(psFSRecord);
        NewAllocatedClusterInfo_s* psClusterInfoToFree = psNewAllocatedClusterInfoToReturn;
        psNewAllocatedClusterInfoToReturn = psNewAllocatedClusterInfoToReturn->psNext;
        
        free(psClusterInfoToFree);
    }
    MultiReadSingleWrite_FreeWrite(&psFSRecord->psClusterChainCache->sClusterChainLck);
    
    DIAGNOSTIC_INSERT(GET_RECORD(*outNode),psParentRecord->sRecordData.uFirstCluster,name);

    goto exit;

free_allocated_clusters:
    if (uAmountOfClustersToAllocate != 0)
    {
        FAT_Access_M_FATChainFree( GET_FSRECORD(psParentRecord), uAllocatedCluster, false );
    }
exit:
    MultiReadSingleWrite_FreeWrite( &psParentRecord->sRecordData.sRecordLck );

    //Unlock dirtybit
    FSOPS_FlushCacheAndFreeLck(psFSRecord);
    //Getting EEXIST here, doesn't require force evict
    bool bReleaseHT = ((iError != 0) && (iError != EEXIST));
    DIROPS_ReleaseHTForDirectory(psParentRecord, bReleaseHT);
    return iError;
}

int
MSDOS_SymLink (UVFSFileNode dirNode, const char *name, const char *contents, const UVFSFileAttributes *attrs, UVFSFileNode *outNode)
{
    VERIFY_NODE_IS_VALID(dirNode);
    if ( !(attrs->fa_validmask & UVFS_FA_VALID_MODE) )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_SymLink: fa_validmask dosen't have UVFS_FA_VALID_MODE [%d].\n",EINVAL);
        return EINVAL;
    }

    NodeRecord_s* psParentRecord = GET_RECORD(dirNode);
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psParentRecord);
    int iError = 0;

    iError = DIROPS_CreateHTForDirectory(psParentRecord);
    if (iError)
        return iError;

    // Lookup if file name exist
    RecordIdentifier_e eRecId;
    NodeDirEntriesData_s sNodeDirEntriesData;
    
    // Lock the parent node
    FSOPS_SetDirtyBitAndAcquireLck(psFSRecord);
    MultiReadSingleWrite_LockWrite( &psParentRecord->sRecordData.sRecordLck );

    //Lock DirEntry lock
    MultiReadSingleWrite_LockRead( &psParentRecord->sRecordData.psFSRecord->sDirEntryAccessLock );
    
    // Lookup for node in the directory
    int iLookupError = DIROPS_LookForDirEntryByName (psParentRecord, name, &eRecId, &sNodeDirEntriesData );
    
    //Unlock DirEntry lock
    MultiReadSingleWrite_FreeRead( &psParentRecord->sRecordData.psFSRecord->sDirEntryAccessLock );
    
    if ( iLookupError != ENOENT )
    {
        if (!iLookupError)
        {
            iError = EEXIST;
        }
        else
        {
            iError = iLookupError;
        }
        if (iError) MSDOS_LOG(LEVEL_ERROR, "MSDOS_SymLink: Lookup For Dir Entry ended with error [%d].\n", iError);
        goto exit;
    }
    
    // Allocate 1 clusters for the new link
    uint32_t uAmountOfClustersToAllocate = ROUND_UP(sizeof(struct symlink),CLUSTER_SIZE(psFSRecord))/CLUSTER_SIZE(psFSRecord);

    uint32_t uAllocatedCluster = 0;
    uint32_t uLastAllocatedCluster = 0;
    uint32_t uAmountOfAllocatedClusters =0;

    iError = FAT_Access_M_AllocateClusters(psFSRecord, uAmountOfClustersToAllocate, 0, &uAllocatedCluster, &uLastAllocatedCluster, &uAmountOfAllocatedClusters, false, true, NULL, false);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_SymLink: Allocate new link cluster ended with error [%d].\n", iError);
        goto exit;
    }
    
    //Fill the contents into the link cluster
    iError = DIROPS_CreateLinkAccordingToContent(psFSRecord,uAllocatedCluster, contents, uAmountOfAllocatedClusters);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_SymLink: Create new dir entry for the link ended with error [%d].\n", iError);
        goto free_allocated_clusters;
    }
    
    // Create direntry in the parent directory
    iError = DIROPS_CreateNewEntry(psParentRecord, name,attrs,uAllocatedCluster,UVFS_FA_TYPE_SYMLINK);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_SymLink: Create new dir entry for the link ended with error [%d].\n", iError);
        goto free_allocated_clusters;
    }
    
    // Lookup for the new file
    iError = DIROPS_LookupInternal( dirNode, name, outNode, true );
    if (iError)
    {
        MSDOS_LOG((iError == ENOENT) ? LEVEL_DEBUG : LEVEL_ERROR, "MSDOS_SymLink:New file lookup ended with error [%d].\n", iError);
        goto free_allocated_clusters;
    }

    DIAGNOSTIC_INSERT(GET_RECORD(*outNode),psParentRecord->sRecordData.uFirstCluster,name);

    goto exit;
    
free_allocated_clusters:
    FAT_Access_M_FATChainFree(psFSRecord, uAllocatedCluster, false);
exit:
    MultiReadSingleWrite_FreeWrite( &psParentRecord->sRecordData.sRecordLck );

    //Unlock dirtybit
    FSOPS_FlushCacheAndFreeLck(psFSRecord);
    //Getting EEXIST here, doesn't require force evict
    bool bReleaseHT = ((iError != 0) && (iError != EEXIST));
    DIROPS_ReleaseHTForDirectory(psParentRecord, bReleaseHT);
    return iError;
}

int
MSDOS_Link(UVFSFileNode fromNode __unused, UVFSFileNode toDirNode __unused, const char *toName __unused, UVFSFileAttributes *outFileAttrs __unused, UVFSFileAttributes *outDirAttrs __unused)
{
	/* No hard links in FAT. */
	return ENOTSUP;
}

enum
{
    RENAME_LOCK_FROM_PARENT  = 0,
    RENAME_LOCK_TO_PARENT,
    RENAME_LOCK_FROM,
    RENAME_LOCK_TO,

    RENAME_LOCK_AMOUNT
};

int
MSDOS_Rename (UVFSFileNode fromDirNode, UVFSFileNode fromNode, const char *fromName, UVFSFileNode toDirNode, UVFSFileNode toNode, const char *toName, uint32_t flags __unused)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Rename\n");
    int iError = 0;
    NodeRecord_s* psLocksNodes[RENAME_LOCK_AMOUNT] = {0};

    NodeRecord_s *psToRecord    = NULL;
    NodeRecord_s *psFromRecord  = NULL;

    VERIFY_NODE_IS_VALID(fromDirNode);
    NodeRecord_s *psFromParentRecord        = GET_RECORD(fromDirNode);
    psLocksNodes[RENAME_LOCK_FROM_PARENT]   = psFromParentRecord;

    VERIFY_NODE_IS_VALID(toDirNode);
    NodeRecord_s *psToParentRecord          = GET_RECORD(toDirNode);
    psLocksNodes[RENAME_LOCK_TO_PARENT]     = psToParentRecord;

    if ( fromNode != NULL )
    {
        VERIFY_NODE_IS_VALID(fromNode);
        psFromRecord                    = GET_RECORD(fromNode);
        psLocksNodes[RENAME_LOCK_FROM]  = psFromRecord;
        if (IS_DIR(psFromRecord))
        {
            //Check if need to create HT for this node
            iError = DIROPS_CreateHTForDirectory(psFromRecord);
            if (iError) return iError;
        }
    }
    if ( toNode != NULL )
    {
        VERIFY_NODE_IS_VALID(toNode);
        psToRecord                      = GET_RECORD(toNode);
        psLocksNodes[RENAME_LOCK_TO]    = psToRecord;
    }

    FileSystemRecord_s *psFSRecord      = GET_FSRECORD(psFromParentRecord);

    UVFSFileAttributes      fileAttrs;
    RecordIdentifier_e      eFromRecId, eToRecId;
    NodeDirEntriesData_s    sFromNodeDirEntriesData, sToNodeDirEntriesData;
    
    uint32_t uFromStartCluster;
    uint32_t uToStartCluster;

    bool bToExists = false;

    iError = DIROPS_CreateHTForDirectory(psFromParentRecord);
    if (iError == 0)
        iError = DIROPS_CreateHTForDirectory(psToParentRecord);
    if (iError)
        return iError;

    // Lock all nodes
    FSOPS_SetDirtyBitAndAcquireLck(psFSRecord);
    FILERECORD_MultiLock(psLocksNodes, RENAME_LOCK_AMOUNT, true, true);

    if (    (fromName == NULL)              ||
            (toName == NULL)                ||
            (strcmp(fromName, ".")  == 0)   ||
            (strcmp(fromName, "..") == 0)   ||
            (strcmp(toName, ".")    == 0)   ||
            (strcmp(toName, "..")   == 0)   )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: Got illegal fromName/toName [%s] [%s]\n", (fromName==NULL? "NULL" : fromName), (toName==NULL? "NULL" : toName) );
        iError = EINVAL;
        goto exit;
    }

    // Lock DirEntryAccess
    MultiReadSingleWrite_LockRead( &psFSRecord->sDirEntryAccessLock );

    // Get 'from' attributes && start cluster && recordId && sFromNodeDirEntriesData.
    if ( psFromRecord != NULL && psFromRecord->sRecordData.bIsNDEValid )
    {
        MSDOS_GetAtrrFromDirEntry(psFromRecord, &fileAttrs);
        uFromStartCluster = psFromRecord->sRecordData.uFirstCluster;
        eFromRecId = psFromRecord->sRecordData.eRecordID;
        memcpy( &sFromNodeDirEntriesData, &psFromRecord->sRecordData.sNDE, sizeof(NodeDirEntriesData_s) );
    }
    else
    {
        // Lookup for node in the directory
        iError = DIROPS_LookForDirEntryByName (psFromParentRecord, fromName, &eFromRecId, &sFromNodeDirEntriesData );
        if ( iError != 0 )
        {
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: 'From' lookup ended with error [%d].\n",iError);
            // Free DirEntry lock
            MultiReadSingleWrite_FreeRead( &psFSRecord->sDirEntryAccessLock );
            goto exit;
        }

        uFromStartCluster = DIROPS_GetStartCluster( psFSRecord, &sFromNodeDirEntriesData.sDosDirEntry );

        // Get file attributes from source
        NodeRecord_s* psTempFromNodeRecord;
        // Allocate record
        iError = FILERECORD_AllocateRecord( &psTempFromNodeRecord, psFSRecord, uFromStartCluster, eFromRecId, &sFromNodeDirEntriesData, fromName );
        if ( iError != 0 )
        {
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename failed to allocate record.\n");
            MultiReadSingleWrite_FreeRead( &psFSRecord->sDirEntryAccessLock );
            goto exit;
        }
        MSDOS_GetAtrrFromDirEntry(psTempFromNodeRecord, &fileAttrs);
        FILERECORD_FreeRecord(psTempFromNodeRecord);
    }

    if ( psToRecord != NULL && psToRecord->sRecordData.bIsNDEValid )
    {
        memcpy( &sToNodeDirEntriesData, &psToRecord->sRecordData.sNDE, sizeof(NodeDirEntriesData_s) );
        eToRecId = psToRecord->sRecordData.eRecordID;
        bToExists = true;
    }
    else
    {
        // Get 'to' info if exist.
        iError =  DIROPS_LookForDirEntryByName (psToParentRecord, toName, &eToRecId, &sToNodeDirEntriesData );
        if ( iError == 0 )
        {
            bToExists = true;
        }
        else
        {
            if ( iError == ENOENT )
            {
                iError = 0;
            }
            else
            {
                MultiReadSingleWrite_FreeRead( &psFSRecord->sDirEntryAccessLock );
                MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: DIROPS_LookForDirEntry of 'to' returned an error [%d].\n",iError);
                goto exit;
            }
        }
    }

    // Free DirEntryAccess
    MultiReadSingleWrite_FreeRead( &psFSRecord->sDirEntryAccessLock );

    if ( bToExists == true ) 
    {
        // In case 'To' is found, ensure the objects are compatible
        if ( eFromRecId == eToRecId )
        {
           if ( (eFromRecId != RECORD_IDENTIFIER_DIR) && (eFromRecId != RECORD_IDENTIFIER_FILE) && (eFromRecId != RECORD_IDENTIFIER_LINK) )
           {
              MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: Invalid objects ('From' is %d 'To' is %d).\n", eFromRecId, eToRecId);
              iError = EINVAL;
              goto exit;
           }
        }
        else // ( eFromRecId != eToRecId )
        {
           if (eFromRecId == RECORD_IDENTIFIER_DIR)
           {
              MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: 'To' is not a directory [eToRecId %d].\n",eToRecId);
              iError = ENOTDIR;
              goto exit;
           }
           else if ( eToRecId == RECORD_IDENTIFIER_DIR)
           {
              MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: 'To' is a directory [eFromRecId %d].\n",eFromRecId);
              iError = EISDIR;
              goto exit;
           }
           else if (((eFromRecId != RECORD_IDENTIFIER_FILE) && (eFromRecId != RECORD_IDENTIFIER_LINK)) || ((eToRecId != RECORD_IDENTIFIER_FILE) && (eToRecId != RECORD_IDENTIFIER_LINK)))
           {// eFromRecId || eToRecId are not files or links
               MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: Invalid objects ('From' is %d 'To' is %d).\n", eFromRecId, eToRecId);
               iError = EINVAL;
               goto exit;
           }
        }

        if ( (psFromParentRecord == psToParentRecord) && (strcmp( toName, fromName ) == 0) )
        {
            MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Rename: 'To' and 'From' are the same");
            // Nothing to do if names are the same
            goto exit;
        }

        uToStartCluster = DIROPS_GetStartCluster( psFSRecord, &sToNodeDirEntriesData.sDosDirEntry );

        // In case of directory rename - check that destination directory is empty
        if ( eToRecId == RECORD_IDENTIFIER_DIR )
        {
            NodeRecord_s* psTempToNode = NULL;

            if ( psToRecord == NULL )
            {
                // Allocate record
                iError = FILERECORD_AllocateRecord( &psTempToNode, psFSRecord, uToStartCluster, eToRecId, &sToNodeDirEntriesData, toName );
                if ( iError != 0 )
                {
                    MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename failed to allocate record.\n");
                    goto exit;
                }
            }
            else
            {
                psTempToNode = psToRecord;
            }

            // Lock DirEntry lock
            MultiReadSingleWrite_LockRead( &psFSRecord->sDirEntryAccessLock );

            iError = DIROPS_isDirEmpty(psTempToNode);

            // Free DirEntry lock
            MultiReadSingleWrite_FreeRead( &psFSRecord->sDirEntryAccessLock );

            //If we allocated, need to free
            if ( psToRecord == NULL )
                FILERECORD_FreeRecord(psTempToNode);

            if ( iError )
            {
                MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: DIROPS_isDirEmpty returned error %d.\n",iError);
                goto exit;
            }
        }

        if ( psToRecord != NULL )
        {
            INVALIDATE_NODE(psToRecord);
#ifdef DIAGNOSTIC
            DIAGNOSTIC_REMOVE(psToRecord);
            psToRecord->sRecordData.bRemovedFromDiag = true;
#endif
        }

        // Update directory version.
        psToParentRecord->sExtraData.sDirData.uDirVersion++;

        // Mark all node directory entries as deleted
        // Lock DirEntry lock
        MultiReadSingleWrite_LockWrite( &psFSRecord->sDirEntryAccessLock );

        iError = DIROPS_MarkNodeDirEntriesAsDeleted( psToParentRecord, &sToNodeDirEntriesData, toName );

        // Free DirEntry lock
        MultiReadSingleWrite_FreeWrite( &psFSRecord->sDirEntryAccessLock );
        if (iError)
        {
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: unable to remove 'to' file / record (iError %d).\n",iError);
            goto exit;
        }

        // Release clusters.
        if ( uToStartCluster != 0 )
        {
            iError = FAT_Access_M_FATChainFree( psFSRecord, uToStartCluster, false );
            if ( iError != 0 )
            {
                MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: FAT Chain Free ended with error [%d].\n", iError);
                goto exit;
            }
        }
    }

    // Perform the switch
    // write new entry in destination directory
    fileAttrs.fa_validmask &= ~READ_ONLY_FA_FIELDS;
    iError = DIROPS_CreateNewEntry(psToParentRecord, toName, &fileAttrs, uFromStartCluster, puRecordId2FaType[eFromRecId]);
    if ( iError )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: unable to create new file / directory entry in destination dir (%d).\n",iError);
        goto exit;
    }

    // Update the necessaries fields of 'from' node if exist.
    if ( psFromRecord != NULL )
    {
        // Get the new direcory entries
        MultiReadSingleWrite_LockRead( &psFSRecord->sDirEntryAccessLock );
        NodeDirEntriesData_s sNewEnries;
        iError = DIROPS_LookForDirEntryByName (psToParentRecord, toName, NULL, &sNewEnries );
        MultiReadSingleWrite_FreeRead( &psFSRecord->sDirEntryAccessLock );

        if ( iError != 0 )
        {
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: fail to lookup for new dir record [%d].\n",iError);
            goto exit;
        }

        // Copy dir entries to 'from' node.
        memcpy( &psFromRecord->sRecordData.sNDE, &sNewEnries, sizeof(NodeDirEntriesData_s) );
        psFromRecord->sRecordData.bIsNDEValid = true;

        //Copy the new Name
        if (psFromRecord->sRecordData.pcUtf8Name)
        {
            free(psFromRecord->sRecordData.pcUtf8Name);
            psFromRecord->sRecordData.pcUtf8Name = NULL;
        }
        CONV_DuplicateName(&psFromRecord->sRecordData.pcUtf8Name, toName);

        DIAGNOSTIC_REMOVE(psFromRecord);
        DIAGNOSTIC_INSERT(psFromRecord, psToParentRecord->sRecordData.uFirstCluster, toName);
    }

    // remove old entry from source directory
    // Lock DirEntry lock
    MultiReadSingleWrite_LockWrite( &psFSRecord->sDirEntryAccessLock );

    iError = DIROPS_MarkNodeDirEntriesAsDeleted( psFromParentRecord, &sFromNodeDirEntriesData, fromName);

    //
    // WE ARE NOW COMMITTED TO THE RENAME -- IT CAN NO LONGER FAIL, UNLESS WE
    // UNWIND THE ENTIRE TRANSACTION.
    //

    // In case of directory rename - we need to update '..' with the new parent cluster
    if ( (eFromRecId == RECORD_IDENTIFIER_DIR) && (psFromParentRecord != psToParentRecord) )
    {
        uint32_t uToParentCluster = 0;
        if ( psToParentRecord->sRecordData.bIsNDEValid )
        {
            uToParentCluster = DIROPS_GetStartCluster(psFSRecord, &psToParentRecord->sRecordData.sNDE.sDosDirEntry);
        }

        NodeRecord_s* psNewDirRecord = NULL;
        if ( psFromRecord != NULL )
        {
            psNewDirRecord = psFromRecord;
        }
        else
        {
            UVFSFileNode sNewDirNode;
            iError = DIROPS_LookupInternal( psToParentRecord, toName, &sNewDirNode, false );
            if ( iError != 0 )
            {
                MultiReadSingleWrite_FreeWrite( &psFSRecord->sDirEntryAccessLock );
                MSDOS_LOG((iError == ENOENT) ? LEVEL_DEBUG : LEVEL_ERROR, "MSDOS_Rename: fail to lookup for new dir record %d).\n",iError);
                goto exit;
            }
            psNewDirRecord = GET_RECORD(sNewDirNode);
        }

        // Get the '..' entry of 'To' Directory
        NodeDirEntriesData_s sDotDotDirEntry;
        iError = DIROPS_LookForDirEntryByName (psNewDirRecord, "..", &eToRecId, &sDotDotDirEntry );
        if ( psFromRecord == NULL )
        {
            FILERECORD_FreeRecord( psNewDirRecord );
        }
        if ( iError != 0 )
        {
            MultiReadSingleWrite_FreeWrite( &psFSRecord->sDirEntryAccessLock );
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: Look for '..' entry return error %d).\n",iError);
            goto exit;
        }

        DIROPS_SetStartCluster( psFSRecord, &sDotDotDirEntry.sDosDirEntry, uToParentCluster );

        NodeRecord_s* psDotDot;
        // Allocate record
        iError = FILERECORD_AllocateRecord( &psDotDot, psFSRecord, uToParentCluster, eToRecId, &sDotDotDirEntry, NULL );
        if ( iError != 0 )
        {
            MultiReadSingleWrite_FreeWrite( &psFSRecord->sDirEntryAccessLock );
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: Fail to allocate record for '..' entry %d).\n", iError);
            goto exit;
        }

        iError = DIROPS_UpdateDirectoryEntry( psDotDot, &sDotDotDirEntry, &sDotDotDirEntry.sDosDirEntry, false );
        FILERECORD_FreeRecord( psDotDot );
        if ( iError != 0 )
        {
            MultiReadSingleWrite_FreeWrite( &psFSRecord->sDirEntryAccessLock );
            MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: Fail to update '..' entry %d).\n", iError);
            goto exit;
        }
    }

    // Free DirEntry lock
    MultiReadSingleWrite_FreeWrite( &psFSRecord->sDirEntryAccessLock );

    //Update Directory version
    psFromParentRecord->sExtraData.sDirData.uDirVersion++;

    if ( iError )
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_Rename: unable to remove old file / directory entry (DIROPS_MarkNodeDirEntriesAsDeleted returned %d).\n",iError);
        goto exit;
    }
    
exit:
    // Release all nodes locks
    FILERECORD_MultiLock(psLocksNodes, RENAME_LOCK_AMOUNT, true, false);
    //Unlock dirtybit
    FSOPS_FlushCacheAndFreeLck(psFSRecord);
    DIROPS_ReleaseHTForDirectory(psFromParentRecord, (iError != 0));
    DIROPS_ReleaseHTForDirectory(psToParentRecord, (iError != 0));
    return iError;
}

int
FILEOPS_PreAllocateClusters(NodeRecord_s* psNodeRecord, LIFilePreallocateArgs_t* psPreAllocReq, LIFilePreallocateArgs_t* psPreAllocRes)
{
    FileSystemRecord_s *psFSRecord      = GET_FSRECORD(psNodeRecord);
    int iErr = 0;
    psPreAllocRes->bytesallocated = 0;
    /* Cannot change size of a directory or symlink! */
    if ( IS_DIR(psNodeRecord) || IS_SYMLINK(psNodeRecord) )
    {
        MSDOS_LOG(LEVEL_ERROR, "FILEOPS_PreAllocateClusters: Cannot change size of a directory or symlink\n");
        return EPERM;
    }
    
    if (psPreAllocReq->flags & LI_PREALLOCATE_ALLOCATEFROMVOL){
        MSDOS_LOG(LEVEL_ERROR, "FILEOPS_PreAllocateClusters: Not supporting LI_PREALLOCATE_ALLOCATEFROMVOL mode\n");
        return ENOTSUP;
    }
    
    if (psPreAllocReq->offset != 0) {
        MSDOS_LOG(LEVEL_ERROR, "FILEOPS_PreAllocateClusters: offset given wasn't 0 -  %lld\n", psPreAllocReq->offset);
        return EINVAL;
    }
    
    FSOPS_SetDirtyBitAndAcquireLck(psFSRecord);
    MultiReadSingleWrite_LockWrite( &psNodeRecord->sRecordData.sRecordLck );
    /* Update file size */
    uint64_t uClusterSize = CLUSTER_SIZE(psFSRecord);
    uint64_t uCurAllocatSize = psNodeRecord->sRecordData.uClusterChainLength * uClusterSize;
    uint64_t uSizeToPreAllocate = psPreAllocReq->length + uCurAllocatSize;
    bool bPartialAllocationAllowed = !(psPreAllocReq->flags & LI_PREALLOCATE_ALLOCATEALL);
    bool bMustContiguousAllocation = psPreAllocReq->flags & LI_PREALLOCATE_ALLOCATECONTIG;
    uint32_t uFirstNewClusAlloc = 0;
    uint32_t uLastClusAlloc     = 0;
    uint32_t uAmountOfAllocatedClusters = 0;
    
    /* Check file size not too big */
    if ( uSizeToPreAllocate > DOS_FILESIZE_MAX )
    {
        iErr = EFBIG;
        goto exit;
    }
    
    // If we already have enough allocted clusters
    if ( uSizeToPreAllocate <= uCurAllocatSize || uSizeToPreAllocate == 0)
    {
        goto exit;
    }
    
    struct dosdirentry *psDirEntry  = &psNodeRecord->sRecordData.sNDE.sDosDirEntry;

    uint64_t uNeedToAllocSize = uSizeToPreAllocate - uCurAllocatSize;
    uNeedToAllocSize = ROUND_UP( uNeedToAllocSize, uClusterSize );
    uint32_t uNeedToAllocClusters = (uint32_t) (uNeedToAllocSize / uClusterSize);

    if ( uNeedToAllocClusters > 0 )
    {
        iErr = FAT_Access_M_AllocateClusters( psFSRecord, (uint32_t)uNeedToAllocClusters, psNodeRecord->sRecordData.uLastAllocatedCluster, &uFirstNewClusAlloc, &uLastClusAlloc, &uAmountOfAllocatedClusters, bPartialAllocationAllowed, false, NULL, bMustContiguousAllocation);
        if ( iErr != 0 && !(bPartialAllocationAllowed && iErr == ENOSPC) )
        {
            goto exit;
        }

        assert(!((uAmountOfAllocatedClusters* CLUSTER_SIZE(psFSRecord) < uNeedToAllocSize) && (bPartialAllocationAllowed && iErr != ENOSPC)));

        if ((uAmountOfAllocatedClusters* CLUSTER_SIZE(psFSRecord) < uNeedToAllocSize) && (bPartialAllocationAllowed && iErr == ENOSPC))
        {
            //If didn't allocate any cluster
            if (uAmountOfAllocatedClusters == 0)
            {
                goto exit;
            }
            else
            {
                //Cleaning error
                iErr = 0;
            }
        }

        if ( psNodeRecord->sRecordData.uFirstCluster == 0 )
        {
            psNodeRecord->sRecordData.uFirstCluster = uFirstNewClusAlloc;
            DIROPS_SetStartCluster( GET_FSRECORD(psNodeRecord), psDirEntry, uFirstNewClusAlloc );
        }
        psNodeRecord->sRecordData.uClusterChainLength   += uAmountOfAllocatedClusters;
        psNodeRecord->sRecordData.uLastAllocatedCluster = uLastClusAlloc;
    }

    iErr = DIROPS_UpdateDirectoryEntry( psNodeRecord, &psNodeRecord->sRecordData.sNDE, psDirEntry, true );

    psNodeRecord->sExtraData.sFileData.bIsPreAllocated = true;
    psFSRecord->uPreAllocatedOpenFiles++;
    
exit:
    psPreAllocRes->bytesallocated = uAmountOfAllocatedClusters * uClusterSize;
    MultiReadSingleWrite_FreeWrite( &psNodeRecord->sRecordData.sRecordLck );
    //Unlock dirtybit
    FSOPS_FlushCacheAndFreeLck(psFSRecord);
    
    return iErr;
}


void FILEOPS_FreeUnusedPreAllocatedClusters(NodeRecord_s* psNodeRecord)
{
    if (IS_DIR(psNodeRecord) || IS_SYMLINK(psNodeRecord))
        return;
    
    if (!psNodeRecord->sExtraData.sFileData.bIsPreAllocated)
        return;
    
    FileSystemRecord_s *psFSRecord = GET_FSRECORD(psNodeRecord);
    bool bNeedToUpdateDirEntry = false;
    
    FSOPS_SetDirtyBitAndAcquireLck(psFSRecord);
    MultiReadSingleWrite_LockWrite( &psNodeRecord->sRecordData.sRecordLck );
    
    struct dosdirentry *psDirEntry  = &psNodeRecord->sRecordData.sNDE.sDosDirEntry;
    
    uint64_t uUsedClusters = ROUND_UP(getuint32(psDirEntry->deFileSize), CLUSTER_SIZE(psFSRecord))/CLUSTER_SIZE(psFSRecord);
    uint64_t uAllocatedClusters = psNodeRecord->sRecordData.uClusterChainLength;
    if (uAllocatedClusters > uUsedClusters)
    {
        
        int error = FAT_Access_M_TruncateLastClusters( psNodeRecord, (uint32_t)(uAllocatedClusters - uUsedClusters) );
        if (error)
        {
            MSDOS_LOG(LEVEL_ERROR, "FILEOPS_FreeUnusedPreAllocatedClusters: failed to evict clusters, will be removed during fsck\n");
            goto exit;
        }
        
        if ( psNodeRecord->sRecordData.uFirstCluster == 0 )
        {
            DIROPS_SetStartCluster( psFSRecord, psDirEntry, 0 );
            bNeedToUpdateDirEntry = true;
        }

        FILERECORD_EvictAllFileChainCacheEntriesFromGivenOffset(psNodeRecord, uUsedClusters * CLUSTER_SIZE(psFSRecord));

        // Read-Modify-Write update dir entry.
        if ( bNeedToUpdateDirEntry )
        {
            error = DIROPS_UpdateDirectoryEntry( psNodeRecord, &psNodeRecord->sRecordData.sNDE, psDirEntry, true );
            if (error)
            {
                MSDOS_LOG(LEVEL_ERROR, "FILEOPS_FreeUnusedPreAllocatedClusters: failed to update dir entry, will be fixed during fsck\n");
                goto exit;
            }
        }
        
        psFSRecord->uPreAllocatedOpenFiles--;
    }

exit:
    //Unlock dirtybit
    MultiReadSingleWrite_FreeWrite( &psNodeRecord->sRecordData.sRecordLck );
    FSOPS_FlushCacheAndFreeLck(psFSRecord);
}
