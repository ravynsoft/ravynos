/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/DittoResolver.h>
#include <pbxbuild/Tool/Context.h>
#include <libutil/FSUtil.h>

namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;

Tool::DittoResolver::
DittoResolver(pbxspec::PBX::Tool::shared_ptr const &tool) :
    _tool(tool)
{
}

void Tool::DittoResolver::
resolve(
    Tool::Context *toolContext,
    std::string const &sourcePath,
    std::string const &targetPath) const
{
    std::string logMessage = "Ditto " + targetPath + " " + sourcePath;

    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::External("/usr/bin/ditto"); // TODO(grp): Ditto is not portable.
    invocation.arguments() = { "-rsrc", sourcePath, targetPath };
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.inputs() = { FSUtil::ResolveRelativePath(sourcePath, toolContext->workingDirectory()) };
    invocation.outputs() = { FSUtil::ResolveRelativePath(targetPath, toolContext->workingDirectory()) };
    invocation.logMessage() = logMessage;
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

std::unique_ptr<Tool::DittoResolver> Tool::DittoResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Tool::shared_ptr dittoTool = specManager->tool(Tool::DittoResolver::ToolIdentifier(), specDomains);
    if (dittoTool == nullptr) {
        fprintf(stderr, "warning: could not find ditto tool\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::DittoResolver>(new Tool::DittoResolver(dittoTool));
}

