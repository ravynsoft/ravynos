//
// Copyright (C) 2009-2022 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "bvh_build_refit.h"
#include "libs/lsc_intrinsics.h"
#include "morton/morton_common.h"

/*

  POSTSORT PHASE2:
  Two kernels here, selected by MORTON_BUILDER_P2_SINGLE_WG_THRESHOLD whish is set to very big value.
  1. parallel_build_phase2_refit - performs refit using global synchronization and mem_fence_gpu_invalidate.
                                   This kernel should be used only for very big bvh, it is faster than non-SLM fallback
                                   in parallel_build_phase2_refit_local.
  2. parallel_build_phase2_refit_local - should be used for most of the cases, we usually fit into SLM with the number of
                                   nodes allocated in phase0, but there is also non-SLM fallback there, as the
                                   decision on which kernel to run is based on the nodes estimates on the host
                                   side.

*/


GRL_INLINE void refit_bottom_up_global_sync(
    global char* bvh_mem,
    global uint* global_refit_startpoints,
    uniform uint nodeId,
    uniform ushort lane)
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;

    BackPointers* backPointers = BVHBase_GetBackPointers( bvh );
    global struct QBVHNodeN* nodeData = BVHBase_nodeData( bvh );

    // Get the node idx that was put here in phase1
    const uint innerNodeIdx = global_refit_startpoints[nodeId];

    // Get the qnode and backpointer
    uniform global struct QBVHNodeN* qnode = nodeData + innerNodeIdx;
    uint backPointer = *InnerNode_GetBackPointer(backPointers, innerNodeIdx);

    varying struct AABB childrenAABB; // one child AABB per lane
    AABB_init(&childrenAABB);

    uniform uint numChildren = (backPointer >> 3) & 0x7;
    if(numChildren == 0) return;

    global struct QBVHNodeN* qnode_child = (global struct QBVHNodeN*)QBVHNodeN_childrenPointer( qnode );
    varying ushort child_idx = (lane < numChildren) ? lane : 0;
    childrenAABB = getAABB_QBVHNodeN( qnode_child + child_idx );

#if MORTON_VERBOSE_LOG
    if(lane == 0)
        printf("REFIT2: index: %d, child_idx: %d\n", innerNodeIdx, child_idx);
#endif

    struct AABB reduce_bounds = AABB_sub_group_reduce_N6( &childrenAABB );
    reduce_bounds = AABB_sub_group_shuffle( &reduce_bounds, 0 );

    subgroup_QBVHNodeN_setBounds(qnode, reduce_bounds, childrenAABB, numChildren, lane);

    uint children_mask = qnode_child[child_idx].instMask;
    qnode->instMask = sub_group_reduce_or_N6(children_mask);

    SUBGROUP_refit_bottom_up( qnode, bvh, reduce_bounds, numChildren, lane, 0 );
}

__attribute__( (reqd_work_group_size( 16, 1, 1 )) ) void kernel
parallel_build_phase2_refit( global char* bvh_mem,
    global uint* global_refit_startpoints )
{
    refit_bottom_up_global_sync(bvh_mem, global_refit_startpoints, get_group_id(0), get_local_id(0));
}


