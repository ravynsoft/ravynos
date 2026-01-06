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

// already in ld::passes::stubs namespace
namespace arm {


class FastBindingPointerAtom : public ld::Atom {
public:
											FastBindingPointerAtom(ld::passes::stubs::Pass& pass)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_fixup(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32, pass.internal()->compressedFastBinderProxy)
					{ pass.addAtom(*this); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return "fast binder pointer"; }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup)[1]; }

private:
	ld::Fixup								_fixup;
	
	static ld::Section						_s_section;
};

ld::Section FastBindingPointerAtom::_s_section("__DATA", "__nl_symbol_ptr", ld::Section::typeNonLazyPointer);


class ImageCachePointerAtom : public ld::Atom {
public:
											ImageCachePointerAtom(ld::passes::stubs::Pass& pass)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeTranslationUnit, ld::Atom::typeUnclassified,
							symbolTableIn, false, false, false, ld::Atom::Alignment(2)) { pass.addAtom(*this); }

	virtual const ld::File*					file() const					{ return NULL; }
	virtual const char*						name() const					{ return "__dyld_private"; }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }

private:

	static ld::Section						_s_section;
};

ld::Section ImageCachePointerAtom::_s_section("__DATA", "__data", ld::Section::typeUnclassified);



class StubHelperHelperAtom : public ld::Atom {
public:
											StubHelperHelperAtom(ld::passes::stubs::Pass& pass)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStubHelper, 
							ld::Atom::symbolTableIn, false, false, false, ld::Atom::Alignment(2)), 
				_fixup1(28, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, compressedImageCache(pass)),
				_fixup2(28, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup3(28, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 16),
				_fixup4(28, ld::Fixup::k4of4, ld::Fixup::kindStoreLittleEndian32),
				_fixup5(32, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, compressedFastBinder(pass)),
				_fixup6(32, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup7(32, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 28),
				_fixup8(32, ld::Fixup::k4of4, ld::Fixup::kindStoreLittleEndian32) 
					{ pass.addAtom(*this); }

	virtual ld::File*						file() const					{ return NULL; }
	virtual const char*						name() const					{ return " stub helpers"; }
	virtual uint64_t						size() const					{ return 36; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		// push lazy-info-offset
		OSWriteLittleInt32(&buffer[ 0], 0, 0xe52dc004); // str ip, [sp, #-4]!
		// push address of dyld_mageLoaderCache
		OSWriteLittleInt32(&buffer[ 4], 0, 0xe59fc010); // ldr	ip, L1
		OSWriteLittleInt32(&buffer[ 8], 0, 0xe08fc00c); // add	ip, pc, ip
		OSWriteLittleInt32(&buffer[12], 0, 0xe52dc004); // str ip, [sp, #-4]!
		// jump through _fast_lazy_bind
		OSWriteLittleInt32(&buffer[16], 0, 0xe59fc008); // ldr	ip, L2
		OSWriteLittleInt32(&buffer[20], 0, 0xe08fc00c); // add	ip, pc, ip
		OSWriteLittleInt32(&buffer[24], 0, 0xe59cf000); // ldr	pc, [ip]
		OSWriteLittleInt32(&buffer[28], 0, 0x00000000); // L1: .long fFastStubGOTAtom - (helperhelper+16)
		OSWriteLittleInt32(&buffer[32], 0, 0x00000000); // L2: .long _fast_lazy_bind - (helperhelper+28)
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const				{ return &((ld::Fixup*)&_fixup8)[1]; }

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

	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
	ld::Fixup								_fixup5;
	ld::Fixup								_fixup6;
	ld::Fixup								_fixup7;
	ld::Fixup								_fixup8;
	
	static ld::Section						_s_section;
};

ld::Section StubHelperHelperAtom::_s_section("__TEXT", "__stub_helper", ld::Section::typeStubHelper);


class StubHelperAtom : public ld::Atom {
public:
											StubHelperAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo, 
														   const ld::Atom* lazyPointer, bool stubToResolver)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStubHelper, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo),  
				_fixup1(4, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARMBranch24, helperHelper(pass, *this, stubToResolver)),
				_fixup2(8, ld::Fixup::k1of2, ld::Fixup::kindSetLazyOffset, lazyPointer),
				_fixup3(8, ld::Fixup::k2of2, ld::Fixup::kindStoreLittleEndian32) { }
	
	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 12; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[0], 0, 0xe59fc000); // ldr  ip, [pc, #0]
		OSWriteLittleInt32(&buffer[4], 0, 0xea000000); // b	_helperhelper
		OSWriteLittleInt32(&buffer[8], 0, 0);		   // .long lazy-info-offset
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
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
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	
	static ld::Section						_s_section;
};

