/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
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
#include <set>
#include <unordered_set>

#include "configure.h"
#include "MachOFileAbstraction.hpp"
#include "Architectures.hpp"


 __attribute__((noreturn))
void throwf(const char* format, ...) 
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
class UnwindPrinter
{
public:
	static bool									validFile(const uint8_t* fileContent);
	static UnwindPrinter<A>*						make(const uint8_t* fileContent, uint32_t fileLength, 
															const char* path, bool showFunctionNames) 
														{ return new UnwindPrinter<A>(fileContent, fileLength, 
																						path, showFunctionNames); }
	virtual										~UnwindPrinter() {}


private:
	typedef typename A::P					P;
	typedef typename A::P::E				E;
	typedef typename A::P::uint_t			pint_t;
	
												UnwindPrinter(const uint8_t* fileContent, uint32_t fileLength, 
																const char* path, bool showFunctionNames);
	bool										findUnwindSection();
	void										printUnwindSection(bool showFunctionNames);
	void										printObjectUnwindSection(bool showFunctionNames);
	void										getSymbolTableInfo();
	const char*									functionName(pint_t addr, uint32_t* offset=NULL);
	const char*									personalityName(const macho_relocation_info<typename A::P>* reloc);
	bool										hasExernReloc(uint64_t sectionOffset, const char** personalityStr, pint_t* addr=NULL);

	static const char*							archName();
	static void									decode(uint32_t encoding, const uint8_t* funcStart, char* str);
		
	const char*									fPath;
	const macho_header<P>*						fHeader;
	uint64_t									fLength;
	const macho_section<P>*						fUnwindSection;
	const char*									fStrings;
	const char*									fStringsEnd;
	const macho_nlist<P>*						fSymbols;
	uint32_t									fSymbolCount;
	pint_t										fMachHeaderAddress;
};


template <>	 const char*	UnwindPrinter<x86>::archName()		{ return "i386"; }
template <>	 const char*	UnwindPrinter<x86_64>::archName()	{ return "x86_64"; }
template <>	 const char*	UnwindPrinter<arm>::archName()		{ return "arm"; }
#if SUPPORT_ARCH_arm64
template <>	 const char*	UnwindPrinter<arm64>::archName()	{ return "arm64"; }
#endif
#if SUPPORT_ARCH_arm64_32
template <>	 const char*	UnwindPrinter<arm64_32>::archName()	{ return "arm64_32"; }
#endif

template <>
bool UnwindPrinter<x86>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_I386 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_OBJECT:
			return true;
	}
	return false;
}

template <>
bool UnwindPrinter<x86_64>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC_64 )
		return false;
	if ( header->cputype() != CPU_TYPE_X86_64 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_OBJECT:
			return true;
	}
	return false;
}


#if SUPPORT_ARCH_arm64
template <>
bool UnwindPrinter<arm64>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC_64 )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM64 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_OBJECT:
			return true;
	}
	return false;
}
#endif

#if SUPPORT_ARCH_arm64_32
template <>
bool UnwindPrinter<arm64_32>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM64_32 )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_OBJECT:
			return true;
	}
	return false;
}
#endif

template <>
bool UnwindPrinter<arm>::validFile(const uint8_t* fileContent)
{	
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM )
		return false;
	switch (header->filetype()) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_BUNDLE:
		case MH_DYLINKER:
		case MH_OBJECT:
			return true;
	}
	return false;
}

template <typename A>
UnwindPrinter<A>::UnwindPrinter(const uint8_t* fileContent, uint32_t fileLength, const char* path, bool showFunctionNames)
 : fHeader(NULL), fLength(fileLength), fUnwindSection(NULL),
   fStrings(NULL), fStringsEnd(NULL), fSymbols(NULL), fSymbolCount(0), fMachHeaderAddress(0)
{
	// sanity check
	if ( ! validFile(fileContent) )
		throw "not a mach-o file that can be checked";

	fPath = strdup(path);
	fHeader = (const macho_header<P>*)fileContent;
	
	getSymbolTableInfo();
	
	if ( findUnwindSection() ) {
		if ( fHeader->filetype() == MH_OBJECT ) 
			printObjectUnwindSection(showFunctionNames);
		else
			printUnwindSection(showFunctionNames);
	}
}


template <typename A>
void UnwindPrinter<A>::getSymbolTableInfo()
{
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
		if ( cmd->cmd()	== LC_SYMTAB) {
			const macho_symtab_command<P>* symtab = (macho_symtab_command<P>*)cmd;
			fSymbolCount = symtab->nsyms();
			fSymbols = (const macho_nlist<P>*)((char*)fHeader + symtab->symoff());
			fStrings = (char*)fHeader + symtab->stroff();
			fStringsEnd = fStrings + symtab->strsize();
		}
		cmd = (const macho_load_command<P>*)endOfCmd;
	}
}

template <typename A>
const char* UnwindPrinter<A>::functionName(pint_t addr, uint32_t* offset)
{
	const macho_nlist<P>* closestSymbol = NULL;
	if ( offset != NULL )
		*offset = 0;
	for (uint32_t i=0; i < fSymbolCount; ++i) {
		uint8_t type = fSymbols[i].n_type();
		if ( ((type & N_STAB) == 0) && ((type & N_TYPE) == N_SECT) ) {
			pint_t value = fSymbols[i].n_value();
			if ( value == addr ) {
				const char* r = &fStrings[fSymbols[i].n_strx()];
				return r;
			}
			if ( fSymbols[i].n_desc() & N_ARM_THUMB_DEF ) 
				value |= 1;
			if ( value == addr ) {
				const char* r = &fStrings[fSymbols[i].n_strx()];
				//fprintf(stderr, "addr=0x%08llX, i=%u, n_type=0x%0X, r=%s\n", (long long)(fSymbols[i].n_value()), i,  fSymbols[i].n_type(), r);
				return r;
			}
			else if ( offset != NULL ) {
				if ( closestSymbol == NULL ) {
					if ( fSymbols[i].n_value() < addr )
						closestSymbol = &fSymbols[i];
				}
				else {
					if ( (fSymbols[i].n_value() < addr) && (fSymbols[i].n_value() > closestSymbol->n_value()) )
						closestSymbol = &fSymbols[i];
				}
			}
		}
	}
	if ( closestSymbol != NULL ) {
		*offset = addr - closestSymbol->n_value();
		return &fStrings[closestSymbol->n_strx()];
	}
	return "--anonymous function--";
}