GRL_INLINE void SUBGROUP_refit_bottom_up_global(
    uniform global struct QBVHNodeN* globalNodeData,
    uniform struct BackPointers* backPointers,
    varying ushort lane,
    varying uint curNodeIndex)
{
    uniform uint backpointer = *InnerNode_GetBackPointer(backPointers, curNodeIndex);

    const uint head_lane = 0;
    uniform struct AABB child_aabb; // this carries reduced aabb between loop turns

    while (curNodeIndex != 0)
    {
        global struct QBVHNodeN* qnode = globalNodeData + curNodeIndex;
        global struct QBVHNodeN* qnode_child = (global struct QBVHNodeN*)QBVHNodeN_childrenPointer( qnode );
        uint numChildren = BackPointer_GetNumChildren(backpointer);

        varying ushort child_idx = (lane < numChildren) ? lane : 0;
        child_aabb = getAABB_QBVHNodeN( qnode_child + child_idx );

        struct AABB reduced_bounds = AABB_sub_group_reduce_N6(&child_aabb);
        reduced_bounds = AABB_sub_group_shuffle(&reduced_bounds, head_lane);

        /* get bounds of all children from child nodes directly */
        subgroup_QBVHNodeN_setBounds(qnode, reduced_bounds, child_aabb, numChildren, lane);

        uchar childrenMask = qnode_child[child_idx].instMask;
        qnode->instMask = sub_group_reduce_or_N6(childrenMask);

        uint parentIndex = BackPointer_GetParentIndex(backpointer);

        mem_fence_gpu_invalidate();

        if (lane == 0)
        {
            backpointer = atomic_inc_global((__global uint *)InnerNode_GetBackPointer(backPointers, parentIndex));

            uint globalBackpointer = (parentIndex << 6) | (numChildren << 3);

            /* set global back pointer */
            *InnerNode_GetBackPointer(backPointers, curNodeIndex) = globalBackpointer;

#if MORTON_VERBOSE_LOG
            printf("BU_INNER: index: %d, first_child_id: %d, offset: %d, parent: %d, numChildren: %d, child_loc_idx: %d reduced_bounds: %f\n",
                   curNodeIndex, curNodeIndex + qnode->offset, qnode->offset, backpointer >> 6, numChildren, child_idx, reduced_bounds.lower.x);
#endif
        }

        backpointer = 1 + intel_sub_group_shuffle(backpointer, head_lane);
        curNodeIndex = parentIndex;

        /* if all children got refitted, then continue */
        uniform uint numChildrenRefitted = (backpointer >> 0) & 0x7;
        uniform uint numChildrenTotal = (backpointer >> 3) & 0x7;

        if (numChildrenRefitted != numChildrenTotal)
                return;
    }

    // process root of the treelet
    {

#if MORTON_DEBUG_CHECKS
        if (curNodeIndex != 0) printf("SUBGROUP_refit_bottom_up_local: this should be local node index 0\n");
#endif

        global struct QBVHNodeN* qnode_child = (global struct QBVHNodeN*)QBVHNodeN_childrenPointer( globalNodeData );
        uint numChildren = BackPointer_GetNumChildren(backpointer);

        varying ushort child_idx = (lane < numChildren) ? lane : 0;
        child_aabb = getAABB_QBVHNodeN( qnode_child + child_idx );

        struct AABB reduced_bounds = AABB_sub_group_reduce_N6(&child_aabb);
        reduced_bounds = AABB_sub_group_shuffle(&reduced_bounds, head_lane);

        /* get bounds of all children from child nodes directly */
        subgroup_QBVHNodeN_setBounds(globalNodeData, reduced_bounds, child_aabb, numChildren, lane);

        uchar childrenMask = qnode_child[child_idx].instMask;
        globalNodeData->instMask = sub_group_reduce_or_N6(childrenMask);

        /* reset refit counter for next refit */
        if (lane == 0)
        {
            /* set global back pointer */
            *InnerNode_GetBackPointer(backPointers, 0) = backpointer & (~7u);

#if MORTON_VERBOSE_LOG
        printf("BU_ROOT: curNodeIndex: %d, index: %d, first_child_id: %d, offset: %d, parent: %d, numChildren: %d, sg_bu_startpoints_cnt: %d\n",
               curNodeIndex, 0, 0 + globalNodeData->offset, globalNodeData->offset, backpointer >> 6, numChildren, sg_bu_startpoints_cnt);
#endif
        }
    }
}


