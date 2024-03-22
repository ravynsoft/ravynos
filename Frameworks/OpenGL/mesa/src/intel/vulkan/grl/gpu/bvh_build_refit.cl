//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "bvh_build_refit.h"
#include "api_interface.h"
#include "common.h"





#if 0 
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( 16, 1, 1 )) )
void kernel
update_instance_leaves( global struct BVHBase* bvh,
    uint64_t dxrInstancesArray,
    uint64_t dxrInstancesPtr,
    global struct AABB3f* instance_aabb_scratch
)
{
    uint num_leaves = BVHBase_GetNumHWInstanceLeaves( bvh );
    uint id = get_local_id( 0 ) + get_local_size( 0 ) * get_group_id( 0 );
    if ( id >= num_leaves )
        return;

    global struct GRL_RAYTRACING_INSTANCE_DESC* instancesArray =
        (global struct GRL_RAYTRACING_INSTANCE_DESC*)dxrInstancesArray;
    global struct GRL_RAYTRACING_INSTANCE_DESC** instancesPtrArray =
        (global struct GRL_RAYTRACING_INSTANCE_DESC**)dxrInstancesPtr;

    global struct HwInstanceLeaf* leafs = (global struct HwInstanceLeaf*) BVHBase_GetHWInstanceLeaves( bvh );

    /* iterate over all children of the instance node and get their bounds */

    uint32_t instanceIdx = HwInstanceLeafPart1_getInstanceIndex( &leafs[id] );
    global struct GRL_RAYTRACING_INSTANCE_DESC* instance = NULL;
    if ( dxrInstancesArray != NULL )
        instance = &instancesArray[instanceIdx];
    else
        instance = instancesPtrArray[instanceIdx];

    struct AffineSpace3f xfm = AffineSpace3f_load_row_major( instance->Transform );
    global struct BVHBase* instanceBvh = (global struct BVHBase*)instance->AccelerationStructure;
    struct AABB3f newSubtreeBounds = instanceBvh->Meta.bounds;
    struct AABB3f bbox = AABB3f_transform( xfm, newSubtreeBounds ); // JDB TODO:  Use faster abs-matrix method

    const bool valid_min = isfinite( bbox.lower[0] ) && isfinite( bbox.lower[1] ) && isfinite( bbox.lower[2] );
    const bool valid_max = isfinite( bbox.upper[0] ) && isfinite( bbox.upper[1] ) && isfinite( bbox.upper[2] );

    uint mask = GRL_get_InstanceMask(instance);

    uint offset = instanceBvh->rootNodeOffset;
    if ( !valid_min || !valid_max )
    {
        bbox.lower[0] = xfm.p.x;
        bbox.lower[1] = xfm.p.y;
        bbox.lower[2] = xfm.p.z;
        bbox.upper[0] = xfm.p.x;
        bbox.upper[1] = xfm.p.y;
        bbox.upper[2] = xfm.p.z;
        offset = NO_NODE_OFFSET;
        mask = 0;
    }

    instance_aabb_scratch[id] = bbox;
    
    HwInstanceLeaf_Constructor( &leafs[id], instance, instanceIdx, offset, mask ); // TODO: No instance opening for refittable BVH   
}
#endif


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel
update_instance_leaves(global struct BVHBase* bvh,
    uint64_t dxrInstancesArray,
    uint64_t dxrInstancesPtr,
    global struct AABB3f* instance_aabb_scratch
)
{
    uint num_leaves = BVHBase_GetNumHWInstanceLeaves(bvh);
    uint id = get_local_id(0) + get_local_size(0) * get_group_id(0);
    if (id >= num_leaves)
        return;

    DO_update_instance_leaves(
        bvh,
        dxrInstancesArray,
        dxrInstancesPtr,
        instance_aabb_scratch,
        id,
        0 );
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel
update_instance_leaves_indirect(global struct BVHBase* bvh,
    uint64_t dxrInstancesArray,
    uint64_t dxrInstancesPtr,
    global struct AABB3f* instance_aabb_scratch,
    global struct IndirectBuildRangeInfo* indirect_data)
{
    uint num_leaves = BVHBase_GetNumHWInstanceLeaves(bvh);
    uint id = get_local_id(0) + get_local_size(0) * get_group_id(0);
    if (id >= num_leaves)
        return;

    DO_update_instance_leaves(
        bvh,
        dxrInstancesArray + indirect_data->primitiveOffset,
        dxrInstancesPtr,
        instance_aabb_scratch,
        id,
        0 );
}

#if 0
/*

  This kernel refit a BVH. The algorithm iterates over all BVH nodes
  to find all leaf nodes, which is where refitting starts. For these
  leaf nodes bounds get recalculated and then propagates up the tree.

  One kernel instance considers a range of inner nodes as startpoints.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(8, 1, 1))) void kernel refit(
    global struct BVHBase *bvh,
    global GRL_RAYTRACING_GEOMETRY_DESC* geosArray,
    global struct AABB3f* instance_leaf_aabbs )
{
    /* here we temporarily store the bounds for the children of a node */
    struct AABB childrenAABB[BVH_NODE_N6];

    /* get pointer to inner nodes and back pointers */
    global struct QBVHNodeN *inner_nodes = BVHBase_rootNode(bvh);
    BackPointers* backPointers = BVHBase_GetBackPointers(bvh);

    /* construct range of nodes that each work group will process */
    const uint numInnerNodes = BVHBase_numNodes(bvh);
    const uint startID = (get_group_id(0) + 0) * numInnerNodes / get_num_groups(0);
    const uint endID = (get_group_id(0) + 1) * numInnerNodes / get_num_groups(0);

    /* each workgroup iterates over its range of nodes */
    for (uint i = startID + get_local_id(0); i < endID; i += get_local_size(0))
    {
        global struct QBVHNodeN* curNode = &inner_nodes[i];
        uint numChildren = refit_bottom(bvh, geosArray,
                                 instance_leaf_aabbs,
                                 curNode,
                                 childrenAABB,
                                 *InnerNode_GetBackPointer(backPointers, i));
        if (numChildren != 0)
        {
            /* update bounds of node */
            QBVHNodeN_setBounds(curNode, childrenAABB, numChildren);

            /* refit upper parts of the BVH */
            // TODO: this will not gonna work for mixed nodes
            refit_bottom_up(curNode, bvh, childrenAABB, numChildren);
        }
    }
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(8, 1, 1)))
void kernel Find_refit_treelets(
    global struct BVHBase* bvh,
    global TreeletNodeData* treelets,
    global uint* scratchStartpoints,
    global uint* startpointAlloc)
{
    find_refit_treelets(bvh,
                        treelets,
                        scratchStartpoints,
                        startpointAlloc);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1))) 
