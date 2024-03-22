//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once
#include "GRLStructs.h"
#include "shared.h"
#include "libs/lsc_intrinsics.h"

typedef struct Geo GRL_RAYTRACING_GEOMETRY_DESC;

typedef struct GRL_RAYTRACING_AABB
{
    float MinX;
    float MinY;
    float MinZ;
    float MaxX;
    float MaxY;
    float MaxZ;
} GRL_RAYTRACING_AABB;

GRL_INLINE void GLR_set_raytracing_aabb(GRL_RAYTRACING_AABB* dest, struct AABB* source)
{
    dest->MinX = source->lower.x;
    dest->MinY = source->lower.y;
    dest->MinZ = source->lower.z;
    dest->MaxX = source->upper.x;
    dest->MaxY = source->upper.y;
    dest->MaxZ = source->upper.z;
}

GRL_INLINE uint3 GRL_load_triangle(global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, const uint triID)
{
    global char* indices = (global char*)geomDesc->Desc.Triangles.pIndexBuffer;
    uint index_format = geomDesc->Desc.Triangles.IndexFormat;

    if (index_format == INDEX_FORMAT_R32_UINT)
    {
        const uint* data = (const uint*)(indices + triID * 3 * 4);
        return (uint3)(data[0], data[1], data[2]);
    }
    else if (index_format == INDEX_FORMAT_NONE)
    {
        return (uint3)(triID * 3, triID * 3 + 1, triID * 3 + 2);
    }
    else
    {
        const ushort* data = (const ushort*)(indices + triID * 3 * 2);
        return (uint3)(data[0], data[1], data[2]);
    }
}

GRL_INLINE uint3 GRL_load_indices_from_buffer(global char* indices, const uint index_format, const uint triID)
{
    if (index_format == INDEX_FORMAT_R32_UINT)
    {
        return load_uint3_L1C_L3C((global uint3*)(indices + triID * 3 * 4), 0);
    }
    else if (index_format == INDEX_FORMAT_NONE)
    {
        return (uint3)(triID * 3, triID * 3 + 1, triID * 3 + 2);
    }
    else
    {
        const ushort* data = (const ushort*)(indices + triID * 3 * 2);
        return (uint3)(data[0], data[1], data[2]);
    }
}

// Load all 3 indices from one triangle, and a single index from another
GRL_INLINE uint4 GRL_load_quad_indices(global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, uint triID, uint triID_1, ushort fourth_vert)
{
    global char* indices = (global char*)geomDesc->Desc.Triangles.pIndexBuffer;
    uint index_format = geomDesc->Desc.Triangles.IndexFormat;

    if (index_format == INDEX_FORMAT_R32_UINT)
    {
        const uint* data0 = (const uint*)(indices + triID * 3 * 4);
        const uint* data1 = (const uint*)(indices + triID_1 * 3 * 4);
        return (uint4)(data0[0], data0[1], data0[2], data1[fourth_vert]);
    }
    else if (index_format == INDEX_FORMAT_NONE)
    {
        return (uint4)(triID * 3, triID * 3 + 1, triID * 3 + 2, triID_1 * 3 + fourth_vert);
    }
    else
    {
        const ushort* data0 = (const ushort*)(indices + triID * 3 * 2);
        const ushort* data1 = (const ushort*)(indices + triID_1 * 3 * 2);
        return (uint4)(data0[0], data0[1], data0[2], data1[fourth_vert]);
    }
}

GRL_INLINE void GRL_set_Type(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, GeometryType type)
{
    geomDesc->Type = type;
}

GRL_INLINE GeometryType GRL_get_Type(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Type;
}

GRL_INLINE void GRL_set_Flags(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, uint8_t flags)
{
    geomDesc->Flags = flags;
}

GRL_INLINE uint8_t GRL_get_Flags(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Flags;
}

GRL_INLINE void GRL_set_triangles_Transform(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, gpuva_t transform)
{
    geomDesc->Desc.Triangles.pTransformBuffer = transform;
}

GRL_INLINE gpuva_t GRL_get_triangles_Transform(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Triangles.pTransformBuffer;
}

GRL_INLINE void GRL_set_triangles_IndexFormat(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, IndexFormat format)
{
    geomDesc->Desc.Triangles.IndexFormat = format;
}

GRL_INLINE IndexFormat GRL_get_triangles_IndexFormat(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Triangles.IndexFormat;
}

GRL_INLINE void GRL_set_triangles_VertexFormat(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, VertexFormat format)
{
    geomDesc->Desc.Triangles.VertexFormat = format;
}

GRL_INLINE VertexFormat GRL_get_triangles_VertexFormat(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Triangles.VertexFormat;
}

GRL_INLINE void GRL_set_triangles_IndexCount(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, dword count)
{
    geomDesc->Desc.Triangles.IndexCount = count;
}

GRL_INLINE dword GRL_get_triangles_IndexCount(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Triangles.IndexCount;
}

GRL_INLINE void GRL_set_triangles_VertexCount(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, dword count)
{
    geomDesc->Desc.Triangles.VertexCount = count;
}

