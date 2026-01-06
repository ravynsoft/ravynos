//===- tools/tapi/tapi.cpp - TAPI Tool --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// The tapi tool is a thin wrapper around the tapi driver.
///
//===----------------------------------------------------------------------===//

#include "tapi/Driver/Driver.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"

using namespace tapi::internal;

int main(int argc, const char *argv[]) {
  // Standard set up, so program fails gracefully.
  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram stackPrinter(argc, argv);
  llvm::llvm_shutdown_obj shutdown;

  if (llvm::sys::Process::FixupStandardFileDescriptors())
    return 1;

  return Driver::run(llvm::ArrayRef(argv, argc)) ? 0 : 1;
}
