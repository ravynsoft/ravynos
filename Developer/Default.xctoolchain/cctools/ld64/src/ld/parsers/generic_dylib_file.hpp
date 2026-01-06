/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef __GENERIC_DYLIB_FILE_H__
#define __GENERIC_DYLIB_FILE_H__

#include "ld.hpp"
#include "Bitcode.hpp"
#include "Options.h"
#include <unordered_map>
#include <unordered_set>

namespace generic {
namespace dylib {

// forward reference
class File;

//
// An ExportAtom has no content.  It exists so that the linker can track which
// imported symbols came from which dynamic libraries.
//
class ExportAtom final : public ld::Atom
{
public:
    ExportAtom(const File& f, const char* nm, const char* im,  uint32_t cv, bool weakDef, bool tlv, uint64_t address);
	// overrides of ld::Atom
    virtual const ld::File*	file() const override final;
	virtual const char*		name() const override final { return _name; }
	virtual uint64_t		size() const override final { return 0; }
	virtual uint64_t		objectAddress() const override final { return _address; }
	virtual void			copyRawContent(uint8_t buffer[]) const override final { }

	virtual void			setScope(Scope) { }
    virtual void            setFile(const ld::File* file) override;
    const char *            installname() const { return _installname; }
    uint32_t                compat_version() const { return _compatVersion; }
private:
	virtual					~ExportAtom() {}

	const File*		        _file;
	const char*				_name;
    const char*             _installname;
	uint64_t				_address;
    uint32_t                _compatVersion;
};


//
// An ImportAtom has no content.  It exists so that when linking a main executable flat-namespace
// the imports of all flat dylibs are checked
//
class ImportAtom final : public ld::Atom
{
public:
	ImportAtom(File& f, std::vector<const char*>& imports);

	// overrides of ld::Atom
    virtual ld::File*				file() const override final;
	virtual const char*				name() const override final { return "import-atom"; }
	virtual uint64_t				size() const override final { return 0; }
	virtual uint64_t				objectAddress() const override final { return 0; }
	virtual ld::Fixup::iterator		fixupsBegin() const	override final { return &_undefs[0]; }
	virtual ld::Fixup::iterator		fixupsEnd()	const override final { return &_undefs[_undefs.size()]; }
	virtual void					copyRawContent(uint8_t buffer[]) const override final { }

	virtual void					setScope(Scope)		{ }

private:
	virtual							~ImportAtom() {}

	File&						_file;
	mutable std::vector<ld::Fixup>	_undefs;
};

//
// A generic representation for the dynamic library files we support (Mach-O and text-based stubs).
// Common state and functionality is consolidated in this class.
//
class File : public ld::dylib::File
{
public:
	File(const char* path, time_t mTime, ld::File::Ordinal ordinal, const ld::VersionSet& platforms,
		 bool allowWeakImports, bool linkingFlatNamespace, bool hoistImplicitPublicDylibs,
		 bool allowSimToMacOSX, bool addVers);

	// overrides of ld::File
	virtual bool							forEachAtom(ld::File::AtomHandler&) const override final;
	virtual bool							justInTimeforEachAtom(const char* name, ld::File::AtomHandler&) const override final;
	virtual uint8_t							swiftVersion() const override final { return _swiftVersion; }
	virtual ld::Bitcode*					getBitcode() const override final { return _bitcode.get(); }
    virtual const ld::VersionSet &          platforms() const override final { return _platforms; }


