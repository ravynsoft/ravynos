//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "common.h"
#include "api_interface.h"
#include "instance.h"
#include "GRLGen12.h"
#include "libs/lsc_intrinsics.h"


__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel
DO_update_instance_leaves(global struct BVHBase* bvh,
    uint64_t dxrInstancesArray,
    uint64_t dxrInstancesPtr,
    global struct AABB3f* instance_aabb_scratch,
    uint id ,
    global struct GRL_RAYTRACING_AABB* procedural_box
)
{

    global struct GRL_RAYTRACING_INSTANCE_DESC* instancesArray =
        (global struct GRL_RAYTRACING_INSTANCE_DESC*)dxrInstancesArray;
    global struct GRL_RAYTRACING_INSTANCE_DESC** instancesPtrArray =
        (global struct GRL_RAYTRACING_INSTANCE_DESC**)dxrInstancesPtr;

    global struct HwInstanceLeaf* leafs = (global struct HwInstanceLeaf*) BVHBase_GetHWInstanceLeaves(bvh);
    

    /* iterate over all children of the instance node and get their bounds */

    uint32_t instanceIdx = HwInstanceLeafPart1_getInstanceIndex(&leafs[id]);
    global struct GRL_RAYTRACING_INSTANCE_DESC* instance = NULL;
    if (dxrInstancesArray != NULL)
        instance = &instancesArray[instanceIdx];
    else
        instance = instancesPtrArray[instanceIdx];

    uint mask = GRL_get_InstanceMask(instance);
    uint offset = NO_NODE_OFFSET;

    struct AffineSpace3f xfm = AffineSpace3f_load_row_major(instance->Transform);
    struct AABB3f bbox;

    if (procedural_box != 0)
    {
        bbox.lower[0] = procedural_box->MinX;
        bbox.lower[1] = procedural_box->MinY;
        bbox.lower[2] = procedural_box->MinZ;
        bbox.upper[0] = procedural_box->MaxX;
        bbox.upper[1] = procedural_box->MaxY;
        bbox.upper[2] = procedural_box->MaxZ;
    }
    else
    {
        global struct BVHBase* instanceBvh = (global struct BVHBase*)instance->AccelerationStructure;
        bbox = instanceBvh->Meta.bounds;
        offset = BVH_ROOT_NODE_OFFSET;
    }


    const bool valid_min = isfinite(bbox.lower[0]) && isfinite(bbox.lower[1]) && isfinite(bbox.lower[2]);
    const bool valid_max = isfinite(bbox.upper[0]) && isfinite(bbox.upper[1]) && isfinite(bbox.upper[2]);

    if (!valid_min || !valid_max )
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
    else
    {
        bbox = AABB3f_transform(xfm, bbox); // JDB TODO:  Use faster abs-matrix method
    }

    instance_aabb_scratch[id] = bbox;

    HwInstanceLeaf_Constructor(&leafs[id], instance, instanceIdx, offset, mask); // TODO: No instance opening for refittable BVH   
}

/*
   This function starts at some BVH node and refits all nodes upwards
   to the root. At some node the algorithm only proceeds upwards if
   all children of the current node have already been processed. This
   is checked as each time a node is reached an atomic counter is
   incremented, which will reach the number of children of the node at
   some time.
 */

GRL_INLINE void refit_bottom_up(global struct QBVHNodeN *qnode_start, // start node to refit (already processed)
                            global struct BVHBase *bvh,           // pointer to BVH
                            struct AABB *childrenAABB,            // temporary data to use
                            uint numChildrenTotal)
{
    global struct QBVHNodeN *nodeData = BVHBase_nodeData(bvh);
    BackPointers* backPointers = BVHBase_GetBackPointers(bvh);

    /* compute the index of the start node */
    uint curNodeIndex = qnode_start - nodeData;

    /* the start node got already processed, thus go to its parent node */
    curNodeIndex = *InnerNode_GetBackPointer(backPointers,curNodeIndex) >> 6;

    /* end at root node */
    while (curNodeIndex != 0x03FFFFFF)
    {
        /* increment refit counter that counts refitted children of current node */
        const uint parentPointer = 1 + atomic_inc_global( (__global uint *) InnerNode_GetBackPointer(backPointers, curNodeIndex));

        /* if all children got refitted, then continue */
        const uint numChildrenRefitted = (parentPointer >> 0) & 0x7;
        numChildrenTotal = (parentPointer >> 3) & 0x7;
        if (numChildrenRefitted != numChildrenTotal)
            return;

        /* reset refit counter for next refit */
        *InnerNode_GetBackPointer(backPointers, curNodeIndex) &= 0xfffffff8;

        /* get bounds of all children from child nodes directly */
        global struct QBVHNodeN *qnode = nodeData + curNodeIndex;
        global struct QBVHNodeN *qnode_child = (global struct QBVHNodeN *)QBVHNodeN_childrenPointer(qnode);
        for (uint k = 0; k < numChildrenTotal; k++)
            childrenAABB[k] = getAABB_QBVHNodeN(qnode_child + k);

        /* update node bounds of all children */
        QBVHNodeN_setBounds(qnode, childrenAABB, numChildrenTotal);

        write_mem_fence(CLK_GLOBAL_MEM_FENCE);

        /* make parent node the current node */
        curNodeIndex = parentPointer >> 6;
    }

    /* update QBVH6 bounds */
    struct AABB bounds;
    AABB_init(&bounds);

    for (uint i = 0; i < numChildrenTotal; i++)
        AABB_extend(&bounds, &childrenAABB[i]);

    setBVHBaseBounds(bvh, &bounds);
}


