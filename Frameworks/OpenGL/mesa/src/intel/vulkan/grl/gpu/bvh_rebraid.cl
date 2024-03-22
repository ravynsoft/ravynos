//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "AABB.h"
#include "GRLGen12.h"
#include "api_interface.h"
#include "common.h"
#include "qbvh6.h"

#define MAX_SPLITS_PER_INSTANCE 64
#define NUM_REBRAID_BINS 32

#define NUM_CHILDREN 6
#define MAX_NODE_OFFSET 65535 // can't open nodes whose offsets exceed this

// OCL/DPC++ *SHOULD* have a uniform keyword... but they dont... so I'm making my own
#define uniform
#define varying

#define SGPRINT_UNIFORM(fmt,val)    {sub_group_barrier(CLK_LOCAL_MEM_FENCE); if( get_sub_group_local_id() == 0 ) { printf(fmt,val); }}

#define SGPRINT_6x(prefix,fmt,type,val)  {\
                                        type v0 = sub_group_broadcast( val, 0 );\
                                        type v1 = sub_group_broadcast( val, 1 );\
                                        type v2 = sub_group_broadcast( val, 2 );\
                                        type v3 = sub_group_broadcast( val, 3 );\
                                        type v4 = sub_group_broadcast( val, 4 );\
                                        type v5 = sub_group_broadcast( val, 5 );\
                                        sub_group_barrier(CLK_LOCAL_MEM_FENCE); \
                                        if( get_sub_group_local_id() == 0 ) { \
                                        printf(prefix fmt fmt fmt fmt fmt fmt "\n" , \
                                            v0,v1,v2,v3,v4,v5);}}


#define SGPRINT_16x(prefix,fmt,type,val)  {\
                                        type v0 = sub_group_broadcast( val, 0 );\
                                        type v1 = sub_group_broadcast( val, 1 );\
                                        type v2 = sub_group_broadcast( val, 2 );\
                                        type v3 = sub_group_broadcast( val, 3 );\
                                        type v4 = sub_group_broadcast( val, 4 );\
                                        type v5 = sub_group_broadcast( val, 5 );\
                                        type v6 = sub_group_broadcast( val, 6 );\
                                        type v7 = sub_group_broadcast( val, 7 );\
                                        type v8 = sub_group_broadcast( val, 8 );\
                                        type v9 = sub_group_broadcast( val, 9 );\
                                        type v10 = sub_group_broadcast( val, 10 );\
                                        type v11 = sub_group_broadcast( val, 11 );\
                                        type v12 = sub_group_broadcast( val, 12 );\
                                        type v13 = sub_group_broadcast( val, 13 );\
                                        type v14 = sub_group_broadcast( val, 14 );\
                                        type v15 = sub_group_broadcast( val, 15 );\
                                        sub_group_barrier(CLK_LOCAL_MEM_FENCE); \
                                        if( get_sub_group_local_id() == 0 ) { \
                                        printf(prefix fmt fmt fmt fmt fmt fmt fmt fmt \
                                                      fmt fmt fmt fmt fmt fmt fmt fmt"\n" , \
                                            v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15);}}

#if 1
#define GRL_ATOMIC_INC(addr) atomic_add(addr, 1);
#else 
#define GRL_ATOMIC_INC(addr) atomic_inc(addr);
#endif

#if 0
#define LOOP_TRIPWIRE_INIT uint _loop_trip=0;

#define LOOP_TRIPWIRE_INCREMENT(max_iterations,name) \
    _loop_trip++;\
    if ( _loop_trip > max_iterations )\
    {\
        printf( "@@@@@@@@@@@@@@@@@@@@ TRIPWIRE!!!!!!!!!!!\n" );\
        printf( name"\n");\
        break;\
    }
#else

#define LOOP_TRIPWIRE_INIT 
#define LOOP_TRIPWIRE_INCREMENT(max_iterations,name)

#endif



typedef struct SGHeap
{
    uint32_t key_value;
    bool lane_mask;
} SGHeap;

GRL_INLINE void SGHeap_init(uniform SGHeap *h)
{
    h->lane_mask = false;
    h->key_value = 0xbaadf00d;
}

GRL_INLINE bool SGHeap_full(uniform SGHeap *h)
{
    return sub_group_all(h->lane_mask);
}
GRL_INLINE bool SGHeap_empty(uniform SGHeap *h)
{
    return sub_group_all(!h->lane_mask);
}

GRL_INLINE bool SGHeap_get_lane_mask(uniform SGHeap *h)
{
    return h->lane_mask;
}
GRL_INLINE uint16_t SGHeap_get_lane_values(uniform SGHeap *h)
{
    return (h->key_value & 0xffff);
}

GRL_INLINE ushort isolate_lowest_bit( ushort m )
{
    return m & ~(m - 1);
}


// lane i receives the index of the ith set bit in mask.  
GRL_INLINE ushort subgroup_bit_rank( uniform ushort mask )
{
    varying ushort lane = get_sub_group_local_id();
    ushort idx = 16;
    for ( uint i = 0; i < NUM_CHILDREN; i++ )
    {
        ushort lo = isolate_lowest_bit( mask );
        mask = mask ^ lo;
        idx = (lane == i) ? lo : idx;
    }

    return ctz( idx );
}

// push a set of elements spread across a subgroup.  Return mask of elements that were not pushed
GRL_INLINE uint16_t SGHeap_vectorized_push(uniform SGHeap *h, varying uint16_t key, varying uint16_t value, uniform ushort push_mask)
{    

#if 0 // an attempt to make this algorithm branchless
    varying uint key_value = (((uint)key) << 16) | ((uint)value);
    uniform ushort free_mask = intel_sub_group_ballot( !h->lane_mask );

    varying ushort free_slot_idx = subgroup_bit_prefix_exclusive( free_mask ); // for each heap slot, what is its position in a compacted list of free slots (prefix sum)
    varying ushort push_idx      = subgroup_bit_prefix_exclusive( push_mask );  // for each lane, what is its position in a compacted list of pushing lanes (prefix sum)

    uniform ushort num_pushes = min( popcount( free_mask ), popcount( push_mask ) );

    varying ushort push_index = subgroup_bit_rank( push_mask ); // lane i gets the index of the i'th set bit in push_mask
    
    varying uint shuffled = intel_sub_group_shuffle( key_value, intel_sub_group_shuffle( push_index, free_slot_idx ) );
    varying bool pushed = false;
    if ( !h->lane_mask && free_slot_idx < num_pushes )
    {
        h->lane_mask = true;
        h->key_value = shuffled;
        pushed = true;
    }

    return push_mask & intel_sub_group_ballot( push_idx >= num_pushes );
#else

    varying uint lane = get_sub_group_local_id();

    varying uint key_value = (((uint)key) << 16) | ((uint)value);
    uniform ushort free_mask = intel_sub_group_ballot(!h->lane_mask);

    // TODO_OPT:  Look for some clever way to remove this loop
    while (free_mask && push_mask)
    {
        // insert first active child into first available lane
        uniform uint child_id = ctz(push_mask);
        uniform uint victim_lane = ctz(free_mask);
        uniform uint kv = sub_group_broadcast( key_value, child_id );
        if (victim_lane == lane)
        {
            h->lane_mask = true;
            h->key_value = kv;
        }
        push_mask ^= (1 << child_id);
        free_mask ^= (1 << victim_lane);
    }

    return push_mask;

#endif
}

// push an item onto a heap that is full except for one slot
GRL_INLINE void SGHeap_push_and_fill(uniform SGHeap *h, uniform uint16_t key, uniform uint16_t value)
{
    uniform uint32_t key_value = (((uint)key) << 16) | value;
    if (!h->lane_mask)
    {
        h->lane_mask = true;
        h->key_value = key_value; // only one lane will be active at this point
    }
}

