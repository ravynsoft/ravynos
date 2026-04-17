/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  FSOPS_Handler.c
 *  usbstorage_plugin
 *
 *  Created by Or Haimovich on 15/10/17.
 */

#include "FSOPS_Handler.h"
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <sys/attr.h>
#include "FAT_Access_M.h"
#include "FileOPS_Handler.h"
#include "DirOPS_Handler.h"
#include "FileRecord_M.h"
#include "FAT_Access_M.h"
#include "Conv.h"
#include <CommonCrypto/CommonDigest.h>
#include "Logger.h"
#include "ZeroFill.h"
#include "diagnostic.h"
#include <spawn.h>

// Maximum amount of a directory to read at one time
#define   MAX_DIR_BLOCK_SIZE (128 * 1024)
#define   PRE_DEFINED_SECTOR_SIZE (512)
//For taste, we need to verify that there isn'y any exfat signiture in the boot sector
#define   EXFAT_SIGNITURE_LENGTH (11)

#define UNALIGNED_BLOCK_SIZE  (4096U)
#define UNALIGNED_BLOCK_MASK  (UNALIGNED_BLOCK_SIZE - 1)

extern int32_t msdos_secondsWest;    /* In msdosfs_conv.c */

//---------------------------------- Functions Declaration ---------------------------------------

static void         FSOPS_InitReadBootSectorAndSetFATType(void** ppvBootSector,FileSystemRecord_s *psFSRecord,int* piErr,uint32_t uBytesPerSector, bool bFailForDirty);
static int          FSOPS_CreateRootRecord(FileSystemRecord_s *psFSRecord, NodeRecord_s** ppvRootNode);
static int          FSOPS_UpdateFSInfoSector(FileSystemRecord_s *psFSRecord);
static void         FSOPS_CopyVolumeLabelFromBootSector(FileSystemRecord_s *psFSRecord, struct extboot *extboot);
static void         FSOPS_CopyVolumeLabelFromVolumeEntry(FileSystemRecord_s *psFSRecord, struct dosdirentry* psDosDirEntry);
static void         FSOPS_CopyVolumeLabel(FileSystemRecord_s *psFSRecord, int8_t uVolShortName[SHORT_NAME_LEN]);
static int          FSOPS_CollectFATStatistics(FileSystemRecord_s *psFSRecord);
//---------------------------------- SPI Declaration ------------------------------------------

static int          MSDOS_Taste (int iDiskFd);
static int          MSDOS_ScanVols (int iDiskFd, UVFSScanVolsRequest *request, UVFSScanVolsReply *reply);
static int          MSDOS_Mount (int iDiskFd, UVFSVolumeId volId, UVFSMountFlags mountFlags,
    UVFSVolumeCredential *volumeCreds, UVFSFileNode *outRootNode);
static int          MSDOS_Unmount (UVFSFileNode rootFileNode, UVFSUnmountHint hint);
static int          MSDOS_Sync (UVFSFileNode node);
static int          MSDOS_Init (void);
static void         MSDOS_Fini (void);

//---------------------------------- Functions Implementation ------------------------------------

static int
FSOPS_CreateRootRecord(FileSystemRecord_s *psFSRecord, NodeRecord_s** ppvRootNode)
{
    int iErr = 0;

    // Create root NodeRecord
    iErr = FILERECORD_AllocateRecord(ppvRootNode, psFSRecord,
                                     psFSRecord->sRootInfo.uRootCluster ,RECORD_IDENTIFIER_ROOT, NULL, "");
    if ( iErr != 0 )
    {
        return iErr;
    }

    DIAGNOSTIC_INSERT(*ppvRootNode,psFSRecord->sFSInfo.uMaxCluster+1,"Root");

    //Make Hash Table
    iErr = DIROPS_CreateHTForDirectory(*ppvRootNode);
    if ( iErr != 0 )
    {
        return iErr;
    }

    // Set dir entry data to 'volume label' entry.
    LookForDirEntryArgs_s sArgs;
    sArgs.eMethod = LU_BY_VOLUME_ENTRY;
    NodeDirEntriesData_s sNodeDirEntriesData;

    iErr = DIROPS_LookForDirEntry( *ppvRootNode, &sArgs, NULL, &sNodeDirEntriesData );
    if ( iErr == 0 )
    {
        MSDOS_LOG( LEVEL_DEBUG, "FSOPS_CreateRootRecord: found volume entry.\n" );
        (*ppvRootNode)->sRecordData.sNDE = sNodeDirEntriesData;
        (*ppvRootNode)->sRecordData.bIsNDEValid = true;

        //Override the rootlabel as well.
        FSOPS_CopyVolumeLabelFromVolumeEntry(psFSRecord, &sNodeDirEntriesData.sDosDirEntry);
    }
    // Can't found volume entry.. ignoring..
    else if ( iErr == ENOENT )
    {
        iErr = 0;
    }

    return iErr;
}

/*
 * Create a version 3 UUID from unique data in the SHA1 "name space".
 * Version 3 UUIDs are derived using MD5 checksum.  Here, the unique
 * data is the 4-byte volume ID and the number of sectors (normalized
 * to a 4-byte little endian value).
 */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations" /* MD5 part of on-disk format */
static void FSOPS_generate_volume_uuid(uuid_t result_uuid, uint8_t volumeID[4], uint32_t totalSectors)
{
    CC_MD5_CTX c;
    uint8_t sectorsLittleEndian[4];
    
    UUID_DEFINE( kFSUUIDNamespaceSHA1, 0xB3, 0xE2, 0x0F, 0x39, 0xF2, 0x92, 0x11, 0xD6, 0x97, 0xA4, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC );

    /*
     * Normalize totalSectors to a little endian value so that this returns the
     * same UUID regardless of endianness.
     */
    putuint32(sectorsLittleEndian, totalSectors);
    
    /*
     * Generate an MD5 hash of our "name space", and our unique bits of data
     * (the volume ID and total sectors).
     */
    CC_MD5_Init(&c);
    CC_MD5_Update(&c, kFSUUIDNamespaceSHA1, sizeof(uuid_t));
    CC_MD5_Update(&c, volumeID, 4);
    CC_MD5_Update(&c, sectorsLittleEndian, sizeof(sectorsLittleEndian));
    CC_MD5_Final(result_uuid, &c);
    
    /* Force the resulting UUID to be a version 3 UUID. */
    result_uuid[6] = (result_uuid[6] & 0x0F) | 0x30;
    result_uuid[8] = (result_uuid[8] & 0x3F) | 0x80;
}
#pragma clang diagnostic pop

/*
 * Copy the label from the boot sector into the mount point.
 *
 * We don't call msdosfs_dos2unicodefn() because it assumes the last three
 * characters are an extension, and it will put a period before the
 * extension.
 */
extern u_int16_t dos2unicode[32];
static void FSOPS_CopyVolumeLabel(FileSystemRecord_s *psFSRecord, int8_t* uVolShortName)
{
    unsigned char uc;
    int           i;

    for (i=0; i<SHORT_NAME_LEN; i++) {
        uc = uVolShortName[i];
        if (i==0 && uc == SLOT_E5)
            uc = 0xE5;
        psFSRecord->sFSInfo.uVolumeLabel[i] = (uc < 0x80 || uc > 0x9F ? uc : dos2unicode[uc - 0x80]);
    }

    /* Remove trailing spaces, add NUL terminator */
    for (i=10; i>=0 && psFSRecord->sFSInfo.uVolumeLabel[i]==' '; --i)
        ;
    psFSRecord->sFSInfo.uVolumeLabel[i+1] = '\0';

    psFSRecord->sFSInfo.bVolumeLabelExist = true;
}

