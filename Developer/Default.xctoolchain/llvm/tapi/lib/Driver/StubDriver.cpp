//===- lib/Driver/StubDriver.cpp - TAPI Stub Driver -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the stub driver for the tapi tool.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/ClangDiagnostics.h"
#include "tapi/Core/FileSystem.h"
#include "tapi/Core/InterfaceFileManager.h"
#include "tapi/Core/Path.h"
#include "tapi/Core/Registry.h"
#include "tapi/Core/Utils.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/Driver.h"
#include "tapi/Driver/Options.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include <queue>
#include <string>

using namespace llvm;
using namespace llvm::MachO;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

// Stub Driver Context.
namespace {

struct Context {
  Context(FileManager &fm, DiagnosticsEngine &diag, bool isBnI)
      : fm(fm), diag(diag),
        interfaceMgr(InterfaceFileManager(fm, /*isVolatile=*/isBnI)) {
    registry.addBinaryReaders();
    registry.addYAMLReaders();
    registry.addYAMLWriters();
    registry.addJSONReaders();
    registry.addJSONWriters();
  }

  Context(const Context &) = delete;

  bool deleteInputFile = false;
  bool inlinePrivateFrameworks = false;
  bool deletePrivateFrameworks = false;
  bool traceLibraryLocation = false;
  bool removeSharedCacheFlag = false;


  PathSeq sysroots;
  std::string inputPath;
  std::string outputPath;
  PathSeq searchPaths;
  PathSeq librarySearchPaths;
  PathSeq frameworkSearchPaths;
  Registry registry;
  FileManager &fm;
  DiagnosticsEngine &diag;
  InterfaceFileManager interfaceMgr;
  FileType fileType;
};

struct SymlinkInfo {
  std::string srcPath;
  std::string symlinkContent;

  SymlinkInfo(std::string path, std::string link)
      : srcPath(std::move(path)), symlinkContent(std::move(link)) {}

  SymlinkInfo(StringRef path, StringRef link)
      : srcPath(std::string(path)), symlinkContent(std::string(link)) {}
};


} // namespace

static std::shared_ptr<InterfaceFile>
findAndGetReexportedLibrary(const StringRef reexportName, Context &ctx,
                            bool printErrors = false) {
  /* given the install name of a reexported library and search paths, try to
   * find a framework on disk, and retrieve its contents with the intent to
   * inline into umbrella framework */
  auto path = findLibrary(reexportName, ctx.fm, ctx.frameworkSearchPaths,
                          ctx.librarySearchPaths, ctx.searchPaths);
  if (path.empty()) {
    if (printErrors)
      ctx.diag.report(diag::err_cannot_find_reexport) << reexportName;
    return nullptr;
  }
  auto bufferOrError = ctx.fm.getBufferForFile(path);
  if (auto ec = bufferOrError.getError()) {
    if (printErrors)
      ctx.diag.report(diag::err_cannot_read_file) << path << ec.message();
    return nullptr;
  }

  if (ctx.traceLibraryLocation)
    errs() << path << "\n";

  if (StringRef(path).endswith(".tbd")) {
    auto file = ctx.registry.readTextFile(std::move(bufferOrError.get()),
                                          ReadFlags::Symbols);
    if (!file) {
      if (printErrors)
        ctx.diag.report(diag::err_cannot_read_file)
            << path << toString(file.takeError());
      return nullptr;
    }
    return std::move(*file);
  }
  auto file =
      ctx.registry.readFile(std::move(bufferOrError.get()), ReadFlags::Symbols);
  if (!file) {
    if (printErrors)
      ctx.diag.report(diag::err_cannot_read_file)
          << path << toString(file.takeError());
    return nullptr;
  }
  auto interface = convertToInterfaceFile(*file);
  if (ctx.removeSharedCacheFlag) 
    interface->setOSLibNotForSharedCache(false);
  return std::move(interface);
}

