//
//  lf_hfs_btrees_internal.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_btrees_internal_h
#define lf_hfs_btrees_internal_h

#include "lf_hfs_file_mgr_internal.h"

enum {
    fsBTInvalidHeaderErr                = btBadHdr,
    fsBTBadRotateErr                    = dsBadRotate,
    fsBTInvalidNodeErr                  = btBadNode,
    fsBTRecordTooLargeErr               = btNoFit,
    fsBTRecordNotFoundErr               = btNotFound,
    fsBTDuplicateRecordErr              = btExists,
    fsBTFullErr                         = btNoSpaceAvail,

    fsBTInvalidFileErr                  = ERR_BASE + 0x0302,    /* no BTreeCB has been allocated for fork*/
    fsBTrFileAlreadyOpenErr             = ERR_BASE + 0x0303,
    fsBTInvalidIteratorErr              = ERR_BASE + 0x0308,
    fsBTEmptyErr                        = ERR_BASE + 0x030A,
    fsBTNoMoreMapNodesErr               = ERR_BASE + 0x030B,
    fsBTBadNodeSize                     = ERR_BASE + 0x030C,
    fsBTBadNodeType                     = ERR_BASE + 0x030D,
    fsBTInvalidKeyLengthErr             = ERR_BASE + 0x030E,
    fsBTStartOfIterationErr             = ERR_BASE + 0x0353,
    fsBTEndOfIterationErr               = ERR_BASE + 0x0354,
    fsBTUnknownVersionErr               = ERR_BASE + 0x0355,
    fsBTTreeTooDeepErr                  = ERR_BASE + 0x0357,
    fsIteratorExitedScopeErr            = ERR_BASE + 0x0A02,    /* iterator exited the scope*/
    fsIteratorScopeExceptionErr         = ERR_BASE + 0x0A03,    /* iterator is undefined due to error or movement of scope locality*/
    fsUnknownIteratorMovementErr        = ERR_BASE + 0x0A04,    /* iterator movement is not defined*/
    fsInvalidIterationMovmentErr        = ERR_BASE + 0x0A05,    /* iterator movement is invalid in current context*/
    fsClientIDMismatchErr               = ERR_BASE + 0x0A06,    /* wrong client process ID*/
    fsEndOfIterationErr                 = ERR_BASE + 0x0A07,    /* there were no objects left to return on iteration*/
    fsBTTimeOutErr                      = ERR_BASE + 0x0A08     /* BTree scan interrupted -- no time left for physical I/O */
};

typedef struct {
    void            *buffer;
    void            *blockHeader;
    daddr64_t       blockNum;    /* logical block number (used by hfs_swap_BTNode) */
    ByteCount       blockSize;
    Boolean         blockReadFromDisk;
    Byte            isModified;             // XXXdbg - for journaling
    Byte            reserved[2];

} BlockDescriptor, *BlockDescPtr;


typedef struct {
    void *          bufferAddress;
    ByteCount       itemSize;
    ItemCount       itemCount;

} FSBufferDescriptor, *FSBufferDescriptorPtr;


/*
 Fork Level Access Method Block get options
 */
enum {
    kGetBlock               = 0x00000000,
    kGetBlockHint           = 0x00000001,    // if set, the block is being looked up using hint
    kForceReadBlock         = 0x00000002,    // how does this relate to Read/Verify? Do we need this?
    kGetEmptyBlock          = 0x00000008
};
typedef u_int32_t    GetBlockOptions;

/*
 Fork Level Access Method Block release options
 */
enum {
    kReleaseBlock        = 0x00000000,
    kForceWriteBlock    = 0x00000001,
    kMarkBlockDirty        = 0x00000002,
    kTrashBlock            = 0x00000004,
    kLockTransaction    = 0x00000100
};
typedef u_int32_t    ReleaseBlockOptions;

typedef    u_int64_t    FSSize;
typedef    u_int32_t    ForkBlockNumber;

/*============================================================================
 Fork Level Buffered I/O Access Method
 ============================================================================*/

typedef    OSStatus    (* GetBlockProcPtr)        (FileReference                fileRefNum,
                                                   uint64_t                     blockNum,
                                                   GetBlockOptions              options,
                                                   BlockDescriptor              *block );


typedef    OSStatus    (* ReleaseBlockProcPtr)    (FileReference                fileRefNum,
                                                   BlockDescPtr                 blockPtr,
                                                   ReleaseBlockOptions          options );

