//===- Debug.cpp ------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/raw_ostream.h"
#include <cstdlib>

namespace llvm {

// Prevent the inclusion of lib/Support/Debug.cpp, because we don't want the
// static initializers.
raw_ostream &dbgs() { return errs(); }

#ifndef NDEBUG
#undef isCurrentDebugType
bool isCurrentDebugType(const char *Type) { abort(); }

bool DebugFlag = false;
#endif

} // end namespace llvm.
