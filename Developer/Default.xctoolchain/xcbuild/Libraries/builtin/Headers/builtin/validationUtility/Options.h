/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_validationUtility_Options_h
#define __builtin_validationUtility_Options_h

#include <libutil/Options.h>

#include <string>
#include <vector>
#include <utility>

namespace builtin {
namespace validationUtility {

class Options {
private:
    ext::optional<std::string> _input;
    ext::optional<std::string> _infoPlistPath;
    ext::optional<bool>        _validateForStore;

public:
    Options();
    ~Options();

public:
    ext::optional<std::string> const &input() const
    { return _input; }
    ext::optional<std::string> const &infoPlistPath() const
    { return _infoPlistPath; }
    bool validateForStore() const
    { return _validateForStore.value_or(false); }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}
}

#endif // !__builtin_validationUtility_Options_h
