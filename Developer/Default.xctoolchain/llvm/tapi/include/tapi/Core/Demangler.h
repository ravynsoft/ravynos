//===- Core/Demangler.h - TAPI Demangler ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the Demangler interface for tapi
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_CORE_DEMANGLER_H
#define TAPI_CORE_DEMANGLER_H

#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

struct DemangledName {
  std::string str;
  bool isItanium;
  bool isSwift;
};

class Demangler {
public:
  Demangler();
  ~Demangler();

  /// Attempt to demangle `mangledName` using Swift and Itanium demangling
  /// schemes.
  ///
  /// Returns a DemangledName containing
  /// - the demangled string with tags for the scheme used, or
  /// - a copy of the input string if no demangling occurred.
  DemangledName demangle(StringRef mangledName);

private:
  using swift_demangle_ft = char *(*)(const char *mangledName,
                                      size_t mangledNameLength,
                                      char *outputBuffer,
                                      size_t *outputBufferSize, uint32_t flags);

  static bool isItaniumEncoding(StringRef mangledName);

  swift_demangle_ft swift_demangle_f;
  void *libswiftCoreHandle;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_DEMANGLER_H
