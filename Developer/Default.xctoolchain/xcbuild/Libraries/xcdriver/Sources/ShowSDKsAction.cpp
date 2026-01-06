/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/ShowSDKsAction.h>
#include <xcdriver/Options.h>
#include <xcsdk/Configuration.h>
#include <xcsdk/Environment.h>
#include <xcsdk/SDK/Manager.h>
#include <xcsdk/SDK/Platform.h>
#include <xcsdk/SDK/Target.h>
#include <libutil/Filesystem.h>
#include <process/Context.h>

using xcdriver::ShowSDKsAction;
using xcdriver::Options;
using libutil::Filesystem;

ShowSDKsAction::
ShowSDKsAction()
{
}

ShowSDKsAction::
~ShowSDKsAction()
{
}

int ShowSDKsAction::
Run(process::User const *user, process::Context const *processContext, Filesystem const *filesystem, Options const &options)
{
    ext::optional<std::string> developerRoot = xcsdk::Environment::DeveloperRoot(user, processContext, filesystem);
    if (!developerRoot) {
        fprintf(stderr, "error: unable to find developer dir\n");
        return 1;
    }

    auto configuration = xcsdk::Configuration::Load(filesystem, xcsdk::Configuration::DefaultPaths(user, processContext));
    auto manager = xcsdk::SDK::Manager::Open(filesystem, *developerRoot, configuration);
    if (manager == nullptr) {
        fprintf(stderr, "error: unable to open developer directory\n");
        return 1;
    }

    for (auto const &platform : manager->platforms()) {
        printf("%s SDKs:\n", platform->description().value_or(platform->name()).c_str());
        for (auto const &target : platform->targets()) {
            printf("\t%-32s-sdk %s\n", target->displayName().value_or(target->bundleName()).c_str(), target->canonicalName().value_or(target->bundleName()).c_str());
        }
        printf("\n");
    }

    return 0;
}
