/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-*
 *
 * Copyright (c) 2005-2011 Apple Inc. All rights reserved.
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
 
// start temp HACK for cross builds
//extern "C" double log2 ( double );

//#define __MATH__
// end temp HACK for cross builds

#ifdef __linux__
#include <algorithm>
#include <string.h>
namespace {
	typedef long double max_align_t;
}
#endif

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
#include <execinfo.h>
#include <mach/mach_time.h>
#include <mach/vm_statistics.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <AvailabilityMacros.h>

#include <string>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <cxxabi.h>

#include "Options.h"

#include "MachOFileAbstraction.hpp"
#include "Architectures.hpp"
#include "ld.hpp"

#include "InputFiles.h"
#include "Resolver.h"
#include "OutputFile.h"
#include "Snapshot.h"

#include "passes/stubs/make_stubs.h"
#include "passes/dtrace_dof.h"
#include "passes/got.h"
#include "passes/tlvp.h"
#include "passes/huge.h"
#include "passes/inits.h"
#include "passes/compact_unwind.h"
#include "passes/order.h"
#include "passes/branch_island.h"
#include "passes/thread_starts.h"
#include "passes/branch_shim.h"
#include "passes/objc.h"
#include "passes/objc_constants.h"
#include "passes/dylibs.h"
#include "passes/bitcode_bundle.h"
#include "passes/code_dedup.h"

#include "parsers/archive_file.h"
#include "parsers/macho_relocatable_file.h"
#include "parsers/macho_dylib_file.h"
#include "parsers/lto_file.h"
#include "parsers/opaque_section_file.h"


const ld::VersionSet ld::File::_platforms;

struct PerformanceStatistics {
	uint64_t						startTool;
	uint64_t						startInputFileProcessing;
	uint64_t						startResolver;
	uint64_t						startDylibs;
	uint64_t						startPasses;
	uint64_t						startOutput;
	uint64_t						startDone;
	vm_statistics_data_t			vmStart;
	vm_statistics_data_t			vmEnd;
};


class InternalState : public ld::Internal
{
public:
											InternalState(const Options& opts) : _options(opts), _atomsOrderedInSections(false) { }
	virtual	ld::Internal::FinalSection*		addAtom(const ld::Atom& atom);
	virtual ld::Internal::FinalSection*		getFinalSection(const ld::Section&);
			ld::Internal::FinalSection*     getFinalSection(const char* seg, const char* sect, ld::Section::Type type);
	
	uint64_t								assignFileOffsets();
	void									setSectionSizesAndAlignments();
	void									sortSections();
	void									markAtomsOrdered() { _atomsOrderedInSections = true; }
	bool									hasReferenceToWeakExternal(const ld::Atom& atom);

	virtual									~InternalState() {}
private:
	bool									inMoveRWChain(const ld::Atom& atom, const char* filePath, bool followedBackBranch, const char*& dstSeg, bool& wildCardMatch);
	bool									inMoveROChain(const ld::Atom& atom, const char* filePath, const char*& dstSeg, bool& wildCardMatch);
	bool									inMoveAuthChain(const ld::Atom& atom, bool followedBackBranch, const char*& dstSeg);

	class FinalSection : public ld::Internal::FinalSection 
	{
	public:
									FinalSection(const ld::Section& sect, uint32_t sectionsSeen, const Options&);
		static const ld::Section&	outputSection(const ld::Section& sect, bool mergeZeroFill);
		static const ld::Section&	objectOutputSection(const ld::Section& sect, const Options&);
	private:
		friend class InternalState;
		static uint32_t		sectionOrder(const ld::Section& sect, uint32_t sectionsSeen, const Options& options);
		static uint32_t		segmentOrder(const ld::Section& sect, const Options& options);
		uint32_t			_segmentOrder;
		uint32_t			_sectionOrder;

		static std::vector<const char*> _s_segmentsSeen;
		static ld::Section		_s_DATA_data;
		static ld::Section		_s_DATA_const;
		static ld::Section		_s_TEXT_text;
		static ld::Section		_s_TEXT_const;
		static ld::Section		_s_DATA_nl_symbol_ptr;
		static ld::Section		_s_DATA_common;
		static ld::Section		_s_DATA_zerofill;
		static ld::Section		_s_DATA_DIRTY_data;
		static ld::Section		_s_DATA_CONST_const;
	};
	
	bool hasZeroForFileOffset(const ld::Section* sect);
	uint64_t pageAlign(uint64_t addr);
	uint64_t pageAlign(uint64_t addr, uint64_t pageSize);
	
	struct SectionHash {
		size_t operator()(const ld::Section*) const;
	};
	struct SectionEquals {
		bool operator()(const ld::Section* left, const ld::Section* right) const;
	};
	typedef std::unordered_map<const ld::Section*, FinalSection*, SectionHash, SectionEquals> SectionInToOut;
	

	SectionInToOut			_sectionInToFinalMap;
	const Options&			_options;
	bool					_atomsOrderedInSections;
	std::unordered_map<const ld::Atom*, const char*> _pendingSegMove;
};

ld::Section	InternalState::FinalSection::_s_DATA_data( "__DATA", "__data",  ld::Section::typeUnclassified);
ld::Section	InternalState::FinalSection::_s_DATA_const("__DATA", "__const", ld::Section::typeUnclassified);
ld::Section	InternalState::FinalSection::_s_TEXT_text( "__TEXT", "__text",  ld::Section::typeCode);
ld::Section	InternalState::FinalSection::_s_TEXT_const("__TEXT", "__const", ld::Section::typeUnclassified);
ld::Section	InternalState::FinalSection::_s_DATA_nl_symbol_ptr("__DATA", "__nl_symbol_ptr", ld::Section::typeNonLazyPointer);
ld::Section	InternalState::FinalSection::_s_DATA_common("__DATA", "__common", ld::Section::typeZeroFill);
ld::Section	InternalState::FinalSection::_s_DATA_zerofill("__DATA", "__zerofill", ld::Section::typeZeroFill);
ld::Section	InternalState::FinalSection::_s_DATA_DIRTY_data( "__DATA_DIRTY", "__data",  ld::Section::typeUnclassified);
ld::Section	InternalState::FinalSection::_s_DATA_CONST_const( "__DATA_CONST", "__const",  ld::Section::typeUnclassified);

std::vector<const char*> InternalState::FinalSection::_s_segmentsSeen;


size_t InternalState::SectionHash::operator()(const ld::Section* sect) const
{
	size_t hash = 0;	
	ld::CStringHash temp;
	hash += temp.operator()(sect->segmentName());
	hash += temp.operator()(sect->sectionName());
	return hash;
}

bool InternalState::SectionEquals::operator()(const ld::Section* left, const ld::Section* right) const
{
	return (*left == *right);
}


InternalState::FinalSection::FinalSection(const ld::Section& sect, uint32_t sectionsSeen, const Options& opts)
	: ld::Internal::FinalSection(sect), 
	  _segmentOrder(segmentOrder(sect, opts)),
	  _sectionOrder(sectionOrder(sect, sectionsSeen, opts))
{
	//fprintf(stderr, "FinalSection(%16s, %16s) _segmentOrder=%3d, _sectionOrder=0x%08X\n",
	//		this->segmentName(), this->sectionName(), _segmentOrder, _sectionOrder);
}

const ld::Section& InternalState::FinalSection::outputSection(const ld::Section& sect, bool mergeZeroFill)
{
	// merge sections in final linked image
	switch ( sect.type() ) {
		case ld::Section::typeLiteral4:
		case ld::Section::typeLiteral8:
		case ld::Section::typeLiteral16:
			if ( strcmp(sect.segmentName(), "__TEXT") == 0 )
				return _s_TEXT_const;
			break;
		case ld::Section::typeUnclassified:
			if ( strcmp(sect.segmentName(), "__DATA") == 0 ) {
				if ( strcmp(sect.sectionName(), "__datacoal_nt") == 0 )
					return _s_DATA_data;
				if ( strcmp(sect.sectionName(), "__const_coal") == 0 )
					return _s_DATA_const;
			}
			else if ( strcmp(sect.segmentName(), "__TEXT") == 0 ) {
				if ( strcmp(sect.sectionName(), "__const_coal") == 0 )
					return _s_TEXT_const;
			}
			else if ( strcmp(sect.segmentName(), "__DATA_DIRTY") == 0 ) {
				if ( strcmp(sect.sectionName(), "__datacoal_nt") == 0 )
					return _s_DATA_DIRTY_data;
			}
			else if ( strcmp(sect.segmentName(), "__DATA_CONST") == 0 ) {
				if ( strcmp(sect.sectionName(), "__const_coal") == 0 )
					return _s_DATA_CONST_const;
			}
			break;
		case ld::Section::typeZeroFill:
			if ( mergeZeroFill )
				return _s_DATA_zerofill;
			break;
		case ld::Section::typeCode:
			if ( strcmp(sect.segmentName(), "__TEXT") == 0 ) {
				if ( strcmp(sect.sectionName(), "__textcoal_nt") == 0 )
					return _s_TEXT_text;
				else if ( strcmp(sect.sectionName(), "__StaticInit") == 0 )
					return _s_TEXT_text;
			}
			break;
		case ld::Section::typeNonLazyPointer:
			if ( strcmp(sect.segmentName(), "__DATA") == 0 ) {
				if ( strcmp(sect.sectionName(), "__nl_symbol_ptr") == 0 )
					return _s_DATA_nl_symbol_ptr;
			}
			else if ( strcmp(sect.segmentName(), "__IMPORT") == 0 ) {
				if ( strcmp(sect.sectionName(), "__pointers") == 0 )
					return _s_DATA_nl_symbol_ptr; 
			}
			break;
		case ld::Section::typeTentativeDefs:
			if ( (strcmp(sect.segmentName(), "__DATA") == 0) && (strcmp(sect.sectionName(), "__comm/tent") == 0) ) {
				if ( mergeZeroFill )
					return _s_DATA_zerofill;
				else
					return _s_DATA_common;
			}
			break;
			// FIX ME: more 
		default:
			break;
	}
	return sect;
}

