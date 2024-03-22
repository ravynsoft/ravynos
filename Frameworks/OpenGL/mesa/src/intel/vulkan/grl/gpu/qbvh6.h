//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "GRLGen12.h"

#include "shared.h"
#include "quad.h"

/* ====== GENERAL BVH config ====== */

#define BVH_NODE_N6 6
#define BVH_NODE_N 8
#define BVH_NODE_N_LOG 3

#define SAH_LOG_BLOCK_SHIFT 2
#define BVH_LEAF_N_MIN BVH_NODE_N6
#define BVH_LEAF_N_MAX BVH_NODE_N6

#define BVH_NODE_DEFAULT_MASK 0xff
#define BVH_NODE_DEGENERATED_MASK 0x00

/* ====== QUANTIZATION config ====== */

#define QUANT_BITS 8
#define QUANT_MIN 0
#define QUANT_MAX 255
#define QUANT_MAX_MANT (255.0f / 256.0f)

#define NO_NODE_OFFSET 0

/* ======================================================================= */
/* ============================== BVH BASE =============================== */
/* ======================================================================= */

GRL_INLINE void setBVHBaseBounds(struct BVHBase *base, struct AABB *aabb)
{
    base->Meta.bounds.lower[0] = aabb->lower.x;
    base->Meta.bounds.lower[1] = aabb->lower.y;
    base->Meta.bounds.lower[2] = aabb->lower.z;

    base->Meta.bounds.upper[0] = aabb->upper.x;
    base->Meta.bounds.upper[1] = aabb->upper.y;
    base->Meta.bounds.upper[2] = aabb->upper.z;
}

GRL_INLINE global struct QBVHNodeN *BVHBase_nodeData(struct BVHBase *bvh)
{
    return (global struct QBVHNodeN *)((void *)bvh + BVH_ROOT_NODE_OFFSET);
}

GRL_INLINE global struct QBVHNodeN *BVHBase_rootNode(struct BVHBase *bvh)
{
    return (global struct QBVHNodeN *)((void *)bvh + BVH_ROOT_NODE_OFFSET);
}

GRL_INLINE global struct Quad *BVHBase_quadLeaves(struct BVHBase *bvh)
{
    return (global struct Quad *)((void *)bvh + 64 * (ulong)bvh->quadLeafStart);
}

GRL_INLINE uint64_t BVHBase_numNodes(struct BVHBase *bvh)
{
    return bvh->nodeDataCur - BVH_ROOT_NODE_OFFSET / 64;
}

GRL_INLINE uint64_t BVHBase_numQuads(struct BVHBase *bvh)
{
    return bvh->quadLeafCur - bvh->quadLeafStart;
}

GRL_INLINE uint64_t BVHBase_numProcedurals(struct BVHBase *bvh)
{
    return bvh->proceduralDataCur - bvh->proceduralDataStart;
}

GRL_INLINE uint64_t BVHBase_numInstances(struct BVHBase *bvh)
{
    return bvh->instanceLeafEnd - bvh->instanceLeafStart;
}

/* =================================================================== */
/* ============================== QBVH =============================== */
/* =================================================================== */

__constant const float ulp = FLT_EPSILON;

GRL_INLINE struct AABB conservativeAABB(struct AABB *aabb)
{
    struct AABB box;
    const float4 v4 = max(fabs(aabb->lower), fabs(aabb->upper));
    const float v = ulp * max(v4.x, max(v4.y, v4.z));
    box.lower = aabb->lower - (float4)v;
    box.upper = aabb->upper + (float4)v;
    return box;
}

GRL_INLINE struct AABB3f conservativeAABB3f(struct AABB3f* aabb3d)
{
    struct AABB aabb4d = AABBfromAABB3f(*aabb3d);
    struct AABB box = conservativeAABB(&aabb4d);
    return AABB3fFromAABB(box);
}

struct QBVH_AABB
{
    uchar lower_x[BVH_NODE_N6];
    uchar upper_x[BVH_NODE_N6];
    uchar lower_y[BVH_NODE_N6];
    uchar upper_y[BVH_NODE_N6];
    uchar lower_z[BVH_NODE_N6];
    uchar upper_z[BVH_NODE_N6];
};

struct QBVHNodeN
{
    float lower[3];
    int offset;
    // 16 bytes
    uchar type;
    uchar pad;
    // 18 bytes
    char exp[3];
    uchar instMask;
    // 22 bytes
    uchar childData[6];
    // 28 bytes
    struct QBVH_AABB qbounds; // + 36 bytes
                              // 64 bytes
};

GRL_INLINE uint QBVHNodeN_blockIncr(struct QBVHNodeN *This, uint childID)
{
    return This->childData[childID] & 0x3;
}

GRL_INLINE uint QBVHNodeN_startPrim(struct QBVHNodeN *This, uint childID)
{
    return (This->childData[childID] >> 2) & 0xF;
}

GRL_INLINE void initQBVHNodeN(struct QBVHNodeN *qnode)
{
    uint *ptr = (uint *)qnode;
    for (uint i = 0; i < 16; i++)
        ptr[i] = 0;
}

