/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/ScriptResolver.h>
#include <pbxbuild/Tool/Context.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Level.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Type.h>
#include <libutil/Escape.h>
#include <libutil/FSUtil.h>

#include <algorithm>
#include <iterator>

namespace Tool = pbxbuild::Tool;
using libutil::Escape;
using libutil::FSUtil;

Tool::ScriptResolver::
ScriptResolver(pbxspec::PBX::Tool::shared_ptr const &tool) :
    _tool(tool)
{
}

static pbxsetting::Level
ScriptInputOutputLevel(std::vector<std::string> const &inputFiles, std::vector<std::string> const &outputFiles, bool multipleInputs)
{
    std::vector<pbxsetting::Setting> settings;

    settings.push_back(pbxsetting::Setting::Create("SCRIPT_OUTPUT_FILE_COUNT", pbxsetting::Type::FormatInteger(outputFiles.size())));
    for (auto it = outputFiles.begin(); it < outputFiles.end(); ++it) {
        size_t index = (it - outputFiles.begin());
        settings.push_back(pbxsetting::Setting::Create("SCRIPT_OUTPUT_FILE_" + pbxsetting::Type::FormatInteger(index), *it));
    }

    if (multipleInputs) {
        settings.push_back(pbxsetting::Setting::Create("SCRIPT_INPUT_FILE_COUNT", pbxsetting::Type::FormatInteger(inputFiles.size())));
        for (auto it = inputFiles.begin(); it < inputFiles.end(); ++it) {
            size_t index = (it - inputFiles.begin());
            settings.push_back(pbxsetting::Setting::Create("SCRIPT_INPUT_FILE_" + pbxsetting::Type::FormatInteger(index), *it));
        }
    } else if (!inputFiles.empty()) {
        settings.push_back(pbxsetting::Setting::Create("SCRIPT_INPUT_FILE", inputFiles.front()));
    }

    return pbxsetting::Level(settings);
}

void Tool::ScriptResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &environment,
    pbxproj::PBX::LegacyTarget::shared_ptr const &legacyTarget) const
{
    std::string logMessage = "ExternalBuildToolExecution " + legacyTarget->name();

    std::string script = environment.expand(legacyTarget->buildArgumentsString());

    std::unordered_map<std::string, std::string> environmentVariables;
    if (legacyTarget->passBuildSettingsInEnvironment()) {
        environmentVariables = environment.computeValues(pbxsetting::Condition::Empty());
    }

    std::string fullWorkingDirectory = FSUtil::ResolveRelativePath(legacyTarget->buildWorkingDirectory(), toolContext->workingDirectory());

    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::Determine(legacyTarget->buildToolPath());
    invocation.arguments() = pbxsetting::Type::ParseList(script);
    invocation.environment() = environmentVariables;
    invocation.workingDirectory() = fullWorkingDirectory;
    invocation.logMessage() = logMessage;
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

void Tool::ScriptResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &environment,
    pbxproj::PBX::ShellScriptBuildPhase::shared_ptr const &buildPhase) const
{
    pbxsetting::Level level = pbxsetting::Level({
        pbxsetting::Setting::Create("BuildPhaseName", (!buildPhase->name().empty() ? buildPhase->name() : "Run Script")),
        pbxsetting::Setting::Create("BuildPhaseIdentifier", buildPhase->blueprintIdentifier()),
    });

    pbxsetting::Environment phaseEnvironment = pbxsetting::Environment(environment);
    phaseEnvironment.insertFront(level, false);

    pbxsetting::Value scriptPath = pbxsetting::Value::Parse("$(TEMP_FILES_DIR)/Script-$(BuildPhaseIdentifier).sh");
    pbxsetting::Value logMessage = pbxsetting::Value::Parse("PhaseScriptExecution $(BuildPhaseName:quote) ") + scriptPath;

    std::vector<std::string> inputFiles;
    std::transform(buildPhase->inputPaths().begin(), buildPhase->inputPaths().end(), std::back_inserter(inputFiles), [&](pbxsetting::Value const &input) -> std::string {
        std::string path = environment.expand(input);
        return FSUtil::ResolveRelativePath(path, toolContext->workingDirectory());
    });

    std::vector<std::string> outputFiles;
    std::transform(buildPhase->outputPaths().begin(), buildPhase->outputPaths().end(), std::back_inserter(outputFiles), [&](pbxsetting::Value const &output) -> std::string {
        std::string path = environment.expand(output);
        return FSUtil::ResolveRelativePath(path, toolContext->workingDirectory());
    });

    std::string scriptFilePath = phaseEnvironment.expand(scriptPath);
    std::string contents = (!buildPhase->shellPath().empty() ? "#!" + buildPhase->shellPath() + "\n" : "") + buildPhase->shellScript();
    auto scriptFile = Tool::AuxiliaryFile::Data(scriptFilePath, std::vector<uint8_t>(contents.begin(), contents.end()), true);

    pbxsetting::Environment scriptEnvironment = pbxsetting::Environment(environment);
    scriptEnvironment.insertFront(ScriptInputOutputLevel(inputFiles, outputFiles, true), false);
    std::unordered_map<std::string, std::string> environmentVariables = scriptEnvironment.computeValues(pbxsetting::Condition::Empty());

#if 0
    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::External("/bin/sh");
    invocation.arguments() = { "-c", "sed -ie 's/\\/bin\\/ln/bsdln/g' " + Escape::Shell(scriptFilePath) + "; " + Escape::Shell(scriptFilePath) };
    invocation.environment() = environmentVariables;
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.phonyInputs() = inputFiles; /* User-specified, may not exist. */
    invocation.outputs() = outputFiles;
    invocation.logMessage() = phaseEnvironment.expand(logMessage);
    invocation.showEnvironmentInLog() = buildPhase->showEnvVarsInLog();
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);

    toolContext->auxiliaryFiles().push_back(scriptFile);
