//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "api_interface.h"
#include "common.h"
#include "instance.h"

#include "bvh_build_primref.h"

//#pragma OPENCL EXTENSION cl_khr_subgroup_non_uniform_vote : enable
//int sub_group_non_uniform_any(int predicate);

#define WINDOW_SIZE 16

/* Representation of two merged triangles. */
struct QuadIndices
{
    uint primID0, primID1;
    uint v0, v1, v2, v3;
};

/*

  This function calculates a PrimRef from a merged quad and writes
  this PrimRef to memory.

 */
GRL_INLINE void create_prim_ref(const uint geomID,
                            const struct QuadIndices quad,
                            global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc,
                            struct AABB *geometryBounds,
                            struct AABB *centroidBounds,
                            global uint *numPrimitives,
                            global struct AABB *primref)
{

    /* load quad vertices */
    const float4 vtx0 = GRL_load_vertex(geomDesc, quad.v0); // FIXME: these multiple load_vertex calls should get merged
    const float4 vtx1 = GRL_load_vertex(geomDesc, quad.v1);
    const float4 vtx2 = GRL_load_vertex(geomDesc, quad.v2);
    const float4 vtx3 = GRL_load_vertex(geomDesc, quad.v3);

    /* calculate bounds for quad */
    float4 lower = min(min(vtx0, vtx1), min(vtx2, vtx3));
    float4 upper = max(max(vtx0, vtx1), max(vtx2, vtx3));

    /* extend geometry and centroid bounds */
    const float4 centroid2 = lower + upper;
    AABB_extendlu(geometryBounds, lower, upper);
    AABB_extendlu(centroidBounds, centroid2, centroid2);

    PrimRef ref;
    PRIMREF_setAABB( &ref, lower.xyz, upper.xyz );
    PRIMREF_setQuadMetaData( &ref, quad.primID0, quad.primID1, geomID, GRL_get_Flags( geomDesc ) );

    /* store primref to memory */
    const uint offset = atomic_add_global(numPrimitives, 1);
    primref[offset] = ref;
}

/*

  This function calculates a PrimRef from a procedural and writes
  this PrimRef to memory.

 */
GRL_INLINE void create_prim_ref_procedural(global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc,
                                       const uint geomID,
                                       const uint primID,
                                       struct AABB *geometryBounds,
                                       struct AABB *centroidBounds,
                                       global uint *numPrimitives,
                                       global struct AABB *primref)
{
    /* load aabb from memory */
    struct GRL_RAYTRACING_AABB aabb = GRL_load_aabb(&geomDesc[geomID], primID);

    /* extend geometry and centroid bounds */
    float4 lower = (float4)(aabb.MinX, aabb.MinY, aabb.MinZ, 0.0f);
    float4 upper = (float4)(aabb.MaxX, aabb.MaxY, aabb.MaxZ, 0.0f);
    const float4 centroid2 = lower + upper;
    AABB_extendlu(geometryBounds, lower, upper);
    AABB_extendlu(centroidBounds, centroid2, centroid2);

    /* encode geomID, primID */
    uint geomFlags = GRL_get_Flags(&geomDesc[geomID]);

    PrimRef ref;
    PRIMREF_setAABB( &ref, lower.xyz, upper.xyz );
    PRIMREF_setProceduralMetaData( &ref, geomID, primID, geomFlags );

    /* store primref to memory */
    const uint offset = atomic_add_global(numPrimitives, 1);
    primref[offset] = ref;
}

/*

   This function performs a binary search to calculate the geomID and
   primID of the i'th primitive of the scene. For the search a
   prefix_sum array is used that stores for each location j the sum of
   the number of primitives of all meshes k with k<j.

*/

struct GeomPrimID
{
    uint geomID, primID;
};

struct GeomPrimID binary_search_geomID_primID(global uint *prefix_sum, const uint prefix_sum_size, const uint i)
{
    uint l = 0;
    uint r = prefix_sum_size;
    uint k = 0;

    while (r - l > 1)
    {
        const uint m = (l + r) / 2;
        k = prefix_sum[m];
        if (k <= i)
        {
            l = m;
        }
        else if (i < k)
        {
            r = m;
        }
    }

    struct GeomPrimID id;
    id.geomID = l;
    id.primID = i - prefix_sum[l];
    return id;
}

/*

  Checks if a vertex contains only finite floating point numbers.

 */

GRL_INLINE bool isfinite_vertex(float4 vtx)
{
    return isfinite(vtx.x) && isfinite(vtx.y) && isfinite(vtx.z);
}