GRL_INLINE struct AABB extractAABB_QBVHNodeN(struct QBVHNodeN *qnode, uint i)
{
    struct AABB aabb;
    const float4 base = (float4)(qnode->lower[0], qnode->lower[1], qnode->lower[2], 0.0f);
    const int4 lower_i = (int4)(qnode->qbounds.lower_x[i], qnode->qbounds.lower_y[i], qnode->qbounds.lower_z[i], 0);
    const int4 upper_i = (int4)(qnode->qbounds.upper_x[i], qnode->qbounds.upper_y[i], qnode->qbounds.upper_z[i], 0);
    const int4 exp_i = (int4)(qnode->exp[0], qnode->exp[1], qnode->exp[2], 0.0f);
    aabb.lower = base + bitShiftLdexp4(convert_float4_rtn(lower_i), exp_i - 8);
    aabb.upper = base + bitShiftLdexp4(convert_float4_rtp(upper_i), exp_i - 8);
    return aabb;
}

GRL_INLINE struct AABB getAABB_QBVHNodeN(struct QBVHNodeN *qnode)
{
    struct AABB aabb;
#if 0
  AABB_init(&aabb);
  for (uint i = 0; i < BVH_NODE_N6; i++)
  {
    struct AABB v = extractAABB_QBVHNodeN(qnode, i);
    AABB_extend(&aabb, &v);
  }
#else
    uint lower_x = qnode->qbounds.lower_x[0];
    uint lower_y = qnode->qbounds.lower_y[0];
    uint lower_z = qnode->qbounds.lower_z[0];

    uint upper_x = qnode->qbounds.upper_x[0];
    uint upper_y = qnode->qbounds.upper_y[0];
    uint upper_z = qnode->qbounds.upper_z[0];

    for (uint i = 1; i < BVH_NODE_N6; i++)
    {
        uint lx = qnode->qbounds.lower_x[i];
        uint ly = qnode->qbounds.lower_y[i];
        uint lz = qnode->qbounds.lower_z[i];

        uint ux = qnode->qbounds.upper_x[i];
        uint uy = qnode->qbounds.upper_y[i];
        uint uz = qnode->qbounds.upper_z[i];

        bool valid = lx <= ux;
        if (valid)
        {
            lower_x = min(lower_x, lx);
            lower_y = min(lower_y, ly);
            lower_z = min(lower_z, lz);

            upper_x = max(upper_x, ux);
            upper_y = max(upper_y, uy);
            upper_z = max(upper_z, uz);
        }
    }

    const float4 base = (float4)(qnode->lower[0], qnode->lower[1], qnode->lower[2], 0.0f);
    const int4 lower_i = (int4)(lower_x, lower_y, lower_z, 0);
    const int4 upper_i = (int4)(upper_x, upper_y, upper_z, 0);
    const int4 exp_i = (int4)(qnode->exp[0], qnode->exp[1], qnode->exp[2], 0.0f);
    aabb.lower = base + bitShiftLdexp4(convert_float4_rtn(lower_i), exp_i - 8);
    aabb.upper = base + bitShiftLdexp4(convert_float4_rtp(upper_i), exp_i - 8);
#endif
    return aabb;
}

GRL_INLINE struct AABB3f InternalNode_getAABB3f(struct InternalNode* node)
{
    return AABB3fFromAABB(getAABB_QBVHNodeN((struct QBVHNodeN*)node));
}

GRL_INLINE uint getNumChildren_QBVHNodeN(struct QBVHNodeN *qnode)
{
    uint children = 0;
    for (uint i = 0; i < BVH_NODE_N6; i++)
    {
        uint lx = qnode->qbounds.lower_x[i];
        uint ux = qnode->qbounds.upper_x[i];
        bool valid = lx <= ux;
        if (valid)
            children++;
    }
    return children;
}

GRL_INLINE long extractQBVHNodeN_offset(struct QBVHNodeN *qnode)
{
    return ((long)qnode->offset) << 6;
}

GRL_INLINE void *QBVHNodeN_childrenPointer(struct QBVHNodeN *qnode)
{
    const int offset = qnode->offset;
    return (void *)(qnode + offset);
}

