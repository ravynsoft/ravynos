//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "api_interface.h"
#include "d3d12.h"
#include "common.h"
#include "mem_utils.h"
#include "misc_shared.h"

#define offsetof(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

GRL_INLINE
uint GroupCountForCopySize(uint size)
{
    return (size >> 8) + 4;
}

GRL_INLINE
uint GroupCountForCopy(BVHBase* base)
{
    return GroupCountForCopySize(base->Meta.allocationSize);
}

GRL_INLINE void copyInstanceDescs(InstanceDesc* instances, D3D12_RAYTRACING_INSTANCE_DESC* descs, uint64_t numInstances)
{
    for (uint64_t instanceIndex = get_local_id(0); instanceIndex < numInstances; instanceIndex += get_local_size(0))
    {
        for (uint row = 0; row < 3; row++)
        {
            for (uint column = 0; column < 4; column++)
            {
                D3D12_set_transform(&descs[instanceIndex], row, column, InstanceDesc_get_transform(&instances[instanceIndex], row, column));
            }
        }
        D3D12_set_instanceID(&descs[instanceIndex], InstanceDesc_get_instanceID(&instances[instanceIndex]));
        D3D12_set_InstanceMask(&descs[instanceIndex], InstanceDesc_get_InstanceMask(&instances[instanceIndex]));
        D3D12_set_InstanceContributionToHitGroupIndex(&descs[instanceIndex], InstanceDesc_get_InstanceContributionToHitGroupIndex(&instances[instanceIndex]));
        D3D12_set_InstanceFlags(&descs[instanceIndex], InstanceDesc_get_InstanceFlags(&instances[instanceIndex]));
        D3D12_set_AccelerationStructure(&descs[instanceIndex], InstanceDesc_get_AccelerationStructure(&instances[instanceIndex]));
    }
}

GRL_INLINE void createGeoDescs(GeoMetaData* geoMetaData, D3D12_RAYTRACING_GEOMETRY_DESC* descs, uint64_t numGeos, const uint64_t dataBufferStart)
{
    if (get_local_id(0) == 0)
    {
        uint64_t previousGeoDataBufferEnd = dataBufferStart;
        for (uint64_t geoIndex = 0; geoIndex < numGeos; geoIndex += 1)
        {
            D3D12_set_Type(&descs[geoIndex], (uint8_t)(0xffff & geoMetaData[geoIndex].Type));
            D3D12_set_Flags(&descs[geoIndex], (uint8_t)(0xffff & geoMetaData[geoIndex].Flags));
            if (geoMetaData[geoIndex].Type == GEOMETRY_TYPE_TRIANGLES)
            {
                // Every triangle is stored separately
                uint64_t vertexBufferSize = 9 * sizeof(float) * geoMetaData[geoIndex].PrimitiveCount;
                D3D12_set_triangles_Transform(&descs[geoIndex], 0);
                D3D12_set_triangles_IndexFormat(&descs[geoIndex], INDEX_FORMAT_NONE);
                D3D12_set_triangles_VertexFormat(&descs[geoIndex], VERTEX_FORMAT_R32G32B32_FLOAT);
                D3D12_set_triangles_IndexCount(&descs[geoIndex], 0);
                D3D12_set_triangles_VertexCount(&descs[geoIndex], geoMetaData[geoIndex].PrimitiveCount * 3);
                D3D12_set_triangles_IndexBuffer(&descs[geoIndex], (D3D12_GPU_VIRTUAL_ADDRESS)previousGeoDataBufferEnd);
                D3D12_set_triangles_VertexBuffer_StartAddress(&descs[geoIndex], (D3D12_GPU_VIRTUAL_ADDRESS)previousGeoDataBufferEnd);
                D3D12_set_triangles_VertexBuffer_StrideInBytes(&descs[geoIndex], 3 * sizeof(float));
                previousGeoDataBufferEnd += vertexBufferSize;
            }
            else
            {
                D3D12_set_procedurals_AABBCount(&descs[geoIndex], geoMetaData[geoIndex].PrimitiveCount);
                D3D12_set_procedurals_AABBs_StartAddress(&descs[geoIndex], (D3D12_GPU_VIRTUAL_ADDRESS)previousGeoDataBufferEnd);
                D3D12_set_procedurals_AABBs_StrideInBytes(&descs[geoIndex], sizeof(D3D12_RAYTRACING_AABB));
                previousGeoDataBufferEnd += sizeof(D3D12_RAYTRACING_AABB) * geoMetaData[geoIndex].PrimitiveCount;
            }
        }
    }
}

