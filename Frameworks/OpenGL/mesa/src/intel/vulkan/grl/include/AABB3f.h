//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "GRLRTASCommon.h"

#include "affinespace.h"

#ifndef __OPENCL_VERSION__
#   include "stdio.h" //for printf
#endif

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(RTAS)

GRL_INLINE void AABB3f_init(struct AABB3f *aabb)
{
    aabb->lower[0] = (float)(INFINITY);
    aabb->lower[1] = (float)(INFINITY);
    aabb->lower[2] = (float)(INFINITY);

    aabb->upper[0] = -(float)(INFINITY);
    aabb->upper[1] = -(float)(INFINITY);
    aabb->upper[2] = -(float)(INFINITY);
}

GRL_INLINE float3 AABB3f_load_lower( const struct AABB3f* aabb )
{
    float3 v = { aabb->lower[0], aabb->lower[1], aabb->lower[2] };
    return v;
}
GRL_INLINE float3 AABB3f_load_upper( const struct AABB3f* aabb )
{
    float3 v = { aabb->upper[0], aabb->upper[1], aabb->upper[2] };
    return v;
}

GRL_INLINE void AABB3f_extend(struct AABB3f *aabb, const struct AABB3f *v)
{
    aabb->lower[0] = fmin(aabb->lower[0], v->lower[0]);
    aabb->lower[1] = fmin(aabb->lower[1], v->lower[1]);
    aabb->lower[2] = fmin(aabb->lower[2], v->lower[2]);
    aabb->upper[0] = fmax(aabb->upper[0], v->upper[0]);
    aabb->upper[1] = fmax(aabb->upper[1], v->upper[1]);
    aabb->upper[2] = fmax(aabb->upper[2], v->upper[2]);
}

GRL_INLINE void AABB3f_intersect(struct AABB3f* aabb, struct AABB3f inters)
{
    aabb->upper[0] = fmin(inters.upper[0],aabb->upper[0]);
    aabb->upper[1] = fmin(inters.upper[1],aabb->upper[1]);
    aabb->upper[2] = fmin(inters.upper[2],aabb->upper[2]);
    aabb->lower[0] = fmax(inters.lower[0],aabb->lower[0]);
    aabb->lower[1] = fmax(inters.lower[1],aabb->lower[1]);
    aabb->lower[2] = fmax(inters.lower[2],aabb->lower[2]);
}

GRL_INLINE void AABB3f_trim_upper(struct AABB3f* aabb, const float* upper)
{
    aabb->upper[0] = fmin(upper[0], aabb->upper[0]);
    aabb->upper[1] = fmin(upper[1], aabb->upper[1]);
    aabb->upper[2] = fmin(upper[2], aabb->upper[2]);
}

GRL_INLINE void AABB3f_set( struct AABB3f* aabb, float3 lower, float3 upper )
{
    aabb->lower[0] = lower.x ;
    aabb->lower[1] = lower.y ;
    aabb->lower[2] = lower.z ;
    aabb->upper[0] = upper.x ;
    aabb->upper[1] = upper.y ;
    aabb->upper[2] = upper.z ;
}

inline void AABB3f_extend_point(struct AABB3f *aabb, const float3 p)
{
    aabb->lower[0] = fmin(aabb->lower[0], p.x);
    aabb->lower[1] = fmin(aabb->lower[1], p.y);
    aabb->lower[2] = fmin(aabb->lower[2], p.z);
    aabb->upper[0] = fmax(aabb->upper[0], p.x);
    aabb->upper[1] = fmax(aabb->upper[1], p.y);
    aabb->upper[2] = fmax(aabb->upper[2], p.z);
}

GRL_INLINE void AABB3f_extendlu(struct AABB3f *aabb, const float3 lower, const float3 upper)
{
    aabb->lower[0] = fmin(aabb->lower[0], lower.x);
    aabb->lower[1] = fmin(aabb->lower[1], lower.y);
    aabb->lower[2] = fmin(aabb->lower[2], lower.z);
    aabb->upper[0] = fmax(aabb->upper[0], upper.x);
    aabb->upper[1] = fmax(aabb->upper[1], upper.y);
    aabb->upper[2] = fmax(aabb->upper[2], upper.z);
}

