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

#ifdef __linux__
#include <algorithm>
#include <string.h>
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
namespace {
	typedef long double max_align_t;
}
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "generic_dylib_file.hpp"
#include <unordered_map>
#include <unordered_set>

namespace generic {
namespace dylib {

ExportAtom::ExportAtom(const File& f, const char* nm, const char* im, uint32_t cv, bool weakDef, bool tlv, uint64_t address)
    : ld::Atom(f._importProxySection, ld::Atom::definitionProxy,
               (weakDef ? ld::Atom::combineByName : ld::Atom::combineNever),
               ld::Atom::scopeLinkageUnit,
               (tlv ? ld::Atom::typeTLV : ld::Atom::typeUnclassified),
               symbolTableNotIn, false, false, false, ld::Atom::Alignment(0)),
      _file(&f),
      _name(nm),
      _installname(im),
      _address(address),
      _compatVersion(cv)
{}

const ld::File* ExportAtom::file() const { return _file; }
void ExportAtom::setFile(const ld::File* file) {
    _file = dynamic_cast<const generic::dylib::File *>(file);
}


ImportAtom::ImportAtom(File& f, std::vector<const char*>& imports)
    : ld::Atom(f._flatDummySection, ld::Atom::definitionRegular, ld::Atom::combineNever,
               ld::Atom::scopeTranslationUnit, ld::Atom::typeUnclassified, symbolTableNotIn, false,
               false, false, ld::Atom::Alignment(0)),
      _file(f)
{
    for(auto *name : imports)
        _undefs.emplace_back(0, ld::Fixup::k1of1, ld::Fixup::kindNone, false, strdup(name));
}

ld::File* ImportAtom::file() const { return &_file; }

bool File::_s_logHashtable = false;

File::File(const char* path, time_t mTime, ld::File::Ordinal ord, const ld::VersionSet& platforms,
              bool allowWeakImports, bool linkingFlatNamespace,
              bool hoistImplicitPublicDylibs,
              bool allowSimToMacOSX, bool addVers)
    : ld::dylib::File(path, mTime, ord),
      _importProxySection("__TEXT", "__import", ld::Section::typeImportProxies, true),
      _flatDummySection("__LINKEDIT", "__flat_dummy", ld::Section::typeLinkEdit, true),
      _providedAtom(false),
      _indirectDylibsProcessed(false),
      _platforms(platforms),
      _importAtom(nullptr),
      _parentUmbrella(nullptr),
      _swiftVersion(0),
      _linkingFlat(linkingFlatNamespace),
      _noRexports(false),
      _explictReExportFound(false),
      _implicitlyLinkPublicDylibs(hoistImplicitPublicDylibs),
      _installPathOverride(false),
      _hasWeakExports(false),
      _deadStrippable(false),
      _hasPublicInstallName(false),
      _appExtensionSafe(false),
      _isUnzipperedTwin(false),
      _allowWeakImports(allowWeakImports),
      _allowSimToMacOSXLinking(allowSimToMacOSX),
      _addVersionLoadCommand(addVers)
{
    char otherPath[PATH_MAX];
	struct stat statBuffer;
    if ( const char* support = strstr(path, "/System/iOSSupport/") ) {
        // in iOSSupport, see if there is a twin outside it
        size_t prefixLen = (support-path);
        strlcpy(otherPath, path, PATH_MAX);
        strlcpy(&otherPath[prefixLen], &support[18], PATH_MAX-prefixLen);
        _isUnzipperedTwin = ( stat(otherPath, &statBuffer) == 0 );
    }
    else if ( const char* sysLib = strstr(path, "/System/Library/") ) {
        // in iOSSupport, see if there is a twin outside it
        size_t prefixLen = (sysLib-path);
        strlcpy(otherPath, path, PATH_MAX);
        otherPath[prefixLen] = '\0';
        strlcat(otherPath, "/System/iOSSupport", PATH_MAX);
        strlcat(otherPath, sysLib, PATH_MAX);
        _isUnzipperedTwin = ( stat(otherPath, &statBuffer) == 0 );
    }
    else if ( const char* usrLib = strstr(path, "/usr/lib/") ) {
        // in iOSSupport, see if there is a twin outside it
        size_t prefixLen = (usrLib-path);
        strlcpy(otherPath, path, PATH_MAX);
        otherPath[prefixLen] = '\0';
        strlcat(otherPath, "/System/iOSSupport", PATH_MAX);
        strlcat(otherPath, usrLib, PATH_MAX);
        _isUnzipperedTwin = ( stat(otherPath, &statBuffer) == 0 );
    }
}

std::pair<bool, bool> File::hasWeakDefinitionImpl(const char* name) const
{
    const auto pos = _atoms.find(name);
    if ( pos != this->_atoms.end() )
        return std::make_pair(true, pos->second.weakDef);

    // look in re-exported libraries.
    for (const auto &dep : _dependentDylibs) {
        if ( dep.reExport ) {
            auto ret = dep.dylib->hasWeakDefinitionImpl(name);
            if ( ret.first )
                return ret;
        }
    }
    return std::make_pair(false, false);
}

bool File::hasDefinitionImpl(const char* name) const
{
    const auto pos = _atoms.find(name);
    if ( pos != this->_atoms.end() )
        return true;

    // look in re-exported libraries.
    for (const auto &dep : _dependentDylibs) {
        if ( dep.reExport ) {
            if ( dep.dylib->hasDefinitionImpl(name) )
                return true;
        }
    }
    return false;
}

bool File::hasWeakDefinition(const char* name) const
{
    // If we are supposed to ignore this export, then pretend we don't have it.
    if ( _ignoreExports.count(name) != 0 )
        return false;

    return hasWeakDefinitionImpl(name).second;
}

bool File::hasDefinition(const char* name) const
{
    // If we are supposed to ignore this export, then pretend we don't have it.
    if ( _ignoreExports.count(name) != 0 )
        return false;

    return hasDefinitionImpl(name);
}

bool File::containsOrReExports(const char* name, AtomAndWeak& atom) const
{
    if ( _ignoreExports.count(name) != 0 )
        return false;

    // check myself
    const auto pos = _atoms.find(name);
    if ( pos != _atoms.end() ) {
        atom = pos->second;
        return true;
    }

    // check dylibs I re-export
    for (const auto& dep : _dependentDylibs) {
        if ( dep.reExport && !dep.dylib->implicitlyLinked() ) {
            if ( dep.dylib->containsOrReExports(name, atom) )
                return true;
        }
    }

    return false;
}

bool File::forEachAtom(ld::File::AtomHandler& handler) const
{
    handler.doFile(*this);

    // if doing flatnamespace and need all this dylib's imports resolve
    // add atom which references alls undefines in this dylib
    if ( _importAtom != nullptr ) {
        handler.doAtom(*_importAtom);
        return true;
    }
    return false;
}

bool File::justInTimeforEachAtom(const char* name, ld::File::AtomHandler& handler) const
{
    // If we are supposed to ignore this export, then pretend we don't have it.
    if ( _ignoreExports.count(name) != 0 )
        return false;


    AtomAndWeak bucket;
    if ( containsOrReExports(name, bucket) ) {
        bucket.atom = new ExportAtom(*this, name, bucket.installname, bucket.compat_version, bucket.weakDef, bucket.tlv, bucket.address);
        _atoms[name] = bucket;
        _providedAtom = true;
        if ( _s_logHashtable )
            fprintf(stderr, "getJustInTimeAtomsFor: %s found in %s\n", name, this->path());
        // call handler with new export atom
        handler.doAtom(*bucket.atom);
        return true;
    }

    return false;
}

void File::forEachExportedSymbol(void (^handler)(const char* symbolName, bool weakDef)) const
{
    for (const auto& entry : _atoms) {
        handler(entry.first, entry.second.weakDef);
    }
}

File* File::createSyntheticDylib(const char* installName, uint32_t version) const {
    auto result = new File(this->path(), this->modificationTime(), this->ordinal(), _platforms, false, false, false, false, true);
    result->_dylibInstallPath                = installName;
    if (version) {
        result->_dylibCurrentVersion            = version;
        result->_dylibCompatibilityVersion    = version;
    } else {
        result->_dylibCurrentVersion            = this->_dylibCurrentVersion;
        result->_dylibCompatibilityVersion    = this->_dylibCompatibilityVersion;
    }
    return result;
}

void File::addExportedSymbol(const ExportAtom* atom) {
    bool tlv = atom->contentType() == ld::Atom::typeTLV;
    bool weakDef = atom->combine() == ld::Atom::combineByName;
    const_cast<ExportAtom *>(atom)->setFile(this);
    AtomAndWeak bucket = { (ld::Atom *)atom, weakDef, tlv, 0, nullptr, 0 };
    _atoms[atom->name()] = bucket;
}

void File::addExportedSymbol(const char *name, bool weakDef, bool tlv, uint64_t address) {
    const char* copiedInstallname = nullptr;
    const char* copiedName = strdup(name);
    uint32_t compat_version = 0;
    if ( strncmp(name, "$ld$", 4) == 0 ) {
        //    $ld$ <action> $ <condition> $ <symbol-name>
        const char* symAction = &name[4];
        // $ld$previous$/tmp/vers0.dylib$$1$10.0$10.4$_testSymbol$
        if (symAction != nullptr && strncmp(symAction, "previous$", 9) == 0) {
            __block uint32_t linkMinOSVersion = 0;
            __block ld::Platform linkPlatform = ld::Platform::unknown;
            this->platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
                //FIXME hack to handle symbol versioning in a zippered world.
                //This will need to be rethought
                if (linkMinOSVersion == 0) {
                    linkMinOSVersion = minVersion;
                    linkPlatform = platform;
                }
                if (platform == ld::Platform::macOS) {
                    linkMinOSVersion = minVersion;
                    linkPlatform = platform;
                }
            });
            auto nextString = [](const char*& begin, bool greedy) {
                char buffer[4096];
                const char *end;
                if (greedy) {
                    end = strrchr(begin, '$');
                } else {
                    end = strchr(begin, '$');
                }
                strlcpy(&buffer[0], begin, end-begin+1);
                begin = end+1;
                std::unique_ptr<char, decltype(&std::free)> result { strdup(&buffer[0]), &std::free };
                return result;
            };
            nextString(symAction, false);
            auto installname = nextString(symAction, false);
            auto compatVersion = nextString(symAction, false);
            auto platformStr = nextString(symAction, false);
            auto startVersion = nextString(symAction, false);
            auto endVersion = nextString(symAction, false);
            auto symbol = nextString(symAction, true);
            if ((int)linkPlatform != atoi(&*platformStr)
                || linkMinOSVersion < Options::parseVersionNumber32(&*startVersion)
                || linkMinOSVersion >=  Options::parseVersionNumber32(&*endVersion)) {
                return;
            }
            if (strlen(&*symbol) == 0) {
                this->_dylibInstallPath = strdup(&*installname);
                this->_installPathOverride = true;
                if (strlen(&*compatVersion) > 0) {
                    this->_dylibCompatibilityVersion = Options::parseVersionNumber32(&*compatVersion);
                }
                return;
            }
            compat_version = Options::parseVersionNumber32(&*compatVersion);
            copiedInstallname = strdup(&*installname);
            free((void*)copiedName);
            copiedName = strdup(&*symbol);
        }
    }

