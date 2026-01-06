//===- tapi/Core/Context.h - TAPI Context -----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief The context used by tapi.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_CONTEXT_H
#define TAPI_CORE_CONTEXT_H

#include "tapi/Core/FileManager.h"
#include "tapi/Core/Registry.h"
#include "tapi/Diagnostics/Diagnostics.h"
#include "tapi/Driver/Options.h"


TAPI_NAMESPACE_INTERNAL_BEGIN

class Context {
public:
  // Make the context not copyable.
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

  Context(Options &opt, DiagnosticsEngine &diag) : _fm(opt.fm), _diag(diag) {}

  // FileManager less init.
  Context(DiagnosticsEngine &diag) : _fm(nullptr), _diag(diag) {}

  FileManager &getFileManager() const {
    assert(_fm.get() != nullptr);
    return *_fm;
  }

  DiagnosticsEngine &getDiag() const { return _diag; }

protected:
  IntrusiveRefCntPtr<FileManager> _fm;
  DiagnosticsEngine &_diag;
};


TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_CONTEXT_H