static bool isPrivatePath(StringRef path, bool isSymlink = false) {
  // Remove the iOSSupport/DriverKit prefix to identify public locations inside
  // the iOSSupport/DriverKit directory.
  path.consume_front(MACCATALYST_PREFIX_PATH);
  path.consume_front(DRIVERKIT_PREFIX_PATH);
  // Also /Library/Apple prefix for ROSP.
  path.consume_front("/Library/Apple");
  // Also /System/Cryptexes/OS for SPLAT.
  path.consume_front(CRYPTEXES_PREFIX_PATH);

  if (path.startswith("/usr/local/lib"))
    return true;

  if (path.startswith("/System/Library/PrivateFrameworks"))
    return true;

  // Everything in /usr/lib/swift (including sub-directories) is now considered
  // public.
  if (path.consume_front("/usr/lib/swift/"))
    return false;

  // Only libraries directly in /usr/lib are public. All other libraries in
  // sub-directories (such as /usr/lib/system) are considered private.
  if (path.consume_front("/usr/lib/")) {
    if (path.contains('/'))
      return true;
    return false;
  }

  // /System/Library/Frameworks/ is a public location
  if (path.startswith("/System/Library/Frameworks/")) {
    StringRef name, rest;
    std::tie(name, rest) =
        path.drop_front(sizeof("/System/Library/Frameworks")).split('.');

    // Allow symlinks to top-level frameworks
    if (isSymlink && rest == "framework")
      return false;

    // only top level framework are public
    // /System/Library/Frameworks/Foo.framework/Foo ==> true
    // /System/Library/Frameworks/Foo.framework/Versions/A/Foo ==> true
    // /System/Library/Frameworks/Foo.framework/Resources/libBar.dylib ==> false
    // /System/Library/Frameworks/Foo.framework/Frameworks/Bar.framework/Bar
    // ==> false
    // /System/Library/Frameworks/Foo.framework/Frameworks/Xfoo.framework/XFoo
    // ==> false
    if (rest.startswith("framework/") &&
        (rest.endswith(name) || rest.endswith((name + ".tbd").str()) ||
         (isSymlink && rest.endswith("Current"))))
      return false;

    return true;
  }

  return false;
}


static bool inlineFrameworks(Context &ctx, InterfaceFile *dylib) {
  assert(ctx.fileType >= FileType::TBD_V3 &&
         "inlining is not supported for earlier TBD versions");
  auto &reexports = dylib->reexportedLibraries();
  for (auto &lib : reexports) {
    if (isPublicDylib(lib.getInstallName()))
      continue;

    if (lib.getInstallName().startswith("@"))
      continue;

    auto reexportedDylib =
        findAndGetReexportedLibrary(lib.getInstallName(), ctx,
                                    /*printErrors = */ true);
    if (!reexportedDylib)
      return false;
    if (!inlineFrameworks(ctx, reexportedDylib.get()))
      return false;
    auto overwriteFramework = false;

    if (!ctx.registry.canWrite(reexportedDylib.get(), ctx.fileType)) {
      ctx.diag.report(diag::err_cannot_convert_dylib)
          << reexportedDylib->getPath();
      return false;
    }
    dylib->inlineLibrary(reexportedDylib, overwriteFramework);
  }

  return true;
}

