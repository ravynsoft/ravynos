//===- tapi/Driver/FileListVisitor.cpp - File List Visitor -----*- C++ -*-===//
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

#include "FileListVisitor.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

void FileListVisitor::visitHeaderFile(FileListReader::HeaderInfo &header) {
  if (!fm.exists(header.path)) {
    diag.report(diag::err_no_such_header_file)
        << header.path << (unsigned)header.type;
    return;
  }

  if (header.type == HeaderType::Project) {
    headerFiles.emplace_back(header.path, header.type,
                             /*relativePath*/ "", /*includeName*/ "",
                             header.language,
                             header.isSwiftCompatibilityHeader);
    return;
  }

  auto includeName = createIncludeHeaderName(header.path);
  headerFiles.emplace_back(header.path, header.type, /*relativePath*/ "",
                           includeName.has_value() ? includeName.value() : "",
                           header.language, header.isSwiftCompatibilityHeader);
}

TAPI_NAMESPACE_INTERNAL_END
