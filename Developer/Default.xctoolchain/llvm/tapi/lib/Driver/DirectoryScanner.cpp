//===- lib/Driver/DirectoryScanner.cpp - Directory Scanner ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the directory scanner.
///
/// Scans the directory for frameworks.
///
//===----------------------------------------------------------------------===//

#include "tapi/Driver/DirectoryScanner.h"
#include "tapi/Core/FileManager.h"
#include "tapi/Core/Framework.h"
#include "tapi/Core/HeaderFile.h"
#include "tapi/Core/Utils.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "clang/Basic/Diagnostic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/VirtualFileSystem.h"

using namespace llvm;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

static bool isFramework(StringRef path) {
  while (path.back() == '/')
    path = path.slice(0, path.size() - 1);

  return StringSwitch<bool>(sys::path::extension(path))
      .Case(".framework", true)
      .Default(false);
}

bool ScannerMode::scanBinaries() const {
  return mode != ScanPublicSDK && mode != ScanInternalSDK;
}
bool ScannerMode::scanBundles() const { return mode == ScanRuntimeRoot; }

bool ScannerMode::scanHeaders() const {
  return mode != ScanRuntimeRoot;
}

bool ScannerMode::scanPrivateHeaders() const {
  return mode != ScanPublicSDK;
}

bool ScannerMode::isRootLayout() const {
  return mode != ScanFrameworks && mode != ScanDylibs;
}

DirectoryScanner::DirectoryScanner(FileManager &fm, DiagnosticsEngine &diag,
                                   ScannerMode mode)
    : _fm(fm), diag(diag), mode(mode) {
  _registry.addBinaryReaders();
}

std::vector<Framework> DirectoryScanner::takeResult() {
  return std::move(frameworks);
}

Framework &DirectoryScanner::getOrCreateFramework(
    StringRef path, std::vector<Framework> &frameworks) const {
  if (path.consume_front(rootPath) && path.empty())
    path = "/";

  auto framework = find_if(
      frameworks, [path](const Framework &f) { return f.getPath() == path; });
  if (framework != frameworks.end())
    return *framework;

  frameworks.emplace_back(path);
  return frameworks.back();
}

bool DirectoryScanner::scanDylibDirectory(
    StringRef directory, std::vector<Framework> &frameworks) const {

  // Check some known sub-directory locations.
  auto getDirectory = [&](const char *subDirectory) {
    SmallString<PATH_MAX> path(directory);
    sys::path::append(path, subDirectory);
    return _fm.getDirectory(path, /*CacheFailure=*/false);
  };
  auto directoryEntryPublic = getDirectory("usr/include");
  auto directoryEntryPrivate = getDirectory("usr/local/include");

  if (!directoryEntryPublic && !directoryEntryPrivate) {
    diag.report(diag::err_cannot_find_header_dir) << directory;
    return false;
  }

  auto &dylib = getOrCreateFramework(directory, frameworks);
  dylib.isDynamicLibrary = true;

  if (directoryEntryPublic) {
    if (!scanHeaders(dylib, (*directoryEntryPublic)->getName(),
                     HeaderType::Public, directory))
      return false;
  }
  if (directoryEntryPrivate) {
    if (!scanHeaders(dylib, (*directoryEntryPrivate)->getName(),
                     HeaderType::Private, directory))
      return false;
  }

  return true;
}

bool DirectoryScanner::scanDirectory(StringRef directory) {
  rootPath = "";

  // We expect a certain directory structure and naming convention to find the
  // frameworks.
  static const char *subDirectories[] = {"System/Library/Frameworks/",
                                         "System/Library/PrivateFrameworks/"};

  // Check if the directory is already a framework.
  if (isFramework(directory)) {
    auto &framework = getOrCreateFramework(directory, frameworks);
    if (!scanFrameworkDirectory(framework, directory))
      return false;
    return true;
  }

  // Check some known sub-directory locations.
  for (const auto *subDirectory : subDirectories) {
    SmallString<PATH_MAX> path(directory);
    sys::path::append(path, subDirectory);

    if (!scanFrameworksDirectory(frameworks, path))
      return false;
  }

  return true;
}

