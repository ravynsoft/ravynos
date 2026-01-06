/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Build_Environment_h
#define __pbxbuild_Build_Environment_h

#include <pbxbuild/Base.h>
#include <pbxsetting/Environment.h>
#include <xcsdk/SDK/Manager.h>

#include <ext/optional>

namespace libutil { class Filesystem; }
namespace process { class Context; }
namespace process { class User; }

namespace pbxbuild {
namespace Build {

/*
 * Represents the collection of utilities needed for a build. These utilties
 * are not tied to the build but are used across the build.
 */
class Environment {
private:
    pbxspec::Manager::shared_ptr         _specManager;
    std::shared_ptr<xcsdk::SDK::Manager> _sdkManager;
    pbxsetting::Environment              _baseEnvironment;
    std::vector<std::string>             _baseExecutablePaths;

public:
    Environment(
        pbxspec::Manager::shared_ptr const &specManager,
        std::shared_ptr<xcsdk::SDK::Manager> const &sdkManager,
        pbxsetting::Environment const &baseEnvironment,
        std::vector<std::string> const &baseExecutablePaths);

public:
    /*
     * The specification manager and specifications for the build.
     */
    pbxspec::Manager::shared_ptr const &specManager() const
    { return _specManager; }

    /*
     * The SDK manager for the build.
     */
    std::shared_ptr<xcsdk::SDK::Manager> const &sdkManager() const
    { return _sdkManager; }

public:
    /*
     * The base environment from the system.
     */
    pbxsetting::Environment const &baseEnvironment() const
    { return _baseEnvironment; }

    /*
     * The base executable paths from the system.
     */
    std::vector<std::string> const &baseExecutablePaths() const
    { return _baseExecutablePaths; }

public:
    /*
     * Creates a build environment from the default configuration
     * of each of the build environment's subcomponents.
     */
    static ext::optional<Environment>
    Default(
        process::User const *user,
        process::Context const *processContext,
        libutil::Filesystem const *filesystem);
};

}
}

#endif // !__pbxbuild_Build_Environment_h
