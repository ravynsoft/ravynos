/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/VersionAction.h>
#include <xcdriver/Options.h>
#include <xcsdk/Configuration.h>
#include <xcsdk/Environment.h>
#include <xcsdk/SDK/Manager.h>
#include <xcsdk/SDK/Platform.h>
#include <xcsdk/SDK/PlatformVersion.h>
#include <xcsdk/SDK/Product.h>
#include <xcsdk/SDK/Target.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <libutil/Filesystem.h>
#include <process/Context.h>

using xcdriver::VersionAction;
using xcdriver::Options;
using libutil::Filesystem;

VersionAction::
VersionAction()
{
}

VersionAction::
~VersionAction()
{
}

int VersionAction::
Run(process::User const *user, process::Context const *processContext, Filesystem const *filesystem, Options const &options)
{
    if (!options.sdk()) {
        // TODO(grp): Real version numbers.
        printf("xcbuild version 0.1\n");
        printf("Build version 1\n");
    } else {
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

        xcsdk::SDK::Target::shared_ptr target = manager->findTarget(*options.sdk());
        if (target == nullptr) {
            fprintf(stderr, "error: cannot find sdk '%s'\n", options.sdk()->c_str());
            return 1;
        }

        /* Use over a standard map to preserve key order. */
        auto values = plist::Dictionary::New();

        if (target->version()) {
            values->set("SDKVersion", plist::String::New(*target->version()));
        }
        values->set("Path", plist::String::New(target->path()));
        if (target->platform()->version()) {
            values->set("PlatformVersion", plist::String::New(*target->platform()->version()));
        }
        values->set("PlatformPath", plist::String::New(target->platform()->path()));
        if (auto product = target->product()) {
            if (product->buildVersion()) {
                values->set("ProductBuildVersion", plist::String::New(*product->buildVersion()));
            }
            if (product->copyright()) {
                values->set("ProductCopyright", plist::String::New(*product->copyright()));
            }
            if (product->name()) {
                values->set("ProductName", plist::String::New(*product->name()));
            }
            if (product->userVisibleVersion()) {
                values->set("ProductUserVisibleVersion", plist::String::New(*product->userVisibleVersion()));
            }
            if (product->version()) {
                values->set("ProductVersion", plist::String::New(*product->version()));
            }
        }

        if (options.actions().empty()) {
            fprintf(stdout,
                "%s - %s (%s)\n",
                target->bundleName().c_str(),
                target->displayName().value_or(target->bundleName()).c_str(),
                target->canonicalName().value_or(target->bundleName()).c_str());

            for (size_t n = 0; n < values->count(); n++) {
                if (plist::String const *value = values->value<plist::String>(n)) {
                    fprintf(stdout, "%s: %s\n", values->key(n).c_str(), value->value().c_str());
                }
            }

            fprintf(stdout, "\n");
        } else {
            for (std::string const &action : options.actions()) {
                if (plist::String const *value = values->value<plist::String>(action)) {
                    fprintf(stdout, "%s\n", value->value().c_str());
                }
            }
        }
    }

    return 0;
}
