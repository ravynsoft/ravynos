/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <vector>
#include <tuple>
#include <set>
#include <unordered_set>

#include "configure.h"
#include "MachOFileAbstraction.hpp"
#include "Architectures.hpp"
#include "MachOTrie.hpp"
#include "../ld/code-sign-blobs/superblob.h"

static bool printRebase = false;
static bool printBind = false;
static bool printWeakBind = false;
static bool printLazyBind = false;
static bool printOpcodes = false;
static bool printExport = false;
static bool printExportGraph = false;
static bool printExportNodes = false;
static bool printSharedRegion = false;
static bool printFunctionStarts = false;
static bool printDylibs = false;
static bool printDRs = false;
static bool printDataCode = false;
static cpu_type_t	sPreferredArch = 0;
static cpu_type_t	sPreferredSubArch = 0;


__attribute__((noreturn))
static void throwf(const char* format, ...) 
{
	va_list	list;
	char*	p;
	va_start(list, format);
	vasprintf(&p, format, list);
	va_end(list);
	
	const char*	t = p;
	throw t;
}


template <typename A>
class DyldInfoPrinter
{
public:
	static bool									validFile(const uint8_t* fileContent);
	static DyldInfoPrinter<A>*					make(const uint8_t* fileContent, uint32_t fileLength, const char* path, bool printArch) 
														{ return new DyldInfoPrinter<A>(fileContent, fileLength, path, printArch); }
	virtual										~DyldInfoPrinter() {}


private:
	typedef typename A::P					P;
	typedef typename A::P::E				E;
	typedef typename A::P::uint_t			pint_t;
	
												DyldInfoPrinter(const uint8_t* fileContent, uint32_t fileLength, const char* path, bool printArch);
	void										printRebaseInfo();
	void										printRebaseInfoOpcodes();
	void										printBindingInfo();
	void										printWeakBindingInfo();
	void										printLazyBindingInfo();
	void										printBindingInfoOpcodes(bool weakBinding);
	void										printWeakBindingInfoOpcodes();
	void										printLazyBindingOpcodes();
	void										printExportInfo();
	void										printExportInfoGraph();
	void										printExportInfoNodes();
	void										printRelocRebaseInfo();
	void										printSymbolTableExportInfo();
	void										printClassicLazyBindingInfo();
	void										printClassicBindingInfo();
	void										printSharedRegionInfo();
	const char*									sharedRegionKindName(uint8_t kind);
	void										printFunctionStartsInfo();
	void										printDylibsInfo();
	void										printDRInfo();
	void										printDataInCode();
	void										printFunctionStartLine(uint64_t addr);
	const uint8_t*								printSharedRegionV1InfoForEachULEB128Address(const uint8_t* p, const uint8_t* end, uint8_t kind);
	const uint8_t*								printSharedRegionV2InfoForEachULEB128Address(const uint8_t* p, const uint8_t* end);
	const uint8_t*								printSharedRegionV2InfoForEachULEB128AddressAndAdj(const uint8_t* p, const uint8_t* end);
	const uint8_t*								printSharedRegionV2SectionPair(const uint8_t* p, const uint8_t* end);
	const uint8_t*								printSharedRegionV2ToSectionOffset(const uint8_t* p, const uint8_t* end);
	const uint8_t*								printSharedRegionV2Kind(const uint8_t* p, const uint8_t* end);

	pint_t										localRelocBase();
	pint_t										externalRelocBase();
	const char*									relocTypeName(uint8_t r_type);
	uint8_t										segmentIndexForAddress(pint_t addr);
	void										processExportGraphNode(const uint8_t* const start, const uint8_t* const end,  
																	const uint8_t* parent, const uint8_t* p,
																	char* cummulativeString, int curStrOffset);
	void										gatherNodeStarts(const uint8_t* const start, const uint8_t* const end,  
																const uint8_t* parent, const uint8_t* p,
																std::vector<uint32_t>& nodeStarts);
	const char*									rebaseTypeName(uint8_t type);
	const char*									bindTypeName(uint8_t type);
	pint_t										segStartAddress(uint8_t segIndex);
	const char*									segmentName(uint8_t segIndex);
	const char*									sectionName(uint8_t segIndex, pint_t address);
	const char*									getSegAndSectName(uint8_t segIndex, pint_t address);
	const char*									ordinalName(int libraryOrdinal);
	const char*									classicOrdinalName(int libraryOrdinal);
	pint_t*										mappedAddressForVMAddress(pint_t vmaddress);
	const char*									symbolNameForAddress(uint64_t);
	const char*									closestSymbolNameForAddress(uint64_t addr, uint64_t* offset, uint8_t sectIndex=0);

		
	const char*									fPath;
	const macho_header<P>*						fHeader;
	uint64_t									fLength;
	const char*									fStrings;
	const char*									fStringsEnd;
	const macho_nlist<P>*						fSymbols;
	uint32_t									fSymbolCount;
	const macho_dyld_info_command<P>*			fInfo;
	const macho_linkedit_data_command<P>*		fSharedRegionInfo;
	const macho_linkedit_data_command<P>*		fFunctionStartsInfo;
	const macho_linkedit_data_command<P>*		fDataInCode;
	const macho_linkedit_data_command<P>*		fDRInfo;
	uint64_t									fBaseAddress;
	const macho_dysymtab_command<P>*			fDynamicSymbolTable;
	const macho_segment_command<P>*				fFirstSegment;
	const macho_segment_command<P>*				fFirstWritableSegment;
	bool										fWriteableSegmentWithAddrOver4G;
	std::vector<const macho_segment_command<P>*>fSegments;
	std::vector<const macho_section<P>*>		fSections;
	std::vector<const char*>					fDylibs;
	std::vector<const macho_dylib_command<P>*>	fDylibLoadCommands;
	macho_section<P>							fMachHeaderPseudoSection;
};



template <>
bool DyldInfoPrinter<ppc>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_POWERPC )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_DYLIB_STUB:
		case MH_BUNDLE:
		case MH_DYLINKER:
			return true;
	}
	return false;
}

template <>
bool DyldInfoPrinter<ppc64>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC_64 )
		return false;
	if ( header->cputype() != CPU_TYPE_POWERPC64 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_DYLIB_STUB:
		case MH_BUNDLE:
		case MH_DYLINKER:
			return true;
	}
	return false;
}

template <>
bool DyldInfoPrinter<x86>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_I386 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_DYLIB_STUB:
		case MH_BUNDLE:
		case MH_DYLINKER:
			return true;
	}
	return false;
}

template <>
bool DyldInfoPrinter<x86_64>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC_64 )
		return false;
	if ( header->cputype() != CPU_TYPE_X86_64 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_DYLIB_STUB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_KEXT_BUNDLE:
			return true;
	}
	return false;
}

#if SUPPORT_ARCH_arm_any
template <>
bool DyldInfoPrinter<arm>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_DYLIB_STUB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_KEXT_BUNDLE:
			return true;
	}
	return false;
}
#endif

#if SUPPORT_ARCH_arm64
template <>
bool DyldInfoPrinter<arm64>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC_64 )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM64 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_DYLIB_STUB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_KEXT_BUNDLE:
		case MH_PRELOAD:
			return true;
		default:
			return false;
	}
	return false;
}
#endif

#if SUPPORT_ARCH_arm64_32
template <>
bool DyldInfoPrinter<arm64_32>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM64_32 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_DYLIB_STUB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_KEXT_BUNDLE:
			return true;
	}
	return false;
}
#endif

template <typename A>
DyldInfoPrinter<A>::DyldInfoPrinter(const uint8_t* fileContent, uint32_t fileLength, const char* path, bool printArch)
 : fHeader(NULL), fLength(fileLength), 
   fStrings(NULL), fStringsEnd(NULL), fSymbols(NULL), fSymbolCount(0), fInfo(NULL), 
   fSharedRegionInfo(NULL), fFunctionStartsInfo(NULL), fDataInCode(NULL), fDRInfo(NULL), 
   fBaseAddress(0), fDynamicSymbolTable(NULL), fFirstSegment(NULL), fFirstWritableSegment(NULL),
   fWriteableSegmentWithAddrOver4G(false)
{
	// sanity check
	if ( ! validFile(fileContent) )
		throw "not a mach-o file that can be checked";

	fPath = strdup(path);
	fHeader = (const macho_header<P>*)fileContent;

	fMachHeaderPseudoSection.set_segname("__TEXT");
	fMachHeaderPseudoSection.set_sectname("");
	fMachHeaderPseudoSection.set_addr(0);
	fSections.push_back(&fMachHeaderPseudoSection);

	// get LC_DYLD_INFO
	const uint8_t* const endOfFile = (uint8_t*)fHeader + fLength;
	const uint8_t* const endOfLoadCommands = (uint8_t*)fHeader + sizeof(macho_header<P>) + fHeader->sizeofcmds();
	const uint32_t cmd_count = fHeader->ncmds();
	const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)fHeader + sizeof(macho_header<P>));
	const macho_load_command<P>* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		const uint8_t* endOfCmd = ((uint8_t*)cmd)+cmd->cmdsize();
		if ( endOfCmd > endOfLoadCommands )
			throwf("load command #%d extends beyond the end of the load commands", i);
		if ( endOfCmd > endOfFile )
			throwf("load command #%d extends beyond the end of the file", i);
		switch ( cmd->cmd() ) {
			case LC_DYLD_INFO:
			case LC_DYLD_INFO_ONLY:
				fInfo = (macho_dyld_info_command<P>*)cmd;
				break;
			case macho_segment_command<P>::CMD:
				{
				const macho_segment_command<P>* segCmd = (const macho_segment_command<P>*)cmd;
				fSegments.push_back(segCmd);
				if (fHeader->filetype() == MH_PRELOAD) {
					if ( (fFirstSegment == NULL) && (segCmd->filesize() != 0) )
						fBaseAddress = segCmd->vmaddr();
				} else {
					if ( (segCmd->fileoff() == 0) && (segCmd->filesize() != 0) )
						fBaseAddress = segCmd->vmaddr();
				}
				if ( fFirstSegment == NULL )
					fFirstSegment = segCmd;
				if ( (segCmd->initprot() & VM_PROT_WRITE) != 0 ) {
					if ( fFirstWritableSegment == NULL )
						fFirstWritableSegment = segCmd;
					if ( segCmd->vmaddr() > 0x100000000ULL )
						fWriteableSegmentWithAddrOver4G = true;
				}
				const macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
				const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
				for(const macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect)
					fSections.push_back(sect);
				}
				break;
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_REEXPORT_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
			case LC_LAZY_LOAD_DYLIB:
				{
				const macho_dylib_command<P>* dylib  = (macho_dylib_command<P>*)cmd;
				fDylibLoadCommands.push_back(dylib);
				const char* lastSlash = strrchr(dylib->name(), '/');
				const char* leafName = (lastSlash != NULL) ? lastSlash+1 : dylib->name();
				const char* firstDot = strchr(leafName, '.');
				if ( firstDot != NULL ) {
					char* t = strdup(leafName);
					t[firstDot-leafName] = '\0';
					fDylibs.push_back(t);
				}
				else {
					fDylibs.push_back(leafName);
				}
				}
				break;
			case LC_DYSYMTAB:
				fDynamicSymbolTable = (macho_dysymtab_command<P>*)cmd;
				break;
			case LC_SYMTAB:
				{
					const macho_symtab_command<P>* symtab = (macho_symtab_command<P>*)cmd;
					fSymbolCount = symtab->nsyms();
					fSymbols = (const macho_nlist<P>*)((char*)fHeader + symtab->symoff());
					fStrings = (char*)fHeader + symtab->stroff();
					fStringsEnd = fStrings + symtab->strsize();
				}
				break;
			case LC_SEGMENT_SPLIT_INFO:
				fSharedRegionInfo = (macho_linkedit_data_command<P>*)cmd;
				break;
			case LC_FUNCTION_STARTS:
				fFunctionStartsInfo = (macho_linkedit_data_command<P>*)cmd;
				break;
			case LC_DATA_IN_CODE:
				fDataInCode = (macho_linkedit_data_command<P>*)cmd;
				break;
			case LC_DYLIB_CODE_SIGN_DRS:
				fDRInfo = (macho_linkedit_data_command<P>*)cmd;
				break;
		}
		cmd = (const macho_load_command<P>*)endOfCmd;
	}
	
	if ( printArch ) {
		for (const ArchInfo* t=archInfoArray; t->archName != NULL; ++t) {
			if ( (cpu_type_t)fHeader->cputype() == t->cpuType ) {
				if ( t->isSubType && ((cpu_subtype_t)fHeader->cpusubtype() != t->cpuSubType) )
					continue;
				printf("for arch %s:\n", t->archName);
			}
		}
	}
	
	if ( printRebase ) {
		if ( fInfo != NULL )
			printRebaseInfo();
		else
			printRelocRebaseInfo();
	}
	if ( printBind ) {
		if ( fInfo != NULL )
			printBindingInfo();
		else
			printClassicBindingInfo();
	}
	if ( printWeakBind ) 
		printWeakBindingInfo();
	if ( printLazyBind ) {
		if ( fInfo != NULL )
			printLazyBindingInfo();
		else
			printClassicLazyBindingInfo();
	}
	if ( printExport ) {
		if ( fInfo != NULL )
			printExportInfo();
		else
			printSymbolTableExportInfo();
	}
	if ( printOpcodes ) {
		printRebaseInfoOpcodes();
		printBindingInfoOpcodes(false);
		printBindingInfoOpcodes(true);
		printLazyBindingOpcodes();
	}
	if ( printExportGraph )
		printExportInfoGraph();
	if ( printExportNodes ) 
		printExportInfoNodes();
	if ( printSharedRegion )
		printSharedRegionInfo();
	if ( printFunctionStarts ) 
		printFunctionStartsInfo();
	if ( printDylibs )
		printDylibsInfo();
	if ( printDRs )
		printDRInfo();
	if ( printDataCode )
		printDataInCode();
}