GRL_INLINE float3 AABB3f_size(struct AABB3f* aabb)
{
    return AABB3f_load_upper(aabb) - AABB3f_load_lower(aabb);
}

GRL_INLINE float AABB3f_halfArea(struct AABB3f *aabb)
{
    const float3 d = AABB3f_load_upper( aabb ) - AABB3f_load_lower( aabb );
    return d.x* (d.y + d.z) + (d.y * d.z);
}

GRL_INLINE float halfArea_AABB3f(struct AABB3f *aabb) // TODO: Remove me
{
    const float3 d = { aabb->upper[0] - aabb->lower[0], aabb->upper[1] - aabb->lower[1], aabb->upper[2] - aabb->lower[2] };
    return fma(d.x, (d.y + d.z), d.y * d.z);
}

GRL_INLINE void AABB3f_set_lower(struct AABB3f* aabb, float3 lower)
{
    aabb->lower[0] = lower.x;
    aabb->lower[1] = lower.y;
    aabb->lower[2] = lower.z;
}

GRL_INLINE void AABB3f_set_upper(struct AABB3f* aabb, float3 upper)
{
    aabb->upper[0] = upper.x;
    aabb->upper[1] = upper.y;
    aabb->upper[2] = upper.z;
}

GRL_INLINE float3 conservativeExtent(float3 extent)
{
    const float v = FLT_EPSILON * fmax(extent.x, fmax(extent.y, extent.z));
    float3 v3 = { v,v,v };
    extent = extent + v3;
    return extent;
}

inline struct AABB3f GRL_OVERLOADABLE transform_aabb(float3 lower, float3 upper, const float* Transform)
{
#if 1
    // We use an abs-matrix to transform the AABB extent vector, which is enough to compute the area
    //     New AABB is center +- Extent.
    //
    // For derivation see:
    //    https://zeux.io/2010/10/17/aabb-from-obb-with-component-wise-abs/
    //

    float3 Center = (upper + lower) * 0.5f;
    float3 Extent = (conservativeExtent(upper) - lower) * 0.5f;

    float cx = Center.x * Transform[0] + Center.y * Transform[1] + Center.z * Transform[2] + Transform[3];
    float cy = Center.x * Transform[4] + Center.y * Transform[5] + Center.z * Transform[6] + Transform[7];
    float cz = Center.x * Transform[8] + Center.y * Transform[9] + Center.z * Transform[10] + Transform[11];
    float ex = Extent.x * fabs(Transform[0]) + Extent.y * fabs(Transform[1]) + Extent.z * fabs(Transform[2]);
    float ey = Extent.x * fabs(Transform[4]) + Extent.y * fabs(Transform[5]) + Extent.z * fabs(Transform[6]);
    float ez = Extent.x * fabs(Transform[8]) + Extent.y * fabs(Transform[9]) + Extent.z * fabs(Transform[10]);

    Center.x = cx; Center.y = cy;  Center.z = cz;
    Extent.x = ex; Extent.y = ey;  Extent.z = ez;

    struct AABB3f box;
    AABB3f_set_lower(&box, Center - Extent);
    AABB3f_set_upper(&box, Center + Extent);
    return box;
#else
    struct AffineSpace3f xfm = AffineSpace3f_load_row_major(Transform);

    float3 plll = { lower.x, lower.y, lower.z };
    float3 pllu = { lower.x, lower.y, upper.z };
    float3 plul = { lower.x, upper.y, lower.z };
    float3 pluu = { lower.x, upper.y, upper.z };
    float3 pull = { upper.x, lower.y, lower.z };
    float3 pulu = { upper.x, lower.y, upper.z };
    float3 puul = { upper.x, upper.y, lower.z };
    float3 puuu = { upper.x, upper.y, upper.z };
    plll  = xfmPoint(xfm, plll) ;
    pllu  = xfmPoint(xfm, pllu) ;
    plul  = xfmPoint(xfm, plul) ;
    pluu  = xfmPoint(xfm, pluu) ;
    pull  = xfmPoint(xfm, pull) ;
    pulu  = xfmPoint(xfm, pulu) ;
    puul  = xfmPoint(xfm, puul) ;
    puuu  = xfmPoint(xfm, puuu) ;

    float3 p1_min = fmin(plll, pull);
    float3 p2_min = fmin(pllu, pulu);
    float3 p3_min = fmin(plul, puul);
    float3 p4_min = fmin(pluu, puuu);
    float3 p1_max = fmax(plll, pull);
    float3 p2_max = fmax(pllu, pulu);
    float3 p3_max = fmax(plul, puul);
    float3 p4_max = fmax(pluu, puuu);
    p1_min = fmin(p1_min, p3_min);
    p2_min = fmin(p2_min, p4_min);
    p1_max = fmax(p1_max, p3_max);
    p2_max = fmax(p2_max, p4_max);
    p1_min = fmin(p1_min, p2_min);
    p1_max = fmax(p1_max, p2_max);

    AABB3f out = {
        {p1_min.x,p1_min.y,p1_min.z},
        {p1_max.x,p1_max.y,p1_max.z}
    };
    return out;
#endif
}

