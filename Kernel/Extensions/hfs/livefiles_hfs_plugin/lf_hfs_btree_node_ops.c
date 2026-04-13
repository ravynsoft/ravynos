//
//  lf_hfs_btree_node_ops.c
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#include "lf_hfs_btrees_private.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_generic_buf.h"

///////////////////////// BTree Module Node Operations //////////////////////////
//
//    GetNode             - Call FS Agent to get node
//    GetNewNode            - Call FS Agent to get a new node
//    ReleaseNode            - Call FS Agent to release node obtained by GetNode.
//    UpdateNode            - Mark a node as dirty and call FS Agent to release it.
//
//    ClearNode            - Clear a node to all zeroes.
//
//    InsertRecord        - Inserts a record into a BTree node.
//    InsertKeyRecord        - Inserts a key and record pair into a BTree node.
//    DeleteRecord        - Deletes a record from a BTree node.
//
//    SearchNode            - Return index for record that matches key.
//    LocateRecord        - Return pointer to key and data, and size of data.
//
//    GetNodeDataSize        - Return the amount of space used for data in the node.
//    GetNodeFreeSize        - Return the amount of free space in the node.
//
//    GetRecordOffset        - Return the offset for record "index".
//    GetRecordAddress    - Return address of record "index".
//    GetOffsetAddress    - Return address of offset for record "index".
//
//    InsertOffset        - Inserts a new offset into a node.
//    DeleteOffset        - Deletes an offset from a node.
//
/////////////////////////////////////////////////////////////////////////////////



////////////////////// Routines Internal To BTreeNodeOps.c //////////////////////

u_int16_t    GetRecordOffset        (BTreeControlBlockPtr     btree,
                                     NodeDescPtr             node,
                                     u_int16_t                 index );

u_int16_t    *GetOffsetAddress    (BTreeControlBlockPtr    btreePtr,
                                   NodeDescPtr             node,
                                   u_int16_t                index );

void        InsertOffset        (BTreeControlBlockPtr     btreePtr,
                                 NodeDescPtr             node,
                                 u_int16_t                 index,
                                 u_int16_t                 delta );

void        DeleteOffset        (BTreeControlBlockPtr     btreePtr,
                                 NodeDescPtr             node,
                                 u_int16_t                 index );


/////////////////////////////////////////////////////////////////////////////////

#define GetRecordOffset(btreePtr,node,index)        (*(short *) ((u_int8_t *)(node) + (btreePtr)->nodeSize - ((index) << 1) - kOffsetSize))


