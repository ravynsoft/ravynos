//===- tapi/APIVerifier/APIVerifier.h - TAPI API Verifier -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the TAPI API Verifier.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_APIVERIFIER_APIVERIFIER_H
#define TAPI_APIVERIFIER_APIVERIFIER_H

#include "tapi/Core/APIVisitor.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Frontend/FrontendContext.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <vector>

TAPI_NAMESPACE_INTERNAL_BEGIN

struct APIVerifierConfiguration {
  using BridgeTypes = std::pair<std::string, std::string>;
  std::vector<std::string> IgnoreObjCClasses;
  std::vector<BridgeTypes> BridgeObjCClasses;

  llvm::Error readConfig(llvm::MemoryBufferRef memBuffer);
  void writeConfig(llvm::raw_ostream &os);
};

enum class APIVerifierDiagStyle {
  Silent = 0,
  Warning = 1,
  Error = 2
};

class APIVerifier {
public:
  APIVerifier(DiagnosticsEngine &diag) : diag(diag) {}
  void verify(FrontendContext &api1, FrontendContext &api2, unsigned depth = 0,
              bool external = true,
              APIVerifierDiagStyle style = APIVerifierDiagStyle::Warning,
              bool diagMissingAPI = false, bool avoidCascadingDiags = false);
  APIVerifierConfiguration &getConfiguration() { return config; }
  bool hasErrorOccurred() const { return hasError; }

private:
  DiagnosticsEngine &diag;
  APIVerifierConfiguration config;
  bool hasError = false;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_APIVERIFIER_APIVERIFIER_H