const ld::Section& InternalState::FinalSection::objectOutputSection(const ld::Section& sect, const Options& options)
{
	// in -r mode the only section that ever changes is __tenative -> __common with -d option
	if ( (sect.type() == ld::Section::typeTentativeDefs) && options.makeTentativeDefinitionsReal())
		return _s_DATA_common;
	return sect;
}

uint32_t InternalState::FinalSection::segmentOrder(const ld::Section& sect, const Options& options)
{
	if ( options.outputKind() == Options::kPreload ) {
		if ( strcmp(sect.segmentName(), "__HEADER") == 0 ) 
			return 0;
		const std::vector<const char*>& order = options.segmentOrder();
		for (size_t i=0; i != order.size(); ++i) {
			if ( strcmp(sect.segmentName(), order[i]) == 0 ) 
				return i+1;
		}
		if ( strcmp(sect.segmentName(), "__TEXT") == 0 ) 
			return order.size()+1;
		if ( strcmp(sect.segmentName(), "__DATA") == 0 ) 
			return order.size()+2;
	}
	else if ( options.outputKind() == Options::kStaticExecutable ) {
		const std::vector<const char*>& order = options.segmentOrder();
		for (size_t i=0; i != order.size(); ++i) {
			if ( strcmp(sect.segmentName(), order[i]) == 0 )
				return i+1;
		}
		if ( strcmp(sect.segmentName(), "__PAGEZERO") == 0 )
			return 0;
		if ( strcmp(sect.segmentName(), "__TEXT") == 0 )
			return order.size()+1;
		if ( strcmp(sect.segmentName(), "__DATA") == 0 )
			return order.size()+2;
	}
	else {
		bool armCloseStubs = (options.architecture() == CPU_TYPE_ARM) && !options.sharedRegionEligible() && !options.makeEncryptable() && !options.makeChainedFixups();
		if ( strcmp(sect.segmentName(), "__PAGEZERO") == 0 ) 
			return 0;
		if ( strcmp(sect.segmentName(), "__TEXT") == 0 ) 
			return 1;
		if ( strcmp(sect.segmentName(), "__TEXT_EXEC") == 0 )
			return 2;
		if ( strcmp(sect.segmentName(), "__DATA_CONST") == 0 )
			return ((options.outputKind() == Options::kKextBundle) || armCloseStubs) ? 5 : 3;
		// in -r mode, want __DATA  last so zerofill sections are at end
		if ( strcmp(sect.segmentName(), "__DATA") == 0 )
			return (options.outputKind() == Options::kObjectFile) ? 7 : 4;
		if ( strcmp(sect.segmentName(), "__OBJC") == 0 ) 
			return 5;
		if ( strcmp(sect.segmentName(), "__IMPORT") == 0 )
			return 6;
	}
	// layout non-standard segments in order seen (+100 to shift beyond standard segments)
	for (uint32_t i=0; i < _s_segmentsSeen.size(); ++i) {
		if ( strcmp(_s_segmentsSeen[i], sect.segmentName()) == 0 )
			return i+100;
	}
	_s_segmentsSeen.push_back(sect.segmentName());
	return _s_segmentsSeen.size()-1+100;
}

uint32_t InternalState::FinalSection::sectionOrder(const ld::Section& sect, uint32_t sectionsSeen, const Options& options)
{
	if ( sect.type() == ld::Section::typeFirstSection )
		return 0;
	if ( sect.type() == ld::Section::typeMachHeader )
		return 1;
	if ( sect.type() == ld::Section::typeLastSection )
		return INT_MAX;
	const std::vector<const char*>* sectionList = options.sectionOrder(sect.segmentName());
	if ( ((options.outputKind() == Options::kPreload) || (options.outputKind() == Options::kDyld) || options.isKernel()) && (sectionList != NULL) ) {
		uint32_t count = 10;
		for (std::vector<const char*>::const_iterator it=sectionList->begin(); it != sectionList->end(); ++it, ++count) {
			if ( strcmp(*it, sect.sectionName()) == 0 ) 
				return count;
		}
	}
	if ( strcmp(sect.segmentName(), "__TEXT") == 0 ) {
		switch ( sect.type() ) {
			case ld::Section::typeCode:
				// <rdar://problem/8346444> make __text always be first "code" section
				if ( strcmp(sect.sectionName(), "__text") == 0 )
					return 10;
				else
					return 11;
			case ld::Section::typeStub:
				return 12;
			case ld::Section::typeStubHelper:
				return 13;
			case ld::Section::typeInitOffsets:
				return 14;
			case ld::Section::typeThreadStarts:
				return INT_MAX-8;
			case ld::Section::typeLSDA:
				return INT_MAX-7;
			case ld::Section::typeUnwindInfo:
				return INT_MAX-6;
			case ld::Section::typeCFI:
				return INT_MAX-5;
			case ld::Section::typeStubClose:
				return INT_MAX - 3;
			case ld::Section::typeNonStdCString:
				if ( (strcmp(sect.sectionName(), "__oslogstring") == 0) && options.makeEncryptable() )
					return INT_MAX-4;
				if ( options.sharedRegionEligible() ) {
					if ( (strcmp(sect.sectionName(), "__objc_classname") == 0) )
						return INT_MAX - 2;
					if ( (strcmp(sect.sectionName(), "__objc_methname") == 0) )
						return INT_MAX - 1;
					if ( (strcmp(sect.sectionName(), "__objc_methtype") == 0) )
						return INT_MAX;
				}
				return sectionsSeen+20;
			default:
				if ( (strcmp(sect.sectionName(), "__objc_methlist") == 0) )
					return 15;
				return sectionsSeen+20;
		}
	}
	else if ( strncmp(sect.segmentName(), "__DATA", 6) == 0 ) {
		switch ( sect.type() ) {
			case ld::Section::typeLazyPointerClose:
				return 8;
			case ld::Section::typeDyldInfo:
				return 9;
			case ld::Section::typeNonLazyPointer:
				return 10;
			case ld::Section::typeLazyPointer:
				return 11;
			case ld::Section::typeInitializerPointers:
				return 12;
			case ld::Section::typeTerminatorPointers:
				return 13;
			case ld::Section::typeTLVInitialValues:
				return INT_MAX-259; // need TLV zero-fill to follow TLV init values
			case ld::Section::typeTLVZeroFill:
				return INT_MAX-258;
			case ld::Section::typeZeroFill:
				// make sure __huge is always last zerofill section
				if ( strcmp(sect.sectionName(), "__huge") == 0 )
					return INT_MAX-1;
				else
					return INT_MAX-256+sectionsSeen; // <rdar://problem/25448494> zero fill need to be last and in "seen" order
			default:
				if ( strcmp(sect.sectionName(), "__auth_ptr") == 0 )
					return 11;
				// <rdar://problem/14348664> __DATA,__const section should be near __mod_init_func not __data
				if ( strcmp(sect.sectionName(), "__const") == 0 )
					return 14;
				// <rdar://problem/17125893> Linker should put __cfstring near __const
				if ( strcmp(sect.sectionName(), "__cfstring") == 0 )
					return 15;
				// <rdar://problem/7435296> Reorder sections to reduce page faults in object files
				else if ( strcmp(sect.sectionName(), "__objc_classlist") == 0 ) 
					return 20;
				else if ( strcmp(sect.sectionName(), "__objc_nlclslist") == 0 ) 
					return 21;
				else if ( strcmp(sect.sectionName(), "__objc_catlist") == 0 ) 
					return 22;
				else if ( strcmp(sect.sectionName(), "__objc_nlcatlist") == 0 ) 
					return 23;
				else if ( strcmp(sect.sectionName(), "__objc_protolist") == 0 ) 
					return 24;
				else if ( strcmp(sect.sectionName(), "__objc_imageinfo") == 0 ) 
					return 25;
				else if ( strcmp(sect.sectionName(), "__objc_const") == 0 ) 
					return 26;
				else if ( strcmp(sect.sectionName(), "__objc_selrefs") == 0 ) 
					return 27;
				else if ( strcmp(sect.sectionName(), "__objc_msgrefs") == 0 ) 
					return 28;
				else if ( strcmp(sect.sectionName(), "__objc_protorefs") == 0 ) 
					return 29;
				else if ( strcmp(sect.sectionName(), "__objc_classrefs") == 0 ) 
					return 30;
				else if ( strcmp(sect.sectionName(), "__objc_superrefs") == 0 ) 
					return 31;
				else if ( strcmp(sect.sectionName(), "__objc_ivar") == 0 ) 
					return 32;
				else if ( strcmp(sect.sectionName(), "__objc_data") == 0 ) 
					return 33;
				else if ( strcmp(sect.sectionName(), "__objc_const_ax") == 0 )
					return 34;
				else
					return sectionsSeen+40;
		}
	}
	else if ( strcmp(sect.segmentName(), "__OBJC_CONST") == 0 ) {
		// First emit the sections we want the shared cache builder to keep in order
		if ( strcmp(sect.sectionName(), "__objc_class_ro") == 0 )
			return 10;
		if ( strcmp(sect.sectionName(), "__cfstring") == 0 )
			return 11;
		return sectionsSeen+10;
	}
	// make sure zerofill in any other section is at end of segment
	if ( sect.type() == ld::Section::typeZeroFill )
		return INT_MAX-256+sectionsSeen;
	return sectionsSeen+20;
}

