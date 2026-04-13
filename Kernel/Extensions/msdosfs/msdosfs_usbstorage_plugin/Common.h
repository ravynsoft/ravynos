/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  Common.h
 *  usbstorage_plugin
 */

#ifndef Common_h
#define Common_h

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include "bootsect.h"
#include "bpb.h"
#include "direntry.h"
#include "fat.h"
#import  <os/log.h>
#include <err.h>
#include <assert.h>
#include <sys/errno.h>
#include <sysexits.h>
#include <UserFS/UserVFS.h>
#include "MR_SW_Lock.h"
#include <stdatomic.h>

#define likely(x)               __builtin_expect(!!(x), 1)
#define unlikely(x)             __builtin_expect(!!(x), 0)
#define clz(_x)                 __builtin_clz(_x)
#define ctz(_x)                 __builtin_ctz(_x)
#define ffs(_x)                 __builtin_ffs(_x)
#define popcount(_x)            __builtin_popcount(_x)

#undef MAX
#define MAX(_x, _y)           (((_x) > (_y))? (_x) : (_y))
#undef MIN
#define MIN(_x, _y)           (((_x) < (_y))? (_x) : (_y))

#undef ROUND_DOWN
#define ROUND_DOWN(_x, _m)    (((_x) / (_m)) * (_m))

#undef ROUND_UP
#define ROUND_UP(_x, _m)      ROUND_DOWN((_x) + (_m) - 1, (_m))



#define GET_RECORD(FileNode) ((NodeRecord_s*) FileNode)
#define GET_FSRECORD(psNodeRecord) ((FileSystemRecord_s*) psNodeRecord->sRecordData.psFSRecord)
#define CLUSTER_SIZE(psFSRecord) (psFSRecord->sFSInfo.uBytesPerCluster)
#define SECTOR_SIZE(psFSRecord) (psFSRecord->sFSInfo.uBytesPerSector)

#define FAT_CACHE_SIZE (4)
#define MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY (10)
#define MAX_CHAIN_CACHE_ENTRIES (100)

#define SYMLINK_LENGTH_LENGTH (4)
#define SYMLINK_MAGIC_LENGTH (5)

#define FAT_MAX_FILENAME_UTF8  (WIN_MAXLEN * 3 + 1)

#define IS_DIR(psNodeRecord) ( (((psNodeRecord->sRecordData.eRecordID == RECORD_IDENTIFIER_DIR) || \
                                    (psNodeRecord->sRecordData.eRecordID == RECORD_IDENTIFIER_ROOT))) ? (1) : (0) )

#define IS_SYMLINK(psNodeRecord) ((psNodeRecord->sRecordData.eRecordID == RECORD_IDENTIFIER_LINK)? (1) : (0))

/*
 * The maximum file size on FAT is 4GB-1, which is the largest value that fits
 * in an unsigned 32-bit integer.
 */
#define    DOS_FILESIZE_MAX    0xffffffff

#define MODE_WRITABLE       (UVFS_FA_MODE_OTH(UVFS_FA_MODE_W) | UVFS_FA_MODE_GRP(UVFS_FA_MODE_W) | UVFS_FA_MODE_USR(UVFS_FA_MODE_W))

typedef enum
{
    RECORD_IDENTIFIER_UNKNOWN,
    RECORD_IDENTIFIER_ROOT      = 1,
    RECORD_IDENTIFIER_DIR,
    RECORD_IDENTIFIER_FILE,
    RECORD_IDENTIFIER_LINK,
    RECORD_IDENTIFIER_AMOUNT
    
} RecordIdentifier_e;

struct unistr255 {
    uint16_t length;
    uint16_t chars[255];
};

/* -------------------------------------------------------------------------------------
 *                                  Cluster Chain cache
 * ------------------------------------------------------------------------------------- */
typedef struct {
    uint32_t uActualStartCluster;
    uint32_t uClusterOffsetFromFileStart;
    uint32_t uAmountOfConsecutiveClusters;

} ConsecutiveClusterChacheEntry_s;

