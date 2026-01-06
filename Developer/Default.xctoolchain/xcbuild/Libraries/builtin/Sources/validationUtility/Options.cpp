/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/validationUtility/Options.h>

using builtin::validationUtility::Options;

Options::
Options()
{
}

Options::
~Options()
{
}

std::pair<bool, std::string> Options::
parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
    std::string const &arg = **it;

    if (arg == "-validate-for-store") {
        return libutil::Options::Current<bool>(&_validateForStore, arg);
    } else if (!arg.empty() && arg[0] != '-') {
        return libutil::Options::Current<std::string>(&_input, arg);
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