/*-------------------------------------------------------------------------------

 Routine:    GetNode    -    Call FS Agent to get node

 Function:    Gets an existing BTree node from FS Agent and verifies it.

 Input:        btreePtr    - pointer to BTree control block
 nodeNum        - number of node to request

 Output:        nodePtr        - pointer to beginning of node (nil if error)

 Result:
 noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus    GetNode        (BTreeControlBlockPtr     btreePtr,
                            u_int32_t                 nodeNum,
                            u_int32_t                 flags,
                            NodeRec                *nodePtr )
{
    OSStatus            err;
    GetBlockProcPtr        getNodeProc;
    u_int32_t            options;


    // is nodeNum within proper range?
    if( nodeNum >= btreePtr->totalNodes )
    {
        LFHFS_LOG(LEVEL_ERROR, "GetNode:nodeNum [%u] >= totalNodes [%u]",nodeNum, btreePtr->totalNodes);
        err = fsBTInvalidNodeErr;
        goto ErrorExit;
    }

    nodePtr->blockSize = btreePtr->nodeSize;    // indicate the size of a node

    options = kGetBlock;
    if ( flags & kGetNodeHint )
    {
        options |= kGetBlockHint;
    }

    getNodeProc = btreePtr->getBlockProc;
    err = getNodeProc (btreePtr->fileRefNum,
                       nodeNum,
                       options,
                       nodePtr );

    if (err != noErr)
    {
        LFHFS_LOG(LEVEL_ERROR, "GetNode: getNodeProc returned error.");
        goto ErrorExit;
    }
    ++btreePtr->numGetNodes;

    return noErr;

ErrorExit:
    nodePtr->buffer         = nil;
    nodePtr->blockHeader    = nil;

    return    err;
}



/*-------------------------------------------------------------------------------

 Routine:    GetNewNode    -    Call FS Agent to get a new node

 Function:    Gets a new BTree node from FS Agent and initializes it to an empty
 state.

 Input:        btreePtr        - pointer to BTree control block
 nodeNum            - number of node to request

 Output:        returnNodePtr    - pointer to beginning of node (nil if error)

 Result:        noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus    GetNewNode    (BTreeControlBlockPtr     btreePtr,
                           u_int32_t                 nodeNum,
                           NodeRec                *returnNodePtr )
{
    OSStatus             err;
    NodeDescPtr             node;
    void                *pos;
    GetBlockProcPtr         getNodeProc;


    //////////////////////// get buffer for new node ////////////////////////////

    returnNodePtr->blockSize = btreePtr->nodeSize;    // indicate the size of a node

    getNodeProc = btreePtr->getBlockProc;
    err = getNodeProc (btreePtr->fileRefNum,
                       nodeNum,
                       kGetBlock+kGetEmptyBlock,
                       returnNodePtr );

    if (err != noErr)
    {
        LFHFS_LOG(LEVEL_ERROR, "GetNewNode: getNodeProc returned error.");
        //    returnNodePtr->buffer = nil;
        return err;
    }
    ++btreePtr->numGetNewNodes;


    ////////////////////////// initialize the node //////////////////////////////

    node = returnNodePtr->buffer;

    GenericLFBuf *psBuf = returnNodePtr->blockHeader;
    ClearNode (btreePtr, node);                     // clear the node
    lf_hfs_generic_buf_set_cache_flag(psBuf, GEN_BUF_LITTLE_ENDIAN);

    pos = (char *)node + btreePtr->nodeSize - 2;    // find address of last offset
    *(u_int16_t *)pos = sizeof (BTNodeDescriptor);    // set offset to beginning of free space


    return noErr;
}



/*-------------------------------------------------------------------------------

 Routine:    ReleaseNode    -    Call FS Agent to release node obtained by GetNode.

 Function:    Informs the FS Agent that a BTree node may be released.

 Input:        btreePtr        - pointer to BTree control block
 nodeNum            - number of node to release

 Result:        noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus    ReleaseNode    (BTreeControlBlockPtr     btreePtr,
                            NodePtr                 nodePtr )
{
    OSStatus             err;
    ReleaseBlockProcPtr     releaseNodeProc;


    err = noErr;

    if (nodePtr->buffer != nil)
    {
        releaseNodeProc = btreePtr->releaseBlockProc;
        err = releaseNodeProc (btreePtr->fileRefNum,
                               nodePtr,
                               kReleaseBlock );
        if (err)
        {
            LFHFS_LOG(LEVEL_ERROR, "ReleaseNode: releaseNodeProc returned error.");
            hfs_assert(0);
        }
        ++btreePtr->numReleaseNodes;
    }

    nodePtr->buffer            = nil;
    nodePtr->blockHeader    = nil;

    return err;
}




/*-------------------------------------------------------------------------------

 Routine:    TrashNode    -    Call FS Agent to release node obtained by GetNode, and
 not store it...mark it as bad.

 Function:    Informs the FS Agent that a BTree node may be released and thrown away.

 Input:        btreePtr        - pointer to BTree control block
 nodeNum            - number of node to release

 Result:        noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus    TrashNode    (BTreeControlBlockPtr     btreePtr,
                          NodePtr                 nodePtr )
{
    OSStatus             err;
    ReleaseBlockProcPtr     releaseNodeProc;


    err = noErr;

    if (nodePtr->buffer != nil)
    {
        releaseNodeProc = btreePtr->releaseBlockProc;
        err = releaseNodeProc (btreePtr->fileRefNum,
                               nodePtr,
                               kReleaseBlock | kTrashBlock );
        if (err)
        {
            LFHFS_LOG(LEVEL_ERROR, "TrashNode: releaseNodeProc returned error.");
            hfs_assert(0);
        }
        ++btreePtr->numReleaseNodes;
    }

    nodePtr->buffer            = nil;
    nodePtr->blockHeader    = nil;

    return err;
}



/*-------------------------------------------------------------------------------

 Routine:    UpdateNode    -    Mark a node as dirty and call FS Agent to release it.

 Function:    Marks a BTree node dirty and informs the FS Agent that it may be released.

 Input:        btreePtr        - pointer to BTree control block
 nodeNum            - number of node to release
 transactionID    - ID of transaction this node update is a part of
 flags            - special flags to pass to ReleaseNodeProc

 Result:        noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus    UpdateNode    (BTreeControlBlockPtr     btreePtr,
                           NodePtr                 nodePtr,
                           u_int32_t                 transactionID,
                           u_int32_t                 flags )
{
#pragma unused(transactionID)

    OSStatus             err;
    ReleaseBlockProcPtr     releaseNodeProc;


    err = noErr;

    if (nodePtr->buffer != nil)            // Why call UpdateNode if nil ?!?
    {
        releaseNodeProc = btreePtr->releaseBlockProc;
        err = releaseNodeProc (btreePtr->fileRefNum,
                               nodePtr,
                               flags | kMarkBlockDirty );
        ++btreePtr->numUpdateNodes;
    }

    nodePtr->buffer         = nil;
    nodePtr->blockHeader    = nil;

    return    err;
}

/*-------------------------------------------------------------------------------

 Routine:    ClearNode    -    Clear a node to all zeroes.

 Function:    Writes zeroes from beginning of node for nodeSize bytes.

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node to clear

 Result:        none
 -------------------------------------------------------------------------------*/

