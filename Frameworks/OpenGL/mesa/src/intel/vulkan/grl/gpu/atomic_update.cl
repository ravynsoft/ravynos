//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "GRLGen12.h"

#include "bvh_build_refit.h"
#include "bvh_build_treelet_refit.h"


struct RefitScratch
{
    float lower[3];
    uint mask;
    float upper[3];
    uint _pad;

};

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(64, 1, 1))) void kernel
init_refit_scratch(
    global struct BVHBase* bvh,
    global struct RefitScratch* scratch )
{
    uint tid = get_local_id(0) + get_group_id(0)*get_local_size(0);

    if ( tid < BVHBase_GetNumInternalNodes(bvh) )
    {        
        float4 v = (float4) (FLT_MAX,FLT_MAX,FLT_MAX,0);        
        store_uint4_L1WB_L3WB( (global uint4*) &scratch[tid], 0, as_uint4(v) );
        store_uint4_L1WB_L3WB( (global uint4*) &scratch[tid], 1, as_uint4(v) );
    }
}

bool is_fat_leaf( InternalNode* curNode )
{
    return curNode->nodeType != BVH_INTERNAL_NODE; // TODO:  Not enough for traversal shaders!! if ts enabled need to check child types
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(64, 1, 1))) void kernel
build_fatleaf_table(
    global struct BVHBase* bvh )
{
    uint tid = get_local_id(0) + get_group_id(0)*get_local_size(0);

    if ( tid < BVHBase_GetNumInternalNodes(bvh) )
    {
        InternalNode* curNode = BVHBase_GetInternalNodes(bvh)+tid;

        if ( is_fat_leaf(curNode) )
        {
            uint offs = atomic_inc_global( &bvh->fatLeafCount );
    
            BackPointers* backPointers = BVHBase_GetBackPointers(bvh);
            uint bp = *InnerNode_GetBackPointer(backPointers, tid);
            
            LeafTableEntry* leaf   = BVHBase_GetFatLeafTable(bvh)+offs;
            leaf->backpointer      = bp;
            leaf->inner_node_index = tid;
            leaf->leaf_index       = (BVH_ROOT_NODE_OFFSET/64) + tid + curNode->childOffset - bvh->quadLeafStart;
        }
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(64, 1, 1))) void kernel
build_fatleaf_table_new_update(
    global struct Globals *globals,
    global struct BVHBase* bvh )
{
    uint tid = get_local_id(0) + get_group_id(0)*get_local_size(0);

    if ( tid < BVHBase_GetNumInternalNodes(bvh) )
    {
        InternalNode* curNode = BVHBase_GetInternalNodes(bvh)+tid;

        if ( is_fat_leaf(curNode) )
        {
            // This implementation uses fatleaf table structure but it is actually quad table
            // Also tested implementation that process 2 fatleafs per SIMD line as we iterate over the children
            // but performance was worse
            BackPointers* backPointers = BVHBase_GetBackPointers(bvh);
            uint bp = *InnerNode_GetBackPointer(backPointers, tid);
            uint fatLeafTableStart = bvh->fatLeafTableStart;
            
            uint leaf_index = (BVH_ROOT_NODE_OFFSET/64) + tid + curNode->childOffset - bvh->quadLeafStart;
            uint numChildren = (bp >> 3) & 0x7;
            
            uint quad_leaf_table_index = leaf_index;
            
            // Check if num children is outside of the % 256 work group
            // If so, move these cases to the offset after numQuads and push them to the leftovers part
            // where fatleaves are stored every 8th pos with additional padding
            // This way we will not have the case in leftovers table where single fatleaf has children in 2 separate work groups
            
            uint prev_group = leaf_index & 255;
            uint next_group = (leaf_index + (numChildren - 1)) & 255;
            uint slm_pos = prev_group;
            bool is_leftover = prev_group > next_group;
            
            if(is_leftover)
            {
                LeafTableEntry* leafBase = (LeafTableEntry*)(((char*)bvh) + (64u * fatLeafTableStart + 12 * quad_leaf_table_index));
                uint numQuads_aligned_256 = (globals->numPrimitives + 255) & ~255;
            
                uint leftovers_offset = atomic_add_global( &bvh->quadLeftoversCountNewAtomicUpdate, 8 );
            
                for(uint i = 0; i < BVH_NODE_N6; i++)
                {
                    uint pos = (i < numChildren) ? i : 0;
                    LeafTableEntry* leaf_null = &leafBase[pos];
                    leaf_null->leaf_index = -1 << 3;
                }
            
                quad_leaf_table_index = numQuads_aligned_256 + leftovers_offset;
                slm_pos = leftovers_offset & 255;
            }
            
            LeafTableEntry* leaf = (LeafTableEntry*)(((char*)bvh) + (64u * fatLeafTableStart + 12 * quad_leaf_table_index));
            
            for(uint i = 0; i < BVH_NODE_N6; i++)
            {
                uint pos = (i < numChildren) ? i : 0;
                LeafTableEntry* leafCur = &leaf[pos];
                leafCur->backpointer = bp;
                leafCur->inner_node_index = (tid << 8) | slm_pos;
                leafCur->leaf_index = (leaf_index << 3) | pos;
            }
            
            // Need to clean the unused area where we pad to 8 for leftovers
            if(is_leftover)
            {
                for(uint i = 1; i < 8; i++)
                {
                    uint pos = (i >= numChildren) ? i : 7;
                    LeafTableEntry* leafCur = &leaf[pos];
                    leafCur->leaf_index = -1 << 3;
                }
            }
        }
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(64, 1, 1))) void kernel
build_innernode_table(
    global struct BVHBase* bvh )
{
    uint tid = get_local_id(0) + get_group_id(0)*get_local_size(0);

    if ( tid < BVHBase_GetNumInternalNodes(bvh) )
    {
        InternalNode* curNode = BVHBase_GetInternalNodes(bvh)+tid;

        if ( !is_fat_leaf( curNode ) )
        {
            uint offs = atomic_inc_global( &bvh->innerCount );

            BackPointers* backPointers = BVHBase_GetBackPointers(bvh);
            uint bp = *InnerNode_GetBackPointer(backPointers, tid);

            InnerNodeTableEntry* inner   = BVHBase_GetInnerNodeTable(bvh)+offs;
            inner->node_index_and_numchildren = (tid<<3) | ((bp>>3) &7);
            inner->first_child = tid + curNode->childOffset;
        }
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(256, 1, 1))) void kernel
fixup_quad_table(
    global struct BVHBase* bvh )
{
    // This kernel has 2 work groups that set the magic number for unused data in
    // fatleaf table. One work group for thelast group of the first part where quads are packed,
    // second one for the last group of the part where quads are stored padded

    uint numQuads = BVHBase_GetNumQuads(bvh);
    uint numQuadLeftovers = bvh->quadLeftoversCountNewAtomicUpdate;
    uint numQuadLeftovers_aligned_256 = (numQuadLeftovers + 255) & ~255;

    uint numQuads_aligned_256 = (numQuads + 255) & ~255;
    uint quadOffsetEnd = numQuads_aligned_256 + get_group_id(0) * numQuadLeftovers_aligned_256;
    uint quadOffsetStart = quadOffsetEnd - 256;

    uint quads_number_last_group = (get_group_id(0) == 0) ? numQuads : numQuads_aligned_256 + numQuadLeftovers;

    uint leftovers = quadOffsetEnd - quads_number_last_group;

    uint tid = get_local_id(0) > (255 - leftovers) ? get_local_id(0) : 256 - leftovers;

    if(leftovers != 0)
    {
        LeafTableEntry* leafBvh = BVHBase_GetFatLeafTable(bvh);
        
        LeafTableEntry* leaf = &leafBvh[quadOffsetStart + tid];
        leaf->leaf_index = -1 << 3;
    }

    if(get_group_id(0) == 1 && get_local_id(0) == 0)
        bvh->quadTableSizeNewAtomicUpdate = quadOffsetEnd;
}


