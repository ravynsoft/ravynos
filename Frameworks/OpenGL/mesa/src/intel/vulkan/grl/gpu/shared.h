//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "GRLGen12.h"
#pragma once

#define sizeof_Quad 64
#define sizeof_Procedural 64
#define sizeof_PrimRef 32
#define sizeof_PresplitItem 8
#define sizeof_HwInstanceLeaf 128
#define MORTON_BUILDER_SUBTREE_THRESHOLD 256
#define MORTON_BUILDER_P2_ELEMENTS_IN_SLM 16 * 1024 / 32
// Temporarily disable localized phase2 due to issues in ELG presi
// This implementation would be replaced with bottom_up + bounding box approach without the need for phase2 refit
#define MORTON_BUILDER_P2_SINGLE_WG_THRESHOLD /*100000*/ 0

#define BVH_QUAD_NODE 4
#define BVH_INSTANCE_NODE 1
#define BVH_INTERNAL_NODE 0
#define BVH_PROCEDURAL_NODE 3
#define BUILDRECORD_STACK_SIZE 48
#define BINS 16

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(RTAS)
GRL_NAMESPACE_BEGIN(GPUBVHBuilder)

struct AABB
{
    float4 lower;
    float4 upper;
};

typedef struct BlockAllocator
{
    unsigned int start;
    unsigned int cur;
} BlockAllocator;

struct Globals
{
    struct AABB centroidBounds;

    unsigned int build_record_start;
    unsigned int numPrimitives;
    unsigned int leafPrimType;
    unsigned int leafSize;

    unsigned int numSplittedPrimitives;
    unsigned int numBuildRecords;

    // spatial split sate
    unsigned int numOriginalPrimitives;
    float presplitPrioritySum;
    float probThreshold;

    // binned-sah bfs state 
    unsigned int counter;
    unsigned int numBuildRecords_extended;
    
    // sync variable used for global-sync on work groups
    unsigned int sync;
    

    /* morton code builder state */
    unsigned int shift;      // used by adaptive mc-builder
    unsigned int shift_mask; // used by adaptive mc-builder
    unsigned int binary_hierarchy_root;
    unsigned int p0_allocated_num;
    unsigned int p0_created_num;
    unsigned int morton_sort_in_flight;
    unsigned int sort_iterations;

    gpuva_t binary_hierarchy_buffer; // pointer to the binary morton code hierarchy.  Stashed here as a debug aid
};

struct Range
{
    unsigned int start, end;
};

struct Triangle
{
    unsigned int vtx[3];
    //unsigned int primID;
    //unsigned int geomID;
};

struct MortonCodePrimitive
{
    uint64_t index_code; // 64bit code + index combo
};

struct BuildRecord
{
    struct AABB centroidBounds;
    unsigned int start, end;
    __global void *current;
};

struct BinaryMortonCodeHierarchy
{
    struct Range range;
    unsigned int leftChild;
    unsigned int rightChild;
   // unsigned int flag;
};

typedef struct MortonFlattenedBoxlessNode {
    uint binary_hierarchy_index; // only needed when type != BVH_INTERNAL_NODE
    uint childOffset_type;       // childOffset : 26, type : 6
    uint backPointer;            // same usage as in bvh
} MortonFlattenedBoxlessNode;

struct StatStackEntry
{
    struct AABB aabb;
    unsigned int node;
    unsigned int type;
    unsigned int depth;
    float area;
};

struct BuildRecordMorton
{
    unsigned int nodeID;
    unsigned int items;
    unsigned int current_index;
    unsigned int parent_index;
};

struct Split
{
    float sah;
    int dim;
    int pos;
};

struct BinMapping
{
    float4 ofs, scale;
};

struct BinInfo
{
    struct AABB3f boundsX[BINS];
    struct AABB3f boundsY[BINS];
    struct AABB3f boundsZ[BINS];
    uint3 counts[BINS];
};

struct BinInfo2
{
    struct AABB3f boundsX[BINS * 2];
    struct AABB3f boundsY[BINS * 2];
    struct AABB3f boundsZ[BINS * 2];
    uint3 counts[BINS * 2];
};

struct GlobalBuildRecord
{
    struct BinInfo2 binInfo;
    struct BinMapping binMapping;
    struct Split split;
    struct Range range;
    struct AABB leftCentroid;
    struct AABB rightCentroid;
    struct AABB leftGeometry;
    struct AABB rightGeometry;
    unsigned int atomicCountLeft;
    unsigned int atomicCountRight;
    unsigned int buildRecordID;
};

GRL_NAMESPACE_END(GPUBVHBuilder)
GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)