void    ClearNode    (BTreeControlBlockPtr    btreePtr, NodeDescPtr     node )
{
    ClearMemory( node, btreePtr->nodeSize );
}

/*-------------------------------------------------------------------------------

 Routine:    InsertRecord    -    Inserts a record into a BTree node.

 Function:

 Note:        Record size must be even!

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node to insert the record
 index            - position record is to be inserted
 recPtr            - pointer to record to insert

 Result:        noErr        - success
 fsBTFullErr    - record larger than remaining free space.
 -------------------------------------------------------------------------------*/

Boolean        InsertRecord    (BTreeControlBlockPtr    btreePtr,
                                NodeDescPtr             node,
                                u_int16_t                 index,
                                RecordPtr                recPtr,
                                u_int16_t                recSize )
{
    u_int16_t    freeSpace;
    u_int16_t    indexOffset;
    u_int16_t    freeOffset;
    u_int16_t    bytesToMove;
    void       *src;
    void       *dst;

    //// will new record fit in node?

    freeSpace = GetNodeFreeSize (btreePtr, node);
    //we could get freeOffset & calc freeSpace
    if ( freeSpace < recSize + 2)
    {
        return false;
    }


    //// make hole for new record

    indexOffset = GetRecordOffset (btreePtr, node, index);
    freeOffset    = GetRecordOffset (btreePtr, node, node->numRecords);

    src = ((Ptr) node) + indexOffset;
    dst = ((Ptr) src)  + recSize;
    bytesToMove = freeOffset - indexOffset;
    if (bytesToMove)
        MoveRecordsRight (src, dst, bytesToMove);


    //// adjust offsets for moved records

    InsertOffset (btreePtr, node, index, recSize);


    //// move in the new record

    dst = ((Ptr) node) + indexOffset;
    MoveRecordsLeft (recPtr, dst, recSize);

    return true;
}



/*-------------------------------------------------------------------------------

 Routine:    InsertKeyRecord    -    Inserts a record into a BTree node.

 Function:

 Note:        Record size must be even!

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node to insert the record
 index            - position record is to be inserted
 keyPtr            - pointer to key for record to insert
 keyLength        - length of key (or maxKeyLength)
 recPtr            - pointer to record to insert
 recSize            - number of bytes to copy for record

 Result:        noErr        - success
 fsBTFullErr    - record larger than remaining free space.
 -------------------------------------------------------------------------------*/