static void
FSOPS_CopyVolumeLabelFromBootSector(FileSystemRecord_s *psFSRecord, struct extboot *spExtboot)
{
    FSOPS_CopyVolumeLabel(psFSRecord, spExtboot->exVolumeLabel);
}

static void
FSOPS_CopyVolumeLabelFromVolumeEntry(FileSystemRecord_s *psFSRecord, struct dosdirentry* psDosDirEntry)
{
    FSOPS_CopyVolumeLabel(psFSRecord, (int8_t*) psDosDirEntry->deName);
}

static void
FSOPS_InitReadBootSectorAndSetFATType(void** ppvBootSector,FileSystemRecord_s *psFSRecord,int* piErr,uint32_t uBytesPerSector, bool bFailForDirty)
{
    //Read boot sector for MSDOS_Mount and validate
    *ppvBootSector = malloc(uBytesPerSector);
    if (*ppvBootSector == NULL)
    {
        *piErr = ENOMEM;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: failed to malloc pvBootSector\n");
        return;
    }
    
    if (pread(psFSRecord->iFD, *ppvBootSector, uBytesPerSector, 0) != uBytesPerSector)
    {
        *piErr = errno;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: failed to read boot record %d\n",*piErr);
        return;
    }
    
    union bootsector *boot = *ppvBootSector;
    struct byte_bpb50 *b50 = (struct byte_bpb50 *)boot->bs50.bsBPB;
    struct byte_bpb710 *b710 = (struct byte_bpb710 *)boot->bs710.bsBPB;
    
    char cOEMName[9] = {0};
    strlcpy(&cOEMName[0], (char *) boot->bs50.bsOemName , 8);
    cOEMName[8] = '\0';
    
    MSDOS_LOG(LEVEL_DEFAULT, "FSOPS_InitReadBootSectorAndSetFATType: OEMName: %s\n", cOEMName);

    // The first three bytes are an Intel x86 jump instruction.  Windows only
    // checks the first byte, so that's what we'll do, too.
    
    if (boot->bs50.bsJump[0] != 0xE9 &&
        boot->bs50.bsJump[0] != 0xEB)
    {
        *piErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: Invalid jump signature (0x%02X)\n", boot->bs50.bsJump[0]);
        return;
    }
    
    // Check the trailing "boot signature"
    if ((boot->bs50.bsBootSectSig0 != BOOTSIG0) || (boot->bs50.bsBootSectSig1 != BOOTSIG1) )
    {
        *piErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: Invalid boot signature (0x%02X 0x%02X)\n", boot->bs50.bsBootSectSig0, boot->bs50.bsBootSectSig1);
        return;
    }
    
    // Compute device quantities from the boot sector.
    psFSRecord->sFSInfo.uBytesPerSector = getuint16(b50->bpbBytesPerSec);
    unsigned uSectorsPerCluster = b50->bpbSecPerClust;
    psFSRecord->sFSInfo.uBytesPerCluster = psFSRecord->sFSInfo.uBytesPerSector * uSectorsPerCluster;
    uint32_t uReservedSectors = getuint16(b50->bpbResSectors);
    uint32_t uRootEntryCount = getuint16(b50->bpbRootDirEnts);
    
    uint32_t uTotalSectors  = (getuint16(b50->bpbSectors) == 0) ? getuint32(b50->bpbHugeSectors) : getuint16(b50->bpbSectors);
    uint32_t uFatSectors    = (getuint16(b50->bpbFATsecs) == 0) ? getuint32(b710->bpbBigFATsecs) : getuint16(b50->bpbFATsecs);
    
    /*
     * Check a few values:
     * - logical sector size: == device's current sector size
     * - sectors per cluster: power of 2, >= 1
     * - number of sectors:   >= 1
     * - number of FAT sectors > 0 (too large values handled later)
     */
    if (psFSRecord->sFSInfo.uBytesPerSector != uBytesPerSector)
    {
        *piErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: logical sector size (%u) != physical sector size (%u)\n", psFSRecord->sFSInfo.uBytesPerSector, uBytesPerSector);
        return;
    }
    
    if (uSectorsPerCluster == 0 || (uSectorsPerCluster & (uSectorsPerCluster - 1)))
    {
        *piErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: invalid sectors per cluster (%u)\n", uSectorsPerCluster);
        return;
    }
    
    if (uTotalSectors == 0)
    {
        //check if we encountered special FAT case
        if ((*((uint8_t*)(ppvBootSector + 0x42))) == 0x29 && (*((uint64_t*)(ppvBootSector + 0x52))) != 0)
        {
            MSDOS_LOG(LEVEL_ERROR, "Encountered special FAT where total sector location is 64bit. Not Supported\n");
        }
        
        *piErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: invalid total sectors (%u)\n", uTotalSectors);
        return;
    }
    
    if (uFatSectors == 0)
    {
        *piErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: invalid sectors per FAT (%u)\n", uFatSectors);
        return;
    }
    
    // For now, assume a FAT12 or FAT16 volume with a dedicated root directory.
    // We need these values before we can figure out how many clusters (and
    // thus whether the volume is FAT32 or not).  Start by calculating sectors.
    psFSRecord->sRootInfo.uRootSector = uReservedSectors + (b50->bpbFATs * uFatSectors);
    psFSRecord->sRootInfo.uRootSize = (uRootEntryCount * sizeof(struct dosdirentry) + uBytesPerSector - 1) / uBytesPerSector;
    psFSRecord->sFSInfo.uClusterOffset = psFSRecord->sRootInfo.uRootSector + psFSRecord->sRootInfo.uRootSize;
    
    if (uFatSectors > uTotalSectors ||
        psFSRecord->sRootInfo.uRootSector < uFatSectors ||           // Catch numeric overflow!
        psFSRecord->sFSInfo.uClusterOffset + uSectorsPerCluster > uTotalSectors)
    {
        // We think there isn't room for even a single cluster.
        *piErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: invalid configuration; no room for clusters\n");
        return;
    }
    
    // Usable clusters are numbered starting at 2, so the maximum usable cluster
    // is (number of clusters) + 1.  Convert the pm_firstcluster to device blocks.
    
    psFSRecord->sFSInfo.uMaxCluster = (uint32_t) ((uTotalSectors - psFSRecord->sFSInfo.uClusterOffset) / uSectorsPerCluster + 1);
    
    // Figure out the FAT type based on the number of clusters.
    if (psFSRecord->sFSInfo.uMaxCluster < (CLUST_RSRVD & FAT12_MASK))
    {
        psFSRecord->sFatInfo.uFatMask = FAT12_MASK;
        psFSRecord->sFSOperations.uFatEntryOffset = FAT_Access_M_Fat12EntryOffset;
        psFSRecord->sFSOperations.uFatEntryGet = FAT_Access_M_Fat12EntryGet;
        psFSRecord->sFSOperations.uFatEntrySet = FAT_Access_M_Fat12EntrySet;
    }
    else if (psFSRecord->sFSInfo.uMaxCluster < (CLUST_RSRVD & FAT16_MASK))
    {
        psFSRecord->sFatInfo.uFatMask = FAT16_MASK;
        psFSRecord->sFSOperations.uFatEntryOffset = FAT_Access_M_Fat16EntryOffset;
        psFSRecord->sFSOperations.uFatEntryGet = FAT_Access_M_Fat16EntryGet;
        psFSRecord->sFSOperations.uFatEntrySet = FAT_Access_M_Fat16EntrySet;
    }
    else if (psFSRecord->sFSInfo.uMaxCluster < (CLUST_RSRVD & FAT32_MASK))
    {
        psFSRecord->sFatInfo.uFatMask = FAT32_MASK;
        psFSRecord->sFSOperations.uFatEntryOffset = FAT_Access_M_Fat32EntryOffset;
        psFSRecord->sFSOperations.uFatEntryGet = FAT_Access_M_Fat32EntryGet;
        psFSRecord->sFSOperations.uFatEntrySet = FAT_Access_M_Fat32EntrySet;
    }
    else
    {
        *piErr = EINVAL;
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: number of clusters (0x%x) is too large\n", psFSRecord->sFSInfo.uMaxCluster + 1);
        return;
    }
    
    // TODO: Check b710->bpbExtFlags to see if FAT mirroring is on, and which
    // copy of the FAT is currently active.
    psFSRecord->sFatInfo.uFatOffset = uReservedSectors * uBytesPerSector;
    psFSRecord->sFatInfo.uFatSize = uFatSectors * uBytesPerSector;
    psFSRecord->sFatInfo.uNumOfFATs = b50->bpbFATs;
    
    struct extboot *extboot;
    uint8_t volume_serial_num[4];

    if (psFSRecord->sFatInfo.uFatMask == FAT32_MASK) 
    {
        extboot = (struct extboot *)boot->bs710.bsExt;
    }
    else
    {
        extboot = (struct extboot *)boot->bs50.bsExt;
    }

    if (extboot->exBootSignature == EXBOOTSIG) // Make sure volume has ext boot 
    {
        // Generate a UUID
        memcpy(volume_serial_num, extboot->exVolumeID, 4);

        memset(psFSRecord->sFSInfo.sUUID,0,sizeof(uuid_t));
        psFSRecord->sFSInfo.bUUIDExist = false;

        if (volume_serial_num[0] || volume_serial_num[1] || volume_serial_num[2] || volume_serial_num[3]) 
        {
            FSOPS_generate_volume_uuid(psFSRecord->sFSInfo.sUUID, volume_serial_num, (uint32_t) uTotalSectors); 
            psFSRecord->sFSInfo.bUUIDExist = true;
        }

        // Get the Volume lable
        FSOPS_CopyVolumeLabelFromBootSector(psFSRecord, extboot);
    }

    //Init FAT
    *piErr = FAT_Access_M_FATInit(psFSRecord);
    if (*piErr)
    {
        return;
    }
    
    bool dirty = false;
    uint32_t uFatFlags;
    // In FAT12 we don't have a dirty bit so we don't have to check if device is dirty
    if (!(psFSRecord->sFatInfo.uFatMask == FAT12_MASK))
    {
        // Get FAT entry for cluster #1.  Get the "clean shut down" bit.
        *piErr = FAT_Access_M_GetClustersFatEntryContent( psFSRecord, 1, &uFatFlags );
        if ( *piErr != 0 )
        {
            return;
        }
        
        dirty = !(uFatFlags & (psFSRecord->sFatInfo.uFatMask == FAT16_MASK ? 0x00008000 : 0x08000000));
        
    }
    
    if (dirty && bFailForDirty)
    {
        MSDOS_LOG(LEVEL_DEFAULT, "FSOPS_InitReadBootSectorAndSetFATType: Device is dirty.\n");
        *piErr = EINVAL;
        return;
    }
    
    // Sanity check some differences between FAT32 and FAT12/16.
    //  Also set up FAT32's root directory and FSInfo sector.
    if (psFSRecord->sFatInfo.uFatMask == FAT32_MASK)
    {
        psFSRecord->sFSInfo.uFsInfoSector = getuint16(b710->bpbFSInfo);
        psFSRecord->sRootInfo.uRootCluster = getuint32(b710->bpbRootClust);
        if (!CLUSTER_IS_VALID(psFSRecord->sRootInfo.uRootCluster, psFSRecord))
        {
            *piErr = EINVAL;
            MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: FAT32 root starting cluster (%u) out of range (%u..%u)\n",
                              psFSRecord->sRootInfo.uRootCluster, CLUST_FIRST, psFSRecord->sFSInfo.uMaxCluster);
            return;
        }
        if (uRootEntryCount)
        {
            *piErr = EINVAL;
            MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: FAT32 has non-zero root directory entry count\n");
            return;
        }
        if (getuint16(b710->bpbFSVers) != 0)
        {
            *piErr = EINVAL;
            MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: FAT32 has non-zero version\n");
            return;
        }
        if (getuint16(b50->bpbSectors) != 0)
        {
            *piErr = EINVAL;
            MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: FAT32 has 16-bit total sectors\n");
            return;
        }
        if (getuint16(b50->bpbFATsecs) != 0)
        {
            *piErr = EINVAL;
            MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: FAT32 has 16-bit FAT sectors\n");
            return;
        }
    }
    else
    {
        if (uRootEntryCount == 0)
        {
            *piErr = EINVAL;
            MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: FAT12/16 has zero-length root directory\n");
            return;
        }
        if (uTotalSectors < 0x10000 && getuint16(b50->bpbSectors) == 0)
        {
            MSDOS_LOG(LEVEL_DEBUG, "FSOPS_InitReadBootSectorAndSetFATType: FAT12/16 total sectors (%u) fit in 16 bits, but stored in 32 bits\n",  uTotalSectors);
        }
        if (getuint16(b50->bpbFATsecs) == 0)
        {
            MSDOS_LOG(LEVEL_DEBUG, "FSOPS_InitReadBootSectorAndSetFATType: FAT12/16 has 32-bit FAT sectors\n");
        }
    }
    
    
    /* Compute number of clusters this FAT could hold based on its total size.
     * Pin the maximum cluster number to the size of the FAT.
     *
     * NOTE: We have to do this AFTER determining the FAT type.  If we did this
     * before, we could end up deducing a different FAT type than what's actually
     * on disk, and that would be very bad. */
    uint32_t clusters = uFatSectors * psFSRecord->sFSInfo.uBytesPerSector;     // Size of FAT in bytes
    if (psFSRecord->sFatInfo.uFatMask == FAT32_MASK)
    {
        clusters /= 4;                                      // FAT32: 4 bytes per FAT entry
    }
    else if (psFSRecord->sFatInfo.uFatMask == FAT16_MASK)
    {
        clusters /= 2;                                      // FAT16: 2 bytes per FAT entry
    }
    else
    {
        clusters = clusters * 2 / 3;                        // FAT12: 3 bytes for every two FAT entries
    }
    
    if (psFSRecord->sFSInfo.uMaxCluster >= clusters)
    {
        MSDOS_LOG(LEVEL_DEBUG, "FSOPS_InitReadBootSectorAndSetFATType: Number of clusters (%d) exceeds FAT capacity (%d)\n",
                          psFSRecord->sFSInfo.uMaxCluster + 1, clusters);
        psFSRecord->sFSInfo.uMaxCluster = clusters - 1;
    }
    
    // Pin the maximum cluster number based on the number of sectors the device
    // is reporting (in case that is smaller than the total sectors field(s)
    // in the boot sector).
    uint64_t uBlockCount;
    *piErr = ioctl(psFSRecord->iFD, DKIOCGETBLOCKCOUNT, &uBlockCount);
    if ( (*piErr == 0) && (uBlockCount < uTotalSectors))
    {
        if (psFSRecord->sFSInfo.uClusterOffset + uSectorsPerCluster > uBlockCount)
        {
            *piErr = EINVAL;
            MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: device sector count (%llu) too small; no room for clusters\n",  uBlockCount);
            return;
        }
        
        uint32_t maxcluster = (uint32_t)((uBlockCount - psFSRecord->sFSInfo.uClusterOffset) / uSectorsPerCluster + 1);
        if (maxcluster < psFSRecord->sFSInfo.uMaxCluster)
        {
            MSDOS_LOG(LEVEL_DEBUG, "FSOPS_InitReadBootSectorAndSetFATType: device sector count (%llu) is less than volume sector count (%u); limiting maximum cluster to %u (was %u)\n",
                              uBlockCount, uTotalSectors, maxcluster, psFSRecord->sFSInfo.uMaxCluster);
            psFSRecord->sFSInfo.uMaxCluster = maxcluster;
        }
        else
        {
            MSDOS_LOG(LEVEL_DEBUG, "FSOPS_InitReadBootSectorAndSetFATType: device sector count (%llu) is less than volume sector count (%u)\n", uBlockCount, uTotalSectors);
        }
    }

    //rdar://problem/36373515 - Workaround to allow working on files and not only on dmg/real devices
    if (*piErr)
    {
//        *piErr = errno;
//        MSDOS_LOG(LEVEL_ERROR, "MSDOS_mount: ioctl(DKIOCGETBLOCKCOUNT) failed, %s\n", *piErr);
//        return;
        *piErr = 0;
    }
    
    // Compute the size (in bytes) of the volume root directory.
    psFSRecord->sRootInfo.uRootLength = (uint32_t) psFSRecord->sRootInfo.uRootSize * psFSRecord->sFSInfo.uBytesPerSector;
    if (psFSRecord->sFatInfo.uFatMask == FAT32_MASK)
    {
        uint32_t uNumClusters = 0;
        uint32_t uLastCluster = 0;
        *piErr =FAT_Access_M_ChainLength(psFSRecord,psFSRecord->sRootInfo.uRootCluster, &uNumClusters, NULL, &uLastCluster);
        if (*piErr)
        {
            return;
        }
        psFSRecord->sRootInfo.uRootLength = (uint32_t) uNumClusters * CLUSTER_SIZE(psFSRecord);
    }
    
    // Calculate the amount to read from a directory at one time.  Default to
    // one cluster, but pin to no more than 128KiB.
    psFSRecord->sFSInfo.sDirBlockSize = (CLUSTER_SIZE(psFSRecord) > MAX_DIR_BLOCK_SIZE) ? MAX_DIR_BLOCK_SIZE : CLUSTER_SIZE(psFSRecord);
    
    // If this is a FAT32 volume, try to read in the FSInfo sector and fetch
    // the free cluster count.
    if (psFSRecord->sFSInfo.uFsInfoSector)
    {
        psFSRecord->pvFSInfoCluster = malloc(psFSRecord->sFSInfo.uBytesPerSector);
        if (psFSRecord->pvFSInfoCluster == NULL)
        {
            *piErr = ENOMEM;
            return;
        }
        
        uint32_t uReadOffset = psFSRecord->sFSInfo.uFsInfoSector*psFSRecord->sFSInfo.uBytesPerSector;

        if (pread(psFSRecord->iFD, psFSRecord->pvFSInfoCluster, psFSRecord->sFSInfo.uBytesPerSector, uReadOffset) != psFSRecord->sFSInfo.uBytesPerSector)
        {
            *piErr = errno;
            MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: Error %d trying to read FSInfo sector; ignoring.\n",  *piErr);
            psFSRecord->sFSInfo.uFsInfoSector = 0;
            *piErr = 0;
        }
        else
        {
            struct fsinfo *fsInfo = psFSRecord->pvFSInfoCluster;
            if (!bcmp(fsInfo->fsisig1, "RRaA", 4) &&
                !bcmp(fsInfo->fsisig2, "rrAa", 4) &&
                !bcmp(fsInfo->fsisig3, "\0\0\125\252", 4))
            {
                psFSRecord->sFSInfo.uFreeClusters = getuint32(fsInfo->fsinfree);
                psFSRecord->sFSInfo.uFirstFreeCluster = getuint32(fsInfo->fsinxtfree);

                // Sanity check the free cluster count.  If it is bigger than the
                // total number of clusters (maxCluster - 1), then ignore it.
                if (psFSRecord->sFSInfo.uFreeClusters > psFSRecord->sFSInfo.uMaxCluster-1)
                    psFSRecord->sFSInfo.uFsInfoSector = 0;
            }
            else
            {
                MSDOS_LOG(LEVEL_DEBUG, "FSOPS_InitReadBootSectorAndSetFATType: FSInfo sector has invalid signature(s); ignoring.\n");
                psFSRecord->sFSInfo.uFsInfoSector = 0;
            }
        }
    }
}

