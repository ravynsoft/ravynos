/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcformatter/DefaultFormatter.h>
#include <pbxbuild/Tool/Invocation.h>
#include <pbxbuild/Build/Context.h>

#include <algorithm>

using xcformatter::DefaultFormatter;

DefaultFormatter::
DefaultFormatter(bool color) :
    Formatter(),
    _color   (color)
{
}

DefaultFormatter::
~DefaultFormatter()
{
}

#define ANSI_STYLE_BOLD    std::string(_color ? "\033[1m"  : "")
#define ANSI_STYLE_NO_BOLD std::string(_color ? "\033[22m" : "")
#define ANSI_COLOR_RED     std::string(_color ? "\x1b[31m" : "")
#define ANSI_COLOR_GREEN   std::string(_color ? "\x1b[32m" : "")
#define ANSI_COLOR_CYAN    std::string(_color ? "\x1b[36m" : "")
#define ANSI_COLOR_RESET   std::string(_color ? "\x1b[0m"  : "")

#define INDENT std::string("    ")

static std::string
FormatInvocation(pbxbuild::Tool::Invocation const &invocation, bool _color)
{
    std::string message = invocation.logMessage();
    std::string::size_type space = message.find(' ');
    if (space == std::string::npos) {
        space = message.size();
    }

    message = ANSI_STYLE_BOLD + message.substr(0, space) + ANSI_STYLE_NO_BOLD + message.substr(space);
    return message;
}

static std::string
FormatAction(std::string const &action)
{
    std::string result = action;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string DefaultFormatter::
begin(pbxbuild::Build::Context const &buildContext)
{
    std::string result;

    pbxsetting::Environment environment = pbxsetting::Environment();
    for (pbxsetting::Level const &level : buildContext.overrideLevels()) {
        environment.insertFront(level, false);
    }

    std::unordered_map<std::string, std::string> values = environment.computeValues(pbxsetting::Condition::Empty());
    if (!values.empty()) {
        std::map<std::string, std::string> orderedValues = std::map<std::string, std::string>(values.begin(), values.end());

        result += "Build settings from the command line:\n";
        for (auto const &entry : orderedValues) {
            result += INDENT + entry.first + "=" + entry.second + "\n";
        }
        result += "\n";
    }

    return result;
}

std::string DefaultFormatter::
success(pbxbuild::Build::Context const &buildContext)
{
    std::string result;

    result += ANSI_STYLE_BOLD + ANSI_COLOR_GREEN;
    result += "** " + FormatAction(buildContext.action()) + " SUCCEEDED **";
    result += ANSI_STYLE_NO_BOLD + ANSI_COLOR_RESET + "\n";

    return result;
}

std::string DefaultFormatter::
failure(pbxbuild::Build::Context const &buildContext, std::vector<pbxbuild::Tool::Invocation> const &failingInvocations)
{
    std::string result;

    result += ANSI_STYLE_BOLD + ANSI_COLOR_RED;
    result += "** " + FormatAction(buildContext.action()) + " FAILED **";
    result += ANSI_STYLE_NO_BOLD + ANSI_COLOR_RESET + "\n";

    result += "\nThe following build commands failed:\n";
    for (pbxbuild::Tool::Invocation const &invocation : failingInvocations) {
        result += INDENT + FormatInvocation(invocation, _color) + "\n";
    }
    result += "(" + std::to_string(failingInvocations.size()) + " failure" + (failingInvocations.size() != 1 ? "s" : "") + ")\n";

    return result;
}

std::string DefaultFormatter::
beginTarget(pbxbuild::Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target)
{
    std::string result;
    result += ANSI_STYLE_BOLD + ANSI_COLOR_CYAN;
    result += "=== ";

    result += FormatAction(buildContext.action()) + " ";
    result += std::string(target->type() == pbxproj::PBX::Target::Type::Legacy ? "LEGACY TARGET" : "TARGET") + " ";
    result += target->name() + " ";
    result += "OF PROJECT " + target->project()->name() + " ";
    if (buildContext.defaultConfiguration()) {
        result += "WITH THE DEFAULT CONFIGURATION (" + buildContext.configuration() + ")";
    } else {
        result += "WITH CONFIGURATION " + buildContext.configuration();
    }

    result += " ===";
    result += ANSI_STYLE_NO_BOLD + ANSI_COLOR_RESET + "\n";

    result += "\n";
    return result;
}

std::string DefaultFormatter::
finishTarget(pbxbuild::Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target)
{
    return std::string();
}

std::string DefaultFormatter::
beginCheckDependencies(pbxproj::PBX::Target::shared_ptr const &target)
{
    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        return "Check dependencies\n";
    } else {
        return std::string();
    }
}

std::string DefaultFormatter::
finishCheckDependencies(pbxproj::PBX::Target::shared_ptr const &target)
{
    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        return "\n";
    } else {
        return std::string();
    }
}

std::string DefaultFormatter::
beginWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target)
{
    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        return "Write auxiliary files\n";
    } else {
        return std::string();
    }
}

std::string DefaultFormatter::
createAuxiliaryDirectory(std::string const &directory)
{
    return "/bin/mkdir -p " + directory + "\n";
}

std::string DefaultFormatter::
writeAuxiliaryFile(std::string const &file)
{
    return "write-file " + file + "\n";
}

std::string DefaultFormatter::
setAuxiliaryExecutable(std::string const &file)
{
    return "chmod 0755 " + file + "\n";
}

std::string DefaultFormatter::
finishWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target)
{
    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        return "\n";
    } else {
        return std::string();
    }
}

std::string DefaultFormatter::
beginCreateProductStructure(pbxproj::PBX::Target::shared_ptr const &target)
{
    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        // TODO(grp): Implement this fully.
        return "Create product structure\n";
    } else {
        return std::string();
    }
}

std::string DefaultFormatter::
finishCreateProductStructure(pbxproj::PBX::Target::shared_ptr const &target)
{
    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        return "\n";
    } else {
        return std::string();
    }
}

std::string DefaultFormatter::
beginInvocation(pbxbuild::Tool::Invocation const &invocation, std::string const &executable, bool simple)
{
    if (simple) {
        std::string message = executable;
        for (std::string const &arg : invocation.arguments()) {
            message += " " + arg;
        }
        message += "\n";
        return message;
    } else {
        std::string message;

        message += FormatInvocation(invocation, _color) + "\n";
        message += INDENT + "cd " + invocation.workingDirectory() + "\n";

        if (invocation.showEnvironmentInLog()) {
            std::map<std::string, std::string> sortedEnvironment = std::map<std::string, std::string>(invocation.environment().begin(), invocation.environment().end());
            for (std::pair<std::string, std::string> const &entry : sortedEnvironment) {
                message += INDENT + "export " + entry.first + "=" + entry.second + "\n";
            }
        }

        message += INDENT + executable;
        for (std::string const &arg : invocation.arguments()) {
            message += " " + arg;
        }
        message += "\n";

        return message;
    }
}

std::string DefaultFormatter::
finishInvocation(pbxbuild::Tool::Invocation const &invocation, std::string const &executable, bool simple)
{
    if (simple) {
        return std::string();
    } else {
        return "\n";
    }
}

std::shared_ptr<DefaultFormatter> DefaultFormatter::
Create(bool color)
{
    return std::make_shared<DefaultFormatter>(color);
}
