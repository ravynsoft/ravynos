//===- tapi/Frontend/Frontend.h - TAPI Frontend -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the TAPI Frontend.
//===----------------------------------------------------------------------===//

#ifndef TAPI_FRONTEND_FRONTEND_H
#define TAPI_FRONTEND_FRONTEND_H

#include "tapi/Core/FileManager.h"
#include "tapi/Core/HeaderFile.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/Path.h"
#include "tapi/Core/SymbolVerifier.h"
#include "tapi/Defines.h"
#include "tapi/Frontend/FrontendContext.h"
#include "clang/Frontend/FrontendOptions.h"
#include "llvm/TargetParser/Triple.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

struct FrontendJob {
  IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs;
  llvm::Triple target;
  clang::Language language = clang::Language::Unknown;
  bool overwriteRTTI = false;
  bool overwriteNoRTTI = false;
  bool enableModules = false;
  bool validateSystemHeaders = false;
  bool useObjectiveCARC = false;
  bool useObjectiveCWeakARC = false;
  bool useUmbrellaHeaderOnly = false;
  bool verbose = false;
  bool createClangReproducer = false;
  bool useRelativePath = false;
  std::string language_std;
  std::string visibility;
  std::string isysroot;
  std::string moduleCachePath;
  std::string productName;
  std::string clangResourcePath;
  std::string clangReproducerPath;
  std::string label;
  std::vector<std::pair<std::string, bool /*isUndef*/>> macros;
  HeaderSeq headerFiles;
  PathSeq quotedIncludePaths;
  PathSeq systemFrameworkPaths;
  PathSeq systemIncludePaths;
  PathSeq afterIncludePaths;
  PathSeq frameworkPaths;
  PathSeq includePaths;
  std::vector<std::string> clangExtraArgs;
  std::vector<std::string> prefixHeaders;
  HeaderType type;
  std::optional<std::string> clangExecutablePath;
  std::shared_ptr<SymbolVerifier> verifier =
      std::make_shared<SymbolVerifier>(SymbolVerifier());
};

extern llvm::Expected<FrontendContext>
runFrontend(const FrontendJob &job, StringRef inputFilename = StringRef());

bool canIgnoreFrontendError(llvm::Error &error);

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_FRONTEND_FRONTEND_H
