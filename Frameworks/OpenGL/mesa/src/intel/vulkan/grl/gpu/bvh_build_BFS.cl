//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "binned_sah_shared.h"

#include "libs/lsc_intrinsics.h"
#include "intrinsics.h"
#include "AABB.h"
#include "AABB3f.h"

#include "qbvh6.h"
#include "common.h"

#include "libs/lsc_intrinsics.h"

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

#define BFS_WG_SIZE  512

#define BFS_NUM_VCONTEXTS 256 // must be multiple of 64

#define TREE_ARITY 6

#define DFS_WG_SIZE  256
#define DFS_THRESHOLD 256


void BFSDispatchQueue_print(struct BFSDispatchQueue* q, uint n)
{
    for (uint i = 0; i < q->num_dispatches; i++)
        printf("   %u,ctx=%u,batch=%u\n", q->wg_count[i], q->records[i].context_id, q->records[i].batch_index);
}

void VContextScheduler_print(struct VContextScheduler* scheduler)
{
    if (get_local_id(0) == 0)
    {
        printf("SCHEDULER:\n");
        printf("    bfs=%u dfs=%u\n", scheduler->num_bfs_wgs, scheduler->num_dfs_wgs);

        printf("BFS QUEUE:\n");
        BFSDispatchQueue_print(&scheduler->bfs_queue, scheduler->num_bfs_wgs);


        printf("DFS QUEUE\n");
        for (uint i = 0; i < scheduler->num_dfs_wgs; i++)
        {
            struct DFSDispatchRecord* r = &scheduler->dfs_queue.records[i];
            printf("    (%u-%u) root=%u  depth=%u  batch_index=%u\n",
                r->primref_base, r->primref_base + r->num_primrefs,
                r->bvh2_base, r->tree_depth, r->batch_index);
        }

        printf("CONTEXTS:\n");
        for (uint i = 0; i < BFS_NUM_VCONTEXTS; i++)
        {
            if (scheduler->vcontext_state[i] != VCONTEXT_STATE_UNALLOCATED)
            {
                printf(" context: %u  state=%u\n", i, scheduler->vcontext_state[i]);
                printf("     prims: %u-%u\n", scheduler->contexts[i].dispatch_primref_begin, scheduler->contexts[i].dispatch_primref_end);
                printf("     depth: %u\n", scheduler->contexts[i].tree_depth);
                printf("     root: %u\n", scheduler->contexts[i].bvh2_root);
                printf("     batch: %u\n", scheduler->contexts[i].batch_index);
            }
        }



    }

}


inline float3 select_min(float3 v, bool mask)
{
    return (float3)(mask ? v.x : (float)(INFINITY),
        mask ? v.y : (float)(INFINITY),
        mask ? v.z : (float)(INFINITY));
}
inline float3 select_max(float3 v, bool mask)
{
    return (float3)(mask ? v.x : -(float)(INFINITY),
        mask ? v.y : -(float)(INFINITY),
        mask ? v.z : -(float)(INFINITY));
}

///////////////////////////////////////////////////////////////////////////

//  The 'LRBounds' structure uses negated-max to allow
//  both atomic_min and atomic_max to be issued fused into one message

struct AABB3f LRBounds_get_left_centroid( LRBounds* b )
{
    struct AABB3f* pbox = &b->boxes.left_centroid_bounds;
    return AABB3f_construct( AABB3f_load_lower(pbox), -AABB3f_load_upper(pbox) );
}
struct AABB3f LRBounds_get_right_centroid( LRBounds* b )
{
    struct AABB3f* pbox = &b->boxes.right_centroid_bounds;
    return AABB3f_construct( AABB3f_load_lower(pbox), -AABB3f_load_upper(pbox) );
}
struct AABB3f LRBounds_get_left_geom( LRBounds* b )
{
    struct AABB3f* pbox = &b->boxes.left_geom_bounds;
    return AABB3f_construct( AABB3f_load_lower(pbox), -AABB3f_load_upper(pbox) );
}
struct AABB3f LRBounds_get_right_geom( LRBounds* b )
{
    struct AABB3f* pbox = &b->boxes.right_geom_bounds;
    return AABB3f_construct( AABB3f_load_lower(pbox), -AABB3f_load_upper(pbox) );
}


void LRBounds_merge_left( local LRBounds* b, float3 CMin, float3 CMax, float3 GMin, float3 GMax )
{
    // All of the input vectors have come from sub-group reductions and are thus uniform
    //   Using atomic_min calls as below results in IGC generating 12 atomic_min messages and a large stack of movs
    //  The code below should result in 1 atomic_min message and a simularly large stack of movs

    float mergeVal0 = INFINITY;
    float mergeVal1 = INFINITY;
    uint i = get_sub_group_local_id();

    // insert the various merge values into one register
    //  We use two parallel variables here to enable some ILP

    uint imod = (i>=6) ? (i-6) : i;
    mergeVal0 = (imod==0)  ?  CMin.x : mergeVal0;
    mergeVal1 = (imod==0)  ?  GMin.x : mergeVal1;

    mergeVal0 = (imod==1)  ?  CMin.y : mergeVal0;
    mergeVal1 = (imod==1)  ?  GMin.y : mergeVal1;

    mergeVal0 = (imod==2)  ?  CMin.z : mergeVal0;
    mergeVal1 = (imod==2)  ?  GMin.z : mergeVal1;

    mergeVal0 = (imod==3)  ? -CMax.x : mergeVal0;
    mergeVal1 = (imod==3)  ? -GMax.x : mergeVal1;

    mergeVal0 = (imod==4)  ? -CMax.y : mergeVal0;
    mergeVal1 = (imod==4)  ? -GMax.y : mergeVal1;

    mergeVal0 = (imod==5)  ? -CMax.z : mergeVal0;
    mergeVal1 = (imod==5)  ? -GMax.z : mergeVal1;

    float merge = (i<6) ? mergeVal0 : mergeVal1;
    if( i < 12 )
        atomic_min( &b->scalars.Array[i], merge );

    //atomic_min( &b->boxes.left_centroid_bounds.lower[0], CMin.x );
    //atomic_min( &b->boxes.left_centroid_bounds.lower[1], CMin.y );
    //atomic_min( &b->boxes.left_centroid_bounds.lower[2], CMin.z );
    //atomic_min( &b->boxes.left_centroid_bounds.upper[0], -CMax.x );
    //atomic_min( &b->boxes.left_centroid_bounds.upper[1], -CMax.y );
    //atomic_min( &b->boxes.left_centroid_bounds.upper[2], -CMax.z );
    //atomic_min( &b->boxes.left_geom_bounds.lower[0],      GMin.x );
    //atomic_min( &b->boxes.left_geom_bounds.lower[1],      GMin.y );
    //atomic_min( &b->boxes.left_geom_bounds.lower[2],      GMin.z );
    //atomic_min( &b->boxes.left_geom_bounds.upper[0], -GMax.x );
    //atomic_min( &b->boxes.left_geom_bounds.upper[1], -GMax.y );
    //atomic_min( &b->boxes.left_geom_bounds.upper[2], -GMax.z );
}

void LRBounds_merge_right( local LRBounds* b, float3 CMin, float3 CMax, float3 GMin, float3 GMax )
{
    // All of the input vectors have come from sub-group reductions and are thus uniform
    //   Using atomic_min calls as below results in IGC generating 12 atomic_min messages and a large stack of movs
    //  The code below should result in 1 atomic_min message and a simularly large stack of movs

    float mergeVal0 = INFINITY;
    float mergeVal1 = INFINITY;
    uint i = get_sub_group_local_id();

    // insert the various merge values into one register
    //  We use two parallel variables here to enable some ILP

    uint imod = (i>=6) ? (i-6) : i;
    mergeVal0 = (imod==0)  ?  CMin.x : mergeVal0;
    mergeVal1 = (imod==0)  ?  GMin.x : mergeVal1;

    mergeVal0 = (imod==1)  ?  CMin.y : mergeVal0;
    mergeVal1 = (imod==1)  ?  GMin.y : mergeVal1;

    mergeVal0 = (imod==2)  ?  CMin.z : mergeVal0;
    mergeVal1 = (imod==2)  ?  GMin.z : mergeVal1;

    mergeVal0 = (imod==3)  ? -CMax.x : mergeVal0;
    mergeVal1 = (imod==3)  ? -GMax.x : mergeVal1;

    mergeVal0 = (imod==4)  ? -CMax.y : mergeVal0;
    mergeVal1 = (imod==4)  ? -GMax.y : mergeVal1;

    mergeVal0 = (imod==5)  ? -CMax.z : mergeVal0;
    mergeVal1 = (imod==5)  ? -GMax.z : mergeVal1;

    float merge = (i<6) ? mergeVal0 : mergeVal1;
    if( i < 12 )
        atomic_min( &b->scalars.Array[i+12], merge );

    //atomic_min( &b->boxes.right_centroid_bounds.lower[0],  CMin.x );
    //atomic_min( &b->boxes.right_centroid_bounds.lower[1],  CMin.y );
    //atomic_min( &b->boxes.right_centroid_bounds.lower[2],  CMin.z );
    //atomic_min( &b->boxes.right_centroid_bounds.upper[0], -CMax.x );
    //atomic_min( &b->boxes.right_centroid_bounds.upper[1], -CMax.y );
    //atomic_min( &b->boxes.right_centroid_bounds.upper[2], -CMax.z );
    //atomic_min( &b->boxes.right_geom_bounds.lower[0],      GMin.x );
    //atomic_min( &b->boxes.right_geom_bounds.lower[1],      GMin.y );
    //atomic_min( &b->boxes.right_geom_bounds.lower[2],      GMin.z );
    //atomic_min( &b->boxes.right_geom_bounds.upper[0],     -GMax.x );
    //atomic_min( &b->boxes.right_geom_bounds.upper[1],     -GMax.y );
    //atomic_min( &b->boxes.right_geom_bounds.upper[2],     -GMax.z );
}

void LRBounds_merge( global LRBounds* globalBounds, local LRBounds* localBounds )
{
    uint i = get_local_id(0);
    if( i < 24 )
        atomic_min(&globalBounds->scalars.Array[i], localBounds->scalars.Array[i] );
}


void LRBounds_init( LRBounds* bounds )
{
    uint i = get_local_id(0) * 4;
    if( i < 24 )
    {
        // compiler should merge it into a 4xdword send
        bounds->scalars.Array[i+0] = INFINITY;
        bounds->scalars.Array[i+1] = INFINITY;
        bounds->scalars.Array[i+2] = INFINITY;
        bounds->scalars.Array[i+3] = INFINITY;
    }

}


inline void LRBounds_init_subgroup( LRBounds* bounds)
{
    uint sg_size = get_sub_group_size();
    uint lane = get_sub_group_local_id();

    for (uint i = lane * 4; i < 24; i += sg_size * 4)
    {
        // compiler should merge it into a 4xdword send
        bounds->scalars.Array[i+0] = INFINITY;
        bounds->scalars.Array[i+1] = INFINITY;
        bounds->scalars.Array[i+2] = INFINITY;
        bounds->scalars.Array[i+3] = INFINITY;
    }

}

///////////////////////////////////////////////////////////////////////////

inline void BinInfo_init(struct BFS_BinInfo* bin_info)
{
    for (uint id = get_local_id(0) * 4; id < 18 * BFS_NUM_BINS; id += get_local_size(0) * 4)
    {
        float inf = INFINITY;
        // compiler should merge it into a 4xdword send
        bin_info->min_max[id+0] = inf;
        bin_info->min_max[id+1] = inf;
        bin_info->min_max[id+2] = inf;
        bin_info->min_max[id+3] = inf;
    }
    for (uint id = get_local_id(0) * 4; id < 3 * BFS_NUM_BINS; id += get_local_size(0) * 4)
    {
        // compiler should merge it into a 4xdword send
        bin_info->counts[id+0] = 0;
        bin_info->counts[id+1] = 0;
        bin_info->counts[id+2] = 0;
        bin_info->counts[id+3] = 0;
    }
}


// copy global to local
inline void BinInfo_copy( local struct BFS_BinInfo* local_bin_info, global struct BFS_BinInfo* global_bin_info )
{
    for (uint id = get_local_id(0); id < 18 * BFS_NUM_BINS; id += get_local_size(0))
    {
        float inf = INFINITY ;
        float f = global_bin_info->min_max[id];
        local_bin_info->min_max[id] = f;
    }
    for (uint id = get_local_id(0); id < 3 * BFS_NUM_BINS; id += get_local_size(0))
    {
        local_bin_info->counts[id] = global_bin_info->counts[id];
    }
}

inline void BinInfo_init_subgroup(struct BFS_BinInfo* bin_info)
{
    uint sg_size = get_sub_group_size();
    uint lane = get_sub_group_local_id();

    for (uint i = lane * 4; i < 3 * BFS_NUM_BINS; i += sg_size * 4)
    {
        // compiler should merge it into a 4xdword send
        bin_info->counts[i+0] = 0;
        bin_info->counts[i+1] = 0;
        bin_info->counts[i+2] = 0;
        bin_info->counts[i+3] = 0;
    }


    for (uint i = lane * 4; i < 18 * BFS_NUM_BINS; i += sg_size * 4)
    {
        // compiler should merge it into a 4xdword send
        bin_info->min_max[i+0] = INFINITY;
        bin_info->min_max[i+1] = INFINITY;
        bin_info->min_max[i+2] = INFINITY;
        bin_info->min_max[i+3] = INFINITY;
    }

}

float3 shuffle_down_float3( float3 a, float3 b, uint delta )
{
    return (float3)(
        intel_sub_group_shuffle_down( a.x, b.x, delta ),
        intel_sub_group_shuffle_down( a.y, b.y, delta ),
        intel_sub_group_shuffle_down( a.z, b.z, delta )
        );
}




void BinInfo_primref_ballot_loop( local struct BFS_BinInfo* bin_info, uint axis, uint bin, float3 lower, float3 upper, bool active_lane )
{
    local float* bins_min = &bin_info->min_max[0];
    local float* bins_max = &bin_info->min_max[3];

    varying uint place = (bin + axis*BFS_NUM_BINS);
    varying uint lane = get_sub_group_local_id();

    uniform uint active_mask = intel_sub_group_ballot(active_lane);

    while( active_mask )
    {
        uniform uint leader     = ctz( active_mask );
        uniform uint lead_place = intel_sub_group_shuffle( place, leader );
        varying bool matching_bin = lead_place == place && active_lane;

        varying float3 lo = (float3)(INFINITY,INFINITY,INFINITY);
        varying float3 hi = (float3)(-INFINITY,-INFINITY,-INFINITY);
        if (matching_bin)
        {
            lo = lower.xyz;
            hi = upper.xyz;
        }

        lo = sub_group_reduce_min_float3( lo );
        hi = sub_group_reduce_max_float3( hi );

        {
            // atomic min operation vectorized across 6 lanes
            //    [ lower.xyz ][-][upper.xyz][-]
            //
            // Lanes 3 and 7 are inactive

            uint lmod = lane % 4;
            uint ldiv = lane / 4;
            float vlo = lo.x;
            float vhi = hi.x;
            vlo = (lmod == 1) ? lo.y : vlo;
            vhi = (lmod == 1) ? hi.y : vhi;
            vlo = (lmod == 2) ? lo.z : vlo;
            vhi = (lmod == 2) ? hi.z : vhi;

            float v = (ldiv == 0) ? vlo : -vhi;

            if( (1<<lane) & 0x77 )
                atomic_min( &bin_info->min_max[ 6*lead_place + lmod + 3*ldiv ], v );
        }

      //if( lane == 0 )
      //    atomic_add_local(&bin_info->counts[lead_place], popcount(active_mask & intel_sub_group_ballot(matching_bin)) );

        active_mask = active_mask & intel_sub_group_ballot(!matching_bin);
    }
}

inline void BinInfo_add_primref(struct BinMapping* binMapping, local struct BFS_BinInfo* bin_info, PrimRef* primref, bool active_lane )
{

    const float4 lower = primref->lower;
    const float4 upper = primref->upper;
    const float4 p = lower + upper;
    const uint4 i = convert_uint4( (p - binMapping->ofs) * binMapping->scale );

    BinInfo_primref_ballot_loop( bin_info, 0, i.x, lower.xyz, upper.xyz, active_lane );
    BinInfo_primref_ballot_loop( bin_info, 1, i.y, lower.xyz, upper.xyz, active_lane );
    BinInfo_primref_ballot_loop( bin_info, 2, i.z, lower.xyz, upper.xyz, active_lane );

    if (active_lane)
    {
        atomic_inc_local( &bin_info->counts[i.x + 0 * BFS_NUM_BINS] );
        atomic_inc_local( &bin_info->counts[i.y + 1 * BFS_NUM_BINS] );
        atomic_inc_local( &bin_info->counts[i.z + 2 * BFS_NUM_BINS] );
    }
}

inline void BinInfo_merge(global struct BFS_BinInfo* global_info, local struct BFS_BinInfo* local_info)
{
    uint id = get_local_id(0);
    for (uint id = get_local_id(0); id < 18 * BFS_NUM_BINS; id += get_local_size(0))
    {
            float v = local_info->min_max[id];
            if( v != INFINITY )
                atomic_min(&global_info->min_max[id], v);
    }
    for (uint id = get_local_id(0); id < 3 * BFS_NUM_BINS; id += get_local_size(0))
    {
            uint c = local_info->counts[id];
            if( c )
                atomic_add_global(&global_info->counts[id], c);
    }
}

inline struct AABB3f BinInfo_get_AABB(struct BFS_BinInfo* bin_info, ushort bin, ushort axis)
{
    float* min = &bin_info->min_max[6*(bin + axis*BFS_NUM_BINS)];
    float* max = min + 3;
    struct AABB3f box;
    for (uint i = 0; i < 3; i++)
    {
        box.lower[i] = min[i];
        box.upper[i] = -max[i];
    }

    return box;
}

inline uint3 BinInfo_get_counts(struct BFS_BinInfo* bin_info, ushort bin)
{
    uint3 counts;
    counts.x = bin_info->counts[bin + 0 * BFS_NUM_BINS]; // TODO: block load these
    counts.y = bin_info->counts[bin + 1 * BFS_NUM_BINS];
    counts.z = bin_info->counts[bin + 2 * BFS_NUM_BINS];
    return counts;
}
inline uint BinInfo_get_count(struct BFS_BinInfo* bin_info, ushort bin, ushort axis)
{
    return bin_info->counts[bin + axis * BFS_NUM_BINS];
}


void BVH2_Initialize( struct BVH2* bvh )
{
    bvh->num_nodes = 1;
}

inline bool BVH2_IsInnerNode( global struct BVH2* bvh, uint node_index )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    return (n->meta_ss & 0x10000) != 0;
}
inline uint BVH2_GetRoot( struct BVH2* bvh )
{
    return 0;
}

//////////////////////////////////////////////
// BVH2NodeMetaData funcs
//////////////////////////////////////////////
struct BVH2NodeMetaData
{
    uint  meta_u;   // leaf:  primref start.  inner: offset from node to its first child
    uint  meta_ss;
};

inline struct BVH2NodeMetaData BVH2_GetNodeMetaData( global struct BVH2* bvh, uint node_index )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    struct BVH2NodeMetaData meta;
    meta.meta_u = n->meta_u;
    meta.meta_ss = n->meta_ss;
    return meta;
}

inline bool BVH2NodeMetaData_IsInnerNode( struct BVH2NodeMetaData* meta )
{
    return (meta->meta_ss & 0x10000) != 0;
}

inline ushort BVH2NodeMetaData_GetLeafPrimCount( struct BVH2NodeMetaData* meta )
{
    return meta->meta_ss & 0xffff;
}

inline uint BVH2NodeMetaData_GetLeafPrimStart( struct BVH2NodeMetaData* meta )
{
    return meta->meta_u;
}

inline uint BVH2NodeMetaData_GetMask( struct BVH2NodeMetaData* meta )
{
    return (meta->meta_ss>>24);
}

//////////////////////////////////////////////

inline ushort BVH2_GetLeafPrimCount( struct BVH2* bvh, uint node_index )
{
    struct BVH2Node* n = ((struct BVH2Node*)(bvh + 1)) + node_index;
    return n->meta_ss & 0xffff;
}
inline uint BVH2_GetLeafPrimStart( struct BVH2* bvh, uint node_index )
{
    struct BVH2Node* n = ((struct BVH2Node*)(bvh + 1)) + node_index;
    return n->meta_u;
}
inline uint2 BVH2_GetChildIndices( struct BVH2* bvh, uint node_index )
{
    struct BVH2Node* n = ((struct BVH2Node*)(bvh + 1)) + node_index;
    uint2 idx;
    idx.x = n->meta_u;
    idx.y = idx.x + (n->meta_ss & 0xffff);
    return idx;
}

inline float BVH2_GetNodeArea( global struct BVH2* bvh, uint node_index )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    return AABB3f_halfArea( &n->box );
}


inline struct AABB3f BVH2_GetNodeBox( global struct BVH2* bvh, uint node_index )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    return n->box;
}
inline void BVH2_SetNodeBox( global struct BVH2* bvh, uint node_index, struct AABB3f* box )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    n->box = *box;
}

inline void BVH2_SetNodeBox_lu( global struct BVH2* bvh, uint node_index, float3 lower, float3 upper )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    AABB3f_set( &n->box, lower, upper );
}

inline void BVH2_InitNodeBox( struct BVH2* bvh, uint node_index )
{
    struct BVH2Node* n = ((struct BVH2Node*)(bvh + 1)) + node_index;
    AABB3f_init( &n->box );
}

inline struct AABB BVH2_GetAABB( global struct BVH2* bvh, uint node_index )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    struct AABB r;
    r.lower.xyz = AABB3f_load_lower( &n->box );
    r.upper.xyz = AABB3f_load_upper( &n->box );
    return r;
}

inline void BVH2_WriteInnerNode( global struct BVH2* bvh, uint node_index, struct AABB3f* box, uint2 child_offsets, uint mask )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    n->box = *box;
    n->meta_u   = child_offsets.x;
    n->meta_ss  = 0x10000 + (child_offsets.y - child_offsets.x) + (mask<<24);
  //  n->is_inner  = true;
}

inline void BVH2_WriteLeafNode( global struct BVH2* bvh, uint node_index, struct AABB3f* box, uint prim_start, uint prim_count, uint mask  )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    n->box = *box;
    n->meta_u   = prim_start;
    n->meta_ss  = prim_count + (mask<<24);
    //  n->is_inner  = true;
}

inline uint BVH2_GetMask( global struct BVH2* bvh, uint node_index )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    return (n->meta_ss>>24);
}


uint BVH2_AllocateNodes( global struct BVH2* bvh, uint num_nodes )
{
    return atomic_add_global( &bvh->num_nodes, num_nodes );
}

inline void BVH2_AtomicMergeNodeBox( global struct BVH2* bvh, uint node_index, float3 lower, float3 upper )
{
    global struct BVH2Node* n = ((global struct BVH2Node*)(bvh + 1)) + node_index;
    AABB3f_atomic_merge_global_lu( &n->box, lower, upper );
}


