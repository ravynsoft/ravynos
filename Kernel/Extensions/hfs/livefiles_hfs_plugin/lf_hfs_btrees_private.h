//
//  lf_hfs_btrees_private.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_btrees_private_h
#define lf_hfs_btrees_private_h

#include "lf_hfs_defs.h"
#include "lf_hfs_file_mgr_internal.h"
#include "lf_hfs_btrees_internal.h"
#include "lf_hfs_logger.h"

/////////////////////////////////// Constants ///////////////////////////////////

#define        kBTreeVersion          1
#define        kMaxTreeDepth         16


#define        kHeaderNodeNum          0
#define        kKeyDescRecord          1


// Header Node Record Offsets
enum {
    kHeaderRecOffset        =    0x000E,
    kKeyDescRecOffset       =    0x0078,
    kHeaderMapRecOffset     =    0x00F8
};

#define        kMinNodeSize        (512)

#define        kMinRecordSize      (6)
// where is minimum record size enforced?

// miscellaneous BTree constants
enum {
    kOffsetSize                = 2
};

// Insert Operations
typedef enum {
    kInsertRecord               = 0,
    kReplaceRecord              = 1
} InsertType;

// illegal string attribute bits set in mask
#define        kBadStrAttribMask        (0xCF)



//////////////////////////////////// Macros /////////////////////////////////////

#define        M_NodesInMap(mapSize)                ((mapSize) << 3)

#define        M_ClearBitNum(integer,bitNumber)     ((integer) &= (~(1<<(bitNumber))))
#define        M_SetBitNum(integer,bitNumber)       ((integer) |= (1<<(bitNumber)))
#define        M_IsOdd(integer)                     (((integer) & 1) != 0)
#define        M_IsEven(integer)                    (((integer) & 1) == 0)

#define        M_MapRecordSize(nodeSize)            (nodeSize - sizeof (BTNodeDescriptor) - 6)
#define        M_HeaderMapRecordSize(nodeSize)      (nodeSize - sizeof(BTNodeDescriptor) - sizeof(BTHeaderRec) - 128 - 8)

#define        M_SWAP_BE16_ClearBitNum(integer,bitNumber)  ((integer) &= SWAP_BE16(~(1<<(bitNumber))))
#define        M_SWAP_BE16_SetBitNum(integer,bitNumber)    ((integer) |= SWAP_BE16(1<<(bitNumber)))

///////////////////////////////////// Types /////////////////////////////////////

typedef struct {                    // fields specific to BTree CBs

    u_int8_t                        keyCompareType;        /* Key string Comparison Type */
    u_int8_t                        btreeType;
    u_int16_t                       treeDepth;
    FileReference                   fileRefNum;            // refNum of btree file
    KeyCompareProcPtr               keyCompareProc;
    u_int32_t                       rootNode;
    u_int32_t                       leafRecords;
    u_int32_t                       firstLeafNode;
    u_int32_t                       lastLeafNode;
    u_int16_t                       nodeSize;
    u_int16_t                       maxKeyLength;
    u_int32_t                       totalNodes;
    u_int32_t                       freeNodes;

    u_int16_t                       reserved3;              // 4-byte alignment

    // new fields
    int16_t                         version;
    u_int32_t                       flags;                  // dynamic flags
    u_int32_t                       attributes;             // persistent flags
    u_int32_t                       writeCount;
    u_int32_t                       lastfsync;              /* Last time that this was fsynced  */

    GetBlockProcPtr                 getBlockProc;
    ReleaseBlockProcPtr             releaseBlockProc;
    SetEndOfForkProcPtr             setEndOfForkProc;

    // statistical information
    u_int32_t                       numGetNodes;
    u_int32_t                       numGetNewNodes;
    u_int32_t                       numReleaseNodes;
    u_int32_t                       numUpdateNodes;
    u_int32_t                       numMapNodesRead;        // map nodes beyond header node
    u_int32_t                       numHintChecks;
    u_int32_t                       numPossibleHints;       // Looks like a formated hint
    u_int32_t                       numValidHints;          // Hint used to find correct record.
    u_int32_t                       reservedNodes;
    BTreeIterator                   iterator;               // useable when holding exclusive b-tree lock

#if DEBUG
    void                        *madeDirtyBy[2];
#endif

} BTreeControlBlock, *BTreeControlBlockPtr;

u_int32_t CalcKeySize(const BTreeControlBlock *btcb, const BTreeKey *key);
#define CalcKeySize(btcb, key)            ( ((btcb)->attributes & kBTBigKeysMask) ? ((key)->length16 + 2) : ((key)->length8 + 1) )

u_int32_t KeyLength(const BTreeControlBlock *btcb, const BTreeKey *key);
#define KeyLength(btcb, key)            ( ((btcb)->attributes & kBTBigKeysMask) ? (key)->length16 : (key)->length8 )


typedef enum {
    kBTHeaderDirty    = 0x00000001
} BTreeFlags;

