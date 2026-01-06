/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxsetting_Environment_h
#define __pbxsetting_Environment_h

#include <pbxsetting/Condition.h>
#include <pbxsetting/Level.h>

#include <list>
#include <string>
#include <unordered_map>

namespace pbxsetting {

/*
 * Represents a hierarchical list of build settings (an ordered list of build
 * setting levels). Can use those levels to evaluate build setting values.
 */
class Environment {
private:
    std::list<Level> _levels;
    size_t           _offset;

public:
    explicit Environment();
    explicit Environment(Environment const &) = default;
    Environment const &operator=(Environment const &) = delete;
    Environment(Environment &&) = default;
    Environment &operator=(Environment &&) = default;

public:
    /*
     * Evaluate a build setting in the environment.
     */
    std::string
    resolve(std::string const &setting, Condition const &condition) const;
    std::string
    resolve(std::string const &setting) const;

public:
    /*
     * Expand an expression. Any build settings in that expression are evaluated.
     */
    std::string
    expand(Value const &value, Condition const &condition) const;
    std::string
    expand(Value const &value) const;

public:
    /*
     * Computes all values for all settings present in the environment.
     */
    std::unordered_map<std::string, std::string>
    computeValues(Condition const &condition) const;

public:
    /*
     * Adds a level to the environment, at the front (will override any existing
     * values for settings in the level). Default levels are added in a separate
     * group that has its own front and back, and is behind non-default levels.
     */
    void insertFront(Level const &level, bool isDefault);

    /*
     * Adds a level to the environment, at the back (will not override existing
     * values for settings in the level). Default levels are added in a separate
     * group that has its own front and back, and is behind non-default levels.
     */
    void insertBack(Level const &level, bool isDefault);

public:
    /*
     * For debugging: print out the contents of all levels.
     */
    void dump() const;

private:
    struct InheritanceContext {
        bool valid;
        std::string setting;
        std::list<Level>::const_iterator it;
    };
    std::string resolveValue(Condition const &condition, Value const &value, InheritanceContext const &context) const;
    std::string resolveInheritance(Condition const &condition, InheritanceContext const &context) const;
    std::string resolveAssignment(Condition const &condition, std::string const &setting) const;
};

}

#endif  // !__pbxsetting_Environment_h
