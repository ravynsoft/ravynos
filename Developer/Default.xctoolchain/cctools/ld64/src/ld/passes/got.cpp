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

#include <vector>
#include <map>

#include "MachOFileAbstraction.hpp"
#include "ld.hpp"
#include "got.h"
#include "configure.h"

namespace ld {
namespace passes {
namespace got {

class File; // forward reference

class GOTEntryAtom : public ld::Atom {
public:
											GOTEntryAtom(ld::Internal& internal, const ld::Atom* target, bool weakImport, bool weakDef, bool is64)
				: ld::Atom(weakDef ? _s_sectionWeak : _s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer, 
							symbolTableNotIn, false, false, false, (is64 ? ld::Atom::Alignment(3) : ld::Atom::Alignment(2))),
				_fixup(0, ld::Fixup::k1of1, (is64 ? ld::Fixup::kindStoreTargetAddressLittleEndian64 : ld::Fixup::kindStoreTargetAddressLittleEndian32), target),
				_target(target),
				_is64(is64)
					{ _fixup.weakImport = weakImport; internal.addAtom(*this); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _target->name(); }
	virtual uint64_t						size() const					{ return (_is64 ? 8 : 4); }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup)[1]; }

private:
	mutable ld::Fixup						_fixup;
	const ld::Atom*							_target;
	bool									_is64;
	
