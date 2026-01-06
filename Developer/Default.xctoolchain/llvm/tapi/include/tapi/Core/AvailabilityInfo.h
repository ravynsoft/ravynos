//===- tapi/Core/AvailabilityInfo.h - TAPI Availability Info ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the Availability Info.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_AVAILABILITY_INFO_H
#define TAPI_CORE_AVAILABILITY_INFO_H

#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Error.h"
#include "llvm/TextAPI/PackedVersion.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

struct AvailabilityInfo {
  PackedVersion _introduced{0};
  PackedVersion _deprecated{0};
  PackedVersion _obsoleted{0};
  bool _unavailable{false};
  bool _isSPIAvailable{false};

  constexpr AvailabilityInfo(bool unavailable = false)
      : _unavailable(unavailable) {}

  constexpr AvailabilityInfo(PackedVersion i, PackedVersion d, PackedVersion o,
                             bool u, bool ud, bool isSPI = false)
      : _introduced(i), _deprecated(d), _obsoleted(o), _unavailable(u),
        _isSPIAvailable(isSPI) {}

  bool isDefault() const { return *this == AvailabilityInfo(); }

  llvm::Error merge(const AvailabilityInfo &other) {
    if (*this == other || other.isDefault())
      return llvm::Error::success();

    if (isDefault()) {
      *this = other;
      return llvm::Error::success();
    }

    return llvm::make_error<llvm::StringError>(
        "availabilities do not match",
        std::make_error_code(std::errc::not_supported));
  }

  std::string str() const;

  bool isUnavailable() const { return _unavailable; }

  bool isSPIAvailable() const { return _isSPIAvailable; }

  bool isObsolete() const { return !_obsoleted.empty(); }

  void print(raw_ostream &os) const;

  friend bool operator==(const AvailabilityInfo &lhs,
                         const AvailabilityInfo &rhs);
  friend bool operator!=(const AvailabilityInfo &lhs,
                         const AvailabilityInfo &rhs);
  friend bool operator<(const AvailabilityInfo &lhs,
                        const AvailabilityInfo &rhs);
};

inline bool operator==(const AvailabilityInfo &lhs,
                       const AvailabilityInfo &rhs) {
  return std::tie(lhs._introduced, lhs._deprecated, lhs._obsoleted,
                  lhs._unavailable, lhs._isSPIAvailable) ==
         std::tie(rhs._introduced, rhs._deprecated, rhs._obsoleted,
                  rhs._unavailable, rhs._isSPIAvailable);
}

inline bool operator!=(const AvailabilityInfo &lhs,
                       const AvailabilityInfo &rhs) {
  return !(lhs == rhs);
}

inline bool operator<(const AvailabilityInfo &lhs,
                      const AvailabilityInfo &rhs) {
  return std::tie(lhs._introduced, lhs._deprecated, lhs._obsoleted,
                  lhs._unavailable, lhs._isSPIAvailable) <
         std::tie(rhs._introduced, rhs._deprecated, rhs._obsoleted,
                  rhs._unavailable, rhs._isSPIAvailable);
}

inline raw_ostream &operator<<(raw_ostream &os, const AvailabilityInfo &avail) {
  avail.print(os);
  return os;
}

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_AVAILABILITY_INFO_H
