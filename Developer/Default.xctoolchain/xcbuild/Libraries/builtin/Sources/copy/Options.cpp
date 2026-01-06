/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/copy/Options.h>

using builtin::copy::Options;

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

    if (arg == "-V") {
        return libutil::Options::Current<bool>(&_verbose, arg);
    } else if (arg == "-preserve-hfs-data") {
        return libutil::Options::Current<bool>(&_preserveHFSData, arg);
    } else if (arg == "-ignore-missing-inputs") {
        return libutil::Options::Current<bool>(&_ignoreMissingInputs, arg);
    } else if (arg == "-resolve-src-symlinks") {
        return libutil::Options::Current<bool>(&_resolveSrcSymlinks, arg);
    } else if (arg == "-exclude") {
        return libutil::Options::AppendNext<std::string>(&_excludes, args, it);
    } else if (arg == "-strip-debug-symbols") {
        return libutil::Options::Current<bool>(&_stripDebugSymbols, arg);
    } else if (arg == "-strip-tool") {
        return libutil::Options::Next<std::string>(&_stripTool, args, it);
    } else if (arg == "-bitcode-strip") {
        ext::optional<std::string> mode;
        std::pair<bool, std::string> result = libutil::Options::Next<std::string>(&mode, args, it);
        if (result.first) {
            if (*mode == "none") {
                _bitcodeStrip = BitcodeStripMode::None;
            } else if (*mode == "replace-with-marker") {
                _bitcodeStrip = BitcodeStripMode::ReplaceWithMarker;
            } else if (*mode == "all") {
                _bitcodeStrip = BitcodeStripMode::All;
            } else {
                return std::make_pair<bool, std::string>(false, "unknown bitcode strip mode '" + *mode + "'");
            }
        }
        return result;
    } else if (arg == "-bitcode-strip-tool") {
        return libutil::Options::Next<std::string>(&_bitcodeStripTool, args, it);
    } else if (!arg.empty() && arg[0] != '-') {
        if (*it == std::prev(args.end())) {
            return libutil::Options::Current<std::string>(&_output, arg);
        } else {
            return libutil::Options::AppendCurrent<std::string>(&_inputs, arg);
        }
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

