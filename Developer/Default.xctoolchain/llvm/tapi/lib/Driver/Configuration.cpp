//===- tapi/Core/Configuration.cpp - Configuration --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the configuration query functions.
///
//===----------------------------------------------------------------------===//

#include "tapi/Driver/Configuration.h"
#include "tapi/Driver/ConfigurationFile.h"
#include "tapi/Driver/HeaderGlob.h"
#include "tapi/Core/Context.h"
#include "tapi/Core/FileManager.h"
#include "tapi/Core/HeaderFile.h"
#include "tapi/Core/Path.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "llvm/BinaryFormat/Magic.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Process.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

static std::string getCanonicalPath(StringRef path) {
  SmallVector<char, PATH_MAX> fullPath(path.begin(), path.end());
  llvm::sys::path::remove_dots(fullPath, /*remove_dot_dot=*/true);
  return std::string(fullPath.begin(), fullPath.end());
}

static std::string getFullPath(StringRef path,
                               StringRef base) {
  SmallString<PATH_MAX> temp(base);
  llvm::sys::path::append(temp, path);
  llvm::sys::fs::make_absolute(temp);
  return getCanonicalPath(temp);
}

static PathSeq updatePathSeq(const PathSeq &paths, StringRef root) {
  PathSeq headers;
  for (auto &p : paths) {
    auto path = getFullPath(p, root);
    headers.emplace_back(path);
  }
  return headers;
}

PathSeq Configuration::updateDirectories(StringRef frameworkPath,
                                         const PathSeq &paths) const {
  PathSeq headers;
  if (!useOverlay(frameworkPath))
    headers = updatePathSeq(paths, rootPath);
  PathSeq headersFromSDK = updatePathSeq(paths, getSysRoot());
  headers.insert(headers.end(), headersFromSDK.begin(), headersFromSDK.end());
  return headers;
}

PathSeq Configuration::updateSDKHeaderFiles(const PathSeq &paths) const {
  PathSeq headers;
  for (auto &p : paths) {
    auto path = getFullPath(p, getSysRoot());
    if (context.getFileManager().isDirectory(path, /*CacheFailure=*/false)) {
      auto result = enumerateHeaderFiles(context.getFileManager(), path);
      if (!result) {
        context.getDiag().report(diag::err)
            << path << toString(result.takeError());
        return headers;
      }
      for (auto &path : *result)
        headers.emplace_back(path);
    } else
      headers.emplace_back(path);
  }
  return headers;
}

static bool isMachOBinary(StringRef path) {
  llvm::file_magic magic;
  auto ec = identify_magic(path, magic);
  if (ec)
    return false;

  switch(magic) {
  case llvm::file_magic::macho_dynamically_linked_shared_lib:
  case llvm::file_magic::macho_dynamically_linked_shared_lib_stub:
  case llvm::file_magic::macho_universal_binary:
    return true;
  default:
    return false;
  }
}

PathSeq Configuration::updateBinaryFiles(const PathSeq &paths) const {
  PathSeq binaries;
  for (auto &p : paths) {
    auto path = getFullPath(p, rootPath);
    if (context.getFileManager().isDirectory(path, /*CacheFailure=*/false)) {
      auto result =
          enumerateFiles(context.getFileManager(), path, isMachOBinary);
      if (!result) {
        context.getDiag().report(diag::err)
            << path << toString(result.takeError());
        return binaries;
      }
      for (auto &path : *result)
        binaries.emplace_back(path);
    } else
      binaries.emplace_back(path);
  }
  return binaries;
}

void Configuration::setConfiguration(ConfigurationFile &&configFile,
                                     Context &context) {
  file = std::move(configFile);
  pathToConfig.clear();

  for (auto &conf : file.frameworkConfigurations) {
    pathToConfig.emplace(conf.path, &conf);
    conf.frameworkPaths.insert(conf.frameworkPaths.end(),
                               file.frameworkPaths.begin(),
                               file.frameworkPaths.end());

    conf.macros.insert(conf.macros.end(), file.macros.begin(),
                       file.macros.end());
  }

  isDriverKit = llvm::sys::Process::GetEnv("DRIVERKIT").value_or("") == "1";

  // Get the project name from environment.
  if (projectName.empty())
    return;

  StringRef projName(projectName);
  // If the project name ends with _iosmac, set the default to iosmac.
  isiOSMac = projName.endswith("_iosmac");
  isDriverKit |= projName.endswith("_driverkit");

  // Find the project setting from configuration file.
  // If there is setting for the project, update them as commandline options.
  for (auto &conf : file.projectConfigurations) {
    if (projName == conf.name) {
      projectConfig.reset(new configuration::v1::ProjectConfiguration(conf));
      break;
    }
    // Handle the forked version.
    auto altName = projName.split('_');
    if (altName.second == conf.name) {
      projectConfig.reset(new configuration::v1::ProjectConfiguration(conf));
      break;
    }
  }

  // For projects end with "_Sim", try search without suffix.
  if (!projectConfig && projName.consume_back("_Sim")) {
    for (auto &conf : file.projectConfigurations) {
      if (projName.endswith(conf.name)) {
        projectConfig.reset(new configuration::v1::ProjectConfiguration(conf));
        break;
      }
    }
  }
}