static int FSOPS_CollectFATStatistics(FileSystemRecord_s *psFSRecord)
{
    int iError = 0;
    uint32_t uCountFreeClusters;
    iError = FAT_Access_M_GetTotalFreeClusters( psFSRecord, &uCountFreeClusters );
    if ( iError != 0 )
    {
        return iError;
    }
    if (psFSRecord->sFSInfo.uFreeClusters == 0)
    {
        psFSRecord->sFSInfo.uFreeClusters = uCountFreeClusters;
    }
    else
    {
        if (psFSRecord->sFSInfo.uFreeClusters != uCountFreeClusters)
        {
            MSDOS_LOG(LEVEL_DEBUG, "FSOPS_InitReadBootSectorAndSetFATType: uFreeClusters read from boot (%u) is different than counted (%d) - replaced\n",
                      psFSRecord->sFSInfo.uFreeClusters, uCountFreeClusters);

            psFSRecord->sFSInfo.uFreeClusters = uCountFreeClusters;
        }
    }

    uint32_t uBootSectorFirstFreeCluster = psFSRecord->sFSInfo.uFirstFreeCluster;

    iError = FAT_Access_M_FindFirstFreeClusterFromGivenCluster(psFSRecord,0);
    if (iError)
    {
        MSDOS_LOG(LEVEL_ERROR, "FSOPS_InitReadBootSectorAndSetFATType: Failed to retrive first free cluster .\n");
    }

    if (psFSRecord->sFSInfo.uFirstFreeCluster != uBootSectorFirstFreeCluster)
    {
        MSDOS_LOG(LEVEL_DEBUG, "FSOPS_InitReadBootSectorAndSetFATType: uFirstFreeCluster read from boot (%u) is different than detected on device (%d) - replaced\n", uBootSectorFirstFreeCluster,
                  psFSRecord->sFSInfo.uFirstFreeCluster);
    }

    return iError;
}

