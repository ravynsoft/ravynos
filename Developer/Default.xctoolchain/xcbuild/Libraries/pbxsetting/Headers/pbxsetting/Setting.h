/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxsetting_Setting_h
#define __pbxsetting_Setting_h

#include <pbxsetting/Condition.h>
#include <pbxsetting/Value.h>

#include <string>
#include <ext/optional>

namespace pbxsetting {

/*
 * Represents the binding of a build setting to a particular value.
 */
class Setting {
private:
    std::string _name;
    Condition   _condition;
    Value       _value;

public:
    Setting(std::string const &name, Condition const &condition, Value const &value);
    ~Setting();

public:
    /*
     * The name of the setting being set.
     */
    std::string const &name() const
    { return _name; }

    /*
     * The conditional options set on this build setting binding.
     */
    Condition const &condition() const
    { return _condition; }

    /*
     * The build setting expression this setting expands to.
     */
    Value const &value() const
    { return _value; }

public:
    /*
     * Determines if this setting matches the parameters. Condition
     * evaluation goes through `Condition::match()`.
     */
    bool
    match(std::string const &name, Condition const &condition) const;

public:
    /*
     * Creates a build setting binding. The value can be an
     * arbitrary build setting expression; see `Value`.
     */
    static Setting
    Create(std::string const &key, Value const &value);

    /*
     * Creates a build setting binding. The value is converted
     * to a value as a string literal, not as an expression.
     */
    static Setting
    Create(std::string const &key, std::string const &value);

public:
    /*
     * Parses xcconfig-style syntax for a build setting:
     *
     *     SETTING_NAME = VALUE
     *
     * Note the setting value supports uses `Value::Parse()`.
     */
    static ext::optional<Setting>
    Parse(std::string const &string);

    /*
     * Creates a setting from the name and by parsing the value.
     * using `Value::Parse()`. Mostly for convenience.
     */
    static Setting
    Parse(std::string const &key, std::string const &value);
};

}

#endif  // !__pbxsetting_Setting_h
