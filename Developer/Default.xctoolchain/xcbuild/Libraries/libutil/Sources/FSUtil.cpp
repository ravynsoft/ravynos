/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/FSUtil.h>
#include <libutil/Absolute.h>
#include <libutil/Relative.h>

#include <cstring>

using libutil::FSUtil;
namespace Path = libutil::Path;

std::string FSUtil::
GetDirectoryName(std::string const &path)
{
    return Path::Relative(path).parent().raw();
}

std::string FSUtil::
GetBaseName(std::string const &path)
{
    return Path::Relative(path).base();
}

std::string FSUtil::
GetBaseNameWithoutExtension(std::string const &path)
{
    return Path::Relative(path).base(false);
}

std::string FSUtil::
GetRelativePath(std::string const &path, std::string const &to)
{
    ext::optional<Path::Absolute> absolute = Path::Absolute::Create(path);
    ext::optional<Path::Absolute> from = Path::Absolute::Create(to);
    if (!absolute || !from) {
        abort();
    }

    ext::optional<Path::Relative> relative = absolute->from(*from);
    if (!relative) {
        abort();
    }

    return relative->raw();
}

std::string FSUtil::
GetFileExtension(std::string const &path)
{
    return Path::Relative(path).extension();
}

bool FSUtil::
IsFileExtension(std::string const &path, std::string const &extension, bool insensitive)
{
    return Path::Relative(path).extension(extension, insensitive);
}

bool FSUtil::
IsFileExtension(std::string const &path, std::initializer_list<std::string> const &extensions, bool insensitive)
{
    Path::Relative relative = Path::Relative(path);
    for (auto const &extension : extensions) {
        if (relative.extension(extension, insensitive)) {
            return true;
        }
    }

    return false;
}

bool FSUtil::
IsAbsolutePath(std::string const &path)
{
    return Path::Relative(path).absolute() != ext::nullopt;
}

std::string FSUtil::
ResolveRelativePath(std::string const &path, std::string const &workingDirectory)
{
    ext::optional<Path::Absolute> absolute = Path::Absolute::Create(workingDirectory);
    if (!absolute) {
        abort();
    }

    return Path::Relative(path).resolved(*absolute).raw();
}

std::string FSUtil::
NormalizePath(std::string const &path)
{
    return Path::Relative(path).normalized();
}