typedef struct sConsecutiveCluster
{
    uint64_t                        uFileOffset;                    // Offset within the file this cluster chain represents
    uint32_t                        uAmountOfClusters;              // Amount of clusters this cluster chain represents
    struct sConsecutiveCluster*     psPrevClusterChainCacheEntry;   // Pointer to the prev cache entry related to this file
    struct sConsecutiveCluster*     psNextClusterChainCacheEntry;   // Pointer to the next cache entry related to this file
    ConsecutiveClusterChacheEntry_s psConsecutiveCluster[MAX_CHAIN_CACHE_ELEMENTS_PER_ENTRY]; // Cluster chain array
    struct sConsecutiveCluster*     psLRUPrev;                      // Pointer to track LRU
    struct sConsecutiveCluster*     psLRUNext;                      // Pointer to track LRU
    void*                           pvFileOwner;                    // Pointer to the FileRecord this cache represent
    uint64_t                        uLRUCounter;

} ClusterChainCacheEntry_s;

typedef struct {
    uint32_t                        uAmountOfEntries;         // Amount of used entries in the cache
    MultiReadSingleWriteHandler_s   sClusterChainLck;
    ClusterChainCacheEntry_s*       psChainCacheLRU;          // Pointer to the LRU entry in the cache
    ClusterChainCacheEntry_s*       psChainCacheMRU;          // Pointer to the MRU entry in the cache
    volatile atomic_uint_least64_t  uMaxLRUCounter;

} ClusterChainCache_s;

/* -------------------------------------------------------------------------------------
 *                                      FAT cache
 * ------------------------------------------------------------------------------------- */

typedef struct
{
    void*       pvFatEntryCache;                     // most recently used block of the FAT
    uint64_t    uFatCacheEntryOffset;                // which block is being cached; relative to start of active FAT
    uint64_t    uLRUCounter;
    bool        bIsDirty;

} FatCacheEntry_s;

typedef struct
{
    pthread_mutex_t                 sFatMutex;
    pthread_mutex_t                 sAlterFatMutex;
    uint64_t                        uLRUCounter;
    uint8_t                         uAmountOfAllocatedCacheEntries;
    FatCacheEntry_s                 psFATCacheEntries[FAT_CACHE_SIZE];
    FatCacheEntry_s                 psAlterFATCacheEntries[FAT_CACHE_SIZE];
    bool                            bDriveDirtyBit;

} VolumeFatCache_s;

/* -------------------------------------------------------------------------------------
 *                                Naming Hash Table cache
 * ------------------------------------------------------------------------------------- */
#define HASH_TABLE_SIZE  (64)

typedef struct LF_TableEntry /* hashtable entry */
{
    struct LF_TableEntry* psNextEntry;
    uint64_t           uEntryNum;
}LF_TableEntry_t;

typedef struct LF_HashTable
{
    uint32_t                        uCounter;
    bool                            bIncomplete;
    MultiReadSingleWriteHandler_s   sHTLck;
    LF_TableEntry_t* psEntries[HASH_TABLE_SIZE];
}LF_HashTable_t;

/* -------------------------------------------------------------------------------------
 *                                  File System Record
 * ------------------------------------------------------------------------------------- */

#if DIAGNOSTIC
#define DIAGNOSTIC_CACHE_SIZE (256)

typedef struct FRHashElement
{
    uint64_t uParentStartCluster;
    struct unistr255 sName;

    struct FRHashElement* psNext;
    struct FRHashElement* psPrev;
}FRHashElement_s;

typedef struct
{
    FRHashElement_s* psDiagnosticCache[DIAGNOSTIC_CACHE_SIZE];
    MultiReadSingleWriteHandler_s   sCacheLock;
}DiagnosticDB_s;
#endif


typedef struct
{
    uint32_t uFatOffset;               // Offset, in bytes, from start of volume to start of active FAT
    uint32_t uFatSize;                 // Size, in bytes, of the FAT
    uint32_t uFatMask;                 // Mask to use for cluster numbers
    uint8_t  uNumOfFATs;               // Number of FAT copies

} FatInfo_s;

typedef struct
{
    uint32_t uRootSector;              // Offset, in sectors, from start of volume to start of root directory
    uint32_t uRootSize;                // Size, in sectors, of root directory
    uint32_t uRootLength;              // Size, in bytes, of root directory
    uint32_t uRootCluster;             // Cluster number of first cluster of root directory (FAT32 only)

} RootInfo_s;

