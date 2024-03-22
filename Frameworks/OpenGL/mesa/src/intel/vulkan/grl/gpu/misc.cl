//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "api_interface.h"
#include "common.h"
#include "instance.h"
#include "misc_shared.h"
#include "mem_utils.h"

#define DBG(x)
#define ENABLE_CHECKS 0

#define CACHELINE_SIZE 64
#define CACHELINE_PER_BLOCK 4
#define BLOCK_SIZE 256 // = CACHELINE_SIZE * CACHELINE_PER_BLOCK;

GRL_INLINE
uint32_t getGeomDescPrimitiveCountAsUint32t(global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc, uint64_t index)
{
    return  (uint32_t)GRL_get_primitive_count(&geomDesc[index]);
}

GRL_INLINE
uint32_t getGeomDescTypeAndFlagsAsUint32t(global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc, uint64_t index)
{
    return (uint32_t)GRL_get_Type(&geomDesc[index]) |
           (((uint32_t)GRL_get_Flags(&geomDesc[index])) << 16);
}

GRL_INLINE
uint64_t getGeomDescAsUint64t(global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc, uint64_t index)
{
    return (uint64_t)getGeomDescPrimitiveCountAsUint32t(geomDesc, index) |
           (((uint64_t)getGeomDescTypeAndFlagsAsUint32t(geomDesc, index)) << 32);
}

// assummed:
// dst is always 64 bytes alligned
GRL_INLINE
void copyGeoMetaData(global char* dst, global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc, uint64_t size, uint numGroups)
{
    uint taskId = get_group_id(0);
    uint localId = get_sub_group_local_id();

    uint cachelinedSize = (size) & (~(CACHELINE_SIZE-1));

    uint reminderOffset = cachelinedSize;
    uint reminderQWSize = (size - reminderOffset) >> 3;

    uint tailCacheLines = cachelinedSize >> 6; // divide by CACHELINE_SIZE
    uint reversedTaskId = (uint)(-(((int)taskId) - ((int)numGroups-1)));
    if (reversedTaskId == tailCacheLines && localId < reminderQWSize)
    {
        uint reminderOffsetQW = reminderOffset >> 3;
        global uint64_t* dstQW = (global uint64_t*)(dst);
        dstQW[localId + reminderOffsetQW] = getGeomDescAsUint64t(geomDesc, localId + reminderOffsetQW);
    }

    uint numCacheLines = cachelinedSize >> 6;
    while (taskId < numCacheLines)
    {
        uint byteOffset = taskId * CACHELINE_SIZE;
        uint geoIdFromOffset = (byteOffset >> 3) + (localId >> 1);

        uint32_t data = 0;
        if (localId & 1)
        {
            data = getGeomDescTypeAndFlagsAsUint32t(geomDesc, geoIdFromOffset);
        }
        else
        {
            data = getGeomDescPrimitiveCountAsUint32t(geomDesc, geoIdFromOffset);
        }
        CacheLineSubgroupWrite(dst + byteOffset, data);

        taskId += numGroups;
    }
}

GRL_INLINE
uint groupCountForInstancesCopySize(uint size)
{
    return (size >> 8) + 3;
}