// updates one quad leaf and gets BBOX contatining it
GRL_INLINE void refit_bottom_child_quad_WB(
    global struct QuadLeaf* quad,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    struct AABB* childAABB)
{    
    /* get the geomID and primID0/1 for both quad triangles */
    const uint geomID = PrimLeaf_GetGeoIndex(&quad->leafDesc);
    const uint primID0 = quad->primIndex0;
    const uint primID1 = primID0 + QuadLeaf_GetPrimIndexDelta(quad);
    ushort fourth_vert = 0;

    if (primID1 != primID0)
    {
        ushort packed_indices = QuadLeaf_GetSecondTriangleIndices(quad);
        fourth_vert = ((packed_indices & 0x0C) == 0x0C) ? 1 : fourth_vert;
        fourth_vert = ((packed_indices & 0x30) == 0x30) ? 2 : fourth_vert;
    }

    global GRL_RAYTRACING_GEOMETRY_DESC* desc = geomDesc + geomID;

    uint4 indices = GRL_load_quad_indices(desc, primID0, primID1, fourth_vert);

    // read the indices of the 4 verts we want
    float3 vtx0, vtx1, vtx2, vtx3;
    GRL_load_quad_vertices(desc, &vtx0, &vtx1, &vtx2, &vtx3, indices);

    childAABB->lower.xyz = min( min( vtx0, vtx1 ), min(vtx2,vtx3) );
    childAABB->upper.xyz = max( max( vtx0, vtx1 ), max(vtx2,vtx3) );

    float4 pack0 = (float4) ( vtx0.x, vtx0.y, vtx0.z, vtx1.x );
    float4 pack1 = (float4) ( vtx1.y, vtx1.z, vtx2.x, vtx2.y );
    float4 pack2 = (float4) ( vtx2.z, vtx3.x, vtx3.y, vtx3.z );

    global uint4* dst_verts = (global uint4*) &(quad->v[0][0]);
    store_uint4_L1WB_L3WB( dst_verts, 0, as_uint4(pack0) );
    store_uint4_L1WB_L3WB( dst_verts, 1, as_uint4(pack1) );
    store_uint4_L1WB_L3WB( dst_verts, 2, as_uint4(pack2) );
}

inline uchar4 uchar4_shuffle_down( uchar4 v, uint offs )
{
    uint vi = as_uint(v);
    return as_uchar4(intel_sub_group_shuffle_down(vi,vi,offs));
}
inline uchar4 uchar4_broadcast( uchar4 v, uint offs )
{
    uint vi = as_uint(v);
    return as_uchar4(sub_group_broadcast(vi,offs));
}

