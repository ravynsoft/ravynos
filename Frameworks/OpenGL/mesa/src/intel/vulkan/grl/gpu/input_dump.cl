//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "api_interface.h"
#include "common.h"
#include "d3d12.h"
#include "mem_utils.h"
#include "misc_shared.h"

/// Align value to 128
///
/// @param value vale to align
/// @return aligned value
GRL_INLINE ulong AlignTo128(ulong value) { return ((value + 127) / 128) * 128; }

GRL_INLINE char* GetVertexBuffersStart(global InputBatchPtrs* batchPtrs) {
    return (global char*)(batchPtrs->dumpDst + AlignTo128(sizeof(InputBatch)));
}

/// Finds max used byte in vertex buffer
///
/// @param indexBuffPtr pointer to index buffer
/// @param vertexBufferUsedByteEnd pointer to max used byte of vertex buffers
/// @param IndexCount number of indices in index buffer
/// @param IndexFormat index format
/// @param VertexCount number of vertices in vertex buffer
/// @param VertexBufferByteStride vertex buffer byte stride
__attribute__((reqd_work_group_size(256, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel find_max_used_byte_in_buff(
    global void* indexBuffPtr,
    global uint* vertexBufferUsedByteEnd,
    dword IndexCount,
    dword IndexFormat,
    dword VertexCount,
    qword VertexBufferByteStride)
{
    local uint sgMax[16];
    uint glob_id = get_group_id(0) * get_local_size(0) + get_local_id(0);

    if (IndexFormat != INDEX_FORMAT_NONE)
    {
        uint endByte = 0;
        if (glob_id < IndexCount)
        {
            if (IndexFormat == INDEX_FORMAT_R16_UINT)
            {
                global ushort* indexBuffPtrShort = (global ushort*) indexBuffPtr;
                endByte = indexBuffPtrShort[glob_id];
            }
            else
            {
                global uint* indexBuffPtrUint = (global uint*) indexBuffPtr;
                endByte = indexBuffPtrUint[glob_id];
            }
        }

        endByte = sub_group_reduce_max(endByte);

        if (get_sub_group_local_id() == 0) { sgMax[get_sub_group_id()] = endByte; }

        barrier(CLK_LOCAL_MEM_FENCE);

        if (get_sub_group_id() == 0)
        {
            endByte = sub_group_reduce_max(sgMax[get_sub_group_local_id()]);
            if (get_sub_group_local_id() == 0) 
            {
                endByte = min(endByte, VertexCount);
                if (endByte < VertexCount && IndexCount != 0)
                    ++endByte;
                endByte *= (dword)VertexBufferByteStride;
                atomic_max(vertexBufferUsedByteEnd, endByte);
            }
        }
    }
    else if (glob_id == 0)
    {
        uint endByte = VertexCount * VertexBufferByteStride;
        atomic_max(vertexBufferUsedByteEnd, endByte);
    }
}

/// Allocates buffer for vertices
///
/// @param batchPtrs batch pointers struct
/// @param vertexBufferUsedByteEnd pointer to sizes of vertex buffers
/// @param vertexBufferOffset pointer to offsets to vertex buffers
/// @param numVertexBuffers number of vertex buffers
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel allocate_linear_offsets_for_vertex_buffers(
    global InputBatchPtrs* batchPtrs,
    global uint* vertexBufferUsedByteEnd,
    global uint* vertexBufferOffset,
    dword numVertexBuffers)
{
    uint glob_id = get_group_id(0) * get_local_size(0) + get_sub_group_local_id();

    if (glob_id < numVertexBuffers)
    {
        uint numBytes = AlignTo128(vertexBufferUsedByteEnd[glob_id]);
        uint position = atomic_add_global( &batchPtrs->vertexBuffersSize, numBytes);
        vertexBufferOffset[glob_id] = position;
    }
}

/// Sets the dst data space for input dump of this batch
///
/// @param inputDumpMainBuffer pointer to main dump buffer
/// @param batchPtrs batch pointers struct
/// @param nonVertexSize size of non vertex data
/// @param batchIdPtr pointer to batch id
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel allocate_data_space_for_inputs(
    global DebugBufferHeader* inputDumpMainBuffer,
    global InputBatchPtrs* batchPtrs,
    uint nonVertexSize,
    global qword* batchIdPtr)
{
    if (get_sub_group_local_id() == 0) 
    {
        uint vertexBufferSize = batchPtrs->vertexBuffersSize;
        uint sizeOfThisBatch = vertexBufferSize + AlignTo128(sizeof(InputBatch)) + nonVertexSize;

        if ((sizeOfThisBatch + sizeof(InputBatch)) > ((inputDumpMainBuffer->totalSize - inputDumpMainBuffer->headStart) / 2)) 
        {
            inputDumpMainBuffer->overflow = 1;
            batchPtrs->dumpDst = 0;
            batchPtrs->globalDumpBuffer = 0;
            batchPtrs->nonVertexDataStart = 0;
            batchPtrs->totalSize = 0;
            return;
        }

        dword prevHead = inputDumpMainBuffer->gpuHead;
        dword newHead;
        bool circled;

        do
        {
            circled = false;
            newHead = prevHead + sizeOfThisBatch;
            dword bufferBegin = prevHead;
            if ((newHead + sizeof(InputBatch)) > inputDumpMainBuffer->totalSize)
            {
                circled = true;
                newHead = inputDumpMainBuffer->headStart + sizeOfThisBatch;
                bufferBegin = inputDumpMainBuffer->headStart;
            }
            dword bufferEnd = newHead + sizeof(InputBatch);

            uint tail;
            uint tail2 = 7;
            bool wait;
            do
            {
                wait = true;
                tail = load_uint_L1UC_L3UC(&inputDumpMainBuffer->tail, 0);

                // dead code, workaround so IGC won't move tail load out of loop
                if (tail > inputDumpMainBuffer->totalSize) 
                {
                   store_uint_L1UC_L3UC(&inputDumpMainBuffer->tail, 0, tail + tail2);
                   tail2 = tail;
                }

                if( prevHead >= tail )
                {
                    //colision example:
                    //  ----------T=======H------------
                    //  -------B=====E-----------------
                    //
                    if((bufferEnd < tail) || (bufferBegin >= prevHead))
                    {
                        wait = false;
                    }
                }
                else 
                {
                    //colision example:
                    //  ==========H-------T============
                    //  B==============E---------------
                    // caution: we will never have H circled completely so that H == T
                    if((bufferEnd < tail) && (bufferBegin >= prevHead)) 
                    {
                        wait = false;
                    }
                }
            } while (wait);
        } while (!atomic_compare_exchange_global(&inputDumpMainBuffer->gpuHead, &prevHead, newHead));

        if (circled)
        {
            global InputBatch* endBufferOp = (global InputBatch*)(((global char*)inputDumpMainBuffer) + prevHead);
            endBufferOp->header.opHeader.operationType = INPUT_DUMP_OP_END_BUFFER;
            prevHead = inputDumpMainBuffer->headStart;
        }

        global char* thisBatchDump = ((global char*)inputDumpMainBuffer) + prevHead;
        batchPtrs->dumpDst = (qword)thisBatchDump;
        batchPtrs->globalDumpBuffer = (qword)inputDumpMainBuffer;
        batchPtrs->nonVertexDataStart = (qword)(thisBatchDump + AlignTo128(sizeof(InputBatch)) + vertexBufferSize);
        batchPtrs->totalSize = sizeOfThisBatch;

        global InputBatch* batchOp = (global InputBatch*) thisBatchDump;
        batchOp->header.opHeader.operationType = INPUT_DUMP_OP_BATCH;
        batchOp->header.opHeader.endOfData = sizeOfThisBatch;
        batchOp->vertexBufferDataSize = vertexBufferSize;
        batchOp->firstContainedOpOffset = AlignTo128(sizeof(InputBatch)) + vertexBufferSize;
        batchOp->batchId = *batchIdPtr;
    }
}

/// Sets the dst data space for output dump of this batch
///
/// @param outputDumpMainBuffer pointer to main dump buffer
/// @param batchPtrs batch pointers struct
/// @param batchIdPtr pointer to batch id
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel allocate_data_space_for_outputs(
    global DebugBufferHeader* outputDumpMainBuffer,
    global OutputBatchPtrs* batchPtrs,
    global qword* batchIdPtr)
{
    if (get_sub_group_local_id() == 0) 
    {
        uint sizeOfThisBatch = AlignTo128(sizeof(OutputBatch)) + batchPtrs->dataSize;

        if ((sizeOfThisBatch + sizeof(OutputBatch)) > ((outputDumpMainBuffer->totalSize - outputDumpMainBuffer->headStart) / 2)) 
        {
            outputDumpMainBuffer->overflow = 1;
            batchPtrs->dumpDst = 0;
            batchPtrs->dataStart = 0;
            batchPtrs->totalSize = 0;
            return;
        }

        dword prevHead = *((volatile global uint*)(&outputDumpMainBuffer->gpuHead));
        dword newHead;
        bool circled;

        do
        {
            //mem_fence_gpu_invalidate();
            //prevHead = *((volatile global uint*)(&outputDumpMainBuffer->gpuHead));
            circled = false;
            newHead = prevHead + sizeOfThisBatch;
            dword bufferBegin = prevHead;
            if ((newHead + sizeof(OutputBatch)) > outputDumpMainBuffer->totalSize)
            {
                circled = true;
                newHead = outputDumpMainBuffer->headStart + sizeOfThisBatch;
                bufferBegin = outputDumpMainBuffer->headStart;
            }
            dword bufferEnd = newHead + sizeof(OutputBatch);

            uint tail;
            uint tail2 = 7;
            bool wait;
            do
            {
                wait = true;
                tail = load_uint_L1UC_L3UC(&outputDumpMainBuffer->tail, 0);

                // dead code, workaround so IGC won't move tail load out of loop
                if (tail > outputDumpMainBuffer->totalSize) 
                {
                   store_uint_L1UC_L3UC(&outputDumpMainBuffer->tail, 0, tail + tail2);
                   tail2 = tail;
                }

                if( prevHead >= tail )
                {
                    //colision example:
                    //  ----------T=======H------------
                    //  -------B=====E-----------------
                    //
                    if((bufferEnd < tail) || (bufferBegin >= prevHead))
                    {
                        wait = false;
                    }
                }
                else 
                {
                    //colision example:
                    //  ==========H-------T============
                    //  B==============E---------------
                    // caution: we will never have H circled completely so that H == T
                    if((bufferEnd < tail) && (bufferBegin >= prevHead)) 
                    {
                        wait = false;
                    }
                }
            } while (wait);
        } while (!atomic_compare_exchange_global(&outputDumpMainBuffer->gpuHead, &prevHead, newHead));

        if (circled)
        {
            global OutputBatch* endBufferOp = (global OutputBatch*)(((global char*)outputDumpMainBuffer) + prevHead);
            endBufferOp->header.opHeader.operationType = OUTPUT_DUMP_OP_END_BUFFER;
            prevHead = outputDumpMainBuffer->headStart;
        }

        global char* thisBatchDump = ((global char*)outputDumpMainBuffer) + prevHead;
        batchPtrs->dumpDst = (qword)thisBatchDump;
        batchPtrs->dataStart = (qword)(thisBatchDump + AlignTo128(sizeof(OutputBatch)));
        batchPtrs->totalSize = sizeOfThisBatch;

        global OutputBatch* batchOp = (global OutputBatch*) thisBatchDump;
        batchOp->header.opHeader.operationType = OUTPUT_DUMP_OP_BATCH;
        batchOp->header.opHeader.endOfData = sizeOfThisBatch;
        batchOp->firstContainedOpOffset = AlignTo128(sizeof(OutputBatch));
        batchOp->batchId = *batchIdPtr;
    }
}

/// Calculates sum of output sizes
///
/// @param pbi pointer to post build infos
/// @param destOffset offset in dest buffer
/// @param numOutputs number of outputs
/// @param batchPtrs batch pointers struct
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel calc_outputs_data_size(
    global PostbuildInfoSerializationDesc* pbi,
    global dword* destOffsets,
    qword numOutputs,
    global OutputBatchPtrs* batchPtrs)
{
    uint offset = 0;
    for (uint i = get_sub_group_local_id(); i < numOutputs + (MAX_HW_SIMD_WIDTH - 1); i += MAX_HW_SIMD_WIDTH)
    {
        uint size = 0;
        if (i < numOutputs)
        {
            size = AlignTo128(pbi[i].SerializedSizeInBytes);
            size += AlignTo128(sizeof(OutputData));
            destOffsets[i] = offset + sub_group_scan_exclusive_add(size);
        }
        offset += sub_group_reduce_add(size);
    }
    if (get_sub_group_local_id() == 0)
        batchPtrs->dataSize = offset;
}

/// Adds output data operation to batch
///
/// @param batchPtrs batch pointers struct
/// @param destOffset offset in dest buffer
/// @param src pointer to source bvh
/// @param pbi pointer to post build info
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel write_output_data_op(
    global OutputBatchPtrs* batchPtrs,
    global dword* destOffset,
    qword src,
    global PostbuildInfoSerializationDesc* pbi)
{
    if (batchPtrs->dataStart == 0)
        return;

    global OutputData* out = (global OutputData*)(batchPtrs->dataStart + *destOffset);
    out->header.operationType = OUTPUT_DUMP_OP_DATA;
    out->header.endOfData = AlignTo128(sizeof(OutputData)) + AlignTo128(pbi->SerializedSizeInBytes);
    out->srcBvhPtr = src;
}

/// Writes indices and transform or procedurals data
///
/// @param batchPtrs batch pointers struct
/// @param srcDesc description of source geometry
/// @param pVertexBufferOffsetInLinearisedUniqueVertexBuffers pointer to offset to vertices in vertex buffer
/// @param dstDescOffset offset to dest geo desc
/// @param dstDataOffset offset to dest geo data
/// @param numThreads number of threads
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) 
void kernel write_geo_data(
    global InputBatchPtrs* batchPtrs,
    global GRL_RAYTRACING_GEOMETRY_DESC* srcDesc,
    global uint* pVertexBufferOffsetInLinearisedUniqueVertexBuffers,
    global uint* pVertexBufferSize,
    qword dstDescOffset,
    qword dstDataOffset,
    dword numThreads)
{
    if (batchPtrs->dumpDst == 0) return;

    uint glob_id = get_group_id(0) * get_sub_group_size() + get_sub_group_local_id();

    GRL_RAYTRACING_GEOMETRY_DESC geoDescToStore = *srcDesc;

    global char* dstDataPtr = (global char*)(
        batchPtrs->nonVertexDataStart + dstDataOffset);

    global char* srcDataPtr;
    global char* dstTransform;
    uint bytesToCopy = 0;

    if (geoDescToStore.Type == GEOMETRY_TYPE_TRIANGLES)
    {
        uint sizeOfMatrix = 0;

        if (geoDescToStore.Desc.Triangles.pTransformBuffer)
        {
            sizeOfMatrix = AlignTo128(4 * 3 * sizeof(float));
            if (glob_id < 12)
            {
                global float* matrixSrc = (global float*)geoDescToStore.Desc.Triangles.pTransformBuffer;
                global float* matrixDst = (global float*)dstDataPtr;
                matrixDst[glob_id] = matrixSrc[glob_id];
                if (glob_id == 0) 
                {
                    geoDescToStore.Desc.Triangles.pTransformBuffer = ((qword)matrixDst) - batchPtrs->globalDumpBuffer;
                }
            }
        }
        
        dstDataPtr += sizeOfMatrix;
        srcDataPtr = (global char*)geoDescToStore.Desc.Triangles.pIndexBuffer;

        bytesToCopy = AlignTo128(geoDescToStore.Desc.Triangles.IndexFormat * geoDescToStore.Desc.Triangles.IndexCount);

        if (bytesToCopy && (glob_id == 0)) 
        {
            qword vertBuff = (qword)(GetVertexBuffersStart(batchPtrs) + *pVertexBufferOffsetInLinearisedUniqueVertexBuffers);
            // for this we remember offset relative to global debug buffer
            geoDescToStore.Desc.Triangles.pVertexBuffer = ((qword)vertBuff) - batchPtrs->globalDumpBuffer;
            geoDescToStore.Desc.Triangles.pIndexBuffer = ((qword)dstDataPtr) - batchPtrs->globalDumpBuffer;
            geoDescToStore.Desc.Triangles.VertexCount = *pVertexBufferSize / geoDescToStore.Desc.Triangles.VertexBufferByteStride;
        }
        else if (geoDescToStore.Desc.Triangles.IndexFormat == INDEX_FORMAT_NONE && geoDescToStore.Desc.Triangles.VertexCount > 0 && glob_id == 0)
        {
            if (geoDescToStore.Desc.Triangles.pVertexBuffer)
            {
                qword vertBuff = (qword)(GetVertexBuffersStart(batchPtrs) + *pVertexBufferOffsetInLinearisedUniqueVertexBuffers);
                // for this we remember offset relative to global debug buffer
                geoDescToStore.Desc.Triangles.pVertexBuffer = ((qword)vertBuff) - batchPtrs->globalDumpBuffer;
            }
        }
        else if (glob_id == 0)
        {
            geoDescToStore.Desc.Triangles.IndexCount = 0;
            geoDescToStore.Desc.Triangles.VertexCount = 0;
            geoDescToStore.Desc.Triangles.pVertexBuffer = 0;
            geoDescToStore.Desc.Triangles.pIndexBuffer = 0;
        }
    }
    else 
    {
        srcDataPtr  = (global char*)geoDescToStore.Desc.Procedural.pAABBs_GPUVA;
        bytesToCopy = AlignTo128(geoDescToStore.Desc.Procedural.AABBByteStride * geoDescToStore.Desc.Procedural.AABBCount);
        if (glob_id == 0) 
        {
            geoDescToStore.Desc.Procedural.pAABBs_GPUVA = ((qword)dstDataPtr) - batchPtrs->globalDumpBuffer;
        }
    }

    if (bytesToCopy) 
    {
        CopyMemory(dstDataPtr, srcDataPtr, bytesToCopy, numThreads);
    }

    if (glob_id == 0) 
    {
        global GRL_RAYTRACING_GEOMETRY_DESC* dstDescPtr = (global GRL_RAYTRACING_GEOMETRY_DESC*)(
            batchPtrs->nonVertexDataStart + dstDescOffset);
        *dstDescPtr = geoDescToStore;
    }
}

/// Adds build operation to batch
///
/// @param batchPtrs batch pointers struct
/// @param buildOpOffset offset in dst buffer
/// @param srcBvh address of src bvh (in case of update)
/// @param dstBvhAddr address of dest bvh buffer
/// @param offsetToEnd offset to end of this operation
/// @param flags build flags
/// @param numGeometries number of geometries in build
/// @param numInstances number of instances in build
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH)))
void kernel write_input_build_op(
    global InputBatchPtrs* batchPtrs,
    qword buildOpOffset,
    qword srcBvh,
    qword dstBvhAddr,
    dword offsetToEnd,
    dword flags,
    dword numGeometries, 
    dword numInstances,
    dword instArrayOfPtrs)
{
    uint glob_id = get_group_id(0) * get_sub_group_size() + get_sub_group_local_id();
    if (batchPtrs->dumpDst == 0 || glob_id != 0) return;
    
    global InputBuild* buildOp = (global InputBuild*)(
        batchPtrs->nonVertexDataStart + buildOpOffset);
    buildOp->header.operationType = srcBvh ? INPUT_DUMP_OP_UPDATE : INPUT_DUMP_OP_BUILD;
    buildOp->header.endOfData = offsetToEnd;
    buildOp->dstBvhPtr = dstBvhAddr;
    buildOp->srcBvhPtr = srcBvh;
    buildOp->flags = flags;
    buildOp->numGeos = numGeometries;
    buildOp->numInstances = numInstances;
    buildOp->instArrayOfPtrs = instArrayOfPtrs;
}

