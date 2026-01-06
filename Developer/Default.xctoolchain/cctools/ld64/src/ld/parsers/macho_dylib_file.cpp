/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2005-2011 Apple Inc. All rights reserved.
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
#define PLATFORM_UNKNOWN 0
#define PLATFORM_MACOS 1
#define CPU_SUBTYPE_X86_ALL 3
namespace {
	typedef long double max_align_t;
}
#endif

#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/mman.h>


#include <vector>
#include <set>
#include <map>
#include <algorithm>

#include "Architectures.hpp"
#include "Bitcode.hpp"
#include "MachOFileAbstraction.hpp"
#include "MachOTrie.hpp"
#include "generic_dylib_file.hpp"
#include "macho_dylib_file.h"
#include "PlatformSupport.h"
#include "../code-sign-blobs/superblob.h"

namespace mach_o {
namespace dylib {

//
// The reader for a dylib extracts all exported symbols names from the memory-mapped
// dylib, builds a hash table, then unmaps the file.  This is an important memory
// savings for large dylibs.
//
template <typename A>
class File final : public generic::dylib::File
{
	using Base = generic::dylib::File;

public:
	static bool								validFile(const uint8_t* fileContent, bool executableOrDylib, bool subTypeMustMatch=false);
											File(const uint8_t* fileContent, uint64_t fileLength, const char* path,   
													time_t mTime, ld::File::Ordinal ordinal, bool linkingFlatNamespace, 
													bool linkingMainExecutable, bool hoistImplicitPublicDylibs, 
												 	const ld::VersionSet& platforms, bool allowWeakImports,
													bool allowSimToMacOSX, bool addVers,  bool buildingForSimulator,
													bool logAllFiles, const char* installPath,
													bool indirectDylib, bool usingBitcode, bool internalSDK,
													bool fromSDK, bool platformMismatchesAreWarning);
	virtual									~File() noexcept {}

private:
	using P = typename A::P;
	using E = typename A::P::E;
	using pint_t = typename A::P::uint_t;

	void				addDyldFastStub();
	void				buildExportHashTableFromExportInfo(uint32_t exportsOffset, uint32_t exportsSize,
															const uint8_t* fileContent);
	void				buildExportHashTableFromSymbolTable(const macho_dysymtab_command<P>* dynamicInfo,
														const macho_nlist<P>* symbolTable, const char* strings,
														const uint8_t* fileContent);
	void				addSymbol(const char* name, bool weakDef = false, bool tlv = false, pint_t address = 0);
	static const char*	objCInfoSegmentName();
	static const char*	objCInfoSectionName();


