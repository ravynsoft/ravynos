//===- tapi/Core/APIJSONSerializer.h - TAPI API JSON Serializer -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the TAPI JSON Serializer
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_APIJSONSERIALIZER_H
#define TAPI_CORE_APIJSONSERIALIZER_H

#include "tapi/Core/APIVisitor.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

struct APIJSONOption {
  bool compact;
  bool noUUID;
  bool noTarget;
  bool noHiddenGlobal;
  bool publicOnly;
  bool ignoreLineCol;
};

class APIJSONSerializer {
public:
  APIJSONSerializer(const API &api, APIJSONOption options = APIJSONOption())
      : api(api), options(options) {}
  ~APIJSONSerializer() {}

  llvm::json::Object getJSONObject() const;

  void serialize(raw_ostream &os) const;

  // static method to parse JSON into API.
  static llvm::Expected<API> parse(StringRef json);
  static llvm::Expected<API> parse(llvm::json::Object *root,
                                   bool publicOnly = false,
                                   llvm::Triple *triple = nullptr);

private:
  const API &api;
  APIJSONOption options;
};

class APIJSONError : public llvm::ErrorInfo<llvm::json::ParseError> {
public:
  APIJSONError(Twine errorMsg) : msg(errorMsg.str()) {}

  void log(llvm::raw_ostream &os) const override {
    os << msg << "\n";
  }
  std::error_code convertToErrorCode() const override {
    return llvm::inconvertibleErrorCode();
  }
private:
  std::string msg;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_APIJSONSERIALIZER_H