/// Copies instance description
///
/// @param batchPtrs batch pointers struct
/// @param instanceDescArr inst desc source
/// @param offset ptr to offset in dst buffer
/// @param numInstances number of instances to copy
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) void kernel
copy_instance_descriptors_array(
    global InputBatchPtrs* batchPtrs,
    global GRL_RAYTRACING_INSTANCE_DESC* instanceDescArr,
    qword offset,                               
    dword numInstances) 
{
    uint glob_id = get_group_id(0) * get_sub_group_size() + get_sub_group_local_id();
    if (batchPtrs->dumpDst == 0) return;

    global GRL_RAYTRACING_INSTANCE_DESC* dst = (global GRL_RAYTRACING_INSTANCE_DESC* )(
        batchPtrs->nonVertexDataStart + offset);

    if (glob_id < numInstances)
    {
        dst[glob_id] = instanceDescArr[glob_id];
    }
}

/// Copies instance description, array of pointers version
///
/// @param batchPtrs batch pointers struct
/// @param pInstanceDescPtrsArr inst desc source
/// @param offset ptr to offset in dst buffer
/// @param numInstances number of instances to copy
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) void kernel
copy_instance_descriptors_array_of_ptrs(
    global InputBatchPtrs* batchPtrs,
    global qword* pInstanceDescPtrsArr,
    qword offset,
    dword numInstances)
{
    uint glob_id = get_group_id(0) * get_sub_group_size() + get_sub_group_local_id();
    if (batchPtrs->dumpDst == 0) return;

    // save gpuva of instance descs for debug
    global qword* gpuvaDst = (global qword*)(batchPtrs->nonVertexDataStart + offset);

    global GRL_RAYTRACING_INSTANCE_DESC* dst = (global GRL_RAYTRACING_INSTANCE_DESC*)(
        batchPtrs->nonVertexDataStart + AlignTo128(numInstances * sizeof(qword)) + offset);
    global GRL_RAYTRACING_INSTANCE_DESC** instanceDescPtrsArr = (global GRL_RAYTRACING_INSTANCE_DESC **)pInstanceDescPtrsArr;

    if (glob_id < numInstances)
    {
        gpuvaDst[glob_id] = (qword)instanceDescPtrsArr[glob_id];
        dst[glob_id] = *(instanceDescPtrsArr[glob_id]);
    }
}

