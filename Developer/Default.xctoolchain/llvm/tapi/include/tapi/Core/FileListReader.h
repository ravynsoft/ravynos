//===- tapi/Core/FileListReader.h - File List Reader ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief The JSON File List is used to communicate additional information to
///        InstallAPI. For now this only includes a header list.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_FILE_LIST_READER_H
#define TAPI_CORE_FILE_LIST_READER_H

#include "tapi/Core/HeaderFile.h"
#include "tapi/Defines.h"
#include "llvm/Support/Error.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class FileListReader {
  class Implementation;

  Implementation &impl;

  FileListReader(std::unique_ptr<MemoryBuffer> inputBuffer, llvm::Error &error);

public:
  static llvm::Expected<std::unique_ptr<FileListReader>>
  get(std::unique_ptr<llvm::MemoryBuffer> inputBuffer);

  ~FileListReader();

  FileListReader(const FileListReader &) = delete;
  FileListReader &operator=(const FileListReader &) = delete;

  int getVersion() const;

  struct HeaderInfo {
    HeaderType type;
    std::string path;
    std::optional<clang::Language> language;
    bool isSwiftCompatibilityHeader;
  };

  /// Visitor used when walking the contents of the file list.
  class Visitor {
  public:
    virtual ~Visitor();

    virtual void visitHeaderFile(HeaderInfo &header);
  };

  /// Visit the contents of the header list file, passing each entity to the
  /// given visitor.
  void visit(Visitor &visitor);
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_FILE_LIST_READER_H
