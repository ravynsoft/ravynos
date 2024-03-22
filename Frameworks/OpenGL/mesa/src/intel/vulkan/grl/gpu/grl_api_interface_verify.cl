//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "api_interface.h"

__attribute__((reqd_work_group_size(1, 1, 1))) void kernel CopyGeom(
    global struct Geo *src,
    global struct Geo *dst,
    global float4 *vec,
    global ushort *indices,
    dword step)
{
    src = src + get_group_id(0);
    dst = dst + get_group_id(0);
    dst->Flags = src->Flags;
    dst->Type = src->Type;
    if (src->Type == GEOMETRY_TYPE_PROCEDURAL)
    {
        dst->Desc.Procedural.AABBByteStride = src->Desc.Procedural.AABBByteStride;
        dst->Desc.Procedural.AABBCount = src->Desc.Procedural.AABBCount;
        dst->Desc.Procedural.AABBByteStride = src->Desc.Procedural.AABBByteStride;
    }
    else
    {
        dst->Desc.Triangles.pTransformBuffer = src->Desc.Triangles.pTransformBuffer;
        if (step == 0)
            return;
        dst->Desc.Triangles.IndexCount = src->Desc.Triangles.IndexCount;
        if (step == 1)
            return;
        dst->Desc.Triangles.VertexCount = src->Desc.Triangles.VertexCount;
        if (step == 2)
            return;
        dst->Desc.Triangles.IndexFormat = src->Desc.Triangles.IndexFormat;
        if (step == 3)
            return;
        dst->Desc.Triangles.pIndexBuffer = src->Desc.Triangles.pIndexBuffer;
        if (step == 4)
            return;
        dst->Desc.Triangles.pVertexBuffer = src->Desc.Triangles.pVertexBuffer;
        if (step == 5)
            return;
        dst->Desc.Triangles.VertexBufferByteStride = src->Desc.Triangles.VertexBufferByteStride;

        dst->Desc.Triangles.VertexFormat = src->Desc.Triangles.VertexFormat;

        for (uint t = 0; t * 3 < dst->Desc.Triangles.IndexCount; t++)
        {
            uint3 tri = GRL_load_triangle(src, t);
            vec[t * 3] = GRL_load_vertex(src, tri[0]);
            vec[t * 3 + 1] = GRL_load_vertex(src, tri[1]);
            vec[t * 3 + 2] = GRL_load_vertex(src, tri[2]);
        }
    }
}