GRL_INLINE void SUBGROUP_refit_bottom_up( 
    uniform global struct QBVHNodeN* qnode_start, // start node to refit (already processed)
    uniform global struct BVHBase* bvh,           // pointer to BVH
    varying struct AABB reduce_bounds,            
    uniform uint numChildrenTotal,
    varying ushort lane,
    varying ushort head_lane)
{
    uniform global struct QBVHNodeN* nodeData = BVHBase_nodeData( bvh );
    uniform BackPointers* backPointers = BVHBase_GetBackPointers( bvh );

    /* compute the index of the start node */
    uniform uint curNodeIndex = qnode_start - nodeData;

    /* the start node got already processed, thus go to its parent node */
    uniform curNodeIndex = *InnerNode_GetBackPointer(backPointers, curNodeIndex) >> 6;

    varying struct AABB childrenAABB;

    /* end at root node */
    while ( curNodeIndex != 0x03FFFFFF )
    {
        mem_fence_gpu_invalidate();

        /* increment refit counter that counts refitted children of current node */
        uniform uint parentPointer = 1;
        if (lane == 0)
        {
            // acquire fence ensures that all previous writes complete before the atomic starts
            parentPointer += atomic_inc_global((__global uint *)InnerNode_GetBackPointer(backPointers, curNodeIndex));
        }

        parentPointer = intel_sub_group_shuffle( parentPointer, head_lane );

        /* if all children got refitted, then continue */
        uniform uint numChildrenRefitted = (parentPointer >> 0) & 0x7;
        numChildrenTotal = (parentPointer >> 3) & 0x7;
        if ( numChildrenRefitted != numChildrenTotal )
            return;

        /* reset refit counter for next refit */
        if (lane == 0)
        {
            *InnerNode_GetBackPointer(backPointers, curNodeIndex) = (parentPointer & 0xfffffff8);
        }

        /* get bounds of all children from child nodes directly */
        global struct QBVHNodeN* qnode = nodeData + curNodeIndex;
        global struct QBVHNodeN* qnode_child = (global struct QBVHNodeN*)QBVHNodeN_childrenPointer( qnode );

        varying ushort child_idx = (lane < numChildrenTotal) ? lane : 0;
        childrenAABB = getAABB_QBVHNodeN( qnode_child + child_idx );

        /* update node bounds of all children */
        reduce_bounds = AABB_sub_group_reduce_N6( &childrenAABB );
        reduce_bounds = AABB_sub_group_shuffle( &reduce_bounds, head_lane );

        subgroup_QBVHNodeN_setBounds(qnode, reduce_bounds, childrenAABB, numChildrenTotal, lane);

        /* update node mask */
        uchar childrenMask = qnode_child[child_idx].instMask;

        qnode->instMask = sub_group_reduce_or_N6(childrenMask);

        /* make parent node the current node */
        curNodeIndex = parentPointer >> 6;
    }

    /* update QBVH6 bounds */
    
    if( lane == 0 )
        setBVHBaseBounds( bvh, &reduce_bounds );
}


GRL_INLINE void quadCopyVertices(
    const struct QuadLeaf* pQuad,
    struct QuadLeaf* newQuad)
{
    const uint4* s = (const uint4*) & (pQuad->v[0][0]);
    uint4* d = (uint4*) & (newQuad->v[0][0]);
    const uint8* s2 = (const uint8*)(s+1);
    uint8* d2 = (uint8*)(d+1);
    *d = *s;
    *d2 = *s2;
}


