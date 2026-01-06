//===- Frontend/FrontendContext.h - TAPI Frontend Context -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the Frontend Context
///
//===----------------------------------------------------------------------===//
#ifndef TAPI_FRONTEND_FRONTENDCONTEXT_H
#define TAPI_FRONTEND_FRONTENDCONTEXT_H

#include "tapi/Core/API.h"
#include "tapi/Core/FileManager.h"
#include "tapi/Core/HeaderFile.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/SymbolVerifier.h"
#include "tapi/Defines.h"
#include "clang/AST/ASTContext.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include <map>

TAPI_NAMESPACE_INTERNAL_BEGIN

struct FrontendContext {
  const llvm::Triple target;
  SymbolVerifier *verifier;
  std::shared_ptr<API> api;
  std::unique_ptr<clang::CompilerInstance> compiler;
  llvm::IntrusiveRefCntPtr<clang::ASTContext> ast;
  llvm::IntrusiveRefCntPtr<clang::SourceManager> sourceMgr;
  std::shared_ptr<clang::Preprocessor> pp;
  llvm::IntrusiveRefCntPtr<FileManager> fileManager;
  HeaderType type;

  using HeaderMap = std::map<const FileEntry *, HeaderType>;
  HeaderMap knownFiles;
  std::map<const std::string, HeaderType> knownIncludes;

  FrontendContext(const llvm::Triple &triple, SymbolVerifier *verifier,
                  IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs = nullptr,
                  HeaderType type = HeaderType::Project);

  void visit(APIVisitor &visitor) const { api->visit(visitor); }
  void visit(APIMutator &visitor) { api->visit(visitor); }

  std::optional<HeaderType> findAndRecordFile(const FileEntry *file);

private:
  std::set<const FileEntry *> unusedFiles;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_FRONTEND_FRONTENDCONTEXT_H