std::string Configuration::getSysRoot() const {
  return !commandLine.isysroot.empty() ? commandLine.isysroot : file.isysroot;
}

clang::Language Configuration::getLanguage(StringRef path) const {
  if (commandLine.language != clang::Language::Unknown)
    return commandLine.language;

  auto it = pathToConfig.find(path.str());
  if (it != pathToConfig.end())
    return it->second->language;

  if (projectConfig)
    return projectConfig->language;

  // DriverKit is c++.
  if (isDriverKitProject())
    return clang::Language::CXX;

  return file.language;
}

std::string Configuration::getLanguageStd() const {
  if (!commandLine.std.empty())
    return commandLine.std;

  if (projectConfig)
    return projectConfig->languageStd;

  return "";
}

template <typename T>
void insertElements(T &base, const T &elements) {
  base.insert(base.end(), elements.begin(), elements.end());
}

std::vector<Macro> Configuration::getMacros(StringRef path) const {
  std::vector<Macro> macros;
  if (!commandLine.macros.empty())
    insertElements(macros, commandLine.macros);

  auto it = pathToConfig.find(path.str());
  if (it != pathToConfig.end())
    insertElements(macros, it->second->macros);

  if (projectConfig)
    insertElements(macros, projectConfig->macros);

  insertElements(macros, file.macros);

  return macros;
}

PathSeq Configuration::getIncludePaths(StringRef path) const {
  PathSeq includePaths;
  if (!commandLine.includePaths.empty())
    insertElements(includePaths, commandLine.includePaths);

  if (projectConfig) {
    auto projectIncludes = updateDirectories(path, projectConfig->includePaths);
    insertElements(includePaths, projectIncludes);
  }

  auto it = pathToConfig.find(path.str());
  if (it != pathToConfig.end()) {
    auto frameworkIncludes = updateDirectories(path, it->second->includePaths);
    insertElements(includePaths, frameworkIncludes);
  }

  auto globalIncludes = updateDirectories(path, file.includePaths);
  insertElements(includePaths, globalIncludes);

  return includePaths;
}

PathSeq Configuration::getFrameworkPaths(StringRef path) const {
  PathSeq frameworkPaths;
  if (!commandLine.frameworkPaths.empty())
    insertElements(frameworkPaths, commandLine.frameworkPaths);

  if (projectConfig) {
    auto projectFrameworks =
        updateDirectories(path, projectConfig->frameworkPaths);
    insertElements(frameworkPaths, projectFrameworks);
  }

  auto it = pathToConfig.find(path.str());
  if (it != pathToConfig.end()) {
    auto frameworkFrameworks =
        updateDirectories(path, it->second->frameworkPaths);
    insertElements(frameworkPaths, frameworkFrameworks);
  }

  auto globalFrameworks = updateDirectories(path, file.frameworkPaths);
  insertElements(frameworkPaths, globalFrameworks);

  return frameworkPaths;
}

PathSeq Configuration::getExtraHeaders(StringRef path, HeaderType type) const {
  if (type == HeaderType::Public) {
    if (!commandLine.extraPublicHeaders.empty())
      return commandLine.extraPublicHeaders;
  } else {
    assert(type == HeaderType::Private && "Unexpected header type.");
    if (!commandLine.extraPrivateHeaders.empty())
      return commandLine.extraPrivateHeaders;
  }
  
  if (projectConfig) {
    if (type == HeaderType::Public)
      return updateSDKHeaderFiles(
          projectConfig->publicHeaderConfiguration.includes);
    else
      return updateSDKHeaderFiles(
          projectConfig->privateHeaderConfiguration.includes);
  }

  auto it = pathToConfig.find(path.str());
  if (it == pathToConfig.end())
    return {};

  if (type == HeaderType::Public)
    return updateSDKHeaderFiles(it->second->publicHeaderConfiguration.includes);

  return updateSDKHeaderFiles(it->second->privateHeaderConfiguration.includes);
}

