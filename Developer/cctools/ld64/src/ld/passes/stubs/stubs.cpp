/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
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
#include <assert.h>
#include <libkern/OSByteOrder.h>

#include <vector>
#include <set>
#include <map>

#include "Options.h"
#include "MachOFileAbstraction.hpp"
#include "ld.hpp"

#include "make_stubs.h"


namespace ld {
namespace passes {
namespace stubs {

class Pass {
public:
								Pass(const Options& opts);
	void						process(ld::Internal& internal);
	void						addAtom(const ld::Atom& atom)	{ _internal->addAtom(atom); }
	bool						usingCompressedLINKEDIT() const { return _compressedLINKEDIT; }
	ld::Internal*				internal() { return _internal; }

	Atom*						compressedHelperHelper;
	Atom*						compressedImageCache;
	Atom*						compressedFastBinderPointer;
	const bool					usingDataConstSegment;

private:

	struct AtomByNameSorter
	{
		 bool operator()(const ld::Atom* left, const ld::Atom* right)
		 {
			  return (strcmp(left->name(), right->name()) < 0);
		 }
	};

	const ld::Atom*				stubableFixup(const ld::Fixup* fixup, ld::Internal&);
	ld::Atom*					makeStub(const ld::Atom& target, bool weakImport);
	void						verifyNoResolverFunctions(ld::Internal& state);