    if (!copiedInstallname && _atoms.find(copiedName) != _atoms.end()) {
        // There is already an entry for symbol (probably via an $ld$previous) and this is not an override, exit early
        return;
    }
    AtomAndWeak bucket = { nullptr, weakDef, tlv, address, copiedInstallname, compat_version };
    if ( _s_logHashtable )
        fprintf(stderr, "  adding %s to hash table for %s\n", copiedName, this->path());
    _atoms[copiedName] = bucket;
}

void File::reservedSymbolSpace(size_t size) {
    _atoms.reserve(size);
}


void File::assertNoReExportCycles(ReExportChain* prev) const
{
    // recursively check my re-exported dylibs
    ReExportChain chain = { prev, this };
    for (const auto &dep : _dependentDylibs) {
        if ( dep.reExport ) {
            auto* child = dep.dylib;
            // check child is not already in chain
            for (auto* p = prev; p != nullptr; p = p->prev) {
                if ( p->file == child ) {
                    throwf("cycle in dylib re-exports with %s and %s", child->path(), this->path());
                }
            }
            if ( dep.dylib != nullptr )
                dep.dylib->assertNoReExportCycles(&chain);
        }
    }
}

bool File::hasReExportedDependentsThatProvidedExportAtom() const
{
    bool result = this->providedExportAtom();
    for (const auto& dep : _dependentDylibs) {
        if ( dep.reExport ) {
            if ( dep.dylib->hasReExportedDependentsThatProvidedExportAtom() )
                result = true;
        }
    }
    return result;
}

