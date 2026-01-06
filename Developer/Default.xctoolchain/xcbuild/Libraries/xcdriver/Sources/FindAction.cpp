/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/FindAction.h>
#include <xcdriver/Options.h>
#include <xcsdk/Configuration.h>
#include <xcsdk/Environment.h>
#include <xcsdk/SDK/Manager.h>
#include <xcsdk/SDK/Target.h>
#include <libutil/Filesystem.h>
#include <process/Context.h>

using xcdriver::FindAction;
using xcdriver::Options;
using libutil::Filesystem;

FindAction::
FindAction()
{
}

FindAction::
~FindAction()
{
}

int FindAction::
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

    std::string sdk = options.sdk().value_or("macosx");

    xcsdk::SDK::Target::shared_ptr target = manager->findTarget(sdk);
    if (target == nullptr) {
        fprintf(stderr, "error: cannot find sdk '%s'\n", sdk.c_str());
        return 1;
    }

    if (options.findExecutable()) {
        ext::optional<std::string> executable = filesystem->findExecutable(*options.findExecutable(), target->executablePaths());
        if (!executable) {
            fprintf(stderr, "error: '%s' not found\n", options.findExecutable()->c_str());
            return 1;
        }

        printf("%s\n", executable->c_str());
        return 0;
    } else if (options.findLibrary()) {
        fprintf(stderr, "warning: finding libraries is not supported\n");
        return 0;
    } else {
        fprintf(stderr, "error: unknown find option\n");
        return 1;
    }
}