static int FSOPS_UpdateFSInfoSector(FileSystemRecord_s *psFSRecord)
{
    int iError =0;

    // If this is a FAT32 volume, try to read in the FSInfo sector and fetch
    // the free cluster count.
    if ( psFSRecord->sFSInfo.uFsInfoSector && (psFSRecord->pvFSInfoCluster!=NULL) )
    {
        struct fsinfo *fsInfo = psFSRecord->pvFSInfoCluster;

        // Get Current counters
        uint32_t uCurrentfsinfree = getuint32(fsInfo->fsinfree);
        uint32_t uCurrentfsinxtfree = getuint32(fsInfo->fsinxtfree);

        //Check if there is a reason to update the FSInfo Sector
        if ( (uCurrentfsinxtfree != psFSRecord->sFSInfo.uFirstFreeCluster) ||
             (uCurrentfsinfree != psFSRecord->sFSInfo.uFreeClusters))
        {
            //Set fsinfree and fsinxtfree
            putuint32(fsInfo->fsinfree, psFSRecord->sFSInfo.uFreeClusters);
            putuint32(fsInfo->fsinxtfree, psFSRecord->sFSInfo.uFirstFreeCluster);

            //Flush the FSInfo sector into the device
            uint32_t uFSInfoSectorOffset = psFSRecord->sFSInfo.uFsInfoSector*psFSRecord->sFSInfo.uBytesPerSector;
            if (pwrite(psFSRecord->iFD, psFSRecord->pvFSInfoCluster, psFSRecord->sFSInfo.uBytesPerSector, uFSInfoSectorOffset) != psFSRecord->sFSInfo.uBytesPerSector)
            {
                iError = errno;
                MSDOS_LOG(LEVEL_ERROR, "FSOPS_UpdateFSInfoSector: Error %d trying to write FSInfo sector\n",  iError);
            }
        }
    }

    return iError;
}

