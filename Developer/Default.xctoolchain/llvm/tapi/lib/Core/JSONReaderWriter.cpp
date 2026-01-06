//===- tapi/Core/JSONReaderWriter.cpp - JSON Reader/Writer ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the JSON reader/writer.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/JSONReaderWriter.h"
#include "tapi/Core/YAMLReaderWriter.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include "llvm/TextAPI/TextAPIReader.h"
#include "llvm/TextAPI/TextAPIWriter.h"

using namespace llvm;
using namespace llvm::MachO;
using namespace tapi::internal;

TAPI_NAMESPACE_INTERNAL_BEGIN

bool JSONReader::canRead(file_magic magic, MemoryBufferRef memBufferRef,
                         FileType types) const {
  if (!memBufferRef.getBufferIdentifier().endswith(".tbd"))
    return false;
  auto result = TextAPIReader::canRead(memBufferRef);
  if (!result) {
    consumeError(result.takeError());
    return false;
  }
  return *result >= TBD_V5;
}

Expected<APIs> JSONReader::readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                                    ReadFlags readFlags,
                                    ArchitectureSet arches) const {

  auto interfaceOrErr = TextAPIReader::get(memBuffer->getMemBufferRef());
  if (!interfaceOrErr)
    return interfaceOrErr.takeError();
  auto interface = std::move(*interfaceOrErr);
  assert(interface->getFileType() >= TBD_V5 &&
         "expected json supported tapi file.");

  APIs apis;
  addInterfaceFileToAPIs(apis, interface.get());
  for (auto &doc : interface->documents())
    addInterfaceFileToAPIs(apis, doc.get());

  return std::move(apis);
}

Error JSONWriter::writeFile(raw_ostream &os, const InterfaceFile *file,
                            FileType fileType) const {
  if (file == nullptr)
    return errorCodeToError(std::make_error_code(std::errc::invalid_argument));

  return TextAPIWriter::writeToStream(os, *file, fileType, /*Compact=*/false);
}

TAPI_NAMESPACE_INTERNAL_END
