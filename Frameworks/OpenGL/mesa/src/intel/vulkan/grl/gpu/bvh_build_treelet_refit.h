//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "bvh_build_refit.h"
#include "libs/lsc_intrinsics.h"


#define REFIT_DEBUG_CHECKS 0
#define REFIT_VERBOSE_LOG 0

#define NUM_STARTPOINTS_IN_SLM (1024)

GRL_INLINE void storeAABBToL1(struct AABB aabb, struct AABB* ptr)
{
    uint8 val = (uint8)(
        as_uint(aabb.lower.x), as_uint(aabb.lower.y), as_uint(aabb.lower.z), as_uint(aabb.lower.w),
        as_uint(aabb.upper.x), as_uint(aabb.upper.y), as_uint(aabb.upper.z), as_uint(aabb.upper.w));

    store_uint8_L1WB_L3WB((__global uint8*) ptr, 0, val);
}

GRL_INLINE void storeAABBToL3(struct AABB aabb, struct AABB* ptr)
{
    uint8 val = (uint8)(
        as_uint(aabb.lower.x), as_uint(aabb.lower.y), as_uint(aabb.lower.z), as_uint(aabb.lower.w),
        as_uint(aabb.upper.x), as_uint(aabb.upper.y), as_uint(aabb.upper.z), as_uint(aabb.upper.w));

    store_uint8_L1UC_L3WB((__global uint8*) ptr, 0, val);
}

typedef struct Treelet_by_single_group_locals
{
    uint   startpoints[NUM_STARTPOINTS_IN_SLM];
} Treelet_by_single_group_locals;

typedef struct SquashedInputGroupDesc {
    qword bvh;
    qword scratch;
    uint  groupInTree;
    uint  totalNumGroups; //valid only for 0th element in array, otherwise its trash padding
} SquashedInputGroupDesc;

//
//
// update primitives
//
//

typedef struct SquashedInput {
    global struct BVHBase* pBvh;
    global void* pInput;
    global struct AABB* bbox_scratch;
} SquashedInput;



// updates one quad leaf and gets BBOX contatining it
GRL_INLINE void refit_bottom_child_quad(
    global struct QuadLeaf* quad,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    struct AABB* childAABB)
{
    struct QuadLeaf Q;
    get_updated_quad(quad, geomDesc, &Q);
    quadCopyVertices(&Q, quad);
    *childAABB = getAABB_Quad((struct Quad*) & Q); // FIXME: support leaves with more than one quad
}

// procedurals will have to go old path at first
#if 0
// updates one procedural leaf and gets BBOX contatining it
GRL_INLINE void refit_bottom_child_procedural(
    global struct ProceduralLeaf** pleaf,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    struct AABB* childAABB)
{
    global struct ProceduralLeaf* leaf = *pleaf;
    /* extract geomID and primID from leaf */
    const uint startPrim = QBVHNodeN_startPrim(curNode, child_idx);
    const uint geomID = ProceduralLeaf_geomIndex(leaf);
    const uint primID = ProceduralLeaf_primIndex(leaf, startPrim); // FIXME: have to iterate over all primitives of leaf!

    /* read bounds from geometry descriptor */
    struct GRL_RAYTRACING_AABB aabb = GRL_load_aabb(&geomDesc[geomID], primID);
    childAABB->lower.x = aabb.MinX;
    childAABB->lower.y = aabb.MinY;
    childAABB->lower.z = aabb.MinZ;
    childAABB->upper.x = aabb.MaxX;
    childAABB->upper.y = aabb.MaxY;
    childAABB->upper.z = aabb.MaxZ;

    /* advance leaf pointer to next child */
    *pleaf = leaf + QBVHNodeN_blockIncr(curNode, child_idx);
}


GRL_INLINE void update_procedural_leafs(
    global struct BVHBase* bvh,
    global void* input,
    global struct AABB* bbox_scratch,
    uint id,
    uint num_done_by_one_thread)
{
    uint numLeaves = BVHBase_GetNumQuads(bvh);
    uint leafsIndexOffset = bvh->proceduralDataStart - BVH_ROOT_NODE_OFFSET / 64;
    global ProceduralLeaf* leafs = (global QuadLeaf*)BVHBase_GetProceduralLeaves(bvh);
    uint start_leaf = id * num_done_by_one_thread;
    uint end_leaf = min(start_leaf + num_done_by_one_thread, numLeaves);

    global GRL_RAYTRACING_GEOMETRY_DESC* geosArray = (global GRL_RAYTRACING_GEOMETRY_DESC*) input;

    for (uint leaf_id = start_leaf; leaf_id < end_leaf; leaf_id++)
    {
        struct AABB theAABB;
        refit_bottom_child_procedural(leafs + leaf_id, geosArray, &theAABB);
        theAABB.lower.w = as_float(0xABBADEFF);
        theAABB.upper.w = 0x00;
        storeAABBToL1(theAABB, &bbox[leafsIndexOffset + leaf_id]);
    }
}
#endif

