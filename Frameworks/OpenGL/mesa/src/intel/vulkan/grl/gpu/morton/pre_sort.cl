//
// Copyright (C) 2009-2022 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "morton/morton_common.h"

GRL_INLINE uint get_morton_shift( uint numPrimitives )
{
    return 32 - clz( numPrimitives );
}

GRL_INLINE uint get_morton_shift_mask( uint numPrimitives )
{
    uint shift = get_morton_shift( numPrimitives );
    uint mask =(uint)(((ulong)1 << shift));
    return mask - 1; // separated due to problems in DX
}

__attribute__((reqd_work_group_size(1, 1, 1))) void kernel init( global struct Globals *globals )
{
    /* variable shift for putting morton code + index to 64 bit */
    const uint shift = 32 - clz(globals->numPrimitives);
    globals->shift = shift;
    globals->shift_mask = (uint)(((ulong)1 << shift));
    globals->shift_mask -= 1; // separated due to problems in DX
    globals->binary_hierarchy_root = 0;
    globals->morton_sort_in_flight = 0;
    globals->sort_iterations = get_morton_sort_lsb_req_iterations(shift);
}

/*

  This kernel create a morton code array containing a morton code and
  index into the primref array.

  The code uses the maximal number of bits for the morton code, such
  that the morton code and index can still both get stored in 64 bits.

  The algorithm first maps the centroids of the primitives and their
  bounding box diagonal into a 4D grid, and then interleaves all 4
  grid coordinates to construct the to morton code.

 */

__attribute__( (reqd_work_group_size( MAX_HW_SIMD_WIDTH, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) ) void kernel
create_morton_codes_indirect( global struct Globals* globals,
    global struct BVHBase* bvh,
    global struct AABB* primref,
    global struct MortonCodePrimitive* morton_codes,
    global struct MortonCodePrimitive* morton_codes_tmp,
    uint use_new_morton_sort)
{
    /* construct range of morton codes each work group should create */
    const uint numPrimitives = globals->numPrimitives;
    const uint startID = get_group_id( 0 ) * get_local_size( 0 );
    const uint endID   = min((uint)(startID + get_local_size(0)), numPrimitives);

    /* get lower and upper bounds of geometry and length of scene diagonal */
    const float3 lower = globals->centroidBounds.lower.xyz;
    const float3 upper = globals->centroidBounds.upper.xyz;
    const float diag = length( AABB3f_size( &bvh->Meta.bounds ).xyz );

    /* calculates the 4D grid */
    const uint shift = get_morton_shift( numPrimitives );
    const uint grid_size = 1 << (64 - shift) / 4;
    const float4 grid_base = (float4)(lower, 0.0f);
    const float4 grid_extend = (float4)(upper - lower, diag);
    const float4 grid_scale = select( (grid_size * 0.99f) / grid_extend, 0.0f, grid_extend == 0.0f ); // FIXME: 0.99f!!!!!

    const uint req_iterations = get_morton_sort_lsb_req_iterations(shift);

    /* each work group iterates over its range of morton codes to create */
    uint primID = startID + get_local_id( 0 );
    if( primID < endID )
    {
        /* calculate position inside 4D grid */
        float4 centroid2 = AABB_centroid2( &primref[primID] );
        centroid2.w = length( AABB_size( &primref[primID] ).xyz );
        const uint4 gridpos = convert_uint4_rtz( (centroid2 - grid_base) * grid_scale );

        /* calculate and store morton code */
        const ulong code = ulong_bitInterleave4D( gridpos );
        const ulong index_code = ((ulong)code << shift) | (ulong)primID;

        // It is required for morton code to be in morton_codes buffer after LSB sort finishes.
        // If there would be odd iteration number needed for sorting, it is needed
        // to skip some iterations of sorting. For odd number of iteration start with morton_codes_tmp buffer
        if(req_iterations & 1 && !use_new_morton_sort)
            morton_codes_tmp[primID].index_code = index_code;
        else
            morton_codes[primID].index_code = index_code;
    }
}

/*

  Initialization of the binary morton code hierarchy.

 */

__attribute__( (reqd_work_group_size( MAX_HW_SIMD_WIDTH, 1, 1 )) ) void kernel init_bottom_up_indirect( global struct Globals* globals,
    global struct BinaryMortonCodeHierarchy* bnodes )
{
    /* construct range each work group will process */
    const uint numPrimitives = globals->numPrimitives;
    const uint startID = get_group_id( 0 ) * get_local_size(0);
    const uint endID   = min((uint)(startID + get_local_size(0)), numPrimitives);

    /* each workgroup iterates over its range to initialize the binary BVH */
    uint i = startID + get_local_id( 0 );
    if( i < endID )
        BinaryMortonCodeHierarchy_init( &bnodes[i], 0, numPrimitives - 1 );
}
