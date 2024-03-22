//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "intrinsics.h"
#include "AABB3f.h"
#include "AABB.h"
#include "GRLGen12.h"
#include "quad.h"
#include "common.h"
#include "instance.h"

#include "api_interface.h"

#include "binned_sah_shared.h"


#if 0
#define LOOP_TRIPWIRE_INIT uint _loop_trip=0;

#define LOOP_TRIPWIRE_INCREMENT(max_iterations) \
    _loop_trip++;\
    if ( _loop_trip > max_iterations  )\
    {\
        if( get_local_id(0) == 0 )\
            printf( "@@@@@@@@@@@@@@@@@@@@ TRIPWIRE!!!!!!!!!!! group=%u\n", get_group_id(0) );\
        break;\
    }
#else

#define LOOP_TRIPWIRE_INIT 
#define LOOP_TRIPWIRE_INCREMENT(max_iterations)

#endif


// =========================================================
//             DFS
// =========================================================

// there are 128 threads x SIMD16 == 2048 lanes in a DSS
//   There is 128KB of SLM.  Upper limit of 64KB per WG, so target is 2 groups of 1024 lanes @ 64K each
//     --> Full occupancy requires using less than 64B per lane
//
//   Groups of 256 lanes gives us 16KB per group
//

// We use subgroups very heavily here in order to avoid 
//    use of per-thread scratch space for intermediate values

#define DFS_WG_SIZE 256
#define DFS_NUM_SUBGROUPS 16
#define DFS_BVH2_NODE_COUNT (2*(DFS_WG_SIZE)-1) 
#define TREE_ARITY 6

// FlatTree node limits:
// these are the derivations if we always collapse to one primitive and pack nodes as tightly as possible
//   If BVH2 construction is allowed to terminate early and place multiple prims in a leaf, these numbers will be too low
#if 0  
   
// maximum flattree size is the number of inner nodes in a full M-ary tree with one leaf per primitive
//  This is given by I = (L-1)/(M-1)
//  For a 256 thread workgroup, L=256, M=6, this gives: 51
#define DFS_MAX_FLATTREE_NODES 51


// A flattree leaf is a node which contains only primitives.  
//
//  The maximum number of leaves is related to the number of nodes as:
//   L(N) = ((M-1)*N + 1) / M
//
#define DFS_MAX_FLATTREE_LEAFS 43  // = 43 for 256 thread WG (L=256, M=6)

#else

//  This is the result of estimate_qbvh6_nodes(256)

#define DFS_MAX_FLATTREE_LEAFS 256 
#define DFS_MAX_FLATTREE_NODES 307 // 256 fat-leaves + 51 inner nodes.  51 = ceil(256/5)
#define DFS_MAX_FLATTREE_DEPTH 52  // number of inner nodes in the worst-case tree

#endif

#define uniform
#define varying


struct DFSArgs
{
    global struct BVHBase* bvh_base;
    global PrimRef* primref_buffer;
    ushort leaf_node_type;
    ushort inner_node_type;
    ushort leaf_size_in_bytes;
    bool need_backpointers;
    bool need_masks;
    ushort num_primrefs;
    global uint* primref_index_buffer;
};


struct DFSPrimRefAABB
{
    half lower[3];
    half upper[3];
};

GRL_INLINE void DFSPrimRefAABB_init( struct DFSPrimRefAABB* bb )
{
    bb->lower[0] = 1;
    bb->lower[1] = 1;
    bb->lower[2] = 1;
    bb->upper[0] = 0;
    bb->upper[1] = 0;
    bb->upper[2] = 0;
}

GRL_INLINE void DFSPrimRefAABB_extend( struct DFSPrimRefAABB* aabb, struct DFSPrimRefAABB* v )
{
    aabb->lower[0] = min( aabb->lower[0], v->lower[0] );
    aabb->lower[1] = min( aabb->lower[1], v->lower[1] );
    aabb->lower[2] = min( aabb->lower[2], v->lower[2] );
    aabb->upper[0] = max( aabb->upper[0], v->upper[0] );
    aabb->upper[1] = max( aabb->upper[1], v->upper[1] );
    aabb->upper[2] = max( aabb->upper[2], v->upper[2] );
}

GRL_INLINE float DFSPrimRefAABB_halfArea( struct DFSPrimRefAABB* aabb )
{
    const half3 d = (half3)(aabb->upper[0] - aabb->lower[0], aabb->upper[1] - aabb->lower[1], aabb->upper[2] - aabb->lower[2]);
    return fma( d.x, (d.y + d.z), d.y * d.z );
}

GRL_INLINE struct DFSPrimRefAABB DFSPrimRefAABB_sub_group_reduce( struct DFSPrimRefAABB* aabb )
{
    struct DFSPrimRefAABB bounds;
    bounds.lower[0] = sub_group_reduce_min( aabb->lower[0] );
    bounds.lower[1] = sub_group_reduce_min( aabb->lower[1] );
    bounds.lower[2] = sub_group_reduce_min( aabb->lower[2] );
    bounds.upper[0] = sub_group_reduce_max( aabb->upper[0] );
    bounds.upper[1] = sub_group_reduce_max( aabb->upper[1] );
    bounds.upper[2] = sub_group_reduce_max( aabb->upper[2] );
    return bounds;
}

struct DFSPrimRef
{
    struct DFSPrimRefAABB aabb;
    uint2 meta;
};

struct PrimRefMeta
{
    uchar2 meta;
};

GRL_INLINE uint PrimRefMeta_GetInputIndex( struct PrimRefMeta* it )
{
    return it->meta.x;
}
GRL_INLINE uint PrimRefMeta_GetInstanceMask( struct PrimRefMeta* it )
{
    return it->meta.y;
}


struct PrimRefSet
{
    struct AABB3f root_aabb;
    struct DFSPrimRefAABB AABB[DFS_WG_SIZE];
    uint2 meta[DFS_WG_SIZE];

};

GRL_INLINE local struct DFSPrimRefAABB* PrimRefSet_GetAABBPointer( local struct PrimRefSet* refs, ushort id )
{
    return &refs->AABB[id];
}

GRL_INLINE float PrimRefSet_GetMaxAABBArea( local struct PrimRefSet* refs )
{
    float3 root_l = AABB3f_load_lower( &refs->root_aabb );
    float3 root_u = AABB3f_load_upper( &refs->root_aabb );
    float3 d = root_u - root_l;
    float scale = 1.0f / max( d.x, max( d.y, d.z ) );

    half3 dh = convert_half3_rtp( d * scale );
    return fma( dh.x, (dh.y + dh.z), dh.y * dh.z );
}

GRL_INLINE float3 ulp3( float3 v ) {

    return fabs(v) * FLT_EPSILON;
}

GRL_INLINE struct AABB PrimRefSet_ConvertAABB( local struct PrimRefSet* refs, struct DFSPrimRefAABB* box )
{
    float3 root_l = AABB3f_load_lower( &refs->root_aabb );
    float3 root_u = AABB3f_load_upper( &refs->root_aabb );
    float3 d = root_u - root_l;
    float scale = max( d.x, max( d.y, d.z ) );

    float3 l = convert_float3_rtz( (half3)(box->lower[0], box->lower[1], box->lower[2]) );
    float3 u = convert_float3_rtp( (half3)(box->upper[0], box->upper[1], box->upper[2]) );
    l =  l * scale + root_l ;
    u =  u * scale + root_l ;

    // clamping is necessary in case that a vertex lies exactly in the upper AABB plane.  
    //   If we use unclamped values, roundoff error in the scale factor calculation can cause us
    //   to snap to a flattened AABB that lies outside of the original one, resulting in missed geometry.
    u = min( u, root_u );
    l = min( l, root_u );

    struct AABB r;
    r.lower.xyz = l.xyz;
    r.upper.xyz = u.xyz;
    return r;
}

GRL_INLINE PrimRef PrimRefSet_GetFullPrecisionAABB( local struct PrimRefSet* refs, ushort id )
{
    struct AABB r;
    r = PrimRefSet_ConvertAABB( refs, &refs->AABB[id] );
    r.lower.w = 0;
    r.upper.w = 0;
    return r;
}

GRL_INLINE uint PrimRefSet_GetInputIndex( local struct PrimRefSet* refs, ushort id )
{
    return refs->meta[id].x;
}

GRL_INLINE uint PrimRefSet_GetInstanceMask( local struct PrimRefSet* refs, ushort id )
{
    return refs->meta[id].y;
}
GRL_INLINE struct PrimRefMeta PrimRefSet_GetMeta( local struct PrimRefSet* refs, ushort id )
{
    struct PrimRefMeta meta;
    meta.meta.x = refs->meta[id].x;
    meta.meta.y = refs->meta[id].y;
    return meta;
}


GRL_INLINE struct DFSPrimRef PrimRefSet_GetPrimRef( local struct PrimRefSet* refs, ushort id )
{
    struct DFSPrimRef r;
    r.aabb = refs->AABB[id];
    r.meta = refs->meta[id];
    return r;
}


GRL_INLINE void PrimRefSet_SetPrimRef_FullPrecision( local struct PrimRefSet* refs, PrimRef ref, ushort id )
{
    
    float3 root_l = AABB3f_load_lower( &refs->root_aabb );
    float3 root_u = AABB3f_load_upper( &refs->root_aabb );
    float3 d      = root_u - root_l;
    float scale   = 1.0f / max(d.x, max(d.y,d.z));
    
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
    refs->meta[id].y = PRIMREF_instanceMask(&ref);


}