template <typename A>
bool UnwindPrinter<A>::findUnwindSection()
{
	const char* unwindSectionName = "__unwind_info";
	const char* unwindSegmentName = "__TEXT";
	if ( fHeader->filetype() == MH_OBJECT ) {
		unwindSectionName = "__compact_unwind";
		unwindSegmentName = "__LD";
	}
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
		if ( cmd->cmd() == macho_segment_command<P>::CMD ) {
			const macho_segment_command<P>* segCmd = (const macho_segment_command<P>*)cmd;
			const macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
			const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
			for(const macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
				if ( (strncmp(sect->sectname(), unwindSectionName, 16) == 0) && (strcmp(sect->segname(), unwindSegmentName) == 0) ) {
					fUnwindSection = sect;
					fMachHeaderAddress = segCmd->vmaddr();
					return fUnwindSection;
				}
			}
		}
		cmd = (const macho_load_command<P>*)endOfCmd;
	}
	return false;
}
	
#define EXTRACT_BITS(value, mask) \
	( (value >> __builtin_ctz(mask)) & (((1 << __builtin_popcount(mask)))-1) )


template <>
void UnwindPrinter<x86_64>::decode(uint32_t encoding, const uint8_t* funcStart, char* str)
{
	*str = '\0';
	switch ( encoding & UNWIND_X86_64_MODE_MASK ) {
		case UNWIND_X86_64_MODE_RBP_FRAME:
		{
			uint32_t savedRegistersOffset = EXTRACT_BITS(encoding, UNWIND_X86_64_RBP_FRAME_OFFSET);
			uint32_t savedRegistersLocations = EXTRACT_BITS(encoding, UNWIND_X86_64_RBP_FRAME_REGISTERS);
			if ( savedRegistersLocations == 0 ) {
				strcpy(str, "rbp frame, no saved registers");
			}
			else {
				sprintf(str, "rbp frame, at -%d:", savedRegistersOffset*8);
				bool needComma = false;
				for (int i=0; i < 5; ++i) {
					if ( needComma ) 
						strcat(str, ",");
					else
						needComma = true;
					switch (savedRegistersLocations & 0x7) {
						case UNWIND_X86_64_REG_NONE:
							strcat(str, "-");
							break;
						case UNWIND_X86_64_REG_RBX:
							strcat(str, "rbx");
							break;
						case UNWIND_X86_64_REG_R12:
							strcat(str, "r12");
							break;
						case UNWIND_X86_64_REG_R13:
							strcat(str, "r13");
							break;
						case UNWIND_X86_64_REG_R14:
							strcat(str, "r14");
							break;
						case UNWIND_X86_64_REG_R15:
							strcat(str, "r15");
							break;
						default:
							strcat(str, "r?");
					}
					savedRegistersLocations = (savedRegistersLocations >> 3);
					if ( savedRegistersLocations == 0 )
						break;
				}
			}
		}
		break;
		case UNWIND_X86_64_MODE_STACK_IMMD:
		case UNWIND_X86_64_MODE_STACK_IND:
		{
			uint32_t stackSize = EXTRACT_BITS(encoding, UNWIND_X86_64_FRAMELESS_STACK_SIZE);
			uint32_t stackAdjust = EXTRACT_BITS(encoding, UNWIND_X86_64_FRAMELESS_STACK_ADJUST);
			uint32_t regCount = EXTRACT_BITS(encoding, UNWIND_X86_64_FRAMELESS_STACK_REG_COUNT);
			uint32_t permutation = EXTRACT_BITS(encoding, UNWIND_X86_64_FRAMELESS_STACK_REG_PERMUTATION);
			if ( (encoding & UNWIND_X86_64_MODE_MASK) == UNWIND_X86_64_MODE_STACK_IND ) {
				// stack size is encoded in subl $xxx,%esp instruction
				uint32_t subl = x86_64::P::E::get32(*((uint32_t*)(funcStart+stackSize)));
				sprintf(str, "stack size=0x%08X, ", subl + 8*stackAdjust);
			}
			else {
				sprintf(str, "stack size=%d, ", stackSize*8);
			}
			if ( regCount == 0 ) {
				strcat(str, "no registers saved");
			}
			else {
				int permunreg[6];
				switch ( regCount ) {
					case 6:
						permunreg[0] = permutation/120;
						permutation -= (permunreg[0]*120);
						permunreg[1] = permutation/24;
						permutation -= (permunreg[1]*24);
						permunreg[2] = permutation/6;
						permutation -= (permunreg[2]*6);
						permunreg[3] = permutation/2;
						permutation -= (permunreg[3]*2);
						permunreg[4] = permutation;
						permunreg[5] = 0;
						break;
					case 5:
						permunreg[0] = permutation/120;
						permutation -= (permunreg[0]*120);
						permunreg[1] = permutation/24;
						permutation -= (permunreg[1]*24);
						permunreg[2] = permutation/6;
						permutation -= (permunreg[2]*6);
						permunreg[3] = permutation/2;
						permutation -= (permunreg[3]*2);
						permunreg[4] = permutation;
						break;
					case 4:
						permunreg[0] = permutation/60;
						permutation -= (permunreg[0]*60);
						permunreg[1] = permutation/12;
						permutation -= (permunreg[1]*12);
						permunreg[2] = permutation/3;
						permutation -= (permunreg[2]*3);
						permunreg[3] = permutation;
						break;
					case 3:
						permunreg[0] = permutation/20;
						permutation -= (permunreg[0]*20);
						permunreg[1] = permutation/4;
						permutation -= (permunreg[1]*4);
						permunreg[2] = permutation;
						break;
					case 2:
						permunreg[0] = permutation/5;
						permutation -= (permunreg[0]*5);
						permunreg[1] = permutation;
						break;
					case 1:
						permunreg[0] = permutation;
						break;
				}
				// renumber registers back to standard numbers
				int registers[6];
				bool used[7] = { false, false, false, false, false, false, false };
				for (int i=0; i < regCount; ++i) {
					int renum = 0; 
					for (int u=1; u < 7; ++u) {
						if ( !used[u] ) {
							if ( renum == permunreg[i] ) {
								registers[i] = u;
								used[u] = true;
								break;
							}
							++renum;
						}
					}
				}
				bool needComma = false;
				for (int i=0; i < regCount; ++i) {
					if ( needComma ) 
						strcat(str, ",");
					else
						needComma = true;
					switch ( registers[i] ) {
						case UNWIND_X86_64_REG_RBX:
							strcat(str, "rbx");
							break;
						case UNWIND_X86_64_REG_R12:
							strcat(str, "r12");
							break;
						case UNWIND_X86_64_REG_R13:
							strcat(str, "r13");
							break;
						case UNWIND_X86_64_REG_R14:
							strcat(str, "r14");
							break;
						case UNWIND_X86_64_REG_R15:
							strcat(str, "r15");
							break;
						case UNWIND_X86_64_REG_RBP:
							strcat(str, "rbp");
							break;
						default:
							strcat(str, "r??");
					}
				}
			}
		}
		break;
		case UNWIND_X86_64_MODE_DWARF:
			sprintf(str, "dwarf offset 0x%08X, ", encoding & UNWIND_X86_64_DWARF_SECTION_OFFSET);
			break;
		default:
			if ( encoding == 0 )
				strcat(str, "no unwind information");
			else
				strcat(str, "tbd ");
	}
	if ( encoding & UNWIND_HAS_LSDA ) {
		strcat(str, " LSDA");
	}

}

