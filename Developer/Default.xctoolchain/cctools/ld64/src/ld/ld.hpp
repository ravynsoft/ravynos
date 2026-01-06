/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2005-2010 Apple Inc. All rights reserved.
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


#ifndef __LD_HPP__
#define __LD_HPP__

#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>

#include <set>
#include <map>
#include <vector>
#include <string>
#include <unordered_set>

#include "configure.h"
#include "PlatformSupport.h"

//FIXME: Only needed until we move VersionSet into PlatformSupport
class Options;

namespace ld {

//
// minumum OS versions
//

struct PlatformVersion {
	Platform platform;
	uint32_t minVersion;
	uint32_t sdkVersion;
	PlatformVersion(Platform P) : PlatformVersion(P, 0, 0) {}
	PlatformVersion(Platform P, uint32_t V) : PlatformVersion(P, V, V) {}
	PlatformVersion(Platform P, uint32_t M, uint32_t S) : platform(P), minVersion(M), sdkVersion(S) {}
	bool operator==(const PlatformVersion& other) const { return platform == other.platform; }
	bool operator<(const PlatformVersion& other) const { return platform < other.platform; }
};

struct VersionSet {
private:
	std::set<PlatformVersion> _versions;
public:
	VersionSet() {}
	VersionSet(const std::set<PlatformVersion>& V) : _versions(V) {}
	void insert(PlatformVersion platformVersion) {
		assert(_versions.find(platformVersion) == _versions.end());
		_versions.insert(platformVersion);
	}
	void erase(const Platform& platform) {
		auto i = std::find_if(_versions.begin(), _versions.end(), [&platform](const PlatformVersion& version) {
			return platform == version.platform;
		});
		if (i == _versions.end()) return;
		_versions.erase(i);
	}
	void updateMinVersion(const Platform& platform, uint32_t minVersion) {
		auto i = std::find_if(_versions.begin(), _versions.end(), [&platform](const PlatformVersion& version) {
			return platform == version.platform;
		});
		if (i == _versions.end()) return;
		auto newVersion = *i;
		newVersion.minVersion = minVersion;
		newVersion.sdkVersion = i->sdkVersion;
		_versions.erase(i);
		_versions.insert(newVersion);
	}
	void updateSDKVersion(const Platform& platform, uint32_t sdkVersion) {
		auto i = std::find_if(_versions.begin(), _versions.end(), [&platform](const PlatformVersion& version) {
			return platform == version.platform;
		});
		if (i == _versions.end()) return;
		auto newVersion = *i;
		newVersion.minVersion = i->minVersion;
		newVersion.sdkVersion = sdkVersion;
		_versions.erase(i);
		_versions.insert(newVersion);
	}
	size_t count() const { return _versions.size(); }
	size_t empty() const { return _versions.empty(); }
	void   clear() { _versions.clear(); }

	void forEach(void (^callback)(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop)) const {
		bool stop = false;
		for (const auto& version : _versions) {
			callback(version.platform, version.minVersion, version.sdkVersion, stop);
			if (stop)
				return;
		}
	}

	bool contains(ld::Platform platform) const {
		auto i = std::find_if(_versions.begin(), _versions.end(), [&platform](const PlatformVersion& version) {
			return platform == version.platform;
		});
		return (i != _versions.end());
	}

	bool contains(const ld::PlatformSet& platforms) const {
		__block bool retval = true;
		forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
			if (platforms.find(platform) == platforms.end()) {
				stop = true;
				retval = false;
			}
		});
		return retval;
	}

	uint32_t minOS(ld::Platform platform) const {
		for (const auto& version : _versions) {
			if (basePlatform(version.platform) == platform) {
				return version.minVersion;
			}
		}
		return 0;
	}

	bool minOS(const PlatformVersion& version) const {
		return minOS(version.platform) >= version.minVersion;
	}

	bool minOS(const ld::VersionSet& requiredMinVersions) const {
		__block bool retval = true;
		forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
			if (!requiredMinVersions.contains(basePlatform(platform)))
				return;
			if (minVersion < requiredMinVersions.minOS(basePlatform(platform))) {
				stop = true;
				retval = false;
			}
		});
		return retval;
	}

	std::string to_str() const {
		std::string retval;
		auto appendPlatform = [&](const std::string& platform) {
			if (retval.empty()) {
				retval = platform;
			} else {
				retval += "/";
				retval += platform;
			}
		};

		forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
			appendPlatform(nameFromPlatform(platform));
		});

		return retval;
	}

	void checkObjectCrosslink(const VersionSet& objectPlatforms, const std::string& targetPath, bool internalSDK,
							  bool bitcode, bool platformMismatchesAreWarning) const;
	void checkDylibCrosslink(const VersionSet& dylibPlatforms, const std::string& targetPath,
										 const std::string& dylibType, bool internalSDK, bool indirectDylib,
										 bool bitcode, bool isUnzipperedTwin, const char* installName,
										 bool fromSDK, bool platformMismatchesAreWarning) const;
	bool operator==(const VersionSet& other) const { return _versions == other._versions; }
	bool operator<(const VersionSet& other) const { return _versions < other._versions; }
};

static const PlatformVersion mac10_4		(Platform::macOS, 0x000A0400);
static const PlatformVersion mac10_5		(Platform::macOS, 0x000A0500);
static const PlatformVersion mac10_6 		(Platform::macOS, 0x000A0600);
static const PlatformVersion mac10_7 		(Platform::macOS, 0x000A0700);
static const PlatformVersion mac10_8 		(Platform::macOS, 0x000A0800);
static const PlatformVersion mac10_9 		(Platform::macOS, 0x000A0900);
static const PlatformVersion mac10_12 		(Platform::macOS, 0x000A0C00);
static const PlatformVersion mac10_14 		(Platform::macOS, 0x000A0E00);
static const PlatformVersion mac10_15		(Platform::macOS, 0x000A0F00);
static const PlatformVersion mac10_16		(Platform::macOS, 0x000A1000);
static const PlatformVersion mac10_Future 	(Platform::macOS, 0x10000000);

static const PlatformVersion iOS_2_0 		(Platform::iOS, 0x00020000);
static const PlatformVersion iOS_3_1 		(Platform::iOS, 0x00030100);
static const PlatformVersion iOS_4_2 		(Platform::iOS, 0x00040200);
static const PlatformVersion iOS_4_3 		(Platform::iOS, 0x00040300);
static const PlatformVersion iOS_5_0 		(Platform::iOS, 0x00050000);
static const PlatformVersion iOS_6_0 		(Platform::iOS, 0x00060000);
static const PlatformVersion iOS_7_0 		(Platform::iOS, 0x00070000);
static const PlatformVersion iOS_8_0 		(Platform::iOS, 0x00080000);
static const PlatformVersion iOS_9_0 		(Platform::iOS, 0x00090000);
static const PlatformVersion iOS_10_0 		(Platform::iOS, 0x000A0000);
static const PlatformVersion iOS_11_0 		(Platform::iOS, 0x000B0000);
static const PlatformVersion iOS_12_0 		(Platform::iOS, 0x000C0000);
static const PlatformVersion iOS_13_0 		(Platform::iOS, 0x000D0000);
static const PlatformVersion iOS_14_0 		(Platform::iOS, 0x000E0000);
static const PlatformVersion iOS_Future 	(Platform::iOS, 0x10000000);

static const PlatformVersion watchOS_1_0 		(Platform::watchOS, 0x00010000);
static const PlatformVersion watchOS_2_0 		(Platform::watchOS, 0x00020000);
static const PlatformVersion watchOS_5_0 		(Platform::watchOS, 0x00050000);
static const PlatformVersion watchOS_6_0 		(Platform::watchOS, 0x00060000);
static const PlatformVersion watchOS_7_0 		(Platform::watchOS, 0x00070000);
static const PlatformVersion watchOS_Future		(Platform::watchOS, 0x10000000);

