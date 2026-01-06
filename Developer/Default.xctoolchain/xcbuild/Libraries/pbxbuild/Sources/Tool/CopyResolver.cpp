/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/CopyResolver.h>
#include <pbxbuild/Tool/Environment.h>
#include <pbxbuild/Tool/OptionsResult.h>
#include <pbxbuild/Tool/Tokens.h>
#include <pbxbuild/Tool/Context.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

#include <algorithm>
#include <iterator>

namespace Tool = pbxbuild::Tool;
using libutil::Filesystem;
using libutil::FSUtil;

Tool::CopyResolver::
CopyResolver(pbxspec::PBX::Tool::shared_ptr const &tool) :
    _tool(tool)
{
}

void Tool::CopyResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &baseEnvironment,
    std::vector<Tool::Input> const &inputs,
    std::string const &outputDirectory,
    std::string const &logMessageTitle) const
{
    /*
     * Add the copy-specific build settings.
     */
    pbxsetting::Level copyLevel = pbxsetting::Level({
        // TODO: pbxsetting::Setting::Create("PBXCP_STRIP_TOOL", toolchain->path() + "/usr/bin/strip"),
        // TODO: pbxsetting::Setting::Create("PBXCP_BITCODE_STRIP_TOOL", toolchain->path() + "/usr/bin/bitcode_strip"),
        pbxsetting::Setting::Create("pbxcp_rule_name", logMessageTitle),
    });

    pbxsetting::Environment environment = pbxsetting::Environment(baseEnvironment);
    environment.insertFront(copyLevel, false);

    /*
     * The output path is the same as the input, but in the output directory.
     */
    std::vector<std::string> outputPaths;
    for (Tool::Input const &input : inputs) {
        std::string outputPath = outputDirectory + "/" + FSUtil::GetBaseName(input.path());
        outputPaths.push_back(outputPath);
    }

    /*
     * Build the arguments: each input file, then the output directory.
     */
    std::vector<std::string> args;
    for (Tool::Input const &input : inputs) {
        args.push_back(input.path());
    }
    args.push_back(outputDirectory);

    /*
     * Resolve the tool options. Inputs can either be full build files or just paths.
     */
    Tool::Environment toolEnvironment = Tool::Environment::Create(_tool, environment, toolContext->workingDirectory(), inputs, outputPaths);
    Tool::OptionsResult options = Tool::OptionsResult::Create(toolEnvironment, toolContext->workingDirectory(), nullptr);
    Tool::Tokens::ToolExpansions tokens = Tool::Tokens::ExpandTool(toolEnvironment, options, std::string(), args);

    // TODO(grp): This should be generic for all tools.
    std::vector<Tool::Invocation::DependencyInfo> dependencyInfo;
    if (_tool->deeplyStatInputDirectories()) {
        for (Tool::Input const &input : inputs) {
            /* Create a dependency info file to track the input directory contents. */
            auto info = Tool::Invocation::DependencyInfo(dependency::DependencyInfoFormat::Directory, input.path());
            dependencyInfo.push_back(info);
        }
    }

    /*
     * Create the copy invocation.
     */
    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::Determine(tokens.executable());
    invocation.arguments() = tokens.arguments();
    invocation.environment() = options.environment();
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.inputs() = toolEnvironment.inputs(toolContext->workingDirectory());
    invocation.outputs() = toolEnvironment.outputs(toolContext->workingDirectory());
    invocation.dependencyInfo() = dependencyInfo;
    invocation.logMessage() = tokens.logMessage();
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

std::unique_ptr<Tool::CopyResolver> Tool::CopyResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Tool::shared_ptr copyTool = specManager->tool(Tool::CopyResolver::ToolIdentifier(), specDomains);
    if (copyTool == nullptr) {
        fprintf(stderr, "warning: could not find copy tool\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::CopyResolver>(new Tool::CopyResolver(copyTool));
}

