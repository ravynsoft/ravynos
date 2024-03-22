//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#include "shared.h"

/// Write cache line to global memory
/// Assumes subgroup_size is 16
///
/// @param dst 64 bytes aligned output pointer
/// @param val value to write
GRL_INLINE void CacheLineSubgroupWrite(global char* dst, uint val)
{
    global uint* addrAligned = (global uint*)(global uint16*)dst;
    intel_sub_group_block_write(addrAligned, val);
}

/// Read cache line from global memory
/// Assumes subgroup_size is 16
///
/// @param src 64 bytes aligned input pointer
/// @return uint read from memory
GRL_INLINE uint CacheLineSubgroupRead(const global char* src)
{
    const global uint* addrAligned = (const global uint*)(global uint16*)src;
    return intel_sub_group_block_read(addrAligned);
}

/// Copy cache line
/// Assumes subgroup_size is 16
///
/// @param dst 64 bytes aligned output pointer
/// @param src input pointer
GRL_INLINE void CopyCacheLine(global char* dst, const global char* src)
{
    global const uint* usrc = (global const uint*) (src);

    uint data = intel_sub_group_block_read(usrc);
    CacheLineSubgroupWrite(dst, data);
}

/// Fast memory copy
/// 
/// @param dst output pointer
/// @param src input pointer
/// @param size number of bytes to copy
/// @param numGroups number of groups that execute this function
GRL_INLINE void CopyMemory(global char* dst, const global char* src, uint size, uint numGroups)
{
    const uint CACHELINE_SIZE = 64;

    uint globalID = get_local_size(0) * get_group_id(0) + get_local_id(0);
    
    // this part copies cacheline per physical thread one write. starting from dst aligned up to cacheline.
    // it copies laso reminder
    {
        uint alignAdd = ((uint)(uint64_t)dst) & (CACHELINE_SIZE - 1);
        alignAdd = (CACHELINE_SIZE - alignAdd) & (CACHELINE_SIZE - 1);

        if (size > alignAdd)
        {
            uint alignedBytesCount = size - alignAdd;
            uint alignedDWsCount = alignedBytesCount >> 2;
            global uint* dstAlignedPart = (global uint*)(dst + alignAdd);
            global uint* srcAlignedPart = (global uint*)(src + alignAdd);

            for (uint id = globalID; id < alignedDWsCount; id += get_local_size(0) * numGroups)
            {
                dstAlignedPart[id] = srcAlignedPart[id];
            }

            if (globalID < alignedBytesCount - (alignedDWsCount << 2))
            {
                global uint8_t* dstByteRem = (global uint8_t*)(dstAlignedPart + alignedDWsCount);
                global uint8_t* srcByteRem = (global uint8_t*)(srcAlignedPart + alignedDWsCount);
                dstByteRem[globalID] = srcByteRem[globalID];
            }
        }
    }
    
    // copy to dst below aligned up to chacheline
    {
        uint misalignmentBytesSize = (4 - (((uint)dst) & /*bytes in DW*/3)) & 3;
        if (misalignmentBytesSize)
        {
            if (globalID < misalignmentBytesSize)
            {
                dst[globalID] = src[globalID];
            }
            dst += misalignmentBytesSize;
            src += misalignmentBytesSize;
        }

        uint misalignmentDWSize = (CACHELINE_SIZE - (((uint)dst) & (CACHELINE_SIZE - 1))) & (CACHELINE_SIZE - 1);
        if (misalignmentDWSize)
        {
            if (globalID < (misalignmentDWSize >> 2))
            {
                ((global uint*)dst)[globalID] = ((global uint*)src)[globalID];
            }
        }
    }
}

#define CACHELINE_SIZE 64
#define CACHELINE_PER_BLOCK 4
#define BLOCK_SIZE 256 // = CACHELINE_SIZE * CACHELINE_PER_BLOCK;

GRL_INLINE
global const char *getInstanceDataToCopy(global const char *array, global const uint64_t *arrayOfPtrs, const uint byteOffset)
{
    if (array != NULL)
    {
        return array + byteOffset;
    }
    else
    {
        return (global char *)arrayOfPtrs[byteOffset >> 6];
    }
}

// assummed:
// dst is always 64 bytes alligned
// size is always multiply of 64 bytes (size of InstanceDesc is always 64 bytes)
GRL_INLINE
void copyInstances(global char *dst, global const char *array, global const uint64_t *arrayOfPtrs, const uint64_t size, const uint numGroups)
{
    uint taskId = get_group_id(0);

    uint blockedSize = (size) & (~(BLOCK_SIZE - 1));

    uint cachelinedTailOffset = blockedSize;
    uint cachelinedTailSize = (size - cachelinedTailOffset) & (~(CACHELINE_SIZE - 1));

    uint tailCacheLines = cachelinedTailSize >> 6; // divide by CACHELINE_SIZE
    uint reversedTaskId = (uint)(-(((int)taskId) - ((int)numGroups - 1)));
    if (reversedTaskId < tailCacheLines)
    {
        uint byteOffset = cachelinedTailOffset + (reversedTaskId * CACHELINE_SIZE);
        global const char *src = getInstanceDataToCopy(array, arrayOfPtrs, byteOffset);
        CopyCacheLine(dst + byteOffset, src);
    }

    uint numBlocks = blockedSize >> 8;
    while (taskId < numBlocks)
    {
        uint byteOffset = (taskId * BLOCK_SIZE);

        for (uint cl = 0; cl < CACHELINE_PER_BLOCK; cl++)
        {
            global const char *src = getInstanceDataToCopy(array, arrayOfPtrs, byteOffset);
            CopyCacheLine(dst + byteOffset, src);
            byteOffset += CACHELINE_SIZE;
        }

        taskId += numGroups;
    }
}