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


#include <dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <assert.h>

#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "MachOFileAbstraction.hpp"
#include "Trie.hpp"
#include "DyldSharedCache.h"
#include "CacheBuilder.h"
#include "MachOLoaded.h"

#define ALIGN_AS_TYPE(value, type) \
        ((value + alignof(type) - 1) & (-alignof(type)))

namespace {

template <typename P>
class SortedStringPool
{
public:
    // add a string and symbol table entry index to be updated later
    void add(uint32_t symbolIndex, const char* symbolName) {
        _map[symbolName].push_back(symbolIndex);
    }

    // copy sorted strings to buffer and update all symbol's string offsets
    uint32_t copyPoolAndUpdateOffsets(char* dstStringPool, macho_nlist<P>* symbolTable) {
        // walk sorted list of strings
        dstStringPool[0] = '\0'; // tradition for start of pool to be empty string
        uint32_t poolOffset = 1;
        for (auto& entry : _map) {
            const std::string& symName = entry.first;
            // append string to pool
            strcpy(&dstStringPool[poolOffset], symName.c_str());
            //  set each string offset of each symbol using it
            for (uint32_t symbolIndex : entry.second) {
                symbolTable[symbolIndex].set_n_strx(poolOffset);
            }
            poolOffset += symName.size() + 1;
        }
        // return size of pool
        return poolOffset;
    }

    size_t size() {
        size_t size = 1;
        for (auto& entry : _map) {
            size += (entry.first.size() + 1);
        }
        return size;
    }


private:
    std::map<std::string, std::vector<uint32_t>> _map;
};


} // anonymous namespace


struct LocalSymbolInfo
{
    uint32_t    dylibOffset;
    uint32_t    nlistStartIndex;
    uint32_t    nlistCount;
};


template <typename P>
class LinkeditOptimizer {
public:
                    LinkeditOptimizer(void* cacheBuffer, macho_header<P>* mh, Diagnostics& diag);