	uint64_t  _fileLength;
	uint32_t  _linkeditStartOffset;

};

template <> const char* File<x86_64>::objCInfoSegmentName() { return "__DATA"; }
template <> const char* File<arm>::objCInfoSegmentName() { return "__DATA"; }
template <typename A> const char* File<A>::objCInfoSegmentName() { return "__OBJC"; }

template <> const char* File<x86_64>::objCInfoSectionName() { return "__objc_imageinfo"; }
template <> const char* File<arm>::objCInfoSectionName() { return "__objc_imageinfo"; }
template <typename A> const char* File<A>::objCInfoSectionName() { return "__image_info"; }

template <typename A>
File<A>::File(const uint8_t* fileContent, uint64_t fileLength, const char* path, time_t mTime,
			  ld::File::Ordinal ord, bool linkingFlatNamespace, bool linkingMainExecutable,
			  bool hoistImplicitPublicDylibs, const ld::VersionSet& cmdLinePlatforms, bool allowWeakImports,
			  bool allowSimToMacOSX, bool addVers, bool buildingForSimulator, bool logAllFiles,
			  const char* targetInstallPath, bool indirectDylib, bool usingBitcode, bool internalSDK,
			  bool fromSDK, bool platformMismatchesAreWarning)
	: Base(strdup(path), mTime, ord, cmdLinePlatforms, allowWeakImports, linkingFlatNamespace,
		   hoistImplicitPublicDylibs, allowSimToMacOSX, addVers), _fileLength(fileLength), _linkeditStartOffset(0)
{
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	const uint32_t cmd_count = header->ncmds();
	const macho_load_command<P>* const cmds = (macho_load_command<P>*)((char*)header + sizeof(macho_header<P>));
	const macho_load_command<P>* const cmdsEnd = (macho_load_command<P>*)((char*)header + sizeof(macho_header<P>) + header->sizeofcmds());
	const uint8_t* const endOfFile = fileContent + fileLength;

	// write out path for -t option
	if ( logAllFiles )
		printf("%s\n", path);

	// a "blank" stub has zero load commands
	if ( (header->filetype() == MH_DYLIB_STUB) && (cmd_count == 0) ) {	
		// no further processing needed
		munmap((caddr_t)fileContent, fileLength);
		return;
	}


	// optimize the case where we know there is no reason to look at indirect dylibs
	this->_noRexports =    (header->flags() & MH_NO_REEXPORTED_DYLIBS)
						|| (header->filetype() == MH_BUNDLE)
					    || (header->filetype() == MH_EXECUTE);  // bundles and exectuables can be used via -bundle_loader
	this->_hasWeakExports = (header->flags() & MH_WEAK_DEFINES);
	this->_deadStrippable = (header->flags() & MH_DEAD_STRIPPABLE_DYLIB);
	this->_appExtensionSafe = (header->flags() & MH_APP_EXTENSION_SAFE);
	
	// pass 1: get pointers, and see if this dylib uses compressed LINKEDIT format
	const macho_dysymtab_command<P>* dynamicInfo = nullptr;
	const macho_dyld_info_command<P>* dyldInfo = nullptr;
	const macho_linkedit_data_command<P>* exportsTrie = nullptr;
	const macho_nlist<P>* symbolTable = nullptr;
	const macho_symtab_command<P>* symtab = nullptr;
	const char*	strings = nullptr;
	bool compressedLinkEdit = false;
	uint32_t dependentLibCount = 0;
	ld::VersionSet lcPlatforms;
	const macho_load_command<P>* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		macho_dylib_command<P>* dylibID;
		uint32_t cmdLength = cmd->cmdsize();
		if ( (cmdLength & 3) != 0 )
			throwf("load command #%d has a unaligned size", i);
		const uint8_t* endOfCmd = ((uint8_t*)cmd)+cmd->cmdsize();
		if ( endOfCmd > (uint8_t*)cmdsEnd )
			throwf("load command #%d extends beyond the end of the load commands", i);
		if ( endOfCmd > endOfFile )
			throwf("load command #%d extends beyond the end of the file", i);
		switch (cmd->cmd()) {
			case LC_SYMTAB:
				symtab = (macho_symtab_command<P>*)cmd;
				symbolTable = (const macho_nlist<P>*)((char*)header + symtab->symoff());
				strings = (char*)header + symtab->stroff();
				if ( (symtab->stroff() + symtab->strsize()) > fileLength )
					throwf("mach-o string pool extends beyond end of file in %s", path);
				break;
			case LC_DYSYMTAB:
				dynamicInfo = (macho_dysymtab_command<P>*)cmd;
				break;
			case LC_DYLD_INFO:
			case LC_DYLD_INFO_ONLY:
				dyldInfo = (macho_dyld_info_command<P>*)cmd;
				compressedLinkEdit = true;
				break;
			case LC_DYLD_EXPORTS_TRIE:
				exportsTrie = (macho_linkedit_data_command<P>*)cmd;
				compressedLinkEdit = true;
				break;
			case LC_ID_DYLIB:
				dylibID = (macho_dylib_command<P>*)cmd;
				if ( dylibID->name_offset() > cmdLength )
					throwf("malformed mach-o: LC_ID_DYLIB load command has offset (%u) outside its size (%u)", dylibID->name_offset(), cmdLength);
				if ( (dylibID->name_offset() + strlen(dylibID->name()) + 1) > cmdLength )
					throwf("malformed mach-o: LC_ID_DYLIB load command string extends beyond end of load command");
				this->_dylibInstallPath				= strdup(dylibID->name());
				this->_dylibTimeStamp				= dylibID->timestamp();
				this->_dylibCurrentVersion			= dylibID->current_version();
				this->_dylibCompatibilityVersion	= dylibID->compatibility_version();
				this->_hasPublicInstallName			= this->isPublicLocation(this->_dylibInstallPath);
				break;
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
				++dependentLibCount;
				break;
			case LC_REEXPORT_DYLIB:
				this->_explictReExportFound = true;
				++dependentLibCount;
				break;
			case LC_SUB_FRAMEWORK:
				this->_parentUmbrella = strdup(((macho_sub_framework_command<P>*)cmd)->umbrella());
				break;
			case LC_SUB_CLIENT:
				this->_allowableClients.push_back(strdup(((macho_sub_client_command<P>*)cmd)->client()));
				// <rdar://problem/20627554> Don't hoist "public" (in /usr/lib/) dylibs that should not be directly linked
				this->_hasPublicInstallName = false;
				break;
			case LC_RPATH:
				this->_rpaths.push_back(strdup(((macho_rpath_command<P>*)cmd)->path()));
				break;
			case LC_VERSION_MIN_MACOSX:
			case LC_VERSION_MIN_IPHONEOS:
			case LC_VERSION_MIN_WATCHOS:
			case LC_VERSION_MIN_TVOS:
				lcPlatforms.insert(ld::PlatformVersion(ld::platformForLoadCommand(cmd->cmd(), (mach_header*)header), ((macho_version_min_command<P>*)cmd)->version()));
				break;
			case LC_BUILD_VERSION:
				{
					const macho_build_version_command<P>* buildVersCmd = (macho_build_version_command<P>*)cmd;
					lcPlatforms.insert(ld::PlatformVersion((ld::Platform)buildVersCmd->platform(), buildVersCmd->minos()));
				}
				break;
			case LC_CODE_SIGNATURE:
				break;
			case macho_segment_command<P>::CMD:
				// check for Objective-C info
				if ( strncmp(((macho_segment_command<P>*)cmd)->segname(), objCInfoSegmentName(), 6) == 0 ) {
					const macho_segment_command<P>* segment = (macho_segment_command<P>*)cmd;
					const macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segment + sizeof(macho_segment_command<P>));
					const macho_section<P>* const sectionsEnd = &sectionsStart[segment->nsects()];
					for (const macho_section<P>* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						if ( strncmp(sect->sectname(), objCInfoSectionName(), strlen(objCInfoSectionName())) == 0 ) {
							//	struct objc_image_info  {
							//		uint32_t	version;	// initially 0
							//		uint32_t	flags;
							//	};
							// #define OBJC_IMAGE_SUPPORTS_GC   2
							// #define OBJC_IMAGE_GC_ONLY       4
						        // #define OBJC_IMAGE_IS_SIMULATED  32
							//
							const uint32_t* contents = (uint32_t*)(&fileContent[sect->offset()]);
							if ( (sect->size() >= 8) && (contents[0] == 0) ) {
								uint32_t flags = E::get32(contents[1]);
								this->_swiftVersion = ((flags >> 8) & 0xFF);
							}
							else if ( sect->size() > 0 ) {
								warning("can't parse %s/%s section in %s", objCInfoSegmentName(), objCInfoSectionName(), path);
							}
						}
					}
				}
				// Construct bitcode if there is a bitcode bundle section in the dylib
				// Record the size of the section because the content is not checked
				else if ( strcmp(((macho_segment_command<P>*)cmd)->segname(), "__LLVM") == 0 ) {
					const macho_section<P>* const sect = (macho_section<P>*)((char*)cmd + sizeof(macho_segment_command<P>));
					if ( strncmp(sect->sectname(), "__bundle", 8) == 0 )
						this->_bitcode = std::unique_ptr<ld::Bitcode>(new ld::Bitcode(NULL, sect->size()));
				}
				else if ( strcmp(((macho_segment_command<P>*)cmd)->segname(), "__LINKEDIT") == 0 ) {
					_linkeditStartOffset = ((macho_segment_command<P>*)cmd)->fileoff();
				}
		}
		cmd = (const macho_load_command<P>*)(((char*)cmd)+cmdLength);
		if ( cmd > cmdsEnd )
			throwf("malformed dylb, load command #%d is outside size of load commands in %s", i, path);
	}

