//
//  lf_hfs_btree_misc_ops.c
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#include "lf_hfs_btrees_private.h"
#include "lf_hfs_btrees_io.h"
#include "lf_hfs_utils.h"

////////////////////////////// Routine Definitions //////////////////////////////

/*-------------------------------------------------------------------------------
 Routine:    CalcKeyRecordSize    -    Return size of combined key/record structure.

 Function:    Rounds keySize and recSize so they will end on word boundaries.
 Does NOT add size of offset.

 Input:        keySize        - length of key (including length field)
 recSize        - length of record data

 Output:        none

 Result:        u_int16_t    - size of combined key/record that will be inserted in btree
 -------------------------------------------------------------------------------*/

u_int16_t        CalcKeyRecordSize        (u_int16_t                 keySize,
                                           u_int16_t                 recSize )
{
    if ( M_IsOdd (keySize) )    keySize += 1;    // pad byte

    if (M_IsOdd (recSize) )        recSize += 1;    // pad byte

    return    (keySize + recSize);
}



/*-------------------------------------------------------------------------------
 Routine:    VerifyHeader    -    Validate fields of the BTree header record.

 Function:    Examines the fields of the BTree header record to determine if the
 fork appears to contain a valid BTree.

 Input:        forkPtr        - pointer to fork control block
 header        - pointer to BTree header


 Result:        noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus    VerifyHeader    (FCB                *filePtr,
                             BTHeaderRec             *header )
{
    u_int64_t        forkSize;
    u_int32_t        totalNodes;


    switch (header->nodeSize)                            // node size == 512*2^n
    {
        case   512:
        case  1024:
        case  2048:
        case  4096:
        case  8192:
        case 16384:
        case 32768:        break;
        default:        return    fsBTInvalidHeaderErr;            //E_BadNodeType
    }

    totalNodes = header->totalNodes;

    forkSize = (u_int64_t)totalNodes * (u_int64_t)header->nodeSize;

    if ( forkSize > (u_int64_t)filePtr->fcbEOF )
        return fsBTInvalidHeaderErr;

    if ( header->freeNodes >= totalNodes )
        return fsBTInvalidHeaderErr;

    if ( header->rootNode >= totalNodes )
        return fsBTInvalidHeaderErr;

    if ( header->firstLeafNode >= totalNodes )
        return fsBTInvalidHeaderErr;

    if ( header->lastLeafNode >= totalNodes )
        return fsBTInvalidHeaderErr;

    if ( header->treeDepth > kMaxTreeDepth )
        return fsBTInvalidHeaderErr;


    /////////////////////////// Check BTree Type ////////////////////////////////

    switch (header->btreeType)
    {
        case    0:                    // HFS Type - no Key Descriptor
        case    kUserBTreeType:        // with Key Descriptors etc.
        case    kReservedBTreeType:    // Desktop Mgr BTree ?
            break;

        default:                    return fsBTUnknownVersionErr;
    }

    return noErr;
}



OSStatus TreeIsDirty(BTreeControlBlockPtr btreePtr)
{
    return (btreePtr->flags & kBTHeaderDirty);
}



/*-------------------------------------------------------------------------------
 Routine:    UpdateHeader    -    Write BTreeInfoRec fields to Header node.

 Function:    Checks the kBTHeaderDirty flag in the BTreeInfoRec and updates the
 header node if necessary.

 Input:        btreePtr        - pointer to BTreeInfoRec


 Result:        noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus UpdateHeader(BTreeControlBlockPtr btreePtr, Boolean forceWrite)
{
    OSStatus                err;
    BlockDescriptor            node;
    BTHeaderRec    *header;
    u_int32_t options;

    if ((btreePtr->flags & kBTHeaderDirty) == 0)            // btree info already flushed
        return    noErr;

    err = GetNode (btreePtr, kHeaderNodeNum, 0, &node );
    if (err != noErr) {
        return    err;
    }

    // XXXdbg
    ModifyBlockStart(btreePtr->fileRefNum, &node);

    header = (BTHeaderRec*) ((char *)node.buffer + sizeof(BTNodeDescriptor));

    header->treeDepth        = btreePtr->treeDepth;
    header->rootNode        = btreePtr->rootNode;
    header->leafRecords        = btreePtr->leafRecords;
    header->firstLeafNode    = btreePtr->firstLeafNode;
    header->lastLeafNode    = btreePtr->lastLeafNode;
    header->nodeSize        = btreePtr->nodeSize;            // this shouldn't change
    header->maxKeyLength    = btreePtr->maxKeyLength;        // neither should this
    header->totalNodes        = btreePtr->totalNodes;
    header->freeNodes        = btreePtr->freeNodes;
    header->btreeType        = btreePtr->btreeType;

    // ignore    header->clumpSize;                            // rename this field?

    if (forceWrite)
        options = kForceWriteBlock;
    else
        options = kLockTransaction;

    err = UpdateNode (btreePtr, &node, 0, options);

    btreePtr->flags &= (~kBTHeaderDirty);

    return    err;
}



/*-------------------------------------------------------------------------------
 Routine:    FindIteratorPosition    -    One_line_description.

 Function:    Brief_description_of_the_function_and_any_side_effects

 Algorithm:    see FSC.BT.BTIterateRecord.PICT

 Note:        // document side-effects of bad node hints

 Input:        btreePtr        - description
 iterator        - description


 Output:        iterator        - description
 left            - description
 middle            - description
 right            - description
 nodeNum            - description
 returnIndex        - description
 foundRecord        - description


 Result:        noErr        - success
 != noErr    - failure
 -------------------------------------------------------------------------------*/

