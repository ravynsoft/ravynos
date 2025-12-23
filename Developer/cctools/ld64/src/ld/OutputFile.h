/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-*
 *
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#ifndef __OUTPUT_FILE_H__
#define __OUTPUT_FILE_H__

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#ifndef __linux__
#include <sys/sysctl.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <mach/mach_time.h>
#include <mach/vm_statistics.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>

#include <vector>

#include "Options.h"
#include "ld.hpp"

namespace ld {
namespace tool {

class OutputFile
{
public:
								OutputFile(const Options& opts, ld::Internal& state);

	
	// iterates all atoms in initial files
	void						write(ld::Internal&);
	bool						findSegment(ld::Internal& state, uint64_t addr, uint64_t* start, uint64_t* end, uint32_t* index);
	void						setLazyBindingInfoOffset(uint64_t lpAddress, uint32_t lpInfoOffset);
	uint32_t					dylibCount();
	const ld::dylib::File*		dylibByOrdinal(unsigned int ordinal);
	uint32_t					dylibToOrdinal(const ld::dylib::File*) const;
	uint32_t					encryptedTextStartOffset()	{ return _encryptedTEXTstartOffset; }
	uint32_t					encryptedTextEndOffset()	{ return _encryptedTEXTendOffset; }
	int							compressedOrdinalForAtom(const ld::Atom* target) const;
	uint64_t					fileSize() const { return _fileSize; }

	bool						needsBind(const ld::Atom* toTarget, bool authPtr, uint64_t* accumulator = nullptr,
										  uint64_t* inlineAddend = nullptr, uint32_t* bindOrdinal = nullptr,
										  uint32_t* libOrdinal = nullptr) const;
	
	
	bool						usesWeakExternalSymbols;
	bool						overridesWeakExternalSymbols;
	bool						_noReExportedDylibs;
	bool						pieDisabled;
	bool						hasDataInCode;
	ld::Internal::FinalSection*	headerAndLoadCommandsSection;
	ld::Internal::FinalSection*	rebaseSection;
	ld::Internal::FinalSection*	bindingSection;
	ld::Internal::FinalSection*	weakBindingSection;
	ld::Internal::FinalSection*	lazyBindingSection;
	ld::Internal::FinalSection*	exportSection;
	ld::Internal::FinalSection*	splitSegInfoSection;
	ld::Internal::FinalSection*	functionStartsSection;
	ld::Internal::FinalSection*	dataInCodeSection;
	ld::Internal::FinalSection*	optimizationHintsSection;
	ld::Internal::FinalSection*	symbolTableSection;
	ld::Internal::FinalSection*	stringPoolSection;
	ld::Internal::FinalSection*	localRelocationsSection;
	ld::Internal::FinalSection*	externalRelocationsSection;
	ld::Internal::FinalSection*	sectionRelocationsSection;
	ld::Internal::FinalSection*	indirectSymbolTableSection;
	ld::Internal::FinalSection*	threadedPageStartsSection;
	ld::Internal::FinalSection*	chainInfoSection;
	ld::Internal::FinalSection*	codeSignatureSection;

	struct RebaseInfo {
						RebaseInfo(uint8_t t, uint64_t addr) : _type(t), _address(addr) {}
		uint8_t			_type;
		uint64_t		_address;
		// for sorting
		int operator<(const RebaseInfo& rhs) const {
			// sort by type, then address
			if ( this->_type != rhs._type )
				return  (this->_type < rhs._type );
			return  (this->_address < rhs._address );
		}
	};

	struct BindingInfo {
						BindingInfo(uint8_t t, int ord, const char* sym, bool weak_import, uint64_t addr, int64_t add) 
							: _type(t), _flags(weak_import ? BIND_SYMBOL_FLAGS_WEAK_IMPORT : 0 ),
								_threadedBindOrdinal(0), _libraryOrdinal(ord),
								_symbolName(sym), _address(addr), _addend(add) {}
						BindingInfo(uint8_t t, const char* sym, bool non_weak_definition, uint64_t addr, int64_t add) 
							: _type(t), _flags(non_weak_definition ? BIND_SYMBOL_FLAGS_NON_WEAK_DEFINITION : 0 ), 
							 _threadedBindOrdinal(0), _libraryOrdinal(0), _symbolName(sym), _address(addr), _addend(add) {}
		uint8_t			_type;
		uint8_t			_flags;
		uint16_t		_threadedBindOrdinal;
		int				_libraryOrdinal;
		const char*		_symbolName;
		uint64_t		_address;
		int64_t			_addend;
		