/// \brief Scan the directory for frameworks.
bool DirectoryScanner::scanFrameworksDirectory(
    std::vector<Framework> &frameworks, StringRef directory) const {
  std::error_code ec;
  auto &fs = _fm.getVirtualFileSystem();
  for (vfs::directory_iterator i = fs.dir_begin(directory, ec), ie; i != ie;
       i.increment(ec)) {
    auto path = i->path();

    // Skip files that not exist. This usually happens for broken symlinks.
    if (ec == std::errc::no_such_file_or_directory) {
      ec.clear();
      continue;
    }

    if (ec) {
      diag.report(diag::err) << path << ec.message();
      return false;
    }

    if (_fm.isSymlink(path))
      continue;

    if (isFramework(path)) {
      if (!_fm.isDirectory(path, /*CacheFailure=*/false))
        continue;

      auto &framework = getOrCreateFramework(path, frameworks);
      if (!scanFrameworkDirectory(framework, path))
        return false;
    } else if (mode.scanBinaries() &&
               !_fm.isDirectory(path, /*CacheFailure*/ false)) {
      // Check for dynamic libs.
      auto result = isDynamicLibrary(path);
      if (!result) {
        diag.report(diag::err) << path << toString(result.takeError());
        return false;
      }

      auto &framework = getOrCreateFramework(path, frameworks);
      if (result.get())
        framework.addDynamicLibraryFile(path);
    }
  }

  return true;
}

bool DirectoryScanner::scanSubFrameworksDirectory(
    std::vector<Framework> &frameworks, StringRef path) const {
  if (_fm.isDirectory(path, /*CacheFailure=*/false))
    return scanFrameworksDirectory(frameworks, path);

  diag.report(diag::err_no_directory) << path;
  return false;
}

bool DirectoryScanner::scanFrameworkDirectory(Framework &framework,
                                              StringRef path) const {
  // Unfortunately we cannot identify symlinks in the VFS. We assume that if
  // there is a Versions directory, then we have symlinks and directly proceed
  // to the Versiosn folder.
  std::error_code ec;
  auto &fs = _fm.getVirtualFileSystem();

  // If the framework is inside Kernel or IOKit, scan headers in the different
  // directory separately.
  framework.isDynamicLibrary =
      path.contains("Kernel.framework") || path.contains("IOKit.framework");

  for (vfs::directory_iterator i = fs.dir_begin(path, ec), ie;
       i != ie; i.increment(ec)) {
    auto path = i->path();

    // Skip files that not exist. This usually happens for broken symlinks.
    if (ec == std::errc::no_such_file_or_directory) {
      ec.clear();
      continue;
    }

    if (ec) {
      diag.report(diag::err) << path << ec.message();
      return false;
    }

    if (_fm.isSymlink(path))
      continue;

    StringRef fileName = sys::path::filename(path);
    // Scan all "public" headers.
    if (fileName.compare("Headers") == 0) {
      if (!scanHeaders(framework, path, HeaderType::Public, path))
        return false;
      continue;
    }
    // Scan all "private" headers.
    if (fileName.compare("PrivateHeaders") == 0) {
      if (!scanHeaders(framework, path, HeaderType::Private, path))
        return false;
      continue;
    }
    // Scan for module maps.
    if (fileName.compare("Modules") == 0) {
      if (!scanModules(framework, path))
        return false;
      continue;
    }
    // Check for sub frameworks.
    if (fileName.compare("Frameworks") == 0) {
      if (!scanSubFrameworksDirectory(framework._subFrameworks, path))
        return false;
      continue;
    }
    // Check for versioned frameworks.
    if (fileName.compare("Versions") == 0) {
      if (!scanFrameworkVersionsDirectory(framework, path))
        return false;
      continue;
    }

    // If it is a directory, scan the directory to check for dynamic libs.
    if (_fm.isDirectory(path, /*CacheFailure=*/false)) {
      if (!scanLibraryDirectory(framework, path))
        return false;
      continue;
    }

    if (!mode.scanBinaries())
      continue;

    // Check for dynamic libs.
    auto result = isDynamicLibrary(path);
    if (!result) {
      diag.report(diag::err) << path << toString(result.takeError());
      return false;
    }

    if (result.get())
      framework.addDynamicLibraryFile(path);
  }

  return true;
}

