//===- tapi/Core/MachODylibReader.h - TAPI MachO Dylib Reader ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the MachO Dynamic Library Reader.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_MACHO_DYLIB_READER_H
#define TAPI_CORE_MACHO_DYLIB_READER_H

#include "tapi/Core/API.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/Registry.h"
#include "tapi/Defines.h"
#include "llvm/BinaryFormat/Magic.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/TextAPI/ArchitectureSet.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

class MachODylibReader final : public Reader {
public:
  MachODylibReader() : Reader(Binary) {}
  bool canRead(file_magic magic, MemoryBufferRef bufferRef,
               FileType types) const override;
  Expected<FileType> getFileType(file_magic magic,
                                 MemoryBufferRef bufferRef) const override;
  Expected<APIs> readFile(std::unique_ptr<MemoryBuffer> memBuffer,
                          ReadFlags readFlags,
                          ArchitectureSet arches) const override;

  static bool classof(const Reader *reader) {
    return reader->getKind() == Binary;
  }
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_MACHO_DYLIB_READER_H
