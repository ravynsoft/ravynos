//
// Copyright (C) 2009-2022 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "common.h"

#define MORTON_DEBUG_CHECKS 0
#define MORTON_VERBOSE_LOG 0

GRL_INLINE uint get_morton_sort_lsb_req_iterations( uint shift )
{
#if 0 // turn off, because current hierarchy build requires full sort
    // Difference between max iterations needed for LSB sorting and
    // number of iterations needed for LSB sorting without primIDs
    // This indicates how many of first iterations would be skipped in LSB
    return 8 - (8 - (shift >> 3));
#else
    return 0;
#endif
}

typedef struct BuildRecordLocalMortonFlattener
{
    unsigned int leftChild;  // global
    unsigned int rightChild; // global
    unsigned int rangeStart; // global
    unsigned int local_parent_index__numItems;
} BuildRecordLocalMortonFlattener;

// TODO: Currently sizeof UPerNodeData is 32, AABB struct allocates more data than needed and can be reduced
typedef union UPerNodeData {
    float4                           four_DWs;
    BuildRecordLocalMortonFlattener  buildRecord;
    MortonFlattenedBoxlessNode             boxlessNode;
    struct AABB                      box;
} UPerNodeData;

GRL_INLINE uint MortonFlattenedBoxlessNode_GetChildOffset(MortonFlattenedBoxlessNode bn)
{
    return bn.childOffset_type >> 6;
}

GRL_INLINE uint MortonFlattenedBoxlessNode_GetType(MortonFlattenedBoxlessNode bn)
{
    return bn.childOffset_type & ((1<<6) -1);
}

GRL_INLINE void set_2xSG_arr_first_write(uint index, uint* arr, ushort val, short lane)
{
    short lane_used = index % get_sub_group_size();
    short shift = (index / get_sub_group_size()) * get_sub_group_size();
    if (lane_used == lane) {
        *arr |= (val << shift);
    }
}

GRL_INLINE short get_from_2xSG_arr(uint index, uint arr, short lane)
{
    short r = 0;
    short lane_used = index % get_sub_group_size();
    short shift =    (index / get_sub_group_size()) * get_sub_group_size();
        r = arr >> shift;
    r = sub_group_broadcast(r, lane_used);
    return r;
}

GRL_INLINE void unpack_from_2xSG_arr(uint count, uint arr, short lane, ushort* dst)
{
    if (lane < count)
    {
        dst[lane]=(ushort)(arr & 0xFFFF);
        short hi_idx = lane + get_sub_group_size();
        if (hi_idx < count) {
            dst[hi_idx] = (ushort)(arr >> 16);
        }
    }
}


GRL_INLINE void pack_from_2xSG_arr(ushort* src, uint count, uint *arr, short lane)
{
    if (lane < count)
    {
        *arr = src[lane];
        short hi_idx = lane + get_sub_group_size();
        if (hi_idx < count) {
            *arr |= ((uint)(src[hi_idx])) << 16u;
        }
    }
}

GRL_INLINE void set_2xSG_arr(uint index, uint* arr, short val, short lane)
{
    short lane_used = index % get_sub_group_size();
    short shift = (index / get_sub_group_size()) * get_sub_group_size();
    if (lane_used == lane) {
        uint rem_val = (*arr) & (0xFFFF0000 >> shift); //calculate the ramaining other half in the uint
        *arr = (val << shift) | rem_val;
    }
}

