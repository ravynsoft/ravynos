/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/productPackagingUtility/Options.h>

using builtin::productPackagingUtility::Options;

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

    if (arg == "-removefile") {
        return libutil::Options::Current<bool>(&_removeFile, arg);
    } else if (arg == "-entitlements") {
        return libutil::Options::Current<bool>(&_entitlements, arg);
    } else if (arg == "-resourcerules") {
        return libutil::Options::Current<bool>(&_resourceRules, arg);
    } else if (arg == "-o") {
        return libutil::Options::Next<std::string>(&_output, args, it);
    } else if (!arg.empty() && arg[0] != '-') {
        return libutil::Options::Current<std::string>(&_input, arg);
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

