/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2006-2010 Apple Inc. All rights reserved.
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

#ifndef __LTO_READER_H__
#define __LTO_READER_H__

#ifdef __linux__
#include <algorithm>
#include <Availability.h>
#define __OSX_AVAILABLE_STARTING(...)
namespace {
	typedef long double max_align_t;
}
#endif

#include <stdlib.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <atomic>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include "MachOFileAbstraction.hpp"
#include "Architectures.hpp"
#include "ld.hpp"
#include "macho_relocatable_file.h"
#include "lto_file.h"

#include "llvm-c/lto.h"


namespace lto {

//
// ld64 only tracks non-internal symbols from an llvm bitcode file.  
// We model this by having an InternalAtom which represent all internal functions and data.
// All non-interal symbols from a bitcode file are represented by an Atom
// and each Atom has a reference to the InternalAtom.  The InternalAtom
// also has references to each symbol external to the bitcode file. 
//
class InternalAtom : public ld::Atom
{
public:
												InternalAtom(class File& f);
	// overrides of ld::Atom
	ld::File*							file() const override		{ return &_file; }
	const char*							name() const override		{ return "import-atom"; }
	uint64_t							size() const override		{ return 0; }
	uint64_t							objectAddress() const override { return 0; }
	void								copyRawContent(uint8_t buffer[]) const override { }
	ld::Fixup::iterator					fixupsBegin() const override	{ return &_undefs[0]; }
	ld::Fixup::iterator					fixupsEnd()	const override 	{ return &_undefs[_undefs.size()]; }

	// for adding references to symbols outside bitcode file
	void										addReference(const char* nm)
																	{ _undefs.push_back(ld::Fixup(0, ld::Fixup::k1of1, 
																				ld::Fixup::kindNone, false, strdup(nm))); }
private:

	ld::File&									_file;
	mutable std::vector<ld::Fixup>				_undefs;
};


//
// LLVM bitcode file 
//
class File : public ld::relocatable::File
{
public:
											File(const char* path, time_t mTime, ld::File::Ordinal ordinal, 
													 const uint8_t* content, uint32_t contentLength, cpu_type_t arch);
											~File() override;

	// overrides of ld::File
	bool										forEachAtom(ld::File::AtomHandler&) const override;
	bool										justInTimeforEachAtom(const char* name, ld::File::AtomHandler&) const override
																					{ return false; }
	uint32_t									cpuSubType() const override			{ return _cpuSubType; }
	
	// overrides of ld::relocatable::File 
	DebugInfoKind								debugInfo()	const override			{ return _debugInfo; }
	const char*									debugInfoPath() const override		{ return _debugInfoPath; }
	time_t										debugInfoModificationTime() const override
																					{ return _debugInfoModTime; }
	const std::vector<ld::relocatable::File::Stab>*	stabs()	const override			{ return NULL; }
	bool										canScatterAtoms() const override		{ return true; }
	void										forEachLtoSymbol(void (^handler)(const char*)) const override;
	LinkerOptionsList*							linkerOptions() const override		{ return NULL; }
	const ToolVersionList&						toolVersions() const override		{ return _toolVersions; }
	bool												isThinLTO() const			{ return _isThinLTO; }
	void												setIsThinLTO(bool ThinLTO) 	{ _isThinLTO = ThinLTO; }
	// fixme rdar://24734472 objCConstraint() and objcHasCategoryClassProperties()
	void												release();
	lto_module_t										module()					{ return _module; }
	class InternalAtom&									internalAtom()				{ return _internalAtom; }
	void												setDebugInfo(ld::relocatable::File::DebugInfoKind k,
																	const char* pth, time_t modTime, uint32_t subtype, uint8_t subtypeflags)
																					{	_debugInfo = k; 
																						_debugInfoPath = pth; 
																						_debugInfoModTime = modTime; 
																						_cpuSubType = subtype;
																						_cpuSubTypeFlags = subtypeflags;
																					}

    static bool                                         sSupportsLocalContext;
    static bool                                         sHasTriedLocalContext;
    bool                                                mergeIntoGenerator(lto_code_gen_t generator, bool useSetModule);
#if LTO_API_VERSION >= 18
	void                                                addToThinGenerator(thinlto_code_gen_t generator, int id);
#endif
private:
	friend class Atom;
	friend class InternalAtom;
	friend class Parser;

	bool									_isThinLTO;
	cpu_type_t								_architecture;
	class InternalAtom						_internalAtom;
	class Atom*								_atomArray;
	uint32_t								_atomArrayCount;
	lto_module_t							_module;
	const char*                             _path;
	const uint8_t*                          _content;
	uint32_t                                _contentLength;
	const char*								_debugInfoPath;
	time_t									_debugInfoModTime;
	ld::Section								_section;
	ld::Fixup								_fixupToInternal;
	ld::relocatable::File::DebugInfoKind	_debugInfo; 
	uint32_t								_cpuSubType;
	uint8_t									_cpuSubTypeFlags;
	ToolVersionList							_toolVersions;  // unused, may some day contain version of clang the created bitcode
};

//
// Atom acts as a proxy Atom for the symbols that are exported by LLVM bitcode file. Initially,
// Reader creates Atoms to allow linker proceed with usual symbol resolution phase. After
// optimization is performed, real Atoms are created for these symobls. However these real Atoms
// are not inserted into global symbol table. Atom holds real Atom and forwards appropriate
// methods to real atom.
//
class Atom : public ld::Atom
{
public:
										Atom(File& f, const char* name, ld::Atom::Scope s, 
												ld::Atom::Definition d, ld::Atom::Combine c, ld::Atom::Alignment a, bool ah);

	// overrides of ld::Atom
	const ld::File*				file() const override		{ return (_compiledAtom ? _compiledAtom->file() : &_file ); }
	const ld::File*				originalFile() const override	{ return &_file; }
	const char*					translationUnitSource() const override
															{ return (_compiledAtom ? _compiledAtom->translationUnitSource() : NULL); }
	const char*					name() const override		{ return _name; }
	uint64_t					size() const override		{ return (_compiledAtom ? _compiledAtom->size() : 0); }
	uint64_t					objectAddress() const override { return (_compiledAtom ? _compiledAtom->objectAddress() : 0); }
	void						copyRawContent(uint8_t buffer[]) const override
															{ if (_compiledAtom) _compiledAtom->copyRawContent(buffer); }
	const uint8_t*				rawContentPointer() const override
															{ return (_compiledAtom ? _compiledAtom->rawContentPointer() : NULL);  }
	unsigned long				contentHash(const class ld::IndirectBindingTable& ibt) const override
															{ return (_compiledAtom ? _compiledAtom->contentHash(ibt) : 0);  }
	bool						canCoalesceWith(const ld::Atom& rhs, const class ld::IndirectBindingTable& ibt) const override
															{ return (_compiledAtom ? _compiledAtom->canCoalesceWith(rhs,ibt) : false); }
	ld::Fixup::iterator				fixupsBegin() const override
															{ return (_compiledAtom ? _compiledAtom->fixupsBegin() : (ld::Fixup*)&_file._fixupToInternal); }
	ld::Fixup::iterator				fixupsEnd() const override
															{ return (_compiledAtom ? _compiledAtom->fixupsEnd() : &((ld::Fixup*)&_file._fixupToInternal)[1]); }
	ld::Atom::UnwindInfo::iterator	beginUnwind() const override
															{ return (_compiledAtom ? _compiledAtom->beginUnwind() : NULL); }
	ld::Atom::UnwindInfo::iterator	endUnwind() const override
															{ return (_compiledAtom ? _compiledAtom->endUnwind() : NULL); }
	ld::Atom::LineInfo::iterator	beginLineInfo() const override
															{ return (_compiledAtom ? _compiledAtom->beginLineInfo() : NULL); }
	ld::Atom::LineInfo::iterator	endLineInfo() const override
															{ return (_compiledAtom ? _compiledAtom->endLineInfo() : NULL); }
															
	const ld::Atom*						compiledAtom()		{ return _compiledAtom; }
	void								setCompiledAtom(const ld::Atom& atom);

private:

	File&								_file;
	const char*							_name;
	const ld::Atom*						_compiledAtom;
};

											





class Parser 
{
public:
	static bool						validFile(const uint8_t* fileContent, uint64_t fileLength, cpu_type_t architecture, cpu_subtype_t subarch);
	static const char*				fileKind(const uint8_t* fileContent, uint64_t fileLength);
	static File*					parse(const uint8_t* fileContent, uint64_t fileLength, const char* path, 
											time_t modTime, ld::File::Ordinal ordinal, cpu_type_t architecture, cpu_subtype_t subarch,
											bool logAllFiles, bool verboseOptimizationHints);
	static bool						libLTOisLoaded() { return (::lto_get_version() != NULL); }
	static bool						optimize(   const std::vector<const ld::Atom*>&	allAtoms,
												ld::Internal&						state,
												const OptimizeOptions&				options,
												ld::File::AtomHandler&				handler,
												std::vector<const ld::Atom*>&		newAtoms, 
												std::vector<const char*>&			additionalUndefines);

	static const char*				ltoVersion()	{ return ::lto_get_version(); }

private:

	static const char*				tripletPrefixForArch(cpu_type_t arch);
	static ld::relocatable::File*	parseMachOFile(const uint8_t* p, size_t len, const std::string &path, const OptimizeOptions& options,
												   ld::File::Ordinal ordinal);
#if LTO_API_VERSION >= 7
	static void ltoDiagnosticHandler(lto_codegen_diagnostic_severity_t, const char*, void*);
#endif

	typedef	std::unordered_set<const char*, ld::CStringHash, ld::CStringEquals>  CStringSet;
	typedef std::unordered_map<const char*, Atom*, ld::CStringHash, ld::CStringEquals> CStringToAtom;
	
	class AtomSyncer : public ld::File::AtomHandler {
	public:
							AtomSyncer(std::vector<const char*>& a, std::vector<const ld::Atom*>&na,
										const CStringToAtom &la, const CStringToAtom &dla, const OptimizeOptions& options) :
										_options(options), _additionalUndefines(a), _newAtoms(na), _llvmAtoms(la), _deadllvmAtoms(dla), _lastProxiedAtom(NULL), _lastProxiedFile(NULL) {}
		void		doAtom(const class ld::Atom&) override;
		void		doFile(const class ld::File&) override { }
		
		const OptimizeOptions&			_options;
		std::vector<const char*>&		_additionalUndefines;
		std::vector<const ld::Atom*>&	_newAtoms;
		const CStringToAtom					&_llvmAtoms;
		const CStringToAtom					&_deadllvmAtoms;
		const ld::Atom*					_lastProxiedAtom;
		const ld::File*					_lastProxiedFile;
	};

	static void						setPreservedSymbols(const std::vector<const ld::Atom*>&	allAtoms,
														ld::Internal&						state,
														const OptimizeOptions&				options,
														CStringToAtom &deadllvmAtoms,
														CStringToAtom &llvmAtoms,
														lto_code_gen_t generator);

