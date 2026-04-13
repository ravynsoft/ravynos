//
//  lf_hfs_endian.c
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#include <stdio.h>


#include "lf_hfs_endian.h"
#include "lf_hfs_btrees_private.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_generic_buf.h"

#define DEBUG_BTNODE_SWAP 0

/*
 * Internal swapping routines
 *
 * These routines handle swapping the records of leaf and index nodes.  The
 * layout of the keys and records varies depending on the kind of B-tree
 * (determined by fileID).
 *
 * The direction parameter must be kSwapBTNodeBigToHost or kSwapBTNodeHostToBig.
 * The kSwapBTNodeHeaderRecordOnly "direction" is not valid for these routines.
 */
int hfs_swap_HFSPlusBTInternalNode (BlockDescriptor *src, HFSCatalogNodeID fileID, enum HFSBTSwapDirection direction);
void hfs_swap_HFSPlusForkData (HFSPlusForkData *src);

/*
 * hfs_swap_HFSPlusForkData
 */
void
hfs_swap_HFSPlusForkData (
                          HFSPlusForkData *src
                          )
{
    int i;

    src->logicalSize        = SWAP_BE64 (src->logicalSize);

    src->clumpSize            = SWAP_BE32 (src->clumpSize);
    src->totalBlocks        = SWAP_BE32 (src->totalBlocks);

    for (i = 0; i < kHFSPlusExtentDensity; i++) {
        src->extents[i].startBlock    = SWAP_BE32 (src->extents[i].startBlock);
        src->extents[i].blockCount    = SWAP_BE32 (src->extents[i].blockCount);
    }
}

/*
 * hfs_swap_BTNode
 *
 *  NOTE: This operation is not naturally symmetric.
 *        We have to determine which way we're swapping things.
 */