GRL_INLINE void get_updated_quad(
    global const struct QuadLeaf* pQuad,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDescs,
    struct QuadLeaf* newQuad)
{
    struct QuadLeaf tempQuad;

    // fetch non vtx data;
    {
        uint4* tempQuad4U = (uint4*)&tempQuad;
        global const uint4* pQuad4U = (global const uint4*)pQuad;
        *tempQuad4U = *pQuad4U;
    }   

    /* get the geomID and primID0/1 for both quad triangles */
    const uint geomID = PrimLeaf_GetGeoIndex(&tempQuad.leafDesc);
    const uint primID0 = tempQuad.primIndex0;
    const uint primID1 = tempQuad.primIndex0 + QuadLeaf_GetPrimIndexDelta(&tempQuad);
    ushort fourth_vert = 0;

    if (primID1 != primID0)
    {
        ushort packed_indices = QuadLeaf_GetSecondTriangleIndices(&tempQuad);
        fourth_vert = ((packed_indices & 0x0C) == 0x0C) ? 1 : fourth_vert;
        fourth_vert = ((packed_indices & 0x30) == 0x30) ? 2 : fourth_vert;
    }

    global GRL_RAYTRACING_GEOMETRY_DESC* desc = geomDescs + geomID;

    uint4 indices = GRL_load_quad_indices(desc, primID0, primID1, fourth_vert);

    // read the indices of the 4 verts we want
    float3 vtx0, vtx1, vtx2, vtx3;
    GRL_load_quad_vertices(desc, &vtx0, &vtx1, &vtx2, &vtx3, indices);

    QuadLeaf_SetVertices(&tempQuad, vtx0, vtx1, vtx2, vtx3);

    *newQuad = tempQuad;
}

// This calculates children BBs for innerNode having *all* children leafs.
// mixed nodes will be updated by passing through bottom-up thread.
GRL_INLINE uint refit_bottom( global struct BVHBase* bvh, 
                          global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,                       
                          global struct AABB3f* instance_leaf_aabbs,
                          global struct QBVHNodeN* curNode,
                          struct AABB *childrenAABB,
                          uint backPointer)
{
    uint numChildren = 0;

    /* we start refit at leaf nodes, this case is for quad nodes */
    if (curNode->type == BVH_QUAD_NODE)
    {
        global struct QuadLeaf* quads = (global struct QuadLeaf*)QBVHNodeN_childrenPointer(curNode);

        /* iterate over all quads of the quad node and get their bounds */
        numChildren = (backPointer >> 3) & 0x7;
        for (uint k = 0; k < numChildren; k++)
        {
            struct QuadLeaf Q;
            get_updated_quad(&quads[k], geomDesc, &Q);
            quadCopyVertices(&Q, &quads[k]);
            childrenAABB[k] = getAABB_Quad((struct Quad*)&Q); // FIXME: support leaves with more than one quad
        }
    }

    /* we start refit at leaf nodes, this case is for procedural nodes */
    else if (curNode->type == BVH_PROCEDURAL_NODE)
    {
        global struct ProceduralLeaf* leaf = (global struct ProceduralLeaf*)QBVHNodeN_childrenPointer(curNode);

        /* iterate over all children of the procedural node and get their bounds */
        numChildren = (backPointer >> 3) & 0x7;
        for (uint k = 0; k < numChildren; k++)
        {
            /* extract geomID and primID from leaf */
            const uint startPrim = QBVHNodeN_startPrim(curNode, k);
            const uint geomID = ProceduralLeaf_geomIndex(leaf);
            const uint primID = ProceduralLeaf_primIndex(leaf, startPrim); // FIXME: have to iterate over all primitives of leaf!

            /* read bounds from geometry descriptor */
            struct GRL_RAYTRACING_AABB aabb = GRL_load_aabb(&geomDesc[geomID], primID);
            childrenAABB[k].lower.x = aabb.MinX;
            childrenAABB[k].lower.y = aabb.MinY;
            childrenAABB[k].lower.z = aabb.MinZ;
            childrenAABB[k].upper.x = aabb.MaxX;
            childrenAABB[k].upper.y = aabb.MaxY;
            childrenAABB[k].upper.z = aabb.MaxZ;

            /* advance leaf pointer to next child */
            leaf += QBVHNodeN_blockIncr(curNode, k);
        }
    }

    /* we start refit at leaf nodes, this case is for instance nodes */
    else if (curNode->type == BVH_INSTANCE_NODE)
    {
        global struct HwInstanceLeaf* instancesLeaves = (global struct HwInstanceLeaf*)QBVHNodeN_childrenPointer(curNode);
        global struct HwInstanceLeaf* leafBase = (global struct HwInstanceLeaf*) BVHBase_GetHWInstanceLeaves( bvh );

        /* iterate over all children of the instance node and get their bounds */
        numChildren = (backPointer >> 3) & 0x7;
        for (uint k = 0; k < numChildren; k++)
        {
            uint leafindex = (instancesLeaves + k) - leafBase;
            childrenAABB[k].lower.xyz = AABB3f_load_lower( &instance_leaf_aabbs[leafindex] );
            childrenAABB[k].upper.xyz = AABB3f_load_upper( &instance_leaf_aabbs[leafindex] );
        }
    }

    return numChildren;
}





