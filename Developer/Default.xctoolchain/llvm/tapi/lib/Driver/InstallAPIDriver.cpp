//===- lib/Driver/InstallAPIDriver.cpp - TAPI Driver ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the driver for the tapi tool.
///
//===----------------------------------------------------------------------===//

#include "APINormalizer.h"
#include "FileListVisitor.h"
#include "tapi/APIVerifier/APIVerifier.h"
#include "tapi/Core/APIPrinter.h"
#include "tapi/Core/ClangDiagnostics.h"
#include "tapi/Core/HeaderFile.h"
#include "tapi/Core/InterfaceFileManager.h"
#include "tapi/Core/Path.h"
#include "tapi/Core/Registry.h"
#include "tapi/Core/SymbolVerifier.h"
#include "tapi/Core/Utils.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/DirectoryScanner.h"
#include "tapi/Driver/Driver.h"
#include "tapi/Driver/HeaderGlob.h"
#include "tapi/Driver/Options.h"
#include "tapi/Frontend/Frontend.h"
#include "tapi/LinkerInterfaceFile.h"
#include "tapi/SDKDB/PartialSDKDB.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/BinaryFormat/MachO.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TextAPI/Symbol.h"
#include <algorithm>
#include <numeric>
#include <string>

using namespace llvm;
using namespace llvm::opt;
using namespace llvm::MachO;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

using LibAttrs = llvm::SmallSet<InterfaceFileRef, 5>;

static bool verifyBinaryInfo(bool &autoZippered,
                             const std::vector<Triple> &targets,
                             const APIs &dylibFile, const BinaryInfo &apiInfo,
                             LibAttrs &apiClients, LibAttrs &apiReexports,
                             LibAttrs &apiRelinks, LibAttrs &apiRPaths,
                             DiagnosticsEngine &diag, const FileType tbdType) {

  TargetList dylibTargets;
  const BinaryInfo &dylibInfo = (*dylibFile.begin())->getBinaryInfo();
  LibAttrs dylibReexports;
  LibAttrs dylibClients;
  LibAttrs dylibRelinks;
  LibAttrs dylibRPaths;
  auto addLib = [&](StringRef libName, LibAttrs &libs, Target &target) -> void {
    auto it = llvm::find_if(
        libs, [&](const auto &ref) { return ref.getInstallName() == libName; });
    if (it == libs.end()) {
      libs.insert({libName, {target}});
      return;
    }
    InterfaceFileRef lib = *it;
    libs.erase(*it);
    lib.addTarget(target);
    libs.insert(lib);
  };

  for (auto &api : dylibFile) {
    auto target = api->getTarget();
    dylibTargets.emplace_back(target);
    const auto &binInfo = api->getBinaryInfo();
    for (const StringRef libName : binInfo.reexportedLibraries)
      addLib(libName, dylibReexports, target);
    for (const StringRef libName : binInfo.allowableClients)
      addLib(libName, dylibClients, target);
    if (tbdType >= FileType::TBD_V5) {
      for (const StringRef name : binInfo.rpaths)
        addLib(name, dylibRPaths, target);
      for (const StringRef libName : binInfo.relinkedLibraries)
        addLib(libName, dylibRelinks, target);
    }
  }

  // Check targets first.
  const TargetList apiTargets(targets.begin(), targets.end());
  auto apiPlatforms = mapToPlatformVersionSet(apiTargets);
  auto dylibPlatforms = mapToPlatformVersionSet(dylibTargets);
  if (apiPlatforms != dylibPlatforms) {
    const bool diffMinOS =
        mapToPlatformSet(apiTargets) == mapToPlatformSet(dylibTargets);
    if (autoZippered || diffMinOS)
      diag.report(diag::warn_platform_mismatch)
          << apiPlatforms << dylibPlatforms;
    else {
      diag.report(diag::err_platform_mismatch)
          << apiPlatforms << dylibPlatforms;
      return false;
    }
  }

  auto apiArchs = mapToArchitectureSet(apiTargets);
  auto dylibArchs = mapToArchitectureSet(dylibTargets);
  if (apiArchs != dylibArchs) {
    diag.report(diag::err_architecture_mismatch) << apiArchs << dylibArchs;
    return false;
  }

  if (apiInfo.installName != dylibInfo.installName) {
    diag.report(diag::err_install_name_mismatch)
        << apiInfo.installName << dylibInfo.installName;
    return false;
  }

  if (apiInfo.currentVersion != dylibInfo.currentVersion) {
    diag.report(diag::err_current_version_mismatch)
        << apiInfo.currentVersion << dylibInfo.currentVersion;
    return false;
  }

  if (apiInfo.compatibilityVersion != dylibInfo.compatibilityVersion) {
    diag.report(diag::err_compatibility_version_mismatch)
        << apiInfo.compatibilityVersion << dylibInfo.compatibilityVersion;
    return false;
  }

  if (apiInfo.isAppExtensionSafe != dylibInfo.isAppExtensionSafe) {
    diag.report(diag::err_appextension_safe_mismatch)
        << (apiInfo.isAppExtensionSafe ? "true" : "false")
        << (dylibInfo.isAppExtensionSafe ? "true" : "false");
    return false;
  }

  if (!dylibInfo.isTwoLevelNamespace) {
    diag.report(diag::err_no_twolevel_namespace);
    return false;
  }

  if (apiInfo.isOSLibNotForSharedCache != dylibInfo.isOSLibNotForSharedCache) {
    diag.report(diag::err_shared_cache_eligiblity_mismatch)
        << (apiInfo.isOSLibNotForSharedCache ? "true" : "false")
        << (dylibInfo.isOSLibNotForSharedCache ? "true" : "false");
    return false;
  }

  if (apiInfo.parentUmbrella.empty() && !dylibInfo.parentUmbrella.empty()) {
    diag.report(diag::err_parent_umbrella_missing)
        << "tapi option" << dylibInfo.parentUmbrella;
    return false;
  }

  if (!apiInfo.parentUmbrella.empty() && dylibInfo.parentUmbrella.empty()) {
    diag.report(diag::err_parent_umbrella_missing)
        << "binary file" << apiInfo.parentUmbrella;
    return false;
  }

  if ((!apiInfo.parentUmbrella.empty()) &&
      (apiInfo.parentUmbrella != dylibInfo.parentUmbrella)) {
    diag.report(diag::err_parent_umbrella_mismatch)
        << apiInfo.parentUmbrella << dylibInfo.parentUmbrella;
    return false;
  }

  auto compareLibraries = [&](const LibAttrs &headers, const LibAttrs &dylib,
                              unsigned diagID_missing,
                              unsigned diagID_mismatch) {
    if (headers == dylib)
      return true;

    for (const InterfaceFileRef &ref1 : headers) {
      auto it = find_if(dylib, [&](const InterfaceFileRef &ref2) {
        return ref1.getInstallName() == ref2.getInstallName();
      });

      if (it == dylib.end()) {
        diag.report(diagID_missing) << "binary file" << ref1;
        return false;
      }

      if ((*it).getArchitectures() != ref1.getArchitectures()) {
        diag.report(diagID_mismatch) << ref1 << *it;
        return false;
      }
    }

    for (const auto &ref2 : dylib) {
      auto it = find_if(headers, [&](const InterfaceFileRef &ref1) {
        return ref1.getInstallName() == ref2.getInstallName();
      });

      if (it == headers.end()) {
        diag.report(diagID_missing) << "tapi option" << ref2;
        return false;
      }

      if ((*it).getArchitectures() != ref2.getArchitectures()) {
        llvm_unreachable("this case was already covered above.");
      }
    }
    // Ignore differences in platform or versions. 
    return true;
  };

  if (!compareLibraries(apiReexports, dylibReexports,
                        diag::err_reexported_libraries_missing,
                        diag::err_reexported_libraries_mismatch))
    return false;

  if (!compareLibraries(apiClients, dylibClients,
                        diag::err_allowable_clients_missing,
                        diag::err_allowable_clients_mismatch))
    return false;

  if (tbdType >= FileType::TBD_V5) {
    if (!compareLibraries(apiRelinks, dylibRelinks,
                          diag::err_relinkable_libraries_missing,
                          diag::err_relinkable_libraries_mismatch))
      return false;

    // Ignore rpath differences if building asan variant, since the compiler
    // injects additional paths.
    if (!apiInfo.installName.endswith("_asan")) {
      if (!compareLibraries(apiRPaths, dylibRPaths, diag::warn_rpaths_missing,
                            diag::warn_rpaths_mismatch))
        return true;
    }
  }

  return true;
}

