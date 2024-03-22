//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "shared.h"
#include "intrinsics.h"
#include "AABB.h"
#include "AABB3f.h"
#include "qbvh6.h"

/* ====== BVH_BUILDER config ====== */

__constant const float cfg_intCost = 4.0f;
__constant const float cfg_travCost = 1.0f;
__constant const uint cfg_minLeafSize = BVH_LEAF_N_MIN;
__constant const uint cfg_maxLeafSize = BVH_LEAF_N_MAX;
__constant const uint cfg_maxDepth = BUILDRECORD_STACK_SIZE;

#define ENABLE_CONVERSION_CHECKS 0

#ifdef ENABLE_BIG_REG_ANNOTATION
#define GRL_ANNOTATE_BIG_REG_REQ __attribute__((annotate("num-thread-per-eu 4")))
#else
#define GRL_ANNOTATE_BIG_REG_REQ
#endif

#ifdef ENABLE_IGC_DO_NOT_SPILL
#define GRL_ANNOTATE_IGC_DO_NOT_SPILL __attribute__((annotate("igc-do-not-spill")))
#else
#define GRL_ANNOTATE_IGC_DO_NOT_SPILL
#endif

#define ERROR()

/* =================================================================================================================================================== */
/* =================================================================================================================================================== */
/* =================================================================================================================================================== */
/* =================================================================================================================================================== */

GRL_INLINE unsigned int getNumLeafPrims(unsigned int offset)
{
    return (offset & 0x7) - 3;
}

GRL_INLINE unsigned int getLeafOffset(unsigned int offset)
{
    return offset & (~0x7);
}

GRL_INLINE float4 triangleNormal(const float4 v0, const float4 v1, const float4 v2)
{
    const float4 a = v1 - v0;
    const float4 b = v2 - v0;
    return cross(a, b);
}

GRL_INLINE float areaTriangle(const float4 v0, const float4 v1, const float4 v2)
{
    const float4 normal = triangleNormal(v0, v1, v2);
    return length((float3)(normal.x, normal.y, normal.z)) * 0.5f;
}

GRL_INLINE float det2(const float2 a, const float2 b)
{
    return a.x * b.y - a.y * b.x;
}

GRL_INLINE float areaProjectedTriangle(const float4 v0, const float4 v1, const float4 v2)
{
    const float xy = 0.5f * fabs(det2(v1.xy - v0.xy, v2.xy - v0.xy));
    const float yz = 0.5f * fabs(det2(v1.yz - v0.yz, v2.yz - v0.yz));
    const float zx = 0.5f * fabs(det2(v1.zx - v0.zx, v2.zx - v0.zx));
    return xy + yz + zx;
}

typedef struct Block64B  {
    char data[64];
} Block64B __attribute__((aligned(64)));

typedef char byte_align64B __attribute__((aligned(64)));

/* ====================================================================== */
/* ============================== GLOBALS =============================== */
/* ====================================================================== */

GRL_INLINE bool Globals_OnFinish(global struct Globals *globals)
{
    /* last active HW thread ? */
    if (get_local_id(0) == 0)
    {
        const uint sync = atomic_add(&globals->sync, 1);
        if (sync + 1 == get_num_groups(0))
        {
            globals->sync = 0;
            return true;
        }
    }
    return false;
}

GRL_INLINE uint BlockAllocator_BytesUsed(struct BlockAllocator *p)
{
    return p->cur - p->start;
};

GRL_INLINE uint BlockAllocator_Alloc(__global struct BlockAllocator *p, const uint size)
{
    return atomic_add(&p->cur, size);
}

GRL_INLINE uint BlockAllocator_Alloc_Single(__global struct BlockAllocator *p, const uint size)
{
    uint offset = 0;
    if (get_sub_group_local_id() == 0)
        offset = atomic_add(&p->cur, size);
    return sub_group_broadcast(offset, 0);
}

// node allocation returns an offset from beginning of BVH to allocated node
//  in multiples of 64B
GRL_INLINE uint allocate_inner_nodes(global struct BVHBase* base, uint num_nodes )
{
    return atomic_add_global( &base->nodeDataCur, num_nodes );
}
GRL_INLINE uint allocate_procedural_leaves(global struct BVHBase* base, uint num_nodes)
{
    return atomic_add_global(&base->proceduralDataCur, num_nodes);
}

GRL_INLINE uint allocate_quad_leaves(global struct BVHBase* base, uint num_nodes)
{
    return atomic_add_global(&base->quadLeafCur, num_nodes);
}

#if 0
GRL_INLINE uint alloc_node_mem(global struct Globals *globals, const uint size)
{
    const uint aligned_size = ((size + 63) / 64) * 64; /* allocate in 64 bytes blocks */
    return BlockAllocator_Alloc(&globals->node_mem_allocator, aligned_size);
}

GRL_INLINE uint alloc_single_node_mem(global struct Globals *globals, const uint size)
{
    const uint aligned_size = ((size + 63) / 64) * 64; /* allocate in 64 bytes blocks */
    return BlockAllocator_Alloc_Single(&globals->node_mem_allocator, aligned_size);
}