ld::Section StubHelperAtom::_s_section("__TEXT", "__stub_helper", ld::Section::typeStubHelper);


class ResolverHelperAtom : public ld::Atom {
public:
											ResolverHelperAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo, 
																					const ld::Atom* lazyPointer)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStubHelper, 
							ld::Atom::symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo),  
				_fixup1( 4, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARMBranch24, &stubTo),
				_fixup2(32, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, lazyPointer),
				_fixup3(32, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup4(32, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 20),
				_fixup5(32, ld::Fixup::k4of4, ld::Fixup::kindStoreLittleEndian32) { }
	
	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 36; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
		OSWriteLittleInt32(&buffer[ 0], 0, 0xe92d400f); // push	{r0, r1, r2, r3, lr}
		OSWriteLittleInt32(&buffer[ 4], 0, 0xebfffffd); // bl	_foo
		OSWriteLittleInt32(&buffer[ 8], 0, 0xe59fc010); // ldr	ip, [pc, #16]
		OSWriteLittleInt32(&buffer[12], 0, 0xe08fc00c); // add	ip, pc, ip
		OSWriteLittleInt32(&buffer[16], 0, 0xe58c0000); // str	r0, [ip]
		OSWriteLittleInt32(&buffer[20], 0, 0xe1a0c000); // mov	ip, r0
		OSWriteLittleInt32(&buffer[24], 0, 0xe8bd400f); // pop	{r0, r1, r2, r3, lr}
		OSWriteLittleInt32(&buffer[28], 0, 0xe12fff1c); // bx	ip
		OSWriteLittleInt32(&buffer[32], 0, 0x00000000); // .long foo$lazyptr - helper + 20
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd() const				{ return &((ld::Fixup*)&_fixup5)[1]; }

private:
	const ld::Atom&							_stubTo;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
	ld::Fixup								_fixup5;
	
	static ld::Section						_s_section;
};

ld::Section ResolverHelperAtom::_s_section("__TEXT", "__stub_helper", ld::Section::typeStubHelper);


class LazyPointerAtom : public ld::Atom {
public:
											LazyPointerAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
															bool stubToGlobalWeakDef, bool stubToResolver, 
															bool weakImport, bool close, bool usingDataConst)
				: ld::Atom(selectSection(close, stubToGlobalWeakDef, stubToResolver, usingDataConst),
							ld::Atom::definitionRegular,
							ld::Atom::combineNever, ld::Atom::scopeLinkageUnit, ld::Atom::typeLazyPointer, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo),
				_helper(pass, stubTo, this, stubToResolver),
				_resolverHelper(pass, stubTo, this),
				_fixup1(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressLittleEndian32, 
								stubToResolver ? &_resolverHelper : (stubToGlobalWeakDef ?  &stubTo : &_helper)),
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
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	static ld::Section& selectSection(bool close, bool stubToGlobalWeakDef, bool stubToResolver, bool usingDataConst) {
		if ( close )
			return _s_sectionClose;
		else if ( stubToGlobalWeakDef && usingDataConst )
			return _s_sectionWeak;
		else if ( stubToResolver && usingDataConst )
			return _s_sectionResolver;
		else
			return _s_section;
	}

	const ld::Atom&							_stubTo;
	StubHelperAtom							_helper;
	ResolverHelperAtom						_resolverHelper;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	
	static ld::Section						_s_section;
	static ld::Section						_s_sectionClose;
	static ld::Section						_s_sectionResolver;
	static ld::Section						_s_sectionWeak;
};

ld::Section LazyPointerAtom::_s_section("__DATA", "__la_symbol_ptr", ld::Section::typeLazyPointer);
ld::Section LazyPointerAtom::_s_sectionClose("__DATA", "__lazy_symbol", ld::Section::typeLazyPointerClose);
ld::Section LazyPointerAtom::_s_sectionResolver("__DATA_DIRTY", "__la_resolver", ld::Section::typeLazyPointer);
ld::Section LazyPointerAtom::_s_sectionWeak("__DATA", "__la_weak_ptr", ld::Section::typeLazyPointer);


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
	static ld::Section						_s_sectionClose;
};

ld::Section NonLazyPointerAtom::_s_section("__DATA", "__nl_symbol_ptr", ld::Section::typeNonLazyPointer);


