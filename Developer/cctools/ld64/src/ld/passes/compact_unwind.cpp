/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
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


#ifdef __linux__
#include <algorithm>
#include <string.h>
#endif

#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <dlfcn.h>
#include <mach/machine.h>
#include <mach-o/compact_unwind_encoding.h>

#include <vector>
#include <map>

#include "ld.hpp"
#include "compact_unwind.h"
#include "Architectures.hpp"
#include "MachOFileAbstraction.hpp"


namespace ld {
namespace passes {
namespace compact_unwind {


struct UnwindEntry { 
						UnwindEntry(const ld::Atom* f, uint64_t a, uint32_t o, const ld::Atom* d, 
															const ld::Atom* l, const ld::Atom* p, uint32_t en)
							: func(f), fde(d), lsda(l), personalityPointer(p), funcTentAddress(a), 
								functionOffset(o), encoding(en) { }
	const ld::Atom*				func; 
	const ld::Atom*				fde; 
	const ld::Atom*				lsda; 
	const ld::Atom*				personalityPointer; 
	uint64_t					funcTentAddress;
	uint32_t					functionOffset;
	compact_unwind_encoding_t	encoding; 
};

struct LSDAEntry { 
	const ld::Atom*		func; 
	const ld::Atom*		lsda; 
};


template <typename A>
class UnwindInfoAtom : public ld::Atom {
public:
											UnwindInfoAtom(const std::vector<UnwindEntry>& entries,uint64_t ehFrameSize);
											~UnwindInfoAtom();
											
	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return "compact unwind info"; }
	virtual uint64_t						size() const					{ return _headerSize+_pagesSize; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const;
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixups[0]; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return (ld::Fixup*)&_fixups[_fixups.size()]; }

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;

	typedef macho_unwind_info_compressed_second_level_page_header<P> CSLP;

	bool						encodingMeansUseDwarf(compact_unwind_encoding_t enc);
	bool						encodingCannotBeMerged(compact_unwind_encoding_t enc);
	void						compressDuplicates(const std::vector<UnwindEntry>& entries,
													std::vector<UnwindEntry>& uniqueEntries);
	void						makePersonalityIndexes(std::vector<UnwindEntry>& entries, 
														std::map<const ld::Atom*, uint32_t>& personalityIndexMap);
	void						findCommonEncoding(const std::vector<UnwindEntry>& entries, 
													std::map<compact_unwind_encoding_t, unsigned int>& commonEncodings);
	void						makeLsdaIndex(const std::vector<UnwindEntry>& entries, std::vector<LSDAEntry>& lsdaIndex, 
																std::map<const ld::Atom*, uint32_t>& lsdaIndexOffsetMap);
	unsigned int				makeCompressedSecondLevelPage(const std::vector<UnwindEntry>& uniqueInfos,   
													const std::map<compact_unwind_encoding_t,unsigned int> commonEncodings,  
													uint32_t pageSize, unsigned int endIndex, uint8_t*& pageEnd);
	unsigned int				makeRegularSecondLevelPage(const std::vector<UnwindEntry>& uniqueInfos, uint32_t pageSize,  
															unsigned int endIndex, uint8_t*& pageEnd);
	void						addCompressedAddressOffsetFixup(uint32_t offset, const ld::Atom* func, const ld::Atom* fromFunc);
	void						addCompressedEncodingFixup(uint32_t offset, const ld::Atom* fde);
	void						addRegularAddressFixup(uint32_t offset, const ld::Atom* func);
	void						addRegularFDEOffsetFixup(uint32_t offset, const ld::Atom* fde);
	void						addImageOffsetFixup(uint32_t offset, const ld::Atom* targ);
	void						addImageOffsetFixupPlusAddend(uint32_t offset, const ld::Atom* targ, uint32_t addend);

	uint8_t*								_pagesForDelete;
	uint8_t*								_pageAlignedPages;
	uint8_t*								_pages;
	uint64_t								_pagesSize;
	uint8_t*								_header;
	uint64_t								_headerSize;
	std::vector<ld::Fixup>					_fixups;
	