/*
  Create primrefs from array of instance descriptors.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) void kernel
primrefs_from_DXR_instances(global struct Globals *globals,
                            global struct BVHBase* bvh,
                            global __const struct GRL_RAYTRACING_INSTANCE_DESC* instances,
                            uint numInstances,
                            global struct AABB *primrefs,
                            uint allowUpdate)
{
    const uint instanceIndex = get_sub_group_local_id() + get_group_id(0) * MAX_HW_SIMD_WIDTH;
    if (instanceIndex < numInstances)
    {
        global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instances + instanceIndex;

        primrefs_from_instances(
            globals,
            bvh,
            instance,
            instanceIndex,
            primrefs,
            0,
            allowUpdate);
    }
}

/*
  Create primrefs from array of instance descriptors.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
void kernel
primrefs_from_DXR_instances_indirect(global struct Globals *globals,
                            global struct BVHBase* bvh,
                            global __const struct GRL_RAYTRACING_INSTANCE_DESC* instances,
                            global struct IndirectBuildRangeInfo* indirect_data,
                            global struct AABB *primrefs,
                            uint allowUpdate)
{
    // TODO: On DG2, we have 8 dwords of 'inline data' which can be pushed
    // directly to the kernel. THe rest of the kernel args are pulled using
    // loads from memory. It may be more efficient to put 'numInstances' and
    // 'allowUpdate' into 'globals'

    const uint instanceIndex =  get_local_id(0) + get_group_id(0) * MAX_HW_SIMD_WIDTH;

    if (instanceIndex < indirect_data->primitiveCount)
    {
        instances = (global __const struct GRL_RAYTRACING_INSTANCE_DESC*)
            (((global char*)instances) + indirect_data->primitiveOffset);
        global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instances + instanceIndex;
        primrefs_from_instances(
            globals,
            bvh,
            instance,
            instanceIndex,
            primrefs,
            0,
            allowUpdate);
    }
}

/*
  Create primrefs from array of pointers to instance descriptors.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) void kernel
primrefs_from_DXR_instances_pointers(global struct Globals *globals,
                                     global struct BVHBase* bvh,
                                     global void *instances_in,
                                     uint numInstances,
                                     global struct AABB *primrefs,
                                     uint allowUpdate)
{
    global const struct GRL_RAYTRACING_INSTANCE_DESC **instances =
        (global const struct GRL_RAYTRACING_INSTANCE_DESC **)instances_in;

    const uint instanceIndex = get_sub_group_local_id() + get_group_id(0) * MAX_HW_SIMD_WIDTH;
    if (instanceIndex < numInstances)
    {
        global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instances[instanceIndex];

        primrefs_from_instances(
            globals,
            bvh,
            instance,
            instanceIndex,
            primrefs,
            0,
            allowUpdate);
    }
}

/*
  Create primrefs from array of pointers to instance descriptors.
 */
 GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
