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

// already in ld::passes::stubs namespace
namespace arm {
namespace classic {


class LazyPointerAtom : public ld::Atom {
public:
											LazyPointerAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
															bool forLazyDylib, bool weakImport)
				: ld::Atom(forLazyDylib ? _s_sectionLazy : _s_section, ld::Atom::definitionRegular, 
							ld::Atom::combineNever, ld::Atom::scopeLinkageUnit, ld::Atom::typeLazyPointer, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32,
						forLazyDylib ? pass.internal()->lazyBindingHelper : pass.internal()->classicBindingHelper),
				_fixup2(0, ld::Fixup::k1of1, ld::Fixup::kindLazyTarget, &stubTo)
					{ _fixup2.weakImport = weakImport; pass.addAtom(*this);  }

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const ld::Atom&							_stubTo;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	
	static ld::Section						_s_section;
	static ld::Section						_s_sectionLazy;
};

ld::Section LazyPointerAtom::_s_section("__DATA", "__la_symbol_ptr", ld::Section::typeLazyPointer);
ld::Section LazyPointerAtom::_s_sectionLazy("__DATA", "__ld_symbol_ptr", ld::Section::typeLazyDylibPointer);


class StubPICAtom : public ld::Atom {
public:
											StubPICAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
																	bool forLazyDylib, bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo), 
				_lazyPointer(pass, stubTo, forLazyDylib, weakImport),
				_fixup1(12, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, &_lazyPointer),
				_fixup2(12, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup3(12, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 12),
				_fixup4(12, ld::Fixup::k4of4, ld::Fixup::kindStoreLittleEndian32) 
				{ pass.addAtom(*this); }

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 16; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
			OSWriteLittleInt32(&buffer[ 0], 0, 0xe59fc004);	// 	ldr ip, pc + 12
			OSWriteLittleInt32(&buffer[ 4], 0, 0xe08fc00c);	// 	add ip, pc, ip
			OSWriteLittleInt32(&buffer[ 8], 0, 0xe59cf000);	// 	ldr pc, [ip]
			OSWriteLittleInt32(&buffer[12], 0, 0x00000000);	// 	.long L_foo$lazy_ptr - (L1$scv + 8)
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup4)[1]; }

private:
	const ld::Atom&							_stubTo;
	LazyPointerAtom							_lazyPointer;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
	
	static ld::Section						_s_section;
};

ld::Section StubPICAtom::_s_section("__TEXT", "__picsymbolstub4", ld::Section::typeStub);



class StubNoPICAtom : public ld::Atom {
public:
											StubNoPICAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
															bool forLazyDylib, bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo), 
				_lazyPointer(pass, stubTo, forLazyDylib, weakImport),
				_fixup(8, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32, &_lazyPointer) 
					{ pass.addAtom(*this); }

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
			OSWriteLittleInt32(&buffer[ 0], 0, 0xe59fc000);	// 	ldr ip, [pc, #0]
			OSWriteLittleInt32(&buffer[ 4], 0, 0xe59cf000);	// 	ldr pc, [ip]
			OSWriteLittleInt32(&buffer[ 8], 0, 0x00000000);	// 	.long   L_foo$lazy_ptr
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup; }
	virtual ld::Fixup::iterator				fixupsEnd()	const				{ return &((ld::Fixup*)&_fixup)[1]; }

private:
	const ld::Atom&							_stubTo;
	LazyPointerAtom							_lazyPointer;
	ld::Fixup								_fixup;
	
	static ld::Section						_s_section;
};

ld::Section StubNoPICAtom::_s_section("__TEXT", "__symbol_stub4", ld::Section::typeStub);




} // namespace classic 
} // namespace arm 

