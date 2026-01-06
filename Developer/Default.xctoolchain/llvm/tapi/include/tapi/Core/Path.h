//===- tapi/Core/Path.h - Path Operating System Concept ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Modified path support to handle frameworks.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CORE_PATH_H
#define TAPI_CORE_PATH_H

#include "tapi/Core/LLVM.h"
#include "tapi/Defines.h"
#include "llvm/Support/Error.h"

#include <optional>
#include <functional>
#include <string>
#include <vector>

TAPI_NAMESPACE_INTERNAL_BEGIN

class FileManager;

using PathSeq = std::vector<std::string>;
using PathToPlatform = std::pair<std::string, std::optional<PlatformType>>;
using PathToPlatformSeq = std::vector<PathToPlatform>;

void replace_extension(SmallVectorImpl<char> &path, const Twine &extension);

llvm::Expected<PathSeq>
enumerateFiles(FileManager &fm, StringRef path,
               const std::function<bool(StringRef)> &func);

llvm::Expected<PathSeq> enumerateHeaderFiles(FileManager &fm, StringRef path);

PathSeq getPathsForPlatform(const PathToPlatformSeq &paths,
                            PlatformType platform);

PathSeq getAllPaths(const PathToPlatformSeq &paths);

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CORE_PATH_H