	// check cross-linking
	cmdLinePlatforms.checkDylibCrosslink(lcPlatforms, path, "dylib", internalSDK, indirectDylib, usingBitcode, _isUnzipperedTwin, _dylibInstallPath, fromSDK, platformMismatchesAreWarning);

	// figure out if we need to examine dependent dylibs
	// with compressed LINKEDIT format, MH_NO_REEXPORTED_DYLIBS can be trusted
	bool processDependentLibraries = true;
	if  ( compressedLinkEdit && this->_noRexports && !linkingFlatNamespace)
		processDependentLibraries = false;
	
	if ( processDependentLibraries ) {
		// pass 2 builds list of all dependent libraries
		this->_dependentDylibs.reserve(dependentLibCount);
		cmd = cmds;
		unsigned int reExportDylibCount = 0;  
		for (uint32_t i = 0; i < cmd_count; ++i) {
			uint32_t cmdLength = cmd->cmdsize();
			const macho_dylib_command<P>* dylibCmd = (macho_dylib_command<P>*)cmd;
			switch (cmd->cmd()) {
				case LC_LOAD_DYLIB:
				case LC_LOAD_WEAK_DYLIB:
					// with new linkedit format only care about LC_REEXPORT_DYLIB
					if ( compressedLinkEdit && !linkingFlatNamespace ) 
						break;
				case LC_REEXPORT_DYLIB:
					++reExportDylibCount;
					if ( dylibCmd->name_offset() > cmdLength )
						throwf("malformed mach-o: LC_*_DYLIB load command has offset (%u) outside its size (%u)", dylibCmd->name_offset(), cmdLength);
					if ( (dylibCmd->name_offset() + strlen(dylibCmd->name()) + 1) > cmdLength )
						throwf("malformed mach-o: LC_*_DYLIB load command string extends beyond end of load command");
					const char *path = strdup(dylibCmd->name());
					bool reExport = (cmd->cmd() == LC_REEXPORT_DYLIB);
					if ( (targetInstallPath == nullptr) || (strcmp(targetInstallPath, path) != 0) )
						this->_dependentDylibs.emplace_back(path, reExport);
					break;
			}
			cmd = (const macho_load_command<P>*)(((char*)cmd)+cmdLength);
		}
		// verify MH_NO_REEXPORTED_DYLIBS bit was correct
		if ( compressedLinkEdit && !linkingFlatNamespace ) {
			if ( reExportDylibCount == 0 )
				throwf("malformed dylib has MH_NO_REEXPORTED_DYLIBS flag but no LC_REEXPORT_DYLIB load commands: %s", path);
		}
		// pass 3 add re-export info
		cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			const char* frameworkLeafName;
			const char* dylibBaseName;
			switch (cmd->cmd()) {
				case LC_SUB_UMBRELLA:
					frameworkLeafName = ((macho_sub_umbrella_command<P>*)cmd)->sub_umbrella();
					for (auto &dep : this->_dependentDylibs) {
						const char* dylibName = dep.path;
						const char* lastSlash = strrchr(dylibName, '/');
						if ( (lastSlash != nullptr) && (strcmp(&lastSlash[1], frameworkLeafName) == 0) )
							dep.reExport = true;
					}
					break;
				case LC_SUB_LIBRARY:
					dylibBaseName = ((macho_sub_library_command<P>*)cmd)->sub_library();
					for (auto &dep : this->_dependentDylibs) {
						const char* dylibName = dep.path;
						const char* lastSlash = strrchr(dylibName, '/');
						const char* leafStart = &lastSlash[1];
						if ( lastSlash == nullptr )
							leafStart = dylibName;
						const char* firstDot = strchr(leafStart, '.');
						int len = strlen(leafStart);
						if ( firstDot != nullptr )
							len = firstDot - leafStart;
						if ( strncmp(leafStart, dylibBaseName, len) == 0 )
							dep.reExport = true;
					}
					break;
			}
			cmd = (const macho_load_command<P>*)(((char*)cmd)+cmd->cmdsize());
		}
	}

	// if framework, capture framework name
	if ( this->_dylibInstallPath != NULL ) {
		const char* lastSlash = strrchr(this->_dylibInstallPath, '/');
		if ( lastSlash != NULL ) {
			const char* leafName = lastSlash+1;
			char frname[strlen(leafName)+32];
			strcpy(frname, leafName);
			strcat(frname, ".framework/");

			if ( strstr(this->_dylibInstallPath, frname) != NULL )
			this->_frameworkName = leafName;
		}
	}

	// validate minimal load commands
	if ( (this->_dylibInstallPath == nullptr) && ((header->filetype() == MH_DYLIB) || (header->filetype() == MH_DYLIB_STUB)) )
		throwf("dylib %s missing LC_ID_DYLIB load command", path);
	if ( dyldInfo == nullptr ) {
		if ( symbolTable == nullptr )
			throw "binary missing LC_SYMTAB load command";
		if ( dynamicInfo == nullptr )
			throw "binary missing LC_DYSYMTAB load command";
	}

	if ( symtab != nullptr ) {
		if ( (symtab->nsyms() != 0) && (symtab->symoff() < _linkeditStartOffset) )
			throwf("malformed mach-o, symbol table not in __LINKEDIT");
		if ( symtab->stroff() < _linkeditStartOffset )
			throwf("malformed mach-o, symbol table strings not in __LINKEDIT");
	}

	// if linking flat and this is a flat dylib, create one atom that references all imported symbols
	if ( linkingFlatNamespace && linkingMainExecutable && ((header->flags() & MH_TWOLEVEL) == 0) ) {
		std::vector<const char*> importNames;
		importNames.reserve(dynamicInfo->nundefsym());
		const macho_nlist<P>* start = &symbolTable[dynamicInfo->iundefsym()];
		const macho_nlist<P>* end = &start[dynamicInfo->nundefsym()];
		for (const macho_nlist<P>* sym=start; sym < end; ++sym) {
			importNames.push_back(&strings[sym->n_strx()]);
		}
		this->_importAtom = new generic::dylib::ImportAtom(*this, importNames);
	}

	// build hash table
	if ( dyldInfo != nullptr )
		buildExportHashTableFromExportInfo(dyldInfo->export_off(), dyldInfo->export_size(), fileContent);
	else if ( exportsTrie != nullptr )
		buildExportHashTableFromExportInfo(exportsTrie->dataoff(), exportsTrie->datasize(), fileContent);
	else
		buildExportHashTableFromSymbolTable(dynamicInfo, symbolTable, strings, fileContent);
	
	// unmap file
	munmap((caddr_t)fileContent, fileLength);
}