GRL_INLINE
uint groupCountForGeoMetaDataCopySize(uint size)
{
    return (size >> 6) + 1;
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_instances(global char* dest, global char* instancesArray, uint64_t size)
{
  //  global char *dest = (global char *)((unsigned long)bvh + bvh->Meta.instanceDescsStart);
    copyInstances(dest, instancesArray, NULL, size, groupCountForInstancesCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_instances_indirect(global char* dest, global char* instancesArray, global const struct IndirectBuildRangeInfo* const indirect_data)
{
    uint64_t size = indirect_data->primitiveCount * sizeof(InstanceDesc);
    instancesArray += indirect_data->primitiveOffset;
    uint tid = get_sub_group_local_id() + get_group_id(0) * MAX_HW_SIMD_WIDTH;
    if (tid == 0)
    {
        struct BVHBase* bvh     = (struct BVHBase*)dest;
        bvh->Meta.instanceCount = indirect_data->primitiveCount;
    }
    copyInstances(dest, instancesArray, NULL, size, groupCountForInstancesCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_instance_ptrs(global char* dest, global uint64_t* arrayOfPtrs, uint64_t size)
{
    //global char *dest = (global char *)((unsigned long)bvh + bvh->Meta.instanceDescsStart);
    copyInstances(dest, NULL, arrayOfPtrs, size, groupCountForInstancesCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_instance_ptrs_indirect(global char* dest, global uint64_t* arrayOfPtrs, global struct IndirectBuildRangeInfo const * const indirect_data)
{
    uint64_t size = indirect_data->primitiveCount * sizeof(InstanceDesc);
    arrayOfPtrs += indirect_data->primitiveOffset;
    uint tid = get_sub_group_local_id() + get_group_id(0) * MAX_HW_SIMD_WIDTH;
    if (tid == 0)
    {
        struct BVHBase* bvh     = (struct BVHBase*)dest;
        bvh->Meta.instanceCount = indirect_data->primitiveCount;
    }
    copyInstances(dest, NULL, arrayOfPtrs, size, groupCountForInstancesCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_instances_base_ptr(global BVHBase* bvh, global char* instancesArray, uint64_t size)
{
    global char *dest = (global char *)((unsigned long)bvh + bvh->Meta.instanceDescsStart);
    copyInstances(dest, instancesArray, NULL, size, groupCountForInstancesCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_instances_base_ptr_indirect(global BVHBase* bvh, global char* instancesArray, global struct IndirectBuildRangeInfo const * const indirect_data)
{
    global char* dest = (global char*)((unsigned long)bvh + bvh->Meta.instanceDescsStart);
    uint64_t     size = indirect_data->primitiveCount * sizeof(InstanceDesc);
    instancesArray += indirect_data->primitiveOffset;
    copyInstances(dest, instancesArray, NULL, size, groupCountForInstancesCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_instance_ptrs_base_ptr(global BVHBase* bvh, global uint64_t* arrayOfPtrs, uint64_t size)
{
    global char *dest = (global char *)((unsigned long)bvh + bvh->Meta.instanceDescsStart);
    copyInstances(dest, NULL, arrayOfPtrs, size, groupCountForInstancesCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_instance_ptrs_base_ptr_indirect(global BVHBase* bvh, global uint64_t* arrayOfPtrs, global struct IndirectBuildRangeInfo const * const indirect_data)
{
    global char* dest = (global char*)((unsigned long)bvh + bvh->Meta.instanceDescsStart);
    uint64_t     size = indirect_data->primitiveCount * sizeof(InstanceDesc);
    arrayOfPtrs += indirect_data->primitiveOffset;
    copyInstances(dest, NULL, arrayOfPtrs, size, groupCountForInstancesCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel copy_geo_meta_data(global char* dest, global char* src, uint64_t size)
{
    //global char *dest = (global char *)((unsigned long)bvh + bvh->Meta.geoDescsStart);
    global GRL_RAYTRACING_GEOMETRY_DESC *geomDesc = (global GRL_RAYTRACING_GEOMETRY_DESC *)((unsigned long)src);
    copyGeoMetaData(dest, geomDesc, size, groupCountForGeoMetaDataCopySize(size));
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__( ( reqd_work_group_size( MAX_HW_SIMD_WIDTH, 1, 1 ) ) )
__attribute__( ( intel_reqd_sub_group_size( MAX_HW_SIMD_WIDTH ) ) )
void kernel copy_geo_descs_indirect_build(global char* dest, global char* src, global struct IndirectBuildRangeInfo const * const indirect_data, uint numGeometries)
{
    uint32_t gid = get_local_id(0) + get_group_id(0) * get_local_size(0);
    if (gid < numGeometries) {
        global GRL_RAYTRACING_GEOMETRY_DESC* dstDesc = (global GRL_RAYTRACING_GEOMETRY_DESC*)(dest);
        global GRL_RAYTRACING_GEOMETRY_DESC* srcDesc = (global GRL_RAYTRACING_GEOMETRY_DESC*)(src);

        GRL_RAYTRACING_GEOMETRY_DESC geo = srcDesc[gid];

        uint primitiveCount  = indirect_data[gid].primitiveCount;
        uint primitiveOffset = indirect_data[gid].primitiveOffset;
        uint firstVertex     = indirect_data[gid].firstVertex;
        uint transformOffset = indirect_data[gid].transformOffset;

        if (srcDesc[gid].Type == GEOMETRY_TYPE_TRIANGLES)
        {
            if (geo.Desc.Triangles.IndexFormat == INDEX_FORMAT_NONE)
            {
                geo.Desc.Triangles.VertexCount = primitiveCount * 3;
                geo.Desc.Triangles.pVertexBuffer += primitiveOffset
                                                    + firstVertex * geo.Desc.Triangles.VertexBufferByteStride;
            }
            else
            {
                geo.Desc.Triangles.IndexCount = primitiveCount * 3;
                geo.Desc.Triangles.pIndexBuffer += primitiveOffset;
                geo.Desc.Triangles.pVertexBuffer += firstVertex * geo.Desc.Triangles.VertexBufferByteStride;
            }
            if (geo.Desc.Triangles.pTransformBuffer) {
                geo.Desc.Triangles.pTransformBuffer += transformOffset;
            }
        } else {
            // GEOMETRY_TYPE_PROCEDURAL
            geo.Desc.Procedural.AABBCount = primitiveCount;
            geo.Desc.Procedural.pAABBs_GPUVA += primitiveOffset;
        }

        dstDesc[gid] = geo;
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(1, 1, 1))) void kernel batched_init_globals(global struct BatchedInitGlobalsData *data)
{
    uint groupID = get_group_id(0);

    struct BatchedInitGlobalsData entry = data[groupID];

    global struct Globals* globals = (global struct Globals*)entry.p_build_globals;
    global char *bvh_mem = (global char*)entry.p_bvh_buffer;
    uint numPrimitives = entry.numPrimitives;
    uint numGeometries = entry.numGeometries;
    uint numInstances = entry.numInstances;
    uint instance_descs_start = entry.instance_descs_start;
    uint geo_meta_data_start = entry.geo_meta_data_start;
    uint node_data_start = entry.node_data_start;
    uint quad_data_start = entry.leaf_data_start;
    uint instance_data_start = entry.leaf_data_start;
    uint procedural_data_start = entry.procedural_data_start;
    uint back_pointer_start = entry.back_pointer_start;
    uint build_record_start = entry.leaf_data_start;
    uint totalBytes = entry.sizeTotal;
    uint leafPrimType = entry.leafType;
    uint leafSize = entry.leafSize;

    uint root_node_offset = node_data_start;
    struct BVHBase *base = (struct BVHBase *)bvh_mem;

    base->Meta.instanceCount      = numInstances;
    base->Meta.geoCount           = numGeometries;
    base->Meta.instanceDescsStart = instance_descs_start;
    base->Meta.geoDescsStart      = geo_meta_data_start;
    base->Meta.allocationSize     = totalBytes;
    // This doesnt work correctly
    //ERROR_INFO initErr = { 0, 0, 0, 0xAAABBAAA };
    //base->Meta.errors = initErr;
    base->Meta.errors.type = 0;
    base->Meta.errors.offset_in_BVH = 0; //in 64B units
    base->Meta.errors.when = 0;
    base->Meta.errors.reserved = 0xAAABBAAA;

    base->nodeDataCur = node_data_start / 64;
    base->quadLeafStart = quad_data_start / 64;
    base->quadLeafCur = quad_data_start / 64;
    base->instanceLeafStart = instance_data_start / 64;
    base->instanceLeafEnd = instance_data_start / 64;
    base->proceduralDataStart = procedural_data_start / 64;
    base->proceduralDataCur = procedural_data_start / 64;
    base->backPointerDataStart = back_pointer_start / 64;
    base->refitTreeletsDataStart = totalBytes / 64;
    base->refitStartPointDataStart = totalBytes / 64;
    base->BVHDataEnd = totalBytes / 64;
    base->refitTreeletCnt = 0;
    base->refitTreeletCnt2 = 0;
    base->rootNodeOffset = root_node_offset;

    base->fatLeafCount = 0;
    base->fatLeafTableStart = entry.fatleaf_table_start / 64;
    base->innerCount = 0;
    base->innerTableStart = entry.innernode_table_start / 64;
    base->quadLeftoversCountNewAtomicUpdate = 0;
    base->quadTableSizeNewAtomicUpdate = 0;
    base->quadIndicesDataStart = entry.quad_indices_data_start / 64;

    if (back_pointer_start != totalBytes)
    {
        BackPointers* back_pointers = BVHBase_GetBackPointers(base);
        uint root_node_idx = root_node_offset - node_data_start;
        global uint *root_node_backpointer = (global uint *)InnerNode_GetBackPointer(back_pointers,root_node_idx);
        *root_node_backpointer = ((uint)-1) << 6;
    }

    AABB3f_init(&base->Meta.bounds);
    AABB_init(&globals->centroidBounds);

    globals->build_record_start = build_record_start;

    globals->numBuildRecords = 0;
    globals->numBuildRecords_extended = 0;
    globals->numPrimitives = numPrimitives;
    globals->numSplittedPrimitives = 0;
    globals->sync = 0;
    globals->probThreshold = 0.0f;
    globals->leafPrimType = leafPrimType;
    globals->leafSize = leafSize;
}



// This is temporary WA for mock in DXR
GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1))) void kernel copy_mock(global char *dest,
                                                                                     global char *src,
                                                                                     uint32_t size)
{
    uint32_t globalId = get_local_id(0) + get_group_id(0) * get_local_size(0);
    uint32_t globalSize = get_num_groups(0) * get_local_size(0);
    for (uint32_t i = globalId; i < size; i += globalSize)
    {
        dest[i] = src[i];
    }
}


GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(32, 1, 1)))
void kernel mem_set(global char *dest,
    dword byte,
    dword size)
{
    uint32_t globalId = get_local_id(0) + get_group_id(0) * get_local_size(0);
    if (globalId < size)
    {
        dest[globalId] = (char)byte;
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(32, 1, 1)))
void kernel mem_set_size_ptr(global char *dest,
    dword byte,
    global qword* sizePtr)
{
    uint32_t globalId = get_local_id(0) + get_group_id(0) * get_local_size(0);
    if (globalId < *sizePtr)
    {
        dest[globalId] = (char)byte;
    }
}
