//===- lib/Driver/ArchiveDriver.cpp - TAPI Archive Driver -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the archive driver for the tapi tool.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/InterfaceFileManager.h"
#include "tapi/Core/Registry.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/Driver.h"
#include "tapi/Driver/Options.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "llvm/TextAPI/Architecture.h"
#include "llvm/TextAPI/TextAPIError.h"

using namespace llvm;
using namespace llvm::MachO;

TAPI_NAMESPACE_INTERNAL_BEGIN

/// \brief Merge or thin text-based stub files.
bool Driver::Archive::run(DiagnosticsEngine &diag, Options &opts) {
  auto &fm = opts.getFileManager();
  const InterfaceFileManager manager(fm,
                                     /*isVolatile=*/opts.tapiOptions.isBnI);

  // Handle input files.
  if (opts.driverOptions.inputs.empty()) {
    diag.report(clang::diag::err_drv_no_input_files);
    return false;
  }

  switch (opts.archiveOptions.action) {
  default:
    break;
  case ArchiveAction::ShowInfo:
  case ArchiveAction::ExtractArchitecture:
  case ArchiveAction::RemoveArchitecture:
  case ArchiveAction::VerifyArchitecture:
  case ArchiveAction::ListSymbols:
    if (opts.driverOptions.inputs.size() != 1) {
      diag.report(diag::err_expected_one_input_file);
      return false;
    }
    break;
  }

  switch (opts.archiveOptions.action) {
  default:
    break;
  case ArchiveAction::ExtractArchitecture:
  case ArchiveAction::RemoveArchitecture:
  case ArchiveAction::Merge:
    if (opts.driverOptions.outputPath.empty()) {
      diag.report(diag::err_no_output_file);
      return false;
    }
    break;
  }

  Registry registry;
  registry.addYAMLReaders();
  registry.addYAMLWriters();
  registry.addJSONReaders();
  registry.addJSONWriters();

  std::vector<std::unique_ptr<InterfaceFile>> inputs;
  for (const auto &path : opts.driverOptions.inputs) {
    auto bufferOr = fm.getBufferForFile(path);
    if (auto ec = bufferOr.getError()) {
      diag.report(diag::err_cannot_read_file) << path << ec.message();
      return false;
    }

    auto file = registry.readTextFile(std::move(bufferOr.get()));
    if (!file) {
      diag.report(diag::err_cannot_read_file)
          << path << toString(file.takeError());
      return false;
    }

    if (file.get()->getFileType() == FileType::Invalid) {
      diag.report(diag::err_unsupported_file_type);
      return false;
    }
    inputs.emplace_back(std::move(file.get()));
  }

  std::unique_ptr<InterfaceFile> output;
  switch (opts.archiveOptions.action) {
  default:
    return false;

  case ArchiveAction::ShowInfo:
    assert(inputs.size() == 1 && "expecting exactly one input file");
    outs() << "Architectures: " << inputs.front()->getArchitectures() << '\n';
    break;

  case ArchiveAction::ExtractArchitecture: {
    assert(inputs.size() == 1 && "expecting exactly one input file");
    auto file = inputs.front()->extract(opts.archiveOptions.arch);
    if (!file) {
      diag.report(diag::err)
          << inputs.front()->getPath() << toString(file.takeError());
      return false;
    }
    output = std::move(file.get());
    break;
  }

  case ArchiveAction::RemoveArchitecture: {
    assert(inputs.size() == 1 && "expecting exactly one input file");
    auto file = inputs.front()->remove(opts.archiveOptions.arch);
    file = handleExpected(
        std::move(file), [&]() { return std::move(inputs.front()); },
        [&](std::unique_ptr<TextAPIError> error) -> Error {
          if (error->EC != TextAPIErrorCode::NoSuchArchitecture)
            return Error(std::move(error));
          diag.report(diag::warn)
              << ("file doesn't have architecture '" +
                  getArchitectureName(opts.archiveOptions.arch) + "'")
                     .str();
          return Error::success();
        });

    if (!file) {
      diag.report(diag::err)
          << inputs.front()->getPath() << toString(file.takeError());
      return false;
    }
    output = std::move(file.get());
    break;
  }

  case ArchiveAction::VerifyArchitecture:
    assert(inputs.size() == 1 && "expecting exactly one input file");
    return inputs.front()->getArchitectures().has(opts.archiveOptions.arch);

  case ArchiveAction::Merge: {
    assert(!inputs.empty() && "expecting at least one input file");
    for (auto &file : inputs) {
      if (!output) {
        output = std::move(file);
        continue;
      }

      auto result = output->merge(file.get());
      if (!result) {
        diag.report(diag::err)
            << file->getPath() << toString(result.takeError());
        return false;
      }
      output = std::move(result.get());
    }
    break;
  }
  case ArchiveAction::ListSymbols: {
    llvm_unreachable("unimplemented");
    break;
  }
  }

  if (output) {
    auto result = manager.writeFile(opts.driverOptions.outputPath, output.get(),
                                    output.get()->getFileType());
    if (result) {
      diag.report(diag::err_cannot_write_file)
          << opts.driverOptions.outputPath << toString(std::move(result));
      return false;
    }
  }

  return true;
}

TAPI_NAMESPACE_INTERNAL_END