GRL_INLINE dword GRL_get_triangles_VertexCount(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Triangles.VertexCount;
}

GRL_INLINE void GRL_set_triangles_IndexBuffer(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, gpuva_t buffer)
{
    geomDesc->Desc.Triangles.pIndexBuffer = buffer;
}

GRL_INLINE gpuva_t GRL_get_triangles_IndexBuffer(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Triangles.pIndexBuffer;
}

GRL_INLINE void GRL_set_triangles_VertexBuffer_StartAddress(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, gpuva_t address)
{
    geomDesc->Desc.Triangles.pVertexBuffer = address;
}

GRL_INLINE gpuva_t GRL_get_triangles_VertexBuffer_StartAddress(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Triangles.pVertexBuffer;
}

GRL_INLINE void GRL_set_triangles_VertexBuffer_StrideInBytes(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, unsigned long stride)
{
    geomDesc->Desc.Triangles.VertexBufferByteStride = stride;
}

GRL_INLINE unsigned long GRL_get_triangles_VertexBuffer_StrideInBytes(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Triangles.VertexBufferByteStride;
}

GRL_INLINE unsigned long GRL_get_triangles_IndexFormatSizeInBytes(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return (unsigned long)(geomDesc->Desc.Triangles.IndexFormat);
}

GRL_INLINE void GRL_set_procedurals_AABBCount(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, dword count)
{
    geomDesc->Desc.Procedural.AABBCount = count;
}

GRL_INLINE dword GRL_get_procedurals_AABBCount(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Procedural.AABBCount;
}

GRL_INLINE void GRL_set_procedurals_AABBs_StartAddress(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, gpuva_t address)
{
    geomDesc->Desc.Procedural.pAABBs_GPUVA = address;
}

GRL_INLINE gpuva_t GRL_get_procedurals_AABBs_StartAddress(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Procedural.pAABBs_GPUVA;
}

GRL_INLINE void GRL_set_procedurals_AABBs_StrideInBytes(GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, qword stride)
{
    geomDesc->Desc.Procedural.AABBByteStride = stride;
}

GRL_INLINE qword GRL_get_procedurals_AABBs_StrideInBytes(const GRL_RAYTRACING_GEOMETRY_DESC* geomDesc)
{
    return geomDesc->Desc.Procedural.AABBByteStride;
}

GRL_INLINE uint GRL_is_procedural(GRL_RAYTRACING_GEOMETRY_DESC* desc)
{
    return desc->Type == (unsigned char)GEOMETRY_TYPE_PROCEDURAL;
}

GRL_INLINE uint GRL_is_triangle(GRL_RAYTRACING_GEOMETRY_DESC* desc)
{
    return desc->Type != (unsigned char)GEOMETRY_TYPE_PROCEDURAL;
}

GRL_INLINE unsigned int GRL_get_ShaderIndex_Mask(GRL_RAYTRACING_GEOMETRY_DESC* desc)
{
    return 0x00FFFFFF;
}

GRL_INLINE dword GRL_atomic_add_triangles_VertexCount(GRL_RAYTRACING_GEOMETRY_DESC* desc, dword value)
{
    return atomic_add((global uint*) & desc->Desc.Triangles.VertexCount, value);
}

GRL_INLINE unsigned int GRL_get_primitive_count(GRL_RAYTRACING_GEOMETRY_DESC* desc)
{
    if (GRL_is_triangle(desc))
    {
        if (desc->Desc.Triangles.IndexFormat == INDEX_FORMAT_NONE)
        {
            return desc->Desc.Triangles.VertexCount / 3;
        }
        else
        {
            return desc->Desc.Triangles.IndexCount / 3;
        }
    }
    else
    {
        return desc->Desc.Procedural.AABBCount;
    }
}

#pragma OPENCL EXTENSION cl_khr_fp16 : enable // to leaf half values

GRL_INLINE float snorm_to_float(short v)
{
    return min(1.0f, max(-1.0f, ((float)v) * (1.0f / 32767.0f))); // FIXME: do we have intrinsic for this?
}

GRL_INLINE float snorm8_to_float(signed char v)
{
    return min(1.0f, max(-1.0f, ((float)v) * (1.0f / 127.0f))); // FIXME: do we have intrinsic for this?
}

GRL_INLINE float unorm_to_float(unsigned short v)
{
    return min(1.0f, max(0.0f, ((float)v) * (1.0f / 65535.0f))); // FIXME: do we have intrinsic for this?
}

//only lower 10 bits of v are used
GRL_INLINE float unorm10_to_float(unsigned v)
{
    const unsigned short mask = (unsigned short)((1u << 10u) - 1u);
    const unsigned short v10 = (unsigned short)v & mask;
    return min(1.0f, max(0.0f, ((float)v10) * (1.0f / 1023.0f))); // FIXME: do we have intrinsic for this?
}

GRL_INLINE float unorm8_to_float(unsigned char v)
{
    return min(1.0f, max(0.0f, ((float)v) * (1.0f / 255.0f))); // FIXME: do we have intrinsic for this?
}

