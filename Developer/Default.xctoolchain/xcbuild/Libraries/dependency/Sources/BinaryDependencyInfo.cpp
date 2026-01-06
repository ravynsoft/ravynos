/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <dependency/BinaryDependencyInfo.h>
#include <dependency/DependencyInfo.h>

#include <algorithm>

using dependency::BinaryDependencyInfo;
using dependency::DependencyInfo;

BinaryDependencyInfo::
BinaryDependencyInfo()
{
}

enum class BinaryDependencyCommand : uint8_t {
    Version = 0x00,
    Input = 0x10,
    Missing = 0x11,
    Output = 0x40,
    // ??? = 0x02,
};

static void
WriteCommand(std::vector<uint8_t> *result, BinaryDependencyCommand command, std::string const &string)
{
    result->push_back(static_cast<uint8_t>(command));
    for (char c : string) {
        result->push_back(static_cast<uint8_t>(c));
    }
    result->push_back('\0');
}

std::vector<uint8_t> BinaryDependencyInfo::
serialize() const
{
    std::vector<uint8_t> result;

    /* Write version. */
    if (!_version.empty()) {
        WriteCommand(&result, BinaryDependencyCommand::Version, _version);
    }

    /* Write output. */
    for (std::string const &output : _dependencyInfo.outputs()) {
        WriteCommand(&result, BinaryDependencyCommand::Output, output);
    }

    /* Write input. */
    for (std::string const &input : _dependencyInfo.inputs()) {
        WriteCommand(&result, BinaryDependencyCommand::Input, input);
    }

    /* Write missing. */
    for (std::string const &missing : _missing) {
        WriteCommand(&result, BinaryDependencyCommand::Missing, missing);
    }

    return result;
}

ext::optional<BinaryDependencyInfo> BinaryDependencyInfo::
Deserialize(std::vector<uint8_t> const &contents)
{
    std::string version;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<std::string> missing;

    for (auto it = contents.begin(); it != contents.end(); ++it) {
        /* Read command. */
        BinaryDependencyCommand command = static_cast<BinaryDependencyCommand>(*it);
        if (++it == contents.end()) {
            /* No string after command. */
            return ext::nullopt;
        }

        /* Find end of string. */
        auto end = std::find(it, contents.end(), '\0');
        if (end == contents.end()) {
            /* Unterminated string. */
            return ext::nullopt;
        }

        /* Read string. */
        std::string string = std::string(it, end);

        if (command == BinaryDependencyCommand::Version) {
            if (!version.empty()) {
                /* Multiple version commands. */
                return ext::nullopt;
            }

            version = string;
        } else if (command == BinaryDependencyCommand::Input) {
            inputs.push_back(string);
        } else if (command == BinaryDependencyCommand::Output) {
            outputs.push_back(string);
        } else if (command == BinaryDependencyCommand::Missing) {
            missing.push_back(string);
        } else {
            /* Unknown command. */
            return ext::nullopt;
        }

        /* Move onto the next entry. */
        it = end;
    }

    /* Create dependency info. */
    BinaryDependencyInfo binaryInfo;
    binaryInfo.version() = version;
    binaryInfo.missing() = missing;
    binaryInfo.dependencyInfo().inputs() = inputs;
    binaryInfo.dependencyInfo().outputs() = outputs;
    return BinaryDependencyInfo(binaryInfo);
}