template <>
void UnwindPrinter<x86>::decode(uint32_t encoding, const uint8_t* funcStart, char* str)
{
	*str = '\0';
	switch ( encoding & UNWIND_X86_MODE_MASK ) {
		case UNWIND_X86_MODE_EBP_FRAME:
		{
			uint32_t savedRegistersOffset = EXTRACT_BITS(encoding, UNWIND_X86_EBP_FRAME_OFFSET);
			uint32_t savedRegistersLocations = EXTRACT_BITS(encoding, UNWIND_X86_EBP_FRAME_REGISTERS);
			if ( savedRegistersLocations == 0 ) {
				strcpy(str, "ebp frame, no saved registers");
			}
			else {
				sprintf(str, "ebp frame, at -%d:", savedRegistersOffset*4);
				bool needComma = false;
				for (int i=0; i < 5; ++i) {
					if ( needComma ) 
						strcat(str, ",");
					else
						needComma = true;
					switch (savedRegistersLocations & 0x7) {
						case UNWIND_X86_REG_NONE:
							strcat(str, "-");
							break;
						case UNWIND_X86_REG_EBX:
							strcat(str, "ebx");
							break;
						case UNWIND_X86_REG_ECX:
							strcat(str, "ecx");
							break;
						case UNWIND_X86_REG_EDX:
							strcat(str, "edx");
							break;
						case UNWIND_X86_REG_EDI:
							strcat(str, "edi");
							break;
						case UNWIND_X86_REG_ESI:
							strcat(str, "esi");
							break;
						default:
							strcat(str, "e??");
					}
					savedRegistersLocations = (savedRegistersLocations >> 3);
					if ( savedRegistersLocations == 0 )
						break;
				}
			}
		}
		break;
		case UNWIND_X86_MODE_STACK_IMMD:
		case UNWIND_X86_MODE_STACK_IND:
		{
			uint32_t stackSize = EXTRACT_BITS(encoding, UNWIND_X86_FRAMELESS_STACK_SIZE);
			uint32_t stackAdjust = EXTRACT_BITS(encoding, UNWIND_X86_FRAMELESS_STACK_ADJUST);
			uint32_t regCount = EXTRACT_BITS(encoding, UNWIND_X86_FRAMELESS_STACK_REG_COUNT);
			uint32_t permutation = EXTRACT_BITS(encoding, UNWIND_X86_FRAMELESS_STACK_REG_PERMUTATION);
			if ( (encoding & UNWIND_X86_MODE_MASK) == UNWIND_X86_MODE_STACK_IND ) {
				// stack size is encoded in subl $xxx,%esp instruction
				uint32_t subl = x86::P::E::get32(*((uint32_t*)(funcStart+stackSize)));
				sprintf(str, "stack size=0x%08X, ", subl+4*stackAdjust);
			}
			else {
				sprintf(str, "stack size=%d, ", stackSize*4);
			}
			if ( regCount == 0 ) {
				strcat(str, "no saved regs");
			}
			else {
				int permunreg[6];
				switch ( regCount ) {
					case 6:
						permunreg[0] = permutation/120;
						permutation -= (permunreg[0]*120);
						permunreg[1] = permutation/24;
						permutation -= (permunreg[1]*24);
						permunreg[2] = permutation/6;
						permutation -= (permunreg[2]*6);
						permunreg[3] = permutation/2;
						permutation -= (permunreg[3]*2);
						permunreg[4] = permutation;
						permunreg[5] = 0;
						break;
					case 5:
						permunreg[0] = permutation/120;
						permutation -= (permunreg[0]*120);
						permunreg[1] = permutation/24;
						permutation -= (permunreg[1]*24);
						permunreg[2] = permutation/6;
						permutation -= (permunreg[2]*6);
						permunreg[3] = permutation/2;
						permutation -= (permunreg[3]*2);
						permunreg[4] = permutation;
						break;
					case 4:
						permunreg[0] = permutation/60;
						permutation -= (permunreg[0]*60);
						permunreg[1] = permutation/12;
						permutation -= (permunreg[1]*12);
						permunreg[2] = permutation/3;
						permutation -= (permunreg[2]*3);
						permunreg[3] = permutation;
						break;
					case 3:
						permunreg[0] = permutation/20;
						permutation -= (permunreg[0]*20);
						permunreg[1] = permutation/4;
						permutation -= (permunreg[1]*4);
						permunreg[2] = permutation;
						break;
					case 2:
						permunreg[0] = permutation/5;
						permutation -= (permunreg[0]*5);
						permunreg[1] = permutation;
						break;
					case 1:
						permunreg[0] = permutation;
						break;
				}
				// renumber registers back to standard numbers
				int registers[6];
				bool used[7] = { false, false, false, false, false, false, false };
				for (int i=0; i < regCount; ++i) {
					int renum = 0; 
					for (int u=1; u < 7; ++u) {
						if ( !used[u] ) {
							if ( renum == permunreg[i] ) {
								registers[i] = u;
								used[u] = true;
								break;
							}
							++renum;
						}
					}
				}
				bool needComma = false;
				for (int i=0; i < regCount; ++i) {
					if ( needComma ) 
						strcat(str, ",");
					else
						needComma = true;
					switch ( registers[i] ) {
						case UNWIND_X86_REG_EBX:
							strcat(str, "ebx");
							break;
						case UNWIND_X86_REG_ECX:
							strcat(str, "ecx");
							break;
						case UNWIND_X86_REG_EDX:
							strcat(str, "edx");
							break;
						case UNWIND_X86_REG_EDI:
							strcat(str, "edi");
							break;
						case UNWIND_X86_REG_ESI:
							strcat(str, "esi");
							break;
						case UNWIND_X86_REG_EBP:
							strcat(str, "ebp");
							break;
						default:
							strcat(str, "e??");
					}
				}
			}
		}
		break;
		case UNWIND_X86_MODE_DWARF:
			sprintf(str, "dwarf offset 0x%08X, ", encoding & UNWIND_X86_DWARF_SECTION_OFFSET);
			break;
		default:
			if ( encoding == 0 )
				strcat(str, "no unwind information");
			else
				strcat(str, "tbd ");
	}
	if ( encoding & UNWIND_HAS_LSDA ) {
		strcat(str, " LSDA");
	}

}