GRL_INLINE void sg_InternalNode_setFields(
    struct InternalNode* node, 
    struct AABB reduced_aabb, 
    const int offset, const uint nodeType, struct AABB* input_aabb, 
    const uint numChildren, const uchar nodeMask )
{
    const float up = 1.0f + ulp;
    const float down = 1.0f - ulp;

    struct AABB conservative_aabb = conservativeAABB(&reduced_aabb);
    const float3 org = conservative_aabb.lower.xyz;
    
    const float3 len = AABB_size(&conservative_aabb).xyz * up;
    int3 exp;
    const float3 mant = frexp_vec3(len, &exp);
    exp += (mant > (float3)QUANT_MAX_MANT ? (int3)1 : (int3)0);

    uchar4 lower_uchar = 0x80;
    uchar4 upper_uchar = 0;

    ushort lane = get_sub_group_local_id();
    ushort simd8_id     = lane/8;
    ushort logical_lane = lane%8;

    if( logical_lane < numChildren )
    {
        struct AABB child_aabb = conservativeAABB( input_aabb ); // conservative ???

        float3 lower = floor( bitShiftLdexp3( (child_aabb.lower.xyz - org) * down, -exp + 8 ) );
        lower = clamp( lower, (float)(QUANT_MIN), (float)(QUANT_MAX) );
        float3 upper = ceil( bitShiftLdexp3( (child_aabb.upper.xyz - org) * up, -exp + 8 ) );
        upper = clamp( upper, (float)(QUANT_MIN), (float)(QUANT_MAX) );
        lower_uchar.xyz = convert_uchar3_rtn( lower );
        upper_uchar.xyz = convert_uchar3_rtp( upper );
    }

    uchar4 lo0 = lower_uchar;
    uchar4 lo1 = uchar4_shuffle_down( lower_uchar, 1 );
    uchar4 lo2 = uchar4_shuffle_down( lower_uchar, 2 );
    uchar4 lo3 = uchar4_shuffle_down( lower_uchar, 3 );
    uchar4 lo4 = uchar4_shuffle_down( lower_uchar, 4 );
    uchar4 lo5 = uchar4_shuffle_down( lower_uchar, 5 );

    uchar4 hi0 = upper_uchar;
    uchar4 hi1 = uchar4_shuffle_down( upper_uchar,1 );
    uchar4 hi2 = uchar4_shuffle_down( upper_uchar,2 );
    uchar4 hi3 = uchar4_shuffle_down( upper_uchar,3 );
    uchar4 hi4 = uchar4_shuffle_down( upper_uchar,4 );
    uchar4 hi5 = uchar4_shuffle_down( upper_uchar,5 );

    if( logical_lane == 0 )
    {
        uchar childBlockStride = 0x01 + (uint)(nodeType == NODE_TYPE_INSTANCE);

        uint4 block0 = (uint4)(as_uint(org.x), as_uint(org.y), as_uint(org.z), offset);

        char3 exp_char = (char3)(exp.x,exp.y,exp.z);

        uint4 block1 = (uint4)(
            as_uint((uchar4)(nodeType, 0 /* padding */, exp_char.x, exp_char.y)),
            as_uint((uchar4)(exp_char.z, nodeMask, childBlockStride, childBlockStride)) ,
            as_uint((uchar4)(childBlockStride, childBlockStride, childBlockStride, childBlockStride)) ,
            as_uint((uchar4)(lo0.x,lo1.x,lo2.x,lo3.x))
        );
 
        uint4 block2 = (uint4)(
            as_uint((uchar4)(lo4.x,lo5.x,hi0.x,hi1.x)) ,
            as_uint((uchar4)(hi2.x,hi3.x,hi4.x,hi5.x)) ,
            as_uint((uchar4)(lo0.y,lo1.y,lo2.y,lo3.y)) ,
            as_uint((uchar4)(lo4.y,lo5.y,hi0.y,hi1.y)) 
            );

        uint4 block3 = (uint4)(
            as_uint((uchar4)(hi2.y,hi3.y,hi4.y,hi5.y)),
            as_uint((uchar4)(lo0.z,lo1.z,lo2.z,lo3.z)),
            as_uint((uchar4)(lo4.z,lo5.z,hi0.z,hi1.z)),
            as_uint((uchar4)(hi2.z,hi3.z,hi4.z,hi5.z))
            );

        global uint4* pNode = (global uint4*)node;

#if 0
        printf(
            "block0 = %08x,%08x,%08x,%08x    %08x,%08x,%08x,%08x \n"
            "block1 = %08x,%08x,%08x,%08x    %08x,%08x,%08x,%08x \n"
            "block2 = %08x,%08x,%08x,%08x    %08x,%08x,%08x,%08x \n"
            "block3 = %08x,%08x,%08x,%08x    %08x,%08x,%08x,%08x \n" ,
            block0.x,block0.y,block0.z,block0.w, 
            pNode[0].x, pNode[0].y, pNode[0].z, pNode[0].w,
            block1.x,block1.y,block1.z,block1.w, 
            pNode[1].x, pNode[1].y, pNode[1].z, pNode[1].w,
            block2.x,block2.y,block2.z,block2.w, 
            pNode[2].x, pNode[2].y, pNode[2].z, pNode[2].w ,
            block3.x,block3.y,block3.z,block3.w, 
            pNode[3].x, pNode[3].y, pNode[3].z, pNode[3].w );
#endif

         store_uint4_L1WB_L3WB( pNode, 0, block0 );
         store_uint4_L1WB_L3WB( pNode, 1, block1 );
         store_uint4_L1WB_L3WB( pNode, 2, block2 );
         store_uint4_L1WB_L3WB( pNode, 3, block3 );
    }

}



GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(256, 1, 1))) 
void kernel
traverse_aabbs_quad(
        global struct BVHBase* bvh,
        global struct RefitScratch* scratch,
        global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc
    )
{

    uniform uint num_nodes = BVHBase_GetNumInternalNodes(bvh);
    varying ushort lane = get_sub_group_local_id();

    uniform uint num_leaves = bvh->fatLeafCount;

    local struct RefitScratch local_scratch[256];
    if( get_local_id(0) < min(num_nodes,256u) )
    {
        for( uint i=0; i<3; i++ ){
            local_scratch[get_local_id(0)].lower[i] = FLT_MAX;
            local_scratch[get_local_id(0)].upper[i] = FLT_MAX;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    
    ushort SIMD8_PER_SG   = get_sub_group_size()/8;
    ushort SIMD8_PER_WG   = get_num_sub_groups()*SIMD8_PER_SG;
    ushort simd8_local_id = get_sub_group_local_id()/8;
    ushort simd8_id       = get_sub_group_id()*SIMD8_PER_SG + simd8_local_id; 
    ushort logical_lane   = lane%8;

    uniform uint fatleaf_index = simd8_id + get_group_id(0)*SIMD8_PER_WG;


    if ( fatleaf_index < num_leaves )
    {            
        LeafTableEntry* leaf = BVHBase_GetFatLeafTable(bvh)+fatleaf_index;
        uint innerNodeIdx = leaf->inner_node_index;
        uint bp           = leaf->backpointer;
        uint leaf_index   = leaf->leaf_index;

        varying InternalNode* curNode = BVHBase_GetInternalNodes(bvh)+innerNodeIdx;
        varying QuadLeaf* quad =  BVHBase_GetQuadLeaves(bvh) + leaf_index;

        uint childOffs = (((char*)quad) - ((char*)curNode))/64;

        varying struct AABB childrenBox;
        AABB_init(&childrenBox);

        uint numChildren = (bp >> 3) & 0x7;
        if (logical_lane < numChildren)
        {    
            refit_bottom_child_quad_WB(  
                (global struct QuadLeaf*) &quad[logical_lane],
                geomDesc,
                &childrenBox );
        }

        struct AABB reduce_bounds0 = AABB_sub_group_reduce_N6(&childrenBox);
        struct AABB reduce_bounds = AABB_sub_group_broadcast(&reduce_bounds0,0);
        for (uint i = 1; i < SIMD8_PER_SG; i++)
        {
            struct AABB reduce_bounds1 = AABB_sub_group_broadcast(&reduce_bounds0, 8*i);
            int3 is_upper_lane = ((uint3)(i)) == simd8_local_id;
            reduce_bounds.lower.xyz = select( reduce_bounds.lower.xyz, reduce_bounds1.lower.xyz, is_upper_lane );
            reduce_bounds.upper.xyz = select( reduce_bounds.upper.xyz, reduce_bounds1.upper.xyz, is_upper_lane );
        }

        sg_InternalNode_setFields(
            curNode, 
            reduce_bounds,
            childOffs,
            NODE_TYPE_QUAD,
            &childrenBox,
            numChildren,
            0xff );

        // atomic min operation vectorized across 6 lanes
        //    [ lower.xyz ][-][upper.xyz][-]
        //
        // Lanes 3 and 7 are inactive.   'upper' is negated
        bool atomic_mask = (1<<logical_lane) & 0x77;

        uint lmod = logical_lane % 4;
        uint ldiv = logical_lane / 4;
        float vlo = reduce_bounds.lower.x;
        float vhi = reduce_bounds.upper.x;
        vlo = (lmod == 1) ? reduce_bounds.lower.y : vlo;
        vhi = (lmod == 1) ? reduce_bounds.upper.y : vhi;
        vlo = (lmod == 2) ? reduce_bounds.lower.z : vlo;
        vhi = (lmod == 2) ? reduce_bounds.upper.z : vhi;
        float v = (ldiv == 0) ? vlo : -vhi;


        global float* pv = (global float*) &scratch[innerNodeIdx];

        store_uint_L1WB_L3WB( (global uint*)(pv+logical_lane), 0, as_uint(v));

        BackPointers* backPointers = BVHBase_GetBackPointers(bvh);
        uint parent = (bp >> 6);

        // check for parent != 0x03FFFFFF once to be sure we don't enter parent >= 256
        if(atomic_mask && parent != 0x03FFFFFF)
        {
            while( parent >= 256 )
            {
                innerNodeIdx = parent;
                bp =  *InnerNode_GetBackPointer(backPointers, innerNodeIdx);
                atomic_min( ((global float*) &(scratch[innerNodeIdx]))+logical_lane, v );
                parent = bp >> 6;
            }
            while( parent != 0x03FFFFFF )
            {
                innerNodeIdx = parent;
                bp =  *InnerNode_GetBackPointer(backPointers, innerNodeIdx);
                atomic_min( ((local float*) &(local_scratch[innerNodeIdx]))+logical_lane, v );
                parent = bp >> 6;
            }
        }
        
    }


    barrier( CLK_LOCAL_MEM_FENCE );
    num_nodes = min(num_nodes,256u);

    local float* in = (local float*)&local_scratch[0];
    global float* out = (global float*)&scratch[0];

    for (uint i = get_local_id(0); i < num_nodes*6; i += 256 )
    {
        // since we want to save [ lower.xyz ][-][upper.xyz][-] i.e 0,1,2, 4,5,6 etc. we need to offset +1 for every triplet
        uint idx = i + (i/3);

        float v = in[idx];
        if( v != FLT_MAX )
            atomic_min( out + idx , v );
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(64, 1, 1))) 
void kernel
write_inner_nodes(
    global struct BVHBase* bvh,
    global struct RefitScratch* scratch
    )
{
    uint SIMD8_PER_SG = get_sub_group_size()/8;
    uniform uint node_id    = SIMD8_PER_SG * get_sub_group_global_id() + (get_sub_group_local_id()/8);
    varying ushort lane = get_sub_group_local_id() % 8;
    varying uint num_inners = bvh->innerCount;

    if ( node_id < num_inners )
    {
        InnerNodeTableEntry* entry = BVHBase_GetInnerNodeTable(bvh) + node_id;
        uint node_index  = entry->node_index_and_numchildren>>3;
        uint numChildren = entry->node_index_and_numchildren & 7;
        uint first_child = entry->first_child;

        varying InternalNode* curNode = BVHBase_GetInternalNodes(bvh)+node_index;

        varying struct AABB childAABB;
        AABB_init(&childAABB);

        if( lane < numChildren )
        {            
            uint child = first_child + lane;
            childAABB.lower.x = scratch[child].lower[0];
            childAABB.lower.y = scratch[child].lower[1];
            childAABB.lower.z = scratch[child].lower[2];
            childAABB.upper.x = -scratch[child].upper[0];
            childAABB.upper.y = -scratch[child].upper[1];
            childAABB.upper.z = -scratch[child].upper[2];
        }

        varying struct AABB reduce_bounds0 = AABB_sub_group_reduce_N6(&childAABB);
        struct AABB reduce_bounds = AABB_sub_group_broadcast(&reduce_bounds0,0);
        for (uint i = 1; i < SIMD8_PER_SG; i++)
        {
            struct AABB reduce_bounds1 = AABB_sub_group_broadcast(&reduce_bounds0, 8*i);
            int3 is_upper_lane = ((uint3)(i)) ==  (get_sub_group_local_id()/8);
            reduce_bounds.lower.xyz = select( reduce_bounds.lower.xyz, reduce_bounds1.lower.xyz, is_upper_lane );
            reduce_bounds.upper.xyz = select( reduce_bounds.upper.xyz, reduce_bounds1.upper.xyz, is_upper_lane );
        }
        
        sg_InternalNode_setFields(
            curNode, 
            reduce_bounds,
            first_child - node_index,
            NODE_TYPE_INTERNAL,
            &childAABB,
            numChildren,
            0xff );

    }

    if (node_id == 0 && lane == 0 )
    {
        bvh->Meta.bounds.lower[0] = scratch[0].lower[0];
        bvh->Meta.bounds.lower[1] = scratch[0].lower[1];
        bvh->Meta.bounds.lower[2] = scratch[0].lower[2];
        bvh->Meta.bounds.upper[0] = -scratch[0].upper[0];
        bvh->Meta.bounds.upper[1] = -scratch[0].upper[1];
        bvh->Meta.bounds.upper[2] = -scratch[0].upper[2];
    }

}



#if 1
#define SLM_BOX_COUNT 1024

struct AABB load_box( uint place,  local struct AABB* local_boxes, global struct AABB* extra_boxes )
{
    if( place < SLM_BOX_COUNT )
        return local_boxes[place];
    else
        return extra_boxes[place-SLM_BOX_COUNT];
}

void store_box( struct AABB box, uint place, local struct AABB* local_boxes, global struct AABB* extra_boxes )
{
    if (place < SLM_BOX_COUNT)
    {
        local_boxes[place] = box;
    }
    else
    {
        global uint4* ptr = (global uint4*)&extra_boxes[place-SLM_BOX_COUNT];
        store_uint4_L1WB_L3WB( ptr,   0, as_uint4(box.lower) );
        store_uint4_L1WB_L3WB( ptr+1, 0, as_uint4(box.upper) );
    }
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(512, 1, 1))) 
__attribute__((intel_reqd_sub_group_size(16)))
void kernel
update_single_group_quads(
    global struct BVHBase* bvh,
    global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    global struct AABB* extra_boxes
)
{
    uniform uint tid = get_sub_group_global_id();
    uniform uint num_nodes = BVHBase_GetNumInternalNodes(bvh);
    uniform uint num_leaves = bvh->fatLeafCount;
    uniform uint num_inners = bvh->innerCount;

    varying ushort lane = get_sub_group_local_id();
    
    local struct AABB local_boxes[SLM_BOX_COUNT]; // == 32KB
   
    // initialize nodes
    for (uint i = get_local_id( 0 ); i < num_nodes; i+= get_local_size(0))
    {
        struct AABB tmp;
        AABB_init(&tmp);
        tmp.upper = -tmp.upper;
        store_box( tmp, i, local_boxes, extra_boxes );
    }


    if( num_nodes > SLM_BOX_COUNT )
        mem_fence_workgroup_default();

    barrier( CLK_LOCAL_MEM_FENCE );
    

    ushort SIMD8_PER_SG   = get_sub_group_size()/8;
    ushort NUM_SIMD8      = get_num_sub_groups()*SIMD8_PER_SG;
    ushort simd8_local_id = get_sub_group_local_id()/8;
    ushort simd8_id       = get_sub_group_id()*SIMD8_PER_SG + simd8_local_id; 
    ushort logical_lane = lane%8;


    for ( uint i = simd8_id; i < num_leaves; i+= NUM_SIMD8 )
    {
        LeafTableEntry* leaf = BVHBase_GetFatLeafTable(bvh)+i;
        uint innerNodeIdx = leaf->inner_node_index;
        uint bp           = leaf->backpointer;
        uint leaf_index   = leaf->leaf_index;

        varying InternalNode* curNode = BVHBase_GetInternalNodes(bvh)+innerNodeIdx;
        QuadLeaf* quad = BVHBase_GetQuadLeaves(bvh) + leaf_index;

        uint childOffs = (((char*)quad) - ((char*)curNode))/64;

        varying struct AABB childrenBox;
        AABB_init(&childrenBox);

        uint numChildren = (bp >> 3) & 0x7;
        if (logical_lane < numChildren)
        {                
            
            refit_bottom_child_quad_WB(  
                (global struct QuadLeaf*) &quad[logical_lane],
                geomDesc,
                &childrenBox );
        }

        struct AABB reduce_bounds0 = AABB_sub_group_reduce_N6(&childrenBox);
        struct AABB reduce_bounds = AABB_sub_group_broadcast(&reduce_bounds0,0);
        for (uint i = 1; i < SIMD8_PER_SG; i++)
        {
            struct AABB reduce_bounds1 = AABB_sub_group_broadcast(&reduce_bounds0, 8*i);
            int3 is_upper_lane = ((uint3)(i)) == simd8_local_id;
            reduce_bounds.lower.xyz = select( reduce_bounds.lower.xyz, reduce_bounds1.lower.xyz, is_upper_lane );
            reduce_bounds.upper.xyz = select( reduce_bounds.upper.xyz, reduce_bounds1.upper.xyz, is_upper_lane );
        }

       
        if( logical_lane == 0 )
        {
            struct AABB negated = reduce_bounds;
            negated.upper = -negated.upper;
            store_box( negated, innerNodeIdx, local_boxes, extra_boxes );
        }

        sg_InternalNode_setFields(
            curNode, 
            reduce_bounds,
            childOffs,
            NODE_TYPE_QUAD,
            &childrenBox,
            numChildren,
            0xff );

    
        // atomic min operation vectorized across 6 lanes
        //    [ lower.xyz ][-][upper.xyz][-]
        //
        // Lanes 3 and 7 are inactive.   'upper' is negated
        uint lmod = logical_lane % 4;
        uint ldiv = logical_lane / 4;
        float vlo = reduce_bounds.lower.x;
        float vhi = reduce_bounds.upper.x;
        vlo = (lmod == 1) ? reduce_bounds.lower.y : vlo;
        vhi = (lmod == 1) ? reduce_bounds.upper.y : vhi;
        vlo = (lmod == 2) ? reduce_bounds.lower.z : vlo;
        vhi = (lmod == 2) ? reduce_bounds.upper.z : vhi;
        float v = (ldiv == 0) ? vlo : -vhi;
        bool atomic_mask = (1<<logical_lane) & 0x77;

        BackPointers* backPointers = BVHBase_GetBackPointers(bvh);
        uint parent = (bp >> 6);

        // check for parent != 0x03FFFFFF once to be sure we don't enter parent >= SLM_BOX_COUNT
        if(atomic_mask && parent != 0x03FFFFFF)
        {
            while( parent >= SLM_BOX_COUNT )
            {
                innerNodeIdx = parent;
                bp =  *InnerNode_GetBackPointer(backPointers, innerNodeIdx);
                atomic_min( ((global float*) &(extra_boxes[innerNodeIdx-SLM_BOX_COUNT]))+logical_lane, v );
                parent = bp >> 6;
            }
            while( parent != 0x03FFFFFF )
            {
                innerNodeIdx = parent;
                bp =  *InnerNode_GetBackPointer(backPointers, innerNodeIdx);
                atomic_min( ((local float*) &(local_boxes[innerNodeIdx]))+logical_lane, v );
                parent = bp >> 6;
            }
        }

    }

    if( num_nodes > SLM_BOX_COUNT )
        mem_fence_workgroup_default();

    barrier( CLK_LOCAL_MEM_FENCE );

    for ( uint i = simd8_id; i < num_inners; i+= NUM_SIMD8 )
    {
        InnerNodeTableEntry* inner = BVHBase_GetInnerNodeTable(bvh) + i;
        uint node_index  = inner->node_index_and_numchildren>>3;
        uint numChildren = inner->node_index_and_numchildren & 7;
        uint first_child = inner->first_child;
        
        varying InternalNode* curNode = BVHBase_GetInternalNodes(bvh)+ node_index;

        //if (curNode->nodeType == BVH_INTERNAL_NODE) // TODO: Needs updating for traversal shaders
        {                                           // TODO: Consider using an inner node table or UC load to avoid polluting LSC with these reads
            uint child = first_child + logical_lane;

            bool child_valid = (logical_lane < numChildren);
            
            struct AABB childAABB;
            AABB_init(&childAABB);
            if (child_valid)
            {
                childAABB = load_box( child, local_boxes, extra_boxes );
                childAABB.upper = -childAABB.upper;
            }

            varying struct AABB reduce_bounds0 = AABB_sub_group_reduce_N6(&childAABB);
            struct AABB reduce_bounds = AABB_sub_group_broadcast(&reduce_bounds0,0);
            for (uint i = 1; i < SIMD8_PER_SG; i++)
            {
                struct AABB reduce_bounds1 = AABB_sub_group_broadcast(&reduce_bounds0, 8*i);
                int3 is_upper_lane = ((uint3)(i)) ==  (get_sub_group_local_id()/8);
                reduce_bounds.lower.xyz = select( reduce_bounds.lower.xyz, reduce_bounds1.lower.xyz, is_upper_lane );
                reduce_bounds.upper.xyz = select( reduce_bounds.upper.xyz, reduce_bounds1.upper.xyz, is_upper_lane );
            }

            sg_InternalNode_setFields(
                curNode, 
                reduce_bounds,
                first_child - node_index,
                NODE_TYPE_INTERNAL,
                &childAABB,
                numChildren,
                0xff );
        }
    }


    if (get_sub_group_id() == 0 && lane == 0 )
    {
        bvh->Meta.bounds.lower[0] = local_boxes[0].lower.x;
        bvh->Meta.bounds.lower[1] = local_boxes[0].lower.y;
        bvh->Meta.bounds.lower[2] = local_boxes[0].lower.z;
        bvh->Meta.bounds.upper[0] = -local_boxes[0].upper.x;
        bvh->Meta.bounds.upper[1] = -local_boxes[0].upper.y;
        bvh->Meta.bounds.upper[2] = -local_boxes[0].upper.z;
    }

}
#endif

GRL_INLINE void traverse_aabbs_new_update_func(
        global struct BVHBase* bvh,
        global char* vertices,
        global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
        global struct RefitScratch* scratch,
        uint vertex_format,
        local struct AABB3f* children_AABBs,
        local uint* num_fat_leaves,
        local struct LeafTableEntry* leafTable_local,
        const bool single_geo
    )
{
    // The first part of the kernel with vertices loads/stores is executed with quad per work item,
    // using previously prepared QuadDataIndices to get the quad data and vert indices
    // Second part of the kernel that does the reduction, update fatleaf ain bvh and bottom up is
    // executed per simd.
    // For bottom up tested also with local part (using local scratch) but since there is not enough SLM additional
    // barriers were needed to clean and reuse SLM, which curretnly kills performance. Could be worth to revisit
    // on future gens.

    varying uint lid = get_local_id(0);
    varying uint tid = lid + get_group_id(0)*get_local_size(0);

    num_fat_leaves[0] = 0;
    leafTable_local[lid].leaf_index = -1 << 3;

    LeafTableEntry* leaf = (LeafTableEntry*)(((char*)bvh) + (64u * bvh->fatLeafTableStart + 12 * tid));
    uint innerNodeIdx_mem = leaf->inner_node_index;
    uint bp           = leaf->backpointer;
    uint leaf_index_mem = leaf->leaf_index;

    uint numChildren = (bp >> 3) & 0x7;

    uint leaf_index = leaf_index_mem >> 3;
    uint slm_child_offset = leaf_index_mem & 0x7;

    uint innerNodeIdx = innerNodeIdx_mem >> 8;
    uint slm_pos_main = innerNodeIdx_mem & 0xFF;

    uint first_el_of_group = get_group_id(0)*get_local_size(0);
    uint quadsNum = BVHBase_GetNumQuads(bvh);
    uint expected_tid = first_el_of_group < quadsNum ? first_el_of_group : quadsNum - 1;

    // Skip writes when not all children for single fatleaf are present in this work group
    bool skip_tid = leaf_index == 0x1FFFFFFF;
    leaf_index = skip_tid ? expected_tid : leaf_index;

    // Compute bounding box for quads
    varying struct AABB3f childrenBox;
        
    tid = leaf_index + slm_child_offset;
    
    // Read vertex indices and quad header from separate buffer
    uint quadIndicesStart = bvh->quadIndicesDataStart;
    varying struct QuadDataIndices* vertex_indice_ptr = (QuadDataIndices*)(((char*)bvh) + (64u * quadIndicesStart + 32 * tid));
    QuadDataIndices vertexMap = vertex_indice_ptr[0];
    
    varying global uint4* bounds =  (global uint4*)((char*)bvh + (64*bvh->quadLeafStart + 64*tid) );
    uint4 quad_data = (uint4)(vertexMap.header_data[0], vertexMap.header_data[1], vertexMap.header_data[2], vertexMap.header_data[3]);
    uint4 indices = (uint4)(vertexMap.vert_idx[0], vertexMap.vert_idx[1], vertexMap.vert_idx[2], vertexMap.vert_idx[3]);
    
    global GRL_RAYTRACING_GEOMETRY_DESC* desc = geomDesc;

    if(!single_geo)
    {
        uint geomID = vertexMap.header_data[0] & 0xFFFFFF;
        desc += geomID;
        vertices = (global char*)desc->Desc.Triangles.pVertexBuffer;
        vertex_format = desc->Desc.Triangles.VertexFormat;
    }

    float3 vtx0, vtx1, vtx2, vtx3;
    GRL_load_quad_vertices_no_stride(desc, &vtx0, &vtx1, &vtx2, &vtx3, indices, vertex_format, vertices);
    
    for(uint i = 0; i < 3; i++)
        childrenBox.lower[i] = min( min( vtx0[i], vtx1[i] ), min(vtx2[i],vtx3[i]) );

    for(uint i = 0; i < 3; i++)
        childrenBox.upper[i] = max( max( vtx0[i], vtx1[i] ), max(vtx2[i],vtx3[i]) );
    
    float4 pack0 = (float4) ( vtx0.x, vtx0.y, vtx0.z, vtx1.x );
    float4 pack1 = (float4) ( vtx1.y, vtx1.z, vtx2.x, vtx2.y );
    float4 pack2 = (float4) ( vtx2.z, vtx3.x, vtx3.y, vtx3.z );
    
    // Store quad data in bvh
    // Make sure this goes without partial writes to get best perf
    store_uint4_L1WB_L3WB( bounds, 0, quad_data );
    store_uint4_L1WB_L3WB( bounds, 1, as_uint4(pack0) );
    store_uint4_L1WB_L3WB( bounds, 2, as_uint4(pack1) );
    store_uint4_L1WB_L3WB( bounds, 3, as_uint4(pack2) );

    barrier( CLK_LOCAL_MEM_FENCE );
    
    struct AABB reduce_bounds;
    
    if(!skip_tid)
    {
        // Store AABB in SLM, to be used later for children quantization in fatleaf
        children_AABBs[slm_pos_main + slm_child_offset] = childrenBox;
    
        if(slm_child_offset == 0)
        {
           uint offset = atomic_inc_local(&num_fat_leaves[0]);
           leafTable_local[offset].inner_node_index = innerNodeIdx_mem;
           leafTable_local[offset].backpointer = bp;
           leafTable_local[offset].leaf_index = leaf_index_mem;
        }
    }
       
    barrier( CLK_LOCAL_MEM_FENCE );
    
    varying ushort lane   = get_sub_group_local_id();
    ushort SIMD8_PER_SG   = get_sub_group_size()/8;
    ushort SIMD8_PER_WG   = get_num_sub_groups()*SIMD8_PER_SG;
    ushort simd8_local_id = get_sub_group_local_id()/8;
    ushort simd8_id       = get_sub_group_id()*SIMD8_PER_SG + simd8_local_id; 
    ushort logical_lane   = lane%8;
    
    uint fatleaves_aligned_32 = (num_fat_leaves[0] + 31) & ~31;
    
    for(uint offset = 0; offset < fatleaves_aligned_32; offset += 32)
    {
        uniform uint fatleaf_index = simd8_id + offset;
        uint innerNodeIdx_mem = leafTable_local[fatleaf_index].inner_node_index;
        uint bp           = leafTable_local[fatleaf_index].backpointer;
        uint leaf_index_mem   = leafTable_local[fatleaf_index].leaf_index;
    
        uint numChildren = (bp >> 3) & 0x7;
        
        uint leaf_index = leaf_index_mem >> 3;
        uint slm_child_offset = leaf_index_mem & 0x7;
        
        uint innerNodeIdx = innerNodeIdx_mem >> 8;
        uint slm_pos_main = innerNodeIdx_mem & 0xFF;
        
        bool skip_tid = leaf_index == 0x1FFFFFFF;
        bool active_lane = (logical_lane < numChildren);
        uint lane_children = active_lane ? logical_lane : 0;
        
        fatleaf_index = leaf_index;
    
        varying InternalNode* curNode = (InternalNode*)(((char*)bvh) + (BVH_ROOT_NODE_OFFSET + 64 * innerNodeIdx));
        
        global struct Quad *quads = (global struct Quad *)((char*)bvh + 64*bvh->quadLeafStart );
    
        varying struct AABB childrenBox_bu;
        AABB_init(&childrenBox_bu);
        
        if(!skip_tid)
            childrenBox_bu = AABBfromAABB3f(children_AABBs[slm_pos_main + lane_children]);
    
        struct AABB reduce_bounds0 = AABB_sub_group_reduce_N6(&childrenBox_bu);
        struct AABB reduce_bounds = AABB_sub_group_broadcast(&reduce_bounds0,0);
        
        for (uint i = 1; i < SIMD8_PER_SG; i++)
        {
            struct AABB reduce_bounds1 = AABB_sub_group_broadcast(&reduce_bounds0, 8*i);
            int3 is_upper_lane = ((uint3)(i)) == simd8_local_id;
            reduce_bounds.lower.xyz = select( reduce_bounds.lower.xyz, reduce_bounds1.lower.xyz, is_upper_lane );
            reduce_bounds.upper.xyz = select( reduce_bounds.upper.xyz, reduce_bounds1.upper.xyz, is_upper_lane );
        }

        if(!skip_tid)
        {
            uint quad_offset = 64u * bvh->quadLeafStart + 64 * fatleaf_index;
            varying QuadLeaf* quad =  (QuadLeaf*)(((char*)bvh) + quad_offset);
            uint childOffs = (((char*)quad) - ((char*)curNode))/64;
    
            sg_InternalNode_setFields(
            curNode, 
            reduce_bounds,
            childOffs,
            NODE_TYPE_QUAD,
            &childrenBox_bu,
            numChildren,
            0xff );
            
            bool atomic_mask = (1<<logical_lane) & 0x77;
            
            uint lmod = logical_lane % 4;
            uint ldiv = logical_lane / 4;
            float vlo = reduce_bounds.lower.x;
            float vhi = reduce_bounds.upper.x;
            vlo = (lmod == 1) ? reduce_bounds.lower.y : vlo;
            vhi = (lmod == 1) ? reduce_bounds.upper.y : vhi;
            vlo = (lmod == 2) ? reduce_bounds.lower.z : vlo;
            vhi = (lmod == 2) ? reduce_bounds.upper.z : vhi;
            float v = (ldiv == 0) ? vlo : -vhi;
            
            global float* pv = (global float*) &scratch[innerNodeIdx];
            
            store_uint_L1WB_L3WB( (global uint*)(pv+logical_lane), 0, as_uint(v));
            
            BackPointers* backPointers = BVHBase_GetBackPointers(bvh);
            uint parent = (bp >> 6);
            
            global float* parent_v = (global float*) &(scratch[parent]) + logical_lane;
            
            if(atomic_mask && (*parent_v >= v) && (parent != 0x03FFFFFF))
            {
                innerNodeIdx = parent;
                bp =  *InnerNode_GetBackPointer(backPointers, innerNodeIdx);
                atomic_min( parent_v, v );
                parent = bp >> 6;
            
                if(parent != 0x03FFFFFF)
                {
                    while( parent != 0x03FFFFFF )
                    {
                        innerNodeIdx = parent;
                        bp =  *InnerNode_GetBackPointer(backPointers, innerNodeIdx);
                
                        global float* parent_v_global = (global float*) &(scratch[innerNodeIdx]) + logical_lane;
                        if(*parent_v_global >= v)
                            atomic_min( parent_v_global, v );
                        else
                            break;
                
                        parent = bp >> 6;
                    }
                }
            }
        }
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(256, 1, 1))) 
__attribute__( (intel_reqd_sub_group_size( 16 )) )
void kernel
traverse_aabbs_new_update(
        global struct BVHBase* bvh,
        global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
        global struct RefitScratch* scratch
    )
{
    varying uint lid = get_local_id(0);
    varying uint tid = lid + get_group_id(0)*get_local_size(0);

    local struct AABB3f children_AABBs[256];
    local struct LeafTableEntry leafTable_local[256];
    local uint num_fat_leaves;

    traverse_aabbs_new_update_func(bvh, (global char*)geomDesc /* not used */, geomDesc, scratch, (uint)-1 /* not used */,
        &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], false);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(256, 1, 1))) 
