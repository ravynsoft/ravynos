/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/Action.h>
#include <xcdriver/Options.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>

using xcdriver::Action;
using xcdriver::Options;
using libutil::Filesystem;
using libutil::FSUtil;

Action::
Action()
{
}

Action::
~Action()
{
}

std::vector<pbxsetting::Level> Action::
CreateOverrideLevels(process::Context const *processContext, Filesystem const *filesystem, pbxsetting::Environment const &environment, Options const &options, std::string const &workingDirectory)
{
    std::vector<pbxsetting::Level> levels;

    std::vector<pbxsetting::Setting> settings;
    if (options.sdk()) {
        settings.push_back(pbxsetting::Setting::Create("SDKROOT", *options.sdk()));
    }
    if (options.arch()) {
        settings.push_back(pbxsetting::Setting::Create("ARCHS", *options.arch()));
    }
    if (options.derivedDataPath()) {
        settings.push_back(pbxsetting::Setting::Create("DERIVED_DATA_DIR", *options.derivedDataPath()));
    }
    levels.push_back(pbxsetting::Level(settings));

    levels.push_back(options.settings());

    if (options.xcconfig()) {
        std::string path = FSUtil::ResolveRelativePath(*options.xcconfig(), workingDirectory);
        ext::optional<pbxsetting::XC::Config> config = pbxsetting::XC::Config::Load(filesystem, environment, path);
        if (!config) {
            fprintf(stderr, "warning: unable to open xcconfig '%s'\n", options.xcconfig()->c_str());
        } else {
            levels.push_back(config->level());
        }
    }

    if (ext::optional<std::string> configFile = processContext->environmentVariable("XCODE_XCCONFIG_FILE")) {
        std::string path = FSUtil::ResolveRelativePath(*configFile, workingDirectory);
        ext::optional<pbxsetting::XC::Config> config = pbxsetting::XC::Config::Load(filesystem, environment, path);
        if (!config) {
            fprintf(stderr, "warning: unable to open xcconfig from environment '%s'\n", path.c_str());
        } else {
            levels.push_back(config->level());
        }
    }

    return levels;
}

xcexecution::Parameters Action::
CreateParameters(Options const &options, std::vector<pbxsetting::Level> const &overrideLevels)
{
    return xcexecution::Parameters(
        options.workspace(),
        options.project(),
        options.scheme(),
        (!options.target().empty() ? ext::make_optional(options.target()) : ext::nullopt),
        options.allTargets(),
        options.actions(),
        options.configuration(),
        overrideLevels);
}

bool Action::
VerifyBuildActions(std::vector<std::string> const &actions)
{
    for (std::string const &action : actions) {
        if (action != "build" &&
            action != "build-for-testing" &&
            action != "analyze" &&
            action != "archive" &&
            action != "test" &&
            action != "test-without-building" &&
            action != "install-src" &&
            action != "install" &&
            action != "clean") {
            fprintf(stderr, "error: unknown build action '%s'\n", action.c_str());
            return false;
        }
    }

    return true;
}

Action::Type Action::
Determine(Options const &options)
{
    if (options.version()) {
        return Version;
    } else if (options.usage()) {
        return Usage;
    } else if (options.help()) {
        return Help;
    } else if (options.license()) {
        return License;
    } else if (options.checkFirstLaunchStatus()) {
        return CheckFirstLaunch;
    } else if (options.showSDKs()) {
        return ShowSDKs;
    } else if (options.findLibrary() || options.findExecutable()) {
        return Find;
    } else if (options.exportArchive()) {
        return ExportArchive;
    } else if (options.exportLocalizations() || options.importLocalizations()) {
        return Localizations;
    } else if (options.list()) {
        return List;
    } else if (options.showBuildSettings()) {
        return ShowBuildSettings;
    } else {
        return Build;
    }
}
