/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2010-2013 Apple Inc. All rights reserved.
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

#if SUPPORT_ARCH_arm64e

// already in ld::passes::stubs namespace
namespace arm64e {


class ResolverHelperAtom : public ld::Atom {
public:
											ResolverHelperAtom(ld::passes::stubs::Pass& pass, const ld::Atom* lazyPointer,
																							const ld::Atom& stubTo)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStubHelper, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)),
				_stubTo(stubTo),
				_fixup1(40, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Branch26, &stubTo),
				_fixup2(44, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Page21, lazyPointer),
				_fixup3(48, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64PageOff12, lazyPointer) { }
				
	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 100; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[ 0], 0, 0xa9bf7bfd); // stp	fp, lr, [sp, #-16]!
		OSWriteLittleInt32(&buffer[ 4], 0, 0x910003fd); // mov	fp, sp
		OSWriteLittleInt32(&buffer[ 8], 0, 0xa9bf03e1); // stp	x1, x0, [sp, #-16]!
		OSWriteLittleInt32(&buffer[12], 0, 0xa9bf0be3); // stp	x3, x2, [sp, #-16]!
		OSWriteLittleInt32(&buffer[16], 0, 0xa9bf13e5); // stp	x5, x4, [sp, #-16]!
		OSWriteLittleInt32(&buffer[20], 0, 0xa9bf1be7); // stp	x7, x6, [sp, #-16]!
		OSWriteLittleInt32(&buffer[24], 0, 0x6dbf03e1); // stp	d1, d0, [sp, #-16]!
		OSWriteLittleInt32(&buffer[28], 0, 0x6dbf0be3); // stp	d3, d2, [sp, #-16]!
		OSWriteLittleInt32(&buffer[32], 0, 0x6dbf13e5); // stp	d5, d4, [sp, #-16]!
		OSWriteLittleInt32(&buffer[36], 0, 0x6dbf1be7); // stp	d7, d6, [sp, #-16]!
		OSWriteLittleInt32(&buffer[40], 0, 0x94000000); // bl	_foo
		OSWriteLittleInt32(&buffer[44], 0, 0x90000010); // adrp	x16, lazy_pointer@PAGE
		OSWriteLittleInt32(&buffer[48], 0, 0x91000210); // add	x16, x16, lazy_pointer@PAGEOFF
		OSWriteLittleInt32(&buffer[52], 0, 0xf9000200); // str	x0, [x16]
		OSWriteLittleInt32(&buffer[56], 0, 0xaa0003f0); // mov	x16, x0
		OSWriteLittleInt32(&buffer[60], 0, 0x6cc11be7); // ldp	d7, d6, [sp], #16
		OSWriteLittleInt32(&buffer[64], 0, 0x6cc113e5); // ldp	d5, d4, [sp], #16
		OSWriteLittleInt32(&buffer[68], 0, 0x6cc10be3); // ldp	d3, d2, [sp], #16
		OSWriteLittleInt32(&buffer[72], 0, 0x6cc103e1); // ldp	d1, d0, [sp], #16
		OSWriteLittleInt32(&buffer[76], 0, 0xa8c11be7); // ldp	x7, x6, [sp], #16
		OSWriteLittleInt32(&buffer[80], 0, 0xa8c113e5); // ldp	x5, x4, [sp], #16
		OSWriteLittleInt32(&buffer[84], 0, 0xa8c10be3); // ldp	x3, x2, [sp], #16
		OSWriteLittleInt32(&buffer[88], 0, 0xa8c103e1); // ldp	x1, x0, [sp], #16
		OSWriteLittleInt32(&buffer[92], 0, 0xa8c17bfd); // ldp	fp, lr, [sp], #16
		OSWriteLittleInt32(&buffer[96], 0, 0xD61F0A1F); // braaz x16
	}

	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd() const				{ return &((ld::Fixup*)&_fixup3)[1]; }

private:

	const ld::Atom&							_stubTo;
	mutable ld::Fixup						_fixup1;
			ld::Fixup						_fixup2;
			ld::Fixup						_fixup3;
	
	static ld::Section						_s_section;
};

ld::Section						ResolverHelperAtom::_s_section("__TEXT", "__resolver_help", ld::Section::typeStubHelper);