typedef struct
{
    size_t   sDirBlockSize;            // Size in bytes of a single cached block of a directory
    uint32_t uBytesPerSector;
    uint32_t uBytesPerCluster;
    uint32_t uClusterOffset;           // Offset, in sectors, from start of volume to start of first cluster
    uint32_t uMaxCluster;              // Largest valid cluster number (_clusterCount + 1)
    uint32_t uFsInfoSector;            // FAT32 only; zero if no valid FSInfo sector
    uint32_t uFreeClusters;            // FAT32 only; only used if fsInfoSector != 0
    uint32_t uFirstFreeCluster;
    uint8_t  uVolumeLabel[64];         // Volume label (name)
    bool     bVolumeLabelExist;        // Found Volume label
    uuid_t   sUUID;                    // UUID (generated using volume serial number)
    bool     bUUIDExist;               // UUID exists

} FSInfo_s;

typedef struct
{
    struct timespec sLastUsed;
    void*           psOwningNode;
    bool            bBusy;
} DirHTState_s;

#define DIR_LRU_TABLE_MAX           5

typedef struct
{
    uint32_t (*uFatEntryOffset)(uint32_t cluster);
    uint32_t (*uFatEntryGet)(uint32_t cluster, uint8_t *entry);
    uint32_t (*uFatEntrySet)(uint32_t cluster, uint8_t *entry, uint32_t value);

} FSOperation_s;

typedef struct
{
    int                             iFD;                    // File descriptor as recivied from usbstoraged
    FatInfo_s                       sFatInfo;               // Info about the FAT type
    RootInfo_s                      sRootInfo;              // Root Directory info
    FSInfo_s                        sFSInfo;                // File system info
    FSOperation_s                   sFSOperations;          // Operation related to the FAT type
    VolumeFatCache_s                sFATCache;              // FAT cache
    ClusterChainCache_s*            psClusterChainCache;    // Pointer to the cluster chain cache
    MultiReadSingleWriteHandler_s   sDirEntryAccessLock;
    MultiReadSingleWriteHandler_s   sDirtyBitLck;           /* Dirty Bit Lock - prevent false status while sync
                                                             * When metadata is changed/flushed, need to acquire this lock.
                                                             * <rdar://problem/45664056> - in order to prevent deadlock,
                                                             * need to acquire the DirtyBit Lock before locking the NodeRecord */
    volatile atomic_uint_least64_t  uPreAllocatedOpenFiles;

    void*                           pvFSInfoCluster;        //FsInfoSector to allocate in case of FAT32

    // Locking order: NodeRecord -> sDirHTLRUTableLock
    MultiReadSingleWriteHandler_s   sDirHTLRUTableLock;     // XXXJRT should be a mutex
    DirHTState_s                    sDirHTLRUTable[DIR_LRU_TABLE_MAX];
    volatile atomic_uint_least64_t  uDirHTGeneration;

#if DIAGNOSTIC
    DiagnosticDB_s sDiagnosticDB;
#endif
} FileSystemRecord_s;

/* -------------------------------------------------------------------------------------
 *                                  File Record
 * ------------------------------------------------------------------------------------- */


typedef struct
{
    struct dosdirentry  sDosDirEntry;
    
    uint32_t            uClusterSize;
    uint32_t            uFirstClusterIdxInChain;
    uint64_t            uFirstEntryOffset;
    uint32_t            uRelFirstEntryOffset;
    uint32_t            uNumberOfDirEntriesForNode;
    uint64_t            uLastClusterAbsoluteOffset;
    uint64_t            uLastEntryOffsetInCluster;
    
} NodeDirEntriesData_s;


typedef struct
{
    uint32_t    uClusterNum;
    uint8_t*    puClusterData;
    bool        bIsAllocated;
    bool        bIsValid;
    uint64_t    uLength;
    uint64_t    uAbsoluteClusterOffset;
    uint32_t    uAbsoluteClusterNum;
    uint64_t    uLocalDirHTGeneration;

} ClusterData_s;

#define NUM_OF_COND (10)
typedef struct
{
    pthread_cond_t sCond;
    uint64_t uSectorNum;
} RecordDataCond_t;