void kernel
primrefs_from_DXR_instances_pointers_indirect(global struct Globals *globals,
                                              global struct BVHBase* bvh,
                                              global void *instances_in,
                                              global struct AABB *primrefs,
                                              global struct IndirectBuildRangeInfo* indirect_data,
                                              uint allowUpdate)
{
    global const struct GRL_RAYTRACING_INSTANCE_DESC **instances =
        (global const struct GRL_RAYTRACING_INSTANCE_DESC **)instances_in;

    const uint instanceIndex = get_local_id(0) + get_group_id(0) * MAX_HW_SIMD_WIDTH;

    if (instanceIndex < indirect_data->primitiveCount)
    {
        instances = (global const struct GRL_RAYTRACING_INSTANCE_DESC**)
            (((global char*)instances) + indirect_data->primitiveOffset);
        global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instances[instanceIndex];

        primrefs_from_instances(
            globals,
            bvh,
            instance,
            instanceIndex,
            primrefs,
            0,
            allowUpdate);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool can_pair( uint3 a, uint3 b )
{
    bool match0 = any( a.xxx == b.xyz ) ? 1 : 0;
    bool match1 = any( a.yyy == b.xyz ) ? 1 : 0;
    bool match2 = any( a.zzz == b.xyz ) ? 1 : 0;
    return (match0 + match1 + match2) >= 2;
}

void reduce_bounds(
    float3 lower,
    float3 upper,
    global struct Globals* globals,
    global struct BVHBase* bvh )
{

    // reduce centroid bounds... make sure to exclude lanes with invalid AABBs
    float3 cent = lower + upper;
    float3 cent_lower = select( (float3)(INFINITY, INFINITY, INFINITY), cent, lower <= upper);
    float3 cent_upper = select(-(float3)(INFINITY, INFINITY, INFINITY), cent, lower <= upper);

    // reduce geo bounds
    AABB3f_atomic_merge_global_sub_group_lu( &bvh->Meta.bounds, lower, upper );
    AABB_global_atomic_merge_sub_group_lu(&globals->centroidBounds, cent_lower, cent_upper );
}


struct TriState
{
    bool valid;
    uint prim_index;
    uint pairing;
    uint3 indices;
    float3 lower;
    float3 upper;
};

#define NOT_PAIRED 0xffffffff

void load_triangle_data(uniform global char* index_buffer,
                        uniform const uint index_format,
                        uniform global char* vertex_buffer,
                        uniform const uint vertex_format,
                        uniform const uint vertex_stride,
                        uniform global float* transform_buffer,
                        uniform uint total_vert_count,
                        struct TriState* state,
                        float4* v)
{
        state->indices = GRL_load_indices_from_buffer(index_buffer, index_format, state->prim_index );

        const uint last_vertex = total_vert_count - 1;
        const uint x = min(state->indices.x, last_vertex);
        const uint y = min(state->indices.y, last_vertex);
        const uint z = min(state->indices.z, last_vertex);

        GRL_load_triangle_vertices(vertex_buffer, vertex_format, vertex_stride, transform_buffer, x, y, z, v);
}

struct TriState load_triangle( uniform global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
                        uniform uint base,
                        uniform uint num_prims,
                        uniform uint total_vert_count )
{

    struct TriState state;
    state.pairing        = NOT_PAIRED;
    state.valid          = false;
    state.prim_index     = base + get_sub_group_local_id();
    state.lower = (float3)(INFINITY, INFINITY, INFINITY);
    state.upper = -(float3)(INFINITY, INFINITY, INFINITY);

    if (state.prim_index < num_prims)
    {
        state.valid = true;
        float4 v[3];
        load_triangle_data((global char*)geomDesc->Desc.Triangles.pIndexBuffer,
                        geomDesc->Desc.Triangles.IndexFormat,
                        (global char*)geomDesc->Desc.Triangles.pVertexBuffer,
                        geomDesc->Desc.Triangles.VertexFormat,
                        geomDesc->Desc.Triangles.VertexBufferByteStride,
                        (global float*)geomDesc->Desc.Triangles.pTransformBuffer,
                        total_vert_count,
                        &state,
                        v);

        if (state.indices.x >= total_vert_count || state.indices.y >= total_vert_count || state.indices.z >= total_vert_count ||
            !isfinite_vertex(v[0]) || !isfinite_vertex(v[1]) || !isfinite_vertex(v[2]) ||
            state.indices.x == state.indices.y || state.indices.x == state.indices.z || state.indices.y == state.indices.z)
        {
            state.valid = false;
        }
        else
        {
            state.lower.xyz = min(v[2].xyz, min(v[1].xyz, v[0].xyz));
            state.upper.xyz = max(v[2].xyz, max(v[1].xyz, v[0].xyz));
        }
    }
    return state;
}

void broadcast_triangles_local( struct TriState* state  )
{
    varying uint my_prim    = state->prim_index;
    varying uint my_pairing = state->pairing;
    varying float3 my_lower = state->lower;
    varying float3 my_upper = state->upper;
    varying bool valid      = state->valid;
    varying uint3 indices   = state->indices;

    for (uniform uint broadcast_lane = 0; broadcast_lane < get_sub_group_size(); broadcast_lane++)
    {
        // don't broadcast invalid prims
        if ( !sub_group_broadcast( valid, broadcast_lane ) )
            continue;

        uint broadcast_pairing = sub_group_broadcast(my_pairing, broadcast_lane);
        uint broadcast_prim = sub_group_broadcast(my_prim, broadcast_lane);

        if (broadcast_pairing == NOT_PAIRED)
        {
            // if the broadcast prim is not paired already, all unpaired lanes attempt to pair with it
            bool pairable = false;
            uint3 other_indices = sub_group_broadcast_uint3( indices, broadcast_lane );
            if (broadcast_prim != my_prim && my_pairing == NOT_PAIRED && valid )
            {
                pairable = can_pair( indices, other_indices );
            }


            uint pairable_lane = ctz(intel_sub_group_ballot(pairable));
            if (valid && pairable_lane < get_sub_group_size())
            {
                // pair the broadcast primitive with the first lane that can accept it
                float3 broadcast_lower = sub_group_broadcast_float3(my_lower.xyz, broadcast_lane);
                float3 broadcast_upper = sub_group_broadcast_float3(my_upper.xyz, broadcast_lane);
                if (get_sub_group_local_id() == pairable_lane)
                {
                    my_pairing = broadcast_prim;
                    my_lower.xyz = min(my_lower.xyz, broadcast_lower);
                    my_upper.xyz = max(my_upper.xyz, broadcast_upper);
                }

                // pair the broadcast primitive with the same that was paired to it
                uint pairable_prim = sub_group_broadcast(my_pairing, pairable_lane);
                if (get_sub_group_local_id() == broadcast_lane)
                {
                    my_pairing = pairable_prim;
                }
            }
        }
        else
        {
            //
            // if this lane was already paired with the broadcasting tri
            //   in an earlier loop iteration, then record the pairing in this lane's registers
            float3 broadcast_lower = sub_group_broadcast_float3(my_lower.xyz, broadcast_lane);
            float3 broadcast_upper = sub_group_broadcast_float3(my_upper.xyz, broadcast_lane);
            if (broadcast_pairing == my_prim)
            {
                my_pairing = broadcast_prim;
                my_lower.xyz = min(my_lower.xyz, broadcast_lower);
                my_upper.xyz = max(my_upper.xyz, broadcast_upper);
            }
        }
    }

    state->pairing = my_pairing;
    state->lower = my_lower;
    state->upper = my_upper;
}


void broadcast_triangles_nonlocal(struct TriState* state, const struct TriState* other )
{
    varying uint my_prim = state->prim_index;
    varying uint my_pairing = state->pairing;
    varying float3 my_lower = state->lower;
    varying float3 my_upper = state->upper;
    varying bool valid = state->valid;
    varying uint3 indices = state->indices;

    for (uniform uint broadcast_lane = 0; broadcast_lane < get_sub_group_size(); broadcast_lane++)
    {
        // don't broadcast invalid prims
        if (!sub_group_broadcast(other->valid, broadcast_lane))
            continue;

        uint broadcast_pairing = sub_group_broadcast(other->pairing, broadcast_lane);
        uint broadcast_prim = sub_group_broadcast(other->prim_index, broadcast_lane);

        if (broadcast_pairing == NOT_PAIRED)
        {
            // if the broadcast prim is not paired already, all unpaired lanes attempt to pair with it
            bool pairable = false;
            if ( my_pairing == NOT_PAIRED && valid )
            {
                uint3 other_indices = sub_group_broadcast_uint3(other->indices, broadcast_lane);
                pairable = can_pair(indices, other_indices);
            }

            // pair the broadcast primitive with the first lane that can accept it
            uint pairable_mask = intel_sub_group_ballot(pairable);
            if (valid && (ctz(pairable_mask) == get_sub_group_local_id()))
            {
                my_pairing = broadcast_prim;
                my_lower.xyz = min(my_lower.xyz, sub_group_broadcast_float3(other->lower.xyz, broadcast_lane));
                my_upper.xyz = max(my_upper.xyz, sub_group_broadcast_float3(other->upper.xyz, broadcast_lane));
            }
        }

    }

    state->pairing = my_pairing;
    state->lower = my_lower;
    state->upper = my_upper;
}

GRL_INLINE void do_triangles_to_primrefs(
    global struct Globals*               globals,
    global struct BVHBase*               bvh,
    global struct AABB*                  primref,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    uint                                 geomID_and_flags,
    const uint                           num_prims)
{
    uint geomID             = geomID_and_flags & 0x00ffffff;
    uint geom_flags         = geomID_and_flags >> 24;
    uint prim_base          = get_group_id(0) * get_local_size(0);
    uint total_vert_count   = GRL_get_triangles_VertexCount(geomDesc);

    struct TriState tri = load_triangle( geomDesc, prim_base, num_prims, total_vert_count );
    broadcast_triangles_local( &tri );


    // we will produce output if the lane creates a triangle (my_pairing == NOT_PAIRED)
    // or for the lane corresponding to the larger of two triangles
    bool will_write = (tri.pairing > tri.prim_index) && tri.valid;
    uint write_mask = intel_sub_group_ballot(will_write);
    uint write_offs = subgroup_bit_prefix_exclusive( write_mask );
    uint write_count = popcount(write_mask);

    // allocate space in primref buffer
    uint write_base;
    if( get_sub_group_local_id() == 0 )
        write_base = atomic_add_global( &globals->numPrimitives, write_count );
    write_offs += sub_group_broadcast( write_base, 0 );

    uint primID0 = tri.prim_index;
    uint primID1 = (tri.pairing != NOT_PAIRED) ? tri.pairing : tri.prim_index;

    if (will_write)
    {
        PrimRef ref;
        PRIMREF_setAABB(&ref, tri.lower.xyz, tri.upper.xyz);
        PRIMREF_setQuadMetaData(&ref, primID0, primID1, geomID, geom_flags);
        uint8 val = (uint8)(
            as_uint(ref.lower.x), as_uint(ref.lower.y), as_uint(ref.lower.z), as_uint(ref.lower.w),
            as_uint(ref.upper.x), as_uint(ref.upper.y), as_uint(ref.upper.z), as_uint(ref.upper.w));
        store_uint8_L1WB_L3WB((global uint8*)(primref + write_offs), 0, val);
    }

    reduce_bounds( tri.lower, tri.upper, globals, bvh );
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
triangles_to_primrefs(
    global struct Globals* globals,
    global struct BVHBase* bvh,
    global struct AABB* primref,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    uint geomID_and_flags,
    uint num_prims
    )
{
    do_triangles_to_primrefs(globals, bvh, primref, geomDesc, geomID_and_flags, num_prims);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel
triangles_to_primrefs_indirect(
    global struct Globals*                globals,
    global struct BVHBase*                bvh,
    global struct AABB*                   primref,
    global GRL_RAYTRACING_GEOMETRY_DESC*  geomDesc,
    global struct IndirectBuildRangeInfo* indirect_data,
    uint                                  geomID_and_flags)
{
    const uint num_prims = indirect_data->primitiveCount;
    do_triangles_to_primrefs(globals, bvh, primref, geomDesc, geomID_and_flags, num_prims);
}

GRL_INLINE void do_procedurals_to_primrefs(
    global struct Globals* globals,
    global struct BVHBase* bvh,
    global struct AABB* primref,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    uint geomID_and_flags,
    const uint num_prims)
{
    uint geomID    = geomID_and_flags & 0x00ffffff;
    uint geomFlags = geomID_and_flags >> 24;

    uint primID   = get_group_id(0) * get_local_size(0) + get_sub_group_local_id();

    bool create_primref = false;
    float3 lower =  (float3)(INFINITY, INFINITY, INFINITY);
    float3 upper = -(float3)(INFINITY, INFINITY, INFINITY);
    if (primID < num_prims)
    {
        /* check if procedural is valid */
        struct GRL_RAYTRACING_AABB aabb = GRL_load_aabb(geomDesc, primID);
        const bool valid_min = isfinite(aabb.MinX) && isfinite(aabb.MinY) && isfinite(aabb.MinZ);
        const bool valid_max = isfinite(aabb.MaxX) && isfinite(aabb.MaxY) && isfinite(aabb.MaxZ);
        if (valid_min & valid_max)
        {
            /* load aabb from memory */
            float3 l = (float3)(aabb.MinX, aabb.MinY, aabb.MinZ);
            float3 u = (float3)(aabb.MaxX, aabb.MaxY, aabb.MaxZ);

            // convert degenerate boxes to points at the box centroid
            lower = min( l, u );
            upper = max( l, u );

            create_primref = true;
        }
    }

    uint write_mask = intel_sub_group_ballot(create_primref);
    uint write_offs = subgroup_bit_prefix_exclusive(write_mask);
    uint write_count = popcount(write_mask);

    // allocate space in primref buffer
    uint write_base;
    if (get_sub_group_local_id() == 0)
        write_base = atomic_add_global(&globals->numPrimitives, write_count);
    write_offs += sub_group_broadcast(write_base, 0);

    // write the primref
    if (create_primref)
    {
        PrimRef ref;
        PRIMREF_setAABB(&ref, lower.xyz, upper.xyz);
        PRIMREF_setProceduralMetaData(&ref, geomID, primID, geomFlags);
        primref[write_offs] = ref;
    }

    reduce_bounds(lower, upper, globals, bvh);

}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
procedurals_to_primrefs(
    global struct Globals* globals,
    global struct BVHBase* bvh,
    global struct AABB* primref,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    uint geomID_and_flags,
    uint num_prims
    )
{
    do_procedurals_to_primrefs(globals, bvh, primref, geomDesc, geomID_and_flags, num_prims);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
void kernel
procedurals_to_primrefs_indirect(
    global struct Globals* globals,
    global struct BVHBase* bvh,
    global struct AABB* primref,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    global const struct IndirectBuildRangeInfo* indirect_data,
    uint geomID_and_flags
    )
{
    const uint num_prims = indirect_data->primitiveCount;
    do_procedurals_to_primrefs(globals, bvh, primref, geomDesc, geomID_and_flags, num_prims);
}
