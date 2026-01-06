/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/InfoPlistResolver.h>
#include <pbxbuild/Tool/Environment.h>
#include <pbxbuild/Tool/OptionsResult.h>
#include <pbxbuild/Tool/Tokens.h>
#include <pbxbuild/Tool/Context.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Level.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Type.h>

namespace Tool = pbxbuild::Tool;

Tool::InfoPlistResolver::
InfoPlistResolver(pbxspec::PBX::Tool::shared_ptr const &tool) :
    _tool(tool)
{
}

Tool::InfoPlistResolver::
~InfoPlistResolver()
{
}

void Tool::InfoPlistResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &environment,
    Tool::Input const &input) const
{
    bool pkginfoFile = pbxsetting::Type::ParseBoolean(environment.resolve("GENERATE_PKGINFO_FILE"));

    pbxsetting::Level level = pbxsetting::Level({
        pbxsetting::Setting::Parse("GeneratedPkgInfoFile", (pkginfoFile ? "$(TARGET_BUILD_DIR)/$(PKGINFO_PATH)" : "")),
        pbxsetting::Setting::Parse("ExpandBuildSettings", "$(INFOPLIST_EXPAND_BUILD_SETTINGS)"),
        pbxsetting::Setting::Parse("OutputFormat", "$(INFOPLIST_OUTPUT_FORMAT)"),
        pbxsetting::Setting::Create("AdditionalContentFilePaths", pbxsetting::Type::FormatList(toolContext->additionalInfoPlistContents())),
        pbxsetting::Setting::Create("RequiredArchitectures", ""), // TODO(grp): Determine what this is for.
        pbxsetting::Setting::Create("AdditionalInfoFileKeys", ""), // TODO(grp): Determine what these are for.
        pbxsetting::Setting::Create("AdditionalInfoFileValues", ""), // TODO(grp): Determine what these are for.
    });

    pbxsetting::Environment env = pbxsetting::Environment(environment);
    env.insertFront(level, false);

    std::string infoPlistPath = environment.resolve("TARGET_BUILD_DIR") + "/" + environment.resolve("INFOPLIST_PATH");

    Tool::Environment toolEnvironment = Tool::Environment::Create(_tool, env, toolContext->workingDirectory(), { input }, { infoPlistPath });
    Tool::OptionsResult options = Tool::OptionsResult::Create(toolEnvironment, toolContext->workingDirectory(), nullptr);
    Tool::Tokens::ToolExpansions tokens = Tool::Tokens::ExpandTool(toolEnvironment, options);

    /* Pass all build settings for expansion. */
    std::unordered_map<std::string, std::string> environmentVariables = options.environment();
    std::unordered_map<std::string, std::string> buildSettingValues = environment.computeValues(pbxsetting::Condition::Empty());
    environmentVariables.insert(buildSettingValues.begin(), buildSettingValues.end());

    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::Determine(tokens.executable());
    invocation.arguments() = tokens.arguments();
    invocation.environment() = environmentVariables;
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.inputs() = toolEnvironment.inputs(toolContext->workingDirectory());
    invocation.outputs() = toolEnvironment.outputs(toolContext->workingDirectory());
    invocation.inputDependencies() = toolContext->additionalInfoPlistContents();
    invocation.logMessage() = tokens.logMessage();
    invocation.showEnvironmentInLog() = false; /* Hide build settings from log. */
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

std::unique_ptr<Tool::InfoPlistResolver> Tool::InfoPlistResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Tool::shared_ptr infoPlistTool = specManager->tool(Tool::InfoPlistResolver::ToolIdentifier(), specDomains);
    if (infoPlistTool == nullptr) {
        fprintf(stderr, "warning: could not find info plist tool\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::InfoPlistResolver>(new Tool::InfoPlistResolver(infoPlistTool));
}
