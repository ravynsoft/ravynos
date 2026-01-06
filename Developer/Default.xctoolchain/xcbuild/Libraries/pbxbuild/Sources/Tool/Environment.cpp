/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/Environment.h>
#include <libutil/FSUtil.h>

#include <sstream>

namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;

Tool::Environment::
Environment(pbxspec::PBX::Tool::shared_ptr const &tool, pbxsetting::Environment const &environment, std::vector<std::string> const &inputs, std::vector<std::string> const &outputs) :
    _tool           (tool),
    _environment(environment),
    _inputs         (inputs),
    _outputs        (outputs)
{
}

Tool::Environment::
~Environment()
{
}

std::vector<std::string> Tool::Environment::
inputs(std::string const &workingDirectory) const
{
    std::vector<std::string> inputs;
    for (std::string const &input : _inputs) {
        inputs.push_back(FSUtil::ResolveRelativePath(input, workingDirectory));
    }
    return inputs;
}

std::vector<std::string> Tool::Environment::
outputs(std::string const &workingDirectory) const
{
    std::vector<std::string> outputs;
    for (std::string const &output : _outputs) {
        outputs.push_back(FSUtil::ResolveRelativePath(output, workingDirectory));
    }
    return outputs;
}

static pbxsetting::Level
InputLevel(Tool::Input const &input, std::string const &workingDirectory)
{
    std::string absoluteInputPath = FSUtil::ResolveRelativePath(input.path(), workingDirectory);
    std::string relativeInputPath = FSUtil::GetRelativePath(absoluteInputPath, workingDirectory);

    return pbxsetting::Level({
        pbxsetting::Setting::Create("Input", input.path()),
        pbxsetting::Setting::Create("InputPath", input.path()),
        pbxsetting::Setting::Create("InputFile", input.path()),
        pbxsetting::Setting::Parse("InputFileName", "$(InputFile:file)"),
        pbxsetting::Setting::Parse("InputFileBase", "$(InputFile:base)"),
        pbxsetting::Setting::Parse("InputFileSuffix", "$(InputFile:suffix)"),
        pbxsetting::Setting::Create("InputFileRelativePath", relativeInputPath),
        pbxsetting::Setting::Create("InputFileBaseUniquefier", input.fileNameDisambiguator().value_or("")),
        pbxsetting::Setting::Create("InputFileTextEncoding", ""), // TODO(grp): Text encoding.
    });
}

static pbxsetting::Level
OutputLevel(std::string const &output)
{
    return pbxsetting::Level({
        pbxsetting::Setting::Create("Output", output),
        pbxsetting::Setting::Create("OutputPath", output),
        pbxsetting::Setting::Create("OutputFile", output),
        pbxsetting::Setting::Parse("OutputDir", "$(OutputFile:dir)"),
        pbxsetting::Setting::Parse("OutputFileName", "$(OutputFile:file)"),
        pbxsetting::Setting::Parse("OutputFileBase", "$(OutputFile:base)"),
    });
}

Tool::Environment Tool::Environment::
Create(
    pbxspec::PBX::Tool::shared_ptr const &tool,
    pbxsetting::Environment const &baseEnvironment,
    std::string const &workingDirectory,
    std::vector<Tool::Input> const &inputs,
    std::vector<std::string> const &outputs)
{
    /*
     * Create the settings environment for the tool.
     */
    pbxsetting::Environment environment = pbxsetting::Environment(baseEnvironment);

    /*
     * Add default settings from the tool itself.
     */
    environment.insertFront(tool->defaultSettings(), true);

    /*
     * Determine product & temp resources directories. This varies based on localization;
     * the point is so that "copy" type tools can go into the resources folder even when
     * accidentally inserted into Sources build phases.
     */
    std::string productResourcesDirectory = environment.resolve("TARGET_BUILD_DIR") + "/" + environment.resolve("UNLOCALIZED_RESOURCES_FOLDER_PATH");
    std::string tempResourcesDirectory = environment.resolve("TARGET_TEMP_DIR");
    if (!inputs.empty()) {
        Tool::Input const &input = inputs.front();
        if (input.localization()) {
            std::string localizationPath = *input.localization() + ".lproj";
            productResourcesDirectory += "/" + localizationPath;
            tempResourcesDirectory += "/" + localizationPath;
        }
    }

    /*
     * Tool-level settings applicable to any tool.
     */
    pbxsetting::Level toolLevel = pbxsetting::Level({
        pbxsetting::Setting::Parse("DerivedFilesDir", "DERIVED_FILES_DIR"),
        pbxsetting::Setting::Parse("ObjectsDir", "$(OBJECT_FILE_DIR_$(variant))/$(arch)"),
        pbxsetting::Setting::Create("ProductResourcesDir", productResourcesDirectory),
        pbxsetting::Setting::Create("TempResourcesDir", tempResourcesDirectory),
        // TODO(grp): AdditionalFlags
        // TODO(grp): BitcodeArch
        // TODO(grp): BuiltBinaryPath
        // TODO(grp): CommandProgressByType
        // TODO(grp): ShellScriptName
        // TODO(grp): StaticAnalyzerModeNameDescription
    });
    environment.insertFront(toolLevel, false);

    /*
     * Tool-level settings for compilers.
     */
    if (tool->type() == pbxspec::PBX::Compiler::Type()) {
        pbxspec::PBX::Compiler::shared_ptr const &compiler = std::static_pointer_cast<pbxspec::PBX::Compiler>(tool);

        if (compiler->dependencyInfoFile()) {
            pbxsetting::Level compilerLevel = pbxsetting::Level({
                pbxsetting::Setting::Create("DependencyInfoFile", *compiler->dependencyInfoFile()),
            });
            environment.insertFront(compilerLevel, false);
        }
    }

    /*
     * Determine inputs and outputs for the tool.
     */
    std::vector<std::string> inputPaths;
    std::vector<std::string> outputPaths;

    if (!inputs.empty()) {
        /* There are inputs, add them to the environment. */
        Tool::Input const &input = inputs.front();
        environment.insertFront(InputLevel(input, workingDirectory), false);

        for (Tool::Input const &input : inputs) {
            inputPaths.push_back(input.path());
        }
    }

    if (tool->outputs()) {
        if (!outputs.empty()) {
            /* The tool's outputs may reference the variables from the passed-in outputs. */
            std::string const &output = outputs.front();
            environment.insertFront(OutputLevel(output), false);
        }

        if (!tool->outputs()->empty()) {
            /* The tool does specify outputs; use those. */
            std::string output = environment.expand(tool->outputs()->front());
            for (pbxsetting::Value const &output : *tool->outputs()) {
                std::string outputPath = environment.expand(output);

                if (&output == &tool->outputs()->front()) {
                    /* The first output is added to the environment. */
                    environment.insertFront(OutputLevel(outputPath), false);
                }

                outputPaths.push_back(outputPath);
            }
        }
    } else if (tool->outputPath()) {
        /* The tool specifies a single output. */
        std::string const &output = environment.expand(*tool->outputPath());
        environment.insertFront(OutputLevel(output), false);
        outputPaths.push_back(output);
    } else if (!outputs.empty()) {
        /* The tool doesn't specify outputs, just use the passed-in ones. */
        std::string const &output = outputs.front();
        environment.insertFront(OutputLevel(output), false);
        outputPaths.insert(outputPaths.end(), outputs.begin(), outputs.end());
    }

    return Tool::Environment(tool, environment, inputPaths, outputPaths);
}