bool DirectoryScanner::scanHeaders(Framework &framework, StringRef path,
                                   HeaderType type, StringRef basePath,
                                   StringRef parentPath) const {
  if (!mode.scanHeaders())
    return true;

  if (!mode.scanPrivateHeaders() && type == HeaderType::Private)
    return true;

  std::error_code ec;
  auto &fs = _fm.getVirtualFileSystem();
  std::vector<std::string> subDirectories;
  for (vfs::directory_iterator i = fs.dir_begin(path, ec), ie; i != ie;
       i.increment(ec)) {
    auto headerPath = i->path();
    if (ec) {
      diag.report(diag::err) << headerPath << ec.message();
      return false;
    }

    // Ignore tmp files from unifdef.
    auto filename = sys::path::filename(headerPath);
    if (filename.startswith("."))
      continue;

    if (_fm.isSymlink(headerPath))
      continue;

    // If it is a directory, remember the subdirectory.
    if (_fm.isDirectory(headerPath, /*CacheFailure=*/false))
      subDirectories.push_back(headerPath.str());

    if (!isHeaderFile(headerPath))
      continue;

    // Skip files that not exist. This usually happens for broken symlinks.
    if (fs.status(headerPath) == std::errc::no_such_file_or_directory)
      continue;

    auto relativePath =
        sys::path::relative_path(headerPath.drop_front(basePath.size()));

    auto includeName = createIncludeHeaderName(headerPath);
    framework.addHeaderFile(headerPath, type, relativePath,
                            includeName.has_value() ? includeName.value() : "");
  }

  // Go through the subdirectores.
  // Sort the sub-directory first since different file system might have 
  // different traverse order.
  llvm::sort(subDirectories);
  parentPath = parentPath.empty() ? path : parentPath;
  for (auto &dir : subDirectories) {
    if (useSplitHeaderDir) {
      auto &sub = getOrCreateFramework(dir, framework._subFrameworks);
      sub.isDynamicLibrary = framework.isDynamicLibrary;
      sub.isSysRoot = framework.isSysRoot;
      scanHeaders(sub, dir, type, dir, parentPath);
      continue;
    }
    scanHeaders(framework, dir, type, basePath, parentPath);
  }

  return true;
}

bool DirectoryScanner::scanModules(Framework &framework,
                                   StringRef _path) const {
  std::error_code ec;
  auto &fs = _fm.getVirtualFileSystem();
  for (vfs::directory_iterator i = fs.dir_begin(_path, ec), ie; i != ie;
       i.increment(ec)) {
    auto path = i->path();
    if (ec) {
      diag.report(diag::err) << path << ec.message();
      return false;
    }

    // Skip files that not exist. This usually happens for broken symlinks.
    if (fs.status(path) == std::errc::no_such_file_or_directory)
      continue;

    if (path.endswith(".swiftinterface")) {
      framework._swiftModules.emplace_back(path);
      framework._swiftModules.back().addSwiftInterface(path);
    } else if (path.endswith(".swiftmodule"))
      scanSwiftModules(framework, path);
    else if (path.endswith(".modulemap"))
      framework.addModuleMap(path);
  }

  return true;
}

bool DirectoryScanner::scanSwiftModules(Framework &framework,
                                        StringRef path) const {
  if (!_fm.isDirectory(path, /*CacheFailure=*/false))
    return false;

  // Skip symlinked Swift module directory.
  // Some frameworks also install a Framework.swiftmodule under /usr/lib/swift.
  // We don't need to scan it again.
  if (_fm.isSymlink(path))
    return false;

  framework._swiftModules.emplace_back(path);
  auto &module = framework._swiftModules.back();
  std::error_code ec;
  auto &fs = _fm.getVirtualFileSystem();
  for (vfs::recursive_directory_iterator i(fs, path, ec), ie; i != ie;
       i.increment(ec)) {
    auto f = i->path();
    if (ec) {
      diag.report(diag::err) << f << ec.message();
      return false;
    }

    if (fs.status(f) == std::errc::no_such_file_or_directory)
      continue;

    if (f.endswith(".swiftinterface") || f.endswith(".swiftmodule"))
      module.addSwiftInterface(f);
  }
  return true;
}

