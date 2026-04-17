/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_raw_read_write.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 19/3/18.
 */

#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_fsops_handler.h"
#include "lf_hfs_file_mgr_internal.h"
#include "lf_hfs_file_extent_mapping.h"
#include "lf_hfs_vfsutils.h"
#include <UserFS/UserVFS.h>

#define MAX_READ_WRITE_LENGTH (0x7ffff000)

#define ZERO_BUF_SIZE   (1024*1024)

static void* gpvZeroBuf = NULL;


int
raw_readwrite_get_cluster_from_offset( vnode_t psVnode, uint64_t uWantedOffset, uint64_t* puStartCluster, uint64_t* puInClusterOffset, uint64_t* puContigousClustersInBytes )
{
    uint32_t uSectorsInCluster  = HFSTOVCB(psVnode->sFSParams.vnfs_mp->psHfsmount)->blockSize / HFSTOVCB(psVnode->sFSParams.vnfs_mp->psHfsmount)->hfs_physical_block_size;

    uint64_t uStartSector  = 0;
    size_t uAvailableBytes  = 0;

    int iErr = MapFileBlockC( HFSTOVCB(psVnode->sFSParams.vnfs_mp->psHfsmount), VTOF(psVnode), MAX_READ_WRITE_LENGTH, uWantedOffset, (daddr64_t*)&uStartSector, &uAvailableBytes );
    if ( iErr != 0 )
    {
        return iErr;
    }

    if (puStartCluster) {
        *puStartCluster = uStartSector / uSectorsInCluster;
    }
    
    if (puInClusterOffset) {
        *puInClusterOffset = (uStartSector % uSectorsInCluster) * HFSTOVCB(psVnode->sFSParams.vnfs_mp->psHfsmount)->hfs_physical_block_size;
    }
    
    if (puContigousClustersInBytes) {
        *puContigousClustersInBytes = uAvailableBytes;
    }
    return iErr;
}

errno_t raw_readwrite_read_mount( vnode_t psMountVnode, uint64_t uBlockN, uint64_t uClusterSize, void* pvBuf, uint64_t uBufLen, uint64_t *puActuallyRead, uint64_t* puReadStartCluster ) {
    int iErr                    = 0;
    int iFD                     = VNODE_TO_IFD(psMountVnode);
    uint64_t uWantedOffset      = uBlockN * uClusterSize;

    if (puReadStartCluster) 
       *puReadStartCluster = uBlockN;

    hfs_assert( uBufLen >= uClusterSize );

    ssize_t iReadBytes = pread(iFD, pvBuf, uBufLen, uWantedOffset);
    if ( iReadBytes != (ssize_t)uBufLen )
    {
        iErr = ( (iReadBytes < 0) ? errno : EIO );
        HFSLogLevel_e eLogLevel = (VNODE_TO_UNMOUNT_HINT(psMountVnode)==UVFSUnmountHintForce)?LEVEL_DEBUG:LEVEL_ERROR;
        LFHFS_LOG( eLogLevel, "raw_readwrite_read_mount failed [%d]\n", iErr );
    }

    if (puActuallyRead)
       *puActuallyRead = iReadBytes;

    return iErr;
}

errno_t raw_readwrite_write_mount( vnode_t psMountVnode, uint64_t uBlockN, uint64_t uClusterSize, void* pvBuf, uint64_t uBufLen, uint64_t *piActuallyWritten, uint64_t* puWriteStartCluster ) {
    int iErr                   = 0;
    int iFD                    = VNODE_TO_IFD(psMountVnode);
    uint64_t uWantedOffset     = uBlockN * uClusterSize;
    ssize_t  uActuallyWritten  = 0;

    if (puWriteStartCluster)
        *puWriteStartCluster = uBlockN;

    hfs_assert( uBufLen >= uClusterSize );

    uActuallyWritten = pwrite(iFD, pvBuf, (size_t)uBufLen, uWantedOffset);
    if ( uActuallyWritten != (ssize_t)uBufLen ) {
        iErr = ( (uActuallyWritten < 0) ? errno : EIO );
        HFSLogLevel_e eLogLevel = (VNODE_TO_UNMOUNT_HINT(psMountVnode)==UVFSUnmountHintForce)?LEVEL_DEBUG:LEVEL_ERROR;
        LFHFS_LOG( eLogLevel, "raw_readwrite_write_mount failed [%d]\n", iErr );
    }

    if (piActuallyWritten)
        *piActuallyWritten = uActuallyWritten;

    return iErr;
}

