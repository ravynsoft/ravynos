/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/infoPlistUtility/Options.h>

using builtin::infoPlistUtility::Options;

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

    if (arg == "-genpkginfo") {
        return libutil::Options::Next<std::string>(&_genPkgInfo, args, it);
    } else if (arg == "-resourcerulesfile") {
        return libutil::Options::Next<std::string>(&_resourceRulesFile, args, it);
    } else if (arg == "-expandbuildsettings") {
        return libutil::Options::Current<bool>(&_expandBuildSettings, arg);
    } else if (arg == "-format") {
        return libutil::Options::Next<std::string>(&_format, args, it);
    } else if (arg == "-platform") {
        return libutil::Options::Next<std::string>(&_platform, args, it);
    } else if (arg == "-requiredArchitecture") {
        return libutil::Options::AppendNext<std::string>(&_requiredArchitectures, args, it);
    } else if (arg == "-additionalcontentfile") {
        return libutil::Options::AppendNext<std::string>(&_additionalContentFiles, args, it);
    } else if (arg == "-infofilekeys") {
        return libutil::Options::Next<std::string>(&_infoFileKeys, args, it);
    } else if (arg == "-infofilevalues") {
        return libutil::Options::Next<std::string>(&_infoFileValues, args, it);
    } else if (arg == "-o") {
        return libutil::Options::Next<std::string>(&_output, args, it);
    } else if (!arg.empty() && arg[0] != '-') {
        return libutil::Options::Current<std::string>(&_input, arg);
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

