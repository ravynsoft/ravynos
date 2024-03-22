//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "GRLRTASCommon.h"

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(RTAS)
inline float3 GRL_OVERLOADABLE cross(const float3 a, const float3 b)
{
    float3 res = { a.y * b.z - a.z * b.y,
                   a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x };
    return res;
}

struct LinearSpace3f
{
    float3 vx;
    float3 vy;
    float3 vz;
};

/* compute the determinant of the matrix */
GRL_INLINE struct LinearSpace3f LinearSpace3f_Constructor(const float3 vx, const float3 vy, const float3 vz)
{
    struct LinearSpace3f xfm;
    xfm.vx = vx;
    xfm.vy = vy;
    xfm.vz = vz;
    return xfm;
}

/* compute the determinant of the matrix */
GRL_INLINE float LinearSpace3f_det(struct LinearSpace3f xfm)
{
    return dot(xfm.vx, cross(xfm.vy, xfm.vz));
}

/* compute transposed matrix */
GRL_INLINE struct LinearSpace3f LinearSpace3f_transpose(struct LinearSpace3f in)
{
    float3 x = { in.vx.x, in.vy.x, in.vz.x };
    float3 y = { in.vx.y, in.vy.y, in.vz.y };
    float3 z = { in.vx.z, in.vy.z, in.vz.z };

    return LinearSpace3f_Constructor(x,
                                     y,
                                     z);
}

/* compute adjoint matrix */
GRL_INLINE const struct LinearSpace3f LinearSpace3f_adjoint(struct LinearSpace3f in)
{
    return LinearSpace3f_transpose(LinearSpace3f_Constructor(cross(in.vy, in.vz),
                                                             cross(in.vz, in.vx),
                                                             cross(in.vx, in.vy)));
}

/* compute inverse matrix */
GRL_INLINE struct LinearSpace3f LinearSpace3f_invert(struct LinearSpace3f in)
{
    const float det = LinearSpace3f_det(in);
    const struct LinearSpace3f adj = LinearSpace3f_adjoint(in);
    return LinearSpace3f_Constructor(adj.vx / det, adj.vy / det, adj.vz / det);
}

GRL_INLINE float3 GRL_OVERLOADABLE xfmPoint(struct LinearSpace3f xfm, float3 p)
{
    return xfm.vx * p.x + xfm.vy * p.y + xfm.vz * p.z;
}

struct AffineSpace3f
{
    struct LinearSpace3f l;
    float3 p;
};

GRL_INLINE struct AffineSpace3f AffineSpace3f_Constructor(struct LinearSpace3f l, float3 p)
{
    struct AffineSpace3f out;
    out.l = l;
    out.p = p;
    return out;
}

GRL_INLINE struct AffineSpace3f AffineSpace3f_load_row_major(const float *in)
{
    struct AffineSpace3f out;
    out.l.vx.x = in[0];
    out.l.vx.y = in[4];
    out.l.vx.z = in[8];
    out.l.vy.x = in[1];
    out.l.vy.y = in[5];
    out.l.vy.z = in[9];
    out.l.vz.x = in[2];
    out.l.vz.y = in[6];
    out.l.vz.z = in[10];
    out.p.x = in[3];
    out.p.y = in[7];
    out.p.z = in[11];
    return out;
}

// squared proportion of oriented transformed cube to aa box that would contain it.
// the smaller it is the more overhead transformation produces
GRL_INLINE
float transformation_bbox_surf_overhead(const float* Transform)
{
    // We use an abs-matrix to transform the AABB extent vector, which is enough to compute the area
    //     New AABB is center +- Extent.
    //
    // For derivation see:
    //    https://zeux.io/2010/10/17/aabb-from-obb-with-component-wise-abs/
    //


    // take the cube of side 1 and see how big aabb containing it transformed is vs just surface of transformed
    float ex = fabs(Transform[0]) + fabs(Transform[1]) + fabs(Transform[2]);
    float ey = fabs(Transform[4]) + fabs(Transform[5]) + fabs(Transform[6]);
    float ez = fabs(Transform[8]) + fabs(Transform[9]) + fabs(Transform[10]);

    // we will compare squared sizes
    ex = ex * ex;
    ey = ey * ey;
    ez = ez * ez;

    // surface of aabb containing oriented box;
    float aabb_sq_half_surf = ex * ey + ey * ez + ez * ex;

    // ^2 lengths of transformed <1,0,0>, <0,1,0>, <0,0,1>
    float obx = Transform[0] * Transform[0] + Transform[4] * Transform[4] + Transform[8] * Transform[8];
    float oby = Transform[1] * Transform[1] + Transform[5] * Transform[5] + Transform[9] * Transform[9];
    float obz = Transform[2] * Transform[2] + Transform[6] * Transform[6] + Transform[10] * Transform[10];

    float obb_sq_half_surf = obx * oby + oby * obz + obz * obx;

    return obb_sq_half_surf / aabb_sq_half_surf;

    // ex = 2.0
    // ey = 2.0
    // ez = 2.0
    // ex = 4.0
    // ey = 4.0
    // ez = 4.0
    // aabb_half_surf = 16+16 *2.0 +  2.0*2.0+ 2.0*2.0; = 12;
    // aabb_sq_half_surf = 144;
    //
    // obx = 4.0;
    // oby = 4.0;
    // obz = 4.0;
    // obb_sq_half_surf = 16 + 16+ 16;
    // obb_sq_half_surf = 16.0 *3 = 48
}

GRL_INLINE void load_row_major_from_AffineSpace3f(struct AffineSpace3f in, float* out)
{
    out[0]  = in.l.vx.x;
    out[4]  = in.l.vx.y;
    out[8]  = in.l.vx.z;
    out[1]  = in.l.vy.x;
    out[5]  = in.l.vy.y;
    out[9]  = in.l.vy.z;
    out[2]  = in.l.vz.x;
    out[6]  = in.l.vz.y;
    out[10] = in.l.vz.z;

    out[3]  = in.p.x;
    out[7]  = in.p.y;
    out[11] = in.p.z;
}

GRL_INLINE float3 GRL_OVERLOADABLE xfmPoint(struct AffineSpace3f xfm, float3 p)
{
    return xfmPoint(xfm.l, p) + xfm.p;
}

/* compute inverse matrix */
GRL_INLINE struct AffineSpace3f AffineSpace3f_invert(struct AffineSpace3f in)
{
    const struct LinearSpace3f il = LinearSpace3f_invert(in.l);
    float3 ip = -xfmPoint(il, in.p);
    return AffineSpace3f_Constructor(il, ip);
}

GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)
