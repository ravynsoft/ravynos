//===- lib/Driver/APIVerifyDriver.cpp - API Verify Driver -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// A tool compares APIs between two targets/frameworks.
///
//===----------------------------------------------------------------------===//
#include "tapi/APIVerifier/APIVerifier.h"
#include "tapi/Config/Version.h"
#include "tapi/Core/APIJSONSerializer.h"
#include "tapi/Core/APIPrinter.h"
#include "tapi/Core/HeaderFile.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/DirectoryScanner.h"
#include "tapi/Driver/Driver.h"
#include "tapi/Driver/HeaderGlob.h"
#include "tapi/Driver/Options.h"
#include "tapi/Frontend/Frontend.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/Version.inc"
#include "clang/Config/config.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;
using namespace TAPI_INTERNAL;

namespace {

struct APIComparsionContext {
  std::string target;
  std::string sysroot;
  std::vector<std::string> additionalIncludes;
  std::vector<std::string> additionalFrameworks;
  std::string path;
};

struct APIComparsionConfiguration {
  bool skipExtern;
  bool missingAPI;
  bool noCascadingDiags;
  bool comparePrivateHeaders;
  unsigned diagnosticDepth;
  std::string allowlist;
  std::string diagStyle;
  APIComparsionContext base;
  APIComparsionContext variant;
};

} // end anonymous namespace.

// YAML Traits
namespace llvm {
namespace yaml {

template <> struct MappingTraits<APIComparsionContext> {
  static void mapping(IO &io, APIComparsionContext &config) {
    io.mapRequired("target", config.target);
    io.mapRequired("sysroot", config.sysroot);
    io.mapOptional("includes", config.additionalIncludes);
    io.mapOptional("frameworks", config.additionalFrameworks);
    io.mapRequired("path", config.path);
  }
};

template <> struct MappingTraits<APIComparsionConfiguration> {
  static void mapping(IO &io, APIComparsionConfiguration &config) {
    io.mapRequired("base", config.base);
    io.mapRequired("variant", config.variant);
    io.mapOptional("skip-external", config.skipExtern, false);
    io.mapOptional("missing-api", config.missingAPI, false);
    io.mapOptional("no-cascading-diags", config.noCascadingDiags, false);
    io.mapOptional("compare-private-headers", config.comparePrivateHeaders,
                   false);
    io.mapOptional("diag-depth", config.diagnosticDepth, 4);
    io.mapOptional("allowlist", config.allowlist);
    io.mapOptional("diag-style", config.diagStyle);
  }
};

} // end namespace yaml
} // end namespace llvm

static bool populateHeaderSeq(StringRef path, HeaderSeq &headerFiles,
                              FileManager &fm, DiagnosticsEngine &diag) {
  DirectoryScanner scanner(fm, diag, ScannerMode::ScanFrameworks);

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

  if (!framework->_versions.empty())
    framework = &framework->_versions.back();

  for (const auto &header : framework->_headerFiles) {
    auto file = fm.getFile(header.fullPath);
    if (!file) {
      diag.report(diag::err_no_such_header_file)
          << header.fullPath << (unsigned)header.type;
      return false;
    }
    headerFiles.emplace_back(header);
  }

  // Check if the framework has an umbrella header and move that to the
  // beginning.
  auto matchAndMarkUmbrella = [](HeaderSeq &array, Regex &regex,
                                 HeaderType type) -> bool {
    auto it = find_if(array, [&regex, type](const HeaderFile &header) {
      return (header.type == type) && regex.match(header.fullPath);
    });

    if (it == array.end())
      return false;

    it->isUmbrellaHeader = true;
    return true;
  };

  auto frameworkName = sys::path::stem(framework->getName());
  {
    auto umbrellaName = "/" + Regex::escape(frameworkName) + "\\.h";
    Regex umbrellaRegex(umbrellaName);
    matchAndMarkUmbrella(headerFiles, umbrellaRegex, HeaderType::Public);
  }

  {
    auto umbrellaName = "/" + Regex::escape(frameworkName) + "[_]?Private\\.h";
    Regex umbrellaRegex(umbrellaName);

    matchAndMarkUmbrella(headerFiles, umbrellaRegex, HeaderType::Private);
  }

  std::stable_sort(headerFiles.begin(), headerFiles.end());
  return true;
}

