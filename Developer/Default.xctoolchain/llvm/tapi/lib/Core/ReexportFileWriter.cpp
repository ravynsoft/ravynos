//===- lib/Core/ReexportFileWriter.cpp - Reexport File Writer ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the LD64 re-export file writer
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/ReexportFileWriter.h"
#include "tapi/Core/LLVM.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TextAPI/Symbol.h"
#include <system_error>

using namespace llvm;
using namespace llvm::MachO;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

class ReexportFileWriter::Implementation {
public:
  std::vector<std::string> symbols;
  bool isFragileABI;
};

ReexportFileWriter::ReexportFileWriter(const Triple &target)
    : impl(*new ReexportFileWriter::Implementation()) {
  impl.isFragileABI =
      target.getOS() == Triple::MacOSX && target.getArch() == Triple::x86;
}

ReexportFileWriter::~ReexportFileWriter() { delete &impl; }

void ReexportFileWriter::writeToStream(raw_ostream &os) {
  for (const auto &symbol : impl.symbols)
    os << symbol << "\n";
}

void ReexportFileWriter::visitGlobal(const GlobalRecord &record) {
  // Skip non exported symbol.
  if (!record.isExported())
    return;
  // Skip unavailable symbol.
  if (record.availability.isUnavailable())
    return;

  impl.symbols.emplace_back(record.name);
}

void ReexportFileWriter::visitObjCInterface(const ObjCInterfaceRecord &record) {
  if (!record.isExported())
    return;

  if (record.availability.isUnavailable())
    return;

  if (impl.isFragileABI)
    impl.symbols.emplace_back((ObjC1ClassNamePrefix + record.name).str());
  else {
    impl.symbols.emplace_back((ObjC2ClassNamePrefix + record.name).str());
    impl.symbols.emplace_back((ObjC2MetaClassNamePrefix + record.name).str());
  }

  if (record.hasExceptionAttribute())
    impl.symbols.emplace_back((ObjC2EHTypePrefix + record.name).str());

  auto addIvars = [&](ArrayRef<const ObjCInstanceVariableRecord *> ivars) {
    for (const auto *ivar : ivars) {
      if (!ivar->isExported())
        continue;

      // ObjC has an additional mechanism to specify if an ivar is exported or
      // not.
      if (ivar->accessControl == ObjCIvarDecl::Private ||
          ivar->accessControl == ObjCIvarDecl::Package)
        continue;

      impl.symbols.emplace_back(
          (ObjC2IVarPrefix + record.name + "." + ivar->name).str());
    }
  };
  addIvars(record.ivars);
}

TAPI_NAMESPACE_INTERNAL_END
