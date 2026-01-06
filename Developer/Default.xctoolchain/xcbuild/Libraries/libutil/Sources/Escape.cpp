/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/Escape.h>

using libutil::Escape;

std::string Escape::
Shell(std::string const &value)
{
    static std::string const *escaped = nullptr;
    if (escaped == nullptr) {
        std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::string digits = "0123456789";
        escaped = new std::string(alphabet + digits + "@%_-+=:,./");
    }

    if (value.find_first_not_of(*escaped) == std::string::npos) {
        return value;
    } else {
        std::string result;
        result.reserve(value.size() + 4);
        result += "'";

        std::string::size_type offset = 0;
        std::string::size_type previous = 0;
        while ((offset = value.find("'", offset)) != std::string::npos) {
            result.append(value.data() + previous, offset - previous);
            result += "'\\''";

            offset += 1;
            previous = offset;
        }
        result.append(value.data() + previous, value.size() - previous);

        result += "'";
        return result;
    }
}

std::string Escape::
Makefile(std::string const &value)
{
    std::string result;
    result.reserve(value.size());

    for (char c : value) {
        if (isspace(c) || c == '#' || c == '$' || c == '%' || c == ':') {
            result += '\\';
        }

        result += c;
    }

    return result;
}