void BVH2_print( global struct BVH2* bvh, uint start_node )
{
    if ( get_local_id( 0 ) == 0 && get_sub_group_id() == 0 )
    {
        uint num_nodes = bvh->num_nodes;

        uint2 stack[BFS_MAX_DEPTH * 2];
        uint sp = 0;

        printf( "allocated_nodes=%u\n", num_nodes );

        stack[sp++] = (uint2)(start_node, 0);
        while ( sp > 0 )
        {
            uint2 data = stack[--sp];
            uint node = data.x;
            uint depth = data.y;

            for ( uint i = 0; i < depth; i++ )
                printf( "    " );

            if ( BVH2_IsInnerNode( bvh, node ) )
            {
                uint2 kids = BVH2_GetChildIndices( bvh, node );
                printf( " %5u: inner: %u %u \n", node, kids.x, kids.y );
                stack[sp++] = (uint2)(kids.y, depth + 1);
                stack[sp++] = (uint2)(kids.x, depth + 1);

                struct AABB3f l = BVH2_GetNodeBox( bvh, kids.x );
                struct AABB3f r = BVH2_GetNodeBox( bvh, kids.y );
                struct AABB3f p = BVH2_GetNodeBox( bvh, node );

                float3 pl = AABB3f_load_lower( &p );
                float3 pu = AABB3f_load_upper( &p );
                float3 ll = AABB3f_load_lower( &l );
                float3 lu = AABB3f_load_upper( &l );
                float3 rl = AABB3f_load_lower( &r );
                float3 ru = AABB3f_load_upper( &r );
                if ( any( ll < pl ) || any( rl < pl ) ||
                     any( lu > pu ) || any( ru > pu ) )
                {
                    for ( uint i = 0; i < depth; i++ )
                        printf( "    " );

                    printf( "BAD_BOUNDS!!!!!!!! %u\n", node );
                }


            }
            else
            {

                uint start = BVH2_GetLeafPrimStart( bvh, node );
                uint count = BVH2_GetLeafPrimCount( bvh, node );
                printf( " %5u: leaf: start=%u count=%u\n  ",node,start,count );

            }
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );
}


global uint* SAHBuildGlobals_GetPrimrefIndices_In( struct SAHBuildGlobals* globals, bool odd_pass )
{
    uint num_refs = globals->num_primrefs;
    global uint* ib = (global uint*) globals->p_primref_index_buffers;
    return ib + (odd_pass ? num_refs : 0);
}

global uint* SAHBuildGlobals_GetPrimrefIndices_Out( struct SAHBuildGlobals* globals, bool odd_pass )
{
    uint num_refs = globals->num_primrefs;
    global uint* ib = (global uint*) globals->p_primref_index_buffers;
    return ib + (odd_pass ? 0 : num_refs);
}

global PrimRef* SAHBuildGlobals_GetPrimrefs( struct SAHBuildGlobals* globals )
{
    return (global PrimRef*) globals->p_primrefs_buffer;
}

global struct BVH2* SAHBuildGlobals_GetBVH2( struct SAHBuildGlobals* globals )
{
    return (global struct BVH2*)globals->p_bvh2;
}

uint SAHBuildGlobals_GetLeafSizeInBytes( struct SAHBuildGlobals* globals )
{
    return globals->leaf_size;
}

uint SAHBuildGlobals_GetLeafType( struct SAHBuildGlobals* globals )
{
    return globals->leaf_type;
}

uint SAHBuildGlobals_GetInternalNodeType( struct SAHBuildGlobals* globals )
{
    return NODE_TYPE_INTERNAL;
}

global struct BVHBase* SAHBuildGlobals_GetBVHBase( struct SAHBuildGlobals* globals )
{
    return (global struct BVHBase*) globals->p_bvh_base;
}

uint SAHBuildGlobals_GetTotalPrimRefs( struct SAHBuildGlobals* globals )
{
    return globals->num_primrefs;
}

inline bool SAHBuildGlobals_NeedBackPointers( struct SAHBuildGlobals* globals )
{
    return globals->flags & SAH_FLAG_NEED_BACKPOINTERS;
}
inline bool SAHBuildGlobals_NeedMasks( struct SAHBuildGlobals* globals )
{
    return globals->flags & SAH_FLAG_NEED_MASKS;
}


void SAHBuildGlobals_print( struct SAHBuildGlobals* globals )
{
    if ( get_local_id( 0 ) == 0 )
    {
        printf( "SAHBuildGlobals: %p\n", globals );
        printf( "  p_primref_index_buffers =%p\n", globals->p_primref_index_buffers );
        printf( "  p_primrefs_buffer =%p\n",       globals->p_primrefs_buffer );
        printf( "  p_bvh2 =%p\n",                  globals->p_bvh2 );
        printf( "  p_globals =%p\n",               globals->p_globals );
        printf( "  p_bvh_base =%p\n",              globals->p_bvh_base );
        printf( "  num_primrefs = %u\n",           globals->num_primrefs );
        printf( "  leaf_size = %u\n",              globals->leaf_size );
        printf( "  leaf_type = %u\n",              globals->leaf_type );
        printf( "  p_qnode_buffer = %p\n",         globals->p_qnode_root_buffer);
    }

    barrier( CLK_LOCAL_MEM_FENCE );
}


uint get_num_wgs(uint thread_count, uint WG_SIZE)
{
    return (thread_count + WG_SIZE - 1) / WG_SIZE;
}





struct BFSDispatchArgs
{
    global struct VContextScheduler* scheduler;
    global struct VContext* context;
    global struct BVH2* bvh2;
    global uint* primref_index_in;
    global uint* primref_index_out;
    global PrimRef* primref_buffer;

    uint   wg_primref_begin;
    uint   wg_primref_end;
    uint   dispatch_primref_begin;
    uint   dispatch_primref_end;
    uint   context_id;
    uint   num_wgs;
    uint   bvh2_root;
    uint   global_num_primrefs;
    bool   do_mask_processing;
};




// TODO_OPT:  Enable larger WGs
//    We need a way to do this in a portable fashion.
//     Gen12 can support larger WGs than Gen9 can
//
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( 512, 1, 1 )) )
kernel void
begin( global struct VContextScheduler* scheduler,
       dword leaf_size,
       dword leaf_type,
       global uint* primref_index_buffers,
       global PrimRef* primref_buffer,
       global struct BVH2* bvh2,
       global struct BVHBase* bvh_base,
       global struct Globals* globals,
       global struct SAHBuildGlobals* sah_globals,
       global uint2* qnode_root_buffer,
       dword sah_globals_flags
     )
{
    dword num_primrefs = globals->numPrimitives;
    if ( get_local_id( 0 ) == 0 )
    {
        sah_globals->p_primrefs_buffer       = (qword) primref_buffer;
        sah_globals->p_primref_index_buffers = (qword)primref_index_buffers;
        sah_globals->p_bvh2                  = (qword) bvh2;
        sah_globals->p_bvh_base              = (qword) bvh_base;
        sah_globals->leaf_size               = leaf_size;
        sah_globals->leaf_type               = leaf_type;
        sah_globals->num_primrefs            = num_primrefs;
        sah_globals->p_globals               = (qword) globals;
        sah_globals->p_qnode_root_buffer     = (gpuva_t) qnode_root_buffer;
        sah_globals->flags                   = sah_globals_flags;

        // initialize the spill stack
        scheduler->bfs2_spill_stack.size = 0;

        // initialize BVH2 node counter
        BVH2_Initialize( bvh2 );

        // configure first vcontext for first build
        scheduler->contexts[0].dispatch_primref_begin = 0;
        scheduler->contexts[0].dispatch_primref_end   = num_primrefs;
        scheduler->contexts[0].bvh2_root              = BVH2_GetRoot( bvh2 );
        scheduler->contexts[0].tree_depth             = 0;
        scheduler->contexts[0].batch_index            = 0;

        scheduler->bfs_queue.records[0].context_id = 0;

        scheduler->contexts[0].num_left = 0;
        scheduler->contexts[0].num_right = 0;
        scheduler->contexts[0].lr_mask = 0;

        // copy centroid bounds into the BVH2 root node'
        BVH2_SetNodeBox_lu( bvh2, BVH2_GetRoot( bvh2 ), globals->centroidBounds.lower.xyz, globals->centroidBounds.upper.xyz );

        // zero the trivial build counters.. these are only used by the batch-build path
        //  but single-wg QNode path (if used) depends on them
        scheduler->num_trivial_builds = 0;
        scheduler->num_single_builds = 0;

        // initialize the root-buffer counters
        sah_globals->root_buffer_num_produced     = 0;
        sah_globals->root_buffer_num_produced_hi  = 0;
        sah_globals->root_buffer_num_consumed     = 0;
        sah_globals->root_buffer_num_consumed_hi  = 0;
    }

    // initialize vcontext states
    for ( uint i = get_local_id( 0 ); i < BFS_NUM_VCONTEXTS; i += get_local_size( 0 ) )
        scheduler->vcontext_state[i] = (i==0) ? VCONTEXT_STATE_EXECUTING : VCONTEXT_STATE_UNALLOCATED;

    // initialize global bin info in vcontext - only context[0] will be used in first iteration
    BinInfo_init( &scheduler->contexts[0].global_bin_info );
    LRBounds_init( &scheduler->contexts[0].lr_bounds );

   // barrier( CLK_GLOBAL_MEM_FENCE  ); // lsc flush ... driver now does these as part of COMPUTE_WALKER
}

// TODO_OPT:  Enable larger WGs
//    We need a way to do this in a portable fashion.
//     Gen12 can support larger WGs than Gen9 can
//


// TODO_OPT:  Enable larger WGs
//    We need a way to do this in a portable fashion.
//     Gen12 can support larger WGs than Gen9 can
//
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(512, 1, 1)))
kernel void
categorize_builds_and_init_scheduler(
    global struct VContextScheduler* scheduler,
    global gpuva_t* globals_ptrs,                // OCL-C does not allow kernel parameters to be pointer-to-pointer, so we trick it...
    global struct SAHBuildBuffersInfo* buffers_info,
    global struct SAHBuildGlobals* builds_out,
    dword num_builds
)
{
    local uint num_trivial;
    local uint num_single;
    local uint num_full;

    if (get_group_id(0) == 0) // first workgroup performs build categorization
    {
        if (get_local_id(0) == 0)
        {
            num_trivial = 0;
            num_single = 0;
            num_full = 0;
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        // first pass, count builds of each type
        uint triv = 0;
        uint single = 0;
        uint full = 0;
        for (uint i = get_local_id(0); i < num_builds; i += get_local_size(0))
        {
            global struct Globals* globals = (global struct Globals*) globals_ptrs[i];
            dword num_refs = globals->numPrimitives;

            if (num_refs <= TRIVIAL_BUILD_THRESHOLD)
                triv++;
            else if (num_refs <= SINGLE_WG_BUILD_THRESHOLD)
                single++;
            else
                full++;
        }

        // merge counts across work-group.  These variables are now offsets into this thread's ranges
        triv   = atomic_add_local(&num_trivial, triv);
        single = atomic_add_local(&num_single, single);
        full   = atomic_add_local(&num_full, full);

        barrier(CLK_LOCAL_MEM_FENCE);

        global struct SAHBuildGlobals* trivial_builds_out = builds_out;
        global struct SAHBuildGlobals* single_builds_out = builds_out + num_trivial;
        global struct SAHBuildGlobals* full_builds_out = builds_out + num_trivial + num_single;

        for (uint i = get_local_id(0); i < num_builds; i += get_local_size(0))
        {
            global struct Globals* globals = (global struct Globals*) globals_ptrs[i];
            global struct SAHBuildBuffersInfo* buffers = &buffers_info[i];

            dword num_refs = globals->numPrimitives;
            dword leaf_type = globals->leafPrimType;
            dword leaf_size = globals->leafSize;

            global struct SAHBuildGlobals* place;
            if (num_refs <= TRIVIAL_BUILD_THRESHOLD)
                place = trivial_builds_out + (triv++);
            else if (num_refs <= SINGLE_WG_BUILD_THRESHOLD)
                place = single_builds_out + (single++);
            else
                place = full_builds_out + (full++);

            place->p_primref_index_buffers = buffers->p_primref_index_buffers;
            place->p_primrefs_buffer    = buffers->p_primrefs_buffer;
            place->p_bvh2               = buffers->p_bvh2;
            place->p_bvh_base           = buffers->p_bvh_base;
            place->p_globals            = (gpuva_t)globals;
            place->num_primrefs         = num_refs;
            place->leaf_size            = leaf_size;
            place->leaf_type            = leaf_type;
            place->flags                = buffers->sah_globals_flags;
            place->p_qnode_root_buffer  = buffers->p_qnode_root_buffer;

            // only initialize BVH2 if it will actually be used by the build
            //   trivial passes will not use it
            if( num_refs > SINGLE_WG_BUILD_THRESHOLD )
            {
                // initialize BVH2 node counter
                global struct BVH2* bvh2 = SAHBuildGlobals_GetBVH2(place);
                BVH2_Initialize(bvh2);

                // copy centroid bounds into the BVH2 root node'
                BVH2_SetNodeBox_lu(bvh2, BVH2_GetRoot(bvh2), globals->centroidBounds.lower.xyz, globals->centroidBounds.upper.xyz);
            }
        }

        if (get_local_id(0) == 0)
        {
            scheduler->num_trivial_builds   = num_trivial;
            scheduler->num_single_builds    = num_single;
            scheduler->batched_build_offset = num_trivial + num_single;
            scheduler->batched_build_count  = num_full;
        }
    }
    else // second workgroup initializes the scheduler
    {
        // initialize vcontext states
        for (uint i = get_local_id(0); i < BFS_NUM_VCONTEXTS; i += get_local_size(0))
            scheduler->vcontext_state[i] = (i == 0) ? VCONTEXT_STATE_EXECUTING : VCONTEXT_STATE_UNALLOCATED;

        // initialize global bin info in vcontexts
        for (uint i = get_sub_group_id(); i < BFS_NUM_VCONTEXTS; i += get_num_sub_groups())
            BinInfo_init_subgroup(&scheduler->contexts[i].global_bin_info);

        // initialize the spill stack
        if (get_local_id(0) == 0)
            scheduler->bfs2_spill_stack.size = 0;
    }

    //barrier( CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE );// lsc flush ... driver now does these as part of COMPUTE_WALKER
}





GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(BFS_NUM_VCONTEXTS, 1, 1)))
kernel void
begin_batchable(
    global struct VContextScheduler* scheduler,
    global struct SAHBuildGlobals* sah_globals
)
{
    ushort scheduler_build_offset = scheduler->batched_build_offset;
    ushort scheduler_num_builds   = scheduler->batched_build_count;

    ushort num_builds = min( scheduler_num_builds, (ushort)BFS_NUM_VCONTEXTS );

    uint num_wgs = 0;

    ushort tid = get_local_id(0);
    if ( tid < num_builds )
    {
        ushort batch_index = scheduler_build_offset + tid;

        uint num_primrefs = sah_globals[batch_index].num_primrefs;

        // configure first vcontext for first build
        scheduler->contexts[tid].dispatch_primref_begin = 0;
        scheduler->contexts[tid].dispatch_primref_end   = num_primrefs;
        scheduler->contexts[tid].bvh2_root              = BVH2_GetRoot( SAHBuildGlobals_GetBVH2(&sah_globals[batch_index]) );
        scheduler->contexts[tid].tree_depth             = 0;
        scheduler->contexts[tid].batch_index            = batch_index;
        scheduler->vcontext_state[tid] = VCONTEXT_STATE_EXECUTING;

        scheduler->contexts[tid].num_left = 0;
        scheduler->contexts[tid].num_right = 0;
        scheduler->contexts[tid].lr_mask   = 0;

        num_wgs = get_num_wgs( num_primrefs, BFS_WG_SIZE );

        scheduler->bfs_queue.wg_count[tid] = num_wgs;
        scheduler->bfs_queue.records[tid].batch_index = batch_index;
        scheduler->bfs_queue.records[tid].context_id  = tid;
    }

    num_wgs = work_group_reduce_add(num_wgs);

    if (tid == 0)
    {
        // write out build count and offset for next BFS iteration
        scheduler->batched_build_offset = scheduler_build_offset + num_builds;
        scheduler->batched_build_count  = scheduler_num_builds - num_builds;

        // write out initial WG count and loop termination mask for command streamer to consume
        scheduler->batched_build_wg_count  = num_wgs;
        scheduler->batched_build_loop_mask = (scheduler_num_builds > num_builds) ? 1 : 0;

        scheduler->bfs_queue.num_dispatches = num_builds;
    }

    for ( uint i = get_sub_group_id(); i < num_builds; i += get_num_sub_groups() )
        BinInfo_init_subgroup( &scheduler->contexts[i].global_bin_info );

    for ( uint i = get_sub_group_id(); i < num_builds; i += get_num_sub_groups() )
        LRBounds_init_subgroup( &scheduler->contexts[i].lr_bounds );
}



bool is_leaf( uint num_refs )
{
    return num_refs <= TREE_ARITY;
}

bool is_dfs( uint num_refs )
{
    return num_refs > TREE_ARITY&& num_refs <= DFS_THRESHOLD;
}

bool is_bfs( uint num_refs )
{
    return num_refs > DFS_THRESHOLD;
}

int2 is_leaf_2( uint2 num_refs )
{
    return num_refs.xy <= TREE_ARITY;
}
int2 is_bfs_2( uint2 num_refs )
{
    return num_refs.xy > DFS_THRESHOLD;
}

int2 is_dfs_2( uint2 num_refs )
{
    return num_refs.xy > TREE_ARITY && num_refs.xy <= DFS_THRESHOLD;
}

