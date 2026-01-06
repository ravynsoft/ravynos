/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/BaseGroup.h>
#include <pbxproj/PBX/Group.h>
#include <pbxproj/PBX/VariantGroup.h>
#include <pbxproj/XC/VersionGroup.h>
#include <pbxproj/PBX/FileReference.h>
#include <pbxproj/PBX/ReferenceProxy.h>
#include <pbxproj/Context.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::BaseGroup;
using pbxproj::Context;

BaseGroup::
BaseGroup(std::string const &isa, GroupItem::Type type) :
    GroupItem(isa, type)
{
}

bool BaseGroup::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!GroupItem::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("BaseGroup", dict, seen);

    auto Cs = unpack.cast <plist::Array> ("children");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (Cs != nullptr) {
        for (size_t n = 0; n < Cs->count(); n++) {
            auto ID = Cs->value <plist::String> (n);
            if (ID == nullptr) {
                continue;
            }

            if (auto C = context.get <Group> (ID)) {
                auto O = context.parseObject(context.groups, ID->value(), C);
                if (!O) {
                    return false;
                }

                O->_parent = this;
                _children.push_back(O);
            } else if (auto C = context.get <VariantGroup> (ID)) {
                auto O = context.parseObject(context.variantGroups, ID->value(), C);
                if (!O) {
                    return false;
                }

                O->_parent = this;
                _children.push_back(O);
            } else if (auto C = context.get <XC::VersionGroup> (ID)) {
                auto O = context.parseObject(context.versionGroups, ID->value(), C);
                if (!O) {
                    return false;
                }

                O->_parent = this;
                _children.push_back(O);
            } else if (auto C = context.get <FileReference> (ID)) {
                auto O = context.parseObject(context.fileReferences, ID->value(), C);
                if (!O) {
                    return false;
                }

                O->_parent = this;
                _children.push_back(O);
            } else if (auto C = context.get <ReferenceProxy> (ID)) {
                auto O = context.parseObject(context.referenceProxies, ID->value(), C);
                if (!O) {
                    return false;
                }

                O->_parent = this;
                _children.push_back(O);
            } else if (context.objects->value(ID->value()) != nullptr) {
                fprintf(stderr, "warning: group '%s' contains unsupported child reference to '%s'\n",
                        _name.c_str(), ID->value().c_str());
            }
        }
    }

    return true;
}