errno_t
raw_readwrite_read( vnode_t psVnode, uint64_t uOffset, void* pvBuf, uint64_t uLength, size_t *piActuallyRead, uint64_t* puReadStartCluster )
{
    errno_t iErr                    = 0;
    uint64_t uClusterSize           = psVnode->sFSParams.vnfs_mp->psHfsmount->blockSize;
    uint64_t uFileSize              = ((struct filefork *)VTOF(psVnode))->ff_data.cf_blocks * uClusterSize;
    uint64_t uActuallyRead          = 0;
    bool bFirstLoop                 = true;

    *piActuallyRead = 0;
    while ( *piActuallyRead < uLength )
    {
        uint64_t uCurrentCluster            = 0;
        uint64_t uInClusterOffset           = 0;
        uint64_t uContigousClustersInBytes  = 0;

        // Look for the location to read
        iErr = raw_readwrite_get_cluster_from_offset( psVnode, uOffset, &uCurrentCluster, &uInClusterOffset, &uContigousClustersInBytes );
        if ( iErr != 0 )
        {
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_read: raw_readwrite_get_cluster_from_offset failed [%d]\n", iErr );
            return iErr;
        }

        if ( bFirstLoop )
        {
            bFirstLoop = false;
            if (puReadStartCluster)
                *puReadStartCluster = uCurrentCluster;
        }

        // Stop reading if we've reached the end of the file
        if ( (uContigousClustersInBytes == 0) || (uOffset >= uFileSize) )
        {
            break;
        }

        uint64_t uBytesToRead = MIN(uFileSize - uOffset, uLength - *piActuallyRead);

        // Read data
        iErr = raw_readwrite_read_internal( psVnode, uCurrentCluster, uContigousClustersInBytes, uOffset, uBytesToRead, pvBuf, &uActuallyRead );
        if ( iErr != 0 )
        {
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_read_internal: raw_readwrite_read_internal failed [%d]\n", iErr );
            return iErr;
        }

        // Update the amount of bytes alreay read
        *piActuallyRead += uActuallyRead;
        // Update file offset
        uOffset += uActuallyRead;
        // Update buffer offset
        pvBuf = (uint8_t*)pvBuf + uActuallyRead;
    }

    return iErr;
}

