/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/SDK/Platform.h>
#include <xcsdk/SDK/Manager.h>
#include <pbxsetting/Type.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <plist/Keys/Unpack.h>

#include <algorithm>

using xcsdk::SDK::Platform;
using xcsdk::SDK::Manager;
using xcsdk::SDK::Target;
using libutil::Filesystem;
using libutil::FSUtil;

Platform::
Platform() :
    _defaultDebuggerSettings(nullptr)
{
}

Platform::
~Platform()
{
    if (_defaultDebuggerSettings != nullptr) {
        _defaultDebuggerSettings->release();
    }
}

static bool
StartsWith(std::string const &str, std::string const &prefix)
{
    if (prefix.size() > str.size()) {
        return false;
    }

    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

static bool
EndsWith(std::string const &str, std::string const &suffix)
{
    if (suffix.size() > str.size()) {
        return false;
    }

    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

pbxsetting::Level Platform::
settings() const
{
    std::vector<pbxsetting::Setting> settings = {
        pbxsetting::Setting::Create("PLATFORM_NAME", _name),
        pbxsetting::Setting::Create("PLATFORM_DISPLAY_NAME", _description.value_or("")),
        pbxsetting::Setting::Create("PLATFORM_DIR", _path),

        pbxsetting::Setting::Parse("PLATFORM_DEVELOPER_USR_DIR", "$(PLATFORM_DIR)/Developer/usr"),
        pbxsetting::Setting::Parse("PLATFORM_DEVELOPER_BIN_DIR", "$(PLATFORM_DIR)/Developer/usr/bin"),
        pbxsetting::Setting::Parse("PLATFORM_DEVELOPER_APPLICATIONS_DIR", "$(PLATFORM_DIR)/Developer/Applications"),
        pbxsetting::Setting::Parse("PLATFORM_DEVELOPER_LIBRARY_DIR", "$(DEVELOPER_DIR)/../PlugIns/Xcode3Core.ideplugin/Contents/SharedSupport/Developer/Library"), // TODO(grp): Verify.
        pbxsetting::Setting::Parse("PLATFORM_DEVELOPER_SDK_DIR", "$(PLATFORM_DIR)/Developer/SDKs"),
        pbxsetting::Setting::Parse("PLATFORM_DEVELOPER_TOOLS_DIR", "$(PLATFORM_DIR)/Developer/Tools"),

        pbxsetting::Setting::Create("PLATFORM_PRODUCT_BUILD_VERSION", _platformVersion != nullptr ? _platformVersion->buildVersion().value_or("") : ""),
        // TODO(grp): PLATFORM_PREFERRED_ARCH

        // TODO(grp): CORRESPONDING_DEVICE_PLATFORM_NAME
        // TODO(grp): CORRESPONDING_DEVICE_PLATFORM_DIR
        // TODO(grp): CORRESPONDING_SIMULATOR_PLATFORM_NAME
        // TODO(grp): CORRESPONDING_SIMULATOR_PLATFORM_DIR
    };

    std::string flagName;
    std::string settingName;
    std::string envName;
    std::string swiftName;

    if (StartsWith(_name, "macosx")) {
        flagName = "macosx";
        settingName = "MACOSX";
        envName = "MACOSX";
        swiftName = "macosx";
    } else if (StartsWith(_name, "iphone")) {
        flagName = "ios";
        settingName = "IPHONEOS";
        envName = "IPHONEOS";
        swiftName = "ios";
    } else if (StartsWith(_name, "appletv")) {
        flagName = "tvos";
        settingName = "TVOS";
        envName = "TVOS";
        swiftName = "tvos";
    } else if (StartsWith(_name, "watch")) {
        flagName = "watchos";
        settingName = "WATCHOS";
        envName = "WATCHOS";
        swiftName = "watchos";
    } else {
        flagName = _name;

        settingName = _name;
        std::transform(settingName.begin(), settingName.end(), settingName.begin(), ::toupper);

        envName = _name;
        std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);

        swiftName = _name;
    }

    bool simulator = EndsWith(_name, "simulator");
    if (simulator) {
        flagName += "-simulator";
    }

    settings.push_back(pbxsetting::Setting::Create("DEPLOYMENT_TARGET_SETTING_NAME", settingName + "_DEPLOYMENT_TARGET"));
    settings.push_back(pbxsetting::Setting::Create("DEPLOYMENT_TARGET_CLANG_FLAG_NAME", "m" + flagName + "-version-min"));
    settings.push_back(pbxsetting::Setting::Create("DEPLOYMENT_TARGET_CLANG_FLAG_PREFIX", "-m" + flagName + "-version-min="));
    settings.push_back(pbxsetting::Setting::Create("DEPLOYMENT_TARGET_CLANG_ENV_NAME", envName + "_DEPLOYMENT_TARGET"));
    settings.push_back(pbxsetting::Setting::Create("SWIFT_PLATFORM_TARGET_PREFIX", swiftName));

    settings.push_back(pbxsetting::Setting::Parse("EFFECTIVE_PLATFORM_NAME", (_name == "macosx" ? "" : "-$(PLATFORM_NAME)")));

    std::vector<std::string> supportedPlatformNames;
    std::shared_ptr<Manager> manager = _manager.lock();
    if (manager && _familyIdentifier) {
        for (Platform::shared_ptr const &platform : manager->platforms()) {
            if (platform->familyIdentifier() == _familyIdentifier) {
                supportedPlatformNames.push_back(platform->name());
            }
        }
    } else {
        supportedPlatformNames.push_back(_name);
    }
    settings.push_back(pbxsetting::Setting::Create("SUPPORTED_PLATFORMS", pbxsetting::Type::FormatList(supportedPlatformNames)));

    return pbxsetting::Level(settings);
}

std::vector<std::string> Platform::
executablePaths() const
{
    return {
        _path + "/Developer/usr/bin",
        _path + "/usr/local/bin",
        _path + "/usr/bin",
        _path + "/usr/local/bin",
    };
}

bool Platform::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("Platform", dict, &seen);

    auto I   = unpack.cast <plist::String> ("Identifier");
    auto N   = unpack.cast <plist::String> ("Name");
    auto D   = unpack.cast <plist::String> ("Description");
    auto T   = unpack.cast <plist::String> ("Type");
    auto V   = unpack.cast <plist::String> ("Version");
    auto FI  = unpack.cast <plist::String> ("FamilyIdentifier");
    auto FN  = unpack.cast <plist::String> ("FamilyName");
    auto Ic  = unpack.cast <plist::String> ("Icon");
    auto DDS = unpack.cast <plist::Dictionary> ("DefaultDebuggerSettings");
    auto DP  = unpack.cast <plist::Dictionary> ("DefaultProperties");
    auto OP  = unpack.cast <plist::Dictionary> ("OverrideProperties");
    auto AI  = unpack.cast <plist::Dictionary> ("AdditionalInfo");
    auto IDP = unpack.coerce <plist::Boolean> ("IsDeploymentPlatform");
    auto MSV = unpack.cast <plist::String> ("MinimumSDKVersion");
    auto RSS = unpack.cast <plist::String> ("RuntimeSystemSpecification");

    /* Ignored: not platform related. */
    (void)unpack.cast <plist::String> ("CFBundleName");
    (void)unpack.cast <plist::String> ("CFBundleIdentifier");
    (void)unpack.cast <plist::String> ("CFBundleVersion");
    (void)unpack.cast <plist::String> ("CFBundleShortVersionString");
    (void)unpack.cast <plist::Array> ("CFBundleSupportedPlatforms");
    (void)unpack.cast <plist::String> ("CFBundleDevelopmentRegion");

    /* Ignored: build machine info. */
    (void)unpack.cast <plist::String> ("DTSDKBuild");
    (void)unpack.cast <plist::String> ("DTSDKName");
    (void)unpack.cast <plist::String> ("DTPlatformBuild");
    (void)unpack.cast <plist::String> ("DTPlatformVersion");
    (void)unpack.cast <plist::String> ("DTCompiler");
    (void)unpack.cast <plist::String> ("DTXcode");
    (void)unpack.cast <plist::String> ("DTXcodeBuild");
    (void)unpack.cast <plist::String> ("BuildMachineOSBuild");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (I != nullptr) {
        _identifier = I->value();
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (D != nullptr) {
        _description = D->value();
    }

    if (T != nullptr) {
        _type = T->value();
    }

    if (V != nullptr) {
        _version = V->value();
    }

    if (FI != nullptr) {
        _familyIdentifier = FI->value();
    }

    if (FN != nullptr) {
        _familyName = FN->value();
    }

    if (Ic != nullptr) {
        _icon = Ic->value();
    }

    if (DDS != nullptr) {
        _defaultDebuggerSettings = DDS->copy().release();
    }

    if (DP != nullptr) {
        std::vector<pbxsetting::Setting> settings;
        for (size_t n = 0; n < DP->count(); n++) {
            auto DPK = DP->key(n);
            auto DPV = DP->value <plist::String> (DPK);

            if (DPV != nullptr) {
                pbxsetting::Setting setting = pbxsetting::Setting::Parse(DPK, DPV->value());
                settings.push_back(setting);
            }
        }
        _defaultProperties = pbxsetting::Level(settings);
    }

    if (OP != nullptr) {
        std::vector<pbxsetting::Setting> settings;
        for (size_t n = 0; n < OP->count(); n++) {
            auto OPK = OP->key(n);
            auto OPV = OP->value <plist::String> (OPK);

            if (OPV != nullptr) {
                pbxsetting::Setting setting = pbxsetting::Setting::Parse(OPK, OPV->value());
                settings.push_back(setting);
            }
        }
        _overrideProperties = pbxsetting::Level(settings);
    }

    if (AI != nullptr) {
        std::vector<pbxsetting::Setting> settings;
        for (size_t n = 0; n < AI->count(); n++) {
            auto AIK = AI->key(n);
            auto AIV = AI->value <plist::String> (AIK);

            if (AIV != nullptr) {
                pbxsetting::Setting setting = pbxsetting::Setting::Parse(AIK, AIV->value());
                settings.push_back(setting);
            }
        }
        _additionalInfo = pbxsetting::Level(settings);
    }

    if (IDP != nullptr) {
        _isDeploymentPlatform = IDP->value();
    }

    if (MSV != nullptr) {
        _minimumSDKVersion = MSV->value();
    }

    if (RSS != nullptr) {
        _runtimeSystemSpecification = RSS->value();
    }

    return true;
}

Platform::shared_ptr Platform::
Open(Filesystem const *filesystem, std::shared_ptr<Manager> manager, std::string const &path)
{
    if (path.empty()) {
        return nullptr;
    }

    /*
     * Load platform info.
     */
    std::string settingsFileName = path + "/Info.plist";
    if (!filesystem->isReadable(settingsFileName)) {
        return nullptr;
    }

    std::string realPath = filesystem->resolvePath(settingsFileName);
    if (realPath.empty()) {
        return nullptr;
    }

    std::vector<uint8_t> contents;
    if (!filesystem->read(&contents, settingsFileName)) {
        return nullptr;
    }

    /*
     * Parse platform info property list.
     */
    auto result = plist::Format::Any::Deserialize(contents);
    if (result.first == nullptr) {
        return nullptr;
    }

    plist::Dictionary *plist = plist::CastTo<plist::Dictionary>(result.first.get());
    if (plist == nullptr) {
        return nullptr;
    }

    /*
     * Create the platform object.
     */
    auto platform = std::make_shared<Platform>();
    platform->_manager = manager;
    platform->_path = FSUtil::GetDirectoryName(realPath);

    /*
     * Parse platform info dictionary.
     */
    if (!platform->parse(plist)) {
        return nullptr;
    }

    /*
     * Load platform version information.
     */
    platform->_platformVersion = PlatformVersion::Open(filesystem, platform->_path);

    /*
     * Load all the SDKs inside the platform.
     */
    std::string sdksPath = platform->_path + "/Developer/SDKs";
    filesystem->readDirectory(sdksPath, false, [&](std::string const &filename) -> void {
        if (FSUtil::GetFileExtension(filename) != "sdk") {
            return;
        }

        if (auto target = Target::Open(filesystem, manager, platform, sdksPath + "/" + filename)) {
            platform->_targets.push_back(target);
        }
    });

    std::sort(platform->_targets.begin(), platform->_targets.end(), [](Target::shared_ptr const &a, Target::shared_ptr const &b) -> bool {
        return (a->canonicalName() < b->canonicalName());
    });

    return platform;
}
