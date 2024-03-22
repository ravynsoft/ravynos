//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "instance.h"
#include "api_interface.h"

#include "bvh_build_primref.h"
#include "bvh_build_refit.h"

/*
  Create primrefs from array of instance descriptors.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) void kernel
TS_primrefs_from_instances(
    global struct Globals* globals,
    global struct BVHBase* bvh,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instances,
    uint numInstances,
    global struct AABB* primrefs,
    global uchar* pAABBs,
    global uchar* pIsProcedural,
    dword aabb_stride,
    uint allowUpdate
    )
{
    const uint instanceIndex = get_sub_group_local_id() + get_group_id(0) * MAX_HW_SIMD_WIDTH;
    if (instanceIndex < numInstances)
    {
        global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instances + instanceIndex;

        global struct GRL_RAYTRACING_AABB* procedural_bb = 0;
        if ( pIsProcedural[instanceIndex] )
        {
            procedural_bb = (global struct GRL_RAYTRACING_AABB*)(pAABBs + aabb_stride * instanceIndex);
        }

        primrefs_from_instances(
            globals,
            bvh,
            instance,
            instanceIndex,
            primrefs,
            procedural_bb,
            allowUpdate);
    }
}

/*
  Create primrefs from array of instance descriptors.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
void kernel
TS_primrefs_from_instances_indirect(
    global struct Globals* globals,
    global struct BVHBase* bvh,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instances,
    uint numInstances,
    global struct AABB* primrefs,
    global uchar* pAABBs,
    global uchar* pIsProcedural,
    dword aabb_stride,
    uint allowUpdate,
    global struct IndirectBuildRangeInfo* indirect_data
    )
{
    const uint instanceIndex = get_local_id(0) + get_group_id(0) * MAX_HW_SIMD_WIDTH;
    if (instanceIndex < indirect_data->primitiveCount)
    {
        instances = (global __const struct GRL_RAYTRACING_INSTANCE_DESC*)
            (((global char*)instances) + indirect_data->primitiveOffset);
        global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instances + instanceIndex;

        global struct GRL_RAYTRACING_AABB* procedural_bb = 0;
        if ( pIsProcedural[instanceIndex] )
        {
            procedural_bb = (global struct GRL_RAYTRACING_AABB*)(pAABBs + aabb_stride * instanceIndex);
        }

        primrefs_from_instances(
            globals,
            bvh,
            instance,
            instanceIndex,
            primrefs,
            procedural_bb,
            allowUpdate);
    }
}

/*
  Create primrefs from array of pointers to instance descriptors.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) void kernel
TS_primrefs_from_instances_pointers(global struct Globals* globals,
    global struct BVHBase* bvh,
    global void* instances_in,
    uint numInstances,
    global struct AABB* primrefs,
    global uchar* pAABBs,
    global uchar* pIsProcedural,
    dword aabb_stride,
    uint allowUpdate
    )
{
    global const struct GRL_RAYTRACING_INSTANCE_DESC** instances =
        (global const struct GRL_RAYTRACING_INSTANCE_DESC**)instances_in;

    const uint instanceIndex = get_sub_group_local_id() + get_group_id(0) * MAX_HW_SIMD_WIDTH;
    if (instanceIndex < numInstances)
    {
        global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instances[instanceIndex];

        global struct GRL_RAYTRACING_AABB* procedural_bb = 0;
        if (pIsProcedural[instanceIndex])
        {
            procedural_bb = (global struct GRL_RAYTRACING_AABB*)(pAABBs + aabb_stride * instanceIndex);
        }

        primrefs_from_instances(
            globals,
            bvh,
            instance,
            instanceIndex,
            primrefs,
            procedural_bb,
            allowUpdate);
    }
}

/*
  Create primrefs from array of pointers to instance descriptors.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
void kernel
TS_primrefs_from_instances_pointers_indirect(global struct Globals* globals,
    global struct BVHBase* bvh,
    global void* instances_in,
    global struct AABB* primrefs,
    global uchar* pAABBs,
    global uchar* pIsProcedural,
    dword aabb_stride,
    uint allowUpdate,
    global struct IndirectBuildRangeInfo* indirect_data
    )
{
    const uint instanceIndex = get_local_id(0) + get_group_id(0) * MAX_HW_SIMD_WIDTH;
    if (instanceIndex < indirect_data->primitiveCount)
    {
        instances_in = ((global char*)instances_in) + indirect_data->primitiveOffset;
        global const struct GRL_RAYTRACING_INSTANCE_DESC** instances =
            (global const struct GRL_RAYTRACING_INSTANCE_DESC**)instances_in;
        global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instances[instanceIndex];

        global struct GRL_RAYTRACING_AABB* procedural_bb = 0;
        if (pIsProcedural[instanceIndex])
        {
            procedural_bb = (global struct GRL_RAYTRACING_AABB*)(pAABBs + aabb_stride * instanceIndex);
        }

        primrefs_from_instances(
            globals,
            bvh,
            instance,
            instanceIndex,
            primrefs,
            procedural_bb,
            allowUpdate);
    }
}



GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel
TS_update_instance_leaves(global struct BVHBase* bvh,
    uint64_t dxrInstancesArray,
    uint64_t dxrInstancesPtr,
    global struct AABB3f* instance_aabb_scratch,
    global uchar* aabbs,
    global uchar* is_procedural,
    dword aabb_stride
)
{
    uint num_leaves = BVHBase_GetNumHWInstanceLeaves(bvh);
    uint id = get_local_id(0) + get_local_size(0) * get_group_id(0);
    if (id >= num_leaves)
        return;

    struct HwInstanceLeaf* leaves = BVHBase_GetHWInstanceLeaves(bvh);
    uint idx = HwInstanceLeaf_GetInstanceIndex(&leaves[id]);

    global GRL_RAYTRACING_AABB* procedural_box = 0;
    if (is_procedural[idx])
    {
        procedural_box = (global GRL_RAYTRACING_AABB*)(aabbs + (aabb_stride * idx));
    }

    DO_update_instance_leaves(
        bvh,
        dxrInstancesArray,
        dxrInstancesPtr,
        instance_aabb_scratch,
        id,
        procedural_box);
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel
TS_fixup_leaves( global struct BVHBase* bvh,
                 global uchar* primref_index,
                 global PrimRef* primrefs,
                 uint stride )

{
    uint num_inners = BVHBase_GetNumInternalNodes(bvh);
    uint id       = get_local_id(0) + get_local_size(0) * get_group_id(0);

    // assign 8 lanes to each inner node, 6 of which will do useful work
    uint node_id  = id / 8;
    uint child_id = id % 8;

    bool node_valid = (node_id < num_inners);

    if (node_valid )
    {
        global InternalNode* nodes = (global InternalNode*) BVHBase_GetInternalNodes(bvh);
        global InternalNode* my_node = nodes + node_id;

        if (my_node->nodeType == BVH_INSTANCE_NODE)
        {
            bool child_valid = (child_id < 6) && InternalNode_IsChildValid(my_node, child_id);
            if (child_valid)
            {
                global HwInstanceLeaf* leaves = (global HwInstanceLeaf*)InternalNode_GetChildren(my_node);
                uint leafIndex = (leaves - BVHBase_GetHWInstanceLeaves(bvh)) + child_id;

                const uint primrefID = *(uint*)(primref_index + leafIndex * stride);

                uint type = PRIMREF_isProceduralInstance(&primrefs[primrefID]) ?
                                BVH_PROCEDURAL_NODE : BVH_INSTANCE_NODE;

                InternalNode_SetChildType(my_node, child_id, type);
            }

            if (child_id == 0)
                my_node->nodeType = BVH_INTERNAL_NODE;
        }
    }
}





GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(SG_REFIT_WG_SIZE, 1, 1))) void kernel
TS_Refit_per_one_startpoint_sg(
    global struct BVHBase* bvh,
    global struct AABB3f* instance_leaf_aabbs,
    global uchar* procedural_instance_enable_buffer )
{
    DO_Refit_per_one_startpoint_sg(bvh, (global GRL_RAYTRACING_GEOMETRY_DESC*) bvh, instance_leaf_aabbs, procedural_instance_enable_buffer );

}