		// for sorting
		int operator<(const BindingInfo& rhs) const {
			// sort by library, symbol, type, flags, then address
			if ( this->_libraryOrdinal != rhs._libraryOrdinal )
				return  (this->_libraryOrdinal < rhs._libraryOrdinal );
			if ( this->_symbolName != rhs._symbolName )
				return ( strcmp(this->_symbolName, rhs._symbolName) < 0 );
			if ( this->_type != rhs._type )
				return  (this->_type < rhs._type );
			if ( this->_flags != rhs._flags )
				return  (this->_flags >= rhs._flags );
			return  (this->_address < rhs._address );
		}
	};
	
	struct SplitSegInfoEntry {
						SplitSegInfoEntry(uint64_t a, ld::Fixup::Kind k, uint32_t e=0)
							: fixupAddress(a), kind(k), extra(e) {}
		uint64_t		fixupAddress;
		ld::Fixup::Kind	kind;
        uint32_t        extra;
	};
	
	struct SplitSegInfoV2Entry {
						SplitSegInfoV2Entry(uint8_t fi, uint64_t fo, uint8_t ti, uint64_t to, uint8_t k)
							: fixupSectionOffset(fo), targetSectionOffset(to), fixupSectionIndex(fi), targetSectionIndex(ti), referenceKind(k) {}
		uint64_t		fixupSectionOffset;
		uint64_t		targetSectionOffset;
		uint8_t			fixupSectionIndex;
		uint8_t			targetSectionIndex;
		uint8_t			referenceKind;
	};
	static void					dumpAtomsBySection(ld::Internal& state, bool);

	struct ChainedFixupPageInfo
	{
		std::vector<uint16_t> 	fixupOffsets;
		std::vector<uint16_t> 	chainOverflows;
	};