	static bool								_s_log;
	static ld::Section						_s_section;
};

template <typename A>
bool UnwindInfoAtom<A>::_s_log = false;

template <typename A>
ld::Section UnwindInfoAtom<A>::_s_section("__TEXT", "__unwind_info", ld::Section::typeUnwindInfo);


template <typename A>
UnwindInfoAtom<A>::UnwindInfoAtom(const std::vector<UnwindEntry>& entries, uint64_t ehFrameSize)
	: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
				ld::Atom::scopeLinkageUnit, ld::Atom::typeUnclassified, 
				symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)),
		_pagesForDelete(NULL), _pageAlignedPages(NULL), _pages(NULL), _pagesSize(0), _header(NULL), _headerSize(0)
{
	// build new compressed list by removing entries where next function has same encoding 
	std::vector<UnwindEntry> uniqueEntries;
	compressDuplicates(entries, uniqueEntries);

	// reserve room so _fixups vector is not reallocated a bunch of times
	_fixups.reserve(uniqueEntries.size()*3);

	// build personality index, update encodings with personality index
	std::map<const ld::Atom*, uint32_t>	personalityIndexMap;
	makePersonalityIndexes(uniqueEntries, personalityIndexMap);
	if ( personalityIndexMap.size() > 3 ) {
		throw "too many personality routines for compact unwind to encode";
	}

	// put the most common encodings into the common table, but at most 127 of them
	std::map<compact_unwind_encoding_t, unsigned int> commonEncodings;
	findCommonEncoding(uniqueEntries, commonEncodings);
	
	// build lsda index
	std::map<const ld::Atom*, uint32_t> lsdaIndexOffsetMap;
	std::vector<LSDAEntry>	lsdaIndex;
	makeLsdaIndex(uniqueEntries, lsdaIndex, lsdaIndexOffsetMap);
	
	// calculate worst case size for all unwind info pages when allocating buffer
	const unsigned int entriesPerRegularPage = (4096-sizeof(unwind_info_regular_second_level_page_header))/sizeof(unwind_info_regular_second_level_entry);
	assert(uniqueEntries.size() > 0);
	const unsigned int pageCount = ((uniqueEntries.size() - 1)/entriesPerRegularPage) + 2;
	_pagesForDelete = (uint8_t*)calloc(pageCount+1,4096);
	if ( _pagesForDelete == NULL ) {
		warning("could not allocate space for compact unwind info");
		return;
	}
	_pageAlignedPages = (uint8_t*)((((uintptr_t)_pagesForDelete) + 4095) & -4096);
	
	// make last second level page smaller so that all other second level pages can be page aligned
	uint32_t maxLastPageSize = 4096 - (ehFrameSize % 4096);
	uint32_t tailPad = 0;
	if ( maxLastPageSize < 128 ) {
		tailPad = maxLastPageSize;
		maxLastPageSize = 4096;
	}
	
	// fill in pages in reverse order
	const ld::Atom* secondLevelFirstFuncs[pageCount*3];
	uint8_t* secondLevelPagesStarts[pageCount*3];
	unsigned int endIndex = uniqueEntries.size();
	unsigned int secondLevelPageCount = 0;
	uint8_t* pageEnd = &_pageAlignedPages[pageCount*4096];
	uint32_t pageSize = maxLastPageSize;
	while ( endIndex > 0 ) {
		endIndex = makeCompressedSecondLevelPage(uniqueEntries, commonEncodings, pageSize, endIndex, pageEnd);
		secondLevelPagesStarts[secondLevelPageCount] = pageEnd;
		secondLevelFirstFuncs[secondLevelPageCount] = uniqueEntries[endIndex].func;
		++secondLevelPageCount;
		// if this requires more than one page, align so that next starts on page boundary
		if ( (pageSize != 4096) && (endIndex > 0) ) {
			pageEnd = (uint8_t*)((uintptr_t)(pageEnd) & -4096);
			pageSize = 4096;  // last page can be odd size, make rest up to 4096 bytes in size
		}
	}
	_pages = pageEnd;
	_pagesSize = &_pageAlignedPages[pageCount*4096] - pageEnd;

	// calculate section layout
	const uint32_t commonEncodingsArraySectionOffset = sizeof(macho_unwind_info_section_header<P>);
	const uint32_t commonEncodingsArrayCount = commonEncodings.size();
	const uint32_t commonEncodingsArraySize = commonEncodingsArrayCount * sizeof(compact_unwind_encoding_t);
	const uint32_t personalityArraySectionOffset = commonEncodingsArraySectionOffset + commonEncodingsArraySize;
	const uint32_t personalityArrayCount = personalityIndexMap.size();
	const uint32_t personalityArraySize = personalityArrayCount * sizeof(uint32_t);
	const uint32_t indexSectionOffset = personalityArraySectionOffset + personalityArraySize;
	const uint32_t indexCount = secondLevelPageCount+1;
	const uint32_t indexSize = indexCount * sizeof(macho_unwind_info_section_header_index_entry<P>);
	const uint32_t lsdaIndexArraySectionOffset = indexSectionOffset + indexSize;
	const uint32_t lsdaIndexArrayCount = lsdaIndex.size();
	const uint32_t lsdaIndexArraySize = lsdaIndexArrayCount * sizeof(macho_unwind_info_section_header_lsda_index_entry<P>);
	const uint32_t headerEndSectionOffset = lsdaIndexArraySectionOffset + lsdaIndexArraySize;

	// now that we know the size of the header, slide all existing fixups on the pages
	const int32_t fixupSlide = headerEndSectionOffset + (_pageAlignedPages - _pages);
	for(std::vector<ld::Fixup>::iterator it = _fixups.begin(); it != _fixups.end(); ++it) {
		it->offsetInAtom += fixupSlide;
	}

	// allocate and fill in section header
	_headerSize = headerEndSectionOffset;
	_header = new uint8_t[_headerSize];
	bzero(_header, _headerSize);
	macho_unwind_info_section_header<P>* sectionHeader = (macho_unwind_info_section_header<P>*)_header;
	sectionHeader->set_version(UNWIND_SECTION_VERSION);
	sectionHeader->set_commonEncodingsArraySectionOffset(commonEncodingsArraySectionOffset);
	sectionHeader->set_commonEncodingsArrayCount(commonEncodingsArrayCount);
	sectionHeader->set_personalityArraySectionOffset(personalityArraySectionOffset);
	sectionHeader->set_personalityArrayCount(personalityArrayCount);
	sectionHeader->set_indexSectionOffset(indexSectionOffset);
	sectionHeader->set_indexCount(indexCount);
	
	// copy common encodings
	uint32_t* commonEncodingsTable = (uint32_t*)&_header[commonEncodingsArraySectionOffset];
	for (std::map<uint32_t, unsigned int>::iterator it=commonEncodings.begin(); it != commonEncodings.end(); ++it)
		E::set32(commonEncodingsTable[it->second], it->first);
		
	// make references for personality entries
	uint32_t* personalityArray = (uint32_t*)&_header[sectionHeader->personalityArraySectionOffset()];
	for (std::map<const ld::Atom*, unsigned int>::iterator it=personalityIndexMap.begin(); it != personalityIndexMap.end(); ++it) {
		uint32_t offset = (uint8_t*)&personalityArray[it->second-1] - _header;
		this->addImageOffsetFixup(offset, it->first);
	}

	// build first level index and references
	macho_unwind_info_section_header_index_entry<P>* indexTable = (macho_unwind_info_section_header_index_entry<P>*)&_header[indexSectionOffset];
	uint32_t refOffset;
	for (unsigned int i=0; i < secondLevelPageCount; ++i) {
		unsigned int reverseIndex = secondLevelPageCount - 1 - i;
		indexTable[i].set_functionOffset(0);
		indexTable[i].set_secondLevelPagesSectionOffset(secondLevelPagesStarts[reverseIndex]-_pages+headerEndSectionOffset);
		indexTable[i].set_lsdaIndexArraySectionOffset(lsdaIndexOffsetMap[secondLevelFirstFuncs[reverseIndex]]+lsdaIndexArraySectionOffset); 
		refOffset = (uint8_t*)&indexTable[i] - _header;
		this->addImageOffsetFixup(refOffset, secondLevelFirstFuncs[reverseIndex]);
	}
	indexTable[secondLevelPageCount].set_functionOffset(0);
	indexTable[secondLevelPageCount].set_secondLevelPagesSectionOffset(0);
	indexTable[secondLevelPageCount].set_lsdaIndexArraySectionOffset(lsdaIndexArraySectionOffset+lsdaIndexArraySize); 
	refOffset = (uint8_t*)&indexTable[secondLevelPageCount] - _header;
	this->addImageOffsetFixupPlusAddend(refOffset, entries.back().func, entries.back().func->size()+1);
	
	// build lsda references
	uint32_t lsdaEntrySectionOffset = lsdaIndexArraySectionOffset;
	for (std::vector<LSDAEntry>::iterator it = lsdaIndex.begin(); it != lsdaIndex.end(); ++it) {
		this->addImageOffsetFixup(lsdaEntrySectionOffset, it->func);
		this->addImageOffsetFixup(lsdaEntrySectionOffset+4, it->lsda);
		lsdaEntrySectionOffset += sizeof(unwind_info_section_header_lsda_index_entry);
	}
	
}