class LazyPointerAtom : public ld::Atom {
public:
											LazyPointerAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
															bool stubToGlobalWeakDef, bool stubToResolver, bool weakImport, bool dataConstUsed)
				: ld::Atom(selectSection(stubToGlobalWeakDef, stubToResolver, dataConstUsed),
							ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeTranslationUnit, ld::Atom::typeNonLazyPointer, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(3)), 
				_stubTo(stubTo),
				_resolverHelper(pass, this, stubTo),
				_fixup1(0, ld::Fixup::k1of2, ld::Fixup::kindSetAuthData, (ld::Fixup::AuthData){ 0, false, ld::Fixup::AuthData::ptrauth_key_asia }),
				_fixup2(0, ld::Fixup::k2of2, ld::Fixup::kindStoreTargetAddressLittleEndianAuth64, stubToResolver ? &_resolverHelper : &stubTo) {
                        assert(stubToResolver || stubToGlobalWeakDef);
						_fixup2.weakImport = weakImport;
                        pass.addAtom(*this);
						if ( stubToResolver )
							pass.addAtom(_resolverHelper);
					}

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 8; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	static ld::Section& selectSection(bool stubToGlobalWeakDef, bool stubToResolver, bool dataConstUsed) {
		if ( stubToGlobalWeakDef && dataConstUsed )
			return _s_sectionWeak;
		else if ( stubToResolver && dataConstUsed )
			return _s_sectionResolver;
		else
			return _s_section;
	}

	const ld::Atom&							_stubTo;
	ResolverHelperAtom						_resolverHelper;
	mutable ld::Fixup						_fixup1;
	ld::Fixup								_fixup2;

	static ld::Section						_s_section;
	static ld::Section						_s_sectionResolver;
	static ld::Section						_s_sectionWeak;
};

ld::Section LazyPointerAtom::_s_section("__DATA", "__la_symbol_ptr", ld::Section::typeNonLazyPointer);
ld::Section LazyPointerAtom::_s_sectionResolver("__DATA_DIRTY", "__la_resolver", ld::Section::typeNonLazyPointer);
ld::Section LazyPointerAtom::_s_sectionWeak("__DATA", "__la_weak_ptr", ld::Section::typeNonLazyPointer);


class StubAtom : public ld::Atom {
public:
											StubAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
													bool stubToGlobalWeakDef, bool stubToResolver, bool weakImport, bool dataConstUsed)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)),
				_stubTo(stubTo), 
				_lazyPointer(pass, stubTo, stubToGlobalWeakDef, stubToResolver, weakImport, dataConstUsed),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Page21, &_lazyPointer),
				_fixup2(4, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64PageOff12, &_lazyPointer),
				_fixup3(ld::Fixup::kindLinkerOptimizationHint, LOH_ARM64_ADRP_LDR, 0, 4) 
					{ pass.addAtom(*this); 	}

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[0], 0, 0x90000010); // ADRP  X16, lazy_pointer@page
		OSWriteLittleInt32(&buffer[4], 0, 0xF9400210); // LDR   X16, [X16, lazy_pointer@pageoff]
		OSWriteLittleInt32(&buffer[8], 0, 0xD61F0A1F); // BRAAZ X16
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup3)[1]; }

private:
	const ld::Atom&							_stubTo;
	LazyPointerAtom							_lazyPointer;
	mutable ld::Fixup						_fixup1;
	mutable ld::Fixup						_fixup2;
	mutable ld::Fixup						_fixup3;
	
	static ld::Section						_s_section;
};

ld::Section StubAtom::_s_section("__TEXT", "__stubs", ld::Section::typeStub);



class NonLazyPointerAtom : public ld::Atom {
public:
	NonLazyPointerAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
					   bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, 
							ld::Atom::combineNever, ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(3)), 
				_stubTo(stubTo),
	_fixup1(0, ld::Fixup::k1of2, ld::Fixup::kindSetAuthData, (ld::Fixup::AuthData){ 0, true, ld::Fixup::AuthData::ptrauth_key_asia }),
	_fixup2(0, ld::Fixup::k2of2, ld::Fixup::kindStoreTargetAddressLittleEndianAuth64, &stubTo) {
					_fixup2.weakImport = weakImport;
					pass.addAtom(*this);
				}

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 8; }
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
};

ld::Section NonLazyPointerAtom::_s_section("__DATA", "__auth_got", ld::Section::typeNonLazyPointer);


class NonLazyStubAtom : public ld::Atom {
public:
				NonLazyStubAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
								bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)),
				_stubTo(stubTo), 
				_nonLazyPointer(pass, stubTo, weakImport),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Page21, &_nonLazyPointer),
				_fixup2(4, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64PageOff12, &_nonLazyPointer) { 
					asprintf((char**)&_name, "%s.stub", _stubTo.name());
					pass.addAtom(*this);
				}

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 16; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[ 0], 0, 0x90000011); // ADRP  X17, dyld_mageLoaderCache@page
		OSWriteLittleInt32(&buffer[ 4], 0, 0x91000231); // ADD	 X17, X17, dyld_mageLoaderCache@pageoff
		OSWriteLittleInt32(&buffer[ 8], 0, 0xF9400230); // LDR   X16, [X17]
		OSWriteLittleInt32(&buffer[12], 0, 0xD71F0A11); // BRAA  X16, X17
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const ld::Atom&							_stubTo;
	const char*								_name;
	NonLazyPointerAtom						_nonLazyPointer;
	mutable ld::Fixup						_fixup1;
	mutable ld::Fixup						_fixup2;
	
	static ld::Section						_s_section;
};

ld::Section NonLazyStubAtom::_s_section("__TEXT", "__auth_stubs", ld::Section::typeStub);


} // namespace arm64e
#endif