/// Adds copy operation to batch
///
/// @param batchPtrs batch pointers struct
/// @param offset ptr to offset in dst buffer
/// @param src copy source pointer
/// @param dst copy destination pointer
/// @param copyOpType copy type
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) 
void kernel insert_copy_op(
    global InputBatchPtrs* batchPtrs,
    qword offset,
    global void* src,
    global void* dst,
    uint copyOpType)
{
    uint glob_id = get_group_id(0) * get_sub_group_size() + get_sub_group_local_id();
    if (batchPtrs->dumpDst == 0 || glob_id != 0) return;

    global InputCopy* copyOp = (global InputCopy*)(batchPtrs->nonVertexDataStart + offset);

    copyOp->header.operationType = copyOpType;
    copyOp->header.endOfData = AlignTo128(sizeof(InputCopy));
    copyOp->srcBvhPtr = (qword)src;
    copyOp->dstBvhPtr = (qword)dst;
}

/// Copies vertex buffer
///
/// @param batchPtrs batch pointers struct
/// @param src input buffer
/// @param offset ptr to offset in dst buffer
/// @param size ptr to number of bytes to copy
__attribute__((reqd_work_group_size(MAX_HW_SIMD_WIDTH, 1, 1)))
__attribute__((intel_reqd_sub_group_size(MAX_HW_SIMD_WIDTH))) 
void kernel copy_vertex_data(
    global InputBatchPtrs* batchPtrs,
    global const char* src,
    global const uint* offset,
    global const uint* size) 
{
    if (batchPtrs->dumpDst == 0) return;

    global char *dst = (global char *)(GetVertexBuffersStart(batchPtrs) + *offset);
    uint numGroups = (*size >> 6) + 1;
    CopyMemory(dst, src, *size, numGroups);
}

