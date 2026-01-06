/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/SDK/Manager.h>
#include <xcsdk/Configuration.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Type.h>

#include <algorithm>

using xcsdk::Configuration;
using xcsdk::SDK::Manager;
using xcsdk::SDK::Platform;
using xcsdk::SDK::Target;
using xcsdk::SDK::Toolchain;
using libutil::Filesystem;
using libutil::FSUtil;

Manager::
Manager()
{
}

Manager::
~Manager()
{
}

Target::shared_ptr Manager::
findTarget(std::string const &name) const
{
    for (Platform::shared_ptr const &platform : _platforms) {
        for (Target::shared_ptr const &target : platform->targets()) {
            /* Try both the name and the path; either are valid. */
            if (target->canonicalName() == name || target->path() == name) {
                return target;
            }
        }

        /* If the platform name matches but no targets do, use any target. */
        if (platform->name() == name || platform->path() == name) {
            if (!platform->targets().empty()) {
                return platform->targets().back();
            }
        }
    }

    return nullptr;
}

Toolchain::shared_ptr Manager::
findToolchain(std::string const &name) const
{
    for (Toolchain::shared_ptr const &toolchain : _toolchains) {
        /* Match liberally: name, identifier, or path; all are valid. */
        if (toolchain->name() == name || toolchain->identifier() == name || toolchain->path() == name) {
            return toolchain;
        }
    }

    return nullptr;
}

std::vector<Platform::shared_ptr> Manager::
findPlatformFamily(std::string const &identifier)
{
    std::vector<Platform::shared_ptr> platforms;

    for (Platform::shared_ptr const &platform : _platforms) {
        /* Match by family identifier. */
        if (platform->familyIdentifier() == identifier) {
            platforms.push_back(platform);
        }
    }

    return platforms;
}

pbxsetting::Level Manager::
computedSettings(void) const
{
    std::vector<pbxsetting::Setting> settings = {
        pbxsetting::Setting::Create("DEVELOPER_DIR", _path),
        pbxsetting::Setting::Parse("DEVELOPER_USR_DIR", "$(DEVELOPER_DIR)/usr"),
        pbxsetting::Setting::Parse("DEVELOPER_BIN_DIR", "$(DEVELOPER_DIR)/usr/bin"),
        pbxsetting::Setting::Parse("DEVELOPER_APPLICATIONS_DIR", "$(DEVELOPER_DIR)/Applications"),
        pbxsetting::Setting::Parse("DEVELOPER_FRAMEWORKS_DIR", "$(DEVELOPER_DIR)/Library/Frameworks"),
        pbxsetting::Setting::Parse("DEVELOPER_FRAMEWORKS_DIR_QUOTED", "$(DEVELOPER_DIR)/Library/Frameworks"),
        pbxsetting::Setting::Parse("DEVELOPER_LIBRARY_DIR", "$(DEVELOPER_DIR)/Library"),
        pbxsetting::Setting::Parse("DEVELOPER_TOOLS_DIR", "$(DEVELOPER_DIR)/Tools"),
        pbxsetting::Setting::Parse("DEVELOPER_SDK_DIR", "$(DEVELOPER_DIR)/Platforms/MacOSX.platform/Developer/SDKs"), // TODO(grp): Verify.
        pbxsetting::Setting::Parse("LEGACY_DEVELOPER_DIR", "$(DEVELOPER_DIR)/../PlugIns/Xcode3Core.ideplugin/Contents/SharedSupport/Developer"), // TODO(grp): Verify.
        pbxsetting::Setting::Parse("DERIVED_DATA_DIR", "$(USER_LIBRARY_DIR)/Developer/Xcode/DerivedData"),
    };

    if (Toolchain::shared_ptr defaultToolchain = findToolchain(Toolchain::DefaultIdentifier())) {
        settings.push_back(pbxsetting::Setting::Create("DT_TOOLCHAIN_DIR", defaultToolchain->path()));
    } else {
        settings.push_back(pbxsetting::Setting::Create("DT_TOOLCHAIN_DIR", ""));
    }

    std::vector<std::string> platformNames;
    for (Platform::shared_ptr const &platform : _platforms) {
        platformNames.push_back(platform->name());
    }
    settings.push_back(pbxsetting::Setting::Create("AVAILABLE_PLATFORMS", pbxsetting::Type::FormatList(platformNames)));

    return pbxsetting::Level(settings);
}

