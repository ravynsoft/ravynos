//===- IntefaceFileManager.h - TAPI Interface File Manager ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Interface File Manager
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_INTERFACE_FILE_MANAGER_H
#define TAPI_CORE_INTERFACE_FILE_MANAGER_H

#include "tapi/Core/Registry.h"
#include "tapi/Defines.h"
#include "llvm/TextAPI/InterfaceFile.h"
#include <map>

TAPI_NAMESPACE_INTERNAL_BEGIN

class FileManager;

class InterfaceFileManager {
public:
  InterfaceFileManager(FileManager &fm, bool isVolatile);
  Expected<APIs &> readFile(const std::string &path,
                            ReadFlags flags = ReadFlags::Symbols);
  Error writeFile(const std::string &path, const InterfaceFile *file,
                  FileType fileType) const;

private:
  FileManager &_fm;
  Registry _registry;
  std::map<std::string, APIs> _libraries;
  bool isVolatile;

  enum class WriteAction {
    SkipWrite = 0,
    NewFile,
    ReplaceFile,
  };

  WriteAction shouldWrite(const std::string &path, const InterfaceFile *file,
                          FileType fileType) const;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_INTERFACE_FILE_MANAGER_H