GRL_INLINE void subgroup_setQBVHNodeN_setFields_reduced_bounds(const int offset, const uint type, struct AABB* input_aabb, const uint numChildren, const uchar mask, struct QBVHNodeN* qbvh_node, const bool degenerated, struct AABB reduced_aabb)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint k = subgroupLocalID;
    const float up = 1.0f + ulp;
    const float down = 1.0f - ulp;

    struct AABB aabb = reduced_aabb; // needs to execute with full subgroup width
    aabb = AABB_sub_group_broadcast(&aabb, 0);

    if (subgroupLocalID < BVH_NODE_N6)
    {
        struct AABB conservative_aabb = conservativeAABB(&aabb);
        const float3 len = AABB_size(&conservative_aabb).xyz * up;
        int3 exp;
        const float3 mant = frexp_vec3(len, &exp);
        const float3 org = conservative_aabb.lower.xyz;

        exp += (mant > (float3)QUANT_MAX_MANT ? (int3)1 : (int3)0);

        qbvh_node->offset = offset;
        qbvh_node->type = type;

        qbvh_node->lower[0] = org.x;
        qbvh_node->lower[1] = org.y;
        qbvh_node->lower[2] = org.z;

        qbvh_node->exp[0] = exp.x;
        qbvh_node->exp[1] = exp.y;
        qbvh_node->exp[2] = exp.z;

        qbvh_node->instMask = mask;

        uchar3 lower_uchar = (uchar3)(0x80);
        uchar3 upper_uchar = (uchar3)(0);

        if (subgroupLocalID < numChildren)
        {
            struct AABB child_aabb = conservativeAABB(input_aabb);

            float3 lower = floor(bitShiftLdexp3((child_aabb.lower.xyz - org) * down, -exp + 8));
            lower = clamp(lower, (float)(QUANT_MIN), (float)(QUANT_MAX));
            float3 upper = ceil(bitShiftLdexp3((child_aabb.upper.xyz - org) * up, -exp + 8));
            upper = clamp(upper, (float)(QUANT_MIN), (float)(QUANT_MAX));

            lower_uchar = convert_uchar3_rtn(lower);
            upper_uchar = convert_uchar3_rtp(upper);

            if (degenerated)
            {
                lower_uchar = upper_uchar = 0;
            }
        }

        qbvh_node->qbounds.lower_x[k] = lower_uchar.x;
        qbvh_node->qbounds.lower_y[k] = lower_uchar.y;
        qbvh_node->qbounds.lower_z[k] = lower_uchar.z;
        qbvh_node->qbounds.upper_x[k] = upper_uchar.x;
        qbvh_node->qbounds.upper_y[k] = upper_uchar.y;
        qbvh_node->qbounds.upper_z[k] = upper_uchar.z;

        qbvh_node->childData[k] = (type == NODE_TYPE_INSTANCE) ? 2 : 1;

#if ENABLE_CONVERSION_CHECKS == 1

        if (!(exp.x >= -128 && exp.x <= 127))
            printf("exp_x error \n");
        if (!(exp.y >= -128 && exp.y <= 127))
            printf("exp_y error \n");
        if (!(exp.z >= -128 && exp.z <= 127))
            printf("exp_z error \n");

        struct AABB child_qaabb = extractAABB_QBVHNodeN(qbvh_node, k);
        if (!AABB_subset(&child_aabb, &child_qaabb))
        {
            uint3 lower_i = convert_uint3(lower_uchar);
            uint3 upper_i = convert_uint3(upper_uchar);

            printf("\n ERROR %d\n", k);
            printf("lower %f upper %f \n lower_i %d  upper_i %d \n", lower, upper, lower_i, upper_i);
            printf("%i uncompressed \n", k);
            AABB_print(&child_aabb);
            printf("%i compressed \n", k);
            AABB_print(&child_qaabb);

            printf("%i uncompressed (as int) \n", k);
            AABB_printasInt(&child_aabb);
            printf("%i compressed (as int) \n", k);
            AABB_printasInt(&child_qaabb);

            int4 e0 = child_aabb.lower < child_qaabb.lower;
            int4 e1 = child_aabb.upper > child_qaabb.upper;
            printf("e0 %d e1 %d \n", e0, e1);
        }
#endif
    }
}

GRL_INLINE void subgroup_setQBVHNodeN_setFields(const int offset, const uint type, struct AABB* input_aabb, const uint numChildren, const uchar mask, struct QBVHNodeN* qbvh_node, const bool degenerated)
{
    struct AABB aabb = AABB_sub_group_reduce_N6(input_aabb);
    subgroup_setQBVHNodeN_setFields_reduced_bounds(offset, type, input_aabb, numChildren, mask, qbvh_node, degenerated, aabb);
}

GRL_INLINE void subgroup_setQBVHNodeN_setFields_2xSIMD8_in_SIMD16(const int offset, const uint type, struct AABB* input_aabb, const uint numChildren, const uchar mask, struct QBVHNodeN* qbvh_node, const bool degenerated, bool active_lane)
{
    const uint lane = get_sub_group_local_id() % 8;
    const uint node_in_sg = get_sub_group_local_id() / 8;
    const uint k = lane;
    const float up = 1.0f + ulp;
    const float down = 1.0f - ulp;

    struct AABB aabb = AABB_sub_group_reduce_N6(input_aabb); // needs to execute with full subgroup width
    aabb = AABB_sub_group_shuffle(&aabb, node_in_sg * 8);

    if (lane < BVH_NODE_N6 && active_lane)
    {
        struct AABB conservative_aabb = conservativeAABB(&aabb);
        const float3 len = AABB_size(&conservative_aabb).xyz * up;
        int3 exp;
        const float3 mant = frexp_vec3(len, &exp);
        const float3 org = conservative_aabb.lower.xyz;

        exp += (mant > (float3)QUANT_MAX_MANT ? (int3)1 : (int3)0);

        qbvh_node->offset = offset;
        qbvh_node->type = type;

        qbvh_node->lower[0] = org.x;
        qbvh_node->lower[1] = org.y;
        qbvh_node->lower[2] = org.z;

        qbvh_node->exp[0] = exp.x;
        qbvh_node->exp[1] = exp.y;
        qbvh_node->exp[2] = exp.z;

        qbvh_node->instMask = mask;

        uchar3 lower_uchar = (uchar3)(0x80);
        uchar3 upper_uchar = (uchar3)(0);

        if (lane < numChildren)
        {
            struct AABB child_aabb = conservativeAABB(input_aabb);

            float3 lower = floor(bitShiftLdexp3((child_aabb.lower.xyz - org) * down, -exp + 8));
            lower = clamp(lower, (float)(QUANT_MIN), (float)(QUANT_MAX));
            float3 upper = ceil(bitShiftLdexp3((child_aabb.upper.xyz - org) * up, -exp + 8));
            upper = clamp(upper, (float)(QUANT_MIN), (float)(QUANT_MAX));

            lower_uchar = convert_uchar3_rtn(lower);
            upper_uchar = convert_uchar3_rtp(upper);

            if (degenerated)
            {
                lower_uchar = upper_uchar = 0;
            }
        }

        qbvh_node->qbounds.lower_x[k] = lower_uchar.x;
        qbvh_node->qbounds.lower_y[k] = lower_uchar.y;
        qbvh_node->qbounds.lower_z[k] = lower_uchar.z;
        qbvh_node->qbounds.upper_x[k] = upper_uchar.x;
        qbvh_node->qbounds.upper_y[k] = upper_uchar.y;
        qbvh_node->qbounds.upper_z[k] = upper_uchar.z;

        qbvh_node->childData[k] = (type == NODE_TYPE_INSTANCE) ? 2 : 1;

#if ENABLE_CONVERSION_CHECKS == 1

        if (!(exp.x >= -128 && exp.x <= 127))
            printf("exp_x error \n");
        if (!(exp.y >= -128 && exp.y <= 127))
            printf("exp_y error \n");
        if (!(exp.z >= -128 && exp.z <= 127))
            printf("exp_z error \n");

        struct AABB child_qaabb = extractAABB_QBVHNodeN(qbvh_node, k);
        if (!AABB_subset(&child_aabb, &child_qaabb))
        {
            uint3 lower_i = convert_uint3(lower_uchar);
            uint3 upper_i = convert_uint3(upper_uchar);

            printf("\n ERROR %d\n", k);
            printf("lower %f upper %f \n lower_i %d  upper_i %d \n", lower, upper, lower_i, upper_i);
            printf("%i uncompressed \n", k);
            AABB_print(&child_aabb);
            printf("%i compressed \n", k);
            AABB_print(&child_qaabb);

            printf("%i uncompressed (as int) \n", k);
            AABB_printasInt(&child_aabb);
            printf("%i compressed (as int) \n", k);
            AABB_printasInt(&child_qaabb);

            int4 e0 = child_aabb.lower < child_qaabb.lower;
            int4 e1 = child_aabb.upper > child_qaabb.upper;
            printf("e0 %d e1 %d \n", e0, e1);
        }
#endif
    }
}

