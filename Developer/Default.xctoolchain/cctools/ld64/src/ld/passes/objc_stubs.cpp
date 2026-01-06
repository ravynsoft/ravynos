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



#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <libkern/OSByteOrder.h>

#include <algorithm>
#include <vector>

#include "Options.h"
#include "MachOFileAbstraction.hpp"
#include "Containers.h"
#include "ld.hpp"

#include "objc_stubs.h"


namespace ld {
namespace passes {
namespace objc_stubs {


//
// This pass finds callsites like "BL _objc_msgSend$foobar" and creates stub functions
// which load the selector (e.g. "foobar") and jump to _objc_msgSend.
//
// This pass may need to create new __objc_selrefs or __objc_methname atoms if they
// don't already exist for the called selector.  This pass runs before the objc pass
// that transforms method lists into relative method lists, so __objc_selrefs only
// exist for callsites.
//
class Pass {
public:
                                Pass(const Options& opts, ld::Internal& state);
    void                        process();
    void                        addAtom(const ld::Atom& atom)    { _state.addAtom(atom); }
    const char*                 selectorRefName(const ld::Atom* selRefAtom);

private:

    struct AtomByNameSorter
    {
         bool operator()(const ld::Atom* left, const ld::Atom* right)
         {
              return (strcmp(left->name(), right->name()) < 0);
         }
    };

    using SelectorMap = Map<std::string_view, const ld::Atom*>;

    const ld::Atom*              msgSendCallSite(const ld::Fixup* fixup);
    ld::Atom*                    makeObjCStub(const ld::Atom* target, const ld::Atom* msgSendGotAtom, uint32_t msgSendSlot);
    const ld::Atom*              getSelector(std::string_view selectorName);

    ld::Internal&               _state;
    const cpu_type_t            _cpuType;
    const cpu_subtype_t         _cpuSubType;
    bool                        _isLibObjC;
    bool                        _authStubs;
    bool                        _smallStubs;
    SelectorMap                 _selectorRefs;
    SelectorMap                 _selectorNames;
};


Pass::Pass(const Options& opts, ld::Internal& state)
    :   //_options(opts),
        _state(state),
        _cpuType(opts.architecture()),
        _cpuSubType(opts.subArchitecture()),
        _isLibObjC((opts.outputKind() == Options::kDynamicLibrary) && (strcmp(opts.installPath(), "/usr/lib/libobjc.A.dylib") == 0)),
        _authStubs(
    #if SUPPORT_ARCH_arm64e
            opts.useAuthenticatedStubs()
    #else
            false
    #endif
            ),
        _smallStubs(opts.objcSmallStubs())
{
} 


const ld::Atom* Pass::msgSendCallSite(const ld::Fixup* fixup)
{
    if ( fixup->binding == ld::Fixup::bindingsIndirectlyBound ) {
        const ld::Atom* target = _state.indirectBindingTable[fixup->u.bindingIndex];
        switch ( fixup->kind ) {
            case ld::Fixup::kindStoreTargetAddressX86BranchPCRel32:
            case ld::Fixup::kindStoreTargetAddressARMBranch24:
            case ld::Fixup::kindStoreTargetAddressThumbBranch22:
#if SUPPORT_ARCH_arm64
            case ld::Fixup::kindStoreTargetAddressARM64Branch26:
#endif
                assert(target != nullptr);
                // create stub if target is _objc_msgSend$<selector>
                if ( target->definition() == ld::Atom::definitionProxy ) {
                    if ( strncmp(target->name(), "_objc_msgSend$", 14) == 0 )
                        return target;
                }
                break;
            default:
                break;
        }
    }
    return nullptr;
}

static uint8_t stubAlignment(uint32_t cpuType, bool smallStub)
{
    if ( cpuType == CPU_TYPE_X86_64 )
        return 0;
    if ( smallStub )
        return 2;
    return 5; // 32-byte align fast stubs
}

class ObjcStubAtom : public ld::Atom
{
public:
            ObjcStubAtom(Pass& pass, const ld::Atom& stubTo, const ld::Atom* selector, const ld::Atom* msgSendGot,
                         uint32_t msgSendSlot, uint32_t cpuType, bool authStub, bool smallStub)
                : ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
                            ld::Atom::scopeLinkageUnit, ld::Atom::typeStub,
                            symbolTableIn, false, false, false, ld::Atom::Alignment(stubAlignment(cpuType, smallStub))),
                _stubTo(stubTo), _cpuType(cpuType), _authStubs(authStub), _smallStub(smallStub)
                    { addFixups(selector, msgSendGot, msgSendSlot); pass.addAtom(*this);  }