errno_t
raw_readwrite_read_internal( vnode_t psVnode, uint64_t uCluster, uint64_t uContigousClustersInBytes,
                            uint64_t uOffset, uint64_t uBytesToRead, void* pvBuf, uint64_t *piActuallyRead )
{
    errno_t iErr                    = 0;
    int iFD                         = VNODE_TO_IFD(psVnode);
    struct hfsmount *hfsmp          = VTOHFS(psVnode);
    uint64_t uClusterSize           = hfsmp->blockSize;
    uint64_t uSectorSize            = hfsmp->hfs_logical_block_size;
    uint64_t uBytesToCopy           = 0;

    // Calculate offset - offset by sector and need to add the offset by sector
    uint64_t uReadOffset = FSOPS_GetOffsetFromClusterNum( psVnode, uCluster ) + ( ROUND_DOWN(uOffset, uSectorSize)  % uClusterSize );

    // If offset not align to sector size, need to read only 1 sector and memcpy its end
    if ( (uOffset % uSectorSize) != 0 )
    {
        void* pvBuffer = hfs_malloc(uSectorSize);
        if (pvBuffer == NULL)
        {
            return ENOMEM;
        }

        uint64_t uInSectorOffset = uOffset % uSectorSize;
        uBytesToCopy = MIN(uSectorSize - uInSectorOffset, uBytesToRead);

        // Read the content of the file
        ssize_t iReadBytes = pread( iFD, pvBuffer, uSectorSize, uReadOffset );
        if ( iReadBytes != (ssize_t)uSectorSize )
        {
            iErr = ((iReadBytes < 0) ? errno : EIO);
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_read: pread failed to read wanted length\n" );
            hfs_free(pvBuffer);
            return iErr;
        }
        memcpy( (uint8_t *)pvBuf, (uint8_t *)pvBuffer+uInSectorOffset, uBytesToCopy );
        hfs_free(pvBuffer);
    }
    // If uBytesToRead < uClusterSize, need to read 1 sector and memcpy the begining
    else if (uBytesToRead < uSectorSize)
    {
        void* pvBuffer = hfs_malloc(uSectorSize);
        if (pvBuffer == NULL)
        {
            return ENOMEM;
        }

        uBytesToCopy = uBytesToRead;

        // Read the content of the file
        ssize_t iReadBytes = pread( iFD, pvBuffer, uSectorSize, uReadOffset );
        if ( iReadBytes != (ssize_t)uSectorSize )
        {
            iErr = ((iReadBytes < 0) ? errno : EIO);
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_read: pread failed to read wanted length\n" );
            hfs_free(pvBuffer);
            return iErr;
        }

        memcpy((uint8_t *)pvBuf, pvBuffer, uBytesToCopy);
        hfs_free(pvBuffer);
    }
    // Can read buffer size chunk
    else
    {
        uint64_t uAvailSectors     = uContigousClustersInBytes / uSectorSize;
        uint64_t uRemainingSectors = uBytesToRead              / uSectorSize;

        uBytesToCopy = MIN(uAvailSectors, uRemainingSectors) * uSectorSize;
        uBytesToCopy = MIN( uBytesToCopy, MAX_READ_WRITE_LENGTH );

        assert( (uBytesToCopy % uSectorSize) == 0 );
        assert( (uReadOffset  % uSectorSize) == 0 );

        ssize_t iReadBytes = pread( iFD,(uint8_t *)pvBuf, (size_t)uBytesToCopy, uReadOffset ) ;
        if ( iReadBytes != (ssize_t)uBytesToCopy )
        {
            iErr = ((iReadBytes < 0) ? errno : EIO);
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_read: pread failed to read wanted length\n" );
            return iErr;
        }
    }

    // Update the amount of bytes alreay read
    *piActuallyRead = uBytesToCopy;

    return iErr;
}

errno_t
raw_readwrite_write( vnode_t psVnode, uint64_t uOffset, void* pvBuf, uint64_t uLength, uint64_t *piActuallyWritten )
{
    errno_t iErr                    = 0;
    uint64_t uClusterSize           = psVnode->sFSParams.vnfs_mp->psHfsmount->blockSize;
    uint64_t uFileSize              = ((struct filefork *)VTOF(psVnode))->ff_data.cf_blocks * uClusterSize;
    uint64_t uActuallyWritten       = 0;

    *piActuallyWritten = 0;

    // Fill the buffer until the buffer is full or till the end of the file
    while ( *piActuallyWritten < uLength )
    {
        uint64_t uCurrentCluster            = 0;
        uint64_t uInClusterOffset           = 0;
        uint64_t uContigousClustersInBytes  = 0;

        iErr = raw_readwrite_get_cluster_from_offset(psVnode, uOffset, &uCurrentCluster, &uInClusterOffset, &uContigousClustersInBytes );
        if ( iErr != 0 )
        {
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_write: raw_readwrite_get_cluster_from_offset failed [%d]\n", iErr );
            return iErr;
        }

        // Stop writing if we've reached the end of the file
        if ( (uContigousClustersInBytes == 0) || (uOffset >= uFileSize) )
        {
            break;
        }

        /* Calculate how many bytes are still missing to add to the device
         * If offset near end of file need to set only (uFileSize - uOffset)
         * else need to write as much as left (uLength - uAcctuallyRead)
         */
        uint64_t uBytesToWrite = MIN(uFileSize - uOffset, uLength - *piActuallyWritten);

        // Write data
        iErr = raw_readwrite_write_internal( psVnode, uCurrentCluster, uContigousClustersInBytes, uOffset, uBytesToWrite, pvBuf, &uActuallyWritten );
        if ( iErr != 0 )
        {
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_read_internal: raw_readwrite_read_internal failed [%d]\n", iErr );
            return iErr;
        }

        // Update the amount of bytes alreay written
        *piActuallyWritten += uActuallyWritten;
        // Update file offset
        uOffset += uActuallyWritten;
        // Update buffer offset
        pvBuf = (uint8_t*)pvBuf + uActuallyWritten;
    }

    return iErr;
}

