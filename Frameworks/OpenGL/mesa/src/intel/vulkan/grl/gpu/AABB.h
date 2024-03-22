//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "shared.h"
#include "intrinsics.h"
#ifndef __OPENCL_VERSION__
#include "stdio.h"
#endif

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(RTAS)
/* ====== QUAD ENCODING config ====== */

#define QUAD_GEOMID_BITS 27 // dxr limit is 2^24 geos... we have headroom
#define QUAD_PRIMID_DIFF_BITS (32 - QUAD_GEOMID_BITS)
#define QUAD_GEOMID_MASK      ((1<<QUAD_GEOMID_BITS)-1)

#define QUAD_PRIMID_BITS 29 // dxr limit is 2^29 prims total within one blas
#define QUAD_PRIMID_MASK  ((1<<QUAD_PRIMID_BITS)-1)

#define INSTANCE_ID_BITS 24
#define INSTANCE_ID_MASK ((1<<INSTANCE_ID_BITS)-1)

// JDB TODO:  Make this a separate, dedicated structure..  Aliasing a float4 AABB as a primref is needlessly obfuscated

typedef struct AABB PrimRef;

GRL_INLINE void AABB_init(struct AABB *aabb)
{
    aabb->lower = (float4)(INFINITY, INFINITY, INFINITY, 0);
    aabb->upper = -(float4)(INFINITY, INFINITY, INFINITY, 0);
}

GRL_INLINE uint PRIMREF_geomID( PrimRef* aabb)
{
    const uint v = as_uint(aabb->lower.w);
    return v & QUAD_GEOMID_MASK;
}

GRL_INLINE uint PRIMREF_primID0( PrimRef* aabb)
{
    return as_uint( aabb->upper.w ) & QUAD_PRIMID_MASK;
}

GRL_INLINE uint PRIMREF_primID1( PrimRef* aabb)
{
    const uint v = as_uint(aabb->lower.w);
    const uint primID0 = as_uint(aabb->upper.w) & QUAD_PRIMID_MASK;
    const uint deltaID = v >> QUAD_GEOMID_BITS;
    const uint primID1 = primID0 + deltaID;
    return primID1;
}

GRL_INLINE uint PRIMREF_geomFlags( PrimRef* aabb )
{
    const uint v = as_uint( aabb->upper.w );
    return (v >> QUAD_PRIMID_BITS) ;
}

GRL_INLINE uint PRIMREF_instanceIndex( PrimRef* aabb )
{
    return as_uint(aabb->lower.w) & INSTANCE_ID_MASK;
}

GRL_INLINE uchar PRIMREF_instanceMask( PrimRef* aabb )
{
    return as_uint(aabb->lower.w) >> INSTANCE_ID_BITS;
}

GRL_INLINE void PRIMREF_setProceduralMetaData( PrimRef* primref, uint geomID, uint primID, uint geomFlags )
{
    /* encode geomID, primID */
    uint flags = (geomFlags << QUAD_PRIMID_BITS);
    primref->lower.w = as_float( geomID );
    primref->upper.w = as_float( primID | flags );
}

GRL_INLINE void PRIMREF_setQuadMetaData( PrimRef* primref, uint primID0, uint primID1, uint geomID, uint geomFlags )
{
    const uint primID_diff = primID1 - primID0;
    uint flags = geomFlags << QUAD_PRIMID_BITS;
    
    primref->lower.w = as_float( geomID | (primID_diff << QUAD_GEOMID_BITS) );
    primref->upper.w = as_float( (primID0 | flags) );
}

GRL_INLINE void PRIMREF_setAABB( PrimRef* primref, float3 lower, float3 upper )
{
    primref->lower.xyz = lower.xyz;
    primref->upper.xyz = upper.xyz;
}

GRL_INLINE PrimRef PRIMREF_set_instance( float3 lower, float3 upper, uint instanceIndex, uint instanceMask, uint rootOffset, bool is_procedural )
{
    PrimRef new_ref;
    new_ref.lower.xyz = lower;
    new_ref.lower.w = as_float(instanceIndex | (instanceMask << 24));
    new_ref.upper.xyz = upper;
    new_ref.upper.w = as_float(rootOffset + (is_procedural? 0x80000000 : 0));    
    return new_ref;
}

GRL_INLINE bool PRIMREF_isProceduralInstance( PrimRef* primref )
{
    return (as_uint(primref->upper.w) & 0x80000000) != 0;
}

GRL_INLINE uint PRIMREF_instanceRootNodeOffset(PrimRef* primref)
{
    return (as_uint(primref->upper.w) & 0x7fffffff);
}

GRL_INLINE float3 PRIMREF_lower( PrimRef* primref )
{
    return primref->lower.xyz;
}
GRL_INLINE float3 PRIMREF_upper( PrimRef* primref )
{
    return primref->upper.xyz;
}

