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

#if SUPPORT_ARCH_arm64_32

// already in ld::passes::stubs namespace
namespace arm64_32 {



class FastBindingPointerAtom : public ld::Atom {
public:
											FastBindingPointerAtom(ld::passes::stubs::Pass& pass)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_fixup(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32, 
												pass.internal()->compressedFastBinderProxy)
					{ pass.addAtom(*this); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return "fast binder pointer"; }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup)[1]; }

private:
	mutable ld::Fixup						_fixup;

	static ld::Section						_s_section;
};

ld::Section FastBindingPointerAtom::_s_section("__DATA", "__got", ld::Section::typeNonLazyPointer);


class ImageCachePointerAtom : public ld::Atom {
public:
											ImageCachePointerAtom(ld::passes::stubs::Pass& pass)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeTranslationUnit, ld::Atom::typeUnclassified, 
							symbolTableIn, false, false, false, ld::Atom::Alignment(2)) { pass.addAtom(*this); }

	virtual const ld::File*					file() const					{ return NULL; }
    virtual const char*                     name() const                    { return "__dyld_private"; }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }

private:
	
	static ld::Section						_s_section;
};

ld::Section ImageCachePointerAtom::_s_section("__DATA", "__data", ld::Section::typeUnclassified);





//
//  The stub-helper-helper is the common code factored out of each helper function.
//  It is in the same section as the stub-helpers.  
//  Similar to the PLT0 entry in ELF. 
//
class StubHelperHelperAtom : public ld::Atom {
public:
											StubHelperHelperAtom(ld::passes::stubs::Pass& pass)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStubHelper, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_fixup1(0,  ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Page21,    compressedImageCache(pass)),
				_fixup2(4,  ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64PageOff12, compressedImageCache(pass)), 
				_fixup3(12, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Page21,    compressedFastBinder(pass)), 
				_fixup4(16, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64PageOff12, compressedFastBinder(pass)), 
				_fixup5(ld::Fixup::kindLinkerOptimizationHint, LOH_ARM64_ADRP_ADD, 0, 4), 
				_fixup6(ld::Fixup::kindLinkerOptimizationHint, LOH_ARM64_ADRP_LDR, 12, 16) 
					{ pass.addAtom(*this); }

	virtual ld::File*						file() const					{ return NULL; }
	virtual const char*						name() const					{ return "helper helper"; }
	virtual uint64_t						size() const					{ return 24; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[ 0], 0, 0x90000011); //     ADRP  X17, dyld_mageLoaderCache@page
		OSWriteLittleInt32(&buffer[ 4], 0, 0x91000231); //     ADD	 X17, X17, dyld_mageLoaderCache@pageoff
		OSWriteLittleInt32(&buffer[ 8], 0, 0xA9BF47F0); //     STP   X16/X17, [SP, #-16]!
		OSWriteLittleInt32(&buffer[12], 0, 0x90000010); //     ADRP  X16, _fast_lazy_bind@page
		OSWriteLittleInt32(&buffer[16], 0, 0xB9400210); //     LDR	 W16, [X16,_fast_lazy_bind@pageoff]
		OSWriteLittleInt32(&buffer[20], 0, 0xD61F0200); //     BR    X16
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const				{ return &((ld::Fixup*)&_fixup6)[1]; }

private:
	static ld::Atom* compressedImageCache(ld::passes::stubs::Pass& pass) {
		if ( pass.compressedImageCache == NULL ) 
			pass.compressedImageCache = new ImageCachePointerAtom(pass);
		return pass.compressedImageCache;		
	}
	static ld::Atom* compressedFastBinder(ld::passes::stubs::Pass& pass) {
		if ( pass.compressedFastBinderPointer == NULL ) 
			pass.compressedFastBinderPointer = new FastBindingPointerAtom(pass);
		return pass.compressedFastBinderPointer;		
	}
	
	mutable ld::Fixup						_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
	ld::Fixup								_fixup5;
	ld::Fixup								_fixup6;
	
	static ld::Section						_s_section;
};

ld::Section StubHelperHelperAtom::_s_section("__TEXT", "__stub_helper", ld::Section::typeStubHelper);



