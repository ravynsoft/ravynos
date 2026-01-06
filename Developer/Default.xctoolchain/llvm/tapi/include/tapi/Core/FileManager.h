//===- tapi/Core/FileManager.h - File Manager --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Extends the clang file manager.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_FILE_MANAGER_H
#define TAPI_CORE_FILE_MANAGER_H

#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "clang/Basic/FileManager.h"

TAPI_NAMESPACE_INTERNAL_BEGIN

/// \brief Basically the clang FileManager with additonal convenience methods
///        and a recording stat cache.
class FileManager final : public clang::FileManager {
public:
  FileManager(const clang::FileSystemOptions &fileSystemOpts,
              llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs = nullptr);

  /// \brief Check if a particular path exists.
  bool exists(StringRef path);

  /// \brief Check if a particular path is a directory.
  bool isDirectory(StringRef path, bool CacheFailure = true) {
    return (bool)getDirectory(path, CacheFailure);
  }

  /// \brief Check if a particular path is a symlink using directory_iterator.
  bool isSymlink(StringRef path);

private:
  bool initWithVFS = false;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_FILE_MANAGER_H