typedef    OSStatus    (* SetEndOfForkProcPtr)    (FileReference                fileRefNum,
                                                   FSSize                       minEOF,
                                                   FSSize                       maxEOF );

typedef    OSStatus    (* SetBlockSizeProcPtr)    (FileReference                fileRefNum,
                                                   ByteCount                    blockSize,
                                                   ItemCount                    minBlockCount );

OSStatus SetEndOfForkProc ( FileReference fileRefNum, FSSize minEOF, FSSize maxEOF );


/*
 B*Tree Information Version
 */
enum BTreeInformationVersion{
    kBTreeInfoVersion    = 0
};

/*
 B*Tree Iteration Operation Constants
 */
enum BTreeIterationOperations{
        kBTreeFirstRecord,
        kBTreeNextRecord,
        kBTreePrevRecord,
        kBTreeLastRecord,
        kBTreeCurrentRecord
};
typedef u_int16_t BTreeIterationOperation;


/*
 Btree types: 0 is HFS CAT/EXT file, 1~127 are AppleShare B*Tree files, 128~254 unused
 hfsBtreeType       EQU        0              ; control file
 validBTType        EQU        $80            ; user btree type starts from 128
 userBT1Type        EQU        $FF            ; 255 is our Btree type. Used by BTInit and BTPatch
 */
enum BTreeTypes{
    kHFSBTreeType               =   0,        // control file
    kUserBTreeType              = 128,        // user btree type starts from 128
    kReservedBTreeType          = 255         //
};

#define    kBTreeHeaderUserBytes    128

/* B-tree structures */

enum {
    kMaxKeyLength    = 520
};

typedef union {
    u_int8_t    length8;
    u_int16_t    length16;
    u_int8_t    rawData [kMaxKeyLength+2];

} BTreeKey, *BTreeKeyPtr;

/* BTNodeDescriptor -- Every B-tree node starts with these fields. */
typedef struct {
    u_int32_t       fLink;              /* next node at this level*/
    u_int32_t       bLink;              /* previous node at this level*/
    int8_t          kind;               /* kind of node (leaf, index, header, map)*/
    u_int8_t        height;             /* zero for header, map; child is one more than parent*/
    u_int16_t       numRecords;         /* number of records in this node*/
    u_int16_t       reserved;           /* reserved - initialized as zero */

} __attribute__((aligned(2), packed)) BTNodeDescriptor;

/* Constants for BTNodeDescriptor kind */
enum {
    kBTLeafNode     = -1,
    kBTIndexNode    = 0,
    kBTHeaderNode   = 1,
    kBTMapNode      = 2
};

/* BTHeaderRec -- The first record of a B-tree header node */
typedef struct {
    u_int16_t       treeDepth;          /* maximum height (usually leaf nodes) */
    u_int32_t       rootNode;           /* node number of root node */
    u_int32_t       leafRecords;        /* number of leaf records in all leaf nodes */
    u_int32_t       firstLeafNode;      /* node number of first leaf node */
    u_int32_t       lastLeafNode;       /* node number of last leaf node */
    u_int16_t       nodeSize;           /* size of a node, in bytes */
    u_int16_t       maxKeyLength;       /* reserved */
    u_int32_t       totalNodes;         /* total number of nodes in tree */
    u_int32_t       freeNodes;          /* number of unused (free) nodes in tree */
    u_int16_t       reserved1;          /* unused */
    u_int32_t       clumpSize;          /* reserved */
    u_int8_t        btreeType;          /* reserved */
    u_int8_t        keyCompareType;     /* Key string Comparison Type */
    u_int32_t       attributes;         /* persistent attributes about the tree */
    u_int32_t       reserved3[16];      /* reserved */

} __attribute__((aligned(2), packed)) BTHeaderRec;

/* Constants for BTHeaderRec attributes */
enum {
    kBTBadCloseMask         = 0x00000001,    /* reserved */
    kBTBigKeysMask         = 0x00000002,    /* key length field is 16 bits */
    kBTVariableIndexKeysMask = 0x00000004    /* keys in index nodes are variable length */
};

/*
 BTreeInfoRec Structure - for BTGetInformation
 */