#if SUPPORT_ARCH_arm64
template <>
void UnwindPrinter<arm64>::decode(uint32_t encoding, const uint8_t* funcStart, char* str)
{
	uint32_t stackSize;
	switch ( encoding & UNWIND_ARM64_MODE_MASK ) {
		case UNWIND_ARM64_MODE_FRAMELESS:
			stackSize = EXTRACT_BITS(encoding, UNWIND_ARM64_FRAMELESS_STACK_SIZE_MASK);
			if ( stackSize == 0 )
				strcpy(str, "no frame, no saved registers ");
			else
				sprintf(str, "stack size=%d: ", 16 * stackSize);
			if ( encoding & UNWIND_ARM64_FRAME_X19_X20_PAIR )
				strcat(str, "x19/20 ");
			if ( encoding & UNWIND_ARM64_FRAME_X21_X22_PAIR )
				strcat(str, "x21/22 ");
			if ( encoding & UNWIND_ARM64_FRAME_X23_X24_PAIR )
				strcat(str, "x23/24 ");
			if ( encoding & UNWIND_ARM64_FRAME_X25_X26_PAIR )
				strcat(str, "x25/26 ");
			if ( encoding & UNWIND_ARM64_FRAME_X27_X28_PAIR )
				strcat(str, "x27/28 ");
			if ( encoding & UNWIND_ARM64_FRAME_D8_D9_PAIR )
				strcat(str, "d8/9 ");
			if ( encoding & UNWIND_ARM64_FRAME_D10_D11_PAIR )
				strcat(str, "d10/11 ");
			if ( encoding & UNWIND_ARM64_FRAME_D12_D13_PAIR )
				strcat(str, "d12/13 ");
			if ( encoding & UNWIND_ARM64_FRAME_D14_D15_PAIR )
				strcat(str, "d14/15 ");
			break;
			break;
		case UNWIND_ARM64_MODE_DWARF:
			sprintf(str, "dwarf offset 0x%08X, ", encoding & UNWIND_X86_64_DWARF_SECTION_OFFSET);
			break;
		case UNWIND_ARM64_MODE_FRAME:
			strcpy(str, "std frame: ");
			if ( encoding & UNWIND_ARM64_FRAME_X19_X20_PAIR )
				strcat(str, "x19/20 ");
			if ( encoding & UNWIND_ARM64_FRAME_X21_X22_PAIR )
				strcat(str, "x21/22 ");
			if ( encoding & UNWIND_ARM64_FRAME_X23_X24_PAIR )
				strcat(str, "x23/24 ");
			if ( encoding & UNWIND_ARM64_FRAME_X25_X26_PAIR )
				strcat(str, "x25/26 ");
			if ( encoding & UNWIND_ARM64_FRAME_X27_X28_PAIR )
				strcat(str, "x27/28 ");
			if ( encoding & UNWIND_ARM64_FRAME_D8_D9_PAIR )
				strcat(str, "d8/9 ");
			if ( encoding & UNWIND_ARM64_FRAME_D10_D11_PAIR )
				strcat(str, "d10/11 ");
			if ( encoding & UNWIND_ARM64_FRAME_D12_D13_PAIR )
				strcat(str, "d12/13 ");
			if ( encoding & UNWIND_ARM64_FRAME_D14_D15_PAIR )
				strcat(str, "d14/15 ");
			break;
		case UNWIND_ARM64_MODE_FRAME_OLD:
			strcpy(str, "old frame: ");
			if ( encoding & UNWIND_ARM64_FRAME_X21_X22_PAIR_OLD )
				strcat(str, "x21/22 ");
			if ( encoding & UNWIND_ARM64_FRAME_X23_X24_PAIR_OLD )
				strcat(str, "x23/24 ");
			if ( encoding & UNWIND_ARM64_FRAME_X25_X26_PAIR_OLD )
				strcat(str, "x25/26 ");
			if ( encoding & UNWIND_ARM64_FRAME_X27_X28_PAIR_OLD )
				strcat(str, "x27/28 ");
			if ( encoding & UNWIND_ARM64_FRAME_D8_D9_PAIR_OLD )
				strcat(str, "d8/9 ");
			if ( encoding & UNWIND_ARM64_FRAME_D10_D11_PAIR_OLD )
				strcat(str, "d10/11 ");
			if ( encoding & UNWIND_ARM64_FRAME_D12_D13_PAIR_OLD )
				strcat(str, "d12/13 ");
			if ( encoding & UNWIND_ARM64_FRAME_D14_D15_PAIR_OLD )
				strcat(str, "d14/15 ");
			break;
	}
}
#endif