	const Options&				_options;
	const cpu_type_t			_architecture;
	const bool					_compressedLINKEDIT;
	const bool					_mightBeInSharedRegion;
	const bool					_pic;
	const bool					_flatNamespace;
	ld::Internal*				_internal;
	uint32_t					_stubCount;
	bool						_largeText;
};

#include "stub_x86_64.hpp"
#include "stub_x86_64_classic.hpp"
#include "stub_x86.hpp"
#include "stub_x86_classic.hpp"
#include "stub_arm.hpp"
#include "stub_arm_classic.hpp"
#if SUPPORT_ARCH_arm64
#include "stub_arm64.hpp"
#endif
#if SUPPORT_ARCH_arm64e
#include "stub_arm64e.hpp"
#endif
#if SUPPORT_ARCH_arm64_32
#include "stub_arm64_32.hpp"
#endif

Pass::Pass(const Options& opts) 
	:	compressedHelperHelper(NULL), 
		compressedImageCache(NULL),
		compressedFastBinderPointer(NULL),
		usingDataConstSegment(opts.useDataConstSegment()),
		_options(opts),
		_architecture(opts.architecture()),
		_compressedLINKEDIT(opts.makeCompressedDyldInfo()),
		_mightBeInSharedRegion(opts.sharedRegionEligible()), 
		_pic(opts.outputSlidable()),
		_flatNamespace(opts.nameSpace() != Options::kTwoLevelNameSpace),
		_internal(NULL), _stubCount(0), _largeText(false)
{
	
} 


const ld::Atom* Pass::stubableFixup(const ld::Fixup* fixup, ld::Internal& state)
{
	if ( fixup->binding == ld::Fixup::bindingsIndirectlyBound ) {
		const ld::Atom* target = state.indirectBindingTable[fixup->u.bindingIndex];
		switch ( fixup->kind ) {
			case ld::Fixup::kindStoreTargetAddressX86BranchPCRel32:
			case ld::Fixup::kindStoreTargetAddressARMBranch24:
			case ld::Fixup::kindStoreTargetAddressThumbBranch22:
#if SUPPORT_ARCH_arm64
			case ld::Fixup::kindStoreTargetAddressARM64Branch26:
#endif
                assert(target != NULL);
				// create stub if target is in a dylib
				if ( target->definition() == ld::Atom::definitionProxy ) 
					return target;
				// use stub if target is a resolver function in same linkage unit
				if ( target->contentType() == ld::Atom::typeResolver ) 
					return target;
				if ( target->scope() == ld::Atom::scopeGlobal ) {	
					// create stub if target is global weak definition in symbol table
					if ( (target->definition() == ld::Atom::definitionRegular) 
						&& (target->combine() == ld::Atom::combineByName) 
						&& ((target->symbolTableInclusion() == ld::Atom::symbolTableIn) 
						 || (target->symbolTableInclusion() == ld::Atom::symbolTableInAndNeverStrip)) ) {
						// don't make stubs for auto-hide symbols
						if ( target->autoHide() && (!_options.hasExportMaskList() || !_options.shouldExport(target->name())) )
							return NULL;
						return target;
					}
					// create stub if target is interposable
					if ( _options.interposable(target->name()) ) 
						return target;
					if ( _flatNamespace ) {
						// flat namespace does not indirect calls within main exectuables
						if ( _options.outputKind() == Options::kDynamicExecutable ) 
							return NULL;
						// create stub if target is global and building -flat dylib or bundle
						return target;
					}
				}
				break;
			default:
				if ( (target != NULL) && (target->contentType() == ld::Atom::typeResolver) ) {
					// any pointer to a resolver needs to change to pointer to stub
					return target;
				}
				break;
		}
	}
	return NULL;
}



ld::Atom* Pass::makeStub(const ld::Atom& target, bool weakImport)
{
	//fprintf(stderr, "makeStub(target=%p %s in sect %s, def=%d)\n", &target, target.name(), target.section().sectionName(), target.definition());
	bool stubToGlobalWeakDef = ( (target.combine() == ld::Atom::combineByName) &&
								 (((target.definition() == ld::Atom::definitionRegular) && (target.scope() == ld::Atom::scopeGlobal))
								  || (target.definition() == ld::Atom::definitionProxy)) );

	bool forLazyDylib = false;
	bool stubToResolver = (target.contentType() == ld::Atom::typeResolver);
#if SUPPORT_ARCH_arm_any || SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64e
	bool usingDataConst =  _options.useDataConstSegment() && _options.sharedRegionEligible();
#endif

	if ( usingCompressedLINKEDIT() && !forLazyDylib && !_options.noLazyBinding() ) {
		if ( _internal->compressedFastBinderProxy == NULL )
			throwf("symbol dyld_stub_binder not found (normally in libSystem.dylib).  Needed to perform lazy binding to function %s", target.name());
	}

	switch ( _architecture ) {
#if SUPPORT_ARCH_i386
		case CPU_TYPE_I386:
			if ( usingCompressedLINKEDIT() && !forLazyDylib && _options.noLazyBinding() )
				return new ld::passes::stubs::x86::NonLazyStubAtom(*this, target, stubToResolver, weakImport);
			else if ( _options.makeChainedFixups() && !stubToResolver )
				return new ld::passes::stubs::x86::NonLazyStubAtom(*this, target, stubToResolver, weakImport);
			else if ( usingCompressedLINKEDIT() && !forLazyDylib )
				return new ld::passes::stubs::x86::StubAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport);
			else
				return new ld::passes::stubs::x86::classic::StubAtom(*this, target, forLazyDylib, weakImport);
			break;
#endif
#if SUPPORT_ARCH_x86_64
		case CPU_TYPE_X86_64:
			if ( (_options.outputKind() == Options::kKextBundle) && _options.kextsUseStubs() )
				return new ld::passes::stubs::x86_64::NonLazyStubAtom(*this, target, weakImport);
			else if ( _options.makeChainedFixups() ) {
				if ( stubToResolver )
					return new ld::passes::stubs::x86_64::StubAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport);
				else
					return new ld::passes::stubs::x86_64::NonLazyStubAtom(*this, target, weakImport);
			}
			else if ( usingCompressedLINKEDIT() ) {
				if ( forLazyDylib )
					return new ld::passes::stubs::x86_64::classic::StubAtom(*this, target, stubToGlobalWeakDef, weakImport);
				else if ( _options.noLazyBinding() && !stubToResolver )
					return new ld::passes::stubs::x86_64::NonLazyStubAtom(*this, target, weakImport);
				else
					return new ld::passes::stubs::x86_64::StubAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport);
			}
			else
				return new ld::passes::stubs::x86_64::classic::StubAtom(*this, target, stubToGlobalWeakDef, weakImport);
			break;
#endif
#if SUPPORT_ARCH_arm_any
		case CPU_TYPE_ARM: 
			if ( (_options.outputKind() == Options::kKextBundle) && _options.kextsUseStubs() ) {
				// if text relocs are not allows in kext bundles, then linker must create a stub 
				return new ld::passes::stubs::arm::StubPICKextAtom(*this, target, weakImport);
			}
			else if ( _options.makeChainedFixups() ) {
				if ( stubToResolver )
					return new ld::passes::stubs::arm::StubPICAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport, usingDataConst);
				else
					return new ld::passes::stubs::arm::NonLazyStubPICAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport, usingDataConst);
			}
			else if ( usingCompressedLINKEDIT() && !forLazyDylib ) {
				if ( (_stubCount < 900) && !_mightBeInSharedRegion && !_largeText && !_options.makeEncryptable() )
					return new ld::passes::stubs::arm::StubCloseAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport);
				else if ( usingCompressedLINKEDIT() && !forLazyDylib && _options.noLazyBinding() && !stubToResolver)
					return new ld::passes::stubs::arm::StubPICKextAtom(*this, target, weakImport);
				else if ( _pic )
					return new ld::passes::stubs::arm::StubPICAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport, usingDataConst);
				else
					return new ld::passes::stubs::arm::StubNoPICAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport);
			} 
			else {
				if ( _pic )
					return new ld::passes::stubs::arm::classic::StubPICAtom(*this, target, forLazyDylib, weakImport);
				else
					return new ld::passes::stubs::arm::classic::StubNoPICAtom(*this, target, forLazyDylib, weakImport);
			}
			break;
