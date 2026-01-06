//===- tapi/Core/Framework.h - TAPI Framework -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the content of a framework, such as public and private
/// header files, and dynamic libraries.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_FRAMEWORK_H
#define TAPI_CORE_FRAMEWORK_H

#include "tapi/Core/HeaderFile.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/Path.h"
#include "tapi/Defines.h"
#include "tapi/Frontend/FrontendContext.h"
#include "llvm/ADT/StringRef.h"
#include <map>
#include <string>
#include <vector>

TAPI_NAMESPACE_INTERNAL_BEGIN

enum class HeaderType;

struct SwiftModule {
  std::string name;
  PathSeq swiftInterfaces;

  SwiftModule(StringRef path);

  void addSwiftInterface(StringRef path) {
    swiftInterfaces.emplace_back(path);
  }
};

struct Framework {
  std::string _baseDirectory;
  HeaderSeq _headerFiles;
  PathSeq _moduleMaps;
  PathSeq _dynamicLibraryFiles;
  std::vector<SwiftModule> _swiftModules;
  std::vector<Framework> _subFrameworks;
  std::vector<Framework> _versions;
  std::vector<std::unique_ptr<InterfaceFile>> _interfaceFiles;
  std::unique_ptr<SymbolSet> _headerSymbols;
  std::vector<FrontendContext> _frontendResults;
  bool isDynamicLibrary{false};
  bool isSysRoot{false};

  Framework(StringRef directory) : _baseDirectory(directory) {}

  static StringRef getNameFromInstallName(StringRef installName);

  StringRef getName() const;

  StringRef getPath() const { return _baseDirectory; }

  bool isMacCatalyst() const;
  bool isDriverKit() const;

  StringRef getAdditionalIncludePath() const;
  StringRef getAdditionalFrameworkPath() const;

  void addHeaderFile(StringRef fullPath, HeaderType type,
                     StringRef relativePath = StringRef(),
                     StringRef includePath = StringRef()) {
    _headerFiles.emplace_back(fullPath, type, relativePath, includePath);
  }

  void addModuleMap(StringRef path) { _moduleMaps.emplace_back(path); }

  void addDynamicLibraryFile(StringRef path);

  bool empty() {
    return _subFrameworks.empty() && _headerFiles.empty() &&
           _moduleMaps.empty() && _dynamicLibraryFiles.empty() &&
           _versions.empty();
  }
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_FRAMEWORK_H
