//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "common.h"
#include "instance.h"

#define DBG(x)

#define ENABLE_CHECKS 0

#define ENABLE_32BINS_IN_BREADTH_FIRST_PHASE 1

/* todo:                                                     */
/* - new cross WG code path for first splits                 */
/* - optimize find best child loop sequence                  */
/* - subgroup_setQBVHNodeN needs work on 6 slots in parallel */

#define DIVIDE_BY_6 1

inline uint getNumPrims(struct BuildRecord *buildRecord)
{
    return buildRecord->end - buildRecord->start;
}

inline void printBuildRecord(struct BuildRecord *record)
{
    printf("centroidBounds\n");
    AABB_print(&record->centroidBounds);
    printf("start %d end %d size %d depth %d \n", record->start, record->end, record->end - record->start, getBuildRecursionDepth(record));
}

inline void printBinInfo2(struct BinInfo2 *record)
{
    printf("boundsX[%d]\n", BINS * 2);
    for (uint b = 0; b < BINS * 2; b++)
    {
        AABB3f_print(&record->boundsX[b]);
        printf("counts.x = %d\n", record->counts[b].x);
    }
    printf("boundsY[%d]\n", BINS * 2);
    for (uint b = 0; b < BINS * 2; b++)
    {
        AABB3f_print(&record->boundsY[b]);
        printf("counts.y = %d\n", record->counts[b].y);
    }
    printf("boundsZ[%d]\n", BINS * 2);
    for (uint b = 0; b < BINS * 2; b++)
    {
        AABB3f_print(&record->boundsZ[b]);
        printf("counts.z = %d\n", record->counts[b].z);
    }
}

inline void initBinMapping(struct BinMapping *binMapping, struct AABB *centBounds, const uint bins)
{
    const float4 eps = 1E-34f;
    const float4 diag = max(eps, centBounds->upper - centBounds->lower);
    const float4 scale = (float4)(0.99f * (float)bins) / diag;
    binMapping->scale = select((float4)(0.0f), scale, diag > eps);
    binMapping->ofs = centBounds->lower;
}

inline void atomicExtendLocalBuildRecord(local struct BuildRecord *buildRecord, global struct AABB *primref)
{
    const float4 centroid2 = primref->lower + primref->upper;
    AABB_local_atomic_merge(&buildRecord->centroidBounds, centroid2, centroid2);
}

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

inline void initBinInfo(struct BinInfo *binInfo)
{
    for (uint i = 0; i < BINS; i++)
    {
        AABB3f_init(&binInfo->boundsX[i]);
        AABB3f_init(&binInfo->boundsY[i]);
        AABB3f_init(&binInfo->boundsZ[i]);
        binInfo->counts[i] = (uint3)(0);
    }
}

inline void subgroup_initBinInfo(struct BinInfo *binInfo)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();

    for (uint i = subgroupLocalID; i < BINS; i += subgroup_size)
    {
        AABB3f_init(&binInfo->boundsX[i]);
        AABB3f_init(&binInfo->boundsY[i]);
        AABB3f_init(&binInfo->boundsZ[i]);
        binInfo->counts[i] = (uint3)(0);
    }
}

inline void parallel_initBinInfo(struct BinInfo *binInfo)
{
    const uint localID = get_local_id(0);
    if (localID < BINS)
    {
        AABB3f_init(&binInfo->boundsX[localID]);
        AABB3f_init(&binInfo->boundsY[localID]);
        AABB3f_init(&binInfo->boundsZ[localID]);
        binInfo->counts[localID] = (uint3)(0);
    }
}

inline void atomicUpdateLocalBinInfo(struct BinMapping *binMapping, local struct BinInfo *binInfo, global struct AABB *primref)
{
    const float4 lower = primref->lower;
    const float4 upper = primref->upper;
    const float4 p = lower + upper;
    const uint4 i = convert_uint4((p - binMapping->ofs) * binMapping->scale);
    AABB3f_atomic_merge_local(&binInfo->boundsX[i.x], lower, upper);
    AABB3f_atomic_merge_local(&binInfo->boundsY[i.y], lower, upper);
    AABB3f_atomic_merge_local(&binInfo->boundsZ[i.z], lower, upper);
    atomic_add((local uint *)&binInfo->counts[i.x] + 0, 1);
    atomic_add((local uint *)&binInfo->counts[i.y] + 1, 1);
    atomic_add((local uint *)&binInfo->counts[i.z] + 2, 1);
}

inline void atomicUpdateLocalBinInfo_nocheck(struct BinMapping *binMapping, local struct BinInfo *binInfo, global struct AABB *primref)
{
    const float4 lower = primref->lower;
    const float4 upper = primref->upper;
    const float4 p = lower + upper;
    const uint4 i = convert_uint4((p - binMapping->ofs) * binMapping->scale);
    AABB3f_atomic_merge_local_nocheck(&binInfo->boundsX[i.x], lower, upper);
    AABB3f_atomic_merge_local_nocheck(&binInfo->boundsY[i.y], lower, upper);
    AABB3f_atomic_merge_local_nocheck(&binInfo->boundsZ[i.z], lower, upper);
    atomic_add((local uint *)&binInfo->counts[i.x] + 0, 1);
    atomic_add((local uint *)&binInfo->counts[i.y] + 1, 1);
    atomic_add((local uint *)&binInfo->counts[i.z] + 2, 1);
}

inline void updateBins(struct BinMapping *binMapping, struct BinInfo *binInfo, global struct AABB *primref)
{
    const float4 lower = primref->lower;
    const float4 upper = primref->upper;
    const float4 p = lower + upper;
    const uint4 i = convert_uint4((p - binMapping->ofs) * binMapping->scale);
    AABB3f_extendlu(&binInfo->boundsX[i.x], lower.xyz, upper.xyz);
    AABB3f_extendlu(&binInfo->boundsY[i.y], lower.xyz, upper.xyz);
    AABB3f_extendlu(&binInfo->boundsZ[i.z], lower.xyz, upper.xyz);
    binInfo->counts[i.x].x++;
    binInfo->counts[i.y].y++;
    binInfo->counts[i.z].z++;
}

// =====================================================================================================================
// =====================================================================================================================
// =====================================================================================================================

inline void parallel_initBinInfo2(struct BinInfo2 *binInfo, const uint bins)
{
    const uint localID = get_local_id(0);
    if (localID < bins)
    {
        AABB3f_init(&binInfo->boundsX[localID]);
        AABB3f_init(&binInfo->boundsY[localID]);
        AABB3f_init(&binInfo->boundsZ[localID]);
        binInfo->counts[localID] = (uint3)(0);
    }
}

inline void atomicUpdateLocalBinInfo2(struct BinMapping *binMapping, local struct BinInfo2 *binInfo, global struct AABB *primref)
{
    const float4 lower = primref->lower;
    const float4 upper = primref->upper;
    const float4 p = lower + upper;
    const uint4 i = convert_uint4((p - binMapping->ofs) * binMapping->scale);
    AABB3f_atomic_merge_local(&binInfo->boundsX[i.x], lower, upper);
    AABB3f_atomic_merge_local(&binInfo->boundsY[i.y], lower, upper);
    AABB3f_atomic_merge_local(&binInfo->boundsZ[i.z], lower, upper);
    atomic_add((local uint *)&binInfo->counts[i.x] + 0, 1);
    atomic_add((local uint *)&binInfo->counts[i.y] + 1, 1);
    atomic_add((local uint *)&binInfo->counts[i.z] + 2, 1);
}

inline void atomicUpdateGlobalFromLocalBinInfo2(global struct BinInfo2 *dest, local struct BinInfo2 *source, const uint bins)
{
    const uint localID = get_local_id(0);
    if (localID < bins)
    {
        AABB3f_atomic_merge_global_local(&dest->boundsX[localID], &source->boundsX[localID]);
        AABB3f_atomic_merge_global_local(&dest->boundsY[localID], &source->boundsY[localID]);
        AABB3f_atomic_merge_global_local(&dest->boundsZ[localID], &source->boundsZ[localID]);
        atomic_add((global uint *)&dest->counts[localID] + 0, source->counts[localID].x);
        atomic_add((global uint *)&dest->counts[localID] + 1, source->counts[localID].y);
        atomic_add((global uint *)&dest->counts[localID] + 2, source->counts[localID].z);
    }
}

inline uint subgroup_getMaxAreaChild(struct AABB *childrenAABB, const uint numChildren)
{
    const uint subgroupLocalID = get_sub_group_local_id();
#if 0
  /*! find best child to split */
  const float area = (subgroupLocalID < numChildren) & (as_uint(childrenAABB[subgroupLocalID].upper.w) > cfg_minLeafSize) ? childrenAABB[subgroupLocalID].lower.w : -(float)INFINITY;
  const float maxArea = sub_group_reduce_max(area);
  const uint mask = intel_sub_group_ballot(area == maxArea);
  const uint bestChild = maxArea != -(float)INFINITY ? ctz(mask) : -1;
#else
    float bestArea = -(float)INFINITY;
    int bestChild = -1;
    for (int i = 0; i < numChildren; i++)
    {
        /* ignore leaves as they cannot get split */
        if (as_uint(childrenAABB[i].upper.w) <= cfg_minLeafSize)
            continue;

        /* find child with largest surface area */
        if (childrenAABB[i].lower.w > bestArea)
        {
            bestChild = i;
            bestArea = childrenAABB[i].lower.w;
        }
    }
#endif
    return bestChild;
}

