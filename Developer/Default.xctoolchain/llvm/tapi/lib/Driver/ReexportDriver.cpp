//===- lib/Driver/ReexportDriver.cpp - TAPI Reexport Driver -----*- C++ -*-===//
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

#include "tapi/Core/ReexportFileWriter.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/Driver.h"
#include "tapi/Driver/Options.h"
#include "tapi/Frontend/Frontend.h"
#include "clang/Driver/DriverDiagnostic.h"

using namespace llvm;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

/// \brief Parse the headers and generate an re-export file for the linker.
bool Driver::Reexport::run(DiagnosticsEngine &diag, Options &opts) {
  auto &fm = opts.getFileManager();

  // Handle targets.
  if (opts.frontendOptions.targets.empty()) {
    diag.report(diag::err_no_target);
    return false;
  }

  // Set default language option.
  if (opts.frontendOptions.language == clang::Language::Unknown)
    opts.frontendOptions.language = clang::Language::ObjC;

  // Only allow one target.
  if (opts.frontendOptions.targets.size() > 1) {
    diag.report(diag::err_one_target);
    return false;
  }
  const auto &target = opts.frontendOptions.targets.front();

  // Handle input files.
  if (opts.driverOptions.inputs.empty()) {
    diag.report(clang::diag::err_drv_no_input_files);
    return false;
  }

  HeaderSeq files;
  for (const auto &path : opts.driverOptions.inputs) {
    if (fm.exists(path))
      files.emplace_back(path, HeaderType::Project);
    else {
      diag.report(diag::err_cannot_open_file) << path << "File does not exist";
      return false;
    }
  }

  FrontendJob job;
  job.target = target;
  job.language = opts.frontendOptions.language;
  job.language_std = opts.frontendOptions.language_std;
  job.overwriteRTTI = opts.frontendOptions.useRTTI;
  job.overwriteNoRTTI = opts.frontendOptions.useNoRTTI;
  job.visibility = opts.frontendOptions.visibility;
  job.isysroot = opts.frontendOptions.isysroot;
  job.macros = opts.frontendOptions.macros;
  job.systemFrameworkPaths =
      getAllPaths(opts.frontendOptions.systemFrameworkPaths);
  job.systemIncludePaths = opts.frontendOptions.systemIncludePaths;
  job.afterIncludePaths = opts.frontendOptions.afterIncludePaths;
  job.quotedIncludePaths = opts.frontendOptions.quotedIncludePaths;
  job.frameworkPaths = opts.frontendOptions.frameworkPaths;
  job.includePaths = opts.frontendOptions.includePaths;
  job.clangExtraArgs = opts.frontendOptions.clangExtraArgs;
  job.headerFiles = std::move(files);
  job.clangResourcePath = opts.frontendOptions.clangResourcePath;
  job.useObjectiveCARC = opts.frontendOptions.useObjectiveCARC;
  job.useObjectiveCWeakARC = opts.frontendOptions.useObjectiveCWeakARC;
  job.type = HeaderType::Project;
  job.verbose = opts.frontendOptions.verbose;

  // Infer additional include paths.
  std::set<std::string> inferredIncludePaths;
  for (const auto &header : job.headerFiles)
    inferredIncludePaths.insert(sys::path::parent_path(header.fullPath).str());

  job.includePaths.insert(job.includePaths.end(), inferredIncludePaths.begin(),
                          inferredIncludePaths.end());

  auto contextOrError = runFrontend(job);
  if (auto err = contextOrError.takeError())
    return canIgnoreFrontendError(err);

  ReexportFileWriter writer(target);
  contextOrError->visit(writer);

  SmallString<PATH_MAX> outputPath(opts.driverOptions.outputPath);
  if (outputPath.empty()) {
    auto ec = sys::fs::current_path(outputPath);
    if (ec) {
      diag.report(diag::err) << outputPath << ec.message();
      return false;
    }
    sys::path::append(outputPath, "linker.reexport");
  }

  SmallString<PATH_MAX> outputDir(outputPath);
  sys::path::remove_filename(outputDir);
  auto ec = sys::fs::create_directories(outputDir);
  if (ec) {
    diag.report(diag::err_cannot_create_directory) << outputDir << ec.message();
    return false;
  }

  std::error_code err;
  raw_fd_ostream os(outputPath, err);
  if (err) {
    diag.report(diag::err_cannot_write_file)
        << outputPath.str() << err.message();
    return false;
  }

  writer.writeToStream(os);

  os.close();
  if (err) {
    diag.report(diag::err_cannot_write_file)
        << outputPath.str() << err.message();
    return false;
  }

  return true;
}

TAPI_NAMESPACE_INTERNAL_END
