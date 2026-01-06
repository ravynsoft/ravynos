/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/BuildStep.h>
#include <pbxspec/Context.h>
#include <pbxspec/Inherit.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::BuildStep;

BuildStep::
BuildStep() :
    Specification()
{
}

BuildStep::
~BuildStep()
{
}

BuildStep::shared_ptr BuildStep::
Parse(Context *context, plist::Dictionary const *dict)
{
    if (!ParseType(context, dict, Type())) {
        return nullptr;
    }

    BuildStep::shared_ptr result;
    result.reset(new BuildStep());

    std::unordered_set<std::string> seen;
    if (!result->parse(context, dict, &seen, true))
        return nullptr;

    return result;
}

bool BuildStep::
parse(Context *context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Specification::parse(context, dict, seen, false))
        return false;

    auto unpack = plist::Keys::Unpack("BuildStep", dict, seen);

    auto BST = unpack.cast <plist::String> ("BuildStepType");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (BST != nullptr) {
        _buildStepType = BST->value();
    }

    return true;
}

bool BuildStep::
inherit(Specification::shared_ptr const &base)
{
    if (base->type() != BuildStep::Type())
        return false;

    return inherit(std::static_pointer_cast<BuildStep>(base));
}

bool BuildStep::
inherit(BuildStep::shared_ptr const &b)
{
    if (!Specification::inherit(b))
        return false;

    auto base = this->base();

    _buildStepType = Inherit::Override(_buildStepType, base->_buildStepType);

    return true;
}

