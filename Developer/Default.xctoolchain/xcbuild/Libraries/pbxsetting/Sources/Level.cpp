/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxsetting/Level.h>

using pbxsetting::Level;
using pbxsetting::Condition;
using pbxsetting::Setting;
using pbxsetting::Value;

Level::
Level(std::vector<Setting> const &settings) :
    _settings(std::make_shared<std::vector<Setting>>(settings))
{
}

Level::
~Level()
{
}

std::pair<bool, Value> Level::
get(std::string const &setting, Condition const &condition) const
{
    for (auto it = _settings->rbegin(); it != _settings->rend(); ++it) {
        if (it->match(setting, condition)) {
            return std::make_pair(true, it->value());
        }
    }

    return std::make_pair(false, Value::Empty());
}