GRL_INLINE void AABB_extend(struct AABB *aabb, struct AABB *v)
{
    aabb->lower = min(aabb->lower, v->lower);
    aabb->upper = max(aabb->upper, v->upper);
}

GRL_INLINE void AABB_extend_point(struct AABB *aabb, const float4 p)
{
    aabb->lower = min(aabb->lower, p);
    aabb->upper = max(aabb->upper, p);
}

GRL_INLINE void AABB_extendlu(struct AABB *aabb, const float4 lower, const float4 upper)
{
    aabb->lower = min(aabb->lower, lower);
    aabb->upper = max(aabb->upper, upper);
}

GRL_INLINE struct AABB AABB_enlarge(struct AABB *aabb, const float v)
{
    struct AABB box;
    box.lower = aabb->lower - (float4)v;
    box.upper = aabb->upper + (float4)v;
    return box;
}

GRL_INLINE void AABB_intersect(struct AABB *aabb, struct AABB *v)
{
    aabb->lower = max(aabb->lower, v->lower);
    aabb->upper = min(aabb->upper, v->upper);
}

GRL_INLINE float4 AABB_size(struct AABB *aabb)
{
    return aabb->upper - aabb->lower;
}

GRL_INLINE float4 AABB_centroid2(struct AABB *aabb)
{
    return aabb->lower + aabb->upper;
}

GRL_INLINE float AABB_halfArea(struct AABB *aabb)
{
    const float4 d = AABB_size(aabb);
    return halfarea(d.xyz);
}

GRL_INLINE float AABB_intersecion_size(struct AABB* aabb, struct AABB* v)
{
    struct AABB temp = *aabb;
    AABB_intersect(&temp, v);
    float4 len = AABB_size(&temp);
    float ret = 0.0f;
    if (len.x >= 0.0f && len.y >= 0.0f && len.z >= 0.0f) {
        float3 v = { len.x, len.y, len.z };
        ret = halfarea(v);
    }
    return ret;
}

GRL_INLINE bool AABB_subset(struct AABB* small, struct AABB* big)
{
    const int4 b0 = small->lower >= big->lower;
    const int4 b1 = small->upper <= big->upper;
    const int4 b = b0 & b1;
    return b.x & b.y & b.z;
}

GRL_INLINE struct AABB AABBfromAABB3f(const struct AABB3f box)
{
    struct AABB box4d = {
        {box.lower[0], box.lower[1], box.lower[2], 0.0f},
        {box.upper[0], box.upper[1], box.upper[2], 0.0f}
    };
    return box4d;
}

GRL_INLINE struct AABB3f AABB3fFromAABB(const struct AABB box)
{
    struct AABB3f box3d = {
        {box.lower[0], box.lower[1], box.lower[2]},
        {box.upper[0], box.upper[1], box.upper[2]}
    };
    return box3d;
}

GRL_INLINE bool AABB_verify(struct AABB* aabb)
{
    bool error = false;
    if (aabb->lower.x > aabb->upper.x)
        error = true;
    if (aabb->lower.y > aabb->upper.y)
        error = true;
    if (aabb->lower.z > aabb->upper.z)
        error = true;
    if (!isfinite(aabb->lower.x))
        error = true;
    if (!isfinite(aabb->lower.y))
        error = true;
    if (!isfinite(aabb->lower.z))
        error = true;
    if (!isfinite(aabb->upper.x))
        error = true;
    if (!isfinite(aabb->upper.y))
        error = true;
    if (!isfinite(aabb->upper.z))
        error = true;
    return error;
}

GRL_INLINE void AABB_print(struct AABB* aabb)
{
    printf("AABB {\n  area = %f\n  lower = %f\n  upper = %f\n  geomID = %i  primID0 = %i  primID1 = %i\n  aabb->lower.w = %x  aabb->upper.w = %x }\n",
        AABB_halfArea(aabb),
        aabb->lower.xyz,
        aabb->upper.xyz,
        PRIMREF_geomID(aabb),
        PRIMREF_primID0(aabb),
        PRIMREF_primID1(aabb),
        as_uint(aabb->lower.w),
        as_uint(aabb->upper.w));
}

#ifdef __OPENCL_VERSION__

GRL_INLINE PrimRef PrimRef_sub_group_shuffle(PrimRef* primRef, const uint slotID)
{
    PrimRef shuffledPrimref;
    shuffledPrimref.lower.x = intel_sub_group_shuffle(primRef->lower.x, slotID);
    shuffledPrimref.lower.y = intel_sub_group_shuffle(primRef->lower.y, slotID);
    shuffledPrimref.lower.z = intel_sub_group_shuffle(primRef->lower.z, slotID);
    shuffledPrimref.lower.w = intel_sub_group_shuffle(primRef->lower.w, slotID);
    shuffledPrimref.upper.x = intel_sub_group_shuffle(primRef->upper.x, slotID);
    shuffledPrimref.upper.y = intel_sub_group_shuffle(primRef->upper.y, slotID);
    shuffledPrimref.upper.z = intel_sub_group_shuffle(primRef->upper.z, slotID);
    shuffledPrimref.upper.w = intel_sub_group_shuffle(primRef->upper.w, slotID);
    return shuffledPrimref;
}