errno_t
raw_readwrite_write_internal( vnode_t psVnode, uint64_t uCluster, uint64_t uContigousClustersInBytes,
                              uint64_t uOffset, uint64_t uBytesToWrite, void* pvBuf, uint64_t *piActuallyWritten )
{
    errno_t iErr                    = 0;
    int iFD                         = VNODE_TO_IFD(psVnode);
    struct hfsmount *hfsmp          = VTOHFS(psVnode);
    uint64_t uClusterSize           = hfsmp->blockSize;
    uint64_t uSectorSize            = hfsmp->hfs_logical_block_size;
    uint64_t uBytesToCopy           = 0;

    // Calculate offset - offset by sector and need to add the offset by sector
    uint64_t uWriteOffset = FSOPS_GetOffsetFromClusterNum( psVnode, uCluster ) + ( ROUND_DOWN(uOffset, uSectorSize)  % uClusterSize );

    // If offset not align to sector size, need to read the existing data
    //  memcpy it's beginning and write back to the device
    if ( (uOffset % uSectorSize) != 0 )
    {
        void* pvBuffer = hfs_malloc(uSectorSize);
        if (pvBuffer == NULL)
        {
            return ENOMEM;
        }

        uint64_t uInSectorOffset = uOffset % uSectorSize;
        uBytesToCopy             = MIN( uBytesToWrite, uSectorSize - uInSectorOffset );

        // Read the content of the existing file
        ssize_t iReadBytes = pread(iFD, pvBuffer, uSectorSize, uWriteOffset);
        if ( iReadBytes != (ssize_t)uSectorSize )
        {
            iErr = (iReadBytes < 0) ? errno : EIO;
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_write: pread failed to read wanted length\n" );
            hfs_free(pvBuffer);
            return iErr;
        }

        // memcpy the data from the given buffer
        memcpy((uint8_t *)pvBuffer+uInSectorOffset, pvBuf, uBytesToCopy);

        // Write the data into the device
        ssize_t iWriteBytes = pwrite(iFD, pvBuffer, uSectorSize, uWriteOffset);
        if ( iWriteBytes != (ssize_t)uSectorSize )
        {
            iErr = (iWriteBytes < 0) ? errno : EIO;
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_write: pwrite failed to write wanted length\n" );
            hfs_free(pvBuffer);
            return iErr;
        }

        hfs_free(pvBuffer);
    }
    // If uBytesToWrite < uSectorSize, need to R/M/W 1 sector.
    else if ( uBytesToWrite < uSectorSize )
    {
        void* pvBuffer = hfs_malloc(uSectorSize);
        if (pvBuffer == NULL)
        {
            return ENOMEM;
        }

        uBytesToCopy = uBytesToWrite;

        // Read the content of the existing file
        ssize_t iReadBytes = pread(iFD, pvBuffer, uSectorSize, uWriteOffset);
        if ( iReadBytes != (ssize_t)uSectorSize )
        {
            iErr = (iReadBytes < 0) ? errno : EIO;
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_write: pread failed to read wanted length\n" );
            hfs_free(pvBuffer);
            return iErr;
        }

        // memcpy the last data
        memcpy(pvBuffer, (uint8_t *)pvBuf, uBytesToCopy);

        // Write the content to the file
        ssize_t iWriteBytes = pwrite(iFD, pvBuffer, uSectorSize, uWriteOffset);
        if ( iWriteBytes != (ssize_t)uSectorSize)
        {
            iErr = (iWriteBytes < 0) ? errno : EIO;
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_write: pwrite failed to write wanted length\n" );
            hfs_free(pvBuffer);
            return iErr;
        }

        hfs_free(pvBuffer);
    }
    // Can write buffer size chunk
    else
    {
        uint64_t uAvailSectors     = uContigousClustersInBytes / uSectorSize;
        uint64_t uRemainingSectors = uBytesToWrite             / uSectorSize;

        uBytesToCopy = MIN(uAvailSectors, uRemainingSectors) * uSectorSize;
        uBytesToCopy = MIN( uBytesToCopy, MAX_READ_WRITE_LENGTH );

        assert( (uBytesToCopy % uSectorSize) == 0 );
        assert( (uWriteOffset % uSectorSize) == 0 );

        ssize_t iWriteBytes = pwrite(iFD, (uint8_t *)pvBuf, uBytesToCopy, uWriteOffset) ;
        if ( iWriteBytes != (ssize_t) uBytesToCopy)
        {
            iErr = (iWriteBytes < 0) ? errno : EIO;
            LFHFS_LOG( LEVEL_ERROR, "raw_readwrite_write: pwrite failed to write wanted length\n" );
            return iErr;
        }
    }

    // Update the amount of bytes alreay written
    *piActuallyWritten = uBytesToCopy;

    return iErr;
}

