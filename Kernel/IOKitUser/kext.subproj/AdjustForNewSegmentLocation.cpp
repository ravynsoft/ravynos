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

#include "MachOFileAbstraction.hpp"

#include <dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <assert.h>

/* std headers */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
//#include <map>
#include <unordered_map>
//#include <set>
#include <unordered_set>
#include <vector>

#include "Trie.hpp"
#include "kextsplit.h"
#include "OSKext.h"

namespace kcgen {
    
    inline uint64_t align(uint64_t addr, uint8_t p2)
    {
        uint64_t mask = (1 << p2);
        return (addr + mask - 1) & (-mask);
    }
    
    template <typename P>
    class Adjustor {
    public:
        Adjustor(void* cacheBuffer, macho_header<P>* mh, const std::vector<uint64_t>& segOrigStartAddresses,
                 const std::vector<uint64_t>& segNewStartAddresses, const std::vector<uint64_t>& segCacheFileOffset,
                 const std::vector<uint64_t>& segCacheFileSizes);
        void  adjustImageForNewSegmentLocations(std::vector<void*>& pointersForASLR);
        
    private:
        void     adjustReferencesUsingInfoV2(std::vector<void*>& pointersForASLR);
        void     adjustReference(uint32_t kind, uint8_t* mappedAddr, uint64_t fromNewAddress, uint64_t toNewAddress, int64_t adjust, int64_t targetSlide,
                                 uint64_t imageStartAddress, uint64_t imageEndAddress, std::vector<void*>& pointersForASLR, uint32_t*& lastMappedAddr32,
                                 uint32_t& lastKind, uint64_t& lastToNewAddress);
        void     adjustDataPointers(std::vector<void*>& pointersForASLR);
        void     slidePointer(int segIndex, uint64_t segOffset, uint8_t type, std::vector<void*>& pointersForASLR);
        void     adjustSymbolTable();
        void     adjustExportsTrie(std::vector<uint8_t>& newTrieBytes);
        void     rebuildLinkEdit();
        void     adjustCode();
        void     adjustInstruction(uint8_t kind, uint64_t cacheOffset, uint64_t codeToDataDelta);
        void     rebuildLinkEditAndLoadCommands();
        uint64_t slideForOrigAddress(uint64_t addr);
        
        typedef typename P::uint_t pint_t;
        typedef typename P::E E;
        
        void*                                   _cacheBuffer;
        macho_header<P>*                        _mh;
        const uint8_t*                          _kextMachoPtr       = nullptr;
        int64_t                                 _kextMachoOfst     = 0;
        unsigned                                _linkeditSegIndex   = 0;
        bool                                    _maskPointers       = false;
        bool                                    _splitSegInfoV2     = false;
        const char*                             _installName        = nullptr;
        macho_symtab_command<P>*                _symTabCmd          = nullptr;
        macho_dysymtab_command<P>*              _dynSymTabCmd       = nullptr;
        macho_dyld_info_command<P>*             _dyldInfo           = nullptr;
        macho_linkedit_data_command<P>*         _splitSegInfoCmd    = nullptr;
        macho_linkedit_data_command<P>*         _functionStartsCmd  = nullptr;
        macho_linkedit_data_command<P>*         _dataInCodeCmd      = nullptr;
        std::vector<uint64_t>                   _segOrigStartAddresses;
        std::vector<uint64_t>                   _segNewStartAddresses;
        std::vector<uint64_t>                   _segCacheOffsets;
        std::vector<uint64_t>                   _segCacheSizes;
        std::vector<uint64_t>                   _segSlides;
        std::vector<macho_segment_command<P>*>  _segCmds;

        // general adjustor logging
        static const bool _log = false;

        // high-level reference adjustor logging
        static const bool _log_ref = false;

        // log about (almost) every reference adjustments (very verbose)
        static const bool _log_adj = false;

    };

