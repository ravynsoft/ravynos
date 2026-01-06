/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PlistHelpers.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

namespace pbxproj {

plist::Dictionary const *
PlistDictionaryGetPBXObject(plist::Dictionary const *dict,
                            std::string const &key,
                            std::string const &isa)
{
    if (isa.empty())
        return nullptr;

    plist::Dictionary const *object = dict->value <plist::Dictionary> (key);
    if (object == nullptr)
        return nullptr;

    plist::String const *isaObject = object->value <plist::String> ("isa");
    if (isaObject != nullptr && isaObject->value() == isa)
        return object;
    else
        return nullptr;
}

plist::Dictionary const *
PlistDictionaryGetIndirectPBXObject(plist::Dictionary const *objects,
                                    plist::Keys::Unpack *unpack,
                                    std::string const &key,
                                    std::string const &isa,
                                    std::string *id)
{
    auto ID = unpack->cast <plist::String> (key);
    if (ID == nullptr)
        return nullptr;

    if (id != nullptr) {
        *id = ID->value();
    }
    return PlistDictionaryGetPBXObject(objects, ID->value(), isa);
}

}
