/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/MakeDirectoryResolver.h>
#include <pbxbuild/Tool/Context.h>
#include <libutil/FSUtil.h>

namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;

Tool::MakeDirectoryResolver::
MakeDirectoryResolver(pbxspec::PBX::Tool::shared_ptr const &tool) :
    _tool(tool)
{
}

void Tool::MakeDirectoryResolver::
resolve(
    Tool::Context *toolContext,
    std::string const &directory,
    bool productStructure) const
{
    std::string logMessage = "MkDir " + directory;

    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::External("/bin/mkdir");
    invocation.arguments() = { "-p", directory };
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.outputs() = { FSUtil::ResolveRelativePath(directory, toolContext->workingDirectory()) };
    invocation.logMessage() = "MkDir " + directory;
    invocation.createsProductStructure() = productStructure;
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

std::unique_ptr<Tool::MakeDirectoryResolver> Tool::MakeDirectoryResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Tool::shared_ptr mkdirTool = specManager->tool(Tool::MakeDirectoryResolver::ToolIdentifier(), specDomains);
    if (mkdirTool == nullptr) {
        fprintf(stderr, "warning: could not find mkdir tool\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::MakeDirectoryResolver>(new Tool::MakeDirectoryResolver(mkdirTool));
}