#ifndef NDEBUG
static void validateFixups(const ld::Atom& atom)
{
	//fprintf(stderr, "validateFixups %s\n", atom.name());
	bool lastWasClusterEnd = true;
	ld::Fixup::Cluster lastClusterSize = ld::Fixup::k1of1;
	uint32_t curClusterOffsetInAtom = 0;
	for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
		//fprintf(stderr, "  fixup offset=%d, cluster=%d\n", fit->offsetInAtom, fit->clusterSize);
		assert((fit->offsetInAtom <= atom.size()) || (fit->offsetInAtom == 0));
		if ( fit->firstInCluster() ) {
			assert(lastWasClusterEnd);
			curClusterOffsetInAtom = fit->offsetInAtom;
			lastWasClusterEnd = (fit->clusterSize == ld::Fixup::k1of1);
		}
		else {
			assert(!lastWasClusterEnd);
			assert(fit->offsetInAtom == curClusterOffsetInAtom);
			switch ((ld::Fixup::Cluster)fit->clusterSize) {
				case ld::Fixup::k1of1:
				case ld::Fixup::k1of2:
				case ld::Fixup::k1of3:
				case ld::Fixup::k1of4:
				case ld::Fixup::k1of5:
					lastWasClusterEnd = false;
					break;
				case ld::Fixup::k2of2:
					assert(lastClusterSize = ld::Fixup::k1of2);
					lastWasClusterEnd = true;
					break;
				case ld::Fixup::k2of3:
					assert(lastClusterSize = ld::Fixup::k1of3);
					lastWasClusterEnd = false;
					break;
				case ld::Fixup::k2of4:
					assert(lastClusterSize = ld::Fixup::k1of4);
					lastWasClusterEnd = false;
					break;
				case ld::Fixup::k2of5:
					assert(lastClusterSize = ld::Fixup::k1of5);
					lastWasClusterEnd = false;
					break;
				case ld::Fixup::k3of3:
					assert(lastClusterSize = ld::Fixup::k2of3);
					lastWasClusterEnd = true;
					break;
				case ld::Fixup::k3of4:
					assert(lastClusterSize = ld::Fixup::k2of4);
					lastWasClusterEnd = false;
					break;
				case ld::Fixup::k3of5:
					assert(lastClusterSize = ld::Fixup::k2of5);
					lastWasClusterEnd = false;
					break;
				case ld::Fixup::k4of4:
					assert(lastClusterSize = ld::Fixup::k3of4);
					lastWasClusterEnd = true;
					break;
				case ld::Fixup::k4of5:
					assert(lastClusterSize = ld::Fixup::k3of5);
					lastWasClusterEnd = false;
					break;
				case ld::Fixup::k5of5:
					assert(lastClusterSize = ld::Fixup::k4of5);
					lastWasClusterEnd = true;
					break;
			}
		}
		lastClusterSize = fit->clusterSize;
		if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
			assert(fit->u.target != NULL);
		}
	}
	switch (lastClusterSize) {
		case ld::Fixup::k1of1:
		case ld::Fixup::k2of2:
		case ld::Fixup::k3of3:
		case ld::Fixup::k4of4:
		case ld::Fixup::k5of5:
			break;
		default:
			assert(0 && "last fixup was not end of cluster");
			break;
	}
}
#endif

bool InternalState::hasReferenceToWeakExternal(const ld::Atom& atom)
{
	// if __DATA,__const atom has pointer to weak external symbol, don't move to __DATA_CONST
	const ld::Atom* target = NULL;
	for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
		if ( fit->firstInCluster() ) {
			target = NULL;
		}
		switch ( fit->binding ) {
			case ld::Fixup::bindingNone:
			case ld::Fixup::bindingByNameUnbound:
				break;
			case ld::Fixup::bindingByContentBound:
			case ld::Fixup::bindingDirectlyBound:
				target = fit->u.target;
				break;
			case ld::Fixup::bindingsIndirectlyBound:
				target = indirectBindingTable[fit->u.bindingIndex];
				break;
		}
		if ( (target != NULL) && (target->definition() == ld::Atom::definitionRegular)
			&& (target->combine() == ld::Atom::combineByName) && (target->scope() == ld::Atom::scopeGlobal) ) {
			return true;
		}
	}
	return false;
}


// .o files without .subsections_via_symbols have all atoms in a section chained together with kindNoneFollowOn
// If any symbol in section is moved to another segment/section, all the atoms in that section need to be moved too.
// But we don't have a good way to find the start atom, so we just move all atoms started with the choosen one.
//
// For Swift classes, there are two alt_entry symbols in the class structure. If the main symbol or either alt_entry
// is moved, then all the atoms in that chain need to move. We find the start atom via the kindNoneGroupSubordinate.
bool InternalState::inMoveRWChain(const ld::Atom& atom, const char* filePath, bool followedBackBranch, const char*& dstSeg, bool& wildCardMatch)
{
	if ( !_options.hasDataSymbolMoves() )
		return false;

	auto pos = _pendingSegMove.find(&atom);
	if ( pos != _pendingSegMove.end() ) {
		dstSeg = pos->second;
		return true;
	}
	
	bool result = false;
	if ( _options.moveRwSymbol(atom.getUserVisibleName(), filePath, dstSeg, wildCardMatch) )
		result = true;

	for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
		if ( (fit->kind == ld::Fixup::kindNoneFollowOn) && (fit->binding == ld::Fixup::bindingDirectlyBound) ) {
			if ( inMoveRWChain(*(fit->u.target), filePath, followedBackBranch, dstSeg, wildCardMatch) )
				result = true;
		}
		else if ( !followedBackBranch && (fit->kind == ld::Fixup::kindNoneGroupSubordinate) && (fit->binding == ld::Fixup::bindingDirectlyBound) ) {
			// don't recurse forever.  Only recurse if we have not already followed a backbranch (kindNoneGroupSubordinate)
			if ( inMoveRWChain(*(fit->u.target), filePath, true, dstSeg, wildCardMatch) )
				result = true;
		}
	}

	if ( result ) {
		for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
			if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
				if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
					_pendingSegMove[fit->u.target] = dstSeg;
				}
			}
		}
	}

	return result;
}


bool InternalState::inMoveROChain(const ld::Atom& atom, const char* filePath, const char*& dstSeg, bool& wildCardMatch)
{
	if ( !_options.hasCodeSymbolMoves() )
		return false;

	auto pos = _pendingSegMove.find(&atom);
	if ( pos != _pendingSegMove.end() ) {
		dstSeg = pos->second;
		return true;
	}

	bool result = false;
	if ( _options.moveRoSymbol(atom.getUserVisibleName(), filePath, dstSeg, wildCardMatch) )
		result = true;

	for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
		if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
			if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
				if ( inMoveROChain(*(fit->u.target), filePath, dstSeg, wildCardMatch) )
					result = true;
			}
		}
	}

	if ( result ) {
		for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
			if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
				if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
					_pendingSegMove[fit->u.target] = dstSeg;
				}
			}
		}
	}

	return result;
}