#if 0
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void
sg_scheduler( global struct VContextScheduler* scheduler )
{
    local struct BFS1SpillStackEntry SLM_local_spill_stack[BFS_NUM_VCONTEXTS];
    local uchar SLM_context_state[BFS_NUM_VCONTEXTS];
    local vcontext_id_t SLM_free_list[BFS_NUM_VCONTEXTS];
    local vcontext_id_t SLM_exec_list[BFS_NUM_VCONTEXTS];


    varying ushort lane = get_sub_group_local_id();

    uniform uint free_list_size = 0;
    uniform uint exec_list_size = 0;

    // read context states, build lists of free and executing contexts
    for (varying uint i = lane; i < BFS_NUM_VCONTEXTS; i += get_sub_group_size())
    {
        uchar state = scheduler->vcontext_state[i];
        SLM_context_state[i] = state;

        uniform ushort exec_mask = intel_sub_group_ballot(state == VCONTEXT_STATE_EXECUTING);

        varying ushort prefix_exec = subgroup_bit_prefix_exclusive(exec_mask);
        varying ushort prefix_free = lane - prefix_exec;
        varying ushort exec_list_pos = exec_list_size + prefix_exec;
        varying ushort free_list_pos = free_list_size + prefix_free;

        if (state == VCONTEXT_STATE_EXECUTING)
            SLM_exec_list[exec_list_pos] = i;
        else
            SLM_free_list[free_list_pos] = i;

        uniform ushort num_exec = popcount(exec_mask);
        exec_list_size += num_exec;
        free_list_size += get_sub_group_size() - num_exec;
    }

    uniform uint total_bfs_dispatches = 0;
    uniform uint total_dfs_dispatches = 0;
    uniform uint bfs_spill_stack_size   = 0;
    uniform uint total_bfs_wgs      = 0;

    // process executing context.  accumulate bfs/dfs dispatches and free-list entries
    for (uint i = 0; i < exec_list_size; i+= get_sub_group_size() )
    {
        varying ushort num_dfs_dispatches     = 0;
        varying ushort num_bfs_spills         = 0;

        varying ushort num_bfs_children;
        varying ushort context_id;
        struct VContext* context;
        varying uint num_left      ;
        varying uint num_right     ;
        varying uint primref_begin ;
        varying uint primref_end   ;
        varying uint depth         ;

        bool active_lane = (i + lane) < exec_list_size;
        if ( active_lane )
        {
            context_id = SLM_exec_list[i + lane];
            context    = &scheduler->contexts[context_id];

            num_left      = context->num_left;
            num_right     = context->num_right;
            primref_begin = context->dispatch_primref_begin;
            primref_end   = context->dispatch_primref_end;
            depth         = context->tree_depth;

            // get dispatch counts

            num_dfs_dispatches = is_dfs(num_left) + is_dfs(num_right);
            num_bfs_children = is_bfs(num_left) + is_bfs(num_right);
            num_bfs_spills = (num_bfs_children == 2) ? 1 : 0;
        }

        // allocate space for DFS, BFS dispatches, and BFS spills
        varying uint dfs_pos               = total_dfs_dispatches + sub_group_scan_exclusive_add(num_dfs_dispatches);
        varying ushort mask_bfs_spills     = intel_sub_group_ballot(num_bfs_children & 2); // spill if #children == 2
        varying ushort mask_bfs_dispatches = intel_sub_group_ballot(num_bfs_children & 3); // dispatch if #children == 1 or 2
        varying uint bfs_spill_pos         = bfs_spill_stack_size + subgroup_bit_prefix_exclusive(mask_bfs_spills);
        varying uint bfs_dispatch_pos      = total_bfs_dispatches + subgroup_bit_prefix_exclusive(mask_bfs_dispatches);

        total_dfs_dispatches += sub_group_reduce_add(num_dfs_dispatches);
        bfs_spill_stack_size += popcount(mask_bfs_spills);
        total_bfs_dispatches += popcount(mask_bfs_dispatches);

        varying uint num_bfs_wgs = 0;
        if (active_lane)
        {
            if (num_dfs_dispatches)
            {
                if (is_dfs(num_left))
                {
                    scheduler->dfs_queue.records[dfs_pos].primref_base = primref_begin;
                    scheduler->dfs_queue.records[dfs_pos].num_primrefs = num_left;
                    scheduler->dfs_queue.records[dfs_pos].bvh2_base = context->left_bvh2_root;
                    scheduler->dfs_queue.records[dfs_pos].tree_depth = depth + 1;
                    dfs_pos++;
                }
                if (is_dfs(num_right))
                {
                    scheduler->dfs_queue.records[dfs_pos].primref_base = primref_begin + num_left;
                    scheduler->dfs_queue.records[dfs_pos].num_primrefs = num_right;
                    scheduler->dfs_queue.records[dfs_pos].bvh2_base = context->right_bvh2_root;
                    scheduler->dfs_queue.records[dfs_pos].tree_depth = depth + 1;
                }
            }

            uint num_bfs_children = is_bfs(num_left) + is_bfs(num_right);
            if (num_bfs_children == 2)
            {
                // spill the right child.. push an entry onto local spill stack
                SLM_local_spill_stack[bfs_spill_pos].primref_begin = primref_begin + num_left;
                SLM_local_spill_stack[bfs_spill_pos].primref_end = primref_end;
                SLM_local_spill_stack[bfs_spill_pos].bvh2_root = context->right_bvh2_root;
                SLM_local_spill_stack[bfs_spill_pos].tree_depth = depth + 1;

                // setup BFS1 dispatch for left child
                context->dispatch_primref_end = primref_begin + num_left;
                context->bvh2_root = context->left_bvh2_root;
                context->tree_depth = depth + 1;
                num_bfs_wgs = get_num_wgs(num_left, BFS_WG_SIZE);

                scheduler->bfs_queue.wg_count[bfs_dispatch_pos]           = num_bfs_wgs;
                scheduler->bfs_queue.records[bfs_dispatch_pos].context_id = context_id;
            }
            else if (num_bfs_children == 1)
            {
                // setup BFS1 dispatch for whichever child wants it
                if (is_bfs(num_left))
                {
                    // bfs on left child
                    context->dispatch_primref_end = context->dispatch_primref_begin + num_left;
                    context->bvh2_root = context->left_bvh2_root;
                    context->tree_depth = depth + 1;
                    num_bfs_wgs = get_num_wgs(num_left, BFS_WG_SIZE);
                }
                else
                {
                    // bfs on right child
                    context->dispatch_primref_begin = context->dispatch_primref_begin + num_left;
                    context->bvh2_root = context->right_bvh2_root;
                    context->tree_depth = depth + 1;
                    num_bfs_wgs = get_num_wgs(num_right, BFS_WG_SIZE);
                }

                scheduler->bfs_queue.wg_count[bfs_dispatch_pos]           = num_bfs_wgs;
                scheduler->bfs_queue.records[bfs_dispatch_pos].context_id = context_id;
            }
            else
            {
                // no bfs dispatch.. this context is now free
                SLM_context_state[context_id] = VCONTEXT_STATE_UNALLOCATED;
            }
        }

        // count bfs work groups
        total_bfs_wgs += sub_group_reduce_add(num_bfs_wgs);

        // add newly deallocated contexts to the free list
        uniform uint free_mask = intel_sub_group_ballot( active_lane && num_bfs_children == 0);
        varying uint free_list_pos = free_list_size + subgroup_bit_prefix_exclusive(free_mask);
        free_list_size += popcount(free_mask);

        if ( free_mask & (1<<lane) )
            SLM_free_list[free_list_pos] = context_id;

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    // if we have more free contexts than spills, read additional spills from the scheduler's spill stack
    uniform uint memory_spill_stack_size = scheduler->bfs2_spill_stack.size;

    if(bfs_spill_stack_size < free_list_size && memory_spill_stack_size > 0 )
    {
        uniform uint read_count = min(free_list_size - bfs_spill_stack_size, memory_spill_stack_size);

        for (varying uint i = lane; i < read_count; i+= get_sub_group_size())
            SLM_local_spill_stack[bfs_spill_stack_size + i] = scheduler->bfs2_spill_stack.entries[memory_spill_stack_size - 1 - i];

        bfs_spill_stack_size += read_count;
        memory_spill_stack_size -= read_count;
    }

    // steal pending BFS work and assign it to free contexts
    uniform uint num_steals = min(bfs_spill_stack_size, free_list_size);

    for (uniform uint i = 0; i < num_steals; i += get_sub_group_size())
    {
        varying uint num_bfs_wgs = 0;

        if (i + lane < num_steals)
        {
            uint context_id = SLM_free_list[i+lane];
            struct VContext* context = &scheduler->contexts[context_id];
            struct BFS1SpillStackEntry entry = SLM_local_spill_stack[i+lane];

            context->dispatch_primref_begin = entry.primref_begin;
            context->dispatch_primref_end = entry.primref_end;
            context->bvh2_root = entry.bvh2_root;
            context->tree_depth = entry.tree_depth;

            num_bfs_wgs = get_num_wgs(entry.primref_end - entry.primref_begin, BFS_WG_SIZE);

            scheduler->bfs_queue.wg_count[total_bfs_dispatches + i + lane] = num_bfs_wgs;
            scheduler->bfs_queue.records[total_bfs_dispatches + i + lane].context_id = context_id;

            SLM_context_state[context_id] = VCONTEXT_STATE_EXECUTING;
        }

        total_bfs_wgs += sub_group_reduce_add( num_bfs_wgs );
    }

    total_bfs_dispatches += num_steals;

    //  write out excess spills to global spill stack
    uniform uint extra_spills = bfs_spill_stack_size - num_steals;
    for (varying uint i = lane; i < extra_spills; i += get_sub_group_size())
    {
        scheduler->bfs2_spill_stack.entries[memory_spill_stack_size + i] = SLM_local_spill_stack[num_steals+i];
    }


    // write out modified context states
    for ( varying uint i = lane; i < BFS_NUM_VCONTEXTS; i += get_sub_group_size())
        scheduler->vcontext_state[i] = SLM_context_state[i];


    if (get_local_id(0) == 0)
    {
        // write out new memory stack size
        scheduler->bfs2_spill_stack.size = memory_spill_stack_size + extra_spills;

        // store workgroup counters
        scheduler->bfs_queue.num_dispatches = total_bfs_dispatches;
        scheduler->num_bfs_wgs = total_bfs_wgs;
        scheduler->num_dfs_wgs = total_dfs_dispatches;
    }

  //  barrier(CLK_GLOBAL_MEM_FENCE); // make memory writes globally visible// lsc flush ... driver now does these as part of COMPUTE_WALKER
}
#endif

#define SCHEDULER_SG_SIZE 16
#define SCHEDULER_WG_SIZE BFS_NUM_VCONTEXTS
#define SCHEDULER_NUM_SGS (SCHEDULER_WG_SIZE / SCHEDULER_SG_SIZE)


struct BFSDispatchArgs get_bfs_args_from_record_batchable(
    struct BFSDispatchRecord* record,
    global struct VContextScheduler* scheduler,
    global struct SAHBuildGlobals* globals_buffer );

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(SCHEDULER_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(SCHEDULER_SG_SIZE)))
kernel void
scheduler(global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* sah_globals )
{
    local struct BFS1SpillStackEntry SLM_local_spill_stack[2 * BFS_NUM_VCONTEXTS];
    local uint SLM_local_spill_stack_size;
    local uint SLM_dfs_dispatch_count;

    if (get_local_id(0) == 0)
    {
        SLM_local_spill_stack_size = 0;
        SLM_dfs_dispatch_count = 0;
    }

    uint context_id = get_local_id(0);
    uint state = scheduler->vcontext_state[context_id];
    uint initial_state = state;

    uint batch_index = 0;
    global struct VContext* context = &scheduler->contexts[context_id];

    barrier(CLK_LOCAL_MEM_FENCE);


    uint global_spill_stack_size = scheduler->bfs2_spill_stack.size;


    if (state == VCONTEXT_STATE_EXECUTING)
    {
        uint left_bvh2_root;
        uint right_bvh2_root;

        uint num_left = context->num_left;
        uint num_right = context->num_right;

        uint primref_begin = context->dispatch_primref_begin;
        uint primref_end = context->dispatch_primref_end;

        uint depth = context->tree_depth;
        uint batch_index = context->batch_index;

        struct BFSDispatchRecord record;
        record.context_id = context_id;
        record.batch_index = context->batch_index;

        struct BFSDispatchArgs args = get_bfs_args_from_record_batchable( &record, scheduler, sah_globals);

        // do cleanup of bfs_pass2
        {
            // compute geom bounds
            struct AABB3f left_geom_bounds;
            struct AABB3f right_geom_bounds;
            struct AABB3f left_centroid_bounds;
            struct AABB3f right_centroid_bounds;
            uint2 lr_counts = (uint2)(num_left, num_right);

            {
                left_centroid_bounds    = LRBounds_get_left_centroid( &context->lr_bounds );
                left_geom_bounds        = LRBounds_get_left_geom(  &context->lr_bounds );
                right_centroid_bounds   = LRBounds_get_right_centroid( &context->lr_bounds );
                right_geom_bounds       = LRBounds_get_right_geom( &context->lr_bounds );
            }

            int2 v_is_leaf = is_leaf_2( lr_counts );
            int2 v_is_dfs  = is_dfs_2( lr_counts );
            int2 v_is_bfs  = is_bfs_2( lr_counts );
            uint left_mask  = args.do_mask_processing ? context->lr_mask & 0xff : 0xff;
            uint right_mask = args.do_mask_processing ? (context->lr_mask & 0xff00) >> 8 : 0xff;

            // how many BVH2 nodes do we need to allocate?  For DFS, we need to pre-allocate full subtree
            uint2 lr_node_counts = select( (uint2)(1,1), (2*lr_counts-1), v_is_dfs );
            uint left_node_count = lr_node_counts.x;
            uint right_node_count = lr_node_counts.y;

            // allocate the nodes
            uint first_node = BVH2_AllocateNodes( args.bvh2, left_node_count + right_node_count );

            // point our root node at its children
            left_bvh2_root  = first_node;
            right_bvh2_root = first_node + left_node_count;

            // store combined geom bounds in the root node's AABB.. we previously stored centroid bounds there
            //   but node creation requires geom bounds
            struct AABB3f geom_bounds = left_geom_bounds;
            AABB3f_extend(&geom_bounds, &right_geom_bounds);
            BVH2_WriteInnerNode( args.bvh2, args.bvh2_root, &geom_bounds, (uint2)(left_bvh2_root,right_bvh2_root), left_mask | right_mask );

//            printf(" node: %u  mask: %x\n", args.bvh2_root, left_mask|right_mask );

            // store the appropriate AABBs in the child nodes
            //   - BFS passes need centroid bounds
            //   - DFS passes need geom bounds
            //  Here we also write leaf connectivity information (prim start+count)
            //   this will be overwritten later if we are creating an inner node
            struct AABB3f left_box, right_box;
            left_box  = AABB3f_select( left_geom_bounds,  left_centroid_bounds,  v_is_bfs.xxx );
            right_box = AABB3f_select( right_geom_bounds, right_centroid_bounds, v_is_bfs.yyy );

            uint left_start  = primref_begin;
            uint right_start = primref_begin + num_left;
            BVH2_WriteLeafNode( args.bvh2, left_bvh2_root,  &left_box, left_start,  num_left, left_mask );
            BVH2_WriteLeafNode( args.bvh2, right_bvh2_root, &right_box, right_start, num_right, right_mask );

            // make input and output primref index buffers consistent in the event we're creating a leaf
            //   There should only ever be one leaf created, otherwise we'd have done a DFS pass sooner
            if (any( v_is_leaf.xy ))
            {
                uint start    = v_is_leaf.x ? left_start : right_start;
                uint num_refs = v_is_leaf.x ? num_left : num_right;

                for(uint i = 0; i < num_refs; i++)
                {
                    args.primref_index_in[start + i] = args.primref_index_out[start + i];
                }
            }
        }

        // when BFS2 finishes, we need to dispatch two child tasks.
        //   DFS dispatches can run free and do not need a context
        //   BFS dispatches need a context.
        //  In the case where both of the child nodes are BFS, the current context can immediately run one of the child dispatches
        //   and the other is spilled for an unallocated context to pick up

        uint num_dfs_dispatches = is_dfs(num_left) + is_dfs(num_right);
        if (num_dfs_dispatches)
        {
            uint dfs_pos = atomic_add_local(&SLM_dfs_dispatch_count, num_dfs_dispatches);
            if (is_dfs(num_left))
            {
                scheduler->dfs_queue.records[dfs_pos].primref_base = primref_begin;
                scheduler->dfs_queue.records[dfs_pos].num_primrefs = num_left;
                scheduler->dfs_queue.records[dfs_pos].bvh2_base = left_bvh2_root;
                scheduler->dfs_queue.records[dfs_pos].tree_depth = depth + 1;
                scheduler->dfs_queue.records[dfs_pos].batch_index = batch_index;
                dfs_pos++;
            }
            if (is_dfs(num_right))
            {
                scheduler->dfs_queue.records[dfs_pos].primref_base = primref_begin + num_left;
                scheduler->dfs_queue.records[dfs_pos].num_primrefs = num_right;
                scheduler->dfs_queue.records[dfs_pos].bvh2_base = right_bvh2_root;
                scheduler->dfs_queue.records[dfs_pos].tree_depth = depth + 1;
                scheduler->dfs_queue.records[dfs_pos].batch_index = batch_index;
            }
        }

        uint num_bfs_children = is_bfs(num_left) + is_bfs(num_right);
        if (num_bfs_children)
        {
            uint place = atomic_add_local(&SLM_local_spill_stack_size, num_bfs_children);
            if (is_bfs(num_left))
            {
                SLM_local_spill_stack[place].primref_begin = primref_begin;
                SLM_local_spill_stack[place].primref_end = primref_begin + num_left;
                SLM_local_spill_stack[place].bvh2_root = left_bvh2_root;
                SLM_local_spill_stack[place].tree_depth = depth + 1;
                SLM_local_spill_stack[place].batch_index = batch_index;
                place++;
            }
            if (is_bfs(num_right))
            {
                SLM_local_spill_stack[place].primref_begin = primref_begin + num_left;
                SLM_local_spill_stack[place].primref_end = primref_end;
                SLM_local_spill_stack[place].bvh2_root = right_bvh2_root;
                SLM_local_spill_stack[place].tree_depth = depth + 1;
                SLM_local_spill_stack[place].batch_index = batch_index;
                place++;
            }
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    uint local_spill_stack_size = SLM_local_spill_stack_size;

    struct BFS1SpillStackEntry entry;
    state = VCONTEXT_STATE_UNALLOCATED;
    if (context_id < local_spill_stack_size)
    {
        // pull BFS work from the local spill stack if there's enough work there
        entry = SLM_local_spill_stack[context_id];
        state = VCONTEXT_STATE_EXECUTING;
    }
    else if ((context_id - local_spill_stack_size) < (global_spill_stack_size))
    {
        // if there isn't enough work on the local stack, consume from the global one
        uint global_pos = (global_spill_stack_size - 1) - (context_id - local_spill_stack_size);
        entry = scheduler->bfs2_spill_stack.entries[global_pos];
        state = VCONTEXT_STATE_EXECUTING;
    }

    // contexts which received work set themselves up for the next BFS1 dispatch
    uint num_bfs_wgs = 0;
    uint num_bfs_dispatches = 0;
    if (state == VCONTEXT_STATE_EXECUTING)
    {
        context->dispatch_primref_begin = entry.primref_begin;
        context->dispatch_primref_end = entry.primref_end;
        context->bvh2_root = entry.bvh2_root;
        context->tree_depth = entry.tree_depth;
        context->batch_index = entry.batch_index;

        context->num_left = 0;
        context->num_right = 0;
        context->lr_mask = 0;

        batch_index = entry.batch_index;
        num_bfs_wgs = get_num_wgs(entry.primref_end - entry.primref_begin, BFS_WG_SIZE);
        num_bfs_dispatches = 1;
    }


    if (local_spill_stack_size > BFS_NUM_VCONTEXTS)
    {
        // write out additional spills if we produced more work than we can consume
        uint excess_spills = local_spill_stack_size - BFS_NUM_VCONTEXTS;
        uint write_base = global_spill_stack_size;
        uint lid = get_local_id(0);
        if (lid < excess_spills)
            scheduler->bfs2_spill_stack.entries[write_base + lid] = SLM_local_spill_stack[BFS_NUM_VCONTEXTS + lid];

        if (lid == 0)
            scheduler->bfs2_spill_stack.size = global_spill_stack_size + excess_spills;
    }
    else if (global_spill_stack_size > 0)
    {
        // otherwise, if we consumed any spills from the global stack, update the stack size
        if (get_local_id(0) == 0)
        {
            uint global_spills_consumed = min(global_spill_stack_size, BFS_NUM_VCONTEXTS - local_spill_stack_size);
            scheduler->bfs2_spill_stack.size = global_spill_stack_size - global_spills_consumed;
        }
    }


    // Do various WG reductions..  the code below is a hand-written version of the following:
    //
    // uint bfs_dispatch_queue_pos     = work_group_scan_exclusive_add( num_bfs_dispatches );
    // uint reduce_num_bfs_wgs         = work_group_reduce_add(num_bfs_wgs);
    // uint reduce_num_bfs_dispatches  = work_group_reduce_add(num_bfs_dispatches);
    uint bfs_dispatch_queue_pos;
    uint reduce_num_bfs_dispatches;
    uint reduce_num_bfs_wgs;
    local uint partial_dispatches[SCHEDULER_WG_SIZE / SCHEDULER_SG_SIZE];
    local uint partial_wgs[SCHEDULER_WG_SIZE / SCHEDULER_SG_SIZE];
    {
        partial_dispatches[get_sub_group_id()] = sub_group_reduce_add(num_bfs_dispatches);
        partial_wgs[get_sub_group_id()] = sub_group_reduce_add(num_bfs_wgs);

        uint sg_prefix = sub_group_scan_exclusive_add(num_bfs_dispatches);

        uint prefix_dispatches = 0;
        uint total_dispatches = 0;
        uint total_wgs = 0;
        ushort lane = get_sub_group_local_id();

        barrier(CLK_LOCAL_MEM_FENCE);

        for (ushort i = 0; i < SCHEDULER_NUM_SGS; i += SCHEDULER_SG_SIZE) // this loop is intended to be fully unrolled after compilation
        {
            uint p_dispatch = partial_dispatches[i + lane];
            uint p_wg = partial_wgs[i + lane];

            prefix_dispatches += (i + lane < get_sub_group_id()) ? p_dispatch : 0;
            total_dispatches += p_dispatch;
            total_wgs += p_wg;
        }

        bfs_dispatch_queue_pos = sg_prefix + sub_group_reduce_add(prefix_dispatches);
        reduce_num_bfs_dispatches = sub_group_reduce_add(total_dispatches);
        reduce_num_bfs_wgs = sub_group_reduce_add(total_wgs);
    }

    // insert records into BFS queue
    if (num_bfs_dispatches)
    {
        scheduler->bfs_queue.wg_count[bfs_dispatch_queue_pos] = num_bfs_wgs;
        scheduler->bfs_queue.records[bfs_dispatch_queue_pos].context_id = context_id;
        scheduler->bfs_queue.records[bfs_dispatch_queue_pos].batch_index = batch_index;
    }


    // store modified vcontext state if it has changed
    if (initial_state != state)
        scheduler->vcontext_state[context_id] = state;


    // store workgroup counters
    if (get_local_id(0) == 0)
    {
        scheduler->bfs_queue.num_dispatches = reduce_num_bfs_dispatches;
        scheduler->num_bfs_wgs = reduce_num_bfs_wgs;
        scheduler->num_dfs_wgs = SLM_dfs_dispatch_count;
    }

    const uint contexts_to_clear = min( (uint)BFS_NUM_VCONTEXTS, (uint)(local_spill_stack_size+global_spill_stack_size) );

    for ( uint i = get_sub_group_id(); i < contexts_to_clear; i += get_num_sub_groups() )
        BinInfo_init_subgroup( &scheduler->contexts[i].global_bin_info );

    for ( uint i = get_sub_group_id(); i < contexts_to_clear; i += get_num_sub_groups() )
        LRBounds_init_subgroup( &scheduler->contexts[i].lr_bounds );
}

#if 0
uint record_search( struct BFSDispatchRecord* record_out, global struct BFSDispatchQueue* queue )
{
    uint group = get_group_id(0);
    ushort lane = get_sub_group_local_id();
    uint num_dispatches = queue->num_dispatches;
    uint base = 0;
    for (uint i = 0; i < num_dispatches; i += get_sub_group_size())
    {
        uint counts = intel_sub_group_block_read(&queue->wg_count[i]);

        for (uint j = 0; j < get_sub_group_size(); j++)
        {
            uint n = sub_group_broadcast(counts, j);
            if (group < n)
            {
                *record_out = queue->records[i + j];
                return group;
            }
            group -= n;
        }
    }

    return 0; // NOTE: unreachable in practice
}
#endif


uint record_search(struct BFSDispatchRecord* record_out, global struct BFSDispatchQueue* queue)
{
    uint group = get_group_id(0);

    uint num_dispatches = queue->num_dispatches;

    uint dispatch_id = 0;
    uint local_id = 0;
    uint i = 0;
    do
    {
        uint counts = intel_sub_group_block_read(&queue->wg_count[i]);
        uint prefix = sub_group_scan_exclusive_add(counts);

        uint g = group - prefix;
        uint ballot = intel_sub_group_ballot(g < counts);
        if (ballot)
        {
            uint lane = ctz(ballot);
            dispatch_id = i + lane;
            local_id = intel_sub_group_shuffle(g, lane);
            break;
        }

        group -= sub_group_broadcast(prefix + counts, get_sub_group_size() - 1);

        i += get_sub_group_size();
    } while (i < num_dispatches);


    *record_out = queue->records[dispatch_id];
    return local_id;
}




struct BFSDispatchArgs get_bfs_args(struct BFSDispatchRecord* record, global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* globals, uint local_group_id)
{
    uint context_id = record->context_id;
    struct VContext* context = &scheduler->contexts[context_id];
    bool odd_pass = context->tree_depth & 1;

    struct BFSDispatchArgs args;
    args.scheduler              = scheduler;
    args.primref_index_in       = SAHBuildGlobals_GetPrimrefIndices_In( globals, odd_pass );
    args.primref_index_out      = SAHBuildGlobals_GetPrimrefIndices_Out( globals, odd_pass );
    args.primref_buffer         = SAHBuildGlobals_GetPrimrefs( globals );
    args.wg_primref_begin       = context->dispatch_primref_begin + local_group_id * BFS_WG_SIZE;
    args.wg_primref_end         = min( args.wg_primref_begin + BFS_WG_SIZE, context->dispatch_primref_end );
    args.dispatch_primref_begin = context->dispatch_primref_begin;
    args.dispatch_primref_end   = context->dispatch_primref_end;
    args.context_id             = context_id;
    args.context                = &scheduler->contexts[context_id];
    args.num_wgs                = ((args.dispatch_primref_end - args.dispatch_primref_begin) + BFS_WG_SIZE - 1) / BFS_WG_SIZE;
    args.bvh2_root              = context->bvh2_root;
    args.bvh2 = SAHBuildGlobals_GetBVH2( globals );
    args.global_num_primrefs = SAHBuildGlobals_GetTotalPrimRefs( globals );
    args.do_mask_processing = SAHBuildGlobals_NeedMasks( globals );
    return args;
}

struct BFSDispatchArgs get_bfs_args_queue( global struct BFSDispatchQueue* queue,
                                           global struct VContextScheduler* scheduler,
                                           global struct SAHBuildGlobals* globals )
{

    // TODO_OPT:  Load this entire prefix array into SLM instead of searching..
    //    Or use sub-group ops

    struct BFSDispatchRecord record;
    uint local_group_id = record_search(&record, queue);

    return get_bfs_args(&record, scheduler, globals, local_group_id);
}


struct BFSDispatchArgs get_bfs_args_from_record( struct BFSDispatchRecord* record,
                                           global struct VContextScheduler* scheduler,
                                           global struct SAHBuildGlobals* globals )
{
    return get_bfs_args(record, scheduler, globals, 0);
}


struct BFSDispatchArgs get_bfs_args_batchable(
    global struct BFSDispatchQueue* queue,
    global struct VContextScheduler* scheduler,
    global struct SAHBuildGlobals* globals_buffer )
{

    // TODO_OPT:  Load this entire prefix array into SLM instead of searching..
    //    Or use sub-group ops

    struct BFSDispatchRecord record;
    uint local_group_id = record_search(&record, queue);

    global struct SAHBuildGlobals* globals = globals_buffer + record.batch_index;

    return get_bfs_args(&record, scheduler, globals, local_group_id);
}


struct BFSDispatchArgs get_bfs_args_from_record_batchable(
    struct BFSDispatchRecord* record,
    global struct VContextScheduler* scheduler,
    global struct SAHBuildGlobals* globals_buffer )
{
    global struct SAHBuildGlobals* globals = globals_buffer + record->batch_index;

    return get_bfs_args(record, scheduler, globals, 0);
}

struct BFSDispatchArgs get_bfs_args_initial( global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* globals )
{
    uint context_id = 0;

    uint num_refs = SAHBuildGlobals_GetTotalPrimRefs( globals );

    struct BFSDispatchArgs args;
    args.scheduler = scheduler;
    args.primref_index_in   = SAHBuildGlobals_GetPrimrefIndices_In( globals, false );
    args.primref_index_out  = SAHBuildGlobals_GetPrimrefIndices_Out( globals, false );
    args.primref_buffer     = SAHBuildGlobals_GetPrimrefs( globals );
    args.wg_primref_begin   = get_group_id(0) * BFS_WG_SIZE;
    args.wg_primref_end     = min( args.wg_primref_begin + BFS_WG_SIZE, num_refs );
    args.dispatch_primref_begin = 0;
    args.dispatch_primref_end   = num_refs;
    args.context_id = context_id;
    args.context = &scheduler->contexts[context_id];
    args.num_wgs = ((args.dispatch_primref_end - args.dispatch_primref_begin) + BFS_WG_SIZE - 1) / BFS_WG_SIZE;
    args.bvh2 = SAHBuildGlobals_GetBVH2( globals );
    args.bvh2_root = BVH2_GetRoot( args.bvh2 );
    args.global_num_primrefs = SAHBuildGlobals_GetTotalPrimRefs( globals );
    args.do_mask_processing = SAHBuildGlobals_NeedMasks(globals);
    return args;
}


inline void BinMapping_init( struct BinMapping* binMapping, struct AABB3f* centBounds, const uint bins )
{
    const float4 eps = 1E-34f;
    const float4 omega = 1E+34f;
    float3 l = AABB3f_load_lower( centBounds );
    float3 u = AABB3f_load_upper( centBounds );
    float4 diag;
    diag.xyz = max( eps.xyz, u - l );
    diag.w = 0;
    float4 scale = (float4)(0.99f * (float)bins) / diag;
    scale = select( (float4)(0.0f), scale, diag > eps );
    scale = select( (float4)(0.0f), scale, diag < omega );
    binMapping->scale = scale;
    binMapping->ofs.xyz = l.xyz;
    binMapping->ofs.w = 0;
}


inline ulong getBestSplit( float3 sah, uint ID, const float4 scale, const ulong defaultSplit )
{
    ulong splitX = (((ulong)as_uint( sah.x )) << 32) | ((uint)ID << 2) | 0;
    ulong splitY = (((ulong)as_uint( sah.y )) << 32) | ((uint)ID << 2) | 1;
    ulong splitZ = (((ulong)as_uint( sah.z )) << 32) | ((uint)ID << 2) | 2;
    /* ignore zero sized dimensions */
    splitX = select( splitX, defaultSplit, (ulong)(scale.x == 0) );
    splitY = select( splitY, defaultSplit, (ulong)(scale.y == 0) );
    splitZ = select( splitZ, defaultSplit, (ulong)(scale.z == 0) );
    ulong bestSplit = min( min( splitX, splitY ), splitZ );
    bestSplit = sub_group_reduce_min( bestSplit );
    return bestSplit;
}



inline float left_to_right_area16( struct AABB3f* low )
{
    struct AABB3f low_prefix = AABB3f_sub_group_scan_exclusive_min_max( low );
    return halfArea_AABB3f( &low_prefix );
}

inline uint left_to_right_counts16( uint low )
{
    return sub_group_scan_exclusive_add( low );
}

inline float right_to_left_area16( struct AABB3f* low )
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint ID = subgroup_size - 1 - subgroupLocalID;
    struct AABB3f low_reverse = AABB3f_sub_group_shuffle( low, ID );
    struct AABB3f low_prefix = AABB3f_sub_group_scan_inclusive_min_max( &low_reverse );
    const float low_area = intel_sub_group_shuffle( halfArea_AABB3f( &low_prefix ), ID );
    return low_area;
}

inline uint right_to_left_counts16( uint low )
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint ID = subgroup_size - 1 - subgroupLocalID;
    const uint low_reverse = intel_sub_group_shuffle( low, ID );
    const uint low_prefix = sub_group_scan_inclusive_add( low_reverse );
    return intel_sub_group_shuffle( low_prefix, ID );
}

