//===- lib/Core/Framework.cpp - Framework Context ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the Framework context.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/HeaderFile.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Regex.h"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

void HeaderFile::print(raw_ostream &os) const {
  os << "(" << getName(type).lower() << ")";
  os << sys::path::filename(fullPath);
  if (isUmbrellaHeader)
    os << " (umbrella header)";
  if (isExtra)
    os << " (extra header)";
  if (isExcluded)
    os << " (excluded header)";
  if (isPreInclude)
    os << " (pre-include header)";
}

const Regex Rule("/(.+)\\.framework/(.+)?Headers/(.+)");
std::optional<std::string> createIncludeHeaderName(const StringRef dstRoot) {
  // Headers in usr(/local)*/include.
  std::string pattern = "/include/";
  auto pathPrefix = dstRoot.find(pattern);
  if (pathPrefix != StringRef::npos) {
    pathPrefix += pattern.size();
    return dstRoot.drop_front(pathPrefix).str();
  }

  // Framework Headers.
  SmallVector<StringRef, 4> matches;
  Rule.match(dstRoot, &matches);
  // Returned matches are always in stable order.
  if (matches.size() != 4)
    return std::nullopt;

  return matches[1].drop_front(matches[1].rfind('/') + 1).str() + "/" +
         matches[3].str();
}

TAPI_NAMESPACE_INTERNAL_END