GRL_INLINE struct AABB3f GRL_OVERLOADABLE transform_aabb(struct AABB3f box, const float* Transform)
{
    float3 lower = { box.lower[0], box.lower[1], box.lower[2] };
    float3 upper = { box.upper[0], box.upper[1], box.upper[2] };
    return transform_aabb(lower, upper, Transform);
}

GRL_INLINE struct AABB3f AABB3f_transform(struct AffineSpace3f xfm, struct AABB3f in)
{
    struct AABB3f out;
    float rmTransform[12];
    load_row_major_from_AffineSpace3f(xfm, rmTransform);
    out = transform_aabb(in, rmTransform);

    return out;
}

GRL_INLINE bool AABB3f_isIn(struct AABB3f bigger, float3 contained)
{
    bool iscontained =
        contained.x >= bigger.lower[0] &&
        contained.y >= bigger.lower[1] &&
        contained.z >= bigger.lower[2] &&
        contained.x <= bigger.upper[0] &&
        contained.y <= bigger.upper[1] &&
        contained.z <= bigger.upper[2];

    return iscontained;
}

GRL_INLINE bool AABB3f_isSubset(struct AABB3f bigger, struct AABB3f contained)
{
    bool iscontained =
        contained.lower[0] >= bigger.lower[0] &&
        contained.lower[1] >= bigger.lower[1] &&
        contained.lower[2] >= bigger.lower[2] &&
        contained.upper[0] <= bigger.upper[0] &&
        contained.upper[1] <= bigger.upper[1] &&
        contained.upper[2] <= bigger.upper[2];

    return iscontained;
}

GRL_INLINE bool AABB3f_is_degenerate(struct AABB3f* box )
{
    return box->lower[0] > box->upper[0] ||
           box->lower[1] > box->upper[1] ||
           box->lower[2] > box->upper[2];
}

GRL_INLINE void AABB3f_print(struct AABB3f *aabb)
{
    printf("AABB {\n");
    printf("  lower = %f, %f, %f\n", aabb->lower[0], aabb->lower[1], aabb->lower[2]);
    printf("  upper = %f, %f, %f\n", aabb->upper[0], aabb->upper[1], aabb->upper[2]);
    printf("}\n");
}



#ifdef __OPENCL_VERSION__
GRL_INLINE struct AABB3f AABB3f_sub_group_shuffle(struct AABB3f *aabb, const uint slotID)
{
    struct AABB3f bounds;
    bounds.lower[0] = intel_sub_group_shuffle(aabb->lower[0], slotID);
    bounds.lower[1] = intel_sub_group_shuffle(aabb->lower[1], slotID);
    bounds.lower[2] = intel_sub_group_shuffle(aabb->lower[2], slotID);
    bounds.upper[0] = intel_sub_group_shuffle(aabb->upper[0], slotID);
    bounds.upper[1] = intel_sub_group_shuffle(aabb->upper[1], slotID);
    bounds.upper[2] = intel_sub_group_shuffle(aabb->upper[2], slotID);
    return bounds;
}