inline float2 left_to_right_area32( struct AABB3f* low, struct AABB3f* high )
{
    struct AABB3f low_prefix = AABB3f_sub_group_scan_exclusive_min_max( low );
    struct AABB3f low_reduce = AABB3f_sub_group_reduce( low );
    struct AABB3f high_prefix = AABB3f_sub_group_scan_exclusive_min_max( high );
    AABB3f_extend( &high_prefix, &low_reduce );
    const float low_area = halfArea_AABB3f( &low_prefix );
    const float high_area = halfArea_AABB3f( &high_prefix );
    return (float2)(low_area, high_area);
}

inline uint2 left_to_right_counts32( uint low, uint high )
{
    const uint low_prefix = sub_group_scan_exclusive_add( low );
    const uint low_reduce = sub_group_reduce_add( low );
    const uint high_prefix = sub_group_scan_exclusive_add( high );
    return (uint2)(low_prefix, low_reduce + high_prefix);
}

inline float2 right_to_left_area32( struct AABB3f* low, struct AABB3f* high )
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint ID = subgroup_size - 1 - subgroupLocalID;
    struct AABB3f low_reverse = AABB3f_sub_group_shuffle( high, ID );
    struct AABB3f high_reverse = AABB3f_sub_group_shuffle( low, ID );
    struct AABB3f low_prefix = AABB3f_sub_group_scan_inclusive_min_max( &low_reverse );
    struct AABB3f low_reduce = AABB3f_sub_group_reduce( &low_reverse );
    struct AABB3f high_prefix = AABB3f_sub_group_scan_inclusive_min_max( &high_reverse );
    AABB3f_extend( &high_prefix, &low_reduce );
    const float low_area = intel_sub_group_shuffle( halfArea_AABB3f( &high_prefix ), ID );
    const float high_area = intel_sub_group_shuffle( halfArea_AABB3f( &low_prefix ), ID );
    return (float2)(low_area, high_area);
}

inline uint2 right_to_left_counts32( uint low, uint high )
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint ID = subgroup_size - 1 - subgroupLocalID;
    const uint low_reverse = intel_sub_group_shuffle( high, ID );
    const uint high_reverse = intel_sub_group_shuffle( low, ID );
    const uint low_prefix = sub_group_scan_inclusive_add( low_reverse );
    const uint low_reduce = sub_group_reduce_add( low_reverse );
    const uint high_prefix = sub_group_scan_inclusive_add( high_reverse ) + low_reduce;
    return (uint2)(intel_sub_group_shuffle( high_prefix, ID ), intel_sub_group_shuffle( low_prefix, ID ));
}

inline uint fastDivideBy6_uint( uint v )
{
#if 1
    const ulong u = (ulong)v >> 1;
    return (uint)((u * 0x55555556ul) >> 32);
#else
    return v / 6;
#endif
}

inline uint3 fastDivideBy6_uint3( uint3 v )
{
    return (uint3)(fastDivideBy6_uint( v.x ), fastDivideBy6_uint( v.y ), fastDivideBy6_uint( v.z ));
}

#define SAH_LOG_BLOCK_SHIFT 2

inline struct BFS_Split BinInfo_reduce( struct BFS_BinInfo* binInfo, const float4 scale )
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();

    struct AABB3f boundsX = BinInfo_get_AABB( binInfo, subgroupLocalID, 0 );

    const float lr_areaX = left_to_right_area16( &boundsX );
    const float rl_areaX = right_to_left_area16( &boundsX );

    struct AABB3f boundsY = BinInfo_get_AABB( binInfo, subgroupLocalID, 1 );

    const float lr_areaY = left_to_right_area16( &boundsY );
    const float rl_areaY = right_to_left_area16( &boundsY );

    struct AABB3f boundsZ = BinInfo_get_AABB( binInfo, subgroupLocalID, 2 );

    const float lr_areaZ = left_to_right_area16( &boundsZ );
    const float rl_areaZ = right_to_left_area16( &boundsZ );

    const uint3 counts = BinInfo_get_counts( binInfo, subgroupLocalID );

    const uint lr_countsX = left_to_right_counts16( counts.x );
    const uint rl_countsX = right_to_left_counts16( counts.x );
    const uint lr_countsY = left_to_right_counts16( counts.y );
    const uint rl_countsY = right_to_left_counts16( counts.y );
    const uint lr_countsZ = left_to_right_counts16( counts.z );
    const uint rl_countsZ = right_to_left_counts16( counts.z );

    const float3 lr_area = (float3)(lr_areaX, lr_areaY, lr_areaZ);
    const float3 rl_area = (float3)(rl_areaX, rl_areaY, rl_areaZ);

    const uint3 lr_count = fastDivideBy6_uint3( (uint3)(lr_countsX, lr_countsY, lr_countsZ) + 6 - 1 );
    const uint3 rl_count = fastDivideBy6_uint3( (uint3)(rl_countsX, rl_countsY, rl_countsZ) + 6 - 1 );
    float3 sah = fma( lr_area, convert_float3( lr_count ), rl_area * convert_float3( rl_count ) );

    /* first bin is invalid */
    sah.x = select( (float)(INFINITY), sah.x, subgroupLocalID != 0 );
    sah.y = select( (float)(INFINITY), sah.y, subgroupLocalID != 0 );
    sah.z = select( (float)(INFINITY), sah.z, subgroupLocalID != 0 );

    const ulong defaultSplit = (((ulong)as_uint( (float)(INFINITY) )) << 32);

    const ulong bestSplit = getBestSplit( sah, subgroupLocalID, scale, defaultSplit );

    struct BFS_Split split;
    split.sah = as_float( (uint)(bestSplit >> 32) );
    split.dim = (uint)bestSplit & 3;
    split.pos = (uint)bestSplit >> 2;

    return split;
}


struct BFS_BinInfoReduce3_SLM
{
    uint sah[3*BFS_NUM_BINS];
};



