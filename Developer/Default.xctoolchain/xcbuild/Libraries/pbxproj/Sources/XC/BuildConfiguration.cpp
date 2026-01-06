/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/XC/BuildConfiguration.h>
#include <pbxproj/Context.h>

using pbxproj::XC::BuildConfiguration;
using pbxproj::Context;

BuildConfiguration::
BuildConfiguration() :
    Object        (Isa()),
    _buildSettings(pbxsetting::Level({ }))
{
}

BuildConfiguration::~BuildConfiguration()
{
}

bool BuildConfiguration::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("BuildConfiguration", dict, seen);

    std::string BCRID;

    auto BCR = context.indirect <PBX::FileReference> (&unpack, "baseConfigurationReference", &BCRID);
    auto BS  = unpack.cast <plist::Dictionary> ("buildSettings");
    auto N   = unpack.cast <plist::String> ("name");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (BCR != nullptr) {
        _baseConfigurationReference =
          context.parseObject(context.fileReferences, BCRID, BCR);
        if (!_baseConfigurationReference)
            return false;
    }

    if (BS != nullptr) {
        std::vector<pbxsetting::Setting> settings;
        for (size_t n = 0; n < BS->count(); n++) {
            auto BSk = BS->key(n);
            auto BSv = BS->value(BSk);
            pbxsetting::Setting setting = pbxsetting::Setting::Create(BSk, pbxsetting::Value::FromObject(BSv));
            settings.push_back(setting);
        }
        _buildSettings = pbxsetting::Level(settings);
    }

    if (N != nullptr) {
        _name = N->value();
    }

    return true;
}
