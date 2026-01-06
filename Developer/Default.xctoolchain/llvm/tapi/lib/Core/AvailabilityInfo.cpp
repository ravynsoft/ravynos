//===- lib/Core/AvailabilityInfo.cpp - Availability Info --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "tapi/Core/AvailabilityInfo.h"
#include "tapi/Core/LLVM.h"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

std::string AvailabilityInfo::str() const {
  std::string str;
  raw_string_ostream os(str);
  print(os);
  os.flush();
  return str;
}

void AvailabilityInfo::print(raw_ostream &os) const {
  os << "i:" << _introduced << " d:" << _deprecated << " o:" << _obsoleted
     << " u:" << static_cast<int>((bool)_unavailable);
  if (_isSPIAvailable)
    os << " (spi)";
}

TAPI_NAMESPACE_INTERNAL_END
