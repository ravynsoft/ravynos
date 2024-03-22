//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once
#include "GRLStructs.h"
#include "shared.h"

typedef global void *D3D12_GPU_VIRTUAL_ADDRESS;
typedef void *ID3D12StateObjectPrototype;

enum DXGI_FORMAT
{
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_R32G32B32A32_TYPELESS,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_UINT,
    DXGI_FORMAT_R32G32B32A32_SINT,
    DXGI_FORMAT_R32G32B32_TYPELESS,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R16G16B16A16_SINT,
    DXGI_FORMAT_R32G32_TYPELESS,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,
    DXGI_FORMAT_R32G8X24_TYPELESS,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
    DXGI_FORMAT_R10G10B10A2_TYPELESS,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UINT,
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R8G8B8A8_TYPELESS,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_SINT,
    DXGI_FORMAT_R16G16_TYPELESS,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16_UINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16_SINT,
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32_SINT,
    DXGI_FORMAT_R24G8_TYPELESS,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT,
    DXGI_FORMAT_R8G8_TYPELESS,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_UINT,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_R8G8_SINT,
    DXGI_FORMAT_R16_TYPELESS,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R16_SNORM,
    DXGI_FORMAT_R16_SINT,
    DXGI_FORMAT_R8_TYPELESS,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R8_SNORM,
    DXGI_FORMAT_R8_SINT,
    DXGI_FORMAT_A8_UNORM,
    DXGI_FORMAT_R1_UNORM,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
    DXGI_FORMAT_R8G8_B8G8_UNORM,
    DXGI_FORMAT_G8R8_G8B8_UNORM,
    DXGI_FORMAT_BC1_TYPELESS,
    DXGI_FORMAT_BC1_UNORM,
    DXGI_FORMAT_BC1_UNORM_SRGB,
    DXGI_FORMAT_BC2_TYPELESS,
    DXGI_FORMAT_BC2_UNORM,
    DXGI_FORMAT_BC2_UNORM_SRGB,
    DXGI_FORMAT_BC3_TYPELESS,
    DXGI_FORMAT_BC3_UNORM,
    DXGI_FORMAT_BC3_UNORM_SRGB,
    DXGI_FORMAT_BC4_TYPELESS,
    DXGI_FORMAT_BC4_UNORM,
    DXGI_FORMAT_BC4_SNORM,
    DXGI_FORMAT_BC5_TYPELESS,
    DXGI_FORMAT_BC5_UNORM,
    DXGI_FORMAT_BC5_SNORM,
    DXGI_FORMAT_B5G6R5_UNORM,
    DXGI_FORMAT_B5G5R5A1_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_B8G8R8X8_UNORM,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
    DXGI_FORMAT_B8G8R8A8_TYPELESS,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    DXGI_FORMAT_B8G8R8X8_TYPELESS,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
    DXGI_FORMAT_BC6H_TYPELESS,
    DXGI_FORMAT_BC6H_UF16,
    DXGI_FORMAT_BC6H_SF16,
    DXGI_FORMAT_BC7_TYPELESS,
    DXGI_FORMAT_BC7_UNORM,
    DXGI_FORMAT_BC7_UNORM_SRGB,
    DXGI_FORMAT_AYUV,
    DXGI_FORMAT_Y410,
    DXGI_FORMAT_Y416,
    DXGI_FORMAT_NV12,
    DXGI_FORMAT_P010,
    DXGI_FORMAT_P016,
    DXGI_FORMAT_420_OPAQUE,
    DXGI_FORMAT_YUY2,
    DXGI_FORMAT_Y210,
    DXGI_FORMAT_Y216,
    DXGI_FORMAT_NV11,
    DXGI_FORMAT_AI44,
    DXGI_FORMAT_IA44,
    DXGI_FORMAT_P8,
    DXGI_FORMAT_A8P8,
    DXGI_FORMAT_B4G4R4A4_UNORM,
    DXGI_FORMAT_P208,
    DXGI_FORMAT_V208,
    DXGI_FORMAT_V408,
    DXGI_FORMAT_FORCE_UINT
};

typedef enum D3D12_RAYTRACING_GEOMETRY_FLAGS
{
    D3D12_RAYTRACING_GEOMETRY_FLAG_NONE = 0,
    D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE = 0x1,
    D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION = 0x2
} D3D12_RAYTRACING_GEOMETRY_FLAGS;

typedef enum D3D12_RAYTRACING_GEOMETRY_TYPE
{
    D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES = 0,
    D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS = (D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES + 1)
} D3D12_RAYTRACING_GEOMETRY_TYPE;