    uint32_t        linkeditSize() { return _linkeditSize; }
    uint64_t        linkeditAddr() { return _linkeditAddr; }
    const char*     installName() { return _installName; }
    void            copyWeakBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyLazyBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyExportInfo(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyExportedSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex);
    void            copyImportedSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex);
    void            copyLocalSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex,
                                     bool redact, std::vector<LocalSymbolInfo>& localSymbolInfos,
                                     std::vector<macho_nlist<P>>& unmappedLocalSymbols, SortedStringPool<P>& localSymbolsStringPool);
    void            copyFunctionStarts(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyDataInCode(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyIndirectSymbolTable(uint8_t* newLinkEditContent, uint32_t& offset);
    void            updateLoadCommands(uint32_t linkeditStartOffset, uint64_t mergedLinkeditAddr, uint64_t newLinkeditSize,
                                       uint32_t sharedSymbolTableStartOffset, uint32_t sharedSymbolTableCount,
                                       uint32_t sharedSymbolStringsOffset, uint32_t sharedSymbolStringsSize);

    macho_header<P>*                        machHeader() { return _mh; }
    const std::vector<const char*>          getDownwardDependents() { return _downDependentPaths; }
    const std::vector<const char*>          getAllDependents() { return _allDependentPaths; }
    const std::vector<const char*>          getReExportPaths() { return _reExportPaths; }
    const std::vector<uint64_t>             initializerAddresses() { return _initializerAddresses; }
    const std::vector<macho_section<P>*>    dofSections() { return _dofSections; }
    uint32_t                                exportsTrieLinkEditOffset() { return _newExportInfoOffset; }
    uint32_t                                exportsTrieLinkEditSize() { return _exportInfoSize; }
    uint32_t                                weakBindingLinkEditOffset() { return _newWeakBindingInfoOffset; }
    uint32_t                                weakBindingLinkEditSize() { return _newWeakBindingSize; }
    uint64_t                                dyldSectionAddress() { return _dyldSectionAddr; }
    const std::vector<macho_segment_command<P>*>&  segCmds() { return _segCmds; }


    static void optimizeLinkedit(CacheBuilder& builder);
    static void mergeLinkedits(CacheBuilder& builder, std::vector<LinkeditOptimizer<P>*>& optimizers);

private:

    typedef typename P::uint_t pint_t;
    typedef typename P::E E;

    macho_header<P>*                        _mh;
    void*                                   _cacheBuffer;
    Diagnostics&                            _diagnostics;
    uint32_t                                _linkeditSize        = 0;
    uint64_t                                _linkeditAddr        = 0;
    const uint8_t*                          _linkeditBias       = nullptr;
    const char*                             _installName        = nullptr;
    macho_symtab_command<P>*                _symTabCmd          = nullptr;
    macho_dysymtab_command<P>*              _dynSymTabCmd       = nullptr;
    macho_dyld_info_command<P>*             _dyldInfo           = nullptr;
    macho_linkedit_data_command<P>*         _exportTrieCmd      = nullptr;
    macho_linkedit_data_command<P>*         _functionStartsCmd  = nullptr;
    macho_linkedit_data_command<P>*         _dataInCodeCmd      = nullptr;
    std::vector<macho_segment_command<P>*>  _segCmds;
    std::unordered_map<uint32_t,uint32_t>   _oldToNewSymbolIndexes;
    std::vector<const char*>                _reExportPaths;
    std::vector<const char*>                _downDependentPaths;
    std::vector<const char*>                _allDependentPaths;
    std::vector<uint64_t>                   _initializerAddresses;
    std::vector<macho_section<P>*>          _dofSections;
    uint32_t                                _newWeakBindingInfoOffset       = 0;
    uint32_t                                _newLazyBindingInfoOffset       = 0;
    uint32_t                                _newBindingInfoOffset           = 0;
    uint32_t                                _newExportInfoOffset            = 0;
    uint32_t                                _exportInfoSize                 = 0;
    uint32_t                                _newWeakBindingSize             = 0;
    uint32_t                                _newExportedSymbolsStartIndex   = 0;
    uint32_t                                _newExportedSymbolCount         = 0;
    uint32_t                                _newImportedSymbolsStartIndex   = 0;
    uint32_t                                _newImportedSymbolCount         = 0;
    uint32_t                                _newLocalSymbolsStartIndex      = 0;
    uint32_t                                _newLocalSymbolCount            = 0;
    uint32_t                                _newFunctionStartsOffset        = 0;
    uint32_t                                _newDataInCodeOffset            = 0;
    uint32_t                                _newIndirectSymbolTableOffset   = 0;
    uint64_t                                _dyldSectionAddr                = 0;
};



template <typename P>
LinkeditOptimizer<P>::LinkeditOptimizer(void* cacheBuffer, macho_header<P>* mh, Diagnostics& diag)
: _mh(mh), _cacheBuffer(cacheBuffer), _diagnostics(diag)
{
    const unsigned origLoadCommandsSize = mh->sizeofcmds();
    unsigned bytesRemaining = origLoadCommandsSize;
    unsigned removedCount = 0;
    uint64_t    textSegAddr = 0;
    int64_t     slide = 0;
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t cmdCount = mh->ncmds();
    const macho_load_command<P>* cmd = cmds;
    const macho_dylib_command<P>* dylibCmd;
    const macho_routines_command<P>* routinesCmd;
    macho_segment_command<P>* segCmd;
    for (uint32_t i = 0; i < cmdCount; ++i) {
        bool remove = false;
        switch (cmd->cmd()) {
            case LC_ID_DYLIB:
                _installName = ((macho_dylib_command<P>*)cmd)->name();
                break;
            case LC_SYMTAB:
                _symTabCmd = (macho_symtab_command<P>*)cmd;
                break;
            case LC_DYSYMTAB:
                _dynSymTabCmd = (macho_dysymtab_command<P>*)cmd;
                break;
            case LC_DYLD_INFO:
            case LC_DYLD_INFO_ONLY:
                _dyldInfo = (macho_dyld_info_command<P>*)cmd;
                _exportInfoSize = _dyldInfo->export_size();
                break;
            case LC_FUNCTION_STARTS:
                _functionStartsCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_DATA_IN_CODE:
                _dataInCodeCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_DYLD_EXPORTS_TRIE:
                _exportTrieCmd = (macho_linkedit_data_command<P>*)cmd;
                _exportInfoSize = _exportTrieCmd->datasize();
                break;
            case LC_ROUTINES:
            case LC_ROUTINES_64:
                routinesCmd = (macho_routines_command<P>*)cmd;
                _initializerAddresses.push_back(routinesCmd->init_address());
                break;
            case LC_REEXPORT_DYLIB:
            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB:
            case LC_LOAD_UPWARD_DYLIB:
                dylibCmd = (macho_dylib_command<P>*)cmd;
                _allDependentPaths.push_back(dylibCmd->name());
                if ( cmd->cmd() != LC_LOAD_UPWARD_DYLIB )
                    _downDependentPaths.push_back(dylibCmd->name());
                if ( cmd->cmd() == LC_REEXPORT_DYLIB )
                    _reExportPaths.push_back(dylibCmd->name());
               break;
            case macho_segment_command<P>::CMD:
                segCmd = (macho_segment_command<P>*)cmd;
                _segCmds.push_back(segCmd);
                if ( strcmp(segCmd->segname(), "__TEXT") == 0 ) {
                    textSegAddr = segCmd->vmaddr();
                    slide = (uint64_t)mh - textSegAddr;
                }
                else if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
                    _linkeditAddr        = segCmd->vmaddr();
                    _linkeditBias        = (uint8_t*)mh + (_linkeditAddr - textSegAddr) - segCmd->fileoff();
                    _linkeditSize        = (uint32_t)segCmd->vmsize();
                }
                else if ( segCmd->nsects() > 0 ) {
                    macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
                    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
                    for (macho_section<P>* sect=sectionsStart; sect < sectionsEnd; ++sect) {
                        const uint8_t type = sect->flags() & SECTION_TYPE;
                        if ( type == S_MOD_INIT_FUNC_POINTERS ) {
                            const pint_t* inits = (pint_t*)(sect->addr()+slide);
                            const size_t count = sect->size() / sizeof(pint_t);
                            for (size_t j=0; j < count; ++j) {
                                uint64_t func = P::getP(inits[j]);
                                _initializerAddresses.push_back(func);
                            }
                        }
                        else if ( type == S_INIT_FUNC_OFFSETS ) {
                            const uint32_t* inits = (uint32_t*)(sect->addr()+slide);
                            const size_t count = sect->size() / sizeof(uint32_t);
                            for (size_t j=0; j < count; ++j) {
                                uint32_t funcOffset = E::get32(inits[j]);
                                _initializerAddresses.push_back(textSegAddr + funcOffset);
                            }
                        }
                        else if ( type == S_DTRACE_DOF ) {
                            _dofSections.push_back(sect);
                        }
                        else if ( (strcmp(sect->sectname(), "__dyld") == 0) && (strncmp(sect->segname(), "__DATA", 6) == 0) ) {
                            _dyldSectionAddr = sect->addr();
                        }
                    }
                }
                break;
            case LC_DYLD_CHAINED_FIXUPS:
            case LC_SEGMENT_SPLIT_INFO:
                remove = true;
                break;
        }
        uint32_t cmdSize = cmd->cmdsize();
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
    mh->set_ncmds(cmdCount - removedCount);
    mh->set_sizeofcmds(origLoadCommandsSize - bytesRemaining);
}

/*
static void dumpLoadCommands(const uint8_t* mheader)
{
    const mach_header* const mh = (mach_header*)mheader;
    const uint32_t cmd_count = mh->ncmds;
    bool is64 = (mh->magic == MH_MAGIC_64);
    const load_command* cmds = (load_command*)(mheader + (is64 ? sizeof(mach_header_64) : sizeof(mach_header)));
    const load_command* cmd = cmds;
    const segment_command* segCmd;
    const segment_command_64* seg64Cmd;
    const symtab_command* symTab;
    const linkedit_data_command* leData;
    const uint8_t* linkEditBias = NULL;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        switch (cmd->cmd) {
            case LC_SEGMENT:
                segCmd = (const segment_command*)cmd;
                printf("LC_SEGMENT\n");
                printf("  segname  = %s\n",     segCmd->segname);
                printf("  vmaddr   = 0x%08X\n", segCmd->vmaddr);
                printf("  vmsize   = 0x%08X\n", segCmd->vmsize);
                printf("  fileoff  = 0x%08X\n", segCmd->fileoff);
                printf("  filesize = 0x%08X\n", segCmd->filesize);
                if ( strcmp(segCmd->segname, "__TEXT") == 0 ) {
                    linkEditBias = mheader - segCmd->fileoff;
                }
                 break;
            case LC_SEGMENT_64:
                seg64Cmd = (const segment_command_64*)cmd;
                printf("LC_SEGMENT_64\n");
                printf("  segname  = %s\n",        seg64Cmd->segname);
                printf("  vmaddr   = 0x%09llX\n",  seg64Cmd->vmaddr);
                printf("  vmsize   = 0x%09llX\n",  seg64Cmd->vmsize);
                printf("  fileoff  = 0x%09llX\n",  seg64Cmd->fileoff);
                printf("  filesize = 0x%09llX\n",  seg64Cmd->filesize);
                if ( strcmp(seg64Cmd->segname, "__TEXT") == 0 ) {
                    linkEditBias = mheader - seg64Cmd->fileoff;
                }
                break;
            case LC_SYMTAB:
                symTab = (const symtab_command*)cmd;
                printf("LC_SYMTAB\n");
                printf("  symoff   = 0x%08X\n", symTab->symoff);
                printf("  nsyms    = 0x%08X\n", symTab->nsyms);
                printf("  stroff   = 0x%08X\n", symTab->stroff);
                printf("  strsize  = 0x%08X\n", symTab->strsize);
                {
                const char* strPool = (char*)&linkEditBias[symTab->stroff];
                const nlist_64* sym0 = (nlist_64*)(&linkEditBias[symTab->symoff]);
                printf("    sym[0].n_strx = 0x%08X (%s)\n", sym0->n_un.n_strx, &strPool[sym0->n_un.n_strx]);
                printf("    sym[0].n_type = 0x%02X\n", sym0->n_type);
                printf("    sym[0].n_sect = 0x%02X\n", sym0->n_sect);
                printf("    sym[0].n_desc = 0x%04X\n", sym0->n_desc);
                printf("    sym[0].n_value = 0x%llX\n", sym0->n_value);
                const nlist_64* sym1 = (nlist_64*)(&linkEditBias[symTab->symoff+16]);
                printf("    sym[1].n_strx = 0x%08X (%s)\n", sym1->n_un.n_strx, &strPool[sym1->n_un.n_strx]);
                printf("    sym[1].n_type = 0x%02X\n", sym1->n_type);
                printf("    sym[1].n_sect = 0x%02X\n", sym1->n_sect);
                printf("    sym[1].n_desc = 0x%04X\n", sym1->n_desc);
                printf("    sym[1].n_value = 0x%llX\n", sym1->n_value);
                }
                 break;
            case LC_FUNCTION_STARTS:
                leData = (const linkedit_data_command*)cmd;
                printf("LC_FUNCTION_STARTS\n");
                printf("  dataoff  = 0x%08X\n", leData->dataoff);
                printf("  datasize = 0x%08X\n", leData->datasize);
            default:
                //printf("0x%08X\n", cmd->cmd);
                break;
        }
        cmd = (const load_command*)(((uint8_t*)cmd)+cmd->cmdsize);
    }
}
*/

template <typename P>
void LinkeditOptimizer<P>::updateLoadCommands(uint32_t mergedLinkeditStartOffset, uint64_t mergedLinkeditAddr, uint64_t newLinkeditSize,
                                              uint32_t sharedSymbolTableStartOffset, uint32_t sharedSymbolTableCount,
                                              uint32_t sharedSymbolStringsOffset, uint32_t sharedSymbolStringsSize)
{
    // update __LINKEDIT segment in all dylibs to overlap the same shared region
    for (macho_segment_command<P>* segCmd : _segCmds) {
        if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
            segCmd->set_vmaddr(mergedLinkeditAddr);
            segCmd->set_vmsize(newLinkeditSize);
            segCmd->set_fileoff(mergedLinkeditStartOffset);
            segCmd->set_filesize(newLinkeditSize);
        }
        else if ( strcmp(segCmd->segname(), "__TEXT") == 0 ) {
            // HACK until lldb fixed in: <rdar://problem/20357466> DynamicLoaderMacOSXDYLD fixes for Monarch dyld shared cache
            //segCmd->set_fileoff(0);

        }
   }

    // update symbol table to point to shared symbol table
    _symTabCmd->set_symoff(mergedLinkeditStartOffset + sharedSymbolTableStartOffset + _newLocalSymbolsStartIndex*sizeof(macho_nlist<P>));
    _symTabCmd->set_nsyms(_newLocalSymbolCount+_newExportedSymbolCount+_newImportedSymbolCount);
    _symTabCmd->set_stroff(mergedLinkeditStartOffset + sharedSymbolStringsOffset);
    _symTabCmd->set_strsize(sharedSymbolStringsSize);

    // update dynamic symbol table to have proper offsets into shared symbol table
    _dynSymTabCmd->set_ilocalsym(0);
    _dynSymTabCmd->set_nlocalsym(_newLocalSymbolCount);
    _dynSymTabCmd->set_iextdefsym(_newExportedSymbolsStartIndex-_newLocalSymbolsStartIndex);
    _dynSymTabCmd->set_nextdefsym(_newExportedSymbolCount);
    _dynSymTabCmd->set_iundefsym(_newImportedSymbolsStartIndex-_newLocalSymbolsStartIndex);
    _dynSymTabCmd->set_nundefsym(_newImportedSymbolCount);
    _dynSymTabCmd->set_tocoff(0);
    _dynSymTabCmd->set_ntoc(0);
    _dynSymTabCmd->set_modtaboff(0);
    _dynSymTabCmd->set_nmodtab(0);
    _dynSymTabCmd->set_indirectsymoff(mergedLinkeditStartOffset + _newIndirectSymbolTableOffset);
    _dynSymTabCmd->set_extreloff(0);
    _dynSymTabCmd->set_locreloff(0);
    _dynSymTabCmd->set_nlocrel(0);

    // update dyld info
    if ( _dyldInfo != nullptr ) {
        _dyldInfo->set_rebase_off(0);
        _dyldInfo->set_rebase_size(0);
        _dyldInfo->set_bind_off(_dyldInfo->bind_size() ?  mergedLinkeditStartOffset + _newBindingInfoOffset : 0);
        _dyldInfo->set_weak_bind_off(_dyldInfo->weak_bind_size() ?  mergedLinkeditStartOffset + _newWeakBindingInfoOffset : 0 );
        _dyldInfo->set_lazy_bind_off(_dyldInfo->lazy_bind_size() ?  mergedLinkeditStartOffset + _newLazyBindingInfoOffset : 0 );
        _dyldInfo->set_export_off(mergedLinkeditStartOffset + _newExportInfoOffset);
    } else if ( _exportTrieCmd != nullptr ) {
        _exportTrieCmd->set_dataoff(mergedLinkeditStartOffset + _newExportInfoOffset);
    }

    // update function-starts
    if ( _functionStartsCmd != nullptr )
        _functionStartsCmd->set_dataoff(mergedLinkeditStartOffset+_newFunctionStartsOffset);

    // update data-in-code
    if ( _dataInCodeCmd != nullptr )
        _dataInCodeCmd->set_dataoff(mergedLinkeditStartOffset+_newDataInCodeOffset);
}

