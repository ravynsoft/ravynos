/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Base64.h>

#include "rfc4648.h"

using plist::Base64;

void Base64::
Decode(std::string const &in, std::vector<uint8_t> &out)
{
    size_t outsize = rfc4648_get_decoded_size(RFC4648_TYPE_BASE64_SAFE, in.size());
    out.resize(outsize);
    rfc4648_decode(RFC4648_TYPE_BASE64_SAFE, &in[0], in.size(), reinterpret_cast<char *>(&out[0]), &outsize, true);
}

std::string Base64::
Encode(std::vector<uint8_t> const &in)
{
    if (in.empty()) {
        return std::string();
    }

    std::string result;

    size_t outsize = rfc4648_get_encoded_size(RFC4648_TYPE_BASE64_SAFE, in.size());
    result.resize(outsize);
    rfc4648_encode(RFC4648_TYPE_BASE64_SAFE, &in[0], in.size(), &result[0], &outsize);

    return result;
}