static uint64_t read_uleb128(const uint8_t*& p, const uint8_t* end)
{
	uint64_t result = 0;
	int		 bit = 0;
	do {
		if (p == end)
			throwf("malformed uleb128");

		uint64_t slice = *p & 0x7f;

		if (bit >= 64 || slice << bit >> bit != slice)
			throwf("uleb128 too big");
		else {
			result |= (slice << bit);
			bit += 7;
		}
	} 
	while (*p++ & 0x80);
	return result;
}

static int64_t read_sleb128(const uint8_t*& p, const uint8_t* end)
{
	int64_t result = 0;
	int bit = 0;
	uint8_t byte;
	do {
		if (p == end)
			throwf("malformed sleb128");
		byte = *p++;
		result |= (((int64_t)(byte & 0x7f)) << bit);
		bit += 7;
	} while (byte & 0x80);
	// sign extend negative numbers
	if ( ((byte & 0x40) != 0) && (bit < 64) )
		result |= (-1LL) << bit;
	return result;
}


template <typename A>
const char* DyldInfoPrinter<A>::rebaseTypeName(uint8_t type)
{
	switch (type ){
		case REBASE_TYPE_POINTER:
			return "pointer";
		case REBASE_TYPE_TEXT_ABSOLUTE32:
			return "text abs32";
		case REBASE_TYPE_TEXT_PCREL32:
			return "text rel32";
	}
	return "!!unknown!!";
}


template <typename A>
const char* DyldInfoPrinter<A>::bindTypeName(uint8_t type)
{
	switch (type ){
		case BIND_TYPE_POINTER:
			return "pointer";
		case BIND_TYPE_TEXT_ABSOLUTE32:
			return "text abs32";
		case BIND_TYPE_TEXT_PCREL32:
			return "text rel32";
	}
	return "!!unknown!!";
}


template <typename A>
typename A::P::uint_t DyldInfoPrinter<A>::segStartAddress(uint8_t segIndex)
{
	if ( segIndex > fSegments.size() )
		throw "segment index out of range";
	return fSegments[segIndex]->vmaddr();
}

template <typename A>
const char* DyldInfoPrinter<A>::segmentName(uint8_t segIndex)
{
	if ( segIndex > fSegments.size() )
		throw "segment index out of range";
	return fSegments[segIndex]->segname();
}

template <typename A>
const char* DyldInfoPrinter<A>::sectionName(uint8_t segIndex, pint_t address)
{
	if ( segIndex > fSegments.size() )
		throw "segment index out of range";
	const macho_segment_command<P>* segCmd = fSegments[segIndex];
	macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
	macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
	for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
		if ( (sect->addr() <= address) && (address < (sect->addr()+sect->size())) ) {
			if ( strlen(sect->sectname()) > 15 ) {
				static char temp[18];
				strlcpy(temp, sect->sectname(), 17);
				return temp;
			}
			else {
				return sect->sectname();
			}
		}
	}
	return "??";
}

template <typename A>
const char* DyldInfoPrinter<A>::getSegAndSectName(uint8_t segIndex, pint_t address)
{
	static char buffer[64];
	strcpy(buffer, segmentName(segIndex));
	strcat(buffer, "/");
	const macho_segment_command<P>* segCmd = fSegments[segIndex];
	macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
	macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
	for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
		if ( (sect->addr() <= address) && (address < (sect->addr()+sect->size())) ) {
			// section name may not be zero terminated
			char* end = &buffer[strlen(buffer)];
			strlcpy(end, sect->sectname(), 16);
			return buffer;				
		}
	}
	return "??";
}

template <typename A>
uint8_t DyldInfoPrinter<A>::segmentIndexForAddress(pint_t address)
{
	for(unsigned int i=0; i < fSegments.size(); ++i) {
		if ( (fSegments[i]->vmaddr() <= address) && (address < (fSegments[i]->vmaddr()+fSegments[i]->vmsize())) ) {
			return i;
		}
	}
	throwf("address 0x%llX is not in any segment", (uint64_t)address);
}

template <typename A>
typename A::P::uint_t*	DyldInfoPrinter<A>::mappedAddressForVMAddress(pint_t vmaddress)
{
	for(unsigned int i=0; i < fSegments.size(); ++i) {
		if ( (fSegments[i]->vmaddr() <= vmaddress) && (vmaddress < (fSegments[i]->vmaddr()+fSegments[i]->vmsize())) ) {
			unsigned long offsetInMappedFile = fSegments[i]->fileoff()+vmaddress-fSegments[i]->vmaddr();
			return (pint_t*)((uint8_t*)fHeader + offsetInMappedFile);
		}
	}
	throwf("address 0x%llX is not in any segment", (uint64_t)vmaddress);
}

template <typename A>
const char* DyldInfoPrinter<A>::ordinalName(int libraryOrdinal)
{
	switch ( libraryOrdinal) {
		case BIND_SPECIAL_DYLIB_SELF:
			return "this-image";
		case BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE:
			return "main-executable";
		case BIND_SPECIAL_DYLIB_FLAT_LOOKUP:
			return "flat-namespace";
		case BIND_SPECIAL_DYLIB_WEAK_LOOKUP:
			return "weak";
	}
	if ( libraryOrdinal < BIND_SPECIAL_DYLIB_FLAT_LOOKUP )
		throw "unknown special ordinal";
	if ( libraryOrdinal > (int)fDylibs.size() )
		throw "libraryOrdinal out of range";
	return fDylibs[libraryOrdinal-1];
}

template <typename A>
const char* DyldInfoPrinter<A>::classicOrdinalName(int libraryOrdinal)
{
	if ( (fHeader->flags() & MH_TWOLEVEL) ==  0 )
		return "flat-namespace";
	switch ( libraryOrdinal) {
		case SELF_LIBRARY_ORDINAL:
			return "this-image";
		case EXECUTABLE_ORDINAL:
			return "main-executable";
		case DYNAMIC_LOOKUP_ORDINAL:
			return "flat-namespace";
	}
	if ( libraryOrdinal > (int)fDylibs.size() )
		throw "libraryOrdinal out of range";
	return fDylibs[libraryOrdinal-1];
}

