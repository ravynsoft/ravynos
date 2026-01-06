/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-*
 *
 * Copyright (c) 2022 Apple Inc. All rights reserved.
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
#include <string.h>
#include <cxxabi.h>

#include "Mangling.h"

// from FunctionNameDemangle.h
extern "C" size_t fnd_get_demangled_name(const char *mangledName, char *outputBuffer, size_t length);

const char* demangleSymbol(const char* sym) {
	static size_t size = 1024;
	static char* buff = (char*)malloc(size);

#if DEMANGLE_SWIFT
	// only try to demangle symbols that look like Swift symbols
	if ( strncmp(sym, "_$", 2) == 0 ) {
		size_t demangledSize = fnd_get_demangled_name(&sym[1], buff, size);
		if ( demangledSize > size ) {
			size = demangledSize+2;
			buff = (char*)realloc(buff, size);
			demangledSize = fnd_get_demangled_name(&sym[1], buff, size);
		}
		if ( demangledSize != 0 )
			return buff;
	}
#endif

	// only try to demangle symbols that look like C++ symbols
	if ( !resemblesMangledCppSymbol(sym) )
		return sym;

	int status;
	char* result = abi::__cxa_demangle(&sym[1], buff, &size, &status);
	if ( result != NULL ) {
		// if demangling successful, keep buffer for next demangle
		buff = result;
		return buff;
	}
	return sym;
}

bool resemblesMangledCppSymbol(const char* sym) {
	return strncmp(sym, "__Z", 3) == 0;
}