    template <typename P>
    Adjustor<P>::Adjustor(void* cacheBuffer, macho_header<P>* mh, const std::vector<uint64_t>& segOrigStartAddresses,
                          const std::vector<uint64_t>& segNewStartAddresses, const std::vector<uint64_t>& segCacheFileOffsets,
                          const std::vector<uint64_t>& segCacheFileSizes)
    : _cacheBuffer(cacheBuffer), _mh(mh), _segOrigStartAddresses(segOrigStartAddresses), _segNewStartAddresses(segNewStartAddresses),
      _segCacheOffsets(segCacheFileOffsets), _segCacheSizes(segCacheFileSizes)
    {
        macho_segment_command<P>* segCmd;
        const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
        const uint32_t cmd_count = mh->ncmds();
        const macho_load_command<P>* cmd = cmds;
        unsigned segIndex = 0;
        for (uint32_t i = 0; i < cmd_count; ++i) {
            switch (cmd->cmd()) {
                case LC_ID_DYLIB:
                    _installName = ((macho_dylib_command<P>*)cmd)->name();
                    if (_log)
                        kcgen_verboseLog("Found LC_ID_DYLIB @cmd:%d, _idDylib:%p _installName:'%s'", i, cmd, _installName);
                    break;
                case LC_SYMTAB:
                    _symTabCmd = (macho_symtab_command<P>*)cmd;
                    if (_log)
                        kcgen_verboseLog("Found LC_SYMTAB @cmd:%d, _symTabCmd:%p", i, _symTabCmd);
                    break;
                case LC_DYSYMTAB:
                    _dynSymTabCmd = (macho_dysymtab_command<P>*)cmd;
                    if (_log)
                        kcgen_verboseLog("Found LC_DYSYMTAB @cmd:%d, _dynSymTabCmd:%p", i, _dynSymTabCmd);
                    break;
                case LC_DYLD_INFO:
                case LC_DYLD_INFO_ONLY:
                    _dyldInfo = (macho_dyld_info_command<P>*)cmd;
                    if (_log)
                        kcgen_verboseLog("Found LC_DYLD_INFO @cmd:%d, _dyldInfo:%p", i, _dyldInfo);
                    break;
                case LC_SEGMENT_SPLIT_INFO:
                    _splitSegInfoCmd = (macho_linkedit_data_command<P>*)cmd;
                    if (_log)
                        kcgen_verboseLog("Found LC_SEGMENT_SPLIT_INFO @cmd:%d, _splitSegInfoCmd:%p", i, _splitSegInfoCmd);
                    break;
                case LC_FUNCTION_STARTS:
                    _functionStartsCmd = (macho_linkedit_data_command<P>*)cmd;
                    break;
                case LC_DATA_IN_CODE:
                    _dataInCodeCmd = (macho_linkedit_data_command<P>*)cmd;
                    break;
                case macho_segment_command<P>::CMD:
                    segCmd = (macho_segment_command<P>*)cmd;
                    _segCmds.push_back(segCmd);
                    _segSlides.push_back(_segNewStartAddresses[segIndex] - _segOrigStartAddresses[segIndex]);
                    if (_log)
                        kcgen_verboseLog("Found SEGMENT:'%s' @cmd:%d (%p) newVA:0x%llx (slide:%llx)",
                                         segCmd->segname(), i, cmd, _segNewStartAddresses[segIndex], _segSlides.back());
                    if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
                        _kextMachoOfst = segCacheFileOffsets[segIndex] - segCmd->fileoff();
                        _kextMachoPtr = (uint8_t*)cacheBuffer + _kextMachoOfst;
                        _linkeditSegIndex = segIndex;
                        if (_log)
                            kcgen_verboseLog("Found LINKEDIT @idx:%d, kextMachoOfst=%lld (segCacheFileOfst:%lld,segCmd->fileoff:%lld), kextMacho=%p, lsSegIdx=%d",
                                             segIndex, _kextMachoOfst, segCacheFileOffsets[segIndex], segCmd->fileoff(), _kextMachoPtr, _linkeditSegIndex);
                    }
                    ++segIndex;
                    break;
            }
            cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
        }
        _maskPointers = (P::E::get32(mh->cputype()) == CPU_TYPE_ARM64);
        if ( _splitSegInfoCmd != NULL ) {
            const uint8_t* infoStart = &_kextMachoPtr[_splitSegInfoCmd->dataoff()];
            _splitSegInfoV2 = (*infoStart == DYLD_CACHE_ADJ_V2_FORMAT);
            if (_log)
                kcgen_verboseLog("splitSegInfoV2=%c, *infoStart=%d, infoStart=&_linkeditBias[%d]",
                                 _splitSegInfoV2 ? 'T' : 'F', *infoStart, _splitSegInfoCmd->dataoff());
        }
    }

    template <typename P>
    void Adjustor<P>::adjustImageForNewSegmentLocations(std::vector<void*>& pointersForASLR)
    {
        if ( _splitSegInfoV2 ) {
            adjustReferencesUsingInfoV2(pointersForASLR);
        } else {
            adjustDataPointers(pointersForASLR);
            adjustCode();
        }
        if (_symTabCmd != nullptr && _dynSymTabCmd != nullptr)
            adjustSymbolTable();
        if (_dyldInfo)
            rebuildLinkEditAndLoadCommands();
    }

    template <typename P>
    uint64_t Adjustor<P>::slideForOrigAddress(uint64_t addr)
    {
        for (unsigned i=0; i < _segOrigStartAddresses.size(); ++i) {
            if ( (_segOrigStartAddresses[i] <= addr) && (addr < (_segOrigStartAddresses[i]+_segCmds[i]->vmsize())) )
                return _segSlides[i];
        }
        // On arm64, high nibble of pointers can have extra bits
        if ( _maskPointers && (addr & 0xF000000000000000) ) {
            return slideForOrigAddress(addr & 0x0FFFFFFFFFFFFFFF);
        }
        kcgen_terminate("slide not known for given address");
    }
    
    template <typename P>
    void Adjustor<P>::rebuildLinkEditAndLoadCommands()
    {
        // Exports trie is only data structure in LINKEDIT that might grow
        std::vector<uint8_t> newTrieBytes;
        adjustExportsTrie(newTrieBytes);
        
        // Remove: code signature, rebase info, code-sign-dirs, split seg info
        uint32_t bindOffset          = 0;
        uint32_t bindSize            = _dyldInfo->bind_size();
        uint32_t lazyBindOffset      = bindOffset + bindSize;
        uint32_t lazyBindSize        = _dyldInfo->lazy_bind_size();
        uint32_t weakBindOffset      = lazyBindOffset + lazyBindSize;
        uint32_t weakBindSize        = _dyldInfo->weak_bind_size();
        uint32_t exportOffset        = weakBindOffset + weakBindSize;
        uint32_t exportSize          = (uint32_t)newTrieBytes.size();
        uint32_t splitSegInfoOffset  = exportOffset + exportSize;
        uint32_t splitSegInfosSize   = (_splitSegInfoCmd ? _splitSegInfoCmd->datasize() : 0);
        uint32_t funcStartsOffset    = splitSegInfoOffset + splitSegInfosSize;
        uint32_t funcStartsSize      = (_functionStartsCmd ? _functionStartsCmd->datasize() : 0);
        uint32_t dataInCodeOffset    = funcStartsOffset + funcStartsSize;
        uint32_t dataInCodeSize      = (_dataInCodeCmd ? _dataInCodeCmd->datasize() : 0);
        uint32_t symbolTableOffset   = dataInCodeOffset + dataInCodeSize;
        uint32_t symbolTableSize     = _symTabCmd ? (_symTabCmd->nsyms() * sizeof(macho_nlist<P>)) : 0;
        uint32_t indirectTableOffset = symbolTableOffset + symbolTableSize;
        uint32_t indirectTableSize   = _dynSymTabCmd ? (_dynSymTabCmd->nindirectsyms() * sizeof(uint32_t)) : 0;
        uint32_t symbolStringsOffset = indirectTableOffset + indirectTableSize;
        uint32_t symbolStringsSize   = _symTabCmd ? _symTabCmd->strsize() : 0;
        uint32_t newLinkEditSize     = symbolStringsOffset + symbolStringsSize;

        size_t linkeditBufferSize = align(_segCmds[_linkeditSegIndex]->vmsize(), 12);
        if ( linkeditBufferSize < newLinkEditSize ) {
            kcgen_terminate("LINKEDIT overflow in %s", _installName);
        }

        uint32_t linkeditStartOffset = (uint32_t)_segCacheOffsets[_linkeditSegIndex];
        uint8_t* newLinkeditBufer = (uint8_t*)::calloc(linkeditBufferSize, 1);
        if ( bindSize )
            memcpy(&newLinkeditBufer[bindOffset], &_kextMachoPtr[_dyldInfo->bind_off()], bindSize);
        if ( lazyBindSize )
            memcpy(&newLinkeditBufer[lazyBindOffset], &_kextMachoPtr[_dyldInfo->lazy_bind_off()], lazyBindSize);
        if ( weakBindSize )
            memcpy(&newLinkeditBufer[weakBindOffset], &_kextMachoPtr[_dyldInfo->weak_bind_off()], weakBindSize);
        if ( exportSize )
            memcpy(&newLinkeditBufer[exportOffset], &newTrieBytes[0], exportSize);
        if ( splitSegInfosSize )
            memcpy(&newLinkeditBufer[splitSegInfoOffset], &_kextMachoPtr[_splitSegInfoCmd->dataoff()], splitSegInfosSize);
        if ( funcStartsSize )
            memcpy(&newLinkeditBufer[funcStartsOffset], &_kextMachoPtr[_functionStartsCmd->dataoff()], funcStartsSize);
        if ( dataInCodeSize )
            memcpy(&newLinkeditBufer[dataInCodeOffset], &_kextMachoPtr[_dataInCodeCmd->dataoff()], dataInCodeSize);
        if ( symbolTableSize && _symTabCmd )
            memcpy(&newLinkeditBufer[symbolTableOffset], &_kextMachoPtr[_symTabCmd->symoff()], symbolTableSize);
        if ( indirectTableSize && _dynSymTabCmd )
            memcpy(&newLinkeditBufer[indirectTableOffset], &_kextMachoPtr[_dynSymTabCmd->indirectsymoff()], indirectTableSize);
        if ( symbolStringsSize && _symTabCmd )
            memcpy(&newLinkeditBufer[symbolStringsOffset], &_kextMachoPtr[_symTabCmd->stroff()], symbolStringsSize);
        
        memcpy((uint8_t*)_cacheBuffer+linkeditStartOffset, newLinkeditBufer, newLinkEditSize);
        ::bzero((uint8_t*)_cacheBuffer+linkeditStartOffset+newLinkEditSize, linkeditBufferSize-newLinkEditSize);
        ::free(newLinkeditBufer);
        
        // updates load commands and removed ones no longer needed
        macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)_mh + sizeof(macho_header<P>));
        uint32_t cmd_count = _mh->ncmds();
        const macho_load_command<P>* cmd = cmds;
        const unsigned origLoadCommandsSize = _mh->sizeofcmds();
        unsigned bytesRemaining = origLoadCommandsSize;
        unsigned removedCount = 0;
        unsigned segIndex = 0;
        for (uint32_t i = 0; i < cmd_count; ++i) {
            macho_symtab_command<P>*           symTabCmd;
            macho_dysymtab_command<P>*         dynSymTabCmd;
            macho_dyld_info_command<P>*        dyldInfo;
            macho_linkedit_data_command<P>*    functionStartsCmd;
            macho_linkedit_data_command<P>*    dataInCodeCmd;
            macho_linkedit_data_command<P>*    splitSegInfoCmd;
            macho_segment_command<P>*          segCmd;
            macho_routines_command<P>*         routinesCmd;
            macho_dylib_command<P>*            dylibIDCmd;
            uint32_t cmdSize = cmd->cmdsize();
            int32_t segFileOffsetDelta;
            bool remove = false;
            switch ( cmd->cmd() ) {
                case LC_ID_DYLIB:
                    dylibIDCmd = (macho_dylib_command<P>*)cmd;
                    dylibIDCmd->set_timestamp(2); // match what static linker sets in LC_LOAD_DYLIB
                    break;
                case LC_SYMTAB:
                    symTabCmd = (macho_symtab_command<P>*)cmd;
                    symTabCmd->set_symoff(linkeditStartOffset+symbolTableOffset);
                    symTabCmd->set_stroff(linkeditStartOffset+symbolStringsOffset);
                    break;
                case LC_DYSYMTAB:
                    dynSymTabCmd = (macho_dysymtab_command<P>*)cmd;
                    dynSymTabCmd->set_indirectsymoff(linkeditStartOffset+indirectTableOffset);
                    break;
                case LC_DYLD_INFO:
                case LC_DYLD_INFO_ONLY:
                    dyldInfo = (macho_dyld_info_command<P>*)cmd;
                    dyldInfo->set_rebase_off(0);
                    dyldInfo->set_rebase_size(0);
                    dyldInfo->set_bind_off(bindSize ? linkeditStartOffset+bindOffset : 0);
                    dyldInfo->set_bind_size(bindSize);
                    dyldInfo->set_weak_bind_off(weakBindSize ? linkeditStartOffset+weakBindOffset : 0);
                    dyldInfo->set_weak_bind_size(weakBindSize);
                    dyldInfo->set_lazy_bind_off(lazyBindSize ? linkeditStartOffset+lazyBindOffset : 0);
                    dyldInfo->set_lazy_bind_size(lazyBindSize);
                    dyldInfo->set_export_off(exportSize ? linkeditStartOffset+exportOffset : 0);
                    dyldInfo->set_export_size(exportSize);
                    break;
                case LC_FUNCTION_STARTS:
                    functionStartsCmd = (macho_linkedit_data_command<P>*)cmd;
                    functionStartsCmd->set_dataoff(linkeditStartOffset+funcStartsOffset);
                    break;
                case LC_DATA_IN_CODE:
                    dataInCodeCmd = (macho_linkedit_data_command<P>*)cmd;
                    dataInCodeCmd->set_dataoff(linkeditStartOffset+dataInCodeOffset);
                    break;
                case macho_routines_command<P>::CMD:
                    routinesCmd = (macho_routines_command<P>*)cmd;
                    routinesCmd->set_init_address(routinesCmd->init_address()+slideForOrigAddress(routinesCmd->init_address()));
                    break;
                case macho_segment_command<P>::CMD:
                    segCmd = (macho_segment_command<P>*)cmd;
                    segFileOffsetDelta = (int32_t)(_segCacheOffsets[segIndex] - segCmd->fileoff());
                    segCmd->set_vmaddr(_segNewStartAddresses[segIndex]);
                    segCmd->set_vmsize(_segCacheSizes[segIndex]);
                    segCmd->set_fileoff(_segCacheOffsets[segIndex]);
                    segCmd->set_filesize(_segCacheSizes[segIndex]);
                    if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 )
                        segCmd->set_vmsize(linkeditBufferSize);
                    if ( segCmd->nsects() > 0 ) {
                        macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
                        macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
                        for (macho_section<P>*  sect=sectionsStart; sect < sectionsEnd; ++sect) {
                            sect->set_addr(sect->addr() + _segSlides[segIndex]);
                            if ( sect->offset() != 0 )
                                sect->set_offset(sect->offset() + segFileOffsetDelta);
                        }
                    }
                    ++segIndex;
                    break;
                case LC_RPATH:
                    kcgen_warning("kcgen does not support LC_RPATH found in %s", _installName);
                    remove = true;
                    break;
                case LC_SEGMENT_SPLIT_INFO:
                    splitSegInfoCmd = (macho_linkedit_data_command<P>*)cmd;
                    splitSegInfoCmd->set_dataoff(linkeditStartOffset+splitSegInfoOffset);
                    break;
                case LC_CODE_SIGNATURE:
                case LC_DYLIB_CODE_SIGN_DRS:
                    remove = true;
                    break;
                default:
                    break;
            }
            macho_load_command<P>* nextCmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmdSize);
            if ( remove ) {
                ::memmove((void*)cmd, (void*)nextCmd, bytesRemaining);
                ++removedCount;
            }
            else {
                bytesRemaining -= cmdSize;
                cmd = nextCmd;
            }
        }
        // zero out stuff removed
        ::bzero((void*)cmd, bytesRemaining);
        // update header
        _mh->set_ncmds(cmd_count-removedCount);
        _mh->set_sizeofcmds(origLoadCommandsSize-bytesRemaining);
        _mh->set_flags(_mh->flags() | 0x80000000);
    }
    
    
    template <typename P>
    void Adjustor<P>::adjustSymbolTable()
    {
        macho_nlist<P>*  symbolTable = (macho_nlist<P>*)&_kextMachoPtr[_symTabCmd->symoff()];
        
        // adjust global symbol table entries
        macho_nlist<P>* lastExport = &symbolTable[_dynSymTabCmd->iextdefsym()+_dynSymTabCmd->nextdefsym()];
        for (macho_nlist<P>* entry = &symbolTable[_dynSymTabCmd->iextdefsym()]; entry < lastExport; ++entry) {
            if ( (entry->n_type() & N_TYPE) == N_SECT )
                entry->set_n_value(entry->n_value() + slideForOrigAddress(entry->n_value()));
        }
        
        // adjust local symbol table entries
        macho_nlist<P>*  lastLocal = &symbolTable[_dynSymTabCmd->ilocalsym()+_dynSymTabCmd->nlocalsym()];
        for (macho_nlist<P>* entry = &symbolTable[_dynSymTabCmd->ilocalsym()]; entry < lastLocal; ++entry) {
            if ( (entry->n_sect() != NO_SECT) && ((entry->n_type() & N_STAB) == 0) )
                entry->set_n_value(entry->n_value() + slideForOrigAddress(entry->n_value()));
        }
    }
    
    template <typename P>
    void Adjustor<P>::slidePointer(int segIndex, uint64_t segOffset, uint8_t type, std::vector<void*>& pointersForASLR)
    {
        pint_t*  mappedAddrP = (pint_t*)((uint8_t*)_cacheBuffer + _segCacheOffsets[segIndex] + segOffset);
        pint_t valueP;
        switch ( type ) {
            case REBASE_TYPE_POINTER:
                valueP = (pint_t)P::getP(*mappedAddrP);
                P::setP(*mappedAddrP, valueP + slideForOrigAddress(valueP));
                break;
                
            case REBASE_TYPE_TEXT_ABSOLUTE32:
            case REBASE_TYPE_TEXT_PCREL32:
                // text relocations not supported in dyld shared cache
            default:
                kcgen_terminate("unknown rebase type 0x%02X", type);
        }
        pointersForASLR.push_back(mappedAddrP);
    }
    
    
    static bool isThumbMovw(uint32_t instruction)
    {
        return ( (instruction & 0x8000FBF0) == 0x0000F240 );
    }
    
    static bool isThumbMovt(uint32_t instruction)
    {
        return ( (instruction & 0x8000FBF0) == 0x0000F2C0 );
    }
    
    static uint16_t getThumbWord(uint32_t instruction)
    {
        uint32_t i = ((instruction & 0x00000400) >> 10);
        uint32_t imm4 = (instruction & 0x0000000F);
        uint32_t imm3 = ((instruction & 0x70000000) >> 28);
        uint32_t imm8 = ((instruction & 0x00FF0000) >> 16);
        return ((imm4 << 12) | (i << 11) | (imm3 << 8) | imm8);
    }
    
    static uint32_t setThumbWord(uint32_t instruction, uint16_t word) {
        uint32_t imm4 = (word & 0xF000) >> 12;
        uint32_t i =    (word & 0x0800) >> 11;
        uint32_t imm3 = (word & 0x0700) >> 8;
        uint32_t imm8 =  word & 0x00FF;
        return (instruction & 0x8F00FBF0) | imm4 | (i << 10) | (imm3 << 28) | (imm8 << 16);
    }
    
    static bool isArmMovw(uint32_t instruction)
    {
        return (instruction & 0x0FF00000) == 0x03000000;
    }
    
    static bool isArmMovt(uint32_t instruction)
    {
        return (instruction & 0x0FF00000) == 0x03400000;
    }
    
    static uint16_t getArmWord(uint32_t instruction)
    {
        uint32_t imm4 = ((instruction & 0x000F0000) >> 16);
        uint32_t imm12 = (instruction & 0x00000FFF);
        return (imm4 << 12) | imm12;
    }
    
    static uint32_t setArmWord(uint32_t instruction, uint16_t word) {
        uint32_t imm4 = (word & 0xF000) >> 12;
        uint32_t imm12 = word & 0x0FFF;
        return (instruction & 0xFFF0F000) | (imm4 << 16) | imm12;
    }

    template <typename P>
    void Adjustor<P>::adjustReference(uint32_t kind, uint8_t* mappedAddr, uint64_t fromNewAddress, uint64_t toNewAddress,
                                      int64_t adjust, int64_t targetSlide, uint64_t imageStartAddress, uint64_t imageEndAddress,
                                      std::vector<void*>& pointersForASLR, uint32_t*& lastMappedAddr32, uint32_t& lastKind, uint64_t& lastToNewAddress)
    {
        uint64_t value64;
        uint64_t* mappedAddr64 = nullptr;
        uint32_t value32;
        uint32_t* mappedAddr32 = nullptr;
        uint32_t instruction;
        int64_t offsetAdjust;
        int64_t delta;
        switch ( kind ) {
            case DYLD_CACHE_ADJ_V2_DELTA_32:
                mappedAddr32 = (uint32_t*)mappedAddr;
                value32 = P::E::get32(*mappedAddr32);
                delta = (int32_t)value32;
                delta += adjust;
                if ( (delta > 0x80000000) || (-delta > 0x80000000) ) {
                    kcgen_terminate("DYLD_CACHE_ADJ_V2_DELTA_32 can't be adjust by 0x%016llX", adjust);
                }
                P::E::set32(*mappedAddr32, (int32_t)delta);
                break;
            case DYLD_CACHE_ADJ_V2_POINTER_32:
                mappedAddr32 = (uint32_t*)mappedAddr;
                if ( toNewAddress != (uint64_t) (E::get32(*mappedAddr32) + targetSlide) ) {
                    kcgen_terminate("bad DYLD_CACHE_ADJ_V2_POINTER_32 value not as expected at address 0x%llX", fromNewAddress);
                }
                E::set32(*mappedAddr32, (uint32_t)toNewAddress);
                pointersForASLR.push_back(mappedAddr);
                break;
            case DYLD_CACHE_ADJ_V2_POINTER_64:
                mappedAddr64 = (uint64_t*)mappedAddr;

                /* someone (kxld?) has already fixed these up for us */
                if (toNewAddress == E::get64(*mappedAddr64)) {
                    if (_log_adj)
                        kcgen_verboseLog("DYLD_CACHE_ADJ_V2_POINTER_64 at address 0x%llX already adjusted", fromNewAddress);
                    break;
                }
            
                if ( toNewAddress != (E::get64(*mappedAddr64) + targetSlide) ) {
                    kcgen_terminate("bad DYLD_CACHE_ADJ_V2_POINTER_64 value not as expected at address 0x%llX value %llX targetSlide %llx", fromNewAddress, E::get64(*mappedAddr64), targetSlide);
                }
                E::set64(*mappedAddr64, toNewAddress);
                pointersForASLR.push_back(mappedAddr);
                break;
            case DYLD_CACHE_ADJ_V2_DELTA_64:
                mappedAddr64 = (uint64_t*)mappedAddr;
                value64 = P::E::get64(*mappedAddr64);
                E::set64(*mappedAddr64, value64 + adjust);
                break;
            case DYLD_CACHE_ADJ_V2_IMAGE_OFF_32:
                if ( adjust == 0 )
                    break;
                mappedAddr32 = (uint32_t*)mappedAddr;
                value32 = P::E::get32(*mappedAddr32);
                value64 = toNewAddress - imageStartAddress;
                if ( value64 > imageEndAddress ) {
                    kcgen_terminate("DYLD_CACHE_ADJ_V2_IMAGE_OFF_32 can't be adjust to 0x%016llX", toNewAddress);
                }
                P::E::set32(*mappedAddr32, (uint32_t)value64);
                break;
            case DYLD_CACHE_ADJ_V2_ARM64_ADRP:
                mappedAddr32 = (uint32_t*)mappedAddr;
                instruction = P::E::get32(*mappedAddr32);
                if ( (instruction & 0x9F000000) == 0x90000000 ) {
                    int64_t pageDistance = ((toNewAddress & ~0xFFF) - (fromNewAddress & ~0xFFF));
                    int64_t newPage21 = pageDistance >> 12;
                    if ( (newPage21 > 2097151) || (newPage21 < -2097151) ) {
                        kcgen_terminate("DYLD_CACHE_ADJ_V2_ARM64_ADRP can be adjusted that far");
                    }
                    instruction = (instruction & 0x9F00001F) | ((newPage21 << 29) & 0x60000000) | ((newPage21 << 3) & 0x00FFFFE0);
                    P::E::set32(*mappedAddr32, instruction);
                }
                else {
                    // ADRP instructions are sometimes optimized to other instructions (e.g. ADR) after the split-seg-info is generated
                }
                break;
            case DYLD_CACHE_ADJ_V2_ARM64_OFF12:
                mappedAddr32 = (uint32_t*)mappedAddr;
                instruction = P::E::get32(*mappedAddr32);
                offsetAdjust = (adjust & 0xFFF);
                if (_log_adj)
                    kcgen_verboseLog("DYLD_CACHE_ADJ_V2_ARM64_OFF12: %p insn %0x offsetAdjust %lld", mappedAddr32, instruction, offsetAdjust);
                if ( offsetAdjust == 0 ) {
                    break;
                }
                if ( (instruction & 0x3B000000) == 0x39000000 ) {
                    // LDR/STR imm12
                    if ( offsetAdjust != 0 ) {
                        uint32_t encodedAddend = ((instruction & 0x003FFC00) >> 10);
                        uint32_t newAddend = 0;
                        if (_log_adj)
                            kcgen_verboseLog("encodedAddend %x", encodedAddend);
                        switch ( instruction & 0xC0000000 ) {
                            case 0x00000000:
                                if ( (instruction & 0x04800000) == 0x04800000 ) {
                                    if ( offsetAdjust & 0xF ) {
                                        kcgen_terminate("can't adjust off12 scale=16 instruction by %lld bytes at mapped address=%p", offsetAdjust, mappedAddr);
                                    }
                                    if ( encodedAddend*16 >= 4096 ) {
                                        kcgen_terminate("off12 scale=16 instruction points outside its page at mapped address=%p", mappedAddr);
                                    }
                                    newAddend = (encodedAddend + offsetAdjust/16) % 256;
                                }
                                else {
                                    // scale=1
                                    newAddend = (encodedAddend + (int32_t)offsetAdjust) % 4096;
                                }
                                break;
                            case 0x40000000:
                                if ( offsetAdjust & 1 ) {
                                    kcgen_terminate("can't adjust off12 scale=2 instruction by %lld bytes at mapped address=%p", offsetAdjust, mappedAddr);
                                }
                                if ( encodedAddend*2 >= 4096 ) {
                                    kcgen_terminate("off12 scale=2 instruction points outside its page at mapped address=%p", mappedAddr);
                                }
                                newAddend = (encodedAddend + offsetAdjust/2) % 2048;
                                break;
                            case 0x80000000:
                                if ( offsetAdjust & 3 ) {
                                    kcgen_terminate("can't adjust off12 scale=4 instruction by %lld bytes at mapped address=%p", offsetAdjust, mappedAddr);
                                }
                                if ( encodedAddend*4 >= 4096 ) {
                                    kcgen_terminate("off12 scale=4 instruction points outside its page at mapped address=%p", mappedAddr);
                                }
                                newAddend = (encodedAddend + offsetAdjust/4) % 1024;
                                break;
                            case 0xC0000000:
                                if ( offsetAdjust & 7 ) {
                                    kcgen_terminate("can't adjust off12 scale=8 instruction by %lld bytes at mapped address=%p", offsetAdjust, mappedAddr);
                                }
                                if ( encodedAddend*8 >= 4096 ) {
                                    kcgen_terminate("off12 scale=8 instruction points outside its page at mapped address=%p", mappedAddr);
                                }
                                newAddend = (encodedAddend + offsetAdjust/8) % 512;
                                if (_log_adj)
                                    kcgen_verboseLog("off12 scale=8 insn newAddend %x", newAddend);
                                break;
                        }
                        uint32_t newInstruction = (instruction & 0xFFC003FF) | (newAddend << 10);
                        P::E::set32(*mappedAddr32, newInstruction);
                    }
                }
                else if ( (instruction & 0xFFC00000) == 0x91000000 ) {
                    if (_log_adj)
                        kcgen_verboseLog("adjust ADD imm12: %p (%0x)\n", mappedAddr32, instruction);
                    // ADD imm12
                    if ( instruction & 0x00C00000 ) {
                        kcgen_terminate("ADD off12 uses shift at mapped address=%p", mappedAddr);
                    }
                    uint32_t encodedAddend = ((instruction & 0x003FFC00) >> 10);
                    uint32_t newAddend = (encodedAddend + offsetAdjust) & 0xFFF;
                    uint32_t newInstruction = (instruction & 0xFFC003FF) | (newAddend << 10);
                    if (_log_adj)
                        kcgen_verboseLog("encodedAddend %0x newAddend %0x newInstruction %0x", encodedAddend, newAddend, newInstruction);
                    P::E::set32(*mappedAddr32, newInstruction);
                }
                else if ( instruction != 0xD503201F ) {
                    // ignore imm12 instructions optimized into a NOP, but warn about others
                    kcgen_terminate("unknown off12 instruction 0x%08X at 0x%0llX", instruction, fromNewAddress);
                }
                break;

            case DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT:
                mappedAddr32 = (uint32_t*)mappedAddr;
                // to update a movw/movt pair we need to extract the 32-bit they will make,
                // add the adjust and write back the new movw/movt pair.
                if ( lastKind == kind ) {
                    if ( lastToNewAddress == toNewAddress ) {
                        uint32_t instruction1 = P::E::get32(*lastMappedAddr32);
                        uint32_t instruction2 = P::E::get32(*mappedAddr32);
                        if ( isThumbMovw(instruction1) && isThumbMovt(instruction2) ) {
                            uint16_t high = getThumbWord(instruction2);
                            uint16_t low  = getThumbWord(instruction1);
                            uint32_t full = high << 16 | low;
                            full += adjust;
                            instruction1 = setThumbWord(instruction1, full & 0xFFFF);
                            instruction2 = setThumbWord(instruction2, full >> 16);
                        }
                        else if ( isThumbMovt(instruction1) && isThumbMovw(instruction2) ) {
                            uint16_t high = getThumbWord(instruction1);
                            uint16_t low  = getThumbWord(instruction2);
                            uint32_t full = high << 16 | low;
                            full += adjust;
                            instruction2 = setThumbWord(instruction2, full & 0xFFFF);
                            instruction1 = setThumbWord(instruction1, full >> 16);
                        }
                        else {
                            kcgen_terminate("two DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT in a row but not");
                        }
                        P::E::set32(*lastMappedAddr32, instruction1);
                        P::E::set32(*mappedAddr32, instruction2);
                        kind = 0;
                    }
                    else {
                        kcgen_terminate("two DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT in a row but target different addresses");
                    }
                }
                break;
            case DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT:
                mappedAddr32 = (uint32_t*)mappedAddr;
                // to update a movw/movt pair we need to extract the 32-bit they will make,
                // add the adjust and write back the new movw/movt pair.
                if ( lastKind == kind ) {
                    if ( lastToNewAddress == toNewAddress ) {
                        uint32_t instruction1 = P::E::get32(*lastMappedAddr32);
                        uint32_t instruction2 = P::E::get32(*mappedAddr32);
                        if ( isArmMovw(instruction1) && isArmMovt(instruction2) ) {
                            uint16_t high = getArmWord(instruction2);
                            uint16_t low  = getArmWord(instruction1);
                            uint32_t full = high << 16 | low;
                            full += adjust;
                            instruction1 = setArmWord(instruction1, full & 0xFFFF);
                            instruction2 = setArmWord(instruction2, full >> 16);
                        }
                        else if ( isArmMovt(instruction1) && isArmMovw(instruction2) ) {
                            uint16_t high = getArmWord(instruction1);
                            uint16_t low  = getArmWord(instruction2);
                            uint32_t full = high << 16 | low;
                            full += adjust;
                            instruction2 = setArmWord(instruction2, full & 0xFFFF);
                            instruction1 = setArmWord(instruction1, full >> 16);
                        }
                        else {
                            kcgen_terminate("two DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT in a row but not");
                        }
                        P::E::set32(*lastMappedAddr32, instruction1);
                        P::E::set32(*mappedAddr32, instruction2);
                        kind = 0;
                    }
                    else {
                        kcgen_terminate("two DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT in a row but target different addresses");
                    }
                }
                break;
            case DYLD_CACHE_ADJ_V2_ARM64_BR26:
            case DYLD_CACHE_ADJ_V2_THUMB_BR22:
            case DYLD_CACHE_ADJ_V2_ARM_BR24:
                // nothing to do with calls to stubs
                break;
            default:
                kcgen_terminate("unknown split seg kind=%d", kind);
        }
        lastKind = kind;
        lastToNewAddress = toNewAddress;
        lastMappedAddr32 = mappedAddr32;
    }
    
    template <typename P>
    void Adjustor<P>::adjustReferencesUsingInfoV2(std::vector<void*>& pointersForASLR)
    {
        const uint8_t* infoStart = &_kextMachoPtr[_splitSegInfoCmd->dataoff()];
        const uint8_t* infoEnd = &infoStart[_splitSegInfoCmd->datasize()];
        if ( *infoStart++ != DYLD_CACHE_ADJ_V2_FORMAT ) {
            kcgen_terminate("malformed split seg info");
        }

        // build section arrays of slide and mapped address for each section
        std::vector<uint64_t> sectionSlides;
        std::vector<uint64_t> sectionNewAddress;
        std::vector<uint8_t*> sectionMappedAddress;
        sectionSlides.reserve(16);
        sectionNewAddress.reserve(16);
        sectionMappedAddress.reserve(16);
        // section index 0 refers to mach_header
        sectionMappedAddress.push_back((uint8_t*)_cacheBuffer + _segCacheOffsets[0]);
        sectionSlides.push_back(_segSlides[0]);
        sectionNewAddress.push_back(_segNewStartAddresses[0]);
        // section 1 and later refer to real sections
        unsigned sectionIndex = 1; // make section indexes printed here agree with what comes out of the trie
        for (unsigned segmentIndex=0; segmentIndex < _segCmds.size(); ++segmentIndex) {
            macho_segment_command<P>* segCmd = _segCmds[segmentIndex];
            macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
            macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
            for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
                sectionMappedAddress.push_back((uint8_t*)_cacheBuffer + _segCacheOffsets[segmentIndex] + sect->addr() - segCmd->vmaddr());
                sectionSlides.push_back(_segSlides[segmentIndex]);
                sectionNewAddress.push_back(_segNewStartAddresses[segmentIndex] + sect->addr() - segCmd->vmaddr());
                if (_log_ref) {
                    kcgen_log(" %s/%s, sectIndex=%d, mapped at=%llx new address=%llx section slide=%llx",
                              sect->segname(), sect->sectname(), sectionIndex, (uint64_t)sectionMappedAddress.back(), (uint64_t)sectionNewAddress.back(), (uint64_t)sectionSlides.back());
                }
                ++sectionIndex;
            }
        }

        // Whole         :== <count> FromToSection+
        // FromToSection :== <from-sect-index> <to-sect-index> <count> ToOffset+
        // ToOffset      :== <to-sect-offset-delta> <count> FromOffset+
        // FromOffset    :== <kind> <count> <from-sect-offset-delta>
        const uint8_t* p = infoStart;
        uint64_t sectionCount = read_uleb128(p, infoEnd);
        for (uint64_t i=0; i < sectionCount; ++i) {
            uint32_t* lastMappedAddr32 = NULL;
            uint32_t lastKind = 0;
            uint64_t lastToNewAddress = 0;
            uint64_t fromSectionIndex = read_uleb128(p, infoEnd);
            uint64_t toSectionIndex = read_uleb128(p, infoEnd);
            uint64_t toOffsetCount = read_uleb128(p, infoEnd);
            uint64_t fromSectionSlide = sectionSlides[fromSectionIndex];
            uint64_t fromSectionNewAddress = sectionNewAddress[fromSectionIndex];
            uint8_t* fromSectionMappedAddress = sectionMappedAddress[fromSectionIndex];
            uint64_t toSectionSlide = sectionSlides[toSectionIndex];
            uint64_t toSectionNewAddress = sectionNewAddress[toSectionIndex];
            if (_log_ref) {
                kcgen_verboseLog("from sect=%lld (mapped=%p), to sect=%lld (new addr=0x%llX)",
                                 fromSectionIndex, fromSectionMappedAddress, toSectionIndex, toSectionNewAddress);
            }
            uint64_t toSectionOffset = 0;
            for (uint64_t j=0; j < toOffsetCount; ++j) {
                uint64_t toSectionDelta = read_uleb128(p, infoEnd);
                uint64_t fromOffsetCount = read_uleb128(p, infoEnd);
                toSectionOffset += toSectionDelta;
                for (uint64_t k=0; k < fromOffsetCount; ++k) {
                    uint64_t kind = read_uleb128(p, infoEnd);
                    if ( kind > 12 ) {
                        kcgen_terminate("bad kind value");
                    }
                    uint64_t fromSectDeltaCount = read_uleb128(p, infoEnd);
                    uint64_t fromSectionOffset = 0;
                    for (uint64_t l=0; l < fromSectDeltaCount; ++l) {
                        uint64_t delta = read_uleb128(p, infoEnd);
                        fromSectionOffset += delta;
                        int64_t deltaAdjust = toSectionSlide - fromSectionSlide;
                        uint8_t* fromMappedAddr = fromSectionMappedAddress + fromSectionOffset;
                        uint64_t toNewAddress = toSectionNewAddress + toSectionOffset;
                        uint64_t fromNewAddress = fromSectionNewAddress + fromSectionOffset;
                        uint64_t imageStartAddress = sectionNewAddress.front();
                        uint64_t imageEndAddress = sectionNewAddress.back();
                        if ( toSectionIndex != 255 )
                            if (_log_ref) {
                                kcgen_verboseLog("kind:%d, mappedAddr:%p (ofst:0x%llX), fromNewAddr:%p (delta:%lld), "
                                                 "toNewAddr:%p (toDelta:%lld, toOfst:%lld), "
                                                 "start:0x%llX, end:0x%llX, toSectionSlide:0x%llx fromSectionSlide:0x%llx deltaAdjust:%lld",
                                                 (uint32_t)kind, (void *)fromMappedAddr, fromSectionOffset,
                                                 (void *)fromNewAddress, delta,
                                                 (void *)toNewAddress, toSectionDelta, toSectionOffset,
                                                 imageStartAddress, imageEndAddress, toSectionSlide, fromSectionSlide, deltaAdjust);
                            }
                            adjustReference((uint32_t)kind, fromMappedAddr, fromNewAddress, toNewAddress,
                                            deltaAdjust, toSectionSlide, imageStartAddress, imageEndAddress,
                                            pointersForASLR, lastMappedAddr32, lastKind, lastToNewAddress);
                    }
                }
            }
        }
    }

    template <typename P>
    void Adjustor<P>::adjustDataPointers(std::vector<void*>& pointersForASLR)
    {
        const uint8_t* p = &_kextMachoPtr[_dyldInfo->rebase_off()];
        const uint8_t* end = &p[_dyldInfo->rebase_size()];

        uint8_t type = 0;
        int segIndex = 0;
        uint64_t segOffset = 0;
        uint64_t count;
        uint64_t skip;
        bool done = false;
        while ( !done && (p < end) ) {
            uint8_t immediate = *p & REBASE_IMMEDIATE_MASK;
            uint8_t opcode = *p & REBASE_OPCODE_MASK;
            ++p;
            switch (opcode) {
                case REBASE_OPCODE_DONE:
                    done = true;
                    break;
                case REBASE_OPCODE_SET_TYPE_IMM:
                    type = immediate;
                    break;
                case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                    segIndex = immediate;
                    segOffset = read_uleb128(p, end);
                    break;
                case REBASE_OPCODE_ADD_ADDR_ULEB:
                    segOffset += read_uleb128(p, end);
                    break;
                case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
                    segOffset += immediate*sizeof(pint_t);
                    break;
                case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
                    for (int i=0; i < immediate; ++i) {
                        slidePointer(segIndex, segOffset, type, pointersForASLR);
                        segOffset += sizeof(pint_t);
                    }
                    break;
                case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
                    count = read_uleb128(p, end);
                    for (uint32_t i=0; i < count; ++i) {
                        slidePointer(segIndex, segOffset, type, pointersForASLR);
                        segOffset += sizeof(pint_t);
                    }
                    break;
                case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
                    slidePointer(segIndex, segOffset, type, pointersForASLR);
                    segOffset += read_uleb128(p, end) + sizeof(pint_t);
                    break;
                case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
                    count = read_uleb128(p, end);
                    skip = read_uleb128(p, end);
                    for (uint32_t i=0; i < count; ++i) {
                        slidePointer(segIndex, segOffset, type, pointersForASLR);
                        segOffset += skip + sizeof(pint_t);
                    }
                    break;
                default:
                    kcgen_terminate("unknown rebase opcode 0x%02X", opcode);
            }
        }
    }
    
    
    template <typename P>
    void Adjustor<P>::adjustInstruction(uint8_t kind, uint64_t cacheOffset, uint64_t codeToDataDelta)
    {
        uint8_t* fixupLoc = (uint8_t*)_cacheBuffer + cacheOffset;
        uint32_t* fixupLoc32 = (uint32_t*)fixupLoc;
        uint64_t* fixupLoc64 = (uint64_t*)fixupLoc;
        uint32_t instruction;
        uint32_t value32;
        uint64_t value64;
        
        switch (kind) {
            case 1:    // 32-bit pointer (including x86_64 RIP-rel)
                value32 = P::E::get32(*fixupLoc32);
                value32 += codeToDataDelta;
                P::E::set32(*fixupLoc32, value32);
                break;
            case 2: // 64-bit pointer
                value64 =  P::E::get64(*fixupLoc64);
                value64 += codeToDataDelta;
                P::E::set64(*fixupLoc64, value64);
                break;
            case 4:    // only used for i386, a reference to something in the IMPORT segment
                break;
            case 5: // used by thumb2 movw
                instruction = P::E::get32(*fixupLoc32);
                // slide is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
                value32 = (instruction & 0x0000000F) + ((uint32_t)codeToDataDelta >> 12);
                instruction = (instruction & 0xFFFFFFF0) | (value32 & 0x0000000F);
                P::E::set32(*fixupLoc32, instruction);
                break;
            case 6: // used by ARM movw
                instruction = P::E::get32(*fixupLoc32);
                // slide is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
                value32 = ((instruction & 0x000F0000) >> 16) + ((uint32_t)codeToDataDelta >> 12);
                instruction = (instruction & 0xFFF0FFFF) | ((value32 <<16) & 0x000F0000);
                P::E::set32(*fixupLoc32, instruction);
                break;
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E:
            case 0x1F:
                // used by thumb2 movt (low nibble of kind is high 4-bits of paired movw)
            {
                instruction = P::E::get32(*fixupLoc32);
                assert((instruction & 0x8000FBF0) == 0x0000F2C0);
                // extract 16-bit value from instruction
                uint32_t i     = ((instruction & 0x00000400) >> 10);
                uint32_t imm4  =  (instruction & 0x0000000F);
                uint32_t imm3  = ((instruction & 0x70000000) >> 28);
                uint32_t imm8  = ((instruction & 0x00FF0000) >> 16);
                uint32_t imm16 = (imm4 << 12) | (i << 11) | (imm3 << 8) | imm8;
                // combine with codeToDataDelta and kind nibble
                uint32_t targetValue = (imm16 << 16) | ((kind & 0xF) << 12);
                uint32_t newTargetValue = targetValue + (uint32_t)codeToDataDelta;
                // construct new bits slices
                uint32_t imm4_    = (newTargetValue & 0xF0000000) >> 28;
                uint32_t i_       = (newTargetValue & 0x08000000) >> 27;
                uint32_t imm3_    = (newTargetValue & 0x07000000) >> 24;
                uint32_t imm8_    = (newTargetValue & 0x00FF0000) >> 16;
                // update instruction to match codeToDataDelta 
                uint32_t newInstruction = (instruction & 0x8F00FBF0) | imm4_ | (i_ << 10) | (imm3_ << 28) | (imm8_ << 16);
                P::E::set32(*fixupLoc32, newInstruction);
            }
                break;
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23:
            case 0x24:
            case 0x25:
            case 0x26:
            case 0x27:
            case 0x28:
            case 0x29:
            case 0x2A:
            case 0x2B:
            case 0x2C:
            case 0x2D:
            case 0x2E:
            case 0x2F:
                // used by arm movt (low nibble of kind is high 4-bits of paired movw)
            {
                instruction = P::E::get32(*fixupLoc32);
                // extract 16-bit value from instruction
                uint32_t imm4 = ((instruction & 0x000F0000) >> 16);
                uint32_t imm12 = (instruction & 0x00000FFF);
                uint32_t imm16 = (imm4 << 12) | imm12;
                // combine with codeToDataDelta and kind nibble
                uint32_t targetValue = (imm16 << 16) | ((kind & 0xF) << 12);
                uint32_t newTargetValue = targetValue + (uint32_t)codeToDataDelta;
                // construct new bits slices
                uint32_t imm4_  = (newTargetValue & 0xF0000000) >> 28;
                uint32_t imm12_ = (newTargetValue & 0x0FFF0000) >> 16;
                // update instruction to match codeToDataDelta 
                uint32_t newInstruction = (instruction & 0xFFF0F000) | (imm4_ << 16) | imm12_;
                P::E::set32(*fixupLoc32, newInstruction);
            }
                break;
            case 3: // used for arm64 ADRP
                instruction = P::E::get32(*fixupLoc32);
                if ( (instruction & 0x9F000000) == 0x90000000 ) {
                    // codeToDataDelta is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
                    value64 = ((instruction & 0x60000000) >> 17) | ((instruction & 0x00FFFFE0) << 9);
                    value64 += codeToDataDelta;
                    instruction = (instruction & 0x9F00001F) | ((value64 << 17) & 0x60000000) | ((value64 >> 9) & 0x00FFFFE0);
                    P::E::set32(*fixupLoc32, instruction);
                }
                break;
            default:
                break;
        }
    }
    
    template <typename P>
    void Adjustor<P>::adjustCode()
    {
        // find compressed info on how code needs to be updated
        const uint8_t* infoStart = &_kextMachoPtr[_splitSegInfoCmd->dataoff()];
        const uint8_t* infoEnd = &infoStart[_splitSegInfoCmd->datasize()];;
        
        // This encoding only works if all data segments slide by the same amount
        uint64_t codeToDataDelta = _segSlides[1] - _segSlides[0];
        
        // compressed data is:  [ <kind> [uleb128-delta]+ <0> ] + <0>
        for (const uint8_t* p = infoStart; (*p != 0) && (p < infoEnd);) {
            uint8_t kind = *p++;
            uint64_t cacheOffset = _segCacheOffsets[0];
            while (uint64_t delta = read_uleb128(p, infoEnd)) {
                cacheOffset += delta;
                adjustInstruction(kind, cacheOffset, codeToDataDelta);
            }
        }
    }
    
    
    template <typename P>
    void Adjustor<P>::adjustExportsTrie(std::vector<uint8_t>& newTrieBytes)
    {
        // if no export info, nothing to adjust
        if ( _dyldInfo->export_size() == 0 )
            return;
        
        // since export info addresses are offsets from mach_header, everything in __TEXT is fine
        // only __DATA addresses need to be updated
        const uint8_t* start = &_kextMachoPtr[_dyldInfo->export_off()];
        const uint8_t* end = &start[_dyldInfo->export_size()];
        std::vector<ExportInfoTrie::Entry> originalExports;
        try {
            ExportInfoTrie::parseTrie(start, end, originalExports);
        }
        catch (const char* msg) {
            kcgen_terminate("parseTrie exception: %s", msg);
        }
        
        std::vector<ExportInfoTrie::Entry> newExports;
        newExports.reserve(originalExports.size());
        uint64_t baseAddress = _segOrigStartAddresses[0];
        uint64_t baseAddressSlide = slideForOrigAddress(baseAddress);
        for (auto& entry:  originalExports) {
            // remove symbols used by the static linker only
            if (   (strncmp(entry.name.c_str(), "$ld$", 4) == 0)
                || (strncmp(entry.name.c_str(), ".objc_class_name",16) == 0)
                || (strncmp(entry.name.c_str(), ".objc_category_name",19) == 0) ) {
                continue;
            }
            // adjust symbols in slid segments
            entry.info.address += (slideForOrigAddress(entry.info.address + baseAddress) - baseAddressSlide);
            newExports.push_back(entry);
        }
        
        // rebuild export trie
        newTrieBytes.reserve(_dyldInfo->export_size());
        
        ExportInfoTrie(newExports).emit(newTrieBytes);
        // align
        while ( (newTrieBytes.size() % sizeof(pint_t)) != 0 )
            newTrieBytes.push_back(0);
    }
    
    
    /*
     * kcgen::adjustKextSegmentLocations: main entry point for kcgen to adjust the segment layout of a kext.
     *
     * This function is intended to be called once per kext.  All the input arrays
     * should be of equal length, and a given index in each
     * array refers to the same dylib.
     *
     * Parameters:
     *  arch: Architecture for the kernel
     *
     *  pkbuffer: Pointer to the base of a memory region containing a completely
     *            laid out prelinked kernel including all kexts.
     *
     *  segCacheFileOffsets: The offset(s) of the kext's segement(s) relative to
     *                       'pkbuffer'. The first element in this vector
     *                       should point to the kext's MachO header.
     *
     *  segCacheFileSizes: The size(s) of the kext's segment(s).
     *
     *  segNewStartAddresses: The desired virtual addresses for each segment
     *                        in the kext.
     *
     *  pointersForASLR: [OUTPUT] returns a vector of pointers into pkbuffer
     *                   that need to be added to the slideInfo for ASLR.
     *
     * Return:
     *   0 == Success
     *  <0 == Failure
     */
    int adjustKextSegmentLocations(ArchPair arch, uint8_t *pkbuffer,
                                   const std::vector<uint64_t> &segCacheFileOffsets,
                                   const std::vector<uint64_t> &segCacheFileSizes,
                                   const std::vector<uint64_t> &segOrigStartAddresses,
                                   const std::vector<uint64_t> &segNewStartAddresses,
                                   std::vector<void*> &pointersForASLR)
    {
        void* mh = (void *)(pkbuffer + segCacheFileOffsets[0]);
        switch ( arch.arch ) {
            case CPU_TYPE_ARM:
            case CPU_TYPE_I386:
            {
                if ( LittleEndian::get32(*(uint32_t*)mh) != MH_MAGIC )
                    return -EINVAL;
                Adjustor< Pointer32<LittleEndian> > adjustor32(pkbuffer,
                                                               (macho_header< Pointer32<LittleEndian> > *)mh,
                                                               segOrigStartAddresses,
                                                               segNewStartAddresses,
                                                               segCacheFileOffsets,
                                                               segCacheFileSizes);
                adjustor32.adjustImageForNewSegmentLocations(pointersForASLR);
            }
                break;
            case CPU_TYPE_X86_64:
            case CPU_TYPE_ARM64:
            {
                if ( LittleEndian::get32(*(uint32_t*)mh) != MH_MAGIC_64 )
                    return -EINVAL;
                Adjustor< Pointer64<LittleEndian> > adjustor64(pkbuffer,
                                                               (macho_header< Pointer64<LittleEndian> > *)mh,
                                                               segOrigStartAddresses,
                                                               segNewStartAddresses,
                                                               segCacheFileOffsets,
                                                               segCacheFileSizes);
                adjustor64.adjustImageForNewSegmentLocations(pointersForASLR);
            }
                break;
            default:
                kcgen_terminate("unsupported arch 0x%08X", arch.arch);
        }
        return 0;
    }
    
} // kcgen namespace