PathSeq Configuration::getPreIncludedHeaders(StringRef path,
                                             HeaderType type) const {
  PathSeq headers;
  if (projectConfig) {
    if (type == HeaderType::Public)
      insertElements(headers,
                     projectConfig->publicHeaderConfiguration.preIncludes);
    else
      insertElements(headers,
                     projectConfig->privateHeaderConfiguration.preIncludes);
  }

  auto it = pathToConfig.find(path.str());
  if (it != pathToConfig.end()) {
    if (type == HeaderType::Public)
      insertElements(headers,
                     it->second->publicHeaderConfiguration.preIncludes);
    else
      insertElements(headers,
                     it->second->privateHeaderConfiguration.preIncludes);
  }
  return headers;
}

PathSeq Configuration::getExcludedHeaders(StringRef path,
                                          HeaderType type) const {
  PathSeq excludePaths;
  if (type == HeaderType::Public)
    insertElements(excludePaths, commandLine.excludePublicHeaders);
  else {
    assert(type == HeaderType::Private && "Unexpected header type.");
    insertElements(excludePaths, commandLine.excludePrivateHeaders);
  }

  if (projectConfig) {
    if (type == HeaderType::Public)
      insertElements(excludePaths,
                     projectConfig->publicHeaderConfiguration.excludes);
    else
      insertElements(excludePaths,
                     projectConfig->privateHeaderConfiguration.excludes);
  }

  auto it = pathToConfig.find(path.str());
  if (it != pathToConfig.end()) {
    if (type == HeaderType::Public)
      insertElements(excludePaths,
                     it->second->publicHeaderConfiguration.excludes);
    else
      insertElements(excludePaths,
                     it->second->privateHeaderConfiguration.excludes);
  }

  return excludePaths;
}

std::string Configuration::getUmbrellaHeader(StringRef path,
                                             HeaderType type) const {
  if (type == HeaderType::Public) {
    if (!commandLine.publicUmbrellaHeaderPath.empty())
      return commandLine.publicUmbrellaHeaderPath;
  } else {
    assert(type == HeaderType::Private && "Unexpected header type.");
    if (!commandLine.privateUmbrellaHeaderPath.empty())
      return commandLine.privateUmbrellaHeaderPath;
  }

  if (projectConfig) {
    if (type == HeaderType::Public)
      return projectConfig->publicHeaderConfiguration.umbrellaHeader;

    return projectConfig->privateHeaderConfiguration.umbrellaHeader;
  }

  auto it = pathToConfig.find(path.str());
  if (it == pathToConfig.end())
    return {};

  if (type == HeaderType::Public)
    return it->second->publicHeaderConfiguration.umbrellaHeader;

  return it->second->privateHeaderConfiguration.umbrellaHeader;
}

bool Configuration::isiOSMacProject() const {
  return isiOSMac || (projectConfig && projectConfig->isiOSMac);
}
bool Configuration::isZipperedProject() const {
  return projectConfig && projectConfig->isZippered;
}

bool Configuration::useOverlay(StringRef path) const {
  auto it = pathToConfig.find(path.str());
  if (it != pathToConfig.end())
    return it->second->useOverlay;

  if (projectConfig)
    return projectConfig->useOverlay;

  return true; // true by default.
}

bool Configuration::useSplitHeaderDir() const {
  if (projectConfig)
    return projectConfig->useSplitHeaderDir;

  return false;
}

bool Configuration::useUmbrellaOnly() const {
  return projectConfig && projectConfig->useUmbrellaOnly;
}

PathSeq Configuration::getRootMaskPaths() const {
  if (projectConfig)
    return projectConfig->rootMaskPaths;
  return PathSeq();
}

PathSeq Configuration::getSDKMaskPaths() const {
  if (projectConfig)
    return projectConfig->sdkMaskPaths;
  return PathSeq();
}

std::vector<std::string>
Configuration::getClangExtraArgs(StringRef path) const {
  std::vector<std::string> clangExtraArgs = commandLine.clangExtraArgs;
  auto it = pathToConfig.find(path.str());
  if (it != pathToConfig.end())
    llvm::append_range(clangExtraArgs, it->second->clangExtraArgs);
  if (projectConfig)
    llvm::append_range(clangExtraArgs, projectConfig->clangExtraArgs);
  return clangExtraArgs;
}

bool Configuration::isPromotedToPublicDylib(StringRef installName) const {
  auto result = llvm::find_if(file.publicDylibs, [&](StringRef glob) {
    if (installName == glob)
      return true;
    auto regex = createRegexFromGlob(glob);
    if (!regex) {
      consumeError(regex.takeError());
      return false;
    }
    return regex->match(installName);
  });
  return result != file.publicDylibs.end();
}

bool Configuration::scanPublicHeadersInSDKContentRoot() const {
  if (projectConfig)
    return projectConfig->scanPublicHeadersInSDKContentRoot;
  return false;
}

bool Configuration::ignoreExistingPartialSDKDBs() const {
  if (projectConfig)
    return projectConfig->ignoreExistingPartialSDKDBs;
  return false;
}

TAPI_NAMESPACE_INTERNAL_END
