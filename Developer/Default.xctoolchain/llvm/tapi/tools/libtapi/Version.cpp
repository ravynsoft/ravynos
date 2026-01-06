//===- libtapi/Version.cpp - TAPI Version Interface -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the C++ version interface.
///
//===----------------------------------------------------------------------===//
#include <tapi/Version.h>
#include <tuple>

/// \brief Helper macro for TAPI_VERSION_STRING.
#define TAPI_MAKE_STRING2(X) #X

/// \brief A string that describes the TAPI version number, e.g., "1.0.0".
#define TAPI_MAKE_STRING(X) TAPI_MAKE_STRING2(X)

namespace tapi {

unsigned Version::getMajor() noexcept { return TAPI_VERSION_MAJOR; }

unsigned Version::getMinor() noexcept { return TAPI_VERSION_MINOR; }

unsigned Version::getPatch() noexcept { return TAPI_VERSION_PATCH; }

std::string Version::getAsString() noexcept {
  return TAPI_MAKE_STRING(TAPI_VERSION);
}

std::string Version::getFullVersionAsString() noexcept {
  std::string result;
#ifdef TAPI_VENDOR
  result += TAPI_VENDOR;
#endif
  result += "TAPI version " TAPI_MAKE_STRING(TAPI_VERSION);

#ifdef TAPI_REPOSITORY_STRING
  result += " (" TAPI_REPOSITORY_STRING ")";
#endif

  return result;
}

bool Version::isAtLeast(unsigned major, unsigned minor,
                        unsigned patch) noexcept {
  return std::make_tuple(TAPI_VERSION_MAJOR, TAPI_VERSION_MINOR,
                         TAPI_VERSION_PATCH) >= std::tie(major, minor, patch);
}

} // end namespace tapi.
