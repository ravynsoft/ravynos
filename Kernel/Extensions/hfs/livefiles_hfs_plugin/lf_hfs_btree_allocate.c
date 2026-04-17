//
//  lf_hfs_btree_allocate.c
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//
#include "lf_hfs_btrees_io.h"
#include "lf_hfs_endian.h"
#include "lf_hfs_btrees_private.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_generic_buf.h"

///////////////////// Routines Internal To BTreeAllocate.c //////////////////////

static OSStatus    GetMapNode (BTreeControlBlockPtr      btreePtr,
                               BlockDescriptor             *nodePtr,
                               u_int16_t                    **mapPtr,
                               u_int16_t                     *mapSize );

/////////////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------------------

 Routine:    AllocateNode    -    Find Free Node, Mark It Used, and Return Node Number.

 Function:    Searches the map records for the first free node, marks it "in use" and
 returns the node number found. This routine should really only be called
 when we know there are free blocks, otherwise it's just a waste of time.

 Note:        We have to examine map nodes a word at a time rather than a long word
 because the External BTree Mgr used map records that were not an integral
 number of long words. Too bad. In our spare time could develop a more
 sophisticated algorithm that read map records by long words (and long
 word aligned) and handled the spare bytes at the beginning and end
 appropriately.

 Input:        btreePtr    - pointer to control block for BTree file

 Output:        nodeNum        - number of node allocated


 Result:        noErr            - success
 fsBTNoMoreMapNodesErr    - no free blocks were found
 != noErr        - failure
 -------------------------------------------------------------------------------*/

OSStatus    AllocateNode (BTreeControlBlockPtr        btreePtr, u_int32_t    *nodeNum)
{
    OSStatus         err;
    BlockDescriptor     node;
    u_int16_t        *mapPtr, *pos;
    u_int16_t         mapSize, size;
    u_int16_t         freeWord;
    u_int16_t         mask;
    u_int16_t         bitOffset;
    u_int32_t         nodeNumber;


    nodeNumber        = 0;                // first node number of header map record
    node.buffer        = nil;                // clear node.buffer to get header node
    //    - and for ErrorExit
    node.blockHeader = nil;

    while (true)
    {
        err = GetMapNode (btreePtr, &node, &mapPtr, &mapSize);
        M_ExitOnError (err);

        // XXXdbg
        ModifyBlockStart(btreePtr->fileRefNum, &node);

        //////////////////////// Find Word with Free Bit ////////////////////////////

        pos        = mapPtr;
        size    = mapSize;
        size  >>= 1;                        // convert to number of words
        //assumes mapRecords contain an integral number of words

        while ( size-- )
        {
            if ( *pos++ != 0xFFFF )            // assume test fails, and increment pos
                break;
        }

        --pos;                                // whoa! backup

        if (*pos != 0xFFFF)                    // hey, we got one!
            break;

        nodeNumber += mapSize << 3;            // covert to number of bits (nodes)
    }

    ///////////////////////// Find Free Bit in Word /////////////////////////////

    freeWord    = SWAP_BE16 (*pos);
    bitOffset    =  15;
    mask        =  0x8000;

    do {
        if ( (freeWord & mask) == 0)
            break;
        mask >>= 1;
    } while (--bitOffset);

    ////////////////////// Calculate Free Node Number ///////////////////////////

    nodeNumber += ((pos - mapPtr) << 4) + (15 - bitOffset);    // (pos-mapPtr) = # of words!


    ///////////////////////// Check for End of Map //////////////////////////////

    if (nodeNumber >= btreePtr->totalNodes)
    {
        err = fsBTFullErr;
        goto ErrorExit;
    }

    /////////////////////////// Allocate the Node ///////////////////////////////

    *pos |= SWAP_BE16 (mask);                // set the map bit for the node

    err = UpdateNode (btreePtr, &node, 0, kLockTransaction);
    M_ExitOnError (err);

    --btreePtr->freeNodes;
    M_BTreeHeaderDirty(btreePtr);

    /* Account for allocations from node reserve */
    BTUpdateReserve(btreePtr, 1);

    *nodeNum = nodeNumber;

    return noErr;

    ////////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

    (void) ReleaseNode (btreePtr, &node);
    *nodeNum = 0;

    return    err;
}