void File::processIndirectLibraries(ld::dylib::File::DylibHandler* handler, bool addImplicitDylibs)
{
    // only do this once
    if ( _indirectDylibsProcessed )
        return;

    const static bool log = false;
    if ( log )
        fprintf(stderr, "processIndirectLibraries(%s)\n", this->installPath());
    if ( _linkingFlat ) {
        for (auto &dep : _dependentDylibs)
            dep.dylib = (File*)handler->findDylib(dep.path, this, false);
    }
    else if ( _noRexports ) {
        // MH_NO_REEXPORTED_DYLIBS bit set, then nothing to do
    }
    else {
        // two-level, might have re-exports
        for (auto &dep : this->_dependentDylibs) {
            if ( dep.reExport ) {
                if ( log )
                    fprintf(stderr, "processIndirectLibraries() parent=%s, child=%s\n", this->installPath(), dep.path);
                // a LC_REEXPORT_DYLIB, LC_SUB_UMBRELLA or LC_SUB_LIBRARY says we re-export this child
                dep.dylib = (File*)handler->findDylib(dep.path, this, this->speculativelyLoaded());
                if ( dep.dylib->hasPublicInstallName() ) {
                    // promote this child to be automatically added as a direct dependent if this already is
                    if ( (this->explicitlyLinked() || this->implicitlyLinked()) && (strcmp(dep.path, dep.dylib->installPath()) == 0) ) {
                        if ( log )
                            fprintf(stderr, "processIndirectLibraries() implicitly linking %s\n", dep.dylib->installPath());
                        dep.dylib->setImplicitlyLinked();
                    }
                    else if ( dep.dylib->explicitlyLinked() || dep.dylib->implicitlyLinked() ) {
                        if ( log )
                            fprintf(stderr, "processIndirectLibraries() parent is not directly linked, but child is, so no need to re-export child\n");
                    }
                    else {
                        if ( log )
                            fprintf(stderr, "processIndirectLibraries() parent is not directly linked, so parent=%s will re-export child=%s\n", this->installPath(), dep.path);
                    }
                }
                else {
                    // add all child's symbols to me
                    if ( log )
                        fprintf(stderr, "processIndirectLibraries() child is not public, so parent=%s will re-export child=%s\n", this->installPath(), dep.path);
                }
            }
            else if ( !_explictReExportFound ) {
                // see if child contains LC_SUB_FRAMEWORK with my name
                dep.dylib = (File*)handler->findDylib(dep.path, this, this->speculativelyLoaded());
                const char* parentUmbrellaName = dep.dylib->parentUmbrella();
                if ( parentUmbrellaName != nullptr ) {
                    const char* parentName = this->path();
                    const char* lastSlash = strrchr(parentName, '/');
                    if ( (lastSlash != nullptr) && (strcmp(&lastSlash[1], parentUmbrellaName) == 0) ) {
                        // add all child's symbols to me
                        dep.reExport = true;
                        if ( log )
                            fprintf(stderr, "processIndirectLibraries() umbrella=%s will re-export child=%s\n", this->installPath(), dep.path);
                    }
                }
            }
        }
    }

    // check for re-export cycles
    ReExportChain chain = { nullptr, this };
    this->assertNoReExportCycles(&chain);

    _indirectDylibsProcessed = true;
}

