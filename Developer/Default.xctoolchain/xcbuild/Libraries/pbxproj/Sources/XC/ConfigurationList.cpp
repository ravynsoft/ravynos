/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/XC/ConfigurationList.h>
#include <pbxproj/Context.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

#include <cassert>

using pbxproj::XC::ConfigurationList;
using pbxproj::Context;

ConfigurationList::
ConfigurationList() :
    Object                        (Isa()),
    _defaultConfigurationIsVisible(false)
{
}

bool ConfigurationList::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("ConfigurationList", dict, seen);

    auto DCN  = unpack.cast <plist::String> ("defaultConfigurationName");
    auto DCIV = unpack.coerce <plist::Boolean> ("defaultConfigurationIsVisible");
    auto BCs  = unpack.cast <plist::Array> ("buildConfigurations");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (DCN != nullptr) {
        _defaultConfigurationName = DCN->value();
    }

    if (DCIV != nullptr) {
        _defaultConfigurationIsVisible = DCIV->value();
    }

    if (BCs != nullptr) {
        for (size_t n = 0; n < BCs->count(); n++) {
            std::string BCID;

            auto BCd = context.get <BuildConfiguration> (BCs->value(n), &BCID);
            assert(BCd != nullptr);

            auto BC = context.parseObject(context.buildConfigurations, BCID, BCd);
            if (!BC)
                return false;

            _buildConfigurations.push_back(BC);
        }
    }

    return true;
}