template <typename A>
UnwindInfoAtom<A>::~UnwindInfoAtom()
{
	free(_pagesForDelete);
	free(_header);
}

template <typename A>
void UnwindInfoAtom<A>::copyRawContent(uint8_t buffer[]) const
{
	// content is in two parts
	memcpy(buffer, _header, _headerSize);
	memcpy(&buffer[_headerSize], _pages, _pagesSize);
}


template <>
bool UnwindInfoAtom<x86>::encodingMeansUseDwarf(compact_unwind_encoding_t enc)
{
	return ((enc & UNWIND_X86_MODE_MASK) == UNWIND_X86_MODE_DWARF);
}

template <>
bool UnwindInfoAtom<x86_64>::encodingMeansUseDwarf(compact_unwind_encoding_t enc)
{
	return ((enc & UNWIND_X86_64_MODE_MASK) == UNWIND_X86_64_MODE_DWARF);
}

template <>
bool UnwindInfoAtom<arm64>::encodingMeansUseDwarf(compact_unwind_encoding_t enc)
{
	return ((enc & UNWIND_ARM64_MODE_MASK) == UNWIND_ARM64_MODE_DWARF);
}

#if SUPPORT_ARCH_arm64_32
template <>
bool UnwindInfoAtom<arm64_32>::encodingMeansUseDwarf(compact_unwind_encoding_t enc)
{
	return ((enc & UNWIND_ARM64_MODE_MASK) == UNWIND_ARM64_MODE_DWARF);
}
#endif

template <>
bool UnwindInfoAtom<arm>::encodingMeansUseDwarf(compact_unwind_encoding_t enc)
{
	return ((enc & UNWIND_ARM_MODE_MASK) == UNWIND_ARM_MODE_DWARF);
}




template <>
bool UnwindInfoAtom<x86>::encodingCannotBeMerged(compact_unwind_encoding_t enc)
{
	return ((enc & UNWIND_X86_MODE_MASK) == UNWIND_X86_MODE_STACK_IND);
}

template <>
bool UnwindInfoAtom<x86_64>::encodingCannotBeMerged(compact_unwind_encoding_t enc)
{
	return ((enc & UNWIND_X86_64_MODE_MASK) == UNWIND_X86_64_MODE_STACK_IND);
}

template <typename A>
bool UnwindInfoAtom<A>::encodingCannotBeMerged(compact_unwind_encoding_t enc)
{
	return false;
}


template <typename A>
void UnwindInfoAtom<A>::compressDuplicates(const std::vector<UnwindEntry>& entries, std::vector<UnwindEntry>& uniqueEntries)
{
	// build new list removing entries where next function has same encoding 
	uniqueEntries.reserve(entries.size());
	UnwindEntry last(NULL, 0, 0, NULL, NULL, NULL, 0xFFFFFFFF);
	for(std::vector<UnwindEntry>::const_iterator it=entries.begin(); it != entries.end(); ++it) {
		const UnwindEntry& next = *it;
		bool newNeedsDwarf = encodingMeansUseDwarf(next.encoding);
		bool cannotBeMerged = encodingCannotBeMerged(next.encoding);
		// remove entries which have same encoding and personalityPointer as last one
		if ( newNeedsDwarf || (next.encoding != last.encoding) || (next.personalityPointer != last.personalityPointer) 
			 || cannotBeMerged	|| (next.lsda != NULL) || (last.lsda != NULL) ) {
			uniqueEntries.push_back(next);
		}
		last = next;
	}
	if (_s_log) fprintf(stderr, "compressDuplicates() entries.size()=%lu, uniqueEntries.size()=%lu\n", 
								entries.size(), uniqueEntries.size());
}

template <typename A>
void UnwindInfoAtom<A>::makePersonalityIndexes(std::vector<UnwindEntry>& entries, std::map<const ld::Atom*, uint32_t>& personalityIndexMap)
{
	for(std::vector<UnwindEntry>::iterator it=entries.begin(); it != entries.end(); ++it) {
		if ( it->personalityPointer != NULL ) {
			std::map<const ld::Atom*, uint32_t>::iterator pos = personalityIndexMap.find(it->personalityPointer);
			if ( pos == personalityIndexMap.end() ) {
				const uint32_t nextIndex = personalityIndexMap.size() + 1;
				personalityIndexMap[it->personalityPointer] = nextIndex;
			}
			uint32_t personalityIndex = personalityIndexMap[it->personalityPointer];
			it->encoding |= (personalityIndex << (__builtin_ctz(UNWIND_PERSONALITY_MASK)) );
		}
	}
	if (_s_log) fprintf(stderr, "makePersonalityIndexes() %lu personality routines used\n", personalityIndexMap.size()); 
}


template <typename A>
void UnwindInfoAtom<A>::findCommonEncoding(const std::vector<UnwindEntry>& entries, 
											std::map<compact_unwind_encoding_t, unsigned int>& commonEncodings)
{
	// scan infos to get frequency counts for each encoding
	std::map<compact_unwind_encoding_t, unsigned int> encodingsUsed;
	unsigned int mostCommonEncodingUsageCount = 0;
	for(std::vector<UnwindEntry>::const_iterator it=entries.begin(); it != entries.end(); ++it) {
		// never put dwarf into common table
		if ( encodingMeansUseDwarf(it->encoding) )
			continue;
		std::map<compact_unwind_encoding_t, unsigned int>::iterator pos = encodingsUsed.find(it->encoding);
		if ( pos == encodingsUsed.end() ) {
			encodingsUsed[it->encoding] = 1;
		}
		else {
			encodingsUsed[it->encoding] += 1;
			if ( mostCommonEncodingUsageCount < encodingsUsed[it->encoding] )
				mostCommonEncodingUsageCount = encodingsUsed[it->encoding];
		}
	}
	// put the most common encodings into the common table, but at most 127 of them
	for(unsigned int usages=mostCommonEncodingUsageCount; usages > 1; --usages) {
		for (std::map<compact_unwind_encoding_t, unsigned int>::iterator euit=encodingsUsed.begin(); euit != encodingsUsed.end(); ++euit) {
			if ( euit->second == usages ) {
				unsigned int sz = commonEncodings.size();
				if ( sz < 127 ) {
					commonEncodings[euit->first] = sz;
				}
			}
		}
	}
	if (_s_log) fprintf(stderr, "findCommonEncoding() %lu common encodings found\n", commonEncodings.size()); 
}