/// FIXME: How to handle versions? For now scan them separately as independent
/// frameworks.
bool DirectoryScanner::scanFrameworkVersionsDirectory(Framework &framework,
                                                      StringRef path) const {
  std::error_code ec;
  auto &fs = _fm.getVirtualFileSystem();
  for (vfs::directory_iterator i = fs.dir_begin(path, ec), ie; i != ie;
       i.increment(ec)) {
    auto path = i->path();

    // Skip files that not exist. This usually happens for broken symlinks.
    if (ec == std::errc::no_such_file_or_directory) {
      ec.clear();
      continue;
    }

    if (ec) {
      diag.report(diag::err) << path << ec.message();
      return false;
    }

    if (_fm.isSymlink(path))
      continue;

    // Each version is just a framework directory.
    if (!_fm.isDirectory(path, /*CacheFailure=*/false))
      continue;

    auto &version = getOrCreateFramework(path, framework._versions);
    if (!scanFrameworkDirectory(version, path))
      return false;
  }

  return true;
}

bool DirectoryScanner::scanLibraryDirectory(Framework &framework,
                                            StringRef path) const {
  std::error_code ec;
  auto &fs = _fm.getVirtualFileSystem();
  for (vfs::directory_iterator i = fs.dir_begin(path, ec), ie; i != ie;
       i.increment(ec)) {
    auto path = i->path();

    // Skip files that not exist. This usually happens for broken symlinks.
    if (ec == std::errc::no_such_file_or_directory) {
      ec.clear();
      continue;
    }

    if (ec) {
      diag.report(diag::err) << path << ec.message();
      return false;
    }

    if (_fm.isSymlink(path))
      continue;

    if (_fm.isDirectory(path, /*CacheFailure=*/false)) {
      scanLibraryDirectory(framework, path);
      continue;
    }

    if (!mode.scanBinaries())
      continue;

    // Check for dynamic libs.
    auto result = isDynamicLibrary(path);
    if (!result) {
      diag.report(diag::err) << path << toString(result.takeError());
      return false;
    }

    if (result.get())
      framework.addDynamicLibraryFile(path);
  }

  return true;
}

Expected<bool> DirectoryScanner::isDynamicLibrary(StringRef path) const {
  auto bufferOrErr = _fm.getBufferForFile(path);
  if (auto ec = bufferOrErr.getError()) {
    if (ec == std::errc::permission_denied) {
      diag.report(diag::warn_sdkdb_skip_file) << path << ec.message();
      return false;
    }
    return errorCodeToError(ec);
  }

  // Metal Libraries pretend to be MachOs, but they do not contain any
  // framework code that developers can use, so we will just skip them.
  if (path.endswith(".metallib"))
    return false;

  auto fileType = _registry.getFileType(*bufferOrErr.get());
  if (!fileType)
    return fileType;

  if (fileType.get() == FileType::MachO_DynamicLibrary ||
      fileType.get() == FileType::MachO_DynamicLibrary_Stub)
    return true;

  if (mode.scanBundles() && (fileType.get() == FileType::MachO_Bundle))
    return true;

  return false;
}

