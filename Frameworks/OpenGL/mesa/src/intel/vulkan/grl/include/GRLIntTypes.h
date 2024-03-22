//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

//********************************************************************************************
//   WARNING!!!!!
//
// This file is shared by OpenCL and C++ source code and must be a pure C header
//  There should only be C structure definitions and trivial inline functions here
//
//********************************************************************************************

#pragma once

#include "GRLOCLCompatibility.h"

GRL_NAMESPACE_BEGIN(GRL)

    typedef uint32_t dword;
    typedef uint64_t qword;
    typedef qword gpuva_t;


    enum_uint8( InstanceFlags )
    {
        INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 0x1,
        INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x2,
        INSTANCE_FLAG_FORCE_OPAQUE = 0x4,
        INSTANCE_FLAG_FORCE_NON_OPAQUE = 0x8,
    };

    enum_uint8( GeometryFlags )
    {
        GEOMETRY_FLAG_NONE = 0x0,
        GEOMETRY_FLAG_OPAQUE = 0x1,
        GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION = 0x2,
    };

    enum_uint8( GeometryType )
    {
        GEOMETRY_TYPE_TRIANGLES = 0,
        GEOMETRY_TYPE_PROCEDURAL = 1,
        NUM_GEOMETRY_TYPES = 2
    };

    // NOTE: Does NOT match DXR
    enum_uint8( IndexFormat )
    {
        INDEX_FORMAT_NONE     = 0,     // INDEX_FORMAT_NONE Indicates non-indexed geometry
        INDEX_FORMAT_R16_UINT = 2,
        INDEX_FORMAT_R32_UINT = 4,
        INDEX_FORMAT_END = INDEX_FORMAT_R32_UINT + 1
    };

    // NOTE: Does NOT match DXR
    enum_uint8( VertexFormat )
    {
        VERTEX_FORMAT_R32G32_FLOAT          = 0,
        VERTEX_FORMAT_R32G32B32_FLOAT       = 1,
        VERTEX_FORMAT_R16G16_FLOAT          = 2,
        VERTEX_FORMAT_R16G16B16A16_FLOAT    = 3,
        VERTEX_FORMAT_R16G16_SNORM          = 4,
        VERTEX_FORMAT_R16G16B16A16_SNORM    = 5,
        VERTEX_FORMAT_R16G16B16A16_UNORM    = 6,
        VERTEX_FORMAT_R16G16_UNORM          = 7,
        VERTEX_FORMAT_R10G10B10A2_UNORM     = 8,
        VERTEX_FORMAT_R8G8B8A8_UNORM        = 9,
        VERTEX_FORMAT_R8G8_UNORM            = 10,
        VERTEX_FORMAT_R8G8B8A8_SNORM        = 11,
        VERTEX_FORMAT_R8G8_SNORM            = 12,
        VERTEX_FORMAT_END = VERTEX_FORMAT_R8G8_SNORM + 1
    };



    enum_uint32(RTASFlags)
    {
        // These flags match DXR
        BUILD_FLAG_ALLOW_UPDATE                 = 1<<0,
        BUILD_FLAG_ALLOW_COMPACTION             = 1<<1,
        BUILD_FLAG_PREFER_FAST_TRACE            = 1<<2,
        BUILD_FLAG_PREFER_FAST_BUILD            = 1<<3,
        BUILD_FLAG_MINIMIZE_MEMORY              = 1<<4,
        BUILD_FLAG_PERFORM_UPDATE               = 1<<5,

        // internal flags start here
        BUILD_FLAG_DISALLOW_REBRAID             = 1<<16,

        BUILD_FLAG_ALL = 0x0001003f
    };

    enum_uint8(BVHType)
    {
        BVH_TYPE_NONE, // This is a sentinel for drivers to use when compiling out GRL on non-RT devices
        BVH_TYPE_GEN12,
    };

    enum_uint8(PostBuildInfoType)
    {
        PBI_CURRENT_SIZE,
        PBI_COMPACTED_SIZE,
        PBI_DXR_TOOLS_VISUALIZATION_DESC,
        PBI_DXR_SERIALIZATION_DESC,
    };

    enum_uint32(HazardTypes)
    {
        HAZARD_RTAS_READ       = 1 << 0,
        HAZARD_RTAS_WRITE      = 1 << 1,
        HAZARD_READ            = 1 << 2,
        HAZARD_WRITE           = 1 << 3,
        HAZARD_ALL             = 0xf
    };
    
    enum_uint32(RaytracingAccelerationStructureType)
    {
        TOP_LEVEL    = 0x0,
        BOTTOM_LEVEL = 0x1,
    };

    typedef struct PostbuildInfoCurrentSize
    {
        uint64_t CurrentSizeInBytes;
    } PostbuildInfoCurrentSize;

    typedef struct PostbuildInfoCompactedSize
    {
        uint64_t CompactedSizeInBytes;
    } PostbuildInfoCompactedSize;

    typedef struct PostbuildInfoToolsVisualizationDesc
    {
        uint64_t DecodedSizeInBytes;
    } PostbuildInfoToolsVisualizationDesc;

    typedef struct PostbuildInfoSerializationDesc
    {
        uint64_t SerializedSizeInBytes;
        uint64_t NumBottomLevelAccelerationStructurePointers;
    } PostbuildInfoSerializationDesc;

    typedef struct DecodeHeader
    {
        RaytracingAccelerationStructureType Type;
        uint32_t NumDesc;
    } DecodeHeader;


GRL_NAMESPACE_END(GRL)