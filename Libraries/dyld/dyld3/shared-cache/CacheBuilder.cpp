/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#include <assert.h>

#include "MachOFileAbstraction.hpp"
#include "DyldSharedCache.h"
#include "CacheBuilder.h"
#include "Diagnostics.h"


CacheBuilder::CacheBuilder(const DyldSharedCache::CreateOptions& options, const dyld3::closure::FileSystem& fileSystem)
    : _options(options)
    , _fileSystem(fileSystem)
    , _fullAllocatedBuffer(0)
    , _diagnostics(options.loggingPrefix, options.verbose)
    , _allocatedBufferSize(0)
{
}


std::string CacheBuilder::errorMessage()
{
    return _diagnostics.errorMessage();
}

void CacheBuilder::copyRawSegments()
{
    const bool log = false;
    dispatch_apply(_sortedDylibs.size(), DISPATCH_APPLY_AUTO, ^(size_t index) {
        const DylibInfo& dylib = _sortedDylibs[index];
        for (const SegmentMappingInfo& info : dylib.cacheLocation) {
            if (log) fprintf(stderr, "copy %s segment %s (0x%08X bytes) from %p to %p (logical addr 0x%llX) for %s\n",
                             _options.archs->name(), info.segName, info.copySegmentSize, info.srcSegment, info.dstSegment, info.dstCacheUnslidAddress, dylib.input->mappedFile.runtimePath.c_str());
            ::memcpy(info.dstSegment, info.srcSegment, info.copySegmentSize);
        }
    });

    // Copy the coalesced sections
    const uint64_t numCoalescedSections = sizeof(CacheCoalescedText::SupportedSections) / sizeof(*CacheCoalescedText::SupportedSections);
    dispatch_apply(numCoalescedSections, DISPATCH_APPLY_AUTO, ^(size_t index) {
        const CacheCoalescedText::StringSection& cacheStringSection = _coalescedText.getSectionData(CacheCoalescedText::SupportedSections[index]);
        if (log) fprintf(stderr, "copy %s __TEXT_COAL section %s (0x%08X bytes) to %p (logical addr 0x%llX)\n",
                         _options.archs->name(), CacheCoalescedText::SupportedSections[index],
                         cacheStringSection.bufferSize, cacheStringSection.bufferAddr, cacheStringSection.bufferVMAddr);
        for (const auto& stringAndOffset : cacheStringSection.stringsToOffsets)
            ::memcpy(cacheStringSection.bufferAddr + stringAndOffset.second, stringAndOffset.first.data(), stringAndOffset.first.size() + 1);
    });
}

void CacheBuilder::adjustAllImagesForNewSegmentLocations()
{
    __block std::vector<Diagnostics> diags;
    diags.resize(_sortedDylibs.size());

    // Note this cannot to be done in parallel because the LOH Tracker and aslr tracker are not thread safe
    for (size_t index = 0; index != _sortedDylibs.size(); ++index) {
        const DylibInfo& dylib = _sortedDylibs[index];
        adjustDylibSegments(dylib, diags[index]);
    }
    for (const Diagnostics& diag : diags) {
        if ( diag.hasError() ) {
            _diagnostics.error("%s", diag.errorMessage().c_str());
            break;
        }
    }
}


CacheBuilder::ASLR_Tracker::~ASLR_Tracker()
{
    if ( _bitmap != nullptr )
        ::free(_bitmap);
}

void CacheBuilder::ASLR_Tracker::setDataRegion(const void* rwRegionStart, size_t rwRegionSize)
{
    _pageCount   = (unsigned)(rwRegionSize+_pageSize-1)/_pageSize;
    _regionStart = (uint8_t*)rwRegionStart;
    _regionEnd   = (uint8_t*)rwRegionStart + rwRegionSize;
    _bitmap      = (bool*)calloc(_pageCount*(_pageSize/4)*sizeof(bool), 1);
}

