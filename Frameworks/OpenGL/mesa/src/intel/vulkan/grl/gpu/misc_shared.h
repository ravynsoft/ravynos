//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

//
//   This file contains structure definitions shared by GRL OCL kernels and host code
//

#pragma once

#include "GRLGen12.h"

GRL_NAMESPACE_BEGIN(GRL)
GRL_NAMESPACE_BEGIN(RTAS)
GRL_NAMESPACE_BEGIN(MISC)

struct BatchedInitGlobalsData
{
    qword p_build_globals;
    qword p_bvh_buffer;
    dword numPrimitives;
    dword numGeometries;
    dword numInstances;
    dword instance_descs_start;
    dword geo_meta_data_start;
    dword node_data_start;
    dword leaf_data_start;
    dword procedural_data_start;
    dword back_pointer_start;
    dword sizeTotal;
    dword leafType;
    dword leafSize;
    dword fatleaf_table_start;
    dword innernode_table_start;
    dword quad_indices_data_start;
};

/// Header of debug buffer
///
/// Header is placed at the begining of debug buffer. 
/// After header there is circullar buffer space
typedef struct DebugBufferHeader
{
    /// Offset to begin of buffer (after header)
    dword headStart;
    /// Offset to free memory in buffer (used by gpu)
    dword gpuHead;
    /// Offset to end of data in buffer that is ready to read (read on cpu, set on gpu, might be behind gpuHeader)
    dword cpuHead;
    /// Flag for buffer overflow
    dword overflow;
    /// Total size of buffer
    dword totalSize;
    /// Padding needed because otherwise GPU overrides tail with cacheline flush
    dword pad[11];
    /// Offset to begin of data in buffer
    dword tail;
} DebugBufferHeader;

enum InputDumpOperationType
{
    INPUT_DUMP_OP_NOP,
    INPUT_DUMP_OP_BATCH,
    INPUT_DUMP_OP_BUILD,
    INPUT_DUMP_OP_UPDATE,
    INPUT_DUMP_OP_CLONE,
    INPUT_DUMP_OP_COMPACT,
    INPUT_DUMP_OP_SERIALIZE,
    INPUT_DUMP_OP_DESERIALIZE,
    INPUT_DUMP_OP_END_BUFFER
};

// each operation starts with the same header structure and looks like this

//  some defined struct { <-----------------start
//     OpHeader 
//     .... struct type specific data
//  }
//  ... auxilary data of variable len
//  <-------------------------------------- end - indicated by endOfData
typedef struct OpHeader
{
    dword operationType;
    dword endOfData; // offset to end of this primitive
} OpHeader;

// header for batch operations
typedef struct BatchOpHeader
{
    OpHeader opHeader;
} BatchOpHeader;

// interpretation for operationType INPUT_DUMP_OP_BATCH
typedef struct InputBatch
{
    BatchOpHeader header;
    qword batchId;
    dword vertexBufferDataSize;
    dword firstContainedOpOffset;
    
    // layout of batch is as below, each line is 128B aligned:

    // 
    //  InputBatch <-------------------------------- start
    //       optional: batchVertexData
    //  InputBuildDesc/InputCopy <------------------ start + firstContainedOpOffset
    //       optional: extra data of above token
    //  InputBuildDesc/InputCopy
    //       optional: extra data of above token
    //  ...
    //  InputBuildDesc/InputCopy
    //       optional: extra data of above token
    //  <-------------------------------------------- end    = start + endOfData
} InputBatch;

// for operationType:
//   INPUT_DUMP_OP_BUILD,
//   INPUT_DUMP_OP_UPDATE,
// followed by auxilary data of variable len
typedef struct InputBuild
{
    OpHeader header;
    qword srcBvhPtr;
    qword dstBvhPtr;
    dword flags;
    dword numGeos;
    dword numInstances;
    dword instArrayOfPtrs;
} InputBuild;

// for operationType:
//   INPUT_DUMP_OP_CLONE,
//   INPUT_DUMP_OP_COMPACT,
//   INPUT_DUMP_OP_SERIALIZE,
// 
//   Not for INPUT_DUMP_OP_DESERIALIZE!
typedef struct InputCopy
{
    OpHeader header;
    qword srcBvhPtr;
    qword dstBvhPtr;
} InputCopy;

// for INPUT_DUMP_OP_DESERIALIZE
// decode for debug tools follows this format
typedef struct InputDeserialize
{
    OpHeader header;
    qword dstBvhPtr;
} InputDeserialize;

typedef struct InputBatchPtrs
{
    qword dumpDst;
    qword globalDumpBuffer;
    qword nonVertexDataStart;
    dword vertexBuffersSize;
    dword totalSize;
} InputBatchPtrs;

enum OutputDumpOperationType
{
    OUTPUT_DUMP_OP_NOP,
    OUTPUT_DUMP_OP_BATCH,
    OUTPUT_DUMP_OP_DATA,
    OUTPUT_DUMP_OP_END_BUFFER
};

// interpretation for operationType OUTPUT_DUMP_OP_BATCH
typedef struct OutputBatch {
    BatchOpHeader header;
    qword batchId;
    dword firstContainedOpOffset;
} OutputBatch;

// interpretation for operationType OUTPUT_DUMP_OP_DATA
typedef struct OutputData
{
    OpHeader header;
    qword srcBvhPtr;
} OutputData;

typedef struct OutputBatchPtrs 
{
    qword dumpDst;
    qword dataStart;
    dword dataSize;
    dword totalSize;
} OutputBatchPtrs;

GRL_NAMESPACE_END(MISC)
GRL_NAMESPACE_END(RTAS)
GRL_NAMESPACE_END(GRL)