	struct ChainedFixupSegInfo
	{
		const char*  name;
		uint64_t     startAddr;
		uint64_t     endAddr;
		uint32_t	 fileOffset;
		uint32_t	 pageSize;
		uint32_t	 pointerFormat;
		std::vector<ChainedFixupPageInfo> pages;
	};

private:
	void						writeAtoms(ld::Internal& state, uint8_t* wholeBuffer);
	void						computeContentUUID(ld::Internal& state, uint8_t* wholeBuffer);
	void						buildDylibOrdinalMapping(ld::Internal&);
	bool						hasOrdinalForInstallPath(const char* path, int* ordinal);
	void						addLoadCommands(ld::Internal& state);
	void						addLinkEdit(ld::Internal& state);
	void						addPreloadLinkEdit(ld::Internal& state);
	void						generateLinkEditInfo(ld::Internal& state);
	void						buildSymbolTable(ld::Internal& state);
	void						writeOutputFile(ld::Internal& state);
	void						addSectionRelocs(ld::Internal& state, ld::Internal::FinalSection* sect,  
												const ld::Atom* atom, ld::Fixup* fixupWithTarget, 
												ld::Fixup* fixupWithMinusTarget, ld::Fixup* fixupWithAddend,
												 ld::Fixup* fixupWithStore,
#if SUPPORT_ARCH_arm64e
												 ld::Fixup* fixupWithAuthData,
#endif
												const ld::Atom* target, const ld::Atom* minusTarget, 
												uint64_t targetAddend, uint64_t minusTargetAddend);
	void						addDyldInfo(ld::Internal& state, ld::Internal::FinalSection* sect,  
												const ld::Atom* atom, ld::Fixup* fixupWithTarget,
												ld::Fixup* fixupWithMinusTarget, ld::Fixup* fixupWithStore,
												const ld::Atom* target, const ld::Atom* minusTarget, 
												uint64_t targetAddend, uint64_t minusTargetAddend);
	void						addThreadedRebaseInfo(ld::Internal& state, ld::Internal::FinalSection* sect,
													  const ld::Atom* atom, ld::Fixup* fixupWithTarget,
													  ld::Fixup* fixupWithMinusTarget, ld::Fixup* fixupWithStore,
													  const ld::Atom* target, const ld::Atom* minusTarget,
													  uint64_t targetAddend, uint64_t minusTargetAddend);
	void						addChainedFixupLocation(ld::Internal& state, ld::Internal::FinalSection* sect,
													  const ld::Atom* atom, ld::Fixup* fixupWithTarget,
													  ld::Fixup* fixupWithMinusTarget, ld::Fixup* fixupWithStore,
													  const ld::Atom* target, const ld::Atom* minusTarget,
													  uint64_t targetAddend, uint64_t minusTargetAddend);
	void						addClassicRelocs(ld::Internal& state, ld::Internal::FinalSection* sect,
												const ld::Atom* atom, ld::Fixup* fixupWithTarget, 
												ld::Fixup* fixupWithMinusTarget, ld::Fixup* fixupWithStore,
												const ld::Atom* target, const ld::Atom* minusTarget, 
												uint64_t targetAddend, uint64_t minusTargetAddend);
	bool						useExternalSectionReloc(const ld::Atom* atom, const ld::Atom* target, 
															ld::Fixup* fixupWithTarget);
	bool						useSectionRelocAddend(ld::Fixup* fixupWithTarget);
	uint64_t					pageAlign(uint64_t addr);
	uint64_t					pageAlign(uint64_t addr, uint64_t pageSize);
	void						setLoadCommandsPadding(ld::Internal& state);
	void						assignAtomAddresses(ld::Internal& state);
	void						addRebaseInfo(const ld::Atom* atom, const ld::Fixup* fixup, const ld::Atom* target);
	void						makeRebasingInfo(ld::Internal& state);
	void						makeBindingInfo(ld::Internal& state);
	void						updateLINKEDITAddresses(ld::Internal& state);
	void						applyFixUps(ld::Internal& state, uint64_t mhAddress, const ld::Atom*  atom, uint8_t* buffer);
	uint64_t					addressOf(const ld::Internal& state, const ld::Fixup* fixup, const ld::Atom** target);
	uint64_t					addressAndTarget(const ld::Internal& state, const ld::Fixup* fixup, const ld::Atom** target);
	bool						targetIsThumb(ld::Internal& state, const ld::Fixup* fixup);
	uint32_t					lazyBindingInfoOffsetForLazyPointerAddress(uint64_t lpAddress);
	void						copyNoOps(uint8_t* from, uint8_t* to, bool thumb);
	bool						isPointerToTarget(ld::Fixup::Kind kind);
	bool						isPointerFromTarget(ld::Fixup::Kind kind);
	bool						isPcRelStore(const ld::Fixup* fixup);
	bool						isStore(ld::Fixup::Kind kind);
	bool						storeAddendOnly(const ld::Atom* inAtom, const ld::Atom* target, bool pcRel=false);
	bool						setsTarget(const ld::Fixup &fixup);
	void						addFixupOutInfo(ld::Internal& state);
	void						makeRelocations(ld::Internal& state);
	void						makeSectionRelocations(ld::Internal& state);
	void						makeDyldInfo(ld::Internal& state);
	void						buildChainedFixupInfo(ld::Internal& state);
	void						makeSplitSegInfo(ld::Internal& state);
	void						makeSplitSegInfoV2(ld::Internal& state);
	void						writeMapFile(ld::Internal& state);
	void						writeJSONEntry(ld::Internal& state);
	uint64_t					lookBackAddend(ld::Fixup::iterator fit);
	bool						takesNoDiskSpace(const ld::Section* sect);
	bool						hasZeroForFileOffset(const ld::Section* sect);
	
	void						printSectionLayout(ld::Internal& state);
	
	bool						checkThumbBranch22Displacement(int64_t displacement);
	bool						checkArmBranch24Displacement(int64_t displacement);