// This calculates children BBs for innerNode having *all* children leafs.
// mixed nodes will be updated by passing through bottom-up thread.
GRL_INLINE uint SUBGROUP_refit_bottom(
    uniform global struct BVHBase* bvh,
    uniform global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    uniform global struct AABB3f* instance_leaf_aabbs,
    uniform global struct QBVHNodeN* curNode,
    uniform uint backPointer,
    varying struct AABB* childrenAABB,
    varying uchar* childrenMask,
    varying ushort lane,
    global uchar* is_procedural_instance
    )
{
    uniform uint numChildren = 0;
    bool enable_procedural_instance = (is_procedural_instance != 0);

    /* we start refit at leaf nodes, this case is for quad nodes */
    if (curNode->type == BVH_QUAD_NODE)
    {
        /* iterate over all quads of the quad node and get their bounds */
        numChildren = (backPointer >> 3) & 0x7;

        uniform global struct QuadLeaf* quads = (global struct QuadLeaf*)QBVHNodeN_childrenPointer(curNode);

        struct QuadLeaf Q;
        if (lane < numChildren)
        {
            get_updated_quad(&quads[lane], geomDesc, &Q);

            *childrenAABB = getAABB_Quad((struct Quad*) & Q); // FIXME: support leaves with more than one quad

            quadCopyVertices(&Q, &quads[lane]);
            *childrenMask = 0xff;
        }
        // FIXME: support leaves with more than one quad
    }

    /* we start refit at leaf nodes, this case is for procedural nodes */
    else if (curNode->type == BVH_PROCEDURAL_NODE)
    {
        uniform global struct ProceduralLeaf* leaf = (global struct ProceduralLeaf*)QBVHNodeN_childrenPointer(curNode);


        
        /* iterate over all children of the procedural node and get their bounds */
        numChildren = (backPointer >> 3) & 0x7;
        
        varying uint incr = (lane < numChildren) ? InternalNode_GetChildBlockIncr((struct InternalNode*)curNode, lane) : 0;
        incr = sub_group_scan_exclusive_add(incr);

        if( lane < numChildren )
        {
            /* extract geomID and primID from leaf */
            varying uint start_prim = InternalNode_GetChildStartPrim((struct InternalNode*)curNode, lane );
            varying global struct ProceduralLeaf* my_leaf = leaf + incr;
            const uint geomID = ProceduralLeaf_geomIndex(my_leaf);
            const uint primID = ProceduralLeaf_primIndex(my_leaf, start_prim); 

            /* read bounds from geometry descriptor */
            struct GRL_RAYTRACING_AABB aabb = GRL_load_aabb(&geomDesc[geomID], primID);
            childrenAABB->lower.x = aabb.MinX;
            childrenAABB->lower.y = aabb.MinY;
            childrenAABB->lower.z = aabb.MinZ;
            childrenAABB->upper.x = aabb.MaxX;
            childrenAABB->upper.y = aabb.MaxY;
            childrenAABB->upper.z = aabb.MaxZ;
            *childrenMask = 0xff;
        }
    }

    /* we start refit at leaf nodes, this case is for instance nodes */
    else if ( !enable_procedural_instance && curNode->type == BVH_INSTANCE_NODE)
    {
        uniform global struct HwInstanceLeaf* instancesLeaves = (global struct HwInstanceLeaf*)QBVHNodeN_childrenPointer(curNode);
        uniform global struct HwInstanceLeaf* leafBase = (global struct HwInstanceLeaf*) BVHBase_GetHWInstanceLeaves(bvh);

        /* iterate over all children of the instance node and get their bounds and masks */
        numChildren = (backPointer >> 3) & 0x7;
        if( lane < numChildren )
        {
            uint leafindex = (instancesLeaves + lane) - leafBase;
            childrenAABB->lower.xyz = AABB3f_load_lower(&instance_leaf_aabbs[leafindex]);
            childrenAABB->upper.xyz = AABB3f_load_upper(&instance_leaf_aabbs[leafindex]);
            *childrenMask = HwInstanceLeaf_GetInstanceMask(&leafBase[leafindex]);
        }
    }
    else if (enable_procedural_instance && curNode->type == BVH_INTERNAL_NODE)
    {
        // Handle procedural-instance leaves
        //   TODO:  Generalize this!   Should re-write the kernel to work with arbitrary mixed-mode leaves
        
        numChildren = (backPointer >> 3) & 0x7;
        uint childType = BVH_INTERNAL_NODE;
        if ( lane < numChildren )
        {
            childType = InternalNode_GetChildType( (struct InternalNode*)curNode, lane );
            if (childType != BVH_INTERNAL_NODE)
            {
                uniform global struct HwInstanceLeaf* instancesLeaves = (global struct HwInstanceLeaf*)QBVHNodeN_childrenPointer( curNode );
                uniform global struct HwInstanceLeaf* leafBase = (global struct HwInstanceLeaf*) BVHBase_GetHWInstanceLeaves( bvh );
                uint leafindex = (instancesLeaves + lane) - leafBase;
                childrenAABB->lower.xyz = AABB3f_load_lower( &instance_leaf_aabbs[leafindex] );
                childrenAABB->upper.xyz = AABB3f_load_upper( &instance_leaf_aabbs[leafindex] );
                *childrenMask = HwInstanceLeaf_GetInstanceMask( &leafBase[leafindex] );

                // see if the child has flipped from procedural to non-procedural and update the child type field as needed
                uint instanceIndex = HwInstanceLeaf_GetInstanceIndex( &leafBase[leafindex] );
                uint newChildType = is_procedural_instance[instanceIndex] ? BVH_PROCEDURAL_NODE : BVH_INSTANCE_NODE;
                if (newChildType != childType)
                {
                    InternalNode_SetChildType( (struct InternalNode*)curNode, lane, newChildType );
                }
            }            
        }


        // don't ascend the tree for a true internal node
        if (sub_group_all(childType == BVH_INTERNAL_NODE))
            numChildren = 0;
    }
    
    return numChildren;
}

