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

#ifndef __MACHO_RELOCATABLE_FILE_H__
#define __MACHO_RELOCATABLE_FILE_H__

#include "ld.hpp"
#include "Options.h"

namespace mach_o {
namespace relocatable {

struct ParserOptions {
	uint32_t		architecture;
	bool			objSubtypeMustMatch;
	bool			logAllFiles;
	bool			warnUnwindConversionProblems;
	bool			keepDwarfUnwind;
	bool			forceDwarfConversion;
	bool			neverConvertDwarf;
	bool			verboseOptimizationHints;
	bool			armUsesZeroCostExceptions;
#if SUPPORT_ARCH_arm64e
	bool			supportsAuthenticatedPointers;
#endif
	uint32_t		subType;
	ld::VersionSet  platforms;
	ld::relocatable::File::SourceKind	srcKind;
	bool			treateBitcodeAsData;
	bool			usingBitcode;
	uint8_t			maxDefaultCommonAlignment;
	bool			internalSDK;
	bool			forceHidden;
	bool			platformMismatchesAreWarning;
};

extern ld::relocatable::File* parse(const uint8_t* fileContent, uint64_t fileLength, 
									const char* path, time_t modTime, ld::File::Ordinal ordinal, 
									const ParserOptions& opts);
									
extern bool isObjectFile(const uint8_t* fileContent, uint64_t fileLength, const ParserOptions& opts);

extern bool isObjectFile(const uint8_t* fileContent, uint64_t fileLength, cpu_type_t* result, cpu_subtype_t* subResult, ld::VersionSet& platformsFound);

extern bool hasObjC2Categories(const uint8_t* fileContent);					

extern bool hasObjC1Categories(const uint8_t* fileContent);

extern const char* archName(const uint8_t* fileContent);

bool getNonLocalSymbols(const uint8_t* fileContent, std::vector<const char*> &syms);

} // namespace relocatable
} // namespace mach_o


#endif // __MACHO_RELOCATABLE_FILE_H__