	static ld::Section						_s_section;
	static ld::Section						_s_sectionWeak;
};

ld::Section GOTEntryAtom::_s_section("__DATA", "__got", ld::Section::typeNonLazyPointer);
ld::Section GOTEntryAtom::_s_sectionWeak("__DATA", "__got_weak", ld::Section::typeNonLazyPointer);

#if SUPPORT_ARCH_arm64e

class GOTAuthEntryAtom : public ld::Atom {
public:
											GOTAuthEntryAtom(ld::Internal& internal, const ld::Atom* target, bool weakImport, bool weakDef)
				: ld::Atom(weakDef ? _s_sectionWeak : _s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer,
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(3)),
				_fixup1(0, ld::Fixup::k1of2, ld::Fixup::kindSetAuthData, (ld::Fixup::AuthData){ 0, true, ld::Fixup::AuthData::ptrauth_key_asia }),
				_fixup2(0, ld::Fixup::k2of2, ld::Fixup::kindStoreTargetAddressLittleEndianAuth64, target),
				_target(target)
					{ _fixup2.weakImport = weakImport; internal.addAtom(*this); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _target->name(); }
	virtual uint64_t						size() const					{ return 8; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	mutable ld::Fixup						_fixup1;
	mutable ld::Fixup						_fixup2;
	const ld::Atom*							_target;

	static ld::Section						_s_section;
	static ld::Section						_s_sectionWeak;
};

ld::Section GOTAuthEntryAtom::_s_section("__DATA", "__got", ld::Section::typeNonLazyPointer);
ld::Section GOTAuthEntryAtom::_s_sectionWeak("__DATA", "__got_weak", ld::Section::typeNonLazyPointer);

#endif


static bool gotFixup(const Options& opts, ld::Internal& internal, const ld::Atom* targetOfGOT, const ld::Atom* fixupAtom,
					 const ld::Fixup* fixup, bool* optimizable, bool* targetIsExternalWeakDef, bool* targetIsPersonalityFn)
{
	*targetIsExternalWeakDef = false;
	*targetIsPersonalityFn = false;
	switch (fixup->kind) {
		case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoad:
#if SUPPORT_ARCH_arm64
		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
		case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPageOff12:
#endif
			// start by assuming this can be optimized
			*optimizable = true;
			// cannot do LEA optimization if target is in another dylib
			if ( targetOfGOT->definition() == ld::Atom::definitionProxy ) 
				*optimizable = false;
			// cannot do LEA optimization if target in __huge section
			if ( internal.usingHugeSections && (targetOfGOT->size() > 1024*1024)
											&& (   (targetOfGOT->section().type() == ld::Section::typeZeroFill)
												|| (targetOfGOT->section().type() == ld::Section::typeTentativeDefs)) ) {
				*optimizable = false;
			}
			if ( targetOfGOT->scope() == ld::Atom::scopeGlobal ) {
				// cannot do LEA optimization if target is weak exported symbol
				if ( ((targetOfGOT->definition() == ld::Atom::definitionRegular) || (targetOfGOT->definition() == ld::Atom::definitionProxy)) && (targetOfGOT->combine() == ld::Atom::combineByName) ) {
					switch ( opts.outputKind() ) {
						case Options::kDynamicExecutable:
						case Options::kDynamicLibrary:
						case Options::kDynamicBundle:
						case Options::kKextBundle:
							*targetIsExternalWeakDef = true;
							*optimizable = false;
							break;
						case Options::kStaticExecutable:
						case Options::kDyld:
						case Options::kPreload:
						case Options::kObjectFile:
							break;
					}
				}
				// cannot do LEA optimization if target is interposable
				if ( opts.interposable(targetOfGOT->name()) ) 
					*optimizable = false;
				// cannot do LEA optimization if target is resolver function
				if ( targetOfGOT->contentType() == ld::Atom::typeResolver ) 
					*optimizable = false;
				// cannot do LEA optimization for flat-namespace
				if ( opts.nameSpace() != Options::kTwoLevelNameSpace ) 
					*optimizable = false;
			}
			else if ( targetOfGOT->scope() == ld::Atom::scopeLinkageUnit) {
				// <rdar://problem/12379969> don't do optimization if target is in custom segment
				if ( opts.sharedRegionEligible() ) {
					const char* segName = targetOfGOT->section().segmentName();
					if ( (strcmp(segName, "__TEXT") != 0) && (strcmp(segName, "__DATA") != 0) ) {
						*optimizable = false;
					}
				}
			}
			return true;
		case ld::Fixup::kindStoreX86PCRel32GOT:
#if SUPPORT_ARCH_arm64
		case ld::Fixup::kindStoreARM64PCRelToGOT:
#endif
#if SUPPORT_ARCH_arm64e
			// Note, this handles identifying DWARF unwind info personality functions
			if (opts.supportsAuthenticatedPointers()) {
				if (fixupAtom->section().type() == ld::Section::typeCFI)
					*targetIsPersonalityFn = true;
			}
#endif
			*optimizable = false;
			return true;
		case ld::Fixup::kindNoneGroupSubordinatePersonality:
			*optimizable = false;
#if SUPPORT_ARCH_arm64e
			// Note, this is a compact unwind info personality function
			if (opts.supportsAuthenticatedPointers())
				*targetIsPersonalityFn = true;
#endif
			return true;
		default:
			break;
	}
	
	return false;
}

struct AtomByNameSorter
{
	 bool operator()(const ld::Atom* left, const ld::Atom* right)
	 {
		  return (strcmp(left->name(), right->name()) < 0);
	 }
};

struct GotMapEntry {
	const ld::Atom* atom;
	bool isPersonalityFn;

	bool operator<(const GotMapEntry& other) const {
		if (atom != other.atom)
			return atom < other.atom;
		return (int)isPersonalityFn < (int)other.isPersonalityFn;
	}
};

void doPass(const Options& opts, ld::Internal& internal)
{
	const bool log = false;
	
	// only make got section in final linked images
	if ( opts.outputKind() == Options::kObjectFile )
		return;

	// pre-fill gotMap with existing non-lazy pointers
	std::map<GotMapEntry, const ld::Atom*> gotMap;
	for (ld::Internal::FinalSection* sect : internal.sections) {
		if ( sect->type() != ld::Section::typeNonLazyPointer )
			continue;
		for (const ld::Atom* atom : sect->atoms) {
			const ld::Atom* target = NULL;
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
				switch (fit->kind) {
				case ld::Fixup::kindStoreTargetAddressLittleEndian64:
				case ld::Fixup::kindStoreTargetAddressLittleEndian32:
					switch ( fit->binding ) {
						case ld::Fixup::bindingsIndirectlyBound:
							target = internal.indirectBindingTable[fit->u.bindingIndex];
							break;
						case ld::Fixup::bindingDirectlyBound:
							target = fit->u.target;
							break;
						default:
							fprintf(stderr, "non-pointer is got entry\n");
							break;
					}
					break;
				default:
					break;
				}
			}
			if ( target != NULL ) {
				if (log) fprintf(stderr, "found existing got entry to %s\n", target->name());
				gotMap[{ target, false }] = atom;
			}
		}
	}