#endif
#if SUPPORT_ARCH_arm64
		case CPU_TYPE_ARM64:
#if SUPPORT_ARCH_arm64e
			if ( (_options.subArchitecture() == CPU_SUBTYPE_ARM64E) && _options.useAuthenticatedStubs() ) {
				if ( (_options.outputKind() == Options::kKextBundle) && _options.kextsUseStubs() )
					return new ld::passes::stubs::arm64e::NonLazyStubAtom(*this, target, weakImport);
				else if ( stubToResolver )
					return new ld::passes::stubs::arm64e::StubAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport, usingDataConst);
				else
					return new ld::passes::stubs::arm64e::NonLazyStubAtom(*this, target, weakImport);
				break;
			}
#endif
			if ( (_options.outputKind() == Options::kKextBundle) && _options.kextsUseStubs() )
				return new ld::passes::stubs::arm64::NonLazyStubAtom(*this, target, weakImport);
			else if ( usingCompressedLINKEDIT() && !forLazyDylib && _options.noLazyBinding() && !stubToResolver )
				return new ld::passes::stubs::arm64::NonLazyStubAtom(*this, target, weakImport);
			else if ( _options.makeChainedFixups() && !stubToResolver )
				return new ld::passes::stubs::arm64::NonLazyStubAtom(*this, target, weakImport);
			else
				return new ld::passes::stubs::arm64::StubAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport, usingDataConst);
			break;
#endif
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
			if ( (_options.outputKind() == Options::kKextBundle) && _options.kextsUseStubs() )
				return new ld::passes::stubs::arm64_32::NonLazyStubAtom(*this, target, weakImport);
			else if ( _options.makeChainedFixups() && !stubToResolver )
				return new ld::passes::stubs::arm64_32::NonLazyStubAtom(*this, target, weakImport);
			else
				return new ld::passes::stubs::arm64_32::StubAtom(*this, target, stubToGlobalWeakDef, stubToResolver, weakImport);
			break;
#endif
	}
	throw "unsupported arch for stub";
}


void Pass::verifyNoResolverFunctions(ld::Internal& state)
{
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin();  ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			if ( atom->contentType() == ld::Atom::typeResolver ) 
				throwf("resolver function '%s' not supported in type of output", atom->name());
		}
	}
}