GRL_INLINE void PrimRefSet_SetPrimRef( local struct PrimRefSet* refs, struct DFSPrimRef ref, ushort id )
{
    refs->AABB[id] = ref.aabb;
    refs->meta[id] = ref.meta;
}

GRL_INLINE struct AABB3f PrimRefSet_GetRootAABB( local struct PrimRefSet* refs )
{
    return refs->root_aabb;
}

GRL_INLINE void SUBGROUP_PrimRefSet_Initialize( local struct PrimRefSet* refs )
{
    if ( get_sub_group_local_id() == 0 )
        AABB3f_init( &refs->root_aabb ); // TODO_OPT: subgroup-vectorized version of AABB3f_init    
}


GRL_INLINE void PrimRefSet_Printf( local struct PrimRefSet* refs, ushort num_prims )
{

    barrier( CLK_LOCAL_MEM_FENCE );
    if ( get_local_id( 0 ) == 0 )
    {
        printf( "Scene AABB:\n" );
        struct AABB3f rootBox = PrimRefSet_GetRootAABB( refs );
        AABB3f_print( &rootBox );
        
        float ma = PrimRefSet_GetMaxAABBArea( refs );

        for ( uint i = 0; i < num_prims; i++ )
        {
            printf( "Ref: %u\n", i );
            struct AABB r = PrimRefSet_GetFullPrecisionAABB( refs, i );
            AABB_print( &r );          

            float a = DFSPrimRefAABB_halfArea( PrimRefSet_GetAABBPointer( refs, i ) );
            printf( "Scaled Area: %f / %f = %f \n", a, ma, a / ma );

        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );
}



GRL_INLINE void PrimRefSet_CheckBounds( local struct PrimRefSet* refs, ushort num_prims, PrimRef* primref_buffer )
{

    barrier( CLK_LOCAL_MEM_FENCE );
    if ( get_local_id( 0 ) == 0 )
    {

        for ( uint i = 0; i < num_prims; i++ )
        {
            PrimRef ref = primref_buffer[i];
            struct AABB r2 = PrimRefSet_GetFullPrecisionAABB( refs, i );

            struct DFSPrimRefAABB* box = &refs->AABB[i];
            float3 l = convert_float3_rtz( (half3)(box->lower[0], box->lower[1], box->lower[2]) );
            float3 u = convert_float3_rtp( (half3)(box->upper[0], box->upper[1], box->upper[2]) );

            printf( " halfs:{%x,%x,%x}{%x,%x,%x}\n", as_uint(l.x), as_uint(l.y), as_uint(l.z), as_uint(u.x), as_uint(u.y), as_uint(u.z) );

            printf( " {%f,%f,%f} {%f,%f,%f}    {%f,%f,%f} {%f,%f,%f} {%u,%u,%u,%u,%u,%u}\n",
                ref.lower.x, ref.lower.y, ref.lower.z, r2.lower.x, r2.lower.y, r2.lower.z,
                ref.upper.x, ref.upper.y, ref.upper.z, r2.upper.x, r2.upper.y, r2.upper.z,
                r2.lower.x <= ref.lower.x,
                r2.lower.y <= ref.lower.y,
                r2.lower.z <= ref.lower.z,

                r2.upper.x >= ref.upper.x,
                r2.upper.y >= ref.upper.y,
                r2.upper.z >= ref.upper.z );

        }

    }
    barrier( CLK_LOCAL_MEM_FENCE );
}



struct LocalBVH2
{
    uint num_nodes;
    uint nodes[DFS_BVH2_NODE_COUNT];

    // nodes are a bitfield:
    //    bits 8:0 (9b)     ==> number of primrefs in this subtree
    //    
    //    bits 17:9 (9b)    ==> for an inner node:  contains offset to a pair of children
    //                      ==> for a leaf node: contains index of the first primref in this leaf
    //
    //    bits 30:18 (13b)  ==> quantized AABB area (relative to root box)
    //    bit 31 (1b)       ==> is_inner flag
    //
    // NOTE: The left child offset of any node is always odd.. therefore, it is possible to recover a bit if we need it
    //        by storing only the 8 MSBs
};

#define DFS_BVH2_AREA_QUANT 8191.0f



GRL_INLINE void SUBGROUP_LocalBVH2_Initialize( local struct LocalBVH2* tree, ushort num_prims )
{
    tree->num_nodes = 1; // include the root node
    tree->nodes[0] = num_prims; // initialize root node as a leaf containing the full subtree
    
}

GRL_INLINE void LocalBVH2_CreateInnerNode( local struct LocalBVH2* tree, ushort node_index,
                           ushort start_left, ushort start_right,
                           ushort quantized_left_area, ushort quantized_right_area )
{
    uint child_pos   = atomic_add_local( &tree->num_nodes, 2 );
  
    // set the inner node flag and child position in the parent
    // leave the other bits intact
    uint parent_node = tree->nodes[node_index];
    parent_node |= 0x80000000;
    parent_node = (parent_node & ~(0x1ff<<9)) | (child_pos << 9);
    tree->nodes[node_index] = parent_node;

    // setup children as leaf nodes with prim-count zero
    uint left_child  = (convert_uint(start_left) << 9)  | (convert_uint( quantized_left_area )  << 18);
    uint right_child = (convert_uint(start_right) << 9) | (convert_uint( quantized_right_area ) << 18);
    tree->nodes[child_pos]      = left_child;
    tree->nodes[child_pos + 1]  = right_child;

}

GRL_INLINE ushort LocalBVH2_IncrementPrimCount( local struct LocalBVH2* tree, ushort node_index )
{
    // increment only the lower bits.  Given correct tree construction algorithm this will not overflow into MSBs
    return (atomic_inc_local( &tree->nodes[node_index] )) & 0x1ff; 
}

GRL_INLINE ushort LocalBVH2_GetNodeArea( local struct LocalBVH2* tree, ushort nodeID )
{
    return (tree->nodes[nodeID] >> 18) & 0x1FFF;
}

GRL_INLINE bool LocalBVH2_IsInnerNode( local struct LocalBVH2* tree, ushort nodeID )
{
    return (tree->nodes[nodeID] & 0x80000000) != 0;
}


GRL_INLINE ushort2 LocalBVH2_GetChildIndices( local struct LocalBVH2* tree, ushort nodeID )
{
    ushort idx = ((tree->nodes[nodeID] >> 9) & 0x1FF);
    return (ushort2)(idx, idx + 1);
}

GRL_INLINE ushort LocalBVH2_GetSubtreePrimCount( local struct LocalBVH2* tree, ushort node )
{
    return tree->nodes[node] & 0x1FF;
}

GRL_INLINE ushort LocalBVH2_GetLeafPrimStart( local struct LocalBVH2* tree, ushort node )
{
    return ((tree->nodes[node] >> 9) & 0x1FF);
}


GRL_INLINE void LocalBVH2_Printf( local struct LocalBVH2* tree )
{
    barrier( CLK_LOCAL_MEM_FENCE );

    if ( get_local_id( 0 ) == 0 )
    {
        printf( "Nodes: %u\n", tree->num_nodes );

        for ( uint i = 0; i < tree->num_nodes; i++ )
        {
            uint num_prims = LocalBVH2_GetSubtreePrimCount( tree, i );
            printf( "%3u : 0x%08x  %3u 0x%04x ", i, tree->nodes[i], num_prims, LocalBVH2_GetNodeArea(tree,i) ); 
            if ( LocalBVH2_IsInnerNode( tree, i ) )
            {
                ushort2 kids = LocalBVH2_GetChildIndices( tree, i );
                printf( " INNER ( %3u %3u )\n", kids.x, kids.y );
            }
            else
            {
                printf( " LEAF {" );
                for ( uint j = 0; j < num_prims; j++ )
                    printf( " %3u ", LocalBVH2_GetLeafPrimStart( tree, i ) + j );
                printf( "}\n" );
            }
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );
}

struct FlatTreeInnerNode
{
    uint DW0;                // lower 16b are index of corresponding LocalBVH2 node.. Bits 30:16  are an atomic flag used during refit.  Bit 31 is a leaf marker
    ushort parent_index;
    ushort first_child;
    uchar index_in_parent;
    uchar num_children;
    
    //struct DFSPrimRefAABB AABB;
};

struct FlatTree
{
    uint num_nodes;
    uint qnode_byte_offset; // byte offset from the BVHBase to the flat-tree's first QNode
    uint qnode_base_index; 
    
    struct FlatTreeInnerNode nodes[DFS_MAX_FLATTREE_NODES];   
    uchar primref_back_pointers[DFS_WG_SIZE];
};

GRL_INLINE void FlatTree_Printf( local struct FlatTree* flat_tree )
{
    barrier( CLK_LOCAL_MEM_FENCE );
    if ( get_local_id( 0 ) == 0 )
    {
        printf( "NumNodes: %u\n", flat_tree->num_nodes );
        for ( uint i = 0; i < flat_tree->num_nodes; i++ )
        {
            ushort bvh2_node = flat_tree->nodes[i].DW0 & 0xffff;
            printf( "%2u  Parent: %2u  Index_in_parent: %u, NumKids: %u  FirstKid: %3u bvh2: %3u DW0: 0x%x\n",
                i,
                flat_tree->nodes[i].parent_index,
                flat_tree->nodes[i].index_in_parent,
                flat_tree->nodes[i].num_children,
                flat_tree->nodes[i].first_child,
                bvh2_node,
                flat_tree->nodes[i].DW0 );
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );
}




GRL_INLINE ushort FlatTree_GetNodeCount( local struct FlatTree* flat_tree )
{
    return flat_tree->num_nodes;
}

GRL_INLINE uint FlatTree_GetParentIndex( local struct FlatTree* flat_tree, ushort id )
{
    return flat_tree->nodes[id].parent_index;
}

GRL_INLINE ushort FlatTree_GetBVH2Root( local struct FlatTree* flat_tree, ushort node_index )
{
    return (flat_tree->nodes[node_index].DW0) & 0xffff;
}

GRL_INLINE ushort FlatTree_GetNumChildren( local struct FlatTree* flat_tree, ushort node_index )
{
    return flat_tree->nodes[node_index].num_children;
}

GRL_INLINE bool FlatTree_IsLeafNode( local struct FlatTree* flat_tree, ushort node_index )
{
    return (flat_tree->nodes[node_index].DW0 & 0x80000000) != 0;
}


GRL_INLINE uint FlatTree_GetQNodeByteOffset( struct FlatTree* flat_tree, ushort node_index )
{
    return flat_tree->qnode_byte_offset + node_index * sizeof(struct QBVHNodeN);
}

GRL_INLINE uint FlatTree_GetQNodeIndex( struct FlatTree* flat_tree, ushort node_index )
{
    return flat_tree->qnode_base_index + node_index;
}

GRL_INLINE void FlatTree_AllocateQNodes( struct FlatTree* flat_tree, struct DFSArgs args )
{
    uint node_base = 64*allocate_inner_nodes( args.bvh_base, flat_tree->num_nodes );
    flat_tree->qnode_base_index  = (node_base - BVH_ROOT_NODE_OFFSET) / sizeof( struct QBVHNodeN );
    flat_tree->qnode_byte_offset = node_base;
}

GRL_INLINE ushort FlatTree_GetFirstChild( struct FlatTree* flat_tree, ushort node_index )
{
    return flat_tree->nodes[node_index].first_child;
}

GRL_INLINE ushort FlatTree_GetPrimRefStart( struct FlatTree* flat_tree, ushort node_index )
{
    return flat_tree->nodes[node_index].first_child;
}
GRL_INLINE ushort FlatTree_GetPrimRefCount( struct FlatTree* flat_tree, ushort node_index )
{
    return flat_tree->nodes[node_index].num_children;
}

GRL_INLINE uint FlatTree_BuildBackPointer( local struct FlatTree* flat_tree, ushort node_index )
{
    uint parent_index = flat_tree->nodes[node_index].parent_index + flat_tree->qnode_base_index;
    parent_index = (parent_index << 6) | (FlatTree_GetNumChildren( flat_tree, node_index ) << 3);
    return parent_index;
}


GRL_INLINE void SUBGROUP_FlatTree_Initialize( uniform local struct FlatTree* flat_tree, struct DFSArgs args )
{
    if ( get_sub_group_local_id() == 0 )
    {
        flat_tree->num_nodes    = 1;
        flat_tree->nodes[0].DW0 = 0; // point first node at BVH2 root node, which is assumed to be at index zero
    }
    
}
/*
GRL_INLINE void SUBGROUP_FlatTree_ReduceAndSetAABB( uniform local struct FlatTree* flat_tree,
                                         uniform ushort node_index,
                                         varying local struct DFSPrimRefAABB* box )
{
    // TODO_OPT: Replace this with an optimized reduction which exploits the fact that we only ever have 6 active lanes
    //       Try using the "negated max" trick here to compute min/max simultaneously, with max in top 6 lanes
    //          This will replace 6 reductions with 3
    
    // TODO_OPT:  This only utilizes up to 6 SIMD lanes.  We can use up to 12 of them by putting
    //  min into even lanes, and -max into odd lanes, and using a manual min-reduction on pairs of lanes

    struct DFSPrimRefAABB bb = DFSPrimRefAABB_sub_group_reduce( box );
    if( get_sub_group_local_id() )
        flat_tree->nodes[node_index].AABB = bb;
}
*/

GRL_INLINE void SUBGROUP_FlatTree_CreateInnerNode( uniform local struct FlatTree* flat_tree,
                                        uniform ushort flat_tree_root,
                                        varying ushort sg_child_bvh2_root,
                                        uniform ushort num_children )
{
    uniform uint lane = get_sub_group_local_id();
    
    // increment counter to allocate new nodes.. set required root node fields
    uniform uint child_base;
    if ( lane == 0 )
    {
        child_base = atomic_add_local( &flat_tree->num_nodes, num_children );
        flat_tree->nodes[flat_tree_root].first_child  = (uchar) child_base;
        flat_tree->nodes[flat_tree_root].num_children = num_children;

        // initialize mask bits for this node's live children
        uint child_mask = ((1 << num_children) - 1) << 16;
        flat_tree->nodes[flat_tree_root].DW0 |= child_mask;
    }

    child_base = sub_group_broadcast( child_base, 0 );

    // initialize child nodes
    if ( lane < num_children )
    {
        varying uint child = child_base + lane;
        flat_tree->nodes[child].DW0 = sg_child_bvh2_root;
        flat_tree->nodes[child].index_in_parent = lane;
        flat_tree->nodes[child].parent_index = flat_tree_root;
    }

}



GRL_INLINE void SUBGROUP_FlatTree_CreateLeafNode( uniform local struct FlatTree* flat_tree, 
                                       uniform ushort flat_tree_root,
                                       uniform ushort primref_start,
                                       uniform ushort num_prims )
{
    ushort lane = get_sub_group_local_id();
    if ( lane < num_prims )
    {
        flat_tree->primref_back_pointers[primref_start + lane] = (uchar) flat_tree_root;
        if ( lane == 0 )
        {
            flat_tree->nodes[flat_tree_root].first_child  = (uchar) primref_start;
            flat_tree->nodes[flat_tree_root].num_children = (uchar) num_prims;
            flat_tree->nodes[flat_tree_root].DW0 |= 0x80000000;
        }
    }
}


GRL_INLINE uniform bool SUBGROUP_FlatTree_SignalRefitComplete( uniform local struct FlatTree* flat_tree, uniform ushort* p_node_index )
{
    uniform ushort node_index       = *p_node_index;
    uniform ushort parent           = flat_tree->nodes[node_index].parent_index;
    uniform ushort index_in_parent  = flat_tree->nodes[node_index].index_in_parent;

    // clear the corresponding mask bit in the parent node
    uniform uint child_mask         = (0x10000 << index_in_parent);
    uniform uint old_mask_bits = 0;
    if( get_sub_group_local_id() == 0 )
        old_mask_bits = atomic_xor( &flat_tree->nodes[parent].DW0, child_mask );

    old_mask_bits = sub_group_broadcast( old_mask_bits, 0 );

    // if we cleared the last mask bit, this subgroup proceeds up the tree and refits the next node
    //  otherwise, it looks for something else to do
    if ( ((old_mask_bits^child_mask) & 0xffff0000) == 0 )
    {
        *p_node_index = parent;
        return true;
    }

    return false;
}

/*
GRL_INLINE local struct DFSPrimRefAABB* FlatTree_GetChildAABB( local struct FlatTree* flat_tree, 
                                            local struct PrimRefSet* prim_refs, 
                                            ushort node_index, ushort child_index )
{
    ushort child_id = FlatTree_GetFirstChild( flat_tree, node_index ) + child_index;

    if( !FlatTree_IsLeafNode( flat_tree, node_index ) )
        return &flat_tree->nodes[child_id].AABB;
    else
        return PrimRefSet_GetAABBPointer( prim_refs, child_id );
}
*/
GRL_INLINE uint FlatTree_GetPrimRefBackPointer( local struct FlatTree* flat_tree, ushort primref_index )
{
    return flat_tree->primref_back_pointers[primref_index] * sizeof(struct QBVHNodeN) + flat_tree->qnode_byte_offset;
}


GRL_INLINE void FlatTree_check_boxes(local struct FlatTree* flat_tree, 
    global struct AABB* primref_buffer, 
    local struct AABB3f* boxes,
    local struct PrimRefMeta* meta )

{
    barrier(CLK_LOCAL_MEM_FENCE);
    if (get_local_id(0) == 0)
    {
        printf("checking flattree bounds...\n");

        for (uint i = 0; i < flat_tree->num_nodes; i++)
        {            
            struct AABB rb;
            rb.lower.xyz = AABB3f_load_lower(&boxes[i]);
            rb.upper.xyz = AABB3f_load_upper(&boxes[i]);

            uint offs  = FlatTree_GetFirstChild( flat_tree, i );
            uint count = FlatTree_GetNumChildren( flat_tree, i );

            for (uint c = 0; c < count; c++)
            {
                struct AABB lb;
                if (FlatTree_IsLeafNode( flat_tree, i ))
                {
                    lb = primref_buffer[ PrimRefMeta_GetInputIndex( &meta[offs+c] ) ];
                }
                else
                {
                    lb.lower.xyz = AABB3f_load_lower(&boxes[ offs+c ]);
                    lb.upper.xyz = AABB3f_load_upper(&boxes[ offs+c ]);
                }

                if( !AABB_subset( &lb, &rb ) )
                    printf("Bad bounds!!  child %u of %u   %f : %f  %f : %f %f : %f    %f : %f  %f : %f %f : %f \n",
                        c, i ,
                        rb.lower.x, rb.upper.x, rb.lower.y, rb.upper.y, rb.lower.z, rb.upper.z,
                        lb.lower.x, lb.upper.x, lb.lower.y, lb.upper.y, lb.lower.z, lb.upper.z
                        );
            }
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);
}


struct FlatTreeScheduler
{
    int   num_leafs;
    uint  writeout_produce_count;
    uint  writeout_consume_count;
    uint  active_subgroups;
    uint  num_built_nodes;
    uint  num_levels;   // number of depth levels in the tree

    //uchar leaf_indices[DFS_MAX_FLATTREE_LEAFS];     // indices of leaf FlatTree nodes to be refitted
    //uchar writeout_indices[DFS_MAX_FLATTREE_NODES]; // indices of flattree nodes to be written out or collapsed

    ushort level_ordered_nodes[DFS_MAX_FLATTREE_NODES]; // node indices sorted by depth (pre-order, high depth before low depth)
    ushort level_start[DFS_MAX_FLATTREE_DEPTH]; // first node at given level in the level-ordered node array
    uint level_count[DFS_MAX_FLATTREE_DEPTH];  // number of nodes at given level
};

GRL_INLINE void SUBGROUP_FlatTreeScheduler_Initialize( uniform local struct FlatTreeScheduler* scheduler )
{
    scheduler->num_built_nodes = 0;
    scheduler->num_leafs = 0;
    scheduler->writeout_produce_count = 0;
    scheduler->writeout_consume_count = 0;
    scheduler->active_subgroups = DFS_NUM_SUBGROUPS;
}
/*
GRL_INLINE void SUBGROUP_FlatTreeScheduler_QueueLeafForRefit( uniform local struct FlatTreeScheduler* scheduler,
                                                   uniform ushort leaf )
{
    if ( get_sub_group_local_id() == 0 )
        scheduler->leaf_indices[atomic_inc( &scheduler->num_leafs )] = leaf;
}*/

GRL_INLINE void SUBGROUP_FlatTreeScheduler_SignalNodeBuilt( uniform local struct FlatTreeScheduler* scheduler, uniform ushort node )
{
    if ( get_sub_group_local_id() == 0 )
        atomic_inc_local( &scheduler->num_built_nodes );
}

GRL_INLINE uint FlatTreeScheduler_GetNumBuiltNodes( uniform local struct FlatTreeScheduler* scheduler )
{
    return scheduler->num_built_nodes;
}

/*
GRL_INLINE void SUBGROUP_FlatTreeScheduler_QueueNodeForWriteOut( uniform local struct FlatTreeScheduler* scheduler, uniform ushort node )
{
    if ( get_sub_group_local_id() == 0 )
        scheduler->writeout_indices[atomic_inc( &scheduler->writeout_produce_count )] = node;
}*/

/*
GRL_INLINE bool SUBGROUP_FlatTreeScheduler_GetRefitTask( uniform local struct FlatTreeScheduler* scheduler, uniform ushort* leaf_idx )
{
    // schedule the leaves in reverse order to ensure that later leaves
    //   complete before earlier ones.. This prevents contention during the WriteOut stage
    // 
    // There is a barrier between this function and 'QueueLeafForRefit' so we can safely decrement the same counter 
    //   that we incremented earlier
    varying int idx = 0;
    if( get_sub_group_local_id() == 0 )
        idx = atomic_dec( &scheduler->num_leafs ); 

    sub_group_barrier( CLK_LOCAL_MEM_FENCE );
    idx = sub_group_broadcast( idx, 0 );
    
    if ( idx <= 0 )
        return false;

    *leaf_idx = scheduler->leaf_indices[idx-1];
    return true;
}*/

/*
// Signal the scheduler that a subgroup has reached the DONE state.
//  Return true if this is the last subgroup to be done
void SUBGROUP_FlatTreeScheduler_SubGroupDone( local struct FlatTreeScheduler* scheduler )
{
    if ( get_sub_group_local_id() == 0 )
        atomic_dec( &scheduler->active_subgroups );
}
*/

/*

#define STATE_SCHEDULE_REFIT    0x1234
#define STATE_SCHEDULE_WRITEOUT 0x5679
#define STATE_REFIT             0xabcd
#define STATE_WRITEOUT          0xefef
#define STATE_DONE              0xaabb

// Get a flattree node to write out.  Returns the new scheduler state
GRL_INLINE ushort SUBGROUP_FlatTreeScheduler_GetWriteOutTask( uniform local struct FlatTreeScheduler* scheduler,
                                                   uniform ushort num_nodes,
                                                   uniform ushort* node_idx )
{
    uniform ushort return_state = STATE_WRITEOUT;
    uniform ushort idx = 0;
    if ( get_sub_group_local_id() == 0 )
    {
        idx = atomic_inc( &scheduler->writeout_consume_count );     
   
        if ( idx >= scheduler->writeout_produce_count )
        {
            // more consumers than there are produced tasks....

            if ( scheduler->writeout_produce_count == num_nodes )
            {
                // if all nodes have been written out, flattening is done
                return_state = STATE_DONE;
            }
            else
            {
                // some writeout tasks remain, and have not been produced by refit threads yet
                //   we need to put this one back
                atomic_dec( &scheduler->writeout_consume_count );
                return_state = STATE_SCHEDULE_WRITEOUT;
            }
        }
        else
        {
            // scheduled successfully 
            idx = scheduler->writeout_indices[idx];
        }
    }

    *node_idx = sub_group_broadcast( idx, 0 );
    return sub_group_broadcast( return_state, 0 );

}
*/


/*
GRL_INLINE void FlatTreeScheduler_Printf( local struct FlatTreeScheduler* scheduler )
{
    barrier( CLK_LOCAL_MEM_FENCE );

    if ( get_local_id( 0 ) == 0 )
    {
        printf( "***SCHEDULER***\n" );
        printf( "built_nodes=%u  active_sgs=%u  leafs=%u wo_p=%u  wo_c=%u\n", scheduler->num_built_nodes, scheduler->active_subgroups, scheduler->num_leafs,
            scheduler->writeout_produce_count, scheduler->writeout_consume_count );
        printf( "leafs for refit: {" );

        int nleaf = max( scheduler->num_leafs, 0 );

        for ( uint i = 0; i < nleaf; i++ )
            printf( "%u ", scheduler->leaf_indices[i] );
        printf( "}\n" );

        printf( "writeout queue: %u:%u {", scheduler->writeout_produce_count, scheduler->writeout_consume_count );
        for ( uint i = 0; i < scheduler->writeout_produce_count; i++ )
            printf( "%u ", scheduler->writeout_indices[i] );
        printf( "}\n" );
    }

    barrier( CLK_LOCAL_MEM_FENCE );

}
*/


GRL_INLINE void SUBGROUP_BuildFlatTreeNode( local struct LocalBVH2* bvh2,
                                 local struct FlatTree* flat_tree,
                                 local struct FlatTreeScheduler* scheduler,
                                 uniform ushort flat_tree_root )
{
    varying ushort lane = get_sub_group_local_id();
    varying ushort bvh2_root = FlatTree_GetBVH2Root( flat_tree, flat_tree_root );

    if ( !LocalBVH2_IsInnerNode( bvh2, bvh2_root ) )
    {
        uniform ushort num_prims        = LocalBVH2_GetSubtreePrimCount( bvh2, bvh2_root );
        uniform ushort primref_start    = LocalBVH2_GetLeafPrimStart( bvh2, bvh2_root );

        SUBGROUP_FlatTree_CreateLeafNode( flat_tree, flat_tree_root, primref_start, num_prims );
    }
    else
    {
        // collapse BVH2 into BVH6.
        // We will spread the root node's children across the subgroup, and keep adding SIMD lanes until we have enough
        uniform ushort num_children = 2;

        uniform ushort2 kids =  LocalBVH2_GetChildIndices( bvh2, bvh2_root );
        varying ushort sg_bvh2_node = kids.x;
        if ( lane == 1 )
            sg_bvh2_node = kids.y;

        do
        {            
            // choose the inner node with maximum area to replace.  
            // Its left child goes in its old location.  Its right child goes in a new lane

            varying ushort sg_area   = LocalBVH2_GetNodeArea( bvh2, sg_bvh2_node );
            varying bool sg_is_inner = LocalBVH2_IsInnerNode( bvh2, sg_bvh2_node );
            sg_area = (sg_is_inner && lane < num_children) ? sg_area : 0; // prevent early exit if the largest child is a leaf
            
            uniform ushort max_area  = sub_group_reduce_max( sg_area );
            varying bool sg_reducable = max_area == sg_area && (lane < num_children) && sg_is_inner;
            uniform uint mask         = intel_sub_group_ballot( sg_reducable );

            // TODO_OPT:  Some of these ops seem redundant.. look at trimming further
            // TODO_OPT:  sub_group_reduce_max results in too many instructions...... unroll the loop and specialize it..
            //       or ask IGC to give us a version that declares a static maximum number of subgroups to use

            if ( mask == 0 )
                break;

            // choose the inner node with maximum area to replace
            uniform ushort victim_child = ctz( mask );
            uniform ushort victim_node  = sub_group_broadcast( sg_bvh2_node, victim_child );
            uniform ushort2 kids        = LocalBVH2_GetChildIndices( bvh2, victim_node );

            if ( lane == victim_child )
                sg_bvh2_node = kids.x;
            else if ( lane == num_children )
                sg_bvh2_node = kids.y;
            
            
            num_children++;


        }while ( num_children < TREE_ARITY );

        SUBGROUP_FlatTree_CreateInnerNode( flat_tree, flat_tree_root, sg_bvh2_node, num_children );
    }

}


GRL_INLINE void SUBGROUP_DFS_BuildFlatTree( uniform local struct LocalBVH2* bvh2,
                                 uniform local struct FlatTree* flat_tree,
                                 uniform local struct FlatTreeScheduler* scheduler
                                )
{

    uniform ushort flat_tree_node_index = get_sub_group_id();
    uniform ushort num_nodes     = 1;
    uniform ushort num_built     = 0;
    
    uint tid = get_local_id(0);
    if (tid < DFS_MAX_FLATTREE_DEPTH)
    {
        scheduler->level_start[tid] = DFS_MAX_FLATTREE_NODES;
        scheduler->level_count[tid] = 0;
        scheduler->num_levels = 0;
    }

    LOOP_TRIPWIRE_INIT;

    do
    {
        // process one flat tree node per sub group, as many as are available
        //
        //  The first pass will only run one sub-group, the second up to 6, the third up to 36, and so on
        //     nodes will be processed in breadth-first order, but they are not guaranteed to be stored in this order
        //      due to use of atomic counters for node allocation
        //
        if ( flat_tree_node_index < num_nodes )
        {
            SUBGROUP_BuildFlatTreeNode( bvh2, flat_tree, scheduler, flat_tree_node_index );
            SUBGROUP_FlatTreeScheduler_SignalNodeBuilt( scheduler, flat_tree_node_index );            
            flat_tree_node_index += get_num_sub_groups();
        }

        barrier( CLK_LOCAL_MEM_FENCE );

        // bump up the node count if new nodes were created
        // stop as soon as all flattree nodes have been processed
        num_nodes = FlatTree_GetNodeCount( flat_tree );
        num_built = FlatTreeScheduler_GetNumBuiltNodes( scheduler );

        barrier( CLK_LOCAL_MEM_FENCE );
        
        LOOP_TRIPWIRE_INCREMENT( 300 );

    } while ( num_built < num_nodes );
  
    barrier( CLK_LOCAL_MEM_FENCE );


    // determine depth of each node, compute node ranges and counts for each depth level, 
    //  and prepare a depth-ordered node index array
    uint depth = 0;
    uint level_pos = 0;
    for( uint i=tid; i<num_nodes; i += get_local_size(0) )
    {
        // compute depth of this node
        uint node_index = i;
        while ( node_index != 0 )
        {
            node_index = FlatTree_GetParentIndex( flat_tree, node_index );
            depth++;
        }

        // assign this node a position within it's depth level
        level_pos = atomic_inc_local( &scheduler->level_count[depth] );
    
        // compute total number of levels 
        atomic_max_local( &scheduler->num_levels, depth+1 );
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    for( uint i=tid; i<num_nodes; i += get_local_size(0) )
    {
        // prefix-sum level start positions.  Re-computed for each thread
        // TODO:  Hierarchical reduction ??
        uint level_start=0;
        for( uint d=0; d<depth; d++ )
            level_start += scheduler->level_count[d];

        scheduler->level_start[depth] = level_start;

        // scatter node indices into level-ordered node array
        scheduler->level_ordered_nodes[level_start + level_pos] = tid;
    }

    barrier( CLK_LOCAL_MEM_FENCE );

}

/*
GRL_INLINE bool SUBGROUP_RefitNode( uniform local struct FlatTree* flat_tree,
                         uniform local struct PrimRefSet* prim_refs,
                         uniform ushort* p_node_index )
{

    // fetch and reduce child AABBs across the subgroup
    uniform ushort node_index = *p_node_index;
    uniform ushort num_kids = FlatTree_GetNumChildren( flat_tree, node_index );
    varying ushort sg_child_index = (get_sub_group_local_id() < num_kids) ? get_sub_group_local_id() : 0;

    varying local struct DFSPrimRefAABB* box = FlatTree_GetChildAABB( flat_tree, prim_refs, node_index, sg_child_index );

    SUBGROUP_FlatTree_ReduceAndSetAABB( flat_tree, node_index, box );

    if ( node_index == 0 )
        return false; // if we just refitted the root, we can stop now

    // signal the parent node that this node was refitted.  If this was the last child to be refitted
    //    returns true and sets 'node_index' to the parent node, so that this thread can continue refitting
    return SUBGROUP_FlatTree_SignalRefitComplete( flat_tree, p_node_index );
}*/

GRL_INLINE struct QBVHNodeN* qnode_ptr( BVHBase* bvh_mem, uint byte_offset )
{
    return (struct QBVHNodeN*)(((char*)bvh_mem) + byte_offset);
}

GRL_INLINE void SUBGROUP_WriteQBVHNode(        
        uniform local struct FlatTree* flat_tree,
        uniform local struct PrimRefMeta* primref_meta,
        uniform local struct AABB3f* boxes,
        uniform ushort flat_tree_root,
        uniform struct DFSArgs args,
        uniform local uchar* masks
      )
{

    
    uniform ushort num_children = FlatTree_GetNumChildren( flat_tree, flat_tree_root );
    uniform bool is_leaf        = FlatTree_IsLeafNode( flat_tree, flat_tree_root );

    varying ushort lane = get_sub_group_local_id();
    varying ushort sg_child_index = (lane < num_children) ? lane : 0;

    uniform ushort child_base = FlatTree_GetFirstChild( flat_tree, flat_tree_root );

    varying struct AABB sg_box4;
    if (FlatTree_IsLeafNode( flat_tree, flat_tree_root ))
    {
        // fetch AABBs for primrefs               
        sg_box4 = args.primref_buffer[ PrimRefMeta_GetInputIndex( &primref_meta[child_base + sg_child_index] ) ];                    
        
    }
    else
    {
        // fetch AABBs for child nodes
        sg_box4.lower.xyz = AABB3f_load_lower( &boxes[child_base+sg_child_index] );
        sg_box4.upper.xyz = AABB3f_load_upper( &boxes[child_base+sg_child_index] );
    }


    struct QBVHNodeN* qnode = qnode_ptr( args.bvh_base, FlatTree_GetQNodeByteOffset( flat_tree, flat_tree_root ) );

    uniform int offset;
    uniform uint child_type;
    if ( is_leaf )
    { 
        char* leaf_mem = (char*)BVHBase_GetQuadLeaves( args.bvh_base );

        leaf_mem += ( FlatTree_GetPrimRefStart( flat_tree, flat_tree_root )) * args.leaf_size_in_bytes;

        offset = (int)(leaf_mem - (char*)qnode);
        child_type = args.leaf_node_type;
    }
    else
    {
        struct QBVHNodeN* kid = qnode_ptr( args.bvh_base, FlatTree_GetQNodeByteOffset( flat_tree, FlatTree_GetFirstChild( flat_tree, flat_tree_root ) ) );
        offset = (int) ((char*)kid - (char*)qnode);
        child_type = args.inner_node_type;
    }
    offset = offset >> 6;

    if (child_type == NODE_TYPE_INSTANCE)
    {
        uint instanceMask = PrimRefMeta_GetInstanceMask( &primref_meta[child_base + sg_child_index] );
        subgroup_setInstanceQBVHNodeN( offset, &sg_box4, num_children, qnode, lane < num_children ? instanceMask : 0 );
    }
    else
    {
        uint mask = BVH_NODE_DEFAULT_MASK;
        if( args.need_masks )
            mask = masks[flat_tree_root];

        subgroup_setQBVHNodeN( offset, child_type, &sg_box4, num_children, qnode, mask );
    }

    if ( args.need_backpointers )
    {
        global uint* back_pointers = (global uint*) BVHBase_GetBackPointers( args.bvh_base );
        uint idx = FlatTree_GetQNodeIndex( flat_tree, flat_tree_root );
        uint bp = FlatTree_BuildBackPointer( flat_tree, flat_tree_root );
        back_pointers[idx] = bp;
    }

    /*
    // TODO_OPT:  Eventually this section should also handle leaf splitting due to mixed primref types
    //    For now this is done by the leaf creation pipeline, but that path should probably be refactored
    //      such that all inner node creation is done in one place

    uniform ushort num_children = FlatTree_GetNumChildren( flat_tree, flat_tree_root );
    uniform bool is_leaf        = FlatTree_IsLeafNode( flat_tree, flat_tree_root );

    varying ushort lane = get_sub_group_local_id();
    varying ushort sg_child_index = (lane < num_children) ? lane : 0;

    varying local struct DFSPrimRefAABB* sg_box = FlatTree_GetChildAABB( flat_tree, prim_refs, flat_tree_root, sg_child_index );

    varying struct AABB sg_box4 = PrimRefSet_ConvertAABB( prim_refs, sg_box );
    
    struct QBVHNodeN* qnode = qnode_ptr( args.bvh_base, FlatTree_GetQNodeByteOffset( flat_tree, flat_tree_root ) );

    uniform int offset;
    uniform uint child_type;
    if ( is_leaf )
    { 
        char* leaf_mem = (char*)BVHBase_GetQuadLeaves( args.bvh_base );

        leaf_mem += ( FlatTree_GetPrimRefStart( flat_tree, flat_tree_root )) * args.leaf_size_in_bytes;

        offset = (int)(leaf_mem - (char*)qnode);
        child_type = args.leaf_node_type;
    }
    else
    {
        struct QBVHNodeN* kid = qnode_ptr( args.bvh_base, FlatTree_GetQNodeByteOffset( flat_tree, FlatTree_GetFirstChild( flat_tree, flat_tree_root ) ) );
        offset = (int) ((char*)kid - (char*)qnode);
        child_type = args.inner_node_type;
    }
    offset = offset >> 6;
    
    if (child_type == NODE_TYPE_INSTANCE)
    {
        uint instanceMask = PrimRefSet_GetInstanceMask( prim_refs, FlatTree_GetPrimRefStart(flat_tree, flat_tree_root) + lane );
        subgroup_setInstanceQBVHNodeN( offset, &sg_box4, num_children, qnode, lane < num_children ? instanceMask : 0 );
    }
    else
        subgroup_setQBVHNodeN( offset, child_type, &sg_box4, num_children, qnode );

    if ( args.need_backpointers )
    {
        global uint* back_pointers = (global uint*) BVHBase_GetBackPointers( args.bvh_base );
        uint idx = FlatTree_GetQNodeIndex( flat_tree, flat_tree_root );
        uint bp = FlatTree_BuildBackPointer( flat_tree, flat_tree_root );
        back_pointers[idx] = bp;
    }
    */
}

/*
GRL_INLINE void SUBGROUP_DFS_RefitAndWriteOutFlatTree(
    uniform local struct FlatTree* flat_tree,
    uniform local struct PrimRefSet* prim_refs,
    uniform local struct FlatTreeScheduler* scheduler,
    uniform struct DFSArgs args)
{

    uniform ushort state = STATE_SCHEDULE_REFIT;
    uniform ushort node_index = 0;
    uniform ushort num_nodes = FlatTree_GetNodeCount(flat_tree);

    {
        LOOP_TRIPWIRE_INIT;

        bool active = true;
        bool continue_refit = false;
        while (1)
        {
            if (active)
            {
                if (continue_refit || SUBGROUP_FlatTreeScheduler_GetRefitTask(scheduler, &node_index))
                {
                    continue_refit = SUBGROUP_RefitNode(flat_tree, prim_refs, &node_index);
                }
                else
                {
                    active = false;
                    if (get_sub_group_local_id() == 0)
                        atomic_dec(&scheduler->active_subgroups);

                    sub_group_barrier(CLK_LOCAL_MEM_FENCE);
                }
            }

            barrier(CLK_LOCAL_MEM_FENCE); // finish all atomics
            if (scheduler->active_subgroups == 0)
                break;
            barrier(CLK_LOCAL_MEM_FENCE); // finish all checks.. prevent race between thread which loops around and thread which doesn't

            LOOP_TRIPWIRE_INCREMENT(200);
        }
    }

    for (uint i = get_sub_group_id(); i < num_nodes; i += get_num_sub_groups())
        SUBGROUP_WriteQBVHInnerNodes(flat_tree, prim_refs, i, args);

    barrier(CLK_LOCAL_MEM_FENCE);


    // JDB:  Version below attempts to interleave refit and qnode write-out
    //  This could theoretically reduce thread idle time, but it is more complex and does more atomics for scheduling

#if 0
    // after we've constructed the flat tree (phase 1), there are two things that need to happen:
    //   PHASE 2:  Refit the flat tree, computing all of the node ABBs
    //   PHASE 3:  Write the nodes out to memory
    //
    //  all of this is sub-group centric.  Different subgroups can execute phases 2 and 3 concurrently
    //    

    // TODO_OPT:  The scheduling algorithm might need to be re-thought.
    //  Fused EUs are very hard to reason about.   It's possible that by scheduling independent
    //  SGs in this way we would lose a lot of performance due to fused EU serialization.
    //     Needs to be tested experimentally if such a thing is possible

    uniform ushort state = STATE_SCHEDULE_REFIT;
    uniform ushort node_index = 0;
    uniform ushort num_nodes = FlatTree_GetNodeCount(flat_tree);

    LOOP_TRIPWIRE_INIT;

    do
    {
        // barrier necessary to protect access to scheduler->active_subgroups
        barrier(CLK_LOCAL_MEM_FENCE);

        if (state == STATE_SCHEDULE_REFIT)
        {
            if (SUBGROUP_FlatTreeScheduler_GetRefitTask(scheduler, &node_index))
                state = STATE_REFIT;
            else
                state = STATE_SCHEDULE_WRITEOUT; // fallthrough
        }
        if (state == STATE_SCHEDULE_WRITEOUT)
        {
            state = SUBGROUP_FlatTreeScheduler_GetWriteOutTask(scheduler, num_nodes, &node_index);
            if (state == STATE_DONE)
                SUBGROUP_FlatTreeScheduler_SubGroupDone(scheduler);
        }


        // A barrier is necessary to ensure that 'QueueNodeForWriteOut' is synchronized with 'GetWriteOutTask'
        //  Note that in theory we could have the write-out tasks spin until the refit tasks clear, which would make this barrier unnecessary
        //   However, we cannot do this safely on SKUs which do not support independent subgroup forward progress.
        barrier(CLK_LOCAL_MEM_FENCE);

        if (state == STATE_REFIT)
        {
            uniform ushort prev_node = node_index;
            uniform bool continue_refit = SUBGROUP_RefitNode(flat_tree, prim_refs, &node_index);

            SUBGROUP_FlatTreeScheduler_QueueNodeForWriteOut(scheduler, prev_node);

            if (!continue_refit)
                state = STATE_SCHEDULE_REFIT;
        }
        else if (state == STATE_WRITEOUT)
        {
            SUBGROUP_WriteQBVHInnerNodes(flat_tree, prim_refs, node_index, args);
            state = STATE_SCHEDULE_WRITEOUT;
        }
        // A barrier is necessary to ensure that 'QueueNodeForWriteOut' is synchronized with 'GetWriteOutTask'
        barrier(CLK_LOCAL_MEM_FENCE);

        LOOP_TRIPWIRE_INCREMENT(200);

    } while (scheduler->active_subgroups > 0);

#endif
}
*/

GRL_INLINE void DFS_CreatePrimRefSet( struct DFSArgs args,
                           local struct PrimRefSet* prim_refs )
{
    ushort id = get_local_id( 0 );
    ushort num_primrefs = args.num_primrefs;


    PrimRef ref;
    struct AABB3f local_aabb;
    if ( id < num_primrefs )
    {
        ref = args.primref_buffer[id];
        AABB3f_set_lower( &local_aabb, ref.lower.xyz );
        AABB3f_set_upper( &local_aabb, ref.upper.xyz );
    }
    else
    {
        AABB3f_init( &local_aabb );
    }

    AABB3f_atomic_merge_localBB_nocheck( &prim_refs->root_aabb, &local_aabb );

    barrier( CLK_LOCAL_MEM_FENCE );

    if ( id < num_primrefs )
        PrimRefSet_SetPrimRef_FullPrecision( prim_refs, ref, id );    
}



struct BVHBuildLocals
{
    float  Al[DFS_WG_SIZE];
    float  Ar[DFS_WG_SIZE];
    uchar2 axis_and_left_count[ DFS_WG_SIZE ];
    uint   sah[DFS_WG_SIZE];
    uint   num_active_threads;
};


GRL_INLINE void DFS_ConstructBVH2( local struct LocalBVH2* bvh2, 
                        local struct PrimRefSet* prim_refs, 
                        ushort num_prims,
                        local struct BVHBuildLocals* locals )
{   
    ushort tid = get_local_id( 0 );

    ushort bvh2_root         = 0;
    ushort prim_range_start  = 0;
    ushort primref_position = tid;

    bool active_thread       = tid < num_prims;
    float root_area  = PrimRefSet_GetMaxAABBArea( prim_refs );
    float area_scale = DFS_BVH2_AREA_QUANT / root_area;
    
    locals->num_active_threads = num_prims;
    barrier( CLK_LOCAL_MEM_FENCE );

    LOOP_TRIPWIRE_INIT;

    do
    {
        if(active_thread && prim_range_start == primref_position)
            locals->sah[primref_position] = UINT_MAX;

        if ( active_thread )
        {            
            local struct DFSPrimRefAABB* my_box = PrimRefSet_GetAABBPointer( prim_refs, primref_position );

            // each thread evaluates a possible split candidate.  Scan primrefs and compute sah cost
            //  do this axis-by-axis to keep register pressure low
            float best_sah    = INFINITY;
            ushort best_axis  = 3;
            ushort best_count = 0;
            float best_al     = INFINITY;
            float best_ar     = INFINITY;

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
            
                for ( ushort p = 1; p < num_prims; p++ )
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
                float Al = DFSPrimRefAABB_halfArea( &box_left[axis]  );
                float Ar = DFSPrimRefAABB_halfArea( &box_right[axis] );
                
                // Avoid NANs in SAH calculation in the corner case where all prims go right
                //  In this case we set Al=Ar, because such a split will only be selected if all primrefs
                //    are co-incident..  In that case, we will fall back to split-in-the-middle and both subtrees 
                //    should store the same quantized area value
                if ( count_left[axis] == 0 )
                    Al = Ar; 

                // compute sah cost
                ushort count_right = num_prims - count_left[axis];
                float sah = Ar * count_right + Al * count_left[axis];
                
                // keep this split if it is better than the previous one, or if the previous one was a corner-case
                if ( sah < best_sah || best_count == 0 )
                {
                    // yes, keep it
                    best_axis   = axis;
                    best_sah    = sah;
                    best_count  = count_left[axis];
                    best_al     = Al;
                    best_ar     = Ar;
                }               
            }


            // write split information to SLM
            locals->Al[primref_position]             = best_al;
            locals->Ar[primref_position]             = best_ar;
            locals->axis_and_left_count[primref_position].x = best_axis;
            locals->axis_and_left_count[primref_position].y = best_count;

            uint sah = as_uint(best_sah);
            // break ties by axis to ensure deterministic split selection
            //  otherwise builder can produce non-deterministic tree structure run to run
            //  based on the ordering of primitives (which can vary due to non-determinism in atomic counters)
            // Embed split axis and index into sah value; compute min over sah and max over axis
            sah = ( ( sah & ~1023 ) | ( 2 - best_axis ) << 8 | primref_position );

            // reduce on split candidates in our local subtree and decide the best one
            atomic_min_local( &locals->sah[ prim_range_start ], sah);
        }

        barrier( CLK_LOCAL_MEM_FENCE );

        ushort split_index      = locals->sah[ prim_range_start ] & 255;
        ushort split_axis       = locals->axis_and_left_count[split_index].x;
        ushort split_left_count = locals->axis_and_left_count[split_index].y;
        float split_al          = locals->Al[split_index];
        float split_ar          = locals->Ar[split_index];

        if ( (primref_position == prim_range_start) && active_thread )
        {
            // first thread in a given subtree creates the inner node
            ushort quantized_left_area  = convert_ushort_rtn( split_al * area_scale );
            ushort quantized_right_area = convert_ushort_rtn( split_ar * area_scale );
            ushort start_left  = prim_range_start;
            ushort start_right = prim_range_start + split_left_count;
            if ( split_left_count == 0 )
                start_right = start_left + (num_prims / 2); // handle split-in-the-middle case

            LocalBVH2_CreateInnerNode( bvh2, bvh2_root, 
                                      start_left, start_right,
                                      quantized_left_area, quantized_right_area );
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
                split_left_count = (num_prims / 2);
                go_left = (primref_position - prim_range_start < split_left_count);
            }
            else
            {
                // determine what side of the split this thread's primref belongs on
                local struct DFSPrimRefAABB* my_box     = PrimRefSet_GetAABBPointer( prim_refs, primref_position );
                local struct DFSPrimRefAABB* split_box  = PrimRefSet_GetAABBPointer( prim_refs, split_index );
                float c = my_box->lower[split_axis] + my_box->upper[split_axis];
                float Csplit = split_box->lower[split_axis] + split_box->upper[split_axis];
                go_left = c < Csplit;                
            }

            // adjust state variables for next loop iteration
            bvh2_root                    = (go_left) ? kids.x : kids.y;
            num_prims                    = (go_left) ? split_left_count : (num_prims - split_left_count);
            prim_range_start             = (go_left) ? prim_range_start : prim_range_start + split_left_count;

            // determine the new primref position by incrementing a counter in the destination subtree
            new_primref_position = prim_range_start + LocalBVH2_IncrementPrimCount( bvh2, bvh2_root );
            
            // load our primref from its previous position
            ref = PrimRefSet_GetPrimRef( prim_refs, primref_position );
        }
        
        barrier( CLK_LOCAL_MEM_FENCE );

        if ( active_thread )
        {
            // write our primref into its sorted position
            PrimRefSet_SetPrimRef( prim_refs, ref, new_primref_position );
            primref_position = new_primref_position;

            // deactivate all threads whose subtrees are small enough to form a leaf
            if ( num_prims <= TREE_ARITY )
            {
                active_thread = false;
                atomic_dec_local( &locals->num_active_threads );
            }
        } 

        barrier( CLK_LOCAL_MEM_FENCE );

        LOOP_TRIPWIRE_INCREMENT( 50 );


    } while ( locals->num_active_threads > 0 );


}



// fast path for #prims <= TREE_ARITY
GRL_INLINE void Trivial_DFS( struct DFSArgs args )
{

    ushort tid = get_local_id( 0 );

    PrimRef myRef;
    AABB_init( &myRef );
    if( tid < args.num_primrefs )
        myRef = args.primref_buffer[tid];

    uint node_offset;
    if ( tid == 0 )
        node_offset = 64*allocate_inner_nodes( args.bvh_base, 1 );
    node_offset = sub_group_broadcast(node_offset,0);

    char* bvh_mem = (char*) args.bvh_base;
    struct QBVHNodeN* qnode  = (struct QBVHNodeN*) (bvh_mem + node_offset);

    uint child_type = args.leaf_node_type;
    uint prim_base  = args.bvh_base->quadLeafStart*64 ;

    char* leaf_mem = bvh_mem + prim_base;
    int offset = (int)( leaf_mem  - (char*)qnode );

    if (child_type == NODE_TYPE_INSTANCE)
    {
        subgroup_setInstanceQBVHNodeN( offset >> 6, &myRef, args.num_primrefs, qnode, tid < args.num_primrefs ? PRIMREF_instanceMask(&myRef) : 0  );
    }
    else
        subgroup_setQBVHNodeN( offset >> 6, child_type, &myRef, args.num_primrefs, qnode, BVH_NODE_DEFAULT_MASK );

    if ( tid < args.num_primrefs )
    {
        global uint* primref_back_pointers = args.primref_index_buffer + args.num_primrefs;
        uint bp = node_offset;

        // TODO_OPT:  Leaf creation pipeline can be made simpler by having a sideband buffer containing
        //    fatleaf index + position in fatleaf for each primref, instead of forcing leaf creation shader to reconstruct it
        //   should also probably do the fat-leaf splitting here
        args.primref_buffer[tid]        = myRef;
        args.primref_index_buffer[tid]  = tid;

        primref_back_pointers[tid] = bp / sizeof(struct QBVHNodeN);

        if ( tid == 0 && args.need_backpointers )
        {
            uint bp = ((uint)-1) << 6;
            bp |= (args.num_primrefs) << 3;
            *(InnerNode_GetBackPointer(BVHBase_GetBackPointers( args.bvh_base ),0)) = bp;
        }
    }
}





void SUBGROUP_DFS_ComputeFlatTreeBoxesAndMasks( uniform local struct FlatTree* flat_tree,
                                                uniform local struct FlatTreeScheduler* flat_scheduler,
                                                uniform local struct AABB3f* boxes,
                                                uniform local struct PrimRefMeta* primref_meta,
                                                uniform global struct AABB* primref_buffer,
                                                uniform local uchar* masks,
                                                bool need_masks )

{
    uniform int num_levels = (int) flat_scheduler->num_levels;
    varying ushort lane = get_sub_group_local_id();

    // iterate over depth levels in the tree... deepest to shallowest
    for (uniform int level = num_levels - 1; level >= 0; level--)
    {
        // loop over a range of flattree nodes at this level, one node per sub-group
        // TODO_OPT:  Try  and enable this code to process two nodes in a SIMD16 subgroup
        uniform ushort level_start      = flat_scheduler->level_start[level];
        uniform ushort level_node_count = flat_scheduler->level_count[level];
        
        for (uniform ushort i = get_sub_group_id(); i < level_node_count; i += get_num_sub_groups())
        {
            uniform ushort node_index = flat_scheduler->level_ordered_nodes[ level_start + i ];

            varying struct AABB box;
            AABB_init(&box);

            uniform uint child_base   = FlatTree_GetFirstChild( flat_tree, node_index );
            uniform uint num_children = FlatTree_GetNumChildren( flat_tree, node_index );
            varying uint child_index  = child_base + ((lane<num_children)?lane : 0);

            varying uint mask = 0xff;
            if (FlatTree_IsLeafNode( flat_tree, node_index ))
            {
                // fetch AABBs for primrefs               
                box = primref_buffer[ PrimRefMeta_GetInputIndex( &primref_meta[child_index] ) ];      
                if( need_masks )
                    mask = PRIMREF_instanceMask(&box);
            }
            else
            {
                // fetch AABBs for child nodes                
                box.lower.xyz = AABB3f_load_lower( &boxes[child_index] );
                box.upper.xyz = AABB3f_load_upper( &boxes[child_index] );
                if ( need_masks )
                    mask = masks[child_index];                
            }


            // reduce and write box
            box = AABB_sub_group_reduce_N6( &box );
            if( lane == 0 )
                AABB3f_set( &boxes[node_index], box.lower.xyz, box.upper.xyz );

            if( need_masks )
            {
                mask = sub_group_reduce_or_N6(mask);
                masks[node_index] = mask;
            }

        }

        barrier( CLK_LOCAL_MEM_FENCE );
    }
}


void SUBGROUP_DFS_WriteNodes( 
    uniform local struct FlatTree* flat_tree,
    uniform local struct AABB3f* boxes,
    uniform local struct PrimRefMeta* primref_meta,
    uniform struct DFSArgs args, 
    uniform local uchar* masks
    )

{
    uniform uint num_nodes = FlatTree_GetNodeCount(flat_tree);
    
    for ( uniform uint i = get_sub_group_id(); i < num_nodes; i += get_num_sub_groups() )
    {
        SUBGROUP_WriteQBVHNode( flat_tree, primref_meta, boxes, i, args, masks );       
    }

}




struct Single_WG_build_SLM
{
    struct FlatTree           flat_tree;  
    struct FlatTreeScheduler  flat_scheduler; 
    struct PrimRefMeta primitive_meta[DFS_WG_SIZE];

    union
    {
        struct{   
            struct PrimRefSet         prim_refs;           
            struct LocalBVH2          bvh2;  
            struct BVHBuildLocals     bvh2_locals;
        } s1;

        struct {
            struct AABB3f boxes[DFS_MAX_FLATTREE_NODES];
            uchar masks[DFS_MAX_FLATTREE_NODES];
        } s2;
    } u;

};


GRL_INLINE void execute_single_WG_build( 
        struct DFSArgs args,    
        local struct Single_WG_build_SLM* slm
    )
{
    
    ushort tid = get_local_id( 0 );
    
    //
    // Initialize the various SLM structures.  Different sub-groups take different init paths.
    //    NOTE: even numbered subgroups here to avoid the fused-EU serialization bug
    //
    if ( get_sub_group_id() == 0 )
        SUBGROUP_FlatTree_Initialize( &slm->flat_tree, args );
    else if ( get_sub_group_id() == 2 )
        SUBGROUP_LocalBVH2_Initialize( &slm->u.s1.bvh2, args.num_primrefs );
    else if ( get_sub_group_id() == 4 )
        SUBGROUP_FlatTreeScheduler_Initialize( &slm->flat_scheduler );
    else if ( get_sub_group_id() == 6 )
        SUBGROUP_PrimRefSet_Initialize( &slm->u.s1.prim_refs );

    barrier( CLK_LOCAL_MEM_FENCE );

    // load the PrimRefs  
    DFS_CreatePrimRefSet( args, &slm->u.s1.prim_refs );
   
    // build the BVH2
    DFS_ConstructBVH2( &slm->u.s1.bvh2, &slm->u.s1.prim_refs, args.num_primrefs, &slm->u.s1.bvh2_locals );
   
    // copy out metadata for primrefs now that they have been sorted
    if( tid < args.num_primrefs )
    {
        slm->primitive_meta[tid] = PrimRefSet_GetMeta( &slm->u.s1.prim_refs, tid );
    }
    barrier( CLK_LOCAL_MEM_FENCE );

    // collapse into a FlatTree
    SUBGROUP_DFS_BuildFlatTree( &slm->u.s1.bvh2, &slm->flat_tree, &slm->flat_scheduler );

    // allocate output QBVH6 nodes
    if ( get_local_id( 0 ) == 0 )
        FlatTree_AllocateQNodes( &slm->flat_tree, args );

    barrier( CLK_LOCAL_MEM_FENCE );

    SUBGROUP_DFS_ComputeFlatTreeBoxesAndMasks( &slm->flat_tree, &slm->flat_scheduler, &slm->u.s2.boxes[0], slm->primitive_meta, args.primref_buffer, slm->u.s2.masks, args.need_masks );
    
    //FlatTree_Printf( &slm->flat_tree );
    //FlatTree_check_boxes ( &slm->flat_tree, args.primref_buffer, &slm->u.s2.boxes[0], slm->primitive_meta );

    SUBGROUP_DFS_WriteNodes( &slm->flat_tree, &slm->u.s2.boxes[0], slm->primitive_meta, args, slm->u.s2.masks );

   
    // generate sorted primref index buffer and backpointers to feed the leaf creation pipeilne
    if ( tid < args.num_primrefs )
    {
        uint input_index = PrimRefMeta_GetInputIndex(&slm->primitive_meta[tid]);

        uint bp = FlatTree_GetPrimRefBackPointer( &slm->flat_tree, tid );
        global uint* primref_back_pointers = args.primref_index_buffer + args.num_primrefs;

        args.primref_index_buffer[tid] = input_index;

        primref_back_pointers[tid] = bp / sizeof(struct QBVHNodeN);

        if ( tid == 0 && args.need_backpointers  )
        {
            *(InnerNode_GetBackPointer(BVHBase_GetBackPointers( args.bvh_base ),0)) |= ((uint)-1) << 6;
        }
    }
}




GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( DFS_WG_SIZE, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void DFS( global struct Globals* globals,
                 global char* bvh_mem,
                 global PrimRef* primref_buffer,
                 global uint* primref_index_buffer,
                 uint alloc_backpointers
                 )
{
    struct DFSArgs args;
    args.bvh_base             = (global struct BVHBase*) bvh_mem;
    args.leaf_node_type       = globals->leafPrimType;
    args.inner_node_type      = NODE_TYPE_INTERNAL;
    args.leaf_size_in_bytes   = globals->leafSize;
    args.primref_buffer       = primref_buffer;
    args.need_backpointers    = alloc_backpointers != 0;  
    args.num_primrefs         = globals->numPrimitives;
    args.primref_index_buffer = primref_index_buffer;
    args.need_masks           = args.leaf_node_type == NODE_TYPE_INSTANCE;

    if ( args.num_primrefs <= TREE_ARITY )
    {
        // TODO_OPT: This decision should be made using indirect dispatch
        if( get_sub_group_id() == 0 )
            Trivial_DFS( args );
        return;
    }

    local struct Single_WG_build_SLM slm;
   
    execute_single_WG_build( args, &slm );
}




GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( DFS_WG_SIZE, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void DFS_single_wg( 
    global struct Globals* globals,
    global char* bvh_mem,
    global PrimRef* primref_buffer,
    global uint* primref_index_buffer,
    uint sah_flags
)
{
    struct DFSArgs args;
    args.bvh_base = (global struct BVHBase*) bvh_mem;
    args.leaf_node_type = globals->leafPrimType;
    args.inner_node_type = NODE_TYPE_INTERNAL;
    args.leaf_size_in_bytes = globals->leafSize;
    args.primref_buffer = primref_buffer;
    args.need_backpointers = sah_flags & SAH_FLAG_NEED_BACKPOINTERS;
    args.num_primrefs = globals->numPrimitives;
    args.primref_index_buffer = primref_index_buffer;
    args.need_masks = sah_flags & SAH_FLAG_NEED_MASKS;

    local struct Single_WG_build_SLM slm;

    execute_single_WG_build( args, &slm );
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( (reqd_work_group_size( 16, 1, 1 )) )
__attribute__( (intel_reqd_sub_group_size( 16 )) )
kernel void DFS_trivial(
    global struct Globals* globals,
    global char* bvh_mem,
    global PrimRef* primref_buffer,
    global uint* primref_index_buffer,
    uint sah_flags
)
{
    struct DFSArgs args;
    args.bvh_base = (global struct BVHBase*) bvh_mem;
    args.leaf_node_type = globals->leafPrimType;
    args.inner_node_type = NODE_TYPE_INTERNAL;
    args.leaf_size_in_bytes = globals->leafSize;
    args.primref_buffer = primref_buffer;
    args.need_backpointers = sah_flags & SAH_FLAG_NEED_BACKPOINTERS;
    args.num_primrefs = globals->numPrimitives;
    args.primref_index_buffer = primref_index_buffer;
    args.need_masks = sah_flags & SAH_FLAG_NEED_MASKS;

    Trivial_DFS( args );
}


struct DFSArgs dfs_args_from_sah_globals( global struct SAHBuildGlobals* sah_globals )
{
    struct DFSArgs args;
    args.bvh_base               = (global struct BVHBase*) sah_globals->p_bvh_base;
    args.leaf_node_type         = sah_globals->leaf_type;
    args.inner_node_type        = NODE_TYPE_INTERNAL;
    args.leaf_size_in_bytes     = sah_globals->leaf_size;
    args.primref_buffer         = (global PrimRef*) sah_globals->p_primrefs_buffer;
    args.need_backpointers      = sah_globals->flags & SAH_FLAG_NEED_BACKPOINTERS;
    args.num_primrefs           = sah_globals->num_primrefs;
    args.primref_index_buffer   = (global uint*) sah_globals->p_primref_index_buffers;
    args.need_masks             = sah_globals->flags & SAH_FLAG_NEED_MASKS;

    return args;
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(DFS_WG_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void DFS_single_wg_batchable(
    global struct SAHBuildGlobals* globals_buffer,
    global struct VContextScheduler* scheduler
)
{
    global struct SAHBuildGlobals* sah_globals = globals_buffer + scheduler->num_trivial_builds + get_group_id(0);

    struct DFSArgs args = dfs_args_from_sah_globals( sah_globals );
    
    local struct Single_WG_build_SLM slm;

    execute_single_WG_build(args, &slm);
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
kernel void DFS_trivial_batchable(
    global struct SAHBuildGlobals* globals_buffer
)
{
    global struct SAHBuildGlobals* sah_globals = globals_buffer + get_group_id(0);

    struct DFSArgs args = dfs_args_from_sah_globals(sah_globals);

    Trivial_DFS(args);
}