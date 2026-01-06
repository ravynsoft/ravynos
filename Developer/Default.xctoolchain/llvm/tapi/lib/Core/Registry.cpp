//===- lib/Core/Registry.cpp - TAPI Registry --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TAPI Registry.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/Registry.h"
#include "tapi/Core/JSONReaderWriter.h"
#include "tapi/Core/MachODylibReader.h"
#include "tapi/Core/YAMLReaderWriter.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TextAPI/TextAPIReader.h"

using namespace llvm;
using namespace llvm::MachO;

TAPI_NAMESPACE_INTERNAL_BEGIN

namespace {

// Diagnostic reader. It can read all the YAML file with !tapi tag and returns
// proper error when started to read the file.
class DiagnosticReader : public Reader {
public:
  DiagnosticReader() : Reader(Diagnostic) {}
  bool canRead(file_magic fileType, MemoryBufferRef bufferRef,
               FileType types = FileType::All) const override;
  Expected<FileType> getFileType(file_magic magic,
                                 MemoryBufferRef bufferRef) const override;
  Expected<APIs> readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                          ReadFlags readFlags,
                          ArchitectureSet arches) const override;

  static bool classof(const Reader *reader) {
    return reader->getKind() == Diagnostic;
  }
};

} // namespace

bool DiagnosticReader::canRead(file_magic fileType, MemoryBufferRef bufferRef,
                               FileType types) const {
  auto str = bufferRef.getBuffer().trim();
  if (!str.startswith("--- !tapi") || !str.endswith("..."))
    return false;

  return true;
}

Expected<FileType>
DiagnosticReader::getFileType(file_magic magic,
                              MemoryBufferRef bufferRef) const {
  return Invalid;
}

Expected<APIs>
DiagnosticReader::readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                           ReadFlags readFlags, ArchitectureSet arches) const {
  auto str = memBuffer->getBuffer().trim();
  auto tag = str.split('\n').first.drop_front(4);
  return make_error<StringError>(
      "unsupported tapi file type \'" + tag.str() + "\' in YAML",
      std::make_error_code(std::errc::not_supported));
}

bool Registry::canRead(MemoryBufferRef memBuffer, FileType types) const {
  auto data = memBuffer.getBuffer();
  auto magic = identify_magic(data);

  for (const auto &reader : _readers) {
    if (reader->canRead(magic, memBuffer, types))
      return true;
  }

  return false;
}

Expected<FileType> Registry::getFileType(MemoryBufferRef memBuffer) const {
  auto data = memBuffer.getBuffer();
  auto magic = identify_magic(data);

  for (const auto &reader : _readers) {
    auto fileType = reader->getFileType(magic, memBuffer);
    if (!fileType)
      return fileType.takeError();
    if (fileType.get() != FileType::Invalid)
      return fileType;
  }

  return FileType::Invalid;
}

bool Registry::canWrite(const InterfaceFile *file, FileType fileType) const {
  for (const auto &writer : _writers) {
    if (writer->canWrite(file, fileType))
      return true;
  }

  return false;
}

Expected<APIs> Registry::readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                                  ReadFlags readFlags,
                                  ArchitectureSet arches) const {
  auto data = memBuffer->getBuffer();
  auto fileType = identify_magic(data);

  for (const auto &reader : _readers) {
    if (!reader->canRead(fileType, memBuffer->getMemBufferRef()))
      continue;
    return reader->readFile(std::move(memBuffer), readFlags, arches);
  }

  return make_error<StringError>(
      "unsupported file type", std::make_error_code(std::errc::not_supported));
}

Expected<std::unique_ptr<InterfaceFile>>
Registry::readTextFile(std::unique_ptr<MemoryBuffer> memBuffer, ReadFlags readFlags,
                   ArchitectureSet arches) const {
  auto data = memBuffer->getBuffer().trim();
  auto fileType = identify_magic(data);

  if (fileType != file_magic::tapi_file)
    return make_error<StringError>(
      "unsupported file type", std::make_error_code(std::errc::not_supported));

  for (const auto &reader : _readers) {
    if (!reader->canRead(fileType, memBuffer->getMemBufferRef()))
      continue;

    auto interfaceOrErr = TextAPIReader::get(memBuffer->getMemBufferRef());
    if (!interfaceOrErr)
      return interfaceOrErr.takeError();

    if (!(*interfaceOrErr))
      return errorCodeToError(std::make_error_code(std::errc::not_supported));

    return std::move(*interfaceOrErr);
  }

  return make_error<StringError>(
      "unsupported file type", std::make_error_code(std::errc::not_supported));
}

