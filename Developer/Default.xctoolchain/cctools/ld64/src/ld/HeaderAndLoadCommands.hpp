/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-*
 *
 * Copyright (c) 2009-2011 Apple Inc. All rights reserved.
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

#ifndef __HEADER_LOAD_COMMANDS_HPP__
#define __HEADER_LOAD_COMMANDS_HPP__

#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <mach-o/loader.h>

#include <vector>

#include "MachOFileAbstraction.hpp"
#include "Options.h"
#include "ld.hpp"

namespace ld {
namespace tool {

class HeaderAndLoadCommandsAbtract : public ld::Atom
{
public:
							HeaderAndLoadCommandsAbtract(const ld::Section& sect, ld::Atom::Definition d, 
												ld::Atom::Combine c, ld::Atom::Scope s, ld::Atom::ContentType ct, 
												ld::Atom::SymbolTableInclusion i, bool dds, bool thumb, bool al, 
												ld::Atom::Alignment a) : ld::Atom(sect, d, c, s, ct, i, dds, thumb, al, a) { }

	virtual void setUUID(const uint8_t digest[16]) = 0;
	virtual void recopyUUIDCommand() = 0;
	virtual const uint8_t* getUUID() const = 0;
	virtual bool bitcodeBundleCommand(uint64_t& cmdOffset, uint64_t& cmdEnd,
									  uint64_t& sectOffset, uint64_t& sectEnd) const = 0;
	virtual void linkeditCmdInfo(uint64_t& offset, uint64_t& size) const = 0;
	virtual void symbolTableCmdInfo(uint64_t& offset, uint64_t& size) const = 0;

};

template <typename A>
class HeaderAndLoadCommandsAtom : public HeaderAndLoadCommandsAbtract
{
public:
												HeaderAndLoadCommandsAtom(const Options& opts, ld::Internal& state, 
																			OutputFile& writer);

	// overrides of ld::Atom
	virtual ld::File*							file() const		{ return NULL; }
	virtual const char*							name() const		{ return "mach-o header and load commands"; }
	virtual uint64_t							size() const;
	virtual uint64_t							objectAddress() const { return _address; }

	virtual void								copyRawContent(uint8_t buffer[]) const;

	// overrides of HeaderAndLoadCommandsAbtract
	virtual void setUUID(const uint8_t digest[16])	{ memcpy(_uuid, digest, 16); }
	virtual void recopyUUIDCommand();
	virtual const uint8_t* getUUID() const                          { return &_uuid[0]; }
	virtual bool bitcodeBundleCommand(uint64_t& cmdOffset, uint64_t& cmdEnd,
									  uint64_t& sectOffset, uint64_t& sectEnd) const;
	virtual void linkeditCmdInfo(uint64_t& offset, uint64_t& size) const;
	virtual void symbolTableCmdInfo(uint64_t& offset, uint64_t& size) const;


private:
	typedef typename A::P						P;
	typedef typename A::P::E					E;
	typedef typename A::P::uint_t				pint_t;
	
	unsigned int				nonHiddenSectionCount() const;
	unsigned int				segmentCount() const;
	static uint32_t				alignedSize(uint32_t x);
	uint32_t					magic() const;
	uint32_t					cpuType() const;
	uint32_t					cpuSubType() const;
	uint32_t					flags() const;
	uint32_t					fileType() const;
	uint32_t					commandsCount() const;
	uint32_t					threadLoadCommandSize() const;
	uint8_t*					copySingleSegmentLoadCommand(uint8_t* p) const;
	uint8_t*					copySegmentLoadCommands(uint8_t* p, uint8_t* base) const;
	uint8_t*					copyDyldInfoLoadCommand(uint8_t* p) const;
	uint8_t*					copyExportsTrieLoadCommand(uint8_t* p) const;
	uint8_t*					copyChainedFixupsLoadCommand(uint8_t* p) const;
	uint8_t*					copySymbolTableLoadCommand(uint8_t* p, uint8_t* base) const;
	uint8_t*					copyDynamicSymbolTableLoadCommand(uint8_t* p) const;
	uint8_t*					copyDyldLoadCommand(uint8_t* p) const;
	uint8_t*					copyDylibIDLoadCommand(uint8_t* p) const;
	uint8_t*					copyRoutinesLoadCommand(uint8_t* p) const;
	uint8_t*					copyUUIDLoadCommand(uint8_t* p) const;
	uint8_t*					copyVersionLoadCommand(uint8_t* p, ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion) const;
	uint8_t*					copyBuildVersionLoadCommand(uint8_t* p, ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion) const;
	uint8_t*					copySourceVersionLoadCommand(uint8_t* p) const;
	uint8_t*					copyThreadsLoadCommand(uint8_t* p) const;
	uint8_t*					copyEntryPointLoadCommand(uint8_t* p) const;
	uint8_t*					copyEncryptionLoadCommand(uint8_t* p) const;
	uint8_t*					copySplitSegInfoLoadCommand(uint8_t* p) const;
	uint8_t*					copyDylibLoadCommand(uint8_t* p, const ld::dylib::File*) const;
	uint8_t*					copyRPathLoadCommand(uint8_t* p, const char*) const;
	uint8_t*					copySubFrameworkLoadCommand(uint8_t* p) const;
	uint8_t*					copyAllowableClientLoadCommand(uint8_t* p, const char* client) const;
	uint8_t*					copySubLibraryLoadCommand(uint8_t* p, const char* name) const;
	uint8_t*					copySubUmbrellaLoadCommand(uint8_t* p, const char* name) const;
	uint8_t*					copyFunctionStartsLoadCommand(uint8_t* p) const;
	uint8_t*					copyDataInCodeLoadCommand(uint8_t* p) const;
	uint8_t*					copyDyldEnvLoadCommand(uint8_t* p, const char* env) const;
	uint8_t*					copyLinkerOptionsLoadCommand(uint8_t* p, const std::vector<const char*>&) const;
	uint8_t*					copyOptimizationHintsLoadCommand(uint8_t* p) const;
	uint8_t*					copyCodeSignatureLoadCommand(uint8_t* p) const;

	uint32_t					sectionFlags(ld::Internal::FinalSection* sect) const;
	bool						sectionTakesNoDiskSpace(ld::Internal::FinalSection* sect) const;
	

	const Options&				_options;
	ld::Internal&				_state;
	OutputFile&					_writer;
	pint_t						_address;
	bool						_hasDyldInfoLoadCommand;
	bool						_hasDyldLoadCommand;
	bool						_hasDylibIDLoadCommand;
	bool						_hasThreadLoadCommand;
	bool						_hasEntryPointLoadCommand;
	bool						_hasEncryptionLoadCommand;
	bool						_hasSplitSegInfoLoadCommand;
	bool						_hasRoutinesLoadCommand;
	bool						_hasUUIDLoadCommand;
	bool						_hasSymbolTableLoadCommand;
	bool						_hasDynamicSymbolTableLoadCommand;
	bool						_hasRPathLoadCommands;
	bool						_hasSubFrameworkLoadCommand;
	bool						_hasVersionLoadCommand;
	bool						_hasFunctionStartsLoadCommand;
	bool						_hasDataInCodeLoadCommand;
	bool						_hasSourceVersionLoadCommand;
	bool						_hasOptimizationHints;
	bool						_hasExportsTrieLoadCommand;
	bool						_hasChainedFixupsLoadCommand;
	bool						_hasCodeSignature;
	bool						_simulatorSupportDylib;
	ld::VersionSet			 	_platforms;
	uint32_t					_dylibLoadCommmandsCount;
	uint32_t					_allowableClientLoadCommmandsCount;
	uint32_t					_dyldEnvironExrasCount;
	std::vector<const char*>	_subLibraryNames;
	std::vector<const char*>	_subUmbrellaNames;
	uint8_t						_uuid[16];
	mutable macho_uuid_command<P>*	_uuidCmdInOutputBuffer;
	mutable uint32_t			_linkeditCmdOffset;
	mutable uint32_t			_symboltableCmdOffset;
	std::vector< std::vector<const char*> >	 _linkerOptions;
	std::unordered_set<uint64_t>&	_toolsVersions;
	
