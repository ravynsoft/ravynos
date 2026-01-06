/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/SDK/Toolchain.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>
#include <plist/Format/Any.h>

using xcsdk::SDK::Toolchain;
using libutil::Filesystem;
using libutil::FSUtil;

Toolchain::
Toolchain()
{
}

Toolchain::
~Toolchain()
{
}

std::vector<std::string> Toolchain::
executablePaths() const
{
    return {
        _path + "/usr/bin",
        _path + "/usr/libexec",
    };
}

bool Toolchain::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("Toolchain", dict, &seen);

    auto I    = unpack.cast <plist::String> ("Identifier");
    auto As   = unpack.cast <plist::Array> ("Aliases");
    auto DN   = unpack.cast <plist::String> ("DisplayName");
    auto SDN  = unpack.cast <plist::String> ("ShortDisplayName");
    auto CD   = unpack.coerce <plist::String> ("CreatedDate"); /* From plist::Date. */
    auto RPU  = unpack.cast <plist::String> ("ReportProblemURL");
    auto CFBI = unpack.cast <plist::String> ("CFBundleIdentifier");
    auto V    = unpack.cast <plist::String> ("Version");
    auto CV   = unpack.coerce <plist::Integer> ("CompatibilityVersion");
    auto CVDS = unpack.cast <plist::String> ("CompatibilityVersionDisplayString");
    auto PSV  = unpack.coerce <plist::Boolean> ("ProvidesSwiftVersion");
    auto OBS  = unpack.cast <plist::Dictionary> ("OverrideBuildSettings");

    /* Ignored */
    (void)unpack.cast <plist::String> ("DTSDKBuild");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (I != nullptr) {
        _identifier = I->value();
    }

    if (As != nullptr) {
        _aliases = std::vector<std::string>();
        for (size_t n = 0; n < As->count(); n++) {
            if (auto A = As->value<plist::String>(n)) {
                _aliases->push_back(A->value());
            }
        }
    }

    if (DN != nullptr) {
        _displayName = DN->value();
    }

    if (SDN != nullptr) {
        _shortDisplayName = SDN->value();
    }

    if (CD != nullptr) {
        _createdDate = CD->value();
    }

    if (RPU != nullptr) {
        _reportProblemURL = RPU->value();
    }

    if (CFBI != nullptr) {
        _bundleIdentifier = CFBI->value();
    }

    if (V != nullptr) {
        _version = V->value();
    }

    if (CV != nullptr) {
        _compatibilityVersion = CV->value();
    }

    if (CVDS != nullptr) {
        _compatibilityVersionDisplayString = CVDS->value();
    }

    if (PSV != nullptr) {
        _providesSwiftVersion = PSV->value();
    }

    if (OBS != nullptr) {
        std::vector<pbxsetting::Setting> settings;
        for (size_t n = 0; n < OBS->count(); n++) {
            auto OBSK = OBS->key(n);
            auto OBSV = OBS->value <plist::String> (OBSK);

            if (OBSV != nullptr) {
                pbxsetting::Setting setting = pbxsetting::Setting::Parse(OBSK, OBSV->value());
                settings.push_back(setting);
            }
        }
        _overrideBuildSettings = pbxsetting::Level(settings);
    }

    return true;
}

Toolchain::shared_ptr Toolchain::
Open(Filesystem const *filesystem, std::string const &path)
{
    if (path.empty()) {
        return nullptr;
    }

    /*
     * Read toolchain info.
     */
    std::string settingsFileName = path + "/ToolchainInfo.plist";
    if (!filesystem->isReadable(settingsFileName)) {
        settingsFileName = path + "/Info.plist";
        if (!filesystem->isReadable(settingsFileName)) {
            return nullptr;
        }
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
     * Parse property list.
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
     * Create the toolchain object.
     */
    auto toolchain = std::make_shared<Toolchain>();
    toolchain->_path = FSUtil::GetDirectoryName(realPath);
    toolchain->_name = FSUtil::GetBaseNameWithoutExtension(toolchain->_path);

    /*
     * Parse the toolchain dictionary.
     */
    if (!toolchain->parse(plist)) {
        return nullptr;
    }

    return toolchain;
}

std::string Toolchain::
DefaultIdentifier(void)
{
    return "com.apple.dt.toolchain.XcodeDefault";
}