template <typename A>
void UnwindInfoAtom<A>::makeLsdaIndex(const std::vector<UnwindEntry>& entries, std::vector<LSDAEntry>& lsdaIndex, std::map<const ld::Atom*, uint32_t>& lsdaIndexOffsetMap)
{
	for(std::vector<UnwindEntry>::const_iterator it=entries.begin(); it != entries.end(); ++it) {
		lsdaIndexOffsetMap[it->func] = lsdaIndex.size() * sizeof(unwind_info_section_header_lsda_index_entry);
		if ( it->lsda != NULL ) {
			LSDAEntry entry;
			entry.func = it->func;
			entry.lsda = it->lsda;
			lsdaIndex.push_back(entry);
		}
	}
	if (_s_log) fprintf(stderr, "makeLsdaIndex() %lu LSDAs found\n", lsdaIndex.size()); 
}


template <>
void UnwindInfoAtom<x86>::addCompressedAddressOffsetFixup(uint32_t offset, const ld::Atom* func, const ld::Atom* fromFunc)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetAddress, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindSubtractTargetAddress, fromFunc));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndianLow24of32));
}

template <>
void UnwindInfoAtom<x86_64>::addCompressedAddressOffsetFixup(uint32_t offset, const ld::Atom* func, const ld::Atom* fromFunc)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetAddress, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindSubtractTargetAddress, fromFunc));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndianLow24of32));
}

template <>
void UnwindInfoAtom<arm64>::addCompressedAddressOffsetFixup(uint32_t offset, const ld::Atom* func, const ld::Atom* fromFunc)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetAddress, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindSubtractTargetAddress, fromFunc));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndianLow24of32));
}

#if SUPPORT_ARCH_arm64_32
template <>
void UnwindInfoAtom<arm64_32>::addCompressedAddressOffsetFixup(uint32_t offset, const ld::Atom* func, const ld::Atom* fromFunc)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetAddress, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindSubtractTargetAddress, fromFunc));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndianLow24of32));
}
#endif

template <>
void UnwindInfoAtom<arm>::addCompressedAddressOffsetFixup(uint32_t offset, const ld::Atom* func, const ld::Atom* fromFunc)
{
	if ( fromFunc->isThumb() ) {
		_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, func));
		_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, fromFunc));
		_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 1));
		_fixups.push_back(ld::Fixup(offset, ld::Fixup::k4of4, ld::Fixup::kindStoreLittleEndianLow24of32));
	}
	else {
		_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetAddress, func));
		_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindSubtractTargetAddress, fromFunc));
		_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndianLow24of32));
	}
}

template <>
void UnwindInfoAtom<x86>::addCompressedEncodingFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}

template <>
void UnwindInfoAtom<x86_64>::addCompressedEncodingFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}

template <>
void UnwindInfoAtom<arm64>::addCompressedEncodingFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}

#if SUPPORT_ARCH_arm64_32
template <>
void UnwindInfoAtom<arm64_32>::addCompressedEncodingFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}
#endif

template <>
void UnwindInfoAtom<arm>::addCompressedEncodingFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}

template <>
void UnwindInfoAtom<x86>::addRegularAddressFixup(uint32_t offset, const ld::Atom* func)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}

template <>
void UnwindInfoAtom<x86_64>::addRegularAddressFixup(uint32_t offset, const ld::Atom* func)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}

template <>
void UnwindInfoAtom<arm64>::addRegularAddressFixup(uint32_t offset, const ld::Atom* func)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}

#if SUPPORT_ARCH_arm64_32
template <>
void UnwindInfoAtom<arm64_32>::addRegularAddressFixup(uint32_t offset, const ld::Atom* func)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}
#endif

template <>
void UnwindInfoAtom<arm>::addRegularAddressFixup(uint32_t offset, const ld::Atom* func)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, func));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}

template <>
void UnwindInfoAtom<x86>::addRegularFDEOffsetFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}

template <>
void UnwindInfoAtom<x86_64>::addRegularFDEOffsetFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}

template <>
void UnwindInfoAtom<arm64>::addRegularFDEOffsetFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}

#if SUPPORT_ARCH_arm64_32
template <>
void UnwindInfoAtom<arm64_32>::addRegularFDEOffsetFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}
#endif

template <>
void UnwindInfoAtom<arm>::addRegularFDEOffsetFixup(uint32_t offset, const ld::Atom* fde)
{
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k1of2, ld::Fixup::kindSetTargetSectionOffset, fde));
	_fixups.push_back(ld::Fixup(offset+4, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndianLow24of32));
}

template <>
void UnwindInfoAtom<x86>::addImageOffsetFixup(uint32_t offset, const ld::Atom* targ)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}

template <>
void UnwindInfoAtom<x86_64>::addImageOffsetFixup(uint32_t offset, const ld::Atom* targ)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}

template <>
void UnwindInfoAtom<arm64>::addImageOffsetFixup(uint32_t offset, const ld::Atom* targ)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}

#if SUPPORT_ARCH_arm64_32
template <>
void UnwindInfoAtom<arm64_32>::addImageOffsetFixup(uint32_t offset, const ld::Atom* targ)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}
#endif

template <>
void UnwindInfoAtom<arm>::addImageOffsetFixup(uint32_t offset, const ld::Atom* targ)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of2, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32));
}

template <>
void UnwindInfoAtom<x86>::addImageOffsetFixupPlusAddend(uint32_t offset, const ld::Atom* targ, uint32_t addend)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindAddAddend, addend));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndian32));
}

template <>
void UnwindInfoAtom<x86_64>::addImageOffsetFixupPlusAddend(uint32_t offset, const ld::Atom* targ, uint32_t addend)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindAddAddend, addend));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndian32));
}