std::vector<std::string> Manager::
executablePaths() const
{
    return {
        _path + "/usr/bin",
        _path + "/usr/local/bin",
        _path + "/Tools",
    };
}

std::vector<std::string> Manager::
executablePaths(Platform::shared_ptr const &platform, Target::shared_ptr const &target, std::vector<Toolchain::shared_ptr> const &toolchains) const
{
    std::vector<std::string> paths;

    if (target != nullptr) {
        std::vector<std::string> targetPaths = target->executablePaths();
        paths.insert(paths.end(), targetPaths.begin(), targetPaths.end());
    }

    if (platform != nullptr) {
        std::vector<std::string> platformPaths = platform->executablePaths();
        paths.insert(paths.end(), platformPaths.begin(), platformPaths.end());
    }

    for (Toolchain::shared_ptr const &toolchain : toolchains) {
        std::vector<std::string> toolchainPaths = toolchain->executablePaths();
        paths.insert(paths.end(), toolchainPaths.begin(), toolchainPaths.end());
    }

    std::vector<std::string> managerPaths = this->executablePaths();
    paths.insert(paths.end(), managerPaths.begin(), managerPaths.end());

    return paths;
}

std::shared_ptr<Manager> Manager::
Open(Filesystem const *filesystem, std::string const &path, ext::optional<Configuration> const &configuration)
{
    if (path.empty()) {
        fprintf(stderr, "error: empty path for sdk manager\n");
        return nullptr;
    }

    auto manager = std::make_shared <Manager> ();
    manager->_path = path;

    std::vector<std::string> toolchainsPaths = { path + "/" + "Toolchains" };
    if (configuration) {
        std::vector<std::string> const &extraToolchainsPaths = configuration->extraToolchainsPaths();
        toolchainsPaths.insert(toolchainsPaths.end(), extraToolchainsPaths.begin(), extraToolchainsPaths.end());
    }

    std::vector<std::shared_ptr<Toolchain>> toolchains;
    for (std::string const &toolchainsPath : toolchainsPaths) {
        filesystem->readDirectory(toolchainsPath, false, [&](std::string const &filename) -> void {
            if (FSUtil::GetFileExtension(filename) != "xctoolchain") {
                return;
            }

            auto toolchain = SDK::Toolchain::Open(filesystem, toolchainsPath + "/" + filename);
            if (toolchain != nullptr) {
                toolchains.push_back(toolchain);
            }
        });
    }
    manager->_toolchains = toolchains;

    std::vector<std::string> platformsPaths = { path + "/" + "Platforms" };
    if (configuration) {
        std::vector<std::string> const &extraPlatformsPaths = configuration->extraPlatformsPaths();
        platformsPaths.insert(platformsPaths.end(), extraPlatformsPaths.begin(), extraPlatformsPaths.end());
    }

    std::vector<std::shared_ptr<Platform>> platforms;
    for (std::string const &platformsPath : platformsPaths) {
        filesystem->readDirectory(platformsPath, false, [&](std::string const &filename) -> void {
            if (FSUtil::GetFileExtension(filename) != "platform") {
                return;
            }

            auto platform = SDK::Platform::Open(filesystem, manager, platformsPath + "/" + filename);
            if (platform != nullptr) {
                platforms.push_back(platform);
            }
        });
    }
    std::sort(platforms.begin(), platforms.end(), [](Platform::shared_ptr const &a, Platform::shared_ptr const &b) -> bool {
        return (a->description() < b->description());
    });
    manager->_platforms = platforms;

    return manager;
}