int
raw_readwrite_zero_fill_init()
{
    if ( gpvZeroBuf )
    {
        return 0;
    }

    gpvZeroBuf = hfs_malloc( ZERO_BUF_SIZE );
    if ( gpvZeroBuf == NULL )
    {
        return ENOMEM;
    }

    memset( gpvZeroBuf, 0, ZERO_BUF_SIZE );

    return 0;
}

void
raw_readwrite_zero_fill_de_init()
{
    if ( gpvZeroBuf )
    {
        hfs_free( gpvZeroBuf );
    }

    gpvZeroBuf = NULL;
}

int
raw_readwrite_zero_fill_fill( hfsmount_t* psMount, uint64_t uBlock, uint32_t uContigBlocks )
{
    int iErr                    = 0;
    int64_t lWriteSize          = 0;
    uint64_t uCurWriteOffset    = 0;
    uint64_t uCurWriteLen       = 0;
    uint64_t uDataWriten        = 0;

    if ( gpvZeroBuf == NULL )
    {
        iErr = EINVAL;
        goto exit;
    }

    uint64_t uLength = uContigBlocks*psMount->blockSize;
    uint64_t uOffset = psMount->hfsPlusIOPosOffset + uBlock*psMount->blockSize;

    while ( uDataWriten < uLength )
    {
        uCurWriteOffset = uOffset+uDataWriten;
        uCurWriteLen    = MIN( (uLength - uDataWriten), ZERO_BUF_SIZE );

        lWriteSize = pwrite( psMount->hfs_devvp->psFSRecord->iFD, gpvZeroBuf, uCurWriteLen, uCurWriteOffset );
        if ( lWriteSize != (int64_t)uCurWriteLen )
        {
            iErr = errno;
            goto exit;
        }

        uDataWriten += uCurWriteLen;
    }

exit:
    return iErr;
}

errno_t
raw_readwrite_zero_fill_last_block_suffix( vnode_t psVnode )
{

    int iErr = 0;
    int iFD = (VPTOFSRECORD(psVnode))->iFD;
    struct filefork *fp = VTOF(psVnode);
    uint32_t uBlockSize = HFSTOVCB(psVnode->sFSParams.vnfs_mp->psHfsmount)->blockSize;
    uint32_t uBytesToKeep = fp->ff_size % uBlockSize;
    uint64_t uBlockN = 0;
    uint64_t uContigousClustersInBytes;
    uint64_t uInClusterOffset;
    
    uint8_t*  puClusterData = NULL;

    iErr = raw_readwrite_get_cluster_from_offset(psVnode, fp->ff_size - uBytesToKeep, &uBlockN, &uInClusterOffset, &uContigousClustersInBytes);
    if ( iErr != 0 )
    {
        goto exit;
    }

    // Allocate buffer for cluster.
    puClusterData = hfs_malloc( uBlockSize );
    if ( puClusterData == NULL )
    {
        iErr = ENOMEM;
        goto exit;
    }

    // Read the last cluster.
    size_t uBytesRead = pread( iFD, puClusterData, uBlockSize, FSOPS_GetOffsetFromClusterNum( psVnode, uBlockN ) );
    if ( uBytesRead != uBlockSize )
    {
        iErr = errno;
        goto exit;
    }

    memset( puClusterData+uBytesToKeep, 0, uBlockSize-uBytesToKeep );

    // Write the last cluster.
    size_t uBytesWrite = pwrite( iFD, puClusterData, uBlockSize, FSOPS_GetOffsetFromClusterNum( psVnode, uBlockN ) );
    if ( uBytesWrite != uBlockSize )
    {
        iErr = errno;
        goto exit;
    }

exit:
    if ( puClusterData )
        hfs_free( puClusterData );

    return iErr;
}