typedef struct FileRecord_s
{
    MultiReadSingleWriteHandler_s   sRecordLck;
    pthread_mutex_t                 sUnAlignedWriteLck;
    RecordDataCond_t                sCondTable[NUM_OF_COND];

    RecordIdentifier_e              eRecordID;
    FileSystemRecord_s*             psFSRecord;
    ClusterChainCacheEntry_s*       psClusterChain;             // First cluster chain cache entry
    NodeDirEntriesData_s            sNDE;
    bool                            bIsNDEValid;
    uint32_t                        uFirstCluster;              // First data cluster of this record
    uint32_t                        uLastAllocatedCluster;
    uint32_t                        uClusterChainLength;

    char*                           pcUtf8Name;

#ifdef DIAGNOSTIC
    struct unistr255                sName;
    bool bRemovedFromDiag;  // Special case for rename.
    uint64_t uParentCluster;
#endif

}RecordData_s;

typedef struct
{
    uint64_t        uDirVersion;
    ClusterData_s*  psDirClusterData;
    LF_HashTable_t* sHT;
    DirHTState_s*   psDirHTLRUSlot;
    unsigned int    uHTBusyCount;
#ifdef MSDOS_NLINK_IS_CHILD_COUNT
    uint32_t        uChildCount;
#endif
} DirData_s;

typedef struct
{
    uint32_t  uSymLinkLength;

} SymLinkData_s;

typedef struct
{
    bool bIsPreAllocated;
} FileData_s;

typedef struct
{
    uint32_t                uValidNodeMagic1;

    RecordData_s            sRecordData;
    union
    {
        DirData_s       sDirData;
        SymLinkData_s   sSymLinkData;
        FileData_s      sFileData;
    } sExtraData;

    uint32_t                uValidNodeMagic2;

} NodeRecord_s;


#define VALID_NODE_MAGIC        (0xC0BEC0A7)
#define VALID_NODE_BADMAGIC     (0xDEADDAA7)
#define INVALIDATE_NODE(psNodeRecord)                                                   \
    do {                                                                                \
        if ( psNodeRecord != NULL ) {                                                   \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic1 = VALID_NODE_BADMAGIC;      \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic2 = VALID_NODE_BADMAGIC;      \
        }                                                                               \
    } while(0)
#define SET_NODE_AS_VALID(psNodeRecord)                                             \
    do {                                                                            \
        if ( psNodeRecord != NULL ) {                                               \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic1 = VALID_NODE_MAGIC;     \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic2 = VALID_NODE_MAGIC;     \
        }                                                                           \
    } while(0)
#define VERIFY_NODE_IS_VALID(psNodeRecord)                                              \
    do {                                                                                \
        if ((psNodeRecord) &&                                                           \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic1 == VALID_NODE_BADMAGIC &&   \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic2 == VALID_NODE_BADMAGIC ) {  \
            MSDOS_LOG( LEVEL_ERROR, "Got stale node" );                                 \
            return ESTALE;                                                              \
        }                                                                               \
        if ((psNodeRecord == NULL) ||                                                   \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic1 != VALID_NODE_MAGIC ||      \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic2 != VALID_NODE_MAGIC ) {     \
            MSDOS_LOG( LEVEL_ERROR, "Got invalid node" );                               \
            return EINVAL;                                                              \
        }                                                                               \
    } while(0)
#define VERIFY_NODE_IS_VALID_FOR_RECLAIM(psNodeRecord)                                              \
do {                                                                                                \
    if ((psNodeRecord == NULL)                                                          ||          \
        (   ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic1 != VALID_NODE_MAGIC &&                  \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic1 != VALID_NODE_BADMAGIC )    ||          \
        (   ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic2 != VALID_NODE_MAGIC &&                  \
            ((NodeRecord_s*)psNodeRecord)->uValidNodeMagic2 != VALID_NODE_BADMAGIC )) {             \
        MSDOS_LOG( LEVEL_ERROR, "Got invalid node for reclaim" );                                   \
        return EINVAL;                                                                              \
    }                                                                                               \
} while(0)

#define CLUSTER_IS_VALID(cluster, psFSRecord) ((cluster >= CLUST_FIRST && cluster <= ((FileSystemRecord_s*) psFSRecord)->sFSInfo.uMaxCluster))

#endif /* Common_h */