// pop the min item from a full heap
GRL_INLINE void SGHeap_full_pop_min(uniform SGHeap *h, uniform uint16_t *key_out, uniform uint16_t *value_out)
{
    varying uint lane = get_sub_group_local_id();
    uniform uint kv = sub_group_reduce_min(h->key_value);
    if (h->key_value == kv)
        h->lane_mask = false;

    *key_out   = (kv >> 16);
    *value_out = (kv & 0xffff);
}

// pop the max item from a heap
GRL_INLINE void SGHeap_pop_max(uniform SGHeap *h, uniform uint16_t *key_out, uniform uint16_t *value_out)
{
    uniform uint lane = get_sub_group_local_id();
    uniform uint kv = sub_group_reduce_max(h->lane_mask ? h->key_value : 0);
    if (h->key_value == kv)
        h->lane_mask = false;

    *key_out = (kv >> 16);
    *value_out = (kv & 0xffff);
}

GRL_INLINE void SGHeap_printf( SGHeap* heap )
{
    uint key = heap->key_value >> 16;
    uint value = heap->key_value & 0xffff;
    
    if ( get_sub_group_local_id() == 0)
        printf( "HEAP: \n" );
    SGPRINT_16x( "  mask: ", "%6u ", bool, heap->lane_mask );
    SGPRINT_16x( "  key : ", "0x%04x ", uint, key );
    SGPRINT_16x( "  val : ", "0x%04x ", uint, value );

}

GRL_INLINE float transformed_aabb_halfArea(float3 lower, float3 upper, const float *Transform)
{
    // Compute transformed extent per 'transform_aabb'.  Various terms cancel
    float3 Extent = upper - lower;
    float ex = Extent.x * fabs(Transform[0]) + Extent.y * fabs(Transform[1]) + Extent.z * fabs(Transform[2]);
    float ey = Extent.x * fabs(Transform[4]) + Extent.y * fabs(Transform[5]) + Extent.z * fabs(Transform[6]);
    float ez = Extent.x * fabs(Transform[8]) + Extent.y * fabs(Transform[9]) + Extent.z * fabs(Transform[10]);

    return (ex * ey) + (ey * ez) + (ex * ez);
}

GRL_INLINE uint16_t quantize_area(float relative_area)
{
    // clamp relative area at 0.25 (1/4 of root area)
    //  and apply a non-linear distribution because most things in real scenes are small
    relative_area = pow(min(1.0f, relative_area * 4.0f), 0.125f);   
    return convert_ushort_rtn( relative_area * 65535.0f );    
}

GRL_INLINE varying uint16_t SUBGROUP_get_child_areas(uniform InternalNode *n,
                                                 uniform const float *Transform,
                                                 uniform float relative_area_scale)
{
    varying uint16_t area;
    varying uint16_t lane = get_sub_group_local_id();
    varying int exp_x = n->exp_x;
    varying int exp_y = n->exp_y;
    varying int exp_z = n->exp_z;

    {
        // decode the AABB positions.  Lower in the bottom 6 lanes, upper in the top
        uniform uint8_t *px = &n->lower_x[0];
        uniform uint8_t *py = &n->lower_y[0];
        uniform uint8_t *pz = &n->lower_z[0];

        varying float fx = convert_float(px[lane]);
        varying float fy = convert_float(py[lane]);
        varying float fz = convert_float(pz[lane]);
        fx = n->lower[0] + bitShiftLdexp(fx, exp_x - 8);
        fy = n->lower[1] + bitShiftLdexp(fy, exp_y - 8);
        fz = n->lower[2] + bitShiftLdexp(fz, exp_z - 8);

        // transform the AABBs to world space
        varying float3 lower = (float3)(fx, fy, fz);
        varying float3 upper = intel_sub_group_shuffle(lower, lane + 6);

        {
 
            // TODO_OPT:  This is only utilizing 6 lanes.
            //  We might be able to do better by vectorizing the calculation differently
            float a1 = transformed_aabb_halfArea( lower, upper, Transform );
            float a2 = a1 * relative_area_scale;
            area = quantize_area( a2 );
        }
    }

    return area;
}



GRL_INLINE ushort get_child_area( 
    InternalNode* n, 
    ushort child,
    const float* Transform,
    float relative_area_scale )
{
    uint16_t area;
    uint16_t lane = get_sub_group_local_id();
    int exp_x = n->exp_x;
    int exp_y = n->exp_y;
    int exp_z = n->exp_z;

    // decode the AABB positions.  Lower in the bottom 6 lanes, upper in the top
    uint8_t* px = &n->lower_x[0];
    uint8_t* py = &n->lower_y[0];
    uint8_t* pz = &n->lower_z[0];

    float3 lower, upper;
    lower.x = convert_float( n->lower_x[child] );
    lower.y = convert_float( n->lower_y[child] );
    lower.z = convert_float( n->lower_z[child] );
    upper.x = convert_float( n->upper_x[child] );
    upper.y = convert_float( n->upper_y[child] );
    upper.z = convert_float( n->upper_z[child] );

    lower.x = bitShiftLdexp( lower.x, exp_x - 8 ); // NOTE:  the node's 'lower' field cancels out, so don't add it
    lower.y = bitShiftLdexp( lower.y, exp_y - 8 ); //    see transform_aabb_halfArea
    lower.z = bitShiftLdexp( lower.z, exp_z - 8 );
    upper.x = bitShiftLdexp( upper.x, exp_x - 8 );
    upper.y = bitShiftLdexp( upper.y, exp_y - 8 );
    upper.z = bitShiftLdexp( upper.z, exp_z - 8 );

    float a1 = transformed_aabb_halfArea( lower, upper, Transform );
    float a2 = a1 * relative_area_scale;
    area = quantize_area( a2 );

    return area;
}


GRL_INLINE varying int SUBGROUP_get_child_offsets(uniform InternalNode *n)
{
    varying uint lane = get_sub_group_local_id();
    varying uint child = (lane < NUM_CHILDREN) ? lane : 0;

    varying uint block_incr = InternalNode_GetChildBlockIncr( n, child );

    //varying uint prefix = sub_group_scan_exclusive_add( block_incr );
    varying uint prefix;
    if ( NUM_CHILDREN == 6 ) 
    {
        prefix = block_incr + intel_sub_group_shuffle_up( 0u, block_incr, 1u );
        prefix = prefix + intel_sub_group_shuffle_up( 0u, prefix, 2 );
        prefix = prefix + intel_sub_group_shuffle_up( 0u, prefix, 4 );        
        prefix = prefix - block_incr;
    }

    return n->childOffset + prefix;
}


// compute the maximum number of leaf nodes that will be produced given 'num_splits' node openings
GRL_INLINE uint get_num_nodes(uint num_splits, uint max_children)
{
    // each split consumes one node and replaces it with N nodes
    //   there is initially one node
    //  number of nodes is thus:  N*s + 1 - s ==> (N-1)*s + 1
    return (max_children - 1) * num_splits + 1;
}

// compute the number of node openings that can be performed given a fixed extra node budget
GRL_INLINE uint get_num_splits(uint num_nodes, uint max_children)
{
    // inverse of get_num_nodes:   x = (n-1)s + 1
    //                             s = (x-1)/(n-1)
    if (num_nodes == 0)
        return 0;

    return (num_nodes - 1) / (max_children - 1);
}

GRL_INLINE uint get_rebraid_bin_index(uint16_t quantized_area, uint NUM_BINS)
{
    // arrange bins in descending order by size
    float relative_area = quantized_area * (1.0f/65535.0f);
    relative_area = 1.0f - relative_area; // arrange bins largest to smallest
    size_t bin = round(relative_area * (NUM_BINS - 1));
    return bin;
}