int FSOPS_SetDirtyBitAndAcquireLck(FileSystemRecord_s* psFSRecord)
{
    //Acquire Read Lock
    MultiReadSingleWrite_LockRead(&psFSRecord->sDirtyBitLck);

    // Set drive dirty bit.
    int iErr = FATMOD_SetDriveDirtyBit( psFSRecord, true );

    return iErr;
}

int FSOPS_FlushCacheAndFreeLck(FileSystemRecord_s* psFSRecord)
{
    int iErr = FATMOD_FlushAllCacheEntries(psFSRecord);

    //Free Read Lock
    MultiReadSingleWrite_FreeRead(&psFSRecord->sDirtyBitLck);

    return iErr;
}

//---------------------------------- SPI Implementation ------------------------------------------

static int
MSDOS_Taste (int iDiskFd)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Taste\n");
    
    void* pvBootSector = NULL;
    int iError = 0;
    
    uint32_t uBytesPerSector;
    if (ioctl(iDiskFd, DKIOCGETBLOCKSIZE, &uBytesPerSector) < 0)
    {
        //rdar://problem/36373515 - Workaround to allow working on files and not only on dmg/real devices
        //return errno;
        uBytesPerSector = PRE_DEFINED_SECTOR_SIZE;
    }
    
    //Read boot sector
    pvBootSector = malloc(uBytesPerSector);
    if (pvBootSector == NULL)
    {
        iError = ENOMEM;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_taste: failed to malloc pvBootSector\n");
        return iError;
    }
    
    if (pread(iDiskFd, pvBootSector, uBytesPerSector, 0) != uBytesPerSector)
    {
        iError = errno;
        free(pvBootSector);
        return iError;
    }

    /*
     * The first three bytes are an Intel x86 jump instruction.  It should be one
     * of the following forms:
     *    0xE9 0x?? 0x??
     *    0xEB 0x?? 0x90
     * where 0x?? means any byte value is OK.
     *
     * Windows doesn't actually check the third byte if the first byte is 0xEB,
     * so we don't either
     *
     *If Exfat signiture exsits in boot sector return failure
     */
     union bootsector * psBootSector = (union bootsector *) pvBootSector;
    if (((psBootSector->bs50.bsJump[0] != 0xE9) && (psBootSector->bs50.bsJump[0] != 0xEB)) ||
        !memcmp(pvBootSector, "\xEB\x76\x90""EXFAT   ", EXFAT_SIGNITURE_LENGTH))
    {
        iError = ENOTSUP;
    }

    free(pvBootSector);
    return iError;
}

static int
MSDOS_ScanVolGetVolName(char* pcVolName, int iDiskFd)
{
    int iError = 0;
    NodeRecord_s* pvRootNode = NULL;
    FileSystemRecord_s *psFSRecord;
    void* pvBootSector = NULL;

    uint32_t uBytesPerSector = 0;

    // Malloc psVolume
    psFSRecord = (FileSystemRecord_s*) malloc(sizeof(FileSystemRecord_s));
    if (psFSRecord == NULL)
    {
        pcVolName[0] = '\0';
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ScanVolGetVolName: failed to malloc psVolume\n");
        return 0;
    }

    memset((void*)psFSRecord,0,sizeof(FileSystemRecord_s));
    psFSRecord->iFD = iDiskFd;

    //Init chain cache
    FILERECORD_InitChainCache(psFSRecord);

    // Sanity check the device's logical block size.
    if (ioctl(psFSRecord->iFD, DKIOCGETBLOCKSIZE, &uBytesPerSector) < 0)
    {
        //rdar://problem/36373515 - Workaround to allow working on files and not only on dmg/real devices
        //        err = errno;
        //        MSDOS_LOG(LEVEL_ERROR, "MSDOS_mount: ioctl(DKIOCGETBLOCKSIZE) failed, %s\n", strerror(err));
        //        goto free_Volume_and_fail;
        uBytesPerSector = PRE_DEFINED_SECTOR_SIZE;
    }

    if (uBytesPerSector > MAX_BLOCK_SIZE)
    {
        iError = EIO;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_mount: block size is too big (%lu)\n", (unsigned long) uBytesPerSector);
        goto end;
    }

    FSOPS_InitReadBootSectorAndSetFATType(&pvBootSector, psFSRecord, &iError, uBytesPerSector, false);
    if (iError)
    {
        goto end;
    }

    // Create root NodeRecord
    iError = FILERECORD_AllocateRecord(&pvRootNode, psFSRecord, psFSRecord->sRootInfo.uRootCluster ,RECORD_IDENTIFIER_ROOT, NULL, "");
    if ( iError != 0 )
    {
        goto end;
    }

    // Set dir entry data to 'volume label' entry.
    LookForDirEntryArgs_s sArgs;
    sArgs.eMethod = LU_BY_VOLUME_ENTRY;
    NodeDirEntriesData_s sNodeDirEntriesData;

    iError = DIROPS_LookForDirEntry(pvRootNode, &sArgs, NULL, &sNodeDirEntriesData );
    if ( iError == 0 )
    {
        MSDOS_LOG( LEVEL_DEBUG, "FSOPS_CreateRootRecord: found volume entry.\n" );
        pvRootNode->sRecordData.sNDE = sNodeDirEntriesData;
        pvRootNode->sRecordData.bIsNDEValid = true;

        //Override the rootlabel as well.
        FSOPS_CopyVolumeLabelFromVolumeEntry(psFSRecord, &sNodeDirEntriesData.sDosDirEntry);
    }
    // Can't found volume entry.. ignoring..
    else if ( iError == ENOENT )
    {
        iError = 0;
    }

    if (psFSRecord->sFSInfo.bVolumeLabelExist)
        strlcpy(pcVolName, (char*) psFSRecord->sFSInfo.uVolumeLabel, UVFS_SCANVOLS_VOLNAME_MAX);
    else
        pcVolName[0] = '\0';

end:
    if (iError) {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_ScanVolGetVolName: failed with error %d, returning empty name and no error\n",iError);
        pcVolName[0] = '\0';
    }

    if (pvRootNode) FILERECORD_FreeRecord(pvRootNode);

    FAT_Access_M_FATFini(psFSRecord);
    if (psFSRecord->psClusterChainCache)
        FILERECORD_DeInitChainCache(psFSRecord);

    //Free Boot Sector
    if (pvBootSector != NULL)
        free(pvBootSector);
    
    if (psFSRecord->pvFSInfoCluster != NULL)
        free(psFSRecord->pvFSInfoCluster);
    
    free(psFSRecord);
    return 0;
}


