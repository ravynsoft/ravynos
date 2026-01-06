/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/Configuration.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <libutil/Filesystem.h>
#include <process/Context.h>
#include <process/User.h>

using xcsdk::Configuration;
using libutil::Filesystem;

Configuration::
Configuration(
    std::vector<std::string> const &extraPlatformsPaths,
    std::vector<std::string> const &extraToolchainsPaths) :
    _extraPlatformsPaths (extraPlatformsPaths),
    _extraToolchainsPaths(extraToolchainsPaths)
{
}

std::vector<std::string> Configuration::
DefaultPaths(process::User const *user, process::Context const *processContext)
{
    std::vector<std::string> defaultPaths;
    if (ext::optional<std::string> environmentPath = processContext->environmentVariable("XCSDK_CONFIGURATION_PATH")) {
        defaultPaths.push_back(*environmentPath);
    } else {
        ext::optional<std::string> homePath = user->userHomeDirectory();
        if (homePath) {
            defaultPaths.push_back(*homePath + "/.xcsdk/xcsdk_configuration.plist");
        }
        defaultPaths.push_back("/var/db/xcsdk_configuration.plist");
    }
    return defaultPaths;
}

static std::vector<std::string>
LoadArray(plist::Array const *array)
{
    std::vector<std::string> values;

    if (array != nullptr) {
        for (size_t n = 0; n < array->count(); n++) {
            if (auto value = array->value<plist::String>(n)) {
                values.push_back(value->value());
            }
        }
    }

    return values;
}

ext::optional<Configuration> Configuration::
Load(Filesystem const *filesystem, std::vector<std::string> const &paths)
{
    std::vector<uint8_t> contents;
    for (std::string const &path : paths) {
        if (filesystem->read(&contents, path)) {
            break;
        }
    }

    if (contents.size() == 0) {
        return ext::nullopt;
    }

    auto result = plist::Format::Any::Deserialize(contents);
    if (result.first == nullptr) {
        return ext::nullopt;
    }

    plist::Dictionary const *root = plist::CastTo<plist::Dictionary>(result.first.get());
    if (root == nullptr) {
        return ext::nullopt;
    }

    std::vector<std::string> platforms = LoadArray(root->value<plist::Array>("ExtraPlatformsPaths"));
    std::vector<std::string> toolchains = LoadArray(root->value<plist::Array>("ExtraToolchainsPaths"));
    return Configuration(platforms, toolchains);
}
