//===- tapi/Core/FileSystem.h - File System ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Additional file system support that is missing in LLVM.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_FILE_SYSTEM_H
#define TAPI_CORE_FILE_SYSTEM_H

#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <system_error>

TAPI_NAMESPACE_INTERNAL_BEGIN

std::error_code realpath(SmallVectorImpl<char> &path);

std::error_code read_link(const Twine &path, SmallVectorImpl<char> &linkPath);

std::error_code shouldSkipSymlink(const Twine &path, bool &result);

std::error_code make_relative(StringRef from, StringRef to,
                              SmallVectorImpl<char> &relativePath);


// MaskingOverlayFileSystem.
// OverlayFileSystem that is capable of masking some directories so that
// it appears to be not existing. Only works as the top layer of overlay
// file system.
class MaskingOverlayFileSystem : public llvm::vfs::OverlayFileSystem {
public:
  MaskingOverlayFileSystem(IntrusiveRefCntPtr<FileSystem> base);

  llvm::ErrorOr<llvm::vfs::Status> status(const Twine &path) override;
  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(const Twine &path) override;
  llvm::vfs::directory_iterator dir_begin(const Twine &dir,
                                          std::error_code &ec) override;
  std::error_code setCurrentWorkingDirectory(const Twine &path) override;
  std::error_code isLocal(const Twine &path, bool &result) override;

protected:
  virtual bool pathMasked(const Twine &path) const = 0;
};


// Path Masking Overlay File System.
// Mask specific path from tapi and clang.
class PathMaskingOverlayFileSystem : public MaskingOverlayFileSystem {
public:
  PathMaskingOverlayFileSystem(IntrusiveRefCntPtr<FileSystem> base);

  void addExtraMaskingDirectory(StringRef path) {
    extraMaskingPath.emplace_back(path.str());
  }

private:
  bool pathMasked(const Twine &path) const override;

  std::vector<std::string> extraMaskingPath;
};

// PublicSDK Overlay File system.
// Masks the path that are not in the public SDK.
class PublicSDKOverlayFileSystem : public MaskingOverlayFileSystem {
public:
  PublicSDKOverlayFileSystem(IntrusiveRefCntPtr<FileSystem> base,
                             StringRef sysroot);

private:
  bool pathMasked(const Twine &path) const override;

  std::string sysroot;
};

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_FILE_SYSTEM_H