bool File::isPublicLocation(const char* path) const
{
    // -no_implicit_dylibs disables this optimization
    if ( ! _implicitlyLinkPublicDylibs )
        return false;

    // /usr/lib is a public location
    if ( (strncmp(path, "/usr/lib/", 9) == 0) && (strchr(&path[9], '/') == nullptr) )
        return true;

    // /System/Library/Frameworks/ is a public location
    if ( strncmp(path, "/System/Library/Frameworks/", 27) == 0 ) {
        const char* frameworkDot = strchr(&path[27], '.');
        // but only top level framework
        // /System/Library/Frameworks/Foo.framework/Versions/A/Foo                 ==> true
        // /System/Library/Frameworks/Foo.framework/Resources/libBar.dylib         ==> false
        // /System/Library/Frameworks/Foo.framework/Frameworks/Bar.framework/Bar   ==> false
        // /System/Library/Frameworks/Foo.framework/Frameworks/Xfoo.framework/XFoo ==> false
        if ( frameworkDot != nullptr ) {
            int frameworkNameLen = frameworkDot - &path[27];
            if ( strncmp(&path[strlen(path)-frameworkNameLen-1], &path[26], frameworkNameLen+1) == 0 )
                return true;
        }
    }

    return false;
}

// <rdar://problem/5529626> If only weak_import symbols are used, linker should use LD_LOAD_WEAK_DYLIB
bool File::allSymbolsAreWeakImported() const
{
    bool foundNonWeakImport = false;
    bool foundWeakImport = false;
    //fprintf(stderr, "%s:\n", this->path());
    for (const auto &it : _atoms) {
        auto* atom = it.second.atom;
        if ( atom != nullptr ) {
            if ( atom->weakImported() )
                foundWeakImport = true;
            else
                foundNonWeakImport = true;
            //fprintf(stderr, "  weak_import=%d, name=%s\n", atom->weakImported(), it->first);
        }
    }

    // don't automatically weak link dylib with no imports
    // so at least one weak import symbol and no non-weak-imported symbols must be found
    return foundWeakImport && !foundNonWeakImport;
}

} // end namespace dylib
} // end namespace generic