template <typename P>
void LinkeditOptimizer<P>::copyWeakBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dyldInfo == nullptr )
        return;
    unsigned size = _dyldInfo->weak_bind_size();
    if ( size != 0 ) {
        ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dyldInfo->weak_bind_off()], size);
        _newWeakBindingInfoOffset = offset;
        _newWeakBindingSize = size;
        offset += size;
    }
}


template <typename P>
void LinkeditOptimizer<P>::copyLazyBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dyldInfo == nullptr )
        return;
    unsigned size = _dyldInfo->lazy_bind_size();
    if ( size != 0 ) {
        ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dyldInfo->lazy_bind_off()], size);
        _newLazyBindingInfoOffset = offset;
        offset += size;
    }
}

template <typename P>
void LinkeditOptimizer<P>::copyBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dyldInfo == nullptr )
        return;
    unsigned size = _dyldInfo->bind_size();
    if ( size != 0 ) {
        ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dyldInfo->bind_off()], size);
        _newBindingInfoOffset = offset;
        offset += size;
    }
}

template <typename P>
void LinkeditOptimizer<P>::copyExportInfo(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( (_dyldInfo == nullptr) && (_exportTrieCmd == nullptr) )
        return;

    uint32_t exportOffset = _exportTrieCmd ? _exportTrieCmd->dataoff() : _dyldInfo->export_off();
    uint32_t exportSize   = _exportTrieCmd ? _exportTrieCmd->datasize() : _dyldInfo->export_size();
    if ( exportSize != 0 ) {
        ::memcpy(&newLinkEditContent[offset], &_linkeditBias[exportOffset], exportSize);
        _newExportInfoOffset = offset;
        offset += exportSize;
    }
}


