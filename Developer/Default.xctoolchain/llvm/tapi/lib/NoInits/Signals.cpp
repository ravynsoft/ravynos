//===- Signals.cpp ----------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringRef.h"
#include <cstdlib>
#include <string>

namespace llvm {

namespace sys {

bool RemoveFileOnSignal(StringRef Filename, std::string *ErrMsg) { abort(); }

void DontRemoveFileOnSignal(StringRef Filename) { abort(); }

void AddSignalHandler(void (*)(void *), void *) { abort(); }

} // end namespace sys.
} // end namespace llvm.