int
hfs_swap_BTNode (
                 BlockDescriptor *src,
                 vnode_t vp,
                 enum HFSBTSwapDirection direction,
                 u_int8_t allow_empty_node
                 )
{
    
    GenericLFBuf *psBuf = src->blockHeader;
    lf_hfs_generic_buf_lock(psBuf);
    
    switch(direction) {
        case kSwapBTNodeBigToHost:
            lf_hfs_generic_buf_set_cache_flag(psBuf, GEN_BUF_LITTLE_ENDIAN);
            break;
        case kSwapBTNodeHostToBig:
            lf_hfs_generic_buf_clear_cache_flag(psBuf, GEN_BUF_LITTLE_ENDIAN);
            break;
        case kSwapBTNodeHeaderRecordOnly:
            break;
        default:
            panic("invalid direction");
    }

    
    BTNodeDescriptor *srcDesc = src->buffer;
    u_int16_t *srcOffs = NULL;
    BTreeControlBlockPtr btcb = (BTreeControlBlockPtr)VTOF(vp)->fcbBTCBPtr;
    u_int16_t i; /* index to match srcDesc->numRecords */
    int error = 0;
    
    #if DEBUG_BTNODE_SWAP
        printf("hfs_swap_BTNode: direction %u (%s), psVnode %p, blockNum %llu uPhyCluster %llu\n", direction, (direction==0)?"RD":(direction==1)?"WR":"NA", vp, src->blockNum, psBuf->uPhyCluster);
        uint32_t *pData = src->buffer;
        printf("hfs_swap_BTNode: %p before: 0x%x, 0x%x, 0x%x, 0x%x\n", pData, pData[0], pData[1], pData[2], pData[3]);
    #endif

#ifdef ENDIAN_DEBUG
    if (direction == kSwapBTNodeBigToHost) {
        LFHFS_LOG(LEVEL_DEBUG, "hfs: BE -> Native Swap\n");
    } else if (direction == kSwapBTNodeHostToBig) {
        LFHFS_LOG(LEVEL_DEBUG, "hfs: Native -> BE Swap\n");
    } else if (direction == kSwapBTNodeHeaderRecordOnly) {
        LFHFS_LOG(LEVEL_DEBUG, "hfs: Not swapping descriptors\n");
    } else {
        LFHFS_LOG(LEVEL_ERROR, "hfs_swap_BTNode: This is impossible");
        hfs_assert(0);
    }
#endif

    /*
     * If we are doing a swap from on-disk to in-memory, then swap the node
     * descriptor and record offsets before we need to use them.
     */
    if (direction == kSwapBTNodeBigToHost) {
        srcDesc->fLink        = SWAP_BE32 (srcDesc->fLink);
        srcDesc->bLink        = SWAP_BE32 (srcDesc->bLink);

        /*
         * When first opening a BTree, we have to read the header node before the
         * control block is initialized.  In this case, totalNodes will be zero,
         * so skip the bounds checking. Also, we should ignore the header node when
         * checking for invalid forwards and backwards links, since the header node's
         * links can point back to itself legitimately.
         */
        if (btcb->totalNodes != 0) {
            if (srcDesc->fLink >= btcb->totalNodes) {
                LFHFS_LOG( LEVEL_ERROR, "hfs_swap_BTNode: invalid forward link (0x%08x >= 0x%08x)\n", srcDesc->fLink, btcb->totalNodes);
                error = fsBTInvalidHeaderErr;
                goto fail;
            }
            if (srcDesc->bLink >= btcb->totalNodes) {
                LFHFS_LOG( LEVEL_ERROR, "hfs_swap_BTNode: invalid backward link (0x%08x >= 0x%08x)\n", srcDesc->bLink, btcb->totalNodes);
                error = fsBTInvalidHeaderErr;
                goto fail;
            }

            if ((src->blockNum != 0) && (srcDesc->fLink == (u_int32_t) src->blockNum)) {
                LFHFS_LOG( LEVEL_ERROR, "hfs_swap_BTNode: invalid forward link (0x%08x == 0x%08x)\n", srcDesc->fLink, (u_int32_t) src->blockNum);
                error = fsBTInvalidHeaderErr;
                goto fail;
            }
            if ((src->blockNum != 0) && (srcDesc->bLink == (u_int32_t) src->blockNum)) {
                LFHFS_LOG( LEVEL_ERROR, "hfs_swap_BTNode: invalid backward link (0x%08x == 0x%08x)\n", srcDesc->bLink, (u_int32_t) src->blockNum);
                error = fsBTInvalidHeaderErr;
                goto fail;
            }


        }

        /*
         * Check srcDesc->kind.  Don't swap it because it's only one byte.
         */
        if (srcDesc->kind < kBTLeafNode || srcDesc->kind > kBTMapNode) {
            LFHFS_LOG(LEVEL_ERROR , "hfs_swap_BTNode: invalid node kind (%d)\n", srcDesc->kind);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }

        /*
         * Check srcDesc->height.  Don't swap it because it's only one byte.
         */
        if (srcDesc->height > kMaxTreeDepth) {
            LFHFS_LOG(LEVEL_ERROR , "hfs_swap_BTNode: invalid node height (%d)\n", srcDesc->height);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }

        /* Don't swap srcDesc->reserved */

        srcDesc->numRecords    = SWAP_BE16 (srcDesc->numRecords);

        /*
         * Swap the node offsets (including the free space one!).
         */
        srcOffs = (u_int16_t *)((char *)src->buffer + (src->blockSize - ((srcDesc->numRecords + 1) * sizeof (u_int16_t))));

        /*
         * Sanity check that the record offsets are within the node itself.
         */
        if ((char *)srcOffs > ((char *)src->buffer + src->blockSize) ||
            (char *)srcOffs < ((char *)src->buffer + sizeof(BTNodeDescriptor))) {
            LFHFS_LOG(LEVEL_ERROR , "hfs_swap_BTNode: invalid record count (0x%04X)\n", srcDesc->numRecords);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }

        /*
         * Swap and sanity check each of the record offsets.
         */
        for (i = 0; i <= srcDesc->numRecords; i++) {
            srcOffs[i]    = SWAP_BE16 (srcOffs[i]);

            /*
             * Sanity check: must be even, and within the node itself.
             *
             * We may be called to swap an unused node, which contains all zeroes.
             * Unused nodes are expected only when allow_empty_node is true.
             * If it is false and record offset is zero, return error.
             */
            if ((srcOffs[i] & 1) || (
                                     (allow_empty_node == false) && (srcOffs[i] == 0)) ||
                (srcOffs[i] < sizeof(BTNodeDescriptor) && srcOffs[i] != 0) ||
                (srcOffs[i] > (src->blockSize - 2 * (srcDesc->numRecords + 1)))) {
                LFHFS_LOG(LEVEL_ERROR , "hfs_swap_BTNode: offset #%d invalid (0x%04X) (blockSize 0x%x numRecords %d)\n",
                          i, srcOffs[i], (int32_t)src->blockSize, srcDesc->numRecords);
                error = fsBTInvalidHeaderErr;
                goto fail;
            }

            /*
             * Make sure the offsets are strictly increasing.  Note that we're looping over
             * them backwards, hence the order in the comparison.
             */
            if ((i != 0) && (srcOffs[i] >= srcOffs[i-1])) {
                LFHFS_LOG(LEVEL_ERROR , "hfs_swap_BTNode: offsets %d and %d out of order (0x%04X, 0x%04X)\n",
                          i, i-1, srcOffs[i], srcOffs[i-1]);

                error = fsBTInvalidHeaderErr;
                goto fail;
            }
        }
    }

    /*
     * Swap the records (ordered by frequency of access)
     */
    if ((srcDesc->kind == kBTIndexNode) ||
        (srcDesc->kind == kBTLeafNode)) {

        error = hfs_swap_HFSPlusBTInternalNode (src, VTOC(vp)->c_fileid, direction);
        if (error) goto fail;

    } else if (srcDesc-> kind == kBTMapNode) {
        /* Don't swap the bitmaps, they'll be done in the bitmap routines */

    } else if (srcDesc-> kind == kBTHeaderNode) {
        /* The header's offset is hard-wired because we cannot trust the offset pointers. */
        BTHeaderRec *srcHead = (BTHeaderRec *)((char *)src->buffer + sizeof(BTNodeDescriptor));

        srcHead->treeDepth        =    SWAP_BE16 (srcHead->treeDepth);

        srcHead->rootNode        =    SWAP_BE32 (srcHead->rootNode);
        srcHead->leafRecords    =    SWAP_BE32 (srcHead->leafRecords);
        srcHead->firstLeafNode    =    SWAP_BE32 (srcHead->firstLeafNode);
        srcHead->lastLeafNode    =    SWAP_BE32 (srcHead->lastLeafNode);

        srcHead->nodeSize        =    SWAP_BE16 (srcHead->nodeSize);
        srcHead->maxKeyLength    =    SWAP_BE16 (srcHead->maxKeyLength);

        srcHead->totalNodes        =    SWAP_BE32 (srcHead->totalNodes);
        srcHead->freeNodes        =    SWAP_BE32 (srcHead->freeNodes);

        srcHead->clumpSize        =    SWAP_BE32 (srcHead->clumpSize);
        srcHead->attributes        =    SWAP_BE32 (srcHead->attributes);

        /* Don't swap srcHead->reserved1 */
        /* Don't swap srcHead->btreeType; it's only one byte */
        /* Don't swap srcHead->reserved2 */
        /* Don't swap srcHead->reserved3 */
        /* Don't swap bitmap */
    }

    /*
     * If we are doing a swap from in-memory to on-disk, then swap the node
     * descriptor and record offsets after we're done using them.
     */
    if (direction == kSwapBTNodeHostToBig) {
        /*
         * Sanity check and swap the forward and backward links.
         * Ignore the header node since its forward and backwards links can legitimately
         * point to itself.
         */
        if (srcDesc->fLink >= btcb->totalNodes) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: invalid forward link (0x%08X)\n", srcDesc->fLink);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }
        if ((src->blockNum != 0) && (srcDesc->fLink == (u_int32_t) src->blockNum)) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: invalid forward link (0x%08x == 0x%08x)\n",
                      srcDesc->fLink, (u_int32_t) src->blockNum);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }

        if (srcDesc->bLink >= btcb->totalNodes) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: invalid backward link (0x%08X)\n", srcDesc->bLink);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }
        if ((src->blockNum != 0) && (srcDesc->bLink == (u_int32_t) src->blockNum)) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: invalid backward link (0x%08x == 0x%08x)\n",
                      srcDesc->bLink, (u_int32_t) src->blockNum);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }


        srcDesc->fLink        = SWAP_BE32 (srcDesc->fLink);
        srcDesc->bLink        = SWAP_BE32 (srcDesc->bLink);

        /*
         * Check srcDesc->kind.  Don't swap it because it's only one byte.
         */
        if (srcDesc->kind < kBTLeafNode || srcDesc->kind > kBTMapNode) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: invalid node kind (%d)\n", srcDesc->kind);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }

        /*
         * Check srcDesc->height.  Don't swap it because it's only one byte.
         */
        if (srcDesc->height > kMaxTreeDepth) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: invalid node height (%d)\n", srcDesc->height);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }

        /* Don't swap srcDesc->reserved */

        /*
         * Swap the node offsets (including the free space one!).
         */
        srcOffs = (u_int16_t *)((char *)src->buffer + (src->blockSize - ((srcDesc->numRecords + 1) * sizeof (u_int16_t))));

        /*
         * Sanity check that the record offsets are within the node itself.
         */
        if ((char *)srcOffs > ((char *)src->buffer + src->blockSize) ||
            (char *)srcOffs < ((char *)src->buffer + sizeof(BTNodeDescriptor))) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: invalid record count (0x%04X)\n", srcDesc->numRecords);
            error = fsBTInvalidHeaderErr;
            goto fail;
        }

        /*
         * Swap and sanity check each of the record offsets.
         */
        for (i = 0; i <= srcDesc->numRecords; i++) {
            /*
             * Sanity check: must be even, and within the node itself.
             *
             * We may be called to swap an unused node, which contains all zeroes.
             * This can happen when the last record from a node gets deleted.
             * This is why we allow the record offset to be zero.
             * Unused nodes are expected only when allow_empty_node is true
             * (the caller should set it to true for kSwapBTNodeBigToHost).
             */
            if ((srcOffs[i] & 1) ||
                ((allow_empty_node == false) && (srcOffs[i] == 0)) ||
                (srcOffs[i] < sizeof(BTNodeDescriptor) && srcOffs[i] != 0) ||
                (srcOffs[i] > (src->blockSize - 2 * (srcDesc->numRecords + 1)))) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: offset #%d invalid (0x%04X) (blockSize 0x%lx numRecords %d)\n",
                          i, srcOffs[i], src->blockSize, srcDesc->numRecords);
                error = fsBTInvalidHeaderErr;
                goto fail;
            }

            /*
             * Make sure the offsets are strictly increasing.  Note that we're looping over
             * them backwards, hence the order in the comparison.
             */
            if ((i < srcDesc->numRecords) && (srcOffs[i+1] >= srcOffs[i])) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_UNswap_BTNode: offsets %d and %d out of order (0x%04X, 0x%04X)\n",
                          i+1, i, srcOffs[i+1], srcOffs[i]);
                error = fsBTInvalidHeaderErr;
                goto fail;
            }

            srcOffs[i]    = SWAP_BE16 (srcOffs[i]);
        }

        srcDesc->numRecords    = SWAP_BE16 (srcDesc->numRecords);
    }