	static std::tuple<uint8_t *, size_t> codegen(const OptimizeOptions& options,
												 ld::Internal&			state,
												 lto_code_gen_t			generator,
											     std::string&           object_path);


	static void loadMachO(ld::relocatable::File*				machoFile,
						  const OptimizeOptions&		options,
						  ld::File::AtomHandler&		handler,
						  std::vector<const ld::Atom*>&	newAtoms,
						  std::vector<const char*>&		additionalUndefines,
						  CStringToAtom					&llvmAtoms,
						  CStringToAtom					&deadllvmAtoms);

	static bool optimizeLTO(const std::vector<File*> files,
							const std::vector<const ld::Atom*>&	allAtoms,
							ld::Internal&						state,
							const OptimizeOptions&				options,
							ld::File::AtomHandler&				handler,
							std::vector<const ld::Atom*>&		newAtoms,
							std::vector<const char*>&			additionalUndefines);

	static bool optimizeThinLTO(const std::vector<File*>&              Files,
							    const std::vector<const ld::Atom*>&	allAtoms,
								ld::Internal&						state,
								const OptimizeOptions&				options,
								ld::File::AtomHandler&				handler,
								std::vector<const ld::Atom*>&		newAtoms,
								std::vector<const char*>&			additionalUndefines);

#if LTO_API_VERSION >= 18
	static thinlto_code_gen_t init_thinlto_codegen(const std::vector<File*>&           files,
												   const std::vector<const ld::Atom*>& allAtoms,
												   ld::Internal&				       state,
												   const OptimizeOptions&			   options,
												   CStringToAtom&                      deadllvmAtoms,
												   CStringToAtom&                      llvmAtoms);
#endif

