//
//  lf_hfs_file_mgr_internal.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_file_mgr_internal_h
#define lf_hfs_file_mgr_internal_h

#include <MacTypes.h>
#include "lf_hfs.h"
#include "lf_hfs_defs.h"
#include "lf_hfs_format.h"
#include "lf_hfs_cnode.h"


/* CatalogNodeID is used to track catalog objects */
typedef u_int32_t        HFSCatalogNodeID;

/* internal error codes*/
#define ERR_BASE    -32767

enum {

    /* FXM errors*/
    fxRangeErr                      = ERR_BASE + 16,                /* file position beyond mapped range*/
    fxOvFlErr                       = ERR_BASE + 17,                /* extents file overflow*/

    /* Unicode errors*/
    uniTooLongErr                   = ERR_BASE + 24,                /* Unicode string too long to convert to Str31*/
    uniBufferTooSmallErr            = ERR_BASE + 25,                /* Unicode output buffer too small*/
    uniNotMappableErr               = ERR_BASE + 26,                /* Unicode string can't be mapped to given script*/

    /* BTree Manager errors*/
    btNotFound                      = ERR_BASE + 32,                /* record not found*/
    btExists                        = ERR_BASE + 33,                /* record already exists*/
    btNoSpaceAvail                  = ERR_BASE + 34,                /* no available space*/
    btNoFit                         = ERR_BASE + 35,                /* record doesn't fit in node */
    btBadNode                       = ERR_BASE + 36,                /* bad node detected*/
    btBadHdr                        = ERR_BASE + 37,                /* bad BTree header record detected*/
    dsBadRotate                     = ERR_BASE + 64,                /* bad BTree rotate*/

    /* Catalog Manager errors*/
    cmNotFound                      = ERR_BASE + 48,                /* CNode not found*/
    cmExists                        = ERR_BASE + 49,                /* CNode already exists*/
    cmNotEmpty                      = ERR_BASE + 50,                /* directory CNode not empty (valence = 0)*/
    cmRootCN                        = ERR_BASE + 51,                /* invalid reference to root CNode*/
    cmBadNews                       = ERR_BASE + 52,                /* detected bad catalog structure*/
    cmFThdDirErr                    = ERR_BASE + 53,                /* thread belongs to a directory not a file*/
    cmFThdGone                      = ERR_BASE + 54,                /* file thread doesn't exist*/
    cmParentNotFound                = ERR_BASE + 55,                /* CNode for parent ID does not exist*/

    /* TFS internal errors*/
    fsDSIntErr                      = -127                          /* Internal file system error*/
};


/* internal flags*/

enum {
    kEFAllMask          = 0x01,     /* allocate all requested bytes or none */
    kEFContigMask       = 0x02,     /* force contiguous allocation */
    kEFReserveMask      = 0x04,     /* keep block reserve */
    kEFDeferMask        = 0x08,     /* defer file block allocations */
    kEFNoClumpMask      = 0x10,     /* don't round up to clump size */
    kEFMetadataMask     = 0x20,     /* metadata allocation */

    kTFTrunExtBit       = 0,        /*    truncate to the extent containing new PEOF*/
    kTFTrunExtMask      = 1
};

enum {
    kUndefinedStrLen    = 0,        /* Unknown string length */
    kNoHint             = 0,

    /*    FileIDs variables*/
    kNumExtentsToCache            = 4                                /*    just guessing for ExchangeFiles*/
};


/* Universal Extent Key */

typedef union ExtentKey {
    HFSExtentKey                     hfs;
    HFSPlusExtentKey                 hfsPlus;
} ExtentKey;

/* Universal extent descriptor */
typedef union ExtentDescriptor {
    HFSExtentDescriptor             hfs;
    HFSPlusExtentDescriptor         hfsPlus;
} ExtentDescriptor;

/* Universal extent record */
typedef union ExtentRecord {
    HFSExtentRecord                 hfs;
    HFSPlusExtentRecord             hfsPlus;
} ExtentRecord;


enum {
    CMMaxCName = kHFSMaxFileNameChars
};


/* Universal catalog name*/
typedef union CatalogName {
    Str31                             pstr;
    HFSUniStr255                     ustr;
} CatalogName;


#define GetFileControlBlock(fref)        VTOF((fref))
#define GetFileRefNumFromFCB(fcb)        FTOV((fcb))

#define ReturnIfError(result)   do {    if ( (result) != noErr ) return (result);   } while(0)
#define ExitOnError(result)     do {    if ( (result) != noErr ) goto ErrorExit;    } while(0)



/* Catalog Manager Routines (IPI)*/
OSErr ExchangeFileIDs(      ExtendedVCB     *volume,
                      ConstUTF8Param   srcName,
                      ConstUTF8Param   destName,
                      HFSCatalogNodeID srcID,
                      HFSCatalogNodeID destID,
                      u_int32_t        srcHint,
                      u_int32_t        destHint );

OSErr MoveData( ExtendedVCB *vcb, HFSCatalogNodeID srcID, HFSCatalogNodeID destID, int rsrc);

/* BTree Manager Routines*/
typedef int32_t (*KeyCompareProcPtr)(void *a, void *b);

OSErr ReplaceBTreeRecord(   FileReference   refNum,
                         const void      *key,
                         u_int32_t       hint,
                         void            *newData,
                         u_int16_t       dataSize,
                         u_int32_t       *newHint );