	// overrides of ld::dylib::File
	virtual void							processIndirectLibraries(ld::dylib::File::DylibHandler*, bool addImplicitDylibs) override;
	virtual bool							providedExportAtom() const	override final { return _providedAtom; }
    virtual bool                            hasReExportedDependentsThatProvidedExportAtom() const override;
	virtual const char*						parentUmbrella() const override final { return _parentUmbrella; }
	virtual const std::vector<const char*>*	allowableClients() const override final { return _allowableClients.empty() ? nullptr : &_allowableClients; }
    virtual const std::vector<const char*>&	rpaths() const override final { return _rpaths; }
	virtual bool							hasWeakExternals() const override final	{ return _hasWeakExports; }
	virtual bool							deadStrippable() const override final { return _deadStrippable; }
	virtual bool							hasWeakDefinition(const char* name) const override final;
    virtual bool                            hasDefinition(const char* name) const override final;
	virtual bool							hasPublicInstallName() const override final { return _hasPublicInstallName; }
	virtual bool							allSymbolsAreWeakImported() const override final;
	virtual bool							installPathVersionSpecific() const override final { return _installPathOverride; }
	virtual bool							appExtensionSafe() const override final	{ return _appExtensionSafe; };
    virtual void                            forEachExportedSymbol(void (^handler)(const char* symbolName, bool weakDef)) const override;
    virtual bool						    isUnzipperedTwin() const override { return _isUnzipperedTwin; }
    File*                                   createSyntheticDylib(const char* insstallName, uint32_t version) const;
    void                                    addExportedSymbol(const ExportAtom*);
    void                                    addExportedSymbol(const char *name, bool weakDef, bool tlv, uint64_t address);
    void                                    reservedSymbolSpace(size_t size);

private:
	friend class ExportAtom;
	friend class ImportAtom;

	struct CStringHash {
		std::size_t operator()(const char* __s) const {
			unsigned long __h = 0;
			for ( ; *__s; ++__s)
				__h = 5 * __h + *__s;
			return size_t(__h);
		};
	};

protected:
    struct AtomAndWeak { ld::Atom* atom; bool weakDef; bool tlv; uint64_t address; const char * installname; uint32_t compat_version; };
	struct Dependent {
        const char* path;
        File* dylib;
        bool reExport;

        Dependent(const char* path, bool reExport)
            : path(path), dylib(nullptr), reExport(reExport) {}
    };
	struct ReExportChain { ReExportChain* prev; const File* file; };

private:
	using NameToAtomMap = std::unordered_map<const char*, AtomAndWeak, ld::CStringHash, ld::CStringEquals>;
	using NameSet = std::unordered_set<const char*, CStringHash, ld::CStringEquals>;

	std::pair<bool, bool>		hasWeakDefinitionImpl(const char* name) const;
    bool                        hasDefinitionImpl(const char* name) const;
	bool						containsOrReExports(const char* name, AtomAndWeak& atom) const;
	void						assertNoReExportCycles(ReExportChain*) const;

protected:
	bool						isPublicLocation(const char* path) const;

private:
	ld::Section							_importProxySection;
	ld::Section							_flatDummySection;
	mutable bool						_providedAtom;
	bool								_indirectDylibsProcessed;
    mutable NameToAtomMap                _atoms;
    ld::VersionSet                      _platforms;

protected:
	NameSet								_ignoreExports;
	std::vector<Dependent>				_dependentDylibs;
	ImportAtom*						    _importAtom;
	std::vector<const char*>   			_allowableClients;
	std::vector<const char*>   			_rpaths;
	const char*							_parentUmbrella;
	std::unique_ptr<ld::Bitcode>		_bitcode;
	uint8_t								_swiftVersion;
	bool								_linkingFlat;
	bool								_noRexports;
	bool								_explictReExportFound;
	bool								_implicitlyLinkPublicDylibs;
	bool								_installPathOverride;
	bool								_hasWeakExports;
	bool								_deadStrippable;
	bool								_hasPublicInstallName;
	bool								_appExtensionSafe;
    bool                                _isUnzipperedTwin;
    const bool                          _allowWeakImports;
	const bool							_allowSimToMacOSXLinking;
	const bool							_addVersionLoadCommand;

	static bool							_s_logHashtable;
};

} // end namespace dylib
} // end namespace generic

#endif // __GENERIC_DYLIB_FILE_H__