	static std::vector<File*>		_s_files;
	static bool						_s_llvmOptionsProcessed;
};

std::vector<File*> Parser::_s_files;
bool Parser::_s_llvmOptionsProcessed = false;


bool Parser::validFile(const uint8_t* fileContent, uint64_t fileLength, cpu_type_t architecture, cpu_subtype_t subarch)
{
	for (const ArchInfo* t=archInfoArray; t->archName != NULL; ++t) {
		if ( (architecture == t->cpuType) && (!(t->isSubType) || (subarch == t->cpuSubType)) ) {
			bool result = ::lto_module_is_object_file_in_memory_for_target(fileContent, fileLength, t->llvmTriplePrefix);
			if ( !result ) {
				// <rdar://problem/8434487> LTO only supports thumbv7 not armv7
				if ( t->llvmTriplePrefixAlt[0] != '\0' ) {
					result = ::lto_module_is_object_file_in_memory_for_target(fileContent, fileLength, t->llvmTriplePrefixAlt);
				}
			}
			return result;
		}
	}
	return false;
}

const char* Parser::fileKind(const uint8_t* p, uint64_t fileLength)
{
	if ( (p[0] == 0xDE) && (p[1] == 0xC0) && (p[2] == 0x17) && (p[3] == 0x0B) ) {
		cpu_type_t arch = LittleEndian::get32(*((uint32_t*)(&p[16])));
		for (const ArchInfo* t=archInfoArray; t->archName != NULL; ++t) {
			if ( arch == t->cpuType ) {
				 if ( t->isSubType ) {
					if ( ::lto_module_is_object_file_in_memory_for_target(p, fileLength, t->llvmTriplePrefix) )
						return t->archName;
				}
				else {
					return t->archName;
				}
			}
		}
		return "unknown bitcode architecture";
	}
	return NULL;
}

File* Parser::parse(const uint8_t* fileContent, uint64_t fileLength, const char* path, time_t modTime, ld::File::Ordinal ordinal,
													cpu_type_t architecture, cpu_subtype_t subarch, bool logAllFiles, bool verboseOptimizationHints) 
{
	File* f = new File(path, modTime, ordinal, fileContent, fileLength, architecture);
	_s_files.push_back(f);
	if ( logAllFiles ) 
		printf("%s\n", path);
	return f;
}


ld::relocatable::File* Parser::parseMachOFile(const uint8_t* p, size_t len, const std::string &path, const OptimizeOptions& options,
											  ld::File::Ordinal ordinal)
{
	mach_o::relocatable::ParserOptions objOpts;
	objOpts.architecture		= options.arch;
	objOpts.objSubtypeMustMatch = false; 
	objOpts.logAllFiles			= false;
	objOpts.warnUnwindConversionProblems	= options.needsUnwindInfoSection;
	objOpts.keepDwarfUnwind		= options.keepDwarfUnwind;
	objOpts.forceDwarfConversion = false;
	objOpts.neverConvertDwarf   = false;
	objOpts.verboseOptimizationHints = options.verboseOptimizationHints;
	objOpts.armUsesZeroCostExceptions = options.armUsesZeroCostExceptions;
#if SUPPORT_ARCH_arm64e
	objOpts.supportsAuthenticatedPointers = options.supportsAuthenticatedPointers;
#endif
	objOpts.platforms			= options.platforms;
	objOpts.subType				= 0;
	objOpts.srcKind				= ld::relocatable::File::kSourceLTO;
	objOpts.treateBitcodeAsData = false;
	objOpts.usingBitcode		= options.bitcodeBundle;
	objOpts.maxDefaultCommonAlignment = options.maxDefaultCommonAlignment;
	objOpts.internalSDK			= options.internalSDK;
	objOpts.forceHidden			= false;

	const char *object_path = path.c_str();
	if (path.empty())
		object_path = "/tmp/lto.o";

	time_t modTime = 0;
	struct stat statBuffer;
	if ( stat(object_path, &statBuffer) == 0 )
		modTime = statBuffer.st_mtime;

	ld::relocatable::File* result = mach_o::relocatable::parse(p, len, strdup(object_path), modTime, ordinal, objOpts);
	if ( result != NULL )
		return result;
	throw "LLVM LTO, file is not of required architecture";
}



File::File(const char* pth, time_t mTime, ld::File::Ordinal ordinal, const uint8_t* content, uint32_t contentLength, cpu_type_t arch) 
	: ld::relocatable::File(pth,mTime,ordinal), _isThinLTO(false), _architecture(arch), _internalAtom(*this),
	_atomArray(NULL), _atomArrayCount(0), _module(NULL), _path(pth),
	_content(content), _contentLength(contentLength), _debugInfoPath(pth),
	_section("__TEXT_", "__tmp_lto", ld::Section::typeTempLTO),
	_fixupToInternal(0, ld::Fixup::k1of1, ld::Fixup::kindNone, &_internalAtom),
	_debugInfo(ld::relocatable::File::kDebugInfoNone), _cpuSubType(0)
{
	const bool log = false;
	
	// create llvm module
#if LTO_API_VERSION >= 11
	if ( sSupportsLocalContext || !sHasTriedLocalContext ) {
		_module = ::lto_module_create_in_local_context(content, contentLength, pth);
	}
	if ( !sHasTriedLocalContext ) {
		sHasTriedLocalContext = true;
		sSupportsLocalContext = (_module != NULL);
	}
	if ( (_module == NULL) && !sSupportsLocalContext )
#endif
#if LTO_API_VERSION >= 9
	_module = ::lto_module_create_from_memory_with_path(content, contentLength, pth);
	if ( _module == NULL && !sSupportsLocalContext )
#endif
	_module = ::lto_module_create_from_memory(content, contentLength);
    if ( _module == NULL )
		throwf("could not parse object file %s: '%s', using libLTO version '%s'", pth, ::lto_get_error_message(), ::lto_get_version());

	if ( log ) fprintf(stderr, "bitcode file: %s\n", pth);

#if LTO_API_VERSION >= 18
	_isThinLTO = ::lto_module_is_thinlto(_module);
#endif

	// create atom for each global symbol in module
	uint32_t count = ::lto_module_get_num_symbols(_module);
	_atomArray = (Atom*)malloc(sizeof(Atom)*count);
	for (uint32_t i=0; i < count; ++i) {
		const char* name = ::lto_module_get_symbol_name(_module, i);
		lto_symbol_attributes attr = lto_module_get_symbol_attribute(_module, i);

		// <rdar://problem/6378110> LTO doesn't like dtrace symbols
		// ignore dtrace static probes for now
		// later when codegen is done and a mach-o file is produces the probes will be processed
		if ( (strncmp(name, "___dtrace_probe$", 16) == 0) || (strncmp(name, "___dtrace_isenabled$", 20) == 0) )
			continue;
				
		ld::Atom::Definition def;
		ld::Atom::Combine combine = ld::Atom::combineNever;
		switch ( attr & LTO_SYMBOL_DEFINITION_MASK ) {
			case LTO_SYMBOL_DEFINITION_REGULAR:
				def = ld::Atom::definitionRegular;
				break;
			case LTO_SYMBOL_DEFINITION_TENTATIVE:
				def = ld::Atom::definitionTentative;
				break;
			case LTO_SYMBOL_DEFINITION_WEAK:
				def = ld::Atom::definitionRegular;
				combine = ld::Atom::combineByName;
				break;
			case LTO_SYMBOL_DEFINITION_UNDEFINED:
			case LTO_SYMBOL_DEFINITION_WEAKUNDEF:
				def = ld::Atom::definitionProxy;
				break;
			default:
				throwf("unknown definition kind for symbol %s in bitcode file %s", name, pth);
		}

		// make LLVM atoms for definitions and a reference for undefines
		if ( def != ld::Atom::definitionProxy ) {
			ld::Atom::Scope scope;
			bool autohide = false;
			switch ( attr & LTO_SYMBOL_SCOPE_MASK) {
				case LTO_SYMBOL_SCOPE_INTERNAL:
					scope = ld::Atom::scopeTranslationUnit;
					break;
				case LTO_SYMBOL_SCOPE_HIDDEN:
					scope = ld::Atom::scopeLinkageUnit;
					break;
				case LTO_SYMBOL_SCOPE_DEFAULT:
					scope = ld::Atom::scopeGlobal;
					break;
#if LTO_API_VERSION >= 4
				case LTO_SYMBOL_SCOPE_DEFAULT_CAN_BE_HIDDEN:
					scope = ld::Atom::scopeGlobal;
					autohide = true;
					break;
#endif
				default:
					throwf("unknown scope for symbol %s in bitcode file %s", name, pth);
			}
			// only make atoms for non-internal symbols
			if ( scope == ld::Atom::scopeTranslationUnit )
				continue;
			uint8_t alignment = (attr & LTO_SYMBOL_ALIGNMENT_MASK);
			// make Atom using placement new operator
			new (&_atomArray[_atomArrayCount++]) Atom(*this, name, scope, def, combine, alignment, autohide);
			if ( scope != ld::Atom::scopeTranslationUnit )
				_internalAtom.addReference(name);
			if ( log ) fprintf(stderr, "\t0x%08X %s\n", attr, name);
		}
		else {
			// add to list of external references
			_internalAtom.addReference(name);
			if ( log ) fprintf(stderr, "\t%s (undefined)\n", name);
		}
	}

#if LTO_API_VERSION >= 11
	if ( sSupportsLocalContext )
		this->release();
#endif
}

File::~File()
{
	this->release();
}

// this is only called when generating a text map file, to better detail where code came from
void File::forEachLtoSymbol(void (^handler)(const char*)) const
{
#if LTO_API_VERSION >= 11
	if ( lto_module_t module = ::lto_module_create_in_local_context(_content, _contentLength, _path) ) {
		uint32_t count = ::lto_module_get_num_symbols(module);
		for (uint32_t i=0; i < count; ++i) {
			lto_symbol_attributes attr = lto_module_get_symbol_attribute(module, i);
			switch ( attr & LTO_SYMBOL_DEFINITION_MASK ) {
				case LTO_SYMBOL_DEFINITION_REGULAR:
				case LTO_SYMBOL_DEFINITION_TENTATIVE:
				case LTO_SYMBOL_DEFINITION_WEAK:
					handler(::lto_module_get_symbol_name(module, i));
					break;
				default:
					break;
			}
		}
		::lto_module_dispose(module);
	}
#endif
}

bool File::mergeIntoGenerator(lto_code_gen_t generator, bool useSetModule) {
#if LTO_API_VERSION >= 11
    if ( sSupportsLocalContext ) {
        assert(!_module && "Expected module to be disposed");
        _module = ::lto_module_create_in_codegen_context(_content, _contentLength,
                                                        _path, generator);
        if ( _module == NULL )
            throwf("could not reparse object file %s: '%s', using libLTO version '%s'",
                  _path, ::lto_get_error_message(), ::lto_get_version());
    }
#endif
    assert(_module && "Expected module to stick around");
#if LTO_API_VERSION >= 13
    if (useSetModule) {
        // lto_codegen_set_module will transfer ownership of the module to LTO code generator,
        // so we don't need to release the module here.
        ::lto_codegen_set_module(generator, _module);
        return false;
    }
#endif
    if ( ::lto_codegen_add_module(generator, _module) )
        return true;

    // <rdar://problem/15471128> linker should release module as soon as possible
    this->release();
    return false;
}

#if LTO_API_VERSION >= 18
void File::addToThinGenerator(thinlto_code_gen_t generator, int id) {
	assert(!_module && "Expected module to be disposed");
	std::string pathWithID = _path;
	pathWithID += std::to_string(id);
	::thinlto_codegen_add_module(generator, strdup(pathWithID.c_str()), (const char *)_content, _contentLength);
}
#endif

void File::release()
{
	if ( _module != NULL )
		::lto_module_dispose(_module);
	_module = NULL;
}

bool File::forEachAtom(ld::File::AtomHandler& handler) const
{
	handler.doAtom(_internalAtom);
	for(uint32_t i=0; i < _atomArrayCount; ++i) {
		handler.doAtom(_atomArray[i]);
	}
	return true;
}

InternalAtom::InternalAtom(File& f)
	: ld::Atom(f._section, ld::Atom::definitionRegular, ld::Atom::combineNever, ld::Atom::scopeTranslationUnit, 
				ld::Atom::typeLTOtemporary, ld::Atom::symbolTableNotIn, true, false, false, ld::Atom::Alignment(0)),
		_file(f)
{
}

Atom::Atom(File& f, const char* nm, ld::Atom::Scope s, ld::Atom::Definition d, ld::Atom::Combine c, 
			ld::Atom::Alignment a, bool ah)
	: ld::Atom(f._section, d, c, s, ld::Atom::typeLTOtemporary, 
				ld::Atom::symbolTableIn, false, false, false, a),
		_file(f), _name(strdup(nm)), _compiledAtom(NULL)
{
	if ( ah )
		this->setAutoHide();
}

void Atom::setCompiledAtom(const ld::Atom& atom)
{
	// set delegate so virtual methods go to it
	_compiledAtom = &atom;
	
	//fprintf(stderr, "setting lto atom %p to delegate to mach-o atom %p (%s)\n", this, &atom, atom.name());
	
	// update fields in ld::Atom to match newly constructed mach-o atom
	(const_cast<Atom*>(this))->setAttributesFromAtom(atom);
}



// <rdar://problem/12379604> The order that files are merged must match command line order
struct CommandLineOrderFileSorter
{
     bool operator()(File* left, File* right)
     {
        return ( left->ordinal() < right->ordinal() );
     }
};


#if LTO_API_VERSION >= 7
void Parser::ltoDiagnosticHandler(lto_codegen_diagnostic_severity_t severity, const char* message, void*) 
{
	switch ( severity ) {
#if LTO_API_VERSION >= 10
		case LTO_DS_REMARK:
			fprintf(stderr, "ld: LTO remark: %s\n", message);
			break;
#endif
		case LTO_DS_NOTE:
		case LTO_DS_WARNING:
			warning("%s", message);
			break;
		case LTO_DS_ERROR:
			throwf("%s", message);
	}
}
#endif


/// Instruct libLTO about the list of symbols to preserve, compute deadllvmAtoms and llvmAtoms
void Parser::setPreservedSymbols(	const std::vector<const ld::Atom*>&	allAtoms,
									ld::Internal&						state,
									const OptimizeOptions&				options,
									CStringToAtom &deadllvmAtoms,
									CStringToAtom &llvmAtoms,
									lto_code_gen_t generator) {
	const bool logMustPreserve = false;

	// The atom graph uses directed edges (references). Collect all references where
	// originating atom is not part of any LTO Reader. This allows optimizer to optimize an
	// external (i.e. not originated from same .o file) reference if all originating atoms are also
	// defined in llvm bitcode file.
	CStringSet nonLLVMRefs;
	bool hasNonllvmAtoms = false;
	for (std::vector<const ld::Atom*>::const_iterator it = allAtoms.begin(); it != allAtoms.end(); ++it) {
		const ld::Atom* atom = *it;
		// only look at references that come from an atom that is not an LTO atom
		if (atom->contentType() != ld::Atom::typeLTOtemporary ||
			((lto::File *)atom->file())->isThinLTO()) {
			if ( (atom->section().type() != ld::Section::typeMachHeader) && (atom->definition() != ld::Atom::definitionProxy) ) {
				hasNonllvmAtoms = true;
			}
			const ld::Atom* target;
			for (ld::Fixup::iterator fit=atom->fixupsBegin(); fit != atom->fixupsEnd(); ++fit) {
				switch ( fit->binding ) {
					case ld::Fixup::bindingDirectlyBound:
						// that reference an llvm atom
						if ( fit->u.target->contentType() == ld::Atom::typeLTOtemporary )
							nonLLVMRefs.insert(fit->u.target->name());
						break;
					case ld::Fixup::bindingsIndirectlyBound:
						target = state.indirectBindingTable[fit->u.bindingIndex];
						if ( (target != NULL) && (target->contentType() == ld::Atom::typeLTOtemporary) )
							nonLLVMRefs.insert(target->name());
					default:
						break;
				}
			}
		}
		else if ( atom->scope() >= ld::Atom::scopeLinkageUnit ) {
			llvmAtoms[atom->name()] = (Atom*)atom;
		}
	}
	// if entry point is in a llvm bitcode file, it must be preserved by LTO
	if ( state.entryPoint!= NULL ) {
		if ( state.entryPoint->contentType() == ld::Atom::typeLTOtemporary )
			nonLLVMRefs.insert(state.entryPoint->name());
	}

	// deadAtoms are the atoms that the linker coalesced.  For instance weak or tentative definitions
	// overriden by another atom.  If any of these deadAtoms are llvm atoms and they were replaced
	// with a mach-o atom, we need to tell the lto engine to preserve (not optimize away) its dead
	// atom so that the linker can replace it with the mach-o one later.
	for (std::vector<const ld::Atom*>::const_iterator it = allAtoms.begin(); it != allAtoms.end(); ++it) {
		const ld::Atom* atom = *it;
		if ( atom->coalescedAway() && (atom->contentType() == ld::Atom::typeLTOtemporary) ) {
			const char* name = atom->name();
			if ( logMustPreserve ) fprintf(stderr, "lto_codegen_add_must_preserve_symbol(%s) because linker coalesce away and replace with a mach-o atom\n", name);
			::lto_codegen_add_must_preserve_symbol(generator, name);
			deadllvmAtoms[name] = (Atom*)atom;
		}
	}
	for (std::vector<File*>::iterator it=_s_files.begin(); it != _s_files.end(); ++it) {
		File* file = *it;
		for(uint32_t i=0; i < file->_atomArrayCount; ++i) {
			Atom* llvmAtom = &file->_atomArray[i];
			if ( llvmAtom->coalescedAway()  ) {
				const char* name = llvmAtom->name();
				if ( deadllvmAtoms.find(name) == deadllvmAtoms.end() ) {
					if ( logMustPreserve )
						fprintf(stderr, "lto_codegen_add_must_preserve_symbol(%s) because linker coalesce away and replace with a mach-o atom\n", name);
					::lto_codegen_add_must_preserve_symbol(generator, name);
					deadllvmAtoms[name] = (Atom*)llvmAtom;
				}
			}
			else if ( options.linkerDeadStripping && !llvmAtom->live() ) {
				const char* name = llvmAtom->name();
				deadllvmAtoms[name] = (Atom*)llvmAtom;
			}
		}
	}

	// tell code generator about symbols that must be preserved
	for (CStringToAtom::iterator it = llvmAtoms.begin(); it != llvmAtoms.end(); ++it) {
		const char* name = it->first;
		Atom* atom = it->second;
		// Include llvm Symbol in export list if it meets one of following two conditions
		// 1 - atom scope is global (and not linkage unit).
		// 2 - included in nonLLVMRefs set.
		// If a symbol is not listed in exportList then LTO is free to optimize it away.
		if ( (atom->scope() == ld::Atom::scopeGlobal) && options.preserveAllGlobals ) {
			if ( logMustPreserve ) fprintf(stderr, "lto_codegen_add_must_preserve_symbol(%s) because global symbol\n", name);
			::lto_codegen_add_must_preserve_symbol(generator, name);
		}
		else if ( nonLLVMRefs.find(name) != nonLLVMRefs.end() ) {
			if ( logMustPreserve ) fprintf(stderr, "lto_codegen_add_must_preserve_symbol(%s) because referenced by a mach-o atom\n", name);
			::lto_codegen_add_must_preserve_symbol(generator, name);
		}
		else if ( options.relocatable && hasNonllvmAtoms ) {
			// <rdar://problem/14334895> ld -r mode but merging in some mach-o files, so need to keep libLTO from optimizing away anything
			if ( logMustPreserve ) fprintf(stderr, "lto_codegen_add_must_preserve_symbol(%s) because -r mode disable LTO dead stripping\n", name);
			::lto_codegen_add_must_preserve_symbol(generator, name);
		}
	}

	// <rdar://problem/16165191> tell code generator to preserve initial undefines
	for( std::vector<const char*>::const_iterator it=options.initialUndefines->begin(); it != options.initialUndefines->end(); ++it) {
		if ( logMustPreserve ) fprintf(stderr, "lto_codegen_add_must_preserve_symbol(%s) because it is an initial undefine\n", *it);
		::lto_codegen_add_must_preserve_symbol(generator, *it);
	}

	// special case running ld -r on all bitcode files to produce another bitcode file (instead of mach-o)
	if ( options.relocatable && !hasNonllvmAtoms ) {
#if LTO_API_VERSION >= 15
		::lto_codegen_set_should_embed_uselists(generator, false);
#endif
		if ( ! ::lto_codegen_write_merged_modules(generator, options.outputFilePath) ) {
			// HACK, no good way to tell linker we are all done, so just quit
			exit(0);
		}
		warning("could not produce merged bitcode file");
	}

}

// Retrieve the codegen model from the options
static lto_codegen_model getCodeModel(const OptimizeOptions& options) {
	if ( options.mainExecutable ) {
		if ( options.staticExecutable ) {
			// x86_64 "static" or any "-static -pie" is really dynamic code model
			if ( (options.arch == CPU_TYPE_X86_64) || options.pie )
				return LTO_CODEGEN_PIC_MODEL_DYNAMIC;
			else
				return LTO_CODEGEN_PIC_MODEL_STATIC;
		}
		else if ( options.preload ) {
			if ( options.pie )
				return LTO_CODEGEN_PIC_MODEL_DYNAMIC;
			else
				return LTO_CODEGEN_PIC_MODEL_STATIC;
		}
		else {
			if ( options.pie )
				return LTO_CODEGEN_PIC_MODEL_DYNAMIC;
			else
				return LTO_CODEGEN_PIC_MODEL_DYNAMIC_NO_PIC;
		}
	}
	else {
		if ( options.allowTextRelocs )
			return LTO_CODEGEN_PIC_MODEL_DYNAMIC_NO_PIC;
		else
			return LTO_CODEGEN_PIC_MODEL_DYNAMIC;
	}

}

std::tuple<uint8_t *, size_t> Parser::codegen(const OptimizeOptions& options,
											  ld::Internal&			 state,
											  lto_code_gen_t		 generator,
											  std::string&           object_path) {
	uint8_t *machOFile;
	size_t machOFileLen;

	if ( ::lto_codegen_set_pic_model(generator, getCodeModel(options)) )
		throwf("could not create set codegen model: %s", lto_get_error_message());

	// if requested, save off merged bitcode file
	if ( options.saveTemps ) {
		char tempBitcodePath[MAXPATHLEN];
		strcpy(tempBitcodePath, options.outputFilePath);
		strcat(tempBitcodePath, ".lto.bc");
#if LTO_API_VERSION >= 15
		::lto_codegen_set_should_embed_uselists(generator, true);
#endif
		::lto_codegen_write_merged_modules(generator, tempBitcodePath);
	}

#if LTO_API_VERSION >= 3
	// find assembler next to linker
	char path[PATH_MAX];
	uint32_t bufSize = PATH_MAX;
	if ( _NSGetExecutablePath(path, &bufSize) != -1 ) {
		char* lastSlash = strrchr(path, '/');
		if ( lastSlash != NULL ) {
			strcpy(lastSlash+1, "as");
			struct stat statInfo;
			if ( stat(path, &statInfo) == 0 )
				::lto_codegen_set_assembler_path(generator, path);
		}
	}
#endif

	// When lto API version is greater than or equal to 12, we use lto_codegen_optimize and lto_codegen_compile_optimized
	// instead of lto_codegen_compile, and we save the merged bitcode file in between.
	bool useSplitAPI = false;
#if LTO_API_VERSION >= 12
	if ( ::lto_api_version() >= 12)
		useSplitAPI = true;
#endif

	if ( useSplitAPI) {
#if LTO_API_VERSION >= 12
#if LTO_API_VERSION >= 14
		if ( ::lto_api_version() >= 14 && options.ltoCodegenOnly)
			lto_codegen_set_should_internalize(generator, false);
#endif
		// run optimizer
		if ( !options.ltoCodegenOnly && ::lto_codegen_optimize(generator) )
			throwf("could not do LTO optimization: '%s', using libLTO version '%s'", ::lto_get_error_message(), ::lto_get_version());

		if ( options.saveTemps || options.bitcodeBundle ) {
			// save off merged bitcode file
			char tempOptBitcodePath[MAXPATHLEN];
			strcpy(tempOptBitcodePath, options.outputFilePath);
			strcat(tempOptBitcodePath, ".lto.opt.bc");
#if LTO_API_VERSION >= 15
			::lto_codegen_set_should_embed_uselists(generator, true);
#endif
			::lto_codegen_write_merged_modules(generator, tempOptBitcodePath);
			if ( options.bitcodeBundle )
				state.ltoBitcodePath.push_back(tempOptBitcodePath);
		}

		// run code generator
		machOFile = (uint8_t*)::lto_codegen_compile_optimized(generator, &machOFileLen);
#endif
		if ( machOFile == NULL )
			throwf("could not do LTO codegen: '%s', using libLTO version '%s'", ::lto_get_error_message(), ::lto_get_version());
	}
	else {
		// run optimizer and code generator
		machOFile = (uint8_t*)::lto_codegen_compile(generator, &machOFileLen);
		if ( machOFile == NULL )
			throwf("could not do LTO codegen: '%s', using libLTO version '%s'", ::lto_get_error_message(), ::lto_get_version());
		if ( options.saveTemps ) {
			// save off merged bitcode file
			char tempOptBitcodePath[MAXPATHLEN];
			strcpy(tempOptBitcodePath, options.outputFilePath);
			strcat(tempOptBitcodePath, ".lto.opt.bc");
#if LTO_API_VERSION >= 15
			::lto_codegen_set_should_embed_uselists(generator, true);
#endif
			::lto_codegen_write_merged_modules(generator, tempOptBitcodePath);
		}
	}

	// if requested, save off temp mach-o file
	if ( options.saveTemps ) {
		char tempMachoPath[MAXPATHLEN];
		strcpy(tempMachoPath, options.outputFilePath);
		strcat(tempMachoPath, ".lto.o");
		int fd = ::open(tempMachoPath, O_CREAT | O_WRONLY | O_TRUNC, 0666);
		if ( fd != -1) {
			::write(fd, machOFile, machOFileLen);
			::close(fd);
		}
	}

	// if needed, save temp mach-o file to specific location
	if ( !object_path.empty() ) {
		int fd = ::open(object_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
		if ( fd != -1) {
			::write(fd, machOFile, machOFileLen);
			::close(fd);
		}
		else {
			warning("could not write LTO temp file '%s', errno=%d", object_path.c_str(), errno);
		}
	}
	return std::make_tuple(machOFile, machOFileLen);
}

/// Load the MachO located in buffer \p machOFile with size \p machOFileLen.
/// The loaded atoms are sync'ed using all the supplied lists.
void Parser::loadMachO(ld::relocatable::File*				machoFile,
					   const OptimizeOptions&				options,
					   ld::File::AtomHandler&				handler,
					   std::vector<const ld::Atom*>&		newAtoms,
					   std::vector<const char*>&			additionalUndefines,
					   CStringToAtom						&llvmAtoms,
					   CStringToAtom						&deadllvmAtoms) {
	const bool logAtomsBeforeSync = false;

	// sync generated mach-o atoms with existing atoms ld knows about
	if ( logAtomsBeforeSync ) {
		fprintf(stderr, "llvmAtoms:\n");
		for (CStringToAtom::iterator it = llvmAtoms.begin(); it != llvmAtoms.end(); ++it) {
			const char* name = it->first;
			Atom* atom = it->second;
			fprintf(stderr, "\t%p\t%s\n", atom, name);
		}
		fprintf(stderr, "deadllvmAtoms:\n");
		for (CStringToAtom::iterator it = deadllvmAtoms.begin(); it != deadllvmAtoms.end(); ++it) {
			const char* name = it->first;
			Atom* atom = it->second;
			fprintf(stderr, "\t%p\t%s\n", atom, name);
		}
	}
	AtomSyncer syncer(additionalUndefines, newAtoms, llvmAtoms, deadllvmAtoms, options);
	machoFile->forEachAtom(syncer);

	// notify about file level attributes
	handler.doFile(*machoFile);
}

// Full LTO processing
bool Parser::optimizeLTO(const std::vector<File*>				files,
						 const std::vector<const ld::Atom*>&	allAtoms,
						 ld::Internal&							state,
						 const OptimizeOptions&					options,
						 ld::File::AtomHandler&					handler,
						 std::vector<const ld::Atom*>&			newAtoms,
						 std::vector<const char*>&				additionalUndefines) {
	const bool logExtraOptions = false;
	const bool logBitcodeFiles = false;

	if (files.empty())
		return true;

	// create optimizer and add each Reader
	lto_code_gen_t generator = NULL;
#if LTO_API_VERSION >= 11
	if ( File::sSupportsLocalContext )
		generator = ::lto_codegen_create_in_local_context();
	else
#endif
		generator = ::lto_codegen_create();
#if LTO_API_VERSION >= 7
	lto_codegen_set_diagnostic_handler(generator, ltoDiagnosticHandler, NULL);
#endif

	ld::File::Ordinal lastOrdinal;

	// When flto_codegen_only is on and we have a single .bc file, use lto_codegen_set_module instead of
	// lto_codegen_add_module, to make sure the the destination module will be the same as the input .bc file.
	bool useSetModule = false;
#if LTO_API_VERSION >= 13
	useSetModule = (files.size() == 1) && options.ltoCodegenOnly && (::lto_api_version() >= 13);
#endif
	for (auto *f : files) {
		assert(f->ordinal() > lastOrdinal);
		if ( logBitcodeFiles && !useSetModule ) fprintf(stderr, "lto_codegen_add_module(%s)\n", f->path());
		if ( logBitcodeFiles && useSetModule ) fprintf(stderr, "lto_codegen_set_module(%s)\n", f->path());
		if ( f->mergeIntoGenerator(generator, useSetModule) )
			throwf("lto: could not merge in %s because '%s', using libLTO version '%s'", f->path(), ::lto_get_error_message(), ::lto_get_version());
		lastOrdinal = f->ordinal();
	}

	// add any -mllvm command line options
	if ( !_s_llvmOptionsProcessed ) {
		bool shouldUseOldDebugOptions = true;
#if LTO_API_VERSION >= 26
		// Only use the _array version of the function if both the header and the dylib support it.
		// Use the old debug_options version when the dylib version is older.
		if (lto_api_version() >= 26) {
			shouldUseOldDebugOptions = false;
			lto_codegen_debug_options_array(generator, options.llvmOptions->data(), options.llvmOptions->size());
		}
#endif
		for (const char* opt : *options.llvmOptions) {
			if ( logExtraOptions ) fprintf(stderr, "passing option to llvm: %s\n", opt);
			if (shouldUseOldDebugOptions)
				::lto_codegen_debug_options(generator, opt);
		}
		_s_llvmOptionsProcessed = true;
	}

	// <rdar://problem/13687397> Need a way for LTO to get cpu variants (until that info is in bitcode)
	if ( options.mcpu != NULL )
		::lto_codegen_set_cpu(generator, options.mcpu);

	// Compute the preserved symbols
	CStringToAtom deadllvmAtoms, llvmAtoms;
	setPreservedSymbols(allAtoms, state, options, deadllvmAtoms, llvmAtoms, generator);

	size_t machOFileLen = 0;
	const uint8_t* machOFile = NULL;

	// mach-o parsing is done in-memory, but need path for debug notes
	std::string object_path;
	if ( options.tmpObjectFilePath != NULL ) {
		object_path = options.tmpObjectFilePath;
		// If the path exists and is a directory (for instance if some files
		// were processed with ThinLTO before), we create the LTO file inside
		// the directory.
		struct stat statBuffer;
		if( stat(object_path.c_str(), &statBuffer) == 0 && S_ISDIR(statBuffer.st_mode) ) {
			object_path += "/lto.o";
		}
	}

	// Codegen Now
	std::tie(machOFile, machOFileLen) = codegen(options, state, generator, object_path);

	// parse generated mach-o file into a MachOReader
	ld::relocatable::File* machoFile = parseMachOFile(machOFile, machOFileLen, object_path, options, ld::File::Ordinal::LTOOrdinal());

	// Load the generated MachO file
	loadMachO(machoFile, options, handler, newAtoms, additionalUndefines, llvmAtoms, deadllvmAtoms);

	// Remove Atoms from ld if code generator optimized them away
	for (CStringToAtom::iterator li = llvmAtoms.begin(), le = llvmAtoms.end(); li != le; ++li) {
		// check if setRealAtom() called on this Atom
		if ( li->second->compiledAtom() == NULL ) {
			//fprintf(stderr, "llvm optimized away %p %s\n", li->second, li->second->name());
			li->second->setCoalescedAway();
		}
	}

	// if final mach-o file has debug info, update original bitcode files to match
	for (auto *f : files) {
		f->setDebugInfo(machoFile->debugInfo(), machoFile->path(), machoFile->modificationTime(), machoFile->cpuSubType(), machoFile->cpuSubTypeFlags());
	}

	return true;
}

#if LTO_API_VERSION >= 18
// Create the ThinLTO codegenerator
thinlto_code_gen_t Parser::init_thinlto_codegen(const std::vector<File*>&           files,
									            const std::vector<const ld::Atom*>& allAtoms,
												ld::Internal&				       state,
												const OptimizeOptions&			   options,
											    CStringToAtom&                      deadllvmAtoms,
											    CStringToAtom&                      llvmAtoms) {
	const bool logMustPreserve = false;

    thinlto_code_gen_t thingenerator = ::thinlto_create_codegen();

	// Caching control
	if (options.ltoCachePath && !options.bitcodeBundle) {
		struct stat statBuffer;
		if( stat(options.ltoCachePath, &statBuffer) != 0 || !S_ISDIR(statBuffer.st_mode) ) {
			if ( mkdir(options.ltoCachePath, 0700) !=0 ) {
				warning("unable to create ThinLTO cache directory: %s", options.ltoCachePath);
			}
		}
		thinlto_codegen_set_cache_dir(thingenerator, options.ltoCachePath);
		if (options.ltoPruneIntervalOverwrite)
			thinlto_codegen_set_cache_pruning_interval(thingenerator, options.ltoPruneInterval);
		thinlto_codegen_set_cache_entry_expiration(thingenerator, options.ltoPruneAfter);
		thinlto_codegen_set_final_cache_size_relative_to_available_space(thingenerator, options.ltoMaxCacheSize);
	}

	// if requested, ask the code generator to save off intermediate bitcode files
	if ( options.saveTemps ) {
		std::string tempPath = options.outputFilePath;
		tempPath += ".thinlto.bcs/";
		struct stat statBuffer;
		if( stat(tempPath.c_str(), &statBuffer) != 0 || !S_ISDIR(statBuffer.st_mode) ) {
			if ( mkdir(tempPath.c_str(), 0700) !=0 ) {
				warning("unable to create ThinLTO output directory for temporary bitcode files: %s", tempPath.c_str());
			}
		}
		thinlto_codegen_set_savetemps_dir(thingenerator, tempPath.c_str());
	}

	// Set some codegen options
	if ( thinlto_codegen_set_pic_model(thingenerator, getCodeModel(options)) )
		throwf("could not create set codegen model: %s", lto_get_error_message());

	// Expose reachability informations for internalization in LTO

	// The atom graph uses directed edges (references). Collect all references where
	// originating atom is not part of any LTO Reader. This allows optimizer to optimize an
	// external (i.e. not originated from same .o file) reference if all originating atoms are also
	// defined in llvm bitcode file.
	CStringSet nonLLVMRefs;
	CStringSet LLVMRefs;
	for (std::vector<const ld::Atom*>::const_iterator it = allAtoms.begin(); it != allAtoms.end(); ++it) {
		const ld::Atom* atom = *it;
		const ld::Atom* target;
		for (ld::Fixup::iterator fit=atom->fixupsBegin(); fit != atom->fixupsEnd(); ++fit) {
			switch ( fit->binding ) {
				case ld::Fixup::bindingDirectlyBound:
					// that reference a ThinLTO llvm atom
					target = fit->u.target;
					if ( target->contentType() == ld::Atom::typeLTOtemporary &&
						((lto::File *)target->file())->isThinLTO() &&
						atom->file() != target->file()
						) {
						if (atom->contentType() != ld::Atom::typeLTOtemporary ||
							!((lto::File *)atom->file())->isThinLTO())
							nonLLVMRefs.insert(target->name());
						else
							LLVMRefs.insert(target->name());
						if ( logMustPreserve )
							fprintf(stderr, "Found a reference from %s -> %s\n", atom->name(), target->name());
					}
					break;
				case ld::Fixup::bindingsIndirectlyBound:
					target = state.indirectBindingTable[fit->u.bindingIndex];
					if ( (target != NULL) && (target->contentType() == ld::Atom::typeLTOtemporary)  &&
						((lto::File *)target->file())->isThinLTO() &&
						atom->file() != target->file()
						) {
						if (atom->contentType() != ld::Atom::typeLTOtemporary ||
							!((lto::File *)atom->file())->isThinLTO())
							nonLLVMRefs.insert(target->name());
						else
							LLVMRefs.insert(target->name());
						if ( logMustPreserve )
							fprintf(stderr, "Found a reference from %s -> %s\n", atom->name(), target->name());
					}
				default:
					break;
			}
		}
		if (atom->contentType() == ld::Atom::typeLTOtemporary &&
			((lto::File *)atom->file())->isThinLTO() &&
			atom != &((lto::File *)atom->file())->internalAtom()) {
			assert(atom->scope() != ld::Atom::scopeTranslationUnit && "LTO should not expose static atoms");
			assert(llvmAtoms.find(atom->name()) == llvmAtoms.end() && "Unexpected llvmAtom with duplicate name");
			llvmAtoms[atom->name()] = (Atom*)atom;
		}
	}
	// if entry point is in a llvm bitcode file, it must be preserved by LTO
	if ( state.entryPoint != NULL ) {
		if ( state.entryPoint->contentType() == ld::Atom::typeLTOtemporary )
			nonLLVMRefs.insert(state.entryPoint->name());
	}
	for (auto file : files) {
		for(uint32_t i=0; i < file->_atomArrayCount; ++i) {
			Atom* llvmAtom = &file->_atomArray[i];
			if ( llvmAtom->coalescedAway()  ) {
				const char* name = llvmAtom->name();
				if ( deadllvmAtoms.find(name) == deadllvmAtoms.end() ) {
					if ( logMustPreserve )
						fprintf(stderr, "thinlto_codegen_add_must_preserve_symbol(%s) because linker coalesce away and replace with a mach-o atom\n", name);
					::thinlto_codegen_add_must_preserve_symbol(thingenerator, name, strlen(name));
					deadllvmAtoms[name] = (Atom*)llvmAtom;
				}
			}
			else if ( options.linkerDeadStripping && !llvmAtom->live() ) {
				const char* name = llvmAtom->name();
				deadllvmAtoms[name] = (Atom*)llvmAtom;
			}
		}
	}

	// tell code generator about symbols that must be preserved
	for (CStringToAtom::iterator it = llvmAtoms.begin(); it != llvmAtoms.end(); ++it) {
		const char* name = it->first;
		Atom* atom = it->second;
		// Include llvm Symbol in export list if it meets one of following two conditions
		// 1 - atom scope is global (and not linkage unit).
		// 2 - included in nonLLVMRefs set.
		// If a symbol is not listed in exportList then LTO is free to optimize it away.
		if ( (atom->scope() == ld::Atom::scopeGlobal) && options.preserveAllGlobals ) {
			if ( logMustPreserve ) fprintf(stderr, "thinlto_codegen_add_must_preserve_symbol(%s) because global symbol\n", name);
			::thinlto_codegen_add_must_preserve_symbol(thingenerator, name, strlen(name));
		}
		else if ( nonLLVMRefs.find(name) != nonLLVMRefs.end() ) {
			if ( logMustPreserve ) fprintf(stderr, "thinlto_codegen_add_must_preserve_symbol(%s) because referenced from outside of ThinLTO\n", name);
			::thinlto_codegen_add_must_preserve_symbol(thingenerator, name, strlen(name));
		}
		else if ( LLVMRefs.find(name) != LLVMRefs.end() ) {
			if ( logMustPreserve ) fprintf(stderr, "thinlto_codegen_add_cross_referenced_symbol(%s) because referenced from another file\n", name);
			::thinlto_codegen_add_cross_referenced_symbol(thingenerator, name, strlen(name));
		} else {
			if ( logMustPreserve ) fprintf(stderr, "NOT preserving(%s)\n", name);
		}

		// <rdar://problem/16165191> tell code generator to preserve initial undefines
		for (const char* undefName : *options.initialUndefines) {
			if ( logMustPreserve ) fprintf(stderr, "thinlto_codegen_add_cross_referenced_symbol(%s) because it is an initial undefine\n", undefName);
			::thinlto_codegen_add_cross_referenced_symbol(thingenerator, undefName, strlen(undefName));
		}

// FIXME: to be implemented
//		else if ( options.relocatable && hasNonllvmAtoms ) {
//			// <rdar://problem/14334895> ld -r mode but merging in some mach-o files, so need to keep libLTO from optimizing away anything
//			if ( logMustPreserve ) fprintf(stderr, "lto_codegen_add_must_preserve_symbol(%s) because -r mode disable LTO dead stripping\n", name);
//			::thinlto_codegen_add_must_preserve_symbol(thingenerator, name, strlen(name));
//		}
	}

	return thingenerator;
}
#endif

// Full LTO processing
bool Parser::optimizeThinLTO(const std::vector<File*>&              files,
							 const std::vector<const ld::Atom*>&	allAtoms,
							 ld::Internal&							state,
							 const OptimizeOptions&					options,
							 ld::File::AtomHandler&					handler,
							 std::vector<const ld::Atom*>&			newAtoms,
							 std::vector<const char*>&				additionalUndefines) {
	const bool logBitcodeFiles = false;

	if (files.empty())
		return true;

#if LTO_API_VERSION >= 18

	if (::lto_api_version() < 18)
		throwf("lto: could not use -thinlto because libLTO is too old (version '%d', >=18 is required)", ::lto_api_version());

	// Handle -mllvm options
	if ( !_s_llvmOptionsProcessed ) {
		thinlto_debug_options(options.llvmOptions->data(), options.llvmOptions->size());
		_s_llvmOptionsProcessed = true;
	}

	// Create the ThinLTO codegenerator
	CStringToAtom                     deadllvmAtoms;
	CStringToAtom                     llvmAtoms;
	thinlto_code_gen_t thingenerator = init_thinlto_codegen(files, allAtoms, state, options, deadllvmAtoms, llvmAtoms);


	ld::File::Ordinal lastOrdinal;
	int FileId = 0;
	for (auto *f : files) {
		if ( logBitcodeFiles) fprintf(stderr, "thinlto_codegen_add_module(%s)\n", f->path());
		f->addToThinGenerator(thingenerator, FileId++);
		lastOrdinal = f->ordinal();
	}

#if LTO_API_VERSION >= 19
	// In the bitcode bundle case, we first run the generator with codegen disabled
	// and get the bitcode output. These files are added for later bundling, and a
	// new codegenerator is setup with these as input, and the optimizer disabled.
	if (options.bitcodeBundle) {
		// Bitcode Bundle case
		thinlto_codegen_disable_codegen(thingenerator, true);
		// Process the optimizer only
		thinlto_codegen_process(thingenerator);
		auto numObjects = thinlto_module_get_num_objects(thingenerator);
		// Save the codegenerator
		thinlto_code_gen_t bitcode_generator = thingenerator;
		// Create a new codegen generator for the codegen part.
		// Clear out the stored atoms so we can recompute them.
		deadllvmAtoms.clear();
		llvmAtoms.clear();
		thingenerator = init_thinlto_codegen(files, allAtoms, state, options, deadllvmAtoms, llvmAtoms);
		// Disable the optimizer
		thinlto_codegen_set_codegen_only(thingenerator, true);

		// Save bitcode files for later, and add them to the codegen generator.
		for (unsigned bufID = 0; bufID < numObjects; ++bufID) {
			auto machOFile = thinlto_module_get_object(bitcode_generator, bufID);
			std::string tempMachoPath = options.outputFilePath;
			tempMachoPath += ".";
			tempMachoPath += std::to_string(bufID);
			tempMachoPath += ".thinlto.o.bc";
			state.ltoBitcodePath.push_back(tempMachoPath);
			int fd = ::open(tempMachoPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
			if ( fd != -1 ) {
				::write(fd, machOFile.Buffer, machOFile.Size);
				::close(fd);
			} else {
				throwf("unable to write temporary ThinLTO output: %s", tempMachoPath.c_str());
			}

			// Add the optimized bitcode to the codegen generator now.
			::thinlto_codegen_add_module(thingenerator, strdup(tempMachoPath.c_str()), (const char *)machOFile.Buffer, machOFile.Size);
		}
	}

	if (options.ltoCodegenOnly)
		// Disable the optimizer
		thinlto_codegen_set_codegen_only(thingenerator, true);
#endif

	// If object_path_lto is used, we switch to a file-based API: libLTO will
	// generate the files on disk and we'll map them on-demand.

#if LTO_API_VERSION >= 21
	bool useFileBasedAPI = (options.tmpObjectFilePath && ::lto_api_version() >= 21);
	if ( useFileBasedAPI )
		thinlto_set_generated_objects_dir(thingenerator, options.tmpObjectFilePath);
#endif

	// run code generator
	thinlto_codegen_process(thingenerator);

	unsigned numObjects;
#if LTO_API_VERSION >= 21
	if ( useFileBasedAPI )
		numObjects = thinlto_module_get_num_object_files(thingenerator);
	else
#endif
		numObjects = thinlto_module_get_num_objects(thingenerator);
	if ( numObjects == 0 )
		throwf("could not do ThinLTO codegen (thinlto_codegen_process didn't produce any object): '%s', using libLTO version '%s'", ::lto_get_error_message(), ::lto_get_version());

	auto get_thinlto_buffer_or_load_file = [&] (unsigned ID) {
#if LTO_API_VERSION >= 21
		if ( useFileBasedAPI ) {
			const char* path = thinlto_module_get_object_file(thingenerator, ID);
			// map in whole file
			struct stat stat_buf;
			int fd = ::open(path, O_RDONLY, 0);
			if ( fd == -1 )
				throwf("can't open thinlto file '%s', errno=%d", path, errno);
			if ( ::fstat(fd, &stat_buf) != 0 )
				throwf("fstat thinlto file '%s' failed, errno=%d\n", path, errno);
			size_t len = stat_buf.st_size;
			if ( len < 20 )
				throwf("ThinLTO file '%s' too small (length=%zu)", path, len);
			const char* p = (const char*)::mmap(NULL, len, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
			if ( p == (const char*)(-1) )
				throwf("can't map file, errno=%d", errno);
			::close(fd);
			return LTOObjectBuffer{ p, len };
		}
#endif
		return thinlto_module_get_object(thingenerator, ID);
	};

	// if requested, save off objects files
	if ( options.saveTemps ) {
		for (unsigned bufID = 0; bufID < numObjects; ++bufID) {
			auto machOFile = get_thinlto_buffer_or_load_file(bufID);
			std::string tempMachoPath = options.outputFilePath;
			tempMachoPath += ".";
			tempMachoPath += std::to_string(bufID);
			tempMachoPath += ".thinlto.o";
			int fd = ::open(tempMachoPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
			if ( fd != -1 ) {
				::write(fd, machOFile.Buffer, machOFile.Size);
				::close(fd);
			}
			else {
				warning("unable to write temporary ThinLTO output: %s", tempMachoPath.c_str());
			}
		}
	}

	// mach-o parsing is done in-memory, but need path for debug notes
	std::string macho_dirpath = "/tmp/thinlto.o";
	if ( options.tmpObjectFilePath != NULL ) {
		macho_dirpath = options.tmpObjectFilePath;
		struct stat statBuffer;
		if( stat(macho_dirpath.c_str(), &statBuffer) != 0 || !S_ISDIR(statBuffer.st_mode) ) {
			unlink(macho_dirpath.c_str());
			if ( mkdir(macho_dirpath.c_str(), 0700) !=0 ) {
				warning("unable to create ThinLTO output directory for temporary object files: %s", macho_dirpath.c_str());
			}
		}
	}

	auto ordinal = ld::File::Ordinal::LTOOrdinal().nextFileListOrdinal();
	for (unsigned bufID = 0; bufID < numObjects; ++bufID) {
		auto machOFile = get_thinlto_buffer_or_load_file(bufID);
		if (!machOFile.Size) {
			warning("Ignoring empty buffer generated by ThinLTO");
			continue;
		}

		// mach-o parsing is done in-memory, but need path for debug notes
		std::string tmp_path;
#if LTO_API_VERSION >= 21
		if ( useFileBasedAPI ) {
			tmp_path = thinlto_module_get_object_file(thingenerator, bufID);
		}
		else
#endif
		if ( options.tmpObjectFilePath != NULL) {
			tmp_path = macho_dirpath + "/" + std::to_string(bufID) + ".o";
			// if needed, save temp mach-o file to specific location
			int fd = ::open(tmp_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
			if ( fd != -1) {
				::write(fd, (const uint8_t *)machOFile.Buffer, machOFile.Size);
				::close(fd);
			}
			else {
				warning("could not write ThinLTO temp file '%s', errno=%d", tmp_path.c_str(), errno);
			}
		}

		// parse generated mach-o file into a MachOReader
		ld::relocatable::File* machoFile = parseMachOFile((const uint8_t *)machOFile.Buffer, machOFile.Size, tmp_path, options, ordinal);
		ordinal = ordinal.nextFileListOrdinal();

		// Load the generated MachO file
		loadMachO(machoFile, options, handler, newAtoms, additionalUndefines, llvmAtoms, deadllvmAtoms);
	}

	// Remove Atoms from ld if code generator optimized them away
	for (CStringToAtom::iterator li = llvmAtoms.begin(), le = llvmAtoms.end(); li != le; ++li) {
		// check if setRealAtom() called on this Atom
		if ( li->second->compiledAtom() == NULL ) {
			//fprintf(stderr, "llvm optimized away %p %s\n", li->second, li->second->name());
			li->second->setCoalescedAway();
		}
	}

	return true;
#else // ! (LTO_API_VERSION >= 18)
	throwf("lto: could not use -thinlto because ld was built against a version of libLTO too old (version '%d', >=18 is required)", LTO_API_VERSION);
#endif
}

bool Parser::optimize(  const std::vector<const ld::Atom*>&	allAtoms,
						ld::Internal&						state,
						const OptimizeOptions&				options,
						ld::File::AtomHandler&				handler,
						std::vector<const ld::Atom*>&		newAtoms,
						std::vector<const char*>&			additionalUndefines)
{

	// exit quickly if nothing to do
	if ( _s_files.size() == 0 )
		return false;

	// print out LTO version string if -v was used
	if ( options.verbose )
		fprintf(stderr, "%s\n", ::lto_get_version());

	// <rdar://problem/12379604> The order that files are merged must match command line order
	std::sort(_s_files.begin(), _s_files.end(), CommandLineOrderFileSorter());

#if LTO_API_VERSION >= 19
	// If ltoCodegenOnly is set, we don't want to merge any bitcode files and perform FullLTO
	// we just take the ThinLTO path (optimization will be disabled anyway).
	if (options.ltoCodegenOnly) {
		for (auto *file : _s_files) {
			file->setIsThinLTO(true);
		}
	}
#endif

	std::vector<File *> theLTOFiles;
	std::vector<File *> theThinLTOFiles;
	for (auto *file : _s_files) {
		if (file->isThinLTO()) {
			theThinLTOFiles.push_back(file);
		} else {
			theLTOFiles.push_back(file);
		}
		state.filesForLTO.push_back(file);
	}

	auto result =  optimizeThinLTO(theThinLTOFiles, allAtoms, state, options, handler, newAtoms, additionalUndefines) &&
				   optimizeLTO(theLTOFiles, allAtoms, state, options, handler, newAtoms, additionalUndefines);

	// Remove InternalAtoms from ld
	for (std::vector<File*>::iterator it=_s_files.begin(); it != _s_files.end(); ++it) {
		(*it)->internalAtom().setCoalescedAway();
	}

	return result;
}


void Parser::AtomSyncer::doAtom(const ld::Atom& machoAtom)
{
	static const bool log = false;
	// update proxy atoms to point to real atoms and find new atoms
	const char* name = machoAtom.name();
	CStringToAtom::const_iterator pos = _llvmAtoms.find(name);
	if ( (pos != _llvmAtoms.end()) && (machoAtom.scope() != ld::Atom::scopeTranslationUnit) ) {
		// turn Atom into a proxy for this mach-o atom
		if (pos->second->scope() == ld::Atom::scopeLinkageUnit) {
			if (log) fprintf(stderr, "demote %s to hidden after LTO\n", name);
			(const_cast<ld::Atom*>(&machoAtom))->setScope(ld::Atom::scopeLinkageUnit);
		}
		// If both llvmAtom and machoAtom has the same scope and combine, but machoAtom loses auto hide, add it back.
		// rdar://problem/38646854
		if (pos->second->scope() == machoAtom.scope() &&
			pos->second->combine() == machoAtom.combine() &&
			pos->second->autoHide() && !machoAtom.autoHide()) {
			if (log) fprintf(stderr, "set %s to auto hide after LTO\n", name);
			(const_cast<ld::Atom*>(&machoAtom))->setAutoHide();
		}
		pos->second->setCompiledAtom(machoAtom);
		_lastProxiedAtom = &machoAtom;
		_lastProxiedFile = pos->second->file();
		if (log) fprintf(stderr, "AtomSyncer, mach-o atom %p synced to lto atom %p (name=%s)\n", &machoAtom, pos->second, machoAtom.name());
	}
	else {
		// an atom of this name was not in the allAtoms list the linker gave us
		auto llvmAtomPos = _deadllvmAtoms.find(name);
		lto::Atom* llvmAtom = nullptr;
		const lto::File* atomFile = nullptr;
		if ( llvmAtomPos != _deadllvmAtoms.end() ) {
			llvmAtom = llvmAtomPos->second;
			atomFile = (lto::File*)llvmAtom->file();
			if ( atomFile->isThinLTO() && (machoAtom.scope() == ld::Atom::scopeTranslationUnit) ) {
				// <rdar://problem/55067439> don't conflate static symbols in lto output with dead hidden/global symbols
				if (log) fprintf(stderr, "AtomSyncer, static mach-o atom %p not synced to dead lto name=%s\n", &machoAtom, name);
				llvmAtom = nullptr;
			}
		}
		if ( llvmAtom != nullptr ) {
			// this corresponding to an atom that the linker coalesced away or marked not-live
			if ( _options.linkerDeadStripping ) {
				// Do not lose auto-hide optimization if the atom is added back.
				if (llvmAtom->scope() == machoAtom.scope() &&
					llvmAtom->combine() == machoAtom.combine() &&
					llvmAtom->autoHide() && !machoAtom.autoHide()) {
					if (log) fprintf(stderr, "set %s to auto hide after LTO\n", name);
					(const_cast<ld::Atom*>(&machoAtom))->setAutoHide();
				}
				// llvm seems to want this atom and -dead_strip is enabled, so it will be deleted if not needed, so add back
				llvmAtom->setCompiledAtom(machoAtom);
				_newAtoms.push_back(&machoAtom);
				if (log) fprintf(stderr, "AtomSyncer, mach-o atom %p matches dead lto atom %p but adding back (name=%s)\n", &machoAtom, llvmAtom, machoAtom.name());
			}
			else {
				// Don't pass it back as a new atom
				if (log) fprintf(stderr, "AtomSyncer, mach-o atom %p matches dead lto atom %p (name=%s)\n", &machoAtom, llvmAtom, machoAtom.name());
				if ( llvmAtom->coalescedAway() ) {
					if (log) fprintf(stderr, "AtomSyncer: dead coalesced atom %s\n", machoAtom.name());
					// <rdar://problem/28269547>
					// We told libLTO to keep a weak atom that will replaced by an native mach-o atom.
					// We also need to remove any atoms directly dependent on this (FDE, LSDA).
					for (ld::Fixup::iterator fit=machoAtom.fixupsBegin(), fend=machoAtom.fixupsEnd(); fit != fend; ++fit) {
						switch ( fit->kind ) {
							case ld::Fixup::kindNoneGroupSubordinate:
							case ld::Fixup::kindNoneGroupSubordinateFDE:
							case ld::Fixup::kindNoneGroupSubordinateLSDA:
								assert(fit->binding == ld::Fixup::bindingDirectlyBound);
								(const_cast<ld::Atom*>(fit->u.target))->setCoalescedAway();
								if (log) fprintf(stderr, "AtomSyncer: mark coalesced-away subordinate atom %s\n", fit->u.target->name());
								break;
							default:
								break;
						}
					}
				}
			} 
		}
		else
		{
			// this is something new that lto conjured up, tell ld its new
			_newAtoms.push_back(&machoAtom);
			// <rdar://problem/15469363> if new static atom in same section as previous non-static atom, assign to same file as previous
			if ( (_lastProxiedAtom != NULL) && (_lastProxiedAtom->section() == machoAtom.section()) ) {
				ld::Atom* ma = const_cast<ld::Atom*>(&machoAtom);
				ma->setFile(_lastProxiedFile);
				if (log) fprintf(stderr, "AtomSyncer, mach-o atom %s is static and being assigned to %s\n", machoAtom.name(), _lastProxiedFile->path());
			}
			if (log) fprintf(stderr, "AtomSyncer, mach-o atom %p is totally new (name=%s)\n", &machoAtom, machoAtom.name());
		}
	}
	
	// adjust fixups to go through proxy atoms
	if (log) fprintf(stderr, "  adjusting fixups in atom: %s\n", machoAtom.name());
	for (ld::Fixup::iterator fit=machoAtom.fixupsBegin(); fit != machoAtom.fixupsEnd(); ++fit) {
		switch ( fit->binding ) {
			case ld::Fixup::bindingNone:
				break;
			case ld::Fixup::bindingByNameUnbound:
				// don't know if this target has been seen by linker before or if it is new
				// be conservative and tell linker it is new
				_additionalUndefines.push_back(fit->u.name);
				if (log) fprintf(stderr, "    adding by-name symbol %s\n", fit->u.name);
				break;
			case ld::Fixup::bindingDirectlyBound:
				// If mach-o atom is referencing another mach-o atom then 
				// reference is not going through Atom proxy. Fix it here to ensure that all
				// llvm symbol references always go through Atom proxy.
				if ( fit->u.target->scope() != ld::Atom::scopeTranslationUnit )
				{
					const char* targetName = fit->u.target->name();
					CStringToAtom::const_iterator post = _llvmAtoms.find(targetName);
					if ( post != _llvmAtoms.end() ) {
						const ld::Atom* t = post->second;
						if (log) fprintf(stderr, "    updating direct reference to %p to be ref to %p: %s\n", fit->u.target, t, targetName);
						fit->u.target = t;
					}
					else {
						// <rdar://problem/12859831> Don't unbind follow-on reference into by-name reference 
						if ( (_deadllvmAtoms.find(targetName) != _deadllvmAtoms.end()) && (fit->kind != ld::Fixup::kindNoneFollowOn) && (fit->u.target->scope() != ld::Atom::scopeTranslationUnit) ) {
							// target was coalesed away and replace by mach-o atom from a non llvm .o file
							fit->binding = ld::Fixup::bindingByNameUnbound;
							fit->u.name = targetName;
						}
					}
				}
				//fprintf(stderr, "    direct ref to: %s (scope=%d)\n", fit->u.target->name(), fit->u.target->scope());
				break;
			case ld::Fixup::bindingByContentBound:
				//fprintf(stderr, "    direct by content to: %s\n", fit->u.target->name());
				break;
			case ld::Fixup::bindingsIndirectlyBound:
				assert(0 && "indirect binding found in initial mach-o file?");
				//fprintf(stderr, "    indirect by content to: %u\n", fit->u.bindingIndex);
				break;
		}
	}

}

class Mutex {
	static pthread_mutex_t lto_lock;
public:
	Mutex() { pthread_mutex_lock(&lto_lock); }
	~Mutex() { pthread_mutex_unlock(&lto_lock); }
};
pthread_mutex_t Mutex::lto_lock = PTHREAD_MUTEX_INITIALIZER;
bool File::sSupportsLocalContext = false;
bool File::sHasTriedLocalContext = false;

//
// Used by archive reader to see if member is an llvm bitcode file
//
bool isObjectFile(const uint8_t* fileContent, uint64_t fileLength, cpu_type_t architecture, cpu_subtype_t subarch)
{
	Mutex lock;
	return Parser::validFile(fileContent, fileLength, architecture, subarch);
}

//
// Used by archive reader to see if member defines a Category (for -ObjC semantics)
//
bool hasObjCCategory(const uint8_t* fileContent, uint64_t fileLength)
{
#if LTO_API_VERSION >= 20
	// note: if run with older libLTO.dylib that does not implement
	// lto_module_has_objc_category, the call will return 0 which is "false"
	return lto_module_has_objc_category(fileContent, fileLength);
#else
	return false;
#endif
}


static ld::relocatable::File *parseImpl(
          const uint8_t *fileContent, uint64_t fileLength, const char *path,
          time_t modTime, ld::File::Ordinal ordinal, cpu_type_t architecture,
          cpu_subtype_t subarch, bool logAllFiles,
          bool verboseOptimizationHints)
{
	if ( Parser::validFile(fileContent, fileLength, architecture, subarch) )
		return Parser::parse(fileContent, fileLength, path, modTime, ordinal, architecture, subarch, logAllFiles, verboseOptimizationHints);
	else
		return NULL;
}

//
// main function used by linker to instantiate ld::Files
//
ld::relocatable::File* parse(const uint8_t* fileContent, uint64_t fileLength, 
								const char* path, time_t modTime, ld::File::Ordinal ordinal,
								cpu_type_t architecture, cpu_subtype_t subarch, bool logAllFiles,
								bool verboseOptimizationHints)
{
	// do light weight check before acquiring lock
	if ( fileLength < 4 )
		return NULL;
	if ( (fileContent[0] != 0xDE) || (fileContent[1] != 0xC0) || (fileContent[2] != 0x17) || (fileContent[3] != 0x0B) )
		return NULL;

	// Note: Once lto_module_create_in_local_context() and friends are thread safe
	// this lock can be removed.
	Mutex lock;
	return parseImpl(fileContent, fileLength, path, modTime, ordinal,
					architecture, subarch, logAllFiles,
					verboseOptimizationHints);
}

//
// used by "ld -v" to report version of libLTO.dylib being used
//
const char* version()
{
	Mutex lock;
	return ::lto_get_version();
}

//
// used by "ld -v" to report static version of libLTO.dylib API being compiled
//
unsigned int static_api_version()
{
	return LTO_API_VERSION;
}

//
// used by "ld -v" to report version of libLTO.dylib being used
//
unsigned int runtime_api_version()
{
	return ::lto_api_version();
}


//
// used by ld for error reporting
//
bool libLTOisLoaded()
{
	Mutex lock;
	return (::lto_get_version() != NULL);
}

//
// used by ld for error reporting
//
const char* archName(const uint8_t* fileContent, uint64_t fileLength)
{
	return Parser::fileKind(fileContent, fileLength);
}

//
// used by ld for doing link time optimization
//
bool optimize(  const std::vector<const ld::Atom*>&	allAtoms,
				ld::Internal&						state,
				const OptimizeOptions&				options,
				ld::File::AtomHandler&				handler,
				std::vector<const ld::Atom*>&		newAtoms, 
				std::vector<const char*>&			additionalUndefines)
{ 
	Mutex lock;
	return Parser::optimize(allAtoms, state, options, handler, newAtoms, additionalUndefines);
}



}; // namespace lto

static const char *sLTODylib = "@rpath/libLTO.dylib";
static std::atomic<bool> sLTOIsLoaded(false);

static void *getHandle() {
  auto impl = [&]() -> void* {
    void *handle = ::dlopen(sLTODylib, RTLD_LAZY);
    if (handle)
      sLTOIsLoaded.store(true);
    return handle;
  };

  // Memoize.
  static void *handle = impl();
  return handle;
}

namespace lto {
void set_library(const char *dylib) {
  assert(!sLTOIsLoaded);
  sLTODylib = dylib;
}
} // end namespace lto

namespace {
template <class T> struct LTOSymbol;
template <class R, class... Args> struct LTOSymbol<R (Args...)> {
  R (*call)(Args...) = nullptr;

public:
  LTOSymbol() = delete;
  LTOSymbol(const char *name) {
    if (void *handle = getHandle())
      call = reinterpret_cast<R (*)(Args...)>(::dlsym(handle, name));

    if (!call)
      call = [](Args...) -> R { return R{}; };
  }
};
template <class... Args> struct LTOSymbol<void (Args...)> {
  void (*call)(Args...) = nullptr;

public:
  LTOSymbol() = delete;
  LTOSymbol(const char *name) {
    if (void *handle = getHandle())
      call = reinterpret_cast<void (*)(Args...)>(::dlsym(handle, name));

    if (!call)
      call = [](Args...) -> void {};
  }
};
} // end namespace

extern "C" {

#define WRAP_LTO_SYMBOL(RET, NAME, PARAMS, ARGS)                               \
  RET NAME PARAMS {                                                            \
    static LTOSymbol<RET PARAMS> x(#NAME);                                     \
    return (x.call)ARGS;                                                       \
  }

WRAP_LTO_SYMBOL(const char *, lto_get_version, (), ())
WRAP_LTO_SYMBOL(lto_bool_t, lto_module_has_objc_category,
                (const void *mem, size_t length), (mem, length))
WRAP_LTO_SYMBOL(lto_module_t, lto_module_create_from_memory_with_path,
                (const void *mem, size_t length, const char *path),
                (mem, length, path))
WRAP_LTO_SYMBOL(lto_module_t, lto_module_create_in_local_context,
                (const void *mem, size_t length, const char *path),
                (mem, length, path))
WRAP_LTO_SYMBOL(lto_module_t, lto_module_create_from_memory,
                (const void *mem, size_t length), (mem, length))
WRAP_LTO_SYMBOL(void, lto_module_dispose, (lto_module_t mod), (mod))
WRAP_LTO_SYMBOL(unsigned int, lto_module_get_num_symbols, (lto_module_t mod),
                (mod))
WRAP_LTO_SYMBOL(const char *, lto_module_get_symbol_name,
                (lto_module_t mod, unsigned int index), (mod, index))
WRAP_LTO_SYMBOL(lto_symbol_attributes, lto_module_get_symbol_attribute,
                (lto_module_t mod, unsigned int index), (mod, index))
WRAP_LTO_SYMBOL(lto_bool_t, lto_codegen_add_module,
                (lto_code_gen_t cg, lto_module_t mod), (cg, mod))
WRAP_LTO_SYMBOL(lto_bool_t, lto_module_is_thinlto, (lto_module_t mod), (mod))

WRAP_LTO_SYMBOL(void, lto_codegen_set_module,
                (lto_code_gen_t cg, lto_module_t mod), (cg, mod))
WRAP_LTO_SYMBOL(void, thinlto_codegen_add_module,
                (thinlto_code_gen_t cg, const char *identifier,
                 const char *data, int length),
                (cg, identifier, data, length))

WRAP_LTO_SYMBOL(void, thinlto_codegen_add_must_preserve_symbol,
                (thinlto_code_gen_t cg, const char *name, int length),
                (cg, name, length))
WRAP_LTO_SYMBOL(void, thinlto_codegen_add_cross_referenced_symbol,
                (thinlto_code_gen_t cg, const char *name, int length),
                (cg, name, length))
WRAP_LTO_SYMBOL(void, thinlto_codegen_set_cache_dir,
                (thinlto_code_gen_t cg, const char *cache_dir), (cg, cache_dir))
WRAP_LTO_SYMBOL(void, thinlto_codegen_set_cache_pruning_interval,
                (thinlto_code_gen_t cg, int interval), (cg, interval))
WRAP_LTO_SYMBOL(
    void, thinlto_codegen_set_final_cache_size_relative_to_available_space,
    (thinlto_code_gen_t cg, unsigned percentage), (cg, percentage))
WRAP_LTO_SYMBOL(void, thinlto_codegen_set_cache_entry_expiration,
                (thinlto_code_gen_t cg, unsigned expiration), (cg, expiration))

WRAP_LTO_SYMBOL(void, thinlto_codegen_process, (thinlto_code_gen_t cg), (cg))
WRAP_LTO_SYMBOL(unsigned int, thinlto_module_get_num_objects,
                (thinlto_code_gen_t cg), (cg))
WRAP_LTO_SYMBOL(LTOObjectBuffer, thinlto_module_get_object,
                (thinlto_code_gen_t cg, unsigned int index), (cg, index))
WRAP_LTO_SYMBOL(unsigned int, thinlto_module_get_num_object_files,
                (thinlto_code_gen_t cg), (cg))
WRAP_LTO_SYMBOL(const char *, thinlto_module_get_object_file,
                (thinlto_code_gen_t cg, unsigned int index), (cg, index))
WRAP_LTO_SYMBOL(lto_bool_t, thinlto_codegen_set_pic_model,
                (thinlto_code_gen_t cg, lto_codegen_model m), (cg, m))
WRAP_LTO_SYMBOL(void, thinlto_codegen_set_savetemps_dir,
                (thinlto_code_gen_t cg, const char *save_temps_dir),
                (cg, save_temps_dir))
WRAP_LTO_SYMBOL(void, thinlto_set_generated_objects_dir,
                (thinlto_code_gen_t cg, const char *save_temps_dir),
                (cg, save_temps_dir))
WRAP_LTO_SYMBOL(void, thinlto_codegen_set_cpu,
                (thinlto_code_gen_t cg, const char *cpu), (cg, cpu))
WRAP_LTO_SYMBOL(void, thinlto_codegen_disable_codegen,
                (thinlto_code_gen_t cg, lto_bool_t disable), (cg, disable))
WRAP_LTO_SYMBOL(void, thinlto_codegen_set_codegen_only,
                (thinlto_code_gen_t cg, lto_bool_t codegen_only),
                (cg, codegen_only))
WRAP_LTO_SYMBOL(void, thinlto_debug_options,
                (const char *const *options, int number), (options, number))

WRAP_LTO_SYMBOL(lto_code_gen_t, lto_codegen_create_in_local_context, (void), ())
WRAP_LTO_SYMBOL(void, lto_codegen_set_diagnostic_handler,
                (lto_code_gen_t cg, lto_diagnostic_handler_t handler,
                 void *context),
                (cg, handler, context))
WRAP_LTO_SYMBOL(lto_code_gen_t, lto_codegen_create, (void), ())
WRAP_LTO_SYMBOL(void, lto_codegen_debug_options,
                (lto_code_gen_t cg, const char *options), (cg, options))
#if LTO_API_VERSION >= 26
WRAP_LTO_SYMBOL(void, lto_codegen_debug_options_array,
                (lto_code_gen_t cg, const char *const *options, int number),
                (cg, options, number))
#endif
WRAP_LTO_SYMBOL(void, lto_codegen_dispose, (lto_code_gen_t cg), (cg))
WRAP_LTO_SYMBOL(void, lto_codegen_set_assembler_path,
                (lto_code_gen_t cg, const char *path), (cg, path))
WRAP_LTO_SYMBOL(thinlto_code_gen_t, thinlto_create_codegen, (void), ())
WRAP_LTO_SYMBOL(unsigned int, lto_api_version, (void), ())
WRAP_LTO_SYMBOL(const void *, lto_codegen_compile_optimized,
                (lto_code_gen_t cg, size_t *length), (cg, length))
WRAP_LTO_SYMBOL(lto_bool_t, lto_module_is_object_file_in_memory_for_target,
                (const void *mem, size_t length,
                 const char *target_triple_prefix),
                (mem, length, target_triple_prefix))
WRAP_LTO_SYMBOL(void, lto_codegen_set_should_embed_uselists,
                (lto_code_gen_t cg, lto_bool_t ShouldEmbedUselists),
                (cg, ShouldEmbedUselists))
WRAP_LTO_SYMBOL(const void *, lto_codegen_compile,
                (lto_code_gen_t cg, size_t *length), (cg, length))
WRAP_LTO_SYMBOL(const char *, lto_get_error_message, (void), ())
WRAP_LTO_SYMBOL(lto_module_t, lto_module_create_in_codegen_context,
                (const void *mem, size_t length, const char *path,
                 lto_code_gen_t cg),
                (mem, length, path, cg))
WRAP_LTO_SYMBOL(lto_bool_t, lto_codegen_optimize, (lto_code_gen_t cg), (cg))
WRAP_LTO_SYMBOL(lto_bool_t, lto_codegen_write_merged_modules,
                (lto_code_gen_t cg, const char *path), (cg, path))
WRAP_LTO_SYMBOL(void, lto_codegen_set_should_internalize,
                (lto_code_gen_t cg, lto_bool_t ShouldInternalize),
                (cg, ShouldInternalize))
WRAP_LTO_SYMBOL(void, lto_codegen_set_cpu, (lto_code_gen_t cg, const char *cpu),
                (cg, cpu))
WRAP_LTO_SYMBOL(lto_bool_t, lto_codegen_set_pic_model,
                (lto_code_gen_t cg, lto_codegen_model model), (cg, model))
WRAP_LTO_SYMBOL(void, lto_codegen_add_must_preserve_symbol,
                (lto_code_gen_t cg, const char *symbol), (cg, symbol))

#undef WRAP_LTO_SYMBOL

} // end extern "C"

#endif