class StubPICKextAtom : public ld::Atom {
public:
	StubPICKextAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
					bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, true, false, ld::Atom::Alignment(2)),
				_stubTo(stubTo), 
				_nonLazyPointer(pass, stubTo, weakImport),
				_fixup1(0, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, &_nonLazyPointer),
				_fixup2(0, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup3(0, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 12),
				_fixup4(0, ld::Fixup::k4of4, ld::Fixup::kindStoreThumbLow16),
				_fixup5(4, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, &_nonLazyPointer),
				_fixup6(4, ld::Fixup::k2of4, ld::Fixup::kindSubtractTargetAddress, this),
				_fixup7(4, ld::Fixup::k3of4, ld::Fixup::kindSubtractAddend, 12),
				_fixup8(4, ld::Fixup::k4of4, ld::Fixup::kindStoreThumbHigh16) {
					asprintf((char**)&_name, "%s.stub", _stubTo.name());
					pass.addAtom(*this);
				}
  
	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 16; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
			OSWriteLittleInt32(&buffer[ 0], 0, 0x0c00f240);	// 	movw	ip, #lo(nlp - L1)
			OSWriteLittleInt32(&buffer[ 4], 0, 0x0c00f2c0);	// 	movt	ip, #hi(nlp - L1)
			OSWriteLittleInt16(&buffer[ 8], 0, 0x44fc);		// 	add		ip, pc
			OSWriteLittleInt32(&buffer[10], 0, 0xc000f8dc);	// 	ldr.w	ip, [ip]
			OSWriteLittleInt16(&buffer[14], 0, 0x4760);		// 	bx		ip
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup8)[1]; }

private:
	const ld::Atom&							_stubTo;
	const char*								_name;
	NonLazyPointerAtom						_nonLazyPointer;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;
	ld::Fixup								_fixup5;
	ld::Fixup								_fixup6;
	ld::Fixup								_fixup7;
	ld::Fixup								_fixup8;
	
	static ld::Section						_s_section;
};

ld::Section StubPICKextAtom::_s_section("__TEXT", "__stub", ld::Section::typeCode);



class StubPICAtom : public ld::Atom {
public:
											StubPICAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
														bool stubToGlobalWeakDef, bool stubToResolver, bool weakImport, bool usingDataConst)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo), 
				_lazyPointer(pass, stubTo, stubToGlobalWeakDef, stubToResolver, weakImport, false, usingDataConst),
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
														bool stubToGlobalWeakDef, bool stubToResolver, bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo), 
				_lazyPointer(pass, stubTo, stubToGlobalWeakDef, stubToResolver, weakImport, false, false),
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




class StubCloseAtom : public ld::Atom {
public:
											StubCloseAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
														bool stubToGlobalWeakDef, bool stubToResolver, bool weakImport)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub, 
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)), 
				_stubTo(stubTo), 
				_lazyPointer(pass, stubTo, stubToGlobalWeakDef, stubToResolver, weakImport, true, false),
				_fixup(0, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARMLoad12, &_lazyPointer) 
				{ pass.addAtom(*this); }

	virtual const ld::File*					file() const					{ return _stubTo.file(); }
	virtual const char*						name() const					{ return _stubTo.name(); }
	virtual uint64_t						size() const					{ return 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const {
			OSWriteLittleInt32(&buffer[ 0], 0, 0xE59FF000); // 	ldr	pc, [pc, #foo$lazy_ptr]
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

ld::Section StubCloseAtom::_s_section("__TEXT", "__symbolstub1", ld::Section::typeStubClose);


class NonLazyStubPICAtom : public ld::Atom {
public:
											NonLazyStubPICAtom(ld::passes::stubs::Pass& pass, const ld::Atom& stubTo,
														bool stubToGlobalWeakDef, bool stubToResolver, bool weakImport, bool usingDataConst)
				: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
							ld::Atom::scopeLinkageUnit, ld::Atom::typeStub,
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(2)),
				_stubTo(stubTo),
				_nonlazyPointer(pass, stubTo, weakImport),
				_fixup1(12, ld::Fixup::k1of4, ld::Fixup::kindSetTargetAddress, &_nonlazyPointer),
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
			OSWriteLittleInt32(&buffer[12], 0, 0x00000000);	// 	.long L_foo$nonlazy_ptr - (L1$scv + 8)
	}
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup4)[1]; }

private:
	const ld::Atom&							_stubTo;
	NonLazyPointerAtom						_nonlazyPointer;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;
	ld::Fixup								_fixup3;
	ld::Fixup								_fixup4;

	static ld::Section						_s_section;
};

ld::Section NonLazyStubPICAtom::_s_section("__TEXT", "__picsymbolstub5", ld::Section::typeStub);



} // namespace arm 

