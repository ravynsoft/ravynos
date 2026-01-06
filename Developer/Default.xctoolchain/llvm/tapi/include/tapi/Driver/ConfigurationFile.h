//===- tapi/Driver/ConfigurationFile.h - Configuration File -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the configuration file.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_CONFIGURATION_FILE_H
#define TAPI_CORE_CONFIGURATION_FILE_H

#include "tapi/Core/LLVM.h"
#include "tapi/Core/Path.h"
#include "tapi/Defines.h"
#include "clang/Frontend/FrontendOptions.h"
#include "llvm/TextAPI/PackedVersion.h"
#include "llvm/TextAPI/Platform.h"
#include <string>
#include <utility>
#include <vector>


TAPI_NAMESPACE_INTERNAL_BEGIN

using Macro = std::pair<std::string, bool>;
static const clang::Language defaultLanguage = clang::Language::ObjC;

namespace configuration {
namespace v1 {

struct HeaderConfiguration {
  std::string umbrellaHeader;
  PathSeq preIncludes;
  PathSeq includes;
  PathSeq excludes;
};

struct FrameworkConfiguration {
  std::string name;
  std::string path;
  std::string installName;
  clang::Language language;
  PathSeq includePaths;
  PathSeq frameworkPaths;
  std::vector<Macro> macros;
  HeaderConfiguration publicHeaderConfiguration;
  HeaderConfiguration privateHeaderConfiguration;
  bool scanSwiftModule = false;
  bool useOverlay = false;
  std::vector<std::string> clangExtraArgs;
};

struct ProjectConfiguration {
  std::string name;
  clang::Language language;
  std::string languageStd;
  PathSeq includePaths;
  PathSeq frameworkPaths;
  std::vector<Macro> macros;
  bool isiOSMac = false;
  bool isZippered = false;
  bool scanSwiftModule = false;
  bool useOverlay = false;
  bool useUmbrellaOnly = false;
  bool useSplitHeaderDir = false;
  bool scanPublicHeadersInSDKContentRoot = false;
  bool ignoreExistingPartialSDKDBs = false;
  PathSeq rootMaskPaths;
  PathSeq sdkMaskPaths;
  HeaderConfiguration publicHeaderConfiguration;
  HeaderConfiguration privateHeaderConfiguration;
  std::vector<std::string> clangExtraArgs;
};

}
} // end namespace configuration::v1.

class ConfigurationFile {
public:
  PlatformType platform;
  PackedVersion version;
  std::string isysroot;
  clang::Language language{defaultLanguage};
  PathSeq includePaths;
  PathSeq frameworkPaths;
  std::vector<Macro> macros;
  std::vector<std::string> publicDylibs;
  std::vector<configuration::v1::FrameworkConfiguration>
      frameworkConfigurations;
  std::vector<configuration::v1::ProjectConfiguration> projectConfigurations;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_CONFIGURATION_FILE_H