/// Generate unique batch id
///
/// @param batchIds array of unique batch ids
/// @param index index of batch id to generate
__attribute__((reqd_work_group_size(1, 1, 1)))
void kernel generate_unique_batch_id(global unsigned long *batchIds, unsigned int index) {
    global unsigned int *counterPtrs = (global unsigned int *)batchIds;
    atomic_add(&counterPtrs[index * 2 + 1], 1);
    batchIds[index] |= (unsigned long)index;
}

/// Sets batch as ready to read and moves cpuHead forward, inputs case
///
/// @param batchPtrs batch pointers struct
/// @param dumpMainBuffer pointer to main dump buffer
__attribute__((reqd_work_group_size(1, 1, 1)))
void kernel finish_batch_dump_inputs(
    global InputBatchPtrs* batchPtrs,
    global DebugBufferHeader* dumpMainBuffer)
{
    if (batchPtrs->dumpDst == 0)
        return;

    global InputBatch* myBatchOp = (global InputBatch*)batchPtrs->dumpDst;

    dword myDstOffset = (batchPtrs->dumpDst - (qword)dumpMainBuffer);

    dword seven = 7;
    while (true)
    {
        dword currentHead = load_uint_L1UC_L3C(&dumpMainBuffer->cpuHead, 0);
        if (currentHead > dumpMainBuffer->totalSize) // dead code - workaround so IGC won't move currentHead load out of loop
        {
            store_uint_L1UC_L3UC(&dumpMainBuffer->cpuHead, 0, currentHead + seven);
            currentHead = seven;
        }

        if (currentHead == myDstOffset)
        {
            mem_fence_evict_to_memory();
            dumpMainBuffer->cpuHead = currentHead + myBatchOp->header.opHeader.endOfData;
            break;
        }
        else if (myDstOffset == dumpMainBuffer->headStart)
        {
            global InputBatch* curBatchOp = (global InputBatch*)(((global char*)dumpMainBuffer) + currentHead);
            if (curBatchOp->header.opHeader.operationType == INPUT_DUMP_OP_END_BUFFER)
            {
                mem_fence_evict_to_memory();
                dumpMainBuffer->cpuHead = dumpMainBuffer->headStart + myBatchOp->header.opHeader.endOfData;
                break;
            }
        }
    }
}