void CacheBuilder::ASLR_Tracker::add(void* loc)
{
    if (!_enabled)
        return;
    uint8_t* p = (uint8_t*)loc;
    assert(p >= _regionStart);
    assert(p < _regionEnd);
    _bitmap[(p-_regionStart)/4] = true;
}

void CacheBuilder::ASLR_Tracker::remove(void* loc)
{
    if (!_enabled)
        return;
    uint8_t* p = (uint8_t*)loc;
    assert(p >= _regionStart);
    assert(p < _regionEnd);
    _bitmap[(p-_regionStart)/4] = false;
}

bool CacheBuilder::ASLR_Tracker::has(void* loc)
{
    if (!_enabled)
        return true;
    uint8_t* p = (uint8_t*)loc;
    assert(p >= _regionStart);
    assert(p < _regionEnd);
    return _bitmap[(p-_regionStart)/4];
}

void CacheBuilder::ASLR_Tracker::setHigh8(void* p, uint8_t high8)
{
    _high8Map[p] = high8;
}

void CacheBuilder::ASLR_Tracker::setAuthData(void* p, uint16_t diversity, bool hasAddrDiv, uint8_t key)
{
    _authDataMap[p] = {diversity, hasAddrDiv, key};
}

void CacheBuilder::ASLR_Tracker::setRebaseTarget32(void*p, uint32_t targetVMAddr)
{
    _rebaseTarget32[p] = targetVMAddr;
}

void CacheBuilder::ASLR_Tracker::setRebaseTarget64(void*p, uint64_t targetVMAddr)
{
    _rebaseTarget64[p] = targetVMAddr;
}

bool CacheBuilder::ASLR_Tracker::hasHigh8(void* p, uint8_t* highByte)
{
    auto pos = _high8Map.find(p);
    if ( pos == _high8Map.end() )
        return false;
    *highByte = pos->second;
    return true;
}

bool CacheBuilder::ASLR_Tracker::hasAuthData(void* p, uint16_t* diversity, bool* hasAddrDiv, uint8_t* key)
{
    auto pos = _authDataMap.find(p);
    if ( pos == _authDataMap.end() )
        return false;
    *diversity  = pos->second.diversity;
    *hasAddrDiv = pos->second.addrDiv;
    *key        = pos->second.key;
    return true;
}

bool CacheBuilder::ASLR_Tracker::hasRebaseTarget32(void* p, uint32_t* vmAddr)
{
    auto pos = _rebaseTarget32.find(p);
    if ( pos == _rebaseTarget32.end() )
        return false;
    *vmAddr = pos->second;
    return true;
}

bool CacheBuilder::ASLR_Tracker::hasRebaseTarget64(void* p, uint64_t* vmAddr)
{
    auto pos = _rebaseTarget64.find(p);
    if ( pos == _rebaseTarget64.end() )
        return false;
    *vmAddr = pos->second;
    return true;
}

////////////////////////////  DylibTextCoalescer ////////////////////////////////////

bool CacheBuilder::DylibTextCoalescer::sectionWasCoalesced(std::string_view sectionName) const {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, const DylibSectionOffsetToCacheSectionOffset*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    if (it == supportedSections.end())
        return false;
    return !it->second->empty();
}

CacheBuilder::DylibTextCoalescer::DylibSectionOffsetToCacheSectionOffset& CacheBuilder::DylibTextCoalescer::getSectionCoalescer(std::string_view sectionName) {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, DylibSectionOffsetToCacheSectionOffset*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    assert(it != supportedSections.end());
    return *it->second;
}

const CacheBuilder::DylibTextCoalescer::DylibSectionOffsetToCacheSectionOffset& CacheBuilder::DylibTextCoalescer::getSectionCoalescer(std::string_view sectionName) const {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, const DylibSectionOffsetToCacheSectionOffset*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    assert(it != supportedSections.end());
    return *it->second;
}

