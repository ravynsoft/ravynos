//===- tapi/Core/APICommon.h - TAPI API Common Types ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines common API related types.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_API_COMMON_H
#define TAPI_CORE_API_COMMON_H

#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/BitmaskEnum.h"
#include <cstdint>

TAPI_NAMESPACE_INTERNAL_BEGIN

LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();

// clang-format off
enum class APIAccess : uint8_t {
  Unknown = 0, // Unknown, example, from binary.
  Project = 1, // APIs available in project headers.
  Private = 2, // APIs available in private headers.
  Public  = 3, // APIs available in public headers.
};

enum class APILinkage : uint8_t {
  Unknown    = 0, // Unknown.
  Internal   = 1, // API is internal.
  External   = 2, // External interface used.
  Reexported = 3, // API is re-exported.
  Exported   = 4, // API is exported.
};

// clang-format on

class APILoc {
public:
  APILoc() = default;
  APILoc(clang::SourceLocation source, clang::PresumedLoc presumed)
      : sourceLoc(source), presumedLoc(presumed) {}
  APILoc(std::string file, unsigned line, unsigned col);
  APILoc(StringRef file, unsigned line, unsigned col);

  bool isInvalid() const;
  StringRef getFilename() const;
  unsigned getLine() const;
  unsigned getColumn() const;
  clang::PresumedLoc getPresumedLoc() const;
  clang::SourceLocation getSourceLocation() const;

  bool operator==(const APILoc &other) const {
    return std::tie(sourceLoc, file, line, col) ==
           std::tie(other.sourceLoc, other.file, other.line, other.col);
  }

private:
  std::optional<clang::SourceLocation> sourceLoc;
  std::optional<clang::PresumedLoc> presumedLoc;
  std::string file;
  unsigned line;
  unsigned col;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_API_COMMON_H