static bool mergeInterfaces(DiagnosticsEngine &diag,
                            std::unique_ptr<InterfaceFile> &primary,
                            std::unique_ptr<InterfaceFile> &secondary) {
  auto mergedContent = primary->merge(secondary.get());
  if (!mergedContent) {
    diag.report(diag::err_merge_file)
        << secondary->getPath() << toString(mergedContent.takeError());
    return false;
  }
  primary = std::move(mergedContent.get());
  return true;
}

static Expected<std::string> findClangExecutable(DiagnosticsEngine &diag) {
  static int staticSymbol;
  // If environmental variable "_TAPI_TEST_CLANG" is set, just return that.
  if (auto clangFromEnv = sys::Process::GetEnv("_TAPI_TEST_CLANG"))
    return *clangFromEnv;
  // Try to find clang first in the toolchain. If that fails, then fall-back to
  // the default search PATH.
  auto mainExecutable = sys::fs::getMainExecutable("tapi", &staticSymbol);
  StringRef toolchainBinDir = sys::path::parent_path(mainExecutable);
  auto clangBinary = sys::findProgramByName("clang", ArrayRef(toolchainBinDir));
  if (clangBinary.getError()) {
    diag.report(diag::warn) << "cannot find 'clang' in toolchain directory. "
                               "Looking for 'clang' in PATH instead.";
    clangBinary = sys::findProgramByName("clang");
    if (auto ec = clangBinary.getError())
      return make_error<StringError>("unable to find 'clang' in PATH", ec);
  }
  return clangBinary.get();
}

static Expected<APIs> getCodeCoverageSymbols(DiagnosticsEngine &diag,
                                             InterfaceFileManager &manager,
                                             const std::vector<Triple> &targets,
                                             const std::string &isysroot) {
  static int staticSymbol;
  // Try to find clang first in the toolchain. If that fails, then fall-back to
  // the default search PATH.
  auto mainExecutable = sys::fs::getMainExecutable("tapi", &staticSymbol);
  StringRef toolchainBinDir = sys::path::parent_path(mainExecutable);
  auto clangBinary = findClangExecutable(diag);
  if (!clangBinary) {
    return clangBinary.takeError();
  }

  // Create temporary input and output files.
  SmallString<PATH_MAX> inputFile;
  if (auto ec = sys::fs::createTemporaryFile("code_coverage", "c", inputFile))
    return make_error<StringError>("unable to create temporary input file", ec);
  FileRemover removeInputFile(inputFile);

  SmallString<PATH_MAX> outputFile;
  if (auto ec =
          sys::fs::createTemporaryFile("libcodecoverage", "dylib", outputFile))
    return make_error<StringError>("unable to create temporary output file",
                                   ec);
  FileRemover removeOutputFile(outputFile);

  std::error_code ec;
  raw_fd_ostream input(inputFile, ec, sys::fs::OF_None);
  if (ec)
    return make_error<StringError>("cannot open input file", ec);
  input << "static int foo() { return 0; }\n";
  input.close();

  std::string installDir = toolchainBinDir.str();
  APIs apis;
  for (const auto &target : targets) {
    const StringRef clangArgs[] = {*clangBinary,
                                   "-target",
                                   target.str(),
                                   "-dynamiclib",
                                   "-fprofile-instr-generate",
                                   "-fcoverage-mapping",
                                   "-isysroot",
                                   isysroot,
                                   "-o",
                                   outputFile,
                                   inputFile,
                                   "-v"};

    SmallString<PATH_MAX> stderrFile;
    if (auto ec = sys::fs::createTemporaryFile("stderr", "txt", stderrFile))
      return make_error<StringError>("unable to create temporary stderr file",
                                     ec);
    FileRemover removeStderrFile(stderrFile);

    const std::optional<StringRef> redirects[] = {
        /*STDIN=*/std::nullopt,
        /*STDOUT=*/std::nullopt,
        /*STDERR=*/StringRef(stderrFile)};

    bool failed = sys::ExecuteAndWait(clangBinary.get(), clangArgs,
                                      /*env=*/std::nullopt, redirects);

    if (failed) {
      auto bufferOr = MemoryBuffer::getFile(stderrFile);
      if (auto ec = bufferOr.getError())
        return make_error<StringError>("unable to read file", ec);

      std::string message = "'clang' invocation failed:\n";
      for (auto arg : clangArgs) {
        if (arg.empty())
          continue;
        message.append(arg.str()).append(1, ' ');
      }
      message.append(1, '\n');
      message.append(bufferOr.get()->getBuffer().str());

      return make_error<StringError>(
          message, std::make_error_code(std::errc::not_supported));
    }
    auto file = manager.readFile(std::string(outputFile));
    if (!file)
      return file.takeError();
    assert(file->size() == 1 && "only a single target should exist at a time");
    std::shared_ptr<API> api = *file->begin();
    if (api)
      apis.emplace_back(std::move(*file->begin()));
  }
  return apis;
}

