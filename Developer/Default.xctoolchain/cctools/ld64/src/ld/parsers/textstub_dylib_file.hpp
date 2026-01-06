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

#ifndef __TEXTSTUB_DYLIB_FILE_H__
#define __TEXTSTUB_DYLIB_FILE_H__

#include "ld.hpp"
#include "Options.h"

namespace textstub {
namespace dylib {


extern ld::dylib::File* parse(const uint8_t* fileContent, uint64_t fileLength, const char* path,
							  time_t modTime, const Options& opts, ld::File::Ordinal ordinal,
							  bool bundleLoader, bool indirectDylib, bool fromSDK);

extern ld::dylib::File *parse(const char *path, tapi::LinkerInterfaceFile* file, time_t modTime,
                              ld::File::Ordinal ordinal, const Options& opts, bool indirectDylib, bool fromSDK);

extern bool isTextStubFile(const uint8_t* fileContent, uint64_t fileLength, const char* path);

} // namespace dylib
} // namespace textstub


#endif // __TEXTSTUB_DYLIB_FILE_H__
