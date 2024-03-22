//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "input_client_structs.h"
#include "common.h"
#include "instance.h"

#define DBG(x)
#define ENABLE_CHECKS 0

/*

  This kernel implements a exclusive scan addition operation. The
  implementation currently only uses one DSS.

 */
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
parallel_scan_exclusive_add(global uint *input,
                            global uint *output,
                            const uint N)
{
    const uint j = get_local_id(0);
    const uint J = get_local_size(0);
    const uint BLOCKSIZE = (N + J - 1) / J;
    const uint start = min((j + 0) * BLOCKSIZE, N);
    const uint end = min((j + 1) * BLOCKSIZE, N);

    uint base = 0;
    for (uint i = start; i < end; i++)
        base += input[i];

    base = work_group_scan_exclusive_add(base);

    uint accu = 0;
    for (uint i = start; i < end; i++)
    {
        output[i] = base + accu;
        accu += input[i];
    }
}

/*

  This kernel implements a exclusive scan addition operation that can use the entire GPU.

 */
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
parallel_scan_exclusive_add_phase0(global uint *input,
                                   global uint *output,
                                   global uint *prefix_sums,
                                   const uint N)
{
    const uint local_size = get_local_size(0);
    const uint numTasks = get_num_groups(0);
    const uint groupID = get_group_id(0);
    const uint localID = get_local_id(0);
    const uint global_startID = (groupID + 0) * N / numTasks;
    const uint global_endID = (groupID + 1) * N / numTasks;

    uint base = 0;
    for (uint i = global_startID + localID; i < global_endID; i += local_size)
        base += input[i];

    base = work_group_reduce_add(base);

    if (localID == 0)
    {
        prefix_sums[groupID] = base;
        printf("%d -> %d \n", groupID, base);
    }
}

__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
parallel_scan_exclusive_add_phase1(global uint *input,
                                   global uint *output,
                                   global uint *prefix_sums,
                                   const uint N)
{
    const uint local_size = get_local_size(0);
    const uint numTasks = get_num_groups(0);
    const uint groupID = get_group_id(0);
    const uint localID = get_local_id(0);
    const uint global_startID = (groupID + 0) * N / numTasks;
    const uint global_endID = (groupID + 1) * N / numTasks;
    const uint local_range = global_endID - global_startID;

    uint global_base = 0;
    for (uint i = 0; i < groupID; i++)
        global_base += prefix_sums[i];

    const uint j = get_local_id(0);
    const uint J = get_local_size(0);
    const uint BLOCKSIZE = (local_range + J - 1) / J;
    const uint startID = (j + 0) * local_range / J + global_startID;
    const uint endID = (j + 1) * local_range / J + global_startID;

    uint base = 0;
    for (uint i = startID; i < endID; i++)
        base += input[i];

    base = work_group_scan_exclusive_add(base);

    uint accu = 0;
    for (uint i = startID; i < endID; i++)
    {
        output[i] = global_base + base + accu;
        accu += input[i];
    }
}

/* ========================================================================= */
/* ============================== STATISTICS =============================== */
/* ========================================================================= */

/* ====== STATS config ====== */

#define ENABLE_STAT_CHECKS 1
#define DBG_STATS(x)

