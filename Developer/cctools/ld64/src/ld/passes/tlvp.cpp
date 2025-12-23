/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

#include <vector>
#include <map>

#include "ld.hpp"
#include "tlvp.h"

namespace ld {
namespace passes {
namespace tlvp {

class File; // forward reference

class TLVEntryAtom : public ld::Atom {
public:
											TLVEntryAtom(ld::Internal& internal, const ld::Atom* target, bool weakImport, unsigned ptrSize)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(3)), 
				_fixup(0, ld::Fixup::k1of1, (ptrSize ==8) ? ld::Fixup::kindStoreTargetAddressLittleEndian64 : ld::Fixup::kindStoreTargetAddressLittleEndian32, target),
				_target(target),
				_size(ptrSize)
					{	_fixup.weakImport = weakImport; internal.addAtom(*this); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _target->name(); }
	virtual uint64_t						size() const					{ return _size; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup)[1]; }

private:
	mutable ld::Fixup						_fixup;
	const ld::Atom*							_target;
	unsigned 								_size;

	static ld::Section						_s_section;
};

ld::Section TLVEntryAtom::_s_section("__DATA", "__thread_ptrs", ld::Section::typeTLVPointers);


static bool optimizable(const Options& opts, const ld::Atom* targetOfTLV)
{
	// cannot do LEA optimization if target is in another dylib
	if ( targetOfTLV->definition() == ld::Atom::definitionProxy ) 
		return false;
	if ( targetOfTLV->scope() == ld::Atom::scopeGlobal ) {	
		// cannot do LEA optimization if target is weak exported symbol
		if ( (targetOfTLV->definition() == ld::Atom::definitionRegular) && (targetOfTLV->combine() == ld::Atom::combineByName) )
			return false;
		// cannot do LEA optimization if target is interposable
		if ( opts.interposable(targetOfTLV->name()) ) 
			return false;
		// cannot do LEA optimization for flat-namespace
		if ( opts.nameSpace() != Options::kTwoLevelNameSpace ) 
			return false;
	}
	return true;
}

struct AtomByNameSorter
{
	 bool operator()(const ld::Atom* left, const ld::Atom* right)
	 {
		  return (strcmp(left->name(), right->name()) < 0);
	 }
};

struct TlVReferenceCluster
{
	const ld::Atom* targetOfTLV;
	ld::Fixup*		fixupWithTarget;
	ld::Fixup*		fixupWithTLVStore;
	bool			optimizable;
};

void doPass(const Options& opts, ld::Internal& internal)
{
	const bool log = false;
	
	// only make tlv section in final linked images
	if ( opts.outputKind() == Options::kObjectFile )
		return;

	const unsigned ptrSize = (opts.architecture() == CPU_TYPE_ARM64_32) ? 4 : 8;

	// walk all atoms and fixups looking for TLV references and add them to list
	std::vector<TlVReferenceCluster>	references;
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=internal.sections.begin(); sit != internal.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin(); ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			TlVReferenceCluster ref;
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
				if ( fit->firstInCluster() ) {
					ref.targetOfTLV = NULL;
					ref.fixupWithTarget = NULL;
					ref.fixupWithTLVStore = NULL;
				}
				switch ( fit->binding ) {
					case ld::Fixup::bindingsIndirectlyBound:
						ref.targetOfTLV = internal.indirectBindingTable[fit->u.bindingIndex];
						ref.fixupWithTarget = fit;
						break;
					case ld::Fixup::bindingDirectlyBound:
						ref.targetOfTLV = fit->u.target;
						ref.fixupWithTarget = fit;
						break;
                    default:
                        break;    
				}
				switch ( fit->kind ) {
					case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoad:
					case ld::Fixup::kindStoreTargetAddressX86Abs32TLVLoad:
					case ld::Fixup::kindStoreX86PCRel32TLVLoad:
					case ld::Fixup::kindStoreX86Abs32TLVLoad:
#if SUPPORT_ARCH_arm64
					case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPage21:
					case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPageOff12:
#endif
						ref.fixupWithTLVStore = fit;
						break;
					default:
						break;
				}
				if ( fit->lastInCluster() && (ref.fixupWithTLVStore != NULL) ) {
					ref.optimizable = optimizable(opts, ref.targetOfTLV);
					if (log) fprintf(stderr, "found reference to TLV at %s+0x%X to %s\n", 
									atom->name(), ref.fixupWithTLVStore->offsetInAtom, ref.targetOfTLV->name());
					if ( ! opts.canUseThreadLocalVariables() ) {
						throwf("targeted OS version does not support use of thread local variables in %s", atom->name());
					}
					references.push_back(ref);
				}
			}
		}
	}
	
	// compute which TLV references will be weak_imports
	std::map<const ld::Atom*,bool>		weakImportMap;
	for(std::vector<TlVReferenceCluster>::iterator it=references.begin(); it != references.end(); ++it) {
		if ( !it->optimizable ) {
			// record weak_import attribute
			std::map<const ld::Atom*,bool>::iterator pos = weakImportMap.find(it->targetOfTLV);
			if ( pos == weakImportMap.end() ) {
				// target not in weakImportMap, so add
				weakImportMap[it->targetOfTLV] = it->fixupWithTarget->weakImport;
			}
			else {
				// target in weakImportMap, check for weakness mismatch
				if ( pos->second != it->fixupWithTarget->weakImport ) {
					// found mismatch
					switch ( opts.weakReferenceMismatchTreatment() ) {
						case Options::kWeakReferenceMismatchError:
							throwf("mismatching weak references for symbol: %s", it->targetOfTLV->name());
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

	// create TLV pointers for TLV references that cannot be optimized
	std::map<const ld::Atom*,ld::Atom*> variableToPointerMap;
	for(std::map<const ld::Atom*,bool>::iterator it=weakImportMap.begin(); it != weakImportMap.end(); ++it) {
		std::map<const ld::Atom*,ld::Atom*>::iterator pos = variableToPointerMap.find(it->first);
		if ( pos == variableToPointerMap.end() ) {
			if (log) fprintf(stderr, "make TLV pointer for %s\n", it->first->name());
			if ( it->first->contentType() != ld::Atom::typeTLV )
				throwf("illegal thread local variable reference to regular symbol %s", it->first->name());
			TLVEntryAtom* tlvp = new TLVEntryAtom(internal, it->first, it->second, ptrSize);
			variableToPointerMap[it->first] = tlvp;
		}
	}

	// sort new tvlp atoms so links are consistent
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=internal.sections.begin(); sit != internal.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect->type() == ld::Section::typeTLVPointers ) {
			std::sort(sect->atoms.begin(), sect->atoms.end(), AtomByNameSorter());
		}
	}

	// update references to use TLV pointers or TLV object directly
	for(std::vector<TlVReferenceCluster>::iterator it=references.begin(); it != references.end(); ++it) {
		if ( it->optimizable ) {
			// change store to be LEA instead load (mov)
			if (log) fprintf(stderr, "optimizing load of TLV to %s into an LEA\n", it->targetOfTLV->name());
			it->fixupWithTLVStore->binding = ld::Fixup::bindingDirectlyBound;
			it->fixupWithTLVStore->u.target = it->targetOfTLV;
			switch ( it->fixupWithTLVStore->kind ) {
				case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoad:
					it->fixupWithTLVStore->kind = ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoadNowLEA;
					break;
				case ld::Fixup::kindStoreTargetAddressX86Abs32TLVLoad:
					it->fixupWithTLVStore->kind = ld::Fixup::kindStoreTargetAddressX86Abs32TLVLoadNowLEA;
					break;
				case ld::Fixup::kindStoreX86PCRel32TLVLoad:
					it->fixupWithTLVStore->kind = ld::Fixup::kindStoreX86PCRel32TLVLoadNowLEA;
					break;
				case ld::Fixup::kindStoreX86Abs32TLVLoad:
					it->fixupWithTLVStore->kind = ld::Fixup::kindStoreX86Abs32TLVLoadNowLEA;
					break;
#if SUPPORT_ARCH_arm64
				case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPage21:
					it->fixupWithTLVStore->kind = ld::Fixup::kindStoreTargetAddressARM64TLVPLoadNowLeaPage21;
					break;
				case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPageOff12:
					it->fixupWithTLVStore->kind = ld::Fixup::kindStoreTargetAddressARM64TLVPLoadNowLeaPageOff12;
					break;
#endif
				default:
					assert(0 && "bad store kind for TLV optimization");
			}
		}
		else {
			// change target to be new TLV pointer atom
			if (log) fprintf(stderr, "updating load of TLV to %s to load from TLV pointer\n", it->targetOfTLV->name());
			const ld::Atom* tlvpAtom = variableToPointerMap[it->targetOfTLV];
			assert(tlvpAtom != NULL);
			it->fixupWithTarget->binding = ld::Fixup::bindingDirectlyBound;
			it->fixupWithTarget->u.target = tlvpAtom;
		}
	}
	
	
	
	// alter tlv definitions to have an offset as third field
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=internal.sections.begin(); sit != internal.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		if ( sect->type() != ld::Section::typeTLVDefs ) 
			continue;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin();  ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			if ( ! opts.canUseThreadLocalVariables() ) {
				throwf("targeted OS version does not support use of thread local variables in %s", atom->name());
			}
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
				if ( fit->offsetInAtom != 0 ) {
					assert(fit->binding == ld::Fixup::bindingDirectlyBound && "thread variable def contains pointer to global");
					switch( fit->u.target->contentType() ) {
						case ld::Atom::typeTLVZeroFill:
						case ld::Atom::typeTLVInitialValue:
							switch ( fit->kind ) {
								case ld::Fixup::kindSetTargetAddress:
									fit->kind = ld::Fixup::kindSetTargetTLVTemplateOffset;
									break;
								case ld::Fixup::kindStoreTargetAddressLittleEndian32:
									fit->kind = ld::Fixup::kindSetTargetTLVTemplateOffsetLittleEndian32;
									break;
								case ld::Fixup::kindStoreTargetAddressLittleEndian64:
									fit->kind = ld::Fixup::kindSetTargetTLVTemplateOffsetLittleEndian64;
									break;
								default:
									assert(0 && "bad kind for target in tlv defs");
									break;
							}
							break;
						default:
							assert(0 && "wrong content type for target in tlv defs");
							break;
					}
				}
			}
		}
	}
	
}



} // namespace tlvp
} // namespace passes 
} // namespace ld 
