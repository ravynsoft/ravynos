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
#endif
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <dlfcn.h>
#include <libkern/OSByteOrder.h>

#include <vector>
#include <map>

#include "MachOFileAbstraction.hpp"
#include "ld.hpp"
#include "branch_island.h"

namespace ld {
namespace passes {
namespace branch_island {


static std::map<const Atom*, uint64_t> sAtomToAddress;


struct TargetAndOffset { const ld::Atom* atom; uint32_t offset; };
class TargetAndOffsetComparor
{
public:
	bool operator()(const TargetAndOffset& left, const TargetAndOffset& right) const
	{
		if ( left.atom != right.atom )
			return ( left.atom < right.atom );
		return ( left.offset < right.offset );
	}
};


static bool _s_log = false;
static ld::Section _s_text_section("__TEXT", "__text", ld::Section::typeCode);


#if SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64_32

class ARM64BranchIslandAtom : public ld::Atom {
public:
											ARM64BranchIslandAtom(const char* nm, const ld::Atom* target, TargetAndOffset finalTarget)
				: ld::Atom(_s_text_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeBranchIsland, 
							ld::Atom::symbolTableIn, false, false, false, ld::Atom::Alignment(2)), 
				_name(nm),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Branch26, target),
				_fixup2(0, ld::Fixup::k1of1, ld::Fixup::kindIslandTarget, finalTarget.atom) {
					if (_s_log) fprintf(stderr, "%p: ARM64 branch island to final target %s\n",
										this, finalTarget.atom->name());
				}

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(buffer, 0, 0x14000000);
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const char*								_name;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
};
#endif


class ARMtoARMBranchIslandAtom : public ld::Atom {
public:
											ARMtoARMBranchIslandAtom(const char* nm, const ld::Atom* target, TargetAndOffset finalTarget)
				: ld::Atom(_s_text_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeBranchIsland, 
							ld::Atom::symbolTableIn, false, false, false, ld::Atom::Alignment(2)), 
				_name(nm),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARMBranch24, target),
				_fixup2(0, ld::Fixup::k1of1, ld::Fixup::kindIslandTarget, finalTarget.atom) {
					if (_s_log) fprintf(stderr, "%p: ARM-to-ARM branch island to final target %s\n", 
										this, finalTarget.atom->name());
				}

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(buffer, 0, 0xEA000000);
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const char*								_name;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
};



class ARMtoThumb1BranchIslandAtom : public ld::Atom {
public:
											ARMtoThumb1BranchIslandAtom(const char* nm, const ld::Atom* target, TargetAndOffset finalTarget)
				: ld::Atom(_s_text_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeBranchIsland, 
							ld::Atom::symbolTableIn, false, false, false, ld::Atom::Alignment(2)), 
				_name(nm),
				_finalTarget(finalTarget) {
					if (_s_log) fprintf(stderr, "%p: ARM-to-thumb1 branch island to final target %s\n", 
										this, finalTarget.atom->name());
				}

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 16; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		// There is no large displacement thumb1 branch instruction.
		// Instead use ARM instructions that can jump to thumb.
		// we use a 32-bit displacement, so we can directly jump to target which means no island hopping
		int64_t displacement = _finalTarget.atom->finalAddress() + _finalTarget.offset - (this->finalAddress() + 12);
		if ( _finalTarget.atom->isThumb() )
			displacement |= 1;
		OSWriteLittleInt32(&buffer[ 0], 0, 0xe59fc004);	// 	ldr  ip, pc + 4
		OSWriteLittleInt32(&buffer[ 4], 0, 0xe08fc00c);	// 	add	 ip, pc, ip
		OSWriteLittleInt32(&buffer[ 8], 0, 0xe12fff1c);	// 	bx	 ip
		OSWriteLittleInt32(&buffer[12], 0, displacement);	// 	.long target-this		
	}
	virtual void							setScope(Scope)					{ }

private:
	const char*								_name;
	TargetAndOffset							_finalTarget;
};