/*-------------------------------------------------------------------------------

 Routine:    FreeNode    -    Clear allocation bit for node.

 Function:    Finds the bit representing the node specified by nodeNum in the node
 map and clears the bit.


 Input:        btreePtr    - pointer to control block for BTree file
 nodeNum        - number of node to mark free

 Output:        none

 Result:        noErr            - success
 fsBTNoMoreMapNodesErr    - node number is beyond end of node map
 != noErr        - GetNode or ReleaseNode encountered some difficulty
 -------------------------------------------------------------------------------*/

OSStatus    FreeNode (BTreeControlBlockPtr        btreePtr, u_int32_t    nodeNum)
{
    OSStatus         err;
    BlockDescriptor     node;
    u_int32_t         nodeIndex;
    u_int16_t         mapSize = 0;
    u_int16_t        *mapPos = NULL;
    u_int16_t         bitOffset;


    //////////////////////////// Find Map Record ////////////////////////////////
    nodeIndex            = 0;                // first node number of header map record
    node.buffer            = nil;                // invalidate node.buffer to get header node
    node.blockHeader    = nil;

    while (nodeNum >= nodeIndex)
    {
        err = GetMapNode (btreePtr, &node, &mapPos, &mapSize);
        M_ExitOnError (err);

        nodeIndex += mapSize << 3;            // covert to number of bits (nodes)
    }

    //////////////////////////// Mark Node Free /////////////////////////////////

    // XXXdbg
    ModifyBlockStart(btreePtr->fileRefNum, &node);

    nodeNum -= (nodeIndex - (mapSize << 3));            // relative to this map record
    bitOffset = 15 - (nodeNum & 0x0000000F);            // last 4 bits are bit offset
    mapPos += nodeNum >> 4;                                // point to word containing map bit

    M_SWAP_BE16_ClearBitNum (*mapPos, bitOffset);        // clear it

    err = UpdateNode (btreePtr, &node, 0, kLockTransaction);
    M_ExitOnError (err);

    ++btreePtr->freeNodes;
    M_BTreeHeaderDirty(btreePtr);

    return noErr;

ErrorExit:

    (void) ReleaseNode (btreePtr, &node);

    return    err;
}