GRL_INLINE global InternalNode *get_node(global BVHBase *base, int incr)
{
    global char *ptr = (((global char *)base) + BVH_ROOT_NODE_OFFSET); // NOTE: Assuming this will be hoisted out of inner loops

    return (global InternalNode *)(ptr + incr * 64);
}

GRL_INLINE bool is_aabb_valid(float3 lower, float3 upper)
{
    return all(isfinite(lower)) &&
           all(isfinite(upper)) &&
           all(lower <= upper);
}

GRL_INLINE bool is_node_openable(InternalNode *n)
{
    // TODO_OPT: Optimize me by fetching dwords instead of looping over bytes
    // TODO: OPT:  Pre-compute openability and pack into the pad byte next to the nodeType field??
    bool openable = n->nodeType == NODE_TYPE_INTERNAL;
    if ( openable )
    {
        for ( uint i = 0; i < NUM_CHILDREN; i++ )
        {
            bool valid = InternalNode_IsChildValid( n, i );
            uint childType = InternalNode_GetChildType( n, i );
            openable = openable & (!valid || (childType == NODE_TYPE_INTERNAL));
        }
    }

    return openable;
}


GRL_INLINE bool SUBGROUP_can_open_root(
    uniform global BVHBase *bvh_base,
    uniform const struct GRL_RAYTRACING_INSTANCE_DESC* instance
    )
{
    if (bvh_base == 0 || GRL_get_InstanceMask(instance) == 0)
        return false;

    // TODO_OPT: SG-vectorize this AABB test
    uniform float3 root_lower = AABB3f_load_lower(&bvh_base->Meta.bounds);
    uniform float3 root_upper = AABB3f_load_upper(&bvh_base->Meta.bounds);
    if (!is_aabb_valid(root_lower, root_upper))
        return false;

    uniform global InternalNode *node = get_node(bvh_base, 0);
    if ( node->nodeType != NODE_TYPE_INTERNAL )
        return false; 

    varying bool openable = true;
    varying uint lane = get_sub_group_local_id();
    if (lane < NUM_CHILDREN)
    {
        varying uint childType = InternalNode_GetChildType(node, lane);
        varying bool valid = InternalNode_IsChildValid(node, lane);
        openable = childType == NODE_TYPE_INTERNAL || !valid;
    }

    return sub_group_all(openable);
}



GRL_INLINE
varying uint2
SUBGROUP_count_instance_splits(uniform global struct AABB3f *geometry_bounds,
                               uniform global __const struct GRL_RAYTRACING_INSTANCE_DESC *instance)
{
    uniform global BVHBase *bvh_base = (global BVHBase *)instance->AccelerationStructure;
    if (!SUBGROUP_can_open_root(bvh_base, instance))
        return (uint2)(0, 0);

    uniform float relative_area_scale = 1.0f / AABB3f_halfArea(geometry_bounds);
    uniform float3 root_lower = AABB3f_load_lower(&bvh_base->Meta.bounds);
    uniform float3 root_upper = AABB3f_load_upper(&bvh_base->Meta.bounds);

    uniform uint16_t quantized_area = quantize_area(transformed_aabb_halfArea(root_lower, root_upper, instance->Transform) * relative_area_scale);
    uniform uint16_t node_offs = 0;

    uniform SGHeap heap;
    uniform uint num_splits = 0;

    SGHeap_init(&heap);
    varying uint sg_split_counts_hi = 0; // cross-subgroup bin counters
    varying uint sg_split_counts_lo = 0;

    uniform global InternalNode* node_array = get_node( bvh_base, 0 );

    LOOP_TRIPWIRE_INIT;

    while (1)
    {
        uniform global InternalNode* node = node_array + node_offs;

        // count this split
        uniform uint bin = get_rebraid_bin_index(quantized_area, NUM_REBRAID_BINS);
        varying uint lane = get_sub_group_local_id();

        sg_split_counts_hi += ((lane + 16) == bin) ? 1 : 0;
        sg_split_counts_lo += (lane == bin) ? 1 : 0;

        // open this node and push all of its openable children to heap
        varying uint sg_offs = node_offs + SUBGROUP_get_child_offsets(node);
        varying bool sg_openable = 0;
        if (lane < NUM_CHILDREN & sg_offs <= MAX_NODE_OFFSET )
            if (InternalNode_IsChildValid(node, lane))
                sg_openable = is_node_openable( node_array + sg_offs);

        uniform uint openable_children = intel_sub_group_ballot(sg_openable);

        if ( openable_children )
        {
            varying uint16_t sg_area = SUBGROUP_get_child_areas( node, instance->Transform, relative_area_scale );

            if ( !SGHeap_full( &heap ) )
            {         
                openable_children = SGHeap_vectorized_push( &heap, sg_area, sg_offs, openable_children );
            }

            while ( openable_children )
            {
                // pop min element
                uniform uint16_t min_area;
                uniform uint16_t min_offs;
                SGHeap_full_pop_min( &heap, &min_area, &min_offs );

                // eliminate all children smaller than heap minimum
                openable_children &= intel_sub_group_ballot( sg_area > min_area );

                if ( openable_children )
                {
                    // if any children survived,
                    // kick out heap minimum and replace with first child.. otherwise we will re-push the minimum
                    uniform uint child_id = ctz( openable_children );
                    openable_children ^= (1 << child_id);
                    min_area = sub_group_broadcast( sg_area, child_id );
                    min_offs = sub_group_broadcast( sg_offs, child_id );
                }

                // re-insert onto heap
                SGHeap_push_and_fill( &heap, min_area, min_offs );

                // repeat until all children are accounted for.  It is possible
                //  for multiple children to fit in the heap, because heap minimum is now changed and we need to recompute it
            }
        }

        num_splits++;
        if (num_splits == MAX_SPLITS_PER_INSTANCE)
            break;

        if (SGHeap_empty(&heap))
            break;

        // get next node from heap
        SGHeap_pop_max(&heap, &quantized_area, &node_offs);

        LOOP_TRIPWIRE_INCREMENT( 500, "rebraid_count_splits" );

    }

    return (uint2)(sg_split_counts_lo, sg_split_counts_hi);
}

typedef struct RebraidBuffers
{
    global uint *bin_split_counts;    // [num_bins]
    global uint *bin_instance_counts; // [num_bins]
    global uint *instance_bin_counts; // num_intances * num_bins
} RebraidBuffers;

GRL_INLINE RebraidBuffers cast_rebraid_buffers(global uint *scratch, uint instanceID)
{
    RebraidBuffers b;
    b.bin_split_counts = scratch;
    b.bin_instance_counts = scratch + NUM_REBRAID_BINS;
    b.instance_bin_counts = scratch + (2 + instanceID) * NUM_REBRAID_BINS;
    return b;
}

///////////////////////////////////////////////////////////////////////////////////////////
//                            Compute AABB
//                  Dispatch one work item per instance
///////////////////////////////////////////////////////////////////////////////////////////

