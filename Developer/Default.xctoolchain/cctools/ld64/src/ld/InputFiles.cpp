
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
 

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#ifdef __linux__
namespace {
	typedef long double max_align_t;
}
#include <string.h>
#define MAXPATHLEN 1024
#define MIN(a,b) (a < b ? a : b)
//#define OSAtomicAdd64(i, v) __atomic_fetch_add_8((volatile int *)v, i)
//#define OSAtomicIncrement32(v) __atomic_fetch_add_4((volatile int *)v, 1)
#else
#include <sys/sysctl.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <mach/mach_time.h>
#include <mach/vm_statistics.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#include <libkern/OSAtomic.h>

//#include <string>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <algorithm>
#include <dlfcn.h>
#include <AvailabilityMacros.h>

#include "Options.h"

#include "InputFiles.h"
#include "macho_relocatable_file.h"
#include "macho_dylib_file.h"
#include "textstub_dylib_file.hpp"
#include "archive_file.h"
#include "lto_file.h"
#include "opaque_section_file.h"
#include "MachOFileAbstraction.hpp"
#include "Snapshot.h"

const bool _s_logPThreads = false;

namespace ld {
namespace tool {

class IgnoredFile : public ld::File {
public:
	IgnoredFile(const char* pth, time_t modTime, Ordinal ord, Type type) : ld::File(pth, modTime, ord, type) {};
	virtual bool						forEachAtom(AtomHandler&) const { return false; };
	virtual bool						justInTimeforEachAtom(const char* name, AtomHandler&) const { return false; };
};


class DSOHandleAtom : public ld::Atom {
public:
									DSOHandleAtom(const char* nm, ld::Atom::Scope sc, 
														ld::Atom::SymbolTableInclusion inc, ld::Section& sect=_s_section)
										: ld::Atom(sect, ld::Atom::definitionRegular,
												   (sect == _s_section_text) ? ld::Atom::combineByName : ld::Atom::combineNever, 
												   // make "weak def" so that link succeeds even if app defines __dso_handle
													sc, ld::Atom::typeUnclassified, inc, true, false, false, 
													 ld::Atom::Alignment(1)), _name(nm) {}

	virtual ld::File*						file() const					{ return NULL; }
  virtual const char*						name() const					{ return _name; }
	virtual uint64_t						size() const					{ return 0; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const
																			{ }
	virtual void							setScope(Scope)					{ }

	virtual									~DSOHandleAtom() {}
	
	static ld::Section						_s_section;
	static ld::Section						_s_section_preload;
	static ld::Section						_s_section_text;
	static DSOHandleAtom					_s_atomAll;
	static DSOHandleAtom					_s_atomExecutable;
	static DSOHandleAtom					_s_atomDylib;
	static DSOHandleAtom					_s_atomBundle;
	static DSOHandleAtom					_s_atomDyld;
	static DSOHandleAtom					_s_atomObjectFile;
	static DSOHandleAtom					_s_atomPreload;
	static DSOHandleAtom					_s_atomPreloadDSO;
private:
	const char*								_name;
};
ld::Section DSOHandleAtom::_s_section("__TEXT", "__mach_header", ld::Section::typeMachHeader, true);
ld::Section DSOHandleAtom::_s_section_preload("__HEADER", "__mach_header", ld::Section::typeMachHeader, true);
ld::Section DSOHandleAtom::_s_section_text("__TEXT", "__text", ld::Section::typeCode, false);
DSOHandleAtom DSOHandleAtom::_s_atomAll("___dso_handle", ld::Atom::scopeLinkageUnit, ld::Atom::symbolTableNotIn);
DSOHandleAtom DSOHandleAtom::_s_atomExecutable("__mh_execute_header", ld::Atom::scopeGlobal, ld::Atom::symbolTableInAndNeverStrip);
DSOHandleAtom DSOHandleAtom::_s_atomDylib("__mh_dylib_header", ld::Atom::scopeLinkageUnit, ld::Atom::symbolTableNotIn);
DSOHandleAtom DSOHandleAtom::_s_atomBundle("__mh_bundle_header", ld::Atom::scopeLinkageUnit, ld::Atom::symbolTableNotIn);
DSOHandleAtom DSOHandleAtom::_s_atomDyld("__mh_dylinker_header", ld::Atom::scopeLinkageUnit, ld::Atom::symbolTableNotIn);
DSOHandleAtom DSOHandleAtom::_s_atomObjectFile("__mh_object_header", ld::Atom::scopeLinkageUnit, ld::Atom::symbolTableNotIn);
DSOHandleAtom DSOHandleAtom::_s_atomPreload("__mh_preload_header", ld::Atom::scopeLinkageUnit, ld::Atom::symbolTableNotIn, _s_section_preload);
DSOHandleAtom DSOHandleAtom::_s_atomPreloadDSO("___dso_handle", ld::Atom::scopeLinkageUnit, ld::Atom::symbolTableNotIn, _s_section_text);



class PageZeroAtom : public ld::Atom {
public:
									PageZeroAtom(uint64_t sz)
										: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
											ld::Atom::scopeTranslationUnit, ld::Atom::typeZeroFill, 
											symbolTableNotIn, true, false, false, ld::Atom::Alignment(12)),
											_size(sz) {}

	virtual ld::File*						file() const					{ return NULL; }
	virtual const char*						name() const					{ return "page zero"; }
	virtual uint64_t						size() const					{ return _size; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const 
																			{ }
	virtual void							setScope(Scope)					{ }

	virtual									~PageZeroAtom() {}
	
	static ld::Section						_s_section;
	static DSOHandleAtom					_s_atomAll;
private:
	uint64_t								_size;
};
ld::Section PageZeroAtom::_s_section("__PAGEZERO", "__pagezero", ld::Section::typePageZero, true);


class CustomStackAtom : public ld::Atom {
public:
									CustomStackAtom(uint64_t sz)
										: ld::Atom(_s_section, ld::Atom::definitionRegular, ld::Atom::combineNever,
											ld::Atom::scopeTranslationUnit, ld::Atom::typeZeroFill, 
											symbolTableNotIn, false, false, false, ld::Atom::Alignment(12)),
											_size(sz) {}

	virtual ld::File*						file() const					{ return NULL; }
	virtual const char*						name() const					{ return "custom stack"; }
	virtual uint64_t						size() const					{ return _size; }
	virtual uint64_t						objectAddress() const			{ return 0; }
	virtual void							copyRawContent(uint8_t buffer[]) const 
																			{ }
	virtual void							setScope(Scope)					{ }

