//===- tapi-frontend/tapi-frontend.cpp - TAPI Frontend Tool -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// A tool to run the TAPI frontent for testing.
///
//===----------------------------------------------------------------------===//
#include "tapi/APIVerifier/APIVerifier.h"
#include "tapi/Config/Version.h"
#include "tapi/Core/APIJSONSerializer.h"
#include "tapi/Core/APIPrinter.h"
#include "tapi/Core/HeaderFile.h"
#include "tapi/Diagnostics/Diagnostics.h"
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
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;
using namespace TAPI_INTERNAL;

static cl::OptionCategory tapiCategory("tapi-frontend options");

static cl::list<std::string> targets("target", cl::desc("target triple"),
                                     cl::value_desc("triple"),
                                     cl::cat(tapiCategory));

static cl::opt<std::string>
    isysroot("isysroot", cl::desc("Set the system root directory (usually /)"),
             cl::value_desc("dir"), cl::cat(tapiCategory));

static cl::opt<std::string> language_std("std",
                                         cl::desc("Set the language standard"),
                                         cl::value_desc("<lang>"),
                                         cl::cat(tapiCategory));

static cl::list<std::string> xparser("Xparser", cl::cat(tapiCategory));

static cl::opt<std::string> allowlist("allowlist",
                                      cl::desc("allowlist YAML file"),
                                      cl::cat(tapiCategory));

static cl::opt<std::string> inputFilename(cl::Positional,
                                          cl::desc("<input file>"),
                                          cl::cat(tapiCategory));
static cl::opt<std::string> jsonOutput("json", cl::desc("output json file"),
                                       cl::cat(tapiCategory));

static cl::opt<bool> verbose("v", cl::desc("verbose"), cl::cat(tapiCategory));
static cl::opt<bool> noColors("no-colors", cl::desc("don't use color output"), cl::cat(tapiCategory));
static cl::opt<bool> noPrint("no-print", cl::desc("don't print the API"),
                             cl::cat(tapiCategory));
static cl::opt<bool> verify("verify", cl::desc("run verifier"),
                            cl::cat(tapiCategory));
static cl::opt<APIVerifierDiagStyle> diagStyle(
    "verifier-diag-style", cl::init(APIVerifierDiagStyle::Warning),
    cl::desc("APIVerifier Diagnostic Style, options: silent, warning, error"),
    cl::values(clEnumValN(APIVerifierDiagStyle::Silent, "silent", "Silent"),
               clEnumValN(APIVerifierDiagStyle::Warning, "warning", "Warning"),
               clEnumValN(APIVerifierDiagStyle::Error, "error", "Error")));
static cl::opt<bool> skipExtern("skip-external-headers",
                                cl::desc("skip external headers"),
                                cl::cat(tapiCategory));
static cl::opt<bool> missingAPI("diag-missing-api",
                                cl::desc("diagnose missing api"),
                                cl::cat(tapiCategory));
static cl::opt<bool> noCascadingDiags("no-cascading-diagnostics",
                                cl::desc("disable cascading errors"),
                                cl::cat(tapiCategory));
static cl::opt<unsigned>
    diagnosticDepth("diag-depth",
                    cl::desc("depth of diagnostics (0 is ignored)"),
                    cl::cat(tapiCategory));

static std::string getClangResourcesPath(clang::FileManager &fm) {
  // Exists solely for the purpose of lookup of the resource path.
  // This just needs to be some symbol in the binary.
  static int staticSymbol;
  // The driver detects the builtin header path based on the path of the
  // executable.
  auto mainExecutable =
      sys::fs::getMainExecutable("tapi-frontend", &staticSymbol);
  StringRef dir = llvm::sys::path::parent_path(mainExecutable);

  // Compute the path to the resource directory.
  auto fileExists = [&fm](StringRef path) {
    llvm::vfs::Status result;
    if (fm.getNoncachedStatValue(path, result))
      return false;
    return result.exists();
  };

  // Try the default tapi path.
  SmallString<PATH_MAX>
      path(dir);
  llvm::sys::path::append(path, "..", CLANG_INSTALL_LIBDIR_BASENAME, "tapi",
                          TAPI_MAKE_STRING(TAPI_VERSION_MAJOR));
  if (fileExists(path))
    return path.str().str();

  // Try the default clang path. This is used by check-tapi.
  path = dir;
  llvm::sys::path::append(path, "..", CLANG_INSTALL_LIBDIR_BASENAME, "clang",
                          CLANG_VERSION_MAJOR_STRING);
  if (fileExists(path))
    return path.str().str();

  return std::string();
}

int main(int argc, const char *argv[]) {
  // Standard set up, so program fails gracefully.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram stackPrinter(argc, argv);
  llvm_shutdown_obj shutdown;

  if (sys::Process::FixupStandardFileDescriptors())
    return 1;

  cl::HideUnrelatedOptions(tapiCategory);
  cl::ParseCommandLineOptions(argc, argv, "TAPI Frontend Tool\n");

  if (inputFilename.empty()) {
    cl::PrintHelpMessage();
    return 0;
  }

  std::vector<FrontendContext> results;
  HeaderSeq headers;
  SmallString<PATH_MAX> fullPath(inputFilename);
  clang::FileManager fm((clang::FileSystemOptions()));
  fm.makeAbsolutePath(fullPath);
  headers.emplace_back(inputFilename, HeaderType::Public);
  for (const auto &target : targets) {
    FrontendJob job;
    job.target = Triple(target);
    job.isysroot = isysroot;
    job.language_std = language_std;
    job.verbose = verbose;
    job.clangExtraArgs = xparser;
    job.headerFiles = headers;
    job.clangResourcePath = getClangResourcesPath(fm);
    auto contextOrError = runFrontend(job, inputFilename);
    if (auto err = contextOrError.takeError()) {
      if (canIgnoreFrontendError(err))
        continue;
      return -1;
    }
    results.emplace_back(std::move(*contextOrError));
  }

  if (verify) {
    if (results.size() != 2) {
      errs() << "error: invalid number of targets to verify (expected exactly "
                "two taregts)\n";
      return -1;
    }
    DiagnosticsEngine diag;
    APIVerifier apiVerifier(diag);
    if (!allowlist.empty()) {
      auto inputBuf = MemoryBuffer::getFile(allowlist);
      if (!inputBuf) {
        errs() << "cannot open allowlist file: " << allowlist << "\n";
        return -1;
      }

      auto error = apiVerifier.getConfiguration().readConfig(
          (*inputBuf)->getMemBufferRef());
      if (error) {
        errs() << "cannot parse allowlist file: " << toString(std::move(error))
               << "\n";
        return -1;
      }
    }

    apiVerifier.verify(results.front(), results.back(), diagnosticDepth,
                       !skipExtern, diagStyle, missingAPI, noCascadingDiags);
  }

  if (!noPrint) {
    for (auto &result : results) {
      APIPrinter printer(errs(), !noColors);
      result.visit(printer);
    }
  }

  if (!jsonOutput.empty()) {
    std::error_code err;
    raw_fd_ostream jsonOut(jsonOutput, err);
    if (err) {
      errs() << "Cannot open \'" << jsonOutput
             << "\' for json output: " << err.message() << "\n";
      return 1;
    }

    for (auto &r : results) {
      APIJSONSerializer serializer(*r.api);
      serializer.serialize(jsonOut);
    }
  }

  return 0;
}