#if SUPPORT_ARCH_arm64_32
template <>
void UnwindPrinter<arm64_32>::decode(uint32_t encoding, const uint8_t* funcStart, char* str)
{
	uint32_t stackSize;
	switch ( encoding & UNWIND_ARM64_MODE_MASK ) {
		case UNWIND_ARM64_MODE_FRAMELESS:
			stackSize = EXTRACT_BITS(encoding, UNWIND_ARM64_FRAMELESS_STACK_SIZE_MASK);
			if ( stackSize == 0 )
				strcpy(str, "no frame, no saved registers ");
			else
				sprintf(str, "stack size=%d: ", 16 * stackSize);
			if ( encoding & UNWIND_ARM64_FRAME_X19_X20_PAIR )
				strcat(str, "x19/20 ");
			if ( encoding & UNWIND_ARM64_FRAME_X21_X22_PAIR )
				strcat(str, "x21/22 ");
			if ( encoding & UNWIND_ARM64_FRAME_X23_X24_PAIR )
				strcat(str, "x23/24 ");
			if ( encoding & UNWIND_ARM64_FRAME_X25_X26_PAIR )
				strcat(str, "x25/26 ");
			if ( encoding & UNWIND_ARM64_FRAME_X27_X28_PAIR )
				strcat(str, "x27/28 ");
			if ( encoding & UNWIND_ARM64_FRAME_D8_D9_PAIR )
				strcat(str, "d8/9 ");
			if ( encoding & UNWIND_ARM64_FRAME_D10_D11_PAIR )
				strcat(str, "d10/11 ");
			if ( encoding & UNWIND_ARM64_FRAME_D12_D13_PAIR )
				strcat(str, "d12/13 ");
			if ( encoding & UNWIND_ARM64_FRAME_D14_D15_PAIR )
				strcat(str, "d14/15 ");
			break;
			break;
		case UNWIND_ARM64_MODE_DWARF:
			sprintf(str, "dwarf offset 0x%08X, ", encoding & UNWIND_X86_64_DWARF_SECTION_OFFSET);
			break;
		case UNWIND_ARM64_MODE_FRAME:
			strcpy(str, "std frame: ");
			if ( encoding & UNWIND_ARM64_FRAME_X19_X20_PAIR )
				strcat(str, "x19/20 ");
			if ( encoding & UNWIND_ARM64_FRAME_X21_X22_PAIR )
				strcat(str, "x21/22 ");
			if ( encoding & UNWIND_ARM64_FRAME_X23_X24_PAIR )
				strcat(str, "x23/24 ");
			if ( encoding & UNWIND_ARM64_FRAME_X25_X26_PAIR )
				strcat(str, "x25/26 ");
			if ( encoding & UNWIND_ARM64_FRAME_X27_X28_PAIR )
				strcat(str, "x27/28 ");
			if ( encoding & UNWIND_ARM64_FRAME_D8_D9_PAIR )
				strcat(str, "d8/9 ");
			if ( encoding & UNWIND_ARM64_FRAME_D10_D11_PAIR )
				strcat(str, "d10/11 ");
			if ( encoding & UNWIND_ARM64_FRAME_D12_D13_PAIR )
				strcat(str, "d12/13 ");
			if ( encoding & UNWIND_ARM64_FRAME_D14_D15_PAIR )
				strcat(str, "d14/15 ");
			break;
		case UNWIND_ARM64_MODE_FRAME_OLD:
			strcpy(str, "old frame: ");
			if ( encoding & UNWIND_ARM64_FRAME_X21_X22_PAIR_OLD )
				strcat(str, "x21/22 ");
			if ( encoding & UNWIND_ARM64_FRAME_X23_X24_PAIR_OLD )
				strcat(str, "x23/24 ");
			if ( encoding & UNWIND_ARM64_FRAME_X25_X26_PAIR_OLD )
				strcat(str, "x25/26 ");
			if ( encoding & UNWIND_ARM64_FRAME_X27_X28_PAIR_OLD )
				strcat(str, "x27/28 ");
			if ( encoding & UNWIND_ARM64_FRAME_D8_D9_PAIR_OLD )
				strcat(str, "d8/9 ");
			if ( encoding & UNWIND_ARM64_FRAME_D10_D11_PAIR_OLD )
				strcat(str, "d10/11 ");
			if ( encoding & UNWIND_ARM64_FRAME_D12_D13_PAIR_OLD )
				strcat(str, "d12/13 ");
			if ( encoding & UNWIND_ARM64_FRAME_D14_D15_PAIR_OLD )
				strcat(str, "d14/15 ");
			break;
	}
}
#endif

template <>
void UnwindPrinter<arm>::decode(uint32_t encoding, const uint8_t* funcStart, char* str)
{
	*str = '\0';
	switch ( encoding & UNWIND_ARM_MODE_MASK ) {
		case UNWIND_ARM_MODE_DWARF:
			sprintf(str, "dwarf offset 0x%08X, ", encoding & UNWIND_ARM_DWARF_SECTION_OFFSET);
			break;
		case UNWIND_ARM_MODE_FRAME:
		case UNWIND_ARM_MODE_FRAME_D:
			switch ( encoding & UNWIND_ARM_FRAME_STACK_ADJUST_MASK ) {
				case 0x00000000:
					strcpy(str, "std frame: ");
					break;
				case 0x00400000:
					strcat(str, "std frame(sp adj 4): ");
					break;
				case 0x00800000:
					strcat(str, "std frame(sp adj 8): ");
					break;
				case 0x00C00000:
					strcat(str, "std frame(sp adj 12): ");
					break;
			}
			if ( encoding & UNWIND_ARM_FRAME_FIRST_PUSH_R4 )
				strcat(str, "r4 ");
			if ( encoding & UNWIND_ARM_FRAME_FIRST_PUSH_R5 )
				strcat(str, "r5 ");
			if ( encoding & UNWIND_ARM_FRAME_FIRST_PUSH_R6 )
				strcat(str, "r6 ");
				
			if ( encoding & 0x000000F8) 
				strcat(str, " / ");
			if ( encoding & UNWIND_ARM_FRAME_SECOND_PUSH_R8 )
				strcat(str, "r8 ");
			if ( encoding & UNWIND_ARM_FRAME_SECOND_PUSH_R9 )
				strcat(str, "r9 ");
			if ( encoding & UNWIND_ARM_FRAME_SECOND_PUSH_R10 )
				strcat(str, "r10 ");
			if ( encoding & UNWIND_ARM_FRAME_SECOND_PUSH_R11 )
				strcat(str, "r11 ");
			if ( encoding & UNWIND_ARM_FRAME_SECOND_PUSH_R12 )
				strcat(str, "r12 ");
			
			if ( (encoding & UNWIND_ARM_MODE_MASK) == UNWIND_ARM_MODE_FRAME_D ) {
				switch ( encoding & UNWIND_ARM_FRAME_D_REG_COUNT_MASK ) {
					case 0x00000000:
						strcat(str, " / d8 ");
						break;
					case 0x00000100:
						strcat(str, " / d8,d10 ");
						break;
					case 0x00000200:
						strcat(str, " / d8,d10,d12 ");
						break;
					case 0x00000300:
						strcat(str, " / d8,d10,d12,d14 ");
						break;
					case 0x00000400:
						strcat(str, " / d12,d14 / d8,d9,d10 ");
						break;
					case 0x00000500:
						strcat(str, " / d14 / d8,d9,d10,d11,d12");
						break;
					case 0x00000600:
						strcat(str, " / d8,d9,d10,d11,d12,d13,d14 ");
						break;
					case 0x00000700:
						strcat(str, " / d8,d9,d10,d11,d12,d13,d14 ");
						break;
					default:
						strcat(str, " / unknown D register usage ");
						break;
				}
			}
			
			break;
		default:
			if ( encoding == 0 )
				strcpy(str, "no unwind information");
			else
				strcpy(str, "unsupported compact unwind");
			break;
	}
}


template <>
const char* UnwindPrinter<x86_64>::personalityName(const macho_relocation_info<x86_64::P>* reloc)
{
	//assert(reloc->r_extern() && "reloc not extern on personality column in __compact_unwind section");
	//assert((reloc->r_type() == X86_64_RELOC_UNSIGNED) && "wrong reloc type on personality column in __compact_unwind section");
	const macho_nlist<P>& sym = fSymbols[reloc->r_symbolnum()];
	return &fStrings[sym.n_strx()];
}

template <>
const char* UnwindPrinter<x86>::personalityName(const macho_relocation_info<x86::P>* reloc)
{
	//assert(reloc->r_extern() && "reloc not extern on personality column in __compact_unwind section");
	//assert((reloc->r_type() == GENERIC_RELOC_VANILLA) && "wrong reloc type on personality column in __compact_unwind section");
	const macho_nlist<P>& sym = fSymbols[reloc->r_symbolnum()];
	return &fStrings[sym.n_strx()];
}

