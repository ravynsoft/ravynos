//===- lib/Frontend/FrontendContext.cpp - TAPI Frontend Context -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TAPI Frontend Context
///
//===----------------------------------------------------------------------===//

#include "clang/Lex/Preprocessor.h"
#include "tapi/Frontend/FrontendContext.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

FrontendContext::FrontendContext(const llvm::Triple &triple,
                                 SymbolVerifier *verifier,
                                 IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs,
                                 HeaderType type)
    : target(triple), verifier(verifier),
      api(std::make_shared<API>(API(triple))), type(type) {
  fileManager = new FileManager(clang::FileSystemOptions(), vfs);
}

std::optional<HeaderType>
FrontendContext::findAndRecordFile(const FileEntry *file) {
  if (!file)
    return std::nullopt;

  auto it = knownFiles.find(file);
  if (it != knownFiles.end())
    return it->second;

  // Check if file was previously found, but not one tapi is interested in.
  auto unused = unusedFiles.find(file);
  if (unused != unusedFiles.end())
    return std::nullopt;

  // If file was not found, search by how the header was
  // included. This is primarily to resolve headers found
  // in a different location than what passed as input.
  auto includeName = pp->getHeaderSearchInfo().getIncludeNameForHeader(file);
  auto backup = knownIncludes.find(includeName.str());
  if (backup != knownIncludes.end()) {
    knownFiles[file] = backup->second;
    return backup->second;
  }

  // Record that the file was found to avoid future string searches for the
  // same file.
  unusedFiles.insert(file);
  return std::nullopt;
}

TAPI_NAMESPACE_INTERNAL_END