// TODO: Check why 512 wg size has worse performance than 256
__attribute__( (reqd_work_group_size( 512, 1, 1 )) )
__attribute__((intel_reqd_sub_group_size(16))) void kernel
parallel_build_phase2_refit_local( global struct Globals* globals,
    global char* bvh_mem,
    global struct MortonFlattenedBoxlessNode *boxless_nodes)
{
    // Number of nodes created in P0, to be refitted in this stage
    uint p0_created_num = globals->p0_created_num;

    // Return immediately if host executed this kernel but there is nothing to do
    if(p0_created_num == 0)
        return;

    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    BackPointers* backPointers = BVHBase_GetBackPointers( bvh );
    global struct QBVHNodeN* nodeData = BVHBase_nodeData( bvh );
    varying ushort lane = get_sub_group_local_id();

    // Hardcode SLM to max here as we do not know upfront how much mem will be needed
    local union UPerNodeData perNodeData[MORTON_BUILDER_P2_ELEMENTS_IN_SLM]; /* 16kb is max slm for 256 wg_size */

    // Number of allocated nodes in phase0 (p0_created_num + children)
    uint p0_allocated_num = globals->p0_allocated_num;

    // array that will keep 2x8 shorts indices
    varying uint sg_fatleaf_array = 0x0;
    uniform uint8_t sg_bu_startpoints_cnt = 0;

    // Determine if we can fit into SLM with all the nodes allocated in phase0,
    // There are two paths here:
    // 1. Copy all needed flattened nodes and bounding boxes to SLM and reuse bottom up local,
    //    which does refit nad creates qnodes in bvh
    // 2. If not fit into SLM, first create qnodes in bvh, and perform bottom up refit with global atomics synchronization.
    //    It is not performant to do so, keep it as a guardrail here. On the host side we do fallback
    //    to the old refit separated path, with wg_size 8 with better EU reuse.
    if(p0_allocated_num < MORTON_BUILDER_P2_ELEMENTS_IN_SLM)
    {
        for (uint ID = get_sub_group_id(); ID < p0_created_num; ID += get_num_sub_groups() )
        {
            MortonFlattenedBoxlessNode boxless_node = boxless_nodes[ID];
            uint current_id = boxless_node.binary_hierarchy_index >> 6;

            // Put the mask for the children that are subtree roots in the binary_hierarchy_index that is unused
            uchar children_root_mask = (boxless_node.binary_hierarchy_index & 0x3F);

            if(lane == 0)
                perNodeData[current_id].boxlessNode = boxless_node;

            // When no children are subtree roots, we are done and skip to the next iteration
            if(children_root_mask == 0x0)
            {
                continue;
            }
            // When all children are subtree roots, put them to sg_fatleaf_array
            else if(children_root_mask == 0x3F)
            {
                set_2xSG_arr_first_write(sg_bu_startpoints_cnt++, &sg_fatleaf_array, current_id, lane);
            }

            uniform global struct QBVHNodeN* qnode = nodeData + current_id;

            uniform uint numChildren = (boxless_node.backPointer >> 3) & 0x7;
            uint lead_child_offset = MortonFlattenedBoxlessNode_GetChildOffset(boxless_node);
            varying ushort child_idx = (lane < numChildren) ? lane : 0;

            varying struct AABB childrenAABB; // one child AABB per lane
            AABB_init(&childrenAABB);

            uint lead_child_global_id = current_id + lead_child_offset;

            uniform global struct QBVHNodeN* qnode_child = nodeData + lead_child_global_id;
            childrenAABB = getAABB_QBVHNodeN( qnode_child + child_idx );

            // Get only AABBs of children that are p1 subtree roots
            bool lane_active = boxless_node.binary_hierarchy_index & (1 << child_idx);
            if(lane_active)
            {
                uint child_global_id = lead_child_global_id + child_idx;
                perNodeData[child_global_id].box = childrenAABB;
                perNodeData[child_global_id].box.lower.w = as_float((uint)qnode_child->instMask);
            }

#if MORTON_VERBOSE_LOG
            if(lane == 0)
                printf("P2_LOCAL: ID: %d, lead_child_offset: %d, child_idx: %d, lane_active: %d, boxless_node >> 6: %d, perNodeData[ID].box = %f, qnode->offset: %d\n", ID, lead_child_offset, child_idx, lane_active, boxless_node.backPointer >> 6, perNodeData[ID].box.lower.x, qnode->offset);
#endif
        }

        work_group_barrier(CLK_LOCAL_MEM_FENCE);

        SUBGROUP_refit_bottom_up_local(nodeData, backPointers, 0, 0, lane, perNodeData, sg_fatleaf_array, sg_bu_startpoints_cnt);
    }
    else
    {
        for (uint ID = get_sub_group_id(); ID < p0_created_num; ID += get_num_sub_groups() )
        {
            MortonFlattenedBoxlessNode boxless_node = boxless_nodes[ID];
            uint current_id = boxless_node.binary_hierarchy_index >> 6;

            // Put the mask for the children that are subtree roots in the binary_hierarchy_index that is unused
            uchar children_root_mask = (boxless_node.binary_hierarchy_index & 0x3F);
            uniform uint numChildren = (boxless_node.backPointer >> 3) & 0x7;

            uniform global struct QBVHNodeN* qnode = nodeData + current_id;
            uint nodeType = MortonFlattenedBoxlessNode_GetType(boxless_node);
            uint lead_child_offset = MortonFlattenedBoxlessNode_GetChildOffset(boxless_node);

            SUBGROUP_QBVHNodeN_setChildIncr1( qnode );
            if(lane == 0)
            {
                QBVH6Node_set_type( qnode, nodeType );
                qnode->offset = lead_child_offset;
            }

            // When no children are subtree roots, we are done and skip to the next iteration
            if(children_root_mask == 0x0)
            {
                continue;
            }
            // When all children are subtree roots, put them to sg_fatleaf_array
            else if(children_root_mask == 0x3F)
            {
                set_2xSG_arr_first_write(sg_bu_startpoints_cnt++, &sg_fatleaf_array, current_id, lane);
            }

#if MORTON_VERBOSE_LOG
            if(lane == 0)
                printf("P2_GLOBAL: ID: %d, lead_child_offset: %d, child_idx: %d, boxless_node >> 6: %d, perNodeData[ID].box = %f, qnode->offset: %d\n", ID, lead_child_offset, child_idx, boxless_node.backPointer >> 6, reduce_bounds.lower.x, qnode->offset);
#endif
        }

        while (sg_bu_startpoints_cnt > 0)
        {
            uint curNodeIndex = get_from_2xSG_arr(--sg_bu_startpoints_cnt, sg_fatleaf_array, lane);

            SUBGROUP_refit_bottom_up_global(nodeData, backPointers, lane, curNodeIndex);
        }
    }
}
