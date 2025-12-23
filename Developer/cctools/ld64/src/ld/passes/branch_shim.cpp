/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2010-2011 Apple Inc. All rights reserved.
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
#include <string.h>
#include <unistd.h>

#include <vector>
#include <map>

#include "MachOFileAbstraction.hpp"
#include "ld.hpp"
#include "branch_shim.h"

namespace ld {
namespace passes {
namespace branch_shim {



static bool _s_log = false;


class Thumb2ToArmShimAtom : public ld::Atom {
public:
											Thumb2ToArmShimAtom(const ld::Atom* target, const ld::Section& inSect)
				: ld::Atom(inSect, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeUnclassified, 
							ld::Atom::symbolTableIn, false, true, false, ld::Atom::Alignment(2)), 
				_name(NULL),
				_target(target),
				_fixup1(8, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, target),
				_fixup2(8, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup3(8, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 8),
				_fixup4(8, ld::Fixup::k4of4, ld::Fixup::kindStoreLittleEndian32)
				 { asprintf((char**)&_name, "%s$shim", target->name()); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		// Use ARM instructions that can jump to thumb.
		assert( !  _target->isThumb() );
		if (_s_log) fprintf(stderr, "3 Thumb2 instruction shim to jump to %s\n", _target->name());
		OSWriteLittleInt32(&buffer[0], 0, 0xc004f8df);	// 	ldr  ip, pc + 4
		OSWriteLittleInt16(&buffer[4], 0, 0x44fc);		// 	add	 ip, pc, ip
		OSWriteLittleInt16(&buffer[6], 0, 0x4760);		// 	bx	 ip
		OSWriteLittleInt32(&buffer[8], 0, 0x00000000);	// 	.long target-this		
	}

	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const				{ return &((ld::Fixup*)&_fixup4)[1]; }

private:
	const char*								_name;
	const ld::Atom*							_target;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
};


class NoPICThumb2ToArmShimAtom : public ld::Atom {
public:
											NoPICThumb2ToArmShimAtom(const ld::Atom* target, const ld::Section& inSect)
				: ld::Atom(inSect, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeUnclassified, 
							ld::Atom::symbolTableIn, false, true, false, ld::Atom::Alignment(2)), 
				_name(NULL),
				_target(target),
				_fixup1(8, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32, target)
				 { asprintf((char**)&_name, "%s$shim", target->name()); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		// Use ARM instructions that can jump to thumb.
		assert( !  _target->isThumb() );
		if (_s_log) fprintf(stderr, "3 Thumb2 instruction shim to jump to %s\n", _target->name());
		OSWriteLittleInt32(&buffer[0], 0, 0xc004f8df);	// 	ldr  ip, pc + 4
		OSWriteLittleInt16(&buffer[4], 0, 0x4760);		// 	bx	 ip
		OSWriteLittleInt16(&buffer[6], 0, 0x46C0);		// 	nop
		OSWriteLittleInt32(&buffer[8], 0, 0x00000000);	// 	.long target		
	}

	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const				{ return &((ld::Fixup*)&_fixup1)[1]; }

private:
	const char*								_name;
	const ld::Atom*							_target;
	ld::Fixup								_fixup1;
};


class Thumb1ToArmShimAtom : public ld::Atom {
public:
											Thumb1ToArmShimAtom(const ld::Atom* target, const ld::Section& inSect)
				: ld::Atom(inSect, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeUnclassified, 
							ld::Atom::symbolTableIn, false, true, false, ld::Atom::Alignment(2)), 
				_name(NULL),
				_target(target),
				_fixup1(12, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, target),
				_fixup2(12, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup3(12, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 8),
				_fixup4(12, ld::Fixup::k4of4, ld::Fixup::kindStoreLittleEndian32)
				 { asprintf((char**)&_name, "%s$shim", target->name()); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 16; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		// Use ARM instructions that can jump to thumb.
		assert( ! _target->isThumb() );
		if (_s_log) fprintf(stderr, "6 Thumb1 instruction shim to jump to %s\n", _target->name());
		OSWriteLittleInt16(&buffer[ 0], 0, 0xb402);		// 	push	{r1}
		OSWriteLittleInt16(&buffer[ 2], 0, 0x4902);		// 	ldr		r1, [pc, #8]
		OSWriteLittleInt16(&buffer[ 4], 0, 0x4479);		// 	add		r1, pc
		OSWriteLittleInt16(&buffer[ 6], 0, 0x468c);		// 	mov		ip, r1
		OSWriteLittleInt16(&buffer[ 8], 0, 0xbc02);		// 	pop		{r1}
		OSWriteLittleInt16(&buffer[10], 0, 0x4760);		// 	bx		ip
		OSWriteLittleInt32(&buffer[12], 0, 0x00000000);	// 	.long target-this		
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const				{ return &((ld::Fixup*)&_fixup4)[1]; }

private:
	const char*								_name;
	const ld::Atom*							_target;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
};




class ARMtoThumbShimAtom : public ld::Atom {
public:
											ARMtoThumbShimAtom(const ld::Atom* target, const ld::Section& inSect)
				: ld::Atom(inSect, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeUnclassified, 
							ld::Atom::symbolTableIn, false, false, false, ld::Atom::Alignment(2)), 
				_name(NULL),
				_target(target),
				_fixup1(12, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, target),
				_fixup2(12, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup3(12, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 12),
				_fixup4(12, ld::Fixup::k4of4, ld::Fixup::kindStoreLittleEndian32)
				 { asprintf((char**)&_name, "%s$shim", target->name()); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 16; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		// Use ARM instructions that can jump to thumb.
		assert( _target->isThumb() );
		if (_s_log) fprintf(stderr, "4 ARM instruction shim to jump to %s\n", _target->name());
		OSWriteLittleInt32(&buffer[ 0], 0, 0xe59fc004);	// 	ldr  ip, pc + 4
		OSWriteLittleInt32(&buffer[ 4], 0, 0xe08fc00c);	// 	add	 ip, pc, ip
		OSWriteLittleInt32(&buffer[ 8], 0, 0xe12fff1c);	// 	bx	 ip
		OSWriteLittleInt32(&buffer[12], 0, 0);			// 	.long target-this		
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const				{ return &((ld::Fixup*)&_fixup4)[1]; }

private:
	const char*								_name;
	const ld::Atom*							_target;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
};


class NoPICARMtoThumbShimAtom : public ld::Atom {
public:
											NoPICARMtoThumbShimAtom(const ld::Atom* target, const ld::Section& inSect)
				: ld::Atom(inSect, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeUnclassified, 
							ld::Atom::symbolTableIn, false, false, false, ld::Atom::Alignment(2)), 
				_name(NULL),
				_target(target),
				_fixup1(8, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32, target)
				 { asprintf((char**)&_name, "%s$shim", target->name()); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		// Use ARM instructions that can jump to thumb.
		if (_s_log) fprintf(stderr, "3 ARM instruction shim to jump to %s\n", _target->name());
		OSWriteLittleInt32(&buffer[ 0], 0, 0xe59fc000);	// 	ldr  ip, pc + 4
		OSWriteLittleInt32(&buffer[ 4], 0, 0xe12fff1c);	// 	bx	 ip
		OSWriteLittleInt32(&buffer[ 8], 0, 0);			// 	.long target		
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const				{ return &((ld::Fixup*)&_fixup1)[1]; }

private:
	const char*								_name;
	const ld::Atom*							_target;
	ld::Fixup								_fixup1;
};






static void extractTarget(ld::Fixup::iterator fixup, ld::Internal& state, const ld::Atom** target)
{
	switch ( fixup->binding ) {
		case ld::Fixup::bindingNone:
			throw "unexpected bindingNone";
		case ld::Fixup::bindingByNameUnbound:
			throw "unexpected bindingByNameUnbound";
		case ld::Fixup::bindingByContentBound:
		case ld::Fixup::bindingDirectlyBound:
			*target = fixup->u.target;
			break;
		case ld::Fixup::bindingsIndirectlyBound:
			*target = state.indirectBindingTable[fixup->u.bindingIndex];
			break;
	}
}



//
// The tail-call optimization may result in a function ending in a jump (b) 
// to another functions.  At compile time the compiler does not know 
// if the target of the jump will be in the same mode (arm vs thumb).
// The arm/thumb instruction set has a way to change modes in a bl(x)
// insruction, but no instruction to change mode in a jump (b) instruction.
// In those rare cases, the linker needs to insert a shim of code to 
// make the mode switch.
//
void doPass(const Options& opts, ld::Internal& state)
{	
	// only make branch shims in final linked images
	if ( opts.outputKind() == Options::kObjectFile )
		return;

	// only ARM need branch islands
	if ( opts.architecture() != CPU_TYPE_ARM )
		return;
	
	const bool makingKextBundle = (opts.outputKind() == Options::kKextBundle);

	// scan all sections
	for (std::vector<ld::Internal::FinalSection*>::iterator sit=state.sections.begin(); sit != state.sections.end(); ++sit) {
		ld::Internal::FinalSection* sect = *sit;
		std::map<const Atom*, const Atom*> atomToThumbMap;
		std::map<const Atom*, const Atom*> thumbToAtomMap;
		std::vector<const Atom*> shims;
		// scan section for branch instructions that need to switch mode
		for (std::vector<const ld::Atom*>::iterator ait=sect->atoms.begin();  ait != sect->atoms.end(); ++ait) {
			const ld::Atom* atom = *ait;
			const ld::Atom* target = NULL;
			bool targetIsProxy;
			for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
				switch ( fit->kind ) {
					case ld::Fixup::kindStoreTargetAddressThumbBranch22:
						extractTarget(fit, state, &target);
						targetIsProxy = (target->definition() == ld::Atom::definitionProxy);
						if ( ! target->isThumb() ) {
							const uint8_t* fixUpLocation = atom->rawContentPointer();
							// <rdar://problem/9544194> don't try to scan atom for branches if atom unwilling to supply raw content
							if ( fixUpLocation == NULL )
								break;
							fixUpLocation += fit->offsetInAtom;
							uint32_t instruction = *((uint32_t*)fixUpLocation);
							bool is_b = ((instruction & 0xD000F800) == 0x9000F000);
							// need shim for branch from thumb to arm, or for call to function outside kext
							if ( is_b || (targetIsProxy && makingKextBundle) ) {
								if ( _s_log ) fprintf(stderr, "need to add thumb->arm instr=0x%08X shim to %s for %s\n", instruction, target->name(), atom->name()); 
								const Atom* shim = NULL;
								std::map<const Atom*, const Atom*>::iterator pos = thumbToAtomMap.find(target);
								if ( pos == thumbToAtomMap.end() ) {
									if ( opts.archSupportsThumb2() ) {
										// <rdar://problem/9116044> make long-branch style shims for arm kexts
										if ( makingKextBundle && opts.allowTextRelocs() )
											shim = new NoPICThumb2ToArmShimAtom(target, *sect);
										else
											shim = new Thumb2ToArmShimAtom(target, *sect);
									}
									else {
										shim = new Thumb1ToArmShimAtom(target, *sect);
									}
									shims.push_back(shim);
									thumbToAtomMap[target] = shim;
									state.atomToSection[shim] = sect;
								}
								else {
									shim = pos->second;
								}
								fit->binding = ld::Fixup::bindingDirectlyBound;
								fit->u.target = shim;
							}
						}
						break;
					case ld::Fixup::kindStoreTargetAddressARMBranch24:
						extractTarget(fit, state, &target);
						targetIsProxy = (target->definition() == ld::Atom::definitionProxy);
						if ( target->isThumb() || (targetIsProxy && makingKextBundle) ) {
							const uint8_t* fixUpLocation = atom->rawContentPointer();
							// <rdar://problem/9544194> don't try to scan atom for branches if atom unwilling to supply raw content
							if ( fixUpLocation == NULL )
								break;
							fixUpLocation += fit->offsetInAtom;
							uint32_t instruction = *((uint32_t*)fixUpLocation);
							bool is_b = ((instruction & 0x0E000000) == 0x0A000000) && ((instruction & 0xF0000000) != 0xF0000000);
							// need shim for branch from arm to thumb, or for call to function outside kext
							if ( is_b || (targetIsProxy && makingKextBundle) ) {
								if ( _s_log ) fprintf(stderr, "need to add arm->thumb instr=0x%08X shim to %s for %s\n", instruction, target->name(), atom->name()); 
								const Atom* shim = NULL;
								std::map<const Atom*, const Atom*>::iterator pos = atomToThumbMap.find(target);
								if ( pos == atomToThumbMap.end() ) {
									// <rdar://problem/9116044> make long-branch style shims for arm kexts
									if ( makingKextBundle && opts.allowTextRelocs() )
										shim = new NoPICARMtoThumbShimAtom(target, *sect);
									else
										shim = new ARMtoThumbShimAtom(target, *sect);
									shims.push_back(shim);
									atomToThumbMap[target] = shim;
									state.atomToSection[shim] = sect;
								}
								else {
									shim = pos->second;
								}
								fit->binding = ld::Fixup::bindingDirectlyBound;
								fit->u.target = shim;
							}
						}
						break;
					
					//case ld::Fixup::kindStoreARMBranch24:
					//case ld::Fixup::kindStoreThumbBranch22:
					// Note: these fixups will only be seen if the the b/bl is to a symbol plus addend
					// for now we don't handle making shims.  If a shim is needed there will
					// be an error later.
					//	break;
					default:
						break;
				}
			}
		}

		// append all new shims to end of __text
		sect->atoms.insert(sect->atoms.end(), shims.begin(), shims.end());
	}
}


} // namespace branch_shim
} // namespace passes 
} // namespace ld 