/// Sets batch as ready to read and moves cpuHead forward, outputs case
///
/// @param batchPtrs batch pointers struct
/// @param dumpMainBuffer pointer to main dump buffer
__attribute__((reqd_work_group_size(1, 1, 1)))
void kernel finish_batch_dump_outputs(
    global OutputBatchPtrs* batchPtrs,
    global DebugBufferHeader* dumpMainBuffer)
{
    if (batchPtrs->dumpDst == 0)
        return;

    global OutputBatch* myBatchOp = (global OutputBatch*)batchPtrs->dumpDst;

    dword myDstOffset = (batchPtrs->dumpDst - (qword)dumpMainBuffer);

    dword seven = 7;
    while (true)
    {
        dword currentHead = load_uint_L1UC_L3C(&dumpMainBuffer->cpuHead, 0);
        if (currentHead > dumpMainBuffer->totalSize) // dead code - workaround so IGC won't move currentHead load out of loop
        {
            store_uint_L1UC_L3UC(&dumpMainBuffer->cpuHead, 0, currentHead + seven);
            currentHead = seven;
        }

        if (currentHead == myDstOffset)
        {
            mem_fence_evict_to_memory();
            dumpMainBuffer->cpuHead = currentHead + myBatchOp->header.opHeader.endOfData;
            break;
        }
        else if (myDstOffset == dumpMainBuffer->headStart)
        {
            global OutputBatch* curBatchOp = (global OutputBatch*)(((global char*)dumpMainBuffer) + currentHead);
            if (curBatchOp->header.opHeader.operationType == OUTPUT_DUMP_OP_END_BUFFER)
            {
                mem_fence_evict_to_memory();
                dumpMainBuffer->cpuHead = dumpMainBuffer->headStart + myBatchOp->header.opHeader.endOfData;
                break;
            }
        }
    }
}