#define SG_REFIT_WG_SIZE 8

void DO_Refit_per_one_startpoint_sg(
    global struct BVHBase* bvh,
    global GRL_RAYTRACING_GEOMETRY_DESC* geosArray,
    global struct AABB3f* instance_leaf_aabbs,
    global uchar* is_procedural_instance )
{
    /* get pointer to inner nodes and back pointers */
    global struct QBVHNodeN* inner_nodes = BVHBase_rootNode(bvh);
    BackPointers* backPointers = BVHBase_GetBackPointers(bvh);

    /* get the inner node that we will consider as a bottom startpoint */
    const uint numInnerNodes = BVHBase_numNodes(bvh);
    const uint innerNodeIdx = get_sub_group_global_id();

    varying ushort lane = get_sub_group_local_id();

    if (innerNodeIdx >= numInnerNodes) return;

    varying struct AABB childrenAABB; // one child AABB per lane
    AABB_init(&childrenAABB);

    varying uchar childrenMask = 0; // one child mask per lane

    global struct QBVHNodeN* curNode = &inner_nodes[innerNodeIdx];
    uint backPointer = *InnerNode_GetBackPointer(backPointers, innerNodeIdx);
    uint numChildren = SUBGROUP_refit_bottom(
        bvh,
        geosArray,
        instance_leaf_aabbs,
        curNode,
        backPointer,
        &childrenAABB,
        &childrenMask,
        lane,
        is_procedural_instance
         );

    
    if (numChildren != 0)
    {
        /* update bounds of node */
        struct AABB reduce_bounds = AABB_sub_group_reduce_N6(&childrenAABB);
        reduce_bounds = AABB_sub_group_shuffle(&reduce_bounds, 0);
        subgroup_QBVHNodeN_setBounds(curNode, reduce_bounds, childrenAABB, numChildren, lane);
        
        /* update mask of node */
        uchar mask = sub_group_reduce_or_N6(childrenMask);
        curNode->instMask = mask;

        /* Leave this fence for now for all threads, if WG size is increased (tried 128) and fence is done
           only by the first thread (similar to morton phase1) the machine hangs. */
        mem_fence_gpu_invalidate();

        /* refit upper parts of the BVH */
        /* TODO: this will not gonna work for mixed nodes */
        SUBGROUP_refit_bottom_up(curNode, bvh, reduce_bounds, numChildren, lane, 0);
    }
}