static Expected<std::map<SimpleSymbol, SimpleSymbol>>
parseAliasList(FileManager &fm, StringRef path,
               std::unique_ptr<MemoryBuffer> &buffer) {
  auto file = fm.getFile(path);
  if (!file)
    return errorCodeToError(
        std::make_error_code(std::errc::no_such_file_or_directory));

  auto bufferOrErr = fm.getBufferForFile(*file);
  if (!bufferOrErr)
    return errorCodeToError(bufferOrErr.getError());

  buffer = std::move(bufferOrErr.get());
  SmallVector<StringRef, 16> lines;
  std::map<SimpleSymbol, SimpleSymbol> aliases;
  buffer->getBuffer().split(lines, "\n", /*MaxSplit=*/-1, /*KeepEmpty=*/false);
  for (const auto &line : lines) {
    auto l = line.trim();
    if (l.empty())
      continue;

    // Skip comments
    if (l.startswith("#"))
      continue;

    StringRef symbol, remain, alias;
    // The original symbol ends at a whitespace
    std::tie(symbol, remain) = getToken(l);
    // The alias ends before a comment or EOL
    std::tie(alias, remain) = getToken(remain, "#");
    alias = alias.trim();
    if (alias.empty())
      return make_error<StringError>("invalid alias list",
                                     inconvertibleErrorCode());
    aliases[parseSymbol(alias)] = parseSymbol(symbol);
}

return aliases;
}

namespace {
bool collectHeadersFromFramework(DiagnosticsEngine &diag, FileManager &fm,
                                 const Framework &framework,
                                 HeaderSeq &headerFiles) {
  for (const auto &header : framework._headerFiles) {
    if (!fm.getFile(header.fullPath)) {
      diag.report(diag::err_no_such_header_file)
          << header.fullPath << (unsigned)header.type;
      return false;
    }
    headerFiles.emplace_back(header);
  }

  return true;
}

std::optional<APIs> readFile(InterfaceFileManager &manager,
                             DiagnosticsEngine &diag, const std::string &path) {
  // Files in B&I env can be overwritten thus volatile.
  auto file = manager.readFile(path, ReadFlags::DefinedSymbols);
  if (!file) {
    diag.report(diag::err_cannot_read_file)
        << path << toString(file.takeError());
    return std::nullopt;
  }

  return *file;
}

bool setModuleOptions(DiagnosticsEngine &diag, FrontendJob &job,
                      const FrontendOptions &options, bool &customModuleCache) {
  if (!options.enableModules)
    return true;
  job.enableModules = options.enableModules;
  // Generate specific module cache if one was not provided.
  if (job.moduleCachePath.empty()) {
    SmallString<1024> moduleCachePath;
    llvm::sys::path::system_temp_directory(/*erasedOnReboot=*/true,
                                           moduleCachePath);
    const std::error_code ec =
        llvm::sys::fs::createUniqueDirectory(moduleCachePath, moduleCachePath);
    if (ec) {
      diag.report(clang::diag::err_unable_to_make_temp) << ec.message();
      return false;
    }
    job.moduleCachePath =
        std::string(moduleCachePath) + "/org.llvm.clang.tapi/ModuleCache";
    customModuleCache = true;
  }
  return true;
}

std::optional<FrontendJob>
setOverridingOptionsToJob(DiagnosticsEngine &diag, const FrontendJob &job,
                          const FrontendOptions &optionsToOverride,
                          bool &customModuleCache,
                          ArrayRef<std::string> extraXArgs) {
  FrontendJob overrideJob = job;
  llvm::for_each(extraXArgs, [&overrideJob](const auto &arg) {
    overrideJob.clangExtraArgs.emplace_back(arg);
  });

  if (job.type != HeaderType::Project)
    return std::move(overrideJob);

  if (!setModuleOptions(diag, overrideJob, optionsToOverride,
                        customModuleCache))
    return std::nullopt;

  // Because overrides can only be additive, only override options if not empty.
  if (!optionsToOverride.prefixHeaders.empty())
    overrideJob.prefixHeaders = optionsToOverride.prefixHeaders;
  if (optionsToOverride.useObjectiveCARC)
    overrideJob.useObjectiveCARC = optionsToOverride.useObjectiveCARC;
  return std::move(overrideJob);
}

} // end anonymous namespace.

