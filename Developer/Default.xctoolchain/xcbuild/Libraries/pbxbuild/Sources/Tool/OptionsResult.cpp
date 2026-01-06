/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/OptionsResult.h>
#include <pbxbuild/Tool/SearchPaths.h>
#include <pbxbuild/Tool/Environment.h>
#include <pbxsetting/Type.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/Object.h>
#include <plist/String.h>

namespace Tool = pbxbuild::Tool;

Tool::OptionsResult::
OptionsResult(std::vector<std::string> const &arguments, std::unordered_map<std::string, std::string> const &environment, std::vector<std::string> const &linkerArgs) :
    _arguments  (arguments),
    _environment(environment),
    _linkerArgs (linkerArgs)
{
}

Tool::OptionsResult::
~OptionsResult()
{
}

static bool
EvaluateCondition(std::string const &condition, pbxsetting::Environment const &environment)
{
#define WARN_UNHANDLED_CONDITION 0

    // TODO(grp): Evaluate condition expression language correctly.
    std::string expression = environment.expand(pbxsetting::Value::Parse(condition));

    std::string::size_type eq = expression.find(" == ");
    if (eq != std::string::npos) {
        std::string lhs = expression.substr(0, eq);
        std::string rhs = expression.substr(eq + 4);
#if WARN_UNHANDLED_CONDITION
        fprintf(stderr, "warning: unhandled condition evaluation '%s' == '%s'\n", lhs.c_str(), rhs.c_str());
#endif
        return (lhs == rhs);
    }

    std::string::size_type noteq = expression.find(" != ");
    if (noteq != std::string::npos) {
        std::string lhs = expression.substr(0, noteq);
        std::string rhs = expression.substr(noteq + 4);
#if WARN_UNHANDLED_CONDITION
        fprintf(stderr, "warning: unhandled condition evaluation '%s' != '%s'\n", lhs.c_str(), rhs.c_str());
#endif
        return (lhs != rhs);
    }

#if WARN_UNHANDLED_CONDITION
    fprintf(stderr, "warning: unhandled condition evaluation '%s'\n", expression.c_str());
#endif
    return expression != "NO";
}

static void
AddOptionArgumentValue(std::vector<std::string> *arguments, pbxsetting::Environment const &environment, std::vector<pbxsetting::Value> const &args, std::string const &value)
{
    pbxsetting::Environment argEnvironment = pbxsetting::Environment(environment);
    argEnvironment.insertFront(pbxsetting::Level({
        pbxsetting::Setting::Create("value", value),
    }), false);

    for (pbxsetting::Value const &arg : args) {
        std::string result = argEnvironment.expand(arg);
        arguments->push_back(result);
    }
}

static void
AddOptionArgumentValues(std::vector<std::string> *arguments, pbxsetting::Environment const &environment, std::string const &workingDirectory, std::vector<pbxsetting::Value> const &args, pbxspec::PBX::PropertyOption::shared_ptr const &option)
{
    if ((option->type() == "StringList" || option->type() == "stringlist") ||
        (option->type() == "PathList" || option->type() == "pathlist")) {
        std::vector<std::string> values = pbxsetting::Type::ParseList(environment.resolve(option->name()));
        if (option->flattenRecursiveSearchPathsInValue()) {
            values = Tool::SearchPaths::ExpandRecursive(values, environment, workingDirectory);
        }

        for (std::string const &value : values) {
            AddOptionArgumentValue(arguments, environment, args, value);
        }
    } else {
        std::string value = environment.resolve(option->name());
        AddOptionArgumentValue(arguments, environment, args, value);
    }
}

static std::vector<pbxsetting::Value>
ArgumentValuesFromArray(plist::Array const *args)
{
    std::vector<pbxsetting::Value> values;
    for (size_t n = 0; n < args->count(); n++) {
        if (auto arg = args->value <plist::String> (n)) {
            values.push_back(pbxsetting::Value::Parse(arg->value()));
        }
    }
    return values;
}