inline bool AABB_verifyBounds(struct BuildRecord *buildRecord, struct AABB *geometryBounds, struct AABB *primref)
{
    const float4 centroid2 = primref->lower + primref->upper;

    if (centroid2.x < buildRecord->centroidBounds.lower.x)
        return false;
    if (centroid2.y < buildRecord->centroidBounds.lower.y)
        return false;
    if (centroid2.z < buildRecord->centroidBounds.lower.z)
        return false;

    if (centroid2.x > buildRecord->centroidBounds.upper.x)
        return false;
    if (centroid2.y > buildRecord->centroidBounds.upper.y)
        return false;
    if (centroid2.z > buildRecord->centroidBounds.upper.z)
        return false;

    if (primref->lower.x < geometryBounds->lower.x)
        return false;
    if (primref->lower.y < geometryBounds->lower.y)
        return false;
    if (primref->lower.z < geometryBounds->lower.z)
        return false;

    if (primref->upper.x > geometryBounds->upper.x)
        return false;
    if (primref->upper.y > geometryBounds->upper.y)
        return false;
    if (primref->upper.z > geometryBounds->upper.z)
        return false;

    return true;
}

/* initialize primref index array */
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
create_primref_index(global struct Globals *globals,
                     global struct AABB *primref,
                     global unsigned int *primref_index)
{
    const uint local_size = get_local_size(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);
    const uint localID = get_local_id(0);

    const uint startID = (taskID + 0) * globals->numPrimitives / numTasks;
    const uint endID = (taskID + 1) * globals->numPrimitives / numTasks;
    for (uint primID = startID + localID; primID < endID; primID += local_size)
        primref_index[primID] = primID;
}

// ==========================================================================================================
// ==========================================================================================================
// ==========================================================================================================

inline float left_to_right_area16(struct AABB3f *low)
{
    struct AABB3f low_prefix = AABB3f_sub_group_scan_exclusive_min_max(low);
    return halfArea_AABB3f(&low_prefix);
}

inline uint left_to_right_counts16(uint low)
{
    return sub_group_scan_exclusive_add(low);
}

inline float right_to_left_area16(struct AABB3f *low)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint ID = subgroup_size - 1 - subgroupLocalID;
    struct AABB3f low_reverse = AABB3f_sub_group_shuffle(low, ID);
    struct AABB3f low_prefix = AABB3f_sub_group_scan_inclusive_min_max(&low_reverse);
    const float low_area = sub_group_broadcast(halfArea_AABB3f(&low_prefix), ID);
    return low_area;
}

inline uint right_to_left_counts16(uint low)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint ID = subgroup_size - 1 - subgroupLocalID;
    const uint low_reverse = sub_group_broadcast(low, ID);
    const uint low_prefix = sub_group_scan_inclusive_add(low_reverse);
    return sub_group_broadcast(low_prefix, ID);
}

inline float2 left_to_right_area32(struct AABB3f *low, struct AABB3f *high)
{
    struct AABB3f low_prefix = AABB3f_sub_group_scan_exclusive_min_max(low);
    struct AABB3f low_reduce = AABB3f_sub_group_reduce(low);
    struct AABB3f high_prefix = AABB3f_sub_group_scan_exclusive_min_max(high);
    AABB3f_extend(&high_prefix, &low_reduce);
    const float low_area = halfArea_AABB3f(&low_prefix);
    const float high_area = halfArea_AABB3f(&high_prefix);
    return (float2)(low_area, high_area);
}

inline uint2 left_to_right_counts32(uint low, uint high)
{
    const uint low_prefix = sub_group_scan_exclusive_add(low);
    const uint low_reduce = sub_group_reduce_add(low);
    const uint high_prefix = sub_group_scan_exclusive_add(high);
    return (uint2)(low_prefix, low_reduce + high_prefix);
}

inline float2 right_to_left_area32(struct AABB3f *low, struct AABB3f *high)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint ID = subgroup_size - 1 - subgroupLocalID;
    struct AABB3f low_reverse = AABB3f_sub_group_shuffle(high, ID);
    struct AABB3f high_reverse = AABB3f_sub_group_shuffle(low, ID);
    struct AABB3f low_prefix = AABB3f_sub_group_scan_inclusive_min_max(&low_reverse);
    struct AABB3f low_reduce = AABB3f_sub_group_reduce(&low_reverse);
    struct AABB3f high_prefix = AABB3f_sub_group_scan_inclusive_min_max(&high_reverse);
    AABB3f_extend(&high_prefix, &low_reduce);
    const float low_area = sub_group_broadcast(halfArea_AABB3f(&high_prefix), ID);
    const float high_area = sub_group_broadcast(halfArea_AABB3f(&low_prefix), ID);
    return (float2)(low_area, high_area);
}

inline uint2 right_to_left_counts32(uint low, uint high)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint ID = subgroup_size - 1 - subgroupLocalID;
    const uint low_reverse = sub_group_broadcast(high, ID);
    const uint high_reverse = sub_group_broadcast(low, ID);
    const uint low_prefix = sub_group_scan_inclusive_add(low_reverse);
    const uint low_reduce = sub_group_reduce_add(low_reverse);
    const uint high_prefix = sub_group_scan_inclusive_add(high_reverse) + low_reduce;
    return (uint2)(sub_group_broadcast(high_prefix, ID), sub_group_broadcast(low_prefix, ID));
}

inline ulong getBestSplit(float3 sah, uint ID, const float4 scale, const ulong defaultSplit)
{
    ulong splitX = (((ulong)as_uint(sah.x)) << 32) | ((uint)ID << 2) | 0;
    ulong splitY = (((ulong)as_uint(sah.y)) << 32) | ((uint)ID << 2) | 1;
    ulong splitZ = (((ulong)as_uint(sah.z)) << 32) | ((uint)ID << 2) | 2;
    /* ignore zero sized dimensions */
    splitX = select(splitX, defaultSplit, (ulong)(scale.x == 0));
    splitY = select(splitY, defaultSplit, (ulong)(scale.y == 0));
    splitZ = select(splitZ, defaultSplit, (ulong)(scale.z == 0));
    ulong bestSplit = min(min(splitX, splitY), splitZ);
    bestSplit = sub_group_reduce_min(bestSplit);
    return bestSplit;
}

inline uint fastDivideBy6_uint(uint v)
{
#if 1
    const ulong u = (ulong)v >> 1;
    return (uint)((u * 0x55555556ul) >> 32);
#else
    return v / 6;
#endif
}

inline uint3 fastDivideBy6_uint3(uint3 v)
{
    return (uint3)(fastDivideBy6_uint(v.x), fastDivideBy6_uint(v.y), fastDivideBy6_uint(v.z));
}

inline struct Split reduceBinsAndComputeBestSplit16(struct BinInfo *binInfo, const float4 scale, uint startID, uint endID)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();

    struct AABB3f boundsX = binInfo->boundsX[subgroupLocalID];

    const float lr_areaX = left_to_right_area16(&boundsX);
    const float rl_areaX = right_to_left_area16(&boundsX);

    struct AABB3f boundsY = binInfo->boundsY[subgroupLocalID];

    const float lr_areaY = left_to_right_area16(&boundsY);
    const float rl_areaY = right_to_left_area16(&boundsY);

    struct AABB3f boundsZ = binInfo->boundsZ[subgroupLocalID];

    const float lr_areaZ = left_to_right_area16(&boundsZ);
    const float rl_areaZ = right_to_left_area16(&boundsZ);

    const uint3 counts = binInfo->counts[subgroupLocalID];

    const uint lr_countsX = left_to_right_counts16(counts.x);
    const uint rl_countsX = right_to_left_counts16(counts.x);
    const uint lr_countsY = left_to_right_counts16(counts.y);
    const uint rl_countsY = right_to_left_counts16(counts.y);
    const uint lr_countsZ = left_to_right_counts16(counts.z);
    const uint rl_countsZ = right_to_left_counts16(counts.z);

    const float3 lr_area = (float3)(lr_areaX, lr_areaY, lr_areaZ);
    const float3 rl_area = (float3)(rl_areaX, rl_areaY, rl_areaZ);

#if DIVIDE_BY_6 == 0
    const uint blocks_shift = SAH_LOG_BLOCK_SHIFT;
    uint3 blocks_add = (uint3)((1 << blocks_shift) - 1);
    const uint3 lr_count = ((uint3)(lr_countsX, lr_countsY, lr_countsZ) + blocks_add) >> blocks_shift;
    const uint3 rl_count = ((uint3)(rl_countsX, rl_countsY, rl_countsZ) + blocks_add) >> blocks_shift;
#else
    const uint3 lr_count = fastDivideBy6_uint3((uint3)(lr_countsX, lr_countsY, lr_countsZ) + BVH_NODE_N6 - 1);
    const uint3 rl_count = fastDivideBy6_uint3((uint3)(rl_countsX, rl_countsY, rl_countsZ) + BVH_NODE_N6 - 1);
#endif
    float3 sah = fma(lr_area, convert_float3(lr_count), rl_area * convert_float3(rl_count));

    /* first bin is invalid */

    sah.x = select((float)(INFINITY), sah.x, subgroupLocalID != 0);
    sah.y = select((float)(INFINITY), sah.y, subgroupLocalID != 0);
    sah.z = select((float)(INFINITY), sah.z, subgroupLocalID != 0);

    const uint mid = (startID + endID) / 2;
    const ulong defaultSplit = (((ulong)as_uint((float)(INFINITY))) << 32) | ((uint)mid << 2) | 0;

    const ulong bestSplit = getBestSplit(sah, subgroupLocalID, scale, defaultSplit);

    struct Split split;
    split.sah = as_float((uint)(bestSplit >> 32));
    split.dim = (uint)bestSplit & 3;
    split.pos = (uint)bestSplit >> 2;

    return split;
}

