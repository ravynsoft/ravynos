/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_productPackagingUtility_Options_h
#define __builtin_productPackagingUtility_Options_h

#include <libutil/Options.h>

#include <string>
#include <vector>
#include <utility>

namespace builtin {
namespace productPackagingUtility {

class Options {
private:
    ext::optional<std::string> _input;
    ext::optional<std::string> _output;

private:
    ext::optional<bool>        _removeFile;
    ext::optional<bool>        _entitlements;
    ext::optional<bool>        _resourceRules;

private:
    ext::optional<std::string> _format;

public:
    Options();
    ~Options();

public:
    ext::optional<std::string> const &input() const
    { return _input; }
    ext::optional<std::string> const &output() const
    { return _output; }

private:
    bool removeFile() const
    { return _removeFile.value_or(false); }
    bool entitlements() const
    { return _entitlements.value_or(false); }

public:
    ext::optional<std::string> const &format() const
    { return _format; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}
}

#endif // !__builtin_productPackagingUtility_Options_h