Boolean        InsertKeyRecord        (BTreeControlBlockPtr     btreePtr,
                                       NodeDescPtr              node,
                                       u_int16_t                  index,
                                       KeyPtr                     keyPtr,
                                       u_int16_t                 keyLength,
                                       RecordPtr                 recPtr,
                                       u_int16_t                 recSize )
{
    u_int16_t        freeSpace;
    u_int16_t        indexOffset;
    u_int16_t        freeOffset;
    u_int16_t        bytesToMove;
    u_int8_t *        src;
    u_int8_t *        dst;
    u_int16_t        keySize;
    u_int16_t        rawKeyLength;
    u_int16_t        sizeOfLength;

    //// calculate actual key size

    if ( btreePtr->attributes & kBTBigKeysMask )
        keySize = keyLength + sizeof(u_int16_t);
    else
        keySize = keyLength + sizeof(u_int8_t);

    if ( M_IsOdd (keySize) )
        ++keySize;            // add pad byte


    //// will new record fit in node?

    freeSpace = GetNodeFreeSize (btreePtr, node);
    //we could get freeOffset & calc freeSpace
    if ( freeSpace < keySize + recSize + 2)
    {
        return false;
    }


    //// make hole for new record

    indexOffset = GetRecordOffset (btreePtr, node, index);
    freeOffset    = GetRecordOffset (btreePtr, node, node->numRecords);

    src = ((u_int8_t *) node) + indexOffset;
    dst = ((u_int8_t *) src) + keySize + recSize;
    bytesToMove = freeOffset - indexOffset;
    if (bytesToMove)
        MoveRecordsRight (src, dst, bytesToMove);


    //// adjust offsets for moved records

    InsertOffset (btreePtr, node, index, keySize + recSize);


    //// copy record key

    dst = ((u_int8_t *) node) + indexOffset;

    if ( btreePtr->attributes & kBTBigKeysMask )
    {
        *((u_int16_t *)dst) = keyLength;            // use keyLength rather than key.length
        dst = (u_int8_t *) (((u_int16_t *)dst) + 1);
        rawKeyLength = keyPtr->length16;
        sizeOfLength = 2;
    }
    else
    {
        *dst++ = keyLength;                    // use keyLength rather than key.length
        rawKeyLength = keyPtr->length8;
        sizeOfLength = 1;
    }

    MoveRecordsLeft ( ((u_int8_t *) keyPtr) + sizeOfLength, dst, rawKeyLength);    // copy key

    // any pad bytes?
    bytesToMove = keySize - rawKeyLength;
    if (bytesToMove) {
        ClearMemory (dst + rawKeyLength, bytesToMove);    // clear pad bytes in index key
    }


    //// copy record data

    dst = ((u_int8_t *) node) + indexOffset + keySize;
    MoveRecordsLeft (recPtr, dst, recSize);

    return true;
}



/*-------------------------------------------------------------------------------

 Routine:    DeleteRecord    -    Deletes a record from a BTree node.

 Function:

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node to insert the record
 index            - position record is to be inserted

 Result:        none
 -------------------------------------------------------------------------------*/

void        DeleteRecord    (BTreeControlBlockPtr    btreePtr,
                             NodeDescPtr             node,
                             u_int16_t                 index )
{
    int16_t        indexOffset;
    int16_t        nextOffset;
    int16_t        freeOffset;
    int16_t        bytesToMove;
    void       *src;
    void       *dst;

    //// compress records
    indexOffset = GetRecordOffset (btreePtr, node, index);
    nextOffset    = GetRecordOffset (btreePtr, node, index + 1);
    freeOffset    = GetRecordOffset (btreePtr, node, node->numRecords);

    src = ((Ptr) node) + nextOffset;
    dst = ((Ptr) node) + indexOffset;
    bytesToMove = freeOffset - nextOffset;
    if (bytesToMove)
        MoveRecordsLeft (src, dst, bytesToMove);

    //// Adjust the offsets
    DeleteOffset (btreePtr, node, index);

    /* clear out new free space */
    bytesToMove = nextOffset - indexOffset;
    ClearMemory(GetRecordAddress(btreePtr, node, node->numRecords), bytesToMove);

}



/*-------------------------------------------------------------------------------

 Routine:    SearchNode    -    Return index for record that matches key.

 Function:    Returns the record index for the record that matches the search key.
 If no record was found that matches the search key, the "insert index"
 of where the record should go is returned instead.

 Algorithm:    A binary search algorithm is used to find the specified key.

 Input:        btreePtr    - pointer to BTree control block
 node        - pointer to node that contains the record
 searchKey    - pointer to the key to match

 Output:        index        - pointer to beginning of key for record

 Result:        true    - success (index = record index)
 false    - key did not match anything in node (index = insert index)
 -------------------------------------------------------------------------------*/
Boolean
SearchNode( BTreeControlBlockPtr btreePtr,
           NodeDescPtr node,
           KeyPtr searchKey,
           u_int16_t *returnIndex )
{
    int32_t        lowerBound;
    int32_t        upperBound;
    int32_t        index;
    int32_t        result;
    KeyPtr        trialKey;
    u_int16_t    *offset;
    KeyCompareProcPtr compareProc = btreePtr->keyCompareProc;

    lowerBound = 0;
    upperBound = node->numRecords - 1;
    offset = (u_int16_t *) ((u_int8_t *)(node) + (btreePtr)->nodeSize - kOffsetSize);

    while (lowerBound <= upperBound) {
        index = (lowerBound + upperBound) >> 1;

        trialKey = (KeyPtr) ((u_int8_t *)node + *(offset - index));

        result = compareProc(searchKey, trialKey);

        if (result <  0) {
            upperBound = index - 1;      /* search < trial */
        } else if (result >  0) {
            lowerBound = index + 1;      /* search > trial */
        } else {
            *returnIndex = index;      /* search == trial */
            return true;
        }
    }

    *returnIndex = lowerBound;    /* lowerBound is insert index */
    return false;
}