static int
MSDOS_ScanVols (int iDiskFd, UVFSScanVolsRequest *request, UVFSScanVolsReply *reply)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_ScanVols\n");

    if (request == NULL || reply == NULL)
    {
        return EINVAL;
    }
    else if (request->sr_volid > 0)
    {
        return UVFS_SCANVOLS_EOF_REACHED;
    }

    // Tell UVFS that we have a single, non-access controlled volume.
    reply->sr_volid = 0;
    reply->sr_volac = UAC_UNLOCKED;

    return MSDOS_ScanVolGetVolName(reply->sr_volname, iDiskFd);
}

static int
MSDOS_Init (void)
{
    int iErr = 0;
    
    iErr = MSDOS_LoggerInit();
    if ( iErr != 0 )
    {
        goto exit;
    }

    iErr = ZeroFill_Init();
    if ( iErr != 0 )
    {
        goto exit;
    }

exit:
    MSDOS_LOG( LEVEL_DEFAULT, "MSDOS_Init status = %d\n", iErr );
    return iErr;
}

static void
MSDOS_Fini (void)
{
    ZeroFill_DeInit();
    
    MSDOS_LOG(LEVEL_DEFAULT, "MSDOS_Fini\n");
}

static int
MSDOS_GetGMTDiffOffset( void )
{
    time_t uGMT, uRawTime = time(NULL);
    struct tm *psPTM;

    psPTM = gmtime( &uRawTime );

    // Request that mktime() looksup dst in timezone database
    psPTM->tm_isdst = -1;
    uGMT = mktime(psPTM);

    return (int)difftime(uRawTime, uGMT);
}

static int
MSDOS_Mount (int iDiskFd, UVFSVolumeId volId, __unused UVFSMountFlags mountFlags,
    __unused UVFSVolumeCredential *volumeCreds, UVFSFileNode *outRootNode)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Mount\n");

    if (volId != 0)
    {
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_mount: unknown volume ID\n");
        return EINVAL;
    }

    /* Set gmtoffset */
    msdos_secondsWest = -MSDOS_GetGMTDiffOffset();
    
    // Set global file descriptor
    NodeRecord_s* pvRootNode = NULL;
    FileSystemRecord_s *psFSRecord;
    void* pvBootSector = NULL;
    errno_t err = 0;
    uint32_t uBytesPerSector = 0;

    // Malloc psVolume
    psFSRecord = (FileSystemRecord_s*) malloc(sizeof(FileSystemRecord_s));
    if (psFSRecord == NULL)
    {
        err = ENOMEM;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_mount: failed to malloc psVolume\n");
        goto end;
    }

    memset((void*)psFSRecord,0,sizeof(FileSystemRecord_s));
    psFSRecord->iFD = iDiskFd;
    
    // Init dir entry access locks.
    MultiReadSingleWrite_Init( &psFSRecord->sDirEntryAccessLock );
    MultiReadSingleWrite_Init( &psFSRecord->sDirtyBitLck );
    MultiReadSingleWrite_Init( &psFSRecord->sDirHTLRUTableLock );
    psFSRecord->uDirHTGeneration = 1;
    psFSRecord->uPreAllocatedOpenFiles = 0;

    //Init chain cache
    FILERECORD_InitChainCache(psFSRecord);
    
    // Sanity check the device's logical block size.
    if (ioctl(psFSRecord->iFD, DKIOCGETBLOCKSIZE, &uBytesPerSector) < 0)
    {
        //rdar://problem/36373515 - Workaround to allow working on files and not only on dmg/real devices
//        err = errno;
//        MSDOS_LOG(LEVEL_ERROR, "MSDOS_mount: ioctl(DKIOCGETBLOCKSIZE) failed, %s\n", strerror(err));
//        goto free_Volume_and_fail;
        uBytesPerSector = PRE_DEFINED_SECTOR_SIZE;
    }

    if (uBytesPerSector > MAX_BLOCK_SIZE)
    {
        err = EIO;
        MSDOS_LOG(LEVEL_ERROR, "MSDOS_mount: block size is too big (%lu)\n", (unsigned long) uBytesPerSector);
        goto free_Volume_and_fail;
    }
    
    FSOPS_InitReadBootSectorAndSetFATType(&pvBootSector, psFSRecord, &err, uBytesPerSector, true);
    if (err)
    {
        goto free_all_and_fail;
    }

    err = FSOPS_CollectFATStatistics(psFSRecord);
    if (err)
    {
        goto free_all_and_fail;
    }

    DIAGNOSTIC_INIT(psFSRecord);

    // Create roothandle with root directory
    if (!FSOPS_CreateRootRecord(psFSRecord , &pvRootNode))
    {
        free(pvBootSector);
        goto end;
    }

    //If failed free root node
    DIAGNOSTIC_REMOVE(pvRootNode);
    FILERECORD_FreeRecord(pvRootNode);
    pvRootNode = NULL;


free_all_and_fail:
    if (pvBootSector != NULL)
        free(pvBootSector);

    FAT_Access_M_FATFini(psFSRecord);

    if (psFSRecord->psClusterChainCache)
        FILERECORD_DeInitChainCache(psFSRecord);

free_Volume_and_fail:
    MultiReadSingleWrite_DeInit( &psFSRecord->sDirEntryAccessLock );
    MultiReadSingleWrite_DeInit( &psFSRecord->sDirtyBitLck );
    MultiReadSingleWrite_DeInit( &psFSRecord->sDirHTLRUTableLock );
    free(psFSRecord);
end:
    
    *outRootNode = (UVFSFileNode) pvRootNode;
    return err;
}

static int
MSDOS_Sync (UVFSFileNode node)
{
    MSDOS_LOG(LEVEL_DEBUG, "MSDOS_Sync\n");
    VERIFY_NODE_IS_VALID(node);
    
    int iErr = 0;
    FileSystemRecord_s *psFSRecord = GET_FSRECORD(GET_RECORD(node));

    //Acquire the DirtyBit Lock for write
    MultiReadSingleWrite_LockWrite(&psFSRecord->sDirtyBitLck);

    //Update FSinfo sector if needed
    iErr = FSOPS_UpdateFSInfoSector(psFSRecord);
    if ( iErr != 0 )
    {
        goto exit;
    }

    //We will set the device as not dirty, only if we
    if (psFSRecord->uPreAllocatedOpenFiles == 0)
    {
        // Clear drive dirty bit.
        iErr = FATMOD_SetDriveDirtyBit( psFSRecord, false );
    }

exit:
    MultiReadSingleWrite_FreeWrite(&psFSRecord->sDirtyBitLck);

    return iErr;
}