OSStatus    FindIteratorPosition    (BTreeControlBlockPtr     btreePtr,
                                     BTreeIteratorPtr         iterator,
                                     BlockDescriptor        *left,
                                     BlockDescriptor        *middle,
                                     BlockDescriptor        *right,
                                     u_int32_t                *returnNodeNum,
                                     u_int16_t                *returnIndex,
                                     Boolean                *foundRecord )
{
    OSStatus        err;
    Boolean            foundIt;
    u_int32_t        nodeNum;
    u_int16_t        leftIndex,    index,    rightIndex;
    Boolean            validHint;

    // assume btreePtr valid
    // assume left, middle, right point to BlockDescriptors
    // assume nodeNum points to u_int32_t
    // assume index points to u_int16_t
    // assume foundRecord points to Boolean

    left->buffer        = nil;
    left->blockHeader   = nil;
    middle->buffer        = nil;
    middle->blockHeader    = nil;
    right->buffer        = nil;
    right->blockHeader    = nil;

    foundIt                = false;

    if (iterator == nil)                        // do we have an iterator?
    {
        err = fsBTInvalidIteratorErr;
        goto ErrorExit;
    }

    err = IsItAHint (btreePtr, iterator, &validHint);
    M_ExitOnError (err);

    nodeNum = iterator->hint.nodeNum;
    if (! validHint)                            // does the hint appear to be valid?
    {
        goto SearchTheTree;
    }

    err = GetNode (btreePtr, nodeNum, kGetNodeHint, middle);
    if( err == fsBTInvalidNodeErr )    // returned if nodeNum is out of range
        goto SearchTheTree;

    M_ExitOnError (err);

    if ( ((NodeDescPtr) middle->buffer)->kind != kBTLeafNode ||
        ((NodeDescPtr) middle->buffer)->numRecords <= 0 )
    {
        goto SearchTheTree;
    }

    foundIt = SearchNode (btreePtr, middle->buffer, &iterator->key, &index);
    if (foundIt == true)
    {
        ++btreePtr->numValidHints;
        goto SuccessfulExit;
    }
    iterator->hint.nodeNum = 0;

    if (index == 0)
    {
        if (((NodeDescPtr) middle->buffer)->bLink == 0)        // before 1st btree record
        {
            goto SuccessfulExit;
        }

        nodeNum = ((NodeDescPtr) middle->buffer)->bLink;

        // BTree nodes are always grabbed in left to right order.
        // Therefore release the current node before looking up the
        // left node.
        err = ReleaseNode(btreePtr, middle);
        M_ExitOnError(err);

        // Look up the left node
        err = GetNode (btreePtr, nodeNum, 0, left);
        M_ExitOnError (err);

        // Look up the current node again
        err = GetRightSiblingNode (btreePtr, left->buffer, middle);
        M_ExitOnError (err);

        if ( ((NodeDescPtr) left->buffer)->kind != kBTLeafNode ||
            ((NodeDescPtr) left->buffer)->numRecords <= 0 )
        {
            goto SearchTheTree;
        }

        foundIt = SearchNode (btreePtr, left->buffer, &iterator->key, &leftIndex);
        if (foundIt == true)
        {
            *right            = *middle;
            *middle            = *left;
            left->buffer    = nil;
            index            = leftIndex;

            goto SuccessfulExit;
        }

        if (leftIndex == 0)                                    // we're lost!
        {
            goto SearchTheTree;
        }
        else if (leftIndex >= ((NodeDescPtr) left->buffer)->numRecords)
        {
            nodeNum = ((NodeDescPtr) left->buffer)->fLink;
            if (index != 0)
            {
                LFHFS_LOG(LEVEL_ERROR, "FindIteratorPosition: index != 0\n");
                hfs_assert(0);
            }
            goto SuccessfulExit;
        }
        else
        {
            *right            = *middle;
            *middle            = *left;
            left->buffer    = nil;
            index            = leftIndex;

            goto SuccessfulExit;
        }
    }
    else if (index >= ((NodeDescPtr) middle->buffer)->numRecords)
    {
        if (((NodeDescPtr) middle->buffer)->fLink == 0)    // beyond last record
        {
            goto SuccessfulExit;
        }

        nodeNum = ((NodeDescPtr) middle->buffer)->fLink;

        err = GetRightSiblingNode (btreePtr, middle->buffer, right);
        M_ExitOnError (err);

        if ( ((NodeDescPtr) right->buffer)->kind != kBTLeafNode ||
            ((NodeDescPtr) right->buffer)->numRecords <= 0 )
        {
            goto SearchTheTree;
        }

        foundIt = SearchNode (btreePtr, right->buffer, &iterator->key, &rightIndex);
        if (rightIndex >= ((NodeDescPtr) right->buffer)->numRecords)        // we're lost
        {
            goto SearchTheTree;
        }
        else    // we found it, or rightIndex==0, or rightIndex<numRecs
        {
            *left            = *middle;
            *middle            = *right;
            right->buffer    = nil;
            index            = rightIndex;

            goto SuccessfulExit;
        }
    }


    //////////////////////////// Search The Tree ////////////////////////////////

SearchTheTree:
    {
        TreePathTable    treePathTable;        // so we only use stack space if we need to

        err = ReleaseNode (btreePtr, left);            M_ExitOnError (err);
        err = ReleaseNode (btreePtr, middle);        M_ExitOnError (err);
        err = ReleaseNode (btreePtr, right);        M_ExitOnError (err);

        err = SearchTree ( btreePtr, &iterator->key, treePathTable, &nodeNum, middle, &index);
        switch (err)                // separate find condition from exceptions
        {
            case noErr:            foundIt = true;                break;
            case fsBTRecordNotFoundErr:                        break;
            default:                goto ErrorExit;
        }
    }

    /////////////////////////////// Success! ////////////////////////////////////

SuccessfulExit:

    *returnNodeNum    = nodeNum;
    *returnIndex     = index;
    *foundRecord    = foundIt;

    return    noErr;


    ////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

    (void)    ReleaseNode (btreePtr, left);
    (void)    ReleaseNode (btreePtr, middle);
    (void)    ReleaseNode (btreePtr, right);

    *returnNodeNum    = 0;
    *returnIndex     = 0;
    *foundRecord    = false;

    return    err;
}