typedef enum D3D12_RAYTRACING_INSTANCE_FLAGS
{
    D3D12_RAYTRACING_INSTANCE_FLAG_NONE = 0,
    D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 0x1,
    D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x2,
    D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE = 0x4,
    D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE = 0x8
} D3D12_RAYTRACING_INSTANCE_FLAGS;

typedef struct D3D12_GPU_VIRTUAL_ADDRESSAND_STRIDE
{
    D3D12_GPU_VIRTUAL_ADDRESS StartAddress;
    unsigned long StrideInBytes;
} D3D12_GPU_VIRTUAL_ADDRESSAND_STRIDE;

typedef struct D3D12_GPU_VIRTUAL_ADDRESSRANGE
{
    D3D12_GPU_VIRTUAL_ADDRESS StartAddress;
    unsigned long SizeInBytes;
} D3D12_GPU_VIRTUAL_ADDRESSRANGE;

typedef struct D3D12_GPU_VIRTUAL_ADDRESSRANGE_AND_STRIDE
{
    D3D12_GPU_VIRTUAL_ADDRESS StartAddress;
    unsigned long SizeInBytes;
    unsigned long StrideInBytes;
} D3D12_GPU_VIRTUAL_ADDRESSRANGE_AND_STRIDE;

typedef struct D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC
{
    D3D12_GPU_VIRTUAL_ADDRESS Transform;
    enum DXGI_FORMAT IndexFormat;
    enum DXGI_FORMAT VertexFormat;
    unsigned int IndexCount;
    unsigned int VertexCount;
    D3D12_GPU_VIRTUAL_ADDRESS IndexBuffer;
    struct D3D12_GPU_VIRTUAL_ADDRESSAND_STRIDE VertexBuffer;
} D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC;

typedef struct D3D12_RAYTRACING_AABB
{
    float MinX;
    float MinY;
    float MinZ;
    float MaxX;
    float MaxY;
    float MaxZ;
} D3D12_RAYTRACING_AABB;

GRL_INLINE void D3D12_set_raytracing_aabb(D3D12_RAYTRACING_AABB* dest, struct AABB* source)
{
    dest->MinX = source->lower.x;
    dest->MinY = source->lower.y;
    dest->MinZ = source->lower.z;
    dest->MaxX = source->upper.x;
    dest->MaxY = source->upper.y;
    dest->MaxZ = source->upper.z;
}

typedef struct D3D12_RAYTRACING_GEOMETRY_AABBS_DESC
{
    unsigned long AABBCount;
    D3D12_GPU_VIRTUAL_ADDRESSAND_STRIDE AABBs;
} D3D12_RAYTRACING_GEOMETRY_AABBS_DESC;

typedef struct D3D12_RAYTRACING_GEOMETRY_DESC
{
    D3D12_RAYTRACING_GEOMETRY_TYPE Type;
    D3D12_RAYTRACING_GEOMETRY_FLAGS Flags;
    //unsigned int ShaderIndex : 24; // extension
    //unsigned int Mask : 8; // extension
    //unsigned int ShaderIndex_Mask; // extension
    union {
        D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC Triangles;
        D3D12_RAYTRACING_GEOMETRY_AABBS_DESC AABBs;
    };
} D3D12_RAYTRACING_GEOMETRY_DESC;

GRL_INLINE void D3D12_set_Type(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, D3D12_RAYTRACING_GEOMETRY_TYPE type)
{
    geomDesc->Type = type;
}

GRL_INLINE D3D12_RAYTRACING_GEOMETRY_TYPE D3D12_get_Type(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->Type;
}

GRL_INLINE void D3D12_set_Flags(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, D3D12_RAYTRACING_GEOMETRY_FLAGS flags)
{
    geomDesc->Flags = flags;
}

GRL_INLINE D3D12_RAYTRACING_GEOMETRY_FLAGS D3D12_get_Flags(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->Flags;
}

GRL_INLINE void D3D12_set_triangles_Transform(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, D3D12_GPU_VIRTUAL_ADDRESS transform)
{
    geomDesc->Triangles.Transform = transform;
}

GRL_INLINE D3D12_GPU_VIRTUAL_ADDRESS D3D12_get_triangles_Transform(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->Triangles.Transform;
}

GRL_INLINE void D3D12_set_triangles_IndexFormat(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, IndexFormat format)
{
    switch (format)
    {
    case INDEX_FORMAT_NONE:
        geomDesc->Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
        break;
    case INDEX_FORMAT_R16_UINT:
        geomDesc->Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
        break;
    case INDEX_FORMAT_R32_UINT:
        geomDesc->Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
        break;
    }
}