	// walk all atoms and fixups looking for GOT-able references
	// don't create GOT atoms during this loop because that could invalidate the sections iterator
	std::vector<const ld::Atom*> atomsReferencingGOT;
	std::map<const ld::Atom*,bool>		weakImportMap;
	std::map<const ld::Atom*,bool>		weakDefMap;
	atomsReferencingGOT.reserve(128);
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=internal.sections.begin(); sit != internal.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin();  ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			bool atomUsesGOT = false;
			const ld::Atom* targetOfGOT = NULL;
			bool targetIsWeakImport = false;
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
				if ( fit->firstInCluster() ) 
					targetOfGOT = NULL;
				switch ( fit->binding ) {
					case ld::Fixup::bindingsIndirectlyBound:
						targetOfGOT = internal.indirectBindingTable[fit->u.bindingIndex];
						targetIsWeakImport = fit->weakImport;
						break;
					case ld::Fixup::bindingDirectlyBound:
						targetOfGOT = fit->u.target;
						targetIsWeakImport = fit->weakImport;
						break;
                    default:
                        break;   
				}
				bool optimizable;
				bool targetIsExternalWeakDef;
				bool targetIsPersonalityFn;
				if ( !gotFixup(opts, internal, targetOfGOT, atom, fit, &optimizable, &targetIsExternalWeakDef, &targetIsPersonalityFn) )
					continue;
				if ( optimizable ) {
					// change from load of GOT entry to lea of target
					if ( log ) fprintf(stderr, "optimized GOT usage in %s to %s\n", atom->name(), targetOfGOT->name());
					switch ( fit->binding ) {
						case ld::Fixup::bindingsIndirectlyBound:
						case ld::Fixup::bindingDirectlyBound:
							fit->binding = ld::Fixup::bindingDirectlyBound;
							fit->u.target = targetOfGOT;
							switch ( fit->kind ) {
								case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoad:
									fit->kind = ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoadNowLEA;
									break;
#if SUPPORT_ARCH_arm64
								case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
									fit->kind = ld::Fixup::kindStoreTargetAddressARM64GOTLeaPage21;
									break;
								case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPageOff12:
									fit->kind = ld::Fixup::kindStoreTargetAddressARM64GOTLeaPageOff12;
									break;
#endif
								default:
									assert(0 && "unsupported GOT reference kind");
									break;
							}
							break;
						default:
							assert(0 && "unsupported GOT reference");
							break;
					}
				}
				else {
					// remember that we need to use GOT in this function
					if ( log ) fprintf(stderr, "found GOT use in %s\n", atom->name());
					if ( !atomUsesGOT ) {
						atomsReferencingGOT.push_back(atom);
						atomUsesGOT = true;
					}
					if ( gotMap.count({ targetOfGOT, targetIsPersonalityFn }) == 0 )
						gotMap[{ targetOfGOT, targetIsPersonalityFn }] = NULL;
					// record if target is weak def
					weakDefMap[targetOfGOT] = targetIsExternalWeakDef;
					// record weak_import attribute
					std::map<const ld::Atom*,bool>::iterator pos = weakImportMap.find(targetOfGOT);
					if ( pos == weakImportMap.end() ) {
						// target not in weakImportMap, so add
						if ( log ) fprintf(stderr, "weakImportMap[%s] = %d\n", targetOfGOT->name(), targetIsWeakImport);
						weakImportMap[targetOfGOT] = targetIsWeakImport; 
					}
					else {
						// target in weakImportMap, check for weakness mismatch
						if ( pos->second != targetIsWeakImport ) {
							// found mismatch
							switch ( opts.weakReferenceMismatchTreatment() ) {
								case Options::kWeakReferenceMismatchError:
									throwf("mismatching weak references for symbol: %s", targetOfGOT->name());
								case Options::kWeakReferenceMismatchWeak:
									pos->second = true;
									break;
								case Options::kWeakReferenceMismatchNonWeak:
									pos->second = false;
									break;
							}
						}
					}
				}
			}
		}
	}
	
	bool is64 = false;
	switch ( opts.architecture() ) {
#if SUPPORT_ARCH_i386
		case CPU_TYPE_I386:
			is64 = false;
			break;
#endif
#if SUPPORT_ARCH_x86_64
		case CPU_TYPE_X86_64:
			is64 = true;
			break;
#endif
#if SUPPORT_ARCH_arm_any
		case CPU_TYPE_ARM: 
			is64 = false;
			break;
#endif
#if SUPPORT_ARCH_arm64
		case CPU_TYPE_ARM64: 
			is64 = true;
			break;
#endif
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
			is64 = false;
			break;
#endif
	}
	
	// make GOT entries
	for (auto& entry : gotMap) {
		if ( entry.second == NULL ) {
			bool weakDef = opts.useDataConstSegment() && opts.sharedRegionEligible() && weakDefMap[entry.first.atom];
#if SUPPORT_ARCH_arm64e
			if ( entry.first.isPersonalityFn && (opts.supportsAuthenticatedPointers()) ) {
				entry.second = new GOTAuthEntryAtom(internal, entry.first.atom, weakImportMap[entry.first.atom], weakDef);
				if (log) fprintf(stderr, "making new GOT slot for %s, gotMap[%p] = %p\n", entry.first.atom->name(), entry.first.atom, entry.second);
				continue;
			}
#endif
			entry.second = new GOTEntryAtom(internal, entry.first.atom, weakImportMap[entry.first.atom], weakDef, is64);
			if (log) fprintf(stderr, "making new GOT slot for %s, gotMap[%p] = %p\n", entry.first.atom->name(), entry.first.atom, entry.second);
		}
	}


	// update atoms to use GOT entries
	for (std::vector<const ld::Atom*>::iterator it=atomsReferencingGOT.begin(); it != atomsReferencingGOT.end(); ++it) {
		const ld::Atom* atom = *it;
		const ld::Atom* targetOfGOT = NULL;
		ld::Fixup::iterator fitThatSetTarget = NULL;
		for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
			if ( fit->firstInCluster() ) {
				targetOfGOT = NULL;
				fitThatSetTarget = NULL;
			}
			switch ( fit->binding ) {
				case ld::Fixup::bindingsIndirectlyBound:
					targetOfGOT = internal.indirectBindingTable[fit->u.bindingIndex];
					fitThatSetTarget = fit;
					break;
				case ld::Fixup::bindingDirectlyBound:
					targetOfGOT = fit->u.target;
					fitThatSetTarget = fit;
					break;
                default:
                    break;    
			}
			bool optimizable;
			bool targetIsExternalWeakDef;
			bool targetIsPersonalityFn;
			if ( (targetOfGOT == NULL) || !gotFixup(opts, internal, targetOfGOT, atom, fit,
													&optimizable, &targetIsExternalWeakDef, &targetIsPersonalityFn) )
				continue;
			if ( !optimizable ) {
				// GOT use not optimized away, update to bind to GOT entry
				assert(fitThatSetTarget != NULL);
				switch ( fitThatSetTarget->binding ) {
					case ld::Fixup::bindingsIndirectlyBound:
					case ld::Fixup::bindingDirectlyBound:
						if ( log ) fprintf(stderr, "updating GOT use in %s to %s\n", atom->name(), targetOfGOT->name());
						fitThatSetTarget->binding = ld::Fixup::bindingDirectlyBound;
						fitThatSetTarget->u.target = gotMap[{ targetOfGOT, targetIsPersonalityFn }];
						break;
					default:
						assert(0 && "unsupported GOT reference");
						break;
				}
			}
		}
	}
	
	// sort new atoms so links are consistent
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=internal.sections.begin(); sit != internal.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect->type() == ld::Section::typeNonLazyPointer ) {
			std::sort(sect->atoms.begin(), sect->atoms.end(), AtomByNameSorter());
		}
	}
}


} // namespace got
} // namespace passes 
} // namespace ld 
