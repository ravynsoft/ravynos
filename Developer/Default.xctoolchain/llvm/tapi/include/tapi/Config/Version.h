//===- tapi/Config/Version.h - TAPI Version Number --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines version macros and version-related utility functions for
/// tapi.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CONFIG_VERSION_H
#define TAPI_CONFIG_VERSION_H

#include "tapi/Defines.h"
#include <string>

/// \brief Helper macro for TAPI_VERSION_STRING.
#define TAPI_MAKE_STRING2(X) #X

/// \brief A string that describes the TAPI version number, e.g., "1.0.0".
#define TAPI_MAKE_STRING(X) TAPI_MAKE_STRING2(X)

TAPI_NAMESPACE_INTERNAL_BEGIN

/// \brief Retrieves a string representing the full tapi name and version.
std::string getTAPIFullVersion();

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CONFIG_VERSION_H