GRL_INLINE IndexFormat D3D12_get_triangles_IndexFormat(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    switch (geomDesc->Triangles.IndexFormat)
    {
    case DXGI_FORMAT_R16_UINT:
        return INDEX_FORMAT_R16_UINT;
    case DXGI_FORMAT_R32_UINT:
        return INDEX_FORMAT_R32_UINT;
    case DXGI_FORMAT_UNKNOWN:
    default:
        return INDEX_FORMAT_NONE;
    }
}

GRL_INLINE void D3D12_set_triangles_VertexFormat(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, VertexFormat format)
{
    switch (format)
    {
    case VERTEX_FORMAT_R32G32_FLOAT:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R32G32_FLOAT;
        break;
    case VERTEX_FORMAT_R32G32B32_FLOAT:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        break;
    case VERTEX_FORMAT_R16G16_FLOAT:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R16G16_FLOAT;
        break;
    case VERTEX_FORMAT_R16G16B16A16_FLOAT:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
        break;
    case VERTEX_FORMAT_R16G16_SNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R16G16_SNORM;
        break;
    case VERTEX_FORMAT_R16G16B16A16_SNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R16G16B16A16_SNORM;
        break;
    case VERTEX_FORMAT_R16G16B16A16_UNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
        break;
    case VERTEX_FORMAT_R16G16_UNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R16G16_UNORM;
        break;
    case VERTEX_FORMAT_R10G10B10A2_UNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
        break;
    case VERTEX_FORMAT_R8G8B8A8_UNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    case VERTEX_FORMAT_R8G8_UNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R8G8_UNORM;
        break;
    case VERTEX_FORMAT_R8G8B8A8_SNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R8G8B8A8_SNORM;
        break;
    case VERTEX_FORMAT_R8G8_SNORM:
        geomDesc->Triangles.VertexFormat = DXGI_FORMAT_R8G8_SNORM;
        break;
    }
}

GRL_INLINE VertexFormat D3D12_get_triangles_VertexFormat(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    switch(geomDesc->Triangles.VertexFormat)
    {
    case DXGI_FORMAT_R32G32_FLOAT:
        return VERTEX_FORMAT_R32G32_FLOAT;
    case DXGI_FORMAT_R32G32B32_FLOAT:
        return VERTEX_FORMAT_R32G32B32_FLOAT;
    case DXGI_FORMAT_R16G16_FLOAT:
        return VERTEX_FORMAT_R16G16_FLOAT;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return VERTEX_FORMAT_R16G16B16A16_FLOAT;
    case DXGI_FORMAT_R16G16_SNORM:
        return VERTEX_FORMAT_R16G16_SNORM;
    case DXGI_FORMAT_R16G16B16A16_SNORM:
        return VERTEX_FORMAT_R16G16B16A16_SNORM;
    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return VERTEX_FORMAT_R16G16B16A16_UNORM;
    case DXGI_FORMAT_R16G16_UNORM:
        return VERTEX_FORMAT_R16G16_UNORM;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return VERTEX_FORMAT_R10G10B10A2_UNORM;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return VERTEX_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_R8G8_UNORM:
        return VERTEX_FORMAT_R8G8_UNORM;
    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return VERTEX_FORMAT_R8G8B8A8_SNORM;
    case DXGI_FORMAT_R8G8_SNORM:
        return VERTEX_FORMAT_R8G8_SNORM;
    default:
        return VERTEX_FORMAT_R32G32_FLOAT;
    }
}

GRL_INLINE void D3D12_set_triangles_IndexCount(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, unsigned int count)
{
    geomDesc->Triangles.IndexCount = count;
}

GRL_INLINE unsigned int D3D12_get_triangles_IndexCount(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->Triangles.IndexCount;
}

GRL_INLINE void D3D12_set_triangles_VertexCount(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, unsigned int count)
{
    geomDesc->Triangles.VertexCount = count;
}

GRL_INLINE unsigned int D3D12_get_triangles_VertexCount(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->Triangles.VertexCount;
}

GRL_INLINE void D3D12_set_triangles_IndexBuffer(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, D3D12_GPU_VIRTUAL_ADDRESS buffer)
{
    geomDesc->Triangles.IndexBuffer = buffer;
}

GRL_INLINE D3D12_GPU_VIRTUAL_ADDRESS D3D12_get_triangles_IndexBuffer(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->Triangles.IndexBuffer;
}

GRL_INLINE void D3D12_set_triangles_VertexBuffer_StartAddress(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, D3D12_GPU_VIRTUAL_ADDRESS address)
{
    geomDesc->Triangles.VertexBuffer.StartAddress = address;
}

GRL_INLINE D3D12_GPU_VIRTUAL_ADDRESS D3D12_get_triangles_VertexBuffer_StartAddress(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->Triangles.VertexBuffer.StartAddress;
}