#if SUPPORT_ARCH_arm64
template <>
const char* UnwindPrinter<arm64>::personalityName(const macho_relocation_info<arm64::P>* reloc)
{
	//assert(reloc->r_extern() && "reloc not extern on personality column in __compact_unwind section");
	//assert((reloc->r_type() == ARM64_RELOC_UNSIGNED) && "wrong reloc type on personality column in __compact_unwind section");
	const macho_nlist<P>& sym = fSymbols[reloc->r_symbolnum()];
	return &fStrings[sym.n_strx()];
}
#endif

#if SUPPORT_ARCH_arm64_32
template <>
const char* UnwindPrinter<arm64_32>::personalityName(const macho_relocation_info<arm64_32::P>* reloc)
{
	//assert(reloc->r_extern() && "reloc not extern on personality column in __compact_unwind section");
	//assert((reloc->r_type() == ARM64_RELOC_UNSIGNED) && "wrong reloc type on personality column in __compact_unwind section");
	const macho_nlist<P>& sym = fSymbols[reloc->r_symbolnum()];
	return &fStrings[sym.n_strx()];
}
#endif

template <>
const char* UnwindPrinter<arm>::personalityName(const macho_relocation_info<arm::P>* reloc)
{
	//assert(reloc->r_extern() && "reloc not extern on personality column in __compact_unwind section");
	//assert((reloc->r_type() == GENERIC_RELOC_VANILLA) && "wrong reloc type on personality column in __compact_unwind section");
	const macho_nlist<P>& sym = fSymbols[reloc->r_symbolnum()];
	return &fStrings[sym.n_strx()];
}

template <typename A>
bool UnwindPrinter<A>::hasExernReloc(uint64_t sectionOffset, const char** personalityStr, pint_t* addr)
{
	const macho_relocation_info<P>* relocs = (macho_relocation_info<P>*)((uint8_t*)fHeader + fUnwindSection->reloff());
	const macho_relocation_info<P>* relocsEnd = &relocs[fUnwindSection->nreloc()];
	for (const macho_relocation_info<P>* reloc = relocs; reloc < relocsEnd; ++reloc) {
		if ( reloc->r_extern() && (reloc->r_address() == sectionOffset) ) {
			*personalityStr = this->personalityName(reloc);
			if ( addr != NULL )
				*addr = fSymbols[reloc->r_symbolnum()].n_value();
			return true;
		}
	}
	return false;
}


template <typename A>
void UnwindPrinter<A>::printObjectUnwindSection(bool showFunctionNames)
{
	printf("Arch: %s, Section: __LD,__compact_unwind (size=0x%08llX, => %lld entries)\n", 
				archName(), fUnwindSection->size(), fUnwindSection->size() / sizeof(macho_compact_unwind_entry<P>));
	
	const macho_compact_unwind_entry<P>* const entriesStart = (macho_compact_unwind_entry<P>*)((uint8_t*)fHeader + fUnwindSection->offset());
	const macho_compact_unwind_entry<P>* const entriesEnd = (macho_compact_unwind_entry<P>*)((uint8_t*)fHeader + fUnwindSection->offset() + fUnwindSection->size());
	for (const macho_compact_unwind_entry<P>* entry=entriesStart; entry < entriesEnd; ++entry) {
		uint64_t entryAddress = ((char*)entry - (char*)entriesStart) + fUnwindSection->addr();
		printf("0x%08llX:\n", entryAddress);
		const char* functionNameStr;
		pint_t funcAddress;
		uint32_t offsetInFunction;
		if ( hasExernReloc(((char*)entry-(char*)entriesStart)+macho_compact_unwind_entry<P>::codeStartFieldOffset(), &functionNameStr, &funcAddress) ) {
			offsetInFunction = entry->codeStart();
		}
		else {
			functionNameStr = this->functionName(entry->codeStart(), &offsetInFunction);
			funcAddress = entry->codeStart();
		}
		if ( offsetInFunction == 0 )
			printf("  start:        0x%08llX   %s\n", (uint64_t)funcAddress, functionNameStr);
		else
			printf("  start:        0x%08llX   %s+0x%X\n", (uint64_t)funcAddress+offsetInFunction, functionNameStr, offsetInFunction);
		
		printf("  end:          0x%08llX   (len=0x%08X)\n", (uint64_t)(funcAddress+offsetInFunction+entry->codeLen()), entry->codeLen());
		
		char encodingString[200];
		this->decode(entry->compactUnwindInfo(), ((const uint8_t*)fHeader), encodingString);
		printf("  unwind info:  0x%08X   %s\n", entry->compactUnwindInfo(), encodingString);
		
		const char* personalityNameStr;
		if ( hasExernReloc(((char*)entry-(char*)entriesStart)+macho_compact_unwind_entry<P>::personalityFieldOffset(), &personalityNameStr) ) {
			printf("  personality:              %s\n", personalityNameStr);
		}
		else {
			printf("  personality:\n");
		}
		if ( entry->lsda() == 0 ) {
			printf("  lsda:\n");
		}
		else {
			uint32_t lsdaOffset;
			const char* lsdaName = this->functionName(entry->lsda(), &lsdaOffset);
			if ( lsdaOffset == 0 )
				printf("  lsda:         0x%08llX  %s\n", (uint64_t)entry->lsda(), lsdaName);
			else
				printf("  lsda:         0x%08llX  %s+0x%X\n", (uint64_t)entry->lsda(), lsdaName, lsdaOffset);
		}
	}
}



