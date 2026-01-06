/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/SymlinkResolver.h>
#include <pbxbuild/Tool/Context.h>
#include <libutil/FSUtil.h>

namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;

Tool::SymlinkResolver::
SymlinkResolver(pbxspec::PBX::Tool::shared_ptr const &tool) :
    _tool(tool)
{
}

void Tool::SymlinkResolver::
resolve(
    Tool::Context *toolContext,
    std::string const &workingDirectory,
    std::string const &symlinkPath,
    std::string const &targetPath,
    bool productStructure) const
{
    std::string logMessage = "SymLink " + targetPath + " " + symlinkPath;

    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::External("/bin/ln");
    invocation.arguments() = { "-sfh", targetPath, symlinkPath };
    invocation.workingDirectory() = workingDirectory;
    invocation.phonyInputs() = { FSUtil::ResolveRelativePath(targetPath, workingDirectory) };
    invocation.outputs() = { FSUtil::ResolveRelativePath(symlinkPath, workingDirectory) };
    invocation.logMessage() = logMessage;
    invocation.createsProductStructure() = productStructure;
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

std::unique_ptr<Tool::SymlinkResolver> Tool::SymlinkResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Tool::shared_ptr symlinkTool = specManager->tool(Tool::SymlinkResolver::ToolIdentifier(), specDomains);
    if (symlinkTool == nullptr) {
        fprintf(stderr, "warning: could not find symlink tool\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::SymlinkResolver>(new Tool::SymlinkResolver(symlinkTool));
}