/////////////////////////////// CheckInsertParams ///////////////////////////////

OSStatus    CheckInsertParams        (FCB                        *filePtr,
                                      BTreeIterator                *iterator,
                                      FSBufferDescriptor            *record,
                                      u_int16_t                     recordLen )
{
    BTreeControlBlockPtr    btreePtr;

    if (filePtr == nil)                                    return    paramErr;

    btreePtr = (BTreeControlBlockPtr) filePtr->fcbBTCBPtr;
    if (btreePtr == nil)                                return    fsBTInvalidFileErr;
    if (iterator == nil)                                return    paramErr;
    if (record     == nil)                                return    paramErr;

    //    check total key/record size limit
    if ( CalcKeyRecordSize (CalcKeySize(btreePtr, &iterator->key), recordLen) > (btreePtr->nodeSize >> 1))
        return    fsBTRecordTooLargeErr;

    return    noErr;
}



/*-------------------------------------------------------------------------------
 Routine:    TrySimpleReplace    -    Attempts a simple insert, set, or replace.

 Function:    If a hint exitst for the iterator, attempt to find the key in the hint
 node. If the key is found, an insert operation fails. If the is not
 found, a replace operation fails. If the key was not found, and the
 insert position is greater than 0 and less than numRecords, the record
 is inserted, provided there is enough freeSpace.  If the key was found,
 and there is more freeSpace than the difference between the new record
 and the old record, the old record is deleted and the new record is
 inserted.

 Assumptions:    iterator key has already been checked by CheckKey


 Input:        btreePtr        - description
 iterator        - description
 record            - description
 recordLen        - description
 operation        - description


 Output:        recordInserted        - description


 Result:        noErr            - success
 E_RecordExits        - insert operation failure
 != noErr        - GetNode, ReleaseNode, UpdateNode returned an error
 -------------------------------------------------------------------------------*/