static int
MSDOS_Unmount (UVFSFileNode rootFileNode, UVFSUnmountHint hint)
{
    MSDOS_LOG(LEVEL_DEFAULT, "MSDOS_Unmount\n");
	VERIFY_NODE_IS_VALID(rootFileNode);
    
	int iError = 0;
    NodeRecord_s* pvRootRecord = GET_RECORD(rootFileNode);
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(pvRootRecord);
    
    //Free the FAT cache
    FAT_Access_M_FATFini(GET_FSRECORD(pvRootRecord));

    DIAGNOSTIC_REMOVE(pvRootRecord);
    //Free root node
    FILERECORD_FreeRecord(pvRootRecord);
    rootFileNode=NULL;

    // Return ENOTEMPTY in case of error.
    iError = DIAGNOSTIC_DEINIT(psFSRecord);

    // Deinit dir entry access locks.
    MultiReadSingleWrite_DeInit( &psFSRecord->sDirEntryAccessLock );
    MultiReadSingleWrite_DeInit( &psFSRecord->sDirtyBitLck );
    MultiReadSingleWrite_DeInit( &psFSRecord->sDirHTLRUTableLock );
    
    //Free FS Record
    FILERECORD_DeInitChainCache(psFSRecord);

    //Free FSInfoSector
    if (psFSRecord->pvFSInfoCluster != NULL)
        free(psFSRecord->pvFSInfoCluster);

    free(psFSRecord);
    
    return iError;
}


static int
MSDOS_GetFSAttr(UVFSFileNode Node, const char *attr, UVFSFSAttributeValue *val, size_t len, size_t *retlen)
{
    VERIFY_NODE_IS_VALID(Node);

    NodeRecord_s* psNodeRecord      = (NodeRecord_s*)Node;

    if (attr == NULL || val == NULL)
        return EINVAL;
    
    if (strcmp(attr, UVFS_FSATTR_PC_LINK_MAX)==0) 
    {
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = 1; // Unsupported
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_PC_NAME_MAX)==0) 
    {
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = WIN_MAXLEN;
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_PC_NO_TRUNC)==0) 
    {
        *retlen = sizeof(bool);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_bool = true;
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_PC_FILESIZEBITS)==0) 
    {
        // The number of bits used to represent the size (in bytes) of a file
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = 33;
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_PC_XATTR_SIZE_BITS)==0) 
    {
        // The number of bits used to represent the size (in bytes) of an extended attribute - NOT SUPPORTED
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = 0;
        return ENOTSUP;
    }

    if (strcmp(attr, UVFS_FSATTR_BLOCKSIZE)==0) 
    {
        // Size (in bytes) of a fundamental file system block
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = psNodeRecord->sRecordData.psFSRecord->sFSInfo.uBytesPerCluster;
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_IOSIZE)==0) 
    {
        // Size (in bytes) of the optimal transfer block size
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = 32 * 1024; // TBD
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_TOTALBLOCKS)==0) 
    {
        // Total number of file system blocks
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = psNodeRecord->sRecordData.psFSRecord->sFSInfo.uMaxCluster; 
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_BLOCKSFREE)==0) 
    {
        // Total number of free file system blocks
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = psNodeRecord->sRecordData.psFSRecord->sFSInfo.uFreeClusters;
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_BLOCKSAVAIL)==0) 
    {
        // Total number of free file system blocks available for allocation to files (in our case - the same as UVFS_FSATTR_BLOCKSFREE)
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = psNodeRecord->sRecordData.psFSRecord->sFSInfo.uFreeClusters;
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_BLOCKSUSED)==0) 
    {
        // Number of file system blocks currently allocated for some use (TOTAL_BLOCKS - BLOCKSAVAIL)
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = psNodeRecord->sRecordData.psFSRecord->sFSInfo.uMaxCluster - psNodeRecord->sRecordData.psFSRecord->sFSInfo.uFreeClusters; 
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_CNAME)==0)
    {
        //The file name
        if (psNodeRecord->sRecordData.pcUtf8Name ==  NULL)
            return EINVAL;
        
        *retlen = strlen(psNodeRecord->sRecordData.pcUtf8Name) + 1;
        if (len < *retlen)
        {
            return E2BIG;
        }
        strlcpy(val->fsa_string, psNodeRecord->sRecordData.pcUtf8Name,*retlen);
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_FSTYPENAME)==0) 
    {
        *retlen = 4;
        if (len < *retlen) 
        {
            return E2BIG;
        }
        // A string representing the type of file system
        strcpy(val->fsa_string, "fat");
        *(val->fsa_string+3) = 0; // Must be null terminated
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_FSSUBTYPE)==0) 
    {
        // A string representing the variant of the file system
        *retlen = 6;
        if (len < *retlen) 
        {
            return E2BIG;
        }

        switch (psNodeRecord->sRecordData.psFSRecord->sFatInfo.uFatMask) 
        {
            case FAT12_MASK:
                strcpy(val->fsa_string, "fat12");
                break;
            case FAT16_MASK:
                strcpy(val->fsa_string, "fat16");
                break;
            case FAT32_MASK:
                strcpy(val->fsa_string, "fat32");
                break;
            default:
                break;
        }
        *(val->fsa_string+5) = 0; // Must be null terminated
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_VOLNAME)==0) 
    {
        if (psNodeRecord->sRecordData.psFSRecord->sFSInfo.bVolumeLabelExist == false)
        {
            return ENOENT;
        }

        *retlen = strlen((char *)psNodeRecord->sRecordData.psFSRecord->sFSInfo.uVolumeLabel)+1; // Add 1 for the NULL terminator
        if (len < *retlen) 
        {
            return E2BIG;
        }
        strcpy(val->fsa_string, (char *)psNodeRecord->sRecordData.psFSRecord->sFSInfo.uVolumeLabel);
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_VOLUUID)==0) 
    {
        if (psNodeRecord->sRecordData.psFSRecord->sFSInfo.bUUIDExist == false)
        {
            return ENOENT;
        }

        *retlen = sizeof(uuid_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        memcpy(val->fsa_opaque, psNodeRecord->sRecordData.psFSRecord->sFSInfo.sUUID, sizeof(uuid_t));
        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_CAPS_FORMAT)==0) 
    {
        // A bitmask indicating the capabilities of the volume format
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }

        val->fsa_number = VOL_CAP_FMT_SYMBOLICLINKS |
                          VOL_CAP_FMT_NO_ROOT_TIMES |
                          VOL_CAP_FMT_CASE_PRESERVING |
                          VOL_CAP_FMT_HIDDEN_FILES |
                          VOL_CAP_FMT_NO_PERMISSIONS;

        return 0;
    }

    if (strcmp(attr, UVFS_FSATTR_CAPS_INTERFACES)==0) 
    {
        // A bitmask indicating the interface capabilities of the file system
        *retlen = sizeof(uint64_t);
        if (len < *retlen) 
        {
            return E2BIG;
        }
        val->fsa_number = 0;
        return 0;
    }

    return ENOTSUP;
}