GRL_INLINE struct AABB AABB_sub_group_broadcast(struct AABB *aabb, const uint slotID)
{
    struct AABB bounds;
    bounds.lower.x = sub_group_broadcast(aabb->lower.x, slotID);
    bounds.lower.y = sub_group_broadcast(aabb->lower.y, slotID);
    bounds.lower.z = sub_group_broadcast(aabb->lower.z, slotID);
    bounds.lower.w = 0;
    bounds.upper.x = sub_group_broadcast(aabb->upper.x, slotID);
    bounds.upper.y = sub_group_broadcast(aabb->upper.y, slotID);
    bounds.upper.z = sub_group_broadcast(aabb->upper.z, slotID);
    bounds.upper.w = 0;
    return bounds;
}
GRL_INLINE struct AABB AABB_sub_group_shuffle(struct AABB* aabb, const uint slotID)
{
    struct AABB bounds;
    bounds.lower.x = intel_sub_group_shuffle(aabb->lower.x, slotID);
    bounds.lower.y = intel_sub_group_shuffle(aabb->lower.y, slotID);
    bounds.lower.z = intel_sub_group_shuffle(aabb->lower.z, slotID);
    bounds.lower.w = 0;
    bounds.upper.x = intel_sub_group_shuffle(aabb->upper.x, slotID);
    bounds.upper.y = intel_sub_group_shuffle(aabb->upper.y, slotID);
    bounds.upper.z = intel_sub_group_shuffle(aabb->upper.z, slotID);
    bounds.upper.w = 0;
    return bounds;
}

GRL_INLINE uint AABB_sub_group_shuffle_coordPerLane(struct AABB* aabb, const uint slotID)
{
    float coordData[8] = {
        sub_group_broadcast(aabb->lower.x, slotID),
        sub_group_broadcast(aabb->lower.y, slotID),
        sub_group_broadcast(aabb->lower.z, slotID),
        sub_group_broadcast(aabb->lower.w, slotID),
        sub_group_broadcast(aabb->upper.x, slotID),
        sub_group_broadcast(aabb->upper.y, slotID),
        sub_group_broadcast(aabb->upper.z, slotID),
        sub_group_broadcast(aabb->upper.w, slotID) };

    uint coordDataFiltered;
    const uint lane = get_sub_group_local_id();
    if (lane < 8) coordDataFiltered = as_uint(coordData[lane]);
    return coordDataFiltered;
}

GRL_INLINE struct AABB AABB_sub_group_reduce(struct AABB *aabb)
{
    struct AABB bounds;
    bounds.lower.x = sub_group_reduce_min(aabb->lower.x);
    bounds.lower.y = sub_group_reduce_min(aabb->lower.y);
    bounds.lower.z = sub_group_reduce_min(aabb->lower.z);
    bounds.lower.w = 0;
    bounds.upper.x = sub_group_reduce_max(aabb->upper.x);
    bounds.upper.y = sub_group_reduce_max(aabb->upper.y);
    bounds.upper.z = sub_group_reduce_max(aabb->upper.z);
    bounds.upper.w = 0;
    return bounds;
}


GRL_INLINE struct AABB AABB_sub_group_reduce_N6( struct AABB* aabb )
{
    float3 l = aabb->lower.xyz;
    float3 u = aabb->upper.xyz;
    l = min( l, intel_sub_group_shuffle_down( l, l, 4 ) );
    l = min( l, intel_sub_group_shuffle_down( l, l, 2 ) );
    l = min( l, intel_sub_group_shuffle_down( l, l, 1 ) );
    u = max( u, intel_sub_group_shuffle_down( u, u, 4 ) );
    u = max( u, intel_sub_group_shuffle_down( u, u, 2 ) );
    u = max( u, intel_sub_group_shuffle_down( u, u, 1 ) );
    
    struct AABB bounds;
    bounds.lower.x = l.x;
    bounds.lower.y = l.y;
    bounds.lower.z = l.z;
    bounds.lower.w = 0;
    bounds.upper.x = u.x;
    bounds.upper.y = u.y;
    bounds.upper.z = u.z;
    bounds.upper.w = 0;
    return bounds;
}