    virtual const ld::File*                 file() const override                    { return _stubTo.file(); }
    virtual const char*                     name() const override                    { return _stubTo.name(); }
    virtual uint64_t                        size() const override;
    virtual uint64_t                        objectAddress() const override           { return 0; }
    virtual void                            copyRawContent(uint8_t buffer[]) const override;
    virtual ld::Fixup::iterator             fixupsBegin() const override             { return &_fixups.front(); }
    virtual ld::Fixup::iterator             fixupsEnd() const override               { return &_fixups.front() + _fixups.size(); }

private:
    void                                    addFixups(const ld::Atom* selector, const ld::Atom* msgSend, uint32_t msgSendSlot);

    const ld::Atom&                          _stubTo;
    mutable std::vector<ld::Fixup>           _fixups;
    uint32_t                                 _cpuType;
    bool                                     _authStubs;
    bool                                     _smallStub;

    static ld::Section                       _s_section;
};

ld::Section ObjcStubAtom::_s_section("__TEXT", "__objc_stubs", ld::Section::typeStubObjC);


uint64_t ObjcStubAtom::size() const
{
    if ( _cpuType == CPU_TYPE_X86_64 )
        return 13;
    if ( _smallStub )
        return 12;
    return 32;
}

void ObjcStubAtom::addFixups(const ld::Atom* selector, const ld::Atom* msgSendGot, uint32_t msgSendSlot)
{
    switch ( _cpuType ) {
#if SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64e
        case CPU_TYPE_ARM64:
#endif
#if SUPPORT_ARCH_arm64_32
        case CPU_TYPE_ARM64_32:
#endif
            _fixups.emplace_back(0,  ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Page21,    selector);
            _fixups.emplace_back(4,  ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64PageOff12, selector);
            if ( _smallStub ) {
                _fixups.emplace_back(8,  ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Branch26, false, msgSendSlot);
            }
            else {
                _fixups.emplace_back(8,  ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64Page21,    msgSendGot);
                _fixups.emplace_back(12, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressARM64PageOff12, msgSendGot);
            }
            break;
        case CPU_TYPE_X86_64:
			_fixups.emplace_back(3, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressX86PCRel32, selector);
			_fixups.emplace_back(9, ld::Fixup::k1of1, ld::Fixup::kindStoreTargetAddressX86PCRel32, msgSendGot);
            break;
       default:
            throw "objc_msgSend$ rewrite not support for this arch";
    }
}


void ObjcStubAtom::copyRawContent(uint8_t buffer[]) const
{
    switch ( _cpuType ) {
#if SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64e
        case CPU_TYPE_ARM64:
            if ( _authStubs ) {
                OSWriteLittleInt32(&buffer[ 0], 0, 0x90000001);  // ADRP  X1, selector@page
                OSWriteLittleInt32(&buffer[ 4], 0, 0xF9400021);  // LDR   X1, [X1, selector@pageoff]
                OSWriteLittleInt32(&buffer[ 8], 0, 0x90000011);  // ADRP  X17, msgSegGot@page
                OSWriteLittleInt32(&buffer[12], 0, 0x91000231);  // ADD	  X17, X17, msgSegGot@pageoff
                OSWriteLittleInt32(&buffer[16], 0, 0xF9400230);  // LDR   X16, [X17]
                OSWriteLittleInt32(&buffer[20], 0, 0xD71F0A11);  // BRAA  X16, X17
                OSWriteLittleInt32(&buffer[24], 0, 0xD4200020);  // BRK
                OSWriteLittleInt32(&buffer[28], 0, 0xD4200020);  // BRK
            }
            else if ( _smallStub ) {
                OSWriteLittleInt32(&buffer[ 0], 0, 0x90000001);  // ADRP  X1, selector@page
                OSWriteLittleInt32(&buffer[ 4], 0, 0xF9400021);  // LDR   X1, [X1, selector@pageoff]
                OSWriteLittleInt32(&buffer[ 8], 0, 0x14000000);  // B     _objc_msgSend
            }
            else {
                OSWriteLittleInt32(&buffer[ 0], 0, 0x90000001);  // ADRP  X1, selector@page
                OSWriteLittleInt32(&buffer[ 4], 0, 0xF9400021);  // LDR   X1, [X1, selector@pageoff]
                OSWriteLittleInt32(&buffer[ 8], 0, 0x90000010);  // ADRP  X16, msgSegGot@page
                OSWriteLittleInt32(&buffer[12], 0, 0xF9400210);  // LDR   X16, [X16, msgSegGot@pageoff]
                OSWriteLittleInt32(&buffer[16], 0, 0xD61F0200);  // BR    X16
                OSWriteLittleInt32(&buffer[20], 0, 0xD4200020);  // BRK
                OSWriteLittleInt32(&buffer[24], 0, 0xD4200020);  // BRK
                OSWriteLittleInt32(&buffer[28], 0, 0xD4200020);  // BRK
            }
            break;
#endif
#if SUPPORT_ARCH_arm64_32
        case CPU_TYPE_ARM64_32:
            if ( _smallStub ) {
                OSWriteLittleInt32(&buffer[ 0], 0, 0x90000001);  // ADRP  X1, selector@page
                OSWriteLittleInt32(&buffer[ 4], 0, 0xB9400021);  // LDR   W1, [X1, selector@pageoff]
                OSWriteLittleInt32(&buffer[ 8], 0, 0x14000000);  // B     _objc_msgSend
            }
            else {
                OSWriteLittleInt32(&buffer[ 0], 0, 0x90000001);  // ADRP  X1, selector@page
                OSWriteLittleInt32(&buffer[ 4], 0, 0xB9400021);  // LDR   W1, [X1, selector@pageoff]
                OSWriteLittleInt32(&buffer[ 8], 0, 0x90000010);  // ADRP  X16, msgSegGot@page
                OSWriteLittleInt32(&buffer[12], 0, 0xB9400210);  // LDR   W16, [X16, msgSegGot@pageoff]
                OSWriteLittleInt32(&buffer[16], 0, 0xD61F0200);  // BR    X16
                OSWriteLittleInt32(&buffer[20], 0, 0xD4200020);  // BRK
                OSWriteLittleInt32(&buffer[24], 0, 0xD4200020);  // BRK
                OSWriteLittleInt32(&buffer[28], 0, 0xD4200020);  // BRK
            }
            break;
#endif
case CPU_TYPE_X86_64:
			buffer[0]  = 0x48;		// movq selector(%rip), $rsi
			buffer[1]  = 0x8B;
			buffer[2]  = 0x35;
			buffer[3]  = 0x00;
			buffer[4]  = 0x00;
			buffer[5]  = 0x00;
			buffer[6]  = 0x00;
            buffer[7]  = 0xFF;		// jmp *_objc_msgSend(%rip)
            buffer[8]  = 0x25;
            buffer[9]  = 0x00;
            buffer[10] = 0x00;
            buffer[11] = 0x00;
            buffer[12] = 0x00;
            break;
       default:
            throw "objc_msgSend$ rewrite not support for this arch";
    }
}

class SelRefAtom : public ld::Atom {
public:
                                            SelRefAtom(Pass& pass, const ld::Atom* target, bool is64)
                : ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineByNameAndReferences,
                            ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer,
                            ld::Atom::symbolTableNotIn, false, false, false, (is64 ? ld::Atom::Alignment(3) : ld::Atom::Alignment(2))),
                _fixup(0, ld::Fixup::k1of1, (is64 ? ld::Fixup::kindStoreTargetAddressLittleEndian64 : ld::Fixup::kindStoreTargetAddressLittleEndian32), target),
                _target(target),
                _is64(is64)
                    {  pass.addAtom(*this); }

