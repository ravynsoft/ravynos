//===- tapi/SDKDB/PartialSDKDB.h - TAPI PartialSDKDB ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Partial SDKDB representation and helper functions.
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_SDKDB_PARTIALSDKDB_H
#define TAPI_SDKDB_PARTIALSDKDB_H

#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include "tapi/Core/API.h"
#include "tapi/Frontend/FrontendContext.h"

#include <functional>

TAPI_NAMESPACE_INTERNAL_BEGIN

struct PartialSDKDB {

  static const char* version;

  /// return partial SDKDB with binaryInterfaces and publicHeaderInterfaces.
  static llvm::Expected<PartialSDKDB>
  createPublicAPIsFromJSON(llvm::json::Object &input);

  /// return partial SDKDB with binaryInterfaces and privateHeaderInterfaces.
  static llvm::Expected<PartialSDKDB>
  createPrivateAPIsFromJSON(llvm::json::Object &input);

  /// write partial SDKDB from APIs and FrontendContexts.
  static llvm::Error
  serialize(llvm::raw_ostream &os, StringRef project,
            const std::vector<API> &binaryInterfaces,
            const std::vector<FrontendContext> &publicHeaderContext,
            const std::vector<API> &publicHeaderAPIs,
            const std::vector<FrontendContext> &privateHeaderContext,
            const std::vector<API> &privateHeaderAPIs,
            bool hasErrors, bool useCompactFormat = false);

  std::vector<API> binaryInterfaces;
  std::vector<API> headerInterfaces;
  std::string project;
  bool hasError = false;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_SDKDB_PARTIALSDKDB_H
