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

#ifndef __INPUT_FILES_H__
#define __INPUT_FILES_H__

#define HAVE_PTHREADS 1

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#ifndef __linux__
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
#if HAVE_PTHREADS
#include <pthread.h>
#endif

#include <vector>

#include "Options.h"
#include "ld.hpp"

namespace ld {
namespace tool {

class InputFiles : public ld::dylib::File::DylibHandler
{
public:
								InputFiles(Options& opts);

	// implementation from ld::dylib::File::DylibHandler
	virtual ld::dylib::File*	findDylib(const char* installPath, const ld::dylib::File* fromDylib, bool speculative);
	
	// iterates all atoms in initial files
	void						forEachInitialAtom(ld::File::AtomHandler&, ld::Internal& state);
	// searches libraries for name
	bool						searchLibraries(const char* name, bool searchDylibs, bool searchArchives,  
																  bool dataSymbolOnly, ld::File::AtomHandler&) const;
	// see if any linked dylibs export a weak def of symbol
	bool						searchWeakDefInDylib(const char* name) const;
	// copy dylibs to link with in command line order
	void						dylibs(ld::Internal& state);
	
	void						archives(ld::Internal& state);

	void						addLinkerOptionLibraries(ld::Internal& state, ld::File::AtomHandler& handler);
	void						createIndirectDylibs();

	// for -print_statistics
	volatile int64_t			_totalObjectSize;
	volatile int64_t			_totalArchiveSize;
	volatile int32_t			_totalObjectLoaded;
	volatile int32_t			_totalArchivesLoaded;
	volatile int32_t			_totalDylibsLoaded;
	
	
private:
	void						inferArchitecture(Options& opts, const char** archName);
	const char* 				extractFileInfo(const uint8_t* p, unsigned len, const char* path, ld::Platform& platform);
	ld::File*					makeFile(const Options::FileInfo& info, bool indirectDylib);
	ld::File*					addDylib(ld::dylib::File* f,        const Options::FileInfo& info);
	void						logTraceInfo (const char* format, ...) const;
	void						logDylib(ld::File*, bool indirect, bool speculative);
	void						logArchive(ld::File*) const;
	void						markExplicitlyLinkedDylibs();
	void						checkDylibClientRestrictions(ld::dylib::File*);
	void						createOpaqueFileSections();
	bool						libraryAlreadyLoaded(const char* path);
	bool						frameworkAlreadyLoaded(const char* path, const char* frameworkName);

	// for pipelined linking
    void						waitForInputFiles();
	static void					waitForInputFiles(InputFiles *inputFiles);

	// for threaded input file processing
	void						parseWorkerThread();
	static void					parseWorkerThread(InputFiles *inputFiles);
	void						startThread(void (*threadFunc)(InputFiles *)) const;

	typedef std::map<std::string, ld::dylib::File*>	InstallNameToDylib;

	const Options&				_options;
	std::vector<ld::File*>		_inputFiles;
	mutable std::set<class ld::File*>	_archiveFilesLogged;
	mutable std::vector<std::string>	_archiveFilePaths;
	InstallNameToDylib			_installPathToDylibs;
	std::set<ld::dylib::File*>	_allDylibs;
	ld::dylib::File*			_bundleLoader;
    struct strcompclass {
        bool operator() (const char *a, const char *b) const { return ::strcmp(a, b) < 0; }
    };

	// for threaded input file processing
#if HAVE_PTHREADS
	pthread_mutex_t				_parseLock;
	pthread_cond_t				_parseWorkReady;		// used by parse threads to block for work
	pthread_cond_t				_newFileAvailable;		// used by main thread to block for parsed input files
	int							_availableWorkers;		// number of remaining unstarted parse threads
	int							_idleWorkers;			// number of running parse threads that are idle
	int							_neededFileSlot;		// input file the resolver is currently blocked waiting for
	int							_parseCursor;			// slot to begin searching for a file to parse
	int							_availableInputFiles;	// number of input fileinfos with readyToParse==true
#endif
	const char *				_exception;				// passes an exception message from parse thread to main thread
	int							_remainingInputFiles;	// number of input files still to parse
	
	ld::File::Ordinal			_indirectDylibOrdinal;
	ld::File::Ordinal			_linkerOptionOrdinal;
    
    class LibraryInfo {
        ld::File* _lib;
        bool      _isDylib;
    public:
        LibraryInfo(ld::dylib::File* dylib) : _lib(dylib), _isDylib(true) {};
        LibraryInfo(ld::archive::File* dylib) : _lib(dylib), _isDylib(false) {};

        bool isDylib() const { return _isDylib; }
        ld::dylib::File *dylib() const { return (ld::dylib::File*)_lib; }
        ld::archive::File *archive() const { return (ld::archive::File*)_lib; }
    };
    std::vector<LibraryInfo>  _searchLibraries;
};

} // namespace tool 
} // namespace ld 

#endif // __INPUT_FILES_H__