static bool stubifyDynamicLibrary(Context &ctx) {
  auto input = ctx.fm.getFile(ctx.inputPath);
  if (!input) {
    ctx.diag.report(clang::diag::err_drv_no_such_file) << ctx.inputPath;
    return false;
  }
  auto *inputFile = *input;
  auto bufferOrErr = ctx.fm.getBufferForFile(inputFile);
  if (auto ec = bufferOrErr.getError()) {
    ctx.diag.report(diag::err_cannot_read_file)
        << inputFile->getName() << ec.message();
    return false;
  }

  // Is the input file a dynamic library?
  if (!ctx.registry.canRead(bufferOrErr.get()->getMemBufferRef(),
                            FileType::MachO_DynamicLibrary |
                                FileType::MachO_DynamicLibrary_Stub |
                                Registry::getTextFileType())) {
    ctx.diag.report(diag::err_not_a_dylib) << inputFile->getName();
    return false;
  }

  auto file =
      ctx.registry.readFile(std::move(bufferOrErr.get()), ReadFlags::Symbols);
  if (!file) {
    ctx.diag.report(diag::err_cannot_read_file)
        << ctx.inputPath << toString(file.takeError());
    return false;
  }
  if (ctx.traceLibraryLocation)
    errs() << ctx.inputPath << "\n";

  auto interface = convertToInterfaceFile(*file);
  if (ctx.removeSharedCacheFlag) 
    interface->setOSLibNotForSharedCache(false);
  auto *dylib = interface.get();
  if (!ctx.registry.canWrite(dylib, ctx.fileType)) {
    ctx.diag.report(diag::err_cannot_convert_dylib) << dylib->getPath();
    return false;
  }

  if (ctx.inlinePrivateFrameworks) {
    if (!inlineFrameworks(ctx, dylib))
      return false;
  }

  if (auto result =
          ctx.interfaceMgr.writeFile(ctx.outputPath, dylib, ctx.fileType)) {
    ctx.diag.report(diag::err_cannot_write_file)
        << ctx.outputPath << toString(std::move(result));
    return false;
  }

  if (ctx.deleteInputFile) {
    inputFile->closeFile();
    if (auto ec = sys::fs::remove(ctx.inputPath)) {
      ctx.diag.report(diag::err) << ctx.inputPath << ec.message();
      return false;
    }
  }

  return true;
}

