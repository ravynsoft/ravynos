/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/BuildSystem.h>
#include <pbxspec/Context.h>
#include <pbxspec/Inherit.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::BuildSystem;

BuildSystem::
BuildSystem() :
    Specification()
{
}

BuildSystem::
~BuildSystem()
{
}

pbxsetting::Level BuildSystem::
defaultSettings(void) const
{
    std::vector<pbxsetting::Setting> settings;
    if (_properties) {
        for (PBX::PropertyOption::shared_ptr const &option : *_properties) {
            if (ext::optional<pbxsetting::Setting> setting = option->defaultSetting()) {
                settings.push_back(*setting);
            }
        }
    }
    if (_options) {
        for (PBX::PropertyOption::shared_ptr const &option : *_options) {
            if (ext::optional<pbxsetting::Setting> setting = option->defaultSetting()) {
                settings.push_back(*setting);
            }
        }
    }
    return pbxsetting::Level(settings);
}

BuildSystem::shared_ptr BuildSystem::
Parse(Context *context, plist::Dictionary const *dict)
{
    if (!ParseType(context, dict, Type())) {
        return nullptr;
    }

    BuildSystem::shared_ptr result;
    result.reset(new BuildSystem());

    std::unordered_set<std::string> seen;
    if (!result->parse(context, dict, &seen, true))
        return nullptr;

    return result;
}

bool BuildSystem::
parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Specification::parse(context, dict, seen, false))
        return false;

    auto unpack = plist::Keys::Unpack("BuildSystem", dict, seen);

    auto Os  = unpack.cast <plist::Array> ("Options");
    auto Ps  = unpack.cast <plist::Array> ("Properties");
    auto DPs = unpack.cast <plist::Array> ("DeletedProperties");

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

    if (Ps != nullptr) {
        _properties = PropertyOption::vector();
        for (size_t n = 0; n < Ps->count(); n++) {
            if (auto P = Ps->value <plist::Dictionary> (n)) {
                PropertyOption::shared_ptr property;
                property.reset(new PropertyOption);
                if (property->parse(P)) {
                    PropertyOption::Insert(&*_properties, &_propertiesUsed, property);
                }
            }
        }
    }

    if (DPs != nullptr) {
        _deletedProperties = std::unordered_set<std::string>();
        for (size_t n = 0; n < DPs->count(); n++) {
            if (auto DP = DPs->value <plist::String> (n)) {
                _deletedProperties->insert(DP->value());
            }
        }
    }

    return true;
}

bool BuildSystem::
inherit(Specification::shared_ptr const &base)
{
    if (base->type() != BuildSystem::Type())
        return false;

    return inherit(std::static_pointer_cast<BuildSystem>(base));
}

bool BuildSystem::
inherit(BuildSystem::shared_ptr const &b)
{
    if (!Specification::inherit(b))
        return false;

    auto base = this->base();

    _options           = Inherit::Combine(_options, base->_options, &_optionsUsed, &base->_optionsUsed);
    _properties        = Inherit::Combine(_properties, base->_properties, &_propertiesUsed, &base->_propertiesUsed);
    _deletedProperties = Inherit::Combine(_deletedProperties, base->_deletedProperties);

    return true;
}
