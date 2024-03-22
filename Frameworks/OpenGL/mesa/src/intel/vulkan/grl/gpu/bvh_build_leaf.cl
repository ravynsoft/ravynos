//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "api_interface.h"
#include "common.h"
#include "instance.h"


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(32, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel
primref_to_quads(global struct Globals *globals,
                 global struct AABB *primref,
                 global char *primref_index,
                 global char *bvh_mem,
                 global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc,
                 const uint stride,
                 const uint offset,
                 const uint allow_update)
{
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    global struct Quad *quads = (global struct Quad *)(bvh_mem + 64*bvh->quadLeafStart );
    uint quadIndicesStart = bvh->quadIndicesDataStart;

    const uint numPrimitives = globals->numPrimitives;
    uint i = get_group_id( 0 ) * get_local_size( 0 ) + get_local_id(0);
    if (i < numPrimitives)
    {
        global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;

        const uint primrefID = *(uint *)(primref_index + i * stride + offset);

        const uint geomID    = PRIMREF_geomID(&primref[primrefID]);
        const uint primID0   = PRIMREF_primID0(&primref[primrefID]);
        const uint primID1   = PRIMREF_primID1(&primref[primrefID]);
        const uint geomFlags = PRIMREF_geomFlags(&primref[primrefID]);

        const uint3 tri0 = GRL_load_triangle(&geomDesc[geomID], primID0);
        const uint3 tri1 = GRL_load_triangle(&geomDesc[geomID], primID1);

        const struct TrianglePair q = TrianglePair_Constructor(tri0, primID0, tri1, primID1);

        uint vertex_stride = geomDesc[geomID].Desc.Triangles.VertexBufferByteStride;

        const uint4 indices = q.a;

        const uint mask = 0xff; // FIXME: hardcoded mask
        float3 vtx0, vtx1, vtx2, vtx3;
        GRL_load_quad_vertices(&geomDesc[geomID], &vtx0, &vtx1, &vtx2, &vtx3, indices);

        uint j0 = q.lb.x;
        uint j1 = q.lb.y;
        uint j2 = q.lb.z;
        uint shaderIndex = (mask << 24) | geomID;
        uint geomIndex = geomID | (geomFlags << 30);
        uint primIndex0 = primID0;
        const uint delta = primID1 - primID0;
        const uint j = (((j0) << 0) | ((j1) << 2) | ((j2) << 4));
        uint primIndex1Delta = delta | (j << 16) | (1 << 22);

        uint4 pack0 = (uint4)(shaderIndex, geomIndex, primIndex0, primIndex1Delta);
        float4 pack1 = (float4)(vtx0.x, vtx0.y, vtx0.z, vtx1.x);
        float4 pack2 = (float4)(vtx1.y, vtx1.z, vtx2.x, vtx2.y);
        float4 pack3 = (float4)(vtx2.z, vtx3.x, vtx3.y, vtx3.z);
        
        global uint4* dst = (global uint4*)&quads[i];
        store_uint4_L1WB_L3WB(dst, 0, pack0);
        store_uint4_L1WB_L3WB(dst, 1, as_uint4(pack1));
        store_uint4_L1WB_L3WB(dst, 2, as_uint4(pack2));
        store_uint4_L1WB_L3WB(dst, 3, as_uint4(pack3));

        if(allow_update)
        {
            global uint4* vertex_indice_ptr = (global uint4*)(((char*)bvh) + (64u * quadIndicesStart + 32 * i));
        
            uint4 pack_indices = (uint4) ( indices.x , indices.y, indices.z, indices.w );
        
            store_uint4_L1WB_L3WB( vertex_indice_ptr, 0, pack0 );
            store_uint4_L1WB_L3WB( vertex_indice_ptr, 1, pack_indices * vertex_stride);
        }

        if (i == 0)
            bvh->quadLeafCur += numPrimitives ;
    }



#if 0
    global struct BVHBase* bvh = (global struct BVHBase*)bvh_mem;
    global struct Quad *quads = (global struct Quad *)(bvh_mem + 64*bvh->quadLeafStart );

    const uint numPrimitives = globals->numPrimitives;
    const uint startID = get_group_id( 0 ) * get_local_size( 0 );
    const uint endID   = min((uint)(startID + get_local_size( 0 )), numPrimitives);

    for (uint i = startID + get_local_id(0); i < endID; i += get_local_size(0))
    {
        const uint primrefID = *(uint *)(primref_index + i * stride + offset);

        const uint geomID    = PRIMREF_geomID(&primref[primrefID]);
        const uint primID0   = PRIMREF_primID0(&primref[primrefID]);
        const uint primID1   = PRIMREF_primID1(&primref[primrefID]);
        const uint geomFlags = PRIMREF_geomFlags(&primref[primrefID]);

        const uint3 tri0 = GRL_load_triangle(&geomDesc[geomID], primID0);
        const uint3 tri1 = GRL_load_triangle(&geomDesc[geomID], primID1);

        const struct TrianglePair q = TrianglePair_Constructor(tri0, primID0, tri1, primID1);

        const uint4 indices = q.a;
        const uint mask = 0xff; // FIXME: hardcoded mask
        float3 vtx0, vtx1, vtx2, vtx3;
        GRL_load_quad_vertices(&geomDesc[geomID], &vtx0, &vtx1, &vtx2, &vtx3, indices);

        setQuad(&quads[i], (float4)(vtx0,0), (float4)(vtx1,0), (float4)(vtx2,0), (float4)(vtx3,0), q.lb.x, q.lb.y, q.lb.z, geomID, primID0, primID1, mask, geomFlags );
    }

    if (get_local_id(0) + get_group_id(0)*get_local_size(0) == 0)
        bvh->quadLeafCur += numPrimitives ;
#endif
}

GRL_INLINE void create_procedural_leaf(global struct Globals *globals,
                            global struct AABB *primref,
                            local uint *primrefids,
                            uint numProcedurals,
                            struct QBVHNodeN *qnode,
                            global char *bvh_mem,
                            global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    if (get_local_id(0) >= 8)
        return;

    global struct BVHBase* bvh_base = (global struct BVHBase*)bvh_mem;

    /* first read geomID of all primitives */
    uint primrefID = -1;
    uint geomID = -1;
    uint geomFlags = 0;
    if (get_local_id(0) < numProcedurals)
    {
        primrefID = primrefids[get_local_id(0)];
        geomID = PRIMREF_geomID(&primref[primrefID]);
        geomFlags = PRIMREF_geomFlags( &primref[primrefID] );
    }

    // cannot sort by geomID as bounds in parent node are then wrong
    //ulong geomID_primrefID = (((ulong)geomID) << 32) | ((ulong)primrefID);
    //geomID_primrefID = sort8_ascending_ulong(geomID_primrefID);
    //geomID = geomID_primrefID >> 32;
    //primrefID = geomID_primrefID;

    /* We have to split at geomID boundaries into multiple leaves. This
   * block calculates the lane where a leaf starts and ends. */
    const uint geomIDprev = intel_sub_group_shuffle_up(0xFFFFFFFFu, geomID, 1u);
    const uint geomIDnext = intel_sub_group_shuffle_down(geomID, 0xFFFFFFFFu, 1u);
    const uint leaf_start = geomIDprev != geomID;
    const uint leaf_end = geomIDnext != geomID;
    const uint leaf_start_next = intel_sub_group_shuffle_down(leaf_start, 0u, 1u);

    /* This computes which leaf a lane processes. E.g. form geomID =
   * [3,3,4,4,4,0] we get leaf_id = [0,0,1,1,1,2] */
    //const uint leaf_id = sub_group_scan_inclusive_add(leaf_start); // FIXME: exclusive?

    /* This computes the n'th primitive a lane processes inside its
    * leaf. For the example above we compute leaf_prim =
    * [0,1,0,1,2,0]. */
    const uint leaf_prim = get_local_id(0) - sub_group_scan_inclusive_max(leaf_start ? get_local_id(0) : 0);

    /* from here on we allocate data and write to memory, thus only
   * lanes that process a primitive should continue. */
    if (get_local_id(0) >= numProcedurals)
        return;

    /* Here we allocate a single memory block for each required
     * ProceduralLeaf node. We do this from a single lane to ensure
     * the allocation is contiguous. */
    uint leaf_base_offset = 0;
    uint n_leafs = sub_group_reduce_add(leaf_start);
    if (get_local_id(0) == 0)
       leaf_base_offset = allocate_procedural_leaves( bvh_base, n_leafs );
    leaf_base_offset = sub_group_broadcast(leaf_base_offset, 0);

    /* Compute the leaf offset for each lane. */
    uint leaf_offset = leaf_base_offset + sub_group_scan_inclusive_add(leaf_start) - 1;

    struct ProceduralLeaf *pleaf = ((global struct ProceduralLeaf *)(bvh_mem)) + leaf_offset;

    /* write the procedural leaf headers */
    if (leaf_end)
    {
        pleaf->leafDesc.shaderIndex_geomMask = 0xFF000000 | (geomID & 0x00FFFFFF); // FIXME: use accessor function.   Future extensions may have shaderIndex != geomID
        pleaf->leafDesc.geomIndex_flags = geomID | (geomFlags<<30); // FIXME:  Use setter function
        pleaf->DW1 = 0xFFFFFFF0 | (leaf_prim + 1); // !!!
    }
    /* write the procedural leaf primIDs */
    pleaf->_primIndex[leaf_prim] = PRIMREF_primID0(&primref[primrefID]);

    /* update leaf node offset inside parent node */
    if (get_local_id(0) == 0)
    {
        QBVH6Node_set_offset(qnode, pleaf);
        QBVH6Node_set_type(qnode, NODE_TYPE_PROCEDURAL);
    }

    /* Let parent node children point to proper procedural leaf block
   * and primitive. */
    qnode->childData[get_local_id(0)] = leaf_start_next | (leaf_prim << 2);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
GRL_ANNOTATE_BIG_REG_REQ
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
primref_to_procedurals(global struct Globals *globals,
                                 global struct AABB *primref,
                                 global char *primref_index,
                                 global char *bvh_mem,
                                 global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc,
                                 const uint stride,
                                 const uint offset)
{
    global struct BVHBase *bvh = (global struct BVHBase *)bvh_mem;

    const uint numPrimitives = globals->numPrimitives;
    uint startID = get_group_id( 0 ) * get_local_size( 0 );
    uint endID   = min((uint)(startID + get_local_size( 0 )), numPrimitives);

    uint offset1 = stride * globals->numPrimitives;
    if (stride == 8)
        offset1 = 4;

    uint prev_start_back_pointer = startID == 0 ? -1 : *(uint *)(primref_index + (startID-1) * stride + offset1);
    /* start at leaf start */
    while (startID < numPrimitives)
    {
        const uint back_pointer = *(uint *)(primref_index + startID * stride + offset1);
        if (back_pointer != prev_start_back_pointer)
            break;
        startID++;
    }

    uint prev_end_back_pointer = *(uint *)(primref_index + (endID-1) * stride + offset1);
    /* end at next leaf start */
    while (endID < numPrimitives)
    {
        const uint back_pointer = *(uint *)(primref_index + endID * stride + offset1);
        if (back_pointer != prev_end_back_pointer)
            break;
        endID++;
    }

    local uint procedurals[16];

    for (uint lid = startID + get_local_id(0); lid < endID + get_local_id(0);)
    {
        /* load leaf start points and back_pointer */
        const uint primrefID = *(uint *)(primref_index + lid * stride + offset);
        uint back_pointer = *(uint *)(primref_index + lid * stride + offset1);
        uint prev_back_pointer = get_local_id(0) == 0 ? -1 : *(uint *)(primref_index + (lid-1) * stride + offset1);

        const uint leaf_start = back_pointer != prev_back_pointer;
        uint leaf_start_back_pointer = sub_group_broadcast(back_pointer, 0);

        /* compute number of primitives inside the leaf starting at lid */
        const uint leaf_id = sub_group_scan_inclusive_add(leaf_start);
        uint numPrimitives = 0;
        if (back_pointer == leaf_start_back_pointer && lid < endID)
            numPrimitives = sub_group_reduce_add(1);
        numPrimitives = sub_group_broadcast(numPrimitives, 0);

        procedurals[get_local_id(0)] = primrefID;

        struct QBVHNodeN *qnode = (struct QBVHNodeN *)bvh_mem + back_pointer;

        create_procedural_leaf(globals, primref, procedurals, numPrimitives, qnode, bvh_mem, geomDesc);

        lid += numPrimitives;
    }
}

GRL_INLINE void create_HW_instance_leaf(
    global struct BVHBase* bvh,
    global const struct GRL_RAYTRACING_INSTANCE_DESC* instDesc,
    uint dstLeafId,
    uint instanceIndex,
    uint rootNodeByteOffset,
    uint instanceMask)
{
    /* convert DXR instance to instance leaf node */
    global struct HwInstanceLeaf* leaves = (__global struct HwInstanceLeaf*)BVHBase_quadLeaves(bvh);
    HwInstanceLeaf_Constructor(&leaves[dstLeafId], instDesc, instanceIndex, rootNodeByteOffset, instanceMask);
}



GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel create_HW_instance_nodes(
    global const struct Globals *globals,
    global char *primref_index,
    global struct AABB *primref,
    global struct BVHBase *bvh,
    global struct GRL_RAYTRACING_INSTANCE_DESC *src_instances,
    uint32_t stride,
    uint32_t offset)
{
    uint dstLeafId = get_group_id(0) * MAX_HW_SIMD_WIDTH + get_sub_group_local_id();
    uint num_prims = globals->numPrimitives;
    if (dstLeafId >= num_prims)
        return;
    if( dstLeafId == 0 )
        bvh->instanceLeafEnd += 2*num_prims;

    /* get instance ID */
    const uint primrefID = *(uint *)(primref_index + dstLeafId * stride + offset);
    const uint instIndex = PRIMREF_instanceIndex(&primref[primrefID]);
    const uint rootByteOffset = PRIMREF_instanceRootNodeOffset(&primref[primrefID]);
    const uint instMask = PRIMREF_instanceMask(&primref[primrefID]);
    create_HW_instance_leaf(bvh, &src_instances[instIndex], dstLeafId, instIndex, rootByteOffset, instMask );
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel create_HW_instance_nodes_pointers(
    global const struct Globals *globals,
    global char *primref_index,
    global struct AABB *primref,
    global struct BVHBase *bvh,
    global void *instances_in,
    uint32_t stride,
    uint32_t offset)
{
    uint dstLeafId = get_group_id(0) * MAX_HW_SIMD_WIDTH + get_sub_group_local_id();
    uint num_prims = globals->numPrimitives;
    if (dstLeafId >= num_prims)
        return;
    if (dstLeafId == 0)
        bvh->instanceLeafEnd += 2 * num_prims;

    global const struct GRL_RAYTRACING_INSTANCE_DESC **instances =
        (global const struct GRL_RAYTRACING_INSTANCE_DESC **)instances_in;

    /* get instance ID */
    const uint primrefID = *(uint *)(primref_index + dstLeafId * stride + offset);
    const uint instIndex = PRIMREF_instanceIndex(&primref[primrefID]);
    const uint rootByteOffset = PRIMREF_instanceRootNodeOffset(&primref[primrefID]);
    const uint instMask = PRIMREF_instanceMask(&primref[primrefID]);
    create_HW_instance_leaf(bvh, instances[instIndex], dstLeafId, instIndex, rootByteOffset, instMask );
}