/*-------------------------------------------------------------------------------

 Routine:    ExtendBTree    -    Call FSAgent to extend file, and allocate necessary map nodes.

 Function:    This routine calls the the FSAgent to extend the end of fork, if necessary,
 to accomodate the number of nodes requested. It then allocates as many
 map nodes as are necessary to account for all the nodes in the B*Tree.
 If newTotalNodes is less than the current number of nodes, no action is
 taken.

 Note:        Internal HFS File Manager BTree Module counts on an integral number of
 long words in map records, although they are not long word aligned.

 Input:        btreePtr        - pointer to control block for BTree file
 newTotalNodes    - total number of nodes the B*Tree is to extended to

 Output:        none

 Result:        noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus    ExtendBTree    (BTreeControlBlockPtr    btreePtr,
                            u_int32_t                newTotalNodes )
{
    OSStatus                 err;
    FCB                        *filePtr;
    FSSize                     minEOF, maxEOF;
    u_int16_t                 nodeSize;
    u_int32_t                 oldTotalNodes;
    u_int32_t                 newMapNodes;
    u_int32_t                 mapBits, totalMapBits;
    u_int32_t                 recStartBit;
    u_int32_t                 nodeNum, nextNodeNum;
    u_int32_t                 firstNewMapNodeNum, lastNewMapNodeNum;
    BlockDescriptor             mapNode, newNode;
    u_int16_t                *mapPos;
    u_int16_t                *mapStart;
    u_int16_t                 mapSize;
    u_int16_t                 mapNodeRecSize;
    u_int32_t                 bitInWord, bitInRecord;
    u_int16_t                 mapIndex;


    oldTotalNodes = btreePtr->totalNodes;
    if (newTotalNodes <= oldTotalNodes)                // we're done!
        return    noErr;

    nodeSize            = btreePtr->nodeSize;
    filePtr             = GetFileControlBlock(btreePtr->fileRefNum);

    mapNode.buffer      = nil;
    mapNode.blockHeader = nil;
    newNode.buffer      = nil;
    newNode.blockHeader = nil;

    mapNodeRecSize    = nodeSize - sizeof(BTNodeDescriptor) - 6;    // 2 bytes of free space (see note)


    //////////////////////// Count Bits In Node Map /////////////////////////////

    totalMapBits = 0;
    do {
        err = GetMapNode (btreePtr, &mapNode, &mapStart, &mapSize);
        M_ExitOnError (err);

        mapBits        = mapSize << 3;                // mapSize (in bytes) * 8
        recStartBit    = totalMapBits;                // bit number of first bit in map record
        totalMapBits  += mapBits;

    } while ( ((BTNodeDescriptor*)mapNode.buffer)->fLink != 0 );

#if DEBUG
    if (totalMapBits != CalcMapBits (btreePtr))
        LFHFS_LOG(LEVEL_ERROR, "ExtendBTree: totalMapBits != CalcMapBits");
#endif

    /////////////////////// Extend LEOF If Necessary ////////////////////////////

    minEOF = (u_int64_t)newTotalNodes * (u_int64_t)nodeSize;
    if ( (u_int64_t)filePtr->fcbEOF < minEOF )
    {
        maxEOF = (u_int64_t)0x7fffffffLL * (u_int64_t)nodeSize;

        err = btreePtr->setEndOfForkProc (btreePtr->fileRefNum, minEOF, maxEOF);
        M_ExitOnError (err);
    }


    //////////////////// Calc New Total Number Of Nodes /////////////////////////

    newTotalNodes = (uint32_t)(filePtr->fcbEOF / nodeSize);        // hack!
    // do we wish to perform any verification of newTotalNodes at this point?

    btreePtr->totalNodes     =  newTotalNodes;        // do we need to update freeNodes here too?


    ////////////// Calculate Number Of New Map Nodes Required ///////////////////

    newMapNodes        = 0;
    if (newTotalNodes > totalMapBits)
    {
        newMapNodes            = (((newTotalNodes - totalMapBits) >> 3) / mapNodeRecSize) + 1;
        firstNewMapNodeNum    = oldTotalNodes;
        lastNewMapNodeNum    = firstNewMapNodeNum + newMapNodes - 1;
    }
    else
    {
        err = ReleaseNode (btreePtr, &mapNode);
        M_ExitOnError (err);

        goto Success;
    }


    /////////////////////// Initialize New Map Nodes ////////////////////////////
    // XXXdbg - this is the correct place for this:
    ModifyBlockStart(btreePtr->fileRefNum, &mapNode);

    ((BTNodeDescriptor*)mapNode.buffer)->fLink = firstNewMapNodeNum;

    nodeNum        = firstNewMapNodeNum;
    while (true)
    {
        err = GetNewNode (btreePtr, nodeNum, &newNode);
        M_ExitOnError (err);

        // XXXdbg
        ModifyBlockStart(btreePtr->fileRefNum, &newNode);

        ((NodeDescPtr)newNode.buffer)->numRecords    = 1;
        ((NodeDescPtr)newNode.buffer)->kind = kBTMapNode;

        // set free space offset
        *(u_int16_t *)((Ptr)newNode.buffer + nodeSize - 4) = nodeSize - 6;

        if (nodeNum++ == lastNewMapNodeNum)
            break;

        ((BTNodeDescriptor*)newNode.buffer)->fLink = nodeNum;    // point to next map node

        err = UpdateNode (btreePtr, &newNode, 0, kLockTransaction);
        M_ExitOnError (err);
    }

    err = UpdateNode (btreePtr, &newNode, 0, kLockTransaction);
    M_ExitOnError (err);


    ///////////////////// Mark New Map Nodes Allocated //////////////////////////

    nodeNum = firstNewMapNodeNum;
    do {
        bitInRecord    = nodeNum - recStartBit;

        while (bitInRecord >= mapBits)
        {
            nextNodeNum = ((NodeDescPtr)mapNode.buffer)->fLink;
            if ( nextNodeNum == 0)
            {
                err = fsBTNoMoreMapNodesErr;
                goto ErrorExit;
            }

            err = UpdateNode (btreePtr, &mapNode, 0, kLockTransaction);
            M_ExitOnError (err);

            err = GetNode (btreePtr, nextNodeNum, 0, &mapNode);
            M_ExitOnError (err);

            // XXXdbg
            ModifyBlockStart(btreePtr->fileRefNum, &mapNode);

            mapIndex = 0;

            mapStart     = (u_int16_t *) GetRecordAddress (btreePtr, mapNode.buffer, mapIndex);
            mapSize         = GetRecordSize (btreePtr, mapNode.buffer, mapIndex);

#if DEBUG
            if (mapSize != M_MapRecordSize (btreePtr->nodeSize) )
            {
                LFHFS_LOG(LEVEL_ERROR, "ExtendBTree: mapSize != M_MapRecordSize");
            }
#endif

            mapBits        = mapSize << 3;        // mapSize (in bytes) * 8
            recStartBit    = totalMapBits;        // bit number of first bit in map record
            totalMapBits  += mapBits;

            bitInRecord    = nodeNum - recStartBit;
        }

        mapPos        = mapStart + ((nodeNum - recStartBit) >> 4);
        bitInWord    = 15 - ((nodeNum - recStartBit) & 0x0000000F);

        M_SWAP_BE16_SetBitNum (*mapPos, bitInWord);

        ++nodeNum;

    } while (nodeNum <= lastNewMapNodeNum);

    err = UpdateNode (btreePtr, &mapNode, 0, kLockTransaction);
    M_ExitOnError (err);


    //////////////////////////////// Success ////////////////////////////////////

Success:

    btreePtr->totalNodes     =  newTotalNodes;
    btreePtr->freeNodes        += (newTotalNodes - oldTotalNodes) - newMapNodes;

    M_BTreeHeaderDirty(btreePtr);

    /* Force the b-tree header changes to disk */
    (void) UpdateHeader (btreePtr, true);

    return    noErr;


    ////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

    (void) ReleaseNode (btreePtr, &mapNode);
    (void) ReleaseNode (btreePtr, &newNode);

    return    err;
}