template <typename P>
void LinkeditOptimizer<P>::copyFunctionStarts(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _functionStartsCmd == nullptr )
        return;
    unsigned size = _functionStartsCmd->datasize();
    ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_functionStartsCmd->dataoff()], size);
    _newFunctionStartsOffset = offset;
    offset += size;
}

template <typename P>
void LinkeditOptimizer<P>::copyDataInCode(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dataInCodeCmd == nullptr )
        return;
    unsigned size = _dataInCodeCmd->datasize();
    ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dataInCodeCmd->dataoff()], size);
    _newDataInCodeOffset = offset;
    offset += size;
}


template <typename P>
void LinkeditOptimizer<P>::copyLocalSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex,
                                            bool redact, std::vector<LocalSymbolInfo>& localSymbolInfos,
                                            std::vector<macho_nlist<P>>& unmappedLocalSymbols, SortedStringPool<P>& localSymbolsStringPool)
{
    LocalSymbolInfo localInfo;
    localInfo.dylibOffset = (uint32_t)(((uint8_t*)_mh) - (uint8_t*)_cacheBuffer);
    localInfo.nlistStartIndex = (uint32_t)unmappedLocalSymbols.size();
    localInfo.nlistCount = 0;
    _newLocalSymbolsStartIndex = symbolIndex;
    const char* strings = (char*)&_linkeditBias[_symTabCmd->stroff()];
    const macho_nlist<P>* const symbolTable = (macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
    const macho_nlist<P>* const firstExport = &symbolTable[_dynSymTabCmd->ilocalsym()];
    const macho_nlist<P>* const lastExport  = &symbolTable[_dynSymTabCmd->ilocalsym()+_dynSymTabCmd->nlocalsym()];
    for (const macho_nlist<P>* entry = firstExport; entry < lastExport; ++entry) {
        if ( (entry->n_type() & N_TYPE) != N_SECT)
            continue;
         if ( (entry->n_type() & N_STAB) != 0)
            continue;
        const char* name = &strings[entry->n_strx()];
        macho_nlist<P>* newSymbolEntry = (macho_nlist<P>*)&newLinkEditContent[offset];
        *newSymbolEntry = *entry;
        if ( redact ) {
            // if removing local symbols, change __text symbols to "<redacted>" so backtraces don't have bogus names
            if ( entry->n_sect() == 1 ) {
                stringPool.add(symbolIndex, "<redacted>");
                ++symbolIndex;
                offset += sizeof(macho_nlist<P>);
            }
            // copy local symbol to unmmapped locals area
            localSymbolsStringPool.add((uint32_t)unmappedLocalSymbols.size(), name);
            unmappedLocalSymbols.push_back(*entry);
            unmappedLocalSymbols.back().set_n_strx(0);
        }
        else {
            stringPool.add(symbolIndex, name);
            ++symbolIndex;
            offset += sizeof(macho_nlist<P>);
       }
    }
    _newLocalSymbolCount = symbolIndex - _newLocalSymbolsStartIndex;
    localInfo.nlistCount = (uint32_t)unmappedLocalSymbols.size() - localInfo.nlistStartIndex;
    localSymbolInfos.push_back(localInfo);
}


template <typename P>
void LinkeditOptimizer<P>::copyExportedSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex)
{
    _newExportedSymbolsStartIndex = symbolIndex;
    const char* strings = (char*)&_linkeditBias[_symTabCmd->stroff()];
    const macho_nlist<P>* const symbolTable = (macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
    const macho_nlist<P>* const firstExport = &symbolTable[_dynSymTabCmd->iextdefsym()];
    const macho_nlist<P>* const lastExport  = &symbolTable[_dynSymTabCmd->iextdefsym()+_dynSymTabCmd->nextdefsym()];
    uint32_t oldSymbolIndex = _dynSymTabCmd->iextdefsym();
    for (const macho_nlist<P>* entry = firstExport; entry < lastExport; ++entry, ++oldSymbolIndex) {
        if ( (entry->n_type() & N_TYPE) != N_SECT)
            continue;
        const char* name = &strings[entry->n_strx()];
        if ( strncmp(name, ".objc_", 6) == 0 )
            continue;
        if ( strncmp(name, "$ld$", 4) == 0 )
            continue;
        macho_nlist<P>* newSymbolEntry = (macho_nlist<P>*)&newLinkEditContent[offset];
        *newSymbolEntry = *entry;
        newSymbolEntry->set_n_strx(0);
        stringPool.add(symbolIndex, name);
        _oldToNewSymbolIndexes[oldSymbolIndex] = symbolIndex - _newLocalSymbolsStartIndex;
        ++symbolIndex;
        offset += sizeof(macho_nlist<P>);
    }
    _newExportedSymbolCount = symbolIndex - _newExportedSymbolsStartIndex;
}

template <typename P>
void LinkeditOptimizer<P>::copyImportedSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex)
{
    _newImportedSymbolsStartIndex = symbolIndex;
    const char* strings = (char*)&_linkeditBias[_symTabCmd->stroff()];
    const macho_nlist<P>* const symbolTable = (macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
    const macho_nlist<P>* const firstImport = &symbolTable[_dynSymTabCmd->iundefsym()];
    const macho_nlist<P>* const lastImport  = &symbolTable[_dynSymTabCmd->iundefsym()+_dynSymTabCmd->nundefsym()];
    uint32_t oldSymbolIndex = _dynSymTabCmd->iundefsym();
    for (const macho_nlist<P>* entry = firstImport; entry < lastImport; ++entry, ++oldSymbolIndex) {
        if ( (entry->n_type() & N_TYPE) != N_UNDF)
            continue;
        const char* name = &strings[entry->n_strx()];
        macho_nlist<P>* newSymbolEntry = (macho_nlist<P>*)&newLinkEditContent[offset];
        *newSymbolEntry = *entry;
        newSymbolEntry->set_n_strx(0);
        stringPool.add(symbolIndex, name);
        _oldToNewSymbolIndexes[oldSymbolIndex] = symbolIndex - _newLocalSymbolsStartIndex;
        ++symbolIndex;
        offset += sizeof(macho_nlist<P>);
    }
    _newImportedSymbolCount = symbolIndex - _newImportedSymbolsStartIndex;
}

template <typename P>
void LinkeditOptimizer<P>::copyIndirectSymbolTable(uint8_t* newLinkEditContent, uint32_t& offset)
{
    _newIndirectSymbolTableOffset = offset;
     const uint32_t* const indirectTable = (uint32_t*)&_linkeditBias[_dynSymTabCmd->indirectsymoff()];
    uint32_t* newIndirectTable = (uint32_t*)&newLinkEditContent[offset];
    for (uint32_t i=0; i < _dynSymTabCmd->nindirectsyms(); ++i) {
        uint32_t symbolIndex = E::get32(indirectTable[i]);
        if ( (symbolIndex == INDIRECT_SYMBOL_ABS) || (symbolIndex == INDIRECT_SYMBOL_LOCAL) )
            E::set32(newIndirectTable[i], symbolIndex);
        else
            E::set32(newIndirectTable[i], _oldToNewSymbolIndexes[symbolIndex]);
        offset += sizeof(uint32_t);
    }
}

template <typename P>
void LinkeditOptimizer<P>::mergeLinkedits(CacheBuilder& builder, std::vector<LinkeditOptimizer<P>*>& optimizers)
{
    // allocate space for new linkedit data
    uint64_t totalUnoptLinkeditsSize = builder._readOnlyRegion.sizeInUse - builder._nonLinkEditReadOnlySize;
    uint8_t* newLinkEdit = (uint8_t*)calloc(totalUnoptLinkeditsSize, 1);
    SortedStringPool<P> stringPool;
    uint32_t offset = 0;

    builder._diagnostics.verbose("Merged LINKEDIT:\n");

    // copy weak binding info
    uint32_t startWeakBindInfosOffset = offset;
    for (LinkeditOptimizer<P>* op : optimizers) {
        // Skip chained fixups as the in-place linked list isn't valid any more
        const dyld3::MachOFile* mf = (dyld3::MachOFile*)op->machHeader();
        if (!mf->hasChainedFixups())
            op->copyWeakBindingInfo(newLinkEdit, offset);
    }
    builder._diagnostics.verbose("  weak bindings size:      %5uKB\n", (uint32_t)(offset-startWeakBindInfosOffset)/1024);

    // copy export info
    uint32_t startExportInfosOffset = offset;
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyExportInfo(newLinkEdit, offset);
    }
    builder._diagnostics.verbose("  exports info size:       %5uKB\n", (uint32_t)(offset-startExportInfosOffset)/1024);

    // in theory, an optimized cache can drop the binding info
    if ( true ) {
        // copy binding info
        uint32_t startBindingsInfosOffset = offset;
        for (LinkeditOptimizer<P>* op : optimizers) {
            // Skip chained fixups as the in-place linked list isn't valid any more
            const dyld3::MachOFile* mf = (dyld3::MachOFile*)op->machHeader();
            if (!mf->hasChainedFixups())
                op->copyBindingInfo(newLinkEdit, offset);
        }
        builder._diagnostics.verbose("  bindings size:           %5uKB\n", (uint32_t)(offset-startBindingsInfosOffset)/1024);

       // copy lazy binding info
        uint32_t startLazyBindingsInfosOffset = offset;
        for (LinkeditOptimizer<P>* op : optimizers) {
            // Skip chained fixups as the in-place linked list isn't valid any more
            const dyld3::MachOFile* mf = (dyld3::MachOFile*)op->machHeader();
            if (!mf->hasChainedFixups())
                op->copyLazyBindingInfo(newLinkEdit, offset);
        }
        builder._diagnostics.verbose("  lazy bindings size:      %5uKB\n", (offset-startLazyBindingsInfosOffset)/1024);
    }

    // copy symbol table entries
    std::vector<macho_nlist<P>> unmappedLocalSymbols;
    if ( builder._options.excludeLocalSymbols )
        unmappedLocalSymbols.reserve(0x01000000);
    std::vector<LocalSymbolInfo> localSymbolInfos;
        localSymbolInfos.reserve(optimizers.size());
    SortedStringPool<P> localSymbolsStringPool;
    uint32_t symbolIndex = 0;
    const uint32_t sharedSymbolTableStartOffset = offset;
    uint32_t sharedSymbolTableExportsCount = 0;
    uint32_t sharedSymbolTableImportsCount = 0;
    for (LinkeditOptimizer<P>* op : optimizers) {
         op->copyLocalSymbols(newLinkEdit, stringPool, offset, symbolIndex, builder._options.excludeLocalSymbols,
                             localSymbolInfos, unmappedLocalSymbols, localSymbolsStringPool);
        uint32_t x = symbolIndex;
        op->copyExportedSymbols(newLinkEdit, stringPool, offset, symbolIndex);
        sharedSymbolTableExportsCount += (symbolIndex-x);
        uint32_t y = symbolIndex;
        op->copyImportedSymbols(newLinkEdit, stringPool, offset, symbolIndex);
        sharedSymbolTableImportsCount += (symbolIndex-y);
    }
    uint32_t sharedSymbolTableCount = symbolIndex;
    const uint32_t sharedSymbolTableEndOffset = offset;

    // copy function starts
    uint32_t startFunctionStartsOffset = offset;
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyFunctionStarts(newLinkEdit, offset);
    }
    builder._diagnostics.verbose("  function starts size:    %5uKB\n", (offset-startFunctionStartsOffset)/1024);

    // copy data-in-code info
    uint32_t startDataInCodeOffset = offset;
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyDataInCode(newLinkEdit, offset);
    }
    builder._diagnostics.verbose("  data in code size:       %5uKB\n", (offset-startDataInCodeOffset)/1024);

    // copy indirect symbol tables
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyIndirectSymbolTable(newLinkEdit, offset);
    }
    // if indirect table has odd number of entries, end will not be 8-byte aligned
    if ( (offset % sizeof(typename P::uint_t)) != 0 )
        offset += 4;

    // copy string pool
    uint32_t sharedSymbolStringsOffset = offset;
    uint32_t sharedSymbolStringsSize = stringPool.copyPoolAndUpdateOffsets((char*)&newLinkEdit[sharedSymbolStringsOffset], (macho_nlist<P>*)&newLinkEdit[sharedSymbolTableStartOffset]);
    offset += sharedSymbolStringsSize;
    uint32_t newLinkeditUnalignedSize = offset;
    uint64_t newLinkeditAlignedSize = align(offset, 14);
    builder._diagnostics.verbose("  symbol table size:       %5uKB (%d exports, %d imports)\n", (sharedSymbolTableEndOffset-sharedSymbolTableStartOffset)/1024, sharedSymbolTableExportsCount, sharedSymbolTableImportsCount);
    builder._diagnostics.verbose("  symbol string pool size: %5uKB\n", sharedSymbolStringsSize/1024);
    builder._sharedStringsPoolVmOffset = (uint32_t)((builder._readOnlyRegion.unslidLoadAddress - builder._readExecuteRegion.unslidLoadAddress) + builder._nonLinkEditReadOnlySize + sharedSymbolStringsOffset);

    // overwrite mapped LINKEDIT area in cache with new merged LINKEDIT content
    builder._diagnostics.verbose("LINKEDITS optimized from %uMB to %uMB\n", (uint32_t)totalUnoptLinkeditsSize/(1024*1024), (uint32_t)newLinkeditUnalignedSize/(1024*1024));
    ::memcpy(builder._readOnlyRegion.buffer+builder._nonLinkEditReadOnlySize, newLinkEdit, newLinkeditAlignedSize);
    ::free(newLinkEdit);
    builder._readOnlyRegion.sizeInUse = builder._nonLinkEditReadOnlySize + newLinkeditAlignedSize;

    // overwrite end of un-opt linkedits to create a new unmapped region for local symbols
    if ( builder._options.excludeLocalSymbols ) {
        const uint32_t entriesOffset = sizeof(dyld_cache_local_symbols_info);
        const uint32_t entriesCount  = (uint32_t)localSymbolInfos.size();
        const uint32_t nlistOffset   = (uint32_t)align(entriesOffset + entriesCount * sizeof(dyld_cache_local_symbols_info), 4); // 16-byte align start
        const uint32_t nlistCount    = (uint32_t)unmappedLocalSymbols.size();
        const uint32_t stringsSize   = (uint32_t)localSymbolsStringPool.size();
        const uint32_t stringsOffset = nlistOffset + nlistCount * sizeof(macho_nlist<P>);
        // allocate buffer for local symbols
        const size_t localsBufferSize = align(stringsOffset + stringsSize, 14);
        vm_address_t localsBuffer;
        if ( ::vm_allocate(mach_task_self(), &localsBuffer, localsBufferSize, VM_FLAGS_ANYWHERE) == 0 ) {
            dyld_cache_local_symbols_info* infoHeader = (dyld_cache_local_symbols_info*)localsBuffer;
            // fill in header info
            infoHeader->nlistOffset       = nlistOffset;
            infoHeader->nlistCount        = nlistCount;
            infoHeader->stringsOffset     = stringsOffset;
            infoHeader->stringsSize       = stringsSize;
            infoHeader->entriesOffset     = entriesOffset;
            infoHeader->entriesCount      = entriesCount;
            // copy info for each dylib
            dyld_cache_local_symbols_entry* entries = (dyld_cache_local_symbols_entry*)(((uint8_t*)infoHeader)+entriesOffset);
            for (uint32_t i=0; i < entriesCount; ++i) {
                entries[i].dylibOffset        = localSymbolInfos[i].dylibOffset;
                entries[i].nlistStartIndex    = localSymbolInfos[i].nlistStartIndex;
                entries[i].nlistCount         = localSymbolInfos[i].nlistCount;
            }
            // copy nlists
            macho_nlist<P>* newLocalsSymbolTable = (macho_nlist<P>*)(localsBuffer+nlistOffset);
            ::memcpy(newLocalsSymbolTable, &unmappedLocalSymbols[0], nlistCount*sizeof(macho_nlist<P>));
            // copy string pool
            localSymbolsStringPool.copyPoolAndUpdateOffsets(((char*)infoHeader)+stringsOffset, newLocalsSymbolTable);
            // update cache header
            DyldSharedCache* cacheHeader = (DyldSharedCache*)builder._readExecuteRegion.buffer;
            cacheHeader->header.localSymbolsSize    = localsBufferSize;
            // return buffer of local symbols, caller to free() it
            builder._localSymbolsRegion.buffer      = (uint8_t*)localsBuffer;
            builder._localSymbolsRegion.bufferSize  = localsBufferSize;
            builder._localSymbolsRegion.sizeInUse   = localsBufferSize;
        }
        else {
            builder._diagnostics.warning("could not allocate local symbols");
        }
    }

    // update all load commands to new merged layout
    uint64_t linkeditsUnslidStartAddr = builder._readOnlyRegion.unslidLoadAddress + builder._nonLinkEditReadOnlySize;
    uint32_t linkeditsCacheFileOffset = (uint32_t)(builder._readOnlyRegion.cacheFileOffset + builder._nonLinkEditReadOnlySize);
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->updateLoadCommands(linkeditsCacheFileOffset, linkeditsUnslidStartAddr, newLinkeditUnalignedSize,
                               sharedSymbolTableStartOffset, sharedSymbolTableCount,
                               sharedSymbolStringsOffset, sharedSymbolStringsSize);
    }
}