fail:
    lf_hfs_generic_buf_unlock(psBuf);
    if (error) {
        /*
         * Log some useful information about where the corrupt node is.
         */
        LFHFS_LOG( LEVEL_ERROR, "lf_hfs: node=%lld fileID=%u volume=%s\n", src->blockNum, VTOC(vp)->c_fileid, VTOVCB(vp)->vcbVN);
        hfs_mark_inconsistent(VTOVCB(vp), HFS_INCONSISTENCY_DETECTED);
    }
    #if DEBUG_BTNODE_SWAP
        printf("hfs_swap_BTNode: after: 0x%x, 0x%x, 0x%x, 0x%x\n", pData[0], pData[1], pData[2], pData[3]);
    #endif

    return (error);
}

int
hfs_swap_HFSPlusBTInternalNode (
                                BlockDescriptor *src,
                                HFSCatalogNodeID fileID,
                                enum HFSBTSwapDirection direction
                                )
{
    BTNodeDescriptor *srcDesc = src->buffer;
    u_int16_t *srcOffs = (u_int16_t *)((char *)src->buffer + (src->blockSize - (srcDesc->numRecords * sizeof (u_int16_t))));
    char *nextRecord;    /*  Points to start of record following current one */

    /*
     * i is an int32 because it needs to be negative to index the offset to free space.
     * srcDesc->numRecords is a u_int16_t and is unlikely to become 32-bit so this should be ok.
     */

    int32_t i;
    u_int32_t j;

    if (fileID == kHFSExtentsFileID) {
        HFSPlusExtentKey *srcKey;
        HFSPlusExtentDescriptor *srcRec;
        size_t recordSize;    /* Size of the data part of the record, or node number for index nodes */

        if (srcDesc->kind == kBTIndexNode)
            recordSize = sizeof(u_int32_t);
        else
            recordSize = sizeof(HFSPlusExtentDescriptor);

        for (i = 0; i < srcDesc->numRecords; i++) {
            /* Point to the start of the record we're currently checking. */
            srcKey = (HFSPlusExtentKey *)((char *)src->buffer + srcOffs[i]);

            /*
             * Point to start of next (larger offset) record.  We'll use this
             * to be sure the current record doesn't overflow into the next
             * record.
             */
            nextRecord = (char *)src->buffer + srcOffs[i-1];

            /*
             * Make sure the key and data are within the buffer.  Since both key
             * and data are fixed size, this is relatively easy.  Note that this
             * relies on the keyLength being a constant; we verify the keyLength
             * below.
             */
            if ((char *)srcKey + sizeof(HFSPlusExtentKey) + recordSize > nextRecord) {
                
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: extents key #%d offset too big (0x%04X)\n", srcDesc->numRecords-i-1, srcOffs[i]);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }

            if (direction == kSwapBTNodeBigToHost)
                srcKey->keyLength = SWAP_BE16 (srcKey->keyLength);
            if (srcKey->keyLength != sizeof(*srcKey) - sizeof(srcKey->keyLength)) {
                
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: extents key #%d invalid length (%d)\n", srcDesc->numRecords-i-1, srcKey->keyLength);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }
            srcRec = (HFSPlusExtentDescriptor *)((char *)srcKey + srcKey->keyLength + sizeof(srcKey->keyLength));
            if (direction == kSwapBTNodeHostToBig)
                srcKey->keyLength = SWAP_BE16 (srcKey->keyLength);

            /* Don't swap srcKey->forkType; it's only one byte */
            /* Don't swap srcKey->pad */

            srcKey->fileID            = SWAP_BE32 (srcKey->fileID);
            srcKey->startBlock        = SWAP_BE32 (srcKey->startBlock);

            if (srcDesc->kind == kBTIndexNode) {
                /* For index nodes, the record data is just a child node number. */
                *((u_int32_t *)srcRec) = SWAP_BE32 (*((u_int32_t *)srcRec));
            } else {
                /* Swap the extent data */
                for (j = 0; j < kHFSPlusExtentDensity; j++) {
                    srcRec[j].startBlock    = SWAP_BE32 (srcRec[j].startBlock);
                    srcRec[j].blockCount    = SWAP_BE32 (srcRec[j].blockCount);
                }
            }
        }

    } else if (fileID == kHFSCatalogFileID) {
        HFSPlusCatalogKey *srcKey;
        int16_t *srcPtr;
        u_int16_t keyLength;

        for (i = 0; i < srcDesc->numRecords; i++) {
            /* Point to the start of the record we're currently checking. */
            srcKey = (HFSPlusCatalogKey *)((char *)src->buffer + srcOffs[i]);

            /*
             * Point to start of next (larger offset) record.  We'll use this
             * to be sure the current record doesn't overflow into the next
             * record.
             */
            nextRecord = (char *)src->buffer + (uintptr_t)(srcOffs[i-1]);

            /*
             * Make sure we can safely dereference the keyLength and parentID fields.
             */
            if ((char *)srcKey + offsetof(HFSPlusCatalogKey, nodeName.unicode[0]) > nextRecord) {
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: catalog key #%d offset too big (0x%04X)\n", srcDesc->numRecords-i-1, srcOffs[i]);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }

            /*
             * Swap and sanity check the key length
             */
            if (direction == kSwapBTNodeBigToHost)
                srcKey->keyLength = SWAP_BE16 (srcKey->keyLength);
            keyLength = srcKey->keyLength;    /* Put it in a local (native order) because we use it several times */
            if (direction == kSwapBTNodeHostToBig)
                srcKey->keyLength = SWAP_BE16 (keyLength);

            /* Sanity check the key length */
            if (keyLength < kHFSPlusCatalogKeyMinimumLength || keyLength > kHFSPlusCatalogKeyMaximumLength) {
                
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: catalog key #%d invalid length (%d)\n", srcDesc->numRecords-i-1, keyLength);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }

            /*
             * Make sure that we can safely dereference the record's type field or
             * an index node's child node number.
             */
            srcPtr = (int16_t *)((char *)srcKey + keyLength + sizeof(srcKey->keyLength));
            if ((char *)srcPtr + sizeof(u_int32_t) > nextRecord) {
                
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: catalog key #%d too big\n", srcDesc->numRecords-i-1);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }

            srcKey->parentID                        = SWAP_BE32 (srcKey->parentID);

            /*
             * Swap and sanity check the key's node name
             */
            if (direction == kSwapBTNodeBigToHost)
                srcKey->nodeName.length    = SWAP_BE16 (srcKey->nodeName.length);
            /* Make sure name length is consistent with key length */
            if (keyLength < sizeof(srcKey->parentID) + sizeof(srcKey->nodeName.length) +
                srcKey->nodeName.length*sizeof(srcKey->nodeName.unicode[0])) {
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: catalog record #%d keyLength=%d expected=%lu\n",
                          srcDesc->numRecords-i, keyLength, sizeof(srcKey->parentID) + sizeof(srcKey->nodeName.length) +
                          srcKey->nodeName.length*sizeof(srcKey->nodeName.unicode[0]));
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }
            for (j = 0; j < srcKey->nodeName.length; j++) {
                srcKey->nodeName.unicode[j]    = SWAP_BE16 (srcKey->nodeName.unicode[j]);
            }
            if (direction == kSwapBTNodeHostToBig)
                srcKey->nodeName.length    = SWAP_BE16 (srcKey->nodeName.length);

            /*
             * For index nodes, the record data is just the child's node number.
             * Skip over swapping the various types of catalog record.
             */
            if (srcDesc->kind == kBTIndexNode) {
                *((u_int32_t *)srcPtr) = SWAP_BE32 (*((u_int32_t *)srcPtr));
                continue;
            }

            /* Make sure the recordType is in native order before using it. */
            if (direction == kSwapBTNodeBigToHost)
                srcPtr[0] = SWAP_BE16 (srcPtr[0]);

            if (srcPtr[0] == kHFSPlusFolderRecord) {
                HFSPlusCatalogFolder *srcRec = (HFSPlusCatalogFolder *)srcPtr;
                if ((char *)srcRec + sizeof(*srcRec) > nextRecord) {
                    
                    LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: catalog folder record #%d too big\n", srcDesc->numRecords-i-1);
                    if (direction == kSwapBTNodeHostToBig) {
                        hfs_assert(0);
                    }
                    return fsBTInvalidNodeErr;
                }

                srcRec->flags                = SWAP_BE16 (srcRec->flags);
                srcRec->valence                = SWAP_BE32 (srcRec->valence);
                srcRec->folderID            = SWAP_BE32 (srcRec->folderID);
                srcRec->createDate            = SWAP_BE32 (srcRec->createDate);
                srcRec->contentModDate        = SWAP_BE32 (srcRec->contentModDate);
                srcRec->attributeModDate    = SWAP_BE32 (srcRec->attributeModDate);
                srcRec->accessDate            = SWAP_BE32 (srcRec->accessDate);
                srcRec->backupDate            = SWAP_BE32 (srcRec->backupDate);

                srcRec->bsdInfo.ownerID        = SWAP_BE32 (srcRec->bsdInfo.ownerID);
                srcRec->bsdInfo.groupID        = SWAP_BE32 (srcRec->bsdInfo.groupID);

                /* Don't swap srcRec->bsdInfo.adminFlags; it's only one byte */
                /* Don't swap srcRec->bsdInfo.ownerFlags; it's only one byte */

                srcRec->bsdInfo.fileMode            = SWAP_BE16 (srcRec->bsdInfo.fileMode);
                srcRec->bsdInfo.special.iNodeNum    = SWAP_BE32 (srcRec->bsdInfo.special.iNodeNum);

                srcRec->textEncoding        = SWAP_BE32 (srcRec->textEncoding);

                /* Don't swap srcRec->userInfo */
                /* Don't swap srcRec->finderInfo */
                srcRec->folderCount = SWAP_BE32 (srcRec->folderCount);

            } else if (srcPtr[0] == kHFSPlusFileRecord) {
                HFSPlusCatalogFile *srcRec = (HFSPlusCatalogFile *)srcPtr;
                if ((char *)srcRec + sizeof(*srcRec) > nextRecord) {
                    
                    LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: catalog file record #%d too big\n", srcDesc->numRecords-i-1);
                    if (direction == kSwapBTNodeHostToBig) {
                        hfs_assert(0);
                    }
                    return fsBTInvalidNodeErr;
                }

                srcRec->flags                = SWAP_BE16 (srcRec->flags);

                srcRec->fileID               = SWAP_BE32 (srcRec->fileID);

                srcRec->createDate           = SWAP_BE32 (srcRec->createDate);
                srcRec->contentModDate       = SWAP_BE32 (srcRec->contentModDate);
                srcRec->attributeModDate     = SWAP_BE32 (srcRec->attributeModDate);
                srcRec->accessDate           = SWAP_BE32 (srcRec->accessDate);
                srcRec->backupDate           = SWAP_BE32 (srcRec->backupDate);

                srcRec->bsdInfo.ownerID      = SWAP_BE32 (srcRec->bsdInfo.ownerID);
                srcRec->bsdInfo.groupID      = SWAP_BE32 (srcRec->bsdInfo.groupID);

                /* Don't swap srcRec->bsdInfo.adminFlags; it's only one byte */
                /* Don't swap srcRec->bsdInfo.ownerFlags; it's only one byte */

                srcRec->bsdInfo.fileMode            = SWAP_BE16 (srcRec->bsdInfo.fileMode);
                srcRec->bsdInfo.special.iNodeNum    = SWAP_BE32 (srcRec->bsdInfo.special.iNodeNum);

                srcRec->textEncoding        = SWAP_BE32 (srcRec->textEncoding);

                /* If kHFSHasLinkChainBit is set, reserved1 is hl_FirstLinkID.
                 * In all other context, it is expected to be zero.
                 */
                srcRec->reserved1 = SWAP_BE32 (srcRec->reserved1);

                /* Don't swap srcRec->userInfo */
                /* Don't swap srcRec->finderInfo */
                /* Don't swap srcRec->reserved2 */

                hfs_swap_HFSPlusForkData (&srcRec->dataFork);
                hfs_swap_HFSPlusForkData (&srcRec->resourceFork);

            } else if ((srcPtr[0] == kHFSPlusFolderThreadRecord) ||
                       (srcPtr[0] == kHFSPlusFileThreadRecord)) {

                /*
                 * Make sure there is room for parentID and name length.
                 */
                HFSPlusCatalogThread *srcRec = (HFSPlusCatalogThread *)srcPtr;
                if ((char *) &srcRec->nodeName.unicode[0] > nextRecord) {
                    LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: catalog thread record #%d too big\n", srcDesc->numRecords-i-1);
                    if (direction == kSwapBTNodeHostToBig) {
                        hfs_assert(0);
                    }
                    return fsBTInvalidNodeErr;
                }

                /* Don't swap srcRec->reserved */

                srcRec->parentID                        = SWAP_BE32 (srcRec->parentID);

                if (direction == kSwapBTNodeBigToHost)
                    srcRec->nodeName.length    = SWAP_BE16 (srcRec->nodeName.length);

                /*
                 * Make sure there is room for the name in the buffer.
                 * Then swap the characters of the name itself.
                 */
                if ((char *) &srcRec->nodeName.unicode[srcRec->nodeName.length] > nextRecord) {
                    LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: catalog thread record #%d name too big\n", srcDesc->numRecords-i-1);
                    if (direction == kSwapBTNodeHostToBig) {
                        hfs_assert(0);
                    }
                    return fsBTInvalidNodeErr;
                }
                for (j = 0; j < srcRec->nodeName.length; j++) {
                    srcRec->nodeName.unicode[j]    = SWAP_BE16 (srcRec->nodeName.unicode[j]);
                }

                if (direction == kSwapBTNodeHostToBig)
                    srcRec->nodeName.length = SWAP_BE16 (srcRec->nodeName.length);

            } else {
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: unrecognized catalog record type (0x%04X; record #%d)\n", srcPtr[0], srcDesc->numRecords-i-1);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }

            /* We can swap the record type now that we're done using it. */
            if (direction == kSwapBTNodeHostToBig)
                srcPtr[0] = SWAP_BE16 (srcPtr[0]);
        }

    } else if (fileID == kHFSAttributesFileID) {
        HFSPlusAttrKey *srcKey;
        HFSPlusAttrRecord *srcRec;
        u_int16_t keyLength;
        u_int32_t attrSize = 0;

        for (i = 0; i < srcDesc->numRecords; i++) {
            /* Point to the start of the record we're currently checking. */
            srcKey = (HFSPlusAttrKey *)((char *)src->buffer + srcOffs[i]);

            /*
             * Point to start of next (larger offset) record.  We'll use this
             * to be sure the current record doesn't overflow into the next
             * record.
             */
            nextRecord = (char *)src->buffer + srcOffs[i-1];

            /* Make sure there is room in the buffer for a minimal key */
            if ((char *) &srcKey->attrName[1] > nextRecord) {
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: attr key #%d offset too big (0x%04X)\n", srcDesc->numRecords-i-1, srcOffs[i]);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }

            /* Swap the key length field */
            if (direction == kSwapBTNodeBigToHost)
                srcKey->keyLength = SWAP_BE16(srcKey->keyLength);
            keyLength = srcKey->keyLength;    /* Keep a copy in native order */
            if (direction == kSwapBTNodeHostToBig)
                srcKey->keyLength = SWAP_BE16(srcKey->keyLength);

            /*
             * Make sure that we can safely dereference the record's type field or
             * an index node's child node number.
             */
            srcRec = (HFSPlusAttrRecord *)((char *)srcKey + keyLength + sizeof(srcKey->keyLength));
            if ((char *)srcRec + sizeof(u_int32_t) > nextRecord) {
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: attr key #%d too big (%d)\n", srcDesc->numRecords-i-1, keyLength);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }

            srcKey->fileID = SWAP_BE32(srcKey->fileID);
            srcKey->startBlock = SWAP_BE32(srcKey->startBlock);

            /*
             * Swap and check the attribute name
             */
            if (direction == kSwapBTNodeBigToHost)
                srcKey->attrNameLen = SWAP_BE16(srcKey->attrNameLen);
            /* Sanity check the attribute name length */
            if (srcKey->attrNameLen > kHFSMaxAttrNameLen || keyLength < (kHFSPlusAttrKeyMinimumLength + sizeof(u_int16_t)*srcKey->attrNameLen)) {
                
                LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: attr key #%d keyLength=%d attrNameLen=%d\n", srcDesc->numRecords-i-1, keyLength, srcKey->attrNameLen);
                if (direction == kSwapBTNodeHostToBig) {
                    hfs_assert(0);
                }
                return fsBTInvalidNodeErr;
            }
            for (j = 0; j < srcKey->attrNameLen; j++)
                srcKey->attrName[j] = SWAP_BE16(srcKey->attrName[j]);
            if (direction == kSwapBTNodeHostToBig)
                srcKey->attrNameLen = SWAP_BE16(srcKey->attrNameLen);

            /*
             * For index nodes, the record data is just the child's node number.
             * Skip over swapping the various types of attribute record.
             */
            if (srcDesc->kind == kBTIndexNode) {
                *((u_int32_t *)srcRec) = SWAP_BE32 (*((u_int32_t *)srcRec));
                continue;
            }

            /* Swap the record data */
            if (direction == kSwapBTNodeBigToHost)
                srcRec->recordType = SWAP_BE32(srcRec->recordType);
            switch (srcRec->recordType) {
                case kHFSPlusAttrInlineData:
                    /* Is there room for the inline data header? */
                    if ((char *) &srcRec->attrData.attrData[0]  > nextRecord) {
                        
                        LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: attr inline #%d too big\n", srcDesc->numRecords-i-1);
                        if (direction == kSwapBTNodeHostToBig) {
                            hfs_assert(0);
                        }
                        return fsBTInvalidNodeErr;
                    }

                    /* We're not swapping the reserved fields */

                    /* Swap the attribute size */
                    if (direction == kSwapBTNodeHostToBig)
                        attrSize = srcRec->attrData.attrSize;
                    srcRec->attrData.attrSize = SWAP_BE32(srcRec->attrData.attrSize);
                    if (direction == kSwapBTNodeBigToHost)
                        attrSize = srcRec->attrData.attrSize;

                    /* Is there room for the inline attribute data? */
                    if ((char *) &srcRec->attrData.attrData[attrSize] > nextRecord) {
                        LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: attr inline #%d too big (attrSize=%u)\n", srcDesc->numRecords-i-1, attrSize);
                        if (direction == kSwapBTNodeHostToBig) {
                            hfs_assert(0);
                        }
                        return fsBTInvalidNodeErr;
                    }

                    /* Not swapping the attribute data itself */
                    break;

                case kHFSPlusAttrForkData:
                    /* Is there room for the fork data record? */
                    if ((char *)srcRec + sizeof(HFSPlusAttrForkData) > nextRecord) {
                        LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: attr fork data #%d too big\n", srcDesc->numRecords-i-1);
                        if (direction == kSwapBTNodeHostToBig) {
                            hfs_assert(0);
                        }
                        return fsBTInvalidNodeErr;
                    }

                    /* We're not swapping the reserved field */

                    hfs_swap_HFSPlusForkData(&srcRec->forkData.theFork);
                    break;

                case kHFSPlusAttrExtents:
                    /* Is there room for an extent record? */
                    if ((char *)srcRec + sizeof(HFSPlusAttrExtents) > nextRecord) {
                        LFHFS_LOG((direction == kSwapBTNodeHostToBig) ? LEVEL_ERROR : LEVEL_DEBUG, "hfs_swap_HFSPlusBTInternalNode: attr extents #%d too big\n", srcDesc->numRecords-i-1);
                        if (direction == kSwapBTNodeHostToBig) {
                            hfs_assert(0);
                        }
                        return fsBTInvalidNodeErr;
                    }

                    /* We're not swapping the reserved field */

                    for (j = 0; j < kHFSPlusExtentDensity; j++) {
                        srcRec->overflowExtents.extents[j].startBlock =
                        SWAP_BE32(srcRec->overflowExtents.extents[j].startBlock);
                        srcRec->overflowExtents.extents[j].blockCount =
                        SWAP_BE32(srcRec->overflowExtents.extents[j].blockCount);
                    }
                    break;
            }
            if (direction == kSwapBTNodeHostToBig)
                srcRec->recordType = SWAP_BE32(srcRec->recordType);
        }
    }
    else {
        LFHFS_LOG(LEVEL_ERROR, "hfs_swap_HFSPlusBTInternalNode: fileID %u is not a system B-tree\n", fileID);
        hfs_assert(0);
    }


    return (0);
}