static void
AddOptionValuesArguments(std::vector<std::string> *arguments, pbxsetting::Environment const &environment, std::string const &workingDirectory, plist::Array const *values, std::string const &value, pbxspec::PBX::PropertyOption::shared_ptr const &option)
{
    if (values == nullptr) {
        return;
    }

    /*
     * `Values` and `AllowedValues` are arrays of value dictoinaries. Each value has a key `Value`
     * with the expected value itself and `CommandLineFlag` / `CommandLineArguments` to add for it.
     */

    for (size_t n = 0; n < values->count(); n++) {
        if (auto entry = values->value <plist::Dictionary> (n)) {
            if (auto entryValue = entry->value <plist::String> ("Value")) {
                if (entryValue->value() == value) {
                    if (auto entryFlag = entry->value <plist::String> ("CommandLineFlag")) {
                        std::vector<pbxsetting::Value> argsValues = { pbxsetting::Value::Parse(entryFlag->value()) };
                        AddOptionArgumentValues(arguments, environment, workingDirectory, argsValues, option);
                    } else if (auto entryArgs = entry->value <plist::Array> ("CommandLineArgs")) {
                        std::vector<pbxsetting::Value> argsValues = ArgumentValuesFromArray(entryArgs);
                        AddOptionArgumentValues(arguments, environment, workingDirectory, argsValues, option);
                    }
                }
            }
        }
    }
}

static void
AddOptionArgsArguments(std::vector<std::string> *arguments, pbxsetting::Environment const &environment, std::string const &workingDirectory, plist::Object const *argsValue, std::string const &value, pbxspec::PBX::PropertyOption::shared_ptr const &option)
{
    /*
     * `CommandLineArgs` and `AdditionalLinkerArgs` are either arrays of arguments or dictionaries
     * mapping values to arrays of arguments. The key `<<otherwise>>` is special-cased as a fallback.
     */

    if (auto args = plist::CastTo <plist::Array> (argsValue)) {
        std::vector<pbxsetting::Value> argsValues = ArgumentValuesFromArray(args);
        AddOptionArgumentValues(arguments, environment, workingDirectory, argsValues, option);
    } else if (auto argsValues = plist::CastTo <plist::Dictionary> (argsValue)) {
        if (auto args = argsValues->value <plist::Array> (value)) {
            std::vector<pbxsetting::Value> argsValues = ArgumentValuesFromArray(args);
            AddOptionArgumentValues(arguments, environment, workingDirectory, argsValues, option);
        } else if (auto args = argsValues->value <plist::Array> ("<<otherwise>>")) {
            std::vector<pbxsetting::Value> argsValues = ArgumentValuesFromArray(args);
            AddOptionArgumentValues(arguments, environment, workingDirectory, argsValues, option);
        }
    }
}

