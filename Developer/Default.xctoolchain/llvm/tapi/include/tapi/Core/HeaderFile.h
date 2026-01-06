//===- include/Core/HeaderFile.h - Header File ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Header file enums and defines.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_HEADER_FILE_H
#define TAPI_CORE_HEADER_FILE_H

#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "clang/Basic/LangStandard.h"
#include "llvm/ADT/StringRef.h"
#include <string>

TAPI_NAMESPACE_INTERNAL_BEGIN

enum class HeaderType {
  Public,
  Private,
  Project,
};

inline StringRef getName(const HeaderType type) {
  switch (type) {
  case HeaderType::Public:
    return "Public";
  case HeaderType::Private:
    return "Private";
  case HeaderType::Project:
    return "Project";
  }
  llvm_unreachable("unexpected header type");
}

struct HeaderFile {
  std::string fullPath;
  std::string relativePath;
  std::string includeName;
  HeaderType type;
  std::optional<clang::Language> language;
  bool isUmbrellaHeader{false};
  bool isExcluded{false};
  bool isExtra{false};
  bool isPreInclude{false};
  bool isSwiftCompatibilityHeader{false};

  HeaderFile(StringRef fullPath, HeaderType type,
             StringRef relativePath = StringRef(),
             StringRef includeName = StringRef(),
             std::optional<clang::Language> language = std::nullopt,
             bool isSwiftCompatibilityHeader = false)
      : fullPath(fullPath), relativePath(relativePath),
        includeName(includeName), type(type), language(language),
        isSwiftCompatibilityHeader(isSwiftCompatibilityHeader) {}

  bool useIncludeName() const {
    return type != HeaderType::Project && !includeName.empty();
  }

  void print(raw_ostream &os) const;
  friend bool operator<(const HeaderFile &lhs, const HeaderFile &rhs);
};

std::optional<std::string> createIncludeHeaderName(const StringRef dstRoot);

// Sort by type first.
inline bool operator<(const HeaderFile &lhs, const HeaderFile &rhs) {
  if (lhs.isExtra && rhs.isExtra) {
    return std::tie(lhs.type, rhs.isUmbrellaHeader, lhs.isExtra) <
           std::tie(rhs.type, lhs.isUmbrellaHeader, rhs.isExtra);
  }

  return std::tie(lhs.type, rhs.isUmbrellaHeader, lhs.isExtra, lhs.fullPath) <
         std::tie(rhs.type, lhs.isUmbrellaHeader, rhs.isExtra, rhs.fullPath);
}

inline raw_ostream &operator<<(raw_ostream &os, const HeaderFile &file) {
  file.print(os);
  return os;
}

using HeaderSeq = std::vector<HeaderFile>;

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_HEADER_FILE_H