template <typename A>
void File<A>::buildExportHashTableFromSymbolTable(const macho_dysymtab_command<P>* dynamicInfo,
												  const macho_nlist<P>* symbolTable,
												  const char* strings, const uint8_t* fileContent)
{
	if ( dynamicInfo->tocoff() == 0 ) {
		if ( this->_s_logHashtable )
			fprintf(stderr, "ld: building hashtable of %u toc entries for %s\n", dynamicInfo->nextdefsym(), this->path());
		const macho_nlist<P>* start = &symbolTable[dynamicInfo->iextdefsym()];
		const macho_nlist<P>* end = &start[dynamicInfo->nextdefsym()];
		this->reservedSymbolSpace(dynamicInfo->nextdefsym()); // set initial bucket count
		for (const macho_nlist<P>* sym=start; sym < end; ++sym) {
			this->addSymbol(&strings[sym->n_strx()], (sym->n_desc() & N_WEAK_DEF) != 0, false, sym->n_value());
		}
	}
	else {
		int32_t count = dynamicInfo->ntoc();
		this->reservedSymbolSpace(count); // set initial bucket count
		if ( this->_s_logHashtable )
			fprintf(stderr, "ld: building hashtable of %u entries for %s\n", count, this->path());
		const auto* toc = reinterpret_cast<const dylib_table_of_contents*>(fileContent + dynamicInfo->tocoff());
		for (int32_t i = 0; i < count; ++i) {
			const uint32_t index = E::get32(toc[i].symbol_index);
			const macho_nlist<P>* sym = &symbolTable[index];
			this->addSymbol(&strings[sym->n_strx()], (sym->n_desc() & N_WEAK_DEF) != 0, false, sym->n_value());
		}
	}
	
	// special case old libSystem
	if ( (this->_dylibInstallPath != nullptr) && (strcmp(this->_dylibInstallPath, "/usr/lib/libSystem.B.dylib") == 0) )
		addDyldFastStub();
}


