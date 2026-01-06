//===- Statistic.cpp ----------------------------------------------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
#include <cstdlib>

namespace llvm {
namespace TrackingStatistic {

//TODO: determine different avenues to remove the static initializer from upstream Statistic.cpp.o
void RegisterStatistic() { abort(); }

} // namespace TrackingStatistic
} // namespace llvm