GRL_INLINE void rebraid_compute_AABB(
                          global struct BVHBase* bvh,
                          global __const struct GRL_RAYTRACING_INSTANCE_DESC *instance)
{
    // don't open null rtas
    global BVHBase *bvh_base = (global BVHBase *)instance->AccelerationStructure;

    struct AABB new_primref;
    if (bvh_base != 0)
    {
        float3 root_lower = AABB3f_load_lower(&bvh_base->Meta.bounds);
        float3 root_upper = AABB3f_load_upper(&bvh_base->Meta.bounds);
        const float *Transform = instance->Transform;

        if (is_aabb_valid(root_lower, root_upper))
        {
            new_primref = AABBfromAABB3f(transform_aabb(root_lower, root_upper, Transform));
        }
        else
        {
            // degenerate instance which might be updated to be non-degenerate
            //  use AABB position to guide BVH construction
            //
            new_primref.lower.x = Transform[3];
            new_primref.lower.y = Transform[7];
            new_primref.lower.z = Transform[11];
            new_primref.upper = new_primref.lower;
        }
    }
    else
    {
        AABB_init(&new_primref);
    }

    struct AABB subgroup_bbox = AABB_sub_group_reduce(&new_primref);

    if (get_sub_group_local_id() == 0)
    {
        AABB3f_atomic_merge_global_lu(&bvh->Meta.bounds, subgroup_bbox.lower.xyz, subgroup_bbox.upper.xyz );
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
rebraid_computeAABB_DXR_instances(
    global struct BVHBase* bvh,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC *instances)
{
    const uint instanceID = get_local_id(0) + get_group_id(0)*get_local_size(0);
    rebraid_compute_AABB(bvh, instances + instanceID);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
rebraid_computeAABB_DXR_instances_indirect(
    global struct BVHBase* bvh,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC *instances,
    global struct IndirectBuildRangeInfo const * const indirect_data)
{
    const uint instanceID = get_local_id(0) + get_group_id(0)*get_local_size(0);
    instances = (global __const struct GRL_RAYTRACING_INSTANCE_DESC*)
        (((global char*)instances) + indirect_data->primitiveOffset);
    rebraid_compute_AABB(bvh, instances + instanceID);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
rebraid_computeAABB_DXR_instances_pointers(
    global struct BVHBase* bvh,
    global void *instances_in)
{
    global const struct GRL_RAYTRACING_INSTANCE_DESC **instances =
        (global const struct GRL_RAYTRACING_INSTANCE_DESC **)instances_in;

    const uint instanceID = get_local_id(0) + get_group_id(0)*get_local_size(0);
    rebraid_compute_AABB(bvh, instances[instanceID]);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
rebraid_computeAABB_DXR_instances_pointers_indirect(
    global struct BVHBase* bvh,
    global void *instances_in,
    global struct IndirectBuildRangeInfo const * const indirect_data)
{
    instances_in = ((global char*)instances_in) + indirect_data->primitiveOffset;
    global const struct GRL_RAYTRACING_INSTANCE_DESC **instances =
        (global const struct GRL_RAYTRACING_INSTANCE_DESC **)instances_in;

    const uint instanceID = get_local_id(0) + get_group_id(0)*get_local_size(0);
    rebraid_compute_AABB(bvh, instances[instanceID]);
}

///////////////////////////////////////////////////////////////////////////////////////////
//                            Init scratch:  Dispatch one work group
///////////////////////////////////////////////////////////////////////////////////////////

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(64, 1, 1))) void kernel rebraid_init_scratch(global uint *scratch)
{
    scratch[get_local_id(0) + get_group_id(0)*get_local_size(0)] = 0;
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1))) void kernel rebraid_chase_instance_pointers(global struct GRL_RAYTRACING_INSTANCE_DESC *instances_out,
                                                                                           global void *instance_buff)
{
    global const struct GRL_RAYTRACING_INSTANCE_DESC **instances_in =
        (global const struct GRL_RAYTRACING_INSTANCE_DESC **)instance_buff;

    instances_out[get_local_id(0)] = *instances_in[get_local_id(0)];
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1)))
void kernel rebraid_chase_instance_pointers_indirect(
    global struct GRL_RAYTRACING_INSTANCE_DESC*       instances_out,
    global void*                                      instance_buff,
    global struct IndirectBuildRangeInfo const* const indirect_data)
{
    instance_buff = ((global char*)instance_buff) + indirect_data->primitiveOffset;
    global const struct GRL_RAYTRACING_INSTANCE_DESC**
        instances_in = (global const struct GRL_RAYTRACING_INSTANCE_DESC**)instance_buff;

    instances_out[get_local_id(0)] = *instances_in[get_local_id(0)];
}

///////////////////////////////////////////////////////////////////////////////////////////
//                             Count splits
///////////////////////////////////////////////////////////////////////////////////////////

GRL_INLINE void DEBUG_SUBGROUP_print_split_counts( uniform uint instanceID, varying uint split_counts_lo, varying uint split_counts_hi )
{
    uniform uint vals[32] = {
       sub_group_broadcast( split_counts_lo, 0 ),  sub_group_broadcast( split_counts_lo, 1 ),
       sub_group_broadcast( split_counts_lo, 2 ),  sub_group_broadcast( split_counts_lo, 3 ),
       sub_group_broadcast( split_counts_lo, 4 ),  sub_group_broadcast( split_counts_lo, 5 ),
       sub_group_broadcast( split_counts_lo, 6 ),  sub_group_broadcast( split_counts_lo, 7 ),
       sub_group_broadcast( split_counts_lo, 8 ),  sub_group_broadcast( split_counts_lo, 9 ),
       sub_group_broadcast( split_counts_lo, 10 ), sub_group_broadcast( split_counts_lo, 11 ),
       sub_group_broadcast( split_counts_lo, 12 ), sub_group_broadcast( split_counts_lo, 13 ),
       sub_group_broadcast( split_counts_lo, 14 ), sub_group_broadcast( split_counts_lo, 15 ),

       sub_group_broadcast( split_counts_hi, 0 ),  sub_group_broadcast( split_counts_hi, 1 ),
       sub_group_broadcast( split_counts_hi, 2 ),  sub_group_broadcast( split_counts_hi, 3 ),
       sub_group_broadcast( split_counts_hi, 4 ),  sub_group_broadcast( split_counts_hi, 5 ),
       sub_group_broadcast( split_counts_hi, 6 ),  sub_group_broadcast( split_counts_hi, 7 ),
       sub_group_broadcast( split_counts_hi, 8 ),  sub_group_broadcast( split_counts_hi, 9 ),
       sub_group_broadcast( split_counts_hi, 10 ), sub_group_broadcast( split_counts_hi, 11 ),
       sub_group_broadcast( split_counts_hi, 12 ), sub_group_broadcast( split_counts_hi, 13 ),
       sub_group_broadcast( split_counts_hi, 14 ), sub_group_broadcast( split_counts_hi, 15 )
    };

    if ( get_sub_group_local_id() == 0 )
    {
        printf(
            "Instance: %4u  "
            "%2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u "
            "%2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u %2u \n"
            ,
            instanceID,
            vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7],
            vals[8], vals[9], vals[10], vals[11], vals[12], vals[13], vals[14], vals[15],
            vals[16], vals[17], vals[18], vals[19], vals[20], vals[21], vals[22], vals[23],
            vals[24], vals[25], vals[26], vals[27], vals[28], vals[29], vals[30], vals[31]
        );
    }
}

