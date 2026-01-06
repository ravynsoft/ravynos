/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/Tokens.h>
#include <pbxbuild/Tool/OptionsResult.h>
#include <pbxbuild/Tool/Environment.h>

#include <algorithm>
#include <iterator>
#include <sstream>

namespace Tool = pbxbuild::Tool;

std::vector<std::string> Tool::Tokens::
Expand(
    std::string const &tokenized,
    std::string const &executable,
    std::vector<std::string> const &options,
    std::vector<std::string> const &specialArgs,
    std::vector<std::string> const &inputs,
    std::vector<std::string> const &outputs)
{
    /*
     * Split the tokens on spaces.
     */
    std::vector<std::string> tokens;
    std::stringstream sstream(tokenized);
    std::copy(std::istream_iterator<std::string>(sstream), std::istream_iterator<std::string>(), std::back_inserter(tokens));

    /*
     * The tokens that are supported for expansion.
     */
    std::string const &input = (!inputs.empty() ? inputs.front() : "");
    std::string const &output = (!outputs.empty() ? outputs.front() : "");
    std::unordered_map<std::string, std::vector<std::string>> tokenValues = {
        { "input", { input } },
        { "output", { output } },
        { "inputs", inputs },
        { "outputs", outputs },
        { "options", options },
        { "exec-path", { executable } },
        { "special-args", specialArgs },
    };

    std::vector<std::string> result;
    for (std::string const &entry : tokens) {
        /* If the entry is a token, replace it with the token's value. */
        if (entry.find('[') == 0 && entry.find(']') == entry.size() - 1) {
            std::string token = entry.substr(1, entry.size() - 2);
            auto it = tokenValues.find(token);
            if (it != tokenValues.end()) {
                /* A token can have multiple values; append them all. */
                result.insert(result.end(), it->second.begin(), it->second.end());
                continue;
            }
        }

        /* If not, just add the value literally. */
        result.push_back(entry);
    }

    return result;
}

Tool::Tokens::ToolExpansions::
ToolExpansions(std::string const &executable, std::vector<std::string> const &arguments, std::string const &logMessage) :
    _executable(executable),
    _arguments (arguments),
    _logMessage(logMessage)
{
}

Tool::Tokens::ToolExpansions::
~ToolExpansions()
{
}


Tool::Tokens::ToolExpansions Tool::Tokens::
ExpandTool(
    Tool::Environment const &toolEnvironment,
    Tool::OptionsResult const &options,
    std::string const &executable,
    std::vector<std::string> const &specialArguments)
{
    pbxspec::PBX::Tool::shared_ptr const &tool = toolEnvironment.tool();
    pbxsetting::Environment const &environment = toolEnvironment.environment();

    std::vector<std::string> const &arguments = options.arguments();
    std::vector<std::string> const &inputs = toolEnvironment.inputs();
    std::vector<std::string> const &outputs = toolEnvironment.outputs();

    /*
     * Determine the command line template to use. Default to one that covers most cases.
     * Don't expand the command-line here because then it would be split incorrectly (on
     * spaces) when expanding the tokens. Instead, expand each token individually below.
     */
    std::string const &resolvedCommandLine = (tool->commandLine() ? tool->commandLine()->raw() : "[exec-path] [options] [special-args]");

    /*
     * Determine the executable to use. Use the override if available, otherwise the tool.
     */
    std::string toolExecutable = environment.expand(tool->execPath().value_or(pbxsetting::Value::Empty()));
    std::string const &resolvedExecutable = (!executable.empty() ? executable : toolExecutable);

    /*
     * Expand the command line, then expand the settings within it. Can't expand the command
     * line's settings first or, if the values contain spaces, they would be split incorrectly.
     */
    std::vector<std::string> expandedCommandLine = Expand(resolvedCommandLine, resolvedExecutable, arguments, specialArguments, inputs, outputs);
    for (auto &it : expandedCommandLine) {
        it = environment.expand(pbxsetting::Value::Parse(it));
    }

    /*
     * Extract the executable / arguments.
     */
    std::string const &expandedExecutable = (!expandedCommandLine.empty() ? expandedCommandLine.front() : "");
    std::vector<std::string> expandedArguments = std::vector<std::string>(expandedCommandLine.begin() + (!expandedCommandLine.empty() ? 1 : 0), expandedCommandLine.end());

    /*
     * Determine the input for the log message.
     */
    std::string const &resolvedName = (tool->ruleName() ? environment.expand(*tool->ruleName()) : tool->ruleFormat() ? environment.expand(*tool->ruleFormat()) : std::string());

    /*
     * Expand the log message. In this case the setting expansion is less important as the log
     * message is recombined into a string below anyway.
     */
    std::vector<std::string> expandedName = Expand(resolvedName, resolvedExecutable, arguments, specialArguments, inputs, outputs);

    /*
     * Re-combine the result.
     */
    std::string expandedLogMessage;
    for (std::string const &component : expandedName) {
        if (&component != &expandedName.front()) {
            expandedLogMessage += " ";
        }
        expandedLogMessage += component;
    }

    return Tool::Tokens::ToolExpansions(expandedExecutable, expandedArguments, expandedLogMessage);
}