/// \brief Converts all dynamic libraries/frameworks to text-based stubs if
/// possible. Also create the same symlinks as the ones that pointed to the
/// original library. If requested the source library will be deleted.
///
/// inputPath is the canonical path - no symlinks and no path relative elements.
static bool stubifyDirectory(Context &ctx) {
  assert(ctx.inputPath.back() != '/' && "Unexpected / at end of input path.");

  std::map<std::string, std::vector<SymlinkInfo>> symlinks;
  std::map<std::string, std::unique_ptr<InterfaceFile>> dylibs;
  std::map<std::string, std::string> originalNames;
  std::set<std::pair<std::string, bool>> toDelete;
  std::error_code ec;
  for (sys::fs::recursive_directory_iterator i(ctx.inputPath, ec), ie; i != ie;
       i.increment(ec)) {

    if (ec == std::errc::no_such_file_or_directory) {
      ctx.diag.report(diag::err) << i->path() << ec.message();
      continue;
    }

    if (ec) {
      ctx.diag.report(diag::err) << i->path() << ec.message();
      return false;
    }

    // Skip header directories (include/Headers/PrivateHeaders) and module
    // files.
    StringRef path = i->path();
    if (path.endswith("/include") || path.endswith("/Headers") ||
        path.endswith("/PrivateHeaders") || path.endswith("/Modules") ||
        path.endswith(".map") || path.endswith(".modulemap")) {
      i.no_push();
      continue;
    }

    // Check if the entry is a symlink. We don't follow symlinks, but we record
    // their content.
    bool isSymlink;
    if (auto ec = sys::fs::is_symlink_file(path, isSymlink)) {
      ctx.diag.report(diag::err) << path << ec.message();
      return false;
    }

    if (isSymlink) {
      // Don't follow symlink.
      i.no_push();

      bool shouldSkip;
      auto ec2 = shouldSkipSymlink(path, shouldSkip);

      // if assessing symlink is broken for some reason, we should continue
      // trying to repair it before quitting on recording the file.
      if (!ec2 && shouldSkip)
        continue;

      if (ctx.deletePrivateFrameworks &&
          isPrivatePath(path.drop_front(ctx.inputPath.size()), true)) {
        toDelete.emplace(path, false);
        continue;
      }

      SmallString<PATH_MAX> symlinkPath;
      if (auto ec = read_link(path, symlinkPath)) {
        ctx.diag.report(diag::err_cannot_read_file) << path << ec.message();
        return false;
      }

      // Some projects use broken symlinks that are absolute paths, which are
      // invalid during build time, but would be correct during runtime. In the
      // case of an absolute path we should check first if the path exist with
      // the SDKContentRoot as prefix.
      SmallString<PATH_MAX> linkSrc = path;
      SmallString<PATH_MAX> linkTarget;
      if (sys::path::is_absolute(symlinkPath)) {
        linkTarget = ctx.inputPath;
        sys::path::append(linkTarget, symlinkPath);

        if (ctx.fm.exists(linkTarget)) {
          // Convert the absolute path to an relative path.
          if (auto ec = make_relative(linkSrc, linkTarget, symlinkPath)) {
            ctx.diag.report(diag::err) << linkTarget << ec.message();
            return false;
          }
        } else if (!ctx.fm.exists(symlinkPath)) {
          ctx.diag.report(diag::warn_broken_symlink) << path;
          continue;
        } else {
          linkTarget = symlinkPath;
        }
      } else {
        linkTarget = linkSrc;
        sys::path::remove_filename(linkTarget);
        sys::path::append(linkTarget, symlinkPath);
      }

      // The symlink src is guaranteed to be a canonical path, because we don't
      // follow symlinks when scanning the SDK. The symlink target is
      // constructed from the symlink path and need to be canonicalized.
      if (auto ec = realpath(linkTarget)) {
        ctx.diag.report(diag::warn) << (linkTarget + " " + ec.message()).str();
        continue;
      }

      auto itr = symlinks.emplace(
          std::piecewise_construct, std::forward_as_tuple(linkTarget.c_str()),
          std::forward_as_tuple(std::vector<SymlinkInfo>()));
      itr.first->second.emplace_back(linkSrc.str(), symlinkPath.str().str());

      continue;
    }

    // We only have to look at files.
    auto file = ctx.fm.getFile(path);
    if (!file)
      continue;

    if (ctx.deletePrivateFrameworks &&
        isPrivatePath(path.drop_front(ctx.inputPath.size()))) {
      i.no_push();
      toDelete.emplace(path, false);
      continue;
    }

    auto bufferOrErr = ctx.fm.getBufferForFile(*file);
    if (auto ec = bufferOrErr.getError()) {
      ctx.diag.report(diag::err_cannot_read_file) << path << ec.message();
      return false;
    }

    // Check for dynamic libs and text-based stub files.
    if (!ctx.registry.canRead(bufferOrErr.get()->getMemBufferRef(),
                              FileType::MachO_DynamicLibrary |
                                  FileType::MachO_DynamicLibrary_Stub |
                                  Registry::getTextFileType()))
      continue;

    std::unique_ptr<InterfaceFile> interface = nullptr;
    if (path.endswith(".tbd")) {
      auto file2 = ctx.registry.readTextFile(std::move(bufferOrErr.get()),
                                             ReadFlags::Symbols);
      if (!file2) {
        ctx.diag.report(diag::err_cannot_read_file)
            << path << toString(file2.takeError());
        return false;
      }
      interface = std::move(*file2);
    } else {
      auto file2 = ctx.registry.readFile(std::move(bufferOrErr.get()),
                                         ReadFlags::Symbols);
      if (!file2) {
        ctx.diag.report(diag::err_cannot_read_file)
            << path << toString(file2.takeError());
        return false;
      }
      interface = convertToInterfaceFile(*file2);
    }

    if (ctx.traceLibraryLocation)
      errs() << path << "\n";

    // Normalize path for map lookup by removing the extension.
    SmallString<PATH_MAX> normalizedPath(path);
    TAPI_INTERNAL::replace_extension(normalizedPath, "");

    if ((interface->getFileType() == FileType::MachO_DynamicLibrary) ||
        (interface->getFileType() == FileType::MachO_DynamicLibrary_Stub)) {
      originalNames[normalizedPath.c_str()] = interface->getPath();

      // Don't add this MachO dynamic library, because we already have a
      // text-based stub recorded for this path.
      if (dylibs.count(normalizedPath.c_str()))
        continue;
    }

    if (ctx.removeSharedCacheFlag) 
      interface->setOSLibNotForSharedCache(false);

    // FIXME: Once we use C++17, this can be simplified.
    auto it = dylibs.find(normalizedPath.c_str());
    if (it != dylibs.end())
      it->second = std::move(interface);
    else
      dylibs.emplace(std::piecewise_construct,
                     std::forward_as_tuple(normalizedPath.c_str()),
                     std::forward_as_tuple(std::move(interface)));
  }

  for (auto &it : dylibs) {
    auto &dylib = it.second;
    SmallString<PATH_MAX> input(dylib->getPath());
    SmallString<PATH_MAX> output = input;
    TAPI_INTERNAL::replace_extension(output, ".tbd");

    if (!ctx.registry.canWrite(dylib.get(), ctx.fileType)) {
      ctx.diag.report(diag::err_cannot_convert_dylib) << dylib->getPath();
      return false;
    }

    if (ctx.inlinePrivateFrameworks && !inlineFrameworks(ctx, dylib.get()))
      return false;


    auto result = ctx.interfaceMgr.writeFile(std::string(output), dylib.get(),
                                             ctx.fileType);
    if (result) {
      ctx.diag.report(diag::err_cannot_write_file)
          << output << toString(std::move(result));
      return false;
    }

    // Get the original file name.
    SmallString<PATH_MAX> normalizedPath(dylib->getPath());
    TAPI_INTERNAL::replace_extension(normalizedPath, "");
    auto it2 = originalNames.find(normalizedPath.c_str());
    if (it2 == originalNames.end())
      continue;
    auto originalName = it2->second;

    if (ctx.deleteInputFile)
      toDelete.emplace(originalName, true);

    // Don't allow for more than 20 levels of symlinks.
    StringRef toCheck = originalName;
    for (int i = 0; i < 20; ++i) {
      auto itr = symlinks.find(toCheck.str());
      if (itr != symlinks.end()) {
        for (auto &symInfo : itr->second) {
          SmallString<PATH_MAX> linkSrc(symInfo.srcPath);
          SmallString<PATH_MAX> linkTarget(symInfo.symlinkContent);
          TAPI_INTERNAL::replace_extension(linkSrc, "tbd");
          TAPI_INTERNAL::replace_extension(linkTarget, "tbd");

          if (auto ec = sys::fs::remove(linkSrc)) {
            ctx.diag.report(diag::err) << linkSrc << ec.message();
            return false;
          }

          if (auto ec = sys::fs::create_link(linkTarget, linkSrc)) {
            ctx.diag.report(diag::err) << linkTarget << ec.message();
            return false;
          }

          if (ctx.deleteInputFile)
            toDelete.emplace(symInfo.srcPath, true);

          toCheck = symInfo.srcPath;
        }
      } else
        break;
    }
  }

  // Recursively delete the directories (this will abort when they are not empty
  // or we reach the root of the SDK.
  for (auto &it : toDelete) {
    auto &path = it.first;
    auto isInput = it.second;
    if (!isInput && symlinks.count(path))
      continue;

    if (auto ec = sys::fs::remove(path)) {
      ctx.diag.report(diag::err) << path << ec.message();
      return false;
    }

    std::error_code ec;
    auto dir = sys::path::parent_path(path);
    do {
      ec = sys::fs::remove(dir);
      dir = sys::path::parent_path(dir);
      if (!dir.startswith(ctx.inputPath))
        break;
    } while (!ec);
  }

  return true;
}