	static ld::Section			_s_section;
	static ld::Section			_s_preload_section;
};

template <typename A>
ld::Section HeaderAndLoadCommandsAtom<A>::_s_section("__TEXT", "__mach_header", ld::Section::typeMachHeader, true);
template <typename A>
ld::Section HeaderAndLoadCommandsAtom<A>::_s_preload_section("__HEADER", "__mach_header", ld::Section::typeMachHeader, true);


template <typename A>
HeaderAndLoadCommandsAtom<A>::HeaderAndLoadCommandsAtom(const Options& opts, ld::Internal& state, OutputFile& writer)
	: HeaderAndLoadCommandsAbtract((opts.outputKind() == Options::kPreload) ? _s_preload_section : _s_section, 
				ld::Atom::definitionRegular, ld::Atom::combineNever, 
				ld::Atom::scopeTranslationUnit, ld::Atom::typeUnclassified, 
				ld::Atom::symbolTableNotIn, false, false, false, 
				(opts.outputKind() == Options::kPreload) ? ld::Atom::Alignment(0) : ld::Atom::Alignment(log2(opts.segmentAlignment())) ),
		_options(opts), _state(state), _writer(writer), _address(0), _uuidCmdInOutputBuffer(NULL), _linkeditCmdOffset(0), _symboltableCmdOffset(0),
		_toolsVersions(state.toolsVersions)
{
	bzero(_uuid, 16);
	_hasDyldInfoLoadCommand = opts.makeCompressedDyldInfo() || state.cantUseChainedFixups;
	_hasDyldLoadCommand = ((opts.outputKind() == Options::kDynamicExecutable) || (_options.outputKind() == Options::kDyld));
	_hasDylibIDLoadCommand = (opts.outputKind() == Options::kDynamicLibrary);
	_hasThreadLoadCommand = _options.needsThreadLoadCommand();
	_hasEntryPointLoadCommand = _options.needsEntryPointLoadCommand();
	_hasEncryptionLoadCommand = opts.makeEncryptable();
	_hasSplitSegInfoLoadCommand = opts.sharedRegionEligible();
	_hasRoutinesLoadCommand = (opts.initFunctionName() != NULL) && (state.entryPoint != NULL);
	_hasSymbolTableLoadCommand = true;
	_hasUUIDLoadCommand = (opts.UUIDMode() != Options::kUUIDNone);
	_hasOptimizationHints = (_state.someObjectHasOptimizationHints && (opts.outputKind() == Options::kObjectFile));
	_hasExportsTrieLoadCommand = opts.makeChainedFixups() && !state.cantUseChainedFixups && opts.dyldLoadsOutput();
	_hasCodeSignature = opts.adHocSign();
	_hasChainedFixupsLoadCommand = opts.makeChainedFixups() && !state.cantUseChainedFixups && opts.dyldOrKernelLoadsOutput();

	switch ( opts.outputKind() ) {
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
		case Options::kDyld:
		case Options::kKextBundle:
			_hasDynamicSymbolTableLoadCommand = true;
			break;
		case Options::kObjectFile:
			if ( ! state.someObjectFileHasDwarf )
				_hasUUIDLoadCommand = false;
			_hasDynamicSymbolTableLoadCommand = false;
			for (std::vector<ld::Internal::FinalSection*>::iterator it = _state.sections.begin(); it != _state.sections.end(); ++it) {
				if ( ((*it)->type() == ld::Section::typeNonLazyPointer) || ((*it)->type() == ld::Section::typeTLVPointers) ) {
					_hasDynamicSymbolTableLoadCommand = true;
					break;
				}
			}
			for (const char* frameworkName : _state.unprocessedLinkerOptionFrameworks) {
				std::vector<const char*>* lo = new std::vector<const char*>();
				if ( _state.linkerOptionNeededFrameworks.count(frameworkName) )
					lo->push_back("-needed_framework");
				else
					lo->push_back("-framework");
				lo->push_back(frameworkName);
				_linkerOptions.push_back(*lo);
			};
			for (const char* libName : _state.unprocessedLinkerOptionLibraries) {
				std::vector<const char*>* lo = new std::vector<const char*>();
				char * s = new char[strlen(libName)+3];
				if ( _state.linkerOptionNeededLibraries.count(libName) )
					strcpy(s, "-needed-l");
				else
					strcpy(s, "-l");
				strcat(s, libName);
				lo->push_back(s);
				_linkerOptions.push_back(*lo);
			};
			break;
		case Options::kStaticExecutable:
			_hasDynamicSymbolTableLoadCommand = opts.positionIndependentExecutable();
			break;
		case Options::kPreload:
			_hasDynamicSymbolTableLoadCommand = opts.positionIndependentExecutable();
			break;
	}
	_hasRPathLoadCommands = (_options.rpaths().size() != 0);
	_hasSubFrameworkLoadCommand = (_options.umbrellaName() != NULL);
	_platforms = _options.platforms();
	_hasVersionLoadCommand = _options.addVersionLoadCommand();
	// in ld -r mode, only if all input .o files have load command, then add one to output
	if ( !_hasVersionLoadCommand && (_options.outputKind() == Options::kObjectFile) && !state.objectFileFoundWithNoVersion )
		_hasVersionLoadCommand = true;
	_hasFunctionStartsLoadCommand = _options.addFunctionStarts();
	_hasDataInCodeLoadCommand = _options.addDataInCodeInfo();
	_hasSourceVersionLoadCommand = _options.needsSourceVersionLoadCommand();
	_dylibLoadCommmandsCount = _writer.dylibCount();
	_allowableClientLoadCommmandsCount = _options.allowableClients().size();
	_dyldEnvironExrasCount = _options.dyldEnvironExtras().size();
	
	if ( ! _options.useSimplifiedDylibReExports() ) {
		// target OS does not support LC_REEXPORT_DYLIB, so use old complicated load commands
		for(uint32_t ord=1; ord <= _writer.dylibCount(); ++ord) {
			const ld::dylib::File* dylib = _writer.dylibByOrdinal(ord);
			if ( dylib->willBeReExported() ) {
				// if child says it is an sub-framework of the image being created, then nothing to do here
				bool isSubFramework = false;
				const char* childInUmbrella = dylib->parentUmbrella();
				if ( childInUmbrella != NULL ) {
					const char* myLeaf = strrchr(_options.installPath(), '/');
					if ( myLeaf != NULL ) {
						if ( strcmp(childInUmbrella, &myLeaf[1]) == 0 )
							isSubFramework = true;
					}
				}
				// LC_SUB_FRAMEWORK is in child, so do nothing in parent 
				if ( ! isSubFramework ) {
					// this dylib also needs a sub_x load command
					bool isFrameworkReExport = false;
					const char* lastSlash = strrchr(dylib->installPath(), '/');
					if ( lastSlash != NULL ) {
						char frameworkName[strlen(lastSlash)+20];
						sprintf(frameworkName, "/%s.framework/", &lastSlash[1]);
						isFrameworkReExport = (strstr(dylib->installPath(), frameworkName) != NULL);
					}
					if ( isFrameworkReExport ) {
						// needs a LC_SUB_UMBRELLA command
						_subUmbrellaNames.push_back(&lastSlash[1]);
					}
					else {
						// needs a LC_SUB_LIBRARY command
						const char* nameStart = &lastSlash[1];
						if ( lastSlash == NULL )
							nameStart = dylib->installPath();
						int len = strlen(nameStart);
						const char* dot = strchr(nameStart, '.');
						if ( dot != NULL )
							len = dot - nameStart;
						char* subLibName = new char[len+1];
						strlcpy(subLibName, nameStart, len+1);
						_subLibraryNames.push_back(subLibName);
					}
				}
			}
		}
	}
}

template <typename A>
uint32_t HeaderAndLoadCommandsAtom<A>::alignedSize(uint32_t size)
{
	if ( sizeof(pint_t) == 4 )
		return ((size+3) & (-4));	// 4-byte align all load commands for 32-bit mach-o
	else
		return ((size+7) & (-8));	// 8-byte align all load commands for 64-bit mach-o
}


template <typename A>
unsigned int HeaderAndLoadCommandsAtom<A>::nonHiddenSectionCount() const
{
	unsigned int count = 0;
	for (std::vector<ld::Internal::FinalSection*>::iterator it = _state.sections.begin(); it != _state.sections.end(); ++it) {
		if ( ! (*it)->isSectionHidden() && ((*it)->type() != ld::Section::typeTentativeDefs) )
			++count;
	}
	return count;
}

template <typename A>
unsigned int HeaderAndLoadCommandsAtom<A>::segmentCount() const
{
	if ( _options.outputKind() == Options::kObjectFile ) {
		// .o files have one anonymous segment that contains all sections
		return 1;
	}
	
	unsigned int count = 0;
	const char* lastSegName = "";
	for (std::vector<ld::Internal::FinalSection*>::iterator it = _state.sections.begin(); it != _state.sections.end(); ++it) {
		if ( _options.outputKind() == Options::kPreload ) {
			if ( (*it)->type() == ld::Section::typeMachHeader )
				continue; // for -preload, don't put hidden __HEADER segment into output
			if ( (*it)->type() == ld::Section::typeLinkEdit )
				continue; // for -preload, don't put hidden __LINKEDIT segment into output
		}
		if ( strcmp(lastSegName, (*it)->segmentName()) != 0 ) {
			lastSegName = (*it)->segmentName();
			++count;
		}
	}
	return count;
}

template <typename A>
bool HeaderAndLoadCommandsAtom<A>::bitcodeBundleCommand(uint64_t &cmdOffset, uint64_t &cmdEnd,
														uint64_t &sectOffset, uint64_t &sectEnd) const
{
	if ( _options.outputKind() == Options::kObjectFile ) {
		return false;
	}
	cmdOffset = sizeof(macho_header<P>);
	const char* lastSegName = "";
	for (std::vector<ld::Internal::FinalSection*>::iterator it = _state.sections.begin(); it != _state.sections.end(); ++it) {
		if ( strcmp(lastSegName, (*it)->segmentName()) != 0 ) {
			lastSegName = (*it)->segmentName();
			cmdOffset += sizeof(macho_segment_command<P>);
		}
		if ( strcmp((*it)->segmentName(), "__LLVM") == 0 && strcmp((*it)->sectionName(), "__bundle") == 0 ) {
			sectOffset = (*it)->fileOffset;
			sectEnd = (*(it + 1))->fileOffset;
			cmdEnd = cmdOffset + sizeof(macho_section<P>);
			return true;
		}
		if ( ! (*it)->isSectionHidden() )
			cmdOffset += sizeof(macho_section<P>);
	}
	return false;
}

template <typename A>
void HeaderAndLoadCommandsAtom<A>::linkeditCmdInfo(uint64_t &offset, uint64_t &size) const
{
	offset = _linkeditCmdOffset;
	size = sizeof(macho_segment_command<P>);
}

template <typename A>
void HeaderAndLoadCommandsAtom<A>::symbolTableCmdInfo(uint64_t &offset, uint64_t &size) const
{
	offset = _symboltableCmdOffset;
	size = sizeof(macho_symtab_command<P>);
}


template <typename A>
uint64_t HeaderAndLoadCommandsAtom<A>::size() const
{
	__block uint32_t sz = sizeof(macho_header<P>);
	
	sz += sizeof(macho_segment_command<P>) * this->segmentCount();
	sz += sizeof(macho_section<P>) * this->nonHiddenSectionCount();

	if ( _hasDylibIDLoadCommand )
		sz += alignedSize(sizeof(macho_dylib_command<P>) + strlen(_options.installPath()) + 1);
		
	if ( _hasDyldInfoLoadCommand )
		sz += sizeof(macho_dyld_info_command<P>);

	if ( _hasChainedFixupsLoadCommand )
		sz += sizeof(linkedit_data_command);

	if ( _hasExportsTrieLoadCommand )
		sz += sizeof(linkedit_data_command);

	if ( _hasSymbolTableLoadCommand )
		sz += sizeof(macho_symtab_command<P>);
		
	if ( _hasDynamicSymbolTableLoadCommand )
		sz += sizeof(macho_dysymtab_command<P>);
	
	if ( _hasDyldLoadCommand )
		sz += alignedSize(sizeof(macho_dylinker_command<P>) + strlen(_options.dyldInstallPath()) + 1);

	if ( _hasRoutinesLoadCommand ) 
		sz += sizeof(macho_routines_command<P>);
		
	if ( _hasUUIDLoadCommand )
		sz += sizeof(macho_uuid_command<P>);

	if ( _hasVersionLoadCommand ) {
		if ( _hasVersionLoadCommand ) {
			_options.platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
				if (_options.shouldUseBuildVersion(platform, minVersion)) {
					sz += alignedSize(sizeof(macho_build_version_command<P>) + sizeof(macho_build_tool_version<P>)*_toolsVersions.size());
				} else {
					sz += sizeof(macho_version_min_command<P>);
				}
			});
		}
	}

