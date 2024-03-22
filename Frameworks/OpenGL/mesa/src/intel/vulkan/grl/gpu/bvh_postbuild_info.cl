//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "api_interface.h"
#include "d3d12.h"
#include "common.h"

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1))) void kernel compacted_size(global char *bvh_mem,
                                                                          global char *postbuild_info)
{
    BVHBase *base = (BVHBase *)bvh_mem;
    PostbuildInfoCompactedSize *postbuildInfoCompacted = (PostbuildInfoCompactedSize *)postbuild_info;

    postbuildInfoCompacted->CompactedSizeInBytes = compute_compacted_size(base);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1))) void kernel current_size(global char *bvh_mem,
                                                                        global char *postbuild_info)
{

    BVHBase *base = (BVHBase *)bvh_mem;
    PostbuildInfoCurrentSize *postbuildInfoCurrent = (PostbuildInfoCurrentSize *)postbuild_info;

    postbuildInfoCurrent->CurrentSizeInBytes = base->Meta.allocationSize;
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1))) void kernel serialized_size(global char *bvh_mem,
                                                                           global char *postbuild_info)
{

    BVHBase *base = (BVHBase *)bvh_mem;
    PostbuildInfoSerializationDesc *postbuildInfoSerialization = (PostbuildInfoSerializationDesc *)postbuild_info;

    uint64_t headerSize = sizeof(SerializationHeader);
    uint64_t numInstances = base->Meta.instanceCount;

    postbuildInfoSerialization->SerializedSizeInBytes = sizeof(SerializationHeader) +
                                                        numInstances * sizeof(gpuva_t) +
                                                        compute_compacted_size(base);
                                                        //base->Meta.allocationSize;
    postbuildInfoSerialization->NumBottomLevelAccelerationStructurePointers = numInstances;
}

void countTrianglesAndProcedurals(GeoMetaData *geoMetaData,
                                  uint64_t numGeos,
                                  uint64_t *numTriangles,
                                  uint64_t *numProcedurals)
{
    uint64_t numTrianglesLoc = 0;
    uint64_t numProceduralsLoc = 0;

    for (uint64_t geoIndex = get_local_id(0); geoIndex < numGeos; geoIndex += get_local_size(0))
    {
        if (geoMetaData[geoIndex].Type == GEOMETRY_TYPE_TRIANGLES)
        {
            *numTriangles += geoMetaData[geoIndex].PrimitiveCount;
        }
        else
        {
            *numProcedurals += geoMetaData[geoIndex].PrimitiveCount;
        }
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1))) void kernel decoded_size(global char *bvh_mem,
                                                                        global char *postbuild_info)
{
    BVHBase *base = (BVHBase *)bvh_mem;
    PostbuildInfoToolsVisualizationDesc *postbuildInfoDecoded = (PostbuildInfoToolsVisualizationDesc *)postbuild_info;

    uint64_t numTriangles = 0;
    uint64_t numProcedurals = 0;
    countTrianglesAndProcedurals((GeoMetaData *)((uint64_t)base + (uint64_t)base->Meta.geoDescsStart),
                                 base->Meta.geoCount,
                                 &numTriangles,
                                 &numProcedurals);
    uint64_t numInstances = base->Meta.instanceCount;
    uint64_t numDescs = base->Meta.geoCount;
    uint64_t headerSize = sizeof(DecodeHeader);
    uint64_t descsSize = numDescs * sizeof(D3D12_RAYTRACING_GEOMETRY_DESC) +
                         numInstances * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);

    // Each triangle is stored separately - 3 vertices (9 floats) per triangle
    uint64_t triangleDataSize = 9 * sizeof(float);
    uint64_t proceduralDataSize = sizeof(D3D12_RAYTRACING_AABB);
    uint64_t geoDataSize = numTriangles * triangleDataSize + numProcedurals * proceduralDataSize;

    postbuildInfoDecoded->DecodedSizeInBytes = headerSize + descsSize + geoDataSize;
}
