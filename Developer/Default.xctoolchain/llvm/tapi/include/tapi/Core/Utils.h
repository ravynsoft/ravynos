//===- tapi/Core/Utils.h - TAPI Utility Methods -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Misc utility methods.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_UTILS_H
#define TAPI_CORE_UTILS_H

#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Process.h"
#include "llvm/TextAPI/InterfaceFile.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class FileManager;
class API;

#define MACCATALYST_PREFIX_PATH "/System/iOSSupport"
#define DRIVERKIT_PREFIX_PATH "/System/DriverKit"
#define CRYPTEXES_PREFIX_PATH "/System/Cryptexes/OS"

/// Return true if the install name of the dylib is in a public SDK location.
bool isPublicDylib(StringRef installName);

/// Return true if the install name is in an SDK location (public or private).
bool isSDKDylib(StringRef installName);

/// Return true if the path is not in any known private SDK location.
/// To determine whether a library's binary is public, use `isPublicDylib`
/// instead.
///
bool isWithinPublicLocation(StringRef path);

/// Return true if the path has extension of a header.
bool isHeaderFile(StringRef path);

/// Search the path to find the library specificed by installName.
std::string findLibrary(StringRef installName, FileManager &fm,
                        ArrayRef<std::string> frameworkSearchPaths,
                        ArrayRef<std::string> librarySearchPaths,
                        ArrayRef<std::string> searchPaths);

/// Determine if tapi is running in a B&I context.
inline bool inBnIEnvironment() {
  if (auto isBnI = llvm::sys::Process::GetEnv("RC_XBS"))
    return (isBnI.value() == "YES") &&
           (!llvm::sys::Process::GetEnv("RC_BUILDIT"));
  return false;
}

using APIs = llvm::SmallVector<std::shared_ptr<API>, 4>;
std::unique_ptr<InterfaceFile> convertToInterfaceFile(const APIs &apis);

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_UTILS_H