static const PlatformVersion tvOS_9_0 			(Platform::tvOS, 0x00090000);
static const PlatformVersion tvOS_12_0 			(Platform::tvOS, 0x000C0000);
static const PlatformVersion tvOS_13_0 			(Platform::tvOS, 0x000D0000);
static const PlatformVersion tvOS_14_0 			(Platform::tvOS, 0x000E0000);
static const PlatformVersion tvOS_Future		(Platform::tvOS, 0x10000000);

static const PlatformVersion bridgeOS_1_0 			(Platform::bridgeOS, 0x00010000);
static const PlatformVersion bridgeOS_4_0 			(Platform::bridgeOS, 0x00040000);
static const PlatformVersion bridgeOS_5_0 			(Platform::bridgeOS, 0x00050000);
static const PlatformVersion bridgeOS_Future		(Platform::bridgeOS, 0x10000000);


// Platform Sets

static const PlatformSet simulatorPlatforms ( {Platform::iOS_simulator, Platform::tvOS_simulator, Platform::watchOS_simulator} );

//FIXME do we need to add simulatots to these?
//FIXME Are the dates correct?
static const VersionSet version2007		({mac10_4, iOS_2_0});
static const VersionSet version2008 	({mac10_5, iOS_2_0});
static const VersionSet version2008Fall ({mac10_5, iOS_3_1});
static const VersionSet version2009 	({mac10_6, iOS_3_1});
static const VersionSet version2010 	({mac10_7, iOS_4_2});
static const VersionSet version2010Fall ({mac10_7, iOS_4_3});

static const VersionSet version2012 	({mac10_8, iOS_6_0});
static const VersionSet version2013 	({mac10_9, iOS_7_0});
static const VersionSet version2019Fall ({mac10_15, iOS_13_0, watchOS_6_0, tvOS_13_0, bridgeOS_4_0});
static const VersionSet version2020Fall ({mac10_16, iOS_14_0, watchOS_7_0, tvOS_14_0, bridgeOS_5_0});

static const VersionSet supportsSplitSegV2 		({mac10_12, iOS_9_0, watchOS_2_0, tvOS_9_0});
// FIXME: Use the comment out line instead.
static const VersionSet supportsLCBuildVersion 	({mac10_14, iOS_12_0, watchOS_5_0, tvOS_12_0, bridgeOS_1_0});
static const VersionSet supportsPIE				({mac10_5, iOS_4_2});
static const VersionSet supportsTLV  			({mac10_7, iOS_9_0});
static const VersionSet supportsChainedFixups 	({mac10_16, iOS_14_0, watchOS_7_0, tvOS_14_0, bridgeOS_Future});

// Forward declaration for bitcode support
class Bitcode;

//
// ld::File 
//
// Abstract base class for all object or library files the linker processes.
// 
// forEachAtom() iterates over the Atoms in the order they occur in the file.
//
// justInTimeforEachAtom(name) iterates over lazily created Atoms.  For instance if
// File is a static library, justInTimeforEachAtom() will iterate over the base set
// of Atoms from the archive member implementing 'name'.
//
class File
{
public:

	class AtomHandler {
	public:
		virtual				~AtomHandler() {}
		virtual void		doAtom(const class Atom&) = 0;
		virtual void		doFile(const class File&) = 0;
	};

	//
	// ld::File::Ordinal 
	//
	// Codifies the rules of ordering input files for symbol precedence. These are:
	// - Input files listed on the command line are ordered according to their index in the argument list.
	// - Input files listed in a file list are ordered first at the index of the file list argument, then
	//   by index in the file list
	// - Input files extracted from archives are ordered using the ordinal of the archive itself plus the
	//   index of the object file within the archive
	// - Indirect dylibs are ordered after all input files derived from the command line, in the order that
	//   they are discovered.
	// - The LTO object file is last.
	//
	class Ordinal
	{
	private:
		// The actual numeric ordinal. Lower values have higher precedence and a zero value is invalid.
		// The 64 bit ordinal is broken into 4 16 bit chunks. The high 16 bits are a "partition" that
		// is used to distinguish major ordinal groups: command line, indirect dylib, LTO.
		// The remaining chunks are used according to the partition (see below).
		uint64_t	_ordinal;
		
		Ordinal (uint64_t ordinal) : _ordinal(ordinal) {}
		
		enum { kArgListPartition=0, kIndirectDylibPartition=1, kLTOPartition = 2, kLinkerOptionPartition = 3, InvalidParition=0xffff };
		Ordinal(uint16_t partition, uint16_t majorIndex, uint16_t minorIndex, uint16_t counter) {
			_ordinal = ((uint64_t)partition<<48) | ((uint64_t)majorIndex<<32) | ((uint64_t)minorIndex<<16) | ((uint64_t)counter<<0);
		}
		
		const uint16_t	partition() const		{ return (_ordinal>>48)&0xffff; }
		const uint16_t	majorIndex() const		{ return (_ordinal>>32)&0xffff; }
		const uint16_t	minorIndex() const		{ return (_ordinal>>16)&0xffff; }
		const uint16_t	counter() const			{ return (_ordinal>>00)&0xffff; }

		const Ordinal nextMajorIndex()		const { assert(majorIndex() < 0xffff); return Ordinal(_ordinal+((uint64_t)1<<32)); }
		const Ordinal nextMinorIndex()		const { assert(minorIndex() < 0xffff); return Ordinal(_ordinal+((uint64_t)1<<16)); }
		const Ordinal nextCounter()		const { assert(counter() < 0xffff); return Ordinal(_ordinal+((uint64_t)1<<0)); }
		
	public:
		Ordinal() : _ordinal(0) {};

		static const Ordinal NullOrdinal()		{ return Ordinal((uint64_t)0); }
		
		const bool validOrdinal() const { return _ordinal != 0; }
		
		bool operator ==(const Ordinal& rhs) const { return _ordinal == rhs._ordinal; }
		bool operator !=(const Ordinal& rhs) const {	return _ordinal != rhs._ordinal; }
		bool operator < (const Ordinal& rhs) const { return _ordinal < rhs._ordinal; }
		bool operator > (const Ordinal& rhs) const { return _ordinal > rhs._ordinal; }
		
		// For ordinals derived from the command line args the partition is ArgListPartition
		// The majorIndex is the arg index that pulls in the file, file list, or archive.
		// The minorIndex is used for files pulled in by a file list and the value is the index of the file in the file list.
		// The counter is used for .a files and the value is the index of the object in the archive.
		// Thus, an object pulled in from a .a that was listed in a file list could use all three fields.
		static const Ordinal makeArgOrdinal(uint16_t argIndex) { return Ordinal(kArgListPartition, argIndex, 0, 0); };
		const Ordinal nextFileListOrdinal() const { return nextMinorIndex(); }
		const Ordinal archiveOrdinalWithMemberIndex(uint16_t memberIndex) const { return Ordinal(partition(), majorIndex(), minorIndex(), memberIndex); }
		
		// For indirect libraries the partition is IndirectDylibPartition and the counter is used or order the libraries.
		static const ld::File::Ordinal indirectDylibBase() { return Ordinal(kIndirectDylibPartition, 0, 0, 0); }
		const Ordinal nextIndirectDylibOrdinal() const { return nextCounter(); }
		
		// For the LTO mach-o the partition is LTOPartition. As there is only one LTO file no other fields are needed.
		static const ld::File::Ordinal LTOOrdinal()			{ return Ordinal(kLTOPartition, 0, 0, 0); }

		// For linker options embedded in object files
		static const ld::File::Ordinal linkeOptionBase() { return Ordinal(kIndirectDylibPartition, 1, 0, 0); }
		const Ordinal nextLinkerOptionOrdinal() { return nextCounter(); };

	};
	
	typedef enum { Reloc, Dylib, Archive, Other } Type;
	