    virtual const ld::File*                 file() const override                    { return nullptr; }
    virtual const char*                     name() const override                    { return _target->name(); }
    virtual uint64_t                        size() const override                    { return (_is64 ? 8 : 4); }
    virtual uint64_t                        objectAddress() const override           { return 0; }
    virtual void                            copyRawContent(uint8_t buffer[]) const override { }
    virtual ld::Fixup::iterator             fixupsBegin() const override             { return &_fixup; }
    virtual ld::Fixup::iterator             fixupsEnd() const override               { return &((ld::Fixup*)&_fixup)[1]; }

private:
    mutable ld::Fixup                       _fixup;
    const ld::Atom*                         _target;
    bool                                    _is64;

    static ld::Section                      _s_section;
};

ld::Section SelRefAtom::_s_section("__DATA", "__objc_selrefs", ld::Section::typeCStringPointer);


class GOTAtom : public ld::Atom {
public:
                GOTAtom(Pass& pass, const ld::Atom& gotTarget, bool is64)
				: ld::Atom(_s_section, ld::Atom::definitionRegular,
							ld::Atom::combineNever, ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer,
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(is64 ? 3 : 2)),
				_target(gotTarget), _is64(is64),
				_fixup1(0, ld::Fixup::k1of1, (is64 ? ld::Fixup::kindStoreTargetAddressLittleEndian64 : ld::Fixup::kindStoreTargetAddressLittleEndian32), &gotTarget) {
					pass.addAtom(*this);
				}