template <typename A>
void UnwindPrinter<A>::printUnwindSection(bool showFunctionNames)
{
	const uint8_t* sectionContent = (uint8_t*)fHeader + fUnwindSection->offset();
	macho_unwind_info_section_header<P>* sectionHeader = (macho_unwind_info_section_header<P>*)(sectionContent);
	
	printf("Arch: %s, Section: __TEXT,__unwind_info (addr=0x%08llX, size=0x%08llX, fileOffset=0x%08X)\n", 
				archName(), fUnwindSection->addr(), fUnwindSection->size(), fUnwindSection->offset());
	printf("\tversion=0x%08X\n", sectionHeader->version());
	printf("\tcommonEncodingsArraySectionOffset=0x%08X\n", sectionHeader->commonEncodingsArraySectionOffset());
	printf("\tcommonEncodingsArrayCount=0x%08X\n", sectionHeader->commonEncodingsArrayCount());
	printf("\tpersonalityArraySectionOffset=0x%08X\n", sectionHeader->personalityArraySectionOffset());
	printf("\tpersonalityArrayCount=0x%08X\n", sectionHeader->personalityArrayCount());
	printf("\tindexSectionOffset=0x%08X\n", sectionHeader->indexSectionOffset());
	printf("\tindexCount=0x%08X\n", sectionHeader->indexCount());
	printf("\tcommon encodings: (count=%u)\n", sectionHeader->commonEncodingsArrayCount());
	const uint32_t* commonEncodings = (uint32_t*)&sectionContent[sectionHeader->commonEncodingsArraySectionOffset()];
	for (uint32_t i=0; i < sectionHeader->commonEncodingsArrayCount(); ++i) {
		printf("\t\tencoding[%3u]=0x%08X\n", i, A::P::E::get32(commonEncodings[i]));
	}
	printf("\tpersonalities: (count=%u)\n", sectionHeader->personalityArrayCount());
	const uint32_t* personalityArray = (uint32_t*)&sectionContent[sectionHeader->personalityArraySectionOffset()];
	for (uint32_t i=0; i < sectionHeader->personalityArrayCount(); ++i) {
		printf("\t\t[%2u]=0x%08X\n", i+1, A::P::E::get32(personalityArray[i]));
	}
	printf("\tfirst level index: (count=%u)\n", sectionHeader->indexCount());
	macho_unwind_info_section_header_index_entry<P>* indexes = (macho_unwind_info_section_header_index_entry<P>*)&sectionContent[sectionHeader->indexSectionOffset()];
	for (uint32_t i=0; i < sectionHeader->indexCount(); ++i) {
		printf("\t\t[%2u] funcOffset=0x%08X, pageOffset=0x%08X, lsdaOffset=0x%08X\n", 
					i, indexes[i].functionOffset(), indexes[i].secondLevelPagesSectionOffset(), indexes[i].lsdaIndexArraySectionOffset());
	}
	uint32_t lsdaIndexArraySectionOffset = indexes[0].lsdaIndexArraySectionOffset();
	uint32_t lsdaIndexArrayEndSectionOffset = indexes[sectionHeader->indexCount()-1].lsdaIndexArraySectionOffset();
	uint32_t lsdaIndexArrayCount = (lsdaIndexArrayEndSectionOffset-lsdaIndexArraySectionOffset)/sizeof(macho_unwind_info_section_header_lsda_index_entry<P>);
	printf("\tLSDA table: (section offset 0x%08X, count=%u)\n", lsdaIndexArraySectionOffset, lsdaIndexArrayCount);
	macho_unwind_info_section_header_lsda_index_entry<P>* lindex = (macho_unwind_info_section_header_lsda_index_entry<P>*)&sectionContent[lsdaIndexArraySectionOffset];
	for (uint32_t i=0; i < lsdaIndexArrayCount; ++i) {
		const char* name = showFunctionNames ? functionName(lindex[i].functionOffset()+fMachHeaderAddress) : "";
		printf("\t\t[%3u] funcOffset=0x%08X, lsdaOffset=0x%08X,  %s\n", i, lindex[i].functionOffset(), lindex[i].lsdaOffset(), name);
		if ( *(((uint8_t*)fHeader) + lindex[i].lsdaOffset()) != 0xFF )
			fprintf(stderr, "BAD LSDA entry (does not start with 0xFF) for %s\n", functionName(lindex[i].functionOffset()+fMachHeaderAddress));
	}
	for (uint32_t i=0; i < sectionHeader->indexCount()-1; ++i) {
		printf("\tsecond level index[%u] sectionOffset=0x%08X, count=%u, fileOffset=0x%08X\n", i, indexes[i].secondLevelPagesSectionOffset(), 
				sectionHeader->indexCount(), fUnwindSection->offset()+indexes[i].secondLevelPagesSectionOffset());
		macho_unwind_info_regular_second_level_page_header<P>* page = (macho_unwind_info_regular_second_level_page_header<P>*)&sectionContent[indexes[i].secondLevelPagesSectionOffset()];
		if ( page->kind() == UNWIND_SECOND_LEVEL_REGULAR ) {
			printf("\t\tkind=UNWIND_SECOND_LEVEL_REGULAR\n");
			printf("\t\tentryPageOffset=0x%08X\n", page->entryPageOffset());
			printf("\t\tentryCount=0x%08X\n", page->entryCount());
			const macho_unwind_info_regular_second_level_entry<P>* entry = (macho_unwind_info_regular_second_level_entry<P>*)((char*)page+page->entryPageOffset());
			for (uint32_t j=0; j < page->entryCount(); ++j) {
				uint32_t funcOffset = entry[j].functionOffset();
				if ( entry[j].encoding() & UNWIND_HAS_LSDA ) {
					// verify there is a corresponding entry in lsda table
					bool found = false;
					for (uint32_t k=0; k < lsdaIndexArrayCount; ++k) {
						if ( lindex[k].functionOffset() == funcOffset ) {
							found = true;
							break;
						}
					}
					if ( !found ) {
						fprintf(stderr, "MISSING LSDA entry for %s\n", functionName(funcOffset+fMachHeaderAddress));
					}
				} 
				char encodingString[100];
				decode(entry[j].encoding(), ((const uint8_t*)fHeader)+funcOffset, encodingString);
				const char* name = showFunctionNames ? functionName(funcOffset+fMachHeaderAddress) : "";
				printf("\t\t\t[%3u] funcOffset=0x%08X, encoding=0x%08X (%-56s) %s\n", 
					j, funcOffset, entry[j].encoding(), encodingString, name);
			}
		}
		else if ( page->kind() == UNWIND_SECOND_LEVEL_COMPRESSED ) {
			macho_unwind_info_compressed_second_level_page_header<P>* cp = (macho_unwind_info_compressed_second_level_page_header<P>*)page;
			printf("\t\tkind=UNWIND_SECOND_LEVEL_COMPRESSED\n");
			printf("\t\tentryPageOffset=0x%08X\n", cp->entryPageOffset());
			printf("\t\tentryCount=0x%08X\n", cp->entryCount());
			printf("\t\tencodingsPageOffset=0x%08X\n", cp->encodingsPageOffset());
			printf("\t\tencodingsCount=0x%08X\n", cp->encodingsCount());
			const uint32_t* entries = (uint32_t*)(((uint8_t*)page)+cp->entryPageOffset());
			const uint32_t* encodings = (uint32_t*)(((uint8_t*)page)+cp->encodingsPageOffset());
			const uint32_t baseFunctionOffset = indexes[i].functionOffset();
			for (uint32_t j=0; j < cp->entryCount(); ++j) {
				uint8_t encodingIndex = UNWIND_INFO_COMPRESSED_ENTRY_ENCODING_INDEX(entries[j]);
				uint32_t encoding;
				if ( encodingIndex < sectionHeader->commonEncodingsArrayCount() )
					encoding =  A::P::E::get32(commonEncodings[encodingIndex]);
				else
					encoding =  A::P::E::get32(encodings[encodingIndex-sectionHeader->commonEncodingsArrayCount()]);
				char encodingString[100];
				uint32_t funcOff = UNWIND_INFO_COMPRESSED_ENTRY_FUNC_OFFSET(entries[j])+baseFunctionOffset;
				decode(encoding, ((const uint8_t*)fHeader)+funcOff, encodingString);
				const char* name = showFunctionNames ? functionName(funcOff+fMachHeaderAddress) : "";
				if ( encoding & UNWIND_HAS_LSDA ) {
					// verify there is a corresponding entry in lsda table
					bool found = false;
					for (uint32_t k=0; k < lsdaIndexArrayCount; ++k) {
						if ( lindex[k].functionOffset() == funcOff ) {
							found = true;
							break;
						}
					}
					if ( !found ) {
						fprintf(stderr, "MISSING LSDA entry for %s\n", name);
					}
				} 
				printf("\t\t\t[%3u] funcOffset=0x%08X, encoding[%3u]=0x%08X (%-56s) %s\n", 
					j, funcOff, encodingIndex, encoding, encodingString, name);
			}					
		}
		else {
			fprintf(stderr, "\t\tbad page header\n");
		}
	}

}