										File(const char* pth, time_t modTime, Ordinal ord, Type type)
											: _path(pth), _modTime(modTime), _ordinal(ord), _type(type) { }
	virtual								~File() {}
			const char*					path() const			{ return _path; }
			time_t						modificationTime() const{ return _modTime; }
	Ordinal								ordinal() const			{ return _ordinal; }
	virtual bool						forEachAtom(AtomHandler&) const = 0;
	virtual bool						justInTimeforEachAtom(const char* name, AtomHandler&) const = 0;
	virtual uint8_t						swiftVersion() const	{ return 0; }		// ABI version, now fixed
	virtual uint16_t					swiftLanguageVersion() const	{ return 0; }	// language version in 4.4 format
	virtual uint32_t					cpuSubType() const		{ return 0; }
	virtual uint8_t						cpuSubTypeFlags() const	{ return 0; }
	virtual uint32_t					subFileCount() const	{ return 1; }
	virtual const VersionSet&			platforms() const		{ return _platforms; }
    bool								fileExists() const     { return _modTime != 0; }
	Type								type() const { return _type; }
	virtual Bitcode*					getBitcode() const		{ return NULL; }
	const char*							leafName() const;

private:
	const char*							_path;
	time_t								_modTime;
	const Ordinal						_ordinal;
	const Type							_type;
	// Note this is just a placeholder as platforms() needs something to return
	static const VersionSet				_platforms;
};


inline const char* File::leafName() const {
	const char* pth = this->path();
	if ( pth == NULL )
		return "<internal>";
	const char* lastSlash = strrchr(pth, '/');
	return (lastSlash != NULL) ? lastSlash+1 : pth;
}


namespace relocatable {
	//
	// ld::relocatable::File 
	//
	// Abstract base class for object files the linker processes.
	// 
	// debugInfo() returns if the object file contains debugger information (stabs or dwarf).
	//
	// stabs() lazily creates a vector of Stab objects for each atom
	//
	// canScatterAtoms() true for all compiler generated code.  Hand written assembly can opt-in
	// via .subsections_via_symbols directive.  When true it means the linker can break up section
	// content at symbol boundaries and do optimizations like coalescing, dead code stripping, or
	// apply order files.
	//
	// optimize() used by libLTO to lazily generate code from llvm bit-code files
	// 
	class File : public ld::File
	{
	public:
		enum DebugInfoKind { kDebugInfoNone=0, kDebugInfoStabs=1, kDebugInfoDwarf=2, kDebugInfoStabsUUID=3 };
		enum SourceKind { kSourceUnknown=0, kSourceObj, kSourceLTO, kSourceArchive, kSourceCompilerArchive };
		struct Stab {
			const class Atom*	atom;
			uint8_t				type;
			uint8_t				other;
			uint16_t			desc;
			uint32_t			value;
			const char*			string;
		};
		typedef const std::vector< std::vector<const char*> > LinkerOptionsList;
		typedef std::vector<std::pair<uint32_t,uint32_t>> ToolVersionList;
		struct AstTimeAndPath { uint64_t time; std::string path; };

											File(const char* pth, time_t modTime, Ordinal ord)
												: ld::File(pth, modTime, ord, Reloc) { }
		virtual								~File() {}
		virtual DebugInfoKind				debugInfo() const = 0;
		virtual const char*					debugInfoPath() const { return path(); }
		virtual time_t						debugInfoModificationTime() const { return modificationTime(); }
		virtual const std::vector<Stab>*	stabs() const = 0;
		virtual bool						canScatterAtoms() const = 0;
		virtual bool						hasLongBranchStubs()		{ return false; }
		virtual bool						hasllvmProfiling() const    { return false; }
		virtual bool  						hasObjC() const				{ return false; }
		virtual bool						objcHasCategoryClassPropertiesField() const { return false; }
		virtual LinkerOptionsList*			linkerOptions() const = 0;
		virtual const ToolVersionList&		toolVersions() const = 0;
		virtual SourceKind					sourceKind() const { return kSourceUnknown; }
		virtual const uint8_t*				fileContent() const { return nullptr; }
		virtual const std::vector<AstTimeAndPath>*	astFiles() const { return nullptr; }
		virtual void						forEachLtoSymbol(void (^handler)(const char*)) const { }
	};
} // namespace relocatable


namespace dylib {

	//
	// ld::dylib::File 
	//
	// Abstract base class for dynamic shared libraries read by the linker processes.
	//
	class File : public ld::File
	{
	public:
		class DylibHandler
		{
		public:
			virtual				~DylibHandler()	{}
			virtual File*		findDylib(const char* installPath, const ld::dylib::File* fromDylib, bool speculative) = 0;
		};
			
											File(const char* pth, time_t modTime, Ordinal ord)
												: ld::File(pth, modTime, ord, Dylib), _dylibInstallPath(NULL), _frameworkName(NULL),
												_dylibTimeStamp(0), _dylibCurrentVersion(0), _dylibCompatibilityVersion(0),
												_explicitlyLinked(false), _implicitlyLinked(false), _speculativelyLoaded(false),
												_forcedWeakLinked(false), _needed(false), _reExported(false),
												_upward(false), _dead(false) { }
				const char*					installPath() const			{ return _dylibInstallPath; }
				const char*					frameworkName() const		{ return _frameworkName; }
				uint32_t					timestamp() const			{ return _dylibTimeStamp; }
				uint32_t					currentVersion() const		{ return _dylibCurrentVersion; }
				uint32_t					compatibilityVersion() const{ return _dylibCompatibilityVersion; }
				void						setExplicitlyLinked()		{ _explicitlyLinked = true; }
				bool						explicitlyLinked() const	{ return _explicitlyLinked; }
				void						setImplicitlyLinked()		{ _implicitlyLinked = true; }
				bool						implicitlyLinked() const	{ return _implicitlyLinked; }
				void						setSpeculativelyLoaded()	{ _speculativelyLoaded = true; }
				bool						speculativelyLoaded() const	{ return _speculativelyLoaded; }

				// attributes of how dylib will be used when linked
				void						setForcedWeakLinked()			{ _forcedWeakLinked = true; }
				bool						forcedWeakLinked() const		{ return _forcedWeakLinked; }
				void						setNeededDylib()				{ _needed = true; }
				bool						neededDylib() const				{ return _needed; }

				void						setWillBeReExported()			{ _reExported = true; }
				bool						willBeReExported() const		{ return _reExported; }
				void						setWillBeUpwardDylib()			{ _upward = true; }
				bool						willBeUpwardDylib() const		{ return _upward; }
				void						setWillBeRemoved(bool value)	{ _dead = value; }
				bool						willRemoved() const				{ return _dead; }
				
		virtual void						processIndirectLibraries(DylibHandler* handler, bool addImplicitDylibs) = 0;
		virtual bool						providedExportAtom() const = 0;
		virtual const char*					parentUmbrella() const = 0;
		virtual const std::vector<const char*>*	allowableClients() const = 0;
		virtual const std::vector<const char*>&	rpaths() const = 0;
		virtual bool						hasWeakExternals() const = 0;
		virtual bool						deadStrippable() const = 0;
		virtual bool						hasWeakDefinition(const char* name) const = 0;
		virtual bool						hasDefinition(const char* name) const = 0;
		virtual bool						hasPublicInstallName() const = 0;
		virtual bool						allSymbolsAreWeakImported() const = 0;
		virtual bool						installPathVersionSpecific() const { return false; }
		virtual bool						appExtensionSafe() const = 0;
		virtual void						forEachExportedSymbol(void (^handler)(const char* symbolName, bool weakDef)) const = 0;
		virtual bool						hasReExportedDependentsThatProvidedExportAtom() const { return false; }
		virtual bool						isUnzipperedTwin() const { return false; }

	public:
		const char*							_dylibInstallPath;
		const char*							_frameworkName;
		uint32_t							_dylibTimeStamp;
		uint32_t							_dylibCurrentVersion;
		uint32_t							_dylibCompatibilityVersion;
		bool								_explicitlyLinked;
		bool								_implicitlyLinked;
		bool								_speculativelyLoaded;
		bool								_forcedWeakLinked;
		bool								_needed;
		bool								_reExported;
		bool								_upward;
		bool								_dead;
	};
} // namespace dylib


namespace archive {
	//
	// ld::archive::File 
	//
	// Abstract base class for static libraries read by the linker processes.
	//
	class File : public ld::File
	{
	public:
											File(const char* pth, time_t modTime, Ordinal ord)
												: ld::File(pth, modTime, ord, Archive) { }
		virtual								~File() {}
		virtual bool						justInTimeDataOnlyforEachAtom(const char* name, AtomHandler&) const = 0;
	};
} // namespace archive 