GRL_INLINE struct AABB3f AABB3f_sub_group_reduce(struct AABB3f *aabb)
{
    struct AABB3f bounds;
    bounds.lower[0] = sub_group_reduce_min(aabb->lower[0]);
    bounds.lower[1] = sub_group_reduce_min(aabb->lower[1]);
    bounds.lower[2] = sub_group_reduce_min(aabb->lower[2]);
    bounds.upper[0] = sub_group_reduce_max(aabb->upper[0]);
    bounds.upper[1] = sub_group_reduce_max(aabb->upper[1]);
    bounds.upper[2] = sub_group_reduce_max(aabb->upper[2]);
    return bounds;
}

GRL_INLINE struct AABB3f AABB3f_sub_group_scan_exclusive_min_max(struct AABB3f *aabb)
{
    struct AABB3f bounds;
    bounds.lower[0] = sub_group_scan_exclusive_min(aabb->lower[0]);
    bounds.lower[1] = sub_group_scan_exclusive_min(aabb->lower[1]);
    bounds.lower[2] = sub_group_scan_exclusive_min(aabb->lower[2]);
    bounds.upper[0] = sub_group_scan_exclusive_max(aabb->upper[0]);
    bounds.upper[1] = sub_group_scan_exclusive_max(aabb->upper[1]);
    bounds.upper[2] = sub_group_scan_exclusive_max(aabb->upper[2]);
    return bounds;
}

GRL_INLINE struct AABB3f AABB3f_sub_group_scan_inclusive_min_max(struct AABB3f *aabb)
{
    struct AABB3f bounds;
    bounds.lower[0] = sub_group_scan_inclusive_min(aabb->lower[0]);
    bounds.lower[1] = sub_group_scan_inclusive_min(aabb->lower[1]);
    bounds.lower[2] = sub_group_scan_inclusive_min(aabb->lower[2]);
    bounds.upper[0] = sub_group_scan_inclusive_max(aabb->upper[0]);
    bounds.upper[1] = sub_group_scan_inclusive_max(aabb->upper[1]);
    bounds.upper[2] = sub_group_scan_inclusive_max(aabb->upper[2]);
    return bounds;
}

GRL_INLINE void AABB3f_atomic_merge_local_nocheck(local struct AABB3f *aabb, const float4 lower, const float4 upper)
{
    atomic_min((local float *)&aabb->lower + 0, lower.x);
    atomic_min((local float *)&aabb->lower + 1, lower.y);
    atomic_min((local float *)&aabb->lower + 2, lower.z);
    atomic_max((local float *)&aabb->upper + 0, upper.x);
    atomic_max((local float *)&aabb->upper + 1, upper.y);
    atomic_max((local float *)&aabb->upper + 2, upper.z);
}


GRL_INLINE void AABB3f_atomic_merge_global_lu( global struct AABB3f* aabb, const float3 lower, const float3 upper )
{
    atomic_min( (global float*) & aabb->lower + 0, lower.x );
    atomic_min( (global float*) & aabb->lower + 1, lower.y );
    atomic_min( (global float*) & aabb->lower + 2, lower.z );
    atomic_max( (global float*) & aabb->upper + 0, upper.x );
    atomic_max( (global float*) & aabb->upper + 1, upper.y );
    atomic_max( (global float*) & aabb->upper + 2, upper.z );
}

GRL_INLINE void AABB3f_atomic_merge_local_lu( local struct AABB3f* aabb, const float3 lower, const float3 upper )
{
    atomic_min( (local float*) & aabb->lower + 0, lower.x );
    atomic_min( (local float*) & aabb->lower + 1, lower.y );
    atomic_min( (local float*) & aabb->lower + 2, lower.z );
    atomic_max( (local float*) & aabb->upper + 0, upper.x );
    atomic_max( (local float*) & aabb->upper + 1, upper.y );
    atomic_max( (local float*) & aabb->upper + 2, upper.z );
}

