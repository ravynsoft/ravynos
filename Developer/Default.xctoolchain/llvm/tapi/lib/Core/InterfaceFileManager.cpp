//===- InterfaceFileManager.cpp - TAPI Interface File Manager ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TAPI Interface File Manager.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/InterfaceFileManager.h"
#include "tapi/Core/FileManager.h"
#include "tapi/Core/Registry.h"
#include "tapi/Defines.h"
#include "llvm/Support/Error.h"
#include "llvm/TextAPI/TextAPIError.h"

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

InterfaceFileManager::InterfaceFileManager(FileManager &fm,
                                           const bool isVolatile)
    : _fm(fm), isVolatile(isVolatile) {
  _registry.addYAMLReaders();
  _registry.addYAMLWriters();
  _registry.addBinaryReaders();
  _registry.addJSONReaders();
  _registry.addJSONWriters();
}

Expected<APIs &> InterfaceFileManager::readFile(const std::string &path,
                                                const ReadFlags flags) {
  auto file = _fm.getFile(path);
  if (!file)
    return errorCodeToError(file.getError());

  auto bufferOrErr = _fm.getBufferForFile(*file,
                                          /*RequiresNullTerminator=*/true,
                                          /*IsVolatile=*/isVolatile);
  if (!bufferOrErr)
    return errorCodeToError(bufferOrErr.getError());

  auto apis = _registry.readFile(std::move(bufferOrErr.get()), flags);
  if (!apis)
    return apis.takeError();

  // Use path location for lookup because
  // it's possible to contain different interfaces for the same library.
  if (!apis->empty()) {
    auto api = *apis->begin();
    if (api->hasBinaryInfo()) {
      auto it = _libraries.find(api->getBinaryInfo().path.str());
      if (it != _libraries.end())
        return it->second;
    }
  }

  auto result = _libraries.emplace(path, std::move(apis.get()));
  return result.first->second;
}

InterfaceFileManager::WriteAction
InterfaceFileManager::shouldWrite(const std::string &path,
                                  const InterfaceFile *file,
                                  FileType fileType) const {
  auto existingFile = _fm.getFile(path);
  if (auto err = existingFile.getError())
    return WriteAction::NewFile;

  auto bufferOrErr = _fm.getBufferForFile(*existingFile,
                                          /*RequiresNullTerminator=*/true,
                                          /*IsVolatile=*/isVolatile);
  if (auto err = bufferOrErr.getError())
    return WriteAction::NewFile;

  if (bufferOrErr.get()->getBufferSize() == 0)
    return WriteAction::ReplaceFile;

  auto existingIFOrErr = _registry.readTextFile(std::move(bufferOrErr.get()));
  if (auto err = existingIFOrErr.takeError()) {
    return WriteAction::NewFile;
  }
  const auto existingIF = std::move(*existingIFOrErr);
  if (existingIF->getFileType() != fileType)
    return WriteAction::ReplaceFile;

  if (*existingIF == *file)
    return WriteAction::SkipWrite;

  return WriteAction::ReplaceFile;
}

Error InterfaceFileManager::writeFile(const std::string &path,
                                      const InterfaceFile *file,
                                      FileType fileType) const {
  switch (shouldWrite(path, file, fileType)) {
  case WriteAction::SkipWrite:
    return Error::success();
  case WriteAction::NewFile:
    return _registry.writeFile(path, file, fileType, /*replaceFile=*/false);
  case WriteAction::ReplaceFile:
    return _registry.writeFile(path, file, fileType, /*replaceFile=*/true);
  }
  llvm_unreachable("unexpected WriteAction result");
}

TAPI_NAMESPACE_INTERNAL_END