	virtual const ld::File*					file() const					{ return _target.file(); }
	virtual const char*						name() const					{ return _target.name(); }
	virtual uint64_t						size() const					{ return _is64 ? 8 : 4; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup1)[1]; }

private:
	const ld::Atom&							_target;
    bool                                    _is64;
	ld::Fixup								_fixup1;

	static ld::Section						_s_section;
};

ld::Section GOTAtom::_s_section("__DATA", "__got", ld::Section::typeNonLazyPointer);

#if SUPPORT_ARCH_arm64e
class AuthGOTAtom : public ld::Atom {
public:
                AuthGOTAtom(Pass& pass, const ld::Atom& gotTarget)
				: ld::Atom(_s_section, ld::Atom::definitionRegular,
							ld::Atom::combineNever, ld::Atom::scopeLinkageUnit, ld::Atom::typeNonLazyPointer,
							symbolTableNotIn, false, false, false, ld::Atom::Alignment(3)),
				_target(gotTarget),
                _fixup1(0, ld::Fixup::k1of2, ld::Fixup::kindSetAuthData, (ld::Fixup::AuthData){ 0, true, ld::Fixup::AuthData::ptrauth_key_asia }),
                _fixup2(0, ld::Fixup::k2of2, ld::Fixup::kindStoreTargetAddressLittleEndianAuth64, &gotTarget) {
					pass.addAtom(*this);
				}

	virtual const ld::File*					file() const					{ return _target.file(); }
	virtual const char*						name() const					{ return _target.name(); }
	virtual uint64_t						size() const					{ return 8; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const { }
	virtual void							setScope(Scope)					{ }
	virtual ld::Fixup::iterator				fixupsBegin() const				{ return (ld::Fixup*)&_fixup1; }
	virtual ld::Fixup::iterator				fixupsEnd()	const 				{ return &((ld::Fixup*)&_fixup2)[1]; }

private:
	const ld::Atom&							_target;
	ld::Fixup								_fixup1;
	ld::Fixup								_fixup2;

	static ld::Section						_s_section;
};

ld::Section AuthGOTAtom::_s_section("__DATA", "__auth_got", ld::Section::typeNonLazyPointer);
#endif // SUPPORT_ARCH_arm64e

class MethodNameAtom : public ld::Atom {
public:
                                            MethodNameAtom(Pass& pass, std::string_view name)
                : ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
                            ld::Atom::scopeLinkageUnit, ld::Atom::typeCString,
                            ld::Atom::symbolTableNotIn, false, false, false, ld::Atom::Alignment(1) ),
                 _selectorName(name)
                    {  pass.addAtom(*this); }

