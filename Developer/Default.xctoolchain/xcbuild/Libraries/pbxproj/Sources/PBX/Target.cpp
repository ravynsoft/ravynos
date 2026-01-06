/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/Target.h>
#include <pbxproj/PBX/NativeTarget.h>
#include <pbxproj/PBX/BuildPhases.h>
#include <pbxproj/Context.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::Target;
using pbxproj::Context;

Target::
Target(std::string const &isa, Type type) :
    Object(isa),
    _type (type)
{
}

pbxsetting::Level Target::
settings(void) const
{
    std::vector<pbxsetting::Setting> settings = {
        pbxsetting::Setting::Create("TARGETNAME", _name),
        pbxsetting::Setting::Create("TARGET_NAME", _name),
        pbxsetting::Setting::Create("PRODUCT_NAME", _productName),
    };

    if (_type == Type::Native) {
        PBX::NativeTarget const *nativeTarget = static_cast <PBX::NativeTarget const *> (this);
        if (PBX::FileReference::shared_ptr const &productReference = nativeTarget->productReference()) {
            pbxsetting::Setting setting = pbxsetting::Setting::Create("FULL_PRODUCT_NAME", productReference->name());
            settings.push_back(setting);
        }
    }

    return pbxsetting::Level(settings);
}

bool Target::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    _project = context.project;

    auto unpack = plist::Keys::Unpack("Target", dict, seen);

    std::string BCLID;

    auto N   = unpack.cast <plist::String> ("name");
    auto PN  = unpack.cast <plist::String> ("productName");
    auto BCL = context.indirect <XC::ConfigurationList> (&unpack, "buildConfigurationList", &BCLID);
    auto BPs = unpack.cast <plist::Array> ("buildPhases");
    auto Ds  = unpack.cast <plist::Array> ("dependencies");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (PN != nullptr) {
        _productName = PN->value();
    }

    if (BCL != nullptr) {
        _buildConfigurationList = context.parseObject(context.configurationLists, BCLID, BCL);
        if (!_buildConfigurationList) {
            return false;
        }
    }

    if (BPs != nullptr) {
        for (size_t n = 0; n < BPs->count(); n++) {
            auto ID = BPs->value <plist::String> (n);
            if (ID == nullptr) {
                continue;
            }

            if (auto BPd = context.get <HeadersBuildPhase> (ID)) {
                auto O = context.parseObject(context.headersBuildPhases, ID->value(), BPd);
                if (!O) {
                    return false;
                }

                _buildPhases.push_back(O);
            } else if (auto BPd = context.get <SourcesBuildPhase> (ID)) {
                auto O = context.parseObject(context.sourcesBuildPhases, ID->value(), BPd);
                if (!O) {
                    return false;
                }

                _buildPhases.push_back(O);
            } else if (auto BPd = context.get <ResourcesBuildPhase> (ID)) {
                auto O = context.parseObject(context.resourcesBuildPhases, ID->value(), BPd);
                if (!O) {
                    return false;
                }

                _buildPhases.push_back(O);
            } else if (auto BPd = context.get <FrameworksBuildPhase> (ID)) {
                auto O = context.parseObject(context.frameworksBuildPhases, ID->value(), BPd);
                if (!O) {
                    return false;
                }

                _buildPhases.push_back(O);
            } else if (auto BPd = context.get <CopyFilesBuildPhase> (ID)) {
                auto O = context.parseObject(context.copyFilesBuildPhases, ID->value(), BPd);
                if (!O) {
                    return false;
                }

                _buildPhases.push_back(O);
            } else if (auto BPd = context.get <ShellScriptBuildPhase> (ID)) {
                auto O = context.parseObject(context.shellScriptBuildPhases, ID->value(), BPd);
                if (!O) {
                    return false;
                }

                _buildPhases.push_back(O);
            } else if (auto BPd = context.get <AppleScriptBuildPhase> (ID)) {
                auto O = context.parseObject(context.appleScriptBuildPhases, ID->value(), BPd);
                if (!O) {
                    return false;
                }

                _buildPhases.push_back(O);
            } else if (auto BPd = context.get <RezBuildPhase> (ID)) {
                auto O = context.parseObject(context.rezBuildPhases, ID->value(), BPd);
                if (!O) {
                    return false;
                }

                _buildPhases.push_back(O);
            } else {
                fprintf(stderr, "warning: target '%s' contains unsupported build phase reference to '%s'\n",
                        _name.c_str(), ID->value().c_str());
            }
        }
    }

    if (Ds != nullptr) {
        for (size_t n = 0; n < Ds->count(); n++) {
            std::string DID;
            auto D = context.get <TargetDependency> (Ds->value(n), &DID);
            if (D != nullptr) {
                auto TD = context.parseObject(context.targetDependencies, DID, D);
                if (!TD) {
                    return false;
                }

                _dependencies.push_back(TD);
            }
        }
    }

    return true;
}
