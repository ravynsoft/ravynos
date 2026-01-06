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
namespace x86_64 {
namespace classic {




class StubHelperAtom : public ld::Atom {
public:
					StubHelperAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo, 
																const ld::Atom& lazyPointer, bool forLazyDylib)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStubHelper, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo), 
				_fixup1(3, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressX86PCRel32, &lazyPointer),
				_fixup2(8, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressX86BranchPCRel32, 
						forLazyDylib ? pass.internal()->lazyBindingHelper : pass.internal()->classicBindingHelper) 
					{ pass.addAtom(*this); }
	
	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
			buffer[0]  = 0x4C;		// lea foo$lazy_ptr(%rip),%r11
			buffer[1]  = 0x8D;
			buffer[2]  = 0x1D;
			buffer[3]  = 0x00;
			buffer[4]  = 0x00;
			buffer[5]  = 0x00;
			buffer[6]  = 0x00;
			buffer[7]  = 0xE9;		// jmp dyld_stub_binding_helper
			buffer[8]  = 0x00;
			buffer[9]  = 0x00;
			buffer[10] = 0x00;
			buffer[11] = 0x00;
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd() const				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const ld::Atom&							_stubTo;
	mutable ld::Fixup						_fixup1;
	mutable ld::Fixup						_fixup2;
	
	static ld::Section						_s_section;
};

ld::Section StubHelperAtom::_s_section("__TEXT", "__stub_helper", ld::Section::typeStubHelper);



class LazyPointerAtom : public ld::Atom {
public:
											LazyPointerAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo, 
																					bool forLazyDylib, bool weakImport)
				: ld::Atom( forLazyDylib ? _s_sectionLazy : _s_section, 
							ld::Atom::definitionRegular, ld::Atom::combineNever, 
							ld::Atom::scopeTranslationUnit,
							forLazyDylib ? ld::Atom::typeLazyDylibPointer : ld::Atom::typeLazyPointer,
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(3)), 
				_stubTo(stubTo),
				_helper(pass, stubTo, *this, forLazyDylib),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian64, &_helper),
				_fixup2(0, ld::Fixup::k1of1, ld::Fixup::kindLazyTarget, &stubTo)
					{ _fixup2.weakImport = weakImport; pass.addAtom(*this);  }

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 8; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const ld::Atom&							_stubTo;
	StubHelperAtom							_helper;
	mutable ld::Fixup						_fixup1;
	mutable ld::Fixup						_fixup2;
	
	static ld::Section						_s_section;
	static ld::Section						_s_sectionLazy;
};

ld::Section LazyPointerAtom::_s_section("__DATA", "__la_symbol_ptr", ld::Section::typeLazyPointer);
ld::Section LazyPointerAtom::_s_sectionLazy("__DATA", "__ld_symbol_ptr", ld::Section::typeLazyDylibPointer);



class StubAtom : public ld::Atom {
public:
											StubAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo, 
																					bool forLazyDylib, bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(1)), 
				_stubTo(stubTo), 
				_lazyPointer(pass, stubTo, forLazyDylib, weakImport),
				_fixup(2, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressX86PCRel32, &_lazyPointer) { pass.addAtom(*this); }

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 6; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
			buffer[0] = 0xFF;		// jmp *foo$lazy_pointer(%rip)
			buffer[1] = 0x25;
			buffer[2] = 0x00;
			buffer[3] = 0x00;
			buffer[4] = 0x00;
			buffer[5] = 0x00;
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup)[1]; }

private:
	const ld::Atom&							_stubTo;
	LazyPointerAtom							_lazyPointer;
	mutable ld::Fixup						_fixup;
	
	static ld::Section						_s_section;
};

ld::Section StubAtom::_s_section("__TEXT", "__stubs", ld::Section::typeStub);


} // namespace classic
} // namespace x86_64 

