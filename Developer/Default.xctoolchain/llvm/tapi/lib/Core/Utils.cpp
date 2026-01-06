//===- lib/Core/Utils.cpp - TAPI Utility Methods ----------------*- C++ -*-===//
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

#include "tapi/Core/Utils.h"
#include "tapi/Core/API.h"
#include "tapi/Core/API2SymbolConverter.h"
#include "tapi/Core/FileManager.h"
#include "tapi/Core/Path.h"
#include "tapi/Defines.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Path.h"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

static StringRef consumeSDKPrefixes(StringRef path) {
  // Remove SPLAT prefix first (can contain iOSSupport)
  path.consume_front(CRYPTEXES_PREFIX_PATH);

  // Remove the iOSSupport/DriverKit prefix to identify SDK locations inside
  // the iOSSupport/DriverKit directory.
  path.consume_front(MACCATALYST_PREFIX_PATH);
  path.consume_front(DRIVERKIT_PREFIX_PATH);

  // Also /Library/Apple prefix for ROSP.
  path.consume_front("/Library/Apple");

  return path;
}

bool isSDKDylib(StringRef installName) {
  return StringSwitch<bool>(consumeSDKPrefixes(installName))
      .StartsWith("/usr/lib/", true)
      .StartsWith("/usr/local/", true)
      .StartsWith("/System/Library/Frameworks/", true)
      .StartsWith("/System/Library/PrivateFrameworks/", true)
      .Default(false);
}

bool isPublicDylib(StringRef installName) {
  installName = consumeSDKPrefixes(installName);

  // Everything in /usr/lib/swift (including sub-directories) is now considered
  // public.
  if (installName.consume_front("/usr/lib/swift/"))
    return true;

  // Only libraries directly in /usr/lib are public. All other libraries in
  // sub-directories (such as /usr/lib/system) are considered private.
  if (installName.consume_front("/usr/lib/")) {
    if (installName.contains('/'))
      return false;
    return true;
  }

  // /System/Library/Frameworks/ is a public location
  if (installName.consume_front("/System/Library/Frameworks/")) {
    StringRef name, rest;
    std::tie(name, rest) = installName.split('.');

    // but only top level framework
    // /System/Library/Frameworks/Foo.framework/Foo ==> true
    // /System/Library/Frameworks/Foo.framework/Versions/A/Foo ==> true
    // /System/Library/Frameworks/Foo.framework/Resources/libBar.dylib ==> false
    // /System/Library/Frameworks/Foo.framework/Frameworks/Bar.framework/Bar
    // ==> false
    // /System/Library/Frameworks/Foo.framework/Frameworks/XFoo.framework/XFoo
    // ==> false
    if (rest.startswith("framework/") && (sys::path::filename(rest) == name))
      return true;

    return false;
  }

  return false;
}

bool isWithinPublicLocation(StringRef path) {
  path = consumeSDKPrefixes(path);

  if (path.startswith("/usr/local/") || 
      path.startswith("/System/Library/PrivateFrameworks/"))
    return false;

  if (path.consume_front("/System/Library/Frameworks/")) {
    // Exclude everything from PrivateHeaders.
    while (!path.empty()) {
      auto split = path.split('/');
      if (split.first == "PrivateHeaders")
        return false;
      path = split.second;
    }
  }

  return true;
}

bool isHeaderFile(StringRef path) {
  return StringSwitch<bool>(sys::path::extension(path))
      .Cases(".h", ".H", ".hh", ".hpp", ".hxx", true)
      .Default(false);
}

