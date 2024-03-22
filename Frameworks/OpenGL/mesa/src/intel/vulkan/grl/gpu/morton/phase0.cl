//
// Copyright (C) 2009-2022 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "libs/lsc_intrinsics.h"
#include "morton/morton_common.h"

GRL_INLINE void SUBGROUP_create_node_phase0(
    uniform global struct Globals* globals,
    uniform global struct BinaryMortonCodeHierarchy* bnodes,
    uniform global char* bvh_mem,
    uniform global uint *global_refit_startpoints,
    uniform uint rID,
    uniform local uint* local_numRecords,
    uniform local uint* local_QNodeOffset,
    uniform global struct BuildRecordMorton* records,
    uniform struct BuildRecordMorton current,
    uniform local uint* local_startpoints_num)
{
    uniform global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    uniform const uint rootNodeOffset = BVH_ROOT_NODE_OFFSET;
    uniform global struct QBVHNodeN* nodeData = BVHBase_nodeData( bvh );
    uniform BackPointers* backPointers = BVHBase_GetBackPointers( bvh );

    varying ushort lane = get_sub_group_local_id();

    /* initialize child array */
    uniform uint numChildren = 2;
    varying struct BuildRecordMorton sg_children;
    sg_children.items = 0;
    sg_children.nodeID = (lane == 0) ? bnodes[current.nodeID].leftChild : bnodes[current.nodeID].rightChild;

    if ( lane < numChildren )
        sg_children.items = BinaryMortonCodeHierarchy_getNumPrimitives( bnodes, sg_children.nodeID );

    /* fill QBVH6 node with up to 6 children */
    while ( numChildren < BVH_NODE_N6 )
    {
        varying bool sg_is_leaf = sg_children.items <= cfg_minLeafSize;
        if ( sub_group_all( sg_is_leaf ) )
            break;

        uniform uint bestItems = sub_group_reduce_max_N6( sg_children.items );
        uniform ushort bestChild = ctz( intel_sub_group_ballot( sg_children.items == bestItems ) );
        uniform uint bestNodeID = sub_group_broadcast( sg_children.nodeID, bestChild );

        varying uint nodeID = (lane == bestChild) ? bnodes[bestNodeID].leftChild : bnodes[bestNodeID].rightChild;

        if ( lane == numChildren || lane == bestChild )
        {
            sg_children.nodeID = nodeID;
            sg_children.items = BinaryMortonCodeHierarchy_getNumPrimitives( bnodes, nodeID );
        }

        numChildren++;
    }

    const uint current_index = current.current_index;
    struct QBVHNodeN* qnode = nodeData + current_index;
    SUBGROUP_QBVHNodeN_setChildIncr1( qnode );

    uniform uint global_offset;
    uniform uint child_node_offset;

    // Check if all children will be roots for the local subgtrees in phase1. If so we keep the node ids to be later
    // used in global refit after phase1
    varying uchar is_children_root = (lane < numChildren) ? (sg_children.items <= MORTON_BUILDER_SUBTREE_THRESHOLD) : 0;
    uniform uchar children_roots_num = sub_group_reduce_add(is_children_root);

    if ( lane == 0 )
    {
        child_node_offset = atomic_add_local(local_QNodeOffset,64*numChildren);

        /* create node, but to not set bounds yet as these get calculated during refit */
        QBVH6Node_set_type( qnode, BVH_INTERNAL_NODE );
        QBVH6Node_set_offset( qnode, (global struct QBVHNodeN*)(bvh_mem + child_node_offset) );
        /* set back pointers */
        uint backpointer = (current.parent_index << 6) | (numChildren << 3);

        global_offset = atomic_add_local( local_numRecords, numChildren - 1 );

#if MORTON_VERBOSE_LOG
        printf("PHASE0: loc_id: %d, index: %d, first_child_id: %d, offset: %d, parent: %d, numChildren: %d\n",
               rID, current_index, current_index + qnode->offset, qnode->offset, current.parent_index, numChildren);
#endif

        if(children_roots_num == numChildren)
        {
            uint startpoints_offset = atomic_inc_local( local_startpoints_num );
            global_refit_startpoints[startpoints_offset] = current_index;
        }
        else
        {
            backpointer += children_roots_num;
        }

        *InnerNode_GetBackPointer(backPointers, current_index) = backpointer;
    }

    child_node_offset = sub_group_broadcast( child_node_offset, 0 );
    global_offset = sub_group_broadcast( global_offset, 0 );

    uniform global struct QBVHNodeN* childNodes = (global struct QBVHNodeN*)(bvh_mem + child_node_offset);

    sg_children.current_index = childNodes - nodeData + lane;
    sg_children.parent_index = current_index;

    if ( lane < numChildren )
    {
        uint write_position = (lane == 0) ? rID : global_offset + lane - 1;
        records[write_position] = sg_children;
    }
}


