//===- lib/Core/FileSystem.cpp - File System --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the additional file system support.
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/FileSystem.h"
#include "tapi/Core/LLVM.h"
#include "tapi/Core/Utils.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

using namespace llvm;

TAPI_NAMESPACE_INTERNAL_BEGIN

std::error_code realpath(SmallVectorImpl<char> &path) {
  if (path.back() != '\0')
    path.append({'\0'});
  SmallString<PATH_MAX> result;

  errno = 0;
  const char *ptr = nullptr;
  if ((ptr = ::realpath(path.data(), result.data())) == nullptr)
    return {errno, std::generic_category()};

  assert(ptr == result.data() && "Unexpected pointer");
  result.resize_for_overwrite(strlen(result.data()));
  path.swap(result);
  return {};
}

std::error_code read_link(const Twine &path, SmallVectorImpl<char> &linkPath) {
  errno = 0;
  SmallString<PATH_MAX> pathStorage;
  auto p = path.toNullTerminatedStringRef(pathStorage);
  SmallString<PATH_MAX> result;
  ssize_t len;
  if ((len = ::readlink(p.data(), result.data(), PATH_MAX)) == -1)
    return {errno, std::generic_category()};

  result.resize_for_overwrite(len);
  linkPath.swap(result);

  return {};
}

std::error_code shouldSkipSymlink(const Twine &path, bool &result) {
  result = false;
  SmallString<PATH_MAX> pathStorage;
  auto p = path.toNullTerminatedStringRef(pathStorage);
  sys::fs::file_status stat1;
  auto ec = sys::fs::status(p.data(), stat1);
  if (ec == std::errc::too_many_symbolic_link_levels) {
    result = true;
    return {};
  }

  if (ec)
    return ec;

  StringRef parent = sys::path::parent_path(p);
  while (!parent.empty()) {
    sys::fs::file_status stat2;
    if (auto ec = sys::fs::status(parent, stat2))
      return ec;

    if (sys::fs::equivalent(stat1, stat2)) {
      result = true;
      return {};
    }

    parent = sys::path::parent_path(parent);
  }

  return {};
}

std::error_code make_relative(StringRef from, StringRef to,
                              SmallVectorImpl<char> &relativePath) {
  SmallString<PATH_MAX> src = from;
  SmallString<PATH_MAX> dst = to;
  if (auto ec = sys::fs::make_absolute(src))
    return ec;

  if (auto ec = sys::fs::make_absolute(dst))
    return ec;

  SmallString<PATH_MAX> result;
  src = sys::path::parent_path(from);
  auto it1 = sys::path::begin(src), it2 = sys::path::begin(dst),
       ie1 = sys::path::end(src), ie2 = sys::path::end(dst);
  // ignore the common part.
  for (; it1 != ie1 && it2 != ie2; ++it1, ++it2) {
    if (*it1 != *it2)
      break;
  }

  for (; it1 != ie1; ++it1)
    sys::path::append(result, "../");

  for (; it2 != ie2; ++it2)
    sys::path::append(result, *it2);

  if (result.empty())
    result = ".";

  relativePath.swap(result);

  return {};
}

MaskingOverlayFileSystem::MaskingOverlayFileSystem(
    IntrusiveRefCntPtr<FileSystem> base)
    : OverlayFileSystem(base) {}

ErrorOr<vfs::Status> MaskingOverlayFileSystem::status(const Twine &path) {
  if (pathMasked(path))
    return llvm::errc::no_such_file_or_directory;

  return OverlayFileSystem::status(path);
}
vfs::directory_iterator
MaskingOverlayFileSystem::dir_begin(const Twine &dir, std::error_code &ec) {
  if (pathMasked(dir)) {
    ec = llvm::errc::no_such_file_or_directory;
    return vfs::directory_iterator();
  }

  return OverlayFileSystem::dir_begin(dir, ec);
}

std::error_code
MaskingOverlayFileSystem::setCurrentWorkingDirectory(const Twine &path) {
  if (pathMasked(path))
    return llvm::errc::no_such_file_or_directory;

  return OverlayFileSystem::setCurrentWorkingDirectory(path);
}

std::error_code MaskingOverlayFileSystem::isLocal(const Twine &path,
                                                  bool &result) {
  if (pathMasked(path))
    return llvm::errc::no_such_file_or_directory;

  return OverlayFileSystem::isLocal(path, result);
}

ErrorOr<std::unique_ptr<vfs::File>>
MaskingOverlayFileSystem::openFileForRead(const Twine &path) {
  if (pathMasked(path))
    return llvm::errc::no_such_file_or_directory;

  return OverlayFileSystem::openFileForRead(path);
}

PathMaskingOverlayFileSystem::PathMaskingOverlayFileSystem(
    IntrusiveRefCntPtr<FileSystem> base)
    : MaskingOverlayFileSystem(base) {}

bool PathMaskingOverlayFileSystem::pathMasked(const Twine &path) const {
  SmallString<PATH_MAX> realPath;
  auto p = path.toStringRef(realPath);
  for (auto &mask : extraMaskingPath) {
    if (p.startswith(mask))
      return true;
  }
  return false;
}

PublicSDKOverlayFileSystem::PublicSDKOverlayFileSystem(
    IntrusiveRefCntPtr<FileSystem> base, StringRef sysroot)
    : MaskingOverlayFileSystem(base), sysroot(sysroot.data(), sysroot.size()) {}

bool PublicSDKOverlayFileSystem::pathMasked(const Twine &path) const {
  SmallString<PATH_MAX> realPath;
  auto p = path.toStringRef(realPath);
  // If the path is from sysroot, try to test if that is a public location.
  // This is a looser check than strict public location check for now.
  if (p.consume_front(sysroot))
    return !isWithinPublicLocation(p);

  return false;
}

TAPI_NAMESPACE_INTERNAL_END