GRL_INLINE float4 GRL_load_vertex(global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, const uint vtxID)
{
    float4 v = (float4)(0, 0, 0, 0);
    global char* vertices = (global char*)geomDesc->Desc.Triangles.pVertexBuffer;
    uint vertex_stride = geomDesc->Desc.Triangles.VertexBufferByteStride;
    uint vertex_format = geomDesc->Desc.Triangles.VertexFormat;

    if (vertex_format == VERTEX_FORMAT_R32G32B32_FLOAT)
    {
        const float* data = (const float*)(vertices + vtxID * vertex_stride);
        v = (float4)(data[0], data[1], data[2], 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R32G32_FLOAT)
    {
        const float* data = (const float*)(vertices + vtxID * vertex_stride);
        v = (float4)(data[0], data[1], 0.0f, 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_FLOAT)
    {
        const half* data = (const half*)(vertices + vtxID * vertex_stride);
        v = (float4)(data[0], data[1], data[2], 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_FLOAT)
    {
        const half* data = (const half*)(vertices + vtxID * vertex_stride);
        v = (float4)(data[0], data[1], 0.0f, 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_SNORM)
    {
        const short* data = (const short*)(vertices + vtxID * vertex_stride);
        v = (float4)(snorm_to_float(data[0]),
            snorm_to_float(data[1]),
            snorm_to_float(data[2]),
            0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_SNORM)
    {
        const short* data = (const short*)(vertices + vtxID * vertex_stride);
        v = (float4)(snorm_to_float(data[0]),
            snorm_to_float(data[1]),
            0.0f,
            0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_UNORM)
    {
        const unsigned short* data = (const unsigned short*)(vertices + vtxID * vertex_stride);
        v = (float4)(unorm_to_float(data[0]),
            unorm_to_float(data[1]),
            unorm_to_float(data[2]),
            0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_UNORM)
    {
        const unsigned short* data = (const unsigned short*)(vertices + vtxID * vertex_stride);
        v = (float4)(unorm_to_float(data[0]),
            unorm_to_float(data[1]),
            0.0f,
            0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R10G10B10A2_UNORM)
    {
        const unsigned data = *(const unsigned*)(vertices + vtxID * vertex_stride);
        v = (float4)(unorm10_to_float(data),
            unorm10_to_float((data >> 10)),
            unorm10_to_float((data >> 20)),
            0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8B8A8_UNORM)
    {
        const unsigned char* data = (const unsigned char*)(vertices + vtxID * vertex_stride);
        v = (float4)(unorm8_to_float(data[0]),
            unorm8_to_float(data[1]),
            unorm8_to_float(data[2]),
            0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8_UNORM)
    {
        const unsigned char* data = (const unsigned char*)(vertices + vtxID * vertex_stride);
        v = (float4)(unorm8_to_float(data[0]),
            unorm8_to_float(data[1]),
            0.0f,
            0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8B8A8_SNORM)
    {
        const unsigned char* data = (const unsigned char*)(vertices + vtxID * vertex_stride);
        v = (float4)(snorm8_to_float(data[0]),
            snorm8_to_float(data[1]),
            snorm8_to_float(data[2]),
            0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8_SNORM)
    {
        const unsigned char* data = (const unsigned char*)(vertices + vtxID * vertex_stride);
        v = (float4)(snorm8_to_float(data[0]),
            snorm8_to_float(data[1]),
            0.0f,
            0.0f);
    }

    /* perform vertex transformation */
    if (geomDesc->Desc.Triangles.pTransformBuffer)
    {
        global float* xfm = (global float*)geomDesc->Desc.Triangles.pTransformBuffer;
        const float x = xfm[0] * v.x + xfm[1] * v.y + xfm[2] * v.z + xfm[3];
        const float y = xfm[4] * v.x + xfm[5] * v.y + xfm[6] * v.z + xfm[7];
        const float z = xfm[8] * v.x + xfm[9] * v.y + xfm[10] * v.z + xfm[11];
        v = (float4)(x, y, z, 0.0f);
    }

    return v;
}

GRL_INLINE void GRL_load_triangle_vertices(global char* vertices, const uint vertex_format, const uint vertex_stride, global float* transform_buffer, const uint vtx0ID, const uint vtx1ID, const uint vtx2ID, float4* out)
{
    if (vertex_format == VERTEX_FORMAT_R32G32B32_FLOAT)
    {
        const float3 data0 = as_float3(load_uint3_L1C_L3C((global uint3*)(vertices + vtx0ID * vertex_stride), 0));
        const float3 data1 = as_float3(load_uint3_L1C_L3C((global uint3*)(vertices + vtx1ID * vertex_stride), 0));
        const float3 data2 = as_float3(load_uint3_L1C_L3C((global uint3*)(vertices + vtx2ID * vertex_stride), 0));
        out[0] = (float4)(data0[0], data0[1], data0[2], 0.0f);
        out[1] = (float4)(data1[0], data1[1], data1[2], 0.0f);
        out[2] = (float4)(data2[0], data2[1], data2[2], 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R32G32_FLOAT)
    {
        const float* data0 = (const float*)(vertices + vtx0ID * vertex_stride);
        const float* data1 = (const float*)(vertices + vtx1ID * vertex_stride);
        const float* data2 = (const float*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(data0[0], data0[1], 0.0f, 0.0f);
        out[1] = (float4)(data1[0], data1[1], 0.0f, 0.0f);
        out[2] = (float4)(data2[0], data2[1], 0.0f, 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_FLOAT)
    {
        const half* data0 = (const half*)(vertices + vtx0ID * vertex_stride);
        const half* data1 = (const half*)(vertices + vtx1ID * vertex_stride);
        const half* data2 = (const half*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(data0[0], data0[1], data0[2], 0.0f);
        out[1] = (float4)(data1[0], data1[1], data1[2], 0.0f);
        out[2] = (float4)(data2[0], data2[1], data2[2], 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_FLOAT)
    {
        const half* data0 = (const half*)(vertices + vtx0ID * vertex_stride);
        const half* data1 = (const half*)(vertices + vtx1ID * vertex_stride);
        const half* data2 = (const half*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(data0[0], data0[1], 0.0f, 0.0f);
        out[1] = (float4)(data1[0], data1[1], 0.0f, 0.0f);
        out[2] = (float4)(data2[0], data2[1], 0.0f, 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_SNORM)
    {
        const short* data0 = (const short*)(vertices + vtx0ID * vertex_stride);
        const short* data1 = (const short*)(vertices + vtx1ID * vertex_stride);
        const short* data2 = (const short*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(snorm_to_float(data0[0]), snorm_to_float(data0[1]), snorm_to_float(data0[2]), 0.0f);
        out[1] = (float4)(snorm_to_float(data1[0]), snorm_to_float(data1[1]), snorm_to_float(data1[2]), 0.0f);
        out[2] = (float4)(snorm_to_float(data2[0]), snorm_to_float(data2[1]), snorm_to_float(data2[2]), 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_SNORM)
    {
        const short* data0 = (const short*)(vertices + vtx0ID * vertex_stride);
        const short* data1 = (const short*)(vertices + vtx1ID * vertex_stride);
        const short* data2 = (const short*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(snorm_to_float(data0[0]), snorm_to_float(data0[1]), 0.0f, 0.0f);
        out[1] = (float4)(snorm_to_float(data1[0]), snorm_to_float(data1[1]), 0.0f, 0.0f);
        out[2] = (float4)(snorm_to_float(data2[0]), snorm_to_float(data2[1]), 0.0f, 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_UNORM)
    {
        const unsigned short* data0 = (const unsigned short*)(vertices + vtx0ID * vertex_stride);
        const unsigned short* data1 = (const unsigned short*)(vertices + vtx1ID * vertex_stride);
        const unsigned short* data2 = (const unsigned short*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(unorm_to_float(data0[0]), unorm_to_float(data0[1]), unorm_to_float(data0[2]), 0.0f);
        out[1] = (float4)(unorm_to_float(data1[0]), unorm_to_float(data1[1]), unorm_to_float(data1[2]), 0.0f);
        out[2] = (float4)(unorm_to_float(data2[0]), unorm_to_float(data2[1]), unorm_to_float(data2[2]), 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_UNORM)
    {
        const unsigned short* data0 = (const unsigned short*)(vertices + vtx0ID * vertex_stride);
        const unsigned short* data1 = (const unsigned short*)(vertices + vtx1ID * vertex_stride);
        const unsigned short* data2 = (const unsigned short*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(unorm_to_float(data0[0]), unorm_to_float(data0[1]), 0.0f, 0.0f);
        out[1] = (float4)(unorm_to_float(data1[0]), unorm_to_float(data1[1]), 0.0f, 0.0f);
        out[2] = (float4)(unorm_to_float(data2[0]), unorm_to_float(data2[1]), 0.0f, 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R10G10B10A2_UNORM)
    {
        const unsigned data0 = *(const unsigned*)(vertices + vtx0ID * vertex_stride);
        const unsigned data1 = *(const unsigned*)(vertices + vtx1ID * vertex_stride);
        const unsigned data2 = *(const unsigned*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(unorm10_to_float(data0), unorm10_to_float(data0 >> 10), unorm10_to_float(data0 >> 20), 0.0f);
        out[1] = (float4)(unorm10_to_float(data1), unorm10_to_float(data1 >> 10), unorm10_to_float(data1 >> 20), 0.0f);
        out[2] = (float4)(unorm10_to_float(data2), unorm10_to_float(data2 >> 10), unorm10_to_float(data2 >> 20), 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8B8A8_UNORM)
    {
        const unsigned char* data0 = (const unsigned char*)(vertices + vtx0ID * vertex_stride);
        const unsigned char* data1 = (const unsigned char*)(vertices + vtx1ID * vertex_stride);
        const unsigned char* data2 = (const unsigned char*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(unorm8_to_float(data0[0]), unorm8_to_float(data0[1]), unorm8_to_float(data0[2]), 0.0f);
        out[1] = (float4)(unorm8_to_float(data1[0]), unorm8_to_float(data1[1]), unorm8_to_float(data1[2]), 0.0f);
        out[2] = (float4)(unorm8_to_float(data2[0]), unorm8_to_float(data2[1]), unorm8_to_float(data2[2]), 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8_UNORM)
    {
        const unsigned char* data0 = (const unsigned char*)(vertices + vtx0ID * vertex_stride);
        const unsigned char* data1 = (const unsigned char*)(vertices + vtx1ID * vertex_stride);
        const unsigned char* data2 = (const unsigned char*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(unorm8_to_float(data0[0]), unorm8_to_float(data0[1]), 0.0f, 0.0f);
        out[1] = (float4)(unorm8_to_float(data1[0]), unorm8_to_float(data1[1]), 0.0f, 0.0f);
        out[2] = (float4)(unorm8_to_float(data2[0]), unorm8_to_float(data2[1]), 0.0f, 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8B8A8_SNORM)
    {
        const unsigned char* data0 = (const unsigned char*)(vertices + vtx0ID * vertex_stride);
        const unsigned char* data1 = (const unsigned char*)(vertices + vtx1ID * vertex_stride);
        const unsigned char* data2 = (const unsigned char*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(snorm8_to_float(data0[0]), snorm8_to_float(data0[1]), snorm8_to_float(data0[2]), 0.0f);
        out[1] = (float4)(snorm8_to_float(data1[0]), snorm8_to_float(data1[1]), snorm8_to_float(data1[2]), 0.0f);
        out[2] = (float4)(snorm8_to_float(data2[0]), snorm8_to_float(data2[1]), snorm8_to_float(data2[2]), 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8_SNORM)
    {
        const unsigned char* data0 = (const unsigned char*)(vertices + vtx0ID * vertex_stride);
        const unsigned char* data1 = (const unsigned char*)(vertices + vtx1ID * vertex_stride);
        const unsigned char* data2 = (const unsigned char*)(vertices + vtx2ID * vertex_stride);
        out[0] = (float4)(snorm8_to_float(data0[0]), snorm8_to_float(data0[1]), 0.0f, 0.0f);
        out[1] = (float4)(snorm8_to_float(data1[0]), snorm8_to_float(data1[1]), 0.0f, 0.0f);
        out[2] = (float4)(snorm8_to_float(data2[0]), snorm8_to_float(data2[1]), 0.0f, 0.0f);
    }

    /* perform vertex transformation */
    if (transform_buffer)
    {
        global float* xfm = (global float*)transform_buffer;
        for (uint i = 0; i < 3; ++i)
        {
            const float x = xfm[0] * out[i].x + xfm[1] * out[i].y + xfm[2] * out[i].z + xfm[3];
            const float y = xfm[4] * out[i].x + xfm[5] * out[i].y + xfm[6] * out[i].z + xfm[7];
            const float z = xfm[8] * out[i].x + xfm[9] * out[i].y + xfm[10] * out[i].z + xfm[11];
            out[i] = (float4)(x, y, z, 0.0f);
        }
    }
}

GRL_INLINE void GRL_load_quad_vertices_no_stride(global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    float3* out0, float3* out1, float3* out2, float3* out3,
    const uint4 vtxID, const uint vertex_format, global char* vertices)
{
    float3 v0, v1, v2, v3;

    if (vertex_format == VERTEX_FORMAT_R32G32B32_FLOAT)
    {
        const float* data0 = (const float*)(vertices + vtxID.x);
        const float* data1 = (const float*)(vertices + vtxID.y);
        const float* data2 = (const float*)(vertices + vtxID.z);
        const float* data3 = (const float*)(vertices + vtxID.w);
        v0 = (float3)(data0[0], data0[1], data0[2]);
        v1 = (float3)(data1[0], data1[1], data1[2]);
        v2 = (float3)(data2[0], data2[1], data2[2]);
        v3 = (float3)(data3[0], data3[1], data3[2]);
    }
    else if (vertex_format == VERTEX_FORMAT_R32G32_FLOAT)
    {
        const float* data0 = (const float*)(vertices + vtxID.x);
        const float* data1 = (const float*)(vertices + vtxID.y);
        const float* data2 = (const float*)(vertices + vtxID.z);
        const float* data3 = (const float*)(vertices + vtxID.w);
        v0 = (float3)(data0[0], data0[1], 0.0f);
        v1 = (float3)(data1[0], data1[1], 0.0f);
        v2 = (float3)(data2[0], data2[1], 0.0f);
        v3 = (float3)(data3[0], data3[1], 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_FLOAT)
    {
        const half* data0 = (const half*)(vertices + vtxID.x);
        const half* data1 = (const half*)(vertices + vtxID.y);
        const half* data2 = (const half*)(vertices + vtxID.z);
        const half* data3 = (const half*)(vertices + vtxID.w);
        v0 = (float3)(data0[0], data0[1], data0[2]);
        v1 = (float3)(data1[0], data1[1], data1[2]);
        v2 = (float3)(data2[0], data2[1], data2[2]);
        v3 = (float3)(data3[0], data3[1], data3[2]);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_FLOAT)
    {
        const half* data0 = (const half*)(vertices + vtxID.x);
        const half* data1 = (const half*)(vertices + vtxID.y);
        const half* data2 = (const half*)(vertices + vtxID.z);
        const half* data3 = (const half*)(vertices + vtxID.w);
        v0 = (float3)(data0[0], data0[1], 0.0f);
        v1 = (float3)(data1[0], data1[1], 0.0f);
        v2 = (float3)(data2[0], data2[1], 0.0f);
        v3 = (float3)(data3[0], data3[1], 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_SNORM)
    {
        const short* data0 = (const short*)(vertices + vtxID.x);
        const short* data1 = (const short*)(vertices + vtxID.y);
        const short* data2 = (const short*)(vertices + vtxID.z);
        const short* data3 = (const short*)(vertices + vtxID.w);
        v0 = (float3)(snorm_to_float(data0[0]), snorm_to_float(data0[1]), snorm_to_float(data0[2]));
        v1 = (float3)(snorm_to_float(data1[0]), snorm_to_float(data1[1]), snorm_to_float(data1[2]));
        v2 = (float3)(snorm_to_float(data2[0]), snorm_to_float(data2[1]), snorm_to_float(data2[2]));
        v3 = (float3)(snorm_to_float(data3[0]), snorm_to_float(data3[1]), snorm_to_float(data3[2]));
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_SNORM)
    {
        const short* data0 = (const short*)(vertices + vtxID.x);
        const short* data1 = (const short*)(vertices + vtxID.y);
        const short* data2 = (const short*)(vertices + vtxID.z);
        const short* data3 = (const short*)(vertices + vtxID.w);
        v0 = (float3)(snorm_to_float(data0[0]), snorm_to_float(data0[1]), 0.0f);
        v1 = (float3)(snorm_to_float(data1[0]), snorm_to_float(data1[1]), 0.0f);
        v2 = (float3)(snorm_to_float(data2[0]), snorm_to_float(data2[1]), 0.0f);
        v3 = (float3)(snorm_to_float(data3[0]), snorm_to_float(data3[1]), 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16B16A16_UNORM)
    {
        const unsigned short* data0 = (const unsigned short*)(vertices + vtxID.x);
        const unsigned short* data1 = (const unsigned short*)(vertices + vtxID.y);
        const unsigned short* data2 = (const unsigned short*)(vertices + vtxID.z);
        const unsigned short* data3 = (const unsigned short*)(vertices + vtxID.w);
        v0 = (float3)(unorm_to_float(data0[0]), unorm_to_float(data0[1]), unorm_to_float(data0[2]));
        v1 = (float3)(unorm_to_float(data1[0]), unorm_to_float(data1[1]), unorm_to_float(data1[2]));
        v2 = (float3)(unorm_to_float(data2[0]), unorm_to_float(data2[1]), unorm_to_float(data2[2]));
        v3 = (float3)(unorm_to_float(data3[0]), unorm_to_float(data3[1]), unorm_to_float(data3[2]));
    }
    else if (vertex_format == VERTEX_FORMAT_R16G16_UNORM)
    {
        const unsigned short* data0 = (const unsigned short*)(vertices + vtxID.x);
        const unsigned short* data1 = (const unsigned short*)(vertices + vtxID.y);
        const unsigned short* data2 = (const unsigned short*)(vertices + vtxID.z);
        const unsigned short* data3 = (const unsigned short*)(vertices + vtxID.w);
        v0 = (float3)(unorm_to_float(data0[0]), unorm_to_float(data0[1]), 0.0f);
        v1 = (float3)(unorm_to_float(data1[0]), unorm_to_float(data1[1]), 0.0f);
        v2 = (float3)(unorm_to_float(data2[0]), unorm_to_float(data2[1]), 0.0f);
        v3 = (float3)(unorm_to_float(data3[0]), unorm_to_float(data3[1]), 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R10G10B10A2_UNORM)
    {
        const unsigned data0 = *(const unsigned*)(vertices + vtxID.x);
        const unsigned data1 = *(const unsigned*)(vertices + vtxID.y);
        const unsigned data2 = *(const unsigned*)(vertices + vtxID.z);
        const unsigned data3 = *(const unsigned*)(vertices + vtxID.w);
        v0 = (float3)(unorm10_to_float(data0), unorm10_to_float((data0 >> 10)), unorm10_to_float((data0 >> 20)));
        v1 = (float3)(unorm10_to_float(data1), unorm10_to_float((data1 >> 10)), unorm10_to_float((data1 >> 20)));
        v2 = (float3)(unorm10_to_float(data2), unorm10_to_float((data2 >> 10)), unorm10_to_float((data2 >> 20)));
        v3 = (float3)(unorm10_to_float(data3), unorm10_to_float((data3 >> 10)), unorm10_to_float((data3 >> 20)));
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8B8A8_UNORM)
    {
        const unsigned char* data0 = (const unsigned char*)(vertices + vtxID.x);
        const unsigned char* data1 = (const unsigned char*)(vertices + vtxID.y);
        const unsigned char* data2 = (const unsigned char*)(vertices + vtxID.z);
        const unsigned char* data3 = (const unsigned char*)(vertices + vtxID.w);
        v0 = (float3)(unorm8_to_float(data0[0]), unorm8_to_float(data0[1]), unorm8_to_float(data0[2]));
        v1 = (float3)(unorm8_to_float(data1[0]), unorm8_to_float(data1[1]), unorm8_to_float(data1[2]));
        v2 = (float3)(unorm8_to_float(data2[0]), unorm8_to_float(data2[1]), unorm8_to_float(data2[2]));
        v3 = (float3)(unorm8_to_float(data3[0]), unorm8_to_float(data3[1]), unorm8_to_float(data3[2]));
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8_UNORM)
    {
        const unsigned char* data0 = (const unsigned char*)(vertices + vtxID.x);
        const unsigned char* data1 = (const unsigned char*)(vertices + vtxID.y);
        const unsigned char* data2 = (const unsigned char*)(vertices + vtxID.z);
        const unsigned char* data3 = (const unsigned char*)(vertices + vtxID.w);
        v0 = (float3)(unorm8_to_float(data0[0]), unorm8_to_float(data0[1]), 0.0f);
        v1 = (float3)(unorm8_to_float(data1[0]), unorm8_to_float(data1[1]), 0.0f);
        v2 = (float3)(unorm8_to_float(data2[0]), unorm8_to_float(data2[1]), 0.0f);
        v3 = (float3)(unorm8_to_float(data3[0]), unorm8_to_float(data3[1]), 0.0f);
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8B8A8_SNORM)
    {
        const signed char* data0 = (const signed char*)(vertices + vtxID.x);
        const signed char* data1 = (const signed char*)(vertices + vtxID.y);
        const signed char* data2 = (const signed char*)(vertices + vtxID.z);
        const signed char* data3 = (const signed char*)(vertices + vtxID.w);
        v0 = (float3)(snorm8_to_float(data0[0]), snorm8_to_float(data0[1]), snorm8_to_float(data0[2]));
        v1 = (float3)(snorm8_to_float(data1[0]), snorm8_to_float(data1[1]), snorm8_to_float(data1[2]));
        v2 = (float3)(snorm8_to_float(data2[0]), snorm8_to_float(data2[1]), snorm8_to_float(data2[2]));
        v3 = (float3)(snorm8_to_float(data3[0]), snorm8_to_float(data3[1]), snorm8_to_float(data3[2]));
    }
    else if (vertex_format == VERTEX_FORMAT_R8G8_SNORM)
    {
        const signed char* data0 = (const signed char*)(vertices + vtxID.x);
        const signed char* data1 = (const signed char*)(vertices + vtxID.y);
        const signed char* data2 = (const signed char*)(vertices + vtxID.z);
        const signed char* data3 = (const signed char*)(vertices + vtxID.w);
        v0 = (float3)(snorm8_to_float(data0[0]), snorm8_to_float(data0[1]), 0.0f);
        v1 = (float3)(snorm8_to_float(data1[0]), snorm8_to_float(data1[1]), 0.0f);
        v2 = (float3)(snorm8_to_float(data2[0]), snorm8_to_float(data2[1]), 0.0f);
        v3 = (float3)(snorm8_to_float(data3[0]), snorm8_to_float(data3[1]), 0.0f);
    }
    else
    {
        v0 = (float3)(0.0f, 0.0f, 0.0f);
        v1 = (float3)(0.0f, 0.0f, 0.0f);
        v2 = (float3)(0.0f, 0.0f, 0.0f);
        v3 = (float3)(0.0f, 0.0f, 0.0f);
    }


    /* perform vertex transformation */
    if (geomDesc->Desc.Triangles.pTransformBuffer)
    {
        global float* xfm = (global float*)geomDesc->Desc.Triangles.pTransformBuffer;

        v0.xyz = (float3)(
            xfm[0] * v0.x + xfm[1] * v0.y + xfm[2] * v0.z + xfm[3],
            xfm[4] * v0.x + xfm[5] * v0.y + xfm[6] * v0.z + xfm[7],
            xfm[8] * v0.x + xfm[9] * v0.y + xfm[10] * v0.z + xfm[11]
            );

        v1.xyz = (float3)(
            xfm[0] * v1.x + xfm[1] * v1.y + xfm[2] * v1.z + xfm[3],
            xfm[4] * v1.x + xfm[5] * v1.y + xfm[6] * v1.z + xfm[7],
            xfm[8] * v1.x + xfm[9] * v1.y + xfm[10] * v1.z + xfm[11]
            );

        v2.xyz = (float3)(
            xfm[0] * v2.x + xfm[1] * v2.y + xfm[2] * v2.z + xfm[3],
            xfm[4] * v2.x + xfm[5] * v2.y + xfm[6] * v2.z + xfm[7],
            xfm[8] * v2.x + xfm[9] * v2.y + xfm[10] * v2.z + xfm[11]
            );

        v3.xyz = (float3)(
            xfm[0] * v3.x + xfm[1] * v3.y + xfm[2] * v3.z + xfm[3],
            xfm[4] * v3.x + xfm[5] * v3.y + xfm[6] * v3.z + xfm[7],
            xfm[8] * v3.x + xfm[9] * v3.y + xfm[10] * v3.z + xfm[11]
            );
    }

    *out0 = v0;
    *out1 = v1;
    *out2 = v2;
    *out3 = v3;
}


GRL_INLINE void GRL_load_quad_vertices(global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc,
    float3* out0, float3* out1, float3* out2, float3* out3,
    uint4 vtxID)
{
    global char* vertices = (global char*)geomDesc->Desc.Triangles.pVertexBuffer;
    uint vertex_format = geomDesc->Desc.Triangles.VertexFormat;
    uint vertex_stride = geomDesc->Desc.Triangles.VertexBufferByteStride;

    vtxID *= vertex_stride;

    GRL_load_quad_vertices_no_stride(geomDesc, out0, out1, out2, out3,
        vtxID, vertex_format, vertices);
}


GRL_INLINE GRL_RAYTRACING_AABB GRL_load_aabb(global GRL_RAYTRACING_GEOMETRY_DESC* geomDesc, const uint primID)
{
    global char* aabb0 = (global char*)geomDesc->Desc.Procedural.pAABBs_GPUVA;
    global char* aabb = aabb0 + (primID * geomDesc->Desc.Procedural.AABBByteStride);
    return *(global GRL_RAYTRACING_AABB*)aabb;
}

// same as for d3d12
typedef struct GRL_RAYTRACING_INSTANCE_DESC
{
    float Transform[12];
    //     unsigned int InstanceID : 24;
    //     unsigned int InstanceMask : 8;
    uint32_t DW0;
    //     unsigned int InstanceContributionToHitGroupIndex : 24;
    //     unsigned int Flags : 8;
    uint32_t DW1;
    global char* AccelerationStructure;
} GRL_RAYTRACING_INSTANCE_DESC;

GRL_INLINE float GRL_get_transform(const GRL_RAYTRACING_INSTANCE_DESC* d, const uint32_t row, const uint32_t column)
{
    return d->Transform[row * 4 + column];
}

GRL_INLINE uint32_t GRL_get_instanceID(const GRL_RAYTRACING_INSTANCE_DESC* d)
{
    return d->DW0 & ((1 << 24) - 1);
}

GRL_INLINE uint32_t GRL_get_InstanceMask(const GRL_RAYTRACING_INSTANCE_DESC* d)
{
    return d->DW0 >> 24;
}

GRL_INLINE uint32_t GRL_get_InstanceContributionToHitGroupIndex(const GRL_RAYTRACING_INSTANCE_DESC* d)
{
    return d->DW1 & ((1 << 24) - 1);
}

GRL_INLINE uint32_t GRL_get_InstanceFlags(const GRL_RAYTRACING_INSTANCE_DESC* d)
{
    return d->DW1 >> 24;
}

GRL_INLINE gpuva_t GRL_get_AccelerationStructure(const GRL_RAYTRACING_INSTANCE_DESC* d)
{
    return (gpuva_t)d->AccelerationStructure;
}

GRL_INLINE void GRL_set_transform(GRL_RAYTRACING_INSTANCE_DESC* d, const uint32_t row, const uint32_t column, float value)
{
    d->Transform[row * 4 + column] = value;
}

GRL_INLINE void GRL_set_instanceID(GRL_RAYTRACING_INSTANCE_DESC* d, const uint32_t id)
{
    d->DW0 &= 255 << 24;
    d->DW0 |= id & ((1 << 24) - 1);
}

GRL_INLINE void GRL_set_InstanceMask(GRL_RAYTRACING_INSTANCE_DESC* d, const uint32_t mask)
{
    d->DW0 &= ((1 << 24) - 1);
    d->DW0 |= mask << 24;
}

GRL_INLINE void GRL_set_InstanceContributionToHitGroupIndex(GRL_RAYTRACING_INSTANCE_DESC* d, const uint32_t contribution)
{
    d->DW1 &= 255 << 24;
    d->DW1 |= contribution & ((1 << 24) - 1);
}

GRL_INLINE void GRL_set_InstanceFlags(GRL_RAYTRACING_INSTANCE_DESC* d, const uint32_t flags)
{
    d->DW1 &= ((1 << 24) - 1);
    d->DW1 |= flags << 24;
}

GRL_INLINE void GRL_set_AccelerationStructure(GRL_RAYTRACING_INSTANCE_DESC* d, gpuva_t address)
{
    d->AccelerationStructure = (global char*)address;
}
