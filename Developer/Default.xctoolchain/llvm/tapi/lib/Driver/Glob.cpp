//===- lib/Driver/Glob.cpp - Glob -------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements glob.
///
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallString.h"
#include <tapi/Driver/Glob.h>

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

static StringLiteral regexMetachars = "()^$|+.[]\\{}";

Expected<Regex> createRegexFromGlob(StringRef globString) {
  SmallString<128> regexString("^");
  unsigned numWildcards = 0;
  for (unsigned i = 0; i < globString.size(); ++i) {
    char c = globString[i];
    switch (c) {
    case '?':
      regexString += '.';
      break;
    case '*': {
      const char *previousChar = i > 0 ? globString.data() + i - 1 : nullptr;
      numWildcards = 1;
      ++i;
      while (i < globString.size() && globString[i] == '*') {
        ++numWildcards;
        ++i;
      }
      const char *nextChar =
          i < globString.size() ? globString.data() + i : nullptr;

      if ((numWildcards > 1) &&
          (previousChar == nullptr || *previousChar == '/') &&
          (nextChar == nullptr || *nextChar == '/')) {
        regexString += "(([^/]*(/|$))*)";
      } else
        regexString += "([^/]*)";
      break;
    }
    default:
      if (regexMetachars.find(c) != StringRef::npos)
        regexString.push_back('\\');
      regexString.push_back(c);
    }
  }
  regexString.push_back('$');
  if (numWildcards == 0)
    return make_error<StringError>("not a glob", inconvertibleErrorCode());

  auto regex = Regex(regexString);
  std::string error;
  if (!regex.isValid(error))
    return make_error<StringError>(error, inconvertibleErrorCode());

  return regex;
}

TAPI_NAMESPACE_INTERNAL_END
