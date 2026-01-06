//===- tapi/Core/ClangDiagnostics.h - TAPI Diagnostics Streams --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Stream Operator Declaration for llvm/TextAPI Types &
/// clang/Diagnostics
///
//===----------------------------------------------------------------------===//
//
#ifndef TAPI_CLANG_DIAGNOSTICS_H
#define TAPI_CLANG_DIAGNOSTICS_H

#include "clang/Basic/Diagnostic.h"
#include "llvm/TextAPI/Architecture.h"
#include "llvm/TextAPI/ArchitectureSet.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include "llvm/TextAPI/Platform.h"

namespace llvm {
namespace MachO {

const clang::DiagnosticBuilder &operator<<(const clang::DiagnosticBuilder &db,
                                           PlatformType platform);

const clang::DiagnosticBuilder &operator<<(const clang::DiagnosticBuilder &db,
                                           PlatformVersionSet platforms);

const clang::DiagnosticBuilder &operator<<(const clang::DiagnosticBuilder &db,
                                           Architecture arch);

const clang::DiagnosticBuilder &operator<<(const clang::DiagnosticBuilder &db,
                                           ArchitectureSet architectureSet);

const clang::DiagnosticBuilder &operator<<(const clang::DiagnosticBuilder &db,
                                           FileType type);

const clang::DiagnosticBuilder &operator<<(const clang::DiagnosticBuilder &db,
                                           const PackedVersion &version);

inline const clang::DiagnosticBuilder &
operator<<(const clang::DiagnosticBuilder &db, const InterfaceFileRef &ref) {
  auto str = std::string(ref.getInstallName()) + " [ " +
             std::string(ref.getArchitectures()) + " ]";
  db.AddString(str);
  return db;
}

} // namespace MachO
} // namespace llvm
#endif // TAPI_CLANG_DIAGNOSTICS_H