GRL_INLINE uint alloc_quad_leaf_mem(global struct Globals *globals, const uint size)
{
    const uint aligned_size = ((size + 63) / 64) * 64; /* allocate in 64 bytes blocks */
    return BlockAllocator_Alloc(&globals->quad_mem_allocator, aligned_size);
}

GRL_INLINE uint alloc_procedural_leaf_mem(global struct Globals *globals, const uint size)
{
    const uint aligned_size = ((size + 63) / 64) * 64; /* allocate in 64 bytes blocks */
    return BlockAllocator_Alloc(&globals->procedural_mem_allocator, aligned_size);
}
#endif

GRL_INLINE global struct BuildRecord *getBuildRecords(char *bvh_mem, struct Globals *globals)
{
    return (global struct BuildRecord *)(bvh_mem + globals->build_record_start);
}

/* ======================================================================= */
/* ============================== TRIANGLE =============================== */
/* ======================================================================= */

/*GRL_INLINE void printTriangle(struct Triangle *t)
{
  printf("vtx[0] %d vtx[1] %d vtx[2] %d primID %d geomID %d \n",t->vtx[0],t->vtx[1],t->vtx[2],t->primID,t->geomID);
  }*/

/* ==================================================================== */
/* ============================== SPLIT =============================== */
/* ==================================================================== */

GRL_INLINE void printSplit(struct Split *split)
{
    printf("split sah %f dim %d pos %d \n", split->sah, split->dim, split->pos);
}

/* ========================================================================== */
/* ============================== BUILDRECORD =============================== */
/* ========================================================================== */

GRL_INLINE void initBuildRecord(struct BuildRecord *buildRecord, uint start, uint end)
{
    AABB_init(&buildRecord->centroidBounds);
    buildRecord->start = start;
    buildRecord->end = end;
}

GRL_INLINE void extendBuildRecord(struct BuildRecord *buildRecord, struct AABB *primref)
{
    AABB_extend_point(&buildRecord->centroidBounds, AABB_centroid2(primref));
}

GRL_INLINE uint getBuildRecursionDepth(struct BuildRecord *buildRecord)
{
    return as_uint(buildRecord->centroidBounds.upper.w);
}

GRL_INLINE void setBuildRecursionDepth(struct BuildRecord *buildRecord, uint depth)
{
    buildRecord->centroidBounds.upper.w = as_float(depth);
}

GRL_INLINE uint getNumPrimsBuildRecord(struct BuildRecord *buildRecord)
{
    return buildRecord->end - buildRecord->start;
}

/* ========================================================================== */
/* =================== BinaryMortonCodeHierarchy ============================= */
/* ========================================================================== */

GRL_INLINE void BinaryMortonCodeHierarchy_init(struct BinaryMortonCodeHierarchy *record, uint start, uint end)
{
    record->range.start = start;
    record->range.end = end;
    record->leftChild = -1;
    record->rightChild = -1;
//    record->flag = 0;
}

GRL_INLINE uint BinaryMortonCodeHierarchy_getNumPrimitives(global struct BinaryMortonCodeHierarchy *nodes, uint nodeID)
{
    /* leaf case */
    if (nodeID & (uint)(1 << 31))
        return 1;

    /* inner node case*/
    else
        return nodes[nodeID].range.end - nodes[nodeID].range.start + 1;
}

GRL_INLINE struct BinaryMortonCodeHierarchy BinaryMortonCodeHierarchy_getEntry(global struct BinaryMortonCodeHierarchy* nodes, uint nodeID)
{
    struct BinaryMortonCodeHierarchy entry;

    if (nodeID & (uint)(1 << 31)) {
        /* leaf case */
        uint rangeStart = nodeID ^ (uint)(1 << 31);
        BinaryMortonCodeHierarchy_init(&entry, rangeStart, rangeStart);
    }
    else {
        /* inner node case*/
        entry = nodes[nodeID];
    }

    return entry;
}

GRL_INLINE uint BinaryMortonCodeHierarchy_getRangeStart(global struct BinaryMortonCodeHierarchy *nodes, uint nodeID)
{
    /* leaf case */
    if (nodeID & (uint)(1 << 31))
        return nodeID ^ (uint)(1 << 31);

    /* inner node case*/
    else
        return nodes[nodeID].range.start;
}

/* ==================================================================== */
/* ============================== RANGE =============================== */
/* ==================================================================== */

GRL_INLINE void printRange(struct Range *range)
{
    printf("start %d end %d \n", range->start, range->end);
}

GRL_INLINE bool equalRange(struct Range *range0, struct Range *range1)
{
    if (range0->start == range1->start &&
        range0->end == range1->end)
        return true;
    return false;
}

GRL_INLINE uint getSizeRange(struct Range *range)
{
    return range->end - range->start;
}

/* ==================================================================== */
/* ========================= ProceduralLeaf =========================== */
/* ==================================================================== */

#if 0
struct ProceduralLeaf
{
  uint shaderIndex_geomMask;
  uint geomIndex_flags;
  uint N_last;
  uint primIndex[13];
};
#endif