// .o files without .subsections_via_symbols have all atoms in a section chained together with kindNoneFollowOn
// If any symbol in section is moved to another segment/section, all the atoms in that section need to be moved too.
// But we don't have a good way to find the start atom, so we just move all atoms started with the choosen one.
//
// For Swift classes, there are two alt_entry symbols in the class structure. If the main symbol or either alt_entry
// is moved, then all the atoms in that chain need to move. We find the start atom via the kindNoneGroupSubordinate.
#if SUPPORT_ARCH_arm64e
bool InternalState::inMoveAuthChain(const ld::Atom& atom, bool followedBackBranch, const char*& dstSeg)
{
	if ( !_options.useAuthDataSegment() )
		return false;

	auto pos = _pendingSegMove.find(&atom);
	if ( pos != _pendingSegMove.end() ) {
		dstSeg = pos->second;
		return true;
	}

	auto hasAuthenticatedFixups = [](const ld::Atom& atom) -> bool {
		for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
			switch (fit->kind) {
				case ld::Fixup::kindStoreLittleEndianAuth64:
				case ld::Fixup::kindStoreTargetAddressLittleEndianAuth64:
				case ld::Fixup::kindSetAuthData:
					return true;
				default:
					break;
			}
		}
		return false;
	};

	bool result = hasAuthenticatedFixups(atom);

	for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
		if ( (fit->kind == ld::Fixup::kindNoneFollowOn) && (fit->binding == ld::Fixup::bindingDirectlyBound) ) {
			if ( inMoveAuthChain(*(fit->u.target), followedBackBranch, dstSeg) )
				result = true;
		}
		else if ( !followedBackBranch && (fit->kind == ld::Fixup::kindNoneGroupSubordinate) && (fit->binding == ld::Fixup::bindingDirectlyBound) ) {
			// don't recurse forever.  Only recurse if we have not already followed a backbranch (kindNoneGroupSubordinate)
			if ( inMoveAuthChain(*(fit->u.target), true, dstSeg) )
				result = true;
		}
	}

	if ( result ) {
		for (ld::Fixup::iterator fit=atom.fixupsBegin(); fit != atom.fixupsEnd(); ++fit) {
			if ( fit->kind == ld::Fixup::kindNoneFollowOn ) {
				if ( fit->binding == ld::Fixup::bindingDirectlyBound ) {
					_pendingSegMove[fit->u.target] = dstSeg;
				}
			}
		}
	}

	return result;
}
#endif




ld::Internal::FinalSection* InternalState::addAtom(const ld::Atom& atom)
{
	//fprintf(stderr, "addAtom: %s\n", atom.name());
	ld::Internal::FinalSection* fs = NULL;
	const char* curSectName = atom.section().sectionName();
	const char* curSegName = atom.section().segmentName();
	ld::Section::Type sectType = atom.section().type();
	const ld::File* f = atom.file();
	const char* path = (f != NULL) ? f->path() : NULL;
	if ( atom.section().type() == ld::Section::typeTentativeDefs ) {
		// tentative definitions don't have a real section name yet
		sectType = ld::Section::typeZeroFill;
		if ( _options.mergeZeroFill() )
			curSectName = FinalSection::_s_DATA_zerofill.sectionName();
		else
			curSectName = FinalSection::_s_DATA_common.sectionName();
	}

	// Support for -move_to_r._segment
	if ( atom.symbolTableInclusion() == ld::Atom::symbolTableIn ) {
		const char* dstSeg;
		bool wildCardMatch;
		if ( inMoveRWChain(atom, path, false, dstSeg, wildCardMatch) ) {
			if ( (sectType != ld::Section::typeZeroFill) 
			  && (sectType != ld::Section::typeUnclassified) 
			  && (sectType != ld::Section::typeTentativeDefs)
			  && (sectType != ld::Section::typeTLVDefs)
			  && (sectType != ld::Section::typeDyldInfo) ) {
				if ( !wildCardMatch )
					warning("cannot move symbol '%s' from file %s to segment '%s' because symbol is not data (is %d)", atom.name(), path, dstSeg, sectType);
			}
			else {
				curSegName = dstSeg;
				if ( _options.traceSymbolLayout() )
					printf("symbol '%s', -move_to_rw_segment mapped it to %s/%s\n", atom.name(), curSegName, curSectName);
				fs = this->getFinalSection(curSegName, curSectName, sectType);
			}
		}
		else {
			const char* symName = atom.name();
			if ( strncmp(symName, "__OBJC_$_INSTANCE_METHODS_", 26) == 0 ) {
				if ( _options.moveAXMethodList(&symName[26]) ) {
					curSectName  = "__objc_const_ax";
					fs = this->getFinalSection(curSegName, curSectName, sectType);
					if ( _options.traceSymbolLayout() )
						printf("symbol '%s', .axsymbol mapped it to %s/%s\n", atom.name(), curSegName, curSectName);
				}
			}
			else if ( strncmp(symName, "__OBJC_$_CLASS_METHODS_", 23) == 0 ) {
				if ( _options.moveAXMethodList(&symName[23]) ) {
					curSectName  = "__objc_const_ax";
					fs = this->getFinalSection(curSegName, curSectName, sectType);
					if ( _options.traceSymbolLayout() )
						printf("symbol '%s', .axsymbol mapped it to %s/%s\n", atom.name(), curSegName, curSectName);
				}
			}
#if SUPPORT_ARCH_arm64e
			else if ( (strncmp(symName, "__OBJC_CLASS_RO_", 16) == 0) || (strncmp(symName, "__OBJC_METACLASS_RO_", 20) == 0) ) {
				// The shared cache knows how to strip authenticated pointers from these atoms.
				// Move them to __OBJC_CONST to make it easier to optimize them.
				// Note the magic 72 here is the size of an objc class_ro_t.
				// Swift has a larger class_ro_t which we don't know if we can optimize
				if ( (atom.size() == 72) && _options.supportsAuthenticatedPointers() && _options.sharedRegionEligible() ) {
					curSegName = "__OBJC_CONST";
					curSectName  = "__objc_class_ro";
					fs = this->getFinalSection(curSegName, curSectName, sectType);
					if ( _options.traceSymbolLayout() )
						printf("symbol '%s', class_ro_t mapped it to %s/%s\n", atom.name(), curSegName, curSectName);
				}
			}
#endif
		}
		if ( (fs == NULL) && inMoveROChain(atom, path, dstSeg, wildCardMatch) ) {
			if ( (sectType != ld::Section::typeCode)
			  && (sectType != ld::Section::typeUnclassified) ) {
				if ( !wildCardMatch )
					warning("cannot move symbol '%s' from file %s to segment '%s' because symbol is not code (is %d)", atom.name(), path, dstSeg, sectType);
			}
			else {
				curSegName = dstSeg;
				if ( _options.traceSymbolLayout() )
					printf("symbol '%s', -move_to_ro_segment mapped it to %s/%s\n", atom.name(), curSegName, curSectName);
				fs = this->getFinalSection(curSegName, curSectName, ld::Section::typeCode);
			}
		}
	}
	else if ( atom.symbolTableInclusion() == ld::Atom::symbolTableNotInFinalLinkedImages ) {
		const char* symName = atom.name();
		if ( strncmp(symName, "l_OBJC_$_INSTANCE_METHODS_", 26) == 0 ) {
			if ( _options.moveAXMethodList(&symName[26]) ) {
				curSectName  = "__objc_const_ax";
				fs = this->getFinalSection(curSegName, curSectName, sectType);
				if ( _options.traceSymbolLayout() )
					printf("symbol '%s', .axsymbol mapped it to %s/%s\n", atom.name(), curSegName, curSectName);
			}
		}
		else if ( strncmp(symName, "l_OBJC_$_CLASS_METHODS_", 23) == 0 ) {
			if ( _options.moveAXMethodList(&symName[23]) ) {
				curSectName  = "__objc_const_ax";
				fs = this->getFinalSection(curSegName, curSectName, sectType);
				if ( _options.traceSymbolLayout() )
					printf("symbol '%s', .axsymbol mapped it to %s/%s\n", atom.name(), curSegName, curSectName);
			}
		}
	}

	// support for -rename_section and -rename_segment
	for (const Options::SectionRename& rename : _options.sectionRenames()) {
		if ( (strcmp(curSectName, rename.fromSection) == 0) && (strcmp(curSegName, rename.fromSegment) == 0) ) {
			if ( _options.useDataConstSegment() && _options.sharedRegionEligible() && (strcmp(curSectName, "__const") == 0) && (strcmp(curSegName, "__DATA") == 0) && hasReferenceToWeakExternal(atom) ) {
				// if __DATA,__const atom has pointer to weak external symbol, don't move to __DATA_CONST
				curSectName = "__const_weak";

#if SUPPORT_ARCH_arm64e
				// We may want __AUTH, but double check there isn't a chain already
				// for this atom which will force it in a different segment
				curSegName = "__AUTH";
				if ( !inMoveAuthChain(atom, false, curSegName) )
					curSegName = "__DATA";
#endif

				fs = this->getFinalSection(curSegName, curSectName, sectType);
				if ( _options.traceSymbolLayout() )
					printf("symbol '%s', contains pointers to weak symbols, so mapped it to %s/__const_weak\n", atom.name(), curSegName);
			}
			else if ( _options.useDataConstSegment() && _options.sharedRegionEligible() && (sectType == ld::Section::typeNonLazyPointer) && hasReferenceToWeakExternal(atom) ) {
				// if __DATA,__nl_symbol_ptr atom has pointer to weak external symbol, don't move to __DATA_CONST
				curSectName = "__got_weak";

				curSegName = "__DATA";
#if SUPPORT_ARCH_arm64e
				// We may want __AUTH, but double check there isn't a chain already
				// for this atom which will force it in a different segment
				curSegName = "__AUTH";
				if ( !inMoveAuthChain(atom, false, curSegName) )
					curSegName = "__DATA";
#endif

				fs = this->getFinalSection(curSegName, curSectName, sectType);
				if ( _options.traceSymbolLayout() )
					printf("symbol '%s', contains pointers to weak symbols, so mapped it to %s/__got_weak\n", atom.name(), curSegName);
			}
			else {
				curSegName = rename.toSegment;
				curSectName = rename.toSection;

#if SUPPORT_ARCH_arm64e
				// Actually move to __AUTH_CONST if we are const and authenticated
				if ( !strcmp(curSegName, "__DATA_CONST") ) {
					// We may want __AUTH_CONST, but double check there isn't a chain already
					// for this atom which will force it in a different segment
					curSegName = "__AUTH_CONST";
					if ( !inMoveAuthChain(atom, false, curSegName) )
						curSegName = "__DATA_CONST";
				}
#endif

				fs = this->getFinalSection(curSegName, rename.toSection, sectType);
				if ( _options.traceSymbolLayout() )
					printf("symbol '%s', -rename_section mapped it to %s/%s\n", atom.name(), fs->segmentName(), fs->sectionName());
			}
		}
	}
	for (const Options::SegmentRename& rename : _options.segmentRenames()) {
		if ( strcmp(curSegName, rename.fromSegment) == 0 ) {
			if ( _options.traceSymbolLayout() )
				printf("symbol '%s', -rename_segment mapped it to %s/%s\n", atom.name(), rename.toSegment, curSectName);
			fs = this->getFinalSection(rename.toSegment, curSectName, sectType);
		}
	}

#if SUPPORT_ARCH_arm64e
	if ( fs == NULL ) {
		// Actually move to __AUTH if we are authenticated
		if ( !strcmp(curSegName, "__DATA") ) {
			// We may want __AUTH, but double check there isn't a chain already
			// for this atom which will force it in a different segment
			curSegName = "__AUTH";
			if ( inMoveAuthChain(atom, false, curSegName) ) {
				fs = this->getFinalSection(curSegName, curSectName, sectType);
				if ( _options.traceSymbolLayout() && (atom.symbolTableInclusion() == ld::Atom::symbolTableIn) )
					printf("symbol '%s', contains authenticated pointers, so mapped it to __AUTH/%s\n", atom.name(), fs->sectionName());
			}
		}
	}
#endif

	// if no override, use default location
	if ( fs == NULL ) {
		fs = this->getFinalSection(atom.section());
		if ( _options.traceSymbolLayout() && (atom.symbolTableInclusion() == ld::Atom::symbolTableIn) )
			printf("symbol '%s', use default mapping to %s/%s\n", atom.name(), fs->segmentName(), fs->sectionName());
	}

	//fprintf(stderr, "InternalState::doAtom(%p), name=%s, sect=%s, finalseg=%s\n", &atom, atom.name(), atom.section().sectionName(), fs->segmentName());
#ifndef NDEBUG
	validateFixups(atom);
#endif
	if ( _atomsOrderedInSections ) {
		// make sure this atom is placed before any trailing section$end$ atom
		if ( (fs->atoms.size() > 1) && (fs->atoms.back()->contentType() == ld::Atom::typeSectionEnd) ) {
			// last atom in section$end$ atom, insert before it
			const ld::Atom* endAtom = fs->atoms.back();
			fs->atoms.pop_back();
			fs->atoms.push_back(&atom);
			fs->atoms.push_back(endAtom);
		}
		else {
			// not end atom, just append new atom
			fs->atoms.push_back(&atom);
		}
	}
	else {
		// normal case
		fs->atoms.push_back(&atom);
	}
	this->atomToSection[&atom] = fs;
	return fs;
}



