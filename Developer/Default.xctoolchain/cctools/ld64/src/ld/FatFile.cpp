/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-*
 *
 * Copyright (c) 2021 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */


#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <string.h>

#include "FatFile.h"
#include "MachOFileAbstraction.hpp"


const FatFile* FatFile::isFatFile(const void* fileStart)
{
    const FatFile* fileStartAsFat = (FatFile*)fileStart;
    if ( (fileStartAsFat->magic == OSSwapBigToHostInt32(FAT_MAGIC)) || (fileStartAsFat->magic == OSSwapBigToHostInt32(FAT_MAGIC_64)) )
        return fileStartAsFat;
    else
        return nullptr;
}

const char* FatFile::isInvalidSlice(uint64_t fileLen, const uint8_t* sliceStart, uint64_t sliceLen, uint32_t sliceCpuType, uint32_t sliceCpuSubType) const
{
    if ( strncmp((char*)sliceStart, "!<arch>", 7) == 0 ) {
        // slice is static library
    }
    else if ( *((uint32_t*)sliceStart) == 0x0b17c0de ) {
        // slice is bitcode
    }
    else {
        const mach_header* mh = (mach_header*)sliceStart;
        if ( (mh->magic != MH_MAGIC) && (mh->magic != MH_MAGIC_64) ) {
            if ( (mh->magic == MH_CIGAM) || (mh->magic == MH_CIGAM_64) ) {
                // can't link any big endian arches, so no need for subtype checks
                return nullptr;
            }
            return "slice content is not mach-o or a static library";
        }
         if ( mh->cputype != (cpu_type_t)sliceCpuType )
            return "cpu type in slice does not match fat header";
        else if ( (mh->cpusubtype & ~CPU_SUBTYPE_MASK) != (sliceCpuSubType & ~CPU_SUBTYPE_MASK) )
            return "cpu subtype in slice does not match fat header";
    }
    return nullptr;
}

const char* FatFile::isInvalid(uint64_t fileLen) const
{
    if ( this->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
        const uint64_t maxArchs = ((4096 - sizeof(fat_header)) / sizeof(fat_arch));
        const uint32_t numArchs = OSSwapBigToHostInt32(nfat_arch);
        if ( numArchs > maxArchs ) {
            return "too many slices";
        }
        const fat_arch* const archs = (fat_arch*)(((char*)this)+sizeof(fat_header));
        for (uint32_t i=0; i < numArchs; ++i) {
            uint32_t cpuType    = OSSwapBigToHostInt32(archs[i].cputype);
            uint32_t cpuSubType = OSSwapBigToHostInt32(archs[i].cpusubtype);
            uint32_t offset     = OSSwapBigToHostInt32(archs[i].offset);
            uint32_t len        = OSSwapBigToHostInt32(archs[i].size);
            uint64_t sliceEnd   = offset + len;
            if ( sliceEnd > fileLen )
                return "slice extends beyond end of file";
            if ( const char* errMsg = this->isInvalidSlice(fileLen, ((uint8_t*)this)+offset, len, cpuType, cpuSubType) )
                return errMsg;
        }

        // Look for one more (hidden) slice
        if ( numArchs != maxArchs ) {
            uint32_t cpuType    = OSSwapBigToHostInt32(archs[numArchs].cputype);
            uint32_t cpuSubType = OSSwapBigToHostInt32(archs[numArchs].cpusubtype);
            uint32_t offset     = OSSwapBigToHostInt32(archs[numArchs].offset);
            uint32_t len        = OSSwapBigToHostInt32(archs[numArchs].size);
            if ((cpuType == CPU_TYPE_ARM64) && ((cpuSubType == CPU_SUBTYPE_ARM64_ALL || cpuSubType == CPU_SUBTYPE_ARM64_V8))) {
                if ( const char* errMsg = this->isInvalidSlice(fileLen, ((uint8_t*)this)+offset, len, cpuType, cpuSubType) )
                    return errMsg;
            }
        }
    }
    else if ( this->magic == OSSwapBigToHostInt32(FAT_MAGIC_64) ) {
        if ( OSSwapBigToHostInt32(nfat_arch) > ((4096 - sizeof(fat_header)) / sizeof(fat_arch_64)) ) {
            return "too many slices";
        }
        const fat_arch_64* const archs = (fat_arch_64*)(((char*)this)+sizeof(fat_header));
        for (uint32_t i=0; i < OSSwapBigToHostInt32(nfat_arch); ++i) {
            uint32_t cpuType    = OSSwapBigToHostInt32(archs[i].cputype);
            uint32_t cpuSubType = OSSwapBigToHostInt32(archs[i].cpusubtype);
            uint64_t offset     = OSSwapBigToHostInt64(archs[i].offset);
            uint64_t len        = OSSwapBigToHostInt64(archs[i].size);
            uint64_t sliceEnd   = offset + len;
            if ( sliceEnd > fileLen )
                return "slice extends beyond end of file";
            if ( const char* errMsg = this->isInvalidSlice(fileLen, ((uint8_t*)this)+offset, len, cpuType, cpuSubType) )
                return errMsg;
        }
    }
    else {
        return "not a fat file";
    }
    return nullptr;
}