template <typename A>
void DyldInfoPrinter<A>::printRebaseInfo()
{
	bool seenThreadedRebase = false;
	if ( (fInfo == NULL) || (fInfo->rebase_off() == 0) ) {
		// If we have no rebase opcodes, then we may be using the threaded rebase/bind combined
		// format and need to parse the bind opcodes instead.
		if ( (fInfo->rebase_size() == 0) && (fInfo->bind_size() != 0) ) {
			const uint8_t* p = (uint8_t*)fHeader + fInfo->bind_off();
			const uint8_t* end = &p[fInfo->bind_size()];

			uint8_t type = 0;
			uint8_t flags = 0;
			uint8_t segIndex = 0;
			uint64_t segOffset = 0;
			const char* symbolName = NULL;
			const char* fromDylib = "??";
			int libraryOrdinal = 0;
			int64_t addend = 0;
			uint32_t count;
			uint32_t skip;
			pint_t segStartAddr = 0;
			const char* segName = "??";
			const char* typeName = "??";
			const char* weak_import = "";
			bool done = false;
			while ( !done && (p < end) ) {
				uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
				uint8_t opcode = *p & BIND_OPCODE_MASK;
				++p;
				switch (opcode) {
					case BIND_OPCODE_DONE:
						done = true;
						break;
					case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
						libraryOrdinal = immediate;
						fromDylib = ordinalName(libraryOrdinal);
						break;
					case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
						libraryOrdinal = read_uleb128(p, end);
						fromDylib = ordinalName(libraryOrdinal);
						break;
					case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
						// the special ordinals are negative numbers
						if ( immediate == 0 )
							libraryOrdinal = 0;
						else {
							int8_t signExtended = BIND_OPCODE_MASK | immediate;
							libraryOrdinal = signExtended;
						}
						fromDylib = ordinalName(libraryOrdinal);
						break;
					case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
						symbolName = (char*)p;
						while (*p != '\0')
							++p;
						++p;
						flags = immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT;
						if ( flags != 0 )
							weak_import = " (weak import)";
						else
							weak_import = "";
						break;
					case BIND_OPCODE_SET_TYPE_IMM:
						type = immediate;
						typeName = bindTypeName(type);
						break;
					case BIND_OPCODE_SET_ADDEND_SLEB:
						addend = read_sleb128(p, end);
						break;
					case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
						segIndex = immediate;
						segStartAddr = segStartAddress(segIndex);
						segName = segmentName(segIndex);
						segOffset = read_uleb128(p, end);
						break;
					case BIND_OPCODE_ADD_ADDR_ULEB:
						segOffset += read_uleb128(p, end);
						break;
					case BIND_OPCODE_DO_BIND:
						break;
					case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
						segOffset += read_uleb128(p, end) + sizeof(pint_t);
						break;
					case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
						segOffset += immediate*sizeof(pint_t) + sizeof(pint_t);
						break;
					case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
						count = read_uleb128(p, end);
						skip = read_uleb128(p, end);
						for (uint32_t i=0; i < count; ++i) {
							segOffset += skip + sizeof(pint_t);
						}
						break;
					case BIND_OPCODE_THREADED:
						// Note the immediate is a sub opcode
						switch (immediate) {
							case BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB:
								printf("rebase information (from compressed dyld info):\n");
								printf("segment section          address     type         value\n");
								count = read_uleb128(p, end);
								seenThreadedRebase = true;
								break;
							case BIND_SUBOPCODE_THREADED_APPLY: {
								uint64_t delta = 0;
								do {
									const uint8_t* pointerLocation = (uint8_t*)fHeader + fSegments[segIndex]->fileoff() + segOffset;
									uint64_t value = P::getP(*(uint64_t*)pointerLocation);
#if SUPPORT_ARCH_arm64e
									uint16_t diversity = (uint16_t)(value >> 32);
									bool hasAddressDiversity = (value & (1ULL << 48)) != 0;
									uint8_t key = (uint8_t)((value >> 49) & 0x3);
									bool isAuthenticated = (value & (1ULL << 63)) != 0;
#endif
									bool isRebase = (value & (1ULL << 62)) == 0;
									if (isRebase) {

#if SUPPORT_ARCH_arm64e
										static const char* keyNames[] = {
											"IA", "IB", "DA", "DB"
										};
										if (isAuthenticated) {
											uint64_t targetValue = value & 0xFFFFFFFFULL;
											targetValue += fBaseAddress;
											printf("%-7s %-16s 0x%08llX  %s  0x%08llX (JOP: diversity %d, address %s, %s)\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, targetValue, diversity, hasAddressDiversity ? "true" : "false", keyNames[key]);
										} else
#endif
										{
											// Regular pointer which needs to fit in 51-bits of value.
											// C++ RTTI uses the top bit, so we'll allow the whole top-byte
											// and the signed-extended bottom 43-bits to be fit in to 51-bits.
											uint64_t top8Bits = value & 0x0007F80000000000ULL;
											uint64_t bottom43Bits = value & 0x000007FFFFFFFFFFULL;
											uint64_t targetValue = ( top8Bits << 13 ) | (((intptr_t)(bottom43Bits << 21) >> 21) & 0x00FFFFFFFFFFFFFF);
											printf("%-7s %-16s 0x%08llX  %s  0x%08llX\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, targetValue);
										}
									}
									// The delta is bits [51..61]
									// And bit 62 is to tell us if we are a rebase (0) or bind (1)
									value &= ~(1ULL << 62);
									delta = ( value & 0x3FF8000000000000 ) >> 51;
									segOffset += delta * sizeof(pint_t);
								} while ( delta != 0);
								break;
							}
							default:
								throwf("unknown threaded bind subopcode %d", immediate);
						}
						break;
					default:
						throwf("bad bind opcode %d", *p);
				}
			}
		}

		if (!seenThreadedRebase)
			printf("no compressed rebase info\n");
	}
	else {
		printf("rebase information (from compressed dyld info):\n");
		printf("segment section          address     type         value\n");

		const uint8_t* p = (uint8_t*)fHeader + fInfo->rebase_off();
		const uint8_t* end = &p[fInfo->rebase_size()];
		
		uint8_t type = 0;
		uint64_t segOffset = 0;
		uint32_t count;
		uint32_t skip;
		int segIndex = 0;
		pint_t segStartAddr = 0;
		const char* segName = "??";
		const char* typeName = "??";
		const uint8_t* pointerLocation = nullptr;
		uint64_t value = 0;
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
					typeName = rebaseTypeName(type);
					break;
				case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segIndex = immediate;
					segStartAddr = segStartAddress(segIndex);
					segName = segmentName(segIndex);
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
						pointerLocation = (uint8_t*)fHeader + fSegments[segIndex]->fileoff() + segOffset;
						value = P::getP(*(uint64_t*)pointerLocation);
						printf("%-7s %-16s 0x%08llX  %s  0x%08llX\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, value);
						segOffset += sizeof(pint_t);
					}
					break;
				case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
					count = read_uleb128(p, end);
					for (uint32_t i=0; i < count; ++i) {
						pointerLocation = (uint8_t*)fHeader + fSegments[segIndex]->fileoff() + segOffset;
						value = P::getP(*(uint64_t*)pointerLocation);
						printf("%-7s %-16s 0x%08llX  %s  0x%08llX\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, value);
						segOffset += sizeof(pint_t);
					}
					break;
				case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
					pointerLocation = (uint8_t*)fHeader + fSegments[segIndex]->fileoff() + segOffset;
					value = P::getP(*(uint64_t*)pointerLocation);
					printf("%-7s %-16s 0x%08llX  %s  0x%08llX\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, value);
					segOffset += read_uleb128(p, end) + sizeof(pint_t);
					break;
				case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
					count = read_uleb128(p, end);
					skip = read_uleb128(p, end);
					for (uint32_t i=0; i < count; ++i) {
						pointerLocation = (uint8_t*)fHeader + fSegments[segIndex]->fileoff() + segOffset;
						value = P::getP(*(uint64_t*)pointerLocation);
						printf("%-7s %-16s 0x%08llX  %s  0x%08llX\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, value);
						segOffset += skip + sizeof(pint_t);
					}
					break;
				default:
					throwf("bad rebase opcode %d", *p);
			}
		}
	}
}



template <typename A>
void DyldInfoPrinter<A>::printRebaseInfoOpcodes()
{
	if ( (fInfo == NULL) || (fInfo->rebase_off() == 0) ) {
		printf("no compressed rebase info\n");
	}
	else {
		printf("rebase opcodes:\n");
		const uint8_t* start = (uint8_t*)fHeader + fInfo->rebase_off();
		const uint8_t* end = &start[fInfo->rebase_size()];
		const uint8_t* p = start;

		uint8_t type = 0;
		uint64_t address = fBaseAddress;
		uint32_t count;
		uint32_t skip;
		unsigned int segmentIndex;
		bool done = false;
		while ( !done && (p < end) ) {
			uint8_t immediate = *p & REBASE_IMMEDIATE_MASK;
			uint8_t opcode = *p & REBASE_OPCODE_MASK;
			uint32_t opcodeOffset = p-start;
			++p;
			switch (opcode) {
				case REBASE_OPCODE_DONE:
					done = true;
					printf("0x%04X REBASE_OPCODE_DONE()\n", opcodeOffset);
					break;
				case REBASE_OPCODE_SET_TYPE_IMM:
					type = immediate;
					printf("0x%04X REBASE_OPCODE_SET_TYPE_IMM(%d)\n", opcodeOffset, type);
					break;
				case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segmentIndex = immediate;
					address = read_uleb128(p, end);
					printf("0x%04X REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(%d, 0x%08llX)\n", opcodeOffset, segmentIndex, address);
					break;
				case REBASE_OPCODE_ADD_ADDR_ULEB:
					address = read_uleb128(p, end);
					printf("0x%04X REBASE_OPCODE_ADD_ADDR_ULEB(0x%0llX)\n", opcodeOffset, address);
					break;
				case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
					address = immediate*sizeof(pint_t);
					printf("0x%04X REBASE_OPCODE_ADD_ADDR_IMM_SCALED(0x%0llX)\n", opcodeOffset, address);
					break;
				case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
					printf("0x%04X REBASE_OPCODE_DO_REBASE_IMM_TIMES(%d)\n", opcodeOffset, immediate);
					break;
				case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
					count = read_uleb128(p, end);
					printf("0x%04X REBASE_OPCODE_DO_REBASE_ULEB_TIMES(%d)\n", opcodeOffset, count);
					break;
				case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
					skip = read_uleb128(p, end) + sizeof(pint_t);
					printf("0x%04X REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB(%d)\n", opcodeOffset, skip);
					break;
				case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
					count = read_uleb128(p, end);
					skip = read_uleb128(p, end);
					printf("0x%04X REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB(%d, %d)\n", opcodeOffset, count, skip);
					break;
				default:
					throwf("bad rebase opcode %d", *p);
			}
		}	
	}

}

template <typename A>
void DyldInfoPrinter<A>::printBindingInfoOpcodes(bool weakbinding)
{
	if ( fInfo == NULL ) {
		printf("no compressed binding info\n");
	}
	else if ( !weakbinding && (fInfo->bind_off() == 0) ) {
		printf("no compressed binding info\n");
	}
	else if ( weakbinding && (fInfo->weak_bind_off() == 0) ) {
		printf("no compressed weak binding info\n");
	}
	else {
		const uint8_t* start;
		const uint8_t* end;
		if ( weakbinding ) {
			printf("weak binding opcodes:\n");
			start = (uint8_t*)fHeader + fInfo->weak_bind_off();
			end = &start[fInfo->weak_bind_size()];
		}
		else {
			printf("binding opcodes:\n");
			start = (uint8_t*)fHeader + fInfo->bind_off();
			end = &start[fInfo->bind_size()];
		}
		const uint8_t* p = start;
		uint8_t type = 0;
		uint8_t flags;
		uint64_t address = fBaseAddress;
		const char* symbolName = NULL;
		int libraryOrdinal = 0;
		int64_t addend = 0;
		uint32_t segmentIndex = 0;
		uint32_t count;
		uint32_t skip;
		bool done = false;
		while ( !done && (p < end) ) {
			uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
			uint8_t opcode = *p & BIND_OPCODE_MASK;
			uint32_t opcodeOffset = p-start;
			++p;
			switch (opcode) {
				case BIND_OPCODE_DONE:
					done = true;
					printf("0x%04X BIND_OPCODE_DONE\n", opcodeOffset);
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
					libraryOrdinal = immediate;
					printf("0x%04X BIND_OPCODE_SET_DYLIB_ORDINAL_IMM(%d)\n", opcodeOffset, libraryOrdinal);
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
					libraryOrdinal = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB(%d)\n", opcodeOffset, libraryOrdinal);
					break;
				case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
					// the special ordinals are negative numbers
					if ( immediate == 0 )
						libraryOrdinal = 0;
					else {
						int8_t signExtended = BIND_OPCODE_MASK | immediate;
						libraryOrdinal = signExtended;
					}
					printf("0x%04X BIND_OPCODE_SET_DYLIB_SPECIAL_IMM(%d)\n", opcodeOffset, libraryOrdinal);
					break;
				case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
					flags = immediate;
					symbolName = (char*)p;
					while (*p != '\0')
						++p;
					++p;
					printf("0x%04X BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM(0x%02X, %s)\n", opcodeOffset, flags, symbolName);
					break;
				case BIND_OPCODE_SET_TYPE_IMM:
					type = immediate;
					printf("0x%04X BIND_OPCODE_SET_TYPE_IMM(%d)\n", opcodeOffset, type);
					break;
				case BIND_OPCODE_SET_ADDEND_SLEB:
					addend = read_sleb128(p, end);
					printf("0x%04X BIND_OPCODE_SET_ADDEND_SLEB(%lld)\n", opcodeOffset, addend);
					break;
				case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segmentIndex = immediate;
					address = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(0x%02X, 0x%08llX)\n", opcodeOffset, segmentIndex, address);
					break;
				case BIND_OPCODE_ADD_ADDR_ULEB:
					skip = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_ADD_ADDR_ULEB(0x%08X)\n", opcodeOffset, skip);
					break;
				case BIND_OPCODE_DO_BIND:
					printf("0x%04X BIND_OPCODE_DO_BIND()\n", opcodeOffset);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
					skip = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB(0x%08X)\n", opcodeOffset, skip);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
					skip = immediate*sizeof(pint_t) + sizeof(pint_t);
					printf("0x%04X BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED(0x%08X)\n", opcodeOffset, skip);
					break;
				case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
					count = read_uleb128(p, end);
					skip = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB(%d, 0x%08X)\n", opcodeOffset, count, skip);
					break;
				case BIND_OPCODE_THREADED:
					// Note the immediate is a sub opcode
					switch (immediate) {
						case BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB:
							count = read_uleb128(p, end);
							printf("0x%04X BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB(%d)\n", opcodeOffset, count);
							break;
						case BIND_SUBOPCODE_THREADED_APPLY:
							printf("0x%04X BIND_SUBOPCODE_THREADED_APPLY\n", opcodeOffset);
							break;
						default:
							throwf("unknown threaded bind subopcode %d", immediate);
					}
					break;
				default:
					throwf("unknown bind opcode %d", *p);
			}
		}	
	}

}

struct ThreadedBindData {
	ThreadedBindData(const char* symbolName, int64_t addend, int libraryOrdinal, uint8_t flags, uint8_t type)
		: symbolName(symbolName), addend(addend), libraryOrdinal(libraryOrdinal), flags(flags), type(type) { }

	std::tuple<const char*, int64_t, int, uint8_t, uint8_t> pack() const {
		return std::make_tuple(symbolName, addend, libraryOrdinal, flags, type);
	}

	const char* symbolName 	= nullptr;
	int64_t addend 			= 0;
	int libraryOrdinal 		= 0;
	uint8_t flags			= 0;
	uint8_t type			= 0;
};

template <typename A>
void DyldInfoPrinter<A>::printBindingInfo()
{
	if ( (fInfo == NULL) || (fInfo->bind_off() == 0) ) {
		printf("no compressed binding info\n");
	}
	else {
		printf("bind information:\n");
		printf("segment section          address        type    addend dylib            symbol\n");
		const uint8_t* p = (uint8_t*)fHeader + fInfo->bind_off();
		const uint8_t* end = &p[fInfo->bind_size()];

		uint8_t type = 0;
		uint8_t flags = 0;
		uint8_t segIndex = 0;
		uint64_t segOffset = 0;
		const char* symbolName = NULL;
		const char* fromDylib = "??";
		int libraryOrdinal = 0;
		int64_t addend = 0;
		uint32_t count;
		uint32_t skip;
		pint_t segStartAddr = 0;
		const char* segName = "??";
		const char* typeName = "??";
		const char* weak_import = "";
		std::vector<ThreadedBindData> ordinalTable;
		bool useThreadedRebaseBind = false;
		bool done = false;
		while ( !done && (p < end) ) {
			uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
			uint8_t opcode = *p & BIND_OPCODE_MASK;
			++p;
			switch (opcode) {
				case BIND_OPCODE_DONE:
					done = true;
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
					libraryOrdinal = immediate;
					fromDylib = ordinalName(libraryOrdinal);
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
					libraryOrdinal = read_uleb128(p, end);
					fromDylib = ordinalName(libraryOrdinal);
					break;
				case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
					// the special ordinals are negative numbers
					if ( immediate == 0 )
						libraryOrdinal = 0;
					else {
						int8_t signExtended = BIND_OPCODE_MASK | immediate;
						libraryOrdinal = signExtended;
					}
					fromDylib = ordinalName(libraryOrdinal);
					break;
				case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
					symbolName = (char*)p;
					while (*p != '\0')
						++p;
					++p;
					flags = immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT;
					if ( flags != 0 )
						weak_import = " (weak import)";
					else
						weak_import = "";
					break;
				case BIND_OPCODE_SET_TYPE_IMM:
					type = immediate;
					typeName = bindTypeName(type);
					break;
				case BIND_OPCODE_SET_ADDEND_SLEB:
					addend = read_sleb128(p, end);
					break;
				case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segIndex = immediate;
					segStartAddr = segStartAddress(segIndex);
					segName = segmentName(segIndex);
					segOffset = read_uleb128(p, end);
					break;
				case BIND_OPCODE_ADD_ADDR_ULEB:
					segOffset += read_uleb128(p, end);
					break;
				case BIND_OPCODE_DO_BIND:
					if (!useThreadedRebaseBind) {
						printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import );
						segOffset += sizeof(pint_t);
					} else {
						ordinalTable.push_back(ThreadedBindData(symbolName, addend, libraryOrdinal, flags, type));
					}
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
					printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import );
					segOffset += read_uleb128(p, end) + sizeof(pint_t);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
					printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import );
					segOffset += immediate*sizeof(pint_t) + sizeof(pint_t);
					break;
				case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
					count = read_uleb128(p, end);
					skip = read_uleb128(p, end);
					for (uint32_t i=0; i < count; ++i) {
						printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import );
						segOffset += skip + sizeof(pint_t);
					}
					break;
				case BIND_OPCODE_THREADED:
					// Note the immediate is a sub opcode
					switch (immediate) {
						case BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB:
							count = read_uleb128(p, end);
							ordinalTable.clear();
							ordinalTable.reserve(count);
							useThreadedRebaseBind = true;
							break;
						case BIND_SUBOPCODE_THREADED_APPLY: {
							uint64_t delta = 0;
							do {
								const uint8_t* pointerLocation = (uint8_t*)fHeader + fSegments[segIndex]->fileoff() + segOffset;
								uint64_t value = P::getP(*(uint64_t*)pointerLocation);
#if SUPPORT_ARCH_arm64e
								uint16_t diversity = (uint16_t)(value >> 32);
								bool hasAddressDiversity = (value & (1ULL << 48)) != 0;
								uint8_t key = (uint8_t)((value >> 49) & 0x3);
								bool isAuthenticated = (value & (1ULL << 63)) != 0;
#endif
								bool isRebase = (value & (1ULL << 62)) == 0;

								if (isRebase) {
									//printf("(rebase): %-7s %-16s 0x%08llX  %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName);
								} else {
									// the ordinal is bits [0..15]
									uint16_t ordinal = value & 0xFFFF;
									if (ordinal >= ordinalTable.size()) {
										fprintf(stderr, "bind ordinal is out of range\n");
										break;
									}
									std::tie(symbolName, addend, libraryOrdinal, flags, type) = ordinalTable[ordinal].pack();
									if ( (flags & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 )
										weak_import = " (weak import)";
									else
										weak_import = "";
									fromDylib = ordinalName(libraryOrdinal);
#if SUPPORT_ARCH_arm64e
									if (isAuthenticated) {
										static const char* keyNames[] = {
											"IA", "IB", "DA", "DB"
										};
										printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s with value 0x%016llX (JOP: diversity %d, address %s, %s)\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import, value, diversity, hasAddressDiversity ? "true" : "false", keyNames[key]);
									} else
#endif
									{
										printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s with value 0x%016llX\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import, value );
									}
								}

								// The delta is bits [51..61]
								// And bit 62 is to tell us if we are a rebase (0) or bind (1)
								value &= ~(1ULL << 62);
								delta = ( value & 0x3FF8000000000000 ) >> 51;
								segOffset += delta * sizeof(pint_t);
							} while ( delta != 0);
							break;
						}
						default:
							throwf("unknown threaded bind subopcode %d", immediate);
					}
					break;
				default:
					throwf("bad bind opcode %d", *p);
			}
		}	
	}

}