template <typename A>
void File<A>::buildExportHashTableFromExportInfo(uint32_t exportsOffset, uint32_t exportsSize,
												 const uint8_t* fileContent)
{
	if ( this->_s_logHashtable )
		fprintf(stderr, "ld: building hashtable from export trie in %s\n", this->path());
	if ( exportsSize > 0 ) {
		const uint8_t* start = fileContent + exportsOffset;
		const uint8_t* end = &start[exportsSize];
		if ( (exportsOffset + exportsSize) > _fileLength )
			throwf("malformed mach-o dylib, exports trie extends beyond end of file");
		std::vector<mach_o::trie::Entry> list;
		parseTrie(start, end, list);
		for (const auto &entry : list)
			this->addSymbol(entry.name,
							entry.flags & EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION,
							(entry.flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL,
							entry.address);
	}
}

template <typename A>
void File<A>::addSymbol(const char* name, bool weakDef, bool tlv, pint_t address)
{
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

	// symbols that start with $ld$ are meta-data to the static linker
	// <rdar://problem/5182537> need way for ld and dyld to see different exported symbols in a dylib
	if ( strncmp(name, "$ld$", 4) == 0 ) {
		//    $ld$ <action> $ <condition> $ <symbol-name>
		const char* symAction = &name[4];
		const char* symCond = strchr(symAction, '$');
		// $ld$previous$/tmp/vers0.dylib$$1$10.0$10.4$_testSymbol$
		if ( symCond != nullptr ) {
			// FIXME: Once $ld$previous is submitted and documented we should issue warnings for $ld$add, $ld$hide, etc
			char curOSVers[16];
			sprintf(curOSVers, "$os%d.%d$", (linkMinOSVersion >> 16), ((linkMinOSVersion >> 8) & 0xFF));
			if ( strncmp(symCond, curOSVers, strlen(curOSVers)) == 0 ) {
				const char* symName = strchr(&symCond[1], '$');
				if ( symName != nullptr ) {
					++symName;
					if ( strncmp(symAction, "hide$", 5) == 0 ) {
						if ( this->_s_logHashtable )
							fprintf(stderr, "  adding %s to ignore set for %s\n", symName, this->path());
						this->_ignoreExports.insert(strdup(symName));
						return;
					}
					else if ( strncmp(symAction, "add$", 4) == 0 ) {
						this->addSymbol(symName, weakDef);
						return;
					}
					else if ( strncmp(symAction, "weak$", 5) == 0 ) {
						if ( !this->_allowWeakImports )
							this->_ignoreExports.insert(strdup(symName));
					}
					else if ( strncmp(symAction, "install_name$", 13) == 0 ) {
						this->_dylibInstallPath = strdup(symName);
						this->_installPathOverride = true;
						// <rdar://problem/14448206> CoreGraphics redirects to ApplicationServices, but with wrong compat version
						if ( strcmp(this->_dylibInstallPath, "/System/Library/Frameworks/ApplicationServices.framework/Versions/A/ApplicationServices") == 0 )
							this->_dylibCompatibilityVersion = Options::parseVersionNumber32("1.0");
						return;
					}
					else if ( strncmp(symAction, "compatibility_version$", 22) == 0 ) {
						this->_dylibCompatibilityVersion = Options::parseVersionNumber32(symName);
						return;
					}
					else {
						warning("bad symbol action: %s in dylib %s", name, this->path());
					}
				}
			}
		}
		else {
			warning("bad symbol condition: %s in dylib %s", name, this->path());
		}
	}

	// add symbol as possible export if we are not supposed to ignore it
	if ( this->_ignoreExports.count(name) == 0 ) {
		addExportedSymbol(name,  weakDef, tlv, address);
	}
}

template <>
void File<x86_64>::addDyldFastStub()
{
	addSymbol("dyld_stub_binder");
}

template <>
void File<x86>::addDyldFastStub()
{
	addSymbol("dyld_stub_binder");
}

template <typename A>
void File<A>::addDyldFastStub()
{
	// do nothing
}

template <typename A>
class Parser 
{
public:
	using P = typename A::P;

	static bool				validFile(const uint8_t* fileContent, bool executableOrDyliborBundle, bool subTypeMustMatch=false, uint32_t subType=0);
	static const char*		fileKind(const uint8_t* fileContent);
	static ld::Platform 	findPlatform(const macho_header<P>* header, uint64_t fileLength, uint32_t* minOsVers);
	static uint8_t			loadCommandSizeMask();
	static ld::dylib::File*	parse(const uint8_t* fileContent, uint64_t fileLength, const char* path,
								  time_t mTime, ld::File::Ordinal ordinal, const Options& opts,
								  bool indirectDylib, bool fromSDK)
	{
		return new File<A>(fileContent, fileLength, path, mTime, ordinal, opts.flatNamespace(),
						   opts.linkingMainExecutable(), opts.implicitlyLinkIndirectPublicDylibs(),
						   opts.platforms(), opts.allowWeakImports(),
						   opts.allowSimulatorToLinkWithMacOSX(), opts.addVersionLoadCommand(),
						   opts.targetIOSSimulator(), opts.logAllFiles(), opts.installPath(),
						   indirectDylib, opts.bundleBitcode(), opts.internalSDK(), fromSDK,
						   opts.platformMismatchesAreWarning());
	}

};

template <> uint8_t Parser<x86>::loadCommandSizeMask()		{ return 0x03; }
template <> uint8_t Parser<x86_64>::loadCommandSizeMask()	{ return 0x07; }
template <> uint8_t Parser<arm>::loadCommandSizeMask()		{ return 0x03; }
template <> uint8_t Parser<arm64>::loadCommandSizeMask()	{ return 0x07; }
#if SUPPORT_ARCH_arm64_32
template <> uint8_t Parser<arm64_32>::loadCommandSizeMask()	{ return 0x03; }
#endif

template <typename A>
ld::Platform Parser<A>::findPlatform(const macho_header<P>* header, uint64_t fileLength, uint32_t* minOsVers)
{
	const uint32_t cmd_count = header->ncmds();
	if ( cmd_count == 0 )
		return ld::Platform::unknown;
	const uint8_t* const endOfFile = (uint8_t*)header + fileLength;
	const macho_load_command<P>* const cmds = (macho_load_command<P>*)((char*)header + sizeof(macho_header<P>));
	const macho_load_command<P>* const cmdsEnd = (macho_load_command<P>*)((char*)header + sizeof(macho_header<P>) + header->sizeofcmds());
	const macho_load_command<P>* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		uint32_t size = cmd->cmdsize();
		if ( (size & loadCommandSizeMask()) != 0 )
			throwf("load command #%d has a unaligned size", i);
		const uint8_t* endOfCmd = ((uint8_t*)cmd)+cmd->cmdsize();
		if ( endOfCmd > (uint8_t*)cmdsEnd )
			throwf("load command #%d extends beyond the end of the load commands", i);
		if ( endOfCmd > endOfFile )
			throwf("load command #%d extends beyond the end of the file", i);
		const macho_version_min_command<P>* versCmd = (macho_version_min_command<P>*)cmd;
		const macho_build_version_command<P>* buildVersCmd = (macho_build_version_command<P>*)cmd;
		switch (cmd->cmd()) {
			case LC_VERSION_MIN_MACOSX:
			case LC_VERSION_MIN_IPHONEOS:
			case LC_VERSION_MIN_WATCHOS:
			case LC_VERSION_MIN_TVOS:
				*minOsVers = versCmd->version();
				return ld::platformForLoadCommand(cmd->cmd(), (mach_header*)header);
			case LC_BUILD_VERSION:
				*minOsVers = buildVersCmd->minos();
				return ld::platformFromBuildVersion(buildVersCmd->platform(), (mach_header*)header);
		}
		cmd = (const macho_load_command<P>*)(((char*)cmd)+cmd->cmdsize());
		if ( cmd > cmdsEnd )
			throwf("malformed mach-o file, load command #%d is outside size of load commands", i);
	}
	*minOsVers = 0;
	return ld::Platform::unknown;
}

