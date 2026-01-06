//===- lib/Core/Framework.cpp - Framework Context ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the Framework context.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/Framework.h"
#include "tapi/Core/HeaderFile.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <set>

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

SwiftModule::SwiftModule(StringRef path) {
  auto filename = sys::path::filename(path);
  if (filename.consume_back(".swiftmodule"))
    name = filename.str();
  else if (auto n = filename.consume_back(".swiftinterface"))
    name = filename.str();
  else
    llvm_unreachable("unexpected file extension");
}

const Regex Rule("(.+)/(.+)\\.framework/");
StringRef Framework::getNameFromInstallName(StringRef installName) {
  SmallVector<StringRef, 3> match;
  Rule.match(installName, &match);
  if (match.empty())
    return "";
  return match.back();
}

StringRef Framework::getName() const {
  StringRef path = _baseDirectory;
  // Returns the framework name extract from path.
  while (!path.empty()) {
    if (path.ends_with(".framework"))
      return sys::path::filename(path);
    path = sys::path::parent_path(path);
  }

  // Otherwise, return the name of the baseDirectory.
  // First, remove all the trailing seperator.
  path = _baseDirectory;
  return sys::path::filename(path.rtrim("/"));
}

bool Framework::isMacCatalyst() const {
  return StringRef(_baseDirectory).contains(MACCATALYST_PREFIX_PATH "/");
}

bool Framework::isDriverKit() const {
  return StringRef(_baseDirectory).contains(DRIVERKIT_PREFIX_PATH "/");
}

StringRef Framework::getAdditionalIncludePath() const {
  if (isSysRoot || !isDynamicLibrary)
    return StringRef();

  return _baseDirectory;
}

StringRef Framework::getAdditionalFrameworkPath() const {
  if (isSysRoot || isDynamicLibrary)
    return StringRef();

  auto parentPath = sys::path::parent_path(_baseDirectory);
  if (sys::path::filename(_baseDirectory).ends_with(".framework"))
    return parentPath;

  // For macOS legacy style framework layout.
  if (parentPath.ends_with("Versions"))
    return sys::path::parent_path(sys::path::parent_path(parentPath));

  return StringRef();
}

void Framework::addDynamicLibraryFile(StringRef path) {
  // Check if the dynamic library is a build variant of the main framework
  // library (e.g. _debug or _profile) and do not add it to the list of
  // binaries to be scanned by SDKDB. This removes unnecessary duplicates from
  // the database.
  static const Regex Rule("(.+)/(.+)\\.framework/(.+)(_.+)");
  SmallVector<StringRef, 5> match;
  Rule.match(path, &match);
  if ((match.size() == 5) && match[2] == match[3])
    return;

  _dynamicLibraryFiles.emplace_back(path);
}

TAPI_NAMESPACE_INTERNAL_END
