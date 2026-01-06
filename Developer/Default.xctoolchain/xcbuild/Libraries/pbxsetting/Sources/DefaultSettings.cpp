/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxsetting/DefaultSettings.h>
#include <pbxsetting/Type.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>
#include <process/User.h>

#include <cstdlib>

#if defined(__APPLE__)
#include <unistd.h>
#endif

using pbxsetting::DefaultSettings;
using pbxsetting::Level;
using pbxsetting::Setting;
using pbxsetting::Type;
using libutil::FSUtil;

Level DefaultSettings::
Environment(process::User const *user, process::Context const *processContext)
{
    std::vector<Setting> settings;

    for (auto const &variable : processContext->environmentVariables()) {
        // TODO(grp): Is this right? Should this be filtered at another level?
        if (variable.first.front() != '_') {
            Setting setting = Setting::Create(variable.first, variable.second);
            settings.push_back(setting);
        }
    }

    settings.push_back(Setting::Create("UID", user->userID()));
    settings.push_back(Setting::Create("USER", user->userName()));
    settings.push_back(Setting::Create("GID", user->groupID()));
    settings.push_back(Setting::Create("GROUP", user->groupName()));
    if (ext::optional<std::string> home = user->userHomeDirectory()) {
        settings.push_back(Setting::Create("HOME", *home));
    }

    settings.push_back(Setting::Parse("USER_APPS_DIR", "$(HOME)/Applications"));
    settings.push_back(Setting::Parse("USER_LIBRARY_DIR", "$(HOME)/Library"));

#if defined(__APPLE__)
    // TODO(grp): This is reading system information directly.
    size_t len = confstr(_CS_DARWIN_USER_CACHE_DIR, NULL, 0);
    char *cache = (char *)malloc(len);
    confstr(_CS_DARWIN_USER_CACHE_DIR, cache, len);
    std::string cache_root = std::string(cache);
    free(cache);
#else
    // TODO(grp): Find a better cache root.
    std::string cache_root = "/tmp";
#endif
    cache_root = FSUtil::NormalizePath(cache_root + "/com.apple.DeveloperTools/6.4-$(XCODE_PRODUCT_BUILD_VERSION)/Xcode");
    settings.push_back(Setting::Create("CACHE_ROOT", cache_root));

    // Seems identical to TEMP_FILE_DIR but with an 'S'.
    settings.push_back(Setting::Parse("TEMP_FILES_DIR", "$(TEMP_DIR)"));

    return Level(settings);
}

Level DefaultSettings::
Internal(void)
{
    std::vector<Setting> settings = {
        Setting::Parse("APPLE_INTERNAL_DEVELOPER_DIR", "$(APPLE_INTERNAL_DIR)/Developer"),
        Setting::Parse("APPLE_INTERNAL_DIR", "/AppleInternal"),
        Setting::Parse("APPLE_INTERNAL_DOCUMENTATION_DIR", "$(APPLE_INTERNAL_DIR)/Documentation"),
        Setting::Parse("APPLE_INTERNAL_LIBRARY_DIR", "$(APPLE_INTERNAL_DIR)/Library"),
        Setting::Parse("APPLE_INTERNAL_TOOLS", "$(APPLE_INTERNAL_DEVELOPER_DIR)/Tools"),
    };

    return Level(settings);
}

Level DefaultSettings::
Local(void)
{
    std::vector<Setting> settings = {
        Setting::Create("LOCAL_ADMIN_APPS_DIR", "/Applications/Utilities"),
        Setting::Create("LOCAL_APPS_DIR", "/Applications"),
        Setting::Create("LOCAL_DEVELOPER_DIR", "/Library/Developer"),
        Setting::Create("LOCAL_LIBRARY_DIR", "/Library"),
    };

    return Level(settings);
}