	if ( _hasSourceVersionLoadCommand )
		sz += sizeof(macho_source_version_command<P>);
		
	if ( _hasThreadLoadCommand )
		sz += this->threadLoadCommandSize();

	if ( _hasEntryPointLoadCommand )
		sz += sizeof(macho_entry_point_command<P>);
		
	if ( _hasEncryptionLoadCommand )
		sz += sizeof(macho_encryption_info_command<P>);

	if ( _hasSplitSegInfoLoadCommand )
		sz += sizeof(macho_linkedit_data_command<P>);
	
	for(uint32_t ord=1; ord <= _writer.dylibCount(); ++ord) {
		sz += alignedSize(sizeof(macho_dylib_command<P>) + strlen(_writer.dylibByOrdinal(ord)->installPath()) + 1);
	}
	
	if ( _hasRPathLoadCommands ) {
		const std::vector<const char*>& rpaths = _options.rpaths();
		for (std::vector<const char*>::const_iterator it = rpaths.begin(); it != rpaths.end(); ++it) {
			sz += alignedSize(sizeof(macho_rpath_command<P>) + strlen(*it) + 1);
		}
	}
	
	if ( _hasSubFrameworkLoadCommand )
		sz += alignedSize(sizeof(macho_sub_framework_command<P>) + strlen(_options.umbrellaName()) + 1);
	
	for (std::vector<const char*>::const_iterator it = _subLibraryNames.begin(); it != _subLibraryNames.end(); ++it) {
		sz += alignedSize(sizeof(macho_sub_library_command<P>) + strlen(*it) + 1);
	}

	for (std::vector<const char*>::const_iterator it = _subUmbrellaNames.begin(); it != _subUmbrellaNames.end(); ++it) {
		sz += alignedSize(sizeof(macho_sub_umbrella_command<P>) + strlen(*it) + 1);
	}

	if ( _allowableClientLoadCommmandsCount != 0 ) {
		const std::vector<const char*>& clients = _options.allowableClients();
		for (std::vector<const char*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
			sz += alignedSize(sizeof(macho_sub_client_command<P>) + strlen(*it) + 1);
		}
	}
	
	if ( _dyldEnvironExrasCount != 0 ) {
		const std::vector<const char*>& extras = _options.dyldEnvironExtras();
		for (std::vector<const char*>::const_iterator it = extras.begin(); it != extras.end(); ++it) {
			sz += alignedSize(sizeof(macho_dylinker_command<P>) + strlen(*it) + 1);
		}
	}

	if ( _hasFunctionStartsLoadCommand )
		sz += sizeof(macho_linkedit_data_command<P>);

	if ( _hasDataInCodeLoadCommand )
		sz += sizeof(macho_linkedit_data_command<P>);

	if ( !_linkerOptions.empty() ) {
		for (ld::relocatable::File::LinkerOptionsList::const_iterator it = _linkerOptions.begin(); it != _linkerOptions.end(); ++it) {
			uint32_t s = sizeof(macho_linker_option_command<P>);
			const std::vector<const char*>& options = *it;
			for (std::vector<const char*>::const_iterator t=options.begin(); t != options.end(); ++t) {
				s += (strlen(*t) + 1);
			}
			sz += alignedSize(s);
		}
	}
	
	if ( _hasOptimizationHints )
		sz += sizeof(macho_linkedit_data_command<P>);

	if ( _hasCodeSignature )
		sz += sizeof(macho_linkedit_data_command<P>);

	return sz;
}

template <typename A>
uint32_t HeaderAndLoadCommandsAtom<A>::commandsCount() const
{
	uint32_t count = this->segmentCount();
	
	if ( _hasDylibIDLoadCommand )
		++count;
		
	if ( _hasDyldInfoLoadCommand )
		++count;
	
	if ( _hasChainedFixupsLoadCommand )
		++count;

	if ( _hasExportsTrieLoadCommand )
		++count;

	if ( _hasSymbolTableLoadCommand )
		++count;
		
	if ( _hasDynamicSymbolTableLoadCommand )
		++count;
	
	if ( _hasDyldLoadCommand )
		++count;
		
	if ( _hasRoutinesLoadCommand ) 
		++count;
		
	if ( _hasUUIDLoadCommand )
		++count;

	if ( _hasVersionLoadCommand ) {
		count += _options.platforms().count();
	}

	if ( _hasSourceVersionLoadCommand )
		++count;
		
	if ( _hasThreadLoadCommand )
		++count;
	
	if ( _hasEntryPointLoadCommand )
		++count;
		
	if ( _hasEncryptionLoadCommand )
		++count;
	
	if ( _hasSplitSegInfoLoadCommand )
		++count;
	
	count += _dylibLoadCommmandsCount;

	count += _options.rpaths().size();
	
	if ( _hasSubFrameworkLoadCommand )
		++count;
	
	count += _subLibraryNames.size();
	
	count += _subUmbrellaNames.size();

	count += _allowableClientLoadCommmandsCount;
	
	count += _dyldEnvironExrasCount;
	
	if ( _hasFunctionStartsLoadCommand )
		++count;

	if ( _hasDataInCodeLoadCommand )
		++count;

	if ( !_linkerOptions.empty() ) {
		for (ld::relocatable::File::LinkerOptionsList::const_iterator it = _linkerOptions.begin(); it != _linkerOptions.end(); ++it) {
			++count;
		}
	}

	if ( _hasOptimizationHints )
		++count;
		
	if ( _hasCodeSignature )
		++count;

	return count;
}

template <typename A>
uint32_t HeaderAndLoadCommandsAtom<A>::fileType() const
{
	switch ( _options.outputKind() ) {
		case Options::kDynamicExecutable:
		case Options::kStaticExecutable:
			return MH_EXECUTE;
		case Options::kDynamicLibrary:
			return MH_DYLIB;
		case Options::kDynamicBundle:
			return MH_BUNDLE;
		case Options::kObjectFile:
			return MH_OBJECT;
		case Options::kDyld:
			return MH_DYLINKER;
		case Options::kPreload:
			return MH_PRELOAD;
		case Options::kKextBundle:
			return MH_KEXT_BUNDLE;
	}
	throw "unknonwn mach-o file type";
}

template <typename A>
uint32_t HeaderAndLoadCommandsAtom<A>::flags() const
{
	uint32_t bits = 0;
	if ( _options.outputKind() == Options::kObjectFile ) {
		if ( _state.allObjectFilesScatterable )
			bits = MH_SUBSECTIONS_VIA_SYMBOLS;
	}
	else {
		if ( _options.outputKind() == Options::kStaticExecutable ) {
			bits |= MH_NOUNDEFS;
			if ( _options.positionIndependentExecutable() ) 
				bits |= MH_PIE;
		}
		else if ( _options.outputKind() == Options::kPreload ) {
			bits |= MH_NOUNDEFS;
			if ( _options.positionIndependentExecutable() ) 
				bits |= MH_PIE;
		}
		else {
			bits = MH_DYLDLINK;
			switch ( _options.nameSpace() ) {
				case Options::kTwoLevelNameSpace:
					bits |= MH_TWOLEVEL | MH_NOUNDEFS;
					break;
				case Options::kFlatNameSpace:
					break;
				case Options::kForceFlatNameSpace:
					bits |= MH_FORCE_FLAT;
					break;
			}
			if ( _state.hasWeakExternalSymbols || _writer.overridesWeakExternalSymbols )
				bits |= MH_WEAK_DEFINES;
			if ( _writer.usesWeakExternalSymbols || _state.hasWeakExternalSymbols )
				bits |= MH_BINDS_TO_WEAK;
			if ( (_options.outputKind() == Options::kDynamicLibrary) 
					&& _writer._noReExportedDylibs 
					&& _options.useSimplifiedDylibReExports() ) {
				bits |= MH_NO_REEXPORTED_DYLIBS;
			}
			if ( _options.positionIndependentExecutable() && ! _writer.pieDisabled ) 
				bits |= MH_PIE;
			if ( _options.markAutoDeadStripDylib() ) 
				bits |= MH_DEAD_STRIPPABLE_DYLIB;
			if ( _state.hasThreadLocalVariableDefinitions )
				bits |= MH_HAS_TLV_DESCRIPTORS;
			if ( _options.hasNonExecutableHeap() )
				bits |= MH_NO_HEAP_EXECUTION;
			if ( _options.markAppExtensionSafe() && (_options.outputKind() == Options::kDynamicLibrary) )
				bits |= MH_APP_EXTENSION_SAFE;
			if (_options.isSimulatorSupportDylib())
				bits |= MH_SIM_SUPPORT;
		}
		if ( _options.hasExecutableStack() )
			bits |= MH_ALLOW_STACK_EXECUTION;
	}
	return bits;
}

template <> uint32_t HeaderAndLoadCommandsAtom<x86>::magic() const		{ return MH_MAGIC; }
template <> uint32_t HeaderAndLoadCommandsAtom<x86_64>::magic() const	{ return MH_MAGIC_64; }
template <> uint32_t HeaderAndLoadCommandsAtom<arm>::magic() const		{ return MH_MAGIC; }
template <> uint32_t HeaderAndLoadCommandsAtom<arm64>::magic() const		{ return MH_MAGIC_64; }
#if SUPPORT_ARCH_arm64_32
template <> uint32_t HeaderAndLoadCommandsAtom<arm64_32>::magic() const		{ return MH_MAGIC; }
#endif

