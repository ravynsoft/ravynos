/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/Invocation.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>

namespace Tool = pbxbuild::Tool;
using DependencyInfo = Tool::Invocation::DependencyInfo;
using Executable = Tool::Invocation::Executable;
using libutil::Filesystem;
using libutil::FSUtil;

DependencyInfo::
DependencyInfo(dependency::DependencyInfoFormat format, std::string const &path) :
    _format(format),
    _path  (path)
{
}

Executable::
Executable(
    ext::optional<std::string> const &external,
    ext::optional<std::string> const &builtin) :
    _external(external),
    _builtin (builtin)
{
}

Executable Executable::
External(std::string const &path)
{
    return Executable(path, ext::nullopt);
}

Executable Executable::
Builtin(std::string const &name)
{
    return Executable(ext::nullopt, name);
}

ext::optional<Executable> Executable::
Determine(std::string const &executable)
{
    if (executable.empty()) {
        return ext::nullopt;
    }

    std::string builtinPrefix = "builtin-";
    if (executable.compare(0, builtinPrefix.size(), builtinPrefix) == 0) {
        /* Has a builtin prefix. */
        return Builtin(executable);
    } else {
        /* Unknown external tool. */
        return External(executable);
    }
}

Tool::Invocation::
Invocation() :
    _showEnvironmentInLog   (true),
    _createsProductStructure(false),
    _waitForSwiftArtifacts  (false)
{
}

Tool::Invocation::
~Invocation()
{
}