GRL_INLINE void subgroup_setInstanceQBVHNodeN(const int offset, struct AABB *input_aabb, const uint numChildren, struct QBVHNodeN *qbvh_node, const uint instMask)
{
    const uint subgroupLocalID = get_sub_group_local_id();

    // for degenerated (or inactive) instance ignore this box in exp, origin calculation and make its box be a point in the node origin.
    // if it becomes non_degenerated on update, tree topology will be equivalent to what it would be if we would account this degenerated node here.
    bool degenerated = (instMask == BVH_NODE_DEGENERATED_MASK);

    struct AABB aabb;
    AABB_init(&aabb);

    // if every child is degenerated (or inactive) instance, we need to init aabb with origin point
    uchar commonMask = sub_group_reduce_or_N6(instMask);
    if (subgroupLocalID < numChildren && (!degenerated || commonMask == BVH_NODE_DEGENERATED_MASK))
        aabb = *input_aabb;

    subgroup_setQBVHNodeN_setFields(offset, NODE_TYPE_INSTANCE, &aabb, numChildren, commonMask, qbvh_node, degenerated);
}


// return true if is degenerated
GRL_INLINE bool subgroup_setInstanceBox_2xSIMD8_in_SIMD16(struct AABB* input_aabb, const uint numChildren, uchar* mask, const uint instMask, bool active_lane)
{
    const uint lane = get_sub_group_local_id() % 8;

    // for degenerated (or inactive) instance ignore this box in exp, origin calculation and make its box be a point in the node origin.
    // if it becomes non_degenerated on update, tree topology will be equivalent to what it would be if we would account this degenerated node here.
    bool degenerated = (instMask == BVH_NODE_DEGENERATED_MASK);

    // if every child is degenerated (or inactive) instance, we need to init aabb with origin point
    uchar commonMask = sub_group_reduce_or_N6_2xSIMD8_in_SIMD16(instMask);
    if (active_lane)
        *mask = commonMask;

    if (active_lane && (degenerated && commonMask != BVH_NODE_DEGENERATED_MASK))
        AABB_init(input_aabb);

    return active_lane ? degenerated : false;
}

GRL_INLINE void subgroup_setInstanceQBVHNodeN_x2(const int offset, struct AABB* input_aabb, const uint numChildren, struct QBVHNodeN* qbvh_node, const uint instMask, bool active_lane)
{
    const uint lane = get_sub_group_local_id() % 8;

    // for degenerated (or inactive) instance ignore this box in exp, origin calculation and make its box be a point in the node origin.
    // if it becomes non_degenerated on update, tree topology will be equivalent to what it would be if we would account this degenerated node here.
    bool degenerated = (instMask == BVH_NODE_DEGENERATED_MASK);

    struct AABB aabb;
    AABB_init(&aabb);

    // if every child is degenerated (or inactive) instance, we need to init aabb with origin point
    uchar commonMask = sub_group_reduce_or_N6_2xSIMD8_in_SIMD16(instMask);
    if (lane < numChildren && (!degenerated || commonMask == BVH_NODE_DEGENERATED_MASK))
        aabb = *input_aabb;

    subgroup_setQBVHNodeN_setFields_2xSIMD8_in_SIMD16(offset, NODE_TYPE_INSTANCE, &aabb, numChildren, commonMask, qbvh_node, degenerated, active_lane);
}


GRL_INLINE void subgroup_setQBVHNodeN(const int offset, const uint type, struct AABB* input_aabb, const uint numChildren, struct QBVHNodeN* qbvh_node, uint mask)
{
    const uint subgroupLocalID = get_sub_group_local_id();

    struct AABB aabb;
    AABB_init(&aabb);

    if (subgroupLocalID < numChildren)
        aabb = *input_aabb;

    subgroup_setQBVHNodeN_setFields(offset, type, &aabb, numChildren, mask, qbvh_node, false);
}


