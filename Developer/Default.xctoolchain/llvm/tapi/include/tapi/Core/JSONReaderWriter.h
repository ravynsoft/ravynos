//===- tapi/Core/JSONReaderWriter.h - JSON Reader/Writer -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the JSON Reader/Writer.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_JSON_READER_WRITER_H
#define TAPI_CORE_JSON_READER_WRITER_H

#include "tapi/Core/LLVM.h"
#include "tapi/Core/Registry.h"
#include "tapi/Defines.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class JSONReader final : public Reader {
public:
  JSONReader() : Reader(JSON) {}
  bool canRead(file_magic magic, MemoryBufferRef memBufferRef,
               FileType types) const override;

  Expected<APIs> readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                          ReadFlags readFlags,
                          ArchitectureSet arches) const override;

  Expected<FileType> getFileType(file_magic magic,
                                 MemoryBufferRef bufferRef) const override {
    return canRead(magic, bufferRef, FileType::All) ? FileType::TBD_V5
                                                    : FileType::Invalid;
  }
  static bool classof(const Reader *reader) {
    return reader->getKind() == JSON;
  }
};

class JSONWriter final : public Writer {
public:
  bool canWrite(const InterfaceFile *file, FileType fileType) const override {
    return fileType >= FileType::TBD_V5 ||
           file->getFileType() >= FileType::TBD_V5;
  }

  Error writeFile(raw_ostream &os, const InterfaceFile *file,
                  FileType fileType = FileType::TBD_V5) const override;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_JSON_READER_WRITER_H