TAPI_NAMESPACE_INTERNAL_BEGIN

/// \brief Parses the headers and generate a text-based stub file.
bool Driver::APIVerify::run(DiagnosticsEngine &diag, Options &opts) {
  if (opts.driverOptions.inputs.size() != 1) {
    diag.report(diag::err_expected_one_input_file);
    return false;
  }

  auto inputFilename = opts.driverOptions.inputs.front();
  // Parse input.
  APIComparsionConfiguration config;
  {
    auto inputBuf = MemoryBuffer::getFile(inputFilename);
    if (!inputBuf) {
      diag.report(diag::err_cannot_open_file)
          << inputFilename << inputBuf.getError().message();
      return false;
    }
    yaml::Input yin((*inputBuf)->getMemBufferRef());
    yin >> config;
    if (yin.error()) {
      diag.report(diag::err_invalid_input_file) << inputFilename;
      return false;
    }
  }

  std::vector<FrontendContext> results;
  FileManager fm((clang::FileSystemOptions()));
  auto parseHeaders = [&](APIComparsionContext &context) {
    FrontendJob job;
    HeaderSeq headers;
    if (!populateHeaderSeq(context.path, headers, fm, diag))
      return false;
    job.target = Triple(context.target);
    job.isysroot = context.sysroot;
    job.language = opts.frontendOptions.language == clang::Language::Unknown
                       ? clang::Language::ObjC
                       : opts.frontendOptions.language;
    job.language_std = opts.frontendOptions.language_std;
    job.verbose = opts.frontendOptions.verbose;
    job.clangExtraArgs = opts.frontendOptions.clangExtraArgs;
    job.headerFiles = headers;
    job.type = config.comparePrivateHeaders ? HeaderType::Private : HeaderType::Public;
    job.clangResourcePath = opts.frontendOptions.clangResourcePath;
    job.frameworkPaths = opts.frontendOptions.frameworkPaths;
    job.systemFrameworkPaths = context.additionalFrameworks;
    job.systemIncludePaths = context.additionalIncludes;
    job.afterIncludePaths = opts.frontendOptions.afterIncludePaths;
    auto contextOrError = runFrontend(job);
    if (auto err = contextOrError.takeError())
      return canIgnoreFrontendError(err);
    results.emplace_back(std::move(*contextOrError));
    return true;
  };

  if (!parseHeaders(config.base) || !parseHeaders(config.variant))
    return false;

  APIVerifier apiVerifier(diag);
  if (!config.allowlist.empty()) {
    auto inputBuf = MemoryBuffer::getFile(config.allowlist);
    if (!inputBuf) {
      diag.report(diag::err_invalid_verifier_allowlist_file) << config.allowlist;
      return false;
    }

    auto error = apiVerifier.getConfiguration().readConfig(
        (*inputBuf)->getMemBufferRef());
    if (error) {
      diag.report(diag::err_invalid_verifier_allowlist_file)
          << toString(std::move(error));
      return false;
    }
  }

  auto style = StringSwitch<APIVerifierDiagStyle>(config.diagStyle)
                   .Case("slient", APIVerifierDiagStyle::Silent)
                   .Case("warning", APIVerifierDiagStyle::Warning)
                   .Case("error", APIVerifierDiagStyle::Error)
                   .Default(APIVerifierDiagStyle::Error);
  apiVerifier.verify(results.front(), results.back(), config.diagnosticDepth,
                     !config.skipExtern, style, config.missingAPI,
                     config.noCascadingDiags);

  return 0;
}

TAPI_NAMESPACE_INTERNAL_END
