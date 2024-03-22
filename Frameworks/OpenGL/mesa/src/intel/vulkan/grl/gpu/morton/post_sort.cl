//
// Copyright (C) 2009-2022 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "libs/lsc_intrinsics.h"
#include "morton/morton_common.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
/*

    This kernel constructs a binary hierarchy in bottom up fashion from
    the morton codes.

*/
////////////////////////////////////////////////////////////////////////////////////////////////////////

int Delta(global struct MortonCodePrimitive* mc, const uint64_t key0, const uint i1 )
{
    const uint64_t key1 = mc[i1].index_code;
    return  clz(key0 ^ key1);
}

int sign( int d )
{
    return (d > 0) ? 1 : -1;
}

__attribute__( (reqd_work_group_size( MAX_HW_SIMD_WIDTH, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( MAX_HW_SIMD_WIDTH )) )
void kernel build_bottom_up_indirect( global struct Globals* globals,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global struct MortonCodePrimitive* mc )
{
    /* construct range of primitives that each work group will process */
    const uint numPrimitives = globals->numPrimitives;

    uint i = get_group_id( 0 ) * get_local_size(0) + get_local_id( 0 );

    if (i == 0)
    {
        globals->binary_hierarchy_root = 0;
        if (numPrimitives == 1)
        {
            // special kludge for 1-prim tree.  Make sure the one leaf node is initialized
            bnodes[i].range.start   = 0;
            bnodes[i].range.end     = 0;
            bnodes[i].leftChild     = -1;
            bnodes[i].rightChild    = -1;
        }

        // store pointer to the binary hierarchy in the globals struct.
        //  This will be used
        globals->binary_hierarchy_buffer = (gpuva_t) bnodes;
    }

    uint num_inner_nodes = numPrimitives-1;
    if ( i < num_inner_nodes )
    {
        //
        // direction is 1 if this morton code is the node's first key, -1 if it's the last
        //    By construction every internal node is either the start or the end of a given key range
        //  direction should be towards the neighbor with the most bits in common

        uint64_t ki = mc[i].index_code;

        int direction, delta_min;
        uint lmax;
        if( i == 0 )
        {
            direction = 1;
            delta_min = -1;
            lmax = numPrimitives;
        }
        else
        {
            direction = sign( Delta( mc, ki, i + 1 ) - Delta( mc,  ki, i - 1 ) );
            delta_min = Delta( mc,  ki, i - direction );

            // find upper bound for length of this node's key range
            lmax = 8;
            while ( (i+lmax*direction) < numPrimitives && Delta( mc, ki, i+lmax*direction ) > delta_min)
                lmax = lmax * 2;
        }

        // clamp max length so that the binary searches are fully in-bounds
        uint maxLen = (direction>0) ? (numPrimitives - i) : (i+1);
        lmax = min(lmax, maxLen);

        // find end of range using binary search
        uint length = 0;
        uint end    = lmax-1;
        while (length != end)
        {
            uint mid = length + ((end-length)/2) + ((end-length)%2);
            bool bigger =  Delta( mc, ki, i+mid*direction) > delta_min;
            length = bigger ? mid : length;
            end    = bigger ? end : mid-1;
        }
        uint j = i + length*direction ;

        // find split position using binary search
        uint split = 0;
        end    = length-1;
        int delta_node = Delta(mc, ki, j);
        while (split != end)
        {
            uint mid = split + ((end-split)/2) + ((end-split)%2);
            bool bigger =  Delta( mc, ki, i+mid*direction) > delta_node;
            split = bigger ? mid : split;
            end   = bigger ? end : mid-1;
        }
        split = i + split*direction + min(direction,0);

        uint left  = split;
        uint right = split+1;

        // mark leaves
        if( min(i,j) == split )
            left = left | (1<<31);
        if( max(i,j) == split+1 )
            right = right | (1<<31);

        bnodes[i].range.start = min(i,j);
        bnodes[i].range.end   = max(i,j);
        bnodes[i].leftChild   = left;
        bnodes[i].rightChild  = right;
    }
}





#if 0
__attribute__( (reqd_work_group_size( MAX_HW_SIMD_WIDTH, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( MAX_HW_SIMD_WIDTH )) )
void kernel build_bottom_up_indirect( global struct Globals* globals,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global struct MortonCodePrimitive* mc )
{
    /* construct range of primitives that each work group will process */
    const uint numPrimitives = globals->numPrimitives;

    // RangeFactor determines the distance between adjacent nodeIds in work group.
    // The aim of the nodes distribution within work group, for rangeFactor > 1
    // is to be sure that half of the work groups will entirelly be dropped off
    // at the bottom layer of the graph. This way the EUs can be reused faster.
    // The factor needs to be smaller than MAX_HW_SIMD_WIDTH
    const uint rangeFactor = 2;

    const uint numGroups = ((numPrimitives + MAX_HW_SIMD_WIDTH - 1) / MAX_HW_SIMD_WIDTH);
    const uint globalId = get_group_id( 0 ) * MAX_HW_SIMD_WIDTH + get_local_id( 0 );
    const uint numPrimitivesAlignedToWGSize = MAX_HW_SIMD_WIDTH * numGroups;
    const uint groupsRange = numPrimitivesAlignedToWGSize / rangeFactor;

    /* iterate over all primitives the work group should process */
    const uint i = (globalId * rangeFactor) % numPrimitivesAlignedToWGSize + globalId / groupsRange;

    if ( i < numPrimitives )
    {
        uint node = i | ((uint)1 << 31);
        uint start = i;
        uint end = i;

        /* bottom up */
        while ( true )
        {
            /* goto parent node and link parent node to current node */
            node = updateParent( bnodes, mc, node, start, end, numPrimitives - 1 );

            /* do not continue if we reached this node the first time */
            if ( node == -1 )
                break;

            mem_fence_gpu_invalidate();

            /* update range */
            start = bnodes[node].range.start;
            end = bnodes[node].range.end;

            /* stop when we reached the root node */
            if ( start == 0 && end == numPrimitives - 1 )
            {
                globals->binary_hierarchy_root = node;
                break;
            }
        }
    }
}

#endif

/*

  This function builds one QBVH6 node by opening the provided binary
  BVH nodes until the QBVH node is full.

 */

GRL_INLINE void create_node(global struct Globals *globals,
                        global struct BinaryMortonCodeHierarchy *bnodes,
                        global char *bvh_mem,
                        uint rID,
                        local uint *local_numRecords,
                        local uint *local_QNodeOffset,
                        struct BuildRecordMorton *records,
                        struct BuildRecordMorton *current)
{
    global struct BVHBase *bvh = (global struct BVHBase *)bvh_mem;
    const uint rootNodeOffset = BVH_ROOT_NODE_OFFSET;
    global struct QBVHNodeN *nodeData = BVHBase_nodeData(bvh);
    BackPointers *backPointers = BVHBase_GetBackPointers(bvh);

    /* initialize child array */
    uint numChildren = 2;
    struct BuildRecordMorton children[BVH_NODE_N6];
    children[0].nodeID = bnodes[current->nodeID].leftChild;
    children[0].items = BinaryMortonCodeHierarchy_getNumPrimitives(bnodes, children[0].nodeID);
    children[1].nodeID = bnodes[current->nodeID].rightChild;
    children[1].items = BinaryMortonCodeHierarchy_getNumPrimitives(bnodes, children[1].nodeID);

    /* fill QBVH6 node with up to 6 children */
    while (numChildren < BVH_NODE_N6)
    {
        /*! find best child to split */
        uint bestItems = 0;
        int bestChild = -1;
        for (int i = 0; i < numChildren; i++)
        {
            const uint items = children[i].items;

            /* ignore leaves as they cannot get split */
            if (items <= cfg_minLeafSize)
                continue;

            /* find child with largest number of items */
            if (items > bestItems)
            {
                bestItems = items;
                bestChild = i;
            }
        }
        if (bestChild == -1)
            break;

        /* perform best found split */
        const uint bestNodeID = children[bestChild].nodeID;
        struct BuildRecordMorton *lrecord = &children[bestChild];
        struct BuildRecordMorton *rrecord = &children[numChildren];
        lrecord->nodeID = bnodes[bestNodeID].leftChild;
        lrecord->items = BinaryMortonCodeHierarchy_getNumPrimitives(bnodes, lrecord->nodeID);
        rrecord->nodeID = bnodes[bestNodeID].rightChild;
        rrecord->items = BinaryMortonCodeHierarchy_getNumPrimitives(bnodes, rrecord->nodeID);
        numChildren++;
    }

    /* allocate memory for all children */
    const uint child_node_offset = atomic_add_local(local_QNodeOffset,64*numChildren);
    global struct QBVHNodeN *childNodes = (global struct QBVHNodeN *)(bvh_mem + child_node_offset);

    /* create node, but to not set bounds yet as these get calculated during refit */
    const uint current_index = current->current_index;
    struct QBVHNodeN *qnode = nodeData + current_index;
    QBVH6Node_set_type(qnode, BVH_INTERNAL_NODE);
    QBVHNodeN_setChildIncr1(qnode);
    QBVH6Node_set_offset(qnode, childNodes);

    /* set back pointers */
    *InnerNode_GetBackPointer(backPointers,  current_index) = (current->parent_index << 6) | (numChildren << 3);

    /* update parent pointer of build records of all children */
    for (uint ID = 0; ID < numChildren; ID++)
    {
        children[ID].current_index = childNodes - nodeData + ID;
        children[ID].parent_index = current_index;
    }

    /* write out child build records */
    const uint global_offset = atomic_add_local(local_numRecords, numChildren - 1);
    records[rID] = children[0];

    for (uint i = 1; i < numChildren; i++)
        records[global_offset + i - 1] = children[i];

    mem_fence_workgroup_default();

}

#if 0
/* This function calculates the similarity between two morton
 * codes. It essentially counts how many bits of the morton codes are
 * equal starting at the top. The more bits are equal, the similar the
 * codes, and the closer the primitives are located spatially. */

GRL_INLINE uint64_t delta(global struct MortonCodePrimitive *mc,
                      const uint id)
{
    const uint64_t key0 = mc[id + 0].index_code;
    const uint64_t key1 = mc[id + 1].index_code;
    return clz(key0 ^ key1);
}



/* This function checks for a range [left,right] of morton codes, if
 * it is spatially closer to the left or to the right nodes. */

GRL_INLINE bool merge_to_right(global struct MortonCodePrimitive *mc,
                           const uint left,
                           const uint right,
                           const uint last)
{
    /* merge to right if we are at the left end of the array */
    if (left == 0)
        return true;

    /* merge to left if we are at the right end of the array */
    if (right == last)
        return false;

    /* otherwise merge to the side where the morton code sequence has
   * the largest number of equal bits from the top */
    return delta(mc, right) > delta(mc, left - 1);
}

GRL_INLINE uint updateParent(global struct BinaryMortonCodeHierarchy *bnodes,
                         global struct MortonCodePrimitive *mc,
                         const uint nodeID,
                         const uint left,
                         const uint right,
                         const uint last)
{
    uint parent;

    /* check if we should merge this node to the left or right */
    if (merge_to_right(mc, left, right, last))
    {
        parent = right;
        bnodes[parent].leftChild = nodeID;
        bnodes[parent].range.start = left;
    }
    else
    {
        parent = left - 1;
        bnodes[parent].rightChild = nodeID;
        bnodes[parent].range.end = right;
    }

    mem_fence_gpu_default();

    /* stop ascending the tree if we reached this node the first time */
    const bool first = atomic_inc_global((global uint *)&bnodes[parent].flag) == 0;
    return first ? -1 : parent;
}

GRL_INLINE void
DO_OLD_PARALLEL_BUILD_PHASE1( global struct Globals* globals,
    global struct MortonCodePrimitive* mc,
    global struct AABB* primref,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global char* bvh_mem,
    uint startID, uint endID,
    local uint* local_numRecords,
    local uint* local_numRecordsOld,
    local struct BuildRecordMorton* local_records
)
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    global struct BuildRecordMorton* records = (global struct BuildRecordMorton*)(bvh_mem + bvh->quadLeafStart*64);

    /* iterate over all subtrees this workgroup should build */
    for ( uint recordID = startID; recordID < endID; recordID++ )
    {
        /* add start build record to local stack */
        if ( get_local_id( 0 ) == 0 )
        {
            local_records[0] = records[recordID];
            *local_numRecords = 1;
            *local_numRecordsOld = 0;
        }
        work_group_barrier( CLK_LOCAL_MEM_FENCE );

        /* terminate when all subtrees are leaves */
        while ( *local_numRecords != *local_numRecordsOld )
        {
            /* remember the old number of build records to detect later
       * whether we are done */
            if ( get_local_id( 0 ) == 0 )
            {
                *local_numRecordsOld = *local_numRecords;
            }
            work_group_barrier( CLK_LOCAL_MEM_FENCE );

            /* all work items in the sub group pick a subtree to build */
            for ( uint ID = get_local_id( 0 ); ID < *local_numRecordsOld; ID += get_local_size( 0 ) )
            {
                /* ignore small subtrees */
                if ( local_records[ID].items <= BVH_NODE_N6 )
                    continue;

                /* create QBVH node */
                create_node( globals, bnodes, bvh_mem, ID, local_numRecords, local_records, &local_records[ID] );
            }

            /* wait for all work items to have updated local_records array */
            work_group_barrier( CLK_LOCAL_MEM_FENCE );
        }

        const uint shift_mask = globals->shift_mask;
        const uint leafPrimType = globals->leafPrimType;
        const uint rootNodeOffset = BVH_ROOT_NODE_OFFSET;
        BackPointers* backPointers = BVHBase_GetBackPointers( bvh );
        global struct QBVHNodeN* nodeData = BVHBase_nodeData( bvh );

        /* create all fat leaf nodes and initiate refit */
        for ( uint ID = get_local_id( 0 ); ID < *local_numRecords; ID += get_local_size( 0 ) )
        {
            struct BuildRecordMorton current = local_records[ID];
            const uint primrefID = BinaryMortonCodeHierarchy_getRangeStart( bnodes, current.nodeID );

            global struct QBVHNodeN* qnode = nodeData + current.current_index;

            /* get bounds of all children of the fat leaf node */
            struct AABB bounds[BVH_NODE_N6];
            for ( uint i = 0; i < current.items; i++ )
            {
                /* get primID and bounds of primitive */
                const uint primID = (uint)(mc[primrefID + i].index_code & shift_mask);
                bounds[i] = primref[primID];

                /* For all primitives in a fat leaf we store a back
                 * pointer. This way we can modify the fat leaf node at leaf construction time. */
                const uint back_pointer = qnode - (struct QBVHNodeN*)bvh_mem;

                /* Store back pointer and primID inside morton code array to
                 * be later used by leaf creation. */
                mc[primrefID + i].index_code = ((ulong)back_pointer) << 32 | (ulong)primID;
            }

            /* update fat leaf node */
            QBVHNodeN_setType( qnode, leafPrimType );
            global void* offset;
            if ( leafPrimType != BVH_INSTANCE_NODE )
            {
                offset = bvh_mem + 64*bvh->quadLeafStart + primrefID * sizeof( struct Quad );
                QBVHNodeN_setChildIncr1( qnode );
            }
            else
            {
                offset = bvh_mem + 64*bvh->instanceLeafStart + primrefID * sizeof( struct HwInstanceLeaf );
                QBVHNodeN_setChildIncr2( qnode );
            }
            QBVH6Node_set_offset( qnode, offset );
            QBVHNodeN_setBounds( qnode, bounds, current.items );

            /* set back pointers for fat leaf nodes */
            *InnerNode_GetBackPointer(backPointers, current.current_index) = (current.parent_index << 6) | (current.items << 3);

            /* bottom up refit */
            refit_bottom_up( qnode, bvh, bounds, current.items );
        }
    }
}

/*

  This phase takes the build records calculated in phase0 as input and
  finished the BVH construction for all these subtrees.

*/
__attribute__((reqd_work_group_size(8, 1, 1)))
old_parallel_build_phase1(global struct Globals *globals,
                      global struct MortonCodePrimitive *mc,
                      global struct AABB *primref,
                      global struct BinaryMortonCodeHierarchy *bnodes,
                      global char *bvh_mem)
{
    global struct BVHBase *bvh = (global struct BVHBase *)bvh_mem;
    global struct BuildRecordMorton *records = (global struct BuildRecordMorton *)(bvh_mem + 64*bvh->quadLeafStart);

    /* a queue of build records */
    local struct BuildRecordMorton local_records[MORTON_BUILDER_SUBTREE_THRESHOLD];
    local uint local_numRecords;
    local uint local_numRecordsOld;

    /* construct range of build records that each sub group will process */
    const uint numRecords = globals->numBuildRecords;
    const uint startID = (get_group_id(0) + 0) * numRecords / get_num_groups(0);
    const uint endID = (get_group_id(0) + 1) * numRecords / get_num_groups(0);

    DO_OLD_PARALLEL_BUILD_PHASE1( globals, mc, primref, bnodes, bvh_mem, startID, endID, &local_numRecords, &local_numRecordsOld, local_records );

}

__attribute__( (reqd_work_group_size( 8, 1, 1 )) )
old_parallel_build_phase1_Indirect( global struct Globals* globals,
    global struct MortonCodePrimitive* mc,
    global struct AABB* primref,
    global struct BinaryMortonCodeHierarchy* bnodes,
    global char* bvh_mem )
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    global struct BuildRecordMorton* records = (global struct BuildRecordMorton*)(bvh_mem + 64*bvh->quadLeafStart);

    /* a queue of build records */
    local struct BuildRecordMorton local_records[MORTON_BUILDER_SUBTREE_THRESHOLD];
    local uint local_numRecords;
    local uint local_numRecordsOld;

    /* construct range of build records that each sub group will process */
    const uint numRecords = globals->numBuildRecords;
    uint startID = get_group_id( 0 );
    uint endID   = startID + 1;

    DO_OLD_PARALLEL_BUILD_PHASE1( globals, mc, primref, bnodes, bvh_mem, startID, endID, &local_numRecords, &local_numRecordsOld, local_records );

}
#endif
