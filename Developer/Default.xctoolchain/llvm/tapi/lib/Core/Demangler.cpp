//===- Core/Demangler.cpp - TAPI Demangler ----------------------*- C++ -*-===//
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
#include "tapi/Core/Demangler.h"
#include "llvm/Demangle/Demangle.h"
#include <dlfcn.h>

TAPI_NAMESPACE_INTERNAL_BEGIN

Demangler::Demangler() {
  // dlopen the swiftruntime to workaround bug: rdar://103567569
  libswiftCoreHandle = dlopen("/usr/lib/swift/libswiftCore.dylib", RTLD_LAZY);
  swift_demangle_f =
      (swift_demangle_ft)dlsym(libswiftCoreHandle, "swift_demangle");
}

Demangler::~Demangler() { dlclose(libswiftCoreHandle); }

bool Demangler::isItaniumEncoding(StringRef mangledName) {
  // Itanium encoding requires 1 or 3 leading underscores, followed by 'Z'.
  return mangledName.starts_with("_Z") || mangledName.starts_with("___Z");
}

DemangledName Demangler::demangle(StringRef mangledName) {
  DemangledName result{
      .str = mangledName.str(), .isSwift = false, .isItanium = false};
  char *demangled = nullptr;

  if (isItaniumEncoding(mangledName)) {
    demangled = llvm::itaniumDemangle(mangledName.str().c_str());
    result.isItanium = true;
  } else if (mangledName.starts_with("_") &&
             isItaniumEncoding(mangledName.drop_front())) {
    demangled = llvm::itaniumDemangle(mangledName.str().c_str() + 1);
    result.isItanium = true;
  } else if ((demangled = swift_demangle_f(
                  mangledName.str().c_str(), mangledName.size(),
                  /*outputBuffer=*/nullptr,
                  /*outputBufferSize=*/nullptr, /*flags=*/0)))
    result.isSwift = true;

  if (demangled) {
    result.str = demangled;
    if (result.isItanium)
      std::free(demangled);
  }

  return result;
}

TAPI_NAMESPACE_INTERNAL_END
