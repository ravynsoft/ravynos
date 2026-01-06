/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_BrandAssetRole_h
#define __xcassets_BrandAssetRole_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * How an asset in a brand assets is used.
 */
enum class BrandAssetRole {
    PrimaryAppIcon,
    TopShelfImage,
    TopShelfImageWide,
};

class BrandAssetRoles {
private:
    BrandAssetRoles();
    ~BrandAssetRoles();

public:
    /*
     * Parse a brand asset role string.
     */
    static ext::optional<BrandAssetRole> Parse(std::string const &value);

    /*
     * String representation of a brand asset role.
     */
    static std::string String(BrandAssetRole brandAssetRole);
};

}

#endif // !__xcassets_BrandAssetRole_h