GRL_INLINE void subgroup_setQBVHNodeN_x2(const int offset, const uint type, struct AABB* input_aabb, const uint numChildren, struct QBVHNodeN* qbvh_node, bool active_lane)
{
    const uint lane = get_sub_group_local_id() % 8;

    struct AABB aabb;
    AABB_init(&aabb);

    if (lane < numChildren)
        aabb = *input_aabb;

    subgroup_setQBVHNodeN_setFields_2xSIMD8_in_SIMD16(offset, type, &aabb, numChildren, BVH_NODE_DEFAULT_MASK, qbvh_node, false, active_lane);
}


GRL_INLINE void subgroup_QBVHNodeN_setBounds( uniform struct QBVHNodeN* qbvh_node, 
                                              uniform struct AABB reduced_bounds,
                                              varying struct AABB input_aabb, 
                                              uniform uint numChildren,
                                              varying ushort lane )
{
    const float up = 1.0f + ulp;
    const float down = 1.0f - ulp;

    int3 exp;

    struct AABB conservative_aabb = conservativeAABB( &reduced_bounds);
    const float3 len = AABB_size( &conservative_aabb ).xyz * up;
    const float3 mant = frexp_vec3( len, &exp );
    const float3 org = conservative_aabb.lower.xyz;

    exp += (mant > ( float3 )QUANT_MAX_MANT ? (int3)1 : (int3)0);

    qbvh_node->lower[0] = org.x;
    qbvh_node->lower[1] = org.y;
    qbvh_node->lower[2] = org.z;

    qbvh_node->exp[0] = exp.x;
    qbvh_node->exp[1] = exp.y;
    qbvh_node->exp[2] = exp.z;

    qbvh_node->instMask = 0xff;

    uchar3 lower_uchar = 0x80;
    uchar3 upper_uchar = 0;

    if ( lane < BVH_NODE_N6 )
    {
        ushort k = lane;
        if( lane < numChildren )
        {
            struct AABB child_aabb = conservativeAABB( &input_aabb ); // conservative ???

            float3 lower = floor( bitShiftLdexp3( (child_aabb.lower.xyz - org) * down, -exp + 8 ) );
            lower = clamp( lower, (float)(QUANT_MIN), (float)(QUANT_MAX) );
            float3 upper = ceil( bitShiftLdexp3( (child_aabb.upper.xyz - org) * up, -exp + 8 ) );
            upper = clamp( upper, (float)(QUANT_MIN), (float)(QUANT_MAX) );

            lower_uchar = convert_uchar3_rtn( lower );
            upper_uchar = convert_uchar3_rtp( upper );
        }

        qbvh_node->qbounds.lower_x[k] = lower_uchar.x;
        qbvh_node->qbounds.lower_y[k] = lower_uchar.y;
        qbvh_node->qbounds.lower_z[k] = lower_uchar.z;
        qbvh_node->qbounds.upper_x[k] = upper_uchar.x;
        qbvh_node->qbounds.upper_y[k] = upper_uchar.y;
        qbvh_node->qbounds.upper_z[k] = upper_uchar.z;
    }
   
}

GRL_INLINE void QBVHNodeN_setBounds(struct QBVHNodeN *qbvh_node, struct AABB *input_aabb, const uint numChildren)
{
    const float up = 1.0f + ulp;
    const float down = 1.0f - ulp;

    int3 exp;
    struct AABB aabb;
    AABB_init(&aabb);
    for (uint i = 0; i < numChildren; i++)
        AABB_extend(&aabb, &input_aabb[i]);

    struct AABB conservative_aabb = conservativeAABB(&aabb);
    const float3 len = AABB_size(&conservative_aabb).xyz * up;
    const float3 mant = frexp_vec3(len, &exp);
    const float3 org = conservative_aabb.lower.xyz;

    exp += (mant > (float3)QUANT_MAX_MANT ? (int3)1 : (int3)0);

    qbvh_node->lower[0] = org.x;
    qbvh_node->lower[1] = org.y;
    qbvh_node->lower[2] = org.z;

    qbvh_node->exp[0] = exp.x;
    qbvh_node->exp[1] = exp.y;
    qbvh_node->exp[2] = exp.z;

    qbvh_node->instMask = 0xff;

    for (uint k = 0; k < numChildren; k++)
    {
        struct AABB child_aabb = conservativeAABB(&input_aabb[k]); // conservative ???

        float3 lower = floor(bitShiftLdexp3((child_aabb.lower.xyz - org) * down, -exp + 8));
        lower = clamp(lower, (float)(QUANT_MIN), (float)(QUANT_MAX));
        float3 upper = ceil(bitShiftLdexp3((child_aabb.upper.xyz - org) * up, -exp + 8));
        upper = clamp(upper, (float)(QUANT_MIN), (float)(QUANT_MAX));

        uchar3 lower_uchar = convert_uchar3_rtn(lower);
        uchar3 upper_uchar = convert_uchar3_rtp(upper);

        qbvh_node->qbounds.lower_x[k] = lower_uchar.x;
        qbvh_node->qbounds.lower_y[k] = lower_uchar.y;
        qbvh_node->qbounds.lower_z[k] = lower_uchar.z;
        qbvh_node->qbounds.upper_x[k] = upper_uchar.x;
        qbvh_node->qbounds.upper_y[k] = upper_uchar.y;
        qbvh_node->qbounds.upper_z[k] = upper_uchar.z;

#if ENABLE_CONVERSION_CHECKS == 1
        if (!(exp.x >= -128 && exp.x <= 127))
            printf("exp_x error \n");
        if (!(exp.y >= -128 && exp.y <= 127))
            printf("exp_y error \n");
        if (!(exp.z >= -128 && exp.z <= 127))
            printf("exp_z error \n");

        struct AABB child_qaabb = extractAABB_QBVHNodeN(qbvh_node, k);
        if (!AABB_subset(&child_aabb, &child_qaabb))
        {
            uint3 lower_i = convert_uint3(lower_uchar);
            uint3 upper_i = convert_uint3(upper_uchar);

            printf("\n ERROR %d\n", k);
            printf("lower %f upper %f \n lower_i %d  upper_i %d \n", lower, upper, lower_i, upper_i);
            printf("%i uncompressed \n", k);
            AABB_print(&child_aabb);
            printf("%i compressed \n", k);
            AABB_print(&child_qaabb);

            printf("%i uncompressed (as int) \n", k);
            AABB_printasInt(&child_aabb);
            printf("%i compressed (as int) \n", k);
            AABB_printasInt(&child_qaabb);

            int4 e0 = child_aabb.lower < child_qaabb.lower;
            int4 e1 = child_aabb.upper > child_qaabb.upper;
            printf("e0 %d e1 %d \n", e0, e1);
        }
#endif
    }
    for (uint k = numChildren; k < BVH_NODE_N6; k++)
    {
        qbvh_node->qbounds.lower_x[k] = 0x80;
        qbvh_node->qbounds.lower_y[k] = 0x80;
        qbvh_node->qbounds.lower_z[k] = 0x80;
        qbvh_node->qbounds.upper_x[k] = 0;
        qbvh_node->qbounds.upper_y[k] = 0;
        qbvh_node->qbounds.upper_z[k] = 0;
    }
}