typedef struct {
    u_int16_t            version;
    u_int16_t            nodeSize;
    u_int16_t            maxKeyLength;
    u_int16_t            treeDepth;
    u_int32_t            lastfsync;        /* Last time that this was fsynced  */
    ItemCount            numRecords;
    ItemCount            numNodes;
    ItemCount            numFreeNodes;
    u_int8_t            keyCompareType;
    u_int8_t            reserved[3];
} BTreeInfoRec, *BTreeInfoRecPtr;

/*
 BTreeHint can never be exported to the outside. Use u_int32_t BTreeHint[4],
 u_int8_t BTreeHint[16], etc.
 */
typedef struct {
    ItemCount                writeCount;
    u_int32_t                nodeNum;            // node the key was last seen in
    u_int16_t                index;                // index then key was last seen at
    u_int16_t                reserved1;
    u_int32_t                reserved2;
} BTreeHint, *BTreeHintPtr;

/*
 BTree Iterator
 */
typedef struct {
    BTreeHint                hint;
    u_int16_t                version;
    u_int16_t                reserved;
    u_int32_t                hitCount;            // Total number of leaf records hit
    u_int32_t                maxLeafRecs;        // Max leaf records over iteration
    BTreeKey                key;
} BTreeIterator, *BTreeIteratorPtr;


/*============================================================================
 B*Tree SPI
 ============================================================================*/

/*
 Key Comparison Function ProcPtr Type - for BTOpenPath
 */
//typedef int32_t                 (* KeyCompareProcPtr)(BTreeKeyPtr a, BTreeKeyPtr b);


typedef int32_t (* IterateCallBackProcPtr)(BTreeKeyPtr key, void * record, void * state);

OSStatus    BTOpenPath            (FCB *filePtr, KeyCompareProcPtr keyCompareProc);

OSStatus    BTClosePath           (FCB *filePtr );


OSStatus    BTSearchRecord        (FCB                          *filePtr,
                                   BTreeIterator                *searchIterator,
                                   FSBufferDescriptor           *btRecord,
                                   u_int16_t                    *recordLen,
                                   BTreeIterator                *resultIterator );

OSStatus    BTIterateRecord       (FCB                          *filePtr,
                                   BTreeIterationOperation      operation,
                                   BTreeIterator                *iterator,
                                   FSBufferDescriptor           *btRecord,
                                   u_int16_t                    *recordLen );


OSStatus BTIterateRecords         (FCB                          *filePtr,
                                   BTreeIterationOperation      operation,
                                   BTreeIterator                *iterator,
                                   IterateCallBackProcPtr       callBackProc,
                                   void                         *callBackState);

OSStatus    BTInsertRecord        (FCB                          *filePtr,
                                   BTreeIterator                *iterator,
                                   FSBufferDescriptor           *btrecord,
                                   u_int16_t                    recordLen );

OSStatus    BTReplaceRecord       (FCB                          *filePtr,
                                   BTreeIterator                *iterator,
                                   FSBufferDescriptor           *btRecord,
                                   u_int16_t                    recordLen );

OSStatus    BTUpdateRecord        (FCB                         *filePtr,
                                   BTreeIterator               *iterator,
                                   IterateCallBackProcPtr      callBackProc,
                                   void                        *callBackState );

OSStatus    BTDeleteRecord        (FCB                         *filePtr,
                                   BTreeIterator               *iterator );

OSStatus    BTGetInformation     (FCB                          *filePtr,
                                  u_int16_t                    vers,
                                  BTreeInfoRec                 *info );

OSStatus    BTIsDirty            (FCB *filePtr);

OSStatus    BTFlushPath          (FCB *filePtr);

OSStatus    BTReloadData         (FCB *filePtr);

OSStatus    BTInvalidateHint     (BTreeIterator *iterator );

OSStatus    BTGetLastSync        (FCB                        *filePtr,
                                  u_int32_t                  *lastfsync );

OSStatus    BTSetLastSync        (FCB                         *filePtr,
                                  u_int32_t                   lastfsync );

OSStatus    BTHasContiguousNodes(FCB *filePtr);

OSStatus    BTGetUserData(FCB *filePtr, void * dataPtr, int dataSize);

OSStatus    BTSetUserData(FCB *filePtr, void * dataPtr, int dataSize);

/* B-tree node reserve routines. */
void BTReserveSetup(void);

int  BTReserveSpace(FCB *file, int operations, void * data);

int  BTReleaseReserve(FCB *file, void * data);

int  BTZeroUnusedNodes(FCB *file);


#endif /* lf_hfs_btrees_internal_h */