/*    Prototypes for exported routines in VolumeAllocation.c*/

/*
 * Flags for BlockAllocate(), BlockDeallocate() and hfs_block_alloc.
 * Some of these are for internal use only.  See the comment at the
 * top of hfs_alloc_int for more details on the semantics of these
 * flags.
 */
#define HFS_ALLOC_FORCECONTIG           0x001    //force contiguous block allocation; minblocks must be allocated
#define HFS_ALLOC_METAZONE              0x002    //can use metazone blocks
#define HFS_ALLOC_SKIPFREEBLKS          0x004    //skip checking/updating freeblocks during alloc/dealloc
#define HFS_ALLOC_FLUSHTXN              0x008    //pick best fit for allocation, even if a jnl flush is req'd
#define HFS_ALLOC_TENTATIVE             0x010    //reserved allocation that can be claimed back
#define HFS_ALLOC_LOCKED                0x020    //reserved allocation that can't be claimed back
#define HFS_ALLOC_IGNORE_TENTATIVE      0x040    //Steal tentative blocks if necessary
#define HFS_ALLOC_IGNORE_RESERVED       0x080    //Ignore tentative/committed blocks
#define HFS_ALLOC_USE_TENTATIVE         0x100    //Use the supplied tentative range (if possible)
#define HFS_ALLOC_COMMIT                0x200    //Commit the supplied extent to disk
#define HFS_ALLOC_TRY_HARD              0x400    //Search hard to try and get maxBlocks; implies HFS_ALLOC_FLUSHTXN
#define HFS_ALLOC_ROLL_BACK             0x800    //Reallocate blocks that were just deallocated
//#define HFS_ALLOC_FAST_DEV              0x1000  //Prefer fast device for allocation

typedef uint32_t hfs_block_alloc_flags_t;


OSErr BlockAllocate(        ExtendedVCB             *vcb,
                    u_int32_t               startingBlock,
                    u_int32_t               minBlocks,
                    u_int32_t               maxBlocks,
                    hfs_block_alloc_flags_t flags,
                    u_int32_t               *startBlock,
                    u_int32_t               *actualBlocks );

struct rl_entry;
typedef struct hfs_alloc_extra_args {
    // Used with HFS_ALLOC_TRY_HARD and HFS_ALLOC_FORCECONTIG
    uint32_t                max_blocks;

    // Used with with HFS_ALLOC_USE_TENTATIVE & HFS_ALLOC_COMMIT
    struct rl_entry          **reservation_in;

    // Used with HFS_ALLOC_TENTATIVE & HFS_ALLOC_LOCKED
    struct rl_entry          **reservation_out;

    /*
     * If the maximum cannot be returned, the allocation will be
     * trimmed to the specified alignment after taking
     * @alignment_offset into account.  @alignment and
     * @alignment_offset are both in terms of blocks, *not* bytes.
     * The result will be such that:
     *
     *   (block_count + @alignment_offset) % @alignment == 0
     *
     * Alignment is *not* guaranteed.
     *
     * One example where alignment might be useful is in the case
     * where the page size is greater than the allocation block size
     * and I/O is being performed in multiples of the page size.
     */
    int                        alignment;
    int                        alignment_offset;
} hfs_alloc_extra_args_t;

/*
 * Same as BlockAllocate but slightly different API.
 * @extent.startBlock is a hint for where to start searching and
 * @extent.blockCount is the minimum number of blocks acceptable.
 * Additional arguments can be passed in @extra_args and use will
 * depend on @flags.  See comment at top of hfs_block_alloc_int for
 * more information.
 */
errno_t hfs_block_alloc(    hfsmount_t              *hfsmp,
                        HFSPlusExtentDescriptor *extent,
                        hfs_block_alloc_flags_t flags,
                        hfs_alloc_extra_args_t  *extra_args );

OSErr BlockDeallocate(  ExtendedVCB             *vcb,
                      u_int32_t               firstBlock,
                      u_int32_t               numBlocks,
                      hfs_block_alloc_flags_t flags );

OSErr BlockMarkAllocated( ExtendedVCB *vcb, u_int32_t startingBlock, u_int32_t numBlocks );

OSErr BlockMarkFree( ExtendedVCB *vcb, u_int32_t startingBlock, u_int32_t numBlocks );

OSErr BlockMarkFreeUnused( ExtendedVCB *vcb, u_int32_t startingBlock, u_int32_t numBlocks );

u_int32_t MetaZoneFreeBlocks( ExtendedVCB *vcb );

u_int32_t ScanUnmapBlocks( struct hfsmount *hfsmp );

int hfs_init_summary( struct hfsmount *hfsmp );

errno_t hfs_find_free_extents( struct hfsmount *hfsmp, void (*callback)(void *data, off_t), void *callback_arg );

void hfs_free_tentative( hfsmount_t *hfsmp, struct rl_entry **reservation );

void hfs_free_locked( hfsmount_t *hfsmp, struct rl_entry **reservation );

/*    Get the current time in UTC (GMT)*/
u_int32_t GetTimeUTC( void );

u_int32_t LocalToUTC( u_int32_t localTime );

u_int32_t UTCToLocal( u_int32_t utcTime );

#endif /* lf_hfs_file_mgr_internal_h */