bool DirectoryScanner::scanSDKContent(StringRef directory) {
  // Build a Unix framework for information in /usr/include and /usr/lib
  rootPath = directory;
  auto &SDKFramework = getOrCreateFramework(directory, frameworks);
  SDKFramework.isSysRoot = true;
  SDKFramework.isDynamicLibrary = true;
  auto getDirectory = [](StringRef subDirectory, StringRef root) {
    SmallString<PATH_MAX> path(root);
    sys::path::append(path, subDirectory);
    return path;
  };

  auto scanHeaderAndLibrary = [&](StringRef directory) {
    SmallString<PATH_MAX> root(rootPath);
    sys::path::append(root, directory);

    // Scan headers.
    if (!scanHeaders(SDKFramework, getDirectory("usr/include", root),
                     HeaderType::Public, rootPath))
      return false;
    if (!scanHeaders(SDKFramework, getDirectory("usr/local/include", root),
                     HeaderType::Private, rootPath))
      return false;
    // Scan dylibs.
    if (!scanLibraryDirectory(SDKFramework, getDirectory("usr/lib", root)))
      return false;

    if (!scanLibraryDirectory(SDKFramework,
                              getDirectory("usr/local/lib", root)))
      return false;

    // Adding all the frameworks in /System.
    if (!scanFrameworksDirectory(
            SDKFramework._subFrameworks,
            getDirectory("System/Library/Frameworks", root)))
      return false;

    if (!scanFrameworksDirectory(
            SDKFramework._subFrameworks,
            getDirectory("System/Library/PrivateFrameworks", root)))
      return false;

    // scan swift modules.
    if (!scanModules(SDKFramework, getDirectory("usr/lib/swift", root)))
      return false;

    return true;
  };

  // Scan SDKRoot.
  if (!scanHeaderAndLibrary(""))
    return false;

  // Adding iOSSupport locations.
  if (!scanHeaderAndLibrary(MACCATALYST_PREFIX_PATH))
    return false;

  // Adding DriverKit locations.
  if (!scanHeaderAndLibrary(DRIVERKIT_PREFIX_PATH))
    return false;

  // On macOS, there is a special path for frameworks excluded from ROSP.
  if (!scanHeaderAndLibrary("Library/Apple"))
    return false;

  // On macOS and iOS, there is now a special path for SPLAT.
  if (!scanHeaderAndLibrary(CRYPTEXES_PREFIX_PATH))
    return false;
  if (!scanHeaderAndLibrary(CRYPTEXES_PREFIX_PATH MACCATALYST_PREFIX_PATH))
    return false;

  // Scan the bundles and extensions in /System/Library.
  std::error_code ec;
  auto &fs = _fm.getVirtualFileSystem();
  for (auto i = fs.dir_begin(getDirectory("System/Library", rootPath), ec);
       i != vfs::directory_iterator(); i.increment(ec)) {
    auto path = i->path();

    // Skip files that not exist. This usually happens for broken symlinks.
    if (ec == std::errc::no_such_file_or_directory) {
      ec.clear();
      continue;
    }

    if (ec) {
      diag.report(diag::err) << path << ec.message();
      return false;
    }

    // Skip framework directories that is handled above.
    if (path.endswith("Frameworks") || path.endswith("PrivateFrameworks"))
      continue;

    // Skip all that is not a directory.
    if (_fm.isDirectory(path, /*CacheFailure=*/false)) {
      auto &sub = getOrCreateFramework(path, SDKFramework._subFrameworks);
      if (!scanLibraryDirectory(sub, path))
        return false;
    }
  }

  return true;
}

bool DirectoryScanner::scan(StringRef directory) {
  if (mode.getMode() == ScannerMode::ScanFrameworks)
    return scanDirectory(directory);

  if (mode.getMode() == ScannerMode::ScanDylibs)
    return scanDylibDirectory(directory, frameworks);

  return scanSDKContent(directory);
}

static std::string removeVersionsFromPath(StringRef path) {
  // Search for /Versions in the path.
  auto components = path.rsplit("/Versions/");
  // Return the original path if there is no Versions component.
  if (components.second.empty())
    return path.str();

  // Construct new path by combining the path component without Versions/A.
  SmallString<PATH_MAX> newPath(components.first);
  auto second = components.second.split("/").second;
  sys::path::append(newPath, removeVersionsFromPath(second));

  return newPath.str().str();
}

void DirectoryScanner::addVFSForFramework(FileMap &output, StringRef sysroot,
                                          ArrayRef<StringRef> rootPaths,
                                          const Framework &framework) const {
  auto computeAndAddPath = [&](StringRef path) {
    StringRef relativePath = path;
    for (StringRef rootPath : rootPaths) {
      if (!relativePath.consume_front(rootPath))
        continue;
      SmallString<PATH_MAX> mappedPath(sysroot);
      sys::path::append(mappedPath, relativePath);
      output.emplace_back(mappedPath, path);
      auto altPath = removeVersionsFromPath(mappedPath);
      if (altPath != mappedPath)
        output.emplace_back(altPath, path);
      break;
    }
  };

  for (auto &header : framework._headerFiles)
    computeAndAddPath(header.fullPath);
  for (auto &module : framework._moduleMaps)
    computeAndAddPath(module);
  for (auto &module : framework._swiftModules) {
    for (auto &interface : module.swiftInterfaces)
      computeAndAddPath(interface);
  }

  for (auto &ver : framework._versions)
    addVFSForFramework(output, sysroot, rootPaths, ver);
  for (auto &sub : framework._subFrameworks)
    addVFSForFramework(output, sysroot, rootPaths, sub);
}

DirectoryScanner::FileMap
DirectoryScanner::getVFSFileMap(StringRef sysroot,
                                ArrayRef<StringRef> rootPaths) const {
  DirectoryScanner::FileMap output;
  for (auto &framework : frameworks)
    addVFSForFramework(output, sysroot, rootPaths, framework);

  return output;
}

TAPI_NAMESPACE_INTERNAL_END