template <>
void UnwindInfoAtom<arm64>::addImageOffsetFixupPlusAddend(uint32_t offset, const ld::Atom* targ, uint32_t addend)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindAddAddend, addend));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndian32));
}

#if SUPPORT_ARCH_arm64_32
template <>
void UnwindInfoAtom<arm64_32>::addImageOffsetFixupPlusAddend(uint32_t offset, const ld::Atom* targ, uint32_t addend)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindAddAddend, addend));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndian32));
}
#endif

template <>
void UnwindInfoAtom<arm>::addImageOffsetFixupPlusAddend(uint32_t offset, const ld::Atom* targ, uint32_t addend)
{
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k1of3, ld::Fixup::kindSetTargetImageOffset, targ));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k2of3, ld::Fixup::kindAddAddend, addend));
	_fixups.push_back(ld::Fixup(offset, ld::Fixup::k3of3, ld::Fixup::kindStoreLittleEndian32));
}




template <typename A>
unsigned int UnwindInfoAtom<A>::makeRegularSecondLevelPage(const std::vector<UnwindEntry>& uniqueInfos, uint32_t pageSize,  
															unsigned int endIndex, uint8_t*& pageEnd)
{
	const unsigned int maxEntriesPerPage = (pageSize - sizeof(unwind_info_regular_second_level_page_header))/sizeof(unwind_info_regular_second_level_entry);
	const unsigned int entriesToAdd = ((endIndex > maxEntriesPerPage) ? maxEntriesPerPage : endIndex);
	uint8_t* pageStart = pageEnd 
						- entriesToAdd*sizeof(unwind_info_regular_second_level_entry) 
						- sizeof(unwind_info_regular_second_level_page_header);
	macho_unwind_info_regular_second_level_page_header<P>* page = (macho_unwind_info_regular_second_level_page_header<P>*)pageStart;
	page->set_kind(UNWIND_SECOND_LEVEL_REGULAR);
	page->set_entryPageOffset(sizeof(macho_unwind_info_regular_second_level_page_header<P>));
	page->set_entryCount(entriesToAdd);
	macho_unwind_info_regular_second_level_entry<P>* entryTable = (macho_unwind_info_regular_second_level_entry<P>*)(pageStart + page->entryPageOffset());
	for (unsigned int i=0; i < entriesToAdd; ++i) {
		const UnwindEntry& info = uniqueInfos[endIndex-entriesToAdd+i];
		entryTable[i].set_functionOffset(0);
		entryTable[i].set_encoding(info.encoding);
		// add fixup for address part of entry
		uint32_t offset = (uint8_t*)(&entryTable[i]) - _pageAlignedPages;
		this->addRegularAddressFixup(offset, info.func);
		if ( encodingMeansUseDwarf(info.encoding) ) {
			// add fixup for dwarf offset part of page specific encoding
			uint32_t encOffset = (uint8_t*)(&entryTable[i]) - _pageAlignedPages;
			this->addRegularFDEOffsetFixup(encOffset, info.fde);
		}
	}
	if (_s_log) fprintf(stderr, "regular page with %u entries\n", entriesToAdd);
	pageEnd = pageStart;
	return endIndex - entriesToAdd;
}


