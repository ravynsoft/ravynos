//===--- tapi/Driver/DriverUtils.h - TAPI Driver Utilities ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef TAPI_DRIVER_DRIVER_UTILS_H
#define TAPI_DRIVER_DRIVER_UTILS_H

#include "tapi/Core/HeaderFile.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/Path.h"
#include "tapi/Defines.h"
#include <vector>

namespace clang {
class FileEntry;
}

TAPI_NAMESPACE_INTERNAL_BEGIN

class FileManager;
class DiagnosticsEngine;

bool findAndAddHeaderFiles(HeaderSeq &headersOut, FileManager &fm,
                           DiagnosticsEngine &diag, PathSeq headersIn,
                           HeaderType type, StringRef sysroot,
                           StringRef basePath);

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_DRIVER_DRIVER_UTILS_H