    virtual const ld::File*                 file() const override                   { return nullptr; }
    virtual const char*                     name() const override                   { return "selector"; }
    virtual uint64_t                        size() const override                   { return _selectorName.size() + 1;  }
    virtual uint64_t                        objectAddress() const override          { return 0; }
    virtual void                            copyRawContent(uint8_t buffer[]) const override { memcpy((char*)buffer, _selectorName.data(), _selectorName.size() + 1); }
    virtual const uint8_t*                  rawContentPointer() const override      { return (const uint8_t*)(_selectorName.data()); }

private:
    std::string_view                        _selectorName;

    static ld::Section                      _s_section;
};

ld::Section MethodNameAtom::_s_section("__TEXT", "__objc_methname", ld::Section::typeNonStdCString);


const ld::Atom* Pass::getSelector(std::string_view selectorName)
{
    const ld::Atom* selectorRefAtom = _selectorRefs[selectorName];
    if ( selectorRefAtom != nullptr )
        return selectorRefAtom;

    const ld::Atom* selectorStringAtom = _selectorNames[selectorName];
    if ( selectorStringAtom == nullptr )
        selectorStringAtom = new MethodNameAtom(*this, selectorName);

    bool is64 = ((_cpuType & CPU_ARCH_ABI64) != 0);
    selectorRefAtom = new SelRefAtom(*this, selectorStringAtom, is64);
    _selectorRefs[selectorName] = selectorRefAtom;
    return selectorRefAtom;
}

ld::Atom* Pass::makeObjCStub(const ld::Atom* target, const ld::Atom* msgSendGotAtom, uint32_t msgSendSlot)
{
    // get selector-ref atom that stub will load from
    const char* targetName = target->name();
    assert(strncmp(targetName, "_objc_msgSend$", 14) == 0);
    const char* selectorName = &targetName[14];
    const ld::Atom* selectorAtom = getSelector(selectorName);

    switch ( _cpuType ) {
#if SUPPORT_ARCH_arm64 || SUPPORT_ARCH_arm64e
        case CPU_TYPE_ARM64:
#endif
#if SUPPORT_ARCH_arm64_32
        case CPU_TYPE_ARM64_32:
#endif
        case CPU_TYPE_X86_64:
            return new ObjcStubAtom(*this, *target, selectorAtom, msgSendGotAtom, msgSendSlot, _cpuType, _authStubs, _smallStubs);
       default:
            throw "objc_msgSend$ rewrite not support for this arch";
    }
}

const char* Pass::selectorRefName(const ld::Atom* selRefAtom)
{
	ld::Fixup::iterator fit = selRefAtom->fixupsBegin();
	const ld::Atom* targetAtom = nullptr;
	switch ( fit->binding ) {
		case ld::Fixup::bindingByContentBound:
			targetAtom = fit->u.target;
			break;
		case ld::Fixup::bindingsIndirectlyBound:
            targetAtom = _state.indirectBindingTable[fit->u.bindingIndex];
			break;
		case ld::Fixup::bindingDirectlyBound:
			targetAtom = fit->u.target;
			break;
		default:
			assert(0 && "unsupported reference to selector");
	}
	assert(targetAtom != nullptr);
	assert(targetAtom->contentType() == ld::Atom::typeCString);
	return (char*)targetAtom->rawContentPointer();
}

struct StubTargetInfo {
        ld::Atom* stub = nullptr;
        std::vector<ld::Fixup*> references;
};