std::string findLibrary(StringRef installName, FileManager &fm,
                        ArrayRef<std::string> frameworkSearchPaths,
                        ArrayRef<std::string> librarySearchPaths,
                        ArrayRef<std::string> searchPaths) {
  auto filename = sys::path::filename(installName);
  bool isFramework = sys::path::parent_path(installName)
                         .endswith((filename + ".framework").str());

  if (isFramework) {
    for (const auto &path : frameworkSearchPaths) {
      SmallString<PATH_MAX> fullPath(path);
      sys::path::append(fullPath, filename + StringRef(".framework"), filename);

      SmallString<PATH_MAX> tbdPath = fullPath;
      TAPI_INTERNAL::replace_extension(tbdPath, ".tbd");
      if (fm.exists(tbdPath))
        return tbdPath.str().str();

      if (fm.exists(fullPath))
        return fullPath.str().str();
    }
  } else {
    // Copy ld64's behavior: If this is a .dylib inside a framework, do not
    // search -L paths.
    bool embeddedDylib = (sys::path::extension(installName) == ".dylib") &&
                         installName.contains(".framework/");
    if (!embeddedDylib) {
      for (const auto &path : librarySearchPaths) {
        SmallString<PATH_MAX> fullPath(path);
        sys::path::append(fullPath, filename);

        SmallString<PATH_MAX> tbdPath = fullPath;
        TAPI_INTERNAL::replace_extension(tbdPath, ".tbd");

        if (fm.exists(tbdPath))
          return tbdPath.str().str();

        if (fm.exists(fullPath))
          return fullPath.str().str();
      }
    }
  }

  for (const auto &path : searchPaths) {
    SmallString<PATH_MAX> fullPath(path);
    sys::path::append(fullPath, installName);

    SmallString<PATH_MAX> tbdPath = fullPath;
    TAPI_INTERNAL::replace_extension(tbdPath, ".tbd");

    if (fm.exists(tbdPath))
      return tbdPath.str().str();

    if (fm.exists(fullPath))
      return fullPath.str().str();
  }

  return std::string();
}

namespace {

std::unique_ptr<InterfaceFile> createInterfaceFile(const APIs &apis,
                                                   StringRef installName) {
  // Pickup symbols first.
  auto symbols = std::make_unique<SymbolSet>();
  for (auto &api : apis) {
    auto libName = api->getInstallName();
    if (!libName || *libName != installName)
      continue;
    bool includeUndefs =
        api->hasBinaryInfo() && !api->getBinaryInfo().isTwoLevelNamespace;
    const auto target = Target(api->getTarget());
    API2SymbolConverter converter(symbols.get(), target, includeUndefs);
    api->visit(converter);
  }

  auto file = std::make_unique<InterfaceFile>(std::move(symbols));
  // Assign other attributes.
  for (auto &api : apis) {
    auto libName = api->getInstallName();
    if (!libName || *libName != installName)
      continue;
    const auto target = Target(api->getTarget());
    file->addTarget(target);
    if (!api->hasBinaryInfo())
      continue;
    auto &binaryInfo = api->getBinaryInfo();
    file->setFileType(binaryInfo.fileType);
    if (binaryInfo.isAppExtensionSafe)
      file->setApplicationExtensionSafe();
    if (binaryInfo.isTwoLevelNamespace)
      file->setTwoLevelNamespace();
    if (binaryInfo.isOSLibNotForSharedCache)
      file->setOSLibNotForSharedCache();
    file->setCurrentVersion(binaryInfo.currentVersion);
    file->setCompatibilityVersion(binaryInfo.compatibilityVersion);
    file->addParentUmbrella(target, binaryInfo.parentUmbrella);
    file->setSwiftABIVersion(binaryInfo.swiftABIVersion);
    file->setPath(binaryInfo.path);
    if (!binaryInfo.installName.empty())
      file->setInstallName(binaryInfo.installName);
    for (const auto &client : binaryInfo.allowableClients)
      file->addAllowableClient(client, target);
    for (const auto &lib : binaryInfo.reexportedLibraries)
      file->addReexportedLibrary(lib, target);
  }

  return file;
}

} // namespace

std::unique_ptr<InterfaceFile> convertToInterfaceFile(const APIs &apis) {

  auto file = std::make_unique<InterfaceFile>();
  if (apis.empty())
    return file;

  llvm::SetVector<StringRef> installNames;
  for (auto &api : apis) {
    auto libOr = api->getInstallName();
    if (!libOr)
      continue;
    installNames.insert(*libOr);
  }

  file = createInterfaceFile(apis, *installNames.begin());
  for (auto it = std::next(installNames.begin()); it != installNames.end();
       ++it)
    file->addDocument(createInterfaceFile(apis, *it));

  return file;
}

TAPI_NAMESPACE_INTERNAL_END