//
// ld::Section
//
class Section
{
public:
	enum Type { typeUnclassified, typeCode, typePageZero, typeImportProxies, typeLinkEdit, typeMachHeader, typeStack,
				typeLiteral4, typeLiteral8, typeLiteral16, typeConstants, typeTempLTO, typeTempAlias,
				typeCString, typeNonStdCString, typeCStringPointer, typeUTF16Strings, typeCFString, typeObjC1Classes,
				typeCFI, typeLSDA, typeDtraceDOF, typeUnwindInfo, typeObjCClassRefs, typeObjC2CategoryList, typeObjC2ClassList,
				typeZeroFill, typeTentativeDefs, typeLazyPointer, typeStub, typeNonLazyPointer, typeDyldInfo, 
				typeLazyDylibPointer, typeStubHelper, typeInitializerPointers, typeTerminatorPointers,
				typeStubClose, typeLazyPointerClose, typeAbsoluteSymbols, typeThreadStarts, typeChainStarts,
				typeTLVDefs, typeTLVZeroFill, typeTLVInitialValues, typeTLVInitializerPointers, typeTLVPointers,
				typeFirstSection, typeLastSection, typeDebug, typeSectCreate, typeInitOffsets };


					Section(const char* sgName, const char* sctName,
								Type t, bool hidden=false)
								: _segmentName(sgName), _sectionName(sctName),
								_type(t), _hidden(hidden)  {}
					Section(const Section& sect)
								: _segmentName(sect.segmentName()), _sectionName(sect.sectionName()),
								_type(sect.type()), _hidden(sect.isSectionHidden())  {}
								
	bool			operator==(const Section& rhs) const { return ( (_hidden==rhs._hidden) &&
														(strcmp(_segmentName, rhs._segmentName)==0) &&
														(strcmp(_sectionName, rhs._sectionName)==0) ); }
	bool			operator!=(const Section& rhs) const { return ! (*this == rhs); }
	const char*			segmentName() const			{ return _segmentName; }
	const char*			sectionName() const			{ return _sectionName; }
	Type				type() const				{ return _type; }
	bool				isSectionHidden() const		{ return _hidden; }
	
private:
	const char*			_segmentName;
	const char*			_sectionName;
	Type				_type;
	bool				_hidden;
};



//
// ld::Fixup
//
// A Fixup describes how part of an Atom's content must be fixed up.  For instance,
// an instruction may contain a displacement to another Atom that must be 
// fixed up by the linker.  
//
// A Fixup my reference another Atom. There are two kinds of references: direct and by-name.  
// With a direct reference, the target is bound by the File that created it. 
// For instance a reference to a static would produce a direct reference.  
// A by-name reference requires the linker to find the target Atom with the 
// required name in order to be bound.
//
// For a link to succeed all Fixup must be bound.
//
// A Reference also has a fix-up-offset.  This is the offset into the content of the
// Atom holding the reference where the fix-up (relocation) will be applied.
//
//
struct Fixup 
{
	enum TargetBinding { bindingNone, bindingByNameUnbound, bindingDirectlyBound, bindingByContentBound, bindingsIndirectlyBound };
	enum Cluster { k1of1, k1of2, k2of2, k1of3, k2of3, k3of3, k1of4, k2of4, k3of4, k4of4, k1of5, k2of5, k3of5, k4of5, k5of5 };
	enum Kind	{	kindNone, kindNoneFollowOn, 
					// grouping
					kindNoneGroupSubordinate, 
					kindNoneGroupSubordinateFDE, kindNoneGroupSubordinateLSDA, kindNoneGroupSubordinatePersonality,
					// value calculations
					kindSetTargetAddress,
					kindSubtractTargetAddress,
					kindAddAddend,
					kindSubtractAddend,
					kindSetTargetImageOffset,
					kindSetTargetSectionOffset,
					kindSetTargetTLVTemplateOffset,
					// pointer store kinds (of current calculated value)
					kindStore8,
					kindStoreLittleEndian16,
					kindStoreLittleEndianLow24of32,
					kindStoreLittleEndian32,
					kindStoreLittleEndian64,
					kindStoreBigEndian16,
					kindStoreBigEndianLow24of32,
					kindStoreBigEndian32,
					kindStoreBigEndian64,
					// Intel specific store kinds
					kindStoreX86BranchPCRel8, kindStoreX86BranchPCRel32, 
					kindStoreX86PCRel8, kindStoreX86PCRel16,  
					kindStoreX86PCRel32, kindStoreX86PCRel32_1, kindStoreX86PCRel32_2, kindStoreX86PCRel32_4, 
					kindStoreX86PCRel32GOTLoad, kindStoreX86PCRel32GOTLoadNowLEA, kindStoreX86PCRel32GOT, 
					kindStoreX86PCRel32TLVLoad, kindStoreX86PCRel32TLVLoadNowLEA,
					kindStoreX86Abs32TLVLoad, kindStoreX86Abs32TLVLoadNowLEA,
					// ARM specific store kinds
					kindStoreARMBranch24, kindStoreThumbBranch22, 
					kindStoreARMLoad12,
					kindStoreARMLow16, kindStoreARMHigh16, 
					kindStoreThumbLow16, kindStoreThumbHigh16, 
#if SUPPORT_ARCH_arm64
					// ARM64 specific store kinds
					kindStoreARM64Branch26,  
					kindStoreARM64Page21, kindStoreARM64PageOff12,
					kindStoreARM64GOTLoadPage21, kindStoreARM64GOTLoadPageOff12,
					kindStoreARM64GOTLeaPage21, kindStoreARM64GOTLeaPageOff12,
					kindStoreARM64TLVPLoadPage21, kindStoreARM64TLVPLoadPageOff12,
					kindStoreARM64TLVPLoadNowLeaPage21, kindStoreARM64TLVPLoadNowLeaPageOff12,
					kindStoreARM64PointerToGOT, kindStoreARM64PCRelToGOT,
#endif
#if SUPPORT_ARCH_arm64_32
					kindStoreARM64PointerToGOT32,
#endif
					// dtrace probes
					kindDtraceExtra,
					kindStoreX86DtraceCallSiteNop, kindStoreX86DtraceIsEnableSiteClear,
					kindStoreARMDtraceCallSiteNop, kindStoreARMDtraceIsEnableSiteClear,
					kindStoreARM64DtraceCallSiteNop, kindStoreARM64DtraceIsEnableSiteClear,
					kindStoreThumbDtraceCallSiteNop, kindStoreThumbDtraceIsEnableSiteClear,
					// lazy binding
					kindLazyTarget, kindSetLazyOffset,
					// islands
					kindIslandTarget,
					// data-in-code markers
					kindDataInCodeStartData, kindDataInCodeStartJT8, kindDataInCodeStartJT16, 
					kindDataInCodeStartJT32, kindDataInCodeStartJTA32, kindDataInCodeEnd,
					// linker optimization hints
					kindLinkerOptimizationHint,
					// pointer store combinations
					kindStoreTargetAddressLittleEndian32,	// kindSetTargetAddress + kindStoreLittleEndian32
					kindStoreTargetAddressLittleEndian64,	// kindSetTargetAddress + kindStoreLittleEndian64
					kindStoreTargetAddressBigEndian32,		// kindSetTargetAddress + kindStoreBigEndian32
					kindStoreTargetAddressBigEndian64,		// kindSetTargetAddress + kindStoreBigEndian364
					kindSetTargetTLVTemplateOffsetLittleEndian32,  // kindSetTargetTLVTemplateOffset + kindStoreLittleEndian32
					kindSetTargetTLVTemplateOffsetLittleEndian64,  // kindSetTargetTLVTemplateOffset + kindStoreLittleEndian64
					// Intel value calculation and store combinations
					kindStoreTargetAddressX86PCRel32,		// kindSetTargetAddress + kindStoreX86PCRel32
					kindStoreTargetAddressX86BranchPCRel32, // kindSetTargetAddress + kindStoreX86BranchPCRel32
					kindStoreTargetAddressX86PCRel32GOTLoad,// kindSetTargetAddress + kindStoreX86PCRel32GOTLoad
					kindStoreTargetAddressX86PCRel32GOTLoadNowLEA,// kindSetTargetAddress + kindStoreX86PCRel32GOTLoadNowLEA
					kindStoreTargetAddressX86PCRel32TLVLoad, // kindSetTargetAddress + kindStoreX86PCRel32TLVLoad
					kindStoreTargetAddressX86PCRel32TLVLoadNowLEA, // kindSetTargetAddress + kindStoreX86PCRel32TLVLoadNowLEA
					kindStoreTargetAddressX86Abs32TLVLoad,		// kindSetTargetAddress + kindStoreX86Abs32TLVLoad
					kindStoreTargetAddressX86Abs32TLVLoadNowLEA,	// kindSetTargetAddress + kindStoreX86Abs32TLVLoadNowLEA
					// ARM value calculation and store combinations
					kindStoreTargetAddressARMBranch24,		// kindSetTargetAddress + kindStoreARMBranch24
					kindStoreTargetAddressThumbBranch22,	// kindSetTargetAddress + kindStoreThumbBranch22
					kindStoreTargetAddressARMLoad12,		// kindSetTargetAddress + kindStoreARMLoad12
#if SUPPORT_ARCH_arm64
					// ARM64 value calculation and store combinations
					kindStoreTargetAddressARM64Branch26,		// kindSetTargetAddress + kindStoreARM64Branch26
					kindStoreTargetAddressARM64Page21,			// kindSetTargetAddress + kindStoreARM64Page21
					kindStoreTargetAddressARM64PageOff12,		// kindSetTargetAddress + kindStoreARM64PageOff12
					kindStoreTargetAddressARM64PageOff12ConvertAddToLoad, // kindSetTargetAddress + kindStoreARM64PageOff12 and convert add to load
					kindStoreTargetAddressARM64GOTLoadPage21,	// kindSetTargetAddress + kindStoreARM64GOTLoadPage21
					kindStoreTargetAddressARM64GOTLoadPageOff12,// kindSetTargetAddress + kindStoreARM64GOTLoadPageOff12
					kindStoreTargetAddressARM64GOTLeaPage21,	// kindSetTargetAddress + kindStoreARM64GOTLeaPage21
					kindStoreTargetAddressARM64GOTLeaPageOff12,	// kindSetTargetAddress + kindStoreARM64GOTLeaPageOff12
					kindStoreTargetAddressARM64TLVPLoadPage21,	// kindSetTargetAddress + kindStoreARM64TLVPLoadPage21
					kindStoreTargetAddressARM64TLVPLoadPageOff12,// kindSetTargetAddress + kindStoreARM64TLVPLoadPageOff12
					kindStoreTargetAddressARM64TLVPLoadNowLeaPage21,	// kindSetTargetAddress + kindStoreARM64TLVPLoadNowLeaPage21
					kindStoreTargetAddressARM64TLVPLoadNowLeaPageOff12,	// kindSetTargetAddress + kindStoreARM64TLVPLoadNowLeaPageOff12
#endif
#if SUPPORT_ARCH_arm64e
					kindStoreLittleEndianAuth64,
					kindStoreTargetAddressLittleEndianAuth64,	// kindSetTargetAddress + kindStoreLittleEndianAuth64
					kindSetAuthData,
#endif
			};

#if SUPPORT_ARCH_arm64e
	struct AuthData {
		// clang encodes the combination of the key bits as these values.
		typedef enum {
			ptrauth_key_asia = 0,
			ptrauth_key_asib = 1,
			ptrauth_key_asda = 2,
			ptrauth_key_asdb = 3,
		} ptrauth_key;