GRL_INLINE void do_rebraid_count_splits_SG(
    uniform global struct BVHBase* bvh,
    uniform global __const struct GRL_RAYTRACING_INSTANCE_DESC *instances,
    uniform global uint *rebraid_scratch)
{
    uniform const uint instanceID = get_sub_group_global_id();
    uniform RebraidBuffers buffers = cast_rebraid_buffers(rebraid_scratch,instanceID);

    varying uint lane = get_sub_group_local_id();
    varying uint2 splits = SUBGROUP_count_instance_splits(&bvh->Meta.bounds, instances + instanceID);
    varying uint split_counts_lo = splits.x;
    varying uint split_counts_hi = splits.y;

    // write this instance's per-bin counts
    global uint* counts = buffers.instance_bin_counts;
    intel_sub_group_block_write2( counts, splits );

    // update the per-bin split and instance counters
    if (split_counts_lo > 0)
    {
        atomic_add(&buffers.bin_split_counts[lane], split_counts_lo);
        GRL_ATOMIC_INC(&buffers.bin_instance_counts[lane]);
    }
    if (split_counts_hi > 0)
    {
        atomic_add(&buffers.bin_split_counts[lane + 16], split_counts_hi);
        GRL_ATOMIC_INC(&buffers.bin_instance_counts[lane + 16]);
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
rebraid_count_splits_SG(
    uniform global struct BVHBase* bvh,
    uniform global __const struct GRL_RAYTRACING_INSTANCE_DESC *instances,
    uniform global uint *rebraid_scratch)
{
    do_rebraid_count_splits_SG(bvh, instances, rebraid_scratch);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
rebraid_count_splits_SG_indirect(
    uniform global struct BVHBase* bvh,
    uniform global __const struct GRL_RAYTRACING_INSTANCE_DESC *instances,
    uniform global uint *rebraid_scratch,
    global struct IndirectBuildRangeInfo const * const indirect_data)
{
    instances = (global __const struct GRL_RAYTRACING_INSTANCE_DESC*)
        (((global char*)instances) + indirect_data->primitiveOffset);
    do_rebraid_count_splits_SG(bvh, instances, rebraid_scratch);
}


#define HEAP_SIZE 16 
#define COUNT_SPLITS_WG_SIZE 16

struct SLMHeapNode
{
    short offs;
    ushort area;
};

struct SLMHeap
{
    struct SLMHeapNode nodes[HEAP_SIZE];
    ushort size;
    ushort min_key;
};

GRL_INLINE bool SLMHeapNode_Greater( struct SLMHeapNode a, struct SLMHeapNode b )
{
    return a.area > b.area;
}

GRL_INLINE ushort SLMHeapNode_UnpackKey( struct SLMHeapNode a )
{
    return a.area;
}

GRL_INLINE void SLMHeapNode_Unpack( struct SLMHeapNode a, ushort* area_out, short* offs_out )
{
    *area_out = a.area;
    *offs_out = a.offs;
}

GRL_INLINE struct SLMHeapNode SLMHeapNode_Pack( ushort area, short offs )
{
    struct SLMHeapNode n;
    n.offs = offs;
    n.area = area;
    return n;
}


GRL_INLINE void SLMHeap_Init( struct SLMHeap* heap )
{
    heap->size = 0;
    heap->min_key = 0xffff;
}

GRL_INLINE bool SLMHeap_empty( struct SLMHeap* heap )
{
    return heap->size == 0;
}

GRL_INLINE bool SLMHeap_full( struct SLMHeap* heap )
{
    return heap->size == HEAP_SIZE;
}


GRL_INLINE void SLMHeap_push( struct SLMHeap* heap, ushort area, short offs )
{
    ushort insert_pos;
    if ( SLMHeap_full( heap ) )
    {
        ushort current_min_key = heap->min_key;
        if ( area <= current_min_key )
            return; // don't push stuff that's smaller than the current minimum

        // search for the minimum element
        //  The heap is laid out in level order, so it is sufficient to search only the last half 
        ushort last_leaf = HEAP_SIZE - 1;
        ushort first_leaf = (last_leaf / 2) + 1;

        // as we search, keep track of what the new min-key will be so we can cull future pushes
        ushort new_min_key = area;
        ushort min_pos = 0;

        do
        {
            ushort idx = first_leaf++;

            ushort current_key = SLMHeapNode_UnpackKey( heap->nodes[idx] );
            bool found_min_pos = (min_pos == 0) && (current_key == current_min_key);
            
            if ( found_min_pos )
                min_pos = idx;
            else
                new_min_key = min( current_key, new_min_key );

        } while ( first_leaf != last_leaf );

        heap->min_key = new_min_key;
        insert_pos = min_pos;
    }
    else
    {
        insert_pos = heap->size++;
        heap->min_key = min( area, heap->min_key );
    }

    heap->nodes[insert_pos] = SLMHeapNode_Pack( area, offs );

    // heap-up
    while ( insert_pos )
    {
        ushort parent = insert_pos / 2;

        struct SLMHeapNode parent_node = heap->nodes[parent];
        struct SLMHeapNode current_node = heap->nodes[insert_pos];
        if ( SLMHeapNode_Greater( parent_node, current_node ) )
            break;
         
        heap->nodes[insert_pos]    = parent_node;
        heap->nodes[parent]        = current_node;
        insert_pos = parent;
    }

}

bool SLMHeap_pop_max( struct SLMHeap* heap, ushort* area_out, short* offs_out )
{
    if ( SLMHeap_empty( heap ) )
        return false;

    SLMHeapNode_Unpack( heap->nodes[0], area_out, offs_out );

    // heap down
    ushort size = heap->size;
    ushort idx = 0;
    do
    {
        ushort left = 2 * idx + 1;
        ushort right = 2 * idx + 2;
        if ( left >= size )
            break;

        if ( right >= size )
        {
            heap->nodes[idx] = heap->nodes[left];
            break;
        }

        struct SLMHeapNode left_node = heap->nodes[left];
        struct SLMHeapNode right_node = heap->nodes[right];
        bool go_left = SLMHeapNode_Greater( left_node, right_node );
        heap->nodes[idx] = go_left ? left_node : right_node;
        idx = go_left ? left : right;

    } while ( 1 );

    heap->size = size - 1;
    return true;
}

void SLMHeap_Print( struct SLMHeap* heap )
{
    printf( " size=%u min=%u {", heap->size, heap->min_key );
    for ( uint i = 0; i < heap->size; i++ )
        printf( "%04x:%04x", heap->nodes[i].area, heap->nodes[i].offs );
}


GRL_INLINE bool can_open_root( 
    global struct BVHBase* bvh_base, 
    const struct GRL_RAYTRACING_INSTANCE_DESC* instance 
    )
{    
    float3 root_lower = AABB3f_load_lower( &bvh_base->Meta.bounds );
    float3 root_upper = AABB3f_load_upper( &bvh_base->Meta.bounds );
    if ( !is_aabb_valid( root_lower, root_upper ) || GRL_get_InstanceMask(instance) == 0 )
        return false;

    global InternalNode* node = get_node( bvh_base, 0 );
    if ( node->nodeType != NODE_TYPE_INTERNAL )
        return false;

    return is_node_openable( node );
}


GRL_INLINE void count_instance_splits(
    global struct AABB3f* geometry_bounds,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance,
    local ushort* bin_split_counts,
    local struct SLMHeap* heap
)
{
    global BVHBase* bvh_base = (global BVHBase*)instance->AccelerationStructure;
    
    SLMHeap_Init( heap );

    float relative_area_scale = 1.0f / AABB3f_halfArea( geometry_bounds );
    float3 root_lower = AABB3f_load_lower( &bvh_base->Meta.bounds );
    float3 root_upper = AABB3f_load_upper( &bvh_base->Meta.bounds );

    ushort quantized_area = quantize_area( transformed_aabb_halfArea( root_lower, root_upper, instance->Transform ) * relative_area_scale );
    short  node_offs = 0;
    ushort num_splits = 0;
    
    global InternalNode* node_array = get_node( bvh_base, 0 );

    while ( 1 )
    {
        global InternalNode* node = node_array + node_offs;

        // count this split
        uint bin = get_rebraid_bin_index( quantized_area, NUM_REBRAID_BINS );
        bin_split_counts[bin]++;

        // open this node and push children to heap

        // TODO_OPT:  Restructure this control flow to prevent differnet lanes from skipping different loop iterations and diverging
        // TODO_OPT:  Precompute openability masks in BLAS nodes at build time... one bit for self and N bits for each child
        int offs = node->childOffset;
        for ( ushort i = 0; i < NUM_CHILDREN; i++ )
        {
            if ( InternalNode_IsChildValid( node, i ) )
            {
                if ( offs >= SHRT_MIN && offs <= SHRT_MAX )
                {
                    if ( is_node_openable( node_array + offs ) )
                    {
                        ushort area = get_child_area( node, i, instance->Transform, relative_area_scale );
                        SLMHeap_push( heap, area, (short)offs );
                    }
                }
            }
            offs += InternalNode_GetChildBlockIncr( node, i );
        }

        num_splits++;
        if ( num_splits == MAX_SPLITS_PER_INSTANCE )
            break;

        if ( !SLMHeap_pop_max( heap, &quantized_area, &node_offs ) )
            break;
    }

}

GRL_ANNOTATE_IGC_DO_NOT_SPILL      
__attribute__( (reqd_work_group_size( COUNT_SPLITS_WG_SIZE, 1, 1 )) )
void kernel
rebraid_count_splits(
    global struct BVHBase* bvh_base,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instances,
    global uint* rebraid_scratch,
    uint num_instances
    )
{
    local struct SLMHeap heap[COUNT_SPLITS_WG_SIZE];
    local ushort split_counts[COUNT_SPLITS_WG_SIZE][NUM_REBRAID_BINS];

    // initialize stuff
    // TODO_OPT:  transpose this and subgroup-vectorize it so that
    //     block-writes can be used
    for ( uint i = 0; i < NUM_REBRAID_BINS; i++ )
        split_counts[get_local_id( 0 )][i] = 0;


    // count splits for this thread's instance
    uniform uint base_instance = get_group_id( 0 ) * get_local_size( 0 );
    uint instanceID = base_instance + get_local_id( 0 );
    
    if ( instanceID < num_instances )
    {
        global BVHBase* bvh_base = (global BVHBase*)instances[instanceID].AccelerationStructure;
        if ( can_open_root( bvh_base, &instances[instanceID] ) )
        {
            count_instance_splits( &bvh_base->Meta.bounds,
                &instances[instanceID],
                &split_counts[get_local_id( 0 )][0],
                &heap[get_local_id(0)] );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );
    
    RebraidBuffers buffers = cast_rebraid_buffers( rebraid_scratch, instanceID );


    // reduce bins
    for ( uint bin = get_local_id( 0 ); bin < NUM_REBRAID_BINS; bin += get_local_size( 0 ) )
    {
        // TODO_OPT:  There's probably a better way to arrange this computation
        uint bin_split_count = 0;
        uint bin_instance_count = 0;
        for ( uint i = 0; i < COUNT_SPLITS_WG_SIZE; i++ )
        {
            uint s = split_counts[i][bin];
            bin_split_count     += s;
            bin_instance_count  += (s > 0) ? 1 : 0;
        }

        if ( bin_split_count > 0 )
        {
            atomic_add( &buffers.bin_split_counts[bin], bin_split_count );
            atomic_add( &buffers.bin_instance_counts[bin], bin_instance_count );
        }
    }

    // write out bin counts for each instance
    for ( uniform uint i = get_sub_group_id(); i < COUNT_SPLITS_WG_SIZE; i += get_num_sub_groups() )
    {
        uniform uint iid = base_instance + i;
        if ( iid > num_instances )
            break;

        global uint* instance_bin_counts = cast_rebraid_buffers( rebraid_scratch, iid ).instance_bin_counts;

        for ( uniform ushort j = 0; j < NUM_REBRAID_BINS; j += get_sub_group_size() )
        {
            uint count = split_counts[i][j + get_sub_group_local_id() ];
            intel_sub_group_block_write( instance_bin_counts + j, count );
        }
    }

}




///////////////////////////////////////////////////////////////////////////////////////////
//                             Build PrimRefs
///////////////////////////////////////////////////////////////////////////////////////////

GRL_INLINE uint get_instance_split_count(RebraidBuffers buffers, uint instanceID, uint available_splits)
{
    global uint* instance_desired_split_count = buffers.instance_bin_counts;
    global uint *bin_split_counts = buffers.bin_split_counts;
    global uint *bin_instance_counts = buffers.bin_instance_counts;

    uint total_splits = 0;
    uint remaining_available_splits = available_splits;
    uint max_bin = 0;
    uint desired_splits_this_bin = 0;
    uint instance_splits = 0;

    do
    {
        // stop when we reach a level where we can't satisfy the demand
        desired_splits_this_bin = instance_desired_split_count[max_bin];
        uint total_bin_splits = bin_split_counts[max_bin];

        if (total_bin_splits > remaining_available_splits)
            break;

        // we have enough budget to give all instances everything they want at this level, so do it
        remaining_available_splits -= total_bin_splits;
        instance_splits += desired_splits_this_bin;
        desired_splits_this_bin = 0;
        max_bin++;

    } while (max_bin < NUM_REBRAID_BINS);

    if (max_bin < NUM_REBRAID_BINS)
    {
        // we have more split demand than we have splits available.  The current bin is the last one that gets any splits
        //   distribute the leftovers as evenly as possible to instances that want them
        if (desired_splits_this_bin > 0)
        {
            // this instance wants splits.  how many does it want?
            uint desired_total = instance_splits + desired_splits_this_bin;
            
            // distribute to all instances as many as possible
            uint count = bin_instance_counts[max_bin];
            uint whole = remaining_available_splits / count;
            remaining_available_splits -= whole * count;

            // distribute remainder to lower numbered instances
            size_t partial = (instanceID < remaining_available_splits) ? 1 : 0;

            // give the instance its share.
            instance_splits += whole + partial;
            instance_splits = min(instance_splits, desired_total); // don't give it more than it needs
        }
    }

    return instance_splits;
}

GRL_INLINE void build_unopened_primref(
    struct AABB3f* centroid_bounds,
    global __const BVHBase *bvh_base,
    global volatile uint *primref_counter,
    global struct AABB *primref_buffer,
    global __const float *Transform,
    uint instanceID,
    float matOverhead,
    ushort instanceMask)
{
    float3 root_lower = AABB3f_load_lower(&bvh_base->Meta.bounds);
    float3 root_upper = AABB3f_load_upper(&bvh_base->Meta.bounds);

    struct AABB primRef;
    AABB_init( &primRef );
    
    uint bvhoffset = (uint)BVH_ROOT_NODE_OFFSET;
    if (is_aabb_valid(root_lower, root_upper) && instanceMask != 0)
    {
        primRef = AABBfromAABB3f(compute_xfm_bbox(Transform, BVHBase_GetRootNode(bvh_base), XFM_BOX_NOT_REFINED_TAKE_CLIPBOX, &bvh_base->Meta.bounds, matOverhead));
    }
    else
    {
        primRef.lower.x = Transform[3];
        primRef.lower.y = Transform[7];
        primRef.lower.z = Transform[11];
        primRef.upper.xyz = primRef.lower.xyz;

        instanceMask = 0;
        bvhoffset = NO_NODE_OFFSET;
    }

    primRef.lower.w = as_float(instanceID | (instanceMask << 24));
    primRef.upper.w = as_float(bvhoffset);

    float3 centroid = primRef.lower.xyz + primRef.upper.xyz;
    centroid_bounds->lower[0] = centroid.x;
    centroid_bounds->upper[0] = centroid.x;
    centroid_bounds->lower[1] = centroid.y;
    centroid_bounds->upper[1] = centroid.y;
    centroid_bounds->lower[2] = centroid.z;
    centroid_bounds->upper[2] = centroid.z;

    uint place = GRL_ATOMIC_INC(primref_counter);
    primref_buffer[place] = primRef;
}

GRL_INLINE void build_opened_primrefs(
    varying bool lane_mask,
    varying uint offset,
    varying InternalNode* node,
    varying struct AABB3f* centroid_bounds,
    uniform global BVHBase *bvh_base,
    uniform volatile global uint *primref_counter,
    uniform global struct AABB *primref_buffer,
    uniform uint instanceID,
    uniform const float *Transform,
    uniform float matOverhead, 
    varying ushort instanceMask)
{
    // TODO_OPT: This function is often called with <= 6 active lanes
    //  If lanes are sparse, consider jumping to a sub-group vectorized variant...

    if (lane_mask)
    {
        varying uint place = GRL_ATOMIC_INC(primref_counter);
        
        struct AABB box = AABBfromAABB3f(compute_xfm_bbox(Transform, node, XFM_BOX_NOT_REFINED_CLIPPED, &bvh_base->Meta.bounds, matOverhead));

        box.lower.w = as_float(instanceID | (instanceMask << 24));
        box.upper.w = as_float(offset * 64 + (uint)BVH_ROOT_NODE_OFFSET);
        primref_buffer[place] = box;

        AABB3f_extend_point( centroid_bounds, box.lower.xyz + box.upper.xyz );
    }
}


GRL_INLINE void SUBGROUP_open_nodes(
    uniform global struct AABB3f *geometry_bounds,
    uniform uint split_limit,
    uniform global __const struct GRL_RAYTRACING_INSTANCE_DESC *instance,
    uniform uint instanceID,
    uniform volatile global uint *primref_counter,
    uniform global struct AABB *primref_buffer,
    varying struct AABB3f* centroid_bounds, 
    float transformOverhead)
{
    uniform SGHeap heap;
    SGHeap_init(&heap);

    uniform float relative_area_scale = 1.0f / AABB3f_halfArea(geometry_bounds);
    uniform global BVHBase *bvh_base = (global BVHBase *)instance->AccelerationStructure;

    uniform uint16_t node_offs = 0;
    varying uint lane = get_sub_group_local_id();

    uniform InternalNode* node_array = get_node( bvh_base, 0 );

    LOOP_TRIPWIRE_INIT;

    while ( 1 )
    {        
        uniform InternalNode *node = node_array + node_offs;

        varying uint sg_offs   = node_offs + SUBGROUP_get_child_offsets(node);
        varying bool sg_valid = false;
        varying bool sg_openable = false;
        if (lane < NUM_CHILDREN)
        {
            sg_valid = InternalNode_IsChildValid(node, lane);
            if (sg_valid && (sg_offs <= MAX_NODE_OFFSET))
            {
                sg_openable = is_node_openable( node_array + sg_offs);
            }
        }

        uniform uint16_t valid_children = intel_sub_group_ballot(sg_valid);
        uniform uint16_t openable_children = intel_sub_group_ballot(sg_openable);
        uniform uint16_t unopenable_children = valid_children & (~openable_children);

        if ( openable_children )
        {
            varying uint16_t sg_area = SUBGROUP_get_child_areas( node, instance->Transform, relative_area_scale );

            // try to push all openable children to the heap
            if ( !SGHeap_full( &heap ) )
            {
                openable_children = SGHeap_vectorized_push( &heap, sg_area, sg_offs, openable_children );
            }

            // we have more openable children than will fit in the heap
            //  process these one by one.
            //          TODO: Try re-writing with sub_group_any() and see if compiler does a better job
            while ( openable_children )
            {
                // pop min element
                uniform uint16_t min_area;
                uniform uint16_t min_offs;
                SGHeap_full_pop_min( &heap, &min_area, &min_offs );

                // eliminate all children smaller than heap minimum.
                // mark eliminated children as unopenable
                varying uint culled_children = openable_children & intel_sub_group_ballot( sg_area <= min_area );
                unopenable_children ^= culled_children;
                openable_children &= ~culled_children;

                if ( openable_children )
                {
                    // if any children survived the purge
                    //   find the first such child and swap its offset for the one from the heap
                    //
                    uniform uint child_id = ctz( openable_children );
                    uniform uint16_t old_min_offs = min_offs;
                    min_area = sub_group_broadcast( sg_area, child_id );
                    min_offs = sub_group_broadcast( sg_offs, child_id );

                    if ( lane == child_id )
                        sg_offs = old_min_offs;

                    openable_children ^= (1 << child_id);
                    unopenable_children ^= (1 << child_id);
                }

                SGHeap_push_and_fill( &heap, min_area, min_offs );

            }
        }

        if (unopenable_children)
        {
            varying bool sg_create_primref = ((1 << lane) & unopenable_children);
            build_opened_primrefs(sg_create_primref, sg_offs, node_array + sg_offs, centroid_bounds, bvh_base, primref_counter, primref_buffer, instanceID, instance->Transform, transformOverhead, GRL_get_InstanceMask(instance));
        }

        --split_limit;
        if (split_limit == 0)
        {
            // split limit exceeded
            //  create primrefs for all remaining openable nodes in heap
            varying bool sg_mask = SGHeap_get_lane_mask(&heap);
            sg_offs = SGHeap_get_lane_values(&heap);
            build_opened_primrefs(sg_mask, sg_offs, node_array + sg_offs, centroid_bounds, bvh_base, primref_counter, primref_buffer, instanceID, instance->Transform, transformOverhead, GRL_get_InstanceMask(instance));

            break;
        }

       
        // NOTE: the heap should never be empty.  If it is, the instance was given too many splits.

        // get next node from heap
        uint16_t quantized_area;
        SGHeap_pop_max(&heap, &quantized_area, &node_offs);

        LOOP_TRIPWIRE_INCREMENT( 500, "rebraid_build_primrefs" );

    }
}


#define OPEN_QUEUE_SIZE 256
#define OPEN_QUEUE_NUM_SGS 16

typedef struct OpenQueueEntry
{
    uint instanceID;
    ushort num_splits;
} OpenQueueEntry;

typedef struct OpenQueue
{
    uint num_produced;
    uint num_consumed;
    OpenQueueEntry Q[OPEN_QUEUE_SIZE];
} OpenQueue;

uniform uint SUBGROUP_GetNextQueueEntry( local OpenQueue* queue )
{
    uint next = 0;
    if ( get_sub_group_local_id() == 0 )
        next = GRL_ATOMIC_INC( &queue->num_consumed );
    return sub_group_broadcast( next, 0 );
}


GRL_INLINE void do_rebraid_build_primrefs(
    local struct AABB3f* SLM_CentroidBounds,
    local OpenQueue* SLM_Q,
    global struct Globals* globals,
    global struct BVHBase* base,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance_buffer,
    global uint* rebraid_scratch,
    global struct AABB* primref_buffer,
    uint extra_primref_count,
    uint num_instances)
{
    varying uint instanceID = get_sub_group_size() * get_sub_group_global_id() + get_sub_group_local_id();

    uniform volatile global uint* primref_counter = &globals->numPrimitives;
    uniform RebraidBuffers buffers = cast_rebraid_buffers( rebraid_scratch, instanceID );
    uniform uint available_splits = get_num_splits( extra_primref_count, NUM_CHILDREN );



    varying struct AABB3f centroidBounds;
    AABB3f_init( &centroidBounds );

    if ( get_local_id( 0 ) == 0 )
    {
        SLM_Q->num_produced = 0;
        SLM_Q->num_consumed = 0;
        AABB3f_init( SLM_CentroidBounds );
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    // assign splits to unopened instances.  Build primrefs for unsplit instances in vectorized form
    varying uint num_splits = 0;
    if ( instanceID < num_instances )
    {
        num_splits = get_instance_split_count( buffers, instanceID, available_splits );
        if ( num_splits == 0 )
        {
            varying global const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instance_buffer + instanceID;
            varying global BVHBase* bvh_base = (global BVHBase*)instance->AccelerationStructure;
            if ( bvh_base != 0 )
            {
                build_unopened_primref( &centroidBounds, bvh_base, primref_counter, primref_buffer, instance->Transform, instanceID, 0.0f, GRL_get_InstanceMask(instance));
            }
        }
        else
        {
            // defer opened instances
            uint place = GRL_ATOMIC_INC( &SLM_Q->num_produced );
            SLM_Q->Q[place].instanceID = instanceID;
            SLM_Q->Q[place].num_splits = (ushort)num_splits;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    // if there were opened instances, process them, one per subgroup
    uniform uint num_produced = SLM_Q->num_produced;
    uniform uint next = SUBGROUP_GetNextQueueEntry( SLM_Q );

    while ( next < num_produced )
    {
        uniform uint instanceID = SLM_Q->Q[next].instanceID;
        uniform uint num_splits = SLM_Q->Q[next].num_splits;

        uniform global const struct GRL_RAYTRACING_INSTANCE_DESC* instance = instance_buffer + instanceID;

        float transformOverhead =
#if FINE_TRANSFORM_NODE_BOX
            transformation_bbox_surf_overhead(instance->Transform);
#else
            0.0f;
#endif

        SUBGROUP_open_nodes(
            &base->Meta.bounds,
            num_splits,
            instance,
            instanceID,
            primref_counter,
            primref_buffer,
            &centroidBounds,
            transformOverhead);

        next = SUBGROUP_GetNextQueueEntry( SLM_Q );
    }

    // reduce the centroid bounds AABB
    struct AABB3f reduced = AABB3f_sub_group_reduce( &centroidBounds );    
    if ( get_sub_group_local_id() == 0 )
        AABB3f_atomic_merge_localBB_nocheck( SLM_CentroidBounds, &reduced );

    barrier( CLK_LOCAL_MEM_FENCE );

    if( get_local_id(0) == 0 )
    {
        atomic_min( (global float*) (&globals->centroidBounds.lower) + 0, SLM_CentroidBounds->lower[0] );
        atomic_min( (global float*) (&globals->centroidBounds.lower) + 1, SLM_CentroidBounds->lower[1] );
        atomic_min( (global float*) (&globals->centroidBounds.lower) + 2, SLM_CentroidBounds->lower[2] );
        atomic_max( (global float*) (&globals->centroidBounds.upper) + 0, SLM_CentroidBounds->upper[0] );
        atomic_max( (global float*) (&globals->centroidBounds.upper) + 1, SLM_CentroidBounds->upper[1] );
        atomic_max( (global float*) (&globals->centroidBounds.upper) + 2, SLM_CentroidBounds->upper[2] );
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( OPEN_QUEUE_SIZE, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
void kernel rebraid_build_primrefs(
    global struct Globals* globals,
    global struct BVHBase* base,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance_buffer,
    global uint* rebraid_scratch,
    global struct AABB* primref_buffer,
    uint extra_primref_count,
    uint num_instances)
{
    local struct AABB3f SLM_CentroidBounds;
    local OpenQueue SLM_Q;
    do_rebraid_build_primrefs(
        &SLM_CentroidBounds,
        &SLM_Q,
        globals,
        base,
        instance_buffer,
        rebraid_scratch,
        primref_buffer,
        extra_primref_count,
        num_instances);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( OPEN_QUEUE_SIZE, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
void kernel rebraid_build_primrefs_indirect(
    global struct Globals* globals,
    global struct BVHBase* base,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance_buffer,
    global uint* rebraid_scratch,
    global struct AABB* primref_buffer,
    global struct IndirectBuildRangeInfo const * const indirect_data,
    uint extra_primref_count )
{
    local struct AABB3f SLM_CentroidBounds;
    local OpenQueue SLM_Q;

    instance_buffer = (global __const struct GRL_RAYTRACING_INSTANCE_DESC*)
        (((global char*)instance_buffer) + indirect_data->primitiveOffset);

    do_rebraid_build_primrefs(
        &SLM_CentroidBounds,
        &SLM_Q,
        globals,
        base,
        instance_buffer,
        rebraid_scratch,
        primref_buffer,
        extra_primref_count,
        indirect_data->primitiveCount);
}


///////////////////////////////////////////////////////////////////////////////////////////
//                             Misc
///////////////////////////////////////////////////////////////////////////////////////////



GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
ISA_TEST(global InternalNode *n, global uint *out, global float *xform, float scale)
{

    out[get_sub_group_local_id()] = InternalNode_IsChildValid(n, get_sub_group_local_id());
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( 1, 1, 1 )) ) void kernel
DEBUG_PRINT( 
    global struct Globals* globals,
    global __const struct GRL_RAYTRACING_INSTANCE_DESC* instance_buffer,
    global uint* rebraid_scratch,
    global struct AABB* primref_buffer,
    dword num_extra, 
    dword input_instances )
{
#if 0
    // validate primrefs
    if ( (get_local_id(0) + get_group_id(0)*get_local_size(0)) == 0 )
    {
        uint refs = globals->numPrimitives;
        for ( uint i = 0; i < refs; i++ )
        {
            if ( any( primref_buffer[i].lower.xyz < globals->geometryBounds.lower.xyz ) ||
                 any( primref_buffer[i].upper.xyz > globals->geometryBounds.upper.xyz ) ||
                 any( isnan(primref_buffer[i].lower.xyz) ) ||
                any( isnan(primref_buffer[i].upper.xyz) ) )
            {
                struct AABB box = primref_buffer[i];
                printf( "BAD BOX:      %u  {%f,%f,%f} {%f,%f,%f} %u\n", as_uint( box.lower.w ),
                    box.lower.x, box.lower.y, box.lower.z,
                    box.upper.x, box.upper.y, box.upper.z,
                    as_uint( box.lower.w ) );
            }

            const uint instIndex = PRIMREF_instanceID(&primref_buffer[i]);    // TODO: Refactor me.  We should not be using struct AABB for primRefs
            const uint rootByteOffset = as_uint( primref_buffer[i].upper.w ); // It should be struct PrimRef
            if ( instIndex >= input_instances )
                printf( "BAD INSTANCE INDEX: %u", i );
            else
            {
                global struct BVHBase* blas = (global struct BVHBase*)instance_buffer[instIndex].AccelerationStructure;
                if ( blas )
                {
                    struct InternalNode* start = BVHBase_GetInternalNodes( blas );
                    struct InternalNode* end = BVHBase_GetInternalNodesEnd( blas );

                    InternalNode* entryPoint = (struct InternalNode*)((char*)instance_buffer[instIndex].AccelerationStructure + rootByteOffset);
                    if ( entryPoint < start || entryPoint >= end )
                        printf( "BAD ENTRYPOINT: %u\n", i );
                    if ( (rootByteOffset & 63) != 0 )
                        printf( "MISALIGNED ENTRYPOInt: %u\n", i );
                    
                }
            }
        }
    }
#endif
#if 0
    if ( (get_local_id(0) + get_group_id(0)*get_local_size(0)) == 0 )
        printf( "REBRAIDED: %u\n", globals->numPrimitives );

    // print instance bin information
    if ( (get_local_id(0) + get_group_id(0)*get_local_size(0)) == 0 )
    {
        printf( "REBRAIDED: %u\n", globals->numPrimitives );
        for( uint i=0; i<231; i++  )
        {
            RebraidBuffers buffers = cast_rebraid_buffers( rebraid_scratch,i );
            printf( " ID:%4u ", i );
            for ( uint j = 0; j < NUM_REBRAID_BINS; j++ )
            {
                global uint* count = buffers.instance_bin_counts;
                printf( " %2u ", count[j] );
            }
            printf( "\n" );
        }
    }
#endif
#if 0
    if ( (get_local_id(0) + get_group_id(0)*get_local_size(0)) == 0 )
    {
        printf( "Instances: %u\n", globals->numPrimitives );

        for ( uint i = 0; i < globals->numPrimitives; i++ )
        {
            if ( any( primref_buffer[i].lower.xyz < globals->geometryBounds.lower.xyz ) ||
                 any( primref_buffer[i].upper.xyz > globals->geometryBounds.upper.xyz ) )
            {
                struct AABB box = primref_buffer[i];
                printf( "      %u  {%f,%f,%f} {%f,%f,%f} %u\n", as_uint( box.lower.w ),
                    box.lower.x, box.lower.y, box.lower.z,
                    box.upper.x, box.upper.y, box.upper.z,
                    as_uint( box.lower.w ) );
            }

        }
    }
#endif
}

