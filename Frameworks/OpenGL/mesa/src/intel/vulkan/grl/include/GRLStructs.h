//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "GRLIntTypes.h"

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(_INTERNAL)

    struct GeometryTriangles
    {
        gpuva_t        pTransformBuffer;
        gpuva_t        pIndexBuffer;
        gpuva_t        pVertexBuffer;
        qword          VertexBufferByteStride;
        dword          IndexCount;
        dword          VertexCount;
        IndexFormat    IndexFormat;
        VertexFormat   VertexFormat;
    };

    struct GeometryProcedural
    {
        gpuva_t  pAABBs_GPUVA; ///<elements of pAABBs_GPUVA are gpuAABB format.
        qword    AABBByteStride;
        dword    AABBCount;
    };

    // TODO we miss 'unsigned int ShaderIndex_Mask; // extension' field
    struct Geo
    {
        union
        {
            struct GeometryTriangles Triangles;
            struct GeometryProcedural Procedural;
        } Desc;

        GeometryType Type;
        uint8_t Flags;
    };

    // Matches the Vulkan VkAccelerationStructureBuildRangeInfoKHR structure
    // See Vulkan spec for data access rules:
    //     https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkAccelerationStructureBuildRangeInfoKHR.html
    //
    struct IndirectBuildRangeInfo
    {
        dword    primitiveCount;        // Number of primitives
        dword    primitiveOffset;       // Byte offset to primitive data
        dword    firstVertex;           // Index of first vertex
        dword    transformOffset;       // Byte offset to transform data (for triangle Geo with non-null transform)
    };

GRL_NAMESPACE_END(_INTERNAL)
GRL_NAMESPACE_END(GRL)