inline struct Split reduceBinsAndComputeBestSplit32(struct BinInfo2 *binInfo, const float4 scale, uint startID, uint endID)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();

    struct AABB3f boundsX_low = binInfo->boundsX[subgroupLocalID];
    struct AABB3f boundsX_high = binInfo->boundsX[subgroupLocalID + subgroup_size];

    const float2 lr_areaX = left_to_right_area32(&boundsX_low, &boundsX_high);
    const float2 rl_areaX = right_to_left_area32(&boundsX_low, &boundsX_high);

    struct AABB3f boundsY_low = binInfo->boundsY[subgroupLocalID];
    struct AABB3f boundsY_high = binInfo->boundsY[subgroupLocalID + subgroup_size];

    const float2 lr_areaY = left_to_right_area32(&boundsY_low, &boundsY_high);
    const float2 rl_areaY = right_to_left_area32(&boundsY_low, &boundsY_high);

    struct AABB3f boundsZ_low = binInfo->boundsZ[subgroupLocalID];
    struct AABB3f boundsZ_high = binInfo->boundsZ[subgroupLocalID + subgroup_size];

    const float2 lr_areaZ = left_to_right_area32(&boundsZ_low, &boundsZ_high);
    const float2 rl_areaZ = right_to_left_area32(&boundsZ_low, &boundsZ_high);

    const uint3 counts_low = binInfo->counts[subgroupLocalID];
    const uint3 counts_high = binInfo->counts[subgroupLocalID + subgroup_size];

    const uint2 lr_countsX = left_to_right_counts32(counts_low.x, counts_high.x);
    const uint2 rl_countsX = right_to_left_counts32(counts_low.x, counts_high.x);
    const uint2 lr_countsY = left_to_right_counts32(counts_low.y, counts_high.y);
    const uint2 rl_countsY = right_to_left_counts32(counts_low.y, counts_high.y);
    const uint2 lr_countsZ = left_to_right_counts32(counts_low.z, counts_high.z);
    const uint2 rl_countsZ = right_to_left_counts32(counts_low.z, counts_high.z);

    const uint blocks_shift = SAH_LOG_BLOCK_SHIFT;
    uint3 blocks_add = (uint3)((1 << blocks_shift) - 1);

    /* low part: bins 0..15 */
    const float3 lr_area_low = (float3)(lr_areaX.x, lr_areaY.x, lr_areaZ.x);
    const float3 rl_area_low = (float3)(rl_areaX.x, rl_areaY.x, rl_areaZ.x);

#if DIVIDE_BY_6 == 0
    const uint3 lr_count_low = ((uint3)(lr_countsX.x, lr_countsY.x, lr_countsZ.x) + blocks_add) >> blocks_shift;
    const uint3 rl_count_low = ((uint3)(rl_countsX.x, rl_countsY.x, rl_countsZ.x) + blocks_add) >> blocks_shift;

#else
    //const uint3 lr_count_low = ((uint3)(lr_countsX.x,lr_countsY.x,lr_countsZ.x)+BVH_NODE_N6-1) / BVH_NODE_N6;
    //const uint3 rl_count_low = ((uint3)(rl_countsX.x,rl_countsY.x,rl_countsZ.x)+BVH_NODE_N6-1) / BVH_NODE_N6;

    /* skip blocks for breadth-first phase */
    const uint3 lr_count_low = ((uint3)(lr_countsX.x, lr_countsY.x, lr_countsZ.x));
    const uint3 rl_count_low = ((uint3)(rl_countsX.x, rl_countsY.x, rl_countsZ.x));

#endif

    float3 sah_low = fma(lr_area_low, convert_float3(lr_count_low), rl_area_low * convert_float3(rl_count_low));

    /* first bin is invalid */
    // sah_low.x = (subgroupLocalID == 0) ? (float)(INFINITY) : sah_low.x;
    // sah_low.y = (subgroupLocalID == 0) ? (float)(INFINITY) : sah_low.y;
    // sah_low.z = (subgroupLocalID == 0) ? (float)(INFINITY) : sah_low.z;

    sah_low.x = select((float)(INFINITY), sah_low.x, subgroupLocalID != 0);
    sah_low.y = select((float)(INFINITY), sah_low.y, subgroupLocalID != 0);
    sah_low.z = select((float)(INFINITY), sah_low.z, subgroupLocalID != 0);

    /* high part: bins 16..31 */

    const float3 lr_area_high = (float3)(lr_areaX.y, lr_areaY.y, lr_areaZ.y);
    const float3 rl_area_high = (float3)(rl_areaX.y, rl_areaY.y, rl_areaZ.y);
#if DIVIDE_BY_6 == 0
    const uint3 lr_count_high = ((uint3)(lr_countsX.y, lr_countsY.y, lr_countsZ.y) + blocks_add) >> blocks_shift;
    const uint3 rl_count_high = ((uint3)(rl_countsX.y, rl_countsY.y, rl_countsZ.y) + blocks_add) >> blocks_shift;
#else
    //const uint3 lr_count_high = ((uint3)(lr_countsX.y,lr_countsY.y,lr_countsZ.y)+BVH_NODE_N6-1) / BVH_NODE_N6;
    //const uint3 rl_count_high = ((uint3)(rl_countsX.y,rl_countsY.y,rl_countsZ.y)+BVH_NODE_N6-1) / BVH_NODE_N6;

    /* skip blocks for breadth-first phase */
    const uint3 lr_count_high = ((uint3)(lr_countsX.y, lr_countsY.y, lr_countsZ.y));
    const uint3 rl_count_high = ((uint3)(rl_countsX.y, rl_countsY.y, rl_countsZ.y));

#endif
    const float3 sah_high = fma(lr_area_high, convert_float3(lr_count_high), rl_area_high * convert_float3(rl_count_high));

    const uint mid = (startID + endID) / 2;
    const ulong defaultSplit = (((ulong)as_uint((float)(INFINITY))) << 32) | ((uint)mid << 2) | 0;

    const ulong bestSplit_low = getBestSplit(sah_low, subgroupLocalID, scale, defaultSplit);
    const ulong bestSplit_high = getBestSplit(sah_high, subgroupLocalID + subgroup_size, scale, defaultSplit);
    const ulong bestSplit = min(bestSplit_low, bestSplit_high);

    struct Split split;
    split.sah = as_float((uint)(bestSplit >> 32));
    split.dim = (uint)bestSplit & 3;
    split.pos = (uint)bestSplit >> 2;

    return split;
}

// =====================================================================

inline float leafSAH(float geometryArea, uint prims, uint block_shift)
{
    return geometryArea * convert_float((prims + (1 << block_shift) - 1) >> block_shift);
}

inline bool is_left(struct BinMapping *binMapping, struct Split *split, struct AABB *primref)
{
    const uint dim = split->dim;
    const float lower = primref->lower[dim];
    const float upper = primref->upper[dim];
    const float c = lower + upper;
    const uint pos = convert_uint_rtz((c - binMapping->ofs[dim]) * binMapping->scale[dim]);
    return pos < split->pos;
}

inline void serial_find_split(global struct AABB *primref,
                              struct BinMapping *binMapping,
                              struct BuildRecord *buildRecord,
                              local struct Split *split,
                              local struct BinInfo *binInfo,
                              global uint *primref_index0,
                              global uint *primref_index1)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();

    const uint startID = buildRecord->start;
    const uint endID = buildRecord->end;

    subgroup_initBinInfo(binInfo);

    for (uint t = startID + subgroupLocalID; t < endID; t += subgroup_size)
    {
        const uint index = primref_index0[t];
        primref_index1[t] = index;
        atomicUpdateLocalBinInfo_nocheck(binMapping, binInfo, &primref[index]);
    }
}

