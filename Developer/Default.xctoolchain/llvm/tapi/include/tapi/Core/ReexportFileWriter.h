//===- tapi/Core/ReexportFileWriter.h - Reexport File Writer ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the Reexport File Writer
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_REEXPORT_FILE_WRITER_H
#define TAPI_CORE_REEXPORT_FILE_WRITER_H

#include "tapi/Core/APIVisitor.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "llvm/Support/Error.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class ReexportFileWriter : public APIVisitor {
  class Implementation;
  Implementation &impl;

public:
  ReexportFileWriter(const llvm::Triple &target);
  ~ReexportFileWriter();

  ReexportFileWriter(const ReexportFileWriter &) = delete;
  ReexportFileWriter &operator=(const ReexportFileWriter &) = delete;

  void writeToStream(llvm::raw_ostream &os);

  void visitGlobal(const GlobalRecord &record) override;
  void visitObjCInterface(const ObjCInterfaceRecord &record) override;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_REEXPORT_FILE_WRITER_H
