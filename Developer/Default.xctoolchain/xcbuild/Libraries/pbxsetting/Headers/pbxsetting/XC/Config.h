/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxsetting_XC_Config_h
#define __pbxsetting_XC_Config_h

#include <pbxsetting/Environment.h>
#include <pbxsetting/Level.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Value.h>

#include <memory>
#include <vector>
#include <ext/optional>

namespace libutil { class Filesystem; }

namespace pbxsetting { namespace XC {

class Config {
public:
    /*
     * The type of value in the config.
     */
    class Entry {
    public:
        enum class Type {
            /*
             * A setting definition.
             */
            Setting,
            /*
             * An included config.
             */
            Include,
        };

    private:
        Type                    _type;

    private:
        ext::optional<Setting>  _setting;

    private:
        ext::optional<Value>    _path;
        std::shared_ptr<Config> _config;

    public:
        Entry(Setting const &setting);
        Entry(Value const &path, std::shared_ptr<Config> const &config);

    public:
        /*
         * The type of entry.
         */
        Type type() const
        { return _type; }

    public:
        /*
         * A definition of a build setting.
         */
        ext::optional<Setting> const &setting() const
        { return _setting; }

    public:
        /*
         * The included path.
         */
        ext::optional<Value> const &path() const
        { return _path; }

        /*
         * The included config.
         */
        std::shared_ptr<Config> const &config() const
        { return _config; }
    };

private:
    std::string        _path;
    std::vector<Entry> _contents;

public:
    Config(std::string const &path, std::vector<Entry> const &contents);
    ~Config();

public:
    /*
     * The full path to the config file.
     */
    std::string const &path() const
    { return _path; }

    /*
     * The contents of the config.
     */
    std::vector<Entry> const &contents() const
    { return _contents; }

public:
    /*
     * The settings defined by the config file.
     */
    Level level() const;

public:
    /*
     * Load a config from a file in a filesystem.
     */
    static ext::optional<Config>
    Load(libutil::Filesystem const *filesystem, Environment const &environment, std::string const &path);
};

} }

#endif  // !__pbxsetting_XC_Config_h