static void dump(const char* path, const std::set<cpu_type_t>& onlyArchs, bool showFunctionNames)
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
				unsigned int cputype = OSSwapBigToHostInt32(archs[i].cputype);
				if ( onlyArchs.count(cputype) ) {
					switch(cputype) {
					case CPU_TYPE_I386:
						if ( UnwindPrinter<x86>::validFile(p + offset) )
							UnwindPrinter<x86>::make(p + offset, size, path, showFunctionNames);
						else
							throw "in universal file, i386 slice does not contain i386 mach-o";
						break;
					case CPU_TYPE_X86_64:
						if ( UnwindPrinter<x86_64>::validFile(p + offset) )
							UnwindPrinter<x86_64>::make(p + offset, size, path, showFunctionNames);
						else
							throw "in universal file, x86_64 slice does not contain x86_64 mach-o";
						break;
#if SUPPORT_ARCH_arm64
					case CPU_TYPE_ARM64:
						if ( UnwindPrinter<arm64>::validFile(p + offset) )
							UnwindPrinter<arm64>::make(p + offset, size, path, showFunctionNames);
						else
							throw "in universal file, arm64 slice does not contain arm64 mach-o";
						break;
#endif
#if SUPPORT_ARCH_arm64_32
					case CPU_TYPE_ARM64_32:
						if ( UnwindPrinter<arm64_32>::validFile(p + offset) )
							UnwindPrinter<arm64_32>::make(p + offset, size, path, showFunctionNames);
						else
							throw "in universal file, arm64_32 slice does not contain arm64_32 mach-o";
						break;
#endif
					case CPU_TYPE_ARM:
						if ( UnwindPrinter<arm>::validFile(p + offset) )
							UnwindPrinter<arm>::make(p + offset, size, path, showFunctionNames);
						else
							throw "in universal file, arm slice does not contain arm mach-o";
						break;
					default:
							throwf("in universal file, unknown architecture slice 0x%x\n", cputype);
					}
				}
			}
		}
		else if ( UnwindPrinter<x86>::validFile(p) && onlyArchs.count(CPU_TYPE_I386) ) {
			UnwindPrinter<x86>::make(p, length, path, showFunctionNames);
		}
		else if ( UnwindPrinter<x86_64>::validFile(p) && onlyArchs.count(CPU_TYPE_X86_64) ) {
			UnwindPrinter<x86_64>::make(p, length, path, showFunctionNames);
		}
#if SUPPORT_ARCH_arm64
		else if ( UnwindPrinter<arm64>::validFile(p) && onlyArchs.count(CPU_TYPE_ARM64) ) {
			UnwindPrinter<arm64>::make(p, length, path, showFunctionNames);
		}
#endif
#if SUPPORT_ARCH_arm64_32
		else if ( UnwindPrinter<arm64_32>::validFile(p) && onlyArchs.count(CPU_TYPE_ARM64_32) ) {
			UnwindPrinter<arm64_32>::make(p, length, path, showFunctionNames);
		}
#endif
		else if ( UnwindPrinter<arm>::validFile(p) && onlyArchs.count(CPU_TYPE_ARM) ) {
			UnwindPrinter<arm>::make(p, length, path, showFunctionNames);
		}
		else {
			throw "not a known file type";
		}
	}
	catch (const char* msg) {
		throwf("%s in %s", msg, path);
	}
}


int main(int argc, const char* argv[])
{
	std::set<cpu_type_t> onlyArchs;
	std::vector<const char*> files;
	bool showFunctionNames = true;
	
	try {
		for(int i=1; i < argc; ++i) {
			const char* arg = argv[i];
			if ( arg[0] == '-' ) {
				if ( strcmp(arg, "-arch") == 0 ) {
					const char* arch = argv[++i];
					if ( strcmp(arch, "i386") == 0 )
						onlyArchs.insert(CPU_TYPE_I386);
					else if ( strcmp(arch, "x86_64") == 0 )
						onlyArchs.insert(CPU_TYPE_X86_64);
#if SUPPORT_ARCH_arm64
					else if ( strcmp(arch, "arm64") == 0 )
						onlyArchs.insert(CPU_TYPE_ARM64);
#endif
#if SUPPORT_ARCH_arm64_32
					else if ( strcmp(arch, "arm64_32") == 0 )
						onlyArchs.insert(CPU_TYPE_ARM64_32);
#endif
					else if ( strcmp(arch, "armv7k") == 0 )
						onlyArchs.insert(CPU_TYPE_ARM);
					else 
						throwf("unknown architecture %s", arch);
				}
				else if ( strcmp(arg, "-no_symbols") == 0 ) {
					showFunctionNames = false;
				}
				else {
					throwf("unknown option: %s\n", arg);
				}
			}
			else {
				files.push_back(arg);
			}
		}
		
		// use all architectures if no restrictions specified
		if ( onlyArchs.size() == 0 ) {
			onlyArchs.insert(CPU_TYPE_I386);
			onlyArchs.insert(CPU_TYPE_X86_64);
#if SUPPORT_ARCH_arm64
			onlyArchs.insert(CPU_TYPE_ARM64);
#endif
#if SUPPORT_ARCH_arm64_32
			onlyArchs.insert(CPU_TYPE_ARM64_32);
#endif
			onlyArchs.insert(CPU_TYPE_ARM);
		}
		
		// process each file
		for(std::vector<const char*>::iterator it=files.begin(); it != files.end(); ++it) {
			dump(*it, onlyArchs, showFunctionNames);
		}
		
	}
	catch (const char* msg) {
		fprintf(stderr, "UnwindDump failed: %s\n", msg);
		return 1;
	}
	
	return 0;
}