inline struct BFS_Split BinInfo_reduce3( local struct BFS_BinInfoReduce3_SLM* slm, struct BFS_BinInfo* binInfo, const float4 scale )
{
    // process each bin/axis combination across sub-groups
    for (uint i = get_sub_group_id(); i < 3 * BFS_NUM_BINS; i += get_num_sub_groups())
    {
        uint my_bin  = i % BFS_NUM_BINS;
        uint my_axis = i / BFS_NUM_BINS;

        float3 left_lower  = (float3)(INFINITY,INFINITY,INFINITY);
        float3 left_upper  = -left_lower;
        float3 right_lower = (float3)(INFINITY,INFINITY,INFINITY);
        float3 right_upper = -right_lower;

        // load the other bins and assign them to the left or to the right
        //  of this subgroup's bin
        uint lane = get_sub_group_local_id();
        struct AABB3f sg_bins = BinInfo_get_AABB(binInfo,lane,my_axis);

        bool is_left = (lane < my_bin);
        float3 lower = AABB3f_load_lower(&sg_bins);
        float3 upper = AABB3f_load_upper(&sg_bins);

        float3 lower_l = select_min( lower, is_left  );
        float3 upper_l = select_max( upper, is_left  );
        float3 lower_r = select_min( lower, !is_left );
        float3 upper_r = select_max( upper, !is_left );

        lower_l = sub_group_reduce_min_float3( lower_l );
        lower_r = sub_group_reduce_min_float3( lower_r );
        upper_l = sub_group_reduce_max_float3( upper_l );
        upper_r = sub_group_reduce_max_float3( upper_r );
        float3 dl = upper_l - lower_l;
        float3 dr = upper_r - lower_r;
        float area_l =  dl.x* (dl.y + dl.z) + (dl.y * dl.z);
        float area_r =  dr.x* (dr.y + dr.z) + (dr.y * dr.z);

        // get the counts
        uint sg_bin_count = BinInfo_get_count(binInfo, lane, my_axis);
        uint count_l = (is_left) ?  sg_bin_count : 0;
        uint count_r = (is_left) ?  0 : sg_bin_count;
        count_l = sub_group_reduce_add(count_l);
        count_r = sub_group_reduce_add(count_r);

        // compute sah
        count_l = fastDivideBy6_uint(count_l + 6 - 1);
        count_r = fastDivideBy6_uint(count_r + 6 - 1);
        float lr_partial = area_l * count_l;
        float rl_partial = area_r * count_r;
        float sah = lr_partial + rl_partial;

        // first bin is invalid
        sah = select((float)(INFINITY), sah, my_bin != 0);

        // ignore zero sized dimensions
        sah = select( sah, (float)(INFINITY), (scale.x == 0 && my_axis == 0) );
        sah = select( sah, (float)(INFINITY), (scale.y == 0 && my_axis == 1) );
        sah = select( sah, (float)(INFINITY), (scale.z == 0 && my_axis == 2) );

        // tuck the axis into the bottom bits of sah cost.
        //  The result is an integer between 0 and +inf (7F800000)
        //  If we have 3 axes with infinite sah cost, we will select axis 0
        slm->sah[i] = (as_uint(sah)&~0x3) | my_axis;
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    // reduce split candidates down to one subgroup
    //  sah is strictly positive, so integer compares can be used
    //   which results in a faster sub_group_reduce_min()
    //
    uint best_sah = 0xffffffff;

    uint lid = get_sub_group_local_id();
    if (lid < BFS_NUM_BINS)
    {
        best_sah = slm->sah[lid];
        lid += BFS_NUM_BINS;
        best_sah = min( best_sah, slm->sah[lid] );
        lid += BFS_NUM_BINS;
        best_sah = min( best_sah, slm->sah[lid] );
    }

    uint reduced_bestsah = sub_group_reduce_min( best_sah );
    uint best_bin = ctz(intel_sub_group_ballot(best_sah == reduced_bestsah));
    uint best_axis = as_uint(reduced_bestsah) & 0x3;

    struct BFS_Split ret;
    ret.sah = as_float(reduced_bestsah);
    ret.dim = best_axis;
    ret.pos = best_bin;
    return ret;
}


struct BFS_BinInfoReduce_SLM
{
    struct
    {
        float sah;
        uint bin;
    } axisInfo[3];
};



inline struct BFS_Split BinInfo_reduce2( local struct BFS_BinInfoReduce_SLM* slm, struct BFS_BinInfo* binInfo, const float4 scale, uint num_primrefs)
{
    ushort my_axis = get_sub_group_id();
    ushort my_bin  = get_sub_group_local_id();

    if (my_axis < 3)
    {
        struct AABB3f aabb = BinInfo_get_AABB(binInfo, my_bin, my_axis);
        uint count         = BinInfo_get_count(binInfo, my_bin, my_axis);

        float lr_area = left_to_right_area16(&aabb);
        float rl_area = right_to_left_area16(&aabb);

        uint lr_count = sub_group_scan_exclusive_add(count);
        uint rl_count = num_primrefs - lr_count;

        lr_count = fastDivideBy6_uint(lr_count + 6 - 1);
        rl_count = fastDivideBy6_uint(rl_count + 6 - 1);
        float lr_partial = lr_area * lr_count;
        float rl_partial = rl_area * rl_count;
        float sah = lr_partial + rl_partial;

        // first bin is invalid
        sah = select((float)(INFINITY), sah, my_bin != 0);

        float best_sah = sub_group_reduce_min( sah );
        uint best_bin = ctz(intel_sub_group_ballot(sah == best_sah));

        // ignore zero sized dimensions
        best_sah = select( best_sah, (float)(INFINITY), (scale.x == 0 && my_axis == 0) );
        best_sah = select( best_sah, (float)(INFINITY), (scale.y == 0 && my_axis == 1) );
        best_sah = select( best_sah, (float)(INFINITY), (scale.z == 0 && my_axis == 2) );

        if (get_sub_group_local_id() == 0)
        {
            slm->axisInfo[my_axis].sah = best_sah;
            slm->axisInfo[my_axis].bin = best_bin;
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

    float sah = (float)(INFINITY);
    if( get_sub_group_local_id() < 3 )
        sah = slm->axisInfo[get_sub_group_local_id()].sah;

    float bestsah = min(sub_group_broadcast(sah, 0), min(sub_group_broadcast(sah, 1), sub_group_broadcast(sah, 2)));
    uint bestAxis = ctz( intel_sub_group_ballot(bestsah == sah) );

    struct BFS_Split split;
    split.sah = bestsah;
    split.dim = bestAxis;
    split.pos = slm->axisInfo[bestAxis].bin;
    return split;
}


inline bool is_left( struct BinMapping* binMapping, struct BFS_Split* split, struct AABB* primref )
{
    const uint dim = split->dim;
    const float lower = primref->lower[dim];
    const float upper = primref->upper[dim];
    const float c = lower + upper;
    const uint pos = convert_uint_rtz( (c - binMapping->ofs[dim]) * binMapping->scale[dim] );
    return pos < split->pos;
}

struct BFS_Pass1_SLM
{
    struct BFS_BinInfo bin_info;
//    struct BFS_BinInfoReduce3_SLM reduce3;
};


void DO_BFS_pass1( local struct BFS_Pass1_SLM*  slm,
                   uint thread_primref_id,
                   bool thread_primref_valid,
                   struct BFSDispatchArgs args
                  )
{
    local struct BFS_BinInfo* local_bin_info = &slm->bin_info;
    global struct VContext* context  = args.context;
    struct AABB3f centroid_bounds    = BVH2_GetNodeBox( args.bvh2, args.bvh2_root ); // root AABB is initialized to centroid bounds

    struct BinMapping bin_mapping;
    BinMapping_init( &bin_mapping, &centroid_bounds, BFS_NUM_BINS );

    // fetch this thread's primref
    PrimRef ref;
    if ( thread_primref_valid )
        ref = args.primref_buffer[thread_primref_id];

    // init bin info
    BinInfo_init( local_bin_info );

    // fence on local bin-info init
    barrier( CLK_LOCAL_MEM_FENCE );

    // merge this thread's primref into local bin info
    BinInfo_add_primref( &bin_mapping, local_bin_info, &ref, thread_primref_valid );

    // fence on local bin-info update
    barrier( CLK_LOCAL_MEM_FENCE );

    BinInfo_merge(&context->global_bin_info, local_bin_info);
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size(BFS_WG_SIZE,1,1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void
BFS_pass1_indexed(
    global struct VContextScheduler* scheduler,
    global struct SAHBuildGlobals* sah_globals )
{
    local struct BFS_Pass1_SLM slm;
    struct BFSDispatchArgs args = get_bfs_args_queue( &scheduler->bfs_queue, scheduler, sah_globals );

    bool thread_primref_valid = (args.wg_primref_begin + get_local_id( 0 )) < args.wg_primref_end;
    uint thread_primref_id = 0;
    if ( thread_primref_valid )
        thread_primref_id = args.primref_index_in[args.wg_primref_begin + get_local_id( 0 )];

    DO_BFS_pass1( &slm, thread_primref_id, thread_primref_valid, args );
}


__attribute__( (reqd_work_group_size( BFS_WG_SIZE, 1, 1 )) )
__attribute__((intel_reqd_sub_group_size(16)))
kernel void
BFS_pass1_initial( global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* sah_globals )
{
    local struct BFS_Pass1_SLM slm;
    struct BFSDispatchArgs args = get_bfs_args_initial( scheduler, sah_globals );

    uint thread_primref_id    = args.wg_primref_begin + get_local_id( 0 );
    bool thread_primref_valid = thread_primref_id < args.wg_primref_end;

    DO_BFS_pass1( &slm, thread_primref_id, thread_primref_valid, args );
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(BFS_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void
BFS_pass1_indexed_batchable(
    global struct VContextScheduler* scheduler,
    global struct SAHBuildGlobals* globals_buffer )
{
    local struct BFS_Pass1_SLM slm;
    struct BFSDispatchArgs args = get_bfs_args_batchable( &scheduler->bfs_queue, scheduler, globals_buffer );

    bool thread_primref_valid = (args.wg_primref_begin + get_local_id(0)) < args.wg_primref_end;
    uint thread_primref_id = 0;
    if (thread_primref_valid)
        thread_primref_id = args.primref_index_in[args.wg_primref_begin + get_local_id(0)];

    DO_BFS_pass1(&slm, thread_primref_id, thread_primref_valid, args);
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(BFS_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void
BFS_pass1_initial_batchable( global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* globals_buffer )
{
    local struct BFS_Pass1_SLM slm;
    struct BFSDispatchArgs args = get_bfs_args_batchable( &scheduler->bfs_queue, scheduler, globals_buffer );

    uint thread_primref_id = args.wg_primref_begin + get_local_id(0);
    bool thread_primref_valid = thread_primref_id < args.wg_primref_end;

    DO_BFS_pass1(&slm, thread_primref_id, thread_primref_valid, args);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
///
///        BVH2 construction -- BFS Phase Pass2
///
/////////////////////////////////////////////////////////////////////////////////////////////////

struct BFS_Pass2_SLM
{
    struct BFS_BinInfoReduce3_SLM reduce3;
    //struct AABB3f left_centroid_bounds;
    //struct AABB3f right_centroid_bounds;
    //struct AABB3f left_geom_bounds;
    //struct AABB3f right_geom_bounds;
    LRBounds lr_bounds;
    uint left_count;
    uint right_count;
    uint lr_mask;
    uint left_primref_base;
    uint right_primref_base;
//    uint num_wgs;

//    uint output_indices[BFS_WG_SIZE];
};







void DO_BFS_pass2(
    local struct BFS_Pass2_SLM* slm,
    uint thread_primref_id,
    bool thread_primref_valid,
    struct BFSDispatchArgs args
)
{
    global struct VContext* context = args.context;

    struct AABB3f centroid_bounds = BVH2_GetNodeBox( args.bvh2, args.bvh2_root );

    // load the thread's primref
    PrimRef ref;
    if ( thread_primref_valid )
        ref = args.primref_buffer[thread_primref_id];

    struct BinMapping bin_mapping;
    BinMapping_init( &bin_mapping, &centroid_bounds, BFS_NUM_BINS );

    // initialize working SLM space
    LRBounds_init(&slm->lr_bounds);
    if(get_local_id(0) == 0)
    {
        slm->left_count  = 0;
        slm->right_count = 0;

        if( args.do_mask_processing )
            slm->lr_mask = 0;
    }

    // compute split - every workgroup does the same computation
    // local barrier inside BinInfo_reduce3
    struct BFS_Split split = BinInfo_reduce3( &slm->reduce3, &context->global_bin_info,bin_mapping.scale );

    uint wg_prim_count = args.wg_primref_end - args.wg_primref_begin;

    // partition primrefs into L/R subsets...
    bool go_left = false;
    if (split.sah == (float)(INFINITY))      // no valid split, split in the middle.. This can happen due to floating-point limit cases in huge scenes
        go_left = get_local_id(0) < (wg_prim_count / 2);
    else
        go_left = is_left( &bin_mapping, &split, &ref );

    // assign this primref a position in the output array, and expand corresponding centroid-bounds
    uint local_index;
    {
        float3 centroid = ref.lower.xyz + ref.upper.xyz;

        uint l_ballot = intel_sub_group_ballot(  go_left && thread_primref_valid );
        uint r_ballot = intel_sub_group_ballot( !go_left && thread_primref_valid );
        if (l_ballot)
        {
            bool active_lane = l_ballot & (1 << get_sub_group_local_id());
            float3 Cmin, Cmax, Gmin, Gmax;
            Cmin = select_min( centroid.xyz, active_lane );
            Cmax = select_max( centroid.xyz, active_lane );
            Gmin = select_min( ref.lower.xyz, active_lane );
            Gmax = select_max( ref.upper.xyz, active_lane );

            Cmin = sub_group_reduce_min_float3( Cmin );
            Cmax = sub_group_reduce_max_float3( Cmax );
            Gmin = sub_group_reduce_min_float3( Gmin );
            Gmax = sub_group_reduce_max_float3( Gmax );

            LRBounds_merge_left( &slm->lr_bounds, Cmin,Cmax,Gmin,Gmax );
        }

        if (r_ballot)
        {
            bool active_lane = r_ballot & (1 << get_sub_group_local_id());
            float3 Cmin, Cmax, Gmin, Gmax;
            Cmin = select_min(centroid.xyz, active_lane);
            Cmax = select_max(centroid.xyz, active_lane);
            Gmin = select_min(ref.lower.xyz, active_lane);
            Gmax = select_max(ref.upper.xyz, active_lane);

            Cmin = sub_group_reduce_min_float3(Cmin);
            Cmax = sub_group_reduce_max_float3(Cmax);
            Gmin = sub_group_reduce_min_float3(Gmin);
            Gmax = sub_group_reduce_max_float3(Gmax);

            LRBounds_merge_right( &slm->lr_bounds, Cmin,Cmax,Gmin,Gmax );
        }

        if( args.do_mask_processing )
        {
            uint mask =0;
            if (thread_primref_valid)
            {
                mask = PRIMREF_instanceMask(&ref) ;
                mask = go_left  ? mask : mask<<8;
            }

            // TODO OPT:  there is no 'sub_group_reduce_or'  and IGC does not do the reduction trick
            //   for atomics on sub-group uniform addresses
            for( uint i= get_sub_group_size()/2; i>0; i/= 2)
                mask = mask | intel_sub_group_shuffle_down(mask,mask,i);
            if( get_sub_group_local_id() == 0 )
                atomic_or_local( &slm->lr_mask, mask );
        }

        uint l_base = 0;
        uint r_base = 0;
        if( get_sub_group_local_id() == 0 && l_ballot )
            l_base = atomic_add_local( &slm->left_count, popcount(l_ballot) );
        if( get_sub_group_local_id() == 0 && r_ballot )
            r_base = atomic_add_local( &slm->right_count, popcount(r_ballot) );

        sub_group_barrier( CLK_LOCAL_MEM_FENCE );
        l_base = sub_group_broadcast(l_base,0);
        r_base = sub_group_broadcast(r_base,0);

        l_base = l_base + subgroup_bit_prefix_exclusive( l_ballot );
        r_base = r_base + subgroup_bit_prefix_exclusive( r_ballot );

        local_index = (go_left) ? l_base : r_base;
    }


    barrier( CLK_LOCAL_MEM_FENCE );

    // merge local into global
    // TODO_OPT:  Look at spreading some of this across subgroups
    if ( get_sub_group_id() == 0 )
    {
        // allocate primref space for this wg and merge local/global centroid bounds
        uint num_left  = slm->left_count;
        {
            if (num_left && get_sub_group_local_id() == 0)
            {
                num_left = atomic_add_global( &context->num_left, num_left );
                slm->left_primref_base = args.dispatch_primref_begin + num_left;
            }
        }
        uint num_right = slm->right_count;
        {
            if (num_right && get_sub_group_local_id() == 0)
            {
                num_right = atomic_add_global( &context->num_right, num_right );
                slm->right_primref_base = (args.dispatch_primref_end - 1) - num_right;
            }
        }

        if( args.do_mask_processing && get_sub_group_local_id() == 0 )
            atomic_or_global( &context->lr_mask, slm->lr_mask );
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    LRBounds_merge( &context->lr_bounds, &slm->lr_bounds );

    // move thread's primref ID into correct position in output index buffer
    if (thread_primref_valid)
    {
        uint pos = go_left ? slm->left_primref_base + local_index
            : slm->right_primref_base - local_index;

        args.primref_index_out[pos] = thread_primref_id;
    }
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( BFS_WG_SIZE, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void
BFS_pass2_indexed( global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* sah_globals )
{
    local struct BFS_Pass2_SLM slm;
    struct BFSDispatchArgs args = get_bfs_args_queue( &scheduler->bfs_queue, scheduler, sah_globals );

    bool thread_primref_valid = (args.wg_primref_begin + get_local_id( 0 )) < args.wg_primref_end;
    uint thread_primref_id = 0;
    if ( thread_primref_valid )
        thread_primref_id = args.primref_index_in[args.wg_primref_begin + get_local_id( 0 )];

    DO_BFS_pass2( &slm, thread_primref_id, thread_primref_valid, args );
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( BFS_WG_SIZE, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void
BFS_pass2_initial( global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* sah_globals )
{
    local struct BFS_Pass2_SLM slm;
    struct BFSDispatchArgs args = get_bfs_args_initial( scheduler, sah_globals );

    uint thread_primref_id    = args.wg_primref_begin + get_local_id( 0 );
    bool thread_primref_valid = thread_primref_id < args.wg_primref_end;

    DO_BFS_pass2( &slm, thread_primref_id, thread_primref_valid, args );
}


__attribute__((reqd_work_group_size(BFS_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void
BFS_pass2_indexed_batchable( global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* globals_buffer )
{
    local struct BFS_Pass2_SLM slm;
    struct BFSDispatchArgs args = get_bfs_args_batchable(&scheduler->bfs_queue, scheduler, globals_buffer );

    bool thread_primref_valid = (args.wg_primref_begin + get_local_id(0)) < args.wg_primref_end;
    uint thread_primref_id = 0;
    if (thread_primref_valid)
        thread_primref_id = args.primref_index_in[args.wg_primref_begin + get_local_id(0)];

    DO_BFS_pass2(&slm, thread_primref_id, thread_primref_valid, args);

}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(BFS_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void
BFS_pass2_initial_batchable(global struct VContextScheduler* scheduler, global struct SAHBuildGlobals* globals_buffer)
{
    local struct BFS_Pass2_SLM slm;
    struct BFSDispatchArgs args = get_bfs_args_batchable(&scheduler->bfs_queue, scheduler, globals_buffer );

    uint thread_primref_id = args.wg_primref_begin + get_local_id(0);
    bool thread_primref_valid = thread_primref_id < args.wg_primref_end;

    DO_BFS_pass2(&slm, thread_primref_id, thread_primref_valid, args);
}




/////////////////////////////////////////////////////////////////////////////////////////////////
///
///        BVH2 construction -- DFS Phase
///
/////////////////////////////////////////////////////////////////////////////////////////////////

struct DFSArgs
{
    uint primref_base;
    uint global_bvh2_base;
    bool do_mask_processing;
    ushort num_primrefs;
    global uint* primref_indices_in;
    global uint* primref_indices_out;
    global PrimRef* primref_buffer;
    global struct BVH2* global_bvh2;
};


struct DFSPrimRefAABB
{
    half lower[3];
    half upper[3];
};

void DFSPrimRefAABB_init( struct DFSPrimRefAABB* bb )
{
    bb->lower[0] = 1;
    bb->lower[1] = 1;
    bb->lower[2] = 1;
    bb->upper[0] = 0;
    bb->upper[1] = 0;
    bb->upper[2] = 0;
}

void DFSPrimRefAABB_extend( struct DFSPrimRefAABB* aabb, struct DFSPrimRefAABB* v )
{
    aabb->lower[0] = min( aabb->lower[0], v->lower[0] );
    aabb->lower[1] = min( aabb->lower[1], v->lower[1] );
    aabb->lower[2] = min( aabb->lower[2], v->lower[2] );
    aabb->upper[0] = max( aabb->upper[0], v->upper[0] );
    aabb->upper[1] = max( aabb->upper[1], v->upper[1] );
    aabb->upper[2] = max( aabb->upper[2], v->upper[2] );
}

half DFSPrimRefAABB_halfArea( struct DFSPrimRefAABB* aabb )
{
    const half3 d = (half3)(aabb->upper[0] - aabb->lower[0], aabb->upper[1] - aabb->lower[1], aabb->upper[2] - aabb->lower[2]);
    return fma( d.x, (d.y + d.z), d.y * d.z );
}

struct DFSPrimRef
{
    struct DFSPrimRefAABB aabb;
    ushort2 meta;
};

void DFSPrimRef_SetBVH2Root( struct DFSPrimRef* ref, ushort root )
{
    ref->meta.y = root;
}

uint DFSPrimRef_GetInputIndex( struct DFSPrimRef* ref )
{
    return ref->meta.x;
}

uint DFSPrimRef_GetBVH2Parent( struct DFSPrimRef* ref )
{
    return ref->meta.y;
}


struct PrimRefSet
{
    struct DFSPrimRefAABB AABB[DFS_WG_SIZE];
    ushort2 meta[DFS_WG_SIZE];
    uint input_indices[DFS_WG_SIZE];
};




local struct DFSPrimRefAABB* PrimRefSet_GetAABBPointer( local struct PrimRefSet* refs, ushort id )
{
    return &refs->AABB[id];
}
struct DFSPrimRef PrimRefSet_GetPrimRef( local struct PrimRefSet* refs, ushort id )
{
    struct DFSPrimRef r;
    r.aabb = refs->AABB[id];
    r.meta = refs->meta[id];
    return r;
}
void PrimRefSet_SetPrimRef( local struct PrimRefSet* refs, struct DFSPrimRef ref, ushort id )
{
    refs->AABB[id] = ref.aabb;
    refs->meta[id] = ref.meta;
}

void PrimRefSet_SetPrimRef_FullPrecision( struct AABB3f* root_aabb, local struct PrimRefSet* refs, PrimRef ref, ushort id )
{
    float3 root_l = AABB3f_load_lower( root_aabb );
    float3 root_u = AABB3f_load_upper( root_aabb );
    float3 d = root_u - root_l;
    float scale = 1.0f / max( d.x, max( d.y, d.z ) );

    float3 l = ref.lower.xyz;
    float3 u = ref.upper.xyz;
    half3 lh = convert_half3_rtz( (l - root_l) * scale );
    half3 uh = convert_half3_rtp( (u - root_l) * scale );

    refs->AABB[id].lower[0] = lh.x;
    refs->AABB[id].lower[1] = lh.y;
    refs->AABB[id].lower[2] = lh.z;
    refs->AABB[id].upper[0] = uh.x;
    refs->AABB[id].upper[1] = uh.y;
    refs->AABB[id].upper[2] = uh.z;
    refs->meta[id].x = id;
    refs->meta[id].y = 0;
}



void DFS_CreatePrimRefSet( struct DFSArgs args,
                           local struct PrimRefSet* prim_refs )
{
    ushort id = get_local_id( 0 );
    ushort num_primrefs = args.num_primrefs;

    struct AABB3f box = BVH2_GetNodeBox( args.global_bvh2, args.global_bvh2_base );
    if ( id < num_primrefs )
    {
        PrimRef ref = args.primref_buffer[args.primref_indices_in[id]];
        prim_refs->input_indices[id] = args.primref_indices_in[id];
        PrimRefSet_SetPrimRef_FullPrecision( &box, prim_refs, ref, id );
    }
}

struct ThreadRangeInfo
{
    uchar start;
    uchar local_num_prims;
    uchar bvh2_root;
    bool  active;
};

struct BVHBuildLocals // size:  ~3.8K
{
    uchar2                 axis_and_left_count[ DFS_WG_SIZE ];
    struct ThreadRangeInfo range[ DFS_WG_SIZE ];
    uint                   sah[ DFS_WG_SIZE ];
};

#define LOCAL_BVH2_NODE_COUNT (2*(DFS_WG_SIZE) -1)

struct LocalBVH2
{
    uint nodes[LOCAL_BVH2_NODE_COUNT];
    uint num_nodes;

    // bit layout is for a node is
    //  uchar child_ptr;    // this is right_child_index >> 1.   right child's msb is always 0
    //  uchar primref_base; // index of the node's first primref.  will be 0 at the root
    //  uchar parent_dist;  // distance in nodes from this node to its parent
    //  uchar prim_counter; // number of prims in this subtree.  For a complete tree (256 prims), the root may be off by 1

    // for a WG size of 256, 8b is enough for parent distance, because the tree is built in level order
    //    the maximum distance between parent and child occurs for a complete tree.
    //    in this scenario the left-most leaf has index 255, its parent has index 127, the deltas to the children are 128 and 129
};


void LocalBVH2_Initialize( struct LocalBVH2* bvh2, ushort num_prims )
{
    bvh2->num_nodes = 1;
    bvh2->nodes[0] = min(num_prims,(ushort)255);
}



void LocalBVH2_Initialize_Presplit(struct LocalBVH2* bvh2, ushort num_prims, ushort left_count, ushort right_count )
{
    bvh2->num_nodes = 3;
    bvh2->nodes[0] = min(num_prims, (ushort)255);

    ushort bvh2_root = 0;
    ushort child_place = 1;

    uint child_ptr = (child_place + 1) >> 1;
    bvh2->nodes[bvh2_root] |= (child_ptr) << 24;

    uint parent_dist = child_place - bvh2_root;

    // initialize child nodes
    ushort primref_base_left = 0;
    ushort primref_base_right = left_count;
    uint left = (primref_base_left << 16) + ((parent_dist << 8)) + left_count;
    uint right = (primref_base_right << 16) + ((parent_dist + 1) << 8) + right_count;
    bvh2->nodes[child_place] = left;
    bvh2->nodes[child_place + 1] = right;
}


void LocalBVH2_CreateInnerNode( local struct LocalBVH2* bvh2, ushort bvh2_root, uint primref_base_left, uint primref_base_right )
{
    ushort child_place = atomic_add_local( &(bvh2-> num_nodes), 2 );

    uint child_ptr   = (child_place + 1) >> 1;
    bvh2->nodes[bvh2_root] |= (child_ptr) << 24;

    uint parent_dist = child_place - bvh2_root;

    // initialize child nodes
    uint left  = (primref_base_left << 16)  + ((parent_dist << 8));
    uint right = (primref_base_right << 16) + ((parent_dist + 1) << 8);
    bvh2->nodes[child_place]     = left;
    bvh2->nodes[child_place + 1] = right;
}

ushort2 LocalBVH2_GetChildIndices( struct LocalBVH2* bvh2, ushort bvh2_root )
{
    ushort right_idx = (bvh2->nodes[bvh2_root] & 0xff000000) >> 23;
    return (ushort2)(right_idx - 1, right_idx);
}


ushort LocalBVH2_IncrementPrimCount( local struct LocalBVH2* bvh2, ushort bvh2_root )
{
    // increment only the lower 8 bits.  Algorithm will not overflow by design
    return atomic_inc_local( &bvh2->nodes[bvh2_root] ) & 0xff;
}

ushort LocalBVH2_SetLeafPrimCount(local struct LocalBVH2* bvh2, ushort bvh2_root, ushort count)
{
    return bvh2->nodes[bvh2_root] |= (count& 0xff);
}

bool LocalBVH2_IsRoot( struct LocalBVH2* bvh2, ushort node_id )
{
    return node_id == 0;
}

ushort LocalBVH2_GetLeafPrimrefStart( struct LocalBVH2* bvh2, ushort bvh2_node_id )
{
    return (bvh2->nodes[bvh2_node_id] >> 16) & 255;
}

bool LocalBVH2_IsLeftChild( struct LocalBVH2* bvh2, ushort parent_node, ushort current_node )
{
    return (current_node & 1); // nodes are allocated in pairs.  first node is root, left child is an odd index
}

ushort LocalBVH2_GetParent( struct LocalBVH2* bvh2, ushort node )
{
    return node - ((bvh2->nodes[node] >> 8) & 255);
}

uint LocalBVH2_GetNodeCount( struct LocalBVH2* bvh2 )
{
    return bvh2->num_nodes;
}

bool LocalBVH2_IsLeaf( struct LocalBVH2* bvh2, ushort node_index )
{
    return (bvh2->nodes[node_index] & 255) <= TREE_ARITY;
}

ushort LocalBVH2_GetLeafPrimCount( struct LocalBVH2* bvh2, ushort node_index )
{
    return (bvh2->nodes[node_index] & 255);
}

void DFS_ConstructBVH2( local struct LocalBVH2* bvh2,
                        local struct PrimRefSet* prim_refs,
                        ushort bvh2_root,
                        ushort prim_range_start,
                        ushort local_num_prims,
                        ushort global_num_prims,
                        local struct BVHBuildLocals* locals,
                        local uint* num_active_threads )
{
    ushort tid = get_local_id( 0 );
    ushort primref_position = tid;

    bool active_thread = tid < global_num_prims;

    // Handle cases where initial binner creates leaves
    if ( active_thread && local_num_prims <= TREE_ARITY )
    {
        struct DFSPrimRef ref = PrimRefSet_GetPrimRef(prim_refs, primref_position);
        DFSPrimRef_SetBVH2Root(&ref, bvh2_root);
        PrimRefSet_SetPrimRef(prim_refs, ref, primref_position);
        active_thread = false;
        if (primref_position == prim_range_start)
            atomic_sub_local(num_active_threads, local_num_prims);
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    locals->range[ tid ].start           = prim_range_start;
    locals->range[ tid ].local_num_prims = local_num_prims;
    locals->range[ tid ].bvh2_root       = bvh2_root;
    locals->range[ tid ].active          = active_thread;

    do
    {
        if(active_thread && prim_range_start == primref_position)
            locals->sah[primref_position] = UINT_MAX;

        barrier( CLK_LOCAL_MEM_FENCE );

        if ( active_thread )
        {
            local struct DFSPrimRefAABB* my_box = PrimRefSet_GetAABBPointer( prim_refs, primref_position );

            // each thread evaluates a possible split candidate.  Scan primrefs and compute sah cost
            //  do this axis-by-axis to keep register pressure low
            float best_sah = INFINITY;
            ushort best_axis = 3;
            ushort best_count = 0;

            struct DFSPrimRefAABB box_left[3];
            struct DFSPrimRefAABB box_right[3];
            float CSplit[3];
            ushort count_left[3];

            for ( ushort axis = 0; axis < 3; axis++ )
            {
                DFSPrimRefAABB_init( &box_left[axis] );
                DFSPrimRefAABB_init( &box_right[axis] );

                CSplit[axis] = my_box->lower[axis] + my_box->upper[axis];
                count_left[axis] = 0;
            }

            // scan primrefs in our subtree and partition using this thread's prim as a split plane
            {
                struct DFSPrimRefAABB box = *PrimRefSet_GetAABBPointer( prim_refs, prim_range_start );

                for ( ushort p = 1; p < local_num_prims; p++ )
                {
                        struct DFSPrimRefAABB next_box = *PrimRefSet_GetAABBPointer( prim_refs, prim_range_start + p ); //preloading box for next iteration

                        for( ushort axis = 0; axis < 3; axis++ )
                        {
                            float c = box.lower[axis] + box.upper[axis];

                            if ( c < CSplit[axis] )
                            {
                                // this primitive is to our left.
                                DFSPrimRefAABB_extend( &box_left[axis], &box );
                                count_left[axis]++;
                            }
                            else
                            {
                                // this primitive is to our right
                                DFSPrimRefAABB_extend( &box_right[axis], &box );
                            }
                        }

                        box = next_box;
                }

                // last iteration without preloading box
                for( ushort axis = 0; axis < 3; axis++ )
                {
                    float c = box.lower[axis] + box.upper[axis];

                    if ( c < CSplit[axis] )
                    {
                        // this primitive is to our left.
                        DFSPrimRefAABB_extend( &box_left[axis], &box );
                        count_left[axis]++;
                    }
                    else
                    {
                        // this primitive is to our right
                        DFSPrimRefAABB_extend( &box_right[axis], &box );
                    }
                }

            }

            for ( ushort axis = 0; axis < 3; axis++ )
            {
                float Al = DFSPrimRefAABB_halfArea( &box_left[axis] );
                float Ar = DFSPrimRefAABB_halfArea( &box_right[axis] );

                // Avoid NANs in SAH calculation in the corner case where all prims go right
                //  In this case we set Al=Ar, because such a split will only be selected if all primrefs
                //    are co-incident..  In that case, we will fall back to split-in-the-middle and both subtrees
                //    should store the same quantized area value
                if ( count_left[axis] == 0 )
                    Al = Ar;

                // compute sah cost
                ushort count_right = local_num_prims - count_left[axis];
                float sah = Ar * count_right + Al * count_left[axis];

                // keep this split if it is better than the previous one, or if the previous one was a corner-case
                if ( sah < best_sah || best_count == 0 )
                {
                    // yes, keep it
                    best_axis = axis;
                    best_sah = sah;
                    best_count = count_left[axis];
                }
            }

            // write split information to SLM
            locals->axis_and_left_count[primref_position].x = best_axis;
            locals->axis_and_left_count[primref_position].y = best_count;
            uint sah = as_uint(best_sah);
            // break ties by axis to ensure deterministic split selection
            //  otherwise builder can produce non-deterministic tree structure run to run
            //  based on the ordering of primitives (which can vary due to non-determinism in atomic counters)
            // Embed split axis and index into sah value; compute min over sah and max over axis
            sah = ( ( sah & ~1023 ) | ( 2 - best_axis ) << 8 | tid );

            // reduce on split candidates in our local subtree and decide the best one
            atomic_min_local( &locals->sah[ prim_range_start ], sah);
        }


        barrier( CLK_LOCAL_MEM_FENCE );

        ushort split_index = locals->sah[ prim_range_start ] & 255;
        ushort split_axis = locals->axis_and_left_count[split_index].x;
        ushort split_left_count = locals->axis_and_left_count[split_index].y;

        if ( (primref_position == split_index) && active_thread )
        {
            // first thread in a given subtree creates the inner node
            ushort start_left  = prim_range_start;
            ushort start_right = prim_range_start + split_left_count;
            if ( split_left_count == 0 )
                start_right = start_left + (local_num_prims / 2); // handle split-in-the-middle case

            LocalBVH2_CreateInnerNode( bvh2, bvh2_root, start_left, start_right );
        }


        barrier( CLK_LOCAL_MEM_FENCE );

        struct DFSPrimRef ref;
        ushort new_primref_position;

        if ( active_thread )
        {
            ushort2 kids = LocalBVH2_GetChildIndices( bvh2, bvh2_root );
            bool go_left;

            if ( split_left_count == 0 )
            {
                // We chose a split with no left-side prims
                //  This will only happen if all primrefs are located in the exact same position
                //   In that case, fall back to split-in-the-middle
                split_left_count = (local_num_prims / 2);
                go_left = (primref_position - prim_range_start < split_left_count);
            }
            else
            {
                // determine what side of the split this thread's primref belongs on
                local struct DFSPrimRefAABB* my_box    = PrimRefSet_GetAABBPointer( prim_refs, primref_position );
                local struct DFSPrimRefAABB* split_box = PrimRefSet_GetAABBPointer( prim_refs, split_index );
                float c = my_box->lower[split_axis] + my_box->upper[split_axis];
                float Csplit = split_box->lower[split_axis] + split_box->upper[split_axis];
                go_left = c < Csplit;
            }

            // adjust state variables for next loop iteration
            bvh2_root = (go_left) ? kids.x : kids.y;
            local_num_prims = (go_left) ? split_left_count : (local_num_prims - split_left_count);
            prim_range_start = (go_left) ? prim_range_start : prim_range_start + split_left_count;

            // determine the new primref position by incrementing a counter in the destination subtree
            new_primref_position = prim_range_start + LocalBVH2_IncrementPrimCount( bvh2, bvh2_root );

            // load our primref from its previous position
            ref = PrimRefSet_GetPrimRef( prim_refs, primref_position );
        }

        barrier( CLK_LOCAL_MEM_FENCE );

        if ( active_thread )
        {
            // write our primref into its sorted position and note which node it went in
            DFSPrimRef_SetBVH2Root( &ref, bvh2_root );
            PrimRefSet_SetPrimRef( prim_refs, ref, new_primref_position );
            primref_position = new_primref_position;


            // deactivate all threads whose subtrees are small enough to form a leaf
            if ( local_num_prims <= TREE_ARITY )
            {
                active_thread = false;
                if( primref_position == prim_range_start )
                    atomic_sub_local( num_active_threads, local_num_prims );
            }

            locals->range[ primref_position ].start           = prim_range_start;
            locals->range[ primref_position ].local_num_prims = local_num_prims;
            locals->range[ primref_position ].bvh2_root       = bvh2_root;
            locals->range[ primref_position ].active          = active_thread;
        }

        barrier( CLK_LOCAL_MEM_FENCE );

        // if we'll have next iteration then load from SLM
        if(*num_active_threads)
        {
            prim_range_start = locals->range[ tid ].start;
            local_num_prims  = locals->range[ tid ].local_num_prims;
            bvh2_root        = locals->range[ tid ].bvh2_root;
            active_thread    = locals->range[ tid ].active;
            primref_position = tid;
        }
        else
        {
            break;
        }

    } while ( true );

}


#define REFIT_BIT_DWORDS (LOCAL_BVH2_NODE_COUNT - DFS_WG_SIZE)/32

struct RefitBits
{
    uint bits[REFIT_BIT_DWORDS];
};

struct DFS_SLM
{
    union
    {
        struct LocalBVH2 bvh2;
        struct {
            struct AABB3f centroid_bounds;
            uint left_count;
            uint right_count;
            struct BFS_BinInfo bins;
            struct BFS_BinInfoReduce3_SLM reduce3;
        } binning;

    } u1;

    union
    {
        struct {
            struct PrimRefSet prim_refs;
            struct BVHBuildLocals locals;
        } pass0;

        struct AABB3f node_boxes[LOCAL_BVH2_NODE_COUNT];

    } u2;

    union
    {
        uchar bytes[DFS_WG_SIZE];
        uint dwords[DFS_WG_SIZE/4];
    } mask_info;

    struct RefitBits refit_bits;

};


void DFS_InitialBinningPass(
    local struct BFS_BinInfo* bins,
    local struct BFS_BinInfoReduce3_SLM* reduce3,
    uniform local struct AABB3f* centroid_bounds,
    local struct PrimRefSet* refs,
    local uint* left_counter,
    local uint* right_counter,
    ushort num_refs )
{
    uint tid = get_local_id(0);

    // initialize SLM structures
    if (tid == 0)
    {
        AABB3f_init(centroid_bounds);
        *left_counter = 0;
        *right_counter = 0;
    }

    BinInfo_init(bins);

    PrimRef ref;
    struct DFSPrimRef dfs_ref;

    if (tid < num_refs)
    {
        dfs_ref = PrimRefSet_GetPrimRef(refs, tid);
        struct DFSPrimRefAABB box = dfs_ref.aabb;
        ref.lower.xyz = (float3)(box.lower[0], box.lower[1], box.lower[2]);
        ref.upper.xyz = (float3)(box.upper[0], box.upper[1], box.upper[2]);
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    // compute centroid bounds so that we can bin
    if (tid < num_refs)
    {
        float3 centroid = ref.lower.xyz + ref.upper.xyz;
        Uniform_AABB3f_atomic_merge_local_sub_group_lu(centroid_bounds, centroid, centroid);
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    // add primrefs to bins
    struct BinMapping mapping;
    BinMapping_init(&mapping, centroid_bounds, BFS_NUM_BINS);

    BinInfo_add_primref( &mapping, bins, &ref, tid<num_refs );

    barrier(CLK_LOCAL_MEM_FENCE);

    // compute split - every sub_group computes different bin
    struct BFS_Split split = BinInfo_reduce3(reduce3, bins, mapping.scale);


    bool go_left = false;
    uint local_pos = 0;
    if (tid < num_refs)
    {
        // partition primrefs into L/R subsets...
        if (split.sah == (float)(INFINITY))      // no valid split, split in the middle.. This can happen due to floating-point limit cases in huge scenes
            go_left = tid < (num_refs / 2);
        else
            go_left = is_left(&mapping, &split, &ref);

        if (go_left)
            local_pos = atomic_inc_local(left_counter);
        else
            local_pos = num_refs - (1+ atomic_inc_local(right_counter));

        PrimRefSet_SetPrimRef(refs, dfs_ref, local_pos);
    }

}


void Do_DFS( struct DFSArgs args, local struct DFS_SLM* slm, local uint* num_active_threads )
{
    local struct LocalBVH2* bvh2 = &slm->u1.bvh2;

    global struct BVH2* global_bvh2 = args.global_bvh2;

    PrimRef ref;
    uint parent_node;

    {
        local struct BVHBuildLocals* locals = &slm->u2.pass0.locals;
        local struct PrimRefSet* prim_refs = &slm->u2.pass0.prim_refs;

        DFS_CreatePrimRefSet(args, prim_refs);

        uint local_id = get_local_id(0);

        ushort bvh2_root = 0;
        ushort prim_range_start = 0;
        ushort local_num_prims = args.num_primrefs;

        if(local_id == 0)
            *num_active_threads = local_num_prims;

        // barrier for DFS_CreatePrimRefSet and num_active_threads
        barrier(CLK_LOCAL_MEM_FENCE);

        // initial binning pass if number of primrefs is large
        if( args.num_primrefs > 32 )
        {
            DFS_InitialBinningPass(&slm->u1.binning.bins, &slm->u1.binning.reduce3, &slm->u1.binning.centroid_bounds, prim_refs,
                &slm->u1.binning.left_count, &slm->u1.binning.right_count, args.num_primrefs);

            barrier(CLK_LOCAL_MEM_FENCE);

            ushort left_count = slm->u1.binning.left_count;
            ushort right_count = args.num_primrefs - left_count;
            if (get_local_id(0) == 0)
                LocalBVH2_Initialize_Presplit(bvh2, args.num_primrefs, left_count, right_count);

            bvh2_root        = (local_id < left_count) ? 1 : 2;
            local_num_prims = (local_id < left_count) ? left_count : right_count;
            prim_range_start = (local_id < left_count) ? 0 : left_count;
        }
        else
        {
            if (get_local_id(0) == 0)
                LocalBVH2_Initialize(bvh2, args.num_primrefs);
        }

        DFS_ConstructBVH2( bvh2, prim_refs, bvh2_root, prim_range_start, local_num_prims, args.num_primrefs, locals, num_active_threads);

        // move the prim refs into their sorted position
        //  keep this thread's primref around for later use
        if ( local_id < args.num_primrefs )
        {
            struct DFSPrimRef dfs_ref = PrimRefSet_GetPrimRef( prim_refs, local_id );

            uint input_id = DFSPrimRef_GetInputIndex( &dfs_ref );

            parent_node = DFSPrimRef_GetBVH2Parent( &dfs_ref );

            uint primref_index = prim_refs->input_indices[input_id];
            ref = args.primref_buffer[primref_index];
            args.primref_indices_out[local_id] = primref_index;
            args.primref_indices_in[local_id] = primref_index;
            // these buffers are not read again until the end of kernel
        }

        barrier( CLK_LOCAL_MEM_FENCE );

    }


    // initialize flags for determining when subtrees are done refit
    if ( get_local_id( 0 ) < REFIT_BIT_DWORDS )
        slm->refit_bits.bits[get_local_id( 0 )] = 0;


    // stash full-precision primref AABBs in slm storage
    local struct AABB3f* slm_boxes = &slm->u2.node_boxes[0];
    bool active_thread = get_local_id( 0 ) < args.num_primrefs;
    if( active_thread )
    {
        AABB3f_set( &slm_boxes[get_local_id( 0 )], ref.lower.xyz, ref.upper.xyz );

        // stash instance masks in SLM storage
        if( args.do_mask_processing )
            slm->mask_info.bytes[get_local_id(0)] = PRIMREF_instanceMask( &ref );
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    // Refit leaf nodes
    uint box_index;
    if ( active_thread )
    {
        // the thread for the first primref in every leaf is the one that will ascend
        // remaining threads merge their AABB/mask into the first one and terminate
        uint first_ref = LocalBVH2_GetLeafPrimrefStart( bvh2, parent_node );
        if ( first_ref != get_local_id( 0 ) )
        {
            AABB3f_atomic_merge_local_lu( &slm_boxes[first_ref], ref.lower.xyz, ref.upper.xyz );

            if( args.do_mask_processing )
            {
                uint dword_index = first_ref/4;
                uint shift       = (first_ref%4)*8;
                uint mask = PRIMREF_instanceMask(&ref) << shift;
                atomic_or_local( &slm->mask_info.dwords[dword_index], mask );
            }
            active_thread = false; // switch off all primref threads except the first one
        }

        box_index = first_ref;
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if ( active_thread )
    {
        uint current_node = parent_node;
        parent_node = LocalBVH2_GetParent( bvh2, current_node );

        // write out the leaf node's AABB
        uint num_prims = LocalBVH2_GetLeafPrimCount( bvh2, current_node );
        uint prim_offs = args.primref_base + LocalBVH2_GetLeafPrimrefStart( bvh2, current_node );

        uint mask = 0xff;
        if( args.do_mask_processing )
            mask = slm->mask_info.bytes[box_index];

        BVH2_WriteLeafNode( global_bvh2, args.global_bvh2_base + current_node, &slm_boxes[box_index], prim_offs, num_prims, mask );

        // we no longer need the BVH2 bits for this node, so re-purpose the memory to store the AABB index
        bvh2->nodes[current_node] = box_index;

        // toggle flag bit in parent node.  The second thread to flip the bit is the one that gets to proceed
        uint thread_mask = (1 << (parent_node % 32));
        if ( (atomic_xor_local( &slm->refit_bits.bits[parent_node / 32], thread_mask ) & thread_mask) == 0 )
            active_thread = false;
    }

    // count how many active threads in sub_group we have and increment wg's number of active threads
    uint sg_active = sub_group_reduce_add(active_thread ? 1 : 0);
    if(get_sub_group_local_id() == 0)
    {
        atomic_add_local(num_active_threads, sg_active);
    }

    // refit internal nodes:
    // walk up the tree and refit AABBs

    do
    {
        barrier( CLK_LOCAL_MEM_FENCE ); // we need this barrier because we need to make sure all threads read num_active_threads before modifying it
        if ( active_thread )
        {
            uint current_node = parent_node;
            parent_node = LocalBVH2_GetParent( bvh2, current_node );

            // pull left/right box indices from current node
            ushort2 kids = LocalBVH2_GetChildIndices( bvh2, current_node );

            uint left_box = bvh2->nodes[kids.x];
            uint right_box = bvh2->nodes[kids.y];

            struct AABB3f left = slm_boxes[left_box];
            struct AABB3f right = slm_boxes[right_box];
            AABB3f_extend( &left, &right );

            uint2 child_offsets = (uint2)(
                args.global_bvh2_base + kids.x,
                args.global_bvh2_base + kids.y);

            uint mask = 0xff;
            if( args.do_mask_processing )
            {
                mask = slm->mask_info.bytes[left_box]
                     | slm->mask_info.bytes[right_box];
                slm->mask_info.bytes[left_box] = mask;
            }

            BVH2_WriteInnerNode( args.global_bvh2, args.global_bvh2_base+current_node, &left, child_offsets, mask );

            slm_boxes[left_box] = left;
            bvh2->nodes[current_node] = left_box;

            // stop at the root
            if ( LocalBVH2_IsRoot( bvh2, current_node ) )
            {
                active_thread = false;
                atomic_dec_local(num_active_threads);
            }
            else
            {
                // toggle flag bit in parent node.  The second thread to flip the bit is the one that gets to proceed
                uint mask = (1 << (parent_node % 32));
                if ( (atomic_xor_local( &slm->refit_bits.bits[parent_node / 32], mask ) & mask) == 0 )
                {
                    active_thread = false;
                    atomic_dec_local(num_active_threads);
                }
            }
        }

        barrier( CLK_LOCAL_MEM_FENCE );
    } while ( *num_active_threads > 0 );
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size(DFS_WG_SIZE,1,1) ))
__attribute__( (intel_reqd_sub_group_size(16)) )
kernel void
DFS( global struct VContextScheduler* scheduler,
     global struct SAHBuildGlobals* globals_buffer )
{
    local struct DFS_SLM slm;
    local struct DFSDispatchRecord record;
    local uint num_active_threads;

    if ( get_local_id( 0 ) == 0  )
    {
        // pop an entry off the DFS dispatch queue
        //uint wg_index = atomic_dec_global( &scheduler->num_dfs_wgs ) - 1;
        //record = scheduler->dfs_queue.records[wg_index];

        // TODO:  The version above races, but is considerably faster... investigate
        uint wg_index = get_group_id(0);
        record = scheduler->dfs_queue.records[wg_index];
        write_mem_fence( CLK_LOCAL_MEM_FENCE );
        atomic_dec_global( &scheduler->num_dfs_wgs );
    }

    barrier( CLK_LOCAL_MEM_FENCE );


    bool odd_pass = record.tree_depth & 1;

    global struct SAHBuildGlobals* sah_globals = globals_buffer + record.batch_index;

    struct DFSArgs args;
    args.num_primrefs = record.num_primrefs;
    args.primref_indices_in   = SAHBuildGlobals_GetPrimrefIndices_In( sah_globals, odd_pass );
    args.primref_indices_out  = SAHBuildGlobals_GetPrimrefIndices_Out( sah_globals, odd_pass );
    args.primref_buffer       = SAHBuildGlobals_GetPrimrefs( sah_globals );
    args.global_bvh2          = SAHBuildGlobals_GetBVH2( sah_globals );
    args.primref_indices_in  += record.primref_base;
    args.primref_indices_out += record.primref_base;
    args.primref_base         = record.primref_base;
    args.global_bvh2_base     = record.bvh2_base;
    args.do_mask_processing   = SAHBuildGlobals_NeedMasks( sah_globals );

    Do_DFS( args, &slm, &num_active_threads );

}


/////////////////////////////////////////////////////////////////////////////////////////////////
///
///        BVH2 to BVH6
///
/////////////////////////////////////////////////////////////////////////////////////////////////



struct BuildFlatTreeArgs
{
    ushort leaf_size_in_bytes;
    ushort leaf_type;
    ushort inner_node_type;
    bool do_mask_processing;

    global uint* primref_indices;
    global PrimRef* primref_buffer;
    global struct Globals* globals;
    global struct BVHBase* bvh_base;
    global struct BVH2* bvh2;
};


// lane i in the return value is the index of the ith largest primref in the input
// the return value can be used with shuffle() to move data into its sorted position
//  the elements of 'key' must be unique.. only the first 6 elements are sorted
varying ushort SUBGROUP_get_sort_indices_N6( varying uint key )
{
    // each lane computes the number of items larger than it
    // this is its position in the descending order
    //   TODO_OPT:  Compiler can vectorize these uint16 adds by packing into lower and upper halves of same GPR.... make sure it does it
    //     if compiler is not generating optimal code, consider moving to Cm

    varying ushort cmp0 = (sub_group_broadcast(key, 0) > key) ? 1 : 0;
    varying ushort cmp1 = (sub_group_broadcast(key, 1) > key) ? 1 : 0;
    varying ushort cmp2 = (sub_group_broadcast(key, 2) > key) ? 1 : 0;
    varying ushort cmp3 = (sub_group_broadcast(key, 3) > key) ? 1 : 0;
    varying ushort cmp4 = (sub_group_broadcast(key, 4) > key) ? 1 : 0;
    varying ushort cmp5 = (sub_group_broadcast(key, 5) > key) ? 1 : 0;
    varying ushort a = cmp0 + cmp2 + cmp4;
    varying ushort b = cmp1 + cmp3 + cmp5;
    varying ushort num_larger = a + b;

    // each lane determines which of the input elements it should pull
    varying ushort lane = get_sub_group_local_id();
    a  = (sub_group_broadcast(num_larger, 0) == lane) ? 0 : 0;
    b  = (sub_group_broadcast(num_larger, 1) == lane) ? 1 : 0;
    a += (sub_group_broadcast(num_larger, 2) == lane) ? 2 : 0;
    b += (sub_group_broadcast(num_larger, 3) == lane) ? 3 : 0;
    a += (sub_group_broadcast(num_larger, 4) == lane) ? 4 : 0;
    b += (sub_group_broadcast(num_larger, 5) == lane) ? 5 : 0;
    return a + b;
}

uint SUBGROUP_area_to_sort_key( varying float area, uniform ushort num_children )
{
    varying ushort lane = get_sub_group_local_id();
    area = (lane < num_children) ? area : 0;        // put inactive nodes last

    // drop LSBs and break ties by lane number to ensure unique keys
    // use descending lane IDs to ensure that sort is stable if the upper MSBs are equal.
    //     If we do not do this it can lead to non-deterministic tree structure
    return (as_uint(area) & 0xffffff80) + (lane^(get_sub_group_size()-1));
}

// lane i in the return value is the index of the ith largest primref in the input
// the return value can be used with shuffle() to move data into its sorted position
//  the elements of 'key' must be unique.. only the first 6 elements are sorted
varying ushort SUBGROUP_get_sort_indices_N6_2xSIMD8_in_SIMD16( varying uint key )
{
    // each lane computes the number of items larger than it
    // this is its position in the descending order
    //   TODO_OPT:  Compiler can vectorize these uint16 adds by packing into lower and upper halves of same GPR.... make sure it does it
    //     if compiler is not generating optimal code, consider moving to Cm

    varying ushort cmp0 = (sub_group_broadcast(key, 0) > key) ? 1 : 0;
    varying ushort cmp1 = (sub_group_broadcast(key, 1) > key) ? 1 : 0;
    varying ushort cmp2 = (sub_group_broadcast(key, 2) > key) ? 1 : 0;
    varying ushort cmp3 = (sub_group_broadcast(key, 3) > key) ? 1 : 0;
    varying ushort cmp4 = (sub_group_broadcast(key, 4) > key) ? 1 : 0;
    varying ushort cmp5 = (sub_group_broadcast(key, 5) > key) ? 1 : 0;
    varying ushort a = cmp0 + cmp2 + cmp4;
    varying ushort b = cmp1 + cmp3 + cmp5;
    varying ushort num_larger = a + b;

    varying ushort cmp0_1 = (sub_group_broadcast(key, 8) > key) ? 1 : 0;
    varying ushort cmp1_1 = (sub_group_broadcast(key, 9) > key) ? 1 : 0;
    varying ushort cmp2_1 = (sub_group_broadcast(key, 10) > key) ? 1 : 0;
    varying ushort cmp3_1 = (sub_group_broadcast(key, 11) > key) ? 1 : 0;
    varying ushort cmp4_1 = (sub_group_broadcast(key, 12) > key) ? 1 : 0;
    varying ushort cmp5_1 = (sub_group_broadcast(key, 13) > key) ? 1 : 0;
    varying ushort a_1 = cmp0_1 + cmp2_1 + cmp4_1;
    varying ushort b_1 = cmp1_1 + cmp3_1 + cmp5_1;
    varying ushort num_larger_1 = a_1 + b_1;

    // each lane determines which of the input elements it should pull
    varying ushort lane = get_sub_group_local_id();
    if(lane < 8)
    {
        a  = (sub_group_broadcast(num_larger, 0) == lane) ? 0 : 0;
        b  = (sub_group_broadcast(num_larger, 1) == lane) ? 1 : 0;
        a += (sub_group_broadcast(num_larger, 2) == lane) ? 2 : 0;
        b += (sub_group_broadcast(num_larger, 3) == lane) ? 3 : 0;
        a += (sub_group_broadcast(num_larger, 4) == lane) ? 4 : 0;
        b += (sub_group_broadcast(num_larger, 5) == lane) ? 5 : 0;
    }
    else
    {
        a  = (sub_group_broadcast(num_larger_1, 8)  == lane-8) ? 8 : 8;
        b  = (sub_group_broadcast(num_larger_1, 9)  == lane-8) ? 1 : 0;
        a += (sub_group_broadcast(num_larger_1, 10) == lane-8) ? 2 : 0;
        b += (sub_group_broadcast(num_larger_1, 11) == lane-8) ? 3 : 0;
        a += (sub_group_broadcast(num_larger_1, 12) == lane-8) ? 4 : 0;
        b += (sub_group_broadcast(num_larger_1, 13) == lane-8) ? 5 : 0;
    }

    return a + b;
}

uint SUBGROUP_area_to_sort_key_2xSIMD8_in_SIMD16( varying float area, uniform ushort num_children )
{
    varying ushort lane = get_sub_group_local_id() % 8;
    area = (lane < num_children) ? area : 0;        // put inactive nodes last

    // drop LSBs and break ties by lane number to ensure unique keys
    // use descending lane IDs to ensure that sort is stable if the upper MSBs are equal.
    //     If we do not do this it can lead to non-deterministic tree structure
    return (as_uint(area) & 0xffffff80) + (lane^7);
}

ushort SUBGROUP_BuildFlatTreeNode(
    uniform struct BuildFlatTreeArgs args,
    uniform uint bvh2_root,
    uniform struct InternalNode* qnode,
    uniform uint qnode_index,
    varying uint3* sg_children_out // if an inner node is created, receives the indices of the 6 child nodes (X), and the QNode position (y), and num_children(z)
                                   //  if a leaf is created, receives number of primrefs (z)
) // return value is the number of child nodes or 0 for a leaf
{
    global struct BVH2* bvh2 = args.bvh2;
    varying ushort lane = get_sub_group_local_id();

    global struct BVHBase* base = args.bvh_base;


    if ( !BVH2_IsInnerNode( bvh2, bvh2_root ) )
    {
        uniform ushort num_prims   = BVH2_GetLeafPrimCount( bvh2, bvh2_root );
        uniform uint primref_start = BVH2_GetLeafPrimStart( bvh2, bvh2_root );
        varying uint primref_index = primref_start + ((lane < num_prims) ? lane : 0);

        varying uint ref_id = args.primref_indices[primref_index];
        varying PrimRef ref = args.primref_buffer[ref_id];
        uniform char* leaf_mem_base = (char*)BVHBase_GetQuadLeaves( args.bvh_base );
        uniform char* leaf_mem = leaf_mem_base + primref_start * args.leaf_size_in_bytes;

        uniform int offset = (int)(leaf_mem - (char*)qnode);
        offset = offset >> 6;

        varying uint key = SUBGROUP_area_to_sort_key(AABB_halfArea(&ref), num_prims );
        varying ushort sort_index = SUBGROUP_get_sort_indices_N6(key);
        ref = PrimRef_sub_group_shuffle(&ref, sort_index);
        ref_id = intel_sub_group_shuffle(ref_id, sort_index);

        if (lane < num_prims)
            args.primref_indices[primref_index] = ref_id;

        uint global_num_prims = args.globals->numPrimitives;
        char* bvh_mem = (char*) args.bvh_base;

        if(lane < num_prims)
            args.primref_indices[primref_index + global_num_prims] = qnode - (struct InternalNode*)bvh_mem;

        if (args.leaf_type == NODE_TYPE_INSTANCE)
            subgroup_setInstanceQBVHNodeN( offset, &ref, num_prims, (struct QBVHNodeN*)qnode, lane < num_prims ? PRIMREF_instanceMask(&ref) : 0 );
        else
            subgroup_setQBVHNodeN( offset, args.leaf_type, &ref, num_prims, (struct QBVHNodeN*)qnode, BVH_NODE_DEFAULT_MASK );

        sg_children_out->z = num_prims;
        return 0;
    }
    else
    {
        // collapse BVH2 into BVH6.
        // We will spread the root node's children across the subgroup, and keep adding SIMD lanes until we have enough
        uniform ushort num_children = 2;

        uniform uint2 kids = BVH2_GetChildIndices( bvh2, bvh2_root );
        varying uint sg_bvh2_node = kids.x;
        if ( lane == 1 )
            sg_bvh2_node = kids.y;

        do
        {
            // choose the inner node with maximum area to replace.
            // Its left child goes in its old location.  Its right child goes in a new lane

            // TODO_OPT:  We re-read the AABBs again and again to compute area
            //   ... store per-lane boxes instead and pre-compute areas

            varying float sg_area = BVH2_GetNodeArea( bvh2, sg_bvh2_node );
            varying bool sg_is_inner = BVH2_IsInnerNode( bvh2, sg_bvh2_node );
            sg_area = (sg_is_inner && lane < num_children) ? sg_area : 0; // prevent early exit if the largest child is a leaf

            uniform float max_area = sub_group_reduce_max_N6( sg_area );
            varying bool sg_reducable = max_area == sg_area && (lane < num_children) && sg_is_inner;
            uniform uint mask = intel_sub_group_ballot( sg_reducable );

            // TODO_OPT:  Some of these ops seem redundant.. look at trimming further

            if ( mask == 0 )
                break;

            // choose the inner node with maximum area to replace
            uniform ushort victim_child = ctz( mask );
            uniform uint victim_node = sub_group_broadcast( sg_bvh2_node, victim_child );
            kids = BVH2_GetChildIndices( bvh2, victim_node );

            if ( lane == victim_child )
                sg_bvh2_node = kids.x;
            else if ( lane == num_children )
                sg_bvh2_node = kids.y;

            num_children++;

        } while ( num_children < TREE_ARITY );

        // allocate inner node space
        uniform uint kids_offset;
        if (get_sub_group_local_id() == 0)
            kids_offset = allocate_inner_nodes( args.bvh_base, num_children );
        kids_offset = sub_group_broadcast(kids_offset, 0);

        uniform struct QBVHNodeN* kid = (((struct QBVHNodeN*)args.bvh_base) + kids_offset);
        uniform int offset = (int)((char*)kid - (char*)qnode) >> 6;

#if 0
        uniform uint kids_offset;
        if ( get_sub_group_local_id() == 0 )
            kids_offset = alloc_node_mem( args.globals, sizeof( struct QBVHNodeN ) * num_children );
        kids_offset = sub_group_broadcast( kids_offset, 0 );


        // create inner node
        uniform struct QBVHNodeN* kid = (struct QBVHNodeN*) ((char*)(args.bvh_base) + kids_offset);
        uniform int offset = (int)((char*)kid - (char*)qnode) >> 6;
#endif
        uniform uint child_type = args.inner_node_type;

        // sort child nodes in descending order by AABB area
        varying struct AABB box   = BVH2_GetAABB( bvh2, sg_bvh2_node );
        varying uint key          = SUBGROUP_area_to_sort_key(AABB_halfArea(&box), num_children );
        varying ushort sort_index = SUBGROUP_get_sort_indices_N6(key);
        box          = AABB_sub_group_shuffle(&box, sort_index);
        sg_bvh2_node = intel_sub_group_shuffle(sg_bvh2_node, sort_index);

        uniform uint node_mask = (args.do_mask_processing) ? BVH2_GetMask( bvh2, bvh2_root ) : 0xff;

        subgroup_setQBVHNodeN( offset, child_type, &box, num_children, (struct QBVHNodeN*)qnode, node_mask );

        // return child information
        *sg_children_out = (uint3)(sg_bvh2_node, qnode_index + offset + get_sub_group_local_id(), num_children );
        return num_children;
    }
}

ushort SUBGROUP_BuildFlatTreeNode_2xSIMD8_in_SIMD16(
    uniform struct BuildFlatTreeArgs args,
    varying uint bvh2_root,
    varying struct InternalNode* qnode_base,
    varying uint qnode_index,
    varying uint3* sg_children_out, // if an inner node is created, receives the indices of the 6 child nodes (X), and the QNode position (y), and num_children(z)
                                   //  if a leaf is created, receives number of primrefs (z)
    bool active_lane
) // return value is the number of child nodes or 0 for a leaf
{
    global struct BVH2* bvh2 = args.bvh2;
    varying ushort SIMD16_lane = get_sub_group_local_id();
    varying ushort SIMD8_lane = get_sub_group_local_id() % 8;
    varying ushort SIMD8_id = get_sub_group_local_id() / 8;
    varying ushort lane = get_sub_group_local_id();
    global struct BVHBase* base = args.bvh_base;

    struct BVH2NodeMetaData nodeMetaData = BVH2_GetNodeMetaData( bvh2, bvh2_root );

    bool is_leaf = active_lane && !BVH2NodeMetaData_IsInnerNode( &nodeMetaData );
    bool is_inner = active_lane && BVH2NodeMetaData_IsInnerNode( &nodeMetaData );

    uchar mask = BVH_NODE_DEFAULT_MASK;
    if(is_inner)
        mask = (args.do_mask_processing) ? BVH2NodeMetaData_GetMask( &nodeMetaData ) : 0xff;

    int offset;

    varying struct InternalNode* qnode = qnode_base + qnode_index;
    // TOOD: we don't need unions, I left them only for readability
    union {
        uint num_prims;
        uint num_children;
    } lane_num_data;

    union {
        PrimRef ref; // this is in fact AABB
        struct AABB box;
    } lane_box_data;

    union {
        uint ref_id;
        uint sg_bvh2_node;
    } lane_id_data;

    // for leafs
    varying uint primref_index;

    if(is_leaf)
    {
        lane_num_data.num_prims   = BVH2NodeMetaData_GetLeafPrimCount( &nodeMetaData );
        uint primref_start = BVH2NodeMetaData_GetLeafPrimStart( &nodeMetaData );
        primref_index = primref_start + ((SIMD8_lane < lane_num_data.num_prims) ? SIMD8_lane : 0);

        lane_id_data.ref_id = args.primref_indices[primref_index];
        lane_box_data.ref = args.primref_buffer[lane_id_data.ref_id];
        char* leaf_mem_base = (char*)BVHBase_GetQuadLeaves( args.bvh_base );
        char* leaf_mem = leaf_mem_base + primref_start * args.leaf_size_in_bytes;

        offset = (int)(leaf_mem - (char*)qnode);
        offset = offset >> 6;
    }


    if(intel_sub_group_ballot(is_inner))
    {
        // collapse BVH2 into BVH6.
        // We will spread the root node's children across the subgroup, and keep adding SIMD lanes until we have enough

        uint2 kids;
        if(is_inner)
        {
            lane_num_data.num_children = 2;
            kids = BVH2_GetChildIndices( bvh2, bvh2_root );

            lane_id_data.sg_bvh2_node = kids.x;
            if ( SIMD8_lane == 1 )
                lane_id_data.sg_bvh2_node = kids.y;
        }

        bool active = is_inner;
        do
        {
            // choose the inner node with maximum area to replace.
            // Its left child goes in its old location.  Its right child goes in a new lane

            // TODO_OPT:  We re-read the AABBs again and again to compute area
            //   ... store per-lane boxes instead and pre-compute areas

            varying float sg_area = 0;
            varying bool sg_is_inner = false;
            if(active)
            {
                sg_area = BVH2_GetNodeArea( bvh2, lane_id_data.sg_bvh2_node );
                sg_is_inner = BVH2_IsInnerNode( bvh2, lane_id_data.sg_bvh2_node );
                sg_area = (sg_is_inner && SIMD8_lane < lane_num_data.num_children) ? sg_area : 0; // prevent early exit if the largest child is a leaf
            }

            float max_area = sub_group_reduce_max_N6_2xSIMD8_in_SIMD16( sg_area );
            varying bool sg_reducable = max_area == sg_area && sg_is_inner && (SIMD8_lane < lane_num_data.num_children);
            uint mask = intel_sub_group_ballot( sg_reducable ) & (0xFF << SIMD8_id * 8); // we'll end up with two different masks for two SIMD8 in SIMD16 due to bits masking

            // TODO_OPT:  Some of these ops seem redundant.. look at trimming further

            if ( mask == 0 )
                active = false;

            // choose the inner node with maximum area to replace
            ushort victim_child = ctz( mask );
            uint victim_node = intel_sub_group_shuffle( lane_id_data.sg_bvh2_node, victim_child );
            if(active)
            {
                kids = BVH2_GetChildIndices( bvh2, victim_node );

                if ( SIMD16_lane == victim_child ) // we use SIMD16_lane, cause victim_child was calculated based on SIMD16 i.e. second node will have victim from 8..13
                    lane_id_data.sg_bvh2_node = kids.x;
                else if ( SIMD8_lane == lane_num_data.num_children )
                    lane_id_data.sg_bvh2_node = kids.y;

                lane_num_data.num_children++;

                if(lane_num_data.num_children >= TREE_ARITY)
                    active = false;
            }

        } while ( intel_sub_group_ballot(active) ); // if any active, then continue

        // sum children from both halfs of SIMD16 to allocate nodes only once per sub_group
        uniform ushort num_children = is_inner ? lane_num_data.num_children : 0;
        uniform ushort first_SIMD8_num_children = sub_group_broadcast(num_children, 0);
        uniform ushort second_SIMD8_num_children = sub_group_broadcast(num_children, 8);

        num_children = first_SIMD8_num_children + second_SIMD8_num_children;
        uint kids_offset;

        // allocate inner node space
        if(num_children && SIMD16_lane == 0)
            kids_offset = allocate_inner_nodes( args.bvh_base, num_children );
        kids_offset = sub_group_broadcast(kids_offset, 0);
        if((is_inner))
        {
            kids_offset += SIMD8_id * first_SIMD8_num_children;

            struct QBVHNodeN* kid = (((struct QBVHNodeN*)args.bvh_base) + kids_offset);

            offset = (int)((char*)kid - (char*)qnode) >> 6;
            lane_box_data.box = BVH2_GetAABB( bvh2, lane_id_data.sg_bvh2_node );
        }
    }

    // sort child nodes in descending order by AABB area
    varying uint key          = SUBGROUP_area_to_sort_key_2xSIMD8_in_SIMD16(AABB_halfArea(&lane_box_data.box), lane_num_data.num_children );
    varying ushort sort_index = SUBGROUP_get_sort_indices_N6_2xSIMD8_in_SIMD16(key);
    lane_box_data.box         = PrimRef_sub_group_shuffle(&lane_box_data.box, sort_index);
    lane_id_data.sg_bvh2_node = intel_sub_group_shuffle(lane_id_data.sg_bvh2_node, sort_index);

    char* bvh_mem = (char*) args.bvh_base;
    if (is_leaf && SIMD8_lane < lane_num_data.num_prims)
    {
        args.primref_indices[primref_index] = lane_id_data.ref_id;
        args.primref_indices[primref_index + args.globals->numPrimitives] = qnode - (struct InternalNode*)bvh_mem;
    }

    bool degenerated = false;
    uint node_type = is_leaf ? args.leaf_type : args.inner_node_type;

    if(args.leaf_type == NODE_TYPE_INSTANCE)
        degenerated = subgroup_setInstanceBox_2xSIMD8_in_SIMD16(&lane_box_data.box, lane_num_data.num_children, &mask, SIMD8_lane < lane_num_data.num_prims ? PRIMREF_instanceMask(&lane_box_data.ref) : 0, is_leaf);

    subgroup_setQBVHNodeN_setFields_2xSIMD8_in_SIMD16(offset, node_type, &lane_box_data.box, lane_num_data.num_children, mask, (struct QBVHNodeN*)(qnode), degenerated, active_lane);

    // return child information
    if(is_inner)
    {
        sg_children_out->x = lane_id_data.sg_bvh2_node;
        sg_children_out->y = qnode_index + offset + SIMD8_lane;
    }

    sg_children_out->z = lane_num_data.num_children;

    return is_inner ? lane_num_data.num_children : 0;
}

void check_primref_integrity( global struct SAHBuildGlobals* globals )
{
    global uint* primref_in = SAHBuildGlobals_GetPrimrefIndices_In( globals, 0 );
    global uint* primref_out = SAHBuildGlobals_GetPrimrefIndices_Out( globals, 0 );
    dword num_primrefs = SAHBuildGlobals_GetTotalPrimRefs( globals );
    if ( get_local_id( 0 ) == 0 )
    {
        for ( uint i = 0; i < num_primrefs; i++ )
        {
            primref_out[i] = 0;
        }

        for ( uint i = 0; i < num_primrefs; i++ )
            primref_out[primref_in[i]]++;

        for ( uint i = 0; i < num_primrefs; i++ )
            if ( primref_out[i] != 1 )
                printf( "Foo: %u   %u\n", i, primref_out[i] );
    }
}




void check_bvh2(global struct SAHBuildGlobals* globals )
{
    global struct BVH2* bvh2 = SAHBuildGlobals_GetBVH2(globals);
    global uint* primref_in = SAHBuildGlobals_GetPrimrefIndices_In(globals, 0);
    global uint* primref_out = SAHBuildGlobals_GetPrimrefIndices_Out(globals, 0);
    dword num_primrefs = SAHBuildGlobals_GetTotalPrimRefs(globals);

    if (get_local_id(0) == 0)
    {
        for (uint i = 0; i < num_primrefs; i++)
            primref_out[i] = 0;

        uint stack[256];
        uint sp=0;
        uint r = BVH2_GetRoot(bvh2);
        stack[sp++] = r;
        while (sp)
        {
            r = stack[--sp];
            if (BVH2_IsInnerNode(bvh2,r))
            {
                uint2 kids = BVH2_GetChildIndices( bvh2, r);
                if (kids.x >= bvh2->num_nodes || kids.y >= bvh2->num_nodes)
                {
                    printf("BVH2!! Bad node index found!\n");
                    return;
                }

                stack[sp++] = kids.x;
                stack[sp++] = kids.y;
            }
            else
            {
                uint ref = BVH2_GetLeafPrimStart(bvh2,r);
                uint count = BVH2_GetLeafPrimCount(bvh2,r);
                if( count == 0 )
                {
                    printf("BVH2!! Empty leaf found!\n");
                    return;
                }
                for (uint i = 0; i < count; i++)
                {
                    if (ref + i > num_primrefs)
                    {
                        printf("BVH2!! Bad leaf range!\n");
                        return;
                    }
                    uint c = primref_out[ref+i];
                    if (c != 0)
                    {
                        printf("BVH2!! overlapped prim ranges\n");
                        return;
                    }
                    primref_out[ref+i] = 1;
                    if (primref_in[ref + i] >= num_primrefs)
                    {
                        printf("BAD PRIMREF ID FOUND!\n");
                        return;
                    }
                }
            }
        }
    }

    printf("bvh2 is ok!\n");
}


#if 0
// TODO_OPT:  Enable larger WGs.  WGSize 512 at SIMD8 hangs on Gen9, but Gen12 can go bigger
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size(256,1,1)) )
__attribute__( (intel_reqd_sub_group_size(8) ) )
kernel void
build_qnodes( global struct SAHBuildGlobals* globals, global struct VContextScheduler* scheduler )
{
    globals = globals + (scheduler->num_trivial_builds + scheduler->num_single_builds);
    globals = globals + get_group_id(0);


    struct BuildFlatTreeArgs args;
    args.leaf_size_in_bytes = SAHBuildGlobals_GetLeafSizeInBytes( globals );
    args.leaf_type          = SAHBuildGlobals_GetLeafType( globals );
    args.inner_node_type    = SAHBuildGlobals_GetInternalNodeType( globals );
    args.primref_indices    = SAHBuildGlobals_GetPrimrefIndices_In( globals, 0 );
    args.primref_buffer     = SAHBuildGlobals_GetPrimrefs( globals );
    args.bvh_base           = SAHBuildGlobals_GetBVHBase( globals );
    args.bvh2               = SAHBuildGlobals_GetBVH2( globals );
    args.globals            = (global struct Globals*) globals->p_globals;
    args.do_mask_processing = SAHBuildGlobals_NeedMasks( globals );

    dword alloc_backpointers = SAHBuildGlobals_NeedBackPointers( globals );
    global uint2* root_buffer = (global uint2*) globals->p_qnode_root_buffer;
    global struct InternalNode* qnodes = (global struct InternalNode*) BVHBase_GetInternalNodes( args.bvh_base );
    global uint* back_pointers = (global uint*) BVHBase_GetBackPointers( args.bvh_base );

    local uint nodes_produced;
    if ( get_sub_group_id() == 0 )
    {
        // allocate first node
        if (get_sub_group_local_id() == 0)
            allocate_inner_nodes( args.bvh_base, 1 );

        // first subgroup does first node
        varying uint3 children_info;
        uniform ushort num_children = SUBGROUP_BuildFlatTreeNode(args, BVH2_GetRoot(args.bvh2), qnodes, 0, &children_info );

        if ( get_sub_group_local_id() < num_children )
            root_buffer[get_sub_group_local_id()] = children_info.xy;

        if ( alloc_backpointers )
        {
            // set root's backpointer
            if( get_sub_group_local_id() == 0 )
                back_pointers[0] = (0xffffffc0) | (children_info.z << 3);

            // point child backpointers at the parent
            if( get_sub_group_local_id() < num_children )
                back_pointers[children_info.y] = 0;
        }

        if ( get_sub_group_local_id() == 0 )
            nodes_produced = num_children;
    }

    barrier( CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE );


    uniform uint buffer_index = get_sub_group_id();
    uniform bool sg_active    = buffer_index < nodes_produced;

    while ( work_group_any( sg_active ) )
    {
        if( sg_active )
        {
            uniform uint bvh2_node    = root_buffer[buffer_index].x;
            uniform uint qnode_index  = root_buffer[buffer_index].y;

            // build a node
            varying uint3 children_info;
            uniform ushort num_children = SUBGROUP_BuildFlatTreeNode( args, bvh2_node, qnodes + qnode_index, qnode_index, &children_info );

            // handle backpointers
            if ( alloc_backpointers )
            {
                // update this node's backpointer with child count
                if ( get_sub_group_local_id() == 0 )
                    back_pointers[qnode_index] |= (children_info.z << 3);

                // point child backpointers at parent
                if ( get_sub_group_local_id() < num_children )
                    back_pointers[children_info.y] = (qnode_index << 6);
            }

            if ( num_children )
            {
                // allocate space in the child buffer
                uint root_buffer_position = 0;
                if ( get_sub_group_local_id() == 0 )
                    root_buffer_position = atomic_add_local( &nodes_produced, num_children );
                root_buffer_position = sub_group_broadcast( root_buffer_position, 0 );

                // store child indices in root buffer
                if ( get_sub_group_local_id() < num_children )
                    root_buffer[root_buffer_position + get_sub_group_local_id()] = children_info.xy;
            }
        }

        // sync everyone
        work_group_barrier( CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE,
                            memory_scope_work_group );


        if( sg_active )
            buffer_index += get_num_sub_groups();

        sg_active = (buffer_index < nodes_produced);
    }
}
#endif







inline bool buffer_may_overflow( uint capacity, uint current_size, uint elements_processed_per_sub_group )
{
    uint num_consumed = min( get_num_sub_groups() * elements_processed_per_sub_group, current_size );
    uint space_available = (capacity - current_size) + num_consumed;
    uint space_needed = TREE_ARITY * num_consumed;
    return space_available < space_needed;
}

inline uint build_qnodes_pc(
    global struct SAHBuildGlobals* globals,
    bool alloc_backpointers,
    bool process_masks,
    uint first_qnode,
    uint first_bvh2_node,

    local uint2* SLM_local_root_buffer,
    local uint* SLM_ring_tail,
    const uint  RING_SIZE
)

{
    struct BuildFlatTreeArgs args;
    args.leaf_size_in_bytes = SAHBuildGlobals_GetLeafSizeInBytes( globals );
    args.leaf_type = SAHBuildGlobals_GetLeafType( globals );
    args.inner_node_type = SAHBuildGlobals_GetInternalNodeType( globals );
    args.primref_indices = SAHBuildGlobals_GetPrimrefIndices_In( globals, 0 );
    args.primref_buffer = SAHBuildGlobals_GetPrimrefs( globals );
    args.bvh_base = SAHBuildGlobals_GetBVHBase( globals );
    args.bvh2 = SAHBuildGlobals_GetBVH2( globals );
    args.globals = (global struct Globals*) globals->p_globals;
    args.do_mask_processing = process_masks;

    global struct InternalNode* qnodes = (global struct InternalNode*) BVHBase_GetInternalNodes( args.bvh_base );
    global uint* back_pointers = (global uint*) BVHBase_GetBackPointers( args.bvh_base );

    // first subgroup adds first node
    if ( get_sub_group_id() == 0 && get_sub_group_local_id() == 0)
    {
        SLM_local_root_buffer[0].x = first_bvh2_node;
        SLM_local_root_buffer[0].y = first_qnode;
        *SLM_ring_tail = 1;

    }

    uint ring_head = 0;
    uint ring_tail = 1;
    uint ring_size = 1;

    barrier( CLK_LOCAL_MEM_FENCE );

    const uniform uint elements_processed_in_sg = 2;

    while ( ring_size > 0 && !buffer_may_overflow( RING_SIZE, ring_size, elements_processed_in_sg ) )
    {
        ushort SIMD16_lane = get_sub_group_local_id();

        // SIMD16 as 2xSIMD8
        ushort SIMD8_lane = get_sub_group_local_id() % 8;
        ushort SIMD8_id = get_sub_group_local_id() / 8;
        bool active_lane;

        uniform uint nodes_consumed = min( get_num_sub_groups() * elements_processed_in_sg, ring_size ); // times two because we process two nodes in subgroup
        uniform bool sg_active = get_sub_group_id() * elements_processed_in_sg < nodes_consumed;
        ushort num_children = 0;
        varying uint3 children_info = 0;

        uint bvh2_node = 0;
        uint qnode_index = 0;

        if (sg_active)
        {
            ushort consumed_pos = get_sub_group_id() * elements_processed_in_sg + SIMD8_id;
            active_lane = consumed_pos < nodes_consumed ? true : false;
            consumed_pos = consumed_pos < nodes_consumed ? consumed_pos : consumed_pos-1;

            uint buffer_index = (ring_head + consumed_pos) % RING_SIZE;

            bvh2_node = SLM_local_root_buffer[buffer_index].x;
            qnode_index = SLM_local_root_buffer[buffer_index].y;
        }

        barrier( CLK_LOCAL_MEM_FENCE );

        if (sg_active)
        {
            // build a node
            num_children = SUBGROUP_BuildFlatTreeNode_2xSIMD8_in_SIMD16(args, bvh2_node, qnodes, qnode_index, &children_info, active_lane);

            // handle backpointers
            // TODO_OPT:  This should be separate shaders not a runtime branch
            //     doing it this way for now because GRLTLK does not make dynamic shader selection on host very easy.
            //     this needs to change... GRLTLK should

            if (alloc_backpointers && active_lane)
            {
                // update this node's backpointer with child count
                if (SIMD8_lane == 0)
                    back_pointers[qnode_index] |= (children_info.z << 3);

                // point child backpointers at parent
                if (SIMD8_lane < num_children)
                    back_pointers[children_info.y] = (qnode_index << 6);
            }

            // save data

            uniform ushort first_SIMD8_num_children  = sub_group_broadcast(num_children, 0);
            uniform ushort second_SIMD8_num_children = sub_group_broadcast(num_children, 8);
            uniform ushort SIMD16_num_children = first_SIMD8_num_children + second_SIMD8_num_children;

            uint root_buffer_position = 0;

            // allocate space in the child buffer
            if (SIMD16_lane == 0 && SIMD16_num_children)
                root_buffer_position = atomic_add_local(SLM_ring_tail, SIMD16_num_children);

            root_buffer_position = sub_group_broadcast( root_buffer_position, 0 );
            root_buffer_position += SIMD8_id * first_SIMD8_num_children; // update offset for second half of SIMD16

            // store child indices in root buffer
            if (SIMD8_lane < num_children)
            {
                uint store_pos = (root_buffer_position + SIMD8_lane) % RING_SIZE;
                SLM_local_root_buffer[store_pos] = children_info.xy;
            }
        }

        // sync everyone
        barrier( CLK_LOCAL_MEM_FENCE );

        ring_head += nodes_consumed;
        ring_tail = *SLM_ring_tail;
        ring_size = ring_tail - ring_head;
    }

    return ring_head;
}




inline void amplify_and_spill(
    global struct SAHBuildGlobals* globals,
    dword alloc_backpointers,
    uint first_qnode,
    uint first_bvh2_node,
    global uint2* global_root_buffer,
    local uint* root_buffer_counter,
    const uint  RING_SIZE
)

{
    struct BuildFlatTreeArgs args;
    args.leaf_size_in_bytes = SAHBuildGlobals_GetLeafSizeInBytes(globals);
    args.leaf_type = SAHBuildGlobals_GetLeafType(globals);
    args.inner_node_type = SAHBuildGlobals_GetInternalNodeType(globals);
    args.primref_indices = SAHBuildGlobals_GetPrimrefIndices_In(globals, 0);
    args.primref_buffer = SAHBuildGlobals_GetPrimrefs(globals);
    args.bvh_base = SAHBuildGlobals_GetBVHBase(globals);
    args.bvh2 = SAHBuildGlobals_GetBVH2(globals);
    args.globals = (global struct Globals*) globals->p_globals;

    global struct InternalNode* qnodes = (global struct InternalNode*) BVHBase_GetInternalNodes(args.bvh_base);
    global uint* back_pointers = (global uint*) BVHBase_GetBackPointers(args.bvh_base);


    varying uint3 children_info;
    uniform ushort num_children = SUBGROUP_BuildFlatTreeNode(args, first_bvh2_node, qnodes + first_qnode, first_qnode, &children_info);

    if (alloc_backpointers)
    {
        // set first node's backpointer
        if (get_sub_group_local_id() == 0)
        {
            // if first node is root, use root sentinel in backpointer
            //   otherwise, need to merge the child count in with the parent offset (which was already put there by the parent's thread)
            uint bp = 0xffffffc0;
            if (first_qnode != 0)
                bp = back_pointers[first_qnode];
            bp |= (children_info.z << 3);

            back_pointers[first_qnode] = bp;
        }

        // point child backpointers at the parent
        if (get_sub_group_local_id() < num_children)
            back_pointers[children_info.y] = (first_qnode << 6);
    }

    if (num_children)
    {
        uint spill_pos = 0;
        if (get_sub_group_local_id() == 0)
            spill_pos = atomic_add_local(root_buffer_counter,num_children);

        spill_pos = sub_group_broadcast(spill_pos, 0);

        if (get_sub_group_local_id() < num_children)
            global_root_buffer[spill_pos+get_sub_group_local_id()] = children_info.xy;
    }

}




inline void build_qnodes_pc_kickoff_func(
    global struct SAHBuildGlobals* globals,
    global uint2* root_buffer,
    bool alloc_backpointers,
    bool process_masks,

    local uint2* SLM_local_root_buffer,
    local uint* SLM_spill_pos,
    local uint* SLM_ring_tail,
    int RING_SIZE
)
{
    // allocate first node
    if ( get_sub_group_id() == 0 && get_sub_group_local_id() == 0 )
        allocate_inner_nodes( SAHBuildGlobals_GetBVHBase(globals), 1 );

    *SLM_spill_pos=0;

    uint ring_head = build_qnodes_pc( globals, alloc_backpointers, process_masks,
                     0, BVH2_GetRoot(SAHBuildGlobals_GetBVH2(globals)), SLM_local_root_buffer, SLM_ring_tail, RING_SIZE );


    uint n = *SLM_ring_tail - ring_head;
    if (n > 0)
    {
#if 0
        // do an additional round of amplification so we can get more nodes into the root buffer and go wider in the next phase
        /// JDB TODO: this is causing hangs on DG2 for metro, so disabling for now...
        for (uint i = get_sub_group_id(); i < n; i+= get_num_sub_groups() )
        {
            uint consume_pos = (ring_head + i) % RING_SIZE;
            uniform uint bvh2_root = SLM_local_root_buffer[consume_pos].x;
            uniform uint qnode_root = SLM_local_root_buffer[consume_pos].y;

            amplify_and_spill( globals, alloc_backpointers, qnode_root, bvh2_root, root_buffer, SLM_spill_pos, RING_SIZE );
        }

        barrier( CLK_LOCAL_MEM_FENCE );
#else
        for (uint i = get_local_id(0); i < n; i += get_local_size(0))
            root_buffer[i] = SLM_local_root_buffer[(ring_head + i) % RING_SIZE];
#endif

        if (get_local_id(0) == 0)
        {
            globals->root_buffer_num_produced = n;
            globals->root_buffer_num_produced_hi = 0;
            globals->root_buffer_num_consumed = 0;
            globals->root_buffer_num_consumed_hi = 0;
        }
    }
}




GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( 256, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void
build_qnodes_pc_kickoff(
    global struct SAHBuildGlobals* globals,
    global uint2* root_buffer,
    dword sah_flags
)
{
    bool alloc_backpointers = sah_flags & SAH_FLAG_NEED_BACKPOINTERS;
    bool process_masks = sah_flags & SAH_FLAG_NEED_MASKS;


    const int RING_SIZE = 64;

    local uint2 SLM_local_root_buffer[RING_SIZE];
    local uint SLM_spill_pos;
    local uint SLM_ring_tail;

    build_qnodes_pc_kickoff_func(globals,
                                 root_buffer,
                                 alloc_backpointers,
                                 process_masks,
                                 SLM_local_root_buffer,
                                 &SLM_spill_pos,
                                 &SLM_ring_tail,
                                 RING_SIZE
                                 );
}




inline void build_qnodes_pc_amplify_func(
    global struct SAHBuildGlobals* globals,
    global uint2* root_buffer,
    bool alloc_backpointers,
    bool process_masks,

    local uint2* SLM_local_root_buffer,
    local uint*  SLM_broadcast,
    local uint*  SLM_ring_tail,
    int RING_SIZE
    )
{
    // TODO_OPT:  Probably don't need this atomic.. could clear 'num_consumed' every time
    //     and just use get_group_id()
    //

    if (get_local_id(0) == 0)
        *SLM_broadcast = atomic_inc_global(&globals->root_buffer_num_consumed);

    barrier( CLK_LOCAL_MEM_FENCE );

    uniform uint consume_pos = *SLM_broadcast;
    uniform uint bvh2_root = root_buffer[consume_pos].x;
    uniform uint qnode_root = root_buffer[consume_pos].y;

    uint ring_head = build_qnodes_pc(globals, alloc_backpointers,process_masks,
        qnode_root, bvh2_root, SLM_local_root_buffer, SLM_ring_tail, RING_SIZE);

    // TODO_OPT:  Instead of spilling the nodes, do one more round of amplification and write
    //   generated children directly into the root buffer.  This should allow faster amplification

    // spill root buffer contents
    uint n = *SLM_ring_tail - ring_head;
    if (n > 0)
    {

        if (get_local_id(0) == 0)
            *SLM_broadcast = atomic_add_global(&globals->root_buffer_num_produced, n);

        barrier( CLK_LOCAL_MEM_FENCE );
        uint produce_pos = *SLM_broadcast;

        for (uint i = get_local_id(0); i < n; i += get_local_size(0))
            root_buffer[produce_pos + i] = SLM_local_root_buffer[(ring_head + i) % RING_SIZE];
    }
}





// Process two nodes per wg during amplification phase.
// DOing it this way ensures maximum parallelism
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void
build_qnodes_pc_amplify(
    global struct SAHBuildGlobals* globals,
    global uint2* root_buffer,
    dword sah_flags )
{
    bool alloc_backpointers = sah_flags & SAH_FLAG_NEED_BACKPOINTERS;

    struct BuildFlatTreeArgs args;
    args.leaf_size_in_bytes = SAHBuildGlobals_GetLeafSizeInBytes(globals);
    args.leaf_type = SAHBuildGlobals_GetLeafType(globals);
    args.inner_node_type = SAHBuildGlobals_GetInternalNodeType(globals);
    args.primref_indices = SAHBuildGlobals_GetPrimrefIndices_In(globals, 0);
    args.primref_buffer = SAHBuildGlobals_GetPrimrefs(globals);
    args.bvh_base = SAHBuildGlobals_GetBVHBase(globals);
    args.bvh2 = SAHBuildGlobals_GetBVH2(globals);
    args.globals = (global struct Globals*) globals->p_globals;
    args.do_mask_processing = sah_flags & SAH_FLAG_NEED_MASKS;

    global struct InternalNode* qnodes = (global struct InternalNode*) BVHBase_GetInternalNodes(args.bvh_base);
    global uint* back_pointers = (global uint*) BVHBase_GetBackPointers(args.bvh_base);

    ushort SIMD16_lane = get_sub_group_local_id();

    // SIMD16 as 2xSIMD8
    ushort SIMD8_lane = get_sub_group_local_id() % 8;
    ushort SIMD8_id = get_sub_group_local_id() / 8;
    bool active_lane = false;

    uint consume_pos;
    consume_pos = globals->root_buffer_num_consumed + get_group_id(0) * 2; // times 2 because we process two nodes in workgroup
    consume_pos += SIMD8_id;

    active_lane = consume_pos < globals->root_buffer_num_to_consume ? true : false;
    consume_pos = consume_pos < globals->root_buffer_num_to_consume ? consume_pos : consume_pos-1;

    uint first_bvh2_node = root_buffer[consume_pos].x;
    uint first_qnode = root_buffer[consume_pos].y;

    varying uint3 children_info;
    ushort num_children = SUBGROUP_BuildFlatTreeNode_2xSIMD8_in_SIMD16(args, first_bvh2_node, qnodes, first_qnode, &children_info, active_lane);

    if (alloc_backpointers && active_lane)
    {
        // set first node's backpointer
        if (SIMD8_lane == 0)
        {
            // if first node is root, use root sentinel in backpointer
            //   otherwise, need to merge the child count in with the parent offset (which was already put there by the parent's thread)
            uint bp = 0xffffffc0;
            if (first_qnode != 0)
                bp = back_pointers[first_qnode];
            bp |= (children_info.z << 3);

            back_pointers[first_qnode] = bp;
        }

        // point child backpointers at the parent
        if (SIMD8_lane < num_children)
            back_pointers[children_info.y] = (first_qnode << 6);
    }

    // save data
    {
        // sum children from both halfs of SIMD16 to do only one atomic per sub_group
        uint produce_pos;
        uniform ushort first_SIMD8_num_children = sub_group_broadcast(num_children, 0);
        uniform ushort second_SIMD8_num_children = sub_group_broadcast(num_children, 8);
        uniform ushort SIMD16_num_children = first_SIMD8_num_children + second_SIMD8_num_children;

        if (SIMD16_lane == 0 && SIMD16_num_children)
            produce_pos = atomic_add_global(&globals->root_buffer_num_produced, SIMD16_num_children);

        produce_pos = sub_group_broadcast(produce_pos, 0);
        produce_pos += SIMD8_id * first_SIMD8_num_children; // update offset for second half of SIMD16

        if (SIMD8_lane < num_children)
        {
            root_buffer[produce_pos + SIMD8_lane] = children_info.xy;
        }
    }
}


//////////
//
// Batched version of qnode creation
//
//////////




GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1)))
kernel void
build_qnodes_init_scheduler_batched(global struct QnodeScheduler* scheduler, dword num_builds, dword num_max_qnode_global_root_buffer_entries)
{

    scheduler->batched_build_offset = scheduler->num_trivial_builds + scheduler->num_single_builds;
    scheduler->batched_build_count = num_builds - scheduler->batched_build_offset;
    scheduler->num_max_qnode_global_root_buffer_entries = num_max_qnode_global_root_buffer_entries;

    const uint num_builds_to_process = scheduler->batched_build_count;
    const uint max_qnode_grb_entries = scheduler->num_max_qnode_global_root_buffer_entries;

    scheduler->batched_builds_to_process = num_builds_to_process;
    scheduler->num_qnode_grb_curr_entries = (num_builds_to_process + 15) / 16; // here we store number of workgroups for "build_qnodes_begin_batchable" kernel
    scheduler->num_qnode_grb_new_entries = num_builds_to_process;
    scheduler->qnode_global_root_buffer.curr_entries_offset = max_qnode_grb_entries;
}




GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void
build_qnodes_begin_batchable(global struct QnodeScheduler* scheduler,
                             global struct SAHBuildGlobals* builds_globals)
{
    const uint tid = get_group_id(0) * get_local_size(0) + get_local_id(0);

    const uint num_builds_to_process = scheduler->batched_builds_to_process;

    if(tid < num_builds_to_process)
    {
        const uint build_idx = scheduler->batched_build_offset + tid;

        uint bvh2_node = BVH2_GetRoot(SAHBuildGlobals_GetBVH2(&builds_globals[build_idx]));
        uint qnode = 0;
        struct QNodeGlobalRootBufferEntry entry = { bvh2_node, qnode, build_idx, 1};
        scheduler->qnode_global_root_buffer.entries[tid] = entry;

        builds_globals[build_idx].root_buffer_num_produced = 0;
        builds_globals[build_idx].root_buffer_num_produced_hi = 0;
        builds_globals[build_idx].root_buffer_num_consumed = 0;
        builds_globals[build_idx].root_buffer_num_consumed_hi = 0;

        // allocate first node for this build
        //allocate_inner_nodes( SAHBuildGlobals_GetBVHBase(&builds_globals[build_idx]), 1 );
        SAHBuildGlobals_GetBVHBase(&builds_globals[build_idx])->nodeDataCur++;
    }
}




GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( 1, 1, 1 )) )
kernel void
build_qnodes_scheduler(global struct QnodeScheduler* scheduler)
{
    const uint max_qnode_grb_entries = scheduler->num_max_qnode_global_root_buffer_entries;

    uint new_entries = min(scheduler->num_qnode_grb_new_entries, max_qnode_grb_entries);

    scheduler->num_qnode_grb_curr_entries = new_entries;
    scheduler->num_qnode_grb_new_entries = 0;
    scheduler->qnode_global_root_buffer.curr_entries_offset = scheduler->qnode_global_root_buffer.curr_entries_offset ? 0 : max_qnode_grb_entries;
}




// TODO_OPT:  Enable larger WGs.  WGSize 512 at SIMD8 hangs on Gen9, but Gen12 can go bigger
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( 32, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void
build_qnodes_pc_amplify_batched(
    global struct SAHBuildGlobals* builds_globals,
    global struct QnodeScheduler* scheduler
    )
{
    const uint group_id = get_group_id(0);

    global struct QNodeGlobalRootBuffer* global_root_buffer = &scheduler->qnode_global_root_buffer;
    const uint curr_entries_offset = global_root_buffer->curr_entries_offset;
    struct QNodeGlobalRootBufferEntry entry = global_root_buffer->entries[curr_entries_offset + group_id];

    const uint build_id = entry.build_idx;

    global struct SAHBuildGlobals* globals = &builds_globals[build_id];
    global uint2* root_buffer = (global uint2*)globals->p_qnode_root_buffer;
    bool alloc_backpointers = SAHBuildGlobals_NeedBackPointers(globals);
    bool process_masks = SAHBuildGlobals_NeedMasks(globals);

    const int RING_SIZE = 32; // for 2 SGs, 16 should result in 2 rounds:  one SG produces 6, then 2 SGs consume 2 and produce 12
                              // for 4 SGs, 32 results in 2 rounds:  one SG produces 6, 4 SGs consume 4 and produce 24, resulting in 26

    local uint2 SLM_local_root_buffer[RING_SIZE];
    local uint  SLM_broadcast;
    local uint  SLM_ring_tail;
    local uint  SLM_grb_broadcast;


    //// This below can be moved to separate function if needed for TLAS ////

    uniform uint bvh2_root = entry.bvh2_node;
    uniform uint qnode_root = entry.qnode;

    uint ring_head = build_qnodes_pc(globals, alloc_backpointers, process_masks,
        qnode_root, bvh2_root, SLM_local_root_buffer, &SLM_ring_tail, RING_SIZE);

    // spill root buffer contents
    uint n = SLM_ring_tail - ring_head;
    if (n > 0)
    {
        const uint max_qnode_grb_entries = scheduler->num_max_qnode_global_root_buffer_entries;

        if (get_local_id(0) == 0)
        {
            SLM_grb_broadcast = atomic_add_global(&scheduler->num_qnode_grb_new_entries, n);

            if(SLM_grb_broadcast >= max_qnode_grb_entries) // if global_root_buffer is full, then make space in build's root_buffer
                SLM_broadcast = atomic_add_global(&globals->root_buffer_num_produced, n);
            else if( (SLM_grb_broadcast + n) >= max_qnode_grb_entries) // if we exceed global_root_buffer with our entries, then make space in build's root_buffer
                SLM_broadcast = atomic_add_global(&globals->root_buffer_num_produced, n - (max_qnode_grb_entries - SLM_grb_broadcast));
        }

        barrier( CLK_LOCAL_MEM_FENCE );

        uint produce_pos = SLM_broadcast;

        uint grb_produce_num = n; // grb stands for global_root_buffer
        uint lrb_produce_num = 0; // lrb stands for local root buffer, meaning this build's root_buffer

        if(SLM_grb_broadcast >= max_qnode_grb_entries) // if global_root_buffer is full, don't write to it
        {
            grb_produce_num = 0;
            lrb_produce_num = n;
        }
        else if( (SLM_grb_broadcast + n) >= max_qnode_grb_entries) // if we exceed global_root_buffer with our entries, then decrease amount of entries and store rest in build's root buffer
        {
            grb_produce_num = max_qnode_grb_entries - SLM_grb_broadcast;
            lrb_produce_num = n - grb_produce_num;
        }

        // save data to global_root_buffer
        for(uint i = get_local_id(0); i < grb_produce_num; i += get_local_size(0))
        {
            const uint2 slm_record = SLM_local_root_buffer[(ring_head + i) % RING_SIZE];

            struct QNodeGlobalRootBufferEntry new_entry;
            new_entry.bvh2_node = slm_record.x;
            new_entry.qnode = slm_record.y;
            new_entry.build_idx = entry.build_idx;

            const uint new_entries_offset = curr_entries_offset ? 0 : max_qnode_grb_entries;
            global_root_buffer->entries[new_entries_offset + SLM_grb_broadcast + i] = new_entry;
        }

        // if anything left, write to build's root buffer
        for (uint i = get_local_id(0); i < lrb_produce_num; i += get_local_size(0))
            root_buffer[produce_pos + i] = SLM_local_root_buffer[(ring_head + i + grb_produce_num) % RING_SIZE];
    }
}




GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( 16, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void
build_qnodes_try_to_fill_grb_batched(
    global struct SAHBuildGlobals* builds_globals,
    global struct QnodeScheduler* scheduler
    )
{
    const uint build_id = scheduler->batched_build_offset + get_group_id(0);
    global struct SAHBuildGlobals* globals = &builds_globals[build_id];
    global uint2* root_buffer = (global uint2*)globals->p_qnode_root_buffer;

    global struct QNodeGlobalRootBuffer* qnode_root_buffer = (global struct QNodeGlobalRootBuffer*)&scheduler->qnode_global_root_buffer;

    const uint num_produced = globals->root_buffer_num_produced;
    const uint num_consumed = globals->root_buffer_num_consumed;
    const uint entries =  num_produced - num_consumed; // entries to build's root buffer

    if(!entries)
        return;

    uint global_root_buffer_offset;
    if(get_local_id(0) == 0)
        global_root_buffer_offset = atomic_add_global(&scheduler->num_qnode_grb_new_entries, entries);

    global_root_buffer_offset = sub_group_broadcast(global_root_buffer_offset, 0);

    const uint max_qnode_grb_entries = scheduler->num_max_qnode_global_root_buffer_entries;

    if(global_root_buffer_offset >= max_qnode_grb_entries) // if global_root_buffer is full, then return
        return;

    uint global_root_buffer_produce_num = entries;
    if(global_root_buffer_offset + entries >= max_qnode_grb_entries) // if we exceed global_root_buffer with our entries, then reduce number of entries to push
        global_root_buffer_produce_num = max_qnode_grb_entries - global_root_buffer_offset;

    for(uint i = get_local_id(0); i < global_root_buffer_produce_num; i += get_local_size(0))
    {
        const uint2 entry = root_buffer[num_consumed + i];

        struct QNodeGlobalRootBufferEntry new_entry;
        new_entry.bvh2_node = entry.x;
        new_entry.qnode = entry.y;
        new_entry.build_idx = build_id;

        const uint new_entries_offset = qnode_root_buffer->curr_entries_offset ? 0 : max_qnode_grb_entries;
        qnode_root_buffer->entries[new_entries_offset + global_root_buffer_offset + i] = new_entry;
    }

    if(get_local_id(0) == 0)
        globals->root_buffer_num_consumed += global_root_buffer_produce_num;
}