GRL_INLINE void QBVHNodeN_setChildren(struct QBVHNodeN *qbvh_node, const int offset, const uint numChildren)
{
    qbvh_node->offset = offset;
    for (uint k = 0; k < BVH_NODE_N6; k++)
        qbvh_node->childData[k] = 1;
}

GRL_INLINE void QBVHNodeN_setChildIncr1(struct QBVHNodeN *qbvh_node)
{
    for (uint k = 0; k < BVH_NODE_N6; k++)
        qbvh_node->childData[k] = 1;
}

GRL_INLINE void SUBGROUP_QBVHNodeN_setChildIncr1(struct QBVHNodeN *qbvh_node)
{
    if( get_sub_group_local_id() < BVH_NODE_N6 )
        qbvh_node->childData[get_sub_group_local_id()] = 1;
}


GRL_INLINE void QBVHNodeN_setChildIncr2(struct QBVHNodeN *qbvh_node)
{
    for (uint k = 0; k < BVH_NODE_N6; k++)
        qbvh_node->childData[k] = 2;
}

GRL_INLINE void QBVHNodeN_setType(struct QBVHNodeN *qbvh_node, const uint type)
{
    qbvh_node->type = type;
}

GRL_INLINE void setQBVHNodeN(const int offset, const uint type, struct AABB *input_aabb, const uint numChildren, struct QBVHNodeN *qbvh_node)
{
    QBVHNodeN_setType(qbvh_node, type);
    QBVHNodeN_setChildren(qbvh_node, offset, numChildren);
    QBVHNodeN_setBounds(qbvh_node, input_aabb, numChildren);
}