Level DefaultSettings::
System(void)
{
    std::vector<Setting> settings = {
        Setting::Parse("SYSTEM_ADMIN_APPS_DIR", "/Applications/Utilities"),
        Setting::Parse("SYSTEM_APPS_DIR", "/Applications"),
        Setting::Parse("SYSTEM_CORE_SERVICES_DIR", "/System/Library/CoreServices"),
        Setting::Parse("SYSTEM_DEMOS_DIR", "/Applications/Extras"),
        Setting::Parse("SYSTEM_DEVELOPER_APPS_DIR", "$(DEVELOPER_APPLICATIONS_DIR)"),
        Setting::Parse("SYSTEM_DEVELOPER_BIN_DIR", "$(DEVELOPER_BIN_DIR)"),
        Setting::Parse("SYSTEM_DEVELOPER_DEMOS_DIR", "$(DEVELOPER_DIR)/Applications/Utilities/Built Examples"),
        Setting::Parse("SYSTEM_DEVELOPER_DIR", "$(DEVELOPER_DIR)"),
        Setting::Parse("SYSTEM_DEVELOPER_DOC_DIR", "$(DEVELOPER_DIR)/ADC Reference Library"),
        Setting::Parse("SYSTEM_DEVELOPER_GRAPHICS_TOOLS_DIR", "$(DEVELOPER_DIR)/Applications/Graphics Tools"),
        Setting::Parse("SYSTEM_DEVELOPER_JAVA_TOOLS_DIR", "$(DEVELOPER_DIR)/Applications/Java Tools"),
        Setting::Parse("SYSTEM_DEVELOPER_PERFORMANCE_TOOLS_DIR", "$(DEVELOPER_DIR)/Applications/Performance Tools"),
        Setting::Parse("SYSTEM_DEVELOPER_RELEASENOTES_DIR", "$(DEVELOPER_DIR)/ADC Reference Library/releasenotes"),
        Setting::Parse("SYSTEM_DEVELOPER_TOOLS", "$(DEVELOPER_TOOLS_DIR)"),
        Setting::Parse("SYSTEM_DEVELOPER_TOOLS_DOC_DIR", "$(DEVELOPER_DIR)/ADC Reference Library/documentation/DeveloperTools"),
        Setting::Parse("SYSTEM_DEVELOPER_TOOLS_RELEASENOTES_DIR", "$(DEVELOPER_DIR)/ADC Reference Library/releasenotes/DeveloperTools"),
        Setting::Parse("SYSTEM_DEVELOPER_USR_DIR", "$(DEVELOPER_USR_DIR)"),
        Setting::Parse("SYSTEM_DEVELOPER_UTILITIES_DIR", "$(DEVELOPER_DIR)/Applications/Utilities"),
        Setting::Parse("SYSTEM_DOCUMENTATION_DIR", "/Library/Documentation"),
        Setting::Parse("SYSTEM_LIBRARY_DIR", "/System/Library"),

        Setting::Create("OS", "MACOS"),
        Setting::Create("MAC_OS_X_PRODUCT_BUILD_VERSION", "14E46"),
        Setting::Create("MAC_OS_X_VERSION_ACTUAL", "101101"),
        Setting::Create("MAC_OS_X_VERSION_MAJOR", "101100"),
        Setting::Create("MAC_OS_X_VERSION_MINOR", "1001"),
    };

    return Level(settings);
}

Level DefaultSettings::
Architecture(void)
{
    std::vector<Setting> settings = {
#if defined(__i386__) || defined(__x86_64__)
        Setting::Create("NATIVE_ARCH_32_BIT", "i386"),
        Setting::Create("NATIVE_ARCH_64_BIT", "x86_64"),
  #if defined(__x86_64__)
        Setting::Create("NATIVE_ARCH_ACTUAL", "x86_64"),
  #else
        Setting::Create("NATIVE_ARCH_ACTUAL", "i386"),
  #endif
#elif defined(__arm__) || defined(__arm64__)
        Setting::Create("NATIVE_ARCH_32_BIT", "armv7"),
        Setting::Create("NATIVE_ARCH_64_BIT", "arm64"),
  #if defined(__arm64__)
        Setting::Create("NATIVE_ARCH_ACTUAL", "arm64"),
  #else
        Setting::Create("NATIVE_ARCH_ACTUAL", "armv7"),
  #endif
#else
        Setting::Create("NATIVE_ARCH_32_BIT", "UNKNOWN"),
        Setting::Create("NATIVE_ARCH_64_BIT", "UNKNOWN"),
        Setting::Create("NATIVE_ARCH_ACTUAL", "UNKNOWN"),
#endif
    };

    return Level(settings);
}

Level DefaultSettings::
Build(void)
{
    std::vector<Setting> settings = {
        Setting::Create("XCODE_PRODUCT_BUILD_VERSION", "7C68"),
        Setting::Create("XCODE_VERSION_ACTUAL", "0720"),
        Setting::Create("XCODE_VERSION_MAJOR", "0700"),
        Setting::Create("XCODE_VERSION_MINOR", "0720"),
        Setting::Parse("XCODE_APP_SUPPORT_DIR", "$(DEVELOPER_LIBRARY_DIR)/Xcode"),
    };

    return Level(settings);
}

std::vector<Level> DefaultSettings::
Levels(process::User const *user, process::Context const *processContext)
{
    return {
        Environment(user, processContext),
        Internal(),
        Local(),
        System(),
        Architecture(),
        Build(),
    };
}