GRL_INLINE void SUBGROUP_refit_bottom_up_local(
    uniform struct QBVHNodeN* globalNodeData,
    uniform struct BackPointers* backPointers,
    uniform uint treeletRootGlobalIndex,
    uniform uint globalBaseForInternalNodes,
    varying ushort lane,
    uniform local union UPerNodeData* local_nodes,
    varying uint sg_bu_startpoints,
    uniform uint sg_bu_startpoints_cnt)
{
    if(sg_bu_startpoints_cnt == 0)
        return;

    const uint head_lane = 0;
    uint curNodeIndex = get_from_2xSG_arr(--sg_bu_startpoints_cnt, sg_bu_startpoints, lane);

    uniform uint prev_loc_index = 0;
    uniform struct AABB child_aabb; // this carries reduced aabb between loop turns

    uniform uint backpointer = local_nodes[curNodeIndex].boxlessNode.backPointer;

    while (curNodeIndex != 0)
    {
        uniform uint lead_child_loc_offset = MortonFlattenedBoxlessNode_GetChildOffset(local_nodes[curNodeIndex].boxlessNode);
        uniform uint nodeType = MortonFlattenedBoxlessNode_GetType(local_nodes[curNodeIndex].boxlessNode);
        varying uint child_loc_idx = lead_child_loc_offset + curNodeIndex + lane;

        uint numChildren = BackPointer_GetNumChildren(backpointer);
        if (child_loc_idx != prev_loc_index &&
            lane < numChildren)
        {
            child_aabb = local_nodes[child_loc_idx].box;
        }
        else if (lane >= numChildren) {
            AABB_init(&child_aabb);
            child_aabb.lower.w = as_float(0u);
        }

        // TODO: perNode data could hold 7 dwords per node instead of 8 as long as we keep it in SLM
        struct AABB reduced_bounds = AABB_sub_group_reduce_N6(&child_aabb);
        reduced_bounds = AABB_sub_group_shuffle( &reduced_bounds, 0 );

        uint instMask = (uint)sub_group_reduce_or_N6(as_uint(child_aabb.lower.w));
        reduced_bounds.lower.w = as_float((uint)instMask);
        uint reduce_bounds_lane = AABB_sub_group_shuffle_coordPerLane(&reduced_bounds, 0);
        local uint* pbox = (local uint*)(local_nodes+ curNodeIndex);
        if (lane < 8)
        {
            pbox[lane] = reduce_bounds_lane;
        }

        uint global_node_idx = globalBaseForInternalNodes + curNodeIndex;
        /* get bounds of all children from child nodes directly */
        struct QBVHNodeN* qnode = globalNodeData + global_node_idx;
        subgroup_setQBVHNodeN_setFields(lead_child_loc_offset, nodeType, &child_aabb, numChildren, instMask, qnode, false);
        child_aabb = reduced_bounds;
        uint parentIndex = BackPointer_GetParentIndex(backpointer);

        write_mem_fence(CLK_LOCAL_MEM_FENCE);

        if (lane == 0)
        {
            backpointer = atomic_inc_local(&(local_nodes[parentIndex].boxlessNode.backPointer));
            uint globalParentIndex = (parentIndex > 0) ? (parentIndex + globalBaseForInternalNodes) : treeletRootGlobalIndex;
            uint globalBackpointer = (globalParentIndex << 6) | (numChildren << 3);

            /* set global back pointer */
            *InnerNode_GetBackPointer(backPointers, global_node_idx) = globalBackpointer;

#if MORTON_VERBOSE_LOG
            printf("BU_INNER: index: %d, first_child_id: %d, offset: %d, parent: %d, lead_child_loc_offset: %d, numChildren: %d, child_loc_idx: %d\n",
                   global_node_idx, global_node_idx + qnode->offset, qnode->offset, globalBackpointer >> 6, lead_child_loc_offset, numChildren, child_loc_idx);
#endif
        }

        backpointer = 1 + intel_sub_group_shuffle(backpointer, head_lane);
        prev_loc_index = curNodeIndex;
        curNodeIndex = parentIndex;

        /* if all children got refitted, then continue */
        uniform uint numChildrenRefitted = (backpointer >> 0) & 0x7;
        uniform uint numChildrenTotal = (backpointer >> 3) & 0x7;
        if (numChildrenRefitted != numChildrenTotal)
        {
            if(sg_bu_startpoints_cnt)
            {
                curNodeIndex = get_from_2xSG_arr(--sg_bu_startpoints_cnt, sg_bu_startpoints, lane);
                backpointer = local_nodes[curNodeIndex].boxlessNode.backPointer;
            }
            else
                return;
        }
    }

    // process root of the treelet
    {

#if MORTON_DEBUG_CHECKS
        if (curNodeIndex != 0) printf("SUBGROUP_refit_bottom_up_local: this should be local node index 0\n");
#endif

        uniform uint lead_child_loc_offset = MortonFlattenedBoxlessNode_GetChildOffset(local_nodes[0].boxlessNode);
        varying uint child_loc_idx = lead_child_loc_offset + 0 + lane;
        uint numChildren = BackPointer_GetNumChildren(backpointer);

        if (child_loc_idx != prev_loc_index &&
            lane < numChildren)
        {
            child_aabb = local_nodes[child_loc_idx].box;
        }
        else if (lane >= numChildren) {
            AABB_init(&child_aabb);
            child_aabb.lower.w = as_float(0u);
        }

        // TODO: perNode data could hold 7 dwords per node instead of 8 as long as we keep it in SLM
        uint instMask = (uint)sub_group_reduce_or_N6(as_uint(child_aabb.lower.w));
        uint nodeType = MortonFlattenedBoxlessNode_GetType(local_nodes[curNodeIndex].boxlessNode);
        uint global_node_idx = treeletRootGlobalIndex;
        uint lead_child_global_idx = globalBaseForInternalNodes + lead_child_loc_offset;

        /* get bounds of all children from child nodes directly */
        struct QBVHNodeN* qnode = globalNodeData + global_node_idx;

        subgroup_setQBVHNodeN_setFields(lead_child_global_idx - global_node_idx, nodeType, &child_aabb, numChildren, instMask, qnode, false);

        /* reset refit counter for next refit */
        if (lane == 0)
        {
            /* set global back pointer */
            *InnerNode_GetBackPointer(backPointers, global_node_idx) = backpointer & (~7u);

            // TODO: Move AABBs to separate buffer, but for now communicate bottom-tip boxes through qnodes

#if MORTON_VERBOSE_LOG
            printf("BU_ROOT: curNodeIndex: %d, index: %d, first_child_id: %d, offset: %d, parent: %d, numChildren: %d, sg_bu_startpoints_cnt: %d\n",
                   curNodeIndex, global_node_idx, global_node_idx + qnode->offset, qnode->offset, backpointer >> 6, numChildren, sg_bu_startpoints_cnt);
#endif
        }
    }
}
