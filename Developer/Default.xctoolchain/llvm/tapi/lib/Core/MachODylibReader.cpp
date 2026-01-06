//===- lib/Core/MachODylibReader.cpp - TAPI MachO Dylib Reader --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the object specific parts of reading the dylib files.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/MachODylibReader.h"
#include "tapi/Core/API.h"
#include "tapi/Core/APIVisitor.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/MachOReader.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/BinaryFormat/Magic.h"
#include "tapi/ObjCMetadata/ObjCMetadata.h"
#include "llvm/Object/MachO.h"
#include "llvm/Object/MachOUniversal.h"
#include "llvm/TargetParser/Triple.h"
#include <tuple>

using namespace llvm;
using namespace llvm::MachO;
using namespace llvm::object;

TAPI_NAMESPACE_INTERNAL_BEGIN

Expected<FileType>
MachODylibReader::getFileType(file_magic magic,
                              MemoryBufferRef bufferRef) const {
  return getMachOFileType(bufferRef);
}

bool MachODylibReader::canRead(file_magic magic, MemoryBufferRef bufferRef,
                               FileType types) const {
  if (!(types & FileType::MachO_DynamicLibrary) &&
      !(types & FileType::MachO_DynamicLibrary_Stub) &&
      !(types & FileType::MachO_Bundle))
    return false;

  auto fileType = getFileType(magic, bufferRef);
  if (!fileType) {
    consumeError(fileType.takeError());
    return false;
  }

  return types & fileType.get();
}

Expected<APIs>
MachODylibReader::readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                           ReadFlags readFlags, ArchitectureSet arches) const {
  MachOParseOption option;
  option.arches = arches;
  if (readFlags < ReadFlags::ObjCMetadata)
    option.parseObjCMetadata = false;
  if (readFlags < ReadFlags::Symbols)
    option.parseUndefined = false;
  if (readFlags < ReadFlags::DefinedSymbols)
    option.parseSymbolTable = false;
  if (readFlags < ReadFlags::Header)
    option.parseObjCMetadata = false;

  auto results = readMachOFile(memBuffer->getMemBufferRef(), option);
  if (!results)
    return results.takeError();

  APIs apis;
  for (auto &result : *results) {
    if (result.second->hasBinaryInfo()) {
      auto &binaryInfo = result.second->getBinaryInfo();
      binaryInfo.path =
          result.second->copyString(memBuffer->getBufferIdentifier());
    }
    apis.emplace_back(std::move(result.second));
  }
  return apis;
}

TAPI_NAMESPACE_INTERNAL_END