		uint16_t discriminator;
		bool hasAddressDiversity;
		ptrauth_key key;
	};
#endif

	union {
		const Atom*	target;
		const char*	name;
		uint64_t	addend;
		uint32_t	bindingIndex;
#if SUPPORT_ARCH_arm64e
		AuthData	authData;
#endif
	} u;
	uint32_t		offsetInAtom;
	Kind			kind : 8;
	Cluster			clusterSize : 4;
	bool			weakImport : 1;
	TargetBinding	binding : 3;
	bool			contentAddendOnly : 1;
	bool			contentDetlaToAddendOnly : 1;
	bool			contentIgnoresAddend : 1;
	
	typedef Fixup*		iterator;

	Fixup() :
		offsetInAtom(0), kind(kindNone), clusterSize(k1of1), weakImport(false), 
		binding(bindingNone),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false) { u.target = NULL; }

	Fixup(Kind k, Atom* targetAtom) :
		offsetInAtom(0), kind(k), clusterSize(k1of1), weakImport(false), 
		binding(Fixup::bindingDirectlyBound),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false)  
			{ assert(targetAtom != NULL); u.target = targetAtom; }

	Fixup(uint32_t off, Cluster c, Kind k) :
		offsetInAtom(off), kind(k), clusterSize(c), weakImport(false), 
		binding(Fixup::bindingNone),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false)  
			{ u.addend = 0; }

	Fixup(uint32_t off, Cluster c, Kind k, bool weakIm, const char* name) :
		offsetInAtom(off), kind(k), clusterSize(c), weakImport(weakIm), 
		binding(Fixup::bindingByNameUnbound),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false) 
			{ assert(name != NULL); u.name = name; }
		
	Fixup(uint32_t off, Cluster c, Kind k, TargetBinding b, const char* name) :
		offsetInAtom(off), kind(k), clusterSize(c), weakImport(false), binding(b),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false) 
			{ assert(name != NULL); u.name = name; }
		
	Fixup(uint32_t off, Cluster c, Kind k, const Atom* targetAtom) :
		offsetInAtom(off), kind(k), clusterSize(c), weakImport(false), 
		binding(Fixup::bindingDirectlyBound),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false) 
			{ assert(targetAtom != NULL); u.target = targetAtom; }
		
	Fixup(uint32_t off, Cluster c, Kind k, TargetBinding b, const Atom* targetAtom) :
		offsetInAtom(off), kind(k), clusterSize(c), weakImport(false), binding(b),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false) 
			{ assert(targetAtom != NULL); u.target = targetAtom; }
		
	Fixup(uint32_t off, Cluster c, Kind k, uint64_t addend) :
		offsetInAtom(off), kind(k), clusterSize(c), weakImport(false), 
		binding(Fixup::bindingNone),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false) 
			{ u.addend = addend; }
		
#if SUPPORT_ARCH_arm64e
	Fixup(uint32_t off, Cluster c, Kind k, AuthData authData) :
		offsetInAtom(off), kind(k), clusterSize(c), weakImport(false), 
		binding(Fixup::bindingNone),  
		contentAddendOnly(false), contentDetlaToAddendOnly(false), contentIgnoresAddend(false) 
			{ u.authData = authData; }