template <> uint32_t HeaderAndLoadCommandsAtom<x86>::cpuType() const	{ return CPU_TYPE_I386; }
template <> uint32_t HeaderAndLoadCommandsAtom<x86_64>::cpuType() const	{ return CPU_TYPE_X86_64; }
template <> uint32_t HeaderAndLoadCommandsAtom<arm>::cpuType() const	{ return CPU_TYPE_ARM; }
template <> uint32_t HeaderAndLoadCommandsAtom<arm64>::cpuType() const	{ return CPU_TYPE_ARM64; }
#if SUPPORT_ARCH_arm64_32
template <> uint32_t HeaderAndLoadCommandsAtom<arm64_32>::cpuType() const	{ return CPU_TYPE_ARM64_32; }
#endif


template <>
uint32_t HeaderAndLoadCommandsAtom<x86>::cpuSubType() const
{
	return CPU_SUBTYPE_I386_ALL;
}

template <>
uint32_t HeaderAndLoadCommandsAtom<x86_64>::cpuSubType() const
{
	if ( (_options.outputKind() == Options::kDynamicExecutable) && (_state.cpuSubType == CPU_SUBTYPE_X86_64_ALL) && _options.platforms().minOS(ld::mac10_5) )
		return (_state.cpuSubType | 0x80000000);
	else
		return _state.cpuSubType;
}

template <>
uint32_t HeaderAndLoadCommandsAtom<arm>::cpuSubType() const
{
	return _state.cpuSubType;
}

template <>
uint32_t HeaderAndLoadCommandsAtom<arm64>::cpuSubType() const
{
	return _state.cpuSubType;
}