GRL_INLINE void printQBVHNodeN(struct QBVHNodeN *qnode)
{
    printf(" offset %d type %d \n", qnode->offset, (int)qnode->type);
    printf(" lower %f %f %f \n", qnode->lower[0], qnode->lower[1], qnode->lower[2]);
    printf(" exp %d %d %d \n", (int)qnode->exp[0], (int)qnode->exp[1], (int)qnode->exp[2]);
    printf(" instMask %d \n", qnode->instMask);

    struct AABB aabb0 = extractAABB_QBVHNodeN(qnode, 0);
    struct AABB aabb1 = extractAABB_QBVHNodeN(qnode, 1);
    struct AABB aabb2 = extractAABB_QBVHNodeN(qnode, 2);
    struct AABB aabb3 = extractAABB_QBVHNodeN(qnode, 3);
    struct AABB aabb4 = extractAABB_QBVHNodeN(qnode, 4);
    struct AABB aabb5 = extractAABB_QBVHNodeN(qnode, 5);

    printf(" lower_x %d %d %d %d %d %d %f %f %f %f %f %f\n", qnode->qbounds.lower_x[0], qnode->qbounds.lower_x[1], qnode->qbounds.lower_x[2], qnode->qbounds.lower_x[3], qnode->qbounds.lower_x[4], qnode->qbounds.lower_x[5], aabb0.lower.x, aabb1.lower.x, aabb2.lower.x, aabb3.lower.x, aabb4.lower.x, aabb5.lower.x);
    printf(" upper_x %d %d %d %d %d %d %f %f %f %f %f %f\n", qnode->qbounds.upper_x[0], qnode->qbounds.upper_x[1], qnode->qbounds.upper_x[2], qnode->qbounds.upper_x[3], qnode->qbounds.upper_x[4], qnode->qbounds.upper_x[5], aabb0.upper.x, aabb1.upper.x, aabb2.upper.x, aabb3.upper.x, aabb4.upper.x, aabb5.upper.x);

    printf(" lower_y %d %d %d %d %d %d %f %f %f %f %f %f\n", qnode->qbounds.lower_y[0], qnode->qbounds.lower_y[1], qnode->qbounds.lower_y[2], qnode->qbounds.lower_y[3], qnode->qbounds.lower_y[4], qnode->qbounds.lower_y[5], aabb0.lower.y, aabb1.lower.y, aabb2.lower.y, aabb3.lower.y, aabb4.lower.y, aabb5.lower.y);
    printf(" upper_y %d %d %d %d %d %d %f %f %f %f %f %f\n", qnode->qbounds.upper_y[0], qnode->qbounds.upper_y[1], qnode->qbounds.upper_y[2], qnode->qbounds.upper_y[3], qnode->qbounds.upper_y[4], qnode->qbounds.upper_y[5], aabb0.upper.y, aabb1.upper.y, aabb2.upper.y, aabb3.upper.y, aabb4.upper.y, aabb5.upper.y);

    printf(" lower_z %d %d %d %d %d %d %f %f %f %f %f %f\n", qnode->qbounds.lower_z[0], qnode->qbounds.lower_z[1], qnode->qbounds.lower_z[2], qnode->qbounds.lower_z[3], qnode->qbounds.lower_z[4], qnode->qbounds.lower_z[5], aabb0.lower.z, aabb1.lower.z, aabb2.lower.z, aabb3.lower.z, aabb4.lower.z, aabb5.lower.z);
    printf(" upper_z %d %d %d %d %d %d %f %f %f %f %f %f\n", qnode->qbounds.upper_z[0], qnode->qbounds.upper_z[1], qnode->qbounds.upper_z[2], qnode->qbounds.upper_z[3], qnode->qbounds.upper_z[4], qnode->qbounds.upper_z[5], aabb0.upper.z, aabb1.upper.z, aabb2.upper.z, aabb3.upper.z, aabb4.upper.z, aabb5.upper.z);
}

GRL_INLINE int encodeOffset(global char *bvh_mem, global void *parent, int global_child_offset)
{
    long global_parent_offset = (long)parent - (long)bvh_mem;
    global_parent_offset = global_parent_offset & (~(64 - 1));        // FIXME: (sw) this should not be necessary?
    int relative_offset = global_child_offset - global_parent_offset; // FIXME: this limits BVH size to 4GB
    //if ((int)relative_offset <= 0) printf("relative offset <= 0 %d global_child_offset %d global_parent_offset %d \n", relative_offset,global_child_offset,global_parent_offset);
    return relative_offset;
}

GRL_INLINE void QBVH6Node_set_offset(struct QBVHNodeN *qnode, void *children)
{
    int ofs = (struct QBVHNodeN *)children - qnode;
    qnode->offset = ofs;
}

GRL_INLINE void QBVH6Node_set_type(struct QBVHNodeN *qnode, uint type)
{
    qnode->type = type;
}

GRL_INLINE uint sortBVHChildrenIDs(uint input)
{
#if BVH_NODE_N == 8
    return sort8_descending(input);
#else
    return sort4_descending(input);
#endif
}

enum XFM_BOX_OPTION {
    XFM_BOX_NO_CLIP = 0,
    XFM_BOX_NOT_REFINED_CLIPPED = 1, //<<use clipbox, for not refined, compute bbox from children, transform after extending to one box
    XFM_BOX_NOT_REFINED_TAKE_CLIPBOX = 2 //<<use clipbox, for not refined, just transform xlipbox, don't take children boxes into account
};

#define DEB_PRINTFS 0
#ifndef FINE_TRANSFORM_NODE_BOX
#define FINE_TRANSFORM_NODE_BOX 0
#endif

