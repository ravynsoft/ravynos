/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcformatter/NullFormatter.h>
#include <pbxbuild/Tool/Invocation.h>
#include <pbxbuild/Build/Context.h>

#include <algorithm>

using xcformatter::NullFormatter;

NullFormatter::
NullFormatter() :
    Formatter()
{
}

NullFormatter::
~NullFormatter()
{
}

std::string NullFormatter::
begin(pbxbuild::Build::Context const &buildContext)
{
    return std::string();
}

std::string NullFormatter::
success(pbxbuild::Build::Context const &buildContext)
{
    return std::string();
}

std::string NullFormatter::
failure(pbxbuild::Build::Context const &buildContext, std::vector<pbxbuild::Tool::Invocation> const &failingInvocations)
{
    return std::string();
}

std::string NullFormatter::
beginTarget(pbxbuild::Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string NullFormatter::
finishTarget(pbxbuild::Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string NullFormatter::
beginCheckDependencies(pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string NullFormatter::
finishCheckDependencies(pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string NullFormatter::
beginWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string NullFormatter::
createAuxiliaryDirectory(std::string const &directory)
{
    return std::string();
}

std::string NullFormatter::
writeAuxiliaryFile(std::string const &file)
{
    return std::string();
}

std::string NullFormatter::
setAuxiliaryExecutable(std::string const &file)
{
    return std::string();
}

std::string NullFormatter::
finishWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string NullFormatter::
beginCreateProductStructure(pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string NullFormatter::
finishCreateProductStructure(pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string NullFormatter::
beginInvocation(pbxbuild::Tool::Invocation const &invocation, std::string const &executable, bool simple)
{
    return std::string();
}

std::string NullFormatter::
finishInvocation(pbxbuild::Tool::Invocation const &invocation, std::string const &executable, bool simple)
{
    return std::string();
}

std::shared_ptr<NullFormatter> NullFormatter::
Create()
{
    return std::make_shared<NullFormatter>();
}