/*-------------------------------------------------------------------------------

 Routine:    GetRecordByIndex    -    Return pointer to key and data, and size of data.

 Function:    Returns a pointer to beginning of key for record, a pointer to the
 beginning of the data for the record, and the size of the record data
 (does not include the size of the key).

 Input:        btreePtr    - pointer to BTree control block
 node        - pointer to node that contains the record
 index        - index of record to get

 Output:        keyPtr        - pointer to beginning of key for record
 dataPtr        - pointer to beginning of data for record
 dataSize    - size of the data portion of the record

 Result:        none
 -------------------------------------------------------------------------------*/

OSStatus    GetRecordByIndex    (BTreeControlBlockPtr     btreePtr,
                                 NodeDescPtr             node,
                                 u_int16_t                 index,
                                 KeyPtr                    *keyPtr,
                                 u_int8_t *                *dataPtr,
                                 u_int16_t                *dataSize )
{
    u_int16_t        offset;
    u_int16_t        nextOffset;
    u_int16_t        keySize;

    //
    //    Make sure index is valid (in range 0..numRecords-1)
    //
    if (index >= node->numRecords)
        return fsBTRecordNotFoundErr;

    //// find keyPtr
    offset        = GetRecordOffset (btreePtr, node, index);
    *keyPtr        = (KeyPtr) ((Ptr)node + offset);

    //// find dataPtr
    keySize    = CalcKeySize(btreePtr, *keyPtr);
    if ( M_IsOdd (keySize) )
        ++keySize;    // add pad byte

    offset += keySize;            // add the key length to find data offset
    *dataPtr = (u_int8_t *) node + offset;

    //// find dataSize
    nextOffset    = GetRecordOffset (btreePtr, node, index + 1);
    *dataSize    = nextOffset - offset;

    return noErr;
}



/*-------------------------------------------------------------------------------

 Routine:    GetNodeDataSize    -    Return the amount of space used for data in the node.

 Function:    Gets the size of the data currently contained in a node, excluding
 the node header. (record data + offset overhead)

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node that contains the record

 Result:        - number of bytes used for data and offsets in the node.
 -------------------------------------------------------------------------------*/

u_int16_t    GetNodeDataSize    (BTreeControlBlockPtr    btreePtr, NodeDescPtr     node )
{
    u_int16_t freeOffset;

    freeOffset = GetRecordOffset (btreePtr, node, node->numRecords);

    return    freeOffset + (node->numRecords << 1) - sizeof (BTNodeDescriptor);
}



/*-------------------------------------------------------------------------------

 Routine:    GetNodeFreeSize    -    Return the amount of free space in the node.

 Function:

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node that contains the record

 Result:        - number of bytes of free space in the node.
 -------------------------------------------------------------------------------*/

u_int16_t        GetNodeFreeSize    (BTreeControlBlockPtr    btreePtr, NodeDescPtr     node )
{
    u_int16_t    freeOffset;

    freeOffset = GetRecordOffset (btreePtr, node, node->numRecords);    //inline?

    return btreePtr->nodeSize - freeOffset - (node->numRecords << 1) - kOffsetSize;
}



/*-------------------------------------------------------------------------------

 Routine:    GetRecordOffset    -    Return the offset for record "index".

 Function:

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node that contains the record
 index            - record to obtain offset for

 Result:        - offset (in bytes) from beginning of node of record specified by index
 -------------------------------------------------------------------------------*/
// make this a macro (for inlining)
#if 0
u_int16_t    GetRecordOffset    (BTreeControlBlockPtr    btreePtr,
                                 NodeDescPtr            node,
                                 u_int16_t                index )
{
    void    *pos;


    pos = (u_int8_t *)node + btreePtr->nodeSize - (index << 1) - kOffsetSize;

    return *(short *)pos;
}
#endif



/*-------------------------------------------------------------------------------

 Routine:    GetRecordAddress    -    Return address of record "index".

 Function:

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node that contains the record
 index            - record to obtain offset address for

 Result:        - pointer to record "index".
 -------------------------------------------------------------------------------*/
// make this a macro (for inlining)
#if 0
u_int8_t *    GetRecordAddress    (BTreeControlBlockPtr    btreePtr,
                                   NodeDescPtr            node,
                                   u_int16_t                index )
{
    u_int8_t *    pos;

    pos = (u_int8_t *)node + GetRecordOffset (btreePtr, node, index);

    return pos;
}
#endif



