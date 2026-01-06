//===- libtapi/APIVersion.cpp - TAPI API Version Interface ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the C++ API version interface.
///
//===----------------------------------------------------------------------===//
#include <tapi/APIVersion.h>
#include <tuple>

namespace tapi {

enum class ABI {
  v1 = 1,
  min = v1,
  max = v1,
};

unsigned APIVersion::getMajor() noexcept { return TAPI_API_VERSION_MAJOR; }

unsigned APIVersion::getMinor() noexcept { return TAPI_API_VERSION_MINOR; }

unsigned APIVersion::getPatch() noexcept { return TAPI_API_VERSION_PATCH; }

bool APIVersion::isAtLeast(unsigned major, unsigned minor,
                           unsigned patch) noexcept {
  return std::make_tuple(TAPI_API_VERSION_MAJOR, TAPI_API_VERSION_MINOR,
                         TAPI_API_VERSION_PATCH) >=
         std::tie(major, minor, patch);
}

bool APIVersion::hasFeature(Feature /*feature*/) noexcept { return false; }

bool APIVersion::hasABI(unsigned abiVersion) noexcept {
  return abiVersion >= static_cast<unsigned>(ABI::min) &&
         abiVersion <= static_cast<unsigned>(ABI::max);
}

} // end namespace tapi.