GRL_INLINE void update_quads(
    global struct BVHBase* bvh,
    global void* input,
    global struct AABB* bbox_scratch,
    uint id,
    uint num_done_by_one_thread)
{
    uint numLeaves = BVHBase_GetNumQuads(bvh);
    uint leafsIndexOffset = bvh->quadLeafStart - BVH_ROOT_NODE_OFFSET / 64;
    global QuadLeaf* leafs = (global QuadLeaf*)BVHBase_GetQuadLeaves(bvh);
    uint start_leaf = id * num_done_by_one_thread;
    uint end_leaf = min(start_leaf + num_done_by_one_thread, numLeaves);

    global GRL_RAYTRACING_GEOMETRY_DESC* geosArray = (global GRL_RAYTRACING_GEOMETRY_DESC*) input;

    for (uint leaf_id = start_leaf; leaf_id < end_leaf; leaf_id++)
    {
        struct AABB theAABB;
        refit_bottom_child_quad(leafs + leaf_id, geosArray, &theAABB);
        theAABB.lower.w = as_float(0xABBADEFF);
        theAABB.upper.w = 0x00;
        storeAABBToL1(theAABB, &bbox_scratch[leafsIndexOffset + leaf_id]);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// core bottom-up update functions
//
//

GRL_INLINE void quantise_bounds(
    struct AABB* input_aabb, float3 len, float3 mant, float3 org, int3 exp,
    uchar3* lower_uchar,
    uchar3* upper_uchar)
{
    const float up = 1.0f + ulp;
    const float down = 1.0f - ulp;

    struct AABB child_aabb = conservativeAABB(input_aabb); // conservative ???

    float3 lower = floor(bitShiftLdexp3((child_aabb.lower.xyz - org) * down, -exp + 8));
    lower = clamp(lower, (float)(QUANT_MIN), (float)(QUANT_MAX));
    float3 upper = ceil(bitShiftLdexp3((child_aabb.upper.xyz - org) * up, -exp + 8));
    upper = clamp(upper, (float)(QUANT_MIN), (float)(QUANT_MAX));

    *lower_uchar = convert_uchar3_rtn(lower);
    *upper_uchar = convert_uchar3_rtp(upper);
}

typedef struct Qbounds_as_DW {
    uint32_t xLL; uint32_t xLU; uint32_t xUU;
    uint32_t yLL; uint32_t yLU; uint32_t yUU;
    uint32_t zLL; uint32_t zLU; uint32_t zUU;
} Qbounds_as_DW;

GRL_INLINE void encodeQuantisedDataAsDW(
    uchar3 lower_uchar,
    uchar3 upper_uchar,
    uint idx,
    Qbounds_as_DW* qbounds)
{
    uint shift_init = idx * 8;
    if (idx >= 4) {
        uint shift = (shift_init - 32);
        qbounds->xLU |= ((uint)lower_uchar.x) << shift;
        qbounds->yLU |= ((uint)lower_uchar.y) << shift;
        qbounds->zLU |= ((uint)lower_uchar.z) << shift;
    }
    else {
        qbounds->xLL |= ((uint)lower_uchar.x) << shift_init;
        qbounds->yLL |= ((uint)lower_uchar.y) << shift_init;
        qbounds->zLL |= ((uint)lower_uchar.z) << shift_init;
    }

    if (idx < 2) {
        uint shift = (shift_init + 16);
        qbounds->xLU |= ((uint)upper_uchar.x) << shift;
        qbounds->yLU |= ((uint)upper_uchar.y) << shift;
        qbounds->zLU |= ((uint)upper_uchar.z) << shift;
    }
    else {
        uint shift = (shift_init - 16);
        
        qbounds->xUU |= ((uint)upper_uchar.x) << shift;
        qbounds->yUU |= ((uint)upper_uchar.y) << shift;
        qbounds->zUU |= ((uint)upper_uchar.z) << shift;
    }
}

GRL_INLINE void encodeChildBounds(uchar3 lower_uchar, uchar3 upper_uchar, uint ch, struct InternalNode* qnode)
{
    qnode->lower_x[ch] = lower_uchar.x; qnode->upper_x[ch] = upper_uchar.x;
    qnode->lower_y[ch] = lower_uchar.y; qnode->upper_y[ch] = upper_uchar.y;
    qnode->lower_z[ch] = lower_uchar.z; qnode->upper_z[ch] = upper_uchar.z;
}
    

GRL_INLINE GRL_OVERLOADABLE void InternalNode_setBounds_skip_prev(struct InternalNode* qbvh_node, uint prevChildIdx, struct AABB* prev_input_aabb, struct AABB* input_aabb, uint childrenIndex, const uint numChildren, struct AABB* aabb_reduced)
{
    
    int3 exp;
    const float up = 1.0f + ulp;
    struct AABB conservative_aabb = conservativeAABB(aabb_reduced);
    const float3 len = AABB_size(&conservative_aabb).xyz * up;
    const float3 mant = frexp_vec3(len, &exp);
    const float3 org = conservative_aabb.lower.xyz;

    exp += (mant > (float3)QUANT_MAX_MANT ? (int3)1 : (int3)0);

    qbvh_node->lower[0] = org.x; qbvh_node->lower[1] = org.y; qbvh_node->lower[2] = org.z;

    qbvh_node->exp_x = exp.x; qbvh_node->exp_y = exp.y;  qbvh_node->exp_z = exp.z;
    
    Qbounds_as_DW qbounds = { 0x0 };


    {
        uchar3 lower_uchar, upper_uchar;
        quantise_bounds(prev_input_aabb, len, mant, org, exp, &lower_uchar, &upper_uchar);

        //encode invalid children. its enough to set 0x80 as lower_x bytes
        uint shift = numChildren * 8;
        uint shift2 = min(shift, 31u);
        qbounds.xLL = (0x80808080u << shift2);
        uint shift3 = max(shift, 32u) - 32;
        qbounds.xLU = (ushort)(((ushort)0x8080) << (ushort)shift3);

        encodeQuantisedDataAsDW(lower_uchar, upper_uchar, prevChildIdx, &qbounds);
        //encodeChildBounds(lower_uchar, upper_uchar, prevChildIdx, qbvh_node);
    }

    uint ch = prevChildIdx == 0;
    while (ch < numChildren) {
        uchar3 lower_uchar, upper_uchar;
        quantise_bounds(input_aabb + ch, len, mant, org, exp, &lower_uchar, &upper_uchar);
        encodeQuantisedDataAsDW(lower_uchar, upper_uchar, ch, &qbounds);
        //encodeChildBounds(lower_uchar, upper_uchar, ch, qbvh_node);
        ch += 1 + (prevChildIdx == (ch + 1));
    }
    Qbounds_as_DW* qbounds_dst = (Qbounds_as_DW*)(&qbvh_node->lower_x[0]);
    *qbounds_dst = qbounds;
    return;
}

GRL_INLINE struct AABB refitReduce2Boxes(struct AABB A, struct AABB B)
{
    AABB_extend(&A, &B);
    // to make it work for TLAS node masks change to this:
    // A.lower.w = as_float(as_uint(A.lower.w) | as_uint(B.lower.w));
    A.lower.w = as_float(0xABBADE00u);
    return A;
}

GRL_INLINE void refitReduceNodePrev(
    uint prevIdx,
    uint leadChildIdx,
    uint numChildren,
    struct AABB* globalBox,
    struct AABB* reduceBox,
    uint depth,
    uint NodeIndex)
{
    uint8_t childIgnored = (prevIdx - leadChildIdx);

#   if REFIT_DEBUG_CHECKS
    bool err = false;
    if ((as_uint(reduceBox->lower.w) & 0xFFFFFF00) != 0xABBADE00u)
    {
        printf("refitReduceNode6 (loc_id %d): prev (used as child %d) not updated! NodeIndex %d, child nodeIdx %d at depth %d\n",
            get_local_id(0),
            childIgnored,
            NodeIndex,
            prevIdx,
            depth);
        err = true;
    }

    if ((as_uint(globalBox[NodeIndex].lower.w) & 0xFFFFFF00) == 0xABBADE00u)
    {
        printf("refitReduceNode6 (loc_id %d): dst node already updated. NodeIndex %d depth %d\n",
            get_local_id(0),
            NodeIndex,
            depth);
    }

    bool fail = false;
    for (uint k = 0; (k < numChildren) && !err; ++k) {
        if (k != childIgnored) {
            if ((as_uint(globalBox[leadChildIdx + k].lower.w) & 0xFFFFFF00) != 0xABBADE00u) {
                printf("refitReduceNode6 (loc_id %d): child %d not updated! use prev %d, NodeIndex %d, child nodeIdx %d at depth %d\n",
                    get_local_id(0),
                    k,
                    prevIdx - leadChildIdx,
                    NodeIndex,
                    leadChildIdx + k,
                    depth);
                fail = true;   
            }
        }
    }
    err |= fail;
#   endif

    // for each child 3 bits contains load index
    const uint32_t indicesEncoded =
        (1 << 0) +
        (2 << 3) +
        (3 << 6) +
        (4 << 9) +
        (5 << 12) +
        (0 << 15) +
        (1 << 18) +
        (2 << 21) +
        (3 << 24) +
        (4 << 27);
    // 1,2,3,4,5


    uint32_t indicesEncodedShifted = indicesEncoded >> (childIgnored * 3);

    struct AABB* childAABB = globalBox + leadChildIdx;
    struct AABB  temp = childAABB[indicesEncodedShifted & 7];
    indicesEncodedShifted >>= 3;
    struct AABB* nextChild = childAABB + (indicesEncodedShifted & 7);
    struct AABB  backlog = temp;

    for (uint child = 2; child < numChildren; child++)
    {
        temp = *nextChild;
        *reduceBox = refitReduce2Boxes(*reduceBox, backlog);
        indicesEncodedShifted >>= 3;
        nextChild = childAABB + (indicesEncodedShifted & 7);
        backlog = temp;
    }

    *reduceBox = refitReduce2Boxes(*reduceBox, backlog);

#if REFIT_DEBUG_CHECKS
    for (uint k = 0; (k < numChildren) && !err; ++k) {
        if (k != childIgnored) {
            if (!AABB_subset(&globalBox[leadChildIdx + k], reduceBox)) {
                printf("refitReduceNode6 (loc_id %d): child AABB %d/%d reduction went wrong! skipped prev %d, NodeIndex %d, child nodeIdx %d at depth %d\n",
                    get_local_id(0),
                    k, numChildren,
                    prevIdx - leadChildIdx,
                    NodeIndex,
                    leadChildIdx + k,
                    depth);

                err = true;
            }
        }
    }
    if (!err && ((as_uint(reduceBox->lower.w) & 0xFFFFFF00) != 0xABBADE00u)) {
        printf("refitReduceNode6: havent set the 0xABBADEXXu marker in result node %d at depth %d!\n",
            NodeIndex,
            depth);
    }
#endif
}


GRL_INLINE uint hash_local_id()
{
    return get_sub_group_local_id() * get_num_sub_groups() + get_sub_group_id();
}

//===============================================================
//
//  Core update function
//
//===============================================================
GRL_INLINE bool refit_treelet_by_single_group(
    global  struct AABB* bbox,
    local Treelet_by_single_group_locals* loc,
    uniform global BVHBase* pBvh,
    uniform RefitTreelet   trltDsc,
    bool encodeQnodes,
    bool isTipTreelet)
{
    BackPointers* backpointers = BVHBase_GetBackPointers(pBvh);    
    InternalNode* internalNodes = BVHBase_GetInternalNodes(pBvh);
    uint local_id = get_local_id(0);   
    StartPoint* startPoints = BVHBase_GetRefitStartPoints(pBvh) + trltDsc.startpoint_offset;
    
    // special case for single path treelets, TODO rewrite it as subgroups based
    if (trltDsc.numStartpoints == 1) {
        if (local_id == 0) {
            RefitTreeletTrivial desc = *((RefitTreeletTrivial*)& trltDsc);
            uint innerNodeIdx   = desc.theOnlyNodeIndex;
            uint numChildren    = desc.numChildrenOfTheNode;
            uint childIndex     = desc.childrenOffsetOfTheNode;
            uint maxDepth       = desc.maxDepth;

            uint prevIdx = childIndex;
            struct AABB myBox = bbox[childIndex];
            struct AABB prevAABB;
            uint backpointer = maxDepth > 0 ? *InnerNode_GetBackPointer(backpointers, innerNodeIdx) : 0;
            InternalNode* curNode = internalNodes + innerNodeIdx;
            uint currDepth = 0;
            
            while (1)
            {
                prevAABB = myBox;
                if (numChildren > 1) { refitReduceNodePrev(prevIdx, childIndex, numChildren, bbox, &myBox, 0, innerNodeIdx); }
                
                if (!encodeQnodes) { myBox.upper.w = encodeQnodes ? 0 : as_float(numChildren + (childIndex << 4)); }
                
                if (++currDepth > maxDepth) { break; }

                if (encodeQnodes) {
                    InternalNode_setBounds_skip_prev(curNode, prevIdx - childIndex, &prevAABB, bbox + childIndex, childIndex, numChildren, &myBox); 
                }
#if !REFIT_DEBUG_CHECKS
                else
#endif
                { storeAABBToL1(myBox, &bbox[innerNodeIdx]); }

                prevIdx = innerNodeIdx;
                innerNodeIdx = BackPointer_GetParentIndex(backpointer);
                backpointer = *InnerNode_GetBackPointer(backpointers, innerNodeIdx);
                numChildren = BackPointer_GetNumChildren(backpointer);
                curNode = internalNodes + innerNodeIdx;
                childIndex = innerNodeIdx + curNode->childOffset;
            }

            if (isTipTreelet) {
                AABB3f reduced3f = AABB3fFromAABB(myBox);
                pBvh->Meta.bounds = reduced3f;
            }
            else {
                storeAABBToL3(myBox, &bbox[innerNodeIdx]);
            }

            if (encodeQnodes || isTipTreelet) {
                InternalNode_setBounds_skip_prev(curNode, prevIdx - childIndex, &prevAABB, bbox + childIndex, childIndex, numChildren, &myBox);
            }
            
#if REFIT_VERBOSE_LOG
            printf("single node treelet: storing node idx %d \n", innerNodeIdx);
#endif
        }
        
        return local_id == 0;
    }

    local uint* loc_startpoints = loc->startpoints;
    

#if REFIT_DEBUG_CHECKS
    if ((trltDsc.numNonTrivialStartpoints > NUM_STARTPOINTS_IN_SLM)) {
        if(local_id == 0) printf("out of SLM space, trltDsc.depthSub_NUM_STARTPOINTS_IN_SLM > 0\n");
        return local_id == 0;
    }
#endif

    uint SLMedStartpointsOffset = trltDsc.numStartpoints - trltDsc.numNonTrivialStartpoints;

    /*=====================================================================
    first phase where we update startpoints nodes only
    ----------------------------------------------------------------------*/
    for (uint startpoint_i = local_id; startpoint_i < trltDsc.numStartpoints; startpoint_i += get_local_size(0)) {
        uint startpoint = (uint)intel_sub_group_block_read_ui((global uint*)(startPoints + startpoint_i));
        uint innerNodeIdx = StartPoint_GetNodeIdx(startpoint);
        uint backpointer = *InnerNode_GetBackPointer(backpointers, innerNodeIdx);
        if (startpoint_i >= SLMedStartpointsOffset) {
            uint idx = startpoint_i - SLMedStartpointsOffset;
            loc_startpoints[idx] = (BackPointer_GetParentIndex(backpointer) << 6) | StartPoint_GetDepth(startpoint);
        }
        
        uint numChildren = BackPointer_GetNumChildren(backpointer);
        InternalNode* curNode = internalNodes + innerNodeIdx;
        uint childIndex = innerNodeIdx + curNode->childOffset;
        
        uint prevIdx = childIndex;
        struct AABB myBox = bbox[childIndex];
        struct AABB prevAABB = myBox;

#   if REFIT_DEBUG_CHECKS
        if (numChildren == 0) {
            printf("this node has no chidren!\n", 0);
            AABB_init(&myBox);
        }
#   endif
        
        if (numChildren > 1) { refitReduceNodePrev(prevIdx, childIndex, numChildren, bbox, &myBox, 0, innerNodeIdx); }
        myBox.upper.w = encodeQnodes ? 0 : as_float(numChildren + (childIndex << 4));
        
#if REFIT_VERBOSE_LOG
        printf("init phase: at depth 0 storing node idx %d \n", innerNodeIdx);
#endif
        storeAABBToL1(myBox, &bbox[innerNodeIdx]);

        if (encodeQnodes) {
            InternalNode_setBounds_skip_prev(curNode, 0, &prevAABB, bbox + childIndex, childIndex, numChildren, &myBox);
        }
    }

    uniform uint CurrPeeledDepth = 1;
    uniform uint numStartpoints = trltDsc.numNonTrivialStartpoints;
    uint nextFloorStartpoint = hash_local_id();

    uint depthOnionEnd = trltDsc.depthLess64;
    if (get_local_size(0) == 128) { depthOnionEnd = trltDsc.depthLess128; }
    if (get_local_size(0) == 256) { depthOnionEnd = trltDsc.depthLess256; }

    /*=====================================================================
    second phase, we update horizontally untill 
    we reach number of active path below grou size
    ----------------------------------------------------------------------*/
    while (CurrPeeledDepth < depthOnionEnd) {
        mem_fence_workgroup_default();

        work_group_barrier(CLK_LOCAL_MEM_FENCE, memory_scope_work_group);
        uint start = nextFloorStartpoint;
        nextFloorStartpoint = numStartpoints;

        for (uint startpoint_i = start; startpoint_i < numStartpoints; startpoint_i += get_local_size(0)) {
            uint startpoint   = loc_startpoints[startpoint_i];
            uint innerNodeIdx = StartPoint_GetNodeIdx(startpoint);
            uint backpointer  = *InnerNode_GetBackPointer(backpointers, innerNodeIdx);

            if (StartPoint_GetDepth(startpoint) > CurrPeeledDepth) {
                StartPoint newSP = (BackPointer_GetParentIndex(backpointer) << 6) | StartPoint_GetDepth(startpoint);
                loc_startpoints[startpoint_i] = newSP;
                nextFloorStartpoint = min(nextFloorStartpoint, startpoint_i);
            }

            InternalNode* curNode = internalNodes + innerNodeIdx;
            uint childIndex = innerNodeIdx + curNode->childOffset;
            uint numChildren = BackPointer_GetNumChildren(backpointer);

            uint prevIdx = childIndex;
            struct AABB myBox = bbox[childIndex];
            struct AABB prevAABB = myBox;
            refitReduceNodePrev(prevIdx, childIndex, numChildren, bbox, &myBox, CurrPeeledDepth, innerNodeIdx);

            myBox.upper.w = encodeQnodes ? 0 : as_float(numChildren + (childIndex << 4));

#if REFIT_VERBOSE_LOG
            printf("onion: startpoint %d <n=%d , d=%d> at depth %d storing node idx %d \n", startpoint_i, StartPoint_GetNodeIdx(startpoint), StartPoint_GetDepth(startpoint), CurrPeeledDepth, innerNodeIdx);
#endif
            storeAABBToL1(myBox, &bbox[innerNodeIdx]);
            if (encodeQnodes) {
                InternalNode_setBounds_skip_prev(curNode, 0, &prevAABB, bbox + childIndex, childIndex, numChildren, &myBox);
            }
        }
        CurrPeeledDepth++;
    }

    uint startpoint_idx = nextFloorStartpoint;
    bool active = startpoint_idx < numStartpoints;

    work_group_barrier(CLK_LOCAL_MEM_FENCE, memory_scope_work_group);
    StartPoint startpoint = loc_startpoints[startpoint_idx];

    struct AABB myBox;
    uint prevIdx = 0;
    uint innerNodeIdx = StartPoint_GetNodeIdx(startpoint);

    /*=====================================================================
    last phase, each thread just continues path to its end
    
    only thread that computes the longest path leaves prematurely 
    (thats why while condition isn't <=) the code for finalizing root of treelet
    is special and hendled afterwards

    TODO: with proper assigning of paths to lanes we should reach only three
    active lanes per physical thread quite soon for this subgroups could be used 
    ----------------------------------------------------------------------*/
    bool prevActive = active;
    while (CurrPeeledDepth < trltDsc.maxDepth) {
        uint backpointer;
        uint childIndex;
        InternalNode* curNode = internalNodes + innerNodeIdx;
        if (active) {
            childIndex = innerNodeIdx + curNode->childOffset;
            backpointer = *InnerNode_GetBackPointer(backpointers, innerNodeIdx);
        } else if(prevActive){
            mem_fence_workgroup_default();
        }

        prevActive = active;

        work_group_barrier(0, memory_scope_work_group);
        //printf("Start node %d at depth %d, innerNodeIdx %d dying! \n", StartPoint_GetNodeIdx(startpoint), CurrPeeledDepth, innerNodeIdx);
        if (active) {

#if REFIT_DEBUG_CHECKS
            if (CurrPeeledDepth > StartPoint_GetDepth(startpoint))
            {
                printf("uppath: startpoint %d <n=%d , d=%d> at depth %d shouldn't be active!\n", startpoint_idx, StartPoint_GetNodeIdx(startpoint), StartPoint_GetDepth(startpoint), CurrPeeledDepth);
            }
#endif
            if (prevIdx == 0) {
                myBox = bbox[childIndex];
                prevIdx = childIndex;
            }
            uint numChildren = BackPointer_GetNumChildren(backpointer);

            struct AABB prevAABB = myBox;
            refitReduceNodePrev(prevIdx, childIndex, numChildren, bbox, &myBox, CurrPeeledDepth, innerNodeIdx);
            myBox.upper.w = encodeQnodes ? 0 : as_float(numChildren + (childIndex << 4));
#if REFIT_VERBOSE_LOG
            printf("uppath: startpoint %d <n=%d , d=%d> at depth %d storing node idx %d \n", startpoint_idx, StartPoint_GetNodeIdx(startpoint), StartPoint_GetDepth(startpoint), CurrPeeledDepth, innerNodeIdx);
#endif
            active = CurrPeeledDepth < StartPoint_GetDepth(startpoint);

            if (encodeQnodes) {
#if !REFIT_DEBUG_CHECKS
                if (!active)
#endif
                { storeAABBToL1(myBox, &bbox[innerNodeIdx]); }
                InternalNode_setBounds_skip_prev(curNode, prevIdx - childIndex, &prevAABB, bbox + childIndex, childIndex, numChildren, &myBox);
            } else {
                storeAABBToL1(myBox, &bbox[innerNodeIdx]);
            }

            prevIdx = innerNodeIdx;
            innerNodeIdx = BackPointer_GetParentIndex(backpointer);
        }

        CurrPeeledDepth++;
    }

    {
        uint backpointer;
        uint childIndex;
        InternalNode* curNode = internalNodes + innerNodeIdx;
        if (active) {
            childIndex = innerNodeIdx + curNode->childOffset;
            backpointer = *InnerNode_GetBackPointer(backpointers, innerNodeIdx);
        } else if(prevActive) {
            mem_fence_workgroup_default();
        }

        work_group_barrier(0, memory_scope_work_group);

        /*=====================================================================
        final step, is special processing of root,
        its different, since its box is transfered cross group (written to L3)
        or is root of whole tree and hence fill global box in bvh MD
        TODO: this should be done in SG as only one thread is active
        ----------------------------------------------------------------------*/
        if (active) {
            if (prevIdx == 0) {
                myBox = bbox[childIndex];
                prevIdx = childIndex;
            }
            uint numChildren = BackPointer_GetNumChildren(backpointer);
            struct AABB prevAABB = myBox;
            refitReduceNodePrev(prevIdx, childIndex, numChildren, bbox, &myBox, CurrPeeledDepth, innerNodeIdx);
            myBox.upper.w = encodeQnodes ? 0 : as_float(numChildren + (childIndex << 4));
            
#if REFIT_VERBOSE_LOG
            printf("root: startpoint %d <n=%d , d=%d> at depth %d storing node idx %d \n", startpoint_idx, StartPoint_GetNodeIdx(startpoint), StartPoint_GetDepth(startpoint), CurrPeeledDepth, innerNodeIdx/*,WeReInSIMD*/);
#endif
            if (isTipTreelet) {
                AABB3f reduced3f = AABB3fFromAABB(myBox);
                pBvh->Meta.bounds = reduced3f;
                InternalNode_setBounds_skip_prev(curNode, prevIdx - childIndex, &prevAABB, bbox + childIndex, childIndex, numChildren, &myBox);
            } else {
                storeAABBToL3(myBox, &bbox[innerNodeIdx]);
                if (encodeQnodes) {
                    InternalNode_setBounds_skip_prev(curNode, prevIdx - childIndex, &prevAABB, bbox + childIndex, childIndex, numChildren, &myBox);
                }
            }
        }
    }

    return active;
}


//////////////////////////////////////////////////////////////////////////////////////
//
// Internal nodes enocding as a separate dispatch
//
//

// encode qnodes as a separate pass
GRL_INLINE void post_refit_encode_qnode_tree_per_group(
    global struct AABB* bbox_scratch,
    global struct BVHBase* bvh)
{
    uint numInnerNodes = BVHBase_GetNumInternalNodes(bvh);
    InternalNode* internalNodes = BVHBase_GetInternalNodes(bvh);

    for (uint nodeIdx = get_local_id(0) + 1 /*+1 because node 0 is already updated*/; nodeIdx < numInnerNodes; nodeIdx += get_local_size(0))
    {
        struct AABB reduced = bbox_scratch[nodeIdx];
#   if REFIT_DEBUG_CHECKS
        if ((as_uint(reduced.lower.w) & 0xFFFFFF00) != 0xABBADE00u) {
            printf("qnode enc group: NodeIndex %d not updated! \n", nodeIdx);
            return;
        }
        for (uint k = 0; k < (as_uint(reduced.upper.w) & 7); ++k) {
            uint childIdx = (as_uint(reduced.upper.w) >> 4) + k;
            if ((as_uint(bbox_scratch[childIdx].lower.w) & 0xFFFFFF00) != 0xABBADE00u) {
                printf("qnode enc group: child not updated! NodeIndex %d, child nodeIdx %d \n", nodeIdx, childIdx);
                return;
            }
        }
#   endif
        struct InternalNode* qbvh_node = internalNodes + nodeIdx;
        uint childIndex = as_uint(reduced.upper.w) >> 4;
        uint numChildren = as_uint(reduced.upper.w) & 7;
        struct AABB* children = bbox_scratch + childIndex;
        //InternalNode_setBounds(internalNodes + nodeIdx, bbox_scratch + (as_uint(reduced.upper.w) >> 4), as_uint(reduced.upper.w) & 7, &reduced);
        InternalNode_setBounds_skip_prev(qbvh_node, 0, children, children, childIndex, numChildren, &reduced);
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//
// Construction of treelets and paths
//
//

// this is tiny bit tricky, when bottom-up thread haven't yet closed treelet this is number of startpoints that are under the node
// when thread closed treelets it the data is starts to be treelet ID
typedef uint TreeletNodeData;

typedef struct TreeletsOpenNodeInfo {
    // bool isTreeletRoot; // : 1 
    short   maxDepth;      // : 14
    uint    numStartpoints;// : 16
} TreeletsOpenNodeInfo;

typedef struct TreeletsClosedNodeInfo {
    // bool isTreeletRoot; // : 1 
    uint    treeletId;     // : 31 (when treelet is closed)
} TreeletsClosedNodeInfo;

GRL_INLINE TreeletNodeData ClearTreeletRoot(TreeletNodeData D)
{
    return D & ((1u << 31u) - 1u);
}

GRL_INLINE uint isTreeletRoot(TreeletNodeData E)
{
    return E >> 31;
}

GRL_INLINE uint getNumStartpoints(TreeletNodeData E)
{
    return E & ((1 << 16) - 1);
}

GRL_INLINE uint getMaxDepth(TreeletNodeData E)
{
    return (E >> 16) & ((1 << 14) - 1);
}

// single startpoint treelet
GRL_INLINE uint isTrivialTreeletRoot(TreeletNodeData E)
{
    return (E >> 31) && (getMaxDepth(E) == 0);
}

GRL_INLINE TreeletNodeData SetTipStartpoint(TreeletNodeData D)
{
    return ClearTreeletRoot(D) | (1 << 30);
}

GRL_INLINE TreeletNodeData SetTreeletRoot(TreeletNodeData D)
{
    return D | (1 << 31);
}

GRL_INLINE TreeletsOpenNodeInfo DecodeOpenInfo(TreeletNodeData E)
{
    TreeletsOpenNodeInfo I;
    I.maxDepth = getMaxDepth(E);
    I.numStartpoints = getNumStartpoints(E);
    return I;
}

GRL_INLINE TreeletNodeData EncodeOpenInfo(TreeletsOpenNodeInfo I, bool isRoot)
{
    TreeletNodeData D = isRoot ? (1 << 31) : 0;
    D |= (I.maxDepth & ((1 << 14) - 1)) << 16;
    D |= I.numStartpoints & ((1 << 16) - 1);
    return D;
}

GRL_INLINE TreeletsClosedNodeInfo DecodeClosedInfo(TreeletNodeData E)
{
    TreeletsClosedNodeInfo I;
    I.treeletId = E & ((1u << 31u) - 1u);
    return I;
}

GRL_INLINE TreeletNodeData GRL_OVERLOADABLE EncodeClosedInfo(TreeletsClosedNodeInfo I)
{
    TreeletNodeData D = (1u << 31u); // closed is always a root!
    D |= I.treeletId & ((1u << 31u) - 1u);
    return D;
}

GRL_INLINE TreeletNodeData GRL_OVERLOADABLE EncodeClosedInfo(uint treeletId)
{
    TreeletNodeData D = (1 << 31); // closed is always a root!
    D |= treeletId & ((1u << 31u) - 1u);
    return D;
}

GRL_INLINE void chk_close_Treelet(
    RefitTreelet* TreeletDescsArr,
    TreeletNodeData* nodeTreeletDataArr,
    uint* StartPointBuffer,
    uint* currStartpoint,
    TreeletNodeData nodeData,
    TreeletsOpenNodeInfo* nodeOpenInfo,
    uint nodeIdx,
    uint* treeletDescIdx)
{
    if (isTreeletRoot(nodeData))
    {
        TreeletNodeData encoded = 0;
        if (nodeOpenInfo->numStartpoints == 1)
        {
            encoded = ClearTreeletRoot(SetTipStartpoint(nodeData));
        }
        else
        {
            RefitTreelet RTdesc;
            RTdesc.startpoint_offset = *currStartpoint;
            *currStartpoint += nodeOpenInfo->numStartpoints;
            RTdesc.numStartpoints = nodeOpenInfo->numStartpoints;
            RTdesc.maxDepth = nodeOpenInfo->maxDepth;
            TreeletDescsArr[*treeletDescIdx] = RTdesc;
            encoded = EncodeClosedInfo(*treeletDescIdx);
            *treeletDescIdx = *treeletDescIdx + 1;
            TreeletsOpenNodeInfo infoDefault = { 0, 0 };
            *nodeOpenInfo = infoDefault;
        }

        nodeTreeletDataArr[nodeIdx] = encoded;
    }
    // printf("close_Treelet %d, nodeOpenInfo.numStartpoints %d, RTdesc.maxDepth %d, RTdesc.startpoint_offset %d\n", treeletDescIdx, nodeOpenInfo.numStartpoints, RTdesc.maxDepth, RTdesc.startpoint_offset);
}


// TreeletNodeData* treelets holds per node property, after running this some of them are marked as treelet root
GRL_INLINE void treelet_bottom_up_mark_treelets(
    global struct BVHBase* bvh,
    global InternalNode* internalNodes,
    global StartPoint* scratch_startpoints,
    uint curNodeIndex,
    BackPointers* backPointers,
    global TreeletNodeData* treelets,
    uint refitTreeletsDataStart,
    uint* startpointAlloc)
{
    TreeletsOpenNodeInfo currInfo;
    currInfo.maxDepth = 0;
    currInfo.numStartpoints = 1;

    global RefitTreelet* treeletDescs = (global RefitTreelet*) (((global char*)bvh) + (refitTreeletsDataStart * 64));

    treelets[curNodeIndex] = EncodeOpenInfo(currInfo, true);

    /* the start node got already processed, thus go to its parent node */
    uint parentPointer = *InnerNode_GetBackPointer(backPointers, curNodeIndex);
    curNodeIndex = parentPointer >> 6;

    bool isInTip = false;
    while (curNodeIndex != 0x03FFFFFF)
    {
        uint numChildrenTotal = 0;
        // numChildrenTotal and parentPointer gets updated...
        // atomic trickery, on backpointers, only the last one thread enters up
        {
            /* increment refit counter that counts refitted children of current node */
            global uint* pCurrentBackpointer = (global uint*)InnerNode_GetBackPointer(backPointers, curNodeIndex);
            mem_fence_gpu_invalidate();
            parentPointer = 1 + atomic_inc_global(pCurrentBackpointer);

            /* if all children got refitted, then continue */
            const uint numChildrenRefitted = (parentPointer >> 0) & 0x7;
            numChildrenTotal = (parentPointer >> 3) & 0x7;

            if (numChildrenRefitted != numChildrenTotal)
                return;

            /* reset refit counter for next refit */
            *pCurrentBackpointer = (parentPointer & 0xfffffff8);
        }

        /* get children treelets */
        global struct InternalNode* node = internalNodes + curNodeIndex;
        uint childrenIndices = curNodeIndex + node->childOffset;
        global TreeletNodeData* childrenTreelets = treelets + childrenIndices;

        // yeah, it is possible we are pulling trash here, but we wont use it.
        // this is for the sake of one non control flow spoiled data pull
        TreeletNodeData dataCh0 = childrenTreelets[0]; TreeletNodeData dataCh1 = childrenTreelets[1];
        TreeletNodeData dataCh2 = childrenTreelets[2]; TreeletNodeData dataCh3 = childrenTreelets[3];
        TreeletNodeData dataCh4 = childrenTreelets[4]; TreeletNodeData dataCh5 = childrenTreelets[5];

        // zero out the potential trash
        if (numChildrenTotal < 3) dataCh2 = 0;
        if (numChildrenTotal < 4) dataCh3 = 0;
        if (numChildrenTotal < 5) dataCh4 = 0;
        if (numChildrenTotal < 6) dataCh5 = 0;

        TreeletsOpenNodeInfo infoCh0 = DecodeOpenInfo(dataCh0);
        TreeletsOpenNodeInfo infoCh1 = DecodeOpenInfo(dataCh1);
        TreeletsOpenNodeInfo infoCh2 = DecodeOpenInfo(dataCh2);
        TreeletsOpenNodeInfo infoCh3 = DecodeOpenInfo(dataCh3);
        TreeletsOpenNodeInfo infoCh4 = DecodeOpenInfo(dataCh4);
        TreeletsOpenNodeInfo infoCh5 = DecodeOpenInfo(dataCh5);

        uint numChildrenBeingRoots = isTreeletRoot(dataCh0) + isTreeletRoot(dataCh1) + isTreeletRoot(dataCh2) + isTreeletRoot(dataCh3) + isTreeletRoot(dataCh4) + isTreeletRoot(dataCh5);
        // see if we should merge the trees, if not then we should move to tip.
        currInfo.numStartpoints = infoCh0.numStartpoints + infoCh1.numStartpoints + infoCh2.numStartpoints + infoCh3.numStartpoints + infoCh4.numStartpoints + infoCh5.numStartpoints;

        bool isTipStartpoint = false;
        if (!isInTip)
        {
            // TODO: threshold could be a dynamic parameter based on the number of actual inner nodes
            bool mergeTreelets = ((currInfo.numStartpoints > 0) && (currInfo.numStartpoints < TREELET_NUM_STARTPOINTS));
            bool allChildrenRootsCurrently = numChildrenTotal == numChildrenBeingRoots;
            if (mergeTreelets && allChildrenRootsCurrently)
            {
                childrenTreelets[0] = ClearTreeletRoot(dataCh0);
                childrenTreelets[1] = ClearTreeletRoot(dataCh1); // -1 will be recognised then as this is not a treelet root.
                if (numChildrenTotal > 2) childrenTreelets[2] = ClearTreeletRoot(dataCh2);
                if (numChildrenTotal > 3) childrenTreelets[3] = ClearTreeletRoot(dataCh3);
                if (numChildrenTotal > 4) childrenTreelets[4] = ClearTreeletRoot(dataCh4);
                if (numChildrenTotal > 5) childrenTreelets[5] = ClearTreeletRoot(dataCh5);
            }
            else
            {
                isInTip = true;
                isTipStartpoint = allChildrenRootsCurrently;
            }
        }

        // close any roots underneath
        if (isInTip && numChildrenBeingRoots)
        {
            uint trivialRoots = isTrivialTreeletRoot(dataCh0) + isTrivialTreeletRoot(dataCh1) + isTrivialTreeletRoot(dataCh2) +
                                isTrivialTreeletRoot(dataCh3) + isTrivialTreeletRoot(dataCh4) + isTrivialTreeletRoot(dataCh5);

            uint treeletId = 0;
            uint bottomStartpointSpace = 0;

            uint startpointsFromTiptree = trivialRoots;

            if (trivialRoots) isTipStartpoint = false;

            if (numChildrenBeingRoots > trivialRoots)
            {
                startpointsFromTiptree += // startpoint ONLY from tiptree
                    (1 - isTreeletRoot(dataCh0)) * infoCh0.numStartpoints +
                    (1 - isTreeletRoot(dataCh1)) * infoCh1.numStartpoints +
                    (1 - isTreeletRoot(dataCh2)) * infoCh2.numStartpoints +
                    (1 - isTreeletRoot(dataCh3)) * infoCh3.numStartpoints +
                    (1 - isTreeletRoot(dataCh4)) * infoCh4.numStartpoints +
                    (1 - isTreeletRoot(dataCh5)) * infoCh5.numStartpoints;
                    
                treeletId = atomic_add_global((global uint*)BVHBase_GetRefitTreeletCntPtr(bvh), numChildrenBeingRoots - trivialRoots);
                bottomStartpointSpace = atomic_add_global((global uint*)startpointAlloc, currInfo.numStartpoints - startpointsFromTiptree);
            }

            currInfo.numStartpoints = startpointsFromTiptree;

            chk_close_Treelet(treeletDescs, treelets, scratch_startpoints, &bottomStartpointSpace, dataCh0, &infoCh0, childrenIndices + 0, &treeletId);
            chk_close_Treelet(treeletDescs, treelets, scratch_startpoints, &bottomStartpointSpace, dataCh1, &infoCh1, childrenIndices + 1, &treeletId);
            chk_close_Treelet(treeletDescs, treelets, scratch_startpoints, &bottomStartpointSpace, dataCh2, &infoCh2, childrenIndices + 2, &treeletId);
            chk_close_Treelet(treeletDescs, treelets, scratch_startpoints, &bottomStartpointSpace, dataCh3, &infoCh3, childrenIndices + 3, &treeletId);
            chk_close_Treelet(treeletDescs, treelets, scratch_startpoints, &bottomStartpointSpace, dataCh4, &infoCh4, childrenIndices + 4, &treeletId);
            chk_close_Treelet(treeletDescs, treelets, scratch_startpoints, &bottomStartpointSpace, dataCh5, &infoCh5, childrenIndices + 5, &treeletId);
        }

        if (isTipStartpoint)
        {
            currInfo.maxDepth = 0;
            currInfo.numStartpoints = 1;
        }
        else
        {
            // reduce max depth and number of startpoint underneath
            currInfo.maxDepth = max(max(max(infoCh0.maxDepth, infoCh1.maxDepth),
                max(infoCh2.maxDepth, infoCh3.maxDepth)),
                max(infoCh4.maxDepth, infoCh5.maxDepth)) + 1;
        }

        treelets[curNodeIndex] = EncodeOpenInfo(
            currInfo,
            !isInTip /*mark marged treelet as an new root iff we are in bottom we */);

        /* make parent node the current node */
        curNodeIndex = parentPointer >> 6;
    }

    uint treeletId = *BVHBase_GetRefitTreeletCntPtr(bvh);

    uint bottomStartpointSpace = atomic_add_global((global uint*)startpointAlloc, currInfo.numStartpoints);

    treelets[0] = EncodeClosedInfo(treeletId);
    RefitTreelet tipTreeletDesc;
    tipTreeletDesc.startpoint_offset = bottomStartpointSpace;
    tipTreeletDesc.numStartpoints = currInfo.numStartpoints;
    tipTreeletDesc.maxDepth = currInfo.maxDepth;
    
    treeletDescs[treeletId] = tipTreeletDesc;

    uint realNumberOfTreelets = treeletId + 1;
    // intentionally we set less by 1, because this number is used in num groups for dispatch which is number of bottom treelets 
    // so substract 1. Except single treelet tree which is should stay 1.
    uint numStartingTreelets = (treeletId == 0) ? 1 : treeletId;

    *BVHBase_GetRefitTreeletCntPtr(bvh) = numStartingTreelets;

    uint treeletDescSpaceIn64B = (realNumberOfTreelets * sizeof(RefitTreelet) + 63) >> 6;
    uint startpointSpaceIn64B = ((bottomStartpointSpace + currInfo.numStartpoints) * sizeof(StartPoint) + 63) >> 6;
    bvh->refitStartPointDataStart = refitTreeletsDataStart + treeletDescSpaceIn64B;
    bvh->BVHDataEnd = refitTreeletsDataStart +treeletDescSpaceIn64B + startpointSpaceIn64B;
    *startpointAlloc = 0;
}


GRL_INLINE void find_refit_treelets(
    global struct BVHBase* bvh,
    global TreeletNodeData* treelets,
    global uint* scratchStartpoints,
    global uint* startpointAlloc)
{
    /* get pointer to inner nodes and back pointers */
    uniform global InternalNode* inner_nodes = (global InternalNode*) BVHBase_GetInternalNodes(bvh);

    /* construct range of nodes that each work group will process */
    uniform const uint numInnerNodes = BVHBase_numNodes(bvh);

    varying ushort lane = get_sub_group_local_id();
    varying uint global_id = get_local_id(0) + get_group_id(0) * get_local_size(0);

    uint numBackpointers = BVHBase_GetNumInternalNodes(bvh);

    // align to 64B and divide
    uint treeletOffsetIn64B = ((numBackpointers * sizeof(uint)) + 63) >> 6;

    uint refitTreeletsDataStart = bvh->backPointerDataStart + treeletOffsetIn64B;
    if (global_id == 0)
    {
        bvh->refitTreeletsDataStart = refitTreeletsDataStart;
    }

    global struct InternalNode* curNode = &inner_nodes[global_id];

    varying ushort has_startpoint = 0;
    if (global_id < numInnerNodes) {
        if ((curNode->nodeType != BVH_INTERNAL_NODE))
        {
            has_startpoint = 1;
        }
    }

    if (has_startpoint == 0)
        return;

    treelet_bottom_up_mark_treelets(
        bvh,
        inner_nodes,
        scratchStartpoints,
        global_id,
        BVHBase_GetBackPointers(bvh),
        treelets,
        refitTreeletsDataStart,
        startpointAlloc);
}

GRL_INLINE void assign_refit_startpoints_to_treelets(
    global struct BVHBase*  bvh,
    global TreeletNodeData* treelets,
    global uint*            scratchStartpoints)
{
    /* get pointer to inner nodes and back pointers */
    uniform global struct InternalNode* inner_nodes = (global struct InternalNode*) BVHBase_GetInternalNodes(bvh);

    /* construct range of nodes that each work group will process */
    uniform const uint numInnerNodes = BVHBase_numNodes(bvh);

    varying ushort lane = get_sub_group_local_id();
    varying uint starPointNode = get_local_id(0) + get_group_id(0) * get_local_size(0);
    varying uint curNodeIndex = starPointNode;
    global struct InternalNode* curNode = &inner_nodes[curNodeIndex];

    varying ushort is_startpoint = 0;

    if (curNodeIndex < numInnerNodes)
    {
        if ((curNode->nodeType != BVH_INTERNAL_NODE))
        {
            is_startpoint = 1;
        }
    }

    if (is_startpoint == 0)
    {
        return;
    }

    BackPointers* backPointers = BVHBase_GetBackPointers(bvh);

    RefitTreelet* treeletDescs = BVHBase_GetRefitTreeletDescs(bvh);
    uint numTreelets = *BVHBase_GetRefitTreeletCntPtr(bvh);
    if (numTreelets > 1) numTreelets++;

    uint myDepthWhenDead = 0;
    uint startpointsBeforeMe = 0;
    bool dead = false;

    uint prevNodeIndex = 0x03FFFFFF;

    while (curNodeIndex != 0x03FFFFFF)
    {
        TreeletNodeData nodeData = treelets[curNodeIndex];

        uint parentPointer = *InnerNode_GetBackPointer(backPointers, curNodeIndex);
        uint numChildren = BackPointer_GetNumChildren(parentPointer);

        // this is counterpart of atomic based entrance decision.
        // the alive path is the longest, if two are equal take the one that came through child with smaller index.
        if (prevNodeIndex != 0x03FFFFFF)
        {
            uint leadChildOfCur = curNodeIndex + inner_nodes[curNodeIndex].childOffset;
            uint childEnd = numChildren + leadChildOfCur;

            uint longestPath = 0;
            uint longestPathChildIdx = leadChildOfCur;

            for (uint child = leadChildOfCur; child < childEnd; child++)
            {
                TreeletNodeData childData = treelets[child];
                if (!isTreeletRoot(childData))
                {
                    TreeletsOpenNodeInfo childinfo = DecodeOpenInfo(childData);
                    if (longestPath <= childinfo.maxDepth) {
                        longestPathChildIdx = child;
                        longestPath = childinfo.maxDepth + 1;
                    }

                    if (child < prevNodeIndex)
                    {
                        // also count how many startpoints are there before me (used to place startpoint in proper slot)
                        startpointsBeforeMe += childinfo.numStartpoints;
                    }
                }
            }

            if (!dead && prevNodeIndex != longestPathChildIdx)
            {
                dead = true;
                //printf("starPointNode %d dies in node %d, myDepthWhenDead %d\n", starPointNode, curNodeIndex, myDepthWhenDead);
            }

            if (!dead) // this "if" is not an "else" to abouve as we might be dead before and comming through the same child index
            {
                myDepthWhenDead = longestPath;
                // it is a startpoint
                //printf("starPointNode %d in node %d lives up, its myDepthWhenDead %d\n", starPointNode, curNodeIndex, myDepthWhenDead);
            }

            if (starPointNode == (uint)-1) {
                // we just entered upper treelet as treelet if we are alive, we can be a new startpoint in new treelet
                if (dead)
                {
                    //printf("starPointNode %d disappears in node %d, myDepthWhenDead %d\n", starPointNode, curNodeIndex, myDepthWhenDead);
                    // and we are dead, so we are not a startpoint of tip, 
                    // so we must disappear to not be added as a startpoint.
                    return;
                }
                else
                {
                    // it is a startpoint
                    //printf("starPointNode %d in node %d becoming its new startpoint\n", starPointNode, curNodeIndex);
                    starPointNode = curNodeIndex;
                }
            }
        }

        if (isTreeletRoot(nodeData))
        {
            TreeletsClosedNodeInfo info = DecodeClosedInfo(nodeData);
            RefitTreelet treeletDesc = treeletDescs[info.treeletId];
            uint startpointSlot = treeletDesc.startpoint_offset + startpointsBeforeMe;
            scratchStartpoints[startpointSlot] = (starPointNode << 6) + (myDepthWhenDead & ((1 << 6) - 1));

            //printf("Adding to treeletID %d at root %d startpoint %d StartNodeIdx %d, depth %d\n", info.treeletId, curNodeIndex, startpointSlot, starPointNode, myDepthWhenDead);

            if (dead) return;
            myDepthWhenDead = 0;
            startpointsBeforeMe = 0;
            starPointNode = (uint)-1;
        }

        /* make parent node the current node */
        prevNodeIndex = curNodeIndex;
        curNodeIndex = BackPointer_GetParentIndex(parentPointer);
        //if(!dead)
        //printf("starPointNode %d move from node %d to %d\n", starPointNode, prevNodeIndex, curNodeIndex);
    }
}

const uint FINALIZE_TREELETS_SLM_DEPTHS_SPACE = 32;

GRL_INLINE void finalize_treelets_in_groups(
    global struct BVHBase* bvh,
    global uint* scratchStartpoints,
    local uint* depths)
{
    uint numTreeletsExecuted = *BVHBase_GetRefitTreeletCntPtr(bvh);

    uint local_id = get_local_id(0);

    uint numTreelets = (numTreeletsExecuted > 1) ? numTreeletsExecuted + 1 : numTreeletsExecuted;

    RefitTreelet* treeletDescs = BVHBase_GetRefitTreeletDescs(bvh);

    for (uint treeletId = get_group_id(0); treeletId < numTreelets; treeletId += numTreeletsExecuted)
    {
        if (treeletId == numTreeletsExecuted && treeletId != 0) { work_group_barrier(CLK_LOCAL_MEM_FENCE); }

        RefitTreelet treeletDesc = treeletDescs[treeletId];
        StartPoint* srcStartpoints = scratchStartpoints + treeletDesc.startpoint_offset;
        if (treeletDesc.numStartpoints <= 1)
        {
            // for smaller latency we store 1 element treelets as RefitTreeletTrivial,
            // this happens most of the time for tip treelet
            if (local_id == 0)
            {
                RefitTreeletTrivial tr = { 0, treeletDesc.numStartpoints, 0, treeletDesc.maxDepth, 0 };
                if (treeletDesc.numStartpoints == 1)
                {
                    StartPoint sp               = srcStartpoints[0];
                    
                    tr.theOnlyNodeIndex         = StartPoint_GetNodeIdx(sp);
                    uint backpointer            = *InnerNode_GetBackPointer(BVHBase_GetBackPointers(bvh), tr.theOnlyNodeIndex);
                    tr.numChildrenOfTheNode     = BackPointer_GetNumChildren(backpointer);
                    tr.childrenOffsetOfTheNode  = BVHBase_GetInternalNodes(bvh)[tr.theOnlyNodeIndex].childOffset + tr.theOnlyNodeIndex;
                }
                RefitTreeletTrivial* trivial = (RefitTreeletTrivial*)(treeletDescs + treeletId);
                *trivial = tr;
#if REFIT_VERBOSE_LOG
                printf("treelet trivial %d {\n  theOnlyNodeIndex = %d;\n  numStartpoints = %d;\n  childrenOffsetOfTheNode = %d;\n  maxDepth =%d;\n  numChildrenOfTheNode = %d;\n}\n",
                    treeletId,
                    tr.theOnlyNodeIndex,
                    tr.numStartpoints,
                    tr.childrenOffsetOfTheNode,
                    tr.maxDepth,
                    tr.numChildrenOfTheNode);
#endif
            }
        }
        else
        {
#define SKIP_PATHS_SORTING 0
#if SKIP_PATHS_SORTING
            StartPoint* dstStartpoints = BVHBase_GetRefitStartPoints(bvh) + treeletDesc.startpoint_offset;
            for (uint startpointID = local_id; startpointID < treeletDesc.numStartpoints; startpointID += get_local_size(0))
            {
                dstStartpoints[startpointID] = srcStartpoints[startpointID];
            }
#else
            //if (local_id == 0) { printf("treelet %d, numStartpoints = %d\n", treeletId, numStartpoints); }

            if (local_id <= treeletDesc.maxDepth) {
                depths[local_id] = 0;
                //    printf("initializing slm treelet %d, depths[%d] = 0\n", treeletId, local_id);
            }
            work_group_barrier(CLK_LOCAL_MEM_FENCE);

            uint loopSize = ((treeletDesc.numStartpoints + (get_sub_group_size() - 1)) / get_sub_group_size()) * get_sub_group_size();

            // collect histogram of how many paths of given length we have

            // keep count of depth 0 
            uint val = 0;

            // optimize: we will load Startpoint only once to 
            uint S_c[8];
            // optimize: keep accumulated numbers in registers to limit number of atomic ops
            uint D_c[8] = { 0 };

            uint cached_threshold = 8 * get_local_size(0);
            cached_threshold = min(cached_threshold, treeletDesc.numStartpoints);

            uint loop_turn = 0;
            uint sgid = get_sub_group_local_id();

            for (uint startpointID = local_id+ cached_threshold; startpointID < treeletDesc.numStartpoints; startpointID += get_local_size(0))
            {
                uint dstSlot = StartPoint_GetDepth(srcStartpoints[startpointID]);
                atomic_inc((volatile local uint*) (depths + dstSlot));
            }

            uint HistogramSG = 0;
            if (treeletDesc.maxDepth < 8)
            {
                for (uint startpointID = local_id; startpointID < cached_threshold; startpointID += get_local_size(0))
                {
                    StartPoint S = srcStartpoints[startpointID];
                    S_c[loop_turn++] = S;
                    uint dstSlot = StartPoint_GetDepth(S);
                    D_c[dstSlot]++;
                }
                
                for (uint d = 0; d <= treeletDesc.maxDepth; d++)
                {
                    val = sub_group_reduce_add(D_c[d]);
                    if (sgid == d)
                    {
                        HistogramSG = val;
                    }
                }
                if (sgid <= treeletDesc.maxDepth && HistogramSG != 0)
                {
                    atomic_add((volatile local uint*) (depths + sgid), HistogramSG);
                }
            }
            else
            {
                for (uint startpointID = local_id; startpointID < cached_threshold; startpointID += get_local_size(0))
                {
                    StartPoint S = srcStartpoints[startpointID];
                    S_c[loop_turn++] = S;
                    uint dstSlot = StartPoint_GetDepth(S);
                    atomic_inc((volatile local uint*) (depths + dstSlot));
                }
            }

            work_group_barrier(CLK_LOCAL_MEM_FENCE);

#if REFIT_VERBOSE_LOG
            if (local_id == 0)
            {
                for (uint d = 0; d <= treeletDesc.maxDepth; d++)
                {
                    printf("treelet %d depths[%d] = %d\n", treeletId, d, depths[d]);
                }
            }
#endif

            if (treeletDesc.maxDepth < get_sub_group_size())
            {
                if (get_sub_group_id() == 0)
                {
                    
                    uint cntOfDepth = 0;
                    if (sgid <= treeletDesc.maxDepth) {
                        cntOfDepth = depths[sgid];
                    }
                    uint pref_sum = sub_group_scan_exclusive_add(cntOfDepth);
                    depths[sgid] = pref_sum;

                    uint numLeft = treeletDesc.numStartpoints - (pref_sum);
                    uint depthLess64  = (numLeft < 64 ) ? (uint)sgid : (uint)treeletDesc.maxDepth;
                    uint depthLess128 = (numLeft < 128) ? (uint)sgid : (uint)treeletDesc.maxDepth;
                    uint depthLess256 = (numLeft < 256) ? (uint)sgid : (uint)treeletDesc.maxDepth;

                    // filling data for thread 0 who will save this to mem
                    treeletDesc.depthLess64 = sub_group_reduce_min(depthLess64);
                    treeletDesc.depthLess128 = sub_group_reduce_min(depthLess128);
                    treeletDesc.depthLess256 = sub_group_reduce_min(depthLess256);
                    treeletDesc.numNonTrivialStartpoints = treeletDesc.numStartpoints - cntOfDepth;

                    if (sgid == 0) {
                        treeletDescs[treeletId] = treeletDesc;
#if REFIT_VERBOSE_LOG
                        printf("treelet %d {\n  startpoint_offset = %d;\n  numStartpoints = %d;\n  numNonTrivialStartpoints = %d;  \n  maxDepth = %d;\n  depthLess64 = %d;\n  depthLess128 = %d;\n  depthLess256 = %d;\n}\n",
                            treeletId,
                            treeletDesc.startpoint_offset,
                            treeletDesc.numStartpoints,
                            treeletDesc.numNonTrivialStartpoints,
                            treeletDesc.maxDepth,
                            treeletDesc.depthLess64,
                            treeletDesc.depthLess128,
                            treeletDesc.depthLess256);
#endif
                    }
                }
            }
            else if (local_id <= treeletDesc.maxDepth) {
                uint thisdepthcount = depths[local_id];
                treeletDesc.depthLess64 = 0;
                treeletDesc.depthLess128 = 0;
                treeletDesc.depthLess256 = 0;
                uint numLeft = treeletDesc.numStartpoints;                
                uint pref_sum = 0;

                for (uint d = 0; d < local_id; d++)
                {
                    uint depthCnt = depths[d];
                    if (numLeft > 64) { treeletDesc.depthLess64 = d + 1; }
                    if (numLeft > 128) { treeletDesc.depthLess128 = d + 1; }
                    if (numLeft > 256) { treeletDesc.depthLess256 = d + 1; }
                    pref_sum += depthCnt;
                    numLeft -= depthCnt;
                    if (d == 0) { treeletDesc.numNonTrivialStartpoints = numLeft; }
                }

                if (local_id == treeletDesc.maxDepth)
                {
                    treeletDescs[treeletId] = treeletDesc;
#if REFIT_VERBOSE_LOG
                    printf("treelet %d {\n  startpoint_offset = %d;\n  numStartpoints = %d;\n  numNonTrivialStartpoints = %d;  maxDepth = %d;\n  depthLess64 = %d;  depthLess128 = %d;  depthLess256 = %d;\n}\n",
                        treeletId,
                        treeletDesc.startpoint_offset,
                        treeletDesc.numStartpoints,
                        treeletDesc.numNonTrivialStartpoints,
                        treeletDesc.maxDepth,
                        treeletDesc.depthLess64,
                        treeletDesc.depthLess128,
                        treeletDesc.depthLess256);
#endif
                }    
            }

            StartPoint* dstStartpoints = BVHBase_GetRefitStartPoints(bvh) + treeletDesc.startpoint_offset;

            work_group_barrier(CLK_LOCAL_MEM_FENCE);
            
            loop_turn = 0;
            if (treeletDesc.maxDepth < 8)
            {
                uint prefixSG = 0;

                // make prefixSG keep interval for paths with sglid depth that is separated out for sg.
                if (sgid <= treeletDesc.maxDepth && HistogramSG != 0)
                {
                    prefixSG = atomic_add((volatile local uint*) (depths + sgid), HistogramSG);
                }

                // from now on all sgs run independently

                // make D_c keep offset interval that is separated out for given lane
                for (uint d = 0; d <= treeletDesc.maxDepth; d++)
                {
                    uint thisDPrefixSg = sub_group_broadcast(prefixSG, d);
                    uint thisLaneCount = D_c[d];
                    uint laneOffset = sub_group_scan_exclusive_add(thisLaneCount);
                    D_c[d] = laneOffset + thisDPrefixSg;
                }

                for (uint startpointID = local_id; startpointID < cached_threshold; startpointID += get_local_size(0))
                {
                    StartPoint S = S_c[loop_turn++];
                    uint d = StartPoint_GetDepth(S);
                    uint dstSlot = D_c[d]++;
                    dstStartpoints[dstSlot] = S;
                }
            }
            else
            {
                for (uint startpointID = local_id; startpointID < cached_threshold; startpointID += get_local_size(0))
                {
                    StartPoint S = S_c[loop_turn++];
                    uint d = StartPoint_GetDepth(S);
                    uint dstSlot = atomic_inc((volatile local uint*) (depths + d));
                    dstStartpoints[dstSlot] = S;
                }
            }

            for (uint srcStartpointID = local_id+ cached_threshold; srcStartpointID < treeletDesc.numStartpoints; srcStartpointID += get_local_size(0))
            {
                StartPoint S = srcStartpoints[srcStartpointID];
                uint d = StartPoint_GetDepth(srcStartpoints[srcStartpointID]);
                uint dstSlot = atomic_inc((volatile local uint*) (depths+ d));
                dstStartpoints[dstSlot] = S;
            }
#endif //skip sorting
        }
    }
}
