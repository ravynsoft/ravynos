/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/Linker.h>
#include <pbxspec/Inherit.h>
#include <pbxspec/Context.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::Linker;

Linker::
Linker() :
    Tool()
{
}

Linker::
~Linker()
{
}

bool Linker::
inherit(Specification::shared_ptr const &base)
{
    if (base->type() != Linker::Type())
        return false;

    return inherit(std::static_pointer_cast<Linker>(base));
}

bool Linker::
inherit(Tool::shared_ptr const &base)
{
    if (base->type() != Linker::Type())
        return false;

    return inherit(std::static_pointer_cast<Linker>(base));
}

bool Linker::
inherit(Linker::shared_ptr const &b)
{
    if (!Tool::inherit(std::static_pointer_cast<Tool>(b)))
        return false;

    auto base = this->base();

    _binaryFormats         = Inherit::Combine(_binaryFormats, base->_binaryFormats);
    _dependencyInfoFile    = Inherit::Override(_dependencyInfoFile, base->_dependencyInfoFile);
    _supportsInputFileList = Inherit::Override(_supportsInputFileList, base->_supportsInputFileList);

    return true;
}

Linker::shared_ptr Linker::
Parse(Context *context, plist::Dictionary const *dict)
{
    if (!ParseType(context, dict, Type())) {
        return nullptr;
    }

    Linker::shared_ptr result;
    result.reset(new Linker());

    std::unordered_set<std::string> seen;
    if (!result->parse(context, dict, &seen, true))
        return nullptr;

    return result;
}

bool Linker::
parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Tool::parse(context, dict, seen, false))
        return false;

    auto unpack = plist::Keys::Unpack("Linker", dict, seen);

    auto BFs  = unpack.cast <plist::Array> ("BinaryFormats");
    auto DIF  = unpack.cast <plist::String> ("DependencyInfoFile");
    auto SIFL = unpack.coerce <plist::Boolean> ("SupportsInputFileList");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (BFs != nullptr) {
        _binaryFormats = std::vector<std::string>();
        for (size_t n = 0; n < BFs->count(); n++) {
            if (auto BF = BFs->value <plist::String> (n)) {
                _binaryFormats->push_back(BF->value());
            }
        }
    }

    if (DIF != nullptr) {
        _dependencyInfoFile = pbxsetting::Value::Parse(DIF->value());
    }

    if (SIFL != nullptr) {
        _supportsInputFileList = SIFL->value();
    }

    return true;
}