/*-------------------------------------------------------------------------------

 Routine:    GetMapNode    -    Get the next map node and pointer to the map record.

 Function:    Given a BlockDescriptor to a map node in nodePtr, GetMapNode releases
 it and gets the next node. If nodePtr->buffer is nil, then the header
 node is retrieved.


 Input:        btreePtr    - pointer to control block for BTree file
 nodePtr        - pointer to a BlockDescriptor of a map node

 Output:        nodePtr        - pointer to the BlockDescriptor for the next map node
 mapPtr        - pointer to the map record within the map node
 mapSize        - number of bytes in the map record

 Result:        noErr            - success
 fsBTNoMoreMapNodesErr    - we've run out of map nodes
 fsBTInvalidNodeErr            - bad node, or not node type kMapNode
 != noErr        - failure
 -------------------------------------------------------------------------------*/

static
OSStatus    GetMapNode (BTreeControlBlockPtr      btreePtr,
                        BlockDescriptor             *nodePtr,
                        u_int16_t                **mapPtr,
                        u_int16_t                 *mapSize )
{
    OSStatus    err;
    u_int16_t    mapIndex;
    u_int32_t    nextNodeNum;

    if (nodePtr->buffer != nil)        // if iterator is valid...
    {
        nextNodeNum = ((NodeDescPtr)nodePtr->buffer)->fLink;
        if (nextNodeNum == 0)
        {
            err = fsBTNoMoreMapNodesErr;
            goto ErrorExit;
        }

        err = ReleaseNode (btreePtr, nodePtr);
        M_ExitOnError (err);

        err = GetNode (btreePtr, nextNodeNum, 0, nodePtr);
        M_ExitOnError (err);

        if ( ((NodeDescPtr)nodePtr->buffer)->kind != kBTMapNode)
        {
            err = fsBTBadNodeType;
            goto ErrorExit;
        }

        ++btreePtr->numMapNodesRead;
        mapIndex = 0;
    } else {
        err = GetNode (btreePtr, kHeaderNodeNum, 0, nodePtr);
        M_ExitOnError (err);

        if ( ((NodeDescPtr)nodePtr->buffer)->kind != kBTHeaderNode)
        {
            err = fsBTInvalidHeaderErr;                //or fsBTBadNodeType
            goto ErrorExit;
        }

        mapIndex = 2;
    }


    *mapPtr        = (u_int16_t *) GetRecordAddress (btreePtr, nodePtr->buffer, mapIndex);
    *mapSize    = GetRecordSize (btreePtr, nodePtr->buffer, mapIndex);

    return noErr;


ErrorExit:

    (void) ReleaseNode (btreePtr, nodePtr);

    *mapPtr        = nil;
    *mapSize    = 0;

    return    err;
}