static inline void M_BTreeHeaderDirty(BTreeControlBlock *bt) {
#if DEBUG
    bt->madeDirtyBy[0] = __builtin_return_address(0);
    bt->madeDirtyBy[1] = __builtin_return_address(1);
#endif
    bt->flags |= kBTHeaderDirty;
}

typedef int8_t              *NodeBuffer;
typedef BlockDescriptor     NodeRec, *NodePtr;        //remove this someday...


//// Tree Path Table - constructed by SearchTree, used by InsertTree and DeleteTree

typedef struct {
    u_int32_t                node;                // node number
    u_int16_t                index;
    u_int16_t                reserved;            // align size to a power of 2

} TreePathRecord, *TreePathRecordPtr;

typedef TreePathRecord TreePathTable [kMaxTreeDepth];


//// InsertKey - used by InsertTree, InsertLevel and InsertNode

typedef struct {
    BTreeKeyPtr         keyPtr;
    u_int8_t *          recPtr;
    u_int16_t           keyLength;
    u_int16_t           recSize;
    Boolean             replacingKey;
    Boolean             skipRotate;
} InsertKey;

//// For Notational Convenience

typedef     BTNodeDescriptor*       NodeDescPtr;
typedef     u_int8_t                *RecordPtr;
typedef     BTreeKeyPtr             KeyPtr;


//////////////////////////////////// Globals ////////////////////////////////////


//////////////////////////////////// Macros /////////////////////////////////////
//    Exit function on error
#define M_ExitOnError( result )    do { if ( ( result ) != noErr )    goto ErrorExit; } while(0)

//    Test for passed condition and return if true
#define    M_ReturnErrorIf( condition, error )    do { if ( condition )    return( error ); } while(0)

//////////////////////////////// Key Operations /////////////////////////////////

int32_t        CompareKeys                (BTreeControlBlockPtr         btreePtr,
                                           KeyPtr                       searchKey,
                                           KeyPtr                       trialKey );

//////////////////////////////// Map Operations /////////////////////////////////

OSStatus    AllocateNode            (BTreeControlBlockPtr     btreePtr,
                                     u_int32_t                *nodeNum);

OSStatus    FreeNode                (BTreeControlBlockPtr     btreePtr,
                                     u_int32_t                nodeNum);

OSStatus    ExtendBTree             (BTreeControlBlockPtr     btreePtr,
                                     u_int32_t                nodes );

u_int32_t    CalcMapBits            (BTreeControlBlockPtr     btreePtr);


void         BTUpdateReserve        (BTreeControlBlockPtr   btreePtr,
                                     int                    nodes);

//////////////////////////////// Misc Operations ////////////////////////////////

u_int16_t    CalcKeyRecordSize        (u_int16_t                keySize,
                                       u_int16_t                recSize );

OSStatus    VerifyHeader            (FCB                        *filePtr,
                                     BTHeaderRec                *header );

OSStatus    UpdateHeader            (BTreeControlBlockPtr       btreePtr,
                                     Boolean                    forceWrite );

OSStatus    FindIteratorPosition    (BTreeControlBlockPtr       btreePtr,
                                     BTreeIteratorPtr           iterator,
                                     BlockDescriptor            *left,
                                     BlockDescriptor            *middle,
                                     BlockDescriptor            *right,
                                     u_int32_t                  *nodeNum,
                                     u_int16_t                  *index,
                                     Boolean                    *foundRecord );

OSStatus    CheckInsertParams        (FCB                       *filePtr,
                                      BTreeIterator             *iterator,
                                      FSBufferDescriptor        *record,
                                      u_int16_t                 recordLen );

OSStatus    TrySimpleReplace        (BTreeControlBlockPtr       btreePtr,
                                     NodeDescPtr                nodePtr,
                                     BTreeIterator              *iterator,
                                     FSBufferDescriptor         *record,
                                     u_int16_t                  recordLen,
                                     Boolean                    *recordInserted );

OSStatus    IsItAHint                (BTreeControlBlockPtr      btreePtr,
                                      BTreeIterator             *iterator,
                                      Boolean                   *answer );

OSStatus    TreeIsDirty             (BTreeControlBlockPtr       btreePtr);

//////////////////////////////// Node Operations ////////////////////////////////

//// Node Operations

OSStatus    GetNode                    (BTreeControlBlockPtr    btreePtr,
                                        u_int32_t               nodeNum,
                                        u_int32_t               flags,
                                        NodeRec                 *returnNodePtr );

/* Flags for GetNode() */
#define        kGetNodeHint    0x1        /* If set, the node is being looked up using a hint */

OSStatus    GetLeftSiblingNode        (BTreeControlBlockPtr     btreePtr,
                                       NodeDescPtr              node,
                                       NodeRec                  *left );

#define        GetLeftSiblingNode(btree,node,left)            GetNode ((btree), ((NodeDescPtr)(node))->bLink, 0, (left))

