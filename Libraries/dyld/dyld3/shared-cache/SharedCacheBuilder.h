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


#ifndef SharedCacheBuilder_h
#define SharedCacheBuilder_h

#include "CacheBuilder.h"
#include "DyldSharedCache.h"
#include "ClosureFileSystem.h"

class SharedCacheBuilder : public CacheBuilder {
public:
    SharedCacheBuilder(const DyldSharedCache::CreateOptions& options, const dyld3::closure::FileSystem& fileSystem);

    void                                        build(std::vector<InputFile>& inputFiles,
                                                      std::vector<DyldSharedCache::FileAlias>& aliases);
    void                                        build(const std::vector<LoadedMachO>& dylibs,
                                                      const std::vector<LoadedMachO>& otherOsDylibsInput,
                                                      const std::vector<LoadedMachO>& osExecutables,
                                                      std::vector<DyldSharedCache::FileAlias>& aliases);
    void                                        build(const std::vector<DyldSharedCache::MappedMachO>&  dylibsToCache,
                                                      const std::vector<DyldSharedCache::MappedMachO>&  otherOsDylibs,
                                                      const std::vector<DyldSharedCache::MappedMachO>&  osExecutables,
                                                      std::vector<DyldSharedCache::FileAlias>& aliases);

    void                                        writeFile(const std::string& path);
    void                                        writeBuffer(uint8_t*& buffer, uint64_t& size);
    void                                        writeMapFile(const std::string& path);
    std::string                                 getMapFileBuffer(const std::string& cacheDisposition) const;
    void                                        deleteBuffer();
    const std::set<std::string>                 warnings();
    const std::set<const dyld3::MachOAnalyzer*> evictions();
    const bool                                  agileSignature();
    const std::string                           cdHashFirst();
    const std::string                           cdHashSecond();
    const std::string                           uuid() const;

    void                                        forEachCacheDylib(void (^callback)(const std::string& path));

private:

    void        writeSlideInfoV1();

    template <typename P> void writeSlideInfoV2(const bool bitmap[], unsigned dataPageCount);
    template <typename P> bool makeRebaseChainV2(uint8_t* pageContent, uint16_t lastLocationOffset, uint16_t newOffset, const struct dyld_cache_slide_info2* info);
    template <typename P> void addPageStartsV2(uint8_t* pageContent, const bool bitmap[], const struct dyld_cache_slide_info2* info,
                                             std::vector<uint16_t>& pageStarts, std::vector<uint16_t>& pageExtras);

    void        writeSlideInfoV3(const bool bitmap[], unsigned dataPageCoun);
    uint16_t    pageStartV3(uint8_t* pageContent, uint32_t pageSize, const bool bitmap[]);
    void        setPointerContentV3(dyld3::MachOLoaded::ChainedFixupPointerOnDisk* loc, uint64_t targetVMAddr, size_t next);

    template <typename P> void writeSlideInfoV4(const bool bitmap[], unsigned dataPageCount);
    template <typename P> bool makeRebaseChainV4(uint8_t* pageContent, uint16_t lastLocationOffset, uint16_t newOffset, const struct dyld_cache_slide_info4* info);
    template <typename P> void addPageStartsV4(uint8_t* pageContent, const bool bitmap[], const struct dyld_cache_slide_info4* info,
                                             std::vector<uint16_t>& pageStarts, std::vector<uint16_t>& pageExtras);

    struct ArchLayout
    {
        uint64_t    sharedMemoryStart;
        uint64_t    sharedMemorySize;
        uint64_t    textAndDataMaxSize;
        uint64_t    sharedRegionPadding;
        uint64_t    pointerDeltaMask;
        const char* archName;
        uint16_t    csPageSize;
        uint8_t     sharedRegionAlignP2;
        uint8_t     slideInfoBytesPerPage;
        bool        sharedRegionsAreDiscontiguous;
        bool        is64;
        bool        useValueAdd;
    };

    static const ArchLayout  _s_archLayout[];
    static const char* const _s_neverStubEliminateDylibs[];
    static const char* const _s_neverStubEliminateSymbols[];

    void        makeSortedDylibs(const std::vector<LoadedMachO>& dylibs, const std::unordered_map<std::string, unsigned> sortOrder);
    void        processSelectorStrings(const std::vector<LoadedMachO>& executables);
    void        parseCoalescableSegments();
    void        assignSegmentAddresses();

    uint64_t    cacheOverflowAmount();
    size_t      evictLeafDylibs(uint64_t reductionTarget, std::vector<const LoadedMachO*>& overflowDylibs);

    void        fipsSign();
    void        codeSign();
    uint64_t    pathHash(const char* path);
    void        writeCacheHeader();
    void        findDylibAndSegment(const void* contentPtr, std::string& dylibName, std::string& segName);
    void        addImageArray();
    void        buildImageArray(std::vector<DyldSharedCache::FileAlias>& aliases);
    void        addOtherImageArray(const std::vector<LoadedMachO>&, std::vector<const LoadedMachO*>& overflowDylibs);
    void        addClosures(const std::vector<LoadedMachO>&);
    void        markPaddingInaccessible();

    bool        writeCache(void (^cacheSizeCallback)(uint64_t size), bool (^copyCallback)(const uint8_t* src, uint64_t size, uint64_t dstOffset));

    // implemented in OptimizerObjC.cpp
    void        optimizeObjC();
    uint32_t    computeReadOnlyObjC(uint32_t selRefCount, uint32_t classDefCount, uint32_t protocolDefCount);
    uint32_t    computeReadWriteObjC(uint32_t imageCount, uint32_t protocolDefCount);

    // implemented in OptimizerBranches.cpp
    void        optimizeAwayStubs();

    typedef std::unordered_map<std::string, const dyld3::MachOAnalyzer*> InstallNameToMA;

    typedef uint64_t                                                CacheOffset;

    UnmappedRegion                              _codeSignatureRegion;
    std::set<const dyld3::MachOAnalyzer*>       _evictions;
    const ArchLayout*                           _archLayout                             = nullptr;
    uint32_t                                    _aliasCount                             = 0;
    uint64_t                                    _slideInfoFileOffset                    = 0;
    uint64_t                                    _slideInfoBufferSizeAllocated           = 0;
    uint8_t*                                    _objcReadOnlyBuffer                     = nullptr;
    uint64_t                                    _objcReadOnlyBufferSizeUsed             = 0;
    uint64_t                                    _objcReadOnlyBufferSizeAllocated        = 0;
    uint8_t*                                    _objcReadWriteBuffer                    = nullptr;
    uint64_t                                    _objcReadWriteBufferSizeAllocated       = 0;
    uint64_t                                    _selectorStringsFromExecutables         = 0;
    InstallNameToMA                             _installNameToCacheDylib;
    std::unordered_map<std::string, uint32_t>   _dataDirtySegsOrder;
    std::map<void*, std::string>                _missingWeakImports;
    const dyld3::closure::ImageArray*           _imageArray                             = nullptr;
    uint8_t                                     _cdHashFirst[20];
    uint8_t                                     _cdHashSecond[20];
    bool                                        _someDylibsUsedChainedFixups            = false;
    std::unordered_map<const dyld3::MachOLoaded*, std::set<CacheOffset>>        _dylibToItsExports;
    std::set<std::pair<const dyld3::MachOLoaded*, CacheOffset>>                 _dylibWeakExports;
    std::unordered_map<CacheOffset, std::vector<dyld_cache_patchable_location>> _exportsToUses;
    std::unordered_map<CacheOffset, std::string>                                _exportsToName;
};



#endif /* SharedCacheBuilder_h */
