/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Build/Environment.h>
#include <xcsdk/Configuration.h>
#include <xcsdk/Environment.h>
#include <pbxsetting/DefaultSettings.h>
#include <pbxsetting/Environment.h>
#include <process/Context.h>
#include <process/User.h>
#include <libutil/Filesystem.h>

namespace Build = pbxbuild::Build;
using libutil::Filesystem;

Build::Environment::
Environment(
    pbxspec::Manager::shared_ptr const &specManager,
    std::shared_ptr<xcsdk::SDK::Manager> const &sdkManager,
    pbxsetting::Environment const &baseEnvironment,
    std::vector<std::string> const &baseExecutablePaths) :
    _specManager(specManager),
    _sdkManager(sdkManager),
    _baseEnvironment(baseEnvironment),
    _baseExecutablePaths(baseExecutablePaths)
{
}

ext::optional<Build::Environment> Build::Environment::
Default(process::User const *user, process::Context const *processContext, Filesystem const *filesystem)
{
    ext::optional<std::string> developerRoot = xcsdk::Environment::DeveloperRoot(user, processContext, filesystem);
    if (!developerRoot) {
        fprintf(stderr, "error: couldn't find developer dir\n");
        return ext::nullopt;
    }

    auto specManager = pbxspec::Manager::Create();
    if (specManager == nullptr) {
        fprintf(stderr, "error: couldn't create spec manager\n");
        return ext::nullopt;
    }

    /*
     * Register global build rules.
     */
    std::vector<std::string> buildRules = pbxspec::Manager::DeveloperBuildRules(*developerRoot);
    for (std::string const &path : buildRules) {
        if (filesystem->isReadable(path) && !specManager->registerBuildRules(filesystem, path)) {
            fprintf(stderr, "error: couldn't register build rules\n");
            return ext::nullopt;
        }
    }

    /*
     * Register global specifications.
     */
    specManager->registerDomains(filesystem, pbxspec::Manager::DefaultDomains(*developerRoot));

    auto configuration = xcsdk::Configuration::Load(filesystem, xcsdk::Configuration::DefaultPaths(user, processContext));
    auto sdkManager = xcsdk::SDK::Manager::Open(filesystem, *developerRoot, configuration);
    if (sdkManager == nullptr) {
        fprintf(stderr, "error: couldn't create SDK manager\n");
        return ext::nullopt;
    }

    /*
     * Register platform-specific specifications.
     */
    std::unordered_map<std::string, std::string> platforms;
    for (xcsdk::SDK::Platform::shared_ptr const &platform : sdkManager->platforms()) {
        platforms.insert({ platform->name(), platform->path() });
    }
    specManager->registerDomains(filesystem, pbxspec::Manager::PlatformDomains(platforms));

    /*
     * Register global specifications, but depend on platform-specific specifications.
     */
    specManager->registerDomains(filesystem, pbxspec::Manager::PlatformDependentDomains(*developerRoot));

    pbxspec::PBX::BuildSystem::shared_ptr buildSystem = specManager->buildSystem("com.apple.build-system.core", { "default" });
    if (buildSystem == nullptr) {
        fprintf(stderr, "error: couldn't create build system\n");
        return ext::nullopt;
    }

    pbxsetting::Environment baseEnvironment;
    baseEnvironment.insertBack(buildSystem->defaultSettings(), true);
    baseEnvironment.insertBack(sdkManager->computedSettings(), false);
    std::vector<pbxsetting::Level> defaultLevels = pbxsetting::DefaultSettings::Levels(user, processContext);
    for (pbxsetting::Level const &level : defaultLevels) {
        baseEnvironment.insertBack(level, false);
    }

    return Build::Environment(specManager, sdkManager, baseEnvironment, processContext->executableSearchPaths());
}