template <typename A>
unsigned int UnwindInfoAtom<A>::makeCompressedSecondLevelPage(const std::vector<UnwindEntry>& uniqueInfos,   
													const std::map<compact_unwind_encoding_t,unsigned int> commonEncodings,  
													uint32_t pageSize, unsigned int endIndex, uint8_t*& pageEnd)
{
	if (_s_log) fprintf(stderr, "makeCompressedSecondLevelPage(pageSize=%u, endIndex=%u)\n", pageSize, endIndex);
	// first pass calculates how many compressed entries we could fit in this sized page
	// keep adding entries to page until:
	//  1) encoding table plus entry table plus header exceed page size
	//  2) the file offset delta from the first to last function > 24 bits
	//  3) custom encoding index reaches 255
	//  4) run out of uniqueInfos to encode
	std::map<compact_unwind_encoding_t, unsigned int> pageSpecificEncodings;
	uint32_t space4 =  (pageSize - sizeof(unwind_info_compressed_second_level_page_header))/sizeof(uint32_t);
	int index = endIndex-1;
	int entryCount = 0;
	uint64_t lastEntryAddress = uniqueInfos[index].funcTentAddress;
	bool canDo = true;
	while ( canDo && (index >= 0) ) {
		const UnwindEntry& info = uniqueInfos[index--];
		// compute encoding index
		unsigned int encodingIndex;
		std::map<compact_unwind_encoding_t, unsigned int>::const_iterator pos = commonEncodings.find(info.encoding);
		if ( pos != commonEncodings.end() ) {
			encodingIndex = pos->second;
			if (_s_log) fprintf(stderr, "makeCompressedSecondLevelPage(): funcIndex=%d, re-use commonEncodings[%d]=0x%08X\n", index, encodingIndex, info.encoding);
		}
		else {
			// no commmon entry, so add one on this page
			uint32_t encoding = info.encoding;
			if ( encodingMeansUseDwarf(encoding) ) {
				// make unique pseudo encoding so this dwarf will gets is own encoding entry slot
				encoding += (index+1);
			}
			std::map<compact_unwind_encoding_t, unsigned int>::iterator ppos = pageSpecificEncodings.find(encoding);
			if ( ppos != pageSpecificEncodings.end() ) {
				encodingIndex = pos->second;
				if (_s_log) fprintf(stderr, "makeCompressedSecondLevelPage(): funcIndex=%d, re-use pageSpecificEncodings[%d]=0x%08X\n", index, encodingIndex, encoding);
			}
			else {
				encodingIndex = commonEncodings.size() + pageSpecificEncodings.size();
				if ( encodingIndex <= 255 ) {
					pageSpecificEncodings[encoding] = encodingIndex;
					if (_s_log) fprintf(stderr, "makeCompressedSecondLevelPage(): funcIndex=%d, pageSpecificEncodings[%d]=0x%08X\n", index, encodingIndex, encoding);
				}
				else {
					canDo = false; // case 3)
					if (_s_log) fprintf(stderr, "end of compressed page with %u entries, %lu custom encodings because too many custom encodings\n", 
											entryCount, pageSpecificEncodings.size());
				}
			}
		}
		// compute function offset
		uint32_t funcOffsetWithInPage = lastEntryAddress - info.funcTentAddress;
		if ( funcOffsetWithInPage > 0x00FFFF00 ) {
			// don't use 0x00FFFFFF because addresses may vary after atoms are laid out again
			canDo = false; // case 2)
			if (_s_log) fprintf(stderr, "can't use compressed page with %u entries because function offset too big\n", entryCount);
		}
		// check room for entry
		if ( (pageSpecificEncodings.size()+entryCount) > space4 ) {
			canDo = false; // case 1)
			--entryCount;
			if (_s_log) fprintf(stderr, "end of compressed page with %u entries because full\n", entryCount);
		}
		//if (_s_log) fprintf(stderr, "space4=%d, pageSpecificEncodings.size()=%ld, entryCount=%d\n", space4, pageSpecificEncodings.size(), entryCount);
		if ( canDo ) {
			++entryCount;
		}
	}
	
	// check for cases where it would be better to use a regular (non-compressed) page
	const unsigned int compressPageUsed = sizeof(unwind_info_compressed_second_level_page_header) 
								+ pageSpecificEncodings.size()*sizeof(uint32_t)
								+ entryCount*sizeof(uint32_t);
	if ( (compressPageUsed < (pageSize-4) && (index >= 0) ) ) {
		const int regularEntriesPerPage = (pageSize - sizeof(unwind_info_regular_second_level_page_header))/sizeof(unwind_info_regular_second_level_entry);
		if ( entryCount < regularEntriesPerPage ) {
			return makeRegularSecondLevelPage(uniqueInfos, pageSize, endIndex, pageEnd);
		}
	}
	
	// check if we need any padding because adding another entry would take 8 bytes but only have room for 4
	uint32_t pad = 0;
	if ( compressPageUsed == (pageSize-4) )
		pad = 4;

	// second pass fills in page 
	uint8_t* pageStart = pageEnd - compressPageUsed - pad;
	CSLP* page = (CSLP*)pageStart;
	page->set_kind(UNWIND_SECOND_LEVEL_COMPRESSED);
	page->set_entryPageOffset(sizeof(CSLP));
	page->set_entryCount(entryCount);
	page->set_encodingsPageOffset(page->entryPageOffset()+entryCount*sizeof(uint32_t));
	page->set_encodingsCount(pageSpecificEncodings.size());
	uint32_t* const encodingsArray = (uint32_t*)&pageStart[page->encodingsPageOffset()];
	// fill in entry table
	uint32_t* const entiresArray = (uint32_t*)&pageStart[page->entryPageOffset()];
	const ld::Atom* firstFunc = uniqueInfos[endIndex-entryCount].func;
	for(unsigned int i=endIndex-entryCount; i < endIndex; ++i) {
		const UnwindEntry& info = uniqueInfos[i];
		uint8_t encodingIndex;
		if ( encodingMeansUseDwarf(info.encoding) ) {
			// dwarf entries are always in page specific encodings
			assert(pageSpecificEncodings.find(info.encoding+i) != pageSpecificEncodings.end());
			encodingIndex = pageSpecificEncodings[info.encoding+i];
		}
		else {
			std::map<uint32_t, unsigned int>::const_iterator pos = commonEncodings.find(info.encoding);
			if ( pos != commonEncodings.end() ) 
				encodingIndex = pos->second;
			else 
				encodingIndex = pageSpecificEncodings[info.encoding];
		}
		uint32_t entryIndex = i - endIndex + entryCount;
		E::set32(entiresArray[entryIndex], encodingIndex << 24);
		// add fixup for address part of entry
		uint32_t offset = (uint8_t*)(&entiresArray[entryIndex]) - _pageAlignedPages;
		this->addCompressedAddressOffsetFixup(offset, info.func, firstFunc);
		if ( encodingMeansUseDwarf(info.encoding) ) {
			// add fixup for dwarf offset part of page specific encoding
			uint32_t encOffset = (uint8_t*)(&encodingsArray[encodingIndex-commonEncodings.size()]) - _pageAlignedPages;
			this->addCompressedEncodingFixup(encOffset, info.fde);
		}
	}
	// fill in encodings table
	for(std::map<uint32_t, unsigned int>::const_iterator it = pageSpecificEncodings.begin(); it != pageSpecificEncodings.end(); ++it) {
		E::set32(encodingsArray[it->second-commonEncodings.size()], it->first);
	}
	
	if (_s_log) fprintf(stderr, "compressed page with %u entries, %lu custom encodings\n", entryCount, pageSpecificEncodings.size());
	
	// update pageEnd;
	pageEnd = pageStart;
	return endIndex-entryCount;  // endIndex for next page
}




static uint64_t calculateEHFrameSize(ld::Internal& state)
{
	bool allCIEs = true;
	uint64_t size = 0;
	for (ld::Internal::FinalSection* sect : state.sections) {
		if ( sect->type() == ld::Section::typeCFI ) {
			for (const ld::Atom* atom : sect->atoms) {
				size += atom->size();
				if ( strcmp(atom->name(), "CIE") != 0 )
					allCIEs = false;
			}
			if ( allCIEs ) {
				// <rdar://problem/21427393> Linker generates eh_frame data even when there's only an unused CIEs in it
				sect->atoms.clear();
				state.sections.erase(std::remove(state.sections.begin(), state.sections.end(), sect), state.sections.end());
				return 0;
			}
		}
	}
	return size;
}

