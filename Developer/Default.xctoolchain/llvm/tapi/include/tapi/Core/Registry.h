//===- tapi/Core/Registry.h - TAPI Registry ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief The TAPI registry keeps track of the supported file formats.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_REGISTRY_H
#define TAPI_CORE_REGISTRY_H

#include "tapi/Core/API.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "llvm/BinaryFormat/Magic.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/TextAPI/ArchitectureSet.h"
#include "llvm/TextAPI/InterfaceFile.h"

using llvm::file_magic;
using llvm::Error;
using llvm::Expected;

TAPI_NAMESPACE_INTERNAL_BEGIN

class Registry;

enum class ReadFlags {
  Header,
  DefinedSymbols,
  Symbols,
  DebugInfo,
  ObjCMetadata,
  All,
};

/// Abstract Reader class - all readers need to inherit from this class and
/// implement the interface.
class Reader {
public:
  enum ReaderKind { YAML, Binary, Diagnostic, JSON };

private:
  const ReaderKind kind;

public:
  ReaderKind getKind() const { return kind; }
  Reader(ReaderKind kind) : kind(kind) {}
  virtual ~Reader() = default;
  virtual bool canRead(file_magic fileType, MemoryBufferRef bufferRef,
                       FileType types = FileType::All) const = 0;
  virtual Expected<FileType> getFileType(file_magic magic,
                                         MemoryBufferRef bufferRef) const = 0;
  virtual Expected<APIs> readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                                  ReadFlags readFlags,
                                  ArchitectureSet arches) const = 0;
};

/// Abstract Writer class - all writers need to inherit from this class and
/// implement the interface.
class Writer {
public:
  virtual ~Writer() = default;
  virtual bool canWrite(const InterfaceFile *file, FileType fileType) const = 0;
  virtual Error writeFile(raw_ostream &os, const InterfaceFile *file,
                          FileType fileType) const = 0;
};

class Registry {
public:
  bool canRead(MemoryBufferRef memBuffer, FileType types = FileType::All) const;
  Expected<FileType> getFileType(MemoryBufferRef memBuffer) const;
  bool canWrite(const InterfaceFile *file, FileType fileType) const;

  static FileType getTextFileType() {
    return FileType::TBD_V1 | FileType::TBD_V2 | FileType::TBD_V3 |
           FileType::TBD_V4 | FileType::TBD_V5;
  }

  Expected<APIs>
  readFile(std::unique_ptr<MemoryBuffer> memBuffer,
           ReadFlags readFlags = ReadFlags::All,
           ArchitectureSet arches = ArchitectureSet::All()) const;
  Expected<std::unique_ptr<InterfaceFile>>
  readTextFile(std::unique_ptr<MemoryBuffer> memBuffer,
           ReadFlags readFlags = ReadFlags::All,
           ArchitectureSet arches = ArchitectureSet::All()) const;
  Error writeFile(const std::string &path, const InterfaceFile *file,
                  FileType fileType, bool replaceFile = true) const;
  Error writeFile(raw_ostream &os, const InterfaceFile *file,
                  FileType fileType) const;

  void add(std::unique_ptr<Reader> reader) {
    _readers.emplace_back(std::move(reader));
  }

  void add(std::unique_ptr<Writer> writer) {
    _writers.emplace_back(std::move(writer));
  }

  void addBinaryReaders();
  void addYAMLReaders();
  void addYAMLWriters();
  void addDiagnosticReader();
  void addJSONReaders();
  void addJSONWriters();

private:
  std::vector<std::unique_ptr<Reader>> _readers;
  std::vector<std::unique_ptr<Writer>> _writers;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_REGISTRY_H