////////////////////////////  CacheCoalescedText ////////////////////////////////////
const char* CacheBuilder::CacheCoalescedText::SupportedSections[] = {
    "__objc_classname",
    "__objc_methname",
    "__objc_methtype",
};

void CacheBuilder::CacheCoalescedText::parseCoalescableText(const dyld3::MachOAnalyzer *ma,
                                                            DylibTextCoalescer& textCoalescer) {
    static const bool log = false;

    // We can only remove sections if we know we have split seg v2 to point to it
    // Otherwise, a PC relative load in the __TEXT segment wouldn't know how to point to the new strings
    // which are no longer in the same segment
    uint32_t splitSegSize = 0;
    const void* splitSegStart = ma->getSplitSeg(splitSegSize);
    if (!splitSegStart)
        return;

    if ((*(const uint8_t*)splitSegStart) != DYLD_CACHE_ADJ_V2_FORMAT)
        return;

    // We can only remove sections from the end of a segment, so cache them all and walk backwards.
    __block std::vector<std::pair<std::string, dyld3::MachOAnalyzer::SectionInfo>> textSectionInfos;
    ma->forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo &sectInfo, bool malformedSectionRange, bool &stop) {
        if (strcmp(sectInfo.segInfo.segName, "__TEXT") != 0)
            return;
        assert(!malformedSectionRange);
        textSectionInfos.push_back({ sectInfo.sectName, sectInfo });
    });

    const std::set<std::string_view> supportedSections(std::begin(SupportedSections), std::end(SupportedSections));
    int64_t slide = ma->getSlide();

    for (auto sectionInfoIt = textSectionInfos.rbegin(); sectionInfoIt != textSectionInfos.rend(); ++sectionInfoIt) {
        const std::string& sectionName = sectionInfoIt->first;
        const dyld3::MachOAnalyzer::SectionInfo& sectInfo = sectionInfoIt->second;

        // If we find a section we can't handle then stop here.  Hopefully we coalesced some from the end.
        if (supportedSections.find(sectionName) == supportedSections.end())
            break;

        StringSection& cacheStringSection = getSectionData(sectionName);

        DylibTextCoalescer::DylibSectionOffsetToCacheSectionOffset& sectionStringData = textCoalescer.getSectionCoalescer(sectionName);

        // Walk the strings in this section
        const uint8_t* content = (uint8_t*)(sectInfo.sectAddr + slide);
        const char* s   = (char*)content;
        const char* end = s + sectInfo.sectSize;
        while ( s < end ) {
            std::string_view str = s;
            auto itAndInserted = cacheStringSection.stringsToOffsets.insert({ str, cacheStringSection.bufferSize });
            if (itAndInserted.second) {
                // If we inserted the string then we need to include it in the total
                cacheStringSection.bufferSize += str.size() + 1;
                if (log)
                    printf("Selector: %s -> %s\n", ma->installName(), s);
            } else {
                // Debugging only.  If we didn't include the string then we saved that many bytes
                cacheStringSection.savedSpace += str.size() + 1;
            }

            // Now keep track of this offset in our source dylib as pointing to this offset
            uint32_t sourceSectionOffset = (uint32_t)((uint64_t)s - (uint64_t)content);
            uint32_t cacheSectionOffset = itAndInserted.first->second;
            sectionStringData[sourceSectionOffset] = cacheSectionOffset;
            s += str.size() + 1;
        }
    }
}

void CacheBuilder::CacheCoalescedText::clear() {
    *this = CacheBuilder::CacheCoalescedText();
}


CacheBuilder::CacheCoalescedText::StringSection& CacheBuilder::CacheCoalescedText::getSectionData(std::string_view sectionName) {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, StringSection*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    assert(it != supportedSections.end());
    return *it->second;
}


const CacheBuilder::CacheCoalescedText::StringSection& CacheBuilder::CacheCoalescedText::getSectionData(std::string_view sectionName) const {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, const StringSection*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    assert(it != supportedSections.end());
    return *it->second;
}