void kernel Assign_refit_startpoints_to_treelets(
    global struct BVHBase* bvh,
    global TreeletNodeData* treelets,
    global uint* scratchStartpoints)
{
    assign_refit_startpoints_to_treelets(bvh, treelets, scratchStartpoints);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(128, 1, 1))) 
__attribute__((intel_reqd_sub_group_size(16)))
void kernel Finalize_treelets_in_groups(
    global struct BVHBase* bvh,
    global uint* scratchStartpoints )
{
    local uint depths[FINALIZE_TREELETS_SLM_DEPTHS_SPACE];

    finalize_treelets_in_groups(bvh, scratchStartpoints, depths);
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(256, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel Refit_quads_tree_per_group(global SquashedInput* psqinputs)
{
    uint group_id = get_group_id(0);
    SquashedInput sqinput = psqinputs[group_id];
    global struct BVHBase* bvh = sqinput.pBvh;
    uint numLeaves = BVHBase_GetNumQuads(bvh);
    global QuadLeaf* leafs = (global QuadLeaf*)BVHBase_GetQuadLeaves(bvh);

    global void* input = sqinput.pInput;
    global struct AABB* bbox_scratch = sqinput.bbox_scratch;

    uint leafsIndexOffset = bvh->quadLeafStart - BVH_ROOT_NODE_OFFSET / 64;
    global GRL_RAYTRACING_GEOMETRY_DESC* geosArray = (global GRL_RAYTRACING_GEOMETRY_DESC*) input;
    uint id = get_local_id(0);

    for (uint leaf_id = id; leaf_id < numLeaves; leaf_id += get_local_size(0))
    {
        struct AABB theAABB;
        refit_bottom_child_quad(leafs + leaf_id, geosArray, &theAABB);
        theAABB.lower.w = as_float(0xABBADEFFu);
        bbox_scratch[leafsIndexOffset + leaf_id] = theAABB;
    }
}



GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(32, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel Refit_quads(
    global struct BVHBase* bvh,
    global void* input,
    global struct AABB* bbox_scratch,
    uint numGroupsExecuted,
    global SquashedInputGroupDesc* sqinput)
{
    uint numLeafs = BVHBase_GetNumQuads(bvh);
    if (numLeafs == 0) return;
    global QuadLeaf* leafs = (global QuadLeaf*)BVHBase_GetQuadLeaves(bvh);
    
    global GRL_RAYTRACING_GEOMETRY_DESC* geosArray = (global GRL_RAYTRACING_GEOMETRY_DESC*) input;
    uint leafsIndexOffset = bvh->quadLeafStart - BVH_ROOT_NODE_OFFSET / 64;

    uint numLeafsPerGr = (numLeafs + (numGroupsExecuted - 1)) / numGroupsExecuted;

    uint id_start = get_group_id(0) * numLeafsPerGr + get_local_id(0);
    uint id_end = min(id_start + numLeafsPerGr, numLeafs);
    for (uint id = id_start; id < id_end; id+= get_local_size(0))
    {
        struct AABB theAABB;
        refit_bottom_child_quad(leafs + id, geosArray, &theAABB);
        theAABB.lower.w = as_float(0xABBADEFFu);
        bbox_scratch[leafsIndexOffset + id] = theAABB;
    }

    if (get_group_id(0) == 0 && get_local_id(0) < 16)
    {
        
        uint groupnr;
        uint treeletCnt = *BVHBase_GetRefitTreeletCntPtr(bvh);
        if (get_sub_group_local_id() == 0) {
            groupnr = atomic_add_global(&sqinput->totalNumGroups, treeletCnt);
        }
        groupnr = sub_group_broadcast(groupnr, 0);
        for (uint subtree = get_sub_group_local_id(); subtree < treeletCnt; subtree += get_sub_group_size())
        {
            uint gr = groupnr + subtree;
            //printf("tree %llx, treelet %d/%d, grId %d, numStartpoints %d\n",  bvh, subtree,treeletCnt, gr, BVHBase_GetRefitTreeletDescs(bvh)[subtree].numStartpoints);
            sqinput[gr].bvh = (qword)bvh;
            sqinput[gr].scratch = (qword)bbox_scratch;
            sqinput[gr].groupInTree = subtree;
        }
        //if (get_local_id(0)==0 && treeletCnt > 1)
        //{
        //    printf("tree %llx, tip treelet %d/%d = numStartpoints %d depth %d\n", bvh, treeletCnt, treeletCnt, BVHBase_GetRefitTreeletDescs(bvh)[treeletCnt].numStartpoints, BVHBase_GetRefitTreeletDescs(bvh)[treeletCnt].maxDepth);
        //}
    }
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(256, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel
Refit_tree_per_group_quad(
    global SquashedInput* psqinputs)
{
    uint group_id = get_group_id(0);
    SquashedInput sqinput = psqinputs[group_id];
    global struct BVHBase* bvh = sqinput.pBvh;
    global struct AABB* bbox_scratch = sqinput.bbox_scratch;
    global void* pInput = sqinput.pInput;
    local Treelet_by_single_group_locals loc;

    if (*BVHBase_GetRefitTreeletCntPtr(bvh) == 0)
        return;

#if REFIT_DEBUG_CHECKS
    uint bottoms_cnt = *BVHBase_GetRefitTreeletCntPtr(bvh);
    if (bottoms_cnt != 1) {
        if (get_local_id(0) == 0)
        {
            printf("Error: this tree has more than 1 treelets!\n");
        }
        return;
    }
#endif

    /* get pointer to inner nodes and back pointers */
    uniform global struct QBVHNodeN* inner_nodes = BVHBase_rootNode(bvh);

    // uniform per group
    uniform RefitTreelet* pTrltDsc = BVHBase_GetRefitTreeletDescs(bvh);    

    uint numLeafs = bvh->quadLeafCur - bvh->quadLeafStart;
    
    if (numLeafs == 0) { return; }

    uint numLeafsByOneThread = (numLeafs + (get_local_size(0) - 1)) / get_local_size(0);

    update_quads(bvh, pInput, bbox_scratch, get_local_id(0), numLeafsByOneThread);

    mem_fence_workgroup_default(); work_group_barrier(0);
    
    RefitTreelet trltDsc = *pTrltDsc;

    refit_treelet_by_single_group(
        bbox_scratch,
        &loc,
        bvh,
        trltDsc,
        false,
        true);
    
    if (trltDsc.maxDepth > 0)
    {
        mem_fence_workgroup_default(); work_group_barrier(0);
        post_refit_encode_qnode_tree_per_group(bbox_scratch,bvh);
    }
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(256, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel
Refit_treelet_per_group(
    global SquashedInputGroupDesc* sqinput)
{
    uint group_id = get_group_id(0);
    global struct AABB*    bbox_scratch = (global struct AABB* )sqinput[group_id].scratch;
    global struct BVHBase* bvh          = (global struct BVHBase* )sqinput[group_id].bvh;
    group_id                            = sqinput[group_id].groupInTree;

    /* get pointer to inner nodes and back pointers */
    uniform global struct QBVHNodeN* inner_nodes = BVHBase_rootNode(bvh);

    uint bottoms_cnt = *BVHBase_GetRefitTreeletCntPtr(bvh);

    // uniform per group
    uniform RefitTreelet* pTrltDsc = BVHBase_GetRefitTreeletDescs(bvh);

    bool should_we_process_treetip = true;
    local Treelet_by_single_group_locals loc;
    local bool* l_should_we_process_treetip = (local bool*)&loc;
#if REFIT_VERBOSE_LOG
    if (group_id != 0) return;
#endif

    if (bottoms_cnt > 1)
    {
#if REFIT_VERBOSE_LOG
        for (; group_id < bottoms_cnt; group_id++)
        {
            if (get_local_id(0) == 0) { printf("\n ====== treelet %d ====== \n", group_id); }
            work_group_barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE, memory_scope_device);
#endif
            bool rootProcThread = refit_treelet_by_single_group(
                bbox_scratch,
                &loc,
                bvh,
                pTrltDsc[group_id],
                true,
                false);

            // we have to make last group that finishes go up and process the treetip
            if (rootProcThread)
            {

                mem_fence_gpu_invalidate();
                uint finished_cnt = atomic_inc_global((global uint*) & bvh->refitTreeletCnt2);
                should_we_process_treetip = finished_cnt + 1 == bottoms_cnt;

                * l_should_we_process_treetip = should_we_process_treetip;

                if (should_we_process_treetip) mem_fence_gpu_invalidate();
            }
#if REFIT_VERBOSE_LOG
        }
#endif
        work_group_barrier(CLK_LOCAL_MEM_FENCE, memory_scope_work_group);

        should_we_process_treetip = *l_should_we_process_treetip;
    }
    
    if (should_we_process_treetip)
    {
        //this group will process treetip
        if (get_local_id(0) == 0) { bvh->refitTreeletCnt2 = 0; }    
        if (bottoms_cnt == 1) { bottoms_cnt = 0; }
        refit_treelet_by_single_group(
            bbox_scratch,
            &loc,
            bvh,
            pTrltDsc[bottoms_cnt],
            true,
            true);
    }
}

/*
  This kernel refit a BVH. The algorithm iterates over all BVH nodes
  to find all leaf nodes, which is where refitting starts. For these
  leaf nodes bounds get recalculated and then propagates up the tree.

  One kernel instance considers exactly one inner_node startpoint. 
  not range of inner nodes.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(8, 1, 1))) void kernel 
Refit_per_one_startpoint(
    global struct BVHBase* bvh,
    global GRL_RAYTRACING_GEOMETRY_DESC* geosArray,
    global struct AABB3f* instance_leaf_aabbs )
{
    /* here we temporarily store the bounds for the children of a node */
    struct AABB childrenAABB[BVH_NODE_N6];

    /* get pointer to inner nodes and back pointers */
    global struct QBVHNodeN* inner_nodes = BVHBase_rootNode(bvh);
    BackPointers* backPointers = BVHBase_GetBackPointers(bvh);
    
    /* get the inner node that we will consider as a bottom startpoint */
    const uint numInnerNodes = BVHBase_numNodes(bvh);
    const uint innerNodeIdx = (get_group_id(0) + 0) * get_local_size(0) + get_local_id(0);

    if (innerNodeIdx >= numInnerNodes) return;

    global struct QBVHNodeN* curNode = &inner_nodes[innerNodeIdx];
    uint numChildren = refit_bottom(
        bvh,
        geosArray,
        instance_leaf_aabbs,
        curNode,
        childrenAABB,
        *InnerNode_GetBackPointer(backPointers, innerNodeIdx));
        
    if (numChildren != 0)
    {
        /* update bounds of node */
        QBVHNodeN_setBounds(curNode, childrenAABB, numChildren);

        /* refit upper parts of the BVH */
        /* TODO: this will not gonna work for mixed nodes */
        refit_bottom_up(curNode, bvh, childrenAABB, numChildren);
    }
}

#endif

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(SG_REFIT_WG_SIZE, 1, 1))) void kernel
Refit_indirect_sg(
    global struct BVHBase* bvh,
    global GRL_RAYTRACING_GEOMETRY_DESC* geosArray,
    global struct AABB3f* instance_leaf_aabbs)
{    
    DO_Refit_per_one_startpoint_sg(bvh, geosArray, instance_leaf_aabbs, 0);

}
