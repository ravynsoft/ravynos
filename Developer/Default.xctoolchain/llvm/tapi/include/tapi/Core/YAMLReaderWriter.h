//===- tapi/Core/YAMLReaderWriter.h - YAML Reader/Writer --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the YAML Reader/Writer.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_YAML_READER_WRITER_H
#define TAPI_CORE_YAML_READER_WRITER_H

#include "tapi/Core/LLVM.h"
#include "tapi/Core/Registry.h"
#include "tapi/Defines.h"
#include "llvm/BinaryFormat/Magic.h"
#include "llvm/Support/Error.h"
#include "llvm/TextAPI/ArchitectureSet.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include "llvm/TextAPI/TextAPIWriter.h"
#include <string>

TAPI_NAMESPACE_INTERNAL_BEGIN

class YAMLBase;
using InterfaceFile = llvm::MachO::InterfaceFile;

struct YAMLContext {
  const YAMLBase &base;
  std::string path;
  std::string errorMessage;
  ReadFlags readFlags;
  FileType fileType;

  YAMLContext(const YAMLBase &base) : base(base) {}
};

class DocumentHandler {
public:
  virtual ~DocumentHandler() = default;
  virtual bool canRead(MemoryBufferRef memBufferRef, FileType types) const = 0;
  virtual FileType getFileType(MemoryBufferRef bufferRef) const = 0;
  virtual bool canWrite(const InterfaceFile *file, FileType fileType) const = 0;

  bool writeFile(raw_ostream &os, const InterfaceFile *file,
                 FileType fileType) const {
    if (Error result = TextAPIWriter::writeToStream(os, *file, fileType)) {
      consumeError(std::move(result));
      return false;
    }

    return true;
  }
};

namespace stub {
namespace v1 {

class YAMLDocumentHandler : public DocumentHandler {
  bool canRead(MemoryBufferRef memBufferRef,
               FileType types = FileType::All) const override;
  FileType getFileType(MemoryBufferRef memBufferRef) const override;
  bool canWrite(const InterfaceFile *file, FileType fileType) const override;
};

} // end namespace v1.
} // end namespace stub.

namespace stub {
namespace v2 {

class YAMLDocumentHandler : public DocumentHandler {
  bool canRead(MemoryBufferRef memBufferRef,
               FileType types = FileType::All) const override;
  FileType getFileType(MemoryBufferRef memBufferRef) const override;
  bool canWrite(const InterfaceFile *file, FileType fileType) const override;
};

} // end namespace v2.
} // end namespace stub.

namespace stub {
namespace v3 {

class YAMLDocumentHandler : public DocumentHandler {
  bool canRead(MemoryBufferRef memBufferRef,
               FileType types = FileType::All) const override;
  FileType getFileType(MemoryBufferRef memBufferRef) const override;
  bool canWrite(const InterfaceFile *file, FileType fileType) const override;
};

} // end namespace v3.
} // end namespace stub.

namespace stub {
namespace v4 {

class YAMLDocumentHandler : public DocumentHandler {
  bool canRead(MemoryBufferRef memBufferRef,
               FileType types = FileType::All) const override;
  FileType getFileType(MemoryBufferRef memBufferRef) const override;
  bool canWrite(const InterfaceFile *file, FileType fileType) const override;
};

} // end namespace v4.
} // end namespace stub.

class YAMLBase {
public:
  bool canRead(MemoryBufferRef memBufferRef, FileType types) const;
  FileType getFileType(MemoryBufferRef bufferRef) const;
  bool canWrite(const InterfaceFile *file, FileType fileType) const;
  bool writeFile(raw_ostream &os, const InterfaceFile *file,
                 FileType fileType) const;

  void add(std::unique_ptr<DocumentHandler> handler) {
    _documentHandlers.emplace_back(std::move(handler));
  }

private:
  std::vector<std::unique_ptr<DocumentHandler>> _documentHandlers;
};

class YAMLReader final : public YAMLBase, public Reader {
public:
  YAMLReader() : Reader(YAML) {}
  bool canRead(file_magic magic, MemoryBufferRef memBufferRef,
               FileType types) const override;
  Expected<FileType> getFileType(file_magic magic,
                                 MemoryBufferRef bufferRef) const override;
  Expected<APIs> readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                          ReadFlags readFlags,
                          ArchitectureSet arches) const override;

  static bool classof(const Reader *reader) {
    return reader->getKind() == YAML;
  }
};

class YAMLWriter final : public YAMLBase, public Writer {
public:
  bool canWrite(const InterfaceFile *file, FileType fileType) const override;
  Error writeFile(raw_ostream &os, const InterfaceFile *file,
                  FileType fileType) const override;
};

void addInterfaceFileToAPIs(APIs &apis, const InterfaceFile *interface);

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_YAML_READER_WRITER_H
