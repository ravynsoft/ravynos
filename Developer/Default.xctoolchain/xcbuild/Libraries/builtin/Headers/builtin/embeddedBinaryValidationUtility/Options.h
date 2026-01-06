/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_embeddedBinaryValidationUtility_Options_h
#define __builtin_embeddedBinaryValidationUtility_Options_h

#include <libutil/Options.h>

#include <string>
#include <vector>
#include <utility>

namespace builtin {
namespace embeddedBinaryValidationUtility {

class Options {
private:
    ext::optional<std::string> _input;
    ext::optional<std::string> _signingCert;
    ext::optional<std::string> _infoPlistPath;

public:
    Options();
    ~Options();

public:
    ext::optional<std::string> const &input() const
    { return _input; }
    ext::optional<std::string> const &signingCert() const
    { return _signingCert; }
    ext::optional<std::string> const &inputPlistPath() const
    { return _infoPlistPath; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}
}

#endif // !__builtin_embeddedBinaryValidationUtility_Options_h