template <>
bool Parser<x86>::validFile(const uint8_t* fileContent, bool executableOrDyliborBundle, bool subTypeMustMatch, uint32_t subType)
{
	const auto* header = reinterpret_cast<const macho_header<P>*>(fileContent);
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_I386 )
		return false;
	switch ( header->filetype() ) {
		case MH_DYLIB:
		case MH_DYLIB_STUB:
			return true;
		case MH_BUNDLE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with bundle (MH_BUNDLE) only dylibs (MH_DYLIB)";
		case MH_EXECUTE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with a main executable";
		default:
			return false;
	}
}

template <>
bool Parser<x86_64>::validFile(const uint8_t* fileContent, bool executableOrDyliborBundle, bool subTypeMustMatch, uint32_t subType)
{
	const auto* header = reinterpret_cast<const macho_header<P>*>(fileContent);
	if ( header->magic() != MH_MAGIC_64 )
		return false;
	if ( header->cputype() != CPU_TYPE_X86_64 )
		return false;
	switch ( header->filetype() ) {
		case MH_DYLIB:
		case MH_DYLIB_STUB:
			return true;
		case MH_BUNDLE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with bundle (MH_BUNDLE) only dylibs (MH_DYLIB)";
		case MH_EXECUTE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with a main executable";
		default:
			return false;
	}
}

template <>
bool Parser<arm>::validFile(const uint8_t* fileContent, bool executableOrDyliborBundle, bool subTypeMustMatch, uint32_t subType)
{
	const auto* header = reinterpret_cast<const macho_header<P>*>(fileContent);
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM )
		return false;
	if ( subTypeMustMatch && (header->cpusubtype() != (subType & ~CPU_SUBTYPE_MASK)) )
		return false;
	switch ( header->filetype() ) {
		case MH_DYLIB:
		case MH_DYLIB_STUB:
			return true;
		case MH_BUNDLE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with bundle (MH_BUNDLE) only dylibs (MH_DYLIB)";
		case MH_EXECUTE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with a main executable";
		default:
			return false;
	}
}