GRL_INLINE struct AABB AABB_work_group_reduce(struct AABB *aabb)
{
    struct AABB bounds;
    bounds.lower.x = work_group_reduce_min(aabb->lower.x);
    bounds.lower.y = work_group_reduce_min(aabb->lower.y);
    bounds.lower.z = work_group_reduce_min(aabb->lower.z);
    bounds.upper.x = work_group_reduce_max(aabb->upper.x);
    bounds.upper.y = work_group_reduce_max(aabb->upper.y);
    bounds.upper.z = work_group_reduce_max(aabb->upper.z);
    return bounds;
}

GRL_INLINE struct AABB AABB_sub_group_scan_exclusive_min_max(struct AABB *aabb)
{
    struct AABB bounds;
    bounds.lower.x = sub_group_scan_exclusive_min(aabb->lower.x);
    bounds.lower.y = sub_group_scan_exclusive_min(aabb->lower.y);
    bounds.lower.z = sub_group_scan_exclusive_min(aabb->lower.z);
    bounds.lower.w = 0;
    bounds.upper.x = sub_group_scan_exclusive_max(aabb->upper.x);
    bounds.upper.y = sub_group_scan_exclusive_max(aabb->upper.y);
    bounds.upper.z = sub_group_scan_exclusive_max(aabb->upper.z);
    bounds.upper.w = 0;
    return bounds;
}

GRL_INLINE struct AABB AABB_sub_group_scan_inclusive_min_max(struct AABB *aabb)
{
    struct AABB bounds;
    bounds.lower.x = sub_group_scan_inclusive_min(aabb->lower.x);
    bounds.lower.y = sub_group_scan_inclusive_min(aabb->lower.y);
    bounds.lower.z = sub_group_scan_inclusive_min(aabb->lower.z);
    bounds.lower.w = 0;
    bounds.upper.x = sub_group_scan_inclusive_max(aabb->upper.x);
    bounds.upper.y = sub_group_scan_inclusive_max(aabb->upper.y);
    bounds.upper.z = sub_group_scan_inclusive_max(aabb->upper.z);
    bounds.upper.w = 0;
    return bounds;
}

GRL_INLINE void AABB_global_atomic_merge(global struct AABB *global_aabb, struct AABB *aabb)
{
    atomic_min((volatile __global float *)&global_aabb->lower + 0, aabb->lower.x);
    atomic_min((volatile __global float *)&global_aabb->lower + 1, aabb->lower.y);
    atomic_min((volatile __global float *)&global_aabb->lower + 2, aabb->lower.z);
    atomic_max((volatile __global float *)&global_aabb->upper + 0, aabb->upper.x);
    atomic_max((volatile __global float *)&global_aabb->upper + 1, aabb->upper.y);
    atomic_max((volatile __global float *)&global_aabb->upper + 2, aabb->upper.z);
}

GRL_INLINE void AABB_global_atomic_merge_lu(global struct AABB* global_aabb, float3 lower, float3 upper )
{
    atomic_min((volatile __global float*) & global_aabb->lower + 0, lower.x);
    atomic_min((volatile __global float*) & global_aabb->lower + 1, lower.y);
    atomic_min((volatile __global float*) & global_aabb->lower + 2, lower.z);
    atomic_max((volatile __global float*) & global_aabb->upper + 0, upper.x);
    atomic_max((volatile __global float*) & global_aabb->upper + 1, upper.y);
    atomic_max((volatile __global float*) & global_aabb->upper + 2, upper.z);
}

GRL_INLINE void AABB_global_atomic_merge_sub_group_lu(uniform global struct AABB* aabb, float3 lower, float3 upper)
{
    uint lane = get_sub_group_local_id();
    float l[3];
    l[0] = sub_group_reduce_min(lower.x);
    l[1] = sub_group_reduce_min(lower.y);
    l[2] = sub_group_reduce_min(lower.z);
    float u[3];
    u[0] = sub_group_reduce_max(upper.x);
    u[1] = sub_group_reduce_max(upper.y);
    u[2] = sub_group_reduce_max(upper.z);

    if (lane < 3)
    {
        atomic_min((global float*)&aabb->lower + lane, l[lane]);
        atomic_max((global float*)&aabb->upper + lane, u[lane]);
    }
}


GRL_INLINE void AABB_local_atomic_merge(local struct AABB *aabb, const float4 lower, const float4 upper)
{
    if (lower.x < aabb->lower.x)
        atomic_min((local float *)&aabb->lower + 0, lower.x);
    if (lower.y < aabb->lower.y)
        atomic_min((local float *)&aabb->lower + 1, lower.y);
    if (lower.z < aabb->lower.z)
        atomic_min((local float *)&aabb->lower + 2, lower.z);
    if (upper.x > aabb->upper.x)
        atomic_max((local float *)&aabb->upper + 0, upper.x);
    if (upper.y > aabb->upper.y)
        atomic_max((local float *)&aabb->upper + 1, upper.y);
    if (upper.z > aabb->upper.z)
        atomic_max((local float *)&aabb->upper + 2, upper.z);
}
#endif

GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)