GRL_INLINE void copyIndiciesAndVerticies(D3D12_RAYTRACING_GEOMETRY_DESC* desc, QuadLeaf* quad)
{
    float* vertices = (float*)D3D12_get_triangles_VertexBuffer_StartAddress(desc);
    uint64_t firstTriangleIndex = quad->primIndex0;
    uint64_t numTriangles = QuadLeaf_IsSingleTriangle(quad) ? 1 : 2;

    vertices[firstTriangleIndex * 9] = quad->v[0][0];
    vertices[firstTriangleIndex * 9 + 1] = quad->v[0][1];
    vertices[firstTriangleIndex * 9 + 2] = quad->v[0][2];

    vertices[firstTriangleIndex * 9 + 3] = quad->v[1][0];
    vertices[firstTriangleIndex * 9 + 4] = quad->v[1][1];
    vertices[firstTriangleIndex * 9 + 5] = quad->v[1][2];

    vertices[firstTriangleIndex * 9 + 6] = quad->v[2][0];
    vertices[firstTriangleIndex * 9 + 7] = quad->v[2][1];
    vertices[firstTriangleIndex * 9 + 8] = quad->v[2][2];

    if (numTriangles == 2)
    {
        uint64_t secondTriangleIndex = firstTriangleIndex + QuadLeaf_GetPrimIndexDelta(quad);
        uint32_t packed_indices = QuadLeaf_GetSecondTriangleIndices(quad);
        for( size_t i=0; i<3; i++ )
        {
            uint32_t idx = packed_indices & 3 ; packed_indices >>= 2;
            for( size_t j=0; j<3; j++ )
                vertices[secondTriangleIndex * 9 + i * 3 + j] = quad->v[idx][j];
        }
    }
}

GRL_INLINE
void storeProceduralDesc(
    struct AABB     procAABB,
    uint32_t        primId,
    D3D12_RAYTRACING_GEOMETRY_DESC* geoDesc)
{
    D3D12_RAYTRACING_AABB* proceduralDescs = (D3D12_RAYTRACING_AABB*)D3D12_get_procedurals_AABBs_StartAddress(geoDesc);
    D3D12_set_raytracing_aabb(&proceduralDescs[primId], &procAABB);
}

GRL_INLINE
void copyDataFromLProcedurals(
    BVHBase* base,
    D3D12_RAYTRACING_GEOMETRY_DESC* descs)
{
    unsigned numProcedurals = BVHBase_GetNumProcedurals(base);
    InternalNode* innerNodes = BVHBase_GetInternalNodes(base);
    unsigned numInnerNodes = BVHBase_GetNumInternalNodes(base);

    if (BVHBase_GetNumProcedurals(base) > 0) //< there's no point entering here if there are no procedurals
    {

        // iterate on all inner nodes to identify those with procedural children, we have to take aabbs from them
        for (uint32_t nodeI = get_local_id(0); nodeI < numInnerNodes; nodeI += get_local_size(0))
        {
            InternalNode* innerNode = innerNodes + nodeI;

            if (innerNode->nodeType == NODE_TYPE_PROCEDURAL)
            {
                float* origin = innerNode->lower;

                global struct ProceduralLeaf* leaf = (global struct ProceduralLeaf*)QBVHNodeN_childrenPointer((struct QBVHNodeN*)innerNode);

                for (uint k = 0; k < 6; k++)
                {
                    if (InternalNode_IsChildValid(innerNode, k))
                    {
                        struct AABB3f qbounds = {
                            (float)(innerNode->lower_x[k]), (float)(innerNode->lower_y[k]), (float)(innerNode->lower_z[k]),
                            (float)(innerNode->upper_x[k]), (float)(innerNode->upper_y[k]), (float)(innerNode->upper_z[k]) };

                        struct AABB dequantizedAABB;

                        dequantizedAABB.lower[0] = origin[0] + bitShiftLdexp(qbounds.lower[0], innerNode->exp_x - 8);
                        dequantizedAABB.lower[1] = origin[1] + bitShiftLdexp(qbounds.lower[1], innerNode->exp_y - 8);
                        dequantizedAABB.lower[2] = origin[2] + bitShiftLdexp(qbounds.lower[2], innerNode->exp_z - 8);
                        dequantizedAABB.upper[0] = origin[0] + bitShiftLdexp(qbounds.upper[0], innerNode->exp_x - 8);
                        dequantizedAABB.upper[1] = origin[1] + bitShiftLdexp(qbounds.upper[1], innerNode->exp_y - 8);
                        dequantizedAABB.upper[2] = origin[2] + bitShiftLdexp(qbounds.upper[2], innerNode->exp_z - 8);

                        dequantizedAABB = conservativeAABB(&dequantizedAABB);
                        /* extract geomID and primID from leaf */
                        const uint startPrim = QBVHNodeN_startPrim((struct QBVHNodeN*) innerNode, k);
                        const uint geomID = ProceduralLeaf_geomIndex(leaf);
                        const uint primID = ProceduralLeaf_primIndex(leaf, startPrim); // FIXME: have to iterate over all primitives of leaf!

                        storeProceduralDesc(dequantizedAABB, primID, descs + geomID);
                    }
                    /* advance leaf pointer to next child */
                    leaf += QBVHNodeN_blockIncr((struct QBVHNodeN*)innerNode, k);
                }

            }
            else if (innerNode->nodeType == NODE_TYPE_MIXED) { ERROR(); }
            else {/* do nothing for other internal node types, they can't have procedural child (directly)*/; }
        }
    }
}

GRL_INLINE
void copyDataFromQuadLeaves(BVHBase* base,
    D3D12_RAYTRACING_GEOMETRY_DESC* descs)
{
    QuadLeaf* quads = BVHBase_GetQuadLeaves(base);
    uint64_t numQuads = BVHBase_GetNumQuads(base);
    for (uint64_t quadIdx = get_local_id(0); quadIdx < numQuads; quadIdx += get_local_size(0))
    {
        uint64_t descIdx = PrimLeaf_GetGeoIndex(&quads[quadIdx].leafDesc);
        copyIndiciesAndVerticies(&descs[descIdx], &quads[quadIdx]);
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel clone_indirect(global char* dest,
    global char* src)
{
    BVHBase* base = (BVHBase*)src;
    uint64_t bvhSize = base->Meta.allocationSize;

    uint numGroups = GroupCountForCopy(base);
    CopyMemory(dest, src, bvhSize, numGroups);
}

GRL_INLINE void compactT(global char* dest, global char* src, uint64_t compactedSize, uint skipCopy, uint groupCnt)
{
    global BVHBase* baseSrc = (global BVHBase*)src;
    global BVHBase* baseDest = (global BVHBase*)dest;

    uint32_t offset = sizeof(BVHBase);
    uint32_t numNodes = BVHBase_GetNumInternalNodes(baseSrc);
    uint32_t nodeSize = numNodes * sizeof(InternalNode);
    offset += nodeSize;

    int quadChildFix = baseSrc->quadLeafStart;
    int procChildFix = baseSrc->proceduralDataStart;
    int instChildFix = baseSrc->instanceLeafStart;

    // serialization already copies part of bvh base so skip this part
    CopyMemory(dest + skipCopy, src + skipCopy, sizeof(BVHBase) - skipCopy, groupCnt);
    baseDest->Meta.allocationSize = compactedSize;

    if (baseSrc->Meta.instanceCount)
    {
        const uint32_t instLeafsSize = BVHBase_GetNumHWInstanceLeaves(baseSrc) * sizeof(HwInstanceLeaf);
        CopyMemory(dest + offset, (global char*)BVHBase_GetHWInstanceLeaves(baseSrc), instLeafsSize, groupCnt);
        const uint instanceLeafStart = (uint)(offset / 64);
        baseDest->instanceLeafStart = instanceLeafStart;
        instChildFix -= instanceLeafStart;
        offset += instLeafsSize;
        baseDest->instanceLeafEnd = (uint)(offset / 64);
    }
    if (baseSrc->Meta.geoCount)
    {
        const uint quadLeafsSize = BVHBase_GetNumQuads(baseSrc) * sizeof(QuadLeaf);
        if (quadLeafsSize)
        {
            CopyMemory(dest + offset, (global char*)BVHBase_GetQuadLeaves(baseSrc), quadLeafsSize, groupCnt);
            const uint quadLeafStart = (uint)(offset / 64);
            baseDest->quadLeafStart = quadLeafStart;
            quadChildFix -= quadLeafStart;
            offset += quadLeafsSize;
            baseDest->quadLeafCur = (uint)(offset / 64);
        }

        const uint procLeafsSize = BVHBase_GetNumProcedurals(baseSrc) * sizeof(ProceduralLeaf);
        if (procLeafsSize)
        {
            CopyMemory(dest + offset, (global char*)BVHBase_GetProceduralLeaves(baseSrc), procLeafsSize, groupCnt);
            const uint proceduralDataStart = (uint)(offset / 64);
            baseDest->proceduralDataStart = proceduralDataStart;
            procChildFix -= proceduralDataStart;
            offset += procLeafsSize;
            baseDest->proceduralDataCur = (uint)(offset / 64);
        }
    }
    // copy nodes with fixed child offsets
    global uint* nodeDest = (global uint*)(dest + sizeof(BVHBase));
    global InternalNode* nodeSrc = (global InternalNode*)BVHBase_GetInternalNodes(baseSrc);
    // used in mixed case
    char* instanceLeavesBegin = (char*)BVHBase_GetHWInstanceLeaves(baseSrc);
    char* instanceLeavesEnd = (char*)BVHBase_GetHWInstanceLeaves_End(baseSrc);
    uint localId = get_sub_group_local_id();
    for (uint i = get_group_id(0); i < numNodes; i += groupCnt)
    {
        uint nodePart = CacheLineSubgroupRead((const global char*)&nodeSrc[i]);
        char nodeType = as_char4(sub_group_broadcast(nodePart, offsetof(InternalNode, nodeType) / 4))[0];
        if (localId * 4 == offsetof(InternalNode, childOffset))
        {
            int childOffset = as_int(nodePart);
            if (nodeType == NODE_TYPE_MIXED)
            {
                char* childPtr = (char*)&nodeSrc[i] + 64 * childOffset;
                if (childPtr > instanceLeavesBegin && childPtr < instanceLeavesEnd)
                    nodePart = as_int(childOffset - instChildFix);
            }
            else if (nodeType == NODE_TYPE_INSTANCE)
                nodePart = as_int(childOffset - instChildFix);
            else if (nodeType == NODE_TYPE_QUAD)
                nodePart = as_int(childOffset - quadChildFix);
            else if (nodeType == NODE_TYPE_PROCEDURAL)
                nodePart = as_int(childOffset - procChildFix);
        }
        nodeDest[i * 16 + localId] = nodePart;
    }

    if (baseSrc->Meta.instanceCount)
    {
        const uint32_t instanceDescSize = baseSrc->Meta.instanceCount * sizeof(InstanceDesc);
        CopyMemory(dest + offset, src + baseSrc->Meta.instanceDescsStart, instanceDescSize, groupCnt);
        baseDest->Meta.instanceDescsStart = offset;
        offset += instanceDescSize;
    }
    if (baseSrc->Meta.geoCount)
    {
        const uint32_t geoMetaSize = baseSrc->Meta.geoCount * sizeof(GeoMetaData);
        CopyMemory(dest + offset, src + baseSrc->Meta.geoDescsStart, geoMetaSize, groupCnt);
        baseDest->Meta.geoDescsStart = offset;
        offset += (geoMetaSize + 63) & ~63; // align to 64
    }

    uint backPointerDataStart     = offset / 64;
    uint refitTreeletsDataStart   = backPointerDataStart;
    uint refitStartPointDataStart = backPointerDataStart;
    uint dataEnd                  = backPointerDataStart;
    uint fatLeafTableStart = dataEnd;
    uint fatLeafCount      = baseSrc->fatLeafCount;
    uint innerTableStart   = dataEnd;
    uint innerCount        = baseSrc->innerCount;
    
    uint quadLeftoversCountNewAtomicUpdate = baseSrc->quadLeftoversCountNewAtomicUpdate;
    uint quadTableSizeNewAtomicUpdate = baseSrc->quadTableSizeNewAtomicUpdate;
    uint quadIndicesDataStart = dataEnd;

    if (BVHBase_HasBackPointers(baseSrc))
    {
#if 0 //
        const uint oldbackpontersDataStart = baseSrc->backPointerDataStart;
        const uint shift = oldbackpontersDataStart - backPointerDataStart;
        const uint refitStructsSize = ((BVHBase_GetRefitStructsDataSize(baseSrc)) + 63) & ~63;

        CopyMemory(dest + offset, (global char*)BVHBase_GetBackPointers(baseSrc), refitStructsSize, groupCnt);

        refitTreeletsDataStart   = baseSrc->refitTreeletsDataStart - shift;
        refitStartPointDataStart = baseSrc->refitStartPointDataStart - shift;
        dataEnd                  = baseSrc->BVHDataEnd - shift;
#else // compacting version
        const uint backpointersSize = ((numNodes*sizeof(uint)) + 63) & ~63;
        CopyMemory(dest + offset, (global char*)BVHBase_GetBackPointers(baseSrc), backpointersSize, groupCnt);
        offset += backpointersSize;

        refitTreeletsDataStart = offset / 64;
        refitStartPointDataStart = offset / 64;

        // TODO: remove treelets from .... everywhere
        const uint treeletExecutedCnt = *BVHBase_GetRefitTreeletCntPtr(baseSrc);

        if (treeletExecutedCnt)
        {
            const uint treeletCnt = treeletExecutedCnt > 1 ? treeletExecutedCnt + 1 : 1;

            refitTreeletsDataStart = offset / 64;
            const uint treeletsSize = ((treeletCnt * sizeof(RefitTreelet)) + 63) & ~63;
            RefitTreelet* destTreelets = (RefitTreelet*)(dest + offset);
            RefitTreelet* srcTreelets = BVHBase_GetRefitTreeletDescs(baseSrc);

            uint numThreads = groupCnt * get_local_size(0);
            uint globalID = (get_group_id(0) * get_local_size(0)) + get_local_id(0);

            for (uint i = globalID; i < treeletCnt; i += numThreads)
            {
                RefitTreelet dsc = srcTreelets[i];
                RefitTreeletTrivial* trivial_dsc = (RefitTreeletTrivial*)&dsc;
                if (trivial_dsc->numStartpoints == 1 && trivial_dsc->childrenOffsetOfTheNode > numNodes) {
                    trivial_dsc->childrenOffsetOfTheNode -= quadChildFix;
                }
                destTreelets[i] = dsc;
            }

            offset += treeletsSize;

            refitStartPointDataStart = offset / 64;
            const uint startPointsSize = (BVHBase_GetRefitStartPointsSize(baseSrc) + 63) & ~63;
            CopyMemory(dest + offset, (global char*)BVHBase_GetRefitStartPoints(baseSrc), startPointsSize, groupCnt);
            offset += startPointsSize;
            dataEnd = offset / 64;
        }

        uint fatleafEntriesSize = ((fatLeafCount * sizeof(LeafTableEntry) + 63) & ~63);
        fatLeafTableStart = offset / 64;
        if (fatleafEntriesSize) {
            CopyMemory(dest + offset, (global char*)BVHBase_GetFatLeafTable(baseSrc), fatleafEntriesSize, groupCnt);
        }
        offset += fatleafEntriesSize;

        // New atomic update
        if(baseSrc->quadIndicesDataStart > baseSrc->backPointerDataStart)
        {
            uint numQuads = BVHBase_GetNumQuads(baseSrc);
            uint quadTableMainBufferSize = (numQuads + 255) & ~255;
            uint quadLeftoversSize = (quadLeftoversCountNewAtomicUpdate + 255) & ~255;
            uint quadTableEntriesSize = (((quadTableMainBufferSize + quadLeftoversSize) * sizeof(LeafTableEntry) + 63) & ~63);
            if (quadTableEntriesSize) {
                CopyMemory(dest + offset, (global char*)BVHBase_GetFatLeafTable(baseSrc), quadTableEntriesSize, groupCnt);
            }
            offset += quadTableEntriesSize;
                
            uint quadIndicesDataSize = ((numQuads * sizeof(QuadDataIndices) + 63) & ~63);
            quadIndicesDataStart = offset / 64;
            if (quadIndicesDataSize) {
                CopyMemory(dest + offset, (global char*)BVHBase_GetQuadDataIndicesTable(baseSrc), quadIndicesDataSize, groupCnt);
            }
            offset += quadIndicesDataSize;
        }

        uint innerEntriesSize = ((innerCount * sizeof(InnerNodeTableEntry) + 63) & ~63);
        innerTableStart = offset / 64;
        if (innerEntriesSize) {
            CopyMemory(dest + offset, (global char*)BVHBase_GetInnerNodeTable(baseSrc), innerEntriesSize, groupCnt);
        }
        offset += innerEntriesSize;

        dataEnd = offset / 64;
#endif
    }

    baseDest->backPointerDataStart = backPointerDataStart;
    baseDest->refitTreeletsDataStart = refitTreeletsDataStart;
    baseDest->refitStartPointDataStart = refitStartPointDataStart;
    baseDest->fatLeafTableStart = fatLeafTableStart ;
    baseDest->fatLeafCount = fatLeafCount;
    baseDest->innerTableStart = innerTableStart;
    baseDest->innerCount = innerCount;

    baseDest->quadLeftoversCountNewAtomicUpdate = quadLeftoversCountNewAtomicUpdate;
    baseDest->quadTableSizeNewAtomicUpdate = quadTableSizeNewAtomicUpdate;
    baseDest->quadIndicesDataStart = quadIndicesDataStart;
    baseDest->BVHDataEnd = dataEnd;
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
void kernel compact(global char* dest,
    global char* src,
    uint groupCnt)
{
    uint64_t compactedSize = compute_compacted_size((BVHBase*)src);
    compactT(dest, src, compactedSize, 0, groupCnt);
}

// set serialization header along all lanes, each lane will get one dword of header plus 64bit reminding data
GRL_INLINE
unsigned prepare_header(
    uint64_t headerSize,
    uint64_t instancePtrSize,
    uint64_t numInstances,
    uint64_t bvhSize,
    uint8_t* driverID,
    uint64_t reminder)
{

    unsigned loc_id = get_sub_group_local_id();

    uint64_t SerializedSizeInBytesIncludingHeader = headerSize + instancePtrSize * numInstances + bvhSize;
    uint64_t DeserializedSizeInBytes = bvhSize;
    uint64_t InstanceHandleCount = numInstances;

    char bvh_magic_str[] = BVH_MAGIC_MACRO;
    uint* bvh_magic_uint = (uint*)bvh_magic_str;

    unsigned headerTempLanePiece;
    if (loc_id < 4) { headerTempLanePiece = *((unsigned*)&driverID[4*loc_id]); }
    else if (loc_id == 4) { headerTempLanePiece = bvh_magic_uint[0]; }
    else if (loc_id == 5) { headerTempLanePiece = bvh_magic_uint[1]; }
    else if (loc_id == 6) { headerTempLanePiece = bvh_magic_uint[2]; }
    else if (loc_id == 7) { headerTempLanePiece = bvh_magic_uint[3]; }
    else if (loc_id == 8) { headerTempLanePiece = (uint)SerializedSizeInBytesIncludingHeader; }
    else if (loc_id == 9) { headerTempLanePiece = (uint)(SerializedSizeInBytesIncludingHeader >> 32ul); }
    else if (loc_id == 10) { headerTempLanePiece = (uint)DeserializedSizeInBytes; }
    else if (loc_id == 11) { headerTempLanePiece = (uint)(DeserializedSizeInBytes >> 32ul); }
    else if (loc_id == 12) { headerTempLanePiece = (uint)InstanceHandleCount; }
    else if (loc_id == 13) { headerTempLanePiece = (uint)(InstanceHandleCount >> 32ul); }
    else if (loc_id == 14) { headerTempLanePiece = (uint)reminder; }
    else if (loc_id == 15) { headerTempLanePiece = (uint)(reminder >> 32ul); }

    return headerTempLanePiece;
}




GRL_INLINE
void serializeT(
    global byte_align64B* dest,
    global byte_align64B* src,
    global uint8_t* driverID,
    uint groups_count)
{
    SerializationHeader* header = (SerializationHeader*)dest;
    BVHBase* base = (BVHBase*)src;

    const uint headerSize = sizeof(SerializationHeader);
    const uint numInstances = base->Meta.instanceCount;
    const uint instancePtrSize = sizeof(gpuva_t);
    const uint compactedSize = compute_compacted_size(base);
    uint local_id = get_sub_group_local_id();

    // this is not 64byte aligned :(
    const uint offsetToBvh = headerSize + instancePtrSize * numInstances;

    global InstanceDesc* src_instances = 0;

    if (numInstances) {
        src_instances = (global InstanceDesc*)((uint64_t)base + base->Meta.instanceDescsStart);
    }

    // effectively this part should end up as one 64B aligned 64B write
    if (get_group_id(0) == groups_count - 1)
    {
        Block64B headerPlus;

        // we patch the missing piece with instance or bhv beginning (TRICK A and B)
        // we assume header is 56B.
        global uint64_t* srcPiece = (numInstances != 0) ? &src_instances[0].AccelerationStructureGPUVA : (global uint64_t*)src;

        unsigned headerTemp;

        headerTemp = prepare_header(
            headerSize,
            instancePtrSize,
            numInstances,
            compactedSize,
            driverID,
            *srcPiece);

        CacheLineSubgroupWrite((global byte_align64B*)dest, headerTemp);
    }

    if (numInstances > 0)
    {
        uint instancesOffset = headerSize;
        uint aligned_instance_ptrs_offset = ((instancesOffset + 63) >> 6) << 6;
        uint unaligned_prefixing_instance_cnt = (aligned_instance_ptrs_offset - instancesOffset) >> 3;
        unaligned_prefixing_instance_cnt = min(unaligned_prefixing_instance_cnt, numInstances);

        global uint64_t* dst_instances = (global uint64_t*)(dest + instancesOffset);

        // we've copied first instance onto a header, (see TRICK A)
        // now we have only instances start at aligned memory
        uint numAlignedInstances = numInstances - unaligned_prefixing_instance_cnt;
        dst_instances += unaligned_prefixing_instance_cnt;
        src_instances += unaligned_prefixing_instance_cnt;

        if (numAlignedInstances)
        {
            // each 8 instances form a cacheline
            uint numCachelines = numAlignedInstances >> 3; //qwords -> 64Bs
            // qwords besides multiple of 8;
            uint startReminder = numAlignedInstances & ~((1 << 3) - 1);
            uint numreminder = numAlignedInstances & ((1 << 3) - 1);

            uint task_id = get_group_id(0);

            while (task_id < numCachelines)
            {
                uint src_id = task_id * 8 + (local_id >> 1);
                uint* src_uncorected = (uint*)& src_instances[src_id].AccelerationStructureGPUVA;
                uint* src = ((local_id & 1) != 0) ? src_uncorected + 1 : src_uncorected;
                uint data = *src;

                global char* dst = (global byte_align64B*)(dst_instances + (8 * task_id));
                CacheLineSubgroupWrite(dst, data);
                task_id += groups_count;
            }

            if (task_id == numCachelines && local_id < 8 && numreminder > 0)
            {
                // this should write full cacheline

                uint index = startReminder + local_id;
                // data will be taken from instances for lanes (local_id < numreminder)
                // copy srcbvh beginning as uint64_t for remaining lanes (TRICK B)
                global uint64_t* srcData = (local_id < numreminder) ?
                    &src_instances[index].AccelerationStructureGPUVA :
                    ((global uint64_t*)src) + (local_id - numreminder);
                dst_instances[index] = *srcData;
            }
        }
    }

    // the parts above copied unaligned dst beginning of bvh (see TRICK B)
    uint32_t unalignedPartCopiedElsewhere = (64u - (offsetToBvh & (64u - 1u)))&(64u - 1u);

    compactT(dest + offsetToBvh, src, compactedSize, unalignedPartCopiedElsewhere, groups_count);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
void kernel serialize_indirect(
    global char* dest,
    global char* src,
    global uint8_t* driverID)
{
    BVHBase* base = (BVHBase*)src;
    uint groups_count = GroupCountForCopy(base);
    serializeT(dest, src, driverID, groups_count);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
void kernel serialize_for_input_dump_indirect(
    global struct OutputBatchPtrs* batchPtrs,
    global dword* dstOffset,
    global char* src,
    global uint8_t* driverID)
{
    BVHBase* base = (BVHBase*)src;
    uint groups_count = GroupCountForCopy(base);
    global char* dest = (global char*)(batchPtrs->dataStart + *dstOffset);
    dest += (sizeof(OutputData) + 127) & ~127;
    serializeT(dest, src, driverID, groups_count);
}

GRL_INLINE
void deserializeT(
    global char* dest,
    global char* src,
    unsigned groupCnt)
{
    SerializationHeader* header = (SerializationHeader*)src;

    const uint64_t headerSize = sizeof(struct SerializationHeader);
    const uint64_t instancePtrSize = sizeof(gpuva_t);
    const uint64_t numInstances = header->InstanceHandleCount;
    const uint64_t offsetToBvh = headerSize + instancePtrSize * numInstances;
    const uint64_t bvhSize = header->DeserializedSizeInBytes;

    if (numInstances)
    {
        const bool instances_mixed_with_inner_nodes = false;
        if (instances_mixed_with_inner_nodes)
        {
            // not implemented !
            // copy each node with 64byte granularity if node is instance, patch it mid-copy
        }
        else
        {
            BVHBase* srcBvhBase = (BVHBase*)(src + offsetToBvh);

            // numHWInstances can be bigger (because of rebraiding) or smaller (because of inactive instances) than
            // numInstances (count of pointers and descriptors).
            uint offsetToHwInstances = srcBvhBase->instanceLeafStart << 6;
            uint numHwInstances = (srcBvhBase->instanceLeafEnd - srcBvhBase->instanceLeafStart) >> 1;

            //
            // instances are in separate memory intervals
            // copy all the other data simple way
            //
            uint nodesEnd = srcBvhBase->Meta.instanceDescsStart;
            // copy before instance leafs
            CopyMemory(dest, (global char*)(src + offsetToBvh), offsetToHwInstances, groupCnt);

            uint offsetPostInstances = srcBvhBase->instanceLeafEnd << 6;
            uint instanceDescStart = srcBvhBase->Meta.instanceDescsStart;
            uint sizePostInstances = instanceDescStart - offsetPostInstances;
            // copy after instance leafs before instance desc
            CopyMemory(dest + offsetPostInstances, (global char*)(src + offsetToBvh + offsetPostInstances), sizePostInstances, groupCnt);

            uint instanceDescEnd = instanceDescStart + numInstances * sizeof(InstanceDesc);
            uint sizePostInstanceDescs = bvhSize - instanceDescEnd;
            // copy after instance desc
            CopyMemory(dest + instanceDescEnd, (global char*)(src + offsetToBvh + instanceDescEnd), sizePostInstanceDescs, groupCnt);

            global gpuva_t* newInstancePtrs = (global gpuva_t*)(src + headerSize);
            global InstanceDesc* dstDesc = (global InstanceDesc*)(dest + instanceDescStart);
            global InstanceDesc* srcDesc = (global InstanceDesc*)(src + offsetToBvh + instanceDescStart);

            // copy and patch instance descriptors
            for (uint64_t instanceIndex = get_group_id(0); instanceIndex < numInstances; instanceIndex += groupCnt)
            {
                InstanceDesc desc = srcDesc[instanceIndex];
                uint64_t newInstancePtr = newInstancePtrs[instanceIndex];
                desc.AccelerationStructureGPUVA = newInstancePtr; // patch it with new ptr;

                dstDesc[instanceIndex] = desc;
            }

            // copy and patch hw instance leafs
            global HwInstanceLeaf* dstInstleafs = (global HwInstanceLeaf*)(dest + offsetToHwInstances);
            global HwInstanceLeaf* srcInstleafs = (global HwInstanceLeaf*)(src + offsetToBvh + offsetToHwInstances);

            for (uint hwLeafIndex = get_group_id(0); hwLeafIndex < numHwInstances; hwLeafIndex += groupCnt)
            {
                // pull the instance from srcBVH
                HwInstanceLeaf tmpInstleaf = srcInstleafs[hwLeafIndex];

                uint swInstanceIndex = HwInstanceLeaf_GetInstanceIndex(&tmpInstleaf);
                uint64_t childBvhPtr = (uint64_t)newInstancePtrs[swInstanceIndex];
                uint64_t originalBvhPtr = (uint64_t)HwInstanceLeaf_GetBVH(&tmpInstleaf);

                HwInstanceLeaf_SetBVH(&tmpInstleaf, childBvhPtr);
                uint64_t startNode = HwInstanceLeaf_GetStartNode(&tmpInstleaf);

                if (startNode != 0) {
                    uint64_t rootNodeOffset = startNode - originalBvhPtr;
                    HwInstanceLeaf_SetStartNode(&tmpInstleaf, childBvhPtr + rootNodeOffset);
                }

                dstInstleafs[hwLeafIndex] = tmpInstleaf;
            }
        }
    }
    else
    {
        CopyMemory(dest, (global char*)(src + offsetToBvh), bvhSize, groupCnt);
    }
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel deserialize_indirect(
    global char* dest,
    global char* src)
{
    SerializationHeader* header = (SerializationHeader*)src;
    const uint64_t bvhSize = header->DeserializedSizeInBytes;
    unsigned groupCnt = GroupCountForCopySize(bvhSize);
    deserializeT(dest, src, groupCnt);
}

GRL_ANNOTATE_IGC_DO_NOT_SPILL
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1))) void kernel dxr_decode(global char* dest,
    global char* src)
{

    DecodeHeader* header = (DecodeHeader*)dest;
    BVHBase* base = (BVHBase*)src;

    uint32_t numGeos = base->Meta.geoCount;
    uint32_t numInstances = base->Meta.instanceCount;

    if (numInstances > 0)
    {
        header->Type = TOP_LEVEL;
        header->NumDesc = numInstances;

        D3D12_RAYTRACING_INSTANCE_DESC* instanceDesc = (D3D12_RAYTRACING_INSTANCE_DESC*)(dest + sizeof(DecodeHeader));
        copyInstanceDescs((InstanceDesc*)((uint64_t)base + (uint64_t)base->Meta.instanceDescsStart),
            instanceDesc,
            numInstances);
    }
    else if (numGeos > 0)
    {
        header->Type = BOTTOM_LEVEL;
        header->NumDesc = numGeos;

        D3D12_RAYTRACING_GEOMETRY_DESC* geomDescs = (D3D12_RAYTRACING_GEOMETRY_DESC*)(dest + sizeof(DecodeHeader));
        uint64_t data = (uint64_t)geomDescs + sizeof(D3D12_RAYTRACING_GEOMETRY_DESC) * numGeos;
        createGeoDescs((GeoMetaData*)((uint64_t)base + (uint64_t)base->Meta.geoDescsStart),
            geomDescs,
            numGeos,
            data);

        work_group_barrier(CLK_GLOBAL_MEM_FENCE);

        copyDataFromQuadLeaves(base,
            geomDescs);

        copyDataFromLProcedurals(base,
            geomDescs);
    }
    else
    {
        header->Type = BOTTOM_LEVEL;
        header->NumDesc = 0;
    }
}