inline void serial_partition_index(global struct AABB *primref,
                                   struct BinMapping *binMapping,
                                   struct BuildRecord *buildRecord,
                                   struct Split *inSplit,
                                   struct BuildRecord *outLeft,
                                   struct BuildRecord *outRight,
                                   struct AABB *outGeometryBoundsLeft,
                                   struct AABB *outGeometryBoundsRight,
                                   global uint *primref_index0,
                                   global uint *primref_index1)
{
    const uint localID = get_local_id(0);
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroupID = get_sub_group_id();
    const uint subgroup_size = get_sub_group_size();

    const uint begin = buildRecord->start;
    const uint end = buildRecord->end;
    struct Split split = *inSplit;

    struct BuildRecord left;
    struct BuildRecord right;
    initBuildRecord(&left, begin, end);
    initBuildRecord(&right, begin, end);

    struct AABB leftAABB;
    struct AABB rightAABB;
    AABB_init(&leftAABB);
    AABB_init(&rightAABB);

    global uint *l = primref_index0 + begin;
    global uint *r = primref_index0 + end;

    /* no valid split, just split in the middle */
    if (split.sah == (float)(INFINITY))
    {
        for (uint i = begin + subgroupLocalID; i < split.pos; i += subgroup_size)
        {
            const uint index = primref_index1[i];
            const uint count = sub_group_reduce_add(1);
            extendBuildRecord(&left, &primref[index]);
            AABB_extendlu(&leftAABB, primref[index].lower, primref[index].upper);
            l[subgroupLocalID] = index;
            l += count;
        }

        for (uint i = split.pos + subgroupLocalID; i < end; i += subgroup_size)
        {
            const uint index = primref_index1[i];
            const uint count = sub_group_reduce_add(1);
            extendBuildRecord(&right, &primref[index]);
            AABB_extendlu(&rightAABB, primref[index].lower, primref[index].upper);
            r -= count;
            r[subgroupLocalID] = index;
        }
    }
    else
    {
        for (uint i = begin + subgroupLocalID; i < end; i += subgroup_size)
        {
            const uint index = primref_index1[i];
            const uint isLeft = is_left(binMapping, &split, &primref[index]) ? 1 : 0;
            const uint isRight = 1 - isLeft;
            const uint countLeft = sub_group_reduce_add(isLeft);
            const uint countRight = sub_group_reduce_add(isRight);
            const uint prefixLeft = sub_group_scan_exclusive_add(isLeft);
            const uint prefixRight = sub_group_scan_exclusive_add(isRight);

            r -= countRight;

            if (isLeft)
            {
                extendBuildRecord(&left, &primref[index]);
                AABB_extendlu(&leftAABB, primref[index].lower, primref[index].upper);
                l[prefixLeft] = index;
            }
            else
            {
                extendBuildRecord(&right, &primref[index]);
                AABB_extendlu(&rightAABB, primref[index].lower, primref[index].upper);
                r[prefixRight] = index;
            }
            l += countLeft;
        }
    }

    left.centroidBounds = AABB_sub_group_reduce(&left.centroidBounds);
    right.centroidBounds = AABB_sub_group_reduce(&right.centroidBounds);
    leftAABB = AABB_sub_group_reduce(&leftAABB);
    rightAABB = AABB_sub_group_reduce(&rightAABB);

    if (subgroupLocalID == 0)
    {
        uint pos = l - primref_index0; // single first thread needs to compute "pos"
        left.end = pos;
        right.start = pos;

        leftAABB.lower.w = AABB_halfArea(&leftAABB);
        rightAABB.lower.w = AABB_halfArea(&rightAABB);

        leftAABB.upper.w = as_float(getNumPrimsBuildRecord(&left));
        rightAABB.upper.w = as_float(getNumPrimsBuildRecord(&right));

        *outLeft = left;
        *outRight = right;
        *outGeometryBoundsLeft = leftAABB;
        *outGeometryBoundsRight = rightAABB;
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

#if ENABLE_CHECKS == 1
    if (subgroupLocalID == 0)
    {
        if (AABB_verify(outLeft))
        {
            printf("outLeft:\n");
            printBuildRecord(outLeft);
        }
        if (AABB_verify(outRight))
        {
            printf("outRight:\n");
            printBuildRecord(outRight);
        }
        if (AABB_verify(outGeometryBoundsLeft))
        {
            printf("outGeometryBoundsLeft:\n");
            AABB_print(outGeometryBoundsLeft);
        }
        if (AABB_verify(outGeometryBoundsRight))
        {
            printf("outGeometryBoundsRight:\n");
            AABB_print(outGeometryBoundsRight);
        }

        for (uint i = outLeft->start; i < outLeft->end; i++)
        {
            const uint index = primref_index0[i];
            if (split.sah != (float)(INFINITY) && !is_left(binMapping, inSplit, &primref[index]))
                printf("check left %d \n", i);
            if (!AABB_verifyBounds(outLeft, outGeometryBoundsLeft, &primref[index]))
                printf("check prim ref bounds left %d \n", i);
        }
        for (uint i = outRight->start; i < outRight->end; i++)
        {
            const uint index = primref_index0[i];
            if (split.sah != (float)(INFINITY) && is_left(binMapping, inSplit, &primref[index]))
                printf("check right %d \n", i);
            if (!AABB_verifyBounds(outRight, outGeometryBoundsRight, &primref[index]))
                printf("check prim ref bounds right %d \n", i);
        }
    }
#endif
}

inline uint subgroup_createLeaf_index(global struct BlockAllocator *allocator,
                                      const uint start,
                                      const uint end,
                                      global struct AABB *primref,
                                      uint primID,
                                      global char *bvh_mem,
                                      unsigned leafSize)
{
    const uint subgroupLocalID = get_sub_group_local_id();
    const uint subgroup_size = get_sub_group_size();
    const uint items = end - start;

#if ENABLE_CHECKS == 1
    if (items > BVH_LEAF_N_MAX)
        printf("error items %d \n", items);
#endif

    // JDB TODO:  Why was this code commented out??
    //uint offset = (subgroupLocalID == 0) ? alloc_leaf_mem(globals,sizeof(struct Quad)*items) : 0;
    //offset = sub_group_broadcast(offset,0);

    //uint offset = globals->leaf_mem_allocator_start + start * leafSize;
    uint offset = allocator->start + start * leafSize;
    return offset;
}

inline uint get_qnode_index_for_backptr(void *qnode_base, void *qnode)
{
    size_t offset = ((size_t)qnode - (size_t)qnode_base) / sizeof(struct QBVHNodeN);
    uint offset_u = (uint)offset;
#if ENABLE_CHECKS
    if ((size_t)((offset_u << 6) >> 6) != offset)
    {
        printf("get_qnode_index_for_backptr - index out of reach");
    }
#endif
    return offset_u;
}

struct SerialBuildRecurseTemplateConst
{
    unsigned leafSize;
    unsigned leafType;
    bool allocateBackpointers;
};

// ====================================================================================
// ====================================================================================
// ====================================================================================
// ====================================================================================
// ====================================================================================

inline void parallel_find_split(global struct AABB *primref,
                                local struct BuildRecord *buildRecord,
                                local struct Split *bestSplit,
                                local struct BinInfo *binInfo,
                                global uint *primref_index0,
                                global uint *primref_index1)
{
    const uint localID = get_local_id(0);
    const uint local_size = get_local_size(0);
    const uint subgroupID = get_sub_group_id();

    const uint startID = buildRecord->start;
    const uint endID = buildRecord->end;

    struct BinMapping binMapping;
    initBinMapping(&binMapping, &buildRecord->centroidBounds, BINS);

    /* init bininfo */
    parallel_initBinInfo(binInfo);

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    for (uint t = startID + localID; t < endID; t += local_size)
    {
        const uint index = primref_index0[t];
        primref_index1[t] = index;
        atomicUpdateLocalBinInfo(&binMapping, binInfo, &primref[index]);
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    /* find best dimension */

    if (subgroupID == 0)
    {
        *bestSplit = reduceBinsAndComputeBestSplit16(binInfo, binMapping.scale, startID, endID);
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);
}

inline void parallel_find_split32(local uint *local_sync,
                                  global struct AABB *primref,
                                  local struct BuildRecord *buildRecord,
                                  local struct Split *bestSplit,
                                  local struct BinInfo2 *binInfo2,
                                  global uint *primref_index0,
                                  global uint *primref_index1)
{

    const uint localID = get_local_id(0);
    const uint local_size = get_local_size(0);
    const uint subgroupID = get_sub_group_id();
    const uint numSubGroups = get_num_sub_groups();
    const uint subgroupLocalID = get_sub_group_local_id();

    const uint startID = buildRecord->start;
    const uint endID = buildRecord->end;

    struct BinMapping binMapping;
    initBinMapping(&binMapping, &buildRecord->centroidBounds, 2 * BINS);

    /* init bininfo */
    parallel_initBinInfo2(binInfo2, 2 * BINS);

    if (localID == 0)
        *local_sync = 0;

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    for (uint t = startID + localID; t < endID; t += local_size)
    {
        const uint index = primref_index0[t];
        primref_index1[t] = index;
        atomicUpdateLocalBinInfo2(&binMapping, binInfo2, &primref[index]);
    }

    /* find best split position using the last subgroup */
    sub_group_barrier(CLK_LOCAL_MEM_FENCE);
    uint syncID = subgroupLocalID == 0 ? generic_atomic_add(local_sync, 1) : 0;
    syncID = sub_group_broadcast(syncID, 0);

    if (syncID + 1 == numSubGroups)
    {
        *bestSplit = reduceBinsAndComputeBestSplit32(binInfo2, binMapping.scale, startID, endID);
        DBG(if (localID == 0) printSplit(bestSplit));
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);
}

inline void parallel_partition_index(local uint *local_sync,
                                     global struct AABB *primref,
                                     struct BinMapping *binMapping,
                                     const uint begin,
                                     const uint end,
                                     struct Split *inSplit,
                                     local struct BuildRecord *outLeft,
                                     local struct BuildRecord *outRight,
                                     local struct AABB *outGeometryBoundsLeft,
                                     local struct AABB *outGeometryBoundsRight,
                                     global uint *primref_index0,
                                     global uint *primref_index1,
                                     uint *atomicCountLeft,
                                     uint *atomicCountRight)
{
    const uint localID = get_local_id(0);
    const uint local_size = get_local_size(0);
    const uint subgroupID = get_sub_group_id();
    const uint numSubGroups = get_num_sub_groups();
    const uint subgroup_size = get_sub_group_size();
    const uint subgroupLocalID = get_sub_group_local_id();

    const uint size = end - begin;
    struct Split split = *inSplit;

    /* init bin bounds */
    if (localID == 0)
    {
        initBuildRecord(outLeft, begin, end);
        initBuildRecord(outRight, begin, end);
        AABB_init(outGeometryBoundsLeft);
        AABB_init(outGeometryBoundsRight);
        *atomicCountLeft = 0;
        *atomicCountRight = 0;
        *local_sync = 0;
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE); // remove ?

    struct BuildRecord left;
    struct BuildRecord right;
    initBuildRecord(&left, begin, end);
    initBuildRecord(&right, begin, end);

    struct AABB leftAABB;
    struct AABB rightAABB;
    AABB_init(&leftAABB);
    AABB_init(&rightAABB);

    if (split.sah == (float)(INFINITY))
    {
        if (subgroupID == 0)
        {
            for (uint i = begin + subgroupLocalID; i < split.pos; i += subgroup_size)
            {
                const uint index = primref_index1[i];
                extendBuildRecord(&left, &primref[index]);
                AABB_extendlu(&leftAABB, primref[index].lower, primref[index].upper);
                primref_index0[i] = index;
            }

            for (uint i = split.pos + subgroupLocalID; i < end; i += subgroup_size)
            {
                const uint index = primref_index1[i];
                extendBuildRecord(&right, &primref[index]);
                AABB_extendlu(&rightAABB, primref[index].lower, primref[index].upper);
                primref_index0[i] = index;
            }

            left.centroidBounds = AABB_sub_group_reduce(&left.centroidBounds);
            right.centroidBounds = AABB_sub_group_reduce(&right.centroidBounds);
            leftAABB = AABB_sub_group_reduce(&leftAABB);
            rightAABB = AABB_sub_group_reduce(&rightAABB);

            if (localID == 0)
            {
                outLeft->centroidBounds = left.centroidBounds;
                outRight->centroidBounds = right.centroidBounds;

                *outGeometryBoundsLeft = leftAABB;
                *outGeometryBoundsRight = rightAABB;

                outLeft->end = split.pos;
                outRight->start = split.pos;

                outGeometryBoundsLeft->lower.w = AABB_halfArea(outGeometryBoundsLeft);
                outGeometryBoundsRight->lower.w = AABB_halfArea(outGeometryBoundsRight);
                outGeometryBoundsLeft->upper.w = as_float(getNumPrimsBuildRecord(outLeft));
                outGeometryBoundsRight->upper.w = as_float(getNumPrimsBuildRecord(outRight));
            }
        }
    }
    else
    {

        const int startID = begin + ((subgroupID + 0) * size / numSubGroups);
        const int endID = begin + ((subgroupID + 1) * size / numSubGroups);

        for (uint i = startID + subgroupLocalID; i < endID; i += subgroup_size)
        {
            const uint index = primref_index1[i];
            const uint isLeft = is_left(binMapping, &split, &primref[index]) ? 1 : 0;
            const uint isRight = 1 - isLeft;
            const uint countLeft = sub_group_reduce_add(isLeft);
            const uint countRight = sub_group_reduce_add(isRight);
            const uint prefixLeft = sub_group_scan_exclusive_add(isLeft);
            const uint prefixRight = sub_group_scan_exclusive_add(isRight);

            uint offsetLeft = subgroupLocalID == 0 ? generic_atomic_add(atomicCountLeft, countLeft) : 0;
            offsetLeft = sub_group_broadcast(offsetLeft, 0);
            uint offsetRight = subgroupLocalID == 0 ? generic_atomic_add(atomicCountRight, countRight) : 0;
            offsetRight = sub_group_broadcast(offsetRight, 0);

            if (isLeft)
            {
                extendBuildRecord(&left, &primref[index]);
                AABB_extendlu(&leftAABB, primref[index].lower, primref[index].upper);
                primref_index0[begin + offsetLeft + prefixLeft] = index;
            }
            else
            {
                extendBuildRecord(&right, &primref[index]);
                AABB_extendlu(&rightAABB, primref[index].lower, primref[index].upper);
                primref_index0[end - (offsetRight + countRight) + prefixRight] = index;
            }
        }
        left.centroidBounds = AABB_sub_group_reduce(&left.centroidBounds);
        right.centroidBounds = AABB_sub_group_reduce(&right.centroidBounds);
        leftAABB = AABB_sub_group_reduce(&leftAABB);
        rightAABB = AABB_sub_group_reduce(&rightAABB);

        AABB_local_atomic_merge(&outLeft->centroidBounds, left.centroidBounds.lower, left.centroidBounds.upper);
        AABB_local_atomic_merge(&outRight->centroidBounds, right.centroidBounds.lower, right.centroidBounds.upper);

        AABB_local_atomic_merge(outGeometryBoundsLeft, leftAABB.lower, leftAABB.upper);
        AABB_local_atomic_merge(outGeometryBoundsRight, rightAABB.lower, rightAABB.upper);

        sub_group_barrier(CLK_LOCAL_MEM_FENCE);

        if (subgroupLocalID == 0)
        {
            const uint sync = atomic_add(local_sync, 1);
            if (sync + 1 == numSubGroups)
            {
                uint pos = begin + *atomicCountLeft; // single thread of last subgroup needs to compute "pos"
                outLeft->end = pos;
                outRight->start = pos;

                outGeometryBoundsLeft->lower.w = AABB_halfArea(outGeometryBoundsLeft);
                outGeometryBoundsRight->lower.w = AABB_halfArea(outGeometryBoundsRight);
                outGeometryBoundsLeft->upper.w = as_float(getNumPrimsBuildRecord(outLeft));
                outGeometryBoundsRight->upper.w = as_float(getNumPrimsBuildRecord(outRight));
            }
        }
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

#if ENABLE_CHECKS == 1
    if (localID == 0)
    {
        if (outLeft->end <= begin)
            printf("pos begin error\n");
        if (outLeft->end > end)
            printf("pos end error\n");

        for (uint i = outLeft->start; i < outLeft->end; i++)
        {
            const uint index = primref_index0[i];
            //printf("left %d -> %d \n",i,index);
            if (!is_left(binMapping, inSplit, &primref[index]))
                printf("check left %d \n", i);
            if (!AABB_verifyBounds(outLeft, outGeometryBoundsLeft, &primref[index]))
                printf("check prim ref bounds left %d \n", i);
        }
        for (uint i = outRight->start; i < outRight->end; i++)
        {
            const uint index = primref_index0[i];
            //printf("right %d -> %d \n",i,index);
            if (is_left(binMapping, inSplit, &primref[index]))
                printf("check right %d \n", i);
            if (!AABB_verifyBounds(outRight, outGeometryBoundsRight, &primref[index]))
                printf("check prim ref bounds right %d \n", i);
        }
    }
#endif
}


#define ENABLE_LOOP_BREADTH_FIRST 0
#if ENABLE_LOOP_BREADTH_FIRST
// TBD It might be that layout of this impact perf.
struct BreadthFirstLoopLocals
{
    struct BuildRecord local_current;
#if ENABLE_32BINS_IN_BREADTH_FIRST_PHASE == 0
    struct BinInfo binInfo;
#else
    struct BinInfo2 binInfo;
#endif
    struct Split split;
    struct BuildRecord children[BVH_NODE_N + 1];
    struct AABB childrenAABB[BVH_NODE_N + 1];
    uint atomicCountLeft;
    uint atomicCountRight;
    uint local_sync;
    uint recordID;
    uint buildRecordIDs[BUILDRECORD_STACK_SIZE];
    uint numBuildRecordIDs;
    bool exit;
};


inline void parallel_build_breadth_first_loopT(global struct Globals *globals,
                                               global struct AABB *primref,
                                               global uint *primref_index,
                                               global char *bvh_mem,
                                               uint subtreeThreshold,
                                               local struct BreadthFirstLoopLocals *L,
                                               struct BreadthFirstTemplateConst T)
{
    const uint global_size = get_global_size(0);
    const uint local_size = get_local_size(0);
    const uint localID = get_local_id(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);

    const uint subgroupID = get_sub_group_id();
    const uint subgroupLocalID = get_sub_group_local_id();

    /* double buffered primref index array */
    global uint *primref_index0 = primref_index;
    global uint *primref_index1 = primref_index + globals->numPrimitives;

    global struct BuildRecord *records = getBuildRecords(bvh_mem, globals);

#if ENABLE_32BINS_IN_BREADTH_FIRST_PHASE == 0
    const uint bins = BINS;
#else
    const uint bins = 2 * BINS;
#endif

    if (localID == 0)
    {
        L->numBuildRecordIDs = 0;
        L->exit = false;
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    while (1)
    {
        if (localID == 0)
        {
            if (L->numBuildRecordIDs == 0)
            {
                L->recordID = generic_atomic_add(&globals->counter, 1);
                if (L->recordID >= globals->numBuildRecords)
                    L->exit = true;
            }
            else
            {
                L->numBuildRecordIDs--;
                L->recordID = L->buildRecordIDs[L->numBuildRecordIDs];
            }
            L->local_current = records[L->recordID];
        }

        work_group_barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

        /* no more buildrecords available ? */

        if (L->exit)
            break;

        local struct BuildRecord *current = &L->local_current;
        const uint items = getNumPrims(current);
        const uint depth = getBuildRecursionDepth(current);

        global unsigned int *num_records_output = &globals->numBuildRecords_extended;

        struct QBVHNodeN *qnode = (struct QBVHNodeN *)current->current;

        /* ignore small buildrecords */
        if (items < max(subtreeThreshold, cfg_minLeafSize))
        {
            // do nothing
        }
        else
        {
            /*! find best split */
#if ENABLE_32BINS_IN_BREADTH_FIRST_PHASE == 0
            parallel_find_split(primref, current, &L->split, &L->binInfo, primref_index0, primref_index1);
#else
            parallel_find_split32(&L->local_sync, primref, current, &L->split, &L->binInfo, primref_index0, primref_index1);
#endif
            uint numChildren = 2;

            /*! find best split */
            struct BinMapping binMapping;
            initBinMapping(&binMapping, &current->centroidBounds, bins);

            parallel_partition_index(&L->local_sync, primref, &binMapping, current->start, current->end, &L->split, &L->children[0], &L->children[1], &L->childrenAABB[0], &L->childrenAABB[1], primref_index0, primref_index1, &L->atomicCountLeft, &L->atomicCountRight);

            while (numChildren < BVH_NODE_N6)
            {
                /*! find best child to split */
                const uint bestChild = subgroup_getMaxAreaChild(L->childrenAABB, numChildren);
                if (bestChild == -1)
                    break;

                /* perform best found split */
                local struct BuildRecord *brecord = &L->children[bestChild];
                local struct BuildRecord *lrecord = &L->children[numChildren + 0];
                local struct BuildRecord *rrecord = &L->children[numChildren + 1];

#if ENABLE_32BINS_IN_BREADTH_FIRST_PHASE == 0
                parallel_find_split(primref, brecord, &L->split, &L->binInfo, primref_index0, primref_index1);
#else
                parallel_find_split32(&L->local_sync, primref, brecord, &L->split, &L->binInfo, primref_index0, primref_index1);
#endif

                initBinMapping(&binMapping, &brecord->centroidBounds, bins);

                parallel_partition_index(&L->local_sync, primref, &binMapping, brecord->start, brecord->end, &L->split, lrecord, rrecord, &L->childrenAABB[numChildren + 0], &L->childrenAABB[numChildren + 1], primref_index0, primref_index1, &L->atomicCountLeft, &L->atomicCountRight);

                *brecord = *rrecord;
                L->childrenAABB[bestChild] = L->childrenAABB[numChildren + 1];

                work_group_barrier(CLK_LOCAL_MEM_FENCE);

                numChildren++;
            }

            //sub_group_barrier(CLK_LOCAL_MEM_FENCE);

            if (localID <= 16 && subgroupID == 0)
            {
                global struct BVHBase *bvh_base = (global struct BVHBase *)bvh_mem;
                global struct QBVHNodeN *nodes_start = BVHBase_nodeData(bvh_base);
                global uint *back_pointers = BVHBase_backPointers(bvh_base);
                uint qnode_index = 0;
                if (T.allocateBackpointers)
                {
                    /* index of internal node, the domain of backpointers map*/
                    qnode_index = get_qnode_index_for_backptr(nodes_start, qnode);
                    // the backpointer is already set, but we need to add/encode the num of children
                    // todo don't like the need of data read (we should just add), maybe should pass grandpa pointer in record..., or use atomic...
                    back_pointers[qnode_index] += (numChildren << 3);
                }

                /* sort children based on rnage size */
                const uint numPrimsIDs = select((uint)0, (as_uint(L->childrenAABB[subgroupLocalID].upper.w) << 3) | subgroupLocalID, subgroupLocalID < numChildren);
                //const uint IDs = sortBVHChildrenIDs(numPrimsIDs) & (BVH_NODE_N-1);
                const uint IDs = numPrimsIDs & 7;
                const uint pushIDs = convertToPushIndices8(IDs);

                /* alloc #numChildren nodes at once */
                const uint node_offset = alloc_single_node_mem(globals, sizeof(struct QBVHNodeN) * numChildren);

                /* update single relative node pointer and type */
                const int offset = encodeOffset(bvh_mem, (global void *)qnode, node_offset) >> 6;
                const uint type = BVH_INTERNAL_NODE;

                /* set parent pointer in child build records */
                if (subgroupLocalID < numChildren)
                {
                    setBuildRecursionDepth(&L->children[subgroupLocalID], depth + 1);
                    global uchar *child_data_ptr = (global uchar *)bvh_mem + node_offset + pushIDs * sizeof(struct QBVHNodeN);
                    L->children[subgroupLocalID].current = child_data_ptr;
                    if (T.allocateBackpointers)
                    {
                        uint child_index = get_qnode_index_for_backptr(nodes_start, child_data_ptr);
                        back_pointers[child_index] = qnode_index << 6;
                    }
                }

                /* write out qbvh node */
                subgroup_setQBVHNodeN(offset, type, &L->childrenAABB[IDs], numChildren, qnode);

                /* write out child buildrecords to memory */

                uint global_records_offset = (subgroupLocalID == 0) ? atomic_add(num_records_output, numChildren - 1) : 0;
                global_records_offset = sub_group_broadcast(global_records_offset, 0);

                if (localID == 0)
                {
                    records[L->recordID] = L->children[0];
                    L->buildRecordIDs[L->numBuildRecordIDs++] = L->recordID;
                    for (uint i = 1; i < numChildren; i++)
                    {
                        const uint ID = globals->numBuildRecords + global_records_offset + i - 1;
                        records[ID] = L->children[i];
                        L->buildRecordIDs[L->numBuildRecordIDs++] = ID;
                    }
                }
            }
        }
        work_group_barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
    }

    /* last active HW thread ? */
    if (localID == 0)
    {
        const uint sync = atomic_add(&globals->sync, 1);
        if (sync + 1 == numTasks)
        {
            globals->sync = 0;
            /* set final number of buildrecords */
            globals->numBuildRecords += globals->numBuildRecords_extended;
            globals->numBuildRecords_extended = 0;
            globals->counter = 0;
        }
    }
}

__attribute__((reqd_work_group_size(MAX_WORKGROUP_SIZE / 2, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
parallel_build_breadth_first_loop(global struct Globals *globals,
                                  global struct AABB *primref,
                                  global uint *primref_index,
                                  global char *bvh_mem,
                                  uint subtreeThreshold)
{
    local struct BreadthFirstLoopLocals L;
    static const struct BreadthFirstTemplateConst T = {
        false // bool allocateBackpointers;
    };

    parallel_build_breadth_first_loopT(globals,
                                       primref,
                                       primref_index,
                                       bvh_mem,
                                       subtreeThreshold,
                                       &L,
                                       T);
}

__attribute__((reqd_work_group_size(MAX_WORKGROUP_SIZE / 2, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
parallel_build_breadth_first_loop_backpointers(global struct Globals *globals,
                                               global struct AABB *primref,
                                               global uint *primref_index,
                                               global char *bvh_mem,
                                               uint subtreeThreshold)
{
    local struct BreadthFirstLoopLocals L;
    static const struct BreadthFirstTemplateConst T = {
        true // bool allocateBackpointers;
    };

    parallel_build_breadth_first_loopT(globals,
                                       primref,
                                       primref_index,
                                       bvh_mem,
                                       subtreeThreshold,
                                       &L,
                                       T);
}
// ===================================================
// =============== experimental code =================
// ===================================================
#endif

#define ENABLE_GLOBAL_SPLIT 0
#if ENABLE_GLOBAL_SPLIT
inline void parallel_partition_segment_index(local uint *local_sync,
                                             global struct AABB *primref,
                                             struct BinMapping *binMapping,
                                             const uint begin,
                                             const uint end,
                                             const uint global_begin,
                                             const uint global_end,
                                             struct Split *inSplit,
                                             local struct AABB *outLeft,
                                             local struct AABB *outRight,
                                             local struct AABB *outGeometryBoundsLeft,
                                             local struct AABB *outGeometryBoundsRight,
                                             global uint *primref_index0,
                                             global uint *primref_index1,
                                             uint *atomicCountLeft,
                                             uint *atomicCountRight)
{
    const uint localID = get_local_id(0);
    const uint local_size = get_local_size(0);
    const uint subgroupID = get_sub_group_id();
    const uint numSubGroups = get_num_sub_groups();
    const uint subgroup_size = get_sub_group_size();
    const uint subgroupLocalID = get_sub_group_local_id();

    const uint size = end - begin;
    struct Split split = *inSplit;

    /* init bin bounds */
    if (localID == 0)
    {
        AABB_init(outLeft);
        AABB_init(outRight);
        AABB_init(outGeometryBoundsLeft);
        AABB_init(outGeometryBoundsRight);
        *local_sync = 0;
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    struct AABB left;
    struct AABB right;
    AABB_init(&left);
    AABB_init(&right);

    struct AABB leftAABB;
    struct AABB rightAABB;
    AABB_init(&leftAABB);
    AABB_init(&rightAABB);

    const int startID = begin + ((subgroupID + 0) * size / numSubGroups);
    const int endID = begin + ((subgroupID + 1) * size / numSubGroups);

    for (uint i = startID + subgroupLocalID; i < endID; i += subgroup_size)
    {
        const uint index = primref_index1[i];
        const uint isLeft = is_left(binMapping, &split, &primref[index]) ? 1 : 0;
        const uint isRight = 1 - isLeft;
        const uint countLeft = sub_group_reduce_add(isLeft);
        const uint countRight = sub_group_reduce_add(isRight);
        const uint prefixLeft = sub_group_scan_exclusive_add(isLeft);
        const uint prefixRight = sub_group_scan_exclusive_add(isRight);

        uint offsetLeft = subgroupLocalID == 0 ? generic_atomic_add(atomicCountLeft, countLeft) : 0;
        offsetLeft = sub_group_broadcast(offsetLeft, 0);
        uint offsetRight = subgroupLocalID == 0 ? generic_atomic_add(atomicCountRight, countRight) : 0;
        offsetRight = sub_group_broadcast(offsetRight, 0);

        if (isLeft)
        {
            AABB_extend_point(&left, AABB_centroid2(&primref[index]));
            AABB_extendlu(&leftAABB, primref[index].lower, primref[index].upper);
            primref_index0[global_begin + offsetLeft + prefixLeft] = index;
        }
        else
        {
            AABB_extend_point(&right, AABB_centroid2(&primref[index]));
            AABB_extendlu(&rightAABB, primref[index].lower, primref[index].upper);
            primref_index0[global_end - (offsetRight + countRight) + prefixRight] = index;
        }
    }
    left = AABB_sub_group_reduce(&left);
    right = AABB_sub_group_reduce(&right);
    leftAABB = AABB_sub_group_reduce(&leftAABB);
    rightAABB = AABB_sub_group_reduce(&rightAABB);

    AABB_local_atomic_merge(outLeft, left.lower, left.upper);
    AABB_local_atomic_merge(outRight, right.lower, right.upper);

    AABB_local_atomic_merge(outGeometryBoundsLeft, leftAABB.lower, leftAABB.upper);
    AABB_local_atomic_merge(outGeometryBoundsRight, rightAABB.lower, rightAABB.upper);

    work_group_barrier(CLK_LOCAL_MEM_FENCE);
}

__attribute__((reqd_work_group_size(BINS * 2, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel global_init_split_iteration(global struct Globals *globals,
                            global struct GlobalBuildRecord *global_record,
                            global char *bvh_mem,
                            const uint subTreeThreshold)
{
    const uint localID = get_local_id(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);

    global struct BuildRecord *records = getBuildRecords(bvh_mem, globals);

    /* for each build record with size > subTreeThreshold initialize a global build record */

    const uint startID = (taskID + 0) * globals->numBuildRecords / numTasks;
    const uint endID = (taskID + 1) * globals->numBuildRecords / numTasks;

    for (uint i = startID; i < endID; i++)
    {
        global struct BuildRecord *buildRecord = &records[i];
        DBG(if (localID == 0) printf("i %d subTreeThreshold %d size %d \n", i, subTreeThreshold, buildRecord->end - buildRecord->start));

        if ((buildRecord->end - buildRecord->start) > subTreeThreshold)
        {
            uint ID = localID == 0 ? generic_atomic_add(&globals->numGlobalBuildRecords, 1) : 0;

            ID = work_group_broadcast(ID, 0);
            global struct BinInfo2 *binInfo = &global_record[ID].binInfo;
            global struct BinMapping *binMapping = &global_record[ID].binMapping;
            initBinMapping(binMapping, &buildRecord->centroidBounds, 2 * BINS);
            parallel_initBinInfo2(binInfo, 2 * BINS);
            if (localID == 0)
            {
                global_record[ID].range.start = buildRecord->start;
                global_record[ID].range.end = buildRecord->end;
                global_record[ID].atomicCountLeft = 0;
                global_record[ID].atomicCountRight = 0;
                global_record[ID].buildRecordID = i;
                AABB_init(&global_record[ID].leftCentroid);
                AABB_init(&global_record[ID].rightCentroid);
                AABB_init(&global_record[ID].leftGeometry);
                AABB_init(&global_record[ID].rightGeometry);
            }
        }
    }
    DBG(
        work_group_barrier(CLK_LOCAL_MEM_FENCE);
        if (localID == 0)
            printf("globals->numGlobalBuildRecords %d \n", globals->numGlobalBuildRecords););
}

__attribute__((reqd_work_group_size(MAX_WORKGROUP_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel global_bin_iteration(global struct Globals *globals,
                     global struct AABB *primref,
                     global uint *primref_index,
                     global char *bvh_mem,
                     global struct GlobalBuildRecord *global_record)
{
    const uint localID = get_local_id(0);
    const uint blockSize = get_local_size(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);

    const uint numGlobalBuildRecords = globals->numGlobalBuildRecords;

    /* early out */
    if (numGlobalBuildRecords == 0)
        return;

    /* double buffered primref index array */
    global uint *primref_index0 = primref_index;
    global uint *primref_index1 = primref_index + globals->numPrimitives;

    uint numBlocks = 0;

    /* get total number of blocks, size of block == WG size */
    for (uint i = 0; i < numGlobalBuildRecords; i++)
        numBlocks += (global_record[i].range.end - global_record[i].range.start + blockSize - 1) / blockSize;

    const uint startBlockID = (taskID + 0) * numBlocks / numTasks;
    const uint endBlockID = (taskID + 1) * numBlocks / numTasks;
    uint numBlockIDs = endBlockID - startBlockID;

    uint splitRecordID = 0;
    uint offset_start = 0;
    uint offset_end = 0;
    uint cur_blocks = 0;

    for (uint blockCounter = 0; splitRecordID < numGlobalBuildRecords; splitRecordID++)
    {
        const uint sizeRecord = global_record[splitRecordID].range.end - global_record[splitRecordID].range.start;
        const uint blocks = (sizeRecord + blockSize - 1) / blockSize;
        if (startBlockID >= blockCounter && startBlockID < blockCounter + blocks)
        {
            const uint preBlocks = startBlockID - blockCounter;
            cur_blocks = min(numBlockIDs, blocks - preBlocks);
            offset_start = preBlocks * blockSize;
            offset_end = min(offset_start + cur_blocks * blockSize, sizeRecord);
            break;
        }
        blockCounter += blocks;
    }

    if (localID == 0)
        DBG(printf("taskID %d numBlocks %d splitRecordID %d numBlockIDs %d offset_start %d offset_end %d cur_blocks %d \n", taskID, numBlocks, splitRecordID, numBlockIDs, offset_start, offset_end, cur_blocks));

    local struct BinInfo2 local_binInfo;
    parallel_initBinInfo2(&local_binInfo, 2 * BINS);
    struct BinMapping binMapping = global_record[splitRecordID].binMapping;

    while (1)
    {
        work_group_barrier(CLK_LOCAL_MEM_FENCE);

        const uint startID = global_record[splitRecordID].range.start + offset_start;
        const uint endID = global_record[splitRecordID].range.start + offset_end;

        if (localID == 0)
            DBG(printf("taskID %d startID %d endID %d \n", taskID, startID, endID));

        for (uint i = startID + localID; i < endID; i += blockSize)
        {
            const uint index = primref_index0[i];
            primref_index1[i] = index;
            atomicUpdateLocalBinInfo2(&binMapping, &local_binInfo, &primref[index]);
        }

        work_group_barrier(CLK_LOCAL_MEM_FENCE); //FIXME: remove, do local sync
        atomicUpdateGlobalFromLocalBinInfo2(&global_record[splitRecordID].binInfo, &local_binInfo, 2 * BINS);
        work_group_barrier(CLK_LOCAL_MEM_FENCE);

        numBlockIDs -= cur_blocks;
        if (numBlockIDs == 0)
            break;

        splitRecordID++;
        parallel_initBinInfo2(&local_binInfo, 2 * BINS);
        binMapping = global_record[splitRecordID].binMapping;

        const uint sizeRecord = global_record[splitRecordID].range.end - global_record[splitRecordID].range.start;
        const uint blocks = (sizeRecord + blockSize - 1) / blockSize;
        cur_blocks = min(numBlockIDs, blocks);
        offset_start = 0;
        offset_end = min(cur_blocks * blockSize, sizeRecord);

        if (localID == 0)
            DBG(printf("taskID %d numBlocks %d splitRecordID %d numBlockIDs %d offset_start %d offset_end %d cur_blocks %d \n", taskID, numBlocks, splitRecordID, numBlockIDs, offset_start, offset_end, cur_blocks));
    }
}

__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
global_compute_best_split_iteration(global struct Globals *globals,
                                    global char *bvh_mem,
                                    global struct GlobalBuildRecord *global_record)
{
    const uint localID = get_local_id(0);
    const uint blockSize = get_local_size(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);

    const uint numGlobalBuildRecords = globals->numGlobalBuildRecords;

    /* early out */
    if (numGlobalBuildRecords == 0)
        return;

    const uint startRecordID = (taskID + 0) * numGlobalBuildRecords / numTasks;
    const uint endRecordID = (taskID + 1) * numGlobalBuildRecords / numTasks;
    for (uint i = startRecordID; i < endRecordID; i++)
    {
        struct Split split = reduceBinsAndComputeBestSplit32(&global_record[i].binInfo,
                                                             global_record[i].binMapping.scale,
                                                             global_record[i].range.start,
                                                             global_record[i].range.end);
        if (localID == 0)
        {
            global_record[i].split = split;
            global_record[i].atomicCountLeft = 0;
            global_record[i].atomicCountRight = 0;
            DBG(printSplit(&global_record[i].split));
        }
    }
}

__attribute__((reqd_work_group_size(MAX_WORKGROUP_SIZE, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16))) void kernel
global_partition_iteration(global struct Globals *globals,
                           global struct AABB *primref,
                           global uint *primref_index,
                           global char *bvh_mem,
                           global struct GlobalBuildRecord *global_record)
{

    const uint localID = get_local_id(0);
    const uint blockSize = get_local_size(0);
    const uint taskID = get_group_id(0);
    const uint numTasks = get_num_groups(0);

    const uint numGlobalBuildRecords = globals->numGlobalBuildRecords;

    /* early out */
    if (numGlobalBuildRecords == 0)
        return;

    /* double buffered primref index array */
    global uint *primref_index0 = primref_index;
    global uint *primref_index1 = primref_index + globals->numPrimitives;

    uint numBlocks = 0;

    /* get total number of blocks, size of block == WG size */
    for (uint i = 0; i < numGlobalBuildRecords; i++)
        numBlocks += (global_record[i].range.end - global_record[i].range.start + blockSize - 1) / blockSize;

    const uint startBlockID = (taskID + 0) * numBlocks / numTasks;
    const uint endBlockID = (taskID + 1) * numBlocks / numTasks;
    uint numBlockIDs = endBlockID - startBlockID;

    uint splitRecordID = 0;
    uint offset_start = 0;
    uint offset_end = 0;
    uint cur_blocks = 0;

    for (uint blockCounter = 0; splitRecordID < numGlobalBuildRecords; splitRecordID++)
    {
        const uint sizeRecord = global_record[splitRecordID].range.end - global_record[splitRecordID].range.start;
        const uint blocks = (sizeRecord + blockSize - 1) / blockSize;
        if (startBlockID >= blockCounter && startBlockID < blockCounter + blocks)
        {
            const uint preBlocks = startBlockID - blockCounter;
            cur_blocks = min(numBlockIDs, blocks - preBlocks);
            offset_start = preBlocks * blockSize;
            offset_end = min(offset_start + cur_blocks * blockSize, sizeRecord);
            break;
        }
        blockCounter += blocks;
    }

    if (localID == 0)
        DBG(printf("partition taskID %d numBlocks %d splitRecordID %d numBlockIDs %d offset_start %d offset_end %d cur_blocks %d \n", taskID, numBlocks, splitRecordID, numBlockIDs, offset_start, offset_end, cur_blocks));

    local struct AABB centroidAABB[2];
    local struct AABB geometryAABB[2];
    local uint local_sync;

    while (1)
    {

        const uint startID = global_record[splitRecordID].range.start + offset_start;
        const uint endID = global_record[splitRecordID].range.start + offset_end;

        struct BinMapping binMapping = global_record[splitRecordID].binMapping;
        struct Split split = global_record[splitRecordID].split;

        const uint global_start = global_record[splitRecordID].range.start;
        const uint global_end = global_record[splitRecordID].range.end;

        if (localID == 0)
            DBG(printf("partition taskID %d startID %d endID %d numBlocks %d splitRecordID %d numBlockIDs %d offset_start %d offset_end %d cur_blocks %d \n", taskID, startID, endID, numBlocks, splitRecordID, numBlockIDs, offset_start, offset_end, cur_blocks));

        parallel_partition_segment_index(&local_sync, primref, &binMapping, startID, endID, global_start, global_end, &split, &centroidAABB[0], &centroidAABB[1], &geometryAABB[0], &geometryAABB[1], primref_index0, primref_index1, &global_record[splitRecordID].atomicCountLeft, &global_record[splitRecordID].atomicCountRight);

        /* update global structures */
        if (localID == 0)
        {
            AABB_global_atomic_merge(&global_record[splitRecordID].leftCentroid, &centroidAABB[0]);
            AABB_global_atomic_merge(&global_record[splitRecordID].rightCentroid, &centroidAABB[1]);
            AABB_global_atomic_merge(&global_record[splitRecordID].leftGeometry, &geometryAABB[0]);
            AABB_global_atomic_merge(&global_record[splitRecordID].rightGeometry, &geometryAABB[1]);
        }

        numBlockIDs -= cur_blocks;
        if (numBlockIDs == 0)
            break;

        splitRecordID++;

        const uint sizeRecord = global_record[splitRecordID].range.end - global_record[splitRecordID].range.start;
        const uint blocks = (sizeRecord + blockSize - 1) / blockSize;
        cur_blocks = min(numBlockIDs, blocks);
        offset_start = 0;
        offset_end = min(cur_blocks * blockSize, sizeRecord);
    }
}

inline void printBinaryNode(struct AABB *aabb)
{
    printf("lower %f upper %f lower.w %d upper.w %d \n", aabb->lower, aabb->upper, as_uint(aabb->lower.w), as_uint(aabb->upper.w));
}

__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(16)))
void kernel global_finalize_iteration(global struct Globals *globals,
                          global struct GlobalBuildRecord *global_record,
                          global char *bvh_mem,
                          global struct AABB *binary_nodes)
{
    const uint localID = get_local_id(0);
    const uint localSize = get_local_size(0);
    const uint groupID = get_group_id(0);
    const uint numGroups = get_num_groups(0);

    global struct BuildRecord *records = getBuildRecords(bvh_mem, globals);

    for (uint i = localID; i < globals->numGlobalBuildRecords; i += localSize)
    {
        const uint buildRecordID = global_record[i].buildRecordID;
        const uint binaryNodeID = as_uint(records[buildRecordID].centroidBounds.lower.w);
        /* left child buildrecord */
        const uint leftID = buildRecordID;
        records[leftID].start = global_record[i].range.start;
        records[leftID].end = global_record[i].range.start + global_record[i].atomicCountLeft;
        records[leftID].centroidBounds = global_record[i].leftCentroid;
        /* right child buildrecord */
        const uint rightID = generic_atomic_add(&globals->numBuildRecords, 1);
        records[rightID].start = global_record[i].range.start + global_record[i].atomicCountLeft;
        records[rightID].end = global_record[i].range.end;
        records[rightID].centroidBounds = global_record[i].rightCentroid;
        /* two binary nodes */
        const uint binaryChildID = generic_atomic_add(&globals->numGlobalBinaryNodes, 2);
        binary_nodes[binaryNodeID].lower.w = as_float(binaryChildID + 0);
        binary_nodes[binaryNodeID].upper.w = as_float(binaryChildID + 1);
        binary_nodes[binaryChildID + 0] = global_record[i].leftGeometry;
        binary_nodes[binaryChildID + 1] = global_record[i].rightGeometry;
        binary_nodes[binaryChildID + 0].lower.w = as_float(leftID);
        binary_nodes[binaryChildID + 0].upper.w = as_float(-1);
        binary_nodes[binaryChildID + 1].lower.w = as_float(rightID);
        binary_nodes[binaryChildID + 1].upper.w = as_float(-1);
        records[leftID].centroidBounds.lower.w = as_float(binaryChildID + 0);
        records[rightID].centroidBounds.lower.w = as_float(binaryChildID + 1);
    }

    sub_group_barrier(CLK_LOCAL_MEM_FENCE);

    if (localID == 0)
    {
        const uint sync = atomic_add(&globals->sync, 1);
        if (sync + 1 == numGroups)
        {
            globals->sync = 0;
            DBG(printf("globals->numBuildRecords %d \n", globals->numBuildRecords));
            DBG(
                for (uint i = 0; i < globals->numBuildRecords; i++) {
                    printf("i %d \n", i);
                    printBuildRecord(&records[i]);
                } printf("Binary Tree \n");
                for (uint i = 0; i < globals->numGlobalBinaryNodes; i++) {
                    printf("i %d \n", i);
                    printBinaryNode(&binary_nodes[i]);
                }

            );
            globals->numGlobalBuildRecords = 0;
        }
    }
}

__attribute__((reqd_work_group_size(1, 1, 1))) void kernel global_build_top_level(global struct Globals *globals,
                                                                                  global struct GlobalBuildRecord *global_record,
                                                                                  global char *bvh_mem,
                                                                                  global struct AABB *binary_nodes)
{
#define MAX_TOP_LEVEL_STACK_DEPTH 32
    struct AABB stack[MAX_TOP_LEVEL_STACK_DEPTH];
    global uchar *stackParentPtrs[MAX_TOP_LEVEL_STACK_DEPTH];
    struct AABB childrenAABB[BVH_NODE_N6];
    float childrenHalfArea[BVH_NODE_N6];

    /* build records */
    global struct BuildRecord *record = getBuildRecords(bvh_mem, globals);

    struct BVHBase *base = (struct BVHBase *)bvh_mem;
    struct QBVHNodeN *qnode_root = (global struct QBVHNodeN *)(bvh_mem + base->rootNodeOffset);

    uint stack_index = 1;
    stack[0] = binary_nodes[0];
    stackParentPtrs[0] = (global uchar *)qnode_root;

    while (stack_index != 0)
    {
        stack_index--;

        childrenAABB[0] = stack[stack_index];
        struct QBVHNodeN *qnode = (struct QBVHNodeN *)stackParentPtrs[stack_index];
        childrenHalfArea[0] = AABB_halfArea(&childrenAABB[0]);

        /* buildrecord leaf => set parent pointer and continue*/
        DBG(
            printf("stack_index %d \n", stack_index);
            printf("as_uint(childrenAABB[0].upper.w) %d \n", as_uint(childrenAABB[0].upper.w)););

        if (as_uint(childrenAABB[0].upper.w) == -1)
        {
            const uint buildRecordID = as_uint(childrenAABB[0].lower.w);
            DBG(
                printf("leaf buildRecordID %d \n", buildRecordID);
                printBuildRecord(&record[buildRecordID]);)

            record[buildRecordID].current = (global uchar *)qnode;
            continue;
        }

        childrenHalfArea[0] = AABB_halfArea(&childrenAABB[0]);

        uint numChildren = 1;
        while (numChildren < BVH_NODE_N6)
        {
            // FIXME

            /*! find best child to split */
            float bestArea = -(float)INFINITY;
            int bestChild = -1;
            for (int i = 0; i < numChildren; i++)
            {
                /* ignore leaves as they cannot get split */
                if (as_uint(childrenAABB[i].upper.w) == -1)
                    continue;

                /* find child with largest surface area */
                if (childrenHalfArea[i] > bestArea)
                {
                    bestChild = i;
                    bestArea = childrenAABB[i].lower.w;
                }
            }
            if (bestChild == -1)
                break;
            const uint leftID = as_uint(childrenAABB[bestChild].lower.w);
            const uint rightID = as_uint(childrenAABB[bestChild].upper.w);
            childrenAABB[bestChild] = binary_nodes[leftID];
            childrenAABB[numChildren] = binary_nodes[rightID];
            childrenHalfArea[bestChild] = AABB_halfArea(&childrenAABB[bestChild]);
            childrenHalfArea[numChildren] = AABB_halfArea(&childrenAABB[numChildren]);
            numChildren++;
        }

        const uint child_node_offset = alloc_single_node_mem(globals, sizeof(struct QBVHNodeN) * numChildren);

        /* update single relative node pointer */
        const int offset = encodeOffset(bvh_mem, (global void *)qnode, child_node_offset) >> 6;
        const uint type = BVH_INTERNAL_NODE;

        setQBVHNodeN(offset, type, childrenAABB, numChildren, qnode);

        DBG(
            printQBVHNodeN(qnode);
            printf("numChildren %d \n", numChildren);
            for (uint i = 0; i < numChildren; i++)
                AABB_print(&childrenAABB[i]););

        /* update parent pointer of build records of all children */
        for (uint ID = 0; ID < numChildren; ID++)
        {
            stack[stack_index] = childrenAABB[ID];
            stackParentPtrs[stack_index] = (global uchar *)bvh_mem + child_node_offset + ID * sizeof(struct QBVHNodeN);
            stack_index++;
        }
    }
}

#endif
