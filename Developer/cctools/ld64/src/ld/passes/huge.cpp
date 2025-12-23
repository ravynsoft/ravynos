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

#include <vector>
#include <map>

#include "ld.hpp"
#include "huge.h"

namespace ld {
namespace passes {
namespace huge {

class NullAtom
{
public:
	bool operator()(const ld::Atom* atom) const {
		return (atom == NULL);
	}
};

class DataPadAtom : public ld::Atom {
public:
											DataPadAtom(ld::Internal& state)
											: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
														ld::Atom::scopeTranslationUnit, ld::Atom::typeUnclassified,
														symbolTableNotIn, false, false, false, ld::Atom::Alignment(3))
											{ state.addAtom(*this); }


	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return "padding"; }
	virtual uint64_t						size() const					{ return 8; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }

protected:
	virtual									~DataPadAtom() {}

	static ld::Section						_s_section;
};

ld::Section DataPadAtom::_s_section("__DATA", "__data", ld::Section::typeUnclassified);


void doPass(const Options& opts, ld::Internal& state)
{
	const bool log = false;

	// <rdar://problem/26015603> add __data section if __DATA segment was gutted by dirty data removal
	if ( (opts.outputKind() == Options::kDynamicLibrary) && opts.useDataConstSegment() && opts.hasDataSymbolMoves() ) {
		uint64_t dataAtomsSize = 0;
		bool foundSegmentDATA_DIRTY = false;
		for (ld::Internal::FinalSection* sect : state.sections) {
			if ( strcmp(sect->segmentName(), "__DATA") == 0 ) {
				for (const ld::Atom* atom : sect->atoms) {
					dataAtomsSize += atom->size();
				}
			}
			else if ( strcmp(sect->segmentName(), "__DATA_DIRTY") == 0 ) {
				foundSegmentDATA_DIRTY = true;
			}
		}
		if ( foundSegmentDATA_DIRTY && (dataAtomsSize == 0) ) {
			new DataPadAtom(state);
		}
	}

	// only make make __huge section in final linked images
	if ( opts.outputKind() == Options::kObjectFile )
		return;

	// only make make __huge section for x86_64
	if ( opts.architecture() != CPU_TYPE_X86_64 )
		return;

	// only needed if some (non-linkedit) atoms have an addresss >2GB from base address 
	state.usingHugeSections = false;
	uint64_t address = 0;
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect->type() == ld::Section::typePageZero )
			continue;
		if ( sect->type() == ld::Section::typeStack )
			continue;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			if ( (address > 0x7FFFFFFFLL) && !sect->isSectionHidden() ) {
				state.usingHugeSections = true;
				if (log) fprintf(stderr, "atom: %s is >2GB (0x%09llX), so enabling huge mode\n", atom->name(), address);
				break;
			}
			address += atom->size();
		}
		if ( state.usingHugeSections )
			break;
	}
	if ( !state.usingHugeSections )
		return;

	// move all zero fill atoms that >1MB in size to a new __huge section
	ld::Internal::FinalSection* hugeSection = state.getFinalSection(*new ld::Section("__DATA", "__huge", ld::Section::typeZeroFill));
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect == hugeSection )
			continue;
		if ( sect->type() == ld::Section::typeZeroFill ) {
			bool movedSome = false;
			for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
				const ld::Atom* atom = *ait;
				if ( atom->size() > 1024*1024 ) {
					hugeSection->atoms.push_back(atom);
					state.atomToSection[atom] = hugeSection;
					if (log) fprintf(stderr, "moved to __huge: %s, size=%llu\n", atom->name(), atom->size());
					*ait = NULL;  // change atom to NULL for later bulk removal
					movedSome = true;
				}
			}
			if ( movedSome ) 
				sect->atoms.erase(std::remove_if(sect->atoms.begin(), sect->atoms.end(), NullAtom()), sect->atoms.end());
		}
	}

	
}


} // namespace huge
} // namespace passes 
} // namespace ld 