////////////////////////////////// CalcMapBits //////////////////////////////////

u_int32_t        CalcMapBits    (BTreeControlBlockPtr     btreePtr)
{
    u_int32_t        mapBits;

    mapBits        = (u_int32_t)(M_HeaderMapRecordSize (btreePtr->nodeSize) << 3);

    while (mapBits < btreePtr->totalNodes)
        mapBits    += M_MapRecordSize (btreePtr->nodeSize) << 3;

    return    mapBits;
}

/*-------------------------------------------------------------------------------
 Routine:    BTZeroUnusedNodes

 Function:    Write zeros to all nodes in the B-tree that are not currently in use.
 -------------------------------------------------------------------------------*/
int
BTZeroUnusedNodes(FCB *filePtr)
{
    int                      err=0;
    u_int16_t                *mapPtr, *pos;
    u_int16_t                mapSize, size;
    u_int16_t                mask;
    u_int16_t                bitNumber;
    u_int16_t                word;

    vnode_t vp = FTOV(filePtr);
    BTreeControlBlockPtr btreePtr = (BTreeControlBlockPtr) filePtr->fcbBTCBPtr;
    GenericLFBufPtr bp = NULL;
    u_int32_t nodeNumber = 0;
    BlockDescriptor  mapNode = {0};
    mapNode.buffer = nil;
    mapNode.blockHeader = nil;

    /* Iterate over map nodes. */
    while (true)
    {
        err = GetMapNode (btreePtr, &mapNode, &mapPtr, &mapSize);
        if (err)
        {
            err = MacToVFSError(err);
            goto ErrorExit;
        }

        pos        = mapPtr;
        size    = mapSize;
        size  >>= 1;                    /* convert to number of 16-bit words */

        /* Iterate over 16-bit words in the map record. */
        while (size--)
        {
            if (*pos != 0xFFFF)            /* Anything free in this word? */
            {
                word = SWAP_BE16(*pos);

                /* Iterate over bits in the word. */
                for (bitNumber = 0, mask = 0x8000;
                     bitNumber < 16;
                     ++bitNumber, mask >>= 1)
                {
                    if (word & mask)
                        continue;                /* This node is in use. */

                    if (nodeNumber + bitNumber >= btreePtr->totalNodes)
                    {
                        /* We've processed all of the nodes. */
                        goto done;
                    }

                    /*
                     * Get a buffer full of zeros and write it to the unused
                     * node.  Since we'll probably be writing a lot of nodes,
                     * bypass the journal (to avoid a transaction that's too
                     * big).  Instead, this behaves more like clearing out
                     * nodes when extending a B-tree (eg., ClearBTNodes).
                     */
                    bp = lf_hfs_generic_buf_allocate(vp, nodeNumber + bitNumber, btreePtr->nodeSize, 0); // buf_getblk(vp, nodeNumber + bitNumber, btreePtr->nodeSize, 0, 0, BLK_META);
                    if (bp == NULL)
                    {
                        LFHFS_LOG(LEVEL_ERROR , "BTZeroUnusedNodes: unable to read node %u\n", nodeNumber + bitNumber);
                        err = EIO;
                        goto ErrorExit;
                    }

                    if (bp->uCacheFlags & GEN_BUF_WRITE_LOCK) {
                        /*
                         * This node is already part of a transaction and will be written when
                         * the transaction is committed, so don't write it here.  If we did, then
                         * we'd hit a panic in hfs_vnop_bwrite because the B_LOCKED bit is still set.
                         */
                        lf_hfs_generic_buf_release(bp);
                        continue;
                    }

                    lf_hfs_generic_buf_clear(bp);

                    err = lf_hfs_generic_buf_write(bp);
                    if (err) {
                        goto ErrorExit;
                    }

                    lf_hfs_generic_buf_release(bp);
                }
            }

            /* Go to the next word in the bitmap */
            ++pos;
            nodeNumber += 16;
        }
    }

ErrorExit:
done:
    (void) ReleaseNode(btreePtr, &mapNode);

    return err;
}