#endif
			
	Fixup(Kind k, uint32_t lohKind, uint32_t off1, uint32_t off2) :
		offsetInAtom(off1), kind(k), clusterSize(k1of1),  
		weakImport(false), binding(Fixup::bindingNone), contentAddendOnly(false), 
		contentDetlaToAddendOnly(false), contentIgnoresAddend(false) {
			assert(k == kindLinkerOptimizationHint);
			LOH_arm64 extra;
			extra.addend = 0;
			extra.info.kind = lohKind;
			extra.info.count = 1;
			extra.info.delta1 = 0;
			extra.info.delta2 = (off2 - off1) >> 2;
			u.addend = extra.addend; 
		}
			

	bool firstInCluster() const { 
		switch (clusterSize) {
			case k1of1:
			case k1of2:
			case k1of3:
			case k1of4:
			case k1of5:
				return true;
			default:
				break;
		}
		return false;
	}
	
	bool lastInCluster() const { 
		switch (clusterSize) {
			case k1of1:
			case k2of2:
			case k3of3:
			case k4of4:
			case k5of5:
				return true;
			default:
				break;
		}
		return false;
	}

	bool isStore() const {
		switch ( kind ) {
			case ld::Fixup::kindNone:
			case ld::Fixup::kindNoneFollowOn:
			case ld::Fixup::kindNoneGroupSubordinate:
			case ld::Fixup::kindNoneGroupSubordinateFDE:
			case ld::Fixup::kindNoneGroupSubordinateLSDA:
			case ld::Fixup::kindNoneGroupSubordinatePersonality:
			case ld::Fixup::kindSetTargetAddress:
			case ld::Fixup::kindSubtractTargetAddress:
			case ld::Fixup::kindAddAddend:
			case ld::Fixup::kindSubtractAddend:
			case ld::Fixup::kindSetTargetImageOffset:
			case ld::Fixup::kindSetTargetSectionOffset:
#if SUPPORT_ARCH_arm64e
			case ld::Fixup::kindSetAuthData:
#endif
				return false;
			default:
				break;
		}
		return true;
	}


	bool setsTarget(bool isObjectFile) const {
		switch ( kind ) {
			case ld::Fixup::kindSetTargetAddress:
			case ld::Fixup::kindLazyTarget:
			case ld::Fixup::kindStoreTargetAddressLittleEndian32:
			case ld::Fixup::kindStoreTargetAddressLittleEndian64:
#if SUPPORT_ARCH_arm64e
			case ld::Fixup::kindStoreTargetAddressLittleEndianAuth64:
#endif
			case ld::Fixup::kindStoreTargetAddressBigEndian32:
			case ld::Fixup::kindStoreTargetAddressBigEndian64:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32:
			case ld::Fixup::kindStoreTargetAddressX86BranchPCRel32:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoad:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoadNowLEA:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoad:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoadNowLEA:
			case ld::Fixup::kindStoreTargetAddressX86Abs32TLVLoad:
			case ld::Fixup::kindStoreTargetAddressARMBranch24:
			case ld::Fixup::kindStoreTargetAddressThumbBranch22:
			case ld::Fixup::kindStoreTargetAddressARMLoad12:
#if SUPPORT_ARCH_arm64
			case ld::Fixup::kindStoreTargetAddressARM64Branch26:
			case ld::Fixup::kindStoreTargetAddressARM64Page21:
			case ld::Fixup::kindStoreTargetAddressARM64PageOff12:
			case ld::Fixup::kindStoreTargetAddressARM64PageOff12ConvertAddToLoad:
			case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
			case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPageOff12:
			case ld::Fixup::kindStoreTargetAddressARM64GOTLeaPage21:
			case ld::Fixup::kindStoreTargetAddressARM64GOTLeaPageOff12:
			case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPage21:
			case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPageOff12:
			case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadNowLeaPage21:
			case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadNowLeaPageOff12:
#endif
				return true;
			case ld::Fixup::kindStoreX86DtraceCallSiteNop:
			case ld::Fixup::kindStoreX86DtraceIsEnableSiteClear:
			case ld::Fixup::kindStoreARMDtraceCallSiteNop:
			case ld::Fixup::kindStoreARMDtraceIsEnableSiteClear:
			case ld::Fixup::kindStoreARM64DtraceCallSiteNop:
			case ld::Fixup::kindStoreARM64DtraceIsEnableSiteClear:
			case ld::Fixup::kindStoreThumbDtraceCallSiteNop:
			case ld::Fixup::kindStoreThumbDtraceIsEnableSiteClear:
				return isObjectFile;
			default:
				break;
		}
		return false;
	}

	bool isPcRelStore(bool isKextBundle) const {
		switch ( kind ) {
			case ld::Fixup::kindStoreX86BranchPCRel8:
			case ld::Fixup::kindStoreX86BranchPCRel32:
			case ld::Fixup::kindStoreX86PCRel8:
			case ld::Fixup::kindStoreX86PCRel16:
			case ld::Fixup::kindStoreX86PCRel32:
			case ld::Fixup::kindStoreX86PCRel32_1:
			case ld::Fixup::kindStoreX86PCRel32_2:
			case ld::Fixup::kindStoreX86PCRel32_4:
			case ld::Fixup::kindStoreX86PCRel32GOTLoad:
			case ld::Fixup::kindStoreX86PCRel32GOTLoadNowLEA:
			case ld::Fixup::kindStoreX86PCRel32GOT:
			case ld::Fixup::kindStoreX86PCRel32TLVLoad:
			case ld::Fixup::kindStoreX86PCRel32TLVLoadNowLEA:
			case ld::Fixup::kindStoreARMBranch24:
			case ld::Fixup::kindStoreThumbBranch22:
			case ld::Fixup::kindStoreARMLoad12:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoad:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32GOTLoadNowLEA:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoad:
			case ld::Fixup::kindStoreTargetAddressX86PCRel32TLVLoadNowLEA:
			case ld::Fixup::kindStoreTargetAddressARMBranch24:
			case ld::Fixup::kindStoreTargetAddressThumbBranch22:
			case ld::Fixup::kindStoreTargetAddressARMLoad12:
#if SUPPORT_ARCH_arm64
			case ld::Fixup::kindStoreARM64Page21:
			case ld::Fixup::kindStoreARM64PageOff12:
			case ld::Fixup::kindStoreARM64GOTLoadPage21:
			case ld::Fixup::kindStoreARM64GOTLoadPageOff12:
			case ld::Fixup::kindStoreARM64GOTLeaPage21:
			case ld::Fixup::kindStoreARM64GOTLeaPageOff12:
			case ld::Fixup::kindStoreARM64TLVPLoadPage21:
			case ld::Fixup::kindStoreARM64TLVPLoadPageOff12:
			case ld::Fixup::kindStoreARM64TLVPLoadNowLeaPage21:
			case ld::Fixup::kindStoreARM64TLVPLoadNowLeaPageOff12:
			case ld::Fixup::kindStoreARM64PCRelToGOT:
			case ld::Fixup::kindStoreTargetAddressARM64Page21:
			case ld::Fixup::kindStoreTargetAddressARM64PageOff12:
			case ld::Fixup::kindStoreTargetAddressARM64PageOff12ConvertAddToLoad:
			case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPage21:
			case ld::Fixup::kindStoreTargetAddressARM64GOTLoadPageOff12:
			case ld::Fixup::kindStoreTargetAddressARM64GOTLeaPage21:
			case ld::Fixup::kindStoreTargetAddressARM64GOTLeaPageOff12:
			case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPage21:
			case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadPageOff12:
			case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadNowLeaPage21:
			case ld::Fixup::kindStoreTargetAddressARM64TLVPLoadNowLeaPageOff12:
#endif
				return true;
			case ld::Fixup::kindStoreTargetAddressX86BranchPCRel32:
#if SUPPORT_ARCH_arm64
			case ld::Fixup::kindStoreTargetAddressARM64Branch26:
#endif
				return !isKextBundle;
#if SUPPORT_ARCH_arm64
			case ld::Fixup::kindStoreARM64Branch26:
#endif
				return !isKextBundle;
			default:
				break;
		}
		return false;
	}
	
	union LOH_arm64 {
		uint64_t	addend;
		struct {
			unsigned	kind	:  6,
						count	:  2,	// 00 => 1 addr, 11 => 4 addrs
						delta1 : 14,	// 16-bit delta, low 2 bits assumed zero
						delta2 : 14,
						delta3 : 14,
						delta4 : 14;	
		} info;
	};
	
};