GRL_INLINE struct AABB3f GRL_OVERLOADABLE compute_xfm_bbox(const float* xfm, InternalNode* pnode, enum XFM_BOX_OPTION clipOpt, const AABB3f* clipBox, float matrixTransformOverhead)
{
    AABB3f childrenbox;
#if FINE_TRANSFORM_NODE_BOX
    struct AffineSpace3f axfm = AffineSpace3f_load_row_major(xfm);
    bool computeFine = matrixTransformOverhead < 0.6f;
    computeFine = sub_group_any(computeFine);
    if (computeFine)
    {
        bool clip = clipOpt != XFM_BOX_NO_CLIP;
        InternalNode node = *pnode;

#if DEB_PRINTFS
        if (InternalNode_IsChildValid(&node, 5) && !InternalNode_IsChildValid(&node, 4))
            printf("child 5 valid && child 4 invalid\n");
        if (InternalNode_IsChildValid(&node, 4) && !InternalNode_IsChildValid(&node, 3))
            printf("child 4 valid && child 3 invalid\n");
        if (InternalNode_IsChildValid(&node, 3) && !InternalNode_IsChildValid(&node, 2))
            printf("child 3 valid && child 2 invalid\n");
        if (InternalNode_IsChildValid(&node, 2) && !InternalNode_IsChildValid(&node, 1))
            printf("child 2 valid && child 1 invalid\n");
        if (InternalNode_IsChildValid(&node, 1) && !InternalNode_IsChildValid(&node, 0))
            printf("child 1 valid && child 0 invalid\n");
#endif

#if DEB_PRINTFS
        printf("F");
#endif
        AABB3f child_bounds0 = InternalNode_GetChildAABB(&node, 0);
        AABB3f child_bounds1 = InternalNode_GetChildAABB(&node, 1);
        AABB3f child_bounds2 = InternalNode_GetChildAABB(&node, 2); 
        AABB3f child_bounds3 = InternalNode_GetChildAABB(&node, 3); 
        AABB3f child_bounds4 = InternalNode_GetChildAABB(&node, 4); 
        AABB3f child_bounds5 = InternalNode_GetChildAABB(&node, 5); 

        // we bravely assumme we will have at least 2 children here.
        if(!InternalNode_IsChildValid(&node, 2)) child_bounds2 = child_bounds0;
        if(!InternalNode_IsChildValid(&node, 3)) child_bounds3 = child_bounds0;
        if(!InternalNode_IsChildValid(&node, 4)) child_bounds4 = child_bounds0;
        if(!InternalNode_IsChildValid(&node, 5)) child_bounds5 = child_bounds0;

        if (clip)
        {
            AABB3f_trim_upper(&child_bounds0, clipBox->upper);
            AABB3f_trim_upper(&child_bounds1, clipBox->upper);
            AABB3f_trim_upper(&child_bounds2, clipBox->upper);
            AABB3f_trim_upper(&child_bounds3, clipBox->upper);
            AABB3f_trim_upper(&child_bounds4, clipBox->upper);
            AABB3f_trim_upper(&child_bounds5, clipBox->upper);
        }

        child_bounds0 = transform_aabb(child_bounds0, xfm);
        child_bounds1 = transform_aabb(child_bounds1, xfm);
        child_bounds2 = transform_aabb(child_bounds2, xfm);
        child_bounds3 = transform_aabb(child_bounds3, xfm);
        child_bounds4 = transform_aabb(child_bounds4, xfm);
        child_bounds5 = transform_aabb(child_bounds5, xfm);
        
        AABB3f_extend(&child_bounds0, &child_bounds1);
        AABB3f_extend(&child_bounds2, &child_bounds3);
        AABB3f_extend(&child_bounds4, &child_bounds5);
        AABB3f_extend(&child_bounds0, &child_bounds2);
        AABB3f_extend(&child_bounds0, &child_bounds4);

        return child_bounds0;
    }
#endif

#if DEB_PRINTFS
    printf("0");
#endif

    struct AABB3f child_bounds;

    if (clipOpt != XFM_BOX_NOT_REFINED_TAKE_CLIPBOX)
    {
        // XFM_BOX_NOT_REFINED_CLIPPED || XFM_BOX_NO_CLIP
        child_bounds = InternalNode_getAABB3f(pnode);
        if (clipOpt != XFM_BOX_NO_CLIP)
        {
            AABB3f_intersect(&child_bounds, *clipBox);
        }
    }
    else
    {
        //XFM_BOX_NOT_REFINED_TAKE_CLIPBOX
        child_bounds = *clipBox;
    }

    child_bounds = transform_aabb(child_bounds, xfm);
    //child_bounds = conservativeAABB3f(&child_bounds);
    return child_bounds;
}

GRL_INLINE AABB3f GRL_OVERLOADABLE compute_xfm_bbox(struct AffineSpace3f xfm, InternalNode* pnode, bool clip, AABB3f* clipBox, float matOverhead)
{
    float transform[12];
    load_row_major_from_AffineSpace3f(xfm, transform);
    return compute_xfm_bbox(transform, pnode, clip, clipBox, matOverhead);
}

GRL_INLINE uint64_t compute_refit_structs_compacted_size(BVHBase* base)
{
    uint dataSize = 0;

    if (BVHBase_HasBackPointers(base))
    {
        const uint fatleafEntrySize = (base->fatLeafCount * sizeof(LeafTableEntry) + 63) & ~63;
        const uint innerEntrySize = (base->innerCount * sizeof(InnerNodeTableEntry) + 63) & ~63;

        // New atomic update
        if(base->quadIndicesDataStart > base->backPointerDataStart)
        {
            uint numQuads = BVHBase_GetNumQuads(base);

            const uint quadTableMainBufferSize = (numQuads + 255) & ~255;
            const uint quadLeftoversSize = (base->quadLeftoversCountNewAtomicUpdate + 255) & ~255;
            const uint quadTableEntriesSize = (((quadTableMainBufferSize + quadLeftoversSize) * sizeof(LeafTableEntry) + 63) & ~63);

            const uint quadIndicesDataSize = (numQuads * sizeof(QuadDataIndices) + 63) & ~63;

            dataSize += quadTableEntriesSize + quadIndicesDataSize;
        }

        dataSize += 
            ((BVHBase_GetNumInternalNodes(base) * sizeof(uint) + 63) & ~63)
            + fatleafEntrySize + innerEntrySize;
    }

    return (uint64_t)dataSize;
}

GRL_INLINE uint64_t compute_compacted_size(BVHBase* base)
{
    uint64_t size = sizeof(BVHBase);
    size += BVHBase_GetNumHWInstanceLeaves(base) * sizeof(HwInstanceLeaf);
    size += BVHBase_GetNumProcedurals(base) * sizeof(ProceduralLeaf);
    size += BVHBase_GetNumQuads(base) * sizeof(QuadLeaf);
    size += compute_refit_structs_compacted_size(base);
    size += BVHBase_GetNumInternalNodes(base) * sizeof(InternalNode);
    size += sizeof(InstanceDesc) * base->Meta.instanceCount;
    size += (sizeof(GeoMetaData) * base->Meta.geoCount + 63) & ~63; // align to 64
    size = (size + 63) & ~63;

    return size;
}
