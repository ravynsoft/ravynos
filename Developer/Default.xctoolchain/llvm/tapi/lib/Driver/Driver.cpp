//===- lib/Driver/Driver.cpp - TAPI Driver ----------------------*- C++ -*-===//
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
/// Currently the driver only provides the basic framework for option parsing.
///
//===----------------------------------------------------------------------===//

#include "tapi/Driver/Driver.h"
#include "tapi/Config/Version.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Driver/Options.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

IntrusiveRefCntPtr<DiagnosticsEngine>
Driver::createDiagnosticsEngine(raw_ostream &errorStream) {
  return new DiagnosticsEngine(errorStream);
}

bool Driver::run(DiagnosticsEngine &diag, Options &options) {
  // Handle -version.
  if (options.driverOptions.printVersion) {
    outs() << getTAPIFullVersion() << "\n";
    return true;
  }

  return true;
}

/// Parses the command line options and performs the requested action.
bool Driver::run(ArrayRef<const char *> args) {
  auto diag = createDiagnosticsEngine();
  // Parse command line options using TAPIOptions.td.

  Options options(*diag, args);

  // Check if there have been errors during the option parsing.
  if (diag->hasErrorOccurred())
    return false;

  if (options.driverOptions.printHelp) {
    options.printHelp();
    return true;
  }

  switch (options.command) {
  case TAPICommand::Driver:
    return Driver::run(*diag, options);
  case TAPICommand::Archive:
    return Archive::run(*diag, options);
  case TAPICommand::Stubify:
    return Stub::run(*diag, options);
  case TAPICommand::InstallAPI:
    return InstallAPI::run(*diag, options);
  case TAPICommand::Reexport:
    return Reexport::run(*diag, options);
  case TAPICommand::SDKDB:
    return SDKDB::run(*diag, options);
  case TAPICommand::APIVerify:
    return APIVerify::run(*diag, options);
  }
  llvm_unreachable("invalid/unknown driver command");
}

TAPI_NAMESPACE_INTERNAL_END
