/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_infoPlistUtility_Options_h
#define __builtin_infoPlistUtility_Options_h

#include <libutil/Options.h>

#include <string>
#include <vector>
#include <utility>

namespace builtin {
namespace infoPlistUtility {

class Options {
private:
    ext::optional<std::string> _input;
    std::vector<std::string>   _additionalContentFiles;
    ext::optional<std::string> _output;

private:
    ext::optional<std::string> _format;
    ext::optional<bool>        _expandBuildSettings;

private:
    ext::optional<std::string> _platform;
    std::vector<std::string>   _requiredArchitectures;

private:
    ext::optional<std::string> _genPkgInfo;
    ext::optional<std::string> _resourceRulesFile;

private:
    ext::optional<std::string> _infoFileKeys;
    ext::optional<std::string> _infoFileValues;

public:
    Options();
    ~Options();

public:
    ext::optional<std::string> const &input() const
    { return _input; }
    std::vector<std::string> const &additionalContentFiles() const
    { return _additionalContentFiles; }
    ext::optional<std::string> const &output() const
    { return _output; }

public:
    ext::optional<std::string> const &format() const
    { return _format; }
    bool expandBuildSettings() const
    { return _expandBuildSettings.value_or(false); }

public:
    ext::optional<std::string> const &platform() const
    { return _platform; }
    std::vector<std::string> const &requiredArchitectures() const
    { return _requiredArchitectures; }

public:
    ext::optional<std::string> const &genPkgInfo() const
    { return _genPkgInfo; }
    ext::optional<std::string> const &resourceRulesFile() const
    { return _resourceRulesFile; }

public:
    ext::optional<std::string> const &infoFileKeys() const
    { return _infoFileKeys; }
    ext::optional<std::string> const &infoFileValues() const
    { return _infoFileValues; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}
}

#endif // !__builtin_infoPlistUtility_Options_h