void Pass::process()
{
    // walk all atoms and fixups looking for calls to _objc_msgSend$Blah, and existing selectors
    const ld::Atom* msgSendAtom = _state.objcMsgSendProxy;
    Map<const ld::Atom*, StubTargetInfo> infoForAtom;
    for (ld::Internal::FinalSection* sect : _state.sections) {
        for (const ld::Atom* atom : sect->atoms) {
            if ( (sect->type() == ld::Section::typeNonStdCString) && (strcmp(sect->sectionName(), "__objc_methname") == 0) ) {
                const char* selectorString = (char*)atom->rawContentPointer();
                _selectorNames[selectorString] = atom;
            }
            else if ( (sect->type() == ld::Section::typeCStringPointer) && (strcmp(sect->sectionName(), "__objc_selrefs") == 0) ) {
                const char* selectorString = selectorRefName(atom);
                _selectorRefs[selectorString] = atom;
            }
            else if ( sect->type() == ld::Section::typeCode ) {
                if ( _isLibObjC ) {
                    // when building libobjc.dylib itself, _objc_msgSend is not a stub, but the actual implementation
                    if ( strcmp(atom->name(), "_objc_msgSend") == 0 ) {
                        msgSendAtom = atom; // switch GOT entry to point to _objc_msgSend in libobjc, rather than a proxy
                    }
                }
                for (ld::Fixup::iterator fit = atom->fixupsBegin(), end=atom->fixupsEnd(); fit != end; ++fit) {
                    if ( const ld::Atom* stubableTargetOfFixup = msgSendCallSite(fit) ) {
                        // reserve slot and add fixup reference
                        infoForAtom[stubableTargetOfFixup].references.push_back(fit);
                    }
                }
            }
        }
    }

    // short circuit if no stubs needed
    if ( infoForAtom.empty() )
        return;

    if ( msgSendAtom == nullptr )
        throw "using Objective-C but missing -lobjc";

    // objc stubs need _objc_msgSend GOT slot
    const ld::Atom* msgSendGotAtom  = nullptr;
    uint32_t        msgSendSlot     = 0;
    if ( _smallStubs ) {
        msgSendSlot = _state.objcMsgSendSlot;
    }
    else {
#if SUPPORT_ARCH_arm64e
        if ( _authStubs )
            msgSendGotAtom = new AuthGOTAtom(*this, *msgSendAtom);
        else
#endif
            msgSendGotAtom = new GOTAtom(*this, *msgSendAtom, (_cpuType & CPU_ARCH_ABI64));
    }
    
    // make objc stub atoms
    for (auto& [atom, info] : infoForAtom) {
        info.stub = makeObjCStub(atom, msgSendGotAtom, msgSendSlot);
    }

    // update fixups to use stubs instead
    for (const auto& [_, info] : infoForAtom) {
        assert(info.stub != nullptr && "stub not created");
        for (ld::Fixup* fit : info.references) {
                fit->binding = ld::Fixup::bindingDirectlyBound;
                fit->u.target = info.stub;
        }
    }


    // sort new atoms so output is reproducible
    for (ld::Internal::FinalSection* sect : _state.sections) {
        switch ( sect->type() ) {
            case ld::Section::typeStubObjC:
            case ld::Section::typeNonLazyPointer:
                // sort __objc_stubs and __got
                std::sort(sect->atoms.begin(), sect->atoms.end(), AtomByNameSorter());
                break;
            case ld::Section::typeCStringPointer:
            case ld::Section::typeNonStdCString:
                // don't need to sort because objc pass after this pass will sort
                break;
            case ld::Section::typeImportProxies:
                // remove _objc_sendMsg$Blah proxies
                sect->atoms.erase(std::remove_if(sect->atoms.begin(), sect->atoms.end(),
                    [&](const ld::Atom* atom) {
                        return infoForAtom.find(atom) != infoForAtom.end();
                    }),
                sect->atoms.end());
                break;
            default:
                break;
        }
    }

}


void doPass(const Options& opts, ld::Internal& state)
{
    if ( opts.dyldLoadsOutput() ) {
        Pass  pass(opts, state);
        pass.process();
    }
}



} // namespace objc_stubs
} // namespace passes 
} // namespace ld 