void FatFile::forEachSlice(void (^callback)(uint32_t sliceCpuType, uint32_t sliceCpuSubType, const void* sliceStart, uint64_t sliceSize, bool& stop)) const
{
    if ( this->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
        bool stop = false;
        const uint32_t numArchs = OSSwapBigToHostInt32(this->nfat_arch);
        const fat_arch* const archs = (fat_arch*)(((char*)this)+sizeof(fat_header));
        for (uint32_t i=0; i < numArchs; ++i) {
            uint32_t sliceCpuType    = OSSwapBigToHostInt32(archs[i].cputype);
            uint32_t sliceCpuSubType = OSSwapBigToHostInt32(archs[i].cpusubtype);
            uint32_t sliceOffset     = OSSwapBigToHostInt32(archs[i].offset);
            uint32_t sliceLength     = OSSwapBigToHostInt32(archs[i].size);
            callback(sliceCpuType, sliceCpuSubType, (uint8_t*)this+sliceOffset, sliceLength, stop);
            if ( stop )
                return;
        }

        // Look for one more (hidden) slice
        uint32_t sliceCpuType    = OSSwapBigToHostInt32(archs[numArchs].cputype);
        uint32_t sliceCpuSubType = OSSwapBigToHostInt32(archs[numArchs].cpusubtype);
        uint32_t sliceOffset     = OSSwapBigToHostInt32(archs[numArchs].offset);
        uint32_t sliceLength     = OSSwapBigToHostInt32(archs[numArchs].size);
        if ((sliceCpuType == CPU_TYPE_ARM64) && ((sliceCpuSubType == CPU_SUBTYPE_ARM64_ALL || sliceCpuSubType == CPU_SUBTYPE_ARM64_V8))) {
            callback(sliceCpuType, sliceCpuSubType, (uint8_t*)this+sliceOffset, sliceLength, stop);
        }
    }
    else if ( this->magic == OSSwapBigToHostInt32(FAT_MAGIC_64) ) {
        bool stop = false;
        const fat_arch_64* const archs = (fat_arch_64*)(((char*)this)+sizeof(fat_header));
        const uint32_t numArchs = OSSwapBigToHostInt32(this->nfat_arch);
        for (uint32_t i=0; i < numArchs; ++i) {
            uint32_t sliceCpuType    = OSSwapBigToHostInt32(archs[i].cputype);
            uint32_t sliceCpuSubType = OSSwapBigToHostInt32(archs[i].cpusubtype);
            uint64_t sliceOffset     = OSSwapBigToHostInt64(archs[i].offset);
            uint64_t sliceLength     = OSSwapBigToHostInt64(archs[i].size);
            callback(sliceCpuType, sliceCpuSubType, (uint8_t*)this+sliceOffset, sliceLength, stop);
            if ( stop )
                return;
        }
    }
}

static const char* archName(cpu_type_t cpuType, cpu_subtype_t cpuSubType)
{
    for (const ArchInfo* t=archInfoArray; t->archName != NULL; ++t) {
        if ( (cpuType == t->cpuType) && (cpuSubType == t->cpuSubType) )
            return t->archName;
    }
    return "unknown";
}

const char* FatFile::archNames(char strBuf[256]) const
{
    strBuf[0] = '\0';
    __block bool  needComma = false;
    this->forEachSlice(^(uint32_t sliceCpuType, uint32_t sliceCpuSubType, const void* sliceStart, uint64_t sliceSize, bool& stop) {
        if ( needComma )
            strlcat(strBuf, ",", 256);
        strlcat(strBuf, archName(sliceCpuType, (sliceCpuSubType  & ~CPU_SUBTYPE_MASK)), 256);
        needComma = true;
    });
    return strBuf;
}

