/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/BuildPhase.h>
#include <pbxproj/Context.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::BuildPhase;
using pbxproj::Context;

BuildPhase::
BuildPhase(std::string const &isa, Type type) :
    Object                             (isa),
    _type                              (type),
    _runOnlyForDeploymentPostprocessing(false),
    _buildActionMask                   (INT32_MAX)
{
}

bool BuildPhase::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("BuildPhase", dict, seen);

    auto N     = unpack.cast <plist::String> ("name");
    auto Fs    = unpack.cast <plist::Array> ("files");
    auto ROFDP = unpack.coerce <plist::Boolean> ("runOnlyForDeploymentPostprocessing");
    auto BAM   = unpack.coerce <plist::Integer> ("buildActionMask");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (Fs != nullptr) {
        for (size_t n = 0; n < Fs->count(); n++) {
            std::string FID;
            auto F = context.get <BuildFile> (Fs->value(n), &FID);
            if (F != nullptr) {
                auto BF = context.parseObject(context.buildFiles, FID, F);
                if (!BF)
                    return false;

                _files.push_back(BF);
            }
        }
    }

    if (ROFDP != nullptr) {
        _runOnlyForDeploymentPostprocessing = ROFDP->value();
    }

    if (BAM != nullptr) {
        _buildActionMask = BAM->value();
    }

    return true;
}
