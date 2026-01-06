/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/BuildSettings.h>
#include <pbxspec/Context.h>
#include <pbxspec/Inherit.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::BuildSettings;

BuildSettings::
BuildSettings() :
    Specification()
{
}

BuildSettings::
~BuildSettings()
{
}

BuildSettings::shared_ptr BuildSettings::
Parse(Context *context, plist::Dictionary const *dict)
{
    if (!ParseType(context, dict, Type())) {
        return nullptr;
    }

    BuildSettings::shared_ptr result;
    result.reset(new BuildSettings());

    std::unordered_set<std::string> seen;
    if (!result->parse(context, dict, &seen, true))
        return nullptr;

    return result;
}

bool BuildSettings::
parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Specification::parse(context, dict, seen, false))
        return false;

    auto unpack = plist::Keys::Unpack("BuildSettings", dict, seen);

    auto Os = unpack.cast <plist::Array> ("Options");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (Os != nullptr) {
        _options = PropertyOption::vector();
        for (size_t n = 0; n < Os->count(); n++) {
            if (auto O = Os->value <plist::Dictionary> (n)) {
                PropertyOption::shared_ptr option;
                option.reset(new PropertyOption);
                if (option->parse(O)) {
                    PropertyOption::Insert(&*_options, &_optionsUsed, option);
                }
            }
        }
    }

    return true;
}

bool BuildSettings::
inherit(Specification::shared_ptr const &base)
{
    if (base->type() != BuildSettings::Type())
        return false;

    return inherit(std::static_pointer_cast<BuildSettings>(base));
}

bool BuildSettings::
inherit(BuildSettings::shared_ptr const &b)
{
    if (!Specification::inherit(b))
        return false;

    auto base = this->base();

    _options            = Inherit::Combine(_options, base->options(), &_optionsUsed, &base->_optionsUsed);

    return true;
}