template <typename P>
void LinkeditOptimizer<P>::optimizeLinkedit(CacheBuilder& builder)
{
    DyldSharedCache* cache = (DyldSharedCache*)builder._readExecuteRegion.buffer;
    // construct a LinkeditOptimizer for each image
    __block std::vector<LinkeditOptimizer<P>*> optimizers;
    cache->forEachImage(^(const mach_header* mh, const char*) {
        optimizers.push_back(new LinkeditOptimizer<P>(cache, (macho_header<P>*)mh, builder._diagnostics));
    });
#if 0
    // add optimizer for each branch pool
    for (uint64_t poolOffset : branchPoolOffsets) {
        macho_header<P>* mh =  (macho_header<P>*)((char*)cache + poolOffset);
        optimizers.push_back(new LinkeditOptimizer<P>(cache, mh, diag));
    }
#endif
    // merge linkedit info
    mergeLinkedits(builder, optimizers);

    // delete optimizers
    for (LinkeditOptimizer<P>* op : optimizers)
        delete op;
}

void CacheBuilder::optimizeLinkedit()
{
    if ( _is64 ) {
        return LinkeditOptimizer<Pointer64<LittleEndian>>::optimizeLinkedit(*this);
    }
    else {
         return LinkeditOptimizer<Pointer32<LittleEndian>>::optimizeLinkedit(*this);
    }
}