OSStatus    TrySimpleReplace        (BTreeControlBlockPtr     btreePtr,
                                     NodeDescPtr             nodePtr,
                                     BTreeIterator            *iterator,
                                     FSBufferDescriptor        *record,
                                     u_int16_t                 recordLen,
                                     Boolean                *recordInserted )
{
    u_int32_t            oldSpace;
    u_int32_t            spaceNeeded;
    u_int16_t            index;
    u_int16_t            keySize;
    Boolean                foundIt;
    Boolean                didItFit;


    *recordInserted    = false;                                // we'll assume this won't work...

    if ( nodePtr->kind != kBTLeafNode )
        return    noErr;    // we're in the weeds!

    foundIt    = SearchNode (btreePtr, nodePtr, &iterator->key, &index);

    if ( foundIt == false )
        return    noErr;    // we might be lost...

    keySize = CalcKeySize(btreePtr, &iterator->key);    // includes length field

    spaceNeeded    = CalcKeyRecordSize (keySize, recordLen);

    oldSpace = GetRecordSize (btreePtr, nodePtr, index);

    if ( spaceNeeded == oldSpace )
    {
        u_int8_t *        dst;

        dst = GetRecordAddress (btreePtr, nodePtr, index);

        if ( M_IsOdd (keySize) )
            ++keySize;            // add pad byte

        dst += keySize;        // skip over key to point at record

        BlockMoveData(record->bufferAddress, dst, recordLen);    // blast away...

        *recordInserted = true;
    }
    else if ( (GetNodeFreeSize(btreePtr, nodePtr) + oldSpace) >= spaceNeeded)
    {
        DeleteRecord (btreePtr, nodePtr, index);

        didItFit = InsertKeyRecord (btreePtr, nodePtr, index,
                                    &iterator->key, KeyLength(btreePtr, &iterator->key),
                                    record->bufferAddress, recordLen);
        if (didItFit == false)
        {
            LFHFS_LOG(LEVEL_ERROR, "TrySimpleInsert: InsertKeyRecord returned false!");
            hfs_assert(0);
        }
        *recordInserted = true;
    }
    // else not enough space...

    return    noErr;
}


/*-------------------------------------------------------------------------------
 Routine:    IsItAHint    -    checks the hint within a BTreeInterator.

 Function:    checks the hint within a BTreeInterator.  If it is non-zero, it may
 possibly be valid.

 Input:        btreePtr    - pointer to control block for BTree file
 iterator    - pointer to BTreeIterator

 Output:        answer        - true if the hint looks reasonable
 - false if the hint is 0

 Result:        noErr            - success
 -------------------------------------------------------------------------------*/


OSStatus    IsItAHint    (BTreeControlBlockPtr btreePtr, BTreeIterator *iterator, Boolean *answer)
{
    ++btreePtr->numHintChecks;
    if (iterator->hint.nodeNum == 0)
    {
        *answer = false;
    }
    else
    {
        *answer = true;
        ++btreePtr->numPossibleHints;
    }

    return noErr;
}