__attribute__( (intel_reqd_sub_group_size( 16 )) )
void kernel
traverse_aabbs_new_update_single_geo(
        global struct BVHBase* bvh,
        global char* vertices,
        global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
        global struct RefitScratch* scratch,
        const uint vertex_format
    )
{
    varying uint lid = get_local_id(0);
    varying uint tid = lid + get_group_id(0)*get_local_size(0);

    local struct AABB3f children_AABBs[256];
    local struct LeafTableEntry leafTable_local[256];
    local uint num_fat_leaves;

    if(vertex_format == VERTEX_FORMAT_R32G32B32_FLOAT)
      traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R32G32B32_FLOAT,
          &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R32G32_FLOAT)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R32G32_FLOAT,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R16G16B16A16_FLOAT)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R16G16B16A16_FLOAT,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R16G16_FLOAT)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R16G16_FLOAT,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R16G16B16A16_SNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R16G16B16A16_SNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R16G16_SNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R16G16_SNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R16G16B16A16_UNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R16G16B16A16_UNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R16G16_UNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R16G16_UNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R10G10B10A2_UNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R10G10B10A2_UNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R8G8B8A8_UNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R8G8B8A8_UNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R8G8_UNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R8G8_UNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R8G8B8A8_SNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R8G8B8A8_SNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else if(vertex_format == VERTEX_FORMAT_R8G8_SNORM)
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, VERTEX_FORMAT_R8G8_SNORM,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
    else
        traverse_aabbs_new_update_func(bvh, vertices, geomDesc, scratch, (uint)-1,
            &children_AABBs[0], &num_fat_leaves, &leafTable_local[0], true);
}
