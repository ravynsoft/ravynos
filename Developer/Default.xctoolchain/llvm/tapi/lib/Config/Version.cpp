//===- lib/Config/Version.cpp - TAPI Version Number --------------*- C++-=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements several version-related utility functions for TAPI.
///
//===----------------------------------------------------------------------===//

#include "llvm/Support/raw_ostream.h"
#include "tapi/Config/Version.h"
#include "tapi/Version.inc"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

std::string getTAPIFullVersion() {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef TAPI_VENDOR
  OS << TAPI_VENDOR;
#endif
  OS << "TAPI version " TAPI_MAKE_STRING(TAPI_VERSION);
#ifdef TAPI_REPOSITORY_STRING
  OS << " (" TAPI_REPOSITORY_STRING ")";
#endif

  return OS.str();
}

TAPI_NAMESPACE_INTERNAL_END