GRL_INLINE uint ProceduralLeaf_geomIndex(global struct ProceduralLeaf *This)
{
    return This->leafDesc.geomIndex_flags & 0x1FFFFFFF;
}

GRL_INLINE uint ProceduralLeaf_primIndex(global struct ProceduralLeaf *This, uint i)
{
    //assert(i < N);
    return This->_primIndex[i];
}

/* ==================================================================== */
/* =========================== TrianglePair =========================== */
/* ==================================================================== */

struct TrianglePair
{
    uint4 a;    // indices of the 4 verts to store in the quad
    uint3 lb;   //   index of the second triangle's verts in 'a'
};

GRL_INLINE struct TrianglePair TrianglePair_Constructor(uint3 tri0, uint primID0, uint3 tri1, uint primID1)
{
    struct TrianglePair q;
    q.a.x = tri0.x;
    q.a.y = tri0.y;
    q.a.z = tri0.z;
    q.a.w = tri0.z;

    uint3 b;
    b.x = tri1.x;
    b.y = tri1.y;
    b.z = tri1.z;

    q.lb = (uint3)(3);

    q.lb.x = (b.x == q.a.x) ? 0 : q.lb.x;
    q.lb.y = (b.y == q.a.x) ? 0 : q.lb.y;
    q.lb.z = (b.z == q.a.x) ? 0 : q.lb.z;

    q.lb.x = (b.x == q.a.y) ? 1 : q.lb.x;
    q.lb.y = (b.y == q.a.y) ? 1 : q.lb.y;
    q.lb.z = (b.z == q.a.y) ? 1 : q.lb.z;

    q.lb.x = (b.x == q.a.z) ? 2 : q.lb.x;
    q.lb.y = (b.y == q.a.z) ? 2 : q.lb.y;
    q.lb.z = (b.z == q.a.z) ? 2 : q.lb.z;

    q.lb.x = (primID0 != primID1) ? q.lb.x : 0;
    q.lb.y = (primID0 != primID1) ? q.lb.y : 0;
    q.lb.z = (primID0 != primID1) ? q.lb.z : 0;

    q.a.w = (q.lb.x == 3) ? b.x : q.a.w;
    q.a.w = (q.lb.y == 3) ? b.y : q.a.w;
    q.a.w = (q.lb.z == 3) ? b.z : q.a.w;

    return q;
}

GRL_INLINE float InstanceDesc_get_transform(const InstanceDesc *d, const uint32_t row, const uint32_t column)
{
    return d->Transform[row][column];
}

GRL_INLINE uint32_t InstanceDesc_get_instanceID(const InstanceDesc *d)
{
    return d->InstanceIDAndMask & (0x00FFFFFF);
}

GRL_INLINE uint32_t InstanceDesc_get_InstanceMask(const InstanceDesc *d)
{
    return d->InstanceIDAndMask >> 24;
}

GRL_INLINE uint32_t InstanceDesc_get_InstanceContributionToHitGroupIndex(const InstanceDesc *d)
{
    return d->InstanceContributionToHitGroupIndexAndFlags & ((1 << 24) - 1);
}

GRL_INLINE uint32_t InstanceDesc_get_InstanceFlags(const InstanceDesc *d)
{
    return d->InstanceContributionToHitGroupIndexAndFlags >> 24;
}

GRL_INLINE gpuva_t InstanceDesc_get_AccelerationStructure(const InstanceDesc *d)
{
    return d->AccelerationStructureGPUVA;
}

GRL_INLINE void InstanceDesc_set_transform(InstanceDesc *d, const uint32_t row, const uint32_t column, float value)
{
    d->Transform[row][column] = value;
}

GRL_INLINE void InstanceDesc_set_instanceID(InstanceDesc *d, const uint32_t id)
{
    d->InstanceIDAndMask &= 255 << 24;
    d->InstanceIDAndMask |= id & ((1 << 24) - 1);
}

GRL_INLINE void InstanceDesc_set_InstanceMask(InstanceDesc *d, const uint32_t mask)
{
    d->InstanceIDAndMask &= ((1 << 24) - 1);
    d->InstanceIDAndMask |= mask << 24;
}

GRL_INLINE void InstanceDesc_set_InstanceContributionToHitGroupIndex(InstanceDesc *d, const uint32_t contribution)
{
    d->InstanceContributionToHitGroupIndexAndFlags &= 255 << 24;
    d->InstanceContributionToHitGroupIndexAndFlags |= contribution & ((1 << 24) - 1);
}

GRL_INLINE void InstanceDesc_set_InstanceFlags(InstanceDesc *d, const uint32_t flags)
{
    d->InstanceContributionToHitGroupIndexAndFlags &= ((1 << 24) - 1);
    d->InstanceContributionToHitGroupIndexAndFlags |= flags << 24;
}

GRL_INLINE void InstanceDesc_set_AccelerationStructure(InstanceDesc *d, gpuva_t address)
{
    d->AccelerationStructureGPUVA = address;
}