/*
 * C interface to the above functionality
 */
extern "C"
int kcgen_adjustKextSegmentLocations(int cputype, int cpusubtype,
                                     uint8_t          *pkbuffer,
                                     int               numSegs,
                                     const uint64_t   *segCacheFileOffsets,
                                     const uint64_t   *segCacheFileSizes,
                                     const uint64_t   *segOrigStartAddresses,
                                     const uint64_t   *segNewStartAddresses,
                                     uintptr_t       **pointersForASLR,
                                     size_t           *numPointers)
{
    /* basic argument validation */
    if (!pkbuffer ||
        numSegs <= 0 ||
        !segCacheFileOffsets ||
        !segCacheFileSizes ||
        !segNewStartAddresses ||
        !pointersForASLR ||
        !numPointers) {
        return -EINVAL;
    }

//  for(int i=0; i < numSegs; i++) {
//     kcgen_verboseLog("[%d] segCacheFileOffset: %llx segCacheFileSizes: %llx segNewStartAddress: %llx", i, segCacheFileOffsets[i], segCacheFileSizes[i], segNewStartAddresses[i]);
//  }
    
    ArchPair arch(cputype, cpusubtype);
    std::vector<uint64_t> v_segCacheFileOffsets(segCacheFileOffsets, segCacheFileOffsets + numSegs);
    std::vector<uint64_t> v_segCacheFileSizes(segCacheFileSizes, segCacheFileSizes + numSegs);
    std::vector<uint64_t> v_segOrigStartAddresses(segOrigStartAddresses, segOrigStartAddresses + numSegs);
    std::vector<uint64_t> v_segNewStartAddresses(segNewStartAddresses, segNewStartAddresses + numSegs);
    
    /* output pointers */
    std::vector<void*> v_pointersForASLR;
    
    int ret = kcgen::adjustKextSegmentLocations(arch, pkbuffer, v_segCacheFileOffsets, v_segCacheFileSizes,
                                                v_segOrigStartAddresses, v_segNewStartAddresses, v_pointersForASLR);
    if (ret < 0)
        return ret;
    
    /* convert the stack std::vector into memory we can return to the caller */
    size_t npointers = v_pointersForASLR.size();
    if (npointers == 0)
        return 0;
    
    uintptr_t *pointers = (uintptr_t *)calloc(npointers, sizeof(uintptr_t));
    if (!pointers) {
        kcgen_terminate("unable to allocate memory for ASLR pointer adjustment return!");
    }
    
    int i = 0;
    for ( auto &p : v_pointersForASLR)
        pointers[i++] = (uintptr_t)p;
    
    *pointersForASLR = pointers;
    *numPointers = npointers;
    return 0;
}

extern "C"
void _kcgen_terminate(const char * __restrict format, ...)
{
    va_list argList;
    va_start(argList, format);

    OSKextVLog(/* kext */NULL,
               kOSKextLogErrorLevel | kOSKextLogGeneralFlag, format, argList);
    abort();
}

extern "C"
void _kcgen_warning(const char * __restrict format, ...)
{
    va_list argList;
    va_start(argList, format);

    OSKextVLog(/* kext */NULL,
               kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
               format, argList);
    va_end(argList);
}

extern "C"
void _kcgen_log(const char * __restrict format, ...)
{
    va_list argList;
    va_start(argList, format);

    OSKextVLog(/* kext */NULL,
               kOSKextLogBasicLevel | kOSKextLogGeneralFlag,
               format, argList);
    va_end(argList);
}

extern "C"
void _kcgen_verboseLog(const char * __restrict format, ...)
{
    va_list argList;
    va_start(argList, format);

    /* turn on kcgen_verboseLog() by using LinkFlag | PatchFlag */
    OSKextVLog(/* kext */NULL,
               kOSKextLogDebugLevel | kOSKextLogLinkFlag | kOSKextLogPatchFlag,
               format, argList);
    va_end(argList);
}