GRL_INLINE void SUBGROUP_create_node_phase0_local_sync(
    uniform global struct Globals* globals,
    uniform global struct BinaryMortonCodeHierarchy* bnodes,
    uniform global char* bvh_mem,
    uniform uint rID,
    uniform local uint* local_numRecords,
    uniform local uint* local_QNodeOffset,
    uniform global struct BuildRecordMorton* records,
    uniform struct BuildRecordMorton current,
    uniform local uint* local_p0_total,
    uniform global struct MortonFlattenedBoxlessNode *boxless_nodes,
    uniform uint nodeDataStart)
{
    uniform global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    uniform const uint rootNodeOffset = bvh->rootNodeOffset;
    uniform global struct QBVHNodeN* nodeData = BVHBase_nodeData( bvh );
    uniform BackPointers* backPointers = BVHBase_GetBackPointers( bvh );

    varying ushort lane = get_sub_group_local_id();

    /* initialize child array */
    uniform uint numChildren = 2;
    varying struct BuildRecordMorton sg_children;
    sg_children.items = 0;
    sg_children.nodeID = (lane == 0) ? bnodes[current.nodeID].leftChild : bnodes[current.nodeID].rightChild;

    if ( lane < numChildren )
        sg_children.items = BinaryMortonCodeHierarchy_getNumPrimitives( bnodes, sg_children.nodeID );

    /* fill QBVH6 node with up to 6 children */
    while ( numChildren < BVH_NODE_N6 )
    {
        varying bool sg_is_leaf = sg_children.items <= cfg_minLeafSize;
        if ( sub_group_all( sg_is_leaf ) )
            break;

        uniform uint bestItems = sub_group_reduce_max_N6( sg_children.items );
        uniform ushort bestChild = ctz( intel_sub_group_ballot( sg_children.items == bestItems ) );
        uniform uint bestNodeID = sub_group_broadcast( sg_children.nodeID, bestChild );

        varying uint nodeID = (lane == bestChild) ? bnodes[bestNodeID].leftChild : bnodes[bestNodeID].rightChild;

        if ( lane == numChildren || lane == bestChild )
        {
            sg_children.nodeID = nodeID;
            sg_children.items = BinaryMortonCodeHierarchy_getNumPrimitives( bnodes, nodeID );
        }

        numChildren++;
    }

    const uint current_index = current.current_index;
    uniform uint global_offset;
    uniform uint child_node_offset;

    // Check if all children will be roots for the local subgtrees in phase1. If so we keep the node ids to be later
    // used in global refit after phase1
    varying uchar is_children_root = (lane < numChildren) ? (sg_children.items <= MORTON_BUILDER_SUBTREE_THRESHOLD) : 0;
    uniform uchar rootMask = sub_group_reduce_or_N6(is_children_root << lane);
    uniform uchar children_roots_num = sub_group_reduce_add(is_children_root);

    if ( lane == 0 )
    {
        child_node_offset = atomic_add_local(local_QNodeOffset,64*numChildren);

        /* Do not create qnodes here */
        uint backpointer = (current.parent_index << 6) | (numChildren << 3);

        global_offset = atomic_add_local( local_numRecords, numChildren - 1 );

#if MORTON_VERBOSE_LOG
        printf("PHASE0: loc_id: %d, index: %d, first_child_id: %d, offset: %d, parent: %d, numChildren: %d, nodeDataStart: %d\n",
               rID, current_index, current_index + qnode->offset, qnode->offset, current.parent_index, numChildren, nodeDataStart);
#endif

        MortonFlattenedBoxlessNode flattened_node;

        if(children_roots_num != numChildren)
            backpointer += children_roots_num;

        flattened_node.binary_hierarchy_index = (current_index << 6) | rootMask;

        uint loc_id = atomic_inc_local( local_p0_total );

        flattened_node.childOffset_type = ((((child_node_offset - nodeDataStart * 64) / 64) - current_index) << 6) | BVH_INTERNAL_NODE;
        flattened_node.backPointer = backpointer;

        //TODO: change this writes to L1WB or streaming
        boxless_nodes[loc_id] = flattened_node;

        *InnerNode_GetBackPointer(backPointers, current_index) = backpointer;
    }

    child_node_offset = sub_group_broadcast( child_node_offset, 0 );
    global_offset = sub_group_broadcast( global_offset, 0 );

    uniform global struct QBVHNodeN* childNodes = (global struct QBVHNodeN*)(bvh_mem + child_node_offset);

    sg_children.current_index = childNodes - nodeData + lane;
    sg_children.parent_index = current_index;

    if ( lane < numChildren )
    {
        uint write_position = (lane == 0) ? rID : global_offset + lane - 1;
        records[write_position] = sg_children;
    }
}