class Thumb2toThumbBranchIslandAtom : public ld::Atom {
public:
											Thumb2toThumbBranchIslandAtom(const char* nm, const ld::Atom* target, TargetAndOffset finalTarget)
				: ld::Atom(_s_text_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeBranchIsland, 
							ld::Atom::symbolTableIn, false, true, false, ld::Atom::Alignment(1)), 
				_name(nm),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressThumbBranch22, target),
				_fixup2(0, ld::Fixup::k1of1, ld::Fixup::kindIslandTarget, finalTarget.atom)		{ 
					if (_s_log) fprintf(stderr, "%p: Thumb-to-thumb branch island to final target %s\n", 
										this, finalTarget.atom->name());
					}

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(buffer, 0, 0xf0008000);
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const char*								_name;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
};



class Thumb2toThumbBranchAbsoluteIslandAtom : public ld::Atom {
public:
											Thumb2toThumbBranchAbsoluteIslandAtom(const char* nm, const ld::Section& inSect, TargetAndOffset finalTarget)
				: ld::Atom(inSect, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeBranchIsland, 
							ld::Atom::symbolTableIn, false, true, false, ld::Atom::Alignment(1)), 
				_name(nm),
				_fixup1(0, ld::Fixup::k1of2, ld::Fixup::kindSetTargetAddress, finalTarget.atom),
				_fixup2(0, ld::Fixup::k2of2, ld::Fixup::kindStoreThumbLow16),
				_fixup3(4, ld::Fixup::k1of2, ld::Fixup::kindSetTargetAddress, finalTarget.atom),
				_fixup4(4, ld::Fixup::k2of2, ld::Fixup::kindStoreThumbHigh16),
				_fixup5(0, ld::Fixup::k1of1, ld::Fixup::kindIslandTarget, finalTarget.atom) {
					if (_s_log) fprintf(stderr, "%p: Thumb-to-thumb absolute branch island to final target %s\n", 
										this, finalTarget.atom->name());
					}

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 10; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[0], 0, 0x0c00f240);	// 	movw	r12, #0x5678
		OSWriteLittleInt32(&buffer[4], 0, 0x0c00f2c0);	// 	movt	r12, #0x1234
		OSWriteLittleInt16(&buffer[8], 0, 0x4760);		// 	bx		r12
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup5)[1]; }

private:
	const char*								_name;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
	ld::Fixup								_fixup5;
};



class NoPicARMtoThumbMBranchIslandAtom : public ld::Atom {
public:
											NoPicARMtoThumbMBranchIslandAtom(const char* nm, const ld::Atom* target, TargetAndOffset finalTarget)
				: ld::Atom(_s_text_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeBranchIsland, 
							ld::Atom::symbolTableIn, false, false, false, ld::Atom::Alignment(2)), 
				_name(nm),
				_finalTarget(finalTarget) {
					if (_s_log) fprintf(stderr, "%p: NoPIC ARM-to-Thumb branch island to final target %s\n", 
										this, finalTarget.atom->name());
					}

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 8; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		// There is no large displacement thumb1 branch instruction.
		// Instead use ARM instructions that can jump to thumb.
		// we use a 32-bit displacement, so we can directly jump to final target which means no island hopping
		uint32_t targetAddr = _finalTarget.atom->finalAddress();
		if ( _finalTarget.atom->isThumb() )
			targetAddr |= 1;
		OSWriteLittleInt32(&buffer[0], 0, 0xe51ff004);	// 	ldr	pc, [pc, #-4]
		OSWriteLittleInt32(&buffer[4], 0, targetAddr);	// 	.long target-this		
	}
	virtual void							setScope(Scope)					{ }

private:
	const char*								_name;
	TargetAndOffset							_finalTarget;
};