ld::Internal::FinalSection* InternalState::getFinalSection(const char* seg, const char* sect, ld::Section::Type type)
{	
	for (std::vector<ld::Internal::FinalSection*>::iterator it=sections.begin(); it != sections.end(); ++it) {
		if ( (strcmp((*it)->segmentName(),seg) == 0) && (strcmp((*it)->sectionName(),sect) == 0) )
			return *it;
	}
	return this->getFinalSection(*new ld::Section(seg, sect, type, false));
}

ld::Internal::FinalSection* InternalState::getFinalSection(const ld::Section& inputSection)
{	
	const ld::Section* baseForFinalSection = &inputSection;
	
	// see if input section already has a FinalSection
	SectionInToOut::iterator pos = _sectionInToFinalMap.find(&inputSection);
	if ( pos != _sectionInToFinalMap.end() ) {
		return pos->second;
	}

	// otherwise, create a new final section
	switch ( _options.outputKind() ) {
		case Options::kStaticExecutable:
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kDyld:
		case Options::kKextBundle:
		case Options::kPreload:
			{
				// coalesce some sections
				const ld::Section& outSect = FinalSection::outputSection(inputSection, _options.mergeZeroFill());
				pos = _sectionInToFinalMap.find(&outSect);
				if ( pos != _sectionInToFinalMap.end() ) {
					_sectionInToFinalMap[&inputSection] = pos->second;
					//fprintf(stderr, "_sectionInToFinalMap[%p] = %p\n", &inputSection, pos->second);
					return pos->second;
				}
				else if ( outSect != inputSection ) {
					// new output section created, but not in map
					baseForFinalSection = &outSect;
				}
			}
			break;
		case Options::kObjectFile:
			baseForFinalSection = &FinalSection::objectOutputSection(inputSection, _options);
			pos = _sectionInToFinalMap.find(baseForFinalSection);
			if ( pos != _sectionInToFinalMap.end() ) {
				_sectionInToFinalMap[&inputSection] = pos->second;
				//fprintf(stderr, "_sectionInToFinalMap[%p] = %p\n", &inputSection, pos->second);
				return pos->second;
			}
			break;
	}

	InternalState::FinalSection* result = new InternalState::FinalSection(*baseForFinalSection, 
																	_sectionInToFinalMap.size(), _options);
	_sectionInToFinalMap[baseForFinalSection] = result;
	//fprintf(stderr, "_sectionInToFinalMap[%p(%s)] = %p\n", baseForFinalSection, baseForFinalSection->sectionName(), result);
	sections.push_back(result);
	return result;
}


void InternalState::sortSections()
{
	//fprintf(stderr, "UNSORTED final sections:\n");
	//for (std::vector<ld::Internal::FinalSection*>::iterator it = sections.begin(); it != sections.end(); ++it) {
	//	fprintf(stderr, "final section %p %s/%s\n", (*it), (*it)->segmentName(), (*it)->sectionName());
	//}
	std::stable_sort(sections.begin(), sections.end(), [](const ld::Internal::FinalSection* l, const ld::Internal::FinalSection* r) {
		const FinalSection* left = (FinalSection*)l;
		const FinalSection* right = (FinalSection*)r;
		if ( left->_segmentOrder != right->_segmentOrder )
			return (left->_segmentOrder < right->_segmentOrder);
		return (left->_sectionOrder < right->_sectionOrder);
	});
	//fprintf(stderr, "SORTED final sections:\n");
	//for (std::vector<ld::Internal::FinalSection*>::iterator it = sections.begin(); it != sections.end(); ++it) {
	//	fprintf(stderr, "final section %p %s/%s\n", (*it), (*it)->segmentName(), (*it)->sectionName());
	//}
	assert((sections[0]->type() == ld::Section::typeMachHeader) 
		|| ((sections[0]->type() == ld::Section::typeFirstSection) && (sections[1]->type() == ld::Section::typeMachHeader))
		|| ((sections[0]->type() == ld::Section::typePageZero) && (sections[1]->type() == ld::Section::typeMachHeader))
		|| ((sections[0]->type() == ld::Section::typePageZero) && (sections[1]->type() == ld::Section::typeFirstSection) && (sections[2]->type() == ld::Section::typeMachHeader)) );
	
}