/*

  In this phase a single large work group performs the construction of
  the top of the BVH and creates a build record array.

  Two varians of this kernel:
  1. Refit with global synchronization - Used for big bvh, where number of allocated nodes will not fit
     in SLM in phase2. Phase0 creates qnodes in bvh, and provides startpoints for bottom up phase
     that is executed after phase1. This refit uses global synchronizations and mem_fence_gpu_invalidate
     that is not effective.
  2. Refit with local synchronization - Flattened boxless nodes are passed via global memory, along with
     number of created nodes. Phase0 does not create qnodes in bvh, it is done in phase2 during refit.
     In phase2, flattened boxless nodes are moved to SLM, along with bounding boxes from phase1.
     Refit is performed only with local synchronization.

*/

__attribute__((reqd_work_group_size(512, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) void kernel
parallel_build_phase0(global struct Globals *globals,
                      global struct BinaryMortonCodeHierarchy *bnodes,
                      global char *bvh_mem,
                      global uint *global_refit_startpoints)
{
    global struct BVHBase *bvh = (global struct BVHBase *)bvh_mem;
    global struct QBVHNodeN *nodeData = BVHBase_nodeData(bvh);

    /* a queue of build records in global memory */
    global struct BuildRecordMorton *records = (global struct BuildRecordMorton *)(bvh_mem + 64*bvh->quadLeafStart);
    local uint local_numRecords;
    local uint local_QNodeOffset;
    local uint local_startpoints_num;

    /* initialize first build record */
    if (get_local_id(0) == 0)
    {
        /* allocate root node */
        uint root_node_offset = 64*bvh->nodeDataCur;
        global struct QBVHNodeN *rootNode = (global struct QBVHNodeN *)(bvh_mem + root_node_offset);

        //assert(root_node_offset == 0);
        records[0].nodeID = globals->binary_hierarchy_root;
        records[0].items = globals->numPrimitives;
        records[0].current_index = rootNode - nodeData;
        records[0].parent_index = -1;

        local_numRecords = 1;
        local_QNodeOffset = root_node_offset + 64;
        local_startpoints_num = 0;

        mem_fence_workgroup_default();
    }

    uint num_records = 1;

    /* terminate when all subtrees are under size threshold */
    while(true)
    {
        work_group_barrier(CLK_LOCAL_MEM_FENCE);

        /* all work items in the work group pick a subtree to build */
        for (uint ID = get_sub_group_id(); ID < num_records; ID += get_num_sub_groups() )
        {
            /* small subtrees will get built in next phase */
            if (records[ID].items <= MORTON_BUILDER_SUBTREE_THRESHOLD) // FIXME: should break at 64 leaves not 64 primitives
                continue;

            /* create QBVH node */
            SUBGROUP_create_node_phase0(globals, bnodes, bvh_mem, global_refit_startpoints, ID, &local_numRecords, &local_QNodeOffset,
                                        records, records[ID], &local_startpoints_num);
        }

        work_group_barrier( CLK_LOCAL_MEM_FENCE );
        mem_fence_workgroup_default();
        uint old_num_records = num_records;
        num_records = local_numRecords;
        if( old_num_records == num_records )
            break;

    }

    /* remember number of build records for next phase */
    if (get_local_id( 0 ) == 0)
    {
        globals->numBuildRecords = local_numRecords;
        globals->p0_created_num = local_startpoints_num;
        bvh->nodeDataCur = local_QNodeOffset / 64;

#if MORTON_VERBOSE_LOG
        printf("PHASE_0: allocated %d nodes. globals->global_refit_startpoints: %d\n", BVHBase_numNodes(bvh), globals->p0_created_num);
#endif
    }
}

__attribute__((reqd_work_group_size(512, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) void kernel
parallel_build_phase0_local_sync(global struct Globals *globals,
                      global struct BinaryMortonCodeHierarchy *bnodes,
                      global char *bvh_mem,
                      global struct MortonFlattenedBoxlessNode *boxless_nodes)
{
    global struct BVHBase *bvh = (global struct BVHBase *)bvh_mem;
    global struct QBVHNodeN *nodeData = BVHBase_nodeData(bvh);
    uint nodeDataStart = BVH_ROOT_NODE_OFFSET / 64;

    /* a queue of build records in global memory */
    global struct BuildRecordMorton *records = (global struct BuildRecordMorton *)(bvh_mem + 64*bvh->quadLeafStart);
    local uint local_numRecords;
    local uint local_QNodeOffset;
    local uint local_p0_total;

    /* initialize first build record */
    if (get_local_id(0) == 0)
    {
        /* allocate root node */
        uint root_node_offset = 64*bvh->nodeDataCur;
        global struct QBVHNodeN *rootNode = (global struct QBVHNodeN *)(bvh_mem + root_node_offset);

        //assert(root_node_offset == 0);
        records[0].nodeID = globals->binary_hierarchy_root;
        records[0].items = globals->numPrimitives;
        records[0].current_index = rootNode - nodeData;
        records[0].parent_index = -1;

        local_numRecords = 1;
        local_QNodeOffset = root_node_offset + 64;
        local_p0_total = 0;

        mem_fence_workgroup_default();
    }

    uint num_records = 1;

    /* terminate when all subtrees are under size threshold */
    while(true)
    {
        work_group_barrier(CLK_LOCAL_MEM_FENCE);

        /* all work items in the work group pick a subtree to build */
        for (uint ID = get_sub_group_id(); ID < num_records; ID += get_num_sub_groups() )
        {
            /* small subtrees will get built in next phase */
            if (records[ID].items <= MORTON_BUILDER_SUBTREE_THRESHOLD) // FIXME: should break at 64 leaves not 64 primitives
                continue;

            /* create QBVH node */
            SUBGROUP_create_node_phase0_local_sync(globals, bnodes, bvh_mem, ID, &local_numRecords, &local_QNodeOffset, records,
                                                   records[ID], &local_p0_total, boxless_nodes, nodeDataStart);
        }

        mem_fence_workgroup_default();
        work_group_barrier( CLK_LOCAL_MEM_FENCE );

        uint old_num_records = num_records;
        num_records = local_numRecords;
        if( old_num_records == num_records )
            break;

    }

    /* remember number of build records for next phase */
    if (get_local_id( 0 ) == 0)
    {
        globals->numBuildRecords = local_numRecords;
        bvh->nodeDataCur = local_QNodeOffset / 64;

        globals->p0_allocated_num = BVHBase_numNodes(bvh);
        globals->p0_created_num = local_p0_total;

#if MORTON_VERBOSE_LOG
            printf("PHASE_0_LOCAL_SYNC: allocated %d nodes. globals->global_refit_startpoints: %d\n", BVHBase_numNodes(bvh), globals->global_refit_startpoints);
#endif
    }
}