#endif /* 0 */
}

void Tool::ScriptResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &environment,
    Tool::Input const &input) const
{
    Target::BuildRules::BuildRule::shared_ptr const &buildRule = input.buildRule();
    if (buildRule == nullptr || buildRule->script().empty()) {
        fprintf(stderr, "warning: invalid or missing build rule for script\n");
        return;
    }

    std::string inputAbsolutePath = FSUtil::ResolveRelativePath(input.path(), toolContext->workingDirectory());
    std::string inputRelativePath = FSUtil::GetRelativePath(inputAbsolutePath, toolContext->workingDirectory());

    pbxsetting::Value logMessage = pbxsetting::Value::Parse("RuleScriptExecution " + inputRelativePath + " $(variant) $(arch)");

    /*
     * Add the public input file build settings. These can be used within the
     * script or inside the list of output paths.
     */
    pbxsetting::Level level = pbxsetting::Level({
        pbxsetting::Setting::Create("INPUT_FILE_PATH", inputAbsolutePath),
        pbxsetting::Setting::Parse("INPUT_FILE_DIR", "$(INPUT_FILE_PATH:dir)"),
        pbxsetting::Setting::Parse("INPUT_FILE_NAME", "$(INPUT_FILE_PATH:file)"),
        pbxsetting::Setting::Parse("INPUT_FILE_BASE", "$(INPUT_FILE_PATH:base)"),
        pbxsetting::Setting::Parse("INPUT_FILE_SUFFIX", "$(INPUT_FILE_PATH:suffix)"),
        pbxsetting::Setting::Create("INPUT_FILE_REGION_PATH_COMPONENT", input.localization().value_or("")), // TODO(grp): Verify format of this.
    });

    pbxsetting::Environment ruleEnvironment = pbxsetting::Environment(environment);
    ruleEnvironment.insertFront(level, false);

    /*
     * Only resolve the output paths once the input paths settings are in the
     * environment. The output paths can use the input path settings.
     */
    std::vector<std::string> outputFiles;
    std::transform(buildRule->outputFiles().begin(), buildRule->outputFiles().end(), std::back_inserter(outputFiles), [&](pbxsetting::Value const &output) -> std::string {
        std::string path = ruleEnvironment.expand(output);
        return FSUtil::ResolveRelativePath(path, toolContext->workingDirectory());
    });

    /*
     * Compute the final environment by adding the standard script levels.
     */
    ruleEnvironment.insertFront(ScriptInputOutputLevel({ inputAbsolutePath }, outputFiles, false), false);
    std::unordered_map<std::string, std::string> environmentVariables = ruleEnvironment.computeValues(pbxsetting::Condition::Empty());

    Tool::Invocation invocation;
    invocation.executable() = Tool::Invocation::Executable::External("/bin/sh");
    invocation.arguments() = { "-c", buildRule->script() };
    invocation.environment() = environmentVariables;
    invocation.workingDirectory() = toolContext->workingDirectory();
    invocation.inputs() = { inputAbsolutePath };
    invocation.outputs() = outputFiles;
    invocation.logMessage() = ruleEnvironment.expand(logMessage);
    invocation.showEnvironmentInLog() = true;
    invocation.priority() = toolContext->currentPhaseInvocationPriority();
    toolContext->invocations().push_back(invocation);
}

std::unique_ptr<Tool::ScriptResolver> Tool::ScriptResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains)
{
    pbxspec::PBX::Tool::shared_ptr scriptTool = specManager->tool(ScriptResolver::ToolIdentifier(), specDomains);
    if (scriptTool == nullptr) {
        fprintf(stderr, "warning: could not find shell script tool\n");
        return nullptr;
    }

    return std::unique_ptr<ScriptResolver>(new ScriptResolver(scriptTool));
}

