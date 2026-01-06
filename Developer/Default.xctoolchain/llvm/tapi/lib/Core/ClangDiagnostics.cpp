//===- lib/Core/ClangDiagnostics.cpp - Diagnostics wrapper ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "tapi/Core/ClangDiagnostics.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/TextAPI/Platform.h"

using clang::DiagnosticBuilder;

namespace llvm {
namespace MachO {
const DiagnosticBuilder &operator<<(const DiagnosticBuilder &db,
                                    Architecture Arch) {
  db.AddString(getArchitectureName(Arch));
  return db;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &db,
                                    ArchitectureSet ArchSet) {
  db.AddString(std::string(ArchSet));
  return db;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &db,
                                    PlatformType platform) {
  db.AddString(getPlatformName(platform));
  return db;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &db,
                                    PlatformVersionSet platforms) {
  std::string diagString;
  diagString.append("[ ");
  unsigned index = 0;
  for (auto &[platform, version] : platforms) {
    const std::string versionStr =
        version == VersionTuple(0) ? "" : version.getAsString();
    if (index > 0)
      diagString.append(", ");
    diagString.append(std::string(getPlatformName(platform)) + versionStr);
    ++index;
  }
  diagString.append(" ]");
  db.AddString(diagString);
  return db;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &db, FileType v) {
  std::string name;
  switch (v) {
  case FileType::MachO_Bundle:
    name = "mach-o bundle";
    break;
  case FileType::MachO_DynamicLibrary:
    name = "mach-o dynamic library";
    break;
  case FileType::MachO_DynamicLibrary_Stub:
    name = "mach-o dynamic library stub";
    break;
  case FileType::TBD_V1:
    name = "tbd-v1";
    break;
  case FileType::TBD_V2:
    name = "tbd-v2";
    break;
  case FileType::TBD_V3:
    name = "tbd-v3";
    break;
  case FileType::TBD_V4:
    name = "tbd-v4";
    break;
  case FileType::TBD_V5:
    name = "tbd-v5";
    break;
  case FileType::Invalid:
  case FileType::All:
    llvm_unreachable("unexpected file type for diagnostics");
    break;
  }
  db.AddString(name);
  return db;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &db,
                                    const PackedVersion &version) {
  SmallString<32> string;
  raw_svector_ostream os(string);
  os << version;
  db.AddString(string);
  return db;
}

} // namespace MachO
} // namespace llvm
