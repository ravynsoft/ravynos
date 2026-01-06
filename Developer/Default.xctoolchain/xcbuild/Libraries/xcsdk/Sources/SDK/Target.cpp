/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/SDK/Target.h>
#include <xcsdk/SDK/Manager.h>
#include <pbxsetting/Type.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <plist/Keys/Unpack.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

using xcsdk::SDK::Target;
using xcsdk::SDK::Manager;
using xcsdk::SDK::Platform;
using libutil::Filesystem;
using libutil::FSUtil;

Target::
Target()
{
}

Target::
~Target()
{
}

pbxsetting::Level Target::
settings(void) const
{
    std::vector<pbxsetting::Setting> settings = {
        pbxsetting::Setting::Create("SDK_NAME", _canonicalName.value_or("")),
        pbxsetting::Setting::Create("SDK_DIR", _path),
        pbxsetting::Setting::Create("SDK_PRODUCT_BUILD_VERSION", (_product ? _product->buildVersion().value_or("") : "")),
    };

    std::vector<std::string> toolchainIdentifiers;
    for (Toolchain::shared_ptr const &toolchain : _toolchains) {
        if (toolchain->identifier()) {
            toolchainIdentifiers.push_back(*toolchain->identifier());
        }
    }
    settings.push_back(pbxsetting::Setting::Create("TOOLCHAINS", pbxsetting::Type::FormatList(toolchainIdentifiers)));

    return pbxsetting::Level(settings);
}

std::vector<std::string> Target::
executablePaths() const
{
    std::vector<std::string> paths;
    return paths;
}

bool Target::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("Target", dict, &seen);

    auto Ts    = unpack.cast <plist::Array> ("Toolchains");
    auto CN    = unpack.cast <plist::String> ("CanonicalName");
    auto DN    = unpack.cast <plist::String> ("DisplayName");
    auto MDN   = unpack.cast <plist::String> ("MinimalDisplayName");
    auto V     = unpack.cast <plist::String> ("Version");
    auto IBS   = unpack.coerce <plist::Boolean> ("IsBaseSDK");
    auto DDT   = unpack.cast <plist::String> ("DefaultDeploymentTarget");
    auto MDT   = unpack.cast <plist::String> ("MaximumDeploymentTarget");
    auto DP    = unpack.cast <plist::Dictionary> ("DefaultProperties");
    auto CP    = unpack.cast <plist::Dictionary> ("CustomProperties");
    auto PCFNs = unpack.cast <plist::Array> ("PropertyConditionFallbackNames");
    auto DSFN  = unpack.cast <plist::String> ("DocSetFeedName");
    auto DSFU  = unpack.cast <plist::String> ("DocSetFeedURL");
    auto MSTV  = unpack.cast <plist::String> ("MinimumSupportedToolsVersion");
    auto SBTCs = unpack.cast <plist::Array> ("SupportedBuildToolComponents");

    /* Ignored: seems to be a typo. */
    (void)unpack.cast <plist::String> ("isBaseSDK");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (Ts  != nullptr) {
        if (std::shared_ptr<Manager> manager = _manager.lock()) {
            for (size_t n = 0; n < Ts ->count(); n++) {
                if (auto T = Ts ->value <plist::String> (n)) {
                    if (auto toolchain = manager->findToolchain(T->value())) {
                        _toolchains.push_back(toolchain);
                    } else if (auto defaultToolchain = manager->findToolchain(Toolchain::DefaultIdentifier())) {
                        _toolchains.push_back(defaultToolchain);
                    }
                }
            }
        }
    } else {
        if (std::shared_ptr<Manager> manager = _manager.lock()) {
            if (auto defaultToolchain = manager->findToolchain(Toolchain::DefaultIdentifier())) {
                _toolchains.push_back(defaultToolchain);
            }
        }
    }

    if (CN != nullptr) {
        _canonicalName = CN->value();
    }

    if (DN != nullptr) {
        _displayName = DN->value();
    }

    if (MDN != nullptr) {
        _minimalDisplayName = MDN->value();
    }

    if (V != nullptr) {
        _version = V->value();
    }

    if (IBS != nullptr) {
        _isBaseSDK = IBS->value();
    }

    if (DDT != nullptr) {
        _defaultDeploymentTarget = DDT->value();
    }

    if (MDT != nullptr) {
        _maximumDeploymentTarget = MDT->value();
    }

    if (CP != nullptr) {
        std::vector<pbxsetting::Setting> settings;
        for (size_t n = 0; n < CP->count(); n++) {
            auto CPK = CP->key(n);
            auto CPV = CP->value <plist::String> (CPK);

            if (CPV != nullptr) {
                pbxsetting::Setting setting = pbxsetting::Setting::Parse(CPK, CPV->value());
                settings.push_back(setting);
            }
        }
        _customProperties = pbxsetting::Level(settings);
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

    if (PCFNs != nullptr) {
        _propertyConditionFallbackNames = std::vector<std::string>();
        for (size_t n = 0; n < PCFNs->count(); n++) {
            if (auto PCFN = PCFNs->value <plist::String> (n)) {
                _propertyConditionFallbackNames->push_back(PCFN->value());
            }
        }
    }

    if (DSFN != nullptr) {
        _docSetFeedName = DSFN->value();
    }

    if (DSFU != nullptr) {
        _docSetFeedURL = DSFU->value();
    }

    if (MSTV != nullptr) {
        _minimumSupportedToolsVersion = MSTV->value();
    }

    if (SBTCs != nullptr) {
        _supportedBuildToolComponents = std::vector<std::string>();
        for (size_t n = 0; n < SBTCs->count(); n++) {
            if (auto SBTC = SBTCs->value <plist::String> (n)) {
                _supportedBuildToolComponents->push_back(SBTC->value());
            }
        }
    }

    return true;
}

Target::shared_ptr Target::
Open(Filesystem const *filesystem, std::shared_ptr<Manager> manager, std::shared_ptr<Platform> platform, std::string const &path)
{
    if (path.empty()) {
        return nullptr;
    }

    /*
     * Load target settings.
     */
    std::string settingsFileName = path + "/SDKSettings.plist";
    if (!filesystem->isReadable(settingsFileName.c_str())) {
        settingsFileName = path + "/Info.plist";
        if (!filesystem->isReadable(settingsFileName.c_str())) {
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
     * Parse settings property list.
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
     * Create the target object.
     */
    auto target = std::make_shared<Target>();
    target->_manager  = manager;
    target->_platform = platform;
    target->_path       = FSUtil::GetDirectoryName(realPath);
    target->_bundleName = FSUtil::GetBaseName(realPath);

    /*
     * Parse the settings dictionary.
     */
    if (!target->parse(plist)) {
        return nullptr;
    }

    /*
     * Parse product information.
     */
    target->_product = Product::Open(filesystem, target->_path);

    return target;
}
