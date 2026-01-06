/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/ReferenceProxy.h>
#include <pbxproj/Context.h>

using pbxproj::PBX::ReferenceProxy;
using pbxproj::Context;

ReferenceProxy::
ReferenceProxy() :
    GroupItem(Isa(), Type::ReferenceProxy)
{
}

bool ReferenceProxy::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!GroupItem::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("ReferenceProxy", dict, seen);

    std::string RRID;

    auto RR = context.indirect <ContainerItemProxy> (&unpack, "remoteRef", &RRID);
    auto FT = unpack.cast <plist::String> ("fileType");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (RR != nullptr) {
        _remoteRef = context.parseObject(context.containerItemProxies, RRID, RR);
        if (_remoteRef == nullptr) {
            return false;
        }
    }

    if (FT != nullptr) {
        _fileType = FT->value();
    }

    return true;
}