__attribute__((reqd_work_group_size(256, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
printBVHStatistics(global struct Globals *globals,
                   global char *bvh_mem,
                   global struct StatStackEntry *global_stack0,
                   global struct StatStackEntry *global_stack1,
                   const uint presplit)
{
    const uint globalID = get_global_id(0);
    const uint localID = get_local_id(0);
    const uint local_size = get_local_size(0);

    struct BVHBase *base = (struct BVHBase *)bvh_mem;
    const uint root = base->rootNodeOffset;

    local uint stack_items[2];
    local uint iterations;

    struct AABB root_aabb = getAABB_QBVHNodeN((global struct QBVHNodeN *)(bvh_mem + root));
    root_aabb = conservativeAABB(&root_aabb);
    const float root_area = AABB_halfArea(&root_aabb);

    global struct QBVHNodeN *root_node = (global struct QBVHNodeN *)(bvh_mem + base->rootNodeOffset);

    if (root_node->type != BVH_INTERNAL_NODE)
    {
        const uint numChildren = getNumChildren_QBVHNodeN(root_node);
        const uint current = root;
        for (uint i = 0; i < numChildren; i++)
        {
            struct AABB aabb = extractAABB_QBVHNodeN(root_node, i);
            const float area = AABB_halfArea(&aabb);

            global_stack0[i].node = current + root_node->offset * 64 + i * sizeof(struct Quad);
            global_stack0[i].type = root_node->type;
            global_stack0[i].area = area;
            global_stack0[i].aabb = aabb;
            global_stack0[i].depth = 0;
        }
        stack_items[0] = numChildren;
        stack_items[1] = 0;
    }
    else
    {
        global_stack0[0].node = root;
        global_stack0[0].type = root_node->type;
        global_stack0[0].area = root_area;
        global_stack0[0].aabb = root_aabb;
        global_stack0[0].depth = 1;
        stack_items[0] = 1;
        stack_items[1] = 0;
    }

    const uint maxInnerNodeOffset = globals->node_mem_allocator.cur;
    const uint maxLeafNodeOffset = globals->quad_mem_allocator.cur;

    DBG_STATS(if (localID == 0) printf("diff %d \n", (globals->node_mem_allocator_cur - globals->node_mem_allocator_start) / 64));

    iterations = 0;

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    float sah_nodes = 0.0f;
    float sah_leaves = 0.0f;
    uint leaves = 0;
    uint inner_nodes = 0;
    uint max_depth = 0;
    uint leaf_items = 0;
    uint inner_nodes_valid_children = 0;

    while (1)
    {
        work_group_barrier(CLK_GLOBAL_MEM_FENCE);
        const uint buffer_index = (iterations % 2) == 0 ? 0 : 1;
        global struct StatStackEntry *input_global_stack = buffer_index == 0 ? global_stack0 : global_stack1;
        global struct StatStackEntry *output_global_stack = buffer_index == 0 ? global_stack1 : global_stack0;

        const uint local_stack_items = stack_items[buffer_index];
        stack_items[1 - buffer_index] = 0;

        DBG_STATS(if (globalID == 0) printf("iterations %d local_stack_items %d \n", iterations, local_stack_items));

        if (local_stack_items == 0)
            break;
        //if (iterations == 5) break;

        work_group_barrier(CLK_GLOBAL_MEM_FENCE);

        if (globalID == 0)
            iterations++;

        for (uint sindex = localID; sindex < local_stack_items; sindex += local_size)
        {

            uint current = input_global_stack[sindex].node;
            uint type = input_global_stack[sindex].type;
            float current_area = input_global_stack[sindex].area;
            struct AABB current_aabb = input_global_stack[sindex].aabb;
            uint current_depth = input_global_stack[sindex].depth;

            //printf("localID %d sindex %d current %d type %d local_stack_items %d \n",localID,sindex,current,type,local_stack_items);

            max_depth = max(max_depth, current_depth);

            if (type == BVH_QUAD_NODE)
            {
                unsigned int prims = 1; //getNumLeafPrims(current);
                if (prims > BVH_LEAF_N_MAX)
                    printf("too many items in leaf %d \n", prims);
                unsigned int prims_offset = current; //getLeafOffset(current);
                //printf("prims_offset %d \n",prims_offset);

                leaf_items += prims;
                sah_leaves += current_area;
                leaves++;
#if ENABLE_STAT_CHECKS == 1
                struct AABB leafAABB;
                AABB_init(&leafAABB);

                global struct Quad *quads = (global struct Quad *)(bvh_mem + prims_offset);
                //printf("prims_offset %d \n",prims_offset);

                for (uint i = 0; i < prims; i++)
                {
                    struct AABB quadAABB = getAABB_Quad(&quads[i]);
                    AABB_extend(&leafAABB, &quadAABB);
                }

                if (!presplit && !AABB_subset(&leafAABB, &current_aabb))
                {
                    printf("leaf error: current %d depth %d \n", current, current_depth);
                    AABB_print(&current_aabb);
                    printf("leaf bounds: \n");
                    AABB_print(&leafAABB);
                }
#endif
            }
            else if (type == BVH_INTERNAL_NODE)
            {
                inner_nodes++;
                sah_nodes += current_area;
                global struct QBVHNodeN *nodeN = (global struct QBVHNodeN *)(bvh_mem + current);

                uint children = 0;
                for (uint i = 0; i < BVH_NODE_N6; i++)
                {
                    if (nodeN->qbounds.lower_x[i] > nodeN->qbounds.upper_x[i])
                        break;
                    children++;
                }
                //printf("children %d \n",children);

#if ENABLE_STAT_CHECKS == 1
                if (children > BVH_NODE_N6 || children == 0)
                {
                    printf("#children not in valid range: %d offset %d localID %d \n", children, current, localID);
                    printQBVHNodeN(nodeN);
                }

                if (nodeN->offset > globals->totalAllocatedMem || (int)nodeN->offset < 0)
                {
                    printf("offset error %d \n", nodeN->offset);
                }
#endif

                uint children_offset = atomic_add(&stack_items[1 - buffer_index], children);

                for (uint i = 0; i < children; i++)
                {
                    inner_nodes_valid_children++;

                    struct AABB aabb = extractAABB_QBVHNodeN(nodeN, i);
                    const float area = AABB_halfArea(&aabb);

                    aabb = conservativeAABB(&aabb);

#if 0 // ENABLE_STAT_CHECKS == 1                            // FIXME: not clear whether parent child property still holds !!!!

                  // if (aabb.lower.x == (float)(INFINITY))
                  //   {
                  //     printf("aabb inf error %d current %d nodeN %d \n",i, current, children);
                  //     break;
                  //   }


                  if (!presplit && !AABB_subset(&aabb,&current_aabb))
                    {
                      printf("Parent: current %d depth %d children %d \n",current, current_depth, children);
                      AABB_print(&current_aabb);
                      printf("Child %d: \n",i);
                      AABB_print(&aabb);
                    }
#endif

                    uint dest_index = children_offset + i;
                    if (nodeN->type == BVH_QUAD_NODE)
                    {
                        output_global_stack[dest_index].node = current + nodeN->offset * 64 + i * sizeof(struct Quad);
                        if (output_global_stack[dest_index].node >= maxLeafNodeOffset)
                        {
                            printf("stack leaf offset error %d %d current %d %d \n", output_global_stack[dest_index].node, output_global_stack[dest_index].node / 64, current, current / 64);
                        }
                    }
                    else if (nodeN->type == BVH_INTERNAL_NODE)
                    {
                        output_global_stack[dest_index].node = (current + nodeN->offset * 64 + i * sizeof(struct QBVHNodeN));
                        if (output_global_stack[dest_index].node >= maxInnerNodeOffset)
                        {
                            printf("stack inner node offset error %d %d current %d %d maxInnerNodeOffset %d \n", output_global_stack[dest_index].node, output_global_stack[dest_index].node / 64, current, current / 64, maxInnerNodeOffset);
                        }
                    }

                    output_global_stack[dest_index].type = nodeN->type;
                    output_global_stack[dest_index].area = area;
                    output_global_stack[dest_index].aabb = aabb;
                    output_global_stack[dest_index].depth = current_depth + 1;
                    //printf("global_stack[dest_index].node %d global_stack[dest_index].type %d \n",global_stack[dest_index].node,global_stack[dest_index].type);
                }
            }
        }
    }

    sah_nodes = work_group_reduce_add(sah_nodes);
    sah_leaves = work_group_reduce_add(sah_leaves);
    leaves = work_group_reduce_add(leaves);
    inner_nodes = work_group_reduce_add(inner_nodes);
    max_depth = work_group_reduce_max(max_depth);
    leaf_items = work_group_reduce_add(leaf_items);
    inner_nodes_valid_children = work_group_reduce_add(inner_nodes_valid_children);

    if (globalID == 0)
    {
        /*
    sah_nodes  *= 1.0f / root_area;
    sah_leaves *= 1.0f / root_area;
    float sah = sah_nodes + sah_leaves;

    const uint globalLeafMemAllocatorOffset = globals->quad_mem_allocator.start;
    const uint totalAllocatedMem = globals->totalAllocatedMem;

    printf("BVH_NODE_N6 %d BVH_LEAF_N_MIN %d BVH_LEAF_N_MAX %d \n",BVH_NODE_N6,BVH_LEAF_N_MIN,BVH_LEAF_N_MAX);
    float node_util = 100.0f * (float)inner_nodes_valid_children / (inner_nodes * BVH_NODE_N6);
    float leaf_util = 100.0f * (float)leaf_items / (leaves);
    printf("allocators: node  %d -> %d ; leaf %d -> %d \n",globals->node_mem_allocator_cur,globals->node_mem_allocator_start,globals->leaf_mem_allocator_cur,globals->leaf_mem_allocator_start);
    printf("inner nodes %d leaves %d  sah %f sah_node %f sah_leaves %f max_depth %d leaf_items %d node util %f leaf util %f (%f) \n",inner_nodes,leaves,sah,sah_nodes,sah_leaves,max_depth,leaf_items,node_util,leaf_util,(float)leaf_items / leaves);
    uint node_mem     = globals->node_mem_allocator_cur;
    uint max_node_mem = globalLeafMemAllocatorOffset;
    float node_mem_ratio = 100.0f * (float)node_mem / max_node_mem;

    uint leaf_mem        = globals->leaf_mem_allocator.cur - globalLeafMemAllocatorOffset;
    uint max_leaf_mem    = totalAllocatedMem - globalLeafMemAllocatorOffset;
    float leaf_mem_ratio = 100.0f * (float)leaf_mem / max_leaf_mem;

    uint total_mem = node_mem + leaf_mem;
    float total_mem_ratio = 100.0f * (float)total_mem / totalAllocatedMem;

    printf("used node memory %d (%f) / used leaf memory %d (%f) / total memory used %d (%f) / total memory allocated %d \n",node_mem, node_mem_ratio, leaf_mem, leaf_mem_ratio, total_mem, total_mem_ratio, totalAllocatedMem);
    */
    }
}
