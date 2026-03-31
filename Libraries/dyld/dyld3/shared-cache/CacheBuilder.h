/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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


#ifndef CacheBuilder_h
#define CacheBuilder_h

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "ClosureFileSystem.h"
#include "DyldSharedCache.h"
#include "Diagnostics.h"
#include "MachOAnalyzer.h"



template <typename P> class LinkeditOptimizer;


class CacheBuilder {
public:
    CacheBuilder(const DyldSharedCache::CreateOptions& options, const dyld3::closure::FileSystem& fileSystem);

    struct InputFile {
        enum State {
            Unset,
            MustBeIncluded,
            MustBeIncludedForDependent,
            MustBeExcludedIfUnused
        };
        InputFile(const char* path, State state) : path(path), state(state) { }
        const char*     path;
        State           state = Unset;
        Diagnostics     diag;

        bool mustBeIncluded() const {
            return (state == MustBeIncluded) || (state == MustBeIncludedForDependent);
        }
    };

    // Contains a MachO which has been loaded from the file system and may potentially need to be unloaded later.
    struct LoadedMachO {
        DyldSharedCache::MappedMachO    mappedFile;
        dyld3::closure::LoadedFileInfo  loadedFileInfo;
        InputFile*                      inputFile;
    };

    std::string                                 errorMessage();

    struct SegmentMappingInfo {
        const void*     srcSegment;
        const char*     segName;
        void*           dstSegment;
        uint64_t        dstCacheUnslidAddress;
        uint32_t        dstCacheFileOffset;
        uint32_t        dstCacheSegmentSize;
        uint32_t        dstCacheFileSize;
        uint32_t        copySegmentSize;
        uint32_t        srcSegmentIndex;
    };

    struct DylibTextCoalescer {

        typedef std::map<uint32_t, uint32_t> DylibSectionOffsetToCacheSectionOffset;

        DylibSectionOffsetToCacheSectionOffset objcClassNames;
        DylibSectionOffsetToCacheSectionOffset objcMethNames;
        DylibSectionOffsetToCacheSectionOffset objcMethTypes;

        bool sectionWasCoalesced(std::string_view sectionName) const;
        DylibSectionOffsetToCacheSectionOffset& getSectionCoalescer(std::string_view sectionName);
        const DylibSectionOffsetToCacheSectionOffset& getSectionCoalescer(std::string_view sectionName) const;
    };

    struct CacheCoalescedText {
        static const char* SupportedSections[3];
        struct StringSection {
            // Map from class name strings to offsets in to the class names buffer
            std::map<std::string_view, uint32_t> stringsToOffsets;
            uint8_t*                             bufferAddr       = nullptr;
            uint32_t                             bufferSize       = 0;
            uint64_t                             bufferVMAddr     = 0;

            // Note this is for debugging only
            uint64_t                             savedSpace       = 0;
        };

        StringSection objcClassNames;
        StringSection objcMethNames;
        StringSection objcMethTypes;

        void parseCoalescableText(const dyld3::MachOAnalyzer* ma,
                                   DylibTextCoalescer& textCoalescer);
        void clear();

        StringSection& getSectionData(std::string_view sectionName);
        const StringSection& getSectionData(std::string_view sectionName) const;
    };

    class ASLR_Tracker
    {
    public:
                ~ASLR_Tracker();

        void        setDataRegion(const void* rwRegionStart, size_t rwRegionSize);
        void        add(void* p);
        void        setHigh8(void* p, uint8_t high8);
        void        setAuthData(void* p, uint16_t diversity, bool hasAddrDiv, uint8_t key);
        void        setRebaseTarget32(void*p, uint32_t targetVMAddr);
        void        setRebaseTarget64(void*p, uint64_t targetVMAddr);
        void        remove(void* p);
        bool        has(void* p);
        const bool* bitmap()        { return _bitmap; }
        unsigned    dataPageCount() { return _pageCount; }
        void        disable()       { _enabled = false; };
        bool        hasHigh8(void* p, uint8_t* highByte);
        bool        hasAuthData(void* p, uint16_t* diversity, bool* hasAddrDiv, uint8_t* key);
        bool        hasRebaseTarget32(void* p, uint32_t* vmAddr);
        bool        hasRebaseTarget64(void* p, uint64_t* vmAddr);

    private:

        uint8_t*     _regionStart    = nullptr;
        uint8_t*     _regionEnd      = nullptr;
        bool*        _bitmap         = nullptr;
        unsigned     _pageCount      = 0;
        unsigned     _pageSize       = 4096;
        bool         _enabled        = true;

        struct AuthData {
            uint16_t    diversity;
            bool        addrDiv;
            uint8_t     key;
        };
        std::unordered_map<void*, uint8_t>  _high8Map;
        std::unordered_map<void*, AuthData> _authDataMap;
        std::unordered_map<void*, uint32_t> _rebaseTarget32;
        std::unordered_map<void*, uint64_t> _rebaseTarget64;
    };

    typedef std::map<uint64_t, std::set<void*>> LOH_Tracker;

    static const uint64_t kRebaseTargetInSideTableArm64e    = 0x7FFFFFFFFFFULL;
    static const uint64_t kRebaseTargetInSideTableArm64     =   0xFFFFFFFFFULL;
    static const uint64_t kRebaseTargetInSideTableGeneric32 =     0x3FFFFFFULL;


    struct Region
    {
        uint8_t*    buffer                 = nullptr;
        uint64_t    bufferSize             = 0;
        uint64_t    sizeInUse              = 0;
        uint64_t    unslidLoadAddress      = 0;
        uint64_t    cacheFileOffset        = 0;
    };

protected:
    template <typename P>
    friend class LinkeditOptimizer;

    struct UnmappedRegion
    {
        uint8_t*    buffer                 = nullptr;
        uint64_t    bufferSize             = 0;
        uint64_t    sizeInUse              = 0;
    };

    struct DylibInfo
    {
        const LoadedMachO*              input;
        std::string                     runtimePath;
        std::vector<SegmentMappingInfo> cacheLocation;
        DylibTextCoalescer              textCoalescer;
    };

    void        copyRawSegments();
    void        adjustAllImagesForNewSegmentLocations();

    // implemented in AdjustDylibSegemnts.cpp
    void        adjustDylibSegments(const DylibInfo& dylib, Diagnostics& diag) const;

    // implemented in OptimizerLinkedit.cpp
    void        optimizeLinkedit();

    const DyldSharedCache::CreateOptions&       _options;
    const dyld3::closure::FileSystem&           _fileSystem;
    Region                                      _readExecuteRegion;
    Region                                      _readWriteRegion;
    Region                                      _readOnlyRegion;
    UnmappedRegion                              _localSymbolsRegion;
    vm_address_t                                _fullAllocatedBuffer;
    uint64_t                                    _nonLinkEditReadOnlySize;
    Diagnostics                                 _diagnostics;
    uint64_t                                    _allocatedBufferSize;
    std::vector<DylibInfo>                      _sortedDylibs;
    CacheCoalescedText                          _coalescedText;
    uint32_t                                    _sharedStringsPoolVmOffset  = 0;
    bool                                        _is64                       = false;
    // Note this is mutable as the only parallel writes to it are done atomically to the bitmap
    mutable ASLR_Tracker                        _aslrTracker;
    mutable LOH_Tracker                         _lohTracker;
};




inline uint64_t align(uint64_t addr, uint8_t p2)
{
    uint64_t mask = (1 << p2);
    return (addr + mask - 1) & (-mask);
}



#endif /* CacheBuilder_h */