void Pass::process(ld::Internal& state)
{
	switch ( _options.outputKind() ) {
		case Options::kObjectFile:
			// these kinds don't use stubs and can have resolver functions
			return;
		case Options::kStaticExecutable:
		case Options::kPreload:
		case Options::kDyld:
			// these kinds don't use stubs and cannot have resolver functions
			verifyNoResolverFunctions(state);
			return;
		case Options::kDynamicLibrary:
			// uses stubs and can have resolver functions
			break;
		case Options::kKextBundle:
			verifyNoResolverFunctions(state);
			// if kext don't use stubs, don't do this pass
			if ( !_options.kextsUseStubs() )
				return;
			break;
		case Options::kDynamicExecutable:
		case Options::kDynamicBundle:
			// these kinds do use stubs and cannot have resolver functions
			verifyNoResolverFunctions(state);
			break;
	}
	
	// walk all atoms and fixups looking for stubable references
	// don't create stubs inline because that could invalidate the sections iterator
	std::vector<const ld::Atom*> atomsCallingStubs;
	std::map<const ld::Atom*,ld::Atom*> stubFor;
	std::map<const ld::Atom*,bool>		weakImportMap;
	atomsCallingStubs.reserve(128);
	uint64_t codeSize = 0;
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin();  ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			codeSize += atom->size();
			bool atomNeedsStub = false;
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
				const ld::Atom* stubableTargetOfFixup = stubableFixup(fit, state);
				if ( stubableTargetOfFixup != NULL ) {
					if ( !atomNeedsStub ) {
						atomsCallingStubs.push_back(atom);
						atomNeedsStub = true;
					}
					stubFor[stubableTargetOfFixup] = NULL;	
					// record weak_import attribute
					std::map<const ld::Atom*,bool>::iterator pos = weakImportMap.find(stubableTargetOfFixup);
					if ( pos == weakImportMap.end() ) {
						// target not in weakImportMap, so add
						weakImportMap[stubableTargetOfFixup] = fit->weakImport;
					}
					else {
						// target in weakImportMap, check for weakness mismatch
						if ( pos->second != fit->weakImport ) {
							// found mismatch
							switch ( _options.weakReferenceMismatchTreatment() ) {
								case Options::kWeakReferenceMismatchError:
									throwf("mismatching weak references for symbol: %s", stubableTargetOfFixup->name());
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
			// all resolver functions must have a corresponding stub
			if ( atom->contentType() == ld::Atom::typeResolver ) {
				if ( _options.outputKind() != Options::kDynamicLibrary ) 
					throwf("resolver functions (%s) can only be used in dylibs", atom->name());
				if ( !_options.makeCompressedDyldInfo() && !_options.makeChainedFixups() ) {
					if ( _options.architecture() == CPU_TYPE_ARM )
						throwf("resolver functions (%s) can only be used when targeting iOS 4.2 or later", atom->name());
					else
						throwf("resolver functions (%s) can only be used when targeting Mac OS X 10.6 or later", atom->name());
				}
				stubFor[atom] = NULL;	
			}
		}
	}

	const bool needStubForMain = _options.needsEntryPointLoadCommand() 
								&& (state.entryPoint != NULL) 
								&& (state.entryPoint->definition() == ld::Atom::definitionProxy);
	if ( needStubForMain ) {
		// _main not found in any .o files.  Currently have proxy to dylib 
		// Add to map, so that a stub will be made
		stubFor[state.entryPoint] = NULL;	
	}
	
	// short circuit if no stubs needed
	_internal = &state;
	_stubCount = stubFor.size();
	if ( _stubCount == 0 )
		return;
	
	// <rdar://problem/8553283> lazily check for helper
	if ( !_options.makeCompressedDyldInfo() && !_options.makeThreadedStartsSection() && !_options.makeChainedFixups() && (state.classicBindingHelper == NULL) && (_options.outputKind() != Options::kKextBundle) )
		throw "symbol dyld_stub_binding_helper not found, normally in crt1.o/dylib1.o/bundle1.o";

	// disable arm close stubs in some cases
	if ( _architecture == CPU_TYPE_ARM ) {
        if ( codeSize > 4*1024*1024 )
            _largeText = true;
        else {
            for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
                ld::Internal::FinalSection* sect = *sit;
                if ( sect->type() == ld::Section::typeMachHeader )
                    continue;
                if ( strcmp(sect->segmentName(), "__TEXT") == 0) {
                    for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin();  ait != sect->atoms.end(); ++ait) {
                        const ld::Atom* atom = *ait;
                        if ( atom->alignment().powerOf2 > 10 ) {
                            // overaligned section means might not be able to keep closestubs sect pushed to end of __TEXT
                            //warning("alignment 1<<%d in atom %s in section %s disables close stubs optimization", 
                            //        atom->alignment().powerOf2, atom->name(), sect->segmentName());
                            _largeText = true;
                            break;
                        }
                    }
                }
            }
        }
    }
	
	// make stub atoms 
	for (std::map<const ld::Atom*,ld::Atom*>::iterator it = stubFor.begin(); it != stubFor.end(); ++it) {
		it->second = makeStub(*it->first, weakImportMap[it->first]);
	}
	
	// updated atoms to use stubs
	for (std::vector<const ld::Atom*>::iterator it=atomsCallingStubs.begin(); it != atomsCallingStubs.end(); ++it) {
		const ld::Atom* atom = *it;
		for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
			const ld::Atom* stubableTargetOfFixup = stubableFixup(fit, state);
			if ( stubableTargetOfFixup != NULL ) {
				ld::Atom* stub = stubFor[stubableTargetOfFixup];
				assert(stub != NULL && "stub not created");
				fit->binding = ld::Fixup::bindingDirectlyBound;
				fit->u.target = stub;
			}
		}
	}
	
	// switch entry point from proxy to stub
	if ( needStubForMain ) {
		const ld::Atom* mainStub = stubFor[state.entryPoint];	
		assert(mainStub != NULL);
		state.entryPoint = mainStub;
	}
	
	// sort new atoms so links are consistent
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		switch ( sect->type() ) {
			case ld::Section::typeStubHelper:
			case ld::Section::typeStub:
			case ld::Section::typeStubClose:
			case ld::Section::typeLazyPointer:
			case ld::Section::typeLazyPointerClose:
				std::sort(sect->atoms.begin(), sect->atoms.end(), AtomByNameSorter());
				break;
			default:
				break;
		}
	}

}


void doPass(const Options& opts, ld::Internal& internal)
{
	Pass  pass(opts);
	pass.process(internal);
}



} // namespace stubs 
} // namespace passes 
} // namespace ld 

