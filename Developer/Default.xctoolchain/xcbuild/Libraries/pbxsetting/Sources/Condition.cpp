/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxsetting/Condition.h>
#include <libutil/Wildcard.h>

using pbxsetting::Condition;
using libutil::Wildcard;

Condition::
Condition(std::unordered_map<std::string, std::string> const &values) :
    _values(values)
{
}

Condition::
~Condition()
{
}

bool Condition::
match(Condition const &condition) const
{
    auto OV = condition._values;
    for (auto const &TE : _values) {
        auto OE = OV.find(TE.first);
        if (OE == OV.end()) {
            return false;
        }

        if (!Wildcard::Match(TE.second, OE->second)) {
            return false;
        }
    }

    return true;
}

Condition const &Condition::
Empty(void)
{
    std::unordered_map<std::string, std::string> emptyMap;
    static Condition *condition = new Condition(emptyMap);
    return *condition;
}