bool InternalState::hasZeroForFileOffset(const ld::Section* sect)
{
	switch ( sect->type() ) {
		case ld::Section::typeZeroFill:
		case ld::Section::typeTLVZeroFill:
			return _options.optimizeZeroFill();
		case ld::Section::typePageZero:
		case ld::Section::typeStack:
		case ld::Section::typeTentativeDefs:
			return true;
		default:
			break;
	}
	return false;
}

uint64_t InternalState::pageAlign(uint64_t addr)
{
	const uint64_t alignment = _options.segmentAlignment();
	return ((addr+alignment-1) & (-alignment)); 
}

uint64_t InternalState::pageAlign(uint64_t addr, uint64_t pageSize)
{
	return ((addr+pageSize-1) & (-pageSize)); 
}

void InternalState::setSectionSizesAndAlignments()
{
	for (std::vector<ld::Internal::FinalSection*>::iterator sit = sections.begin(); sit != sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect->type() == ld::Section::typeAbsoluteSymbols ) {
			// absolute symbols need their finalAddress() to their value
			for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
				const ld::Atom* atom = *ait;
				(const_cast<ld::Atom*>(atom))->setSectionOffset(atom->objectAddress());
			}
		}
		else {
			uint16_t maxAlignment = 0;
			uint64_t offset = 0;
			for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
				const ld::Atom* atom = *ait;
				bool pagePerAtom = false;
				uint32_t atomAlignmentPowerOf2 = atom->alignment().powerOf2;
				uint32_t atomModulus = atom->alignment().modulus;
				if ( _options.pageAlignDataAtoms() && ( strncmp(atom->section().segmentName(), "__DATA", 6) == 0) ) {
					// most objc sections cannot be padded
					bool contiguousObjCSection = ( strncmp(atom->section().sectionName(), "__objc_", 7) == 0 );
					if ( strcmp(atom->section().sectionName(), "__objc_const") == 0 )
						contiguousObjCSection = false;
					if ( strcmp(atom->section().sectionName(), "__objc_data") == 0 )
						contiguousObjCSection = false;
					switch ( atom->section().type() ) {
						case ld::Section::typeUnclassified:
						case ld::Section::typeTentativeDefs:
						case ld::Section::typeZeroFill:
							if ( contiguousObjCSection ) 
								break;
							pagePerAtom = true;
							if ( atomAlignmentPowerOf2 < 12 ) {
								atomAlignmentPowerOf2 = 12;
								atomModulus = 0;
							}
							break;
						default:
							break;
					}
				}
				if ( atomAlignmentPowerOf2 > maxAlignment )
					maxAlignment = atomAlignmentPowerOf2;
				// calculate section offset for this atom
				uint64_t alignment = 1 << atomAlignmentPowerOf2;
				uint64_t currentModulus = (offset % alignment);
				uint64_t requiredModulus = atomModulus;
				if ( currentModulus != requiredModulus ) {
					if ( requiredModulus > currentModulus )
						offset += requiredModulus-currentModulus;
					else
						offset += requiredModulus+alignment-currentModulus;
				}
				// LINKEDIT atoms are laid out later
				if ( sect->type() != ld::Section::typeLinkEdit ) {
					(const_cast<ld::Atom*>(atom))->setSectionOffset(offset);
					offset += atom->size();
					if ( pagePerAtom ) {
						offset = (offset + 4095) & (-4096); // round up to end of page
					}
				}
				if ( (atom->scope() == ld::Atom::scopeGlobal) 
					&& (atom->definition() == ld::Atom::definitionRegular) 
					&& (atom->combine() == ld::Atom::combineByName) 
					&& ((atom->symbolTableInclusion() == ld::Atom::symbolTableIn) 
					 || (atom->symbolTableInclusion() == ld::Atom::symbolTableInAndNeverStrip)) ) {
						this->hasWeakExternalSymbols = true;
						if ( _options.warnWeakExports()	) 
							warning("weak external symbol: %s", atom->name());
						else if ( _options.noWeakExports()	)
							throwf("weak external symbol: %s", atom->name());
				}
			}
			sect->size = offset;
			// section alignment is that of a contained atom with the greatest alignment
			sect->alignment = maxAlignment;
			// unless -sectalign command line option overrides
			if  ( _options.hasCustomSectionAlignment(sect->segmentName(), sect->sectionName()) )
				sect->alignment = _options.customSectionAlignment(sect->segmentName(), sect->sectionName());
			// each atom in __eh_frame has zero alignment to assure they pack together,
			// but compilers usually make the CFIs pointer sized, so we want whole section
			// to start on pointer sized boundary.
			if ( sect->type() == ld::Section::typeCFI )
				sect->alignment = 3;
			if ( sect->type() == ld::Section::typeTLVDefs )
				this->hasThreadLocalVariableDefinitions = true;
		}
	}

	// <rdar://problem/24221680> All __thread_data and __thread_bss sections must have same alignment
	uint8_t maxThreadAlign = 0;
	for (ld::Internal::FinalSection* sect : sections) {
		if ( (sect->type() == ld::Section::typeTLVInitialValues) || (sect->type() == ld::Section::typeTLVZeroFill) ) {
			if ( sect->alignment > maxThreadAlign )
				maxThreadAlign = sect->alignment;
		}
	}
	for (ld::Internal::FinalSection* sect : sections) {
		if ( (sect->type() == ld::Section::typeTLVInitialValues) || (sect->type() == ld::Section::typeTLVZeroFill) ) {
			sect->alignment = maxThreadAlign;
		}
	}

}