/// \brief Generate text-based stub files from dynamic libraries.
bool Driver::Stub::run(DiagnosticsEngine &diag, Options &opts) {
  if (opts.driverOptions.inputs.empty()) {
    diag.report(clang::diag::err_drv_no_input_files);
    return false;
  }

  if ((opts.tapiOptions.fileType < FileType::TBD_V3) &&
      opts.tapiOptions.inlinePrivateFrameworks) {
    diag.report(diag::err_inlining_not_supported) << opts.tapiOptions.fileType;
    return false;
  }

  // FIME: Copy everything for now.
  Context ctx(opts.getFileManager(), diag, opts.tapiOptions.isBnI);
  ctx.deleteInputFile = opts.tapiOptions.deleteInputFile;
  ctx.inlinePrivateFrameworks = opts.tapiOptions.inlinePrivateFrameworks;
  ctx.deletePrivateFrameworks = opts.tapiOptions.deletePrivateFrameworks;
  ctx.traceLibraryLocation = opts.tapiOptions.traceLibraryLocation;
  ctx.removeSharedCacheFlag = opts.tapiOptions.removeSharedCacheFlag;


  // Handle isysroot.
  for (auto &root : opts.tapiOptions.allSysroots) {
    SmallString<PATH_MAX> path(root);
    ctx.fm.makeAbsolutePath(path);
    if (!ctx.fm.exists(path)) {
      diag.report(diag::err_missing_sysroot) << path;
      return false;
    }
    ctx.sysroots.emplace_back(path.str());
  }

  ctx.frameworkSearchPaths =
      getAllPaths(opts.frontendOptions.systemFrameworkPaths);
  ctx.frameworkSearchPaths.insert(ctx.frameworkSearchPaths.end(),
                                  opts.frontendOptions.frameworkPaths.begin(),
                                  opts.frontendOptions.frameworkPaths.end());
  ctx.librarySearchPaths = opts.frontendOptions.libraryPaths;
  ctx.fileType = opts.tapiOptions.fileType;

  // Only expect one input.
  SmallString<PATH_MAX> input;
  for (const auto &path : opts.driverOptions.inputs) {
    if (input.empty())
      input = path;
    else {
      diag.report(clang::diag::err_drv_unknown_argument) << path;
      return false;
    }
  }

  if (auto ec = realpath(input)) {
    diag.report(diag::err) << input << ec.message();
    return false;
  }
  ctx.inputPath = input.str().str();

  bool isDirectory = false;
  bool isFile = false;
  if (auto ec = sys::fs::is_directory(ctx.inputPath, isDirectory)) {
    diag.report(diag::err) << ctx.inputPath << ec.message();
    return false;
  }
  if (!isDirectory) {
    if (auto ec = sys::fs::is_regular_file(ctx.inputPath, isFile)) {
      diag.report(diag::err) << ctx.inputPath << ec.message();
      return false;
    }
  }

  // Expect a directory or a file.
  if (!isDirectory && !isFile) {
    diag.report(diag::err_invalid_input_file) << ctx.inputPath;
    return false;
  }

  // Handle -o.
  if (!opts.driverOptions.outputPath.empty())
    ctx.outputPath = opts.driverOptions.outputPath;
  else if (isFile) {
    SmallString<PATH_MAX> outputPath(ctx.inputPath);
    TAPI_INTERNAL::replace_extension(outputPath, ".tbd");
    ctx.outputPath = outputPath.str().str();
  } else {
    assert(isDirectory && "Expected a directory.");
    ctx.outputPath = ctx.inputPath;
  }

  if (isDirectory) {
    ctx.searchPaths.emplace_back(ctx.inputPath);
    // Remove shared cache flag when processing for the Public SDK, this avoids
    // older toolchains reading newer SDKs that would otherwise look malformed.
    // Secrecy is not a concern here, so this can be revisited once all supported 
    // environments have updated. 
    if (StringRef(ctx.inputPath).contains("PublicSDKContentRoot")) 
      ctx.removeSharedCacheFlag = true;
  }

  for (auto &path : ctx.sysroots)
    ctx.searchPaths.emplace_back(path);

  if (isFile)
    return stubifyDynamicLibrary(ctx);

  return stubifyDirectory(ctx);
}

TAPI_NAMESPACE_INTERNAL_END