OSStatus    GetRightSiblingNode        (BTreeControlBlockPtr        btreePtr,
                                        NodeDescPtr                 node,
                                        NodeRec                     *right );

#define        GetRightSiblingNode(btree,node,right)        GetNode ((btree), ((NodeDescPtr)(node))->fLink, 0, (right))


OSStatus    GetNewNode                (BTreeControlBlockPtr   btreePtr,
                                       u_int32_t              nodeNum,
                                       NodeRec                *returnNodePtr );

OSStatus    ReleaseNode                (BTreeControlBlockPtr        btreePtr,
                                        NodePtr                     nodePtr );

OSStatus    TrashNode                (BTreeControlBlockPtr      btreePtr,
                                      NodePtr                   nodePtr );

OSStatus    UpdateNode                (BTreeControlBlockPtr     btreePtr,
                                       NodePtr                  nodePtr,
                                       u_int32_t                transactionID,
                                       u_int32_t                flags );

//// Node Buffer Operations

void        ClearNode                (BTreeControlBlockPtr      btreePtr,
                                      NodeDescPtr               node );

u_int16_t    GetNodeDataSize            (BTreeControlBlockPtr       btreePtr,
                                         NodeDescPtr                node );

u_int16_t    GetNodeFreeSize            (BTreeControlBlockPtr       btreePtr,
                                         NodeDescPtr                node );


//// Record Operations

Boolean        InsertRecord            (BTreeControlBlockPtr     btreePtr,
                                        NodeDescPtr              node,
                                        u_int16_t                index,
                                        RecordPtr                recPtr,
                                        u_int16_t                recSize );

Boolean        InsertKeyRecord            (BTreeControlBlockPtr     btreePtr,
                                           NodeDescPtr              node,
                                           u_int16_t                index,
                                           KeyPtr                   keyPtr,
                                           u_int16_t                keyLength,
                                           RecordPtr                recPtr,
                                           u_int16_t                recSize );

void        DeleteRecord            (BTreeControlBlockPtr    btree,
                                     NodeDescPtr             node,
                                     u_int16_t               index );


Boolean        SearchNode           (BTreeControlBlockPtr       btree,
                                     NodeDescPtr                node,
                                     KeyPtr                     searchKey,
                                     u_int16_t                  *index );

OSStatus    GetRecordByIndex        (BTreeControlBlockPtr       btree,
                                     NodeDescPtr                node,
                                     u_int16_t                  index,
                                     KeyPtr                     *keyPtr,
                                     u_int8_t *                 *dataPtr,
                                     u_int16_t                  *dataSize );

u_int8_t *    GetRecordAddress        (BTreeControlBlockPtr     btree,
                                       NodeDescPtr              node,
                                       u_int16_t                index );

#define GetRecordAddress(btreePtr,node,index)        ((u_int8_t *)(node) + (*(short *) ((u_int8_t *)(node) + (btreePtr)->nodeSize - ((index) << 1) - kOffsetSize)))


u_int16_t    GetRecordSize            (BTreeControlBlockPtr     btree,
                                       NodeDescPtr              node,
                                       u_int16_t                index );

u_int32_t    GetChildNodeNum            (BTreeControlBlockPtr       btreePtr,
                                         NodeDescPtr                nodePtr,
                                         u_int16_t                  index );

void        MoveRecordsLeft            (u_int8_t *                  src,
                                        u_int8_t *                  dst,
                                        u_int16_t                   bytesToMove );

#define        MoveRecordsLeft(src,dst,bytes)            bcopy((src),(dst),(bytes))

void        MoveRecordsRight        (u_int8_t *                 src,
                                     u_int8_t *                 dst,
                                     u_int16_t                  bytesToMove );

#define        MoveRecordsRight(src,dst,bytes)            bcopy((src),(dst),(bytes))


//////////////////////////////// Tree Operations ////////////////////////////////

OSStatus    SearchTree                (BTreeControlBlockPtr     btreePtr,
                                       BTreeKeyPtr              keyPtr,
                                       TreePathTable            treePathTable,
                                       u_int32_t                *nodeNum,
                                       BlockDescriptor          *nodePtr,
                                       u_int16_t                *index );

OSStatus    InsertTree                (BTreeControlBlockPtr     btreePtr,
                                       TreePathTable            treePathTable,
                                       KeyPtr                   keyPtr,
                                       u_int8_t *               recPtr,
                                       u_int16_t                recSize,
                                       BlockDescriptor          *targetNode,
                                       u_int16_t                index,
                                       u_int16_t                level,
                                       Boolean                  replacingKey,
                                       u_int32_t                *insertNode );

OSStatus    DeleteTree                (BTreeControlBlockPtr     btreePtr,
                                       TreePathTable            treePathTable,
                                       BlockDescriptor          *targetNode,
                                       u_int16_t                index,
                                       u_int16_t                level );


#endif /* lf_hfs_btrees_private_h */