uint64_t InternalState::assignFileOffsets() 
{
  	const bool log = false;
	const bool hiddenSectionsOccupyAddressSpace = ((_options.outputKind() != Options::kObjectFile)
												&& (_options.outputKind() != Options::kPreload));
	const bool segmentsArePageAligned = (_options.outputKind() != Options::kObjectFile);

	uint64_t address = 0;
	const char* lastSegName = "";
	uint64_t floatingAddressStart = _options.baseAddress();
	bool haveFixedSegments = false;
	
	// mark all sections as not having an address yet
	for (std::vector<ld::Internal::FinalSection*>::iterator it = sections.begin(); it != sections.end(); ++it) {
		ld::Internal::FinalSection* sect = *it;
		sect->alignmentPaddingBytes = 0;
		sect->address = ULLONG_MAX;
	}

	// first pass, assign addresses to sections in segments with fixed start addresses
	if ( log ) fprintf(stderr, "Fixed address segments:\n");
	for (std::vector<ld::Internal::FinalSection*>::iterator it = sections.begin(); it != sections.end(); ++it) {
		ld::Internal::FinalSection* sect = *it;
		if ( ! _options.hasCustomSegmentAddress(sect->segmentName()) ) 
			continue;
		haveFixedSegments = true;
		if ( segmentsArePageAligned ) {
			if ( strcmp(lastSegName, sect->segmentName()) != 0 ) {
				address = _options.customSegmentAddress(sect->segmentName());
				lastSegName = sect->segmentName();
			}
		}
		// adjust section address based on alignment
		uint64_t unalignedAddress = address;
		uint64_t alignment = (1 << sect->alignment);
		address = ( (unalignedAddress+alignment-1) & (-alignment) );
	
		// update section info
		sect->address = address;
		sect->alignmentPaddingBytes = (address - unalignedAddress);
		
		// sanity check size
		if ( ((address + sect->size) > _options.maxAddress()) && (_options.outputKind() != Options::kObjectFile) 
															  && (_options.outputKind() != Options::kStaticExecutable) )
			throwf("section %s (address=0x%08llX, size=%llu) would make the output executable exceed available address range", 
						sect->sectionName(), address, sect->size);
		
		if ( log ) fprintf(stderr, "  address=0x%08llX, hidden=%d, alignment=%02d, section=%s,%s\n",
						sect->address, sect->isSectionHidden(), sect->alignment, sect->segmentName(), sect->sectionName());
		// update running totals
		if ( !sect->isSectionHidden() || hiddenSectionsOccupyAddressSpace )
			address += sect->size;
		
		// if TEXT segment address is fixed, then flow other segments after it
		if ( strcmp(sect->segmentName(), "__TEXT") == 0 ) {
			floatingAddressStart = address;
		}
	}

	// second pass, assign section addresses to sections in segments that are ordered after a segment with a fixed address
	if ( haveFixedSegments && !_options.segmentOrder().empty() ) {
		if ( log ) fprintf(stderr, "After Fixed address segments:\n");
		lastSegName = "";
		ld::Internal::FinalSection* lastSect = NULL; 
		for (std::vector<ld::Internal::FinalSection*>::iterator it = sections.begin(); it != sections.end(); ++it) {
			ld::Internal::FinalSection* sect = *it;
			if ( (sect->address == ULLONG_MAX) && _options.segmentOrderAfterFixedAddressSegment(sect->segmentName()) ) {
				address = lastSect->address + lastSect->size;
				if ( (strcmp(lastSegName, sect->segmentName()) != 0) && segmentsArePageAligned ) {
					// round up size of last segment
					address = pageAlign(address, _options.segPageSize(lastSegName));
				}
				// adjust section address based on alignment
				uint64_t unalignedAddress = address;
				uint64_t alignment = (1 << sect->alignment);
				address = ( (unalignedAddress+alignment-1) & (-alignment) );
				sect->alignmentPaddingBytes = (address - unalignedAddress);
				sect->address = address;
				if ( log ) fprintf(stderr, "  address=0x%08llX, hidden=%d, alignment=%02d, section=%s,%s\n",
									sect->address, sect->isSectionHidden(), sect->alignment, sect->segmentName(), sect->sectionName());
				// update running totals
				if ( !sect->isSectionHidden() || hiddenSectionsOccupyAddressSpace )
					address += sect->size;
			}
			lastSegName = sect->segmentName();
			lastSect = sect;
		}
	}

	// last pass, assign addresses to remaining sections
	address = floatingAddressStart;
	lastSegName = "";
	ld::Internal::FinalSection* overlappingFixedSection = NULL;
	ld::Internal::FinalSection* overlappingFlowSection = NULL;
	ld::Internal::FinalSection* prevSect = NULL;
	if ( log ) fprintf(stderr, "Regular layout segments:\n");
	for (std::vector<ld::Internal::FinalSection*>::iterator it = sections.begin(); it != sections.end(); ++it) {
		ld::Internal::FinalSection* sect = *it;
		if ( sect->address != ULLONG_MAX )
			continue;
		if ( (_options.outputKind() == Options::kPreload) && (sect->type() == ld::Section::typeMachHeader) ) {
			sect->alignmentPaddingBytes = 0;
			continue;
		}
		if ( segmentsArePageAligned ) {
			if ( strcmp(lastSegName, sect->segmentName()) != 0 ) {
				// round up size of last segment if needed
				if ( *lastSegName != '\0' ) {
					address = pageAlign(address, _options.segPageSize(lastSegName));
				}
				// set segment address based on end of last segment
				address = pageAlign(address);
				lastSegName = sect->segmentName();
			}
		}
		
		// adjust section address based on alignment
		uint64_t unalignedAddress = address;
		uint64_t alignment = (1 << sect->alignment);
		address = ( (unalignedAddress+alignment-1) & (-alignment) );
	
		// update section info
		sect->address = address;
		sect->alignmentPaddingBytes = (address - unalignedAddress);

		// <rdar://problem/21994854> if first section is more aligned than segment, move segment start up to match
		if ( (prevSect != NULL) && (prevSect->type() == ld::Section::typeFirstSection) && (strcmp(prevSect->segmentName(), sect->segmentName()) == 0) ) {
			assert(prevSect->size == 0);
			if ( prevSect->address != sect->address ) {
				prevSect->alignmentPaddingBytes += (sect->address - prevSect->address);
				prevSect->address = sect->address;
			}
		}

		// sanity check size
		if ( ((address + sect->size) > _options.maxAddress()) && (_options.outputKind() != Options::kObjectFile) 
															  && (_options.outputKind() != Options::kStaticExecutable) )
				throwf("section %s (address=0x%08llX, size=%llu) would make the output executable exceed available address range", 
						sect->sectionName(), address, sect->size);

		// sanity check it does not overlap a fixed address segment
		for (std::vector<ld::Internal::FinalSection*>::iterator sit = sections.begin(); sit != sections.end(); ++sit) {
			ld::Internal::FinalSection* otherSect = *sit;
			if ( ! _options.hasCustomSegmentAddress(otherSect->segmentName()) ) 
				continue;
			if ( otherSect->size == 0 )
				continue;
			if ( sect->size == 0 )
				continue;
			if ( sect->address > otherSect->address ) {
				if ( (otherSect->address+otherSect->size) > sect->address ) {
					overlappingFixedSection = otherSect;
					overlappingFlowSection = sect;
				}
			}
			else {
				if ( (sect->address+sect->size) > otherSect->address ) {
					overlappingFixedSection = otherSect;
					overlappingFlowSection = sect;
				}
			}
		}
		
		if ( log ) fprintf(stderr, "  address=0x%08llX, size=0x%08llX, hidden=%d, alignment=%02d, padBytes=%d, section=%s,%s\n",
							sect->address, sect->size, sect->isSectionHidden(), sect->alignment, sect->alignmentPaddingBytes, 
							sect->segmentName(), sect->sectionName());
		// update running totals
		if ( !sect->isSectionHidden() || hiddenSectionsOccupyAddressSpace )
			address += sect->size;
		prevSect = sect;
	}
	if ( overlappingFixedSection != NULL ) {
		fprintf(stderr, "Section layout:\n");
		for (std::vector<ld::Internal::FinalSection*>::iterator it = sections.begin(); it != sections.end(); ++it) {
			ld::Internal::FinalSection* sect = *it;
			//if ( sect->isSectionHidden() )
			//	continue;
			fprintf(stderr, "  address:0x%08llX, alignment:2^%d, size:0x%08llX, padBytes:%d, section:%s/%s\n",
							sect->address, sect->alignment, sect->size, sect->alignmentPaddingBytes, 
							sect->segmentName(), sect->sectionName());
	
		}
		throwf("Section (%s/%s) overlaps fixed address section (%s/%s)", 
			overlappingFlowSection->segmentName(), overlappingFlowSection->sectionName(),
			overlappingFixedSection->segmentName(), overlappingFixedSection->sectionName());
	}
	
	
	// third pass, assign section file offsets 
	uint64_t fileOffset = 0;
	lastSegName = "";
	if ( log ) fprintf(stderr, "All segments with file offsets:\n");
	for (std::vector<ld::Internal::FinalSection*>::iterator it = sections.begin(); it != sections.end(); ++it) {
		ld::Internal::FinalSection* sect = *it;
		if ( hasZeroForFileOffset(sect) ) {
			// fileoff of zerofill sections is moot, but historically it is set to zero
			sect->fileOffset = 0;

			// <rdar://problem/10445047> align file offset with address layout
			fileOffset += sect->alignmentPaddingBytes;
		}
		else {
			// page align file offset at start of each segment
			if ( segmentsArePageAligned && (*lastSegName != '\0') && (strcmp(lastSegName, sect->segmentName()) != 0) ) {
				fileOffset = pageAlign(fileOffset, _options.segPageSize(lastSegName));
			}
			lastSegName = sect->segmentName();

			// align file offset with address layout
			fileOffset += sect->alignmentPaddingBytes;
			
			// update section info
			sect->fileOffset = fileOffset;
			
			// update running total
			fileOffset += sect->size;
		}
		
		if ( log ) fprintf(stderr, "  fileoffset=0x%08llX, address=0x%08llX, hidden=%d, size=%lld, alignment=%02d, section=%s,%s\n",
				sect->fileOffset, sect->address, sect->isSectionHidden(), sect->size, sect->alignment, 
				sect->segmentName(), sect->sectionName());
	}

#if 0
	// for encrypted iPhoneOS apps
	if ( _options.makeEncryptable() ) { 
		// remember end of __TEXT for later use by load command
		for (std::vector<ld::Internal::FinalSection*>::iterator it = state.sections.begin(); it != state.sections.end(); ++it) {
			ld::Internal::FinalSection* sect = *it;
			if ( strcmp(sect->segmentName(), "__TEXT") == 0 ) {
				_encryptedTEXTendOffset = pageAlign(sect->fileOffset + sect->size);
			}
		}
	}
#endif

	// return total file size
	return fileOffset;
}

static char* commatize(uint64_t in, char* out)
{
	char* result = out;
	char rawNum[30];
	sprintf(rawNum, "%llu", in);
	const int rawNumLen = strlen(rawNum);
	for(int i=0; i < rawNumLen-1; ++i) {
		*out++ = rawNum[i];
		if ( ((rawNumLen-i) % 3) == 1 )
			*out++ = ',';
	}
	*out++ = rawNum[rawNumLen-1];
	*out = '\0';
	return result;
}