template <typename A>
void DyldInfoPrinter<A>::printWeakBindingInfo()
{
	if ( (fInfo == NULL) || (fInfo->weak_bind_off() == 0) ) {
		printf("no weak binding\n");
	}
	else {
		printf("weak binding information:\n");
		printf("segment section          address       type     addend symbol\n");
		const uint8_t* p = (uint8_t*)fHeader + fInfo->weak_bind_off();
		const uint8_t* end = &p[fInfo->weak_bind_size()];
		
		uint8_t type = 0;
		uint8_t segIndex = 0;
		uint64_t segOffset = 0;
		const char* symbolName = NULL;
		int64_t addend = 0;
		uint32_t count;
		uint32_t skip;
		pint_t segStartAddr = 0;
		const char* segName = "??";
		const char* typeName = "??";
		bool done = false;
		while ( !done && (p < end) ) {
			uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
			uint8_t opcode = *p & BIND_OPCODE_MASK;
			++p;
			switch (opcode) {
				case BIND_OPCODE_DONE:
					done = true;
					break;
				case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
					symbolName = (char*)p;
					while (*p != '\0')
						++p;
					++p;
					if ( (immediate & BIND_SYMBOL_FLAGS_NON_WEAK_DEFINITION) != 0 )
						printf("                                       strong          %s\n", symbolName );
					break;
				case BIND_OPCODE_SET_TYPE_IMM:
					type = immediate;
					typeName = bindTypeName(type);
					break;
				case BIND_OPCODE_SET_ADDEND_SLEB:
					addend = read_sleb128(p, end);
					break;
				case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segIndex = immediate;
					segStartAddr = segStartAddress(segIndex);
					segName = segmentName(segIndex);
					segOffset = read_uleb128(p, end);
					break;
				case BIND_OPCODE_ADD_ADDR_ULEB:
					segOffset += read_uleb128(p, end);
					break;
				case BIND_OPCODE_DO_BIND:
					printf("%-7s %-16s 0x%08llX %10s   %5lld %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, symbolName );
					segOffset += sizeof(pint_t);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
					printf("%-7s %-16s 0x%08llX %10s   %5lld %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, symbolName );
					segOffset += read_uleb128(p, end) + sizeof(pint_t);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
					printf("%-7s %-16s 0x%08llX %10s   %5lld %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, symbolName );
					segOffset += immediate*sizeof(pint_t) + sizeof(pint_t);
					break;
				case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
					count = read_uleb128(p, end);
					skip = read_uleb128(p, end);
					for (uint32_t i=0; i < count; ++i) {
					printf("%-7s %-16s 0x%08llX %10s   %5lld %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, symbolName );
							segOffset += skip + sizeof(pint_t);
					}
					break;
				default:
					throwf("unknown weak bind opcode %d", *p);
			}
		}	
	}

}


template <typename A>
void DyldInfoPrinter<A>::printLazyBindingInfo()
{
	if ( fInfo == NULL ) {
		printf("no compressed dyld info\n");
	}
	else if ( fInfo->lazy_bind_off() == 0 ) {
		printf("no compressed lazy binding info\n");
	}
	else {
		printf("lazy binding information (from lazy_bind part of dyld info):\n");
		printf("segment section          address    index  dylib            symbol\n");
		const uint8_t* const start = (uint8_t*)fHeader + fInfo->lazy_bind_off();
		const uint8_t* const end = &start[fInfo->lazy_bind_size()];

		uint8_t type = BIND_TYPE_POINTER;
		uint8_t segIndex = 0;
		uint64_t segOffset = 0;
		const char* symbolName = NULL;
		const char* fromDylib = "??";
		int libraryOrdinal = 0;
		int64_t addend = 0;
		uint32_t lazy_offset = 0;
		pint_t segStartAddr = 0;
		const char* segName = "??";
		const char* typeName = "??";
		const char* weak_import = "";
		for (const uint8_t* p=start; p < end; ) {
			uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
			uint8_t opcode = *p & BIND_OPCODE_MASK;
			++p;
			switch (opcode) {
				case BIND_OPCODE_DONE:
					lazy_offset = p-start;
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
					libraryOrdinal = immediate;
					fromDylib = ordinalName(libraryOrdinal);
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
					libraryOrdinal = read_uleb128(p, end);
					fromDylib = ordinalName(libraryOrdinal);
					break;
				case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
					// the special ordinals are negative numbers
					if ( immediate == 0 )
						libraryOrdinal = 0;
					else {
						int8_t signExtended = BIND_OPCODE_MASK | immediate;
						libraryOrdinal = signExtended;
					}
					fromDylib = ordinalName(libraryOrdinal);
					break;
				case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
					symbolName = (char*)p;
					while (*p != '\0')
						++p;
					++p;
					if ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 )
						weak_import = " (weak import)";
					else
						weak_import = "";
					break;
				case BIND_OPCODE_SET_TYPE_IMM:
					type = immediate;
					typeName = bindTypeName(type);
					break;
				case BIND_OPCODE_SET_ADDEND_SLEB:
					addend = read_sleb128(p, end);
					break;
				case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segIndex = immediate;
					segStartAddr = segStartAddress(segIndex);
					segName = segmentName(segIndex);
					segOffset = read_uleb128(p, end);
					break;
				case BIND_OPCODE_ADD_ADDR_ULEB:
					segOffset += read_uleb128(p, end);
					break;
				case BIND_OPCODE_DO_BIND:
					printf("%-7s %-16s 0x%08llX 0x%04X %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, lazy_offset, fromDylib, symbolName, weak_import);
					segOffset += sizeof(pint_t);
					break;
				default:
					throwf("bad lazy bind opcode %d", *p);
			}
		}
	}

}

template <typename A>
void DyldInfoPrinter<A>::printLazyBindingOpcodes()
{
	if ( fInfo == NULL ) {
		printf("no compressed dyld info\n");
	}
	else if ( fInfo->lazy_bind_off() == 0 ) {
		printf("no compressed lazy binding info\n");
	}
	else {
		printf("lazy binding opcodes:\n");
		const uint8_t* const start = (uint8_t*)fHeader + fInfo->lazy_bind_off();
		const uint8_t* const end = &start[fInfo->lazy_bind_size()];
		uint8_t type = BIND_TYPE_POINTER;
		uint8_t flags;
		uint64_t address = fBaseAddress;
		const char* symbolName = NULL;
		int libraryOrdinal = 0;
		int64_t addend = 0;
		uint32_t segmentIndex = 0;
		uint32_t count;
		uint32_t skip;
		for (const uint8_t* p = start; p < end; ) {
			uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
			uint8_t opcode = *p & BIND_OPCODE_MASK;
			uint32_t opcodeOffset = p-start;
			++p;
			switch (opcode) {
				case BIND_OPCODE_DONE:
					printf("0x%04X BIND_OPCODE_DONE\n", opcodeOffset);
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
					libraryOrdinal = immediate;
					printf("0x%04X BIND_OPCODE_SET_DYLIB_ORDINAL_IMM(%d)\n", opcodeOffset, libraryOrdinal);
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
					libraryOrdinal = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB(%d)\n", opcodeOffset, libraryOrdinal);
					break;
				case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
					// the special ordinals are negative numbers
					if ( immediate == 0 )
						libraryOrdinal = 0;
					else {
						int8_t signExtended = BIND_OPCODE_MASK | immediate;
						libraryOrdinal = signExtended;
					}
					printf("0x%04X BIND_OPCODE_SET_DYLIB_SPECIAL_IMM(%d)\n", opcodeOffset, libraryOrdinal);
					break;
				case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
					flags = immediate;
					symbolName = (char*)p;
					while (*p != '\0')
						++p;
					++p;
					printf("0x%04X BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM(0x%02X, %s)\n", opcodeOffset, flags, symbolName);
					break;
				case BIND_OPCODE_SET_TYPE_IMM:
					type = immediate;
					printf("0x%04X BIND_OPCODE_SET_TYPE_IMM(%d)\n", opcodeOffset, type);
					break;
				case BIND_OPCODE_SET_ADDEND_SLEB:
					addend = read_sleb128(p, end);
					printf("0x%04X BIND_OPCODE_SET_ADDEND_SLEB(%lld)\n", opcodeOffset, addend);
					break;
				case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segmentIndex = immediate;
					address = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(0x%02X, 0x%08llX)\n", opcodeOffset, segmentIndex, address);
					break;
				case BIND_OPCODE_ADD_ADDR_ULEB:
					skip = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_ADD_ADDR_ULEB(0x%08X)\n", opcodeOffset, skip);
					break;
				case BIND_OPCODE_DO_BIND:
					printf("0x%04X BIND_OPCODE_DO_BIND()\n", opcodeOffset);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
					skip = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB(0x%08X)\n", opcodeOffset, skip);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
					skip = immediate*sizeof(pint_t) + sizeof(pint_t);
					printf("0x%04X BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED(0x%08X)\n", opcodeOffset, skip);
					break;
				case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
					count = read_uleb128(p, end);
					skip = read_uleb128(p, end);
					printf("0x%04X BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB(%d, 0x%08X)\n", opcodeOffset, count, skip);
					break;
				default:
					throwf("unknown bind opcode %d", *p);
			}
		}	
	}

}

struct SortExportsByAddress
{
     bool operator()(const mach_o::trie::Entry& left, const mach_o::trie::Entry& right)
     {
        return ( left.address < right.address );
     }
};

template <typename A>
void DyldInfoPrinter<A>::printExportInfo()
{
	if ( (fInfo == NULL) || (fInfo->export_off() == 0) ) {
		printf("no compressed export info\n");
	}
	else {
		printf("export information (from trie):\n");
		const uint8_t* start = (uint8_t*)fHeader + fInfo->export_off();
		const uint8_t* end = &start[fInfo->export_size()];
		std::vector<mach_o::trie::Entry> list;
		parseTrie(start, end, list);
		//std::sort(list.begin(), list.end(), SortExportsByAddress());
		for (std::vector<mach_o::trie::Entry>::iterator it=list.begin(); it != list.end(); ++it) {
			const bool reExport = (it->flags & EXPORT_SYMBOL_FLAGS_REEXPORT);
			const bool weakDef = (it->flags & EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION);
			const bool threadLocal = ((it->flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL);
			const bool abs = ((it->flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE);
			const bool resolver = (it->flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER);
			if ( reExport )
				printf("[re-export] ");
			else
				printf("0x%08llX  ", fBaseAddress+it->address);
			printf("%s", it->name);
			if ( weakDef || threadLocal || resolver || abs ) {
				bool needComma = false;
				printf(" [");
				if ( weakDef ) {
					printf("weak_def");
					needComma = true;
				}
				if ( threadLocal ) {
					if ( needComma ) 
						printf(", ");
					printf("per-thread");
					needComma = true;
				}
				if ( abs ) {
					if ( needComma )
						printf(", ");
					printf("absolute");
					needComma = true;
				}
				if ( resolver ) {
					if ( needComma ) 
						printf(", ");
					printf("resolver=0x%08llX", it->other);
					needComma = true;
				}
				printf("]");
			}
			if ( reExport ) {
				if ( it->importName[0] == '\0' )
					printf(" (from %s)", fDylibs[it->other - 1]);
				else
					printf(" (%s from %s)", it->importName, fDylibs[it->other - 1]);
			}
			printf("\n");
		}
	}
}


template <typename A>
void DyldInfoPrinter<A>::processExportGraphNode(const uint8_t* const start, const uint8_t* const end,  
											const uint8_t* parent, const uint8_t* p,
											char* cummulativeString, int curStrOffset) 
{
	const uint8_t* const me = p;
	const uint64_t terminalSize = read_uleb128(p, end);
	const uint8_t* children = p + terminalSize;
	if ( terminalSize != 0 ) {
		uint32_t flags = read_uleb128(p, end);
		if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
			uint64_t ordinal = read_uleb128(p, end);
			const char* importName = (const char*)p;
			while (*p != '\0')
				++p;
			++p;
			if ( *importName == '\0' ) 
				printf("\tnode%03ld [ label=%s,re-export from dylib=%llu ];\n", (long)(me-start), cummulativeString, ordinal);
			else
				printf("\tnode%03ld [ label=%s,re-export %s from dylib=%llu ];\n", (long)(me-start), cummulativeString, importName, ordinal);
		}
		else {
			uint64_t address = read_uleb128(p, end);
			printf("\tnode%03ld [ label=%s,addr0x%08llX ];\n", (long)(me-start), cummulativeString, address);
			if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
				read_uleb128(p, end);
		}
	}
	else {
		printf("\tnode%03ld;\n", (long)(me-start));
	}
	const uint8_t childrenCount = *children++;
	const uint8_t* s = children;
	for (uint8_t i=0; i < childrenCount; ++i) {
		const char* edgeName = (char*)s;
		int edgeStrLen = 0;
		while (*s != '\0') {
			cummulativeString[curStrOffset+edgeStrLen] = *s++;
			++edgeStrLen;
		}
		cummulativeString[curStrOffset+edgeStrLen] = *s++;
		uint32_t childNodeOffet = read_uleb128(s, end);
		printf("\tnode%03ld -> node%03d [ label=%s ] ;\n", (long)(me-start), childNodeOffet, edgeName);
		processExportGraphNode(start, end, start, start+childNodeOffet, cummulativeString, curStrOffset+edgeStrLen);	
	}
}

template <typename A>
void DyldInfoPrinter<A>::printExportInfoGraph()
{
	if ( (fInfo == NULL) || (fInfo->export_off() == 0) ) {
		printf("no compressed export info\n");
	}
	else {
		const uint8_t* p = (uint8_t*)fHeader + fInfo->export_off();
		const uint8_t* end = &p[fInfo->export_size()];
		char cummulativeString[2000];
		printf("digraph {\n");
		processExportGraphNode(p, end, p, p, cummulativeString, 0);
		printf("}\n");
	}
}

template <typename A>
void DyldInfoPrinter<A>::gatherNodeStarts(const uint8_t* const start, const uint8_t* const end,  
											const uint8_t* parent, const uint8_t* p,
											std::vector<uint32_t>& nodeStarts) 
{
	nodeStarts.push_back(p-start);
	const uint8_t terminalSize = read_uleb128(p, end);
	const uint8_t* children = p + terminalSize;
	
	const uint8_t childrenCount = *children++;
	const uint8_t* s = children;
	for (uint8_t i=0; i < childrenCount; ++i) {
		// skip over edge string
		while (*s != '\0')
			++s;
		++s;
		uint32_t childNodeOffet = read_uleb128(s, end);
		gatherNodeStarts(start, end, start, start+childNodeOffet, nodeStarts);	
	}
}


template <typename A>
void DyldInfoPrinter<A>::printExportInfoNodes()
{
	if ( (fInfo == NULL) || (fInfo->export_off() == 0) ) {
		printf("no compressed export info\n");
	}
	else {
		const uint8_t* start = (uint8_t*)fHeader + fInfo->export_off();
		const uint8_t* end = &start[fInfo->export_size()];
		std::vector<uint32_t> nodeStarts;
		gatherNodeStarts(start, end, start, start, nodeStarts);
		std::sort(nodeStarts.begin(), nodeStarts.end());
		for (std::vector<uint32_t>::const_iterator it=nodeStarts.begin(); it != nodeStarts.end(); ++it) {
			printf("0x%04X: ", *it);
			const uint8_t* p = start + *it;
			uint64_t exportInfoSize = read_uleb128(p, end);
			if ( exportInfoSize != 0 ) {
				// print terminal info
				uint64_t flags = read_uleb128(p, end);
				if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
					uint64_t ordinal = read_uleb128(p, end);
					const char* importName = (const char*)p;
					while (*p != '\0')
						++p;
					++p;
					if ( strlen(importName) == 0 )
						printf("[flags=REEXPORT ordinal=%llu] ", ordinal);
					else
						printf("[flags=REEXPORT ordinal=%llu import=%s] ", ordinal, importName);
				}
				else if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) {
					uint64_t stub = read_uleb128(p, end);
					uint64_t resolver = read_uleb128(p, end);
					printf("[flags=STUB_AND_RESOLVER stub=0x%06llX resolver=0x%06llX] ", stub, resolver);
				}
				else {
					uint64_t address = read_uleb128(p, end);
					if ( (flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_REGULAR )
						printf("[addr=0x%06llX] ", address);
					else if ( (flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL)
						printf("[flags=THREAD_LOCAL addr=0x%06llX] ", address);
					else if ( (flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE)
						printf("[flags=ABSOLUTE addr=0x%06llX] ", address);
					else
						printf("[flags=0x%llX addr=0x%06llX] ", flags, address);
				}
			}
			// print child edges
			const uint8_t childrenCount = *p++;
			for (uint8_t i=0; i < childrenCount; ++i) {
				const char* edgeName = (const char*)p;
				while (*p != '\0')
					++p;
				++p;
				uint32_t childNodeOffet = read_uleb128(p, end);
				printf("%s->0x%04X", edgeName, childNodeOffet);
				if ( i < (childrenCount-1) )
					printf(", ");
			}
			printf("\n");
		}
	}
}



template <typename A>
const uint8_t* DyldInfoPrinter<A>::printSharedRegionV1InfoForEachULEB128Address(const uint8_t* p, const uint8_t* end, uint8_t kind)
{
	const char* kindStr =  "??";
	switch (kind ) {
		case DYLD_CACHE_ADJ_V1_POINTER_32:
			kindStr = "32-bit pointer";
			break;
		case DYLD_CACHE_ADJ_V1_POINTER_64:
			kindStr = "64-bit pointer";
			break;
		case DYLD_CACHE_ADJ_V1_ADRP:
			kindStr = "arm64 ADRP";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+0:
			kindStr = "thumb2 movt low high 4 bits=0";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+1:
			kindStr = "thumb2 movt low high 4 bits=1";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+2:
			kindStr = "thumb2 movt low high 4 bits=2";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+3:
			kindStr = "thumb2 movt low high 4 bits=3";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+4:
			kindStr = "thumb2 movt low high 4 bits=4";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+5:
			kindStr = "thumb2 movt low high 4 bits=5";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+6:
			kindStr = "thumb2 movt low high 4 bits=6";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+7:
			kindStr = "thumb2 movt low high 4 bits=7";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+8:
			kindStr = "thumb2 movt low high 4 bits=8";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+9:
			kindStr = "thumb2 movt low high 4 bits=9";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+10:
			kindStr = "thumb2 movt low high 4 bits=10";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+11:
			kindStr = "thumb2 movt low high 4 bits=11";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+12:
			kindStr = "thumb2 movt low high 4 bits=12";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+13:
			kindStr = "thumb2 movt low high 4 bits=13";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+14:
			kindStr = "thumb2 movt low high 4 bits=14";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_THUMB_MOVT+15:
			kindStr = "thumb2 movt low high 4 bits=15";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+0:
			kindStr = "arm movt low high 4 bits=0";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+1:
			kindStr = "arm movt low high 4 bits=1";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+2:
			kindStr = "arm movt low high 4 bits=2";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+3:
			kindStr = "arm movt low high 4 bits=3";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+4:
			kindStr = "arm movt low high 4 bits=4";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+5:
			kindStr = "arm movt low high 4 bits=5";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+6:
			kindStr = "arm movt low high 4 bits=6";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+7:
			kindStr = "arm movt low high 4 bits=7";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+8:
			kindStr = "arm movt low high 4 bits=8";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+9:
			kindStr = "arm movt low high 4 bits=9";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+10:
			kindStr = "arm movt low high 4 bits=10";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+11:
			kindStr = "arm movt low high 4 bits=11";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+12:
			kindStr = "arm movt low high 4 bits=12";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+13:
			kindStr = "arm movt low high 4 bits=13";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+14:
			kindStr = "arm movt low high 4 bits=14";
			break;
		case DYLD_CACHE_ADJ_V1_ARM_MOVT+15:
			kindStr = "arm movt low high 4 bits=15";
			break;
		default:
			kindStr = "<<unknown>>";
	}
	uint64_t address = 0;
	uint64_t delta = 0;
	do {
		delta = read_uleb128(p, end);
		address += delta;
		printf("0x%0llX   %s\n", address+fBaseAddress, kindStr); 
	} while (delta);

	return p;
}

template <typename A>
void DyldInfoPrinter<A>::printSharedRegionInfo()
{
	if ( (fSharedRegionInfo == NULL) || (fSharedRegionInfo->datasize() == 0) ) {
		printf("no shared region info\n");
	}
	else {
		const uint8_t* infoStart = (uint8_t*)fHeader + fSharedRegionInfo->dataoff();
		const uint8_t* infoEnd = &infoStart[fSharedRegionInfo->datasize()];
		if ( *infoStart == DYLD_CACHE_ADJ_V2_FORMAT ) {
			++infoStart;
			// Whole		 :== <count> FromToSection+
			// FromToSection :== <from-sect-index> <to-sect-index> <count> ToOffset+
			// ToOffset		 :== <to-sect-offset-delta> <count> FromOffset+
			// FromOffset	 :== <kind> <count> <from-sect-offset-delta>
			const uint8_t* p = infoStart;
			uint64_t sectionCount = read_uleb128(p, infoEnd);
			for (uint64_t i=0; i < sectionCount; ++i) {
				uint64_t fromSectionIndex = read_uleb128(p, infoEnd);
				uint64_t toSectionIndex = read_uleb128(p, infoEnd);
				uint64_t toOffsetCount = read_uleb128(p, infoEnd);
				const macho_section<P>* fromSection = fSections[fromSectionIndex];
				const macho_section<P>* toSection = fSections[toSectionIndex];
				char fromSectionName[20];
				strncpy(fromSectionName, fromSection->sectname(), 16);
				fromSectionName[16] = '\0';
				printf("from sect=%s/%s, to sect=%s/%s, count=%lld:\n", fromSection->segname(), fromSectionName, toSection->segname(), toSection->sectname(), toOffsetCount);
				uint64_t toSectionOffset = 0;
				const char* lastFromSymbol = NULL;
				for (uint64_t j=0; j < toOffsetCount; ++j) {
					uint64_t toSectionDelta = read_uleb128(p, infoEnd);
					uint64_t fromOffsetCount = read_uleb128(p, infoEnd);
					toSectionOffset += toSectionDelta;
					for (uint64_t k=0; k < fromOffsetCount; ++k) {
						uint64_t kind = read_uleb128(p, infoEnd);
						uint64_t fromSectDeltaCount = read_uleb128(p, infoEnd);
						uint64_t fromSectionOffset = 0;
						for (uint64_t l=0; l < fromSectDeltaCount; ++l) {
							uint64_t delta = read_uleb128(p, infoEnd);
							fromSectionOffset += delta;
							uint64_t symbolOffset;
							const char* s = closestSymbolNameForAddress(fromSection->addr()+fromSectionOffset, &symbolOffset, fromSectionIndex);
							if ( (s != lastFromSymbol) && (s != NULL) )
								printf("  %s:\n", s);
							const char* toSymbol = closestSymbolNameForAddress(toSection->addr()+toSectionOffset, &symbolOffset, toSectionIndex);
							printf("       from addr=0x%0llX %s to addr=0x%0llX", fromSection->addr()+fromSectionOffset, sharedRegionKindName(kind), toSection->addr()+toSectionOffset);
							if ( toSymbol != NULL ) {
								if ( symbolOffset == 0 )
									printf(" (%s)", toSymbol);
								else
									printf(" (%s + %lld)", toSymbol, symbolOffset);
							}
							printf("\n");
							lastFromSymbol = s;
						}
					}
				}
			}
		}
		else {
			for(const uint8_t* p = infoStart; (*p != 0) && (p < infoEnd);) {
				uint8_t kind = *p++;
				p = this->printSharedRegionV1InfoForEachULEB128Address(p, infoEnd, kind);
			}
		}
	}
}

template <typename A>
const char* DyldInfoPrinter<A>::sharedRegionKindName(uint8_t kind)
{
	switch (kind) {
		default:
			return "<<unknown>>";
		case DYLD_CACHE_ADJ_V2_POINTER_32:
			return "pointer32";
		case DYLD_CACHE_ADJ_V2_POINTER_64:
			return "pointer64";
		case DYLD_CACHE_ADJ_V2_DELTA_32:
			return "delta32";
		case DYLD_CACHE_ADJ_V2_DELTA_64:
			return "delta64";
		case DYLD_CACHE_ADJ_V2_ARM64_ADRP:
			return "adrp";
		case DYLD_CACHE_ADJ_V2_ARM64_OFF12:
			return "off12";
		case DYLD_CACHE_ADJ_V2_ARM64_BR26:
			return "br26";
		case DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT:
			return "movw/movt";
		case DYLD_CACHE_ADJ_V2_ARM_BR24:
			return "br24";
		case DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT:
			return "movw/movt";
		case DYLD_CACHE_ADJ_V2_THUMB_BR22:
			return "br22";
		case DYLD_CACHE_ADJ_V2_IMAGE_OFF_32:
			return "off32";
		case DYLD_CACHE_ADJ_V2_THREADED_POINTER_64:
			return "theaded-pointer64";
	}
}


#if SUPPORT_ARCH_arm_any
template <>
void DyldInfoPrinter<arm>::printFunctionStartLine(uint64_t addr)
{
	if ( addr & 1 )
		printf("0x%0llX [thumb] %s\n", (addr & -2), symbolNameForAddress(addr & -2)); 
	else
		printf("0x%0llX         %s\n", addr, symbolNameForAddress(addr)); 
}
#endif

template <typename A>
void DyldInfoPrinter<A>::printFunctionStartLine(uint64_t addr)
{
	printf("0x%0llX   %s\n", addr, symbolNameForAddress(addr)); 
}


template <typename A>
void DyldInfoPrinter<A>::printFunctionStartsInfo()
{
	if ( (fFunctionStartsInfo == NULL) || (fFunctionStartsInfo->datasize() == 0) ) {
		printf("no function starts info\n");
	}
	else {
		const uint8_t* infoStart = (uint8_t*)fHeader + fFunctionStartsInfo->dataoff();
		const uint8_t* infoEnd = &infoStart[fFunctionStartsInfo->datasize()];
		uint64_t address = fBaseAddress;
		for(const uint8_t* p = infoStart; (*p != 0) && (p < infoEnd); ) {
			uint64_t delta = 0;
			uint32_t shift = 0;
			bool more = true;
			do {
				uint8_t byte = *p++;
				delta |= ((byte & 0x7F) << shift);
				shift += 7;
				if ( byte < 0x80 ) {
					address += delta;
					printFunctionStartLine(address);
					more = false;
				}
			} while (more);
		}
	}
}

template <typename A>
void DyldInfoPrinter<A>::printDylibsInfo()
{
	printf("attributes     dependent dylibs\n");
	for(typename std::vector<const macho_dylib_command<P>*>::iterator it = fDylibLoadCommands.begin(); it != fDylibLoadCommands.end(); ++it) {
		const macho_dylib_command<P>* dylib  = *it;
		const char* attribute = "";
		switch ( dylib->cmd() ) {
			case LC_LOAD_WEAK_DYLIB:
				attribute = "weak_import";
				break;
			case LC_REEXPORT_DYLIB:
				attribute = "re-export";
				break;
			case LC_LOAD_UPWARD_DYLIB:
				attribute = "upward";
				break;
			case LC_LAZY_LOAD_DYLIB:
				attribute = "lazy_load";
				break;
			case LC_LOAD_DYLIB:
			default:
				break;
		}
		printf(" %-12s   %s\n", attribute, dylib->name());
	}
}

template <typename A>
void DyldInfoPrinter<A>::printDRInfo()
{
	if ( fDRInfo == NULL ) {
		printf("no Designated Requirements info\n");
	}
	else {
		printf("dylibs                 DRs\n");
		const uint8_t* start = ((uint8_t*)fHeader + fDRInfo->dataoff());
		//const uint8_t* end   = ((uint8_t*)fHeader + fDRInfo->dataoff() + fDRInfo->datasize());
		typedef Security::SuperBlob<Security::kSecCodeMagicDRList> DRListSuperBlob;
		const DRListSuperBlob* topBlob = (DRListSuperBlob*)start;
		if ( topBlob->validateBlob(fDRInfo->datasize()) ) {
			if ( topBlob->count() == fDylibLoadCommands.size() ) {
				for(unsigned i=0; i < topBlob->count(); ++i) {
					printf(" %-20s   ", fDylibs[i]);
					const Security::BlobCore* item = topBlob->find(i);
					if ( item != NULL ) {
						const uint8_t* itemStart = (uint8_t*)item;
						const uint8_t* itemEnd = itemStart + item->length();
						for(const uint8_t* p=itemStart; p < itemEnd; ++p)
							printf("%02X ", *p);
					}
					else {
						printf("no DR info");
					}
					printf("\n");
				}
			}
			else {
				fprintf(stderr, "superblob of DRs has a different number of elements than dylib load commands\n");
			}
		}
		else {
			fprintf(stderr, "superblob of DRs invalid\n");
		}
	}
}





template <typename A>
void DyldInfoPrinter<A>::printDataInCode()
{
	if ( fDataInCode == NULL ) {
		printf("no data-in-code info\n");
	}
	else {
		printf("offset      length  data-kind\n");
		const macho_data_in_code_entry<P>* start = (macho_data_in_code_entry<P>*)((uint8_t*)fHeader + fDataInCode->dataoff());
		const macho_data_in_code_entry<P>* end = (macho_data_in_code_entry<P>*)((uint8_t*)fHeader + fDataInCode->dataoff() + fDataInCode->datasize());
		for (const macho_data_in_code_entry<P>* p=start; p < end; ++p) {
			const char* kindStr = "???";
			switch ( p->kind() ) {
				case 1:
					kindStr = "data";
					break;
				case 2:
					kindStr = "jumptable8";
					break;
				case 3:
					kindStr = "jumptable16";
					break;
				case 4:
					kindStr = "jumptable32";
					break;
				case 5:
					kindStr = "jumptable32absolute";
					break;
			}
			printf("0x%08X  0x%04X  %s\n", p->offset(), p->length(), kindStr);
		}
	}
}



template <>
x86_64::P::uint_t DyldInfoPrinter<x86_64>::localRelocBase()
{
	if (fHeader->filetype() == MH_KEXT_BUNDLE) {
		// for kext bundles the reloc base address starts at __TEXT segment
		return fFirstSegment->vmaddr();
	}
	// for all other kinds, the x86_64 reloc base address starts at first writable segment (usually __DATA)
	return fFirstWritableSegment->vmaddr();
}

template <typename A>
typename A::P::uint_t DyldInfoPrinter<A>::localRelocBase()
{
	return fFirstSegment->vmaddr();
}


template <>
x86_64::P::uint_t DyldInfoPrinter<x86_64>::externalRelocBase()
{
	if (fHeader->filetype() == MH_KEXT_BUNDLE) {
		// for kext bundles the reloc base address starts at __TEXT segment
		return fFirstSegment->vmaddr();;
	}
	return fFirstWritableSegment->vmaddr();
}

template <typename A>
typename A::P::uint_t DyldInfoPrinter<A>::externalRelocBase()
{
	return 0;
}



template <>
const char*	DyldInfoPrinter<ppc>::relocTypeName(uint8_t r_type)
{
	if ( r_type == GENERIC_RELOC_VANILLA )
		return "pointer";
	else
		return "??";
}
	
template <>
const char*	DyldInfoPrinter<ppc64>::relocTypeName(uint8_t r_type)
{
	if ( r_type == GENERIC_RELOC_VANILLA )
		return "pointer";
	else
		return "??";
}
	
template <>
const char*	DyldInfoPrinter<x86>::relocTypeName(uint8_t r_type)
{
	if ( r_type == GENERIC_RELOC_VANILLA )
		return "pointer";
	else if ( r_type == GENERIC_RELOC_PB_LA_PTR )
		return "pb pointer";
	else
		return "??";
}
	
template <>
const char*	DyldInfoPrinter<x86_64>::relocTypeName(uint8_t r_type)
{
	if ( r_type == X86_64_RELOC_UNSIGNED )
		return "pointer";
	else if ( r_type == X86_64_RELOC_BRANCH )
		return "branch";
	else
		return "??";
}
	
#if SUPPORT_ARCH_arm_any
template <>
const char*	DyldInfoPrinter<arm>::relocTypeName(uint8_t r_type)
{
	if ( r_type == ARM_RELOC_VANILLA )
		return "pointer";
	else if ( r_type == ARM_RELOC_PB_LA_PTR )
		return "pb pointer";
	else
		return "??";
}
#endif

#if SUPPORT_ARCH_arm64	
template <>
const char*	DyldInfoPrinter<arm64>::relocTypeName(uint8_t r_type)
{
	if ( r_type == ARM64_RELOC_UNSIGNED )
		return "pointer";
	return "??";
}
#endif

#if SUPPORT_ARCH_arm64_32
template <>
const char*	DyldInfoPrinter<arm64_32>::relocTypeName(uint8_t r_type)
{
	if ( r_type == ARM64_RELOC_UNSIGNED )
		return "pointer";
	return "??";
}
#endif

template <typename A>
void DyldInfoPrinter<A>::printRelocRebaseInfo()
{
	// First check if we can find a magic section for threaded rebase
	{
		auto rebaseChain = [this](uintptr_t chainStartMappedAddress, uintptr_t chainStartVMAddress, uint64_t stepMultiplier, uintptr_t baseAddress) {
			uint64_t delta = 0;
			uintptr_t mappedAddress = chainStartMappedAddress;
			uintptr_t vmAddress = chainStartVMAddress;
			do {
				uint64_t value = *(uint64_t*)mappedAddress;

				uint8_t segIndex = segmentIndexForAddress(vmAddress);
				const char* segName = segmentName(segIndex);
				const char* sectName = sectionName(segIndex, vmAddress);

#if SUPPORT_ARCH_arm64e
				uint16_t diversity = (uint16_t)(value >> 32);
				bool hasAddressDiversity = (value & (1ULL << 48)) != 0;
				uint8_t key = (uint8_t)((value >> 49) & 0x3);
				bool isAuthenticated = (value & (1ULL << 63)) != 0;
#endif
				bool isRebase = (value & (1ULL << 62)) == 0;
				if (isRebase) {

#if SUPPORT_ARCH_arm64e
					if (isAuthenticated) {
						uint64_t slide = 0;
						static const char* keyNames[] = {
							"IA", "IB", "DA", "DB"
						};
						// The new value for a rebase is the low 32-bits of the threaded value plus the slide.
						uint64_t newValue = (value & 0xFFFFFFFF) + slide;
						// Add in the offset from the mach_header
						newValue += baseAddress;
						// We have bits to merge in to the discriminator
						printf("%-7s %-16s 0x%08llX  %s  0x%08llX with value 0x%016llX (JOP: diversity %d, address %s, %s)\n",
							   segName, sectName, (uint64_t)vmAddress, "pointer", newValue, value,
							   diversity, hasAddressDiversity ? "true" : "false", keyNames[key]);
					} else
#endif
					{
						// Regular pointer which needs to fit in 51-bits of value.
						// C++ RTTI uses the top bit, so we'll allow the whole top-byte
						// and the bottom 43-bits to be fit in to 51-bits.
						uint64_t top8Bits = value & 0x0007F80000000000ULL;
						uint64_t bottom43Bits = value & 0x000007FFFFFFFFFFULL;
						uint64_t targetValue = ( top8Bits << 13 ) | (((intptr_t)(bottom43Bits << 21) >> 21) & 0x00FFFFFFFFFFFFFF);
						printf("%-7s %-16s 0x%08llX  %s  0x%08llX with value 0x%016llX\n", segName, sectName, (uint64_t)vmAddress, "pointer", targetValue, value);
					}
				}

				// The delta is bits [51..61]
				// And bit 62 is to tell us if we are a rebase (0) or bind (1)
				value &= ~(1ULL << 62);
				delta = ( value & 0x3FF8000000000000 ) >> 51;
				mappedAddress += delta * stepMultiplier;
				vmAddress += delta * stepMultiplier;
			} while ( delta != 0 );
		};
		for(const macho_segment_command<P>* segCmd : fSegments) {
			if (strcmp(segCmd->segname(), "__TEXT") != 0)
				continue;
			macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
			macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
			for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
				if (strcmp(sect->sectname(), "__thread_starts") != 0)
					continue;
				printf("rebase information (from __TEXT,__thread_starts):\n");
				printf("segment  section          address     type\n");
				const uint8_t* sectionContent = (uint8_t*)fHeader + sect->offset();
				uint32_t *threadStarts = (uint32_t*)sectionContent;
				uint32_t *threadEnds = (uint32_t*)(sectionContent + sect->size());
				uint32_t threadStartsHeader = threadStarts[0];
				uint64_t stepMultiplier = (threadStartsHeader & 1) == 1 ? 8 : 4;
				for (uint32_t* threadOffset = threadStarts + 1; threadOffset != threadEnds; ++threadOffset) {
					//printf("Thread start offset: 0x%016X\n", *threadOffset);
					// If we get a 0xFFFFFFFF offset then ld64 overestimated the size required.  So skip the remainder
					// of the entries.
					if (*threadOffset == 0xFFFFFFFF)
						break;
					uint64_t chainStartVMAddr = fBaseAddress + *threadOffset;
					uintptr_t chainStartMappedAddress = (uintptr_t)mappedAddressForVMAddress(chainStartVMAddr);
					rebaseChain(chainStartMappedAddress, chainStartVMAddr, stepMultiplier, fBaseAddress);
				}
				return;
			}
		}
	}

	if ( fDynamicSymbolTable == NULL ) {
		printf("no classic dynamic symbol table");
	}
	else {
		printf("rebase information (from local relocation records and indirect symbol table):\n");
		printf("segment      section          address     type\n");
		// walk all local relocations
		pint_t rbase = localRelocBase();
		const macho_relocation_info<P>* const relocsStart = (macho_relocation_info<P>*)(((uint8_t*)fHeader) + fDynamicSymbolTable->locreloff());
		const macho_relocation_info<P>* const relocsEnd = &relocsStart[fDynamicSymbolTable->nlocrel()];
		for (const macho_relocation_info<P>* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
			if ( (reloc->r_address() & R_SCATTERED) == 0 ) {
				pint_t addr = reloc->r_address() + rbase;
				uint8_t segIndex = segmentIndexForAddress(addr);
				const char* typeName = relocTypeName(reloc->r_type());
				const char* segName  = segmentName(segIndex);
				const char* sectName = sectionName(segIndex, addr);
				printf("%-12s %-16s 0x%08llX  %s\n", segName, sectName, (uint64_t)addr, typeName);
			} 
			else {
				const macho_scattered_relocation_info<P>* sreloc = (macho_scattered_relocation_info<P>*)reloc;
				pint_t addr = sreloc->r_address() + rbase;
				uint8_t segIndex = segmentIndexForAddress(addr);
				const char* typeName = relocTypeName(sreloc->r_type());
				const char* segName  = segmentName(segIndex);
				const char* sectName = sectionName(segIndex, addr);
				printf("%-12s %-16s 0x%08llX  %s\n", segName, sectName, (uint64_t)addr, typeName);
			}
		}
		// look for local non-lazy-pointers
		const uint32_t* indirectSymbolTable =  (uint32_t*)(((uint8_t*)fHeader) + fDynamicSymbolTable->indirectsymoff());
		uint8_t segIndex = 0;
		for(typename std::vector<const macho_segment_command<P>*>::iterator segit=fSegments.begin(); segit != fSegments.end(); ++segit, ++segIndex) {
			const macho_segment_command<P>* segCmd = *segit;
			macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
			macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
			for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
				uint8_t type = sect->flags() & SECTION_TYPE;
				if ( type == S_NON_LAZY_SYMBOL_POINTERS ) {
					uint32_t indirectOffset = sect->reserved1();
					uint32_t count = sect->size() / sizeof(pint_t);
					for (uint32_t i=0; i < count; ++i) {
						uint32_t symbolIndex = E::get32(indirectSymbolTable[indirectOffset+i]);
						if ( symbolIndex == INDIRECT_SYMBOL_LOCAL ) {
							pint_t addr = sect->addr() + i*sizeof(pint_t);							
							const char* typeName = "pointer";
							const char* segName  = segmentName(segIndex);
							const char* sectName = sectionName(segIndex, addr);
							printf("%-12s %-16s 0x%08llX  %s\n", segName, sectName, (uint64_t)addr, typeName);
						}
					}
				}
			}
		}
	}
}


template <typename A>
void DyldInfoPrinter<A>::printSymbolTableExportInfo()
{
	if ( fDynamicSymbolTable == NULL ) {
		printf("no classic dynamic symbol table");
	}
	else {
		printf("export information (from symbol table):\n");
		const macho_nlist<P>* lastExport = &fSymbols[fDynamicSymbolTable->iextdefsym()+fDynamicSymbolTable->nextdefsym()];
		for (const macho_nlist<P>* sym = &fSymbols[fDynamicSymbolTable->iextdefsym()]; sym < lastExport; ++sym) {
			const char* flags = "";
			if ( sym->n_desc() & N_WEAK_DEF )
				flags = "[weak_def] ";
			pint_t thumb = 0;
			if ( sym->n_desc() & N_ARM_THUMB_DEF )
				thumb = 1;
			printf("0x%08llX %s%s\n", sym->n_value()+thumb, flags, &fStrings[sym->n_strx()]);
		}
	}
}

template <typename A>
const char* DyldInfoPrinter<A>::closestSymbolNameForAddress(uint64_t addr, uint64_t* offset, uint8_t sectIndex)
{
	const macho_nlist<P>* bestSymbol = NULL;
	if ( fDynamicSymbolTable != NULL ) {
		// find closest match in globals
		const macho_nlist<P>* const globalsStart = &fSymbols[fDynamicSymbolTable->iextdefsym()];
		const macho_nlist<P>* const globalsEnd   = &globalsStart[fDynamicSymbolTable->nextdefsym()];
		for (const macho_nlist<P>* s = globalsStart; s < globalsEnd; ++s) {
			if ( (s->n_type() & N_TYPE) == N_SECT ) {
				if ( (s->n_value() <= addr) && ((s->n_sect() == sectIndex) || (sectIndex ==0)) ) {
					if ( (bestSymbol == NULL) || (bestSymbol->n_value() < s->n_value()) )
						bestSymbol = s;
				}
			}
		}

		// find closest match in locals
		const macho_nlist<P>* const localsStart = &fSymbols[fDynamicSymbolTable->ilocalsym()];
		const macho_nlist<P>* const localsEnd   = &localsStart[fDynamicSymbolTable->nlocalsym()];
		for (const macho_nlist<P>* s = localsStart; s < localsEnd; ++s) {
			if ( ((s->n_type() & N_TYPE) == N_SECT) && ((s->n_type() & N_STAB) == 0) ) {
				if ( (s->n_value() <= addr) && ((s->n_sect() == sectIndex) || (sectIndex ==0)) ) {
					if ( (bestSymbol == NULL) || (bestSymbol->n_value() < s->n_value()) )
						bestSymbol = s;
				}
			}
		}
	}
	else {
		// find closest match in locals
		const macho_nlist<P>* const allStart = &fSymbols[0];
		const macho_nlist<P>* const allEnd   = &fSymbols[fSymbolCount];
		for (const macho_nlist<P>* s = allStart; s < allEnd; ++s) {
			if ( ((s->n_type() & N_TYPE) == N_SECT) && ((s->n_type() & N_STAB) == 0) ) {
				if ( (s->n_value() <= addr) && ((s->n_sect() == sectIndex) || (sectIndex ==0)) ) {
					if ( (bestSymbol == NULL) || (bestSymbol->n_value() < s->n_value()) )
						bestSymbol = s;
				}
			}
		}
	}
	if ( bestSymbol != NULL ) {
		*offset = addr - bestSymbol->n_value();
		return &fStrings[bestSymbol->n_strx()];
	}
	*offset = 0;
	return NULL;
}

template <typename A>
const char* DyldInfoPrinter<A>::symbolNameForAddress(uint64_t addr)
{
	uint64_t offset;
	const char* s = closestSymbolNameForAddress(addr, &offset);
	if ( (offset == 0) && (s != NULL) )
		return s;
	return "?";
}

template <typename A>
void DyldInfoPrinter<A>::printClassicBindingInfo()
{
	if ( fDynamicSymbolTable == NULL ) {
		printf("no classic dynamic symbol table");
	}
	else {
		printf("binding information (from external relocations and indirect symbol table):\n");
		printf("segment      section          address        type   weak  addend dylib            symbol\n");
		// walk all external relocations
		pint_t rbase = externalRelocBase();
		const macho_relocation_info<P>* const relocsStart = (macho_relocation_info<P>*)(((uint8_t*)fHeader) + fDynamicSymbolTable->extreloff());
		const macho_relocation_info<P>* const relocsEnd = &relocsStart[fDynamicSymbolTable->nextrel()];
		for (const macho_relocation_info<P>* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
			pint_t addr = reloc->r_address() + rbase;
			uint32_t symbolIndex = reloc->r_symbolnum();
			const macho_nlist<P>* sym = &fSymbols[symbolIndex];
			const char* symbolName = &fStrings[sym->n_strx()];
			const char* weak_import = (sym->n_desc() & N_WEAK_REF) ? "weak" : "";
			const char* fromDylib = classicOrdinalName(GET_LIBRARY_ORDINAL(sym->n_desc()));
			uint8_t segIndex = segmentIndexForAddress(addr);
			const char* typeName = relocTypeName(reloc->r_type());
			const char* segName  = segmentName(segIndex);
			const char* sectName = sectionName(segIndex, addr);
			const pint_t* addressMapped = mappedAddressForVMAddress(addr);
			int64_t addend = P::getP(*addressMapped);
			if ( strcmp(typeName, "pointer") != 0 )
				addend = 0;
			if ( fHeader->flags() & MH_PREBOUND ) {
				// In prebound binaries the content is already pointing to the target.
				// To get the addend requires subtracting out the base address it was prebound to.
				addend -= sym->n_value();
			}
			printf("%-12s %-16s 0x%08llX %10s %4s  %5lld %-16s %s\n", segName, sectName, (uint64_t)addr,
									typeName, weak_import, addend, fromDylib, symbolName);
		}
		// look for non-lazy pointers
		const uint32_t* indirectSymbolTable =  (uint32_t*)(((uint8_t*)fHeader) + fDynamicSymbolTable->indirectsymoff());
		for(typename std::vector<const macho_segment_command<P>*>::iterator segit=fSegments.begin(); segit != fSegments.end(); ++segit) {
			const macho_segment_command<P>* segCmd = *segit;
			macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
			macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
			for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
				uint8_t type = sect->flags() & SECTION_TYPE;
				if ( type == S_NON_LAZY_SYMBOL_POINTERS ) {
					uint32_t indirectOffset = sect->reserved1();
					uint32_t count = sect->size() / sizeof(pint_t);
					for (uint32_t i=0; i < count; ++i) {
						uint32_t symbolIndex = E::get32(indirectSymbolTable[indirectOffset+i]);
						if ( symbolIndex != INDIRECT_SYMBOL_LOCAL ) {
							const macho_nlist<P>* sym = &fSymbols[symbolIndex];
							const char* symbolName = &fStrings[sym->n_strx()];
							const char* weak_import = (sym->n_desc() & N_WEAK_REF) ? "weak" : "";
							const char* fromDylib = classicOrdinalName(GET_LIBRARY_ORDINAL(sym->n_desc()));
							pint_t addr = sect->addr() + i*sizeof(pint_t);
							uint8_t segIndex = segmentIndexForAddress(addr);
							const char* typeName = "pointer";
							const char* segName  = segmentName(segIndex);
							const char* sectName = sectionName(segIndex, addr);
							int64_t addend = 0;
							printf("%-12s %-16s 0x%08llX %10s %4s  %5lld %-16s %s\n", segName, sectName, (uint64_t)addr,
																	typeName, weak_import, addend, fromDylib, symbolName);
						}
					}
				}
			}
		}
	}
}


template <typename A>
void DyldInfoPrinter<A>::printClassicLazyBindingInfo()
{
	if ( fDynamicSymbolTable == NULL ) {
		printf("no classic dynamic symbol table");
	}
	else {
		printf("lazy binding information (from section records and indirect symbol table):\n");
		printf("segment section          address    index  dylib            symbol\n");
		const uint32_t* indirectSymbolTable =  (uint32_t*)(((uint8_t*)fHeader) + fDynamicSymbolTable->indirectsymoff());
		for(typename std::vector<const macho_segment_command<P>*>::iterator segit=fSegments.begin(); segit != fSegments.end(); ++segit) {
			const macho_segment_command<P>* segCmd = *segit;
			macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
			macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
			for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
				uint8_t type = sect->flags() & SECTION_TYPE;
				if ( type == S_LAZY_SYMBOL_POINTERS ) {
					uint32_t indirectOffset = sect->reserved1();
					uint32_t count = sect->size() / sizeof(pint_t);
					for (uint32_t i=0; i < count; ++i) {
						uint32_t symbolIndex = E::get32(indirectSymbolTable[indirectOffset+i]);
						const macho_nlist<P>* sym = &fSymbols[symbolIndex];
						const char* symbolName = &fStrings[sym->n_strx()];
						const char* fromDylib = classicOrdinalName(GET_LIBRARY_ORDINAL(sym->n_desc()));
						pint_t addr = sect->addr() + i*sizeof(pint_t);
						uint8_t segIndex = segmentIndexForAddress(addr);
						const char* segName  = segmentName(segIndex);
						const char* sectName = sectionName(segIndex, addr);
						printf("%-7s %-16s 0x%08llX 0x%04X %-16s %s\n", segName, sectName, (uint64_t)addr, symbolIndex, fromDylib, symbolName);
					}
				}
				else if ( (type == S_SYMBOL_STUBS) && (((sect->flags() & S_ATTR_SELF_MODIFYING_CODE) != 0)) && (sect->reserved2() == 5) ) {
					// i386 self-modifying stubs
					uint32_t indirectOffset = sect->reserved1();
					uint32_t count = sect->size() / 5;
					for (uint32_t i=0; i < count; ++i) {
						uint32_t symbolIndex = E::get32(indirectSymbolTable[indirectOffset+i]);
						if ( symbolIndex != INDIRECT_SYMBOL_ABS ) {
							const macho_nlist<P>* sym = &fSymbols[symbolIndex];
							const char* symbolName = &fStrings[sym->n_strx()];
							const char* fromDylib = classicOrdinalName(GET_LIBRARY_ORDINAL(sym->n_desc()));
							pint_t addr = sect->addr() + i*5;
							uint8_t segIndex = segmentIndexForAddress(addr);
							const char* segName  = segmentName(segIndex);
							const char* sectName = sectionName(segIndex, addr);
							printf("%-7s %-16s 0x%08llX 0x%04X %-16s %s\n", segName, sectName, (uint64_t)addr, symbolIndex, fromDylib, symbolName);
						}
					}
				}
			}
		}
	}
}

static void dump(const char* path)
{
	struct stat stat_buf;
	
	try {
		int fd = ::open(path, O_RDONLY, 0);
		if ( fd == -1 )
			throw "cannot open file";
		if ( ::fstat(fd, &stat_buf) != 0 ) 
			throwf("fstat(%s) failed, errno=%d\n", path, errno);
		uint32_t length = stat_buf.st_size;
		uint8_t* p = (uint8_t*)::mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
		if ( p == ((uint8_t*)(-1)) )
			throw "cannot map file";
		::close(fd);
		const mach_header* mh = (mach_header*)p;
		if ( mh->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
			const struct fat_header* fh = (struct fat_header*)p;
			const struct fat_arch* archs = (struct fat_arch*)(p + sizeof(struct fat_header));
			for (unsigned long i=0; i < OSSwapBigToHostInt32(fh->nfat_arch); ++i) {
				size_t offset = OSSwapBigToHostInt32(archs[i].offset);
				size_t size = OSSwapBigToHostInt32(archs[i].size);
				cpu_type_t cputype = OSSwapBigToHostInt32(archs[i].cputype);
				cpu_type_t cpusubtype = OSSwapBigToHostInt32(archs[i].cpusubtype);
				if ( ((cputype == sPreferredArch) 
					&& ((sPreferredSubArch==0) || (sPreferredSubArch==cpusubtype)))
					|| (sPreferredArch == 0) ) {	
					switch(cputype) {
					case CPU_TYPE_POWERPC:
						if ( DyldInfoPrinter<ppc>::validFile(p + offset) )
							DyldInfoPrinter<ppc>::make(p + offset, size, path, (sPreferredArch == 0));
						else
							throw "in universal file, ppc slice does not contain ppc mach-o";
						break;
					case CPU_TYPE_I386:
						if ( DyldInfoPrinter<x86>::validFile(p + offset) )
							DyldInfoPrinter<x86>::make(p + offset, size, path, (sPreferredArch == 0));
						else
							throw "in universal file, i386 slice does not contain i386 mach-o";
						break;
					case CPU_TYPE_POWERPC64:
						if ( DyldInfoPrinter<ppc64>::validFile(p + offset) )
							DyldInfoPrinter<ppc64>::make(p + offset, size, path, (sPreferredArch == 0));
						else
							throw "in universal file, ppc64 slice does not contain ppc64 mach-o";
						break;
					case CPU_TYPE_X86_64:
						if ( DyldInfoPrinter<x86_64>::validFile(p + offset) )
							DyldInfoPrinter<x86_64>::make(p + offset, size, path, (sPreferredArch == 0));
						else
							throw "in universal file, x86_64 slice does not contain x86_64 mach-o";
						break;
#if SUPPORT_ARCH_arm_any
					case CPU_TYPE_ARM:
						if ( DyldInfoPrinter<arm>::validFile(p + offset) ) 
							DyldInfoPrinter<arm>::make(p + offset, size, path, (sPreferredArch == 0));
						else
							throw "in universal file, arm slice does not contain arm mach-o";
						break;
#endif
#if SUPPORT_ARCH_arm64
					case CPU_TYPE_ARM64:
						if ( DyldInfoPrinter<arm64>::validFile(p + offset) )
							DyldInfoPrinter<arm64>::make(p + offset, size, path, (sPreferredArch == 0));
						else
							throw "in universal file, arm64 slice does not contain arm64 mach-o";
						break;
#endif
#if SUPPORT_ARCH_arm64_32
					case CPU_TYPE_ARM64_32:
						if ( DyldInfoPrinter<arm64_32>::validFile(p + offset) )
							DyldInfoPrinter<arm64_32>::make(p + offset, size, path, (sPreferredArch == 0));
						else
							throw "in universal file, arm64_32 slice does not contain arm64 mach-o";
						break;
#endif
					default:
							throwf("in universal file, unknown architecture slice 0x%x\n", cputype);
					}
				}
			}
		}
		else if ( DyldInfoPrinter<x86>::validFile(p) ) {
			DyldInfoPrinter<x86>::make(p, length, path, false);
		}
		else if ( DyldInfoPrinter<ppc>::validFile(p) ) {
			DyldInfoPrinter<ppc>::make(p, length, path, false);
		}
		else if ( DyldInfoPrinter<ppc64>::validFile(p) ) {
			DyldInfoPrinter<ppc64>::make(p, length, path, false);
		}
		else if ( DyldInfoPrinter<x86_64>::validFile(p) ) {
			DyldInfoPrinter<x86_64>::make(p, length, path, false);
		}
#if SUPPORT_ARCH_arm_any
		else if ( DyldInfoPrinter<arm>::validFile(p) ) {
			DyldInfoPrinter<arm>::make(p, length, path, false);
		}
#endif
#if SUPPORT_ARCH_arm64
		else if ( DyldInfoPrinter<arm64>::validFile(p) ) {
			DyldInfoPrinter<arm64>::make(p, length, path, false);
		}
#endif
#if SUPPORT_ARCH_arm64_32
		else if ( DyldInfoPrinter<arm64_32>::validFile(p) ) {
			DyldInfoPrinter<arm64_32>::make(p, length, path, false);
		}
#endif
		else {
			throw "not a known file type";
		}
	}
	catch (const char* msg) {
		throwf("%s in %s", msg, path);
	}
}

static void usage()
{
	fprintf(stderr, "Usage: dyldinfo [-arch <arch>] <options> <mach-o file>\n"
			"\t-dylibs           print dependent dylibs\n"
			"\t-dr               print dependent dylibs and show any recorded DR info\n"
			"\t-rebase           print addresses dyld will adjust if file not loaded at preferred address\n"
			"\t-bind             print addresses dyld will set based on symbolic lookups\n"
			"\t-weak_bind        print symbols which dyld must coalesce\n"
			"\t-lazy_bind        print addresses dyld will lazily set on first use\n"
			"\t-export           print addresses of all symbols this file exports\n"
			"\t-opcodes          print opcodes used to generate the rebase and binding information\n"
			"\t-function_starts  print table of function start addresses\n"
			"\t-export_dot       print a GraphViz .dot file of the exported symbols trie\n"
			"\t-data_in_code     print any data-in-code information\n"
		);
}


int main(int argc, const char* argv[])
{
	if ( argc == 1 ) {
		usage();
		return 0;
	}

	try {
		std::vector<const char*> files;
		for(int i=1; i < argc; ++i) {
			const char* arg = argv[i];
			if ( arg[0] == '-' ) {
				if ( strcmp(arg, "-arch") == 0 ) {
					const char* arch = ++i<argc? argv[i]: "";
					if ( strcmp(arch, "ppc64") == 0 )
						sPreferredArch = CPU_TYPE_POWERPC64;
					else if ( strcmp(arch, "ppc") == 0 )
						sPreferredArch = CPU_TYPE_POWERPC;
					else {
						if ( arch == NULL )
							throw "-arch missing architecture name";
						bool found = false;
						for (const ArchInfo* t=archInfoArray; t->archName != NULL; ++t) {
							if ( strcmp(t->archName,arch) == 0 ) {
								sPreferredArch = t->cpuType;
								if ( t->isSubType )
									sPreferredSubArch = t->cpuSubType;
								found = true;
								break;
							}
						}
						if ( !found )
							throwf("unknown architecture %s", arch);
					}
				}
				else if ( strcmp(arg, "-rebase") == 0 ) {
					printRebase = true;
				}
				else if ( strcmp(arg, "-bind") == 0 ) {
					printBind = true;
				}
				else if ( strcmp(arg, "-weak_bind") == 0 ) {
					printWeakBind = true;
				}
				else if ( strcmp(arg, "-lazy_bind") == 0 ) {
					printLazyBind = true;
				}
				else if ( strcmp(arg, "-export") == 0 ) {
					printExport = true;
				}
				else if ( strcmp(arg, "-opcodes") == 0 ) {
					printOpcodes = true;
				}
				else if ( strcmp(arg, "-export_dot") == 0 ) {
					printExportGraph = true;
				}
				else if ( strcmp(arg, "-export_trie_nodes") == 0 ) {
					printExportNodes = true;
				}
				else if ( strcmp(arg, "-shared_region") == 0 ) {
					printSharedRegion = true;
				}
				else if ( strcmp(arg, "-function_starts") == 0 ) {
					printFunctionStarts = true;
				}
				else if ( strcmp(arg, "-dylibs") == 0 ) {
					printDylibs = true;
				}
				else if ( strcmp(arg, "-dr") == 0 ) {
					printDRs = true;
				}
				else if ( strcmp(arg, "-data_in_code") == 0 ) {
					printDataCode = true;
				}
				else {
					throwf("unknown option: %s\n", arg);
				}
			}
			else {
				files.push_back(arg);
			}
		}
		if ( files.size() == 0 )
			usage();
		if ( files.size() == 1 ) {
			dump(files[0]);
		}
		else {
			for(std::vector<const char*>::iterator it=files.begin(); it != files.end(); ++it) {
				printf("\n%s:\n", *it);
				dump(*it);
			}
		}
	}
	catch (const char* msg) {
		fprintf(stderr, "dyldinfo failed: %s\n", msg);
		return 1;
	}
	
	return 0;
}



