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

#ifndef __LTO_FILE_H__
#define __LTO_FILE_H__

#include "ld.hpp"

namespace lto {

extern const char* version();

extern unsigned int runtime_api_version();

extern unsigned int static_api_version();

void set_library(const char *dylib);

extern bool libLTOisLoaded();

extern const char* archName(const uint8_t* fileContent, uint64_t fileLength);

extern bool isObjectFile(const uint8_t* fileContent, uint64_t fileLength, cpu_type_t architecture, cpu_subtype_t subarch);

extern bool hasObjCCategory(const uint8_t* fileContent, uint64_t fileLength);

extern ld::relocatable::File* parse(const uint8_t* fileContent, uint64_t fileLength,
									const char* path, time_t modTime, ld::File::Ordinal ordinal,
									cpu_type_t architecture, cpu_subtype_t subarch, bool logAllFiles,
									bool verboseOptimizationHints);

struct OptimizeOptions {
	const char*							outputFilePath;
	const char*							tmpObjectFilePath;
	const char*							ltoCachePath;
	int									ltoPruneInterval;
	int									ltoPruneAfter;
	unsigned							ltoMaxCacheSize;
	bool								ltoPruneIntervalOverwrite;
	bool								preserveAllGlobals;
	bool								verbose; 
	bool								saveTemps; 
	bool								ltoCodegenOnly;
	bool								pie; 
	bool								mainExecutable; 
	bool								staticExecutable; 
	bool								preload;
	bool								relocatable;
	bool								allowTextRelocs; 
	bool								linkerDeadStripping; 
	bool								needsUnwindInfoSection; 
	bool								keepDwarfUnwind; 
	bool								verboseOptimizationHints;
	bool								armUsesZeroCostExceptions;
	bool								simulator;
	bool								internalSDK;
#if SUPPORT_ARCH_arm64e
	bool								supportsAuthenticatedPointers;
#endif
	bool								bitcodeBundle;
	uint8_t								maxDefaultCommonAlignment;
	cpu_type_t							arch;
	const char*							mcpu;
	ld::VersionSet						platforms;
	const std::vector<const char*>*		llvmOptions;
	const std::vector<const char*>*		initialUndefines;
};

extern bool	optimize(   const std::vector<const ld::Atom*>&	allAtoms,
						ld::Internal&						state,
						const OptimizeOptions&				options,
						ld::File::AtomHandler&				handler,
						std::vector<const ld::Atom*>&		newAtoms, 
						std::vector<const char*>&			additionalUndefines);
						
} // namespace lto


#endif // __LTO_FILE_H__


