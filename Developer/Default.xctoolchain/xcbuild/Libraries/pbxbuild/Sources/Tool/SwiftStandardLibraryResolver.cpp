/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/SwiftStandardLibraryResolver.h>
#include <pbxbuild/Tool/Environment.h>
#include <pbxbuild/Tool/OptionsResult.h>
#include <pbxbuild/Tool/Tokens.h>
#include <pbxbuild/Tool/Context.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Level.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Type.h>

namespace Tool = pbxbuild::Tool;

Tool::SwiftStandardLibraryResolver::
SwiftStandardLibraryResolver(pbxspec::PBX::Tool::shared_ptr const &tool) :
    _tool(tool)
{
}

void Tool::SwiftStandardLibraryResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &baseEnvironment,
    Tool::Input const &executable,
    std::vector<std::string> const &directories) const
{
    pbxsetting::Level level = pbxsetting::Level({
        pbxsetting::Setting::Create("SWIFT_STDLIB_TOOL_FOLDERS_TO_SCAN", pbxsetting::Type::FormatList(directories)),
    });
    pbxsetting::Environment env = pbxsetting::Environment(baseEnvironment);
    env.insertFront(level, false);

    std::string outputPath = env.resolve("TARGET_BUILD_DIR") + "/" + env.resolve("FULL_PRODUCT_NAME");

    Tool::Environment toolEnvironment = Tool::Environment::Create(_tool, env, toolContext->workingDirectory(), { executable }, { outputPath });
    Tool::OptionsResult options = Tool::OptionsResult::Create(toolEnvironment, toolContext->workingDirectory(), nullptr);
    Tool::Tokens::ToolExpansions tokens = Tool::Tokens::ExpandTool(toolEnvironment, options);

    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::Determine(tokens.executable());
    invocation.arguments() = tokens.arguments();
    invocation.environment() = options.environment();
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.inputs() = toolEnvironment.inputs(toolContext->workingDirectory());
    invocation.outputs() = { }; // TODO(grp): Outputs are not known at build time.
    invocation.logMessage() = tokens.logMessage();
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

std::unique_ptr<Tool::SwiftStandardLibraryResolver> Tool::SwiftStandardLibraryResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Tool::shared_ptr swiftStandardLibraryTool = specManager->tool(Tool::SwiftStandardLibraryResolver::ToolIdentifier(), specDomains);
    if (swiftStandardLibraryTool == nullptr) {
        fprintf(stderr, "warning: could not find swift standard library tool\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::SwiftStandardLibraryResolver>(new Tool::SwiftStandardLibraryResolver(swiftStandardLibraryTool));
}
