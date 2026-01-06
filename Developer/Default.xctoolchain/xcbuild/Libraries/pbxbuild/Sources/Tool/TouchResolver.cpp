/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/TouchResolver.h>
#include <pbxbuild/Tool/Context.h>
#include <libutil/FSUtil.h>

namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;

Tool::TouchResolver::
TouchResolver(pbxspec::PBX::Tool::shared_ptr const &tool) :
    _tool(tool)
{
}

Tool::TouchResolver::
~TouchResolver()
{
}

void Tool::TouchResolver::
resolve(
    Tool::Context *toolContext,
    std::string const &input,
    std::vector<std::string> const &dependencies) const
{
    std::string logMessage = "Touch " + input;

    /* Treat the input as an output since it's what gets modified by the touch. */
    std::string output = FSUtil::ResolveRelativePath(input, toolContext->workingDirectory());

    std::vector<std::string> inputDependencies;
    for (std::string const &dependency : dependencies) {
        inputDependencies.push_back(FSUtil::ResolveRelativePath(dependency, toolContext->workingDirectory()));
    }

    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::External("/usr/bin/touch");
    invocation.arguments() = { "-c", input };
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.outputs() = { output };
    invocation.inputDependencies() = inputDependencies;
    invocation.logMessage() = logMessage;
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

std::unique_ptr<Tool::TouchResolver> Tool::TouchResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Tool::shared_ptr touchTool = specManager->tool(Tool::TouchResolver::ToolIdentifier(), specDomains);
    if (touchTool == nullptr) {
        fprintf(stderr, "warning: could not find touch tool\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::TouchResolver>(new Tool::TouchResolver(touchTool));
}