static ld::Atom* makeBranchIsland(const Options& opts, ld::Fixup::Kind kind, int islandRegion, const ld::Atom* nextTarget, 
									TargetAndOffset finalTarget, const ld::Section& inSect, bool crossSectionBranch)
{
	char* name;
	if ( finalTarget.offset == 0 ) {
		if ( islandRegion == 0 )
			asprintf(&name, "%s.island", finalTarget.atom->name());
		else
			asprintf(&name, "%s.island.%d", finalTarget.atom->name(), islandRegion+1);
	}
	else {
		asprintf(&name, "%s_plus_%d.island.%d", finalTarget.atom->name(), finalTarget.offset, islandRegion);
	}

	switch ( kind ) {
		case ld::Fixup::kindStoreARMBranch24:
		case ld::Fixup::kindStoreThumbBranch22:
		case ld::Fixup::kindStoreTargetAddressARMBranch24:
		case ld::Fixup::kindStoreTargetAddressThumbBranch22:
			if ( crossSectionBranch && opts.preferSubArchitecture() && opts.archSupportsThumb2() ) {
				return new Thumb2toThumbBranchAbsoluteIslandAtom(name, inSect, finalTarget);
			}
			else if ( finalTarget.atom->isThumb() ) {
				if ( opts.preferSubArchitecture() && opts.archSupportsThumb2() ) {
					return new Thumb2toThumbBranchIslandAtom(name, nextTarget, finalTarget);
				}
				else if ( opts.outputSlidable() ) {
					return new ARMtoThumb1BranchIslandAtom(name, nextTarget, finalTarget);
				}
				else {
					return new NoPicARMtoThumbMBranchIslandAtom(name, nextTarget, finalTarget);
				}
			}
			else {
				return new ARMtoARMBranchIslandAtom(name, nextTarget, finalTarget);
			}
			break;
#if SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64_32
	    case ld::Fixup::kindStoreARM64Branch26:
		case ld::Fixup::kindStoreTargetAddressARM64Branch26:
			return new ARM64BranchIslandAtom(name, nextTarget, finalTarget);
			break;
#endif
		default:
			assert(0 && "unexpected branch kind");
			break;
	}
	return NULL;
}


static uint64_t textSizeWhenMightNeedBranchIslands(const Options& opts, bool seenThumbBranch)
{
	switch ( opts.architecture() ) {
		case CPU_TYPE_ARM:
			if ( ! seenThumbBranch )
				return 32000000;  // ARM can branch +/- 32MB
			else if ( opts.preferSubArchitecture() && opts.archSupportsThumb2() ) 
				return 16000000;  // thumb2 can branch +/- 16MB
			else
				return  4000000;  // thumb1 can branch +/- 4MB
			break;
#if SUPPORT_ARCH_arm64
		case CPU_TYPE_ARM64:
			return 128000000; // arm64 can branch +/- 128MB
			break;
#endif
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
			return 128000000; // arm64_32 can branch +/- 128MB
			break;
#endif
	}
	assert(0 && "unexpected architecture");
	return 0x100000000LL;
}


static uint64_t maxDistanceBetweenIslands(const Options& opts, bool seenThumbBranch)
{
	switch ( opts.architecture() ) {
		case CPU_TYPE_ARM:
			if ( ! seenThumbBranch )
				return 30*1024*1024;	// 2MB of branch islands per 32MB
			else if ( opts.preferSubArchitecture() && opts.archSupportsThumb2() ) 
				return 14*1024*1024;	// 2MB of branch islands per 16MB
			else
				return 3500000;			// 0.5MB of branch islands per 4MB
			break;
#if SUPPORT_ARCH_arm64
		case CPU_TYPE_ARM64:
			return 124*1024*1024;		 // 4MB of branch islands per 128MB
			break;
#endif
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
			return 124*1024*1024;		 // 4MB of branch islands per 128MB
			break;
#endif
	}
	assert(0 && "unexpected architecture");
	return 0x100000000LL;
}