class StubHelperAtom : public ld::Atom {
public:
											StubHelperAtom(ld::passes::stubs::Pass& pass, const ld::Atom* lazyPointer,
                                                           const ld::Atom& stubTo, bool stubToResolver)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStubHelper, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)),
				_stubTo(stubTo),
				_fixup1(4, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Branch26, helperHelper(pass, *this, stubToResolver)),
				_fixup2(8, ld::Fixup::k1of2, ld::Fixup::kindSetLazyOffset, lazyPointer),
				_fixup3(8, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32)  { }
				
	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[0], 0, 0x18000050); //     LDR   W16, L0
		OSWriteLittleInt32(&buffer[4], 0, 0x14000000); //     B     helperhelper
		OSWriteLittleInt32(&buffer[8], 0, 0x00000000); // L0: .long 0
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd() const				{ return &((ld::Fixup*)&_fixup3)[1]; }

private:
    static ld::Atom* helperHelper(ld::passes::stubs::Pass& pass, StubHelperAtom& stub, bool stubToResolver) {
        // hack for resolvers in chained fixups.  StubHelper is not used by needs to be constructed, so use dummy values
        if ( stubToResolver )
            return &stub;
		if ( pass.compressedHelperHelper == NULL )
			pass.compressedHelperHelper = new StubHelperHelperAtom(pass);
		return pass.compressedHelperHelper;
	}

	const ld::Atom&							_stubTo;
	mutable ld::Fixup						_fixup1;
			ld::Fixup						_fixup2;
			ld::Fixup						_fixup3;
	
	static ld::Section						_s_section;
};

ld::Section						StubHelperAtom::_s_section("__TEXT", "__stub_helper", ld::Section::typeStubHelper);


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
		OSWriteLittleInt32(&buffer[52], 0, 0xb9000200); // str	w0, [x16]
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
		OSWriteLittleInt32(&buffer[96], 0, 0xd61f0200); // br	x16
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

ld::Section						ResolverHelperAtom::_s_section("__TEXT", "__stub_helper", ld::Section::typeStubHelper);



class LazyPointerAtom : public ld::Atom {
public:
											LazyPointerAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
															bool stubToGlobalWeakDef, bool stubToResolver, bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeTranslationUnit, ld::Atom::typeLazyPointer, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo),
				_helper(pass, this, stubTo, stubToResolver),
				_resolverHelper(pass, this, stubTo),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32, 
													stubToResolver ? &_resolverHelper : 
														(stubToGlobalWeakDef ?  &stubTo : &_helper)),
				_fixup2(0, ld::Fixup::k1of1, ld::Fixup::kindLazyTarget, &stubTo) { 
						_fixup2.weakImport = weakImport; pass.addAtom(*this); 
						if ( stubToResolver )
							pass.addAtom(_resolverHelper);
						else if ( !stubToGlobalWeakDef ) 
							pass.addAtom(_helper); 
					}

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return &_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const ld::Atom&							_stubTo;
	StubHelperAtom							_helper;
	ResolverHelperAtom						_resolverHelper;
	mutable ld::Fixup						_fixup1;
	ld::Fixup								_fixup2;
	
	static ld::Section						_s_section;
};

ld::Section LazyPointerAtom::_s_section("__DATA", "__la_symbol_ptr", ld::Section::typeLazyPointer);


class StubAtom : public ld::Atom {
public:
											StubAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
													bool stubToGlobalWeakDef, bool stubToResolver, bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)),
				_stubTo(stubTo), 
				_lazyPointer(pass, stubTo, stubToGlobalWeakDef, stubToResolver, weakImport),
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
		OSWriteLittleInt32(&buffer[4], 0, 0xB9400210); // LDR   W16, [X16, lazy_pointer@pageoff]
		OSWriteLittleInt32(&buffer[8], 0, 0xD61F0200); // BR    X16
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
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32, &stubTo) {
					_fixup1.weakImport = weakImport;
					pass.addAtom(*this);
				}

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup1)[1]; }

private:
	const ld::Atom&							_stubTo;
	ld::Fixup								_fixup1;
	
	static ld::Section						_s_section;
};

ld::Section NonLazyPointerAtom::_s_section("__DATA", "__got", ld::Section::typeNonLazyPointer);


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
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[0], 0, 0x90000010); // ADRP  X16, non_lazy_pointer@page
		OSWriteLittleInt32(&buffer[4], 0, 0xB9400210); // LDR   W16, [X16, non_lazy_pointer@pageoff]
		OSWriteLittleInt32(&buffer[8], 0, 0xD61F0200); // BR    X16
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

ld::Section NonLazyStubAtom::_s_section("__TEXT", "__stubs", ld::Section::typeStub);


} // namespace arm64_32
#endif