static int
MSDOS_SetFSAttr(UVFSFileNode Node, const char *attr, const UVFSFSAttributeValue *val, size_t len, UVFSFSAttributeValue *out_value, size_t out_len)
{
    VERIFY_NODE_IS_VALID(Node);
    
    if (attr == NULL || val == NULL || out_value == NULL) return EINVAL;
    
    if (strcmp(attr, LI_FSATTR_PREALLOCATE) == 0)
    {
        if (len < sizeof (LIFilePreallocateArgs_t) || out_len < sizeof (LIFilePreallocateArgs_t))
            return EINVAL;
        
        LIFilePreallocateArgs_t* psPreAllocReq = (LIFilePreallocateArgs_t*) ((void *) val->fsa_opaque);
        LIFilePreallocateArgs_t* psPreAllocRes = (LIFilePreallocateArgs_t*) ((void *) out_value->fsa_opaque);
        
        memcpy (psPreAllocRes, psPreAllocReq, sizeof(LIFilePreallocateArgs_t));
        return FILEOPS_PreAllocateClusters(Node, psPreAllocReq, psPreAllocRes);
    }
    
    // Reserved for future use
    return ENOTSUP;
}

static int
setup_posix_file_action_for_fsck(posix_spawn_file_actions_t *file_actions, int fd)
{
    int error;

    if (file_actions == NULL || fd < 0)
    {
        return EINVAL;
    }

    error = posix_spawn_file_actions_init(file_actions);
    if (error)
    {
        goto out;
    }

    error = posix_spawn_file_actions_addinherit_np(file_actions, 0);
    if (error)
    {
        goto out;
    }
    error = posix_spawn_file_actions_addinherit_np(file_actions, 1);
    if (error)
    {
        goto out;
    }

    error = posix_spawn_file_actions_addinherit_np(file_actions, 2);
    if (error)
    {
        goto out;
    }
    error = posix_spawn_file_actions_addinherit_np(file_actions, fd);

out:
    return error;
}

static int
setup_spawnattr_for_fsck(posix_spawnattr_t *spawn_attr)
{
    int error;

    error = posix_spawnattr_init(spawn_attr);
    if (error)
    {
        goto out;
    }
    error = posix_spawnattr_setflags(spawn_attr, POSIX_SPAWN_CLOEXEC_DEFAULT);

out:
    return error;
}

#define PATH_TO_FSCK "/System/Library/Filesystems/msdos.fs/Contents/Resources/fsck_msdos"

static int
fsck_msdos(int fd,  check_flags_t how)
{
    pid_t child;
    pid_t child_found;
    int child_status;
    extern char **environ;
    char fdescfs_path[24];
    posix_spawn_file_actions_t file_actions;
    int result;
    posix_spawnattr_t spawn_attr;

    MSDOS_LOG(LEVEL_DEBUG, "fsck_msdos - fsck start for %d", fd);
    snprintf(fdescfs_path, sizeof(fdescfs_path), "/dev/fd/%d", fd);
    const char * argv[] = {"fsck_msdos", "-q", fdescfs_path, NULL};

    switch (how)
    {
        case QUICK_CHECK:
            /* Do nothing, already setup for this */
            break;
        case CHECK:
            argv[1] = "-n";
            break;
        case CHECK_AND_REPAIR:
            argv[1] = "-y";
            break;
        default:
            MSDOS_LOG(LEVEL_ERROR, "Invalid how flags for the check, ignoring; %d", how);
            break;
    }

    result = setup_posix_file_action_for_fsck(&file_actions, fd);
    if (result)
    {
        goto out;
    }

    result = setup_spawnattr_for_fsck(&spawn_attr);
    if (result)
    {
        posix_spawn_file_actions_destroy(&file_actions);
        goto out;
    }

    result = posix_spawn(&child,
                         PATH_TO_FSCK,
                         &file_actions,
                         &spawn_attr,
                         (char * const *)argv,
                         environ);
    
    posix_spawn_file_actions_destroy(&file_actions);
    posix_spawnattr_destroy(&spawn_attr);
    if (result)
    {
        MSDOS_LOG(LEVEL_ERROR, "posix_spawn fsck_msdos: error=%d", result);
        goto out;
    }

    // Wait for child to finish, XXXab: revisit, need sensible timeout?
    do {
        child_found = waitpid(child, &child_status, 0);
    } while (child_found == -1 && errno == EINTR);

    if (child_found == -1)
    {
        result = errno;
        MSDOS_LOG(LEVEL_ERROR, "waitpid fsck_msdos: errno=%d", result);
        goto out;
    }

    if (WIFEXITED(child_status))
    {
        result = WEXITSTATUS(child_status);
        if (result)
        {
            MSDOS_LOG(LEVEL_ERROR, "fsck_msdos: exited with status %d", result);
            result = EILSEQ;
        }
    }
    else
    {
        result = WTERMSIG(child_status);
        MSDOS_LOG(LEVEL_ERROR, "fsck_msdos: terminated by signal %d", result);
        result = EINTR;
    }

out:
    return result;
}

static int
MSDOS_check(int fdToCheck, __unused UVFSVolumeId volId,
    __unused UVFSVolumeCredential *volumeCreds, check_flags_t how)
{
    //XXXab: for now just spawn fsck_msdos for checking
    return fsck_msdos(fdToCheck, how);
}

void livefiles_plugin_init(UVFSFSOps **ops);

UVFSFSOps MSDOS_fsOps = {
    .fsops_version      = UVFS_FSOPS_VERSION_CURRENT,
    .fsops_init         = MSDOS_Init,
    .fsops_fini         = MSDOS_Fini,
    .fsops_taste        = MSDOS_Taste,
    .fsops_scanvols     = MSDOS_ScanVols,
    .fsops_mount        = MSDOS_Mount,
    .fsops_unmount      = MSDOS_Unmount,
    .fsops_getfsattr    = MSDOS_GetFSAttr,
    .fsops_setfsattr    = MSDOS_SetFSAttr,
    .fsops_getattr      = MSDOS_GetAttr,
    .fsops_setattr      = MSDOS_SetAttr,
    .fsops_lookup       = MSDOS_Lookup,
    .fsops_reclaim      = MSDOS_Reclaim,
    .fsops_readlink     = MSDOS_ReadLink,
    .fsops_read         = MSDOS_Read,
    .fsops_write        = MSDOS_Write,
    .fsops_create       = MSDOS_Create,
    .fsops_mkdir        = MSDOS_MkDir,
    .fsops_symlink      = MSDOS_SymLink,
    .fsops_remove       = MSDOS_Remove,
    .fsops_rmdir        = MSDOS_RmDir,
    .fsops_rename       = MSDOS_Rename,
    .fsops_readdir      = MSDOS_ReadDir,
    .fsops_readdirattr  = MSDOS_ReadDirAttr,
    .fsops_sync         = MSDOS_Sync,
    .fsops_link         = MSDOS_Link,
    .fsops_check        = MSDOS_check,
    .fsops_scandir      = MSDOS_ScanDir,
};

__attribute__((visibility("default")))
void livefiles_plugin_init(UVFSFSOps **ops)
{
    if (ops) {
        *ops = &MSDOS_fsOps;
    }

    return;
}