//
// PowerPC can do PC relative branches as far as +/-16MB.
// If a branch target is >16MB then we insert one or more
// "branch islands" between the branch and its target that
// allows island hopping to the target.
//
// Branch Island Algorithm
//
// If the __TEXT segment < 16MB, then no branch islands needed
// Otherwise, every 14MB into the __TEXT segment a region is
// added which can contain branch islands.  Every out-of-range
// bl instruction is checked.  If it crosses a region, an island
// is added to that region with the same target and the bl is
// adjusted to target the island instead.
//
// In theory, if too many islands are added to one region, it
// could grow the __TEXT enough that other previously in-range
// bl branches could be pushed out of range.  We reduce the
// probability this could happen by placing the ranges every
// 14MB which means the region would have to be 2MB (512,000 islands)
// before any branches could be pushed out of range.
//


static void makeIslandsForSection(const Options& opts, ld::Internal& state, ld::Internal::FinalSection* textSection, unsigned stubCount)
{
	// assign section offsets to each atom in __text section, watch for thumb branches, and find total size
	bool hasThumbBranches = false;
	bool haveCrossSectionBranches = false;
	const bool preload = (opts.outputKind() == Options::kPreload);
	uint64_t offset = 0;
	for (std::vector<const ld::Atom*>::iterator ait=textSection->atoms.begin();  ait != textSection->atoms.end(); ++ait) {
		const ld::Atom* atom = *ait;
		// check for thumb branches and cross section branches
		const ld::Atom* target = NULL;
		for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
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
					target = state.indirectBindingTable[fit->u.bindingIndex];
					break;
			}
			bool haveBranch = false;
			switch (fit->kind) {
				case ld::Fixup::kindStoreThumbBranch22:
				case ld::Fixup::kindStoreTargetAddressThumbBranch22:
					hasThumbBranches = true;
					// fall into arm branch case
				case ld::Fixup::kindStoreARMBranch24:
				case ld::Fixup::kindStoreTargetAddressARMBranch24:
					haveBranch = true;
					break;
                default:
                    break;   
			}
			if ( haveBranch && (target->contentType() != ld::Atom::typeStub) ) {
				// <rdar://problem/14792124> haveCrossSectionBranches only applies to -preload builds
				if ( preload && (atom->section() != target->section()) )
					haveCrossSectionBranches = true;
			}
		}
		// align atom
		ld::Atom::Alignment atomAlign = atom->alignment();
		uint64_t atomAlignP2 = (1 << atomAlign.powerOf2);
		uint64_t currentModulus = (offset % atomAlignP2);
		if ( currentModulus != atomAlign.modulus ) {
			if ( atomAlign.modulus > currentModulus )
				offset += atomAlign.modulus-currentModulus;
			else
				offset += atomAlign.modulus+atomAlignP2-currentModulus;		
		}
		(const_cast<ld::Atom*>(atom))->setSectionOffset(offset);
		offset += atom->size();
	}
	uint64_t totalTextSize = offset + stubCount*16;
	if ( (totalTextSize < textSizeWhenMightNeedBranchIslands(opts, hasThumbBranches)) && !haveCrossSectionBranches )
		return;
	if (_s_log) fprintf(stderr, "ld: section %s size=%llu, might need branch islands\n", textSection->sectionName(), totalTextSize);
	
	// Figure out how many regions of branch islands will be needed, and their locations.
	// Construct a vector containing the atoms after which branch islands will be inserted,
	// taking into account follow on fixups. No atom run without an island can exceed kBetweenRegions.
	const uint64_t kBetweenRegions = maxDistanceBetweenIslands(opts, hasThumbBranches); // place regions of islands every 14MB in __text section
	std::vector<const ld::Atom*> branchIslandInsertionPoints; // atoms in the atom list after which branch islands will be inserted
	uint64_t previousIslandEndAddr = 0;
	const ld::Atom *insertionPoint = NULL;
	branchIslandInsertionPoints.reserve(totalTextSize/kBetweenRegions*2);
	for (std::vector<const ld::Atom*>::iterator it=textSection->atoms.begin(); it != textSection->atoms.end(); it++) {
		const ld::Atom* atom = *it;
		// if we move past the next atom, will the run length exceed kBetweenRegions?
		if ( atom->sectionOffset() + atom->size() > previousIslandEndAddr + kBetweenRegions ) {
			// yes. Add the last known good location (atom) for inserting a branch island.
			if ( insertionPoint == NULL )
				throwf("Unable to insert branch island. No insertion point available.");
			branchIslandInsertionPoints.push_back(insertionPoint);
			previousIslandEndAddr = insertionPoint->sectionOffset()+insertionPoint->size();
			insertionPoint = NULL;
		}
		// Can we insert an island after this atom? If so then keep track of it.
		if ( !atom->hasFixupsOfKind(ld::Fixup::kindNoneFollowOn) )
			insertionPoint = atom;
	}
	// add one more island after the last atom if close to limit
	if ( (insertionPoint != NULL) && (insertionPoint->sectionOffset() + insertionPoint->size() > previousIslandEndAddr + (kBetweenRegions-0x100000)) )
		branchIslandInsertionPoints.push_back(insertionPoint);
	if ( haveCrossSectionBranches && branchIslandInsertionPoints.empty() ) {
		branchIslandInsertionPoints.push_back(textSection->atoms.back());
	}
	const int kIslandRegionsCount = branchIslandInsertionPoints.size();

	if (_s_log) fprintf(stderr, "ld: will use %u branch island regions\n", kIslandRegionsCount);
	typedef std::map<TargetAndOffset,const ld::Atom*, TargetAndOffsetComparor> AtomToIsland;
    AtomToIsland* regionsMap[kIslandRegionsCount];
	uint64_t regionAddresses[kIslandRegionsCount];
	std::vector<const ld::Atom*>* regionsIslands[kIslandRegionsCount];
	for(int i=0; i < kIslandRegionsCount; ++i) {
		regionsMap[i] = new AtomToIsland();
		regionsIslands[i] = new std::vector<const ld::Atom*>();
		regionAddresses[i] = branchIslandInsertionPoints[i]->sectionOffset() + branchIslandInsertionPoints[i]->size();
		if (_s_log) fprintf(stderr, "ld: branch islands will be inserted at 0x%08llX after %s\n", regionAddresses[i], branchIslandInsertionPoints[i]->name());
	}
	unsigned int islandCount = 0;
	
	// create islands for branches in __text that are out of range
	for (std::vector<const ld::Atom*>::iterator ait=textSection->atoms.begin(); ait != textSection->atoms.end(); ++ait) {
		const ld::Atom* atom = *ait;
		const ld::Atom* target = NULL;
		uint64_t addend = 0;
		ld::Fixup* fixupWithTarget = NULL;
		for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
			if ( fit->firstInCluster() ) {
				target = NULL;
				fixupWithTarget = NULL;
				addend = 0;
			}
			switch ( fit->binding ) {
				case ld::Fixup::bindingNone:
				case ld::Fixup::bindingByNameUnbound:
					break;
				case ld::Fixup::bindingByContentBound:
				case ld::Fixup::bindingDirectlyBound:
					target = fit->u.target;
					fixupWithTarget = fit;
					break;
				case ld::Fixup::bindingsIndirectlyBound:
					target = state.indirectBindingTable[fit->u.bindingIndex];
					fixupWithTarget = fit;
					break;
			}
			bool haveBranch = false;
			switch (fit->kind) {
				case ld::Fixup::kindAddAddend:
					addend = fit->u.addend;
					break;
				case ld::Fixup::kindStoreARMBranch24:
				case ld::Fixup::kindStoreThumbBranch22:
				case ld::Fixup::kindStoreTargetAddressARMBranch24:
				case ld::Fixup::kindStoreTargetAddressThumbBranch22:
#if SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64_32
				case ld::Fixup::kindStoreARM64Branch26:
				case ld::Fixup::kindStoreTargetAddressARM64Branch26:
#endif
					haveBranch = true;
					break;
                default:
                    break;   
			}
			if ( haveBranch ) {
				bool crossSectionBranch = ( preload && (atom->section() != target->section()) );
				int64_t srcAddr = atom->sectionOffset() + fit->offsetInAtom;
				int64_t dstAddr = target->sectionOffset() + addend;
				if ( preload ) {
					srcAddr = sAtomToAddress[atom] + fit->offsetInAtom;
					dstAddr = sAtomToAddress[target] + addend;
				}
				if ( target->section().type() == ld::Section::typeStub )
					dstAddr = totalTextSize;
				int64_t displacement = dstAddr - srcAddr;
				TargetAndOffset finalTargetAndOffset = { target, (uint32_t)addend };
				const int64_t kBranchLimit = kBetweenRegions;
				if ( crossSectionBranch && ((displacement > kBranchLimit) || (displacement < (-kBranchLimit))) ) {
					const ld::Atom* island;
					AtomToIsland* region = regionsMap[0];
					AtomToIsland::iterator pos = region->find(finalTargetAndOffset);
					if ( pos == region->end() ) {
						island = makeBranchIsland(opts, fit->kind, 0, target, finalTargetAndOffset, atom->section(), true);
						(*region)[finalTargetAndOffset] = island;
						if (_s_log) fprintf(stderr, "added absolute branching island %p %s, displacement=%lld\n", 
												island, island->name(), displacement);
						++islandCount;
						regionsIslands[0]->push_back(island);
						state.atomToSection[island] = textSection;
					}
					else {
						island = pos->second;
					}
					if (_s_log) fprintf(stderr, "using island %p %s for branch to %s from %s\n", island, island->name(), target->name(), atom->name());
					fixupWithTarget->u.target = island;
					fixupWithTarget->binding = ld::Fixup::bindingDirectlyBound;
				}
				else if ( displacement > kBranchLimit ) {
					// create forward branch chain
					const ld::Atom* nextTarget = target;
					if (_s_log) fprintf(stderr, "need forward branching island srcAdr=0x%08llX, dstAdr=0x%08llX, target=%s\n",
														srcAddr, dstAddr, target->name());
					for (int i=kIslandRegionsCount-1; i >=0 ; --i) {
						AtomToIsland* region = regionsMap[i];
						int64_t islandRegionAddr = regionAddresses[i];
						if ( (srcAddr < islandRegionAddr) && ((islandRegionAddr <= dstAddr)) ) { 
							AtomToIsland::iterator pos = region->find(finalTargetAndOffset);
							if ( pos == region->end() ) {
								ld::Atom* island = makeBranchIsland(opts, fit->kind, i, nextTarget, finalTargetAndOffset, atom->section(), false);
								(*region)[finalTargetAndOffset] = island;
								if (_s_log) fprintf(stderr, "added forward branching island %p %s to region %d for %s\n", island, island->name(), i, atom->name());
								regionsIslands[i]->push_back(island);
								state.atomToSection[island] = textSection;
								++islandCount;
								nextTarget = island;
							}
							else {
								nextTarget = pos->second;
							}
						}
					}
					if (_s_log) fprintf(stderr, "using island %p %s for branch to %s from %s\n", nextTarget, nextTarget->name(), target->name(), atom->name());
					fixupWithTarget->u.target = nextTarget;
					fixupWithTarget->binding = ld::Fixup::bindingDirectlyBound;
				}
				else if ( displacement < (-kBranchLimit) ) {
					// create back branching chain
					const ld::Atom* prevTarget = target;
					for (int i=0; i < kIslandRegionsCount ; ++i) {
						AtomToIsland* region = regionsMap[i];
						int64_t islandRegionAddr = regionAddresses[i];
						if ( (dstAddr < islandRegionAddr) && (islandRegionAddr <= srcAddr) ) {
							if (_s_log) fprintf(stderr, "need backward branching island srcAdr=0x%08llX, dstAdr=0x%08llX, target=%s\n", srcAddr, dstAddr, target->name());
							AtomToIsland::iterator pos = region->find(finalTargetAndOffset);
							if ( pos == region->end() ) {
								ld::Atom* island = makeBranchIsland(opts, fit->kind, i, prevTarget, finalTargetAndOffset, atom->section(), false);
								(*region)[finalTargetAndOffset] = island;
								if (_s_log) fprintf(stderr, "added back branching island %p %s to region %d for %s\n", island, island->name(), i, atom->name());
								regionsIslands[i]->push_back(island);
								state.atomToSection[island] = textSection;
								++islandCount;
								prevTarget = island;
							}
							else {
								prevTarget = pos->second;
							}
						}
					}
					if (_s_log) fprintf(stderr, "using back island %p %s for %s\n", prevTarget, prevTarget->name(), atom->name());
					fixupWithTarget->u.target = prevTarget;
					fixupWithTarget->binding = ld::Fixup::bindingDirectlyBound;
				}
			}
		}
	}


	// insert islands into __text section and adjust section offsets
	if ( islandCount > 0 ) {
		if ( _s_log ) fprintf(stderr, "ld: %u branch islands required in %u regions\n", islandCount, kIslandRegionsCount);
		std::vector<const ld::Atom*> newAtomList;
		newAtomList.reserve(textSection->atoms.size()+islandCount);
		
		int regionIndex = 0;
		for (std::vector<const ld::Atom*>::iterator ait=textSection->atoms.begin(); ait != textSection->atoms.end(); ait++) {
			const ld::Atom* atom = *ait;
			newAtomList.push_back(atom);
			if ( (regionIndex < kIslandRegionsCount) && (atom == branchIslandInsertionPoints[regionIndex]) ) {
				std::vector<const ld::Atom*>* islands = regionsIslands[regionIndex];
				newAtomList.insert(newAtomList.end(), islands->begin(), islands->end());
				++regionIndex;
			}
		}
		// swap in new list of atoms for __text section
		textSection->atoms.clear();
		textSection->atoms = newAtomList;
	}

}


