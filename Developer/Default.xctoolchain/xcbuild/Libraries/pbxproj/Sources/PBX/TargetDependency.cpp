/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/TargetDependency.h>
#include <pbxproj/PBX/ContainerItemProxy.h>
#include <pbxproj/PBX/NativeTarget.h>
#include <pbxproj/PBX/AggregateTarget.h>
#include <pbxproj/PBX/LegacyTarget.h>
#include <pbxproj/Context.h>

using pbxproj::PBX::TargetDependency;
using pbxproj::Context;

TargetDependency::
TargetDependency() :
    Object(Isa())
{
}

bool TargetDependency::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    std::string TID;
    std::string TPID;

    auto unpack = plist::Keys::Unpack("TargetDependency", dict, seen);

    auto N  = unpack.cast <plist::String> ("name");
    auto TP = context.indirect <ContainerItemProxy> (&unpack, "targetProxy", &TPID);

    if (auto T = context.indirect <NativeTarget> (&unpack, "target", &TID)) {
        _target = context.parseObject(context.nativeTargets, TID, T);
        if (!_target) {
            return false;
        }
    } else if (auto T = context.indirect <AggregateTarget> (&unpack, "target", &TID)) {
        _target = context.parseObject(context.aggregateTargets, TID, T);
        if (!_target) {
            return false;
        }
    } else if (auto T = context.indirect <LegacyTarget> (&unpack, "target", &TID)) {
        _target = context.parseObject(context.legacyTargets, TID, T);
        if (!_target) {
            return false;
        }
    }

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (TP != nullptr) {
        _targetProxy = context.parseObject(context.containerItemProxies, TPID, TP);
        if (!_targetProxy) {
            return false;
        }
    }

    return true;
}
