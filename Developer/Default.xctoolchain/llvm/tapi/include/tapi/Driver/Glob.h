//===--- tapi/Driver/Glob.h - Glob ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief A simple glob to regex converter.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_GLOB_H
#define TAPI_CORE_GLOB_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/Regex.h"
#include "tapi/Defines.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

llvm::Expected<llvm::Regex> createRegexFromGlob(llvm::StringRef glob);

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_GLOB_H