static void buildAddressMap(const Options& opts, ld::Internal& state) {
	// Assign addresses to sections
	state.setSectionSizesAndAlignments();
	state.assignFileOffsets();
	
	// Assign addresses to atoms in a side table
	const bool log = false;
	if ( log ) fprintf(stderr, "buildAddressMap()\n");
	for (std::vector<ld::Internal::FinalSection*>::iterator sit = state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		uint16_t maxAlignment = 0;
		uint64_t offset = 0;
		if ( log ) fprintf(stderr, "  section=%s/%s, address=0x%08llX\n", sect->segmentName(), sect->sectionName(), sect->address);
		for (std::vector<const ld::Atom*>::iterator ait = sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			uint32_t atomAlignmentPowerOf2 = atom->alignment().powerOf2;
			uint32_t atomModulus = atom->alignment().modulus;
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
			
			if ( log ) fprintf(stderr, "    0x%08llX atom=%p, name=%s\n", sect->address+offset, atom, atom->name());
			sAtomToAddress[atom] = sect->address + offset;
			
			offset += atom->size();
		}
	}

	
}

void doPass(const Options& opts, ld::Internal& state)
{	
	// only make branch islands in final linked images
	if ( opts.outputKind() == Options::kObjectFile )
		return;
	
	// Allow user to disable branch island generation
	if ( !opts.allowBranchIslands() )
		return;
	
	// only ARM[64] needs branch islands
	switch ( opts.architecture() ) {
		case CPU_TYPE_ARM:
#if SUPPORT_ARCH_arm64
		case CPU_TYPE_ARM64:
#endif
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
#endif
			break;
		default:
			return;
	}
	
	if ( opts.outputKind() == Options::kPreload ) {
		buildAddressMap(opts, state);
	}
	
	// scan sections for number of stubs
	unsigned stubCount = 0;
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect->type() == ld::Section::typeStub )
			stubCount = sect->atoms.size();
	}

	// scan sections and add island to each code section
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect->type() == ld::Section::typeCode ) 
			makeIslandsForSection(opts, state, sect, stubCount);
	}
}


} // namespace branch_island
} // namespace passes 
} // namespace ld 
