//===- ErrorHandling.cpp ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <string>

namespace llvm {

void report_bad_alloc_error(const char *, bool) { abort(); }

void report_fatal_error(const char *, bool) { abort(); }

void report_fatal_error(const std::string &, bool) { abort(); }

void report_fatal_error(StringRef, bool) { abort(); }

void report_fatal_error(const Twine &, bool) { abort(); }

void llvm_unreachable_internal(const char *, const char *, unsigned) {
  abort();
}

} // end namespace llvm.