static void getAllUnwindInfos(const ld::Internal& state, std::vector<UnwindEntry>& entries)
{
	uint64_t address = 0;
	for (std::vector<ld::Internal::FinalSection*>::const_iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			// adjust address for atom alignment
			uint64_t alignment = 1 << atom->alignment().powerOf2;
			uint64_t currentModulus = (address % alignment);
			uint64_t requiredModulus = atom->alignment().modulus;
			if ( currentModulus != requiredModulus ) {
				if ( requiredModulus > currentModulus )
					address += requiredModulus-currentModulus;
				else
					address += requiredModulus+alignment-currentModulus;
			}

			if ( atom->beginUnwind() == atom->endUnwind() ) {
				// be sure to mark that we have no unwind info for stuff in the TEXT segment without unwind info
				if ( (atom->section().type() == ld::Section::typeCode) && (atom->size() !=0) ) {
					entries.push_back(UnwindEntry(atom, address, 0, NULL, NULL, NULL, 0));
				}
			}
			else {
				// atom has unwind info(s), add entry for each
				const ld::Atom*	fde = NULL;
				const ld::Atom*	lsda = NULL; 
				const ld::Atom*	personalityPointer = NULL; 
				for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
					switch ( fit->kind ) {
						case ld::Fixup::kindNoneGroupSubordinateFDE:
							assert(fit->binding == ld::Fixup::bindingDirectlyBound);
							fde = fit->u.target;
							break;
						case ld::Fixup::kindNoneGroupSubordinateLSDA:
							assert(fit->binding == ld::Fixup::bindingDirectlyBound);
							lsda = fit->u.target;
							break;
						case ld::Fixup::kindNoneGroupSubordinatePersonality:
							assert(fit->binding == ld::Fixup::bindingDirectlyBound);
							personalityPointer = fit->u.target;
							assert(personalityPointer->section().type() == ld::Section::typeNonLazyPointer);
							break;
						default:
							break;
					}
				}
				if ( fde != NULL ) {
					// find CIE for this FDE
					const ld::Atom*	cie = NULL;
					for (ld::Fixup::iterator fit = fde->fixupsBegin(), end=fde->fixupsEnd(); fit != end; ++fit) {
						if ( fit->kind != ld::Fixup::kindSubtractTargetAddress )
							continue;
						if ( fit->binding != ld::Fixup::bindingDirectlyBound )
							continue;
						cie = fit->u.target;
						// CIE is only direct subtracted target in FDE
						assert(cie->section().type() == ld::Section::typeCFI);
						break;
					}
					if ( cie != NULL ) {
						// if CIE can have just one fixup - to the personality pointer
						for (ld::Fixup::iterator fit = cie->fixupsBegin(), end=cie->fixupsEnd(); fit != end; ++fit) {
							if ( fit->kind == ld::Fixup::kindSetTargetAddress ) {
								switch ( fit->binding ) {
									case ld::Fixup::bindingsIndirectlyBound:
										personalityPointer = state.indirectBindingTable[fit->u.bindingIndex];
										assert(personalityPointer->section().type() == ld::Section::typeNonLazyPointer);
										break;
									case ld::Fixup::bindingDirectlyBound:
										personalityPointer = fit->u.target;
										assert(personalityPointer->section().type() == ld::Section::typeNonLazyPointer);
										break;
									default:
										break;
								}
							}
						}
					}
				}
				for ( ld::Atom::UnwindInfo::iterator uit = atom->beginUnwind(); uit != atom->endUnwind(); ++uit ) {
					entries.push_back(UnwindEntry(atom, address, uit->startOffset, fde, lsda, personalityPointer, uit->unwindInfo));
				}
			}
			address += atom->size();
		}
	}
}


static void makeFinalLinkedImageCompactUnwindSection(const Options& opts, ld::Internal& state)
{
	// walk every atom and gets its unwind info
	std::vector<UnwindEntry> entries;
	entries.reserve(64);
	getAllUnwindInfos(state, entries);

	// don't generate an __unwind_info section if there is no code in this linkage unit
	if ( entries.size() == 0 )
		return;
		
	// calculate size of __eh_frame section, so __unwind_info can go before it and page align
	uint64_t ehFrameSize = calculateEHFrameSize(state);

	// create atom that contains the whole compact unwind table
	switch ( opts.architecture() ) {
#if SUPPORT_ARCH_x86_64
		case CPU_TYPE_X86_64:
			state.addAtom(*new UnwindInfoAtom<x86_64>(entries, ehFrameSize));
			break;
#endif
#if SUPPORT_ARCH_i386
		case CPU_TYPE_I386:
			state.addAtom(*new UnwindInfoAtom<x86>(entries, ehFrameSize));
			break;
#endif
#if SUPPORT_ARCH_arm64
		case CPU_TYPE_ARM64:
			state.addAtom(*new UnwindInfoAtom<arm64>(entries, ehFrameSize));
			break;
#endif
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
			state.addAtom(*new UnwindInfoAtom<arm64_32>(entries, ehFrameSize));
			break;
#endif
#if SUPPORT_ARCH_arm_any
		case CPU_TYPE_ARM:
			if ( opts.armUsesZeroCostExceptions() )
				state.addAtom(*new UnwindInfoAtom<arm>(entries, ehFrameSize));
			break;
#endif
		default:
			assert(0 && "no compact unwind for arch");
	}	
}



template <typename A>
class CompactUnwindAtom : public ld::Atom {
public:
											CompactUnwindAtom(ld::Internal& state,const ld::Atom* funcAtom, 
															  uint32_t startOffset, uint32_t len, uint32_t cui);
											~CompactUnwindAtom() {}
											
	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return "compact unwind info"; }
	virtual uint64_t						size() const					{ return sizeof(macho_compact_unwind_entry<P>); }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const;
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixups[0]; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return (ld::Fixup*)&_fixups[_fixups.size()]; }

private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;
	

	const ld::Atom*							_atom;
	const uint32_t							_startOffset;
	const uint32_t							_len;
	const uint32_t							_compactUnwindInfo;
	std::vector<ld::Fixup>					_fixups;
	
	static ld::Fixup::Kind					_s_pointerKind;
	static ld::Fixup::Kind					_s_pointerStoreKind;
	static ld::Section						_s_section;
};


template <typename A>
ld::Section CompactUnwindAtom<A>::_s_section("__LD", "__compact_unwind", ld::Section::typeDebug);

template <> ld::Fixup::Kind CompactUnwindAtom<x86>::_s_pointerKind = ld::Fixup::kindStoreLittleEndian32;
template <> ld::Fixup::Kind CompactUnwindAtom<x86>::_s_pointerStoreKind = ld::Fixup::kindStoreTargetAddressLittleEndian32;
template <> ld::Fixup::Kind CompactUnwindAtom<x86_64>::_s_pointerKind = ld::Fixup::kindStoreLittleEndian64;
template <> ld::Fixup::Kind CompactUnwindAtom<x86_64>::_s_pointerStoreKind = ld::Fixup::kindStoreTargetAddressLittleEndian64;
#if SUPPORT_ARCH_arm64
template <> ld::Fixup::Kind CompactUnwindAtom<arm64>::_s_pointerKind = ld::Fixup::kindStoreLittleEndian64;
template <> ld::Fixup::Kind CompactUnwindAtom<arm64>::_s_pointerStoreKind = ld::Fixup::kindStoreTargetAddressLittleEndian64;
#endif
#if SUPPORT_ARCH_arm64_32
template <> ld::Fixup::Kind CompactUnwindAtom<arm64_32>::_s_pointerKind = ld::Fixup::kindStoreLittleEndian32;
template <> ld::Fixup::Kind CompactUnwindAtom<arm64_32>::_s_pointerStoreKind = ld::Fixup::kindStoreTargetAddressLittleEndian32;
#endif
template <> ld::Fixup::Kind CompactUnwindAtom<arm>::_s_pointerKind = ld::Fixup::kindStoreLittleEndian32;
template <> ld::Fixup::Kind CompactUnwindAtom<arm>::_s_pointerStoreKind = ld::Fixup::kindStoreTargetAddressLittleEndian32;

