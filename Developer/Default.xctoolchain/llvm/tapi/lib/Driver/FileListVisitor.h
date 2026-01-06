//===- tapi/Driver/FileListVisitor.h - File List Visitor --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines a visitor implementation for retaining headers found in JSON
///        File List.
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_DRIVER_FILE_LIST_VISITOR_H
#define TAPI_DRIVER_FILE_LIST_VISITOR_H

#include "tapi/Core/FileListReader.h"
#include "tapi/Core/FileManager.h"
#include "tapi/Diagnostics/Diagnostics.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class FileListVisitor final : public FileListReader::Visitor {
  FileManager &fm;
  DiagnosticsEngine &diag;
  HeaderSeq &headerFiles;

public:
  FileListVisitor(FileManager &fm, DiagnosticsEngine &diag,
                  HeaderSeq &headerFiles)
      : fm(fm), diag(diag), headerFiles(headerFiles) {}

  void visitHeaderFile(FileListReader::HeaderInfo &header) override;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_DRIVER_FILE_LIST_VISITOR_H