static std::unique_ptr<InterfaceFile>
handleSwiftInput(DiagnosticsEngine &diag, const BinaryInfo &binaryInfo,
                 InterfaceFileManager &manager, std::vector<Triple> allTargets,
                 const PathSeq &swiftInputs) {

  // Assign necessary content to be able to merge.
  auto swiftFile = std::make_unique<InterfaceFile>(InterfaceFile());
  swiftFile->addTargets(allTargets);
  swiftFile->setInstallName(binaryInfo.installName);
  swiftFile->setCurrentVersion(binaryInfo.currentVersion);
  swiftFile->setCompatibilityVersion(binaryInfo.compatibilityVersion);
  swiftFile->setTwoLevelNamespace();
  swiftFile->setApplicationExtensionSafe(binaryInfo.isAppExtensionSafe);

  for (const auto &path : swiftInputs) {
    auto file = readFile(manager, diag, path);
    if (!file)
      return nullptr;
    // TODO: use readYAML instead for swift files.
    auto interface = convertToInterfaceFile(*file);
    if (!mergeInterfaces(diag, swiftFile, interface))
      return nullptr;
  }
  return swiftFile;
}

/// \brief Parses the headers and generate a text-based stub file.
bool Driver::InstallAPI::run(DiagnosticsEngine &diag, Options &opts) {
  auto &fm = opts.getFileManager();

  // Handle targets.
  if (opts.frontendOptions.targets.empty()) {
    diag.report(diag::err_no_target);
    return false;
  }

  // Set default language option.
  if (opts.frontendOptions.language == clang::Language::Unknown)
    opts.frontendOptions.language = clang::Language::ObjC;

  // Handle install name.
  if (opts.linkerOptions.installName.empty()) {
    diag.report(diag::err_no_install_name);
    return false;
  }

  SmallString<PATH_MAX> name =
      sys::path::filename(opts.linkerOptions.installName);
  sys::path::replace_extension(name, "");

  // Handle platform.
  if (mapToPlatformSet(opts.frontendOptions.targets).count(PLATFORM_UNKNOWN)) {
    diag.report(diag::err_no_deployment_target);
    return false;
  }

  diag.setErrorLimit(opts.diagnosticsOptions.errorLimit);

  std::vector<Triple> allTargets;
  allTargets.insert(allTargets.end(), opts.frontendOptions.targets.begin(),
                    opts.frontendOptions.targets.end());
  allTargets.insert(allTargets.end(),
                    opts.frontendOptions.targetVariants.begin(),
                    opts.frontendOptions.targetVariants.end());

  // Check to see if we need to AutoZipper the output.
  // If auto zippered, add ios mac to the platform.
  bool autoZippered = false;
  const auto platforms = mapToPlatformSet(allTargets);

  // Lookup re-exported libraries.
  InterfaceFileManager manager(fm, opts.tapiOptions.isBnI);
  PathSeq frameworkSearchPaths;
  LibAttrs reexportedLibraries;
  std::vector<APIs> reexportedLibraryFiles;

  auto accumulateTargets = [&](const ArchitectureSet &archs) {
    TargetList targets;
    for (const auto arch : archs)
      for (const auto platform : platforms)
        targets.emplace_back(arch, platform);
    return targets;
  };

  auto addToLibrary = [&](const auto &source, LibAttrs &destination) {
    for (const auto &[name, archs] : source)
      destination.insert({name, accumulateTargets(archs)});
  };

  auto accumulateReexports = [&](std::string &path, const auto &archs) {
    auto reexportOr = readFile(manager, diag, path);
    if (!reexportOr)
      return false;
    auto reexport = *reexportOr;
    auto installName = (*reexport.begin())->getInstallName();
    assert(installName && "YAML parse error for install name");
    reexportedLibraries.insert({*installName, accumulateTargets(archs)});
    reexportedLibraryFiles.emplace_back(reexport);
    return true;
  };

  // Search user framework paths before searching system framework paths.
  for (auto &path : opts.frontendOptions.frameworkPaths)
    frameworkSearchPaths.emplace_back(path);

  addToLibrary(opts.linkerOptions.reexportInstallNames, reexportedLibraries);

  for (auto &it : opts.linkerOptions.reexportedLibraries) {
    auto name = "lib" + it.first + ".dylib";
    auto path =
        findLibrary(name, fm, {}, opts.frontendOptions.libraryPaths, {});
    if (path.empty()) {
      diag.report(diag::err_cannot_find) << "re-exported library" << it.first;
      return false;
    }
    if (opts.tapiOptions.traceLibraryLocation)
      errs() << path << "\n";

    accumulateReexports(path, it.second);
  }

  for (auto &it : opts.linkerOptions.reexportedLibraryPaths)
    accumulateReexports(it.first, it.second);

  for (auto &target : allTargets) {
    auto systemFrameworkPaths = getPathsForPlatform(
        opts.frontendOptions.systemFrameworkPaths, mapToPlatformType(target));
    frameworkSearchPaths.insert(frameworkSearchPaths.end(),
                                systemFrameworkPaths.begin(),
                                systemFrameworkPaths.end());
    for (auto &it : opts.linkerOptions.reexportedFrameworks) {
      auto name = it.first + ".framework/" + it.first;
      auto path = findLibrary(name, fm, frameworkSearchPaths, {}, {});
      if (path.empty()) {
        diag.report(diag::err_cannot_find)
            << "re-exported framework" << it.first;
        return false;
      }
      if (opts.tapiOptions.traceLibraryLocation)
        errs() << path << "\n";

      accumulateReexports(path, it.second);
    }
  }

  if (opts.driverOptions.inputs.empty() && opts.tapiOptions.fileLists.empty()) {
    diag.report(clang::diag::err_drv_no_input_files);
    return false;
  }

  PathSeq inputPaths;
  for (const auto &path : opts.driverOptions.inputs) {
    if (sys::path::extension(path) == ".json") {
      opts.tapiOptions.fileLists.emplace_back(path);
      continue;
    }
    inputPaths.emplace_back(path);
  }

  FrontendJob job;
  job.vfs = &fm.getVirtualFileSystem();
  job.language = opts.frontendOptions.language;
  job.language_std = opts.frontendOptions.language_std;
  job.overwriteRTTI = opts.frontendOptions.useRTTI;
  job.overwriteNoRTTI = opts.frontendOptions.useNoRTTI;
  job.visibility = opts.frontendOptions.visibility;
  job.isysroot = opts.frontendOptions.isysroot;
  job.macros = opts.frontendOptions.macros;
  job.systemIncludePaths = opts.frontendOptions.systemIncludePaths;
  job.afterIncludePaths = opts.frontendOptions.afterIncludePaths;
  job.quotedIncludePaths = opts.frontendOptions.quotedIncludePaths;
  job.frameworkPaths = opts.frontendOptions.frameworkPaths;
  job.includePaths = opts.frontendOptions.includePaths;
  job.clangExtraArgs = opts.frontendOptions.clangExtraArgs;
  job.enableModules = opts.frontendOptions.enableModules;
  job.moduleCachePath = opts.frontendOptions.moduleCachePath;
  job.validateSystemHeaders = opts.frontendOptions.validateSystemHeaders;
  job.clangResourcePath = opts.frontendOptions.clangResourcePath;
  job.useObjectiveCARC = opts.frontendOptions.useObjectiveCARC;
  job.useObjectiveCWeakARC = opts.frontendOptions.useObjectiveCWeakARC;
  job.verbose = opts.frontendOptions.verbose;
  job.clangExecutablePath = opts.driverOptions.clangExecutablePath;
  job.productName = opts.frontendOptions.productName;

  job.clangExtraArgs.insert(job.clangExtraArgs.begin(), "-Wprivate-extern");

  // Hack to determine if tapi recieved required
  // search paths to enable angle included headers for parsing.
  // Also angle includes are necessary for modules.
  job.useRelativePath = !job.quotedIncludePaths.empty() || job.enableModules;

  auto createDirForOutput = [&diag](StringRef outputPath, bool isFile = true) {
    SmallString<PATH_MAX> outputDir(outputPath);
    if (isFile)
      sys::path::remove_filename(outputDir);
    auto ec = sys::fs::create_directories(outputDir, /*IgnoreExisting=*/true,
                                   sys::fs::owner_all | sys::fs::all_read |
                                       sys::fs::all_exe);
    if (ec) {
      diag.report(diag::err_cannot_create_directory) << outputDir << ec.message();
      return false;
    }
    return true;
  };

  if (!opts.diagnosticsOptions.serializeDiagnosticsFile.empty()) {
    if (!createDirForOutput(opts.diagnosticsOptions.serializeDiagnosticsFile))
      return false;
    diag.setupDiagnosticsFile(opts.diagnosticsOptions.serializeDiagnosticsFile,
                              /*serialize=*/true);
  }

  //
  // Scan through the directories and create a list of all found frameworks.
  //
  HeaderSeq headerFiles;
  std::string frameworkName;
  bool customModuleCache = false;

  if (!inputPaths.empty()) {
    DirectoryScanner scanner(fm, diag,
                             opts.linkerOptions.isDynamicLibrary
                                 ? ScannerMode::ScanDylibs
                                 : ScannerMode::ScanFrameworks);

    for (const auto &path : inputPaths) {
      if (fm.isDirectory(path, /*CacheFailure=*/false)) {
        SmallString<PATH_MAX> normalizedPath(path);
        fm.getVirtualFileSystem().makeAbsolute(normalizedPath);
        sys::path::remove_dots(normalizedPath, /*remove_dot_dot=*/true);
        if (!scanner.scan(normalizedPath))
          return false;
      } else {
        diag.report(diag::err_no_directory) << path;
        return false;
      }
    }

    auto frameworks = scanner.takeResult();
    if (frameworks.empty()) {
      diag.report(diag::err_no_framework);
      return false;
    }

    if (frameworks.size() > 1) {
      diag.report(diag::err_more_than_one_framework);
      return false;
    }

    auto *framework = &frameworks.back();

    if (job.useRelativePath)
      // Infer framework search patch from directory input path.
      // This is a workaround for makefile projects that don't pass this path
      // directly and is needed when tapi assembles the include names.
      job.frameworkPaths.insert(
          job.frameworkPaths.begin(),
          sys::path::parent_path(framework->getPath()).str());

    if (!setModuleOptions(diag, job, opts.frontendOptions, customModuleCache))
      return false;

    if (!framework->_versions.empty()) {
      if (framework->_versions.back().empty()) {
        diag.report(diag::warn_empty_versions_directory)
            << framework->_versions.back()._baseDirectory;
      } else {
        framework = &framework->_versions.back();
      }
    }

    frameworkName = sys::path::stem(framework->getName()).str();
    if (!collectHeadersFromFramework(diag, fm, *framework, headerFiles))
      return false;
    for (const auto &sub : framework->_subFrameworks) {
      if (!sub.isDynamicLibrary)
        continue;
      if (!collectHeadersFromFramework(diag, fm, sub, headerFiles))
        return false;
    }
  } else if (!opts.tapiOptions.fileLists.empty()) {
    // Only attempt to assign frameworks.
    if (!opts.linkerOptions.isDynamicLibrary)
      frameworkName =
          Framework::getNameFromInstallName(opts.linkerOptions.installName);
  }

  for (const auto &listPath : opts.tapiOptions.fileLists) {
    auto file = fm.getFile(listPath);
    if (!file) {
      diag.report(clang::diag::err_drv_no_such_file) << listPath;
      return false;
    }
    auto bufferOr = fm.getBufferForFile(*file);
    if (auto ec = bufferOr.getError()) {
      diag.report(diag::err_cannot_read_file)
          << (*file)->getName() << ec.message();
      return false;
    }
    auto reader = FileListReader::get(std::move(bufferOr.get()));
    if (!reader) {
      diag.report(diag::err_cannot_read_file)
          << (*file)->getName() << toString(reader.takeError());
      return false;
    }

    FileListVisitor visitor(fm, diag, headerFiles);
    reader.get()->visit(visitor);
    if (diag.hasErrorOccurred())
      return false;
  }

  for (const auto &path : opts.tapiOptions.extraPublicHeaders) {
    if (fm.exists(path)) {
      SmallString<PATH_MAX> fullPath(path);
      fm.makeAbsolutePath(fullPath);
      std::string includeName;
      if (auto nameOrNull = createIncludeHeaderName(fullPath))
        includeName = *nameOrNull;
      headerFiles.emplace_back(fullPath, HeaderType::Public,
                               /*relativePath=*/StringRef(), includeName);
      headerFiles.back().isExtra = true;
    } else {
      diag.report(diag::err_no_such_header_file)
          << path << (unsigned)HeaderType::Public;
      return false;
    }
  }

  for (const auto &path : opts.tapiOptions.extraPrivateHeaders) {
    if (fm.exists(path)) {
      SmallString<PATH_MAX> fullPath(path);
      fm.makeAbsolutePath(fullPath);
      std::string includeName;
      if (auto nameOrNull = createIncludeHeaderName(fullPath))
        includeName = *nameOrNull;
      headerFiles.emplace_back(fullPath, HeaderType::Private,
                               /*relativePath=*/StringRef(), includeName);
      headerFiles.back().isExtra = true;
    } else {
      diag.report(diag::err_no_such_header_file)
          << path << (unsigned)HeaderType::Private;
      return false;
    }
  }

  for (const auto &path : opts.tapiOptions.extraProjectHeaders) {
    if (fm.exists(path)) {
      SmallString<PATH_MAX> fullPath(path);
      fm.makeAbsolutePath(fullPath);
      headerFiles.emplace_back(fullPath, HeaderType::Project);
      headerFiles.back().isExtra = true;
    } else {
      diag.report(diag::err_no_such_header_file)
          << path << (unsigned)HeaderType::Project;
      return false;
    }
  }

  std::vector<std::unique_ptr<HeaderGlob>> excludeHeaderGlobs;
  std::set<const FileEntry *> excludeHeaderFiles;
  auto parseGlobs = [&](const PathSeq &paths, HeaderType type) {
    for (const auto &str : paths) {
      auto glob = HeaderGlob::create(str, type);
      if (glob)
        excludeHeaderGlobs.emplace_back(std::move(glob.get()));
      else {
        consumeError(glob.takeError());
        if (auto file = fm.getFile(str))
          excludeHeaderFiles.emplace(*file);
        else {
          diag.report(diag::err_no_such_header_file) << str << (unsigned)type;
          return false;
        }
      }
    }
    return true;
  };

  if (!parseGlobs(opts.tapiOptions.excludePublicHeaders, HeaderType::Public))
    return false;

  if (!parseGlobs(opts.tapiOptions.excludePrivateHeaders, HeaderType::Private))
    return false;

  if (!parseGlobs(opts.tapiOptions.excludeProjectHeaders, HeaderType::Project))
    return false;

  for (auto &header : headerFiles) {
    for (auto &glob : excludeHeaderGlobs)
      if (glob->match(header))
        header.isExcluded = true;
  }

  if (!excludeHeaderFiles.empty()) {
    for (auto &header : headerFiles) {
      auto file = fm.getFile(header.fullPath);
      if (!file)
        continue;
      if (excludeHeaderFiles.count(*file))
        header.isExcluded = true;
    }
  }

  for (const auto &glob : excludeHeaderGlobs)
    if (!glob->didMatch())
      diag.report(diag::warn_glob_did_not_match) << glob->str();

  // Check if the framework has an umbrella header and move that to the
  // beginning.
  auto markandMoveUmbrellaInHeaders = [](HeaderSeq &array, Regex &regex,
                                         HeaderType type) -> bool {
    auto it = find_if(array, [&regex, type](const HeaderFile &header) {
      return (header.type == type) && regex.match(header.fullPath);
    });

    if (it == array.end())
      return false;

    it->isUmbrellaHeader = true;

    // Because there can be an umbrella header per header type (except project),
    // find the first non umbrella header to swap position with.
    auto beginPos = find_if(array, [](const HeaderFile &header) {
      return !header.isUmbrellaHeader;
    });
    if (beginPos != array.end() && beginPos < it)
      std::swap(*beginPos, *it);
    return true;
  };

  const auto &publicUmbrellaHeaderPath =
      opts.tapiOptions.publicUmbrellaHeaderPath;
  if (!publicUmbrellaHeaderPath.empty()) {
    auto escapedString = Regex::escape(publicUmbrellaHeaderPath);
    Regex umbrellaRegex(escapedString);

    if (!markandMoveUmbrellaInHeaders(headerFiles, umbrellaRegex,
                                      HeaderType::Public)) {
      diag.report(diag::err_no_such_umbrella_header_file)
          << publicUmbrellaHeaderPath << (unsigned)HeaderType::Public;
      return false;
    }
  } else if (!frameworkName.empty()) {
    auto umbrellaName = "/" + Regex::escape(frameworkName) + "\\.h";
    Regex umbrellaRegex(umbrellaName);

    markandMoveUmbrellaInHeaders(headerFiles, umbrellaRegex,
                                 HeaderType::Public);
  }

  const auto &privateUmbrellaHeaderPath =
      opts.tapiOptions.privateUmbrellaHeaderPath;
  if (!privateUmbrellaHeaderPath.empty()) {
    auto escapedString = Regex::escape(privateUmbrellaHeaderPath);
    Regex umbrellaRegex(escapedString);

    if (!markandMoveUmbrellaInHeaders(headerFiles, umbrellaRegex,
                                      HeaderType::Private)) {
      diag.report(diag::err_no_such_umbrella_header_file)
          << privateUmbrellaHeaderPath << (unsigned)HeaderType::Private;
      return false;
    }
  } else if (!frameworkName.empty()) {
    auto umbrellaName = "/" + Regex::escape(frameworkName) + "[_]?Private\\.h";
    Regex umbrellaRegex(umbrellaName);

    markandMoveUmbrellaInHeaders(headerFiles, umbrellaRegex,
                                 HeaderType::Private);
  }

  // When code coverage is enabled we need to generate extra symbols manually.
  // These symbols are defined in libclang_rt.profile_*.a and are pulled in by
  // clang when -fprofile-instr-generate is specified on the command line.
  //
  // This needs to happen after we removed the re-exported library symbols, or
  // we will remove the code coverage symbols too.
  APIs coverageSymbols;
  if (opts.tapiOptions.generateCodeCoverageSymbols) {
    auto syms = getCodeCoverageSymbols(diag, manager, allTargets,
                                       opts.frontendOptions.isysroot);
    if (!syms) {
      diag.report(diag::err) << "could not generate coverage symbols"
                             << toString(syms.takeError());
      return false;
    }
    coverageSymbols = std::move(*syms);
  }

  //
  // Collect binary dylib information.
  //
  BinaryInfo binaryInfo;
  binaryInfo.currentVersion = opts.linkerOptions.currentVersion;
  binaryInfo.compatibilityVersion = opts.linkerOptions.compatibilityVersion;
  binaryInfo.isAppExtensionSafe = opts.linkerOptions.isApplicationExtensionSafe;
  binaryInfo.isOSLibNotForSharedCache =
      opts.linkerOptions.isOSLibNotForSharedCache;
  binaryInfo.parentUmbrella = opts.frontendOptions.umbrella;
  binaryInfo.installName = opts.linkerOptions.installName;
  binaryInfo.path = opts.tapiOptions.verifyAgainst;
  LibAttrs allowableClients;
  addToLibrary(opts.linkerOptions.allowableClients, allowableClients);
  LibAttrs relinkedLibraries;
  relinkedLibraries.insert(opts.linkerOptions.relinkedLibraries.begin(),
                           opts.linkerOptions.relinkedLibraries.end());
  LibAttrs rpaths;
  addToLibrary(opts.linkerOptions.rpaths, rpaths);

  // Collect Swift information.
  std::unique_ptr<InterfaceFile> swiftFile = nullptr;
  if (!opts.tapiOptions.swiftInstallAPIInterfaces.empty()) {
    auto result = handleSwiftInput(diag, binaryInfo, manager, allTargets,
                                   opts.tapiOptions.swiftInstallAPIInterfaces);
    if (!result) // Any errors were already reported.
      return false;
    swiftFile = std::move(result);
  }

  // Collect symbols from alias lists.
  std::map<SimpleSymbol, SimpleSymbol> aliases;
  // Hold ownership of buffer, after the alias list has been parsed.
  llvm::SmallVector<std::unique_ptr<MemoryBuffer>, 2> aliasBuffers;
  for (const auto &it : opts.linkerOptions.aliasLists) {
    std::unique_ptr<MemoryBuffer> buffer = nullptr;
    auto result = parseAliasList(fm, it, buffer);
    if (!result) {
      diag.report(diag::err)
          << "could not read alias list" << toString(result.takeError());
      return false;
    }
    aliases.insert(result.get().begin(), result.get().end());
    aliasBuffers.emplace_back(std::move(buffer));
  }

  //
  // If information was found, initialize verifier.
  //
  const bool verifySyms = !opts.tapiOptions.verifyAgainst.empty();
  bool passedBinary = true;
  APIs dylib;
  if (verifySyms) {
    auto file = readFile(manager, diag, opts.tapiOptions.verifyAgainst);
    if (!file)
      return false;
    dylib = std::move(*file);

    // Compare Binary Info.
    if (!verifyBinaryInfo(autoZippered, allTargets, dylib, binaryInfo,
                          allowableClients, reexportedLibraries,
                          relinkedLibraries, rpaths, diag,
                          opts.tapiOptions.fileType))
      return false;
  }

  bool hasMacOS = false, hasMacCatalyst = false;
  for (const auto &target : allTargets) {
    switch (mapToPlatformType(target)) {
    case PLATFORM_MACOS:
      hasMacOS = true;
      continue;
    case PLATFORM_MACCATALYST:
      hasMacCatalyst = true;
      continue;
    default:
      break;
    }
  }

  job.verifier = std::make_unique<SymbolVerifier>(SymbolVerifier{
      &diag, std::move(dylib), swiftFile.get(),
      opts.tapiOptions.verificationMode, opts.tapiOptions.demangle,
      autoZippered, hasMacOS && hasMacCatalyst,
      std::move(reexportedLibraryFiles), std::move(coverageSymbols),
      std::move(aliases), opts.tapiOptions.dSYM});

  // Ignore swift verification if option is not enabled.
  if (opts.tapiOptions.verifySwift) {
    if (job.verifier->verifySwift() < SymbolVerifier::Result::Ignore)
      return false;
  }

  // Infer additional include paths.
  std::set<std::string> inferredIncludePaths;
  if (opts.tapiOptions.inferIncludePaths) {
    for (const auto &header : headerFiles) {
      // Never infer include path for project headers.
      if (header.type == HeaderType::Project)
        continue;
      if (header.isExcluded)
        continue;
      inferredIncludePaths.insert(
          sys::path::parent_path(header.fullPath).str());

      auto n = header.fullPath.rfind("/include/");
      if (n == std::string::npos)
        continue;
      auto path = header.fullPath.substr(0, n + 8);
      inferredIncludePaths.insert(path);
    }
  }

  if (!job.useRelativePath)
    job.includePaths.insert(job.includePaths.begin(),
                            inferredIncludePaths.begin(),
                            inferredIncludePaths.end());

  // Only sort the headers for framework that didn't have a json input file.
  // Fixme: Need to fix all projects that still depend on this behavior.
  if (!inputPaths.empty())
    std::stable_sort(headerFiles.begin(), headerFiles.end());
  job.headerFiles = headerFiles;

  std::vector<FrontendContext> frontendResults;
  auto runAndRecordFrontendResults = [&](const Twine label,
                                         ArrayRef<std::string> args = {}) {
    job.label = label.str();

    const auto initialArgs = job.clangExtraArgs;
    auto overrideJob = setOverridingOptionsToJob(
        diag, job, opts.getProjectHeaderOptions(), customModuleCache, args);
    if (!overrideJob)
      return false;

    auto contextOrError = runFrontend(*overrideJob);
    if (auto err = contextOrError.takeError())
      return canIgnoreFrontendError(err);

    frontendResults.emplace_back(std::move(*contextOrError));
    return true;
  };

  for (auto &target : allTargets) {
    auto systemFrameworkPaths = getPathsForPlatform(
        opts.frontendOptions.systemFrameworkPaths, mapToPlatformType(target));

    for (auto &path : systemFrameworkPaths)
      frameworkSearchPaths.emplace_back(path);
    frameworkSearchPaths.resize(frameworkSearchPaths.size() -
                                systemFrameworkPaths.size());
    job.systemFrameworkPaths = systemFrameworkPaths;
    job.target = target;
    job.verifier->setTarget(target);
    for (auto type :
         {HeaderType::Public, HeaderType::Private, HeaderType::Project}) {
      job.type = type;
      const StringRef headerLabel = getName(job.type);
      if (!runAndRecordFrontendResults(headerLabel))
        return false;
      // Run extra passes for unique compiler arguments.
      for (const auto &[label, extraArgs] :
           opts.frontendOptions.uniqueClangArgs)
        if (!runAndRecordFrontendResults(label + " " + headerLabel, extraArgs))
          return false;
    }
  }

  // Clean up module cache after clang invocations have fun.
  if (customModuleCache)
    llvm::sys::fs::remove_directories(job.moduleCachePath,
                                      /*IgnoreErrors=*/true);

  if (opts.tapiOptions.printAfter == "frontend") {
    APIPrinter printer(errs());
    for (auto &result : frontendResults) {
      errs() << "triple:" << result.target.str() << "\n";
      result.visit(printer);
      errs() << "\n";
    }
  }

  // Verify remaining symbols from binary per architecture.
  if (verifySyms) {
    for (auto T : allTargets) {
      auto arch = mapToArchitecture(T);
      if (job.verifier->verifyRemainingSymbols(arch) ==
            SymbolVerifier::Result::Invalid)
        passedBinary = false;
    }
  }

  bool passedFrontend =
      job.verifier->getFrontendState() >= SymbolVerifier::Result::Ignore;
  if (!passedFrontend || !passedBinary)
    return false;


  auto scanFile = std::make_unique<InterfaceFile>(job.verifier->getExports());
  scanFile->addTargets(allTargets);
  // TODO: modularize setting the BinaryInfo along with when its done in
  // Utils.cpp
  scanFile->setInstallName(opts.linkerOptions.installName);
  scanFile->setCurrentVersion(opts.linkerOptions.currentVersion);
  scanFile->setCompatibilityVersion(opts.linkerOptions.compatibilityVersion);
  scanFile->setTwoLevelNamespace();
  scanFile->setApplicationExtensionSafe(
      opts.linkerOptions.isApplicationExtensionSafe);
  scanFile->setOSLibNotForSharedCache(
      opts.linkerOptions.isOSLibNotForSharedCache);
  for (const auto &lib : opts.linkerOptions.allowableClients)
    for (const auto &target : scanFile->targets(lib.second))
      scanFile->addAllowableClient(lib.first, target);
  for (const auto &lib : opts.linkerOptions.reexportInstallNames)
    for (const auto &target : scanFile->targets(lib.second))
      scanFile->addReexportedLibrary(lib.first, target);
  for (const auto &lib : reexportedLibraries)
    for (const auto &target : scanFile->targets(lib.getArchitectures()))
      scanFile->addReexportedLibrary(lib.getInstallName(), target);
  if (!opts.frontendOptions.umbrella.empty()) {
    for (const auto &target : scanFile->targets())
      scanFile->addParentUmbrella(target, opts.frontendOptions.umbrella);
  }

  // Guard new attributes unless the file format supports them.
  if (opts.tapiOptions.fileType >= FileType::TBD_V5) {
    for (const auto &[path, archs] : opts.linkerOptions.rpaths)
      for (const auto &target : scanFile->targets(archs))
        scanFile->addRPath(target, path);
  }

  if (swiftFile) {
    // After verification has passed, merge back in swift content for final
    // output.
    if (!mergeInterfaces(diag, scanFile, swiftFile))
      return false;
  }
  scanFile->setFileType(opts.tapiOptions.fileType);

  if (opts.driverOptions.outputPath.empty()) {
    SmallString<PATH_MAX> path;
    if (auto ec = sys::fs::current_path(path)) {
      diag.report(diag::err) << path << ec.message();
      return false;
    }
    auto targetName = sys::path::stem(opts.linkerOptions.installName);
    sys::path::append(path, targetName);
    TAPI_INTERNAL::replace_extension(path, ".tbd");
    opts.driverOptions.outputPath = path.str().str();
  }

  
  if (!createDirForOutput(opts.driverOptions.outputPath))
    return false;
  auto result = manager.writeFile(opts.driverOptions.outputPath, scanFile.get(),
                                  opts.tapiOptions.fileType);
  if (result) {
    diag.report(diag::err_cannot_write_file)
        << opts.driverOptions.outputPath << toString(std::move(result));
    return false;
  }

  if (opts.tapiOptions.sdkdbOutputPath.empty())
    return true;

  // Write SDKDB output.
  SmallString<PATH_MAX> outputPath(opts.tapiOptions.sdkdbOutputPath);
  if (!createDirForOutput(outputPath.str(), /*isFile=*/false))
    return false;

  auto filename = sys::path::filename(opts.driverOptions.outputPath);
  sys::path::append(outputPath, filename);
  sys::path::replace_extension(outputPath, ".partial.sdkdb");

  std::error_code errorCode;
  raw_fd_ostream fs(outputPath, errorCode);
  if (errorCode) {
    diag.report(diag::err_cannot_open_file)
        << outputPath << errorCode.message();
    return false;
  }

  APINormalizer normalizer(isPublicDylib(opts.linkerOptions.installName));
  for (auto &result : frontendResults) {
    result.api->visit(normalizer);
  }

  if (auto err = PartialSDKDB::serialize(
          fs, /*omit name*/ "", std::vector<API>(),
          std::vector<FrontendContext>(), std::vector<API>(), frontendResults,
          std::vector<API>(), /*hasError*/ false)) {
    diag.report(diag::err_cannot_generate_sdkdb) << toString(std::move(err));
    return false;
  }

  return true;
}

TAPI_NAMESPACE_INTERNAL_END