//
// ld::Atom
//
// An atom is the fundamental unit of linking.  A C function or global variable is an atom.
// An atom has content and attributes. The content of a function atom is the instructions
// that implement the function.  The content of a global variable atom is its initial bits.
//
// Name:
// The name of an atom is the label name generated by the compiler.  A C compiler names foo()
// as _foo.  A C++ compiler names foo() as __Z3foov.
// The name refers to the first byte of the content.  An atom cannot have multiple entry points.
// Such code is modeled as multiple atoms, each having a "follow on" reference to the next.
// A "follow on" reference is a contraint to the linker to the atoms must be laid out contiguously.
//
// Scope:
// An atom is in one of three scopes: translation-unit, linkage-unit, or global.  These correspond
// to the C visibility of static, hidden, default.
//
// DefinitionKind:
// An atom is one of five definition kinds:
//	regular			Most atoms.
//	weak			C++ compiler makes some functions weak if there might be multiple copies
//					that the linker needs to coalesce.
//	tentative		A straggler from ancient C when the extern did not exist. "int foo;" is ambiguous.
//					It could be a prototype or it could be a definition.
//	external		This is a "proxy" atom produced by a dylib reader.  It has no content.  It exists
//					so that the graph of Atoms can be complete.
//	external-weak	Same as external, but the definition in the dylib is weak.
//
// SymbolTableInclusion:
// An atom may or may not be in the symbol table in an object file.
//  in				Most atoms for functions or global data
//	not-in			Anonymous atoms such literal c-strings, or other compiler generated data
//  not-in-final	Atom whose name should not be in the symbol table of final linkd image (e.g. 'l' labels .eh labels)
//	in-never-strip	Atom whose name the strip tool should never remove (e.g. REFERENCED_DYNAMICALLY in mach-o)
//
// ContentType:
// Some atoms require specially processing by the linker based on their content.  For instance, zero-fill data
// atom are group together at the end of the DATA segment to reduce disk size.
//
// ObjectAddress:
// For reproducability, the linker lays out atoms in the order they occurred in the source (object) files.
// The objectAddress() method returns the address of an atom in the object file so that the linker 
// can arrange the atoms.
//
//
class Atom
{
public:
	enum Scope { scopeTranslationUnit, scopeLinkageUnit, scopeGlobal };
	enum Definition { definitionRegular, definitionTentative, definitionAbsolute, definitionProxy };
	enum Combine { combineNever, combineByName, combineByNameAndContent, combineByNameAndReferences };
	enum ContentType { typeUnclassified, typeZeroFill, typeCString, typeCFI, typeLSDA, typeSectionStart, 
					typeSectionEnd, typeBranchIsland, typeLazyPointer, typeStub, typeNonLazyPointer, 
					typeLazyDylibPointer, typeStubHelper, typeInitializerPointers, typeTerminatorPointers,
					typeLTOtemporary, typeResolver,
					typeTLV, typeTLVZeroFill, typeTLVInitialValue, typeTLVInitializerPointers, typeTLVPointer };

	enum SymbolTableInclusion { symbolTableNotIn, symbolTableNotInFinalLinkedImages, symbolTableIn,
								symbolTableInAndNeverStrip, symbolTableInAsAbsolute, 
								symbolTableInWithRandomAutoStripLabel };
	enum WeakImportState { weakImportUnset, weakImportTrue, weakImportFalse };
	
	struct Alignment { 
					Alignment(int p2, int m=0) : powerOf2(p2), modulus(m) {}
		uint8_t		trailingZeros() const { return (modulus==0) ? powerOf2 : __builtin_ctz(modulus); }
		uint16_t	powerOf2;  
		uint16_t	modulus; 
	};
	struct LineInfo {
		const char* fileName;
		uint32_t	atomOffset;
		uint32_t	lineNumber;
		
		typedef LineInfo* iterator;
	};
	struct UnwindInfo {
		uint32_t	startOffset;
		uint32_t	unwindInfo;
		
		typedef UnwindInfo* iterator;
	};
 
											Atom(const Section& sect, Definition d, Combine c, Scope s, ContentType ct, 
												SymbolTableInclusion i, bool dds, bool thumb, bool al, Alignment a, bool cold=false) :
													_section(&sect), _address(0), _alignmentModulus(a.modulus), 
													_alignmentPowerOf2(a.powerOf2), _definition(d), _combine(c),   
													_dontDeadStrip(dds), _thumb(thumb), _alias(al), _autoHide(false), 
													_contentType(ct), _symbolTableInclusion(i),
													_scope(s), _mode(modeSectionOffset), 
													_overridesADylibsWeakDef(false), _coalescedAway(false),
													_live(false), _dontDeadStripIfRefLive(false), _cold(cold),
													_machoSection(0), _weakImportState(weakImportUnset)
													 {
													#ifndef NDEBUG
														switch ( _combine ) {
															case combineByNameAndContent:
															case combineByNameAndReferences:
																assert(_symbolTableInclusion != symbolTableIn);
																assert(_scope != scopeGlobal);
                                                                break;
                                                            case combineByName:
                                                            case combineNever:
                                                                break;
														};
													#endif
													 }
	virtual									~Atom() {}

	const Section&							section() const				{ return *_section; }
	Definition								definition() const			{ return _definition; }
	Combine									combine() const				{ return _combine; }
	Scope									scope() const				{ return _scope; }
	ContentType								contentType() const			{ return _contentType; }
	SymbolTableInclusion					symbolTableInclusion() const{ return _symbolTableInclusion; }
	bool									dontDeadStrip() const		{ return _dontDeadStrip; }
	bool									dontDeadStripIfReferencesLive() const { return _dontDeadStripIfRefLive; }
	bool									isThumb() const				{ return _thumb; }
	bool									isAlias() const				{ return _alias; }
	Alignment								alignment() const			{ return Alignment(_alignmentPowerOf2, _alignmentModulus); }
	bool									overridesDylibsWeakDef() const	{ return _overridesADylibsWeakDef; }
	bool									coalescedAway() const		{ return _coalescedAway; }
	bool									weakImported() const		{ return _weakImportState == weakImportTrue; }
	WeakImportState							weakImportState() const		{ return _weakImportState; }
	bool									autoHide() const			{ return _autoHide; }
	bool									cold() const			    { return _cold; }
	bool									live() const				{ return _live; }
	uint8_t									machoSection() const		{ assert(_machoSection != 0); return _machoSection; }

	void									setScope(Scope s)			{ _scope = s; }
	void									setSymbolTableInclusion(SymbolTableInclusion i)			
																		{ _symbolTableInclusion = i; }
	void									setCombine(Combine c)		{ _combine = c; }
	void									setOverridesDylibsWeakDef()	{ _overridesADylibsWeakDef = true; }
	void									setCoalescedAway()			{ _coalescedAway = true; }
	void									setWeakImportState(bool w)	{ assert(_definition == definitionProxy); _weakImportState = ( w ? weakImportTrue : weakImportFalse); }
	void									setAutoHide()				{ _autoHide = true; }
	void									setDontDeadStripIfReferencesLive() { _dontDeadStripIfRefLive = true; }
	void									setLive()					{ _live = true; }
	void									setLive(bool value)			{ _live = value; }
	void									setMachoSection(unsigned x) { assert(x != 0); assert(x < 256); _machoSection = x; }
	void									setSectionOffset(uint64_t o){ assert(_mode == modeSectionOffset); _address = o; _mode = modeSectionOffset; }
	void									setSectionStartAddress(uint64_t a) { assert(_mode == modeSectionOffset); _address += a; _mode = modeFinalAddress; }
	uint64_t								sectionOffset() const		{ assert(_mode == modeSectionOffset); return _address; }
	uint64_t								finalAddress() const		{ assert(_mode == modeFinalAddress); return _address; }
	bool									finalAddressMode() const    { return (_mode == modeFinalAddress); }

	virtual const File*						file() const = 0;
	// Return the original file this atom belongs to, for instance for an LTO atom,
	// file() would return the LTO MachO file instead of the original bitcode file.
	virtual const ld::File*				    originalFile() const       { return file(); }
	virtual const char*						translationUnitSource() const { return NULL; }
	virtual const char*						name() const = 0;
	std::string_view						getUserVisibleName() const {
		auto* nm = name();
		if (!nm)
			return std::string_view();
		std::string_view visibleName(nm);
		auto pos = visibleName.rfind(".llvm.");
		if (pos == std::string_view::npos)
			return visibleName;
		return visibleName.substr(0, pos);
	}
	virtual uint64_t						objectAddress() const = 0;
	virtual uint64_t						size() const = 0;
	virtual void							copyRawContent(uint8_t buffer[]) const = 0;
	virtual const uint8_t*					rawContentPointer() const { return NULL; }
	virtual unsigned long					contentHash(const class IndirectBindingTable&) const { return 0; }
	virtual bool							canCoalesceWith(const Atom& rhs, const class IndirectBindingTable&) const { return false; }
	virtual Fixup::iterator					fixupsBegin() const	{ return NULL; }
	virtual Fixup::iterator					fixupsEnd() const	{ return NULL; }
	bool									hasFixupsOfKind(Fixup::Kind kind) const {
		for (ld::Fixup::iterator fit = fixupsBegin(), end=fixupsEnd(); fit != end; ++fit) {
			if ( fit->kind == kind ) return true;
		}
		return false;
	}
	virtual void							setFile(const File* f)		{ }
	