	void						rangeCheck8(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheck16(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheckBranch32(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheckAbsolute32(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheckRIP32(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheckARM12(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheckARMBranch24(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheckThumbBranch22(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheckARM64Branch26(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
	void						rangeCheckARM64Page21(int64_t delta, ld::Internal& state, const ld::Atom* atom, 
																							const ld::Fixup* fixup);
																							
																							
	uint64_t					sectionOffsetOf(const ld::Internal& state, const ld::Fixup* fixup);
	uint64_t					tlvTemplateOffsetOf(const ld::Internal& state, const ld::Fixup* fixup);
	void						synthesizeDebugNotes(ld::Internal& state);
	const char*					assureFullPath(const char* path);
	const char* 				canonicalOSOPath(const char* path);
	void						noteTextReloc(const ld::Atom* atom, const ld::Atom* target);
	void 						setFixup64(uint8_t* fixUpLocation, uint64_t accumulator, const ld::Atom* toTarget);
	void 						setFixup32(uint8_t* fixUpLocation, uint64_t accumulator, const ld::Atom* toTarget);
#if SUPPORT_ARCH_arm64e
	void 						setFixup64e(uint8_t* fixUpLocation, uint64_t accumulator, Fixup::AuthData authData, const ld::Atom* toTarget);
#endif
	bool 						isFixupForChain(ld::Fixup::iterator fit);
	uint16_t					chainedPointerFormat() const;
	void 						chain32bitPointers(dyld_chained_ptr_32_rebase* prevLoc, dyld_chained_ptr_32_rebase* loc,
													ChainedFixupSegInfo& segInfo, uint8_t* pageBufferStart, uint32_t pageIndex);
	dyld_chained_ptr_32_rebase* farthestChainableLocation(dyld_chained_ptr_32_rebase* start);
	bool 						chainableNonPointer(uint32_t value);
	void 						chain32bitFirmwarePointers(dyld_chained_ptr_32_firmware_rebase* prevLoc, dyld_chained_ptr_32_firmware_rebase* finalLoc,
															ChainedFixupSegInfo& segInfo, uint8_t* pageBufferStart, uint32_t pageIndex);
	dyld_chained_ptr_32_firmware_rebase* farthestChainableLocation(dyld_chained_ptr_32_firmware_rebase* start);

	struct InstructionInfo {
		uint32_t			offsetInAtom;
		const ld::Fixup*	fixup; 
		const ld::Atom*		target;	
		uint64_t			targetAddress;
		uint8_t*			instructionContent;
		uint64_t			instructionAddress;
		uint32_t			instruction;
	};


	class ChainedFixupBinds
	{
	public:
		void  	 ensureTarget(const ld::Atom* atom, bool authPtr, uint64_t addend);
		uint32_t count() const;
		bool  	 hasLargeAddends() const;
		bool     hasHugeAddends() const;
		bool	 hasHugeSymbolStrings() const;
		void	 forEachBind(void (^callback)(unsigned bindOrdinal, const ld::Atom* importAtom, uint64_t addend));
		uint32_t ordinal(const ld::Atom* atom, uint64_t addend) const;
		void	 setMaxRebase(uint64_t max) { _maxRebase = max; }
		uint64_t maxRebase() const { return _maxRebase; }
		
	private:
		struct AtomAndAddend {
			const ld::Atom*		atom;
			uint64_t			addend;
		};
		std::unordered_map<const ld::Atom*, uint32_t> 	_bindOrdinalsWithNoAddend;
		std::vector<AtomAndAddend>						_bindsTargets;
		uint64_t										_maxRebase = 0;
		bool											_hasLargeAddends = false;
		bool											_hasHugeAddends  = false;
	};


	void setInfo(ld::Internal& state, const ld::Atom* atom, uint8_t* buffer, const std::map<uint32_t, const Fixup*>& usedHints, 
						uint32_t offsetInAtom, uint32_t delta, InstructionInfo* info);

	static uint16_t				get16LE(uint8_t* loc);
	static void					set16LE(uint8_t* loc, uint16_t value);
	static uint32_t				get32LE(uint8_t* loc);
	static void					set32LE(uint8_t* loc, uint32_t value);
	static uint64_t				get64LE(uint8_t* loc);
	static void					set64LE(uint8_t* loc, uint64_t value);

	static uint16_t				get16BE(uint8_t* loc);
	static void					set16BE(uint8_t* loc, uint16_t value);
	static uint32_t				get32BE(uint8_t* loc);
	static void					set32BE(uint8_t* loc, uint32_t value);
	static uint64_t				get64BE(uint8_t* loc);
	static void					set64BE(uint8_t* loc, uint64_t value);



	const Options&							_options;
	std::map<const ld::dylib::File*, int>	_dylibToOrdinal;
	std::vector<const ld::dylib::File*>		_dylibsToLoad;
	std::vector<const char*>				_dylibOrdinalPaths;
	const bool								_hasDyldInfo;
	const bool								_hasExportsTrie;
	const bool								_hasChainedFixups;
	const bool								_hasThreadedPageStarts;
	const bool								_hasSymbolTable;
	const bool								_hasSectionRelocations;
	const bool								_hasSplitSegInfo;
	const bool								_hasFunctionStartsInfo;
	const bool								_hasDataInCodeInfo;
		  bool								_hasDynamicSymbolTable;
		  bool								_hasLocalRelocations;
		  bool								_hasExternalRelocations;
		  bool								_hasOptimizationHints;
		  bool								_hasCodeSignature;
	uint64_t								_fileSize;
	std::map<uint64_t, uint32_t>			_lazyPointerAddressToInfoOffset;
	uint32_t								_encryptedTEXTstartOffset;
	uint32_t								_encryptedTEXTendOffset;
public:
	std::vector<const ld::Atom*>			_localAtoms;
	std::vector<const ld::Atom*>			_exportedAtoms;
	std::vector<const ld::Atom*>			_importedAtoms;
	uint32_t								_localSymbolsStartIndex;
	uint32_t								_localSymbolsCount;
	uint32_t								_globalSymbolsStartIndex;
	uint32_t								_globalSymbolsCount;
	uint32_t								_importSymbolsStartIndex;
	uint32_t								_importSymbolsCount;
	std::map<const ld::Atom*, uint32_t>		_atomToSymbolIndex;
	std::vector<RebaseInfo>					_rebaseInfo;
	std::vector<BindingInfo>				_bindingInfo;
	std::vector<BindingInfo>				_lazyBindingInfo;
	std::vector<BindingInfo>				_weakBindingInfo;
	bool									_hasUnalignedFixup = false;
	// Note, <= 0 values are indices in to rebases, > 0 are binds.
	std::vector<int64_t>					_threadedRebaseBindIndices;
	std::vector<uint64_t>				 	_chainedFixupAddresses;
	std::unordered_map<const ld::Atom*, uint32_t> _chainedFixupNoAddendBindOrdinals;
	ChainedFixupBinds						_chainedFixupBinds;
	std::vector<ChainedFixupSegInfo>    	_chainedFixupSegments;
	size_t 									_importedSymbolsCount;
#if SUPPORT_ARCH_arm64e
	std::map<uintptr_t, std::pair<Fixup::AuthData, uint64_t>> _authenticatedFixupData;
#endif
	std::vector<SplitSegInfoEntry>			_splitSegInfos;
	std::vector<SplitSegInfoV2Entry>		_splitSegV2Infos;
	class HeaderAndLoadCommandsAbtract*		_headersAndLoadCommandAtom;
	class RelocationsAtomAbstract*			_sectionsRelocationsAtom;
	class RelocationsAtomAbstract*			_localRelocsAtom;
	class RelocationsAtomAbstract*			_externalRelocsAtom;
	class ClassicLinkEditAtom*				_symbolTableAtom;
	class ClassicLinkEditAtom*				_indirectSymbolTableAtom;
	class StringPoolAtom*					_stringPoolAtom;
	class LinkEditAtom*						_rebasingInfoAtom;
	class LinkEditAtom*						_bindingInfoAtom;
	class LinkEditAtom*						_lazyBindingInfoAtom;
	class LinkEditAtom*						_weakBindingInfoAtom;
	class LinkEditAtom*						_exportInfoAtom;
	class LinkEditAtom*						_splitSegInfoAtom;
	class LinkEditAtom*						_functionStartsAtom;
	class LinkEditAtom*						_dataInCodeAtom;
	class LinkEditAtom*						_optimizationHintsAtom;
	class LinkEditAtom*						_chainedInfoAtom;
	class CodeSignatureAtom*				_codeSignatureAtom;

};

} // namespace tool 
} // namespace ld 

#endif // __OUTPUT_FILE_H__
