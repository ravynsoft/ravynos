/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxsetting_Level_h
#define __pbxsetting_Level_h

#include <pbxsetting/Condition.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Value.h>

#include <vector>
#include <utility>
#include <memory>

namespace pbxsetting {

/*
 * Defines a level of build settings. Build settings are organized in
 * multiple levels, where the settings *within* a level override each
 * other in order but *between* levels can refer to previous bindings.
 */
class Level {
private:
    std::shared_ptr<std::vector<Setting>> _settings;

public:
    /*
     * Creates a level with the given settings.
     */
    Level(std::vector<Setting> const &settings);
    ~Level();

public:
    /*
     * The settings in the level.
     */
    std::vector<Setting> const &settings() const
    { return *_settings; }

public:
    /*
     * Fetches a setting from a level. Fails if the setting is not bound
     * in this level, or is bound but for a condition that doesn't match.
     */
    std::pair<bool, Value>
    get(std::string const &setting, Condition const &condition) const;
};

}

#endif  // !__pbxsetting_Level_h
