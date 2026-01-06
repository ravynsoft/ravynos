//===- lib/Core/FileManager.cpp - File Manager ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the file manager.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/FileManager.h"
#include "tapi/Defines.h"
#include "clang/Basic/FileSystemStatCache.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/VirtualFileSystem.h"

using namespace llvm;
using namespace clang;

TAPI_NAMESPACE_INTERNAL_BEGIN

FileManager::FileManager(const FileSystemOptions &fileSystemOpts,
                         IntrusiveRefCntPtr<vfs::FileSystem> fs)
    : clang::FileManager(fileSystemOpts, fs) {
  // Record if initialized with VFS.
  if (fs)
    initWithVFS = true;
}

bool FileManager::exists(StringRef path) {
  llvm::vfs::Status result;
  if (getNoncachedStatValue(path, result))
    return false;
  return result.exists();
}

bool FileManager::isSymlink(StringRef path) {
  if (initWithVFS)
    return false;
  return sys::fs::is_symlink_file(path);
}

TAPI_NAMESPACE_INTERNAL_END