#if SUPPORT_ARCH_arm64_32
template <>
uint32_t HeaderAndLoadCommandsAtom<arm64_32>::cpuSubType() const
{
	return CPU_SUBTYPE_ARM64_32_V8;
}
#endif


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copySingleSegmentLoadCommand(uint8_t* p) const
{
	// in .o files there is just one segment load command with a blank name
	// and all sections under it
	macho_segment_command<P>* cmd = (macho_segment_command<P>*)p;
	cmd->set_cmd(macho_segment_command<P>::CMD);
	cmd->set_segname("");
	cmd->set_vmaddr(_options.baseAddress());	
	cmd->set_vmsize(0);		// updated after sections set
	cmd->set_fileoff(0);	// updated after sections set
	cmd->set_filesize(0);	// updated after sections set
	cmd->set_maxprot(VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
	cmd->set_initprot(VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
	cmd->set_nsects(this->nonHiddenSectionCount());
	cmd->set_flags(0);
	// add sections array
	macho_section<P>* msect = (macho_section<P>*)&p[sizeof(macho_segment_command<P>)];
	for (std::vector<ld::Internal::FinalSection*>::iterator sit = _state.sections.begin(); sit != _state.sections.end(); ++sit)  {
		ld::Internal::FinalSection* fsect = *sit;
		if ( fsect->isSectionHidden() ) 
			continue;
		if ( fsect->type() == ld::Section::typeTentativeDefs ) 
			continue;
		msect->set_sectname(fsect->sectionName());
		msect->set_segname(fsect->segmentName());
		msect->set_addr(fsect->address);
		msect->set_size(fsect->size);
		msect->set_offset(fsect->fileOffset);
		msect->set_align(fsect->alignment);
		msect->set_reloff((fsect->relocCount == 0) ? 0 : _writer.sectionRelocationsSection->fileOffset + fsect->relocStart * sizeof(macho_relocation_info<P>));
		msect->set_nreloc(fsect->relocCount);
		msect->set_flags(sectionFlags(fsect));
		msect->set_reserved1(fsect->indirectSymTabStartIndex);	
		msect->set_reserved2(fsect->indirectSymTabElementSize);	
		// update segment info
		if ( cmd->fileoff() == 0 )
			cmd->set_fileoff(fsect->fileOffset);
		cmd->set_vmsize(fsect->address + fsect->size - cmd->vmaddr());
		if ( !sectionTakesNoDiskSpace(fsect) )
			cmd->set_filesize(fsect->fileOffset + fsect->size - cmd->fileoff());
		++msect;
	}
	cmd->set_cmdsize(sizeof(macho_segment_command<P>) + cmd->nsects()*sizeof(macho_section<P>));
	return p + cmd->cmdsize();
}

struct SegInfo {
												SegInfo(const char* n, const Options&);
	const char*									segName;
	uint32_t									nonHiddenSectionCount;
	uint32_t									nonSectCreateSections;
	uint32_t									maxProt;
	uint32_t									initProt;
	uint32_t									flags;
	std::vector<ld::Internal::FinalSection*>	sections;
};


SegInfo::SegInfo(const char* n, const Options& opts) 
	: segName(n), nonHiddenSectionCount(0), nonSectCreateSections(0), maxProt(opts.maxSegProtection(n)), initProt(opts.initialSegProtection(n)), flags(0)
{
	if ( opts.readOnlyDataSegment(n) )
		flags = SG_READ_ONLY;
}


template <typename A>
uint32_t HeaderAndLoadCommandsAtom<A>::sectionFlags(ld::Internal::FinalSection* sect) const
{
	uint32_t bits;
	switch ( sect->type() ) {
		case ld::Section::typeUnclassified:
			if ( strcmp(sect->segmentName(), "__OBJC") == 0 )
				return S_REGULAR | S_ATTR_NO_DEAD_STRIP;
			else if ( (strcmp(sect->sectionName(), "__objc_classlist") == 0) && (strcmp(sect->segmentName(), "__DATA") == 0) )
				return S_REGULAR | S_ATTR_NO_DEAD_STRIP;
			else if ( (strcmp(sect->sectionName(), "__objc_catlist") == 0) && (strcmp(sect->segmentName(), "__DATA") == 0) )
				return S_REGULAR | S_ATTR_NO_DEAD_STRIP;
			else if ( (strncmp(sect->sectionName(), "__objc_superrefs", 16) == 0) && (strcmp(sect->segmentName(), "__DATA") == 0) )
				return S_REGULAR | S_ATTR_NO_DEAD_STRIP;
			else if ( (strncmp(sect->sectionName(), "__objc_nlclslist", 16) == 0) && (strcmp(sect->segmentName(), "__DATA") == 0) )
				return S_REGULAR | S_ATTR_NO_DEAD_STRIP;
			else if ( (strncmp(sect->sectionName(), "__objc_nlcatlist", 16) == 0) && (strcmp(sect->segmentName(), "__DATA") == 0) )
				return S_REGULAR | S_ATTR_NO_DEAD_STRIP;
			else if (  (_options.outputKind() == Options::kObjectFile) && !sect->atoms.empty() && sect->atoms.front()->dontDeadStripIfReferencesLive() )
				return S_REGULAR | S_ATTR_LIVE_SUPPORT;
			else
				return S_REGULAR;
		case ld::Section::typeCode:
			bits = S_REGULAR | S_ATTR_SOME_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS;
			if ( sect->hasLocalRelocs && ! _writer.pieDisabled )
				bits |= S_ATTR_LOC_RELOC;
			if ( sect->hasExternalRelocs )
				bits |= S_ATTR_EXT_RELOC;
			return bits;
		case ld::Section::typePageZero:
			return S_REGULAR;
		case ld::Section::typeImportProxies:
			return S_REGULAR;
		case ld::Section::typeLinkEdit:
			return S_REGULAR;
		case ld::Section::typeMachHeader:
			return S_REGULAR;
		case ld::Section::typeStack:
			return S_REGULAR;
		case ld::Section::typeLiteral4:
			return S_4BYTE_LITERALS;
		case ld::Section::typeLiteral8:
			return S_8BYTE_LITERALS;
		case ld::Section::typeLiteral16:
			return S_16BYTE_LITERALS;
		case ld::Section::typeConstants:
			return S_REGULAR;
		case ld::Section::typeTempLTO:
			assert(0 && "typeTempLTO should not make it to final linked image");
			return S_REGULAR;
		case ld::Section::typeTempAlias:
			assert(0 && "typeAlias should not make it to final linked image");
			return S_REGULAR;
		case ld::Section::typeAbsoluteSymbols:
			assert(0 && "typeAbsoluteSymbols should not make it to final linked image");
			return S_REGULAR;
		case ld::Section::typeCString:
		case ld::Section::typeNonStdCString:
			return S_CSTRING_LITERALS;
		case ld::Section::typeCStringPointer:
			return S_LITERAL_POINTERS | S_ATTR_NO_DEAD_STRIP;
		case ld::Section::typeUTF16Strings:
			return S_REGULAR;
		case ld::Section::typeCFString:
			return S_REGULAR;
		case ld::Section::typeObjC1Classes:
			return S_REGULAR | S_ATTR_NO_DEAD_STRIP;
		case ld::Section::typeCFI:
			return S_REGULAR;
		case ld::Section::typeLSDA:
			return S_REGULAR;
		case ld::Section::typeDtraceDOF:
			return S_DTRACE_DOF;
		case ld::Section::typeUnwindInfo:
			return S_REGULAR;
		case ld::Section::typeThreadStarts:
		case ld::Section::typeChainStarts:
			return S_REGULAR;
		case ld::Section::typeObjCClassRefs:
		case ld::Section::typeObjC2CategoryList:
		case ld::Section::typeObjC2ClassList:
			return S_REGULAR | S_ATTR_NO_DEAD_STRIP;
		case ld::Section::typeZeroFill:
			if ( _options.optimizeZeroFill() )
				return S_ZEROFILL;
			else
				return S_REGULAR;
		case ld::Section::typeTentativeDefs:
			assert(0 && "typeTentativeDefs should not make it to final linked image");
			return S_REGULAR;
		case ld::Section::typeLazyPointer:
		case ld::Section::typeLazyPointerClose:
			return S_LAZY_SYMBOL_POINTERS;
		case ld::Section::typeStubClose:
		case ld::Section::typeStub:
			if ( sect->hasLocalRelocs )
				return S_SYMBOL_STUBS | S_ATTR_SOME_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS | S_ATTR_LOC_RELOC;
			else
				return S_SYMBOL_STUBS | S_ATTR_SOME_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS;
		case ld::Section::typeNonLazyPointer:
			if ( _options.outputKind() == Options::kKextBundle  )
				return S_REGULAR;
			else if ( (_options.outputKind() == Options::kStaticExecutable) && _options.positionIndependentExecutable() )
				return S_REGULAR;
			else
				return S_NON_LAZY_SYMBOL_POINTERS;
		case ld::Section::typeDyldInfo:
			return S_REGULAR;
		case ld::Section::typeLazyDylibPointer:
			return S_LAZY_DYLIB_SYMBOL_POINTERS;
		case ld::Section::typeStubHelper:
			if ( sect->hasLocalRelocs )
				return S_REGULAR | S_ATTR_SOME_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS | S_ATTR_LOC_RELOC;
			else
				return S_REGULAR | S_ATTR_SOME_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS;
		case ld::Section::typeInitializerPointers:
			// <rdar://problem/11456679> i386 kexts need different section type
			if ( (_options.outputKind() == Options::kObjectFile) 
					&& (strcmp(sect->sectionName(), "__constructor") == 0) 
					&& (strcmp(sect->segmentName(), "__TEXT") == 0) )
				return S_REGULAR;
			else
				return S_MOD_INIT_FUNC_POINTERS;
		case ld::Section::typeTerminatorPointers:
			return S_MOD_TERM_FUNC_POINTERS;
		case ld::Section::typeTLVInitialValues:
			return S_THREAD_LOCAL_REGULAR;
		case ld::Section::typeTLVZeroFill:
			return S_THREAD_LOCAL_ZEROFILL;
		case ld::Section::typeTLVDefs:
			return S_THREAD_LOCAL_VARIABLES;
		case ld::Section::typeTLVInitializerPointers:
			return S_THREAD_LOCAL_INIT_FUNCTION_POINTERS;
		case ld::Section::typeTLVPointers:
			return S_THREAD_LOCAL_VARIABLE_POINTERS;
		case ld::Section::typeFirstSection:
			assert(0 && "typeFirstSection should not make it to final linked image");
			return S_REGULAR;
		case ld::Section::typeLastSection:
			assert(0 && "typeLastSection should not make it to final linked image");
			return S_REGULAR;
		case ld::Section::typeDebug:
			return S_REGULAR | S_ATTR_DEBUG;
		case ld::Section::typeSectCreate:
			return S_REGULAR;
		case ld::Section::typeInitOffsets:
			return S_INIT_FUNC_OFFSETS;
	}
	return S_REGULAR;
}


template <typename A>
bool HeaderAndLoadCommandsAtom<A>::sectionTakesNoDiskSpace(ld::Internal::FinalSection* sect) const
{
	switch ( sect->type() ) {
		case ld::Section::typeZeroFill:
		case ld::Section::typeTLVZeroFill:
			return _options.optimizeZeroFill();
		case ld::Section::typeAbsoluteSymbols:
		case ld::Section::typeTentativeDefs:
		case ld::Section::typeLastSection:
			return true;
		default:
			break;
	}
	return false;
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copySegmentLoadCommands(uint8_t* p, uint8_t* base) const
{
	// group sections into segments
	std::vector<SegInfo> segs;
	const char* lastSegName = "";
	for (std::vector<ld::Internal::FinalSection*>::iterator it = _state.sections.begin(); it != _state.sections.end(); ++it) {
		ld::Internal::FinalSection* sect = *it;
		if ( _options.outputKind() == Options::kPreload ) {
			if ( (*it)->type() == ld::Section::typeMachHeader )
				continue; // for -preload, don't put hidden __HEADER segment into output
			if ( (*it)->type() == ld::Section::typeLinkEdit )
				continue; // for -preload, don't put hidden __LINKEDIT segment into output
		}
		if ( strcmp(lastSegName, sect->segmentName()) != 0 ) {
			SegInfo si(sect->segmentName(), _options);
			segs.push_back(si);
			lastSegName = sect->segmentName();
		}
		if ( ! sect->isSectionHidden() ) 
			segs.back().nonHiddenSectionCount++;
		if ( sect->type() != ld::Section::typeSectCreate )
			segs.back().nonSectCreateSections++;

		segs.back().sections.push_back(sect);
	}
	// write out segment load commands for each section with trailing sections
	for (std::vector<SegInfo>::iterator it = segs.begin(); it != segs.end(); ++it) {
		SegInfo& si = *it;
		ld::Internal::FinalSection* lastNonZeroFillSection = NULL;
		for (int i=si.sections.size()-1; i >= 0; --i) {
			if ( !sectionTakesNoDiskSpace(si.sections[i]) ) {
				lastNonZeroFillSection = si.sections[i];
				break;
			}
		}
		uint64_t vmsize = si.sections.back()->address + si.sections.back()->size - si.sections.front()->address;
		vmsize = ((vmsize+_options.segmentAlignment()-1) & (-_options.segmentAlignment()));
		uint64_t filesize = 0;
		if ( lastNonZeroFillSection != NULL ) {
			filesize = lastNonZeroFillSection->address + lastNonZeroFillSection->size - si.sections.front()->address;
			// round up all segments to page aligned, except __LINKEDIT
			if ( (si.sections[0]->type() != ld::Section::typeLinkEdit) && (si.sections[0]->type() != ld::Section::typeImportProxies) )
				filesize = (filesize + _options.segmentAlignment()-1) & (-_options.segmentAlignment());
		}
		if ( si.sections.front()->type() == ld::Section::typePageZero )
			filesize = 0;
		else if ( si.sections.front()->type() == ld::Section::typeStack )
			filesize = 0;
		macho_segment_command<P>* segCmd = (macho_segment_command<P>*)p;
		segCmd->set_cmd(macho_segment_command<P>::CMD);
		segCmd->set_cmdsize(sizeof(macho_segment_command<P>) + si.nonHiddenSectionCount*sizeof(macho_section<P>));
		segCmd->set_segname(si.sections.front()->segmentName());
		segCmd->set_vmaddr(si.sections.front()->address);		
		segCmd->set_vmsize(vmsize);	
		segCmd->set_fileoff(si.sections.front()->fileOffset);
		segCmd->set_filesize(filesize); 
		segCmd->set_maxprot(si.maxProt);
		segCmd->set_initprot(si.initProt);
		segCmd->set_nsects(si.nonHiddenSectionCount);
		segCmd->set_flags(si.flags | (si.nonSectCreateSections ? 0 : SG_NORELOC)); // FIXME, really should check all References
		if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 )
			_linkeditCmdOffset = p - base;
		p += sizeof(macho_segment_command<P>);
		macho_section<P>* msect = (macho_section<P>*)p;
		for (std::vector<ld::Internal::FinalSection*>::iterator sit = si.sections.begin(); sit != si.sections.end(); ++sit) {
			ld::Internal::FinalSection* fsect = *sit;
			if ( ! fsect->isSectionHidden() ) {
				msect->set_sectname(fsect->sectionName());
				msect->set_segname(fsect->segmentName());
				msect->set_addr(fsect->address);
				msect->set_size(fsect->size);
				msect->set_offset(sectionTakesNoDiskSpace(fsect) ? 0 : fsect->fileOffset);
				msect->set_align(fsect->alignment);
				msect->set_reloff(0);		
				msect->set_nreloc(0);
				msect->set_flags(sectionFlags(fsect));
				msect->set_reserved1(fsect->indirectSymTabStartIndex);	
				msect->set_reserved2(fsect->indirectSymTabElementSize);	
				p += sizeof(macho_section<P>);
				++msect;
			}
		}
	}

	return p;
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copySymbolTableLoadCommand(uint8_t* p, uint8_t* base) const
{
	_symboltableCmdOffset = p - base;
	// build LC_SYMTAB command
	macho_symtab_command<P>*   symbolTableCmd = (macho_symtab_command<P>*)p;
	symbolTableCmd->set_cmd(LC_SYMTAB);
	symbolTableCmd->set_cmdsize(sizeof(macho_symtab_command<P>));
	symbolTableCmd->set_nsyms(_writer.symbolTableSection->size/sizeof(macho_nlist<P>));
	symbolTableCmd->set_symoff(_writer.symbolTableSection->size == 0 ? 0 : _writer.symbolTableSection->fileOffset);
	symbolTableCmd->set_stroff(_writer.stringPoolSection->size == 0 ? 0 : _writer.stringPoolSection->fileOffset );
	symbolTableCmd->set_strsize(_writer.stringPoolSection->size);
	return p + sizeof(macho_symtab_command<P>);
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyDynamicSymbolTableLoadCommand(uint8_t* p) const
{
	// build LC_SYMTAB command
	macho_dysymtab_command<P>*   dynamicSymbolTableCmd = (macho_dysymtab_command<P>*)p;
	dynamicSymbolTableCmd->set_cmd(LC_DYSYMTAB);
	dynamicSymbolTableCmd->set_cmdsize(sizeof(macho_dysymtab_command<P>));
	dynamicSymbolTableCmd->set_ilocalsym(0);
	dynamicSymbolTableCmd->set_nlocalsym(_writer._localSymbolsCount);
	dynamicSymbolTableCmd->set_iextdefsym(dynamicSymbolTableCmd->ilocalsym()+dynamicSymbolTableCmd->nlocalsym());
	dynamicSymbolTableCmd->set_nextdefsym(_writer._globalSymbolsCount);
	dynamicSymbolTableCmd->set_iundefsym(dynamicSymbolTableCmd->iextdefsym()+dynamicSymbolTableCmd->nextdefsym());
	dynamicSymbolTableCmd->set_nundefsym(_writer._importSymbolsCount);

	// FIX ME: support for 10.3 dylibs which need modules
	//if ( fWriter.fModuleInfoAtom != NULL ) {
	//	dynamicSymbolTableCmd->set_tocoff(fWriter.fModuleInfoAtom->getTableOfContentsFileOffset());
	//	dynamicSymbolTableCmd->set_ntoc(fWriter.fSymbolTableExportCount);
	//	dynamicSymbolTableCmd->set_modtaboff(fWriter.fModuleInfoAtom->getModuleTableFileOffset());
	//	dynamicSymbolTableCmd->set_nmodtab(1);
	//	dynamicSymbolTableCmd->set_extrefsymoff(fWriter.fModuleInfoAtom->getReferencesFileOffset());
	//	dynamicSymbolTableCmd->set_nextrefsyms(fWriter.fModuleInfoAtom->getReferencesCount());
	//}

	bool hasIndirectSymbols = ( (_writer.indirectSymbolTableSection != NULL) && (_writer.indirectSymbolTableSection->size != 0) );
	dynamicSymbolTableCmd->set_indirectsymoff(hasIndirectSymbols ? _writer.indirectSymbolTableSection->fileOffset : 0);
	dynamicSymbolTableCmd->set_nindirectsyms( hasIndirectSymbols ? _writer.indirectSymbolTableSection->size/sizeof(uint32_t) : 0);

	// FIX ME: support for classic relocations
	if ( _options.outputKind() != Options::kObjectFile ) {
		bool hasExternalRelocs = ( (_writer.externalRelocationsSection != NULL) && (_writer.externalRelocationsSection->size != 0) );
		dynamicSymbolTableCmd->set_extreloff(hasExternalRelocs ? _writer.externalRelocationsSection->fileOffset : 0);
		dynamicSymbolTableCmd->set_nextrel(  hasExternalRelocs ? _writer.externalRelocationsSection->size/8 : 0);
		bool hasLocalRelocs = ( (_writer.localRelocationsSection != NULL) && (_writer.localRelocationsSection->size != 0) );
		dynamicSymbolTableCmd->set_locreloff(hasLocalRelocs ? _writer.localRelocationsSection->fileOffset : 0);
		dynamicSymbolTableCmd->set_nlocrel  (hasLocalRelocs ? _writer.localRelocationsSection->size/8 : 0);
	}
	return p + sizeof(macho_dysymtab_command<P>);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyDyldInfoLoadCommand(uint8_t* p) const
{
	// build LC_DYLD_INFO command
	macho_dyld_info_command<P>*  cmd = (macho_dyld_info_command<P>*)p;
	
	cmd->set_cmd(LC_DYLD_INFO_ONLY);
	cmd->set_cmdsize(sizeof(macho_dyld_info_command<P>));
	if ( _writer.rebaseSection->size != 0 ) {
		cmd->set_rebase_off(_writer.rebaseSection->fileOffset);
		cmd->set_rebase_size(_writer.rebaseSection->size);
	}
	if ( _writer.bindingSection->size != 0 ) {
		cmd->set_bind_off(_writer.bindingSection->fileOffset);
		cmd->set_bind_size(_writer.bindingSection->size);
	}
	if ( _writer.weakBindingSection->size != 0 ) {
		cmd->set_weak_bind_off(_writer.weakBindingSection->fileOffset);
		cmd->set_weak_bind_size(_writer.weakBindingSection->size);
	}
	if ( _writer.lazyBindingSection->size != 0 ) {
		cmd->set_lazy_bind_off(_writer.lazyBindingSection->fileOffset);
		cmd->set_lazy_bind_size(_writer.lazyBindingSection->size);
	}
	if ( _writer.exportSection->size != 0 ) {
		cmd->set_export_off(_writer.exportSection->fileOffset);
		cmd->set_export_size(_writer.exportSection->size);
	}
	return p + sizeof(macho_dyld_info_command<P>);
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyExportsTrieLoadCommand(uint8_t* p) const
{
	// build LC_DYLD_EXPORTS_TRIE command
	linkedit_data_command*  cmd = (linkedit_data_command*)p;

	cmd->cmd 		= LC_DYLD_EXPORTS_TRIE;
	cmd->cmdsize	= sizeof(linkedit_data_command);
	cmd->dataoff 	= _writer.exportSection->fileOffset;
	cmd->datasize   = _writer.exportSection->size;

	return p + sizeof(linkedit_data_command);
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyChainedFixupsLoadCommand(uint8_t* p) const
{
	// build LC_DYLD_CHAINED_FIXUPS command
	linkedit_data_command*  cmd = (linkedit_data_command*)p;

	cmd->cmd 		= LC_DYLD_CHAINED_FIXUPS;
	cmd->cmdsize	= sizeof(linkedit_data_command);
	cmd->dataoff 	= _writer.chainInfoSection->fileOffset;
	cmd->datasize   = _writer.chainInfoSection->size;

	return p + sizeof(linkedit_data_command);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyDyldLoadCommand(uint8_t* p) const
{
	uint32_t sz = alignedSize(sizeof(macho_dylinker_command<P>) + strlen(_options.dyldInstallPath()) + 1);
	macho_dylinker_command<P>* cmd = (macho_dylinker_command<P>*)p;
	if ( _options.outputKind() == Options::kDyld )
		cmd->set_cmd(LC_ID_DYLINKER);
	else
		cmd->set_cmd(LC_LOAD_DYLINKER);
	cmd->set_cmdsize(sz);
	cmd->set_name_offset();
	strcpy((char*)&p[sizeof(macho_dylinker_command<P>)], _options.dyldInstallPath());
	return p + sz;
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyDylibIDLoadCommand(uint8_t* p) const
{
	uint32_t sz = alignedSize(sizeof(macho_dylib_command<P>) + strlen(_options.installPath()) + 1);
	macho_dylib_command<P>* cmd = (macho_dylib_command<P>*)p;
	cmd->set_cmd(LC_ID_DYLIB);
	cmd->set_cmdsize(sz);
	cmd->set_name_offset();
	cmd->set_timestamp(1);	// needs to be some constant value that is different than DylibLoadCommandsAtom uses
	cmd->set_current_version(_options.currentVersion32());
	cmd->set_compatibility_version(_options.compatibilityVersion());
	strcpy((char*)&p[sizeof(macho_dylib_command<P>)], _options.installPath());
	return p + sz;
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyRoutinesLoadCommand(uint8_t* p) const
{
	pint_t initAddr = _state.entryPoint->finalAddress(); 
	if ( _state.entryPoint->isThumb() )
		initAddr |= 1ULL;
	macho_routines_command<P>* cmd = (macho_routines_command<P>*)p;
	cmd->set_cmd(macho_routines_command<P>::CMD);
	cmd->set_cmdsize(sizeof(macho_routines_command<P>));
	cmd->set_init_address(initAddr);
	return p + sizeof(macho_routines_command<P>);
}


template <typename A>
void HeaderAndLoadCommandsAtom<A>::recopyUUIDCommand() 
{
	assert(_uuidCmdInOutputBuffer != NULL);
	_uuidCmdInOutputBuffer->set_uuid(_uuid);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyUUIDLoadCommand(uint8_t* p) const
{
	macho_uuid_command<P>* cmd = (macho_uuid_command<P>*)p;
	cmd->set_cmd(LC_UUID);
	cmd->set_cmdsize(sizeof(macho_uuid_command<P>));
	cmd->set_uuid(_uuid);
	_uuidCmdInOutputBuffer = cmd;	 // save for later re-write by recopyUUIDCommand()
	return p + sizeof(macho_uuid_command<P>);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyVersionLoadCommand(uint8_t* p, ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion) const
{
	macho_version_min_command<P>* cmd = (macho_version_min_command<P>*)p;
	const PlatformInfo& info = platformInfo(platform);
	assert(info.loadCommandIfNotUsingBuildVersionLC != 0 && "platform requires LC_BUILD_VERSION");
	cmd->set_cmd(info.loadCommandIfNotUsingBuildVersionLC);
	cmd->set_cmdsize(sizeof(macho_version_min_command<P>));
	cmd->set_version(minVersion);
	cmd->set_sdk(sdkVersion);
	return p + sizeof(macho_version_min_command<P>);
}



template <typename A>
	uint8_t* HeaderAndLoadCommandsAtom<A>::copyBuildVersionLoadCommand(uint8_t* p, ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion) const
{
	macho_build_version_command<P>* cmd = (macho_build_version_command<P>*)p;

	// If we are on iOSMac and see a pre-13.0 SDK version it means we are using an old clang driver and should set the version
	// to 13.0
	if (platform == ld::Platform::iOSMac && sdkVersion < 0x000D0000) {
		sdkVersion = 0x000D0000;
	}
	cmd->set_cmd(LC_BUILD_VERSION);
	cmd->set_cmdsize(alignedSize(sizeof(macho_build_version_command<P>) + sizeof(macho_build_tool_version<P>)*_toolsVersions.size()));
	cmd->set_platform((uint32_t)platform);
	cmd->set_minos(minVersion);
	cmd->set_sdk(sdkVersion);
	cmd->set_ntools(_toolsVersions.size());
	macho_build_tool_version<P>* tools = (macho_build_tool_version<P>*)(p + sizeof(macho_build_version_command<P>));
	for (uint64_t tool : _toolsVersions) {
		tools->set_tool((uint64_t)tool >> 32);
		tools->set_version(tool & 0xFFFFFFFF);
		++tools;
	}

	return p + cmd->cmdsize();
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copySourceVersionLoadCommand(uint8_t* p) const
{
	macho_source_version_command<P>* cmd = (macho_source_version_command<P>*)p;
	cmd->set_cmd(LC_SOURCE_VERSION);
	cmd->set_cmdsize(sizeof(macho_source_version_command<P>));
	cmd->set_version(_options.sourceVersion());
	return p + sizeof(macho_source_version_command<P>);
}


template <>
uint32_t HeaderAndLoadCommandsAtom<x86>::threadLoadCommandSize() const
{
	return this->alignedSize(16 + 16*4);	// base size + i386_THREAD_STATE_COUNT * 4
}

template <>
uint8_t* HeaderAndLoadCommandsAtom<x86>::copyThreadsLoadCommand(uint8_t* p) const
{
	assert(_state.entryPoint != NULL);
	pint_t start = _state.entryPoint->finalAddress(); 
	macho_thread_command<P>* cmd = (macho_thread_command<P>*)p;
	cmd->set_cmd(LC_UNIXTHREAD);
	cmd->set_cmdsize(threadLoadCommandSize());
	cmd->set_flavor(1);				// i386_THREAD_STATE
	cmd->set_count(16);				// i386_THREAD_STATE_COUNT;
	cmd->set_thread_register(10, start);
	if ( _options.hasCustomStack() )
		cmd->set_thread_register(7, _options.customStackAddr());	// r1
	return p + threadLoadCommandSize();
}

template <>
uint32_t HeaderAndLoadCommandsAtom<x86_64>::threadLoadCommandSize() const
{
	return this->alignedSize(16 + 42*4);	// base size + x86_THREAD_STATE64_COUNT * 4
}

template <>
uint8_t* HeaderAndLoadCommandsAtom<x86_64>::copyThreadsLoadCommand(uint8_t* p) const
{
	assert(_state.entryPoint != NULL);
	pint_t start = _state.entryPoint->finalAddress(); 
	macho_thread_command<P>* cmd = (macho_thread_command<P>*)p;
	cmd->set_cmd(LC_UNIXTHREAD);
	cmd->set_cmdsize(threadLoadCommandSize());
	cmd->set_flavor(4);				// x86_THREAD_STATE64
	cmd->set_count(42);				// x86_THREAD_STATE64_COUNT
	cmd->set_thread_register(16, start);		// rip 
	if ( _options.hasCustomStack() )
		cmd->set_thread_register(7, _options.customStackAddr());	// r1
	return p + threadLoadCommandSize();
}

template <>
uint32_t HeaderAndLoadCommandsAtom<arm>::threadLoadCommandSize() const
{
	return this->alignedSize(16 + 17 * 4); // base size + ARM_THREAD_STATE_COUNT * 4
}

template <>
uint8_t* HeaderAndLoadCommandsAtom<arm>::copyThreadsLoadCommand(uint8_t* p) const
{
	assert(_state.entryPoint != NULL);
	pint_t start = _state.entryPoint->finalAddress(); 
	if ( _state.entryPoint->isThumb() )
		start |= 1ULL;
	macho_thread_command<P>* cmd = (macho_thread_command<P>*)p;
	cmd->set_cmd(LC_UNIXTHREAD);
	cmd->set_cmdsize(threadLoadCommandSize());
	cmd->set_flavor(1);			
	cmd->set_count(17);	
	cmd->set_thread_register(15, start);		// pc
	if ( _options.hasCustomStack() )
		cmd->set_thread_register(13, _options.customStackAddr());	// sp
	return p + threadLoadCommandSize();
}


template <>
uint32_t HeaderAndLoadCommandsAtom<arm64>::threadLoadCommandSize() const
{
	return this->alignedSize(16 + 34 * 8); // base size + ARM_EXCEPTION_STATE64_COUNT * 4
}

template <>
uint8_t* HeaderAndLoadCommandsAtom<arm64>::copyThreadsLoadCommand(uint8_t* p) const
{
	assert(_state.entryPoint != NULL);
	pint_t start = _state.entryPoint->finalAddress(); 
	macho_thread_command<P>* cmd = (macho_thread_command<P>*)p;
	cmd->set_cmd(LC_UNIXTHREAD);
	cmd->set_cmdsize(threadLoadCommandSize());
	cmd->set_flavor(6);	 // ARM_THREAD_STATE64
	cmd->set_count(68);	 // ARM_EXCEPTION_STATE64_COUNT
	cmd->set_thread_register(32, start);		// pc 
	if ( _options.hasCustomStack() )
		cmd->set_thread_register(31, _options.customStackAddr());	// sp 
	return p + threadLoadCommandSize();
}

#if SUPPORT_ARCH_arm64_32
template <>
uint32_t HeaderAndLoadCommandsAtom<arm64_32>::threadLoadCommandSize() const
{
	return this->alignedSize(16 + 34 * 8); // base size + ARM_EXCEPTION_STATE64_COUNT * 4
}

template <>
uint8_t* HeaderAndLoadCommandsAtom<arm64_32>::copyThreadsLoadCommand(uint8_t* p) const
{
	assert(_state.entryPoint != NULL);
	pint_t start = _state.entryPoint->finalAddress(); 
	macho_thread_command<P>* cmd = (macho_thread_command<P>*)p;
	cmd->set_cmd(LC_UNIXTHREAD);
	cmd->set_cmdsize(threadLoadCommandSize());
	cmd->set_flavor(6);	 // ARM_THREAD_STATE64
	cmd->set_count(68);	 // ARM_EXCEPTION_STATE64_COUNT
	cmd->set_thread_register(64, start);		// pc
	if ( _options.hasCustomStack() )
		cmd->set_thread_register(62, _options.customStackAddr());	// sp
	return p + threadLoadCommandSize();
}
#endif

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyEntryPointLoadCommand(uint8_t* p) const
{
	macho_entry_point_command<P>* cmd = (macho_entry_point_command<P>*)p;
	cmd->set_cmd(LC_MAIN);
	cmd->set_cmdsize(sizeof(macho_entry_point_command<P>));
	assert(_state.entryPoint != NULL);
	pint_t start = _state.entryPoint->finalAddress(); 
	if ( _state.entryPoint->isThumb() )
		start |= 1ULL;
	cmd->set_entryoff(start - this->finalAddress());
	cmd->set_stacksize(_options.hasCustomStack() ? _options.customStackSize() : 0 );
	return p + sizeof(macho_entry_point_command<P>);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyEncryptionLoadCommand(uint8_t* p) const
{
	macho_encryption_info_command<P>* cmd = (macho_encryption_info_command<P>*)p;
	cmd->set_cmd(sizeof(typename A::P::uint_t) == 4 ? LC_ENCRYPTION_INFO : LC_ENCRYPTION_INFO_64);
	cmd->set_cmdsize(sizeof(macho_encryption_info_command<P>));
	assert(_writer.encryptedTextStartOffset() != 0);
	assert(_writer.encryptedTextEndOffset() != 0);
	cmd->set_cryptoff(_writer.encryptedTextStartOffset());
	cmd->set_cryptsize(_writer.encryptedTextEndOffset()-_writer.encryptedTextStartOffset());
	cmd->set_cryptid(0);
	return p + sizeof(macho_encryption_info_command<P>);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copySplitSegInfoLoadCommand(uint8_t* p) const
{
	macho_linkedit_data_command<P>* cmd = (macho_linkedit_data_command<P>*)p;
	cmd->set_cmd(LC_SEGMENT_SPLIT_INFO);
	cmd->set_cmdsize(sizeof(macho_linkedit_data_command<P>));
	cmd->set_dataoff(_writer.splitSegInfoSection->fileOffset);
	cmd->set_datasize(_writer.splitSegInfoSection->size);
	return p + sizeof(macho_linkedit_data_command<P>);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyDylibLoadCommand(uint8_t* p, const ld::dylib::File* dylib) const
{
	uint32_t sz = alignedSize(sizeof(macho_dylib_command<P>) + strlen(dylib->installPath()) + 1);
	macho_dylib_command<P>* cmd = (macho_dylib_command<P>*)p;
	bool weakLink = dylib->forcedWeakLinked() || dylib->allSymbolsAreWeakImported();
	bool upward = dylib->willBeUpwardDylib() && _options.useUpwardDylibs();
	bool reExport = dylib->willBeReExported() && _options.useSimplifiedDylibReExports();
	if ( weakLink && upward )
		warning("cannot weak upward link.  Dropping weak for %s", dylib->installPath());
	if ( weakLink && reExport )
		warning("cannot weak re-export a dylib.  Dropping weak for %s", dylib->installPath());
	if ( reExport )
		cmd->set_cmd(LC_REEXPORT_DYLIB);
	else if ( upward )
		cmd->set_cmd(LC_LOAD_UPWARD_DYLIB);
	else if ( weakLink )
		cmd->set_cmd(LC_LOAD_WEAK_DYLIB);
	else
		cmd->set_cmd(LC_LOAD_DYLIB);
	cmd->set_cmdsize(sz);
	cmd->set_timestamp(2);	// needs to be some constant value that is different than DylibIDLoadCommandsAtom uses
	cmd->set_current_version(dylib->currentVersion());
	cmd->set_compatibility_version(dylib->compatibilityVersion());
	cmd->set_name_offset();
	strcpy((char*)&p[sizeof(macho_dylib_command<P>)], dylib->installPath());
	return p + sz;
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyRPathLoadCommand(uint8_t* p, const char* path) const
{
	uint32_t sz = alignedSize(sizeof(macho_rpath_command<P>) + strlen(path) + 1);
	macho_rpath_command<P>* cmd = (macho_rpath_command<P>*)p;
	cmd->set_cmd(LC_RPATH);
	cmd->set_cmdsize(sz);
	cmd->set_path_offset();
	strcpy((char*)&p[sizeof(macho_rpath_command<P>)], path);
	return p + sz;
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copySubFrameworkLoadCommand(uint8_t* p) const
{
	const char* umbrellaName = _options.umbrellaName();
	uint32_t sz = alignedSize(sizeof(macho_sub_framework_command<P>) + strlen(umbrellaName) + 1);
	macho_sub_framework_command<P>* cmd = (macho_sub_framework_command<P>*)p;
	cmd->set_cmd(LC_SUB_FRAMEWORK);
	cmd->set_cmdsize(sz);
	cmd->set_umbrella_offset();
	strcpy((char*)&p[sizeof(macho_sub_framework_command<P>)], umbrellaName);
	return p + sz;
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyAllowableClientLoadCommand(uint8_t* p, const char* client) const
{
	uint32_t sz = alignedSize(sizeof(macho_sub_client_command<P>) + strlen(client) + 1);
	macho_sub_client_command<P>* cmd = (macho_sub_client_command<P>*)p;
	cmd->set_cmd(LC_SUB_CLIENT);
	cmd->set_cmdsize(sz);
	cmd->set_client_offset();
	strcpy((char*)&p[sizeof(macho_sub_client_command<P>)], client);
	return p + sz;
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyDyldEnvLoadCommand(uint8_t* p, const char* env) const
{
	uint32_t sz = alignedSize(sizeof(macho_dylinker_command<P>) + strlen(env) + 1);
	macho_dylinker_command<P>* cmd = (macho_dylinker_command<P>*)p;
	cmd->set_cmd(LC_DYLD_ENVIRONMENT);
	cmd->set_cmdsize(sz);
	cmd->set_name_offset();
	strcpy((char*)&p[sizeof(macho_dylinker_command<P>)], env);
	return p + sz;
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copySubUmbrellaLoadCommand(uint8_t* p, const char* nm) const
{
	uint32_t sz = alignedSize(sizeof(macho_sub_umbrella_command<P>) + strlen(nm) + 1);
	macho_sub_umbrella_command<P>* cmd = (macho_sub_umbrella_command<P>*)p;
	cmd->set_cmd(LC_SUB_UMBRELLA);
	cmd->set_cmdsize(sz);
	cmd->set_sub_umbrella_offset();
	strcpy((char*)&p[sizeof(macho_sub_umbrella_command<P>)], nm);
	return p + sz;
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copySubLibraryLoadCommand(uint8_t* p, const char* nm) const
{
	uint32_t sz = alignedSize(sizeof(macho_sub_library_command<P>) + strlen(nm) + 1);
	macho_sub_library_command<P>* cmd = (macho_sub_library_command<P>*)p;
	cmd->set_cmd(LC_SUB_LIBRARY);
	cmd->set_cmdsize(sz);
	cmd->set_sub_library_offset();
	strcpy((char*)&p[sizeof(macho_sub_library_command<P>)], nm);
	return p + sz;
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyFunctionStartsLoadCommand(uint8_t* p) const
{
	macho_linkedit_data_command<P>* cmd = (macho_linkedit_data_command<P>*)p;
	cmd->set_cmd(LC_FUNCTION_STARTS);
	cmd->set_cmdsize(sizeof(macho_linkedit_data_command<P>));
	cmd->set_dataoff(_writer.functionStartsSection->fileOffset);
	cmd->set_datasize(_writer.functionStartsSection->size);
	return p + sizeof(macho_linkedit_data_command<P>);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyDataInCodeLoadCommand(uint8_t* p) const
{
	macho_linkedit_data_command<P>* cmd = (macho_linkedit_data_command<P>*)p;
	cmd->set_cmd(LC_DATA_IN_CODE);
	cmd->set_cmdsize(sizeof(macho_linkedit_data_command<P>));
	cmd->set_dataoff(_writer.dataInCodeSection->fileOffset);
	cmd->set_datasize(_writer.dataInCodeSection->size);
	return p + sizeof(macho_linkedit_data_command<P>);
}


template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyLinkerOptionsLoadCommand(uint8_t* p, const std::vector<const char*>& options) const
{
	macho_linker_option_command<P>* cmd = (macho_linker_option_command<P>*)p;
	cmd->set_cmd(LC_LINKER_OPTION);
	cmd->set_count(options.size());
	char* buffer = cmd->buffer();
	uint32_t sz = sizeof(macho_linker_option_command<P>);
	for (std::vector<const char*>::const_iterator it=options.begin(); it != options.end(); ++it) {
		const char* opt = *it;
		uint32_t len = strlen(opt);
		strcpy(buffer, opt);
		sz += (len + 1);
		buffer += (len + 1);
	}
	sz = alignedSize(sz);
	cmd->set_cmdsize(sz);	
	return p + sz;
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyOptimizationHintsLoadCommand(uint8_t* p) const
{
	macho_linkedit_data_command<P>* cmd = (macho_linkedit_data_command<P>*)p;
	cmd->set_cmd(LC_LINKER_OPTIMIZATION_HINTS);
	cmd->set_cmdsize(sizeof(macho_linkedit_data_command<P>));
	cmd->set_dataoff(_writer.optimizationHintsSection->fileOffset);
	cmd->set_datasize(_writer.optimizationHintsSection->size);
	return p + sizeof(macho_linkedit_data_command<P>);
}

template <typename A>
uint8_t* HeaderAndLoadCommandsAtom<A>::copyCodeSignatureLoadCommand(uint8_t* p) const
{
	macho_linkedit_data_command<P>* cmd = (macho_linkedit_data_command<P>*)p;
	cmd->set_cmd(LC_CODE_SIGNATURE);
	cmd->set_cmdsize(sizeof(macho_linkedit_data_command<P>));
	cmd->set_dataoff(_writer.codeSignatureSection->fileOffset);
	cmd->set_datasize(_writer.codeSignatureSection->size);
	return p + sizeof(macho_linkedit_data_command<P>);
}

template <typename A>
void HeaderAndLoadCommandsAtom<A>::copyRawContent(uint8_t buffer[]) const
{
	macho_header<P>* mh = (macho_header<P>*)buffer;
	bzero(buffer, this->size());

	// copy mach_header
	mh->set_magic(this->magic());
	mh->set_cputype(this->cpuType());
	mh->set_cpusubtype(this->cpuSubType());
	if (_state.hasArm64eABIVersion) {
		if ( _options.dyldLoadsOutput() && !_options.platforms().minOS(ld::version2020Fall) ) {
			// when targeting older OSs (e.g. iOS 13), don't set ABI bits
		}
		else {
			mh->set_cpusubtypeflags(_state.arm64eABIVersion);
		}
	}
	mh->set_filetype(this->fileType());
	mh->set_ncmds(this->commandsCount());
	mh->set_sizeofcmds(this->size()-sizeof(macho_header<P>));
	mh->set_flags(this->flags());

	// copy load commands
	__block uint8_t* p = &buffer[sizeof(macho_header<P>)];
	
	if ( _options.outputKind() == Options::kObjectFile )
		p = this->copySingleSegmentLoadCommand(p);
	else
		p = this->copySegmentLoadCommands(p, buffer);
	
	if ( _hasDylibIDLoadCommand )
		p = this->copyDylibIDLoadCommand(p);
		
	if ( _hasDyldInfoLoadCommand )
		p = this->copyDyldInfoLoadCommand(p);
		
	if ( _hasChainedFixupsLoadCommand )
		p = this->copyChainedFixupsLoadCommand(p);

	if ( _hasExportsTrieLoadCommand )
		p = this->copyExportsTrieLoadCommand(p);

	if ( _hasSymbolTableLoadCommand )
		p = this->copySymbolTableLoadCommand(p, buffer);

	if ( _hasDynamicSymbolTableLoadCommand )
		p = this->copyDynamicSymbolTableLoadCommand(p);
	
	if ( _hasDyldLoadCommand )
		p = this->copyDyldLoadCommand(p);
		
	if ( _hasRoutinesLoadCommand ) 
		p = this->copyRoutinesLoadCommand(p);
		
	if ( _hasUUIDLoadCommand )
		p = this->copyUUIDLoadCommand(p);

	if ( _hasVersionLoadCommand ) {
		_options.platforms().forEach(^(ld::Platform platform, uint32_t minVersion, uint32_t sdkVersion, bool &stop) {
			if (_options.shouldUseBuildVersion(platform, minVersion)) {
				p = this->copyBuildVersionLoadCommand(p, platform, minVersion, sdkVersion);
			} else {
				p = this->copyVersionLoadCommand(p, platform, minVersion, sdkVersion);
			}
		});
	}

	if ( _hasSourceVersionLoadCommand )
		p = this->copySourceVersionLoadCommand(p);

	if ( _hasThreadLoadCommand )
		p = this->copyThreadsLoadCommand(p);
	
	if ( _hasEntryPointLoadCommand )
		p = this->copyEntryPointLoadCommand(p);
		
	if ( _hasEncryptionLoadCommand )
		p = this->copyEncryptionLoadCommand(p);
	
	if ( _hasSplitSegInfoLoadCommand )
		p = this->copySplitSegInfoLoadCommand(p);
		
	for (uint32_t ord=1; ord <= _writer.dylibCount(); ++ord) {
		p = this->copyDylibLoadCommand(p, _writer.dylibByOrdinal(ord));
	}

	if ( _hasRPathLoadCommands ) {
		const std::vector<const char*>& rpaths = _options.rpaths();
		for (std::vector<const char*>::const_iterator it = rpaths.begin(); it != rpaths.end(); ++it) {
			p = this->copyRPathLoadCommand(p, *it);
		}
	}
	
	if ( _hasSubFrameworkLoadCommand )
		p = this->copySubFrameworkLoadCommand(p);
	
	for (std::vector<const char*>::const_iterator it = _subLibraryNames.begin(); it != _subLibraryNames.end(); ++it) {
		p = this->copySubLibraryLoadCommand(p, *it);
	}
	
	for (std::vector<const char*>::const_iterator it = _subUmbrellaNames.begin(); it != _subUmbrellaNames.end(); ++it) {
		p = this->copySubUmbrellaLoadCommand(p, *it);
	}
	
	if ( _allowableClientLoadCommmandsCount != 0 ) {
		const std::vector<const char*>& clients = _options.allowableClients();
		for (std::vector<const char*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
			p = this->copyAllowableClientLoadCommand(p, *it);
		}
	}

	if ( _dyldEnvironExrasCount != 0 ) {
		const std::vector<const char*>& extras = _options.dyldEnvironExtras();
		for (std::vector<const char*>::const_iterator it = extras.begin(); it != extras.end(); ++it) {
			p = this->copyDyldEnvLoadCommand(p, *it);
		}
	}

	if ( _hasFunctionStartsLoadCommand )
		p = this->copyFunctionStartsLoadCommand(p);

	if ( _hasDataInCodeLoadCommand )
		p = this->copyDataInCodeLoadCommand(p);

	if ( !_linkerOptions.empty() ) {
		for (ld::relocatable::File::LinkerOptionsList::const_iterator it = _linkerOptions.begin(); it != _linkerOptions.end(); ++it) {
			p = this->copyLinkerOptionsLoadCommand(p, *it);
		}
	}
	
	if ( _hasOptimizationHints )
		p = this->copyOptimizationHintsLoadCommand(p);
 
	if ( _hasCodeSignature )
		p = this->copyCodeSignatureLoadCommand(p);
}



} // namespace tool 
} // namespace ld 

#endif // __HEADER_LOAD_COMMANDS_HPP__
