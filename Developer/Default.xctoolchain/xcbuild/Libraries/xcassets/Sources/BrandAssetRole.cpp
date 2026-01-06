/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/BrandAssetRole.h>

#include <cstdlib>

using xcassets::BrandAssetRole;
using xcassets::BrandAssetRoles;

ext::optional<BrandAssetRole> BrandAssetRoles::
Parse(std::string const &value)
{
    if (value == "primary-app-icon") {
        return BrandAssetRole::PrimaryAppIcon;
    } else if (value == "top-shelf-image") {
        return BrandAssetRole::TopShelfImage;
    } else if (value == "top-shelf-image-wide") {
        return BrandAssetRole::TopShelfImageWide;
    } else {
        fprintf(stderr, "warning: unknown brand asset role %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string BrandAssetRoles::
String(BrandAssetRole brandAssetRole)
{
    switch (brandAssetRole) {
        case BrandAssetRole::PrimaryAppIcon:
            return "primary-app-icon";
        case BrandAssetRole::TopShelfImage:
            return "top-shelf-image";
        case BrandAssetRole::TopShelfImageWide:
            return "top-shelf-image-wide";
    }

    abort();
}
