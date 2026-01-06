/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/Wildcard.h>

#include <algorithm>

using libutil::Wildcard;

bool Wildcard::
Match(std::string const &pattern, std::string const &string)
{
    std::string::const_iterator sit = string.begin();
    for (std::string::const_iterator fit = pattern.begin(); fit != pattern.end(); ++fit) {
        std::string::const_iterator fend;
        if (*fit == '*') {
            if (++fit == pattern.end() || sit == string.end()) {
                return true;
            }

            while (*sit++ != *fit) {
                if (sit == string.end()) {
                    return false;
                }
            }
        } else if (*fit == '[' && (fend = std::find(fit, pattern.end(), ']')) != pattern.end()) {
            std::string allowed = std::string(std::next(fit), fend);
            if (allowed.find(*sit++) == std::string::npos) {
                return false;
            }
            fit = fend;
        } else if (*fit != *sit++) {
            return false;
        }
    }

    return (sit == string.end());
}