Tool::OptionsResult Tool::OptionsResult::
Create(
    pbxsetting::Environment const &environment,
    std::string const &workingDirectory,
    std::vector<pbxspec::PBX::PropertyOption::shared_ptr> const &options,
    pbxspec::PBX::FileType::shared_ptr const &fileType,
    std::unordered_set<std::string> const &deletedSettings)
{
    std::vector<std::string> arguments;
    std::unordered_map<std::string, std::string> environmentVariables;
    std::vector<std::string> linkerArgs;

    std::string architecture = environment.resolve("arch");

    for (pbxspec::PBX::PropertyOption::shared_ptr const &option : options) {
        if (deletedSettings.find(option->name()) != deletedSettings.end()) {
            continue;
        }

        if (option->condition() && !EvaluateCondition(*option->condition(), environment)) {
            continue;
        }
        if (option->commandLineCondition() && !EvaluateCondition(*option->commandLineCondition(), environment)) {
            continue;
        }

        if (option->architectures() && std::find(option->architectures()->begin(), option->architectures()->end(), architecture) == option->architectures()->end()) {
            continue;
        }

        if (option->fileTypes() && fileType != nullptr && std::find(option->fileTypes()->begin(), option->fileTypes()->end(), fileType->identifier()) == option->fileTypes()->end()) {
            continue;
        }

        // TODO(grp): Use PropertyOption::conditionFlavors().
        std::string value = environment.resolve(option->name());

        if (option->type() == "Boolean" || option->type() == "bool") {
            bool booleanValue = pbxsetting::Type::ParseBoolean(value);
            ext::optional<pbxsetting::Value> const &flag = (booleanValue ? option->commandLineFlag() : option->commandLineFlagIfFalse());

            if (flag) {
                /* Boolean flags don't get the flag value after, since that would be just YES or NO. */
                arguments.push_back(environment.expand(*flag));
            }
        } else {
            if (!value.empty()) {
                if (option->commandLineFlag()) {
                    pbxsetting::Value const &flag = *option->commandLineFlag();

                    /* Pass both the command line flag and the option value itself. */
                    std::vector<pbxsetting::Value> values = { flag, pbxsetting::Value::Variable("value") };
                    AddOptionArgumentValues(&arguments, environment, workingDirectory, values, option);
                }
            }
        }

        AddOptionValuesArguments(&arguments, environment, workingDirectory, plist::CastTo<plist::Array>(option->values()), value, option);
        AddOptionValuesArguments(&arguments, environment, workingDirectory, plist::CastTo<plist::Array>(option->allowedValues()), value, option);

        if (!value.empty()) {
            /* Pass the prefix then the option value in the same argument. */
            if (option->commandLinePrefixFlag()) {
                pbxsetting::Value const &prefix = *option->commandLinePrefixFlag();
                pbxsetting::Value prefixValue = prefix + pbxsetting::Value::Variable("value");
                AddOptionArgumentValues(&arguments, environment, workingDirectory, { prefixValue }, option);
            }
        }

        AddOptionArgsArguments(&arguments, environment, workingDirectory, option->commandLineArgs(), value, option);
        AddOptionArgsArguments(&linkerArgs, environment, workingDirectory, option->additionalLinkerArgs(), value, option);

        if (option->setValueInEnvironmentVariable()) {
            std::string const &variable = environment.expand(*option->setValueInEnvironmentVariable());
            environmentVariables.insert({ variable, value });
        }

        // TODO(grp): Use PropertyOption::conditionFlavors().
        // TODO(grp): Use PropertyOption::isCommand{Input,Output}().
        // TODO(grp): Use PropertyOption::isInputDependency(), PropertyOption::outputDependencies().
        // TODO(grp): Use PropertyOption::outputsAreSourceFiles().
    }

    return Tool::OptionsResult(arguments, environmentVariables, linkerArgs);
}

Tool::OptionsResult Tool::OptionsResult::
Create(
    Tool::Environment const &toolEnvironment,
    std::string const &workingDirectory,
    pbxspec::PBX::FileType::shared_ptr const &fileType)
{
    Tool::OptionsResult optionsResult = Create(
        toolEnvironment.environment(),
        workingDirectory,
        toolEnvironment.tool()->options().value_or(pbxspec::PBX::PropertyOption::vector()),
        fileType,
        toolEnvironment.tool()->deletedProperties().value_or(std::unordered_set<std::string>()));

    /* Add tool-level environment variables. */
    std::unordered_map<std::string, std::string> environmentVariables = optionsResult.environment();
    if (toolEnvironment.tool()->environmentVariables()) {
        for (auto const &variable : *toolEnvironment.tool()->environmentVariables()) {
            environmentVariables.insert({ variable.first, toolEnvironment.environment().expand(variable.second) });
        }
    }

    /* Copy important environment variables for all tools. */
    environmentVariables["PATH"] = toolEnvironment.environment().resolve("PATH");
    environmentVariables["DEVELOPER_DIR"] = toolEnvironment.environment().resolve("DEVELOPER_DIR");

    return Tool::OptionsResult(optionsResult.arguments(), environmentVariables, optionsResult.linkerArgs());
}
