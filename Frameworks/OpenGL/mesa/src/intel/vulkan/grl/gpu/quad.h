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

// JDB TODO:  Use corresponding GRL structures!!!

struct Quad
{
    unsigned int shaderIndex;   // note: also mask
    unsigned int geomIndex;     // note:  also geom flags in upper 2 bits
    unsigned int primIndex0;
    unsigned int primIndex1Delta;
    float v[4][3];
};

GRL_INLINE unsigned int Quad_getGeomIndex(global struct Quad *quad)
{
    return quad->geomIndex;
}

GRL_INLINE unsigned int Quad_getPrimIndex0(global struct Quad *quad)
{
    return quad->primIndex0;
}

GRL_INLINE unsigned int Quad_getPrimIndex1(global struct Quad *quad)
{
    return quad->primIndex0 + (quad->primIndex1Delta & 0xFFFF);
}

GRL_INLINE float3 load_float3(float *p)
{
    return (float3)(p[0], p[1], p[2]);
}

GRL_INLINE float3 load_perm_float3(float *p, const uint3 perm)
{
    return (float3)(p[perm.x], p[perm.y], p[perm.z]);
}

GRL_INLINE float2 load_perm_float2(float *p, const uint2 perm)
{
    return (float2)(p[perm.x], p[perm.y]);
}

GRL_INLINE float load_perm_float(float *p, const uint perm)
{
    return p[perm];
}

GRL_INLINE struct AABB getAABB_Quad(struct Quad *q)
{
    struct AABB aabb;
    const float3 lower = min(min(load_float3(q->v[0]), load_float3(q->v[1])), min(load_float3(q->v[2]), load_float3(q->v[3])));
    const float3 upper = max(max(load_float3(q->v[0]), load_float3(q->v[1])), max(load_float3(q->v[2]), load_float3(q->v[3])));
    aabb.lower = (float4)(lower, 0.0f);
    aabb.upper = (float4)(upper, 0.0f);
    return aabb;
}

GRL_INLINE void Quad_ExtendAABB(struct Quad* q, struct AABB* box)
{
    struct AABB aabb;
    const float3 lower = min(min(load_float3(q->v[0]), load_float3(q->v[1])), min(load_float3(q->v[2]), load_float3(q->v[3])));
    const float3 upper = max(max(load_float3(q->v[0]), load_float3(q->v[1])), max(load_float3(q->v[2]), load_float3(q->v[3])));
    aabb.lower = (float4)(lower, 0.0f);
    aabb.upper = (float4)(upper, 0.0f);
    AABB_extend(box, &aabb);
}

GRL_INLINE float4 getCentroid2_Quad(struct Quad *q)
{
    struct AABB aabb = getAABB_Quad(q);
    return aabb.lower + aabb.upper;
}

GRL_INLINE void setQuad(struct Quad *quad, const float4 v0, const float4 v1, const float4 v2, const float4 v3,
                    const uchar j0, const uchar j1, const uchar j2,
                    const uint geomID, const uint primID0, const uint primID1, const uint geomMask, const uint geomFlags )
{
    quad->v[0][0] = v0.x;
    quad->v[0][1] = v0.y;
    quad->v[0][2] = v0.z;
    quad->v[1][0] = v1.x;
    quad->v[1][1] = v1.y;
    quad->v[1][2] = v1.z;
    quad->v[2][0] = v2.x;
    quad->v[2][1] = v2.y;
    quad->v[2][2] = v2.z;
    quad->v[3][0] = v3.x;
    quad->v[3][1] = v3.y;
    quad->v[3][2] = v3.z;

    quad->shaderIndex = (geomMask << 24) | geomID;
    quad->geomIndex = geomID | (geomFlags << 30);
    quad->primIndex0 = primID0;
    const uint delta = primID1 - primID0;
    const uint j = (((j0) << 0) | ((j1) << 2) | ((j2) << 4));
    quad->primIndex1Delta = delta | (j << 16) | (1 << 22); // single prim in leaf
   
}

GRL_INLINE void setQuadVertices(struct Quad *quad, const float3 v0, const float3 v1, const float3 v2, const float3 v3)
{
    quad->v[0][0] = v0.x;
    quad->v[0][1] = v0.y;
    quad->v[0][2] = v0.z;
    quad->v[1][0] = v1.x;
    quad->v[1][1] = v1.y;
    quad->v[1][2] = v1.z;
    quad->v[2][0] = v2.x;
    quad->v[2][1] = v2.y;
    quad->v[2][2] = v2.z;
    quad->v[3][0] = v3.x;
    quad->v[3][1] = v3.y;
    quad->v[3][2] = v3.z;
}