template <typename A>
CompactUnwindAtom<A>::CompactUnwindAtom(ld::Internal& state,const ld::Atom* funcAtom, uint32_t startOffset,
										uint32_t len, uint32_t cui)
	: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
				ld::Atom::scopeTranslationUnit, ld::Atom::typeUnclassified, 
				symbolTableNotIn, false, false, false, ld::Atom::Alignment(log2(sizeof(pint_t)))),
	_atom(funcAtom), _startOffset(startOffset), _len(len), _compactUnwindInfo(cui)
{
	_fixups.push_back(ld::Fixup(macho_compact_unwind_entry<P>::codeStartFieldOffset(), ld::Fixup::k1of3, ld::Fixup::kindSetTargetAddress, funcAtom));
	_fixups.push_back(ld::Fixup(macho_compact_unwind_entry<P>::codeStartFieldOffset(), ld::Fixup::k2of3, ld::Fixup::kindAddAddend, _startOffset));
	_fixups.push_back(ld::Fixup(macho_compact_unwind_entry<P>::codeStartFieldOffset(), ld::Fixup::k3of3, _s_pointerKind));
	// see if atom has subordinate personality function or lsda
	for (ld::Fixup::iterator fit = funcAtom->fixupsBegin(), end=funcAtom->fixupsEnd(); fit != end; ++fit) {
		switch ( fit->kind ) {
			case ld::Fixup::kindNoneGroupSubordinatePersonality:
				assert(fit->binding == ld::Fixup::bindingsIndirectlyBound);
				_fixups.push_back(ld::Fixup(macho_compact_unwind_entry<P>::personalityFieldOffset(), ld::Fixup::k1of1, _s_pointerStoreKind, state.indirectBindingTable[fit->u.bindingIndex]));
				break;
			case ld::Fixup::kindNoneGroupSubordinateLSDA:
				assert(fit->binding == ld::Fixup::bindingDirectlyBound);
				_fixups.push_back(ld::Fixup(macho_compact_unwind_entry<P>::lsdaFieldOffset(), ld::Fixup::k1of1, _s_pointerStoreKind, fit->u.target));
				break;
			default:
				break;
		}
	}

}

template <typename A>
void CompactUnwindAtom<A>::copyRawContent(uint8_t buffer[]) const
{
	macho_compact_unwind_entry<P>* buf = (macho_compact_unwind_entry<P>*)buffer;
	buf->set_codeStart(0);
	buf->set_codeLen(_len);
	buf->set_compactUnwindInfo(_compactUnwindInfo);
	buf->set_personality(0);
	buf->set_lsda(0);
}


static void makeCompactUnwindAtom(const Options& opts, ld::Internal& state, const ld::Atom* atom, 
											uint32_t startOffset, uint32_t endOffset, uint32_t cui)
{
	switch ( opts.architecture() ) {
#if SUPPORT_ARCH_x86_64
		case CPU_TYPE_X86_64:
			state.addAtom(*new CompactUnwindAtom<x86_64>(state, atom, startOffset, endOffset-startOffset, cui));
			break;
#endif
#if SUPPORT_ARCH_i386
		case CPU_TYPE_I386:
			state.addAtom(*new CompactUnwindAtom<x86>(state, atom, startOffset, endOffset-startOffset, cui));
			break;
#endif
#if SUPPORT_ARCH_arm64
		case CPU_TYPE_ARM64:
			state.addAtom(*new CompactUnwindAtom<arm64>(state, atom, startOffset, endOffset-startOffset, cui));
			break;
#endif
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
			state.addAtom(*new CompactUnwindAtom<arm64_32>(state, atom, startOffset, endOffset-startOffset, cui));
			break;
#endif
		case CPU_TYPE_ARM:
			state.addAtom(*new CompactUnwindAtom<arm>(state, atom, startOffset, endOffset-startOffset, cui));
			break;
	}
}

static void makeRelocateableCompactUnwindSection(const Options& opts, ld::Internal& state)
{
	// can't add CompactUnwindAtom atoms will iterating, so pre-scan
	std::vector<const ld::Atom*> atomsWithUnwind;
	for (std::vector<ld::Internal::FinalSection*>::const_iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			if ( atom->beginUnwind() != atom->endUnwind() ) 
				atomsWithUnwind.push_back(atom);
		}
	}
	// make one CompactUnwindAtom for each compact unwind range in each atom
	for (std::vector<const ld::Atom*>::iterator it = atomsWithUnwind.begin(); it != atomsWithUnwind.end(); ++it) {
		const ld::Atom* atom = *it;
		uint32_t lastOffset = 0;
		uint32_t lastCUE = 0;
		bool first = true;
		for (ld::Atom::UnwindInfo::iterator uit=atom->beginUnwind(); uit != atom->endUnwind(); ++uit) {
			if ( !first ) {
				makeCompactUnwindAtom(opts, state, atom, lastOffset, uit->startOffset, lastCUE);
			}
			lastOffset = uit->startOffset;
			lastCUE = uit->unwindInfo;
			first = false;
		}
		makeCompactUnwindAtom(opts, state, atom, lastOffset, (uint32_t)atom->size(), lastCUE);
	}
}


void doPass(const Options& opts, ld::Internal& state)
{
	if ( opts.outputKind() == Options::kObjectFile )
		makeRelocateableCompactUnwindSection(opts, state);
		
	else if ( opts.needsUnwindInfoSection() ) 
		makeFinalLinkedImageCompactUnwindSection(opts, state);
}


} // namespace compact_unwind
} // namespace passes 
} // namespace ld 
