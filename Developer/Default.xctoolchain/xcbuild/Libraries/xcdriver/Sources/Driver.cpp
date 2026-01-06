/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/Driver.h>
#include <xcdriver/Action.h>
#include <xcdriver/Options.h>
#include <xcdriver/BuildAction.h>
#include <xcdriver/FindAction.h>
#include <xcdriver/HelpAction.h>
#include <xcdriver/LicenseAction.h>
#include <xcdriver/ListAction.h>
#include <xcdriver/ShowSDKsAction.h>
#include <xcdriver/ShowBuildSettingsAction.h>
#include <xcdriver/UsageAction.h>
#include <xcdriver/VersionAction.h>
#include <libutil/Filesystem.h>
#include <process/Context.h>

#include <string>
#include <vector>

using xcdriver::Driver;
using xcdriver::Action;
using xcdriver::Options;
using libutil::Filesystem;

Driver::
Driver()
{
}

Driver::
~Driver()
{
}

int Driver::
Run(process::User const *user, process::Context const *processContext, process::Launcher *processLauncher, Filesystem *filesystem)
{
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, processContext->commandLineArguments());
    if (!result.first) {
        fprintf(stderr, "error: %s\n\n", result.second.c_str());
        UsageAction::Run(processContext);
        return 1;
    }

    Action::Type action = Action::Determine(options);
    switch (action) {
        case Action::Build:
            return BuildAction::Run(user, processContext, processLauncher, filesystem, options);
        case Action::ShowBuildSettings:
            return ShowBuildSettingsAction::Run(user, processContext, filesystem, options);
        case Action::List:
            return ListAction::Run(user, processContext, filesystem, options);
        case Action::Version:
            return VersionAction::Run(user, processContext, filesystem, options);
        case Action::Usage:
            return UsageAction::Run(processContext);
        case Action::Help:
            return HelpAction::Run(processContext);
        case Action::License:
            return LicenseAction::Run();
        case Action::CheckFirstLaunch:
            fprintf(stderr, "warning: check first launch not implemented\n");
            break;
        case Action::ShowSDKs:
            return ShowSDKsAction::Run(user, processContext, filesystem, options);
        case Action::Find:
            return FindAction::Run(user, processContext, filesystem, options);
        case Action::ExportArchive:
            fprintf(stderr, "warning: export archive not implemented\n");
            break;
        case Action::Localizations:
            fprintf(stderr, "warning: localizations not implemented\n");
            break;
    }

    return 0;
}