static void printTime(const char* msg, uint64_t partTime, uint64_t totalTime)
{
	static uint64_t sUnitsPerSecond = 0;
	if ( sUnitsPerSecond == 0 ) {
		struct mach_timebase_info timeBaseInfo;
		if ( mach_timebase_info(&timeBaseInfo) != KERN_SUCCESS )
      return;
    sUnitsPerSecond = 1000000000ULL * timeBaseInfo.denom / timeBaseInfo.numer;
	}
	if ( partTime < sUnitsPerSecond ) {
		uint32_t milliSecondsTimeTen = (partTime*10000)/sUnitsPerSecond;
		uint32_t milliSeconds = milliSecondsTimeTen/10;
		uint32_t percentTimesTen = (partTime*1000)/totalTime;
		uint32_t percent = percentTimesTen/10;
		fprintf(stderr, "%24s: % 4d.%d milliseconds (% 4d.%d%%)\n", msg, milliSeconds, milliSecondsTimeTen-milliSeconds*10, percent, percentTimesTen-percent*10);
	}
	else {
		uint32_t secondsTimeTen = (partTime*10)/sUnitsPerSecond;
		uint32_t seconds = secondsTimeTen/10;
		uint32_t percentTimesTen = (partTime*1000)/totalTime;
		uint32_t percent = percentTimesTen/10;
		fprintf(stderr, "%24s: % 4d.%d seconds (% 4d.%d%%)\n", msg, seconds, secondsTimeTen-seconds*10, percent, percentTimesTen-percent*10);
	}
}


static void getVMInfo(vm_statistics_data_t& info)
{
	mach_msg_type_number_t count = sizeof(vm_statistics_data_t) / sizeof(natural_t);
	kern_return_t error = host_statistics(mach_host_self(), HOST_VM_INFO,
							(host_info_t)&info, &count);
	if (error != KERN_SUCCESS) {
		bzero(&info, sizeof(vm_statistics_data_t));
	}
}




int main(int argc, const char* argv[])
{
	const char* archName = NULL;
	bool showArch = false;
	try {
		PerformanceStatistics statistics;
		statistics.startTool = mach_absolute_time();
		
		// create object to track command line arguments
		Options options(argc, argv);
		InternalState state(options);
		
		// allow libLTO to be overridden by command line -lto_library
		if (const char *dylib = options.overridePathlibLTO())
			lto::set_library(dylib);
		
		// gather vm stats
		if ( options.printStatistics() )
			getVMInfo(statistics.vmStart);

		// update strings for error messages
		showArch = options.printArchPrefix();
		archName = options.architectureName();
		
		// open and parse input files
		statistics.startInputFileProcessing = mach_absolute_time();
		ld::tool::InputFiles inputFiles(options);
		
		// load and resolve all references
		statistics.startResolver = mach_absolute_time();
		ld::tool::Resolver resolver(options, inputFiles, state);
		resolver.resolve();
        
		// add dylibs used
		statistics.startDylibs = mach_absolute_time();
		inputFiles.dylibs(state);
	
		// do initial section sorting so passes have rough idea of the layout
		state.sortSections();

		// run passes
		statistics.startPasses = mach_absolute_time();
		ld::passes::objc::doPass(options, state);
		ld::passes::stubs::doPass(options, state);
		ld::passes::inits::doPass(options, state);
		ld::passes::huge::doPass(options, state);
		ld::passes::got::doPass(options, state);
		//ld::passes::objc_constants::doPass(options, state);
		ld::passes::tlvp::doPass(options, state);
		ld::passes::dylibs::doPass(options, state);	// must be after stubs and GOT passes
		ld::passes::order::doPass(options, state);
		state.markAtomsOrdered();
		ld::passes::dedup::doPass(options, state);
		ld::passes::branch_shim::doPass(options, state);	// must be after stubs
		ld::passes::branch_island::doPass(options, state);	// must be after stubs and order pass
		ld::passes::dtrace::doPass(options, state);
		ld::passes::compact_unwind::doPass(options, state);  // must be after order pass
		ld::passes::bitcode_bundle::doPass(options, state);  // must be after dylib

		// Sort again so that we get the segments in order.
		state.sortSections();
		ld::passes::thread_starts::doPass(options, state);  // must be after dylib
		
		// sort final sections
		state.sortSections();

		options.writeDependencyInfo();

		// write output file
		statistics.startOutput = mach_absolute_time();
		ld::tool::OutputFile out(options, state);
		out.write(state);
		statistics.startDone = mach_absolute_time();
		
		// print statistics
		//mach_o::relocatable::printCounts();
		if ( options.printStatistics() ) {
			getVMInfo(statistics.vmEnd);
			uint64_t totalTime = statistics.startDone - statistics.startTool;
			printTime("ld total time", totalTime, totalTime);
			printTime(" option parsing time", statistics.startInputFileProcessing  -	statistics.startTool,				totalTime);
			printTime(" object file processing", statistics.startResolver			 -	statistics.startInputFileProcessing,totalTime);
			printTime(" resolve symbols", statistics.startDylibs				 -	statistics.startResolver,			totalTime);
			printTime(" build atom list", statistics.startPasses				 -	statistics.startDylibs,				totalTime);
			printTime(" passess", statistics.startOutput				 -	statistics.startPasses,				totalTime);
			printTime(" write output", statistics.startDone				 -	statistics.startOutput,				totalTime);
			fprintf(stderr, "pageins=%u, pageouts=%u, faults=%u\n", 
								statistics.vmEnd.pageins-statistics.vmStart.pageins,
								statistics.vmEnd.pageouts-statistics.vmStart.pageouts, 
								statistics.vmEnd.faults-statistics.vmStart.faults);
			char temp[40];
			fprintf(stderr, "processed %3u object files,  totaling %15s bytes\n", inputFiles._totalObjectLoaded, commatize(inputFiles._totalObjectSize, temp));
			fprintf(stderr, "processed %3u archive files, totaling %15s bytes\n", inputFiles._totalArchivesLoaded, commatize(inputFiles._totalArchiveSize, temp));
			fprintf(stderr, "processed %3u dylib files\n", inputFiles._totalDylibsLoaded);
			fprintf(stderr, "wrote output file            totaling %15s bytes\n", commatize(out.fileSize(), temp));
		}
		// <rdar://problem/6780050> Would like linker warning to be build error.
		if ( options.errorBecauseOfWarnings() ) {
			fprintf(stderr, "ld: fatal warning(s) induced error (-fatal_warnings)\n");
			return 1;
		}
		// <rdar://problem/61228255> need to flush stdout since we skipping some clean up in calling _exit()
		fflush(stdout);

		// <rdar://problem/55031993> don't run terminators until all we can guarantee all threads are stopped
		// <rdar://problem/56200095> don't run C++ destructors of stack objects to gain 5% linking perf win
		_exit(0);
	}
	catch (const char* msg) {
		if ( strstr(msg, "malformed") != NULL )
			fprintf(stderr, "ld: %s\n", msg);
		else if ( showArch && (strstr(msg, archName) == NULL) )
			fprintf(stderr, "ld: %s for architecture %s\n", msg, archName);
		else
			fprintf(stderr, "ld: %s\n", msg);
		// <rdar://50510752> exit but don't run termination routines
		_exit(1);
	}
}


#ifndef NDEBUG
// implement assert() function to print out a backtrace before aborting
void __assert_rtn(const char* func, const char* file, int line, const char* failedexpr)
{
    Snapshot *snapshot = Snapshot::globalSnapshot;
    
    snapshot->setSnapshotMode(Snapshot::SNAPSHOT_DEBUG);
    snapshot->createSnapshot();
	snapshot->recordAssertionMessage("Assertion failed: (%s), function %s, file %s, line %d.\n", failedexpr, func, file, line);

	void* callStack[128];
	int depth = ::backtrace(callStack, 128);
	char* buffer = (char*)malloc(1024);
	for(int i=0; i < depth-1; ++i) {
		Dl_info info;
		dladdr(callStack[i], &info);
		const char* symboName = info.dli_sname;
		if ( (symboName != NULL) && (strncmp(symboName, "_Z", 2) == 0) ) {
			size_t bufLen = 1024;
			int result;
			char* unmangled = abi::__cxa_demangle(symboName, buffer, &bufLen, &result);
			if ( unmangled != NULL )
				symboName = unmangled;
		}
		long offset = (uintptr_t)callStack[i] - (uintptr_t)info.dli_saddr;
		fprintf(stderr, "%d  %p  %s + %ld\n", i, callStack[i], symboName, offset);
		snapshot->recordAssertionMessage("%d  %p  %s + %ld\n", i, callStack[i], symboName, offset);
	}
    fprintf(stderr, "A linker snapshot was created at:\n\t%s\n", snapshot->rootDir());
	fprintf(stderr, "ld: Assertion failed: (%s), function %s, file %s, line %d.\n", failedexpr, func, file, line);
	_exit(1);
}
#endif