GRL_INLINE void D3D12_set_triangles_VertexBuffer_StrideInBytes(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, unsigned long stride)
{
    geomDesc->Triangles.VertexBuffer.StrideInBytes = stride;
}

GRL_INLINE unsigned long D3D12_get_triangles_VertexBuffer_StrideInBytes(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->Triangles.VertexBuffer.StrideInBytes;
}

GRL_INLINE void D3D12_set_procedurals_AABBCount(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, unsigned long count)
{
    geomDesc->AABBs.AABBCount = count;
}

GRL_INLINE unsigned long D3D12_get_procedurals_AABBCount(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->AABBs.AABBCount;
}

GRL_INLINE void D3D12_set_procedurals_AABBs_StartAddress(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, D3D12_GPU_VIRTUAL_ADDRESS address)
{
    geomDesc->AABBs.AABBs.StartAddress = address;
}

GRL_INLINE D3D12_GPU_VIRTUAL_ADDRESS D3D12_get_procedurals_AABBs_StartAddress(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->AABBs.AABBs.StartAddress;
}

GRL_INLINE void D3D12_set_procedurals_AABBs_StrideInBytes(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc, unsigned long stride)
{
    geomDesc->AABBs.AABBs.StrideInBytes = stride;
}

GRL_INLINE unsigned long D3D12_get_procedurals_AABBs_StrideInBytes(D3D12_RAYTRACING_GEOMETRY_DESC *geomDesc)
{
    return geomDesc->AABBs.AABBs.StrideInBytes;
}

typedef struct D3D12_RAYTRACING_INSTANCE_DESC
{
    float Transform[12];
    //     unsigned int InstanceID : 24;
    //     unsigned int InstanceMask : 8;
    uint32_t DW0;
    //     unsigned int InstanceContributionToHitGroupIndex : 24;
    //     unsigned int Flags : 8;
    uint32_t DW1;
    global char *AccelerationStructure;
} D3D12_RAYTRACING_INSTANCE_DESC;

GRL_INLINE float D3D12_get_transform(const D3D12_RAYTRACING_INSTANCE_DESC *d, const uint32_t row, const uint32_t column)
{
    return d->Transform[row * 4 + column];
}

GRL_INLINE uint32_t D3D12_get_instanceID(const D3D12_RAYTRACING_INSTANCE_DESC *d)
{
    return d->DW0 & ((1 << 24) - 1);
}

GRL_INLINE uint32_t D3D12_get_InstanceMask(const D3D12_RAYTRACING_INSTANCE_DESC *d)
{
    return d->DW0 >> 24;
}

GRL_INLINE uint32_t D3D12_get_InstanceContributionToHitGroupIndex(const D3D12_RAYTRACING_INSTANCE_DESC *d)
{
    return d->DW1 & ((1 << 24) - 1);
}

GRL_INLINE uint32_t D3D12_get_InstanceFlags(const D3D12_RAYTRACING_INSTANCE_DESC *d)
{
    return d->DW1 >> 24;
}

GRL_INLINE gpuva_t D3D12_get_AccelerationStructure(const D3D12_RAYTRACING_INSTANCE_DESC *d)
{
    return (gpuva_t)d->AccelerationStructure;
}

GRL_INLINE void D3D12_set_transform(D3D12_RAYTRACING_INSTANCE_DESC *d, const uint32_t row, const uint32_t column, float value)
{
    d->Transform[row * 4 + column] = value;
}

GRL_INLINE void D3D12_set_instanceID(D3D12_RAYTRACING_INSTANCE_DESC *d, const uint32_t id)
{
    d->DW0 &= 255 << 24;
    d->DW0 |= id & ((1 << 24) - 1);
}

GRL_INLINE void D3D12_set_InstanceMask(D3D12_RAYTRACING_INSTANCE_DESC *d, const uint32_t mask)
{
    d->DW0 &= ((1 << 24) - 1);
    d->DW0 |= mask << 24;
}

GRL_INLINE void D3D12_set_InstanceContributionToHitGroupIndex(D3D12_RAYTRACING_INSTANCE_DESC *d, const uint32_t contribution)
{
    d->DW1 &= 255 << 24;
    d->DW1 |= contribution & ((1 << 24) - 1);
}

GRL_INLINE void D3D12_set_InstanceFlags(D3D12_RAYTRACING_INSTANCE_DESC *d, const uint32_t flags)
{
    d->DW1 &= ((1 << 24) - 1);
    d->DW1 |= flags << 24;
}

GRL_INLINE void D3D12_set_AccelerationStructure(D3D12_RAYTRACING_INSTANCE_DESC *d, gpuva_t address)
{
    d->AccelerationStructure = (global char*)address;
}
