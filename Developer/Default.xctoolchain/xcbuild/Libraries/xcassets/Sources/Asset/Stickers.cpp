/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/Stickers.h>
#include <plist/Keys/Unpack.h>
#include <libutil/Filesystem.h>

using xcassets::Asset::Stickers;
using libutil::Filesystem;

bool Stickers::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    /* No contents is allowed for stickers. */
    if (dict != nullptr) {
        auto unpack = plist::Keys::Unpack("Stickers", dict, seen);

        /* No additional contents. */

        if (!unpack.complete(check)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }
    }

    return true;
}

std::unique_ptr<Stickers> Stickers::
Load(libutil::Filesystem const *filesystem, std::string const &path)
{
    auto asset = Asset::Load(filesystem, path, { }, Stickers::Extension());
    return libutil::static_unique_pointer_cast<Stickers>(std::move(asset));
}