GRL_INLINE void Uniform_AABB3f_atomic_merge_local_sub_group_lu(uniform local struct AABB3f* aabb, const float3 lower, const float3 upper)
{
    float lx = sub_group_reduce_min(lower.x);
    float ly = sub_group_reduce_min(lower.y);
    float lz = sub_group_reduce_min(lower.z);

    float ux = sub_group_reduce_max(upper.x);
    float uy = sub_group_reduce_max(upper.y);
    float uz = sub_group_reduce_max(upper.z);

    if (get_sub_group_local_id() == 0)
    {
        atomic_min((local float*) & aabb->lower + 0, lx);
        atomic_min((local float*) & aabb->lower + 1, ly);
        atomic_min((local float*) & aabb->lower + 2, lz);
        atomic_max((local float*) & aabb->upper + 0, ux);
        atomic_max((local float*) & aabb->upper + 1, uy);
        atomic_max((local float*) & aabb->upper + 2, uz);
    }
}

GRL_INLINE void AABB3f_atomic_merge_global_sub_group_lu(uniform global struct AABB3f* aabb, const float3 lower, const float3 upper)
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

GRL_INLINE void AABB3f_atomic_merge_global( global struct AABB3f* aabb, struct AABB3f* other )
{
    float3 lower = AABB3f_load_lower( other );
    float3 upper = AABB3f_load_upper( other );
    atomic_min( (global float*) & aabb->lower + 0, lower.x );
    atomic_min( (global float*) & aabb->lower + 1, lower.y );
    atomic_min( (global float*) & aabb->lower + 2, lower.z );
    atomic_max( (global float*) & aabb->upper + 0, upper.x );
    atomic_max( (global float*) & aabb->upper + 1, upper.y );
    atomic_max( (global float*) & aabb->upper + 2, upper.z );
}

GRL_INLINE void AABB3f_atomic_merge_localBB_nocheck( local struct AABB3f* aabb, struct AABB3f* bb )
{
    atomic_min( (local float*) & aabb->lower + 0, bb->lower[0] );
    atomic_min( (local float*) & aabb->lower + 1, bb->lower[1] );
    atomic_min( (local float*) & aabb->lower + 2, bb->lower[2] );
    atomic_max( (local float*) & aabb->upper + 0, bb->upper[0] );
    atomic_max( (local float*) & aabb->upper + 1, bb->upper[1] );
    atomic_max( (local float*) & aabb->upper + 2, bb->upper[2] );
}

GRL_INLINE void AABB3f_atomic_merge_local(local struct AABB3f *aabb, const float4 lower, const float4 upper)
{
    if (lower.x < aabb->lower[0])
        atomic_min((local float *)&aabb->lower + 0, lower.x);
    if (lower.y < aabb->lower[1])
        atomic_min((local float *)&aabb->lower + 1, lower.y);
    if (lower.z < aabb->lower[2])
        atomic_min((local float *)&aabb->lower + 2, lower.z);
    if (upper.x > aabb->upper[0])
        atomic_max((local float *)&aabb->upper + 0, upper.x);
    if (upper.y > aabb->upper[1])
        atomic_max((local float *)&aabb->upper + 1, upper.y);
    if (upper.z > aabb->upper[2])
        atomic_max((local float *)&aabb->upper + 2, upper.z);
}

GRL_INLINE void AABB3f_atomic_merge_global_local(global struct AABB3f *dest, local struct AABB3f *source)
{
    float3 l = AABB3f_load_lower(source);
    float3 u = AABB3f_load_upper(source);
    atomic_min((global float *)&dest->lower + 0, l.x );
    atomic_min((global float *)&dest->lower + 1, l.y );
    atomic_min((global float *)&dest->lower + 2, l.z );
    atomic_max((global float *)&dest->upper + 0, u.x );
    atomic_max((global float *)&dest->upper + 1, u.y );
    atomic_max((global float *)&dest->upper + 2, u.z );
}


struct AABB3f AABB3f_construct( float3 min, float3 max )
{
    struct AABB3f bb;
    bb.lower[0] = min.x; bb.lower[1] = min.y; bb.lower[2] = min.z;
    bb.upper[0] = max.x; bb.upper[1] = max.y; bb.upper[2] = max.z;
    return bb;
}

struct AABB3f AABB3f_select( struct AABB3f left, struct AABB3f right, int3 cond )
{
    float3 l = select( AABB3f_load_lower(&left), AABB3f_load_lower(&right), cond );
    float3 u = select( AABB3f_load_upper(&left), AABB3f_load_upper(&right), cond );
    return AABB3f_construct( l, u );
}

#endif

GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)