// writeFile attempts to write the file in a temporary location then
// atomically rename it to the final destination(path), if requested to replace
// existing file. This approach was taken from
// CompilerInstance::createOutputFile, which is non static member function.
Error Registry::writeFile(const std::string &path, const InterfaceFile *file,
                          FileType fileType, bool replaceFile) const {
  using namespace llvm::sys;

  auto writeFileWithoutTemporary = [&]() -> Error {
    std::error_code error;
    llvm::raw_fd_ostream os(path, error, fs::OF_None);
    if (auto error = writeFile(os, file, fileType))
      return error;
    return Error::success();
  };

  if (!replaceFile)
    return writeFileWithoutTemporary();

  SmallString<128> tmpPath;
  const StringRef outputExtension = path::extension(path);
  tmpPath = StringRef(path).drop_back(outputExtension.size());
  tmpPath += "-%%%%%%%%";
  tmpPath += outputExtension;
  tmpPath += ".tmp";

  int fd;
  std::error_code error = fs::createUniqueFile(
      tmpPath, fd, tmpPath, fs::OF_None, fs::all_read | fs::all_write);

  // Fall back to not writing to temporary file.
  if (error)
    return writeFileWithoutTemporary();

  llvm::raw_fd_ostream os(fd, /*shouldClose*/ true);
  if (auto error = writeFile(os, file, fileType))
    return error;

  if (auto error = fs::rename(tmpPath, path))
    return llvm::errorCodeToError(error);

  return Error::success();
}

Error Registry::writeFile(raw_ostream &os, const InterfaceFile *file,
                          FileType fileType) const {
  for (const auto &writer : _writers) {
    if (!writer->canWrite(file, fileType))
      continue;
    return writer->writeFile(os, file, fileType);
  }

  return make_error<StringError>(
      "unsupported file type", std::make_error_code(std::errc::not_supported));
}

void Registry::addBinaryReaders() {
  add(std::unique_ptr<Reader>(new MachODylibReader));
}

void Registry::addYAMLReaders() {
  auto reader = std::make_unique<YAMLReader>();
  reader->add(
      std::unique_ptr<DocumentHandler>(new stub::v1::YAMLDocumentHandler));
  reader->add(
      std::unique_ptr<DocumentHandler>(new stub::v2::YAMLDocumentHandler));
  reader->add(
      std::unique_ptr<DocumentHandler>(new stub::v3::YAMLDocumentHandler));
  reader->add(
      std::unique_ptr<DocumentHandler>(new stub::v4::YAMLDocumentHandler));
  add(std::unique_ptr<Reader>(std::move(reader)));
}

void Registry::addYAMLWriters() {
  auto writer = std::make_unique<YAMLWriter>();
  writer->add(
      std::unique_ptr<DocumentHandler>(new stub::v1::YAMLDocumentHandler));
  writer->add(
      std::unique_ptr<DocumentHandler>(new stub::v2::YAMLDocumentHandler));
  writer->add(
      std::unique_ptr<DocumentHandler>(new stub::v3::YAMLDocumentHandler));
  writer->add(
      std::unique_ptr<DocumentHandler>(new stub::v4::YAMLDocumentHandler));
  add(std::unique_ptr<Writer>(std::move(writer)));
}

void Registry::addJSONReaders() {
  add(std::unique_ptr<Reader>(new JSONReader));
}

void Registry::addJSONWriters() {
  add(std::unique_ptr<Writer>(new JSONWriter));
}

void Registry::addDiagnosticReader() {
  add(std::make_unique<DiagnosticReader>());
}

TAPI_NAMESPACE_INTERNAL_END
