//===- CrashRecoveryContext.cpp ---------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/CrashRecoveryContext.h"
#include <cstdlib>

namespace llvm {

bool CrashRecoveryContext::isRecoveringFromCrash() { abort(); }
CrashRecoveryContext *CrashRecoveryContext::GetCurrent() { abort(); }
void CrashRecoveryContext::HandleExit(int) { abort(); }

} // end namespace llvm.