	virtual UnwindInfo::iterator			beginUnwind() const { return NULL; }
	virtual UnwindInfo::iterator			endUnwind() const	{ return NULL; }
	virtual LineInfo::iterator				beginLineInfo() const { return NULL; }
	virtual LineInfo::iterator				endLineInfo() const { return NULL; }
											
											void setAttributesFromAtom(const Atom& a) { 
													_section = a._section; 
													_alignmentModulus = a._alignmentModulus;
													_alignmentPowerOf2 = a._alignmentPowerOf2;
													_definition = a._definition;
													_combine = a._combine;
													_dontDeadStrip = a._dontDeadStrip;
													_dontDeadStripIfRefLive = a._dontDeadStripIfRefLive;
													_cold = a._cold;
													_thumb = a._thumb;
													_autoHide = a._autoHide;
													_contentType = a._contentType;
													_symbolTableInclusion = a._symbolTableInclusion;
													_scope = a._scope;
													_mode = a._mode;
													_overridesADylibsWeakDef = a._overridesADylibsWeakDef;
													_coalescedAway = a._coalescedAway;
													_weakImportState = a._weakImportState;
												}

	const char*								safeFilePath() const {
												const File* f = this->file();
												if ( f != NULL )
													return f->path();
												else
													return "<internal>";
											}

protected:
	enum AddressMode { modeSectionOffset, modeFinalAddress };

	const Section *						_section;
	uint64_t							_address;
	uint16_t							_alignmentModulus;
	uint8_t								_alignmentPowerOf2;
	Definition							_definition : 2;
	Combine								_combine : 2;
	bool								_dontDeadStrip : 1;
	bool								_thumb : 1; 
	bool								_alias : 1;
	int									_autoHide : 1;
	ContentType							_contentType : 5;
	SymbolTableInclusion				_symbolTableInclusion : 3;
	Scope								_scope : 2;
	AddressMode							_mode: 2;
	bool								_overridesADylibsWeakDef : 1;
	bool								_coalescedAway : 1;
	bool								_live : 1;
	bool								_dontDeadStripIfRefLive : 1;
	bool								_cold : 1;
	unsigned							_machoSection : 8;
	WeakImportState						_weakImportState : 2;
};


class IndirectBindingTable
{
public:
	virtual 					~IndirectBindingTable() { }
	virtual const char*			indirectName(uint32_t bindingIndex) const = 0;
	virtual const ld::Atom*		indirectAtom(uint32_t bindingIndex) const = 0;
};



// utility classes for using std::unordered_map with c-strings
struct CStringHash {
	size_t operator()(const char* __s) const {
		size_t __h = 0;
		for ( ; *__s; ++__s)
			__h = 5 * __h + *__s;
		return __h;
	};
};
struct CStringEquals
{
	bool operator()(const char* left, const char* right) const { return (strcmp(left, right) == 0); }
};

typedef	std::unordered_set<const char*, ld::CStringHash, ld::CStringEquals>  CStringSet;


class Internal
{
public:
	class FinalSection : public ld::Section {
	public:
										FinalSection(const Section& sect) : Section(sect), address(0),
												fileOffset(0), size(0), alignment(0),
												indirectSymTabStartIndex(0), indirectSymTabElementSize(0),
												relocStart(0), relocCount(0), 
												hasLocalRelocs(false), hasExternalRelocs(false) {}
		std::vector<const Atom*>		atoms;
		uint64_t						address;
		uint64_t						fileOffset;
		uint64_t						size;
		uint32_t						alignmentPaddingBytes;
		uint8_t							alignment;
		uint32_t						indirectSymTabStartIndex;
		uint32_t						indirectSymTabElementSize;
		uint32_t						relocStart;
		uint32_t						relocCount;
		bool							hasLocalRelocs;
		bool							hasExternalRelocs;
	};
	
	typedef std::map<const ld::Atom*, FinalSection*>	AtomToSection;		

	virtual uint64_t					assignFileOffsets() = 0;
	virtual void						setSectionSizesAndAlignments() = 0;
	virtual ld::Internal::FinalSection*	addAtom(const Atom&) = 0;
	virtual ld::Internal::FinalSection* getFinalSection(const ld::Section& inputSection) = 0;
	virtual								~Internal() {}
										Internal() : bundleLoader(NULL),
											entryPoint(NULL), classicBindingHelper(NULL),
											lazyBindingHelper(NULL), compressedFastBinderProxy(NULL),
											hasObjC(false), hasArm64eABIVersion(false), arm64eABIVersion(0),
											swiftVersion(0), swiftLanguageVersion(0),
											cpuSubType(0), minOSVersion(0),
											objectFileFoundWithNoVersion(false),
											allObjectFilesScatterable(true), 
											someObjectFileHasDwarf(false), usingHugeSections(false),
											someObjectFileHasSwift(false), firstSwiftDylibFile(nullptr),
											hasThreadLocalVariableDefinitions(false),
											hasWeakExternalSymbols(false),
											someObjectHasOptimizationHints(false),
											dropAllBitcode(false), embedMarkerOnly(false),
											forceLoadCompilerRT(false), cantUseChainedFixups(false)	{ }

	std::vector<FinalSection*>					sections;
	std::vector<ld::dylib::File*>				dylibs;
	std::vector<std::string>					archivePaths;
	std::vector<ld::relocatable::File::Stab>	stabs;
	AtomToSection								atomToSection;		
	CStringSet									unprocessedLinkerOptionLibraries;
	CStringSet									unprocessedLinkerOptionFrameworks;
	CStringSet									linkerOptionNeededLibraries;
	CStringSet									linkerOptionNeededFrameworks;
	CStringSet									linkerOptionLibraries;
	CStringSet									linkerOptionFrameworks;
	CStringSet									missingLinkerOptionLibraries;
	CStringSet									missingLinkerOptionFrameworks;
	std::vector<const ld::Atom*>				indirectBindingTable;
	std::vector<const ld::relocatable::File*>	filesWithBitcode;
	std::vector<const ld::relocatable::File*>	filesFromCompilerRT;
	std::vector<const ld::relocatable::File*>	filesForLTO;
	std::vector<const ld::Atom*>				deadAtoms;
	std::unordered_set<const char*>				allUndefProxies;
	std::unordered_set<uint64_t>				toolsVersions;
	const ld::dylib::File*						bundleLoader;
	const Atom*									entryPoint;
	const Atom*									classicBindingHelper;
	const Atom*									lazyBindingHelper;
	const Atom*									compressedFastBinderProxy;
	bool										hasObjC;
	bool										hasArm64eABIVersion;
	uint8_t										arm64eABIVersion;
	uint8_t										swiftVersion;
	uint16_t									swiftLanguageVersion;
	uint32_t									cpuSubType;
	uint32_t									minOSVersion;
	bool										objectFileFoundWithNoVersion;
	bool										allObjectFilesScatterable;
	bool										someObjectFileHasDwarf;
	bool										usingHugeSections;
	bool										someObjectFileHasSwift;
	const ld::dylib::File*						firstSwiftDylibFile;
	bool										hasThreadLocalVariableDefinitions;
	bool										hasWeakExternalSymbols;
	bool										someObjectHasOptimizationHints;
	bool										dropAllBitcode;
	bool										embedMarkerOnly;
	bool										forceLoadCompilerRT;
	bool										cantUseChainedFixups;
	std::vector<std::string>					ltoBitcodePath;
};







} // namespace ld 

#endif // __LD_HPP__