template <>
bool Parser<arm64>::validFile(const uint8_t* fileContent, bool executableOrDyliborBundle, bool subTypeMustMatch, uint32_t subType)
{
	const auto* header = reinterpret_cast<const macho_header<P>*>(fileContent);
	if ( header->magic() != MH_MAGIC_64 )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM64 )
		return false;
	// <rdar://problem/62555624> arm can link with arm64e, but not the other way around
	if ( (subType & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_ARM64E ) {
		if ( header->cpusubtype() != subType )
			return false;
	}
	if ( subTypeMustMatch && (header->cpusubtype() != (subType & ~CPU_SUBTYPE_MASK)) )
		return false;
	switch ( header->filetype() ) {
		case MH_DYLIB:
		case MH_DYLIB_STUB:
			return true;
		case MH_BUNDLE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with bundle (MH_BUNDLE) only dylibs (MH_DYLIB)";
		case MH_EXECUTE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with a main executable";
		default:
			return false;
	}
}

#if SUPPORT_ARCH_arm64_32
template <>
bool Parser<arm64_32>::validFile(const uint8_t* fileContent, bool executableOrDyliborBundle, bool subTypeMustMatch, uint32_t subType)
{
	const macho_header<P>* header = (const macho_header<P>*)fileContent;
	if ( header->magic() != MH_MAGIC )
		return false;
	if ( header->cputype() != CPU_TYPE_ARM64_32 )
		return false;
	switch ( header->filetype() ) {
		case MH_DYLIB:
		case MH_DYLIB_STUB:
			return true;
		case MH_BUNDLE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with bundle (MH_BUNDLE) only dylibs (MH_DYLIB)";
		case MH_EXECUTE:
			if ( executableOrDyliborBundle )
				return true;
			else
				throw "can't link with a main executable";
		default:
			return false;
	}
}
#endif

bool isDylibFile(const uint8_t* fileContent, uint64_t fileLength, cpu_type_t* result, cpu_subtype_t* subResult, ld::Platform* platform, uint32_t* minOsVers)
{
	if ( Parser<x86_64>::validFile(fileContent, false) ) {
		*result = CPU_TYPE_X86_64;
		const auto* header = reinterpret_cast<const macho_header<Pointer64<LittleEndian>>*>(fileContent);
		*subResult = header->cpusubtype();
		*platform = Parser<x86_64>::findPlatform(header, fileLength, minOsVers);
		return true;
	}
	if ( Parser<x86>::validFile(fileContent, false) ) {
		*result = CPU_TYPE_I386;
		*subResult = CPU_SUBTYPE_X86_ALL;
		const auto* header = reinterpret_cast<const macho_header<Pointer32<LittleEndian>>*>(fileContent);
		*platform = Parser<x86>::findPlatform(header, fileLength, minOsVers);
		return true;
	}
	if ( Parser<arm>::validFile(fileContent, false) ) {
		*result = CPU_TYPE_ARM;
		const auto* header = reinterpret_cast<const macho_header<Pointer32<LittleEndian>>*>(fileContent);
		*subResult = header->cpusubtype();
		*platform = Parser<arm>::findPlatform(header, fileLength, minOsVers);
		return true;
	}
	if ( Parser<arm64>::validFile(fileContent, false) ) {
		*result = CPU_TYPE_ARM64;
		const auto* header = reinterpret_cast<const macho_header<Pointer64<LittleEndian>>*>(fileContent);
		*subResult = header->cpusubtype();
		*platform = Parser<arm64>::findPlatform(header, fileLength, minOsVers);
		return true;
	}
#if SUPPORT_ARCH_arm64_32
	if ( Parser<arm64_32>::validFile(fileContent, false) ) {
		*result = CPU_TYPE_ARM64_32;
		*subResult = CPU_SUBTYPE_ARM64_32_V8;
		const auto* header = reinterpret_cast<const macho_header<Pointer32<LittleEndian>>*>(fileContent);
		*platform = Parser<arm64_32>::findPlatform(header, fileLength, minOsVers);
		return true;
	}
#endif
	return false;
}

template <>
const char* Parser<x86>::fileKind(const uint8_t* fileContent)
{
	const auto* header = reinterpret_cast<const macho_header<P>*>(fileContent);
	if ( header->magic() != MH_MAGIC )
		return nullptr;
	if ( header->cputype() != CPU_TYPE_I386 )
		return nullptr;
	return "i386";
}

template <>
const char* Parser<x86_64>::fileKind(const uint8_t* fileContent)
{
	const auto* header = reinterpret_cast<const macho_header<P>*>(fileContent);
	if ( header->magic() != MH_MAGIC_64 )
		return nullptr;
	if ( header->cputype() != CPU_TYPE_X86_64 )
		return nullptr;
	return "x86_64";
}

template <>
const char* Parser<arm>::fileKind(const uint8_t* fileContent)
{
	const auto* header = reinterpret_cast<const macho_header<P>*>(fileContent);
	if ( header->magic() != MH_MAGIC )
		return nullptr;
	if ( header->cputype() != CPU_TYPE_ARM )
		return nullptr;
	for (const auto* t = archInfoArray; t->archName != nullptr; ++t) {
		if ( (t->cpuType == CPU_TYPE_ARM) && ((cpu_subtype_t)header->cpusubtype() == t->cpuSubType) ) {
			return t->archName;
		}
	}
	return "arm???";
}