/*-------------------------------------------------------------------------------

 Routine:    GetRecordSize    -    Return size of record "index".

 Function:

 Note:        This does not work on the FreeSpace index!

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node that contains the record
 index            - record to obtain record size for

 Result:        - size of record "index".
 -------------------------------------------------------------------------------*/

u_int16_t    GetRecordSize        (BTreeControlBlockPtr    btreePtr,
                                   NodeDescPtr            node,
                                   u_int16_t                index )
{
    u_int16_t    *pos;

    pos = (u_int16_t *) ((Ptr)node + btreePtr->nodeSize - (index << 1) - kOffsetSize);

    return  *(pos-1) - *pos;
}



/*-------------------------------------------------------------------------------
 Routine:    GetOffsetAddress    -    Return address of offset for record "index".

 Function:

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node that contains the record
 index            - record to obtain offset address for

 Result:        - pointer to offset for record "index".
 -------------------------------------------------------------------------------*/

u_int16_t     *GetOffsetAddress    (BTreeControlBlockPtr    btreePtr,
                                    NodeDescPtr            node,
                                    u_int16_t                index )
{
    void    *pos;

    pos = (Ptr)node + btreePtr->nodeSize - (index << 1) -2;

    return (u_int16_t *)pos;
}



/*-------------------------------------------------------------------------------
 Routine:    GetChildNodeNum    -    Return child node number from index record "index".

 Function:    Returns the first u_int32_t stored after the key for record "index".

 Assumes:    The node is an Index Node.
 The key.length stored at record "index" is ODD. //change for variable length index keys

 Input:        btreePtr        - pointer to BTree control block
 node            - pointer to node that contains the record
 index            - record to obtain child node number from

 Result:        - child node number from record "index".
 -------------------------------------------------------------------------------*/

u_int32_t    GetChildNodeNum            (BTreeControlBlockPtr     btreePtr,
                                         NodeDescPtr             nodePtr,
                                         u_int16_t                 index )
{
    u_int8_t *        pos;

    pos = GetRecordAddress (btreePtr, nodePtr, index);
    pos += CalcKeySize(btreePtr, (BTreeKey *) pos);        // key.length + size of length field

    return    *(u_int32_t *)pos;
}



/*-------------------------------------------------------------------------------
 Routine:    InsertOffset    -    Add an offset and adjust existing offsets by delta.

 Function:    Add an offset at 'index' by shifting 'index+1' through the last offset
 and adjusting them by 'delta', the size of the record to be inserted.
 The number of records contained in the node is also incremented.

 Input:        btreePtr    - pointer to BTree control block
 node        - pointer to node
 index        - index at which to insert record
 delta        - size of record to be inserted

 Result:        none
 -------------------------------------------------------------------------------*/

void        InsertOffset        (BTreeControlBlockPtr     btreePtr,
                                 NodeDescPtr             node,
                                 u_int16_t                 index,
                                 u_int16_t                 delta )
{
    u_int16_t    *src, *dst;
    u_int16_t     numOffsets;

    src = GetOffsetAddress (btreePtr, node, node->numRecords);    // point to free offset
    dst = src - 1;                                                 // point to new offset
    numOffsets = node->numRecords++ - index;            // subtract index  & postincrement

    do {
        *dst++ = *src++ + delta;                                // to tricky?
    } while (numOffsets--);
}



/*-------------------------------------------------------------------------------

 Routine:    DeleteOffset    -    Delete an offset.

 Function:    Delete the offset at 'index' by shifting 'index+1' through the last offset
 and adjusting them by the size of the record 'index'.
 The number of records contained in the node is also decremented.

 Input:        btreePtr    - pointer to BTree control block
 node        - pointer to node
 index        - index at which to delete record

 Result:        none
 -------------------------------------------------------------------------------*/

void        DeleteOffset        (BTreeControlBlockPtr     btreePtr,
                                 NodeDescPtr             node,
                                 u_int16_t                 index )
{
    u_int16_t        *src, *dst;
    u_int16_t         numOffsets;
    u_int16_t         delta;

    dst            = GetOffsetAddress (btreePtr, node, index);
    src            = dst - 1;
    delta        = *src - *dst;
    numOffsets    = --node->numRecords - index;    // predecrement numRecords & subtract index

    while (numOffsets--)
    {
        *--dst = *--src - delta;                // work our way left
    }
}