	virtual									~CustomStackAtom() {}
	
private:
	uint64_t								_size;
	static ld::Section						_s_section;
};
ld::Section CustomStackAtom::_s_section("__UNIXSTACK", "__stack", ld::Section::typeStack, true);



const char* InputFiles::extractFileInfo(const uint8_t* p, unsigned len, const char* path, ld::Platform& platform)
{
	platform = ld::Platform::unknown;

	const char* result = mach_o::relocatable::archName(p);
	if ( result != NULL ) {
		cpu_type_t    	type;
		cpu_subtype_t 	subtype;
		ld::VersionSet	platformsFound;
		mach_o::relocatable::isObjectFile(p, len, &type, &subtype, platformsFound);
		return result;
	}

    result = mach_o::dylib::archName(p);
    if ( result != NULL ) {
		cpu_type_t    type;
		cpu_subtype_t subtype;
		uint32_t      ignore;
		mach_o::dylib::isDylibFile(p, len, &type, &subtype, &platform, &ignore);
		return result;
	}

	result = lto::archName(p, len);
	if ( result != NULL  )
		 return result;

	const char* archiveArchName;
	if ( ::archive::isArchiveFile(p, len, &platform, &archiveArchName) )
		return archiveArchName;

	if ( textstub::dylib::isTextStubFile(p, len, path) )
		return "text-stub";

	char *unsupported = (char *)malloc(128);
	strcpy(unsupported, "unsupported file format (");
	for (unsigned i=0; i<len && i < 16; i++) {
		char buf[8];
		sprintf(buf, " 0x%02X", p[i]);
		strcat(unsupported, buf);
	}
	strcat(unsupported, " )");
	return unsupported;
}


ld::File* InputFiles::makeFile(const Options::FileInfo& info, bool indirectDylib)
{
	bool fromSDK = _options.fromSDK(info.path);
	// handle inlined framework first.
	if (info.isInlined) {
		auto interface = _options.findTAPIFile(info.path);
		if (!interface)
			throwf("could not find inlined dylib file: %s", info.path);
		auto file = textstub::dylib::parse(info.path, interface, info.modTime, info.ordinal, _options, indirectDylib, fromSDK);
		if (!file)
			throwf("could not parse inlined dylib file: %s(%s)", interface->getInstallName().c_str(), info.path);
		return file;
	}
	// map in whole file
	struct stat stat_buf;
	int fd = ::open(info.path, O_RDONLY, 0);
	if ( fd == -1 )
		throwf("can't open file, errno=%d", errno);
	if ( ::fstat(fd, &stat_buf) != 0 )
		throwf("fstat(%s) failed, errno=%d\n", info.path, errno);
	if ( stat_buf.st_size < 20 )
		throwf("file too small (length=%llu)", stat_buf.st_size);
	int64_t len = stat_buf.st_size;
	uint8_t* p = (uint8_t*)::mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	if ( p == (uint8_t*)(-1) )
		throwf("can't map file, errno=%d", errno);

	// if fat file, skip to architecture we want
	// Note: fat header is always big-endian
	bool isFatFile = false;
	uint32_t sliceToUse, sliceCount;
	const fat_header* fh = (fat_header*)p;
	if ( fh->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
		isFatFile = true;
		const struct fat_arch* archs = (struct fat_arch*)(p + sizeof(struct fat_header));
		bool sliceFound = false;
		sliceCount = OSSwapBigToHostInt32(fh->nfat_arch);
		// first try to find a slice that match cpu-type and cpu-sub-type
		for (uint32_t i=0; i < sliceCount; ++i) {
			if ( (OSSwapBigToHostInt32(archs[i].cputype) == (uint32_t)_options.architecture())
			  && ((OSSwapBigToHostInt32(archs[i].cpusubtype) & ~CPU_SUBTYPE_MASK) == (uint32_t)_options.subArchitecture()) ) {
				sliceToUse = i;
				sliceFound = true;
				break;
			}
		}
		if ( !sliceFound && _options.allowSubArchitectureMismatches() ) {
			// look for any slice that matches just cpu-type
			for (uint32_t i=0; i < sliceCount; ++i) {
				if ( OSSwapBigToHostInt32(archs[i].cputype) == (uint32_t)_options.architecture() ) {
					sliceToUse = i;
					sliceFound = true;
					break;
				}
			}
		}
		if ( !sliceFound ) {
			// Look for a fallback slice.
			for (uint32_t i = 0; i < sliceCount; ++i) {
				if ( OSSwapBigToHostInt32(archs[i].cputype) == (uint32_t)_options.fallbackArchitecture() &&
					(OSSwapBigToHostInt32(archs[i].cpusubtype) & ~CPU_SUBTYPE_MASK) == (uint32_t)_options.fallbackSubArchitecture() ) {
					sliceToUse = i;
					sliceFound = true;
					break;
				}
			}
		}
		if ( !sliceFound && (_options.architecture() == CPU_TYPE_ARM64) && (_options.subArchitecture() == CPU_SUBTYPE_ARM64_ALL) ) {
			// <rdar://problem/65681348> let arm64 executables link against arm64e slice of dylibs
			for (uint32_t i = 0; i < sliceCount; ++i) {
				if ( (OSSwapBigToHostInt32(archs[i].cputype) == CPU_TYPE_ARM64) &&
					(OSSwapBigToHostInt32(archs[i].cpusubtype) & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_ARM64E) {
					sliceToUse = i;
					sliceFound = true;
					break;
				}
			}
		}
		if ( sliceFound ) {
			uint32_t fileOffset = OSSwapBigToHostInt32(archs[sliceToUse].offset);
			len = OSSwapBigToHostInt32(archs[sliceToUse].size);
			if ( fileOffset+len > stat_buf.st_size ) {
				// <rdar://problem/17593430> file size was read awhile ago.  If file is being written, wait a second to see if big enough now
				sleep(1);
				int64_t newFileLen = stat_buf.st_size;
				struct stat statBuffer;
				if ( stat(info.path, &statBuffer) == 0 ) {
					newFileLen = statBuffer.st_size;
				}
				if ( fileOffset+len > newFileLen ) {
					throwf("truncated fat file. Slice from %u to %llu is past end of file with length %llu", 
						fileOffset, fileOffset+len, stat_buf.st_size);
				}
			}
			// if requested architecture is page aligned within fat file, then remap just that portion of file
			if ( (fileOffset & PAGE_MASK) == 0 ) {
				// unmap whole file
				munmap((caddr_t)p, stat_buf.st_size);
				// re-map just part we need
				p = (uint8_t*)::mmap(NULL, len, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, fileOffset);
				if ( p == (uint8_t*)(-1) )
					throwf("can't re-map file, errno=%d", errno);
			}
			else {
				p = &p[fileOffset];
			}
		}
	}
	::close(fd);

	// see if it is an object file
	mach_o::relocatable::ParserOptions objOpts;
	objOpts.architecture		= _options.architecture();
	objOpts.objSubtypeMustMatch = !_options.allowSubArchitectureMismatches();
	objOpts.logAllFiles			= _options.logAllFiles();
	objOpts.warnUnwindConversionProblems	= _options.needsUnwindInfoSection();
	objOpts.keepDwarfUnwind		= _options.keepDwarfUnwind();
	objOpts.forceDwarfConversion= false;
	objOpts.neverConvertDwarf   = !_options.needsUnwindInfoSection();
	objOpts.verboseOptimizationHints = _options.verboseOptimizationHints();
	objOpts.armUsesZeroCostExceptions = _options.armUsesZeroCostExceptions();
#if SUPPORT_ARCH_arm64e
	objOpts.supportsAuthenticatedPointers = _options.supportsAuthenticatedPointers();
#endif
	objOpts.subType				= _options.subArchitecture();
	objOpts.platforms			= _options.platforms();
	objOpts.srcKind				= ld::relocatable::File::kSourceObj;
	objOpts.treateBitcodeAsData	= _options.bitcodeKind() == Options::kBitcodeAsData;
	objOpts.usingBitcode		= _options.bundleBitcode();
	objOpts.maxDefaultCommonAlignment = _options.maxDefaultCommonAlign();
	objOpts.internalSDK 		= _options.internalSDK();
	objOpts.forceHidden			= false;
	objOpts.platformMismatchesAreWarning = _options.platformMismatchesAreWarning();

	ld::relocatable::File* objResult = mach_o::relocatable::parse(p, len, info.path, info.modTime, info.ordinal, objOpts);
	if ( objResult != NULL ) {
		OSAtomicAdd64(len, &_totalObjectSize);
		OSAtomicIncrement32(&_totalObjectLoaded);
		return objResult;
	}

	// see if it is an llvm object file
	objResult = lto::parse(p, len, info.path, info.modTime, info.ordinal, _options.architecture(), _options.subArchitecture(), _options.logAllFiles(), _options.verboseOptimizationHints());
	if ( objResult != NULL ) {
		OSAtomicAdd64(len, &_totalObjectSize);
		OSAtomicIncrement32(&_totalObjectLoaded);
		return objResult;
	}

	// see if it is a dynamic library (or text-based dynamic library)
	ld::dylib::File* dylibResult;
	bool dylibsNotAllowed = false;
	switch ( _options.outputKind() ) {
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:	
			dylibResult = mach_o::dylib::parse(p, len, info.path, info.modTime, _options, info.ordinal, info.options.fBundleLoader, indirectDylib, fromSDK);
			if ( dylibResult != NULL ) {
				return dylibResult;
			}
			dylibResult = textstub::dylib::parse(p, len, info.path, info.modTime, _options, info.ordinal, info.options.fBundleLoader, indirectDylib, fromSDK);
			if ( dylibResult != NULL ) {
				return dylibResult;
			}
			break;
		case Options::kStaticExecutable:
		case Options::kDyld:
		case Options::kPreload:
		case Options::kObjectFile:
		case Options::kKextBundle:
			dylibsNotAllowed = true;
			break;
	}

	// see if it is a static library
	::archive::ParserOptions archOpts;
	archOpts.objOpts				= objOpts;
	archOpts.objOpts.forceHidden	= info.options.fLoadHidden;
	archOpts.forceLoadThisArchive	= info.options.fForceLoad;
	archOpts.forceLoadAll			= _options.fullyLoadArchives();
	archOpts.forceLoadObjC			= _options.loadAllObjcObjectsFromArchives();
	archOpts.objcABI2				= _options.objCABIVersion2POverride();
	archOpts.verboseLoad			= _options.whyLoad();
	archOpts.logAllFiles			= _options.logAllFiles();
	// Set ObjSource Kind, libclang_rt is compiler static library
	if ( isCompilerSupportLib(info.path) )
		archOpts.objOpts.srcKind = ld::relocatable::File::kSourceCompilerArchive;
	else
		archOpts.objOpts.srcKind = ld::relocatable::File::kSourceArchive;
	archOpts.objOpts.treateBitcodeAsData = _options.bitcodeKind() == Options::kBitcodeAsData;
	archOpts.objOpts.usingBitcode = _options.bundleBitcode();

	ld::archive::File* archiveResult = ::archive::parse(p, len, info.path, info.modTime, info.ordinal, archOpts);
	if ( archiveResult != NULL ) {
	
		OSAtomicAdd64(len, &_totalArchiveSize);
		OSAtomicIncrement32(&_totalArchivesLoaded);
		return archiveResult;
	}
	
	// does not seem to be any valid linker input file, check LTO misconfiguration problems
	if ( lto::archName((uint8_t*)p, len) != NULL ) {
		if ( lto::libLTOisLoaded() ) {
			ld::Platform filePlatform;
			const char* fileArchName = extractFileInfo(p, len, info.path, filePlatform);
			throwf("lto file was built for %s which is not the architecture being linked (%s): %s", fileArchName, _options.architectureName(), info.path);
		}
		else {
			const char* libLTO = "libLTO.dylib";
			char ldPath[PATH_MAX];
			char tmpPath[PATH_MAX];
			char libLTOPath[PATH_MAX];
			uint32_t bufSize = PATH_MAX;
			if ( _options.overridePathlibLTO() != NULL ) {
				libLTO = _options.overridePathlibLTO();
			}
			else if ( _NSGetExecutablePath(ldPath, &bufSize) != -1 ) {
				if ( realpath(ldPath, tmpPath) != NULL ) {
					char* lastSlash = strrchr(tmpPath, '/');
					if ( lastSlash != NULL )
						strcpy(lastSlash, "/../lib/libLTO.dylib");
					libLTO = tmpPath;
					if ( realpath(tmpPath, libLTOPath) != NULL ) 
						libLTO = libLTOPath;
				}
			}
			throwf("could not process llvm bitcode object file, because %s could not be loaded", libLTO);
		}
	}

	if ( dylibsNotAllowed ) {
		cpu_type_t dummy1;
		cpu_type_t dummy2;
		ld::Platform ignorePlatform;
		uint32_t ignoreOSVers;
		if ( mach_o::dylib::isDylibFile(p, len, &dummy1, &dummy2, &ignorePlatform, &ignoreOSVers) )
			throw "ignoring unexpected dylib file";
		if ( textstub::dylib::isTextStubFile(p, len, info.path) )
			throw "ignoring unexpected dylib text stub file";
	}

	// error handling
	if ( ((fat_header*)p)->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
		throwf("missing required architecture %s in file %s (%u slices)", _options.architectureName(), info.path, sliceCount);
	}
	else {
		if ( isFatFile )
			throwf("file is universal (%u slices) but does not contain the %s architecture: %s", sliceCount, _options.architectureName(), info.path);
		else {
			ld::Platform filePlatform;
			const char* fileArchName = extractFileInfo(p, len, info.path, filePlatform);
			throwf("building for %s-%s but attempting to link with file built for %s-%s",
					_options.platforms().to_str().c_str(), _options.architectureName(),
					ld::nameFromPlatform(filePlatform), fileArchName);
		}
	}
}

void InputFiles::logDylib(ld::File* file, bool indirect, bool speculative)
{
	if ( _options.traceDylibs() ) {
		const char* fullPath = file->path();
		char realName[MAXPATHLEN];
		if ( realpath(fullPath, realName) != NULL )
			fullPath = realName;
		const ld::dylib::File* dylib = dynamic_cast<const ld::dylib::File*>(file);
		if ( (dylib != NULL ) && dylib->willBeUpwardDylib() ) {
			// don't log upward dylibs when XBS is computing dependencies
			logTraceInfo("[Logging for XBS] Used upward dynamic library: %s\n", fullPath);
		}
		else if ( (dylib != NULL ) && dylib->speculativelyLoaded() ) {
			logTraceInfo("[Logging for XBS] Speculatively loaded dynamic library: %s\n", fullPath);
		}
		else {
			if ( indirect ) {
				if ( speculative )
					logTraceInfo("[Logging for XBS] Speculatively loaded indirect dynamic library: %s\n", fullPath);
				else
					logTraceInfo("[Logging for XBS] Used indirect dynamic library: %s\n", fullPath);
			}
			else {
				logTraceInfo("[Logging for XBS] Used dynamic library: %s\n", fullPath);
			}
		}
	}
	
	if ( _options.dumpDependencyInfo() ) {
		const ld::dylib::File* dylib = dynamic_cast<const ld::dylib::File*>(file);
		if ( file == _bundleLoader ) {
			_options.addDependency(Options::depBundleLoader, file->path());
		}
		else if ( (dylib != NULL ) && dylib->willBeUpwardDylib() ) {
			if ( indirect ) 
				_options.addDependency(Options::depUpwardIndirectDylib, file->path());
			else 
				_options.addDependency(Options::depUpwardDirectDylib, file->path());
		}
		else {
			if ( indirect ) 
				_options.addDependency(Options::depIndirectDylib, file->path());
			else 
				_options.addDependency(Options::depDirectDylib, file->path());
		}
	}
}

void InputFiles::logArchive(ld::File* file) const
{
	if ( (_options.traceArchives() || _options.traceEmitJSON()) && (_archiveFilesLogged.count(file) == 0) ) {
		// <rdar://problem/4947347> LD_TRACE_ARCHIVES should only print out when a .o is actually used from an archive
		_archiveFilesLogged.insert(file);
		const char* fullPath = file->path();
		char realName[MAXPATHLEN];
		if ( realpath(fullPath, realName) != NULL )
			fullPath = realName;
		if ( _options.traceArchives() )
			logTraceInfo("[Logging for XBS] Used static archive: %s\n", fullPath);
		
		std::string archivePath(fullPath);
		_archiveFilePaths.push_back(archivePath);
	}
}


void InputFiles::logTraceInfo(const char* format, ...) const
{
	char trace_buffer[MAXPATHLEN * 2];
    va_list ap;
	va_start(ap, format);
	int length = vsnprintf(trace_buffer, sizeof(trace_buffer), format, ap);
	va_end(ap);
	_options.writeToTraceFile(trace_buffer, length);
}


ld::dylib::File* InputFiles::findDylib(const char* installPath, const ld::dylib::File* fromDylib, bool speculative)
{
	//fprintf(stderr, "findDylib(%s, %s)\n", installPath, fromPath);
	InstallNameToDylib::iterator pos = _installPathToDylibs.find(installPath);
	if ( pos != _installPathToDylibs.end() ) {
		return pos->second;
	}
	else {
		// allow -dylib_path option to override indirect library to use
		for (std::vector<Options::DylibOverride>::const_iterator dit = _options.dylibOverrides().begin(); dit != _options.dylibOverrides().end(); ++dit) {
			if ( strcmp(dit->installName,installPath) == 0 ) {
				try {
					Options::FileInfo info = _options.findFile(dit->useInstead);
					_indirectDylibOrdinal = _indirectDylibOrdinal.nextIndirectDylibOrdinal();
					info.ordinal = _indirectDylibOrdinal;
					info.options.fIndirectDylib = true;
					ld::File* reader = this->makeFile(info, true);
					ld::dylib::File* dylibReader = dynamic_cast<ld::dylib::File*>(reader);
					if ( dylibReader != NULL ) {
						addDylib(dylibReader, info);
						//_installPathToDylibs[strdup(installPath)] = dylibReader;
						this->logDylib(dylibReader, true, speculative);
						return dylibReader;
					}
					else 
						throwf("indirect dylib at %s is not a dylib", dit->useInstead);
				}
				catch (const char* msg) {
					warning("ignoring -dylib_file option, %s", msg);
				}
			}
		}

		// search for dylib using -F and -L paths and expanding @ paths
		Options::FileInfo info = _options.findIndirectDylib(installPath, fromDylib);
		_indirectDylibOrdinal = _indirectDylibOrdinal.nextIndirectDylibOrdinal();
		info.ordinal = _indirectDylibOrdinal;
		info.options.fIndirectDylib = true;
		try {
			ld::File* reader = this->makeFile(info, true);
			ld::dylib::File* dylibReader = dynamic_cast<ld::dylib::File*>(reader);
			if ( dylibReader != NULL ) {
				//assert(_installPathToDylibs.find(installPath) !=  _installPathToDylibs.end());
				//_installPathToDylibs[strdup(installPath)] = dylibReader;
				addDylib(dylibReader, info);
				this->logDylib(dylibReader, true, speculative);
				return dylibReader;
			}
			else 
				throwf("indirect dylib at %s is not a dylib", info.path);
		}
		catch (const char* msg) {
			throwf("in '%s', %s", info.path, msg);
		}
	}
}


// mark all dylibs initially specified as required, and check if they can be used
void InputFiles::markExplicitlyLinkedDylibs()
{	
	for (InstallNameToDylib::iterator it=_installPathToDylibs.begin(); it != _installPathToDylibs.end(); it++) {
		it->second->setExplicitlyLinked();
		this->checkDylibClientRestrictions(it->second);
	}
}

bool InputFiles::frameworkAlreadyLoaded(const char* path, const char* frameworkName)
{
	for (ld::File* file : _inputFiles) {
		if ( strcmp(path, file->path()) == 0 )
			return true;
	}
	for (ld::dylib::File* dylibx : _allDylibs) {
		const char* fname = dylibx->frameworkName();
		if ( fname == NULL )
			continue;
		if ( strcmp(frameworkName, fname) == 0 )
			return true;
	}
	return false;
}

bool InputFiles::libraryAlreadyLoaded(const char* path)
{
	for (ld::File* file : _inputFiles) {
		if ( strcmp(path, file->path()) == 0 )
			return true;
	}
	for (ld::dylib::File* dylib : _allDylibs) {
		if ( strcmp(path, dylib->path()) == 0 )
			return true;
	}
	for (const LibraryInfo& libInfo : _searchLibraries) {
		if ( strcmp(path, libInfo.archive()->path()) == 0 )
			return true;
	}

	char realDylibPath[PATH_MAX];
	if ( (realpath(path, realDylibPath) != NULL) && (strcmp(path, realDylibPath) != 0) ) {
		return libraryAlreadyLoaded(realDylibPath);
	}

	return false;
}


void InputFiles::addLinkerOptionLibraries(ld::Internal& state, ld::File::AtomHandler& handler)
{	
  	if ( _options.outputKind() == Options::kObjectFile )
 		return;
  
	while (! state.unprocessedLinkerOptionLibraries.empty() || ! state.unprocessedLinkerOptionFrameworks.empty()) {

		// process frameworks specified in .o linker options
		CStringSet newFrameworks = std::move(state.unprocessedLinkerOptionFrameworks);
		state.unprocessedLinkerOptionFrameworks.clear();
		for (const char* frameworkName : newFrameworks) {
			if ( state.linkerOptionFrameworks.count(frameworkName) )
				continue;
			try {
				Options::FileInfo info = _options.findFramework(frameworkName);
				if ( ! this->frameworkAlreadyLoaded(info.path, frameworkName) ) {
					_linkerOptionOrdinal = _linkerOptionOrdinal.nextLinkerOptionOrdinal();
					info.ordinal = _linkerOptionOrdinal;
					ld::File* reader = this->makeFile(info, true);
					ld::dylib::File* dylibReader = dynamic_cast<ld::dylib::File*>(reader);
					ld::archive::File* archiveReader = dynamic_cast<ld::archive::File*>(reader);
					if ( dylibReader != NULL ) {
						if ( ! dylibReader->installPathVersionSpecific() ) {
							checkDylibClientRestrictions(dylibReader);
							dylibReader->forEachAtom(handler);
							dylibReader->setImplicitlyLinked();
							dylibReader->setSpeculativelyLoaded();
							if ( state.linkerOptionNeededFrameworks.count(frameworkName) )
								dylibReader->setNeededDylib();
							this->addDylib(dylibReader, info);
						}
					}
					else if ( archiveReader != NULL ) {
						_searchLibraries.push_back(LibraryInfo(archiveReader));
						_options.addDependency(Options::depArchive, archiveReader->path());
						//<rdar://problem/17787306> -force_load_swift_libs
						if (info.options.fForceLoad) {
							archiveReader->forEachAtom(handler);
						}
					}
					else {
						throwf("framework linker option at %s is not a dylib and not an archive", info.path);
 					}
 				}
 			}
			catch (const char* msg) {
				if ( strstr(msg, "but linking") != nullptr )
					warning("%s '%s.framework'", msg, frameworkName);
				// <rdar://problem/40829444> only warn about missing auto-linked framework if some missing symbol error happens later
				state.missingLinkerOptionFrameworks.insert(frameworkName);
			}
			state.linkerOptionFrameworks.insert(frameworkName);
 		}

		// process libraries specified in .o linker options
		// fixme optimize with std::move?
		CStringSet newLibraries = std::move(state.unprocessedLinkerOptionLibraries);
		state.unprocessedLinkerOptionLibraries.clear();
		for (const char* libName : newLibraries) {
			if ( state.linkerOptionLibraries.count(libName) )
				continue;
			try {
				Options::FileInfo info = _options.findLibrary(libName);
				if ( ! this->libraryAlreadyLoaded(info.path) ) {
					_linkerOptionOrdinal = _linkerOptionOrdinal.nextLinkerOptionOrdinal();
					info.ordinal = _linkerOptionOrdinal;
 					//<rdar://problem/17787306> -force_load_swift_libs
					info.options.fForceLoad = _options.forceLoadSwiftLibs() && (strncmp(libName, "swift", 5) == 0);
					ld::File* reader = this->makeFile(info, true);
					ld::dylib::File* dylibReader = dynamic_cast<ld::dylib::File*>(reader);
					ld::archive::File* archiveReader = dynamic_cast<ld::archive::File*>(reader);
					if ( dylibReader != NULL ) {
						checkDylibClientRestrictions(dylibReader);
						dylibReader->forEachAtom(handler);
						dylibReader->setImplicitlyLinked();
						dylibReader->setSpeculativelyLoaded();
						this->addDylib(dylibReader, info);
						if ( state.linkerOptionNeededLibraries.count(libName) )
							dylibReader->setNeededDylib();
					}
					else if ( archiveReader != NULL ) {
						_searchLibraries.push_back(LibraryInfo(archiveReader));
						_options.addDependency(Options::depArchive, archiveReader->path());
						//<rdar://problem/17787306> -force_load_swift_libs
						if (info.options.fForceLoad) {
							archiveReader->forEachAtom(handler);
						}
					}
					else {
						throwf("linker option dylib at %s is not a dylib", info.path);
 					}
 				}
 			}
			catch (const char* msg) {
				if ( strstr(msg, "but linking") != nullptr )
					warning("%s '%s'", msg, libName);
				// <rdar://problem/40829444> only warn about missing auto-linked library if some missing symbol error happens later
				state.missingLinkerOptionLibraries.insert(libName);
			}
			state.linkerOptionLibraries.insert(libName);
		}
	}
}

void InputFiles::createIndirectDylibs()
{	
	// keep processing dylibs until no more dylibs are added
	unsigned long lastMapSize = 0;
	std::set<ld::dylib::File*>  dylibsProcessed;
	while ( lastMapSize != _allDylibs.size() ) {
		lastMapSize = _allDylibs.size();
		// can't iterator _installPathToDylibs while modifying it, so use temp buffer
		std::vector<ld::dylib::File*> unprocessedDylibs;
		for (std::set<ld::dylib::File*>::iterator it=_allDylibs.begin(); it != _allDylibs.end(); it++) {
			if ( dylibsProcessed.count(*it) == 0 )
				unprocessedDylibs.push_back(*it);
		}
		// <rdar://problem/42675402> ld64 output is not deterministic due to dylib processing order
		std::sort(unprocessedDylibs.begin(), unprocessedDylibs.end(), [](const ld::dylib::File* lhs, const ld::dylib::File* rhs) {
			return strcmp(lhs->path(), rhs->path()) < 0;
		});
		for (std::vector<ld::dylib::File*>::iterator it=unprocessedDylibs.begin(); it != unprocessedDylibs.end(); it++) {
			dylibsProcessed.insert(*it);
			(*it)->processIndirectLibraries(this, _options.implicitlyLinkIndirectPublicDylibs());
		}
	}

	// go back over original dylibs and mark sub frameworks as re-exported
	if ( _options.outputKind() == Options::kDynamicLibrary ) {
		const char* myLeaf = strrchr(_options.installPath(), '/');
		if ( myLeaf != NULL ) {
			for (std::vector<class ld::File*>::const_iterator it=_inputFiles.begin(); it != _inputFiles.end(); it++) {
				ld::dylib::File* dylibReader = dynamic_cast<ld::dylib::File*>(*it);
				if ( dylibReader != NULL ) {
					const char* childParent = dylibReader->parentUmbrella();
					if ( childParent != NULL ) {
						if ( strcmp(childParent, &myLeaf[1]) == 0 ) {
							// mark that this dylib will be re-exported
							dylibReader->setWillBeReExported();
						}
					}
				}
			}
		}
	}
	
}

void InputFiles::createOpaqueFileSections()
{
	// extra command line sections always at end
	for (Options::ExtraSection::const_iterator it=_options.extraSectionsBegin(); it != _options.extraSectionsEnd(); ++it) {
		_inputFiles.push_back(opaque_section::parse(it->segmentName, it->sectionName, it->path, it->data, it->dataLen));
		_options.addDependency(Options::depSection, it->path);
	}

}


const char* sGrandfatheredLooseAllow[] = {
	"/CloudKitCode.framework",
	"/CloudKitCodeProtobuf.framework",
	"/Helix.framework",
	"/NewsCore.framework",
	"/NewsServicesInternal.framework",
	"/NewsTransport.framework",
	"/NewsUI.framework",
	"/Silex.framework"
};


void InputFiles::checkDylibClientRestrictions(ld::dylib::File* dylib)
{
	// Check for any restrictions on who can link with this dylib  
	const char* dylibParentName = dylib->parentUmbrella() ;
	const std::vector<const char*>* clients = dylib->allowableClients();
	if ( (dylibParentName != NULL) || (clients != NULL) ) {
		// only dylibs that are in an umbrella or have a client list need verification
		const char* installName = _options.installPath();
		const char* installNameLastSlash = strrchr(installName, '/');
		bool isParent = false;
		bool isSibling = false;
		bool isAllowableClient = false;
		// There are three cases:
		if ( (dylibParentName != NULL) && (installNameLastSlash != NULL) ) {
			// starts after last slash
			const char* myName = &installNameLastSlash[1];
			unsigned int myNameLen = strlen(myName);
			if ( strncmp(myName, "lib", 3) == 0 )
				myName = &myName[3];
			// up to first dot
			const char* firstDot = strchr(myName, '.');
			if ( firstDot != NULL )
				myNameLen = firstDot - myName;
			// up to first underscore
			const char* firstUnderscore = strchr(myName, '_');
			if ( (firstUnderscore != NULL) && ((firstUnderscore - myName) < (int)myNameLen) )
				myNameLen = firstUnderscore - myName;
		
			// case 1) The dylib has a parent umbrella, and we are creating the parent umbrella
			isParent = ( (strlen(dylibParentName) == myNameLen) && (strncmp(myName, dylibParentName, myNameLen) == 0) );
			
			// case 2) The dylib has a parent umbrella, and we are creating a sibling with the same parent
			isSibling = ( (_options.umbrellaName() != NULL) && (strcmp(_options.umbrellaName(), dylibParentName) == 0) );
		}

		if ( !isParent && !isSibling && (clients != NULL) ) {
			// case 3) the dylib has a list of allowable clients, and we are creating one of them
			const char* clientName = _options.clientName();
			int clientNameLen = 0;
			if ( clientName != NULL ) {
				// use client name as specified on command line
				clientNameLen = strlen(clientName);
			}
			else {
				// infer client name from output path (e.g. xxx/libfoo_variant.A.dylib --> foo, Bar.framework/Bar_variant --> Bar)
				clientName = installName;
				clientNameLen = strlen(clientName);
				// starts after last slash
				if ( installNameLastSlash != NULL )
					clientName = &installNameLastSlash[1];
				if ( strncmp(clientName, "lib", 3) == 0 )
					clientName = &clientName[3];
				// up to first dot
				const char* firstDot = strchr(clientName, '.');
				if ( firstDot != NULL )
					clientNameLen = firstDot - clientName;
				// up to first underscore
				const char* firstUnderscore = strchr(clientName, '_');
				if ( (firstUnderscore != NULL) && ((firstUnderscore - clientName) < clientNameLen) )
					clientNameLen = firstUnderscore - clientName;
			}

			// Use clientName to check if this dylib is able to link against the allowable clients.
			for (std::vector<const char*>::const_iterator it = clients->begin(); it != clients->end(); it++) {
				if ( strncmp(*it, clientName, clientNameLen) == 0 )
					isAllowableClient = true;
			}
			// temp fix until projects update allowable clients
			if ( !isAllowableClient ) {
				for (const char* frameworkName : sGrandfatheredLooseAllow) {
					if ( strstr(dylib->installPath(), frameworkName) != NULL ) {
						warning("%s did not mark %s as an allowable client", dylib->installPath(), clientName);
						isAllowableClient = true;
						break;
					}
				}
			}

		}
	
		if ( !isParent && !isSibling && !isAllowableClient ) {
			if ( dylibParentName != NULL ) {
				throwf("cannot link directly with %s.  Link against the umbrella framework '%s.framework' instead.", 
					dylib->path(), dylibParentName);
			}
			else {
				throwf("cannot link directly with dylib/framework, your binary is not an allowed client of %s", dylib->path());
			}
		}
	}
}


InputFiles::InputFiles(Options& opts) 
 : _totalObjectSize(0), _totalArchiveSize(0), 
   _totalObjectLoaded(0), _totalArchivesLoaded(0), _totalDylibsLoaded(0),
	_options(opts), _bundleLoader(NULL), 
	_exception(NULL), 
	_indirectDylibOrdinal(ld::File::Ordinal::indirectDylibBase()),
	_linkerOptionOrdinal(ld::File::Ordinal::linkeOptionBase())
{
//	fStartCreateReadersTime = mach_absolute_time();
#if HAVE_PTHREADS
	pthread_mutex_init(&_parseLock, NULL);
	pthread_cond_init(&_parseWorkReady, NULL);
	pthread_cond_init(&_newFileAvailable, NULL);
	_neededFileSlot = -1;
#endif
	const std::vector<Options::FileInfo>& files = _options.getInputFiles();
	if ( files.size() == 0 )
		throw "no object files specified";

	_inputFiles.reserve(files.size());
#if HAVE_PTHREADS
	unsigned int inputFileSlot = 0;
	_availableInputFiles = 0;
	_parseCursor = 0;
#endif
	Options::FileInfo* entry;
	for (std::vector<Options::FileInfo>::const_iterator it = files.begin(); it != files.end(); ++it) {
		entry = (Options::FileInfo*)&(*it);
#if HAVE_PTHREADS
		// Assign input file slots to all the FileInfos.
		// Also chain all FileInfos into one big list to set up for worker threads to do parsing.
		entry->inputFileSlot = inputFileSlot;
		entry->readyToParse = !entry->fromFileList || !_options.pipelineEnabled();
		if (entry->readyToParse)
			_availableInputFiles++;
		_inputFiles.push_back(NULL);
		inputFileSlot++;
#else
		// In the non-threaded case just parse the file now.
		_inputFiles.push_back(makeFile(*entry, false));
#endif
	}
	
#if HAVE_PTHREADS
	_remainingInputFiles = files.size();
	
	// initialize info for parsing input files on worker threads
	unsigned int ncpus;
#ifdef __linux__
	ncpus = (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
#else
	int mib[2];
	size_t len = sizeof(ncpus);
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	if (sysctl(mib, 2, &ncpus, &len, NULL, 0) != 0) {
		ncpus = 1;
	}
#endif

	_availableWorkers = MIN(ncpus, files.size()); // max # workers we permit
	_idleWorkers = 0;
	
	if (_options.pipelineEnabled()) {
		// start up a thread to listen for available input files
		startThread(InputFiles::waitForInputFiles);
	}

	// Start up one parser thread. More start on demand as parsed input files get consumed.
	startThread(InputFiles::parseWorkerThread);
	_availableWorkers--;
#else
	if (_options.pipelineEnabled()) {
		throwf("pipelined linking not supported on this platform");
	}
#endif
}


#if HAVE_PTHREADS
void InputFiles::startThread(void (*threadFunc)(InputFiles *)) const {
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	// set a nice big stack (same as main thread) because some code uses potentially large stack buffers
	pthread_attr_setstacksize(&attr, 16 * 1024 * 1024);
	pthread_create(&thread, &attr, (void *(*)(void*))threadFunc, (void *)this);
	pthread_detach(thread);
	pthread_attr_destroy(&attr);
}

// Work loop for input file parsing threads
void InputFiles::parseWorkerThread() {
	ld::File *file;
	const char *exception = NULL;
	pthread_mutex_lock(&_parseLock);
	const std::vector<Options::FileInfo>& files = _options.getInputFiles();
	if (_s_logPThreads) printf("worker starting\n");
	do {
		if (_availableInputFiles == 0) {
			_idleWorkers++;
			pthread_cond_wait(&_parseWorkReady, &_parseLock);
			_idleWorkers--;
		} else {
			int slot = _parseCursor;
			while (slot < (int)files.size() && (_inputFiles[slot] != NULL || !files[slot].readyToParse))
				slot++;
			assert(slot < (int)files.size());
			Options::FileInfo& entry = (Options::FileInfo&)files[slot];
			_parseCursor = slot+1;
			_availableInputFiles--;
			entry.readyToParse = false; // to avoid multiple threads finding this file
			pthread_mutex_unlock(&_parseLock);
			if (_s_logPThreads) printf("parsing index %u\n", slot);
			try {
				file = makeFile(entry, false);
			}
			catch (const char *msg) {
				if ( ((strstr(msg, "architecture") != NULL)  || (strstr(msg, "attempting to link") != NULL)) && !_options.errorOnOtherArchFiles() ) {
					if ( _options.ignoreOtherArchInputFiles() ) {
						// ignore, because this is about an architecture not in use
					}
					else {
						warning("ignoring file %s, %s", entry.path, msg);
					}
				} 
				else if ( strstr(msg, "ignoring unexpected") != NULL ) {
					warning("%s, %s", entry.path, msg);
				}
				else {
					asprintf((char**)&exception, "%s file '%s'", msg, entry.path);
				}
				file = new IgnoredFile(entry.path, entry.modTime, entry.ordinal, ld::File::Other);
			}
			pthread_mutex_lock(&_parseLock);
			if (_remainingInputFiles > 0)
				_remainingInputFiles--;
			if (_s_logPThreads) printf("done with index %u, %d remaining\n", slot, _remainingInputFiles);
			if (exception) {
				// We are about to die, so set to zero to stop other threads from doing unneeded work.
				_remainingInputFiles = 0;
				_exception = exception;
			} 
			else {
				_inputFiles[slot] = file;
				if (_neededFileSlot == slot)
					pthread_cond_signal(&_newFileAvailable);
			}
		}
	} while (_remainingInputFiles);
	if (_s_logPThreads) printf("worker exiting\n");
	pthread_cond_broadcast(&_parseWorkReady);
	pthread_cond_signal(&_newFileAvailable);
	pthread_mutex_unlock(&_parseLock);
}


void InputFiles::parseWorkerThread(InputFiles *inputFiles) {
	inputFiles->parseWorkerThread();
}
#endif


ld::File* InputFiles::addDylib(ld::dylib::File* reader, const Options::FileInfo& info)
{
	_allDylibs.insert(reader);
	
	if ( (reader->installPath() == NULL) && !info.options.fBundleLoader ) {
		// this is a "blank" stub
		// silently ignore it
		return reader;
	}
	// store options about how dylib will be used in dylib itself
	if ( info.options.fWeakImport )
		reader->setForcedWeakLinked();
	if ( info.options.fReExport )
		reader->setWillBeReExported();
	if ( info.options.fUpward ) {
		if ( _options.outputKind() == Options::kDynamicLibrary ) 
			reader->setWillBeUpwardDylib();
		else 
			warning("ignoring upward dylib option for %s\n", info.path);
	}
	if ( info.options.fNeeded )
		reader->setNeededDylib();

	// add to map of loaded dylibs
	const char* installPath = reader->installPath();
	if ( installPath != NULL ) {
		InstallNameToDylib::iterator pos = _installPathToDylibs.find(installPath);
		if ( pos == _installPathToDylibs.end() ) {
			_installPathToDylibs[strdup(installPath)] = reader;
		}
		else {
			bool dylibOnCommandLineTwice = ( strcmp(pos->second->path(), reader->path()) == 0 );
			bool isSymlink = false;
			// ignore if this is a symlink to a dylib we've already loaded
			if ( !dylibOnCommandLineTwice ) {
				char existingDylibPath[PATH_MAX];
				if ( realpath(pos->second->path(), existingDylibPath) != NULL ) {
					char newDylibPath[PATH_MAX];
					if ( realpath(reader->path(), newDylibPath) != NULL ) {
						isSymlink = ( strcmp(existingDylibPath, newDylibPath) == 0 );
					}
				}
			}
			// remove warning for <rdar://problem/10860629> Same install name for CoreServices and CFNetwork?
			//if ( !dylibOnCommandLineTwice && !isSymlink )
			//      warning("dylibs with same install name: %p %s and %p %s", pos->second, pos->second->path(), reader, reader->path());
		}
	}
	else if ( info.options.fBundleLoader )
		_bundleLoader = reader;

	// log direct readers
	if ( ! info.options.fIndirectDylib ) 
		this->logDylib(reader, false, false);

	// update stats
	_totalDylibsLoaded++;

	// just add direct libraries to search-first list
	if ( ! info.options.fIndirectDylib ) 
		_searchLibraries.push_back(LibraryInfo(reader));
	
	return reader;
}


#if HAVE_PTHREADS
// Called during pipelined linking to listen for available input files.
// Available files are enqueued for parsing.
void InputFiles::waitForInputFiles()
{
	if (_s_logPThreads) printf("starting pipeline listener\n");
	try {
		const char *fifo = _options.pipelineFifo();
		assert(fifo);
		std::map<const char *, const Options::FileInfo*, strcompclass> fileMap;
		const std::vector<Options::FileInfo>& files = _options.getInputFiles();
		for (std::vector<Options::FileInfo>::const_iterator it = files.begin(); it != files.end(); ++it) {
			const Options::FileInfo& entry = *it;
			if (entry.fromFileList) {
				fileMap[entry.path] = &entry;
			}
		}
		FILE *fileStream = fopen(fifo, "r");
		if (!fileStream)
			throwf("pipelined linking error - failed to open stream. fopen() returns %s for \"%s\"\n", strerror(errno), fifo);
		while (fileMap.size() > 0) {
			char path_buf[PATH_MAX+1];
			if (fgets(path_buf, PATH_MAX, fileStream) == NULL)
				throwf("pipelined linking error - %lu missing input files", fileMap.size());
			int len = strlen(path_buf);
			if (path_buf[len-1] == '\n')
				path_buf[len-1] = 0;
			std::map<const char *, const Options::FileInfo*, strcompclass>::iterator it = fileMap.find(path_buf);
			if (it == fileMap.end())
				throwf("pipelined linking error - not in file list: %s\n", path_buf);
			Options::FileInfo* inputInfo = (Options::FileInfo*)it->second;
			if (!inputInfo->checkFileExists(_options))
				throwf("pipelined linking error - file does not exist: %s\n", inputInfo->path);
			pthread_mutex_lock(&_parseLock);
			if (_idleWorkers)
				pthread_cond_signal(&_parseWorkReady);
			inputInfo->readyToParse = true;
			if (_parseCursor > inputInfo->inputFileSlot)
				_parseCursor = inputInfo->inputFileSlot;
			_availableInputFiles++;
			if (_s_logPThreads) printf("pipeline listener: %s slot=%d, _parseCursor=%d, _availableInputFiles = %d remaining = %ld\n", path_buf, inputInfo->inputFileSlot, _parseCursor, _availableInputFiles, fileMap.size()-1);
			pthread_mutex_unlock(&_parseLock);
			fileMap.erase(it);
		}
	} catch (const char *msg) {
		pthread_mutex_lock(&_parseLock);
		_exception = msg;
		pthread_cond_signal(&_newFileAvailable);
		pthread_mutex_unlock(&_parseLock);
	}
}


void InputFiles::waitForInputFiles(InputFiles *inputFiles) {
	inputFiles->waitForInputFiles();
}
#endif


void InputFiles::forEachInitialAtom(ld::File::AtomHandler& handler, ld::Internal& state)
{
	// add all direct object, archives, and dylibs
	const std::vector<Options::FileInfo>& files = _options.getInputFiles();
	size_t fileIndex;
	for (fileIndex=0; fileIndex<_inputFiles.size(); fileIndex++) {
		ld::File *file;
#if HAVE_PTHREADS
		pthread_mutex_lock(&_parseLock);
		
		// this loop waits for the needed file to be ready (parsed by worker thread)
		while (_inputFiles[fileIndex] == NULL && _exception == NULL) {
			// We are starved for input. If there are still files to parse and we have
			// not maxed out the worker thread count start a new worker thread.
			if (_availableInputFiles > 0 && _availableWorkers > 0) {
				if (_s_logPThreads) printf("starting worker\n");
				startThread(InputFiles::parseWorkerThread);
				_availableWorkers--;
			}
			_neededFileSlot = fileIndex;
			if (_s_logPThreads) printf("consumer blocking for %lu: %s\n", fileIndex, files[fileIndex].path);
			pthread_cond_wait(&_newFileAvailable, &_parseLock);
		}

		if (_exception) {
			// <rdar://problem/16525216> the tool is erroring out.  wait for other threads to finish so we don't destruct global objects out from under them
			sleep(1);
			throw _exception;
		}

		// The input file is parsed. Assimilate it and call its atom iterator.
		if (_s_logPThreads) printf("consuming slot %lu\n", fileIndex);
		file = _inputFiles[fileIndex];
		pthread_mutex_unlock(&_parseLock);
#else
		file = _inputFiles[fileIndex];
#endif
		const Options::FileInfo& info = files[fileIndex];
		switch (file->type()) {
			case ld::File::Reloc:
			{
				ld::relocatable::File* reloc = (ld::relocatable::File*)file;
				_options.snapshot().recordObjectFile(reloc->path());
				_options.addDependency(Options::depObjectFile, reloc->path());
			}
				break;
			case ld::File::Dylib:
			{
				ld::dylib::File* dylib = (ld::dylib::File*)file;
				addDylib(dylib, info);
			}
				break;
			case ld::File::Archive:
			{
				ld::archive::File* archive = (ld::archive::File*)file;
				// <rdar://problem/9740166> force loaded archives should be in LD_TRACE
				if ( (info.options.fForceLoad || _options.fullyLoadArchives()) && (_options.traceArchives() || _options.traceEmitJSON()) )
					logArchive(archive);

				if ( isCompilerSupportLib(info.path) && (info.options.fForceLoad || _options.fullyLoadArchives()) )
					state.forceLoadCompilerRT = true;

				_searchLibraries.push_back(LibraryInfo(archive));
				_options.addDependency(Options::depArchive, archive->path());
			}
				break;
			case ld::File::Other:
				break;
			default:
			{
				throwf("Unknown file type for %s", file->path());
			}
				break;
		}
		try {
			file->forEachAtom(handler);
		}
		catch (const char* msg) {
			asprintf((char**)&_exception, "%s file '%s'", msg, file->path());
		}
	}
	if (_exception) {
		// <rdar://problem/16525216> the tool is erroring out.  wait for other threads to finish so we don't destruct global objects out from under them
		sleep(1);
		throw _exception;
	}

	markExplicitlyLinkedDylibs();
	addLinkerOptionLibraries(state, handler);
	createIndirectDylibs();
	createOpaqueFileSections();
	
	while (fileIndex < _inputFiles.size()) {
		ld::File *file = _inputFiles[fileIndex];
		file->forEachAtom(handler);
		fileIndex++;
	}
    
    switch ( _options.outputKind() ) {
        case Options::kStaticExecutable:
        case Options::kDynamicExecutable:
            // add implicit __dso_handle label
            handler.doAtom(DSOHandleAtom::_s_atomExecutable);
            handler.doAtom(DSOHandleAtom::_s_atomAll);
            if ( _options.pageZeroSize() != 0 ) 
                handler.doAtom(*new PageZeroAtom(_options.pageZeroSize()));
            if ( _options.hasCustomStack() && !_options.needsEntryPointLoadCommand() ) 
                handler.doAtom(*new CustomStackAtom(_options.customStackSize()));
            break;
        case Options::kDynamicLibrary:
            // add implicit __dso_handle label
            handler.doAtom(DSOHandleAtom::_s_atomDylib);
            handler.doAtom(DSOHandleAtom::_s_atomAll);
            break;
        case Options::kDynamicBundle:
            // add implicit __dso_handle label
            handler.doAtom(DSOHandleAtom::_s_atomBundle);
            handler.doAtom(DSOHandleAtom::_s_atomAll);
            break;
        case Options::kDyld:
            // add implicit __dso_handle label
            handler.doAtom(DSOHandleAtom::_s_atomDyld);
            handler.doAtom(DSOHandleAtom::_s_atomAll);
            break;
        case Options::kPreload:
            // add implicit __mh_preload_header label
            handler.doAtom(DSOHandleAtom::_s_atomPreload);
            // add implicit __dso_handle label, but put it in __text section because 
            // with -preload the mach_header is no in the address space.
            handler.doAtom(DSOHandleAtom::_s_atomPreloadDSO);
            break;
        case Options::kObjectFile:
            handler.doAtom(DSOHandleAtom::_s_atomObjectFile);
            break;
        case Options::kKextBundle:
            // add implicit __dso_handle label
            handler.doAtom(DSOHandleAtom::_s_atomAll);
            break;
	}
}


bool InputFiles::searchLibraries(const char* name, bool searchDylibs, bool searchArchives, bool dataSymbolOnly, ld::File::AtomHandler& handler) const
{
	// Check each input library.
    for (std::vector<LibraryInfo>::const_iterator it=_searchLibraries.begin(); it != _searchLibraries.end(); ++it) {
        LibraryInfo lib = *it;
        if (lib.isDylib()) {
            if (searchDylibs) {
                ld::dylib::File *dylibFile = lib.dylib();
                //fprintf(stderr, "searchLibraries(%s), looking in linked %s\n", name, dylibFile->path() );
                if ( dylibFile->justInTimeforEachAtom(name, handler) ) {
                    // we found a definition in this dylib
                    // done, unless it is a weak definition in which case we keep searching
                    _options.snapshot().recordDylibSymbol(dylibFile, name);
                    if ( !dylibFile->hasWeakExternals() || !dylibFile->hasWeakDefinition(name)) {
                        return true;
                    }
                    // else continue search for a non-weak definition
                }
            }
        } else {
            if (searchArchives) {
                ld::archive::File *archiveFile = lib.archive();
                if ( dataSymbolOnly ) {
                    if ( archiveFile->justInTimeDataOnlyforEachAtom(name, handler) ) {
                        if ( _options.traceArchives() || _options.traceEmitJSON())
                            logArchive(archiveFile);
                        _options.snapshot().recordArchive(archiveFile->path());
						// DALLAS _state.archives.push_back(archiveFile);
                        // found data definition in static library, done
                       return true;
                    }
                }
                else {
                    if ( archiveFile->justInTimeforEachAtom(name, handler) ) {
                        if ( _options.traceArchives() || _options.traceEmitJSON())
                            logArchive(archiveFile);
                        _options.snapshot().recordArchive(archiveFile->path());
                        // found definition in static library, done
                        return true;
                    }
                }
            }
        }
    }

	// search indirect dylibs
	if ( searchDylibs ) {
		for (InstallNameToDylib::const_iterator it=_installPathToDylibs.begin(); it != _installPathToDylibs.end(); ++it) {
			ld::dylib::File* dylibFile = it->second;
			bool searchThisDylib = false;
			if ( _options.nameSpace() == Options::kTwoLevelNameSpace ) {
				// for two level namesapce, just check all implicitly linked dylibs
				searchThisDylib = dylibFile->implicitlyLinked() && !dylibFile->explicitlyLinked();
			}
			else {
				// for flat namespace, check all indirect dylibs
				searchThisDylib = ! dylibFile->explicitlyLinked();
			}
			if ( searchThisDylib ) {
				//fprintf(stderr, "searchLibraries(%s), looking in implicitly linked %s\n", name, dylibFile->path() );
				if ( dylibFile->justInTimeforEachAtom(name, handler) ) {
					// we found a definition in this dylib
					// done, unless it is a weak definition in which case we keep searching
                    _options.snapshot().recordDylibSymbol(dylibFile, name);
					if ( !dylibFile->hasWeakExternals() || !dylibFile->hasWeakDefinition(name)) {
						return true;
                    }
					// else continue search for a non-weak definition
				}
			}			
		}
	}

	return false;
}


bool InputFiles::searchWeakDefInDylib(const char* name) const
{
	// search all relevant dylibs to see if any have a weak-def with this name
	for (InstallNameToDylib::const_iterator it=_installPathToDylibs.begin(); it != _installPathToDylibs.end(); ++it) {
		ld::dylib::File* dylibFile = it->second;
		if ( dylibFile->implicitlyLinked() || dylibFile->explicitlyLinked() ) {
			if ( dylibFile->hasWeakExternals() && dylibFile->hasWeakDefinition(name) ) {
				return true;
			}
		}
	}
	return false;
}
	
static bool vectorContains(const std::vector<ld::dylib::File*>& vec, ld::dylib::File* key)
{
	return std::find(vec.begin(), vec.end(), key) != vec.end();
}

struct DylibByInstallNameSorter
{	
	 bool operator()(const ld::dylib::File* left, const ld::dylib::File* right)
	 {
          return (strcmp(left->installPath(), right->installPath()) < 0);
	 }
};

void InputFiles::dylibs(ld::Internal& state)
{
	bool dylibsOK = false;
	switch ( _options.outputKind() ) {
		case Options::kDynamicExecutable:
		case Options::kDynamicLibrary:
		case Options::kDynamicBundle:
			dylibsOK = true;
			break;
		case Options::kStaticExecutable:
		case Options::kDyld:
		case Options::kPreload:
		case Options::kObjectFile:
		case Options::kKextBundle:
			dylibsOK = false;
			break;
	}

	// add command line dylibs in order
	for (std::vector<ld::File*>::const_iterator it=_inputFiles.begin(); it != _inputFiles.end(); ++it) {
		ld::dylib::File* dylibFile = dynamic_cast<ld::dylib::File*>(*it);
		// only add dylibs that are not "blank" dylib stubs
		if ( (dylibFile != NULL) && ((dylibFile->installPath() != NULL) || (dylibFile == _bundleLoader)) ) {
			if ( dylibsOK ) {
				if ( ! vectorContains(state.dylibs, dylibFile) ) {
					state.dylibs.push_back(dylibFile);
				}
			}
			else
				warning("unexpected dylib (%s) on link line", dylibFile->path());
		}
	}
	// add implicitly linked dylibs
	if ( _options.nameSpace() == Options::kTwoLevelNameSpace ) {
		std::vector<ld::dylib::File*> implicitDylibs;
		for (InstallNameToDylib::const_iterator it=_installPathToDylibs.begin(); it != _installPathToDylibs.end(); ++it) {
			ld::dylib::File* dylibFile = it->second;
			if ( dylibFile->implicitlyLinked() && dylibsOK ) {
				if ( !vectorContains(implicitDylibs, dylibFile) && !vectorContains(state.dylibs, dylibFile) ) {
					implicitDylibs.push_back(dylibFile);
				}
			}
		}
		// <rdar://problem/15002251> make implicit dylib order be deterministic by sorting by install_name
		std::sort(implicitDylibs.begin(), implicitDylibs.end(), DylibByInstallNameSorter());

		if ( _options.traceDylibs() ) {
			for (ld::dylib::File* dylib :  implicitDylibs) {
				if ( dylib->speculativelyLoaded() && !dylib->explicitlyLinked() && dylib->providedExportAtom() ) {
					const char* fullPath = dylib->path();
					char realName[MAXPATHLEN];
					if ( realpath(fullPath, realName) != NULL )
						fullPath = realName;
					logTraceInfo("[Logging for XBS] Used dynamic library: %s\n", fullPath);
				}
			}
		}
		state.dylibs.insert(state.dylibs.end(), implicitDylibs.begin(), implicitDylibs.end());
	}

	//fprintf(stderr, "all dylibs:\n");
	//for(std::vector<ld::dylib::File*>::iterator it=state.dylibs.begin(); it != state.dylibs.end(); ++it) {
	//	const ld::dylib::File* dylib = *it;
	//	fprintf(stderr, "    %p impl=%d %s\n", dylib, dylib->implicitlyLinked(), dylib->path());
	//}
	
	// and -bundle_loader
	state.bundleLoader = _bundleLoader;
	
	// <rdar://problem/10807040> give an error when -nostdlib is used and libSystem is missing
	if ( (state.dylibs.size() == 0) && _options.needsEntryPointLoadCommand() && !_options.platforms().contains(ld::Platform::driverKit))  {
		// HACK until 39514191 is fixed
		bool grandfather = false;
		for (const File* inFile : _inputFiles) {
			if ( strstr(inFile->path(), "exit-asm.o") != NULL )
				grandfather = true;
		}
		if ( !grandfather )
			throw "dynamic main executables must link with libSystem.dylib";
	}
}

void InputFiles::archives(ld::Internal& state)
{
	for (const std::string& path :  _archiveFilePaths) {
		
		state.archivePaths.push_back(path);
	}
}


} // namespace tool 
} // namespace ld 