#if SUPPORT_ARCH_arm64
template <>
const char* Parser<arm64>::fileKind(const uint8_t* fileContent)
{
  const auto* header = reinterpret_cast<const macho_header<P>*>(fileContent);
  if ( header->magic() != MH_MAGIC_64 )
    return nullptr;
  if ( header->cputype() != CPU_TYPE_ARM64 )
    return nullptr;
  return "arm64";
}
#endif

#if SUPPORT_ARCH_arm64_32
template <>
const char* Parser<arm64_32>::fileKind(const uint8_t* fileContent)
{
  const macho_header<P>* header = (const macho_header<P>*)fileContent;
  if ( header->magic() != MH_MAGIC )
    return NULL;
  if ( header->cputype() != CPU_TYPE_ARM64_32 )
    return NULL;
  return "arm64_32";
}
#endif

//
// used by linker is error messages to describe mismatched files
//
const char* archName(const uint8_t* fileContent)
{
	if ( Parser<x86_64>::validFile(fileContent, true) ) {
		return Parser<x86_64>::fileKind(fileContent);
	}
	if ( Parser<x86>::validFile(fileContent, true) ) {
		return Parser<x86>::fileKind(fileContent);
	}
	if ( Parser<arm>::validFile(fileContent, true) ) {
		return Parser<arm>::fileKind(fileContent);
	}
#if SUPPORT_ARCH_arm64
	if ( Parser<arm64>::validFile(fileContent, true) ) {
		return Parser<arm64>::fileKind(fileContent);
	}
#endif
#if SUPPORT_ARCH_arm64_32
	if ( Parser<arm64_32>::validFile(fileContent, true) ) {
		return Parser<arm64_32>::fileKind(fileContent);
	}
#endif
	return nullptr;
}


static ld::dylib::File* parseAsArchitecture(const uint8_t* fileContent, uint64_t fileLength, const char* path,
											time_t modTime, const Options& opts, ld::File::Ordinal ordinal,
											bool bundleLoader, bool indirectDylib, bool fromSDK,
											cpu_type_t architecture, cpu_subtype_t subArchitecture)
{
	bool subTypeMustMatch = opts.enforceDylibSubtypesMatch();
	switch ( architecture) {
#if SUPPORT_ARCH_x86_64
		case CPU_TYPE_X86_64:
			if ( Parser<x86_64>::validFile(fileContent, bundleLoader, subTypeMustMatch, subArchitecture) )
				return Parser<x86_64>::parse(fileContent, fileLength, path, modTime, ordinal, opts, indirectDylib, fromSDK);
			break;
#endif
#if SUPPORT_ARCH_i386
		case CPU_TYPE_I386:
			if ( Parser<x86>::validFile(fileContent, bundleLoader, subTypeMustMatch, subArchitecture) )
				return Parser<x86>::parse(fileContent, fileLength, path, modTime, ordinal, opts, indirectDylib, fromSDK);
			break;
#endif
#if SUPPORT_ARCH_arm_any
		case CPU_TYPE_ARM:
			if ( Parser<arm>::validFile(fileContent, bundleLoader, subTypeMustMatch, subArchitecture) )
				return Parser<arm>::parse(fileContent, fileLength, path, modTime, ordinal, opts, indirectDylib, fromSDK);
			break;
#endif
#if SUPPORT_ARCH_arm64
		case CPU_TYPE_ARM64:
			if ( Parser<arm64>::validFile(fileContent, bundleLoader, subTypeMustMatch, subArchitecture) )
				return Parser<arm64>::parse(fileContent, fileLength, path, modTime, ordinal, opts, indirectDylib, fromSDK);
			break;
#endif
#if SUPPORT_ARCH_arm64_32
		case CPU_TYPE_ARM64_32:
			if ( Parser<arm64_32>::validFile(fileContent, bundleLoader, subTypeMustMatch, subArchitecture) )
				return Parser<arm64_32>::parse(fileContent, fileLength, path, modTime, ordinal, opts, indirectDylib, fromSDK);
			break;
#endif
	}
	return nullptr;
}

//
// main function used by linker to instantiate ld::Files
//
ld::dylib::File* parse(const uint8_t* fileContent, uint64_t fileLength, const char* path,
					   time_t modtime, const Options& opts, ld::File::Ordinal ordinal,
					   bool bundleLoader, bool indirectDylib, bool fromSDK)
{
	// First make sure we are even a dylib with a known arch.  If we aren't then there's no point in continuing.
	if (!archName(fileContent))
		return nullptr;

	auto file = parseAsArchitecture(fileContent, fileLength, path, modtime, opts, ordinal, bundleLoader, indirectDylib, fromSDK, opts.architecture(), opts.subArchitecture());

	// If we've been provided with an architecture we can fall back to, try to parse the dylib as that instead.
	if (!file && opts.fallbackArchitecture()) {
		warning("architecture %s not present in dylib file %s, attempting fallback", opts.architectureName(), path);
		file = parseAsArchitecture(fileContent, fileLength, path, modtime, opts, ordinal, bundleLoader, indirectDylib, fromSDK, opts.fallbackArchitecture(), opts.fallbackSubArchitecture());
	}
		
	return file;
}
	


}; // namespace dylib
}; // namespace mach_o
