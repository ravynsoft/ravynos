/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_copy_Options_h
#define __builtin_copy_Options_h

#include <libutil/Options.h>

#include <string>
#include <vector>
#include <utility>

namespace builtin {
namespace copy {

class Options {
public:
    enum class BitcodeStripMode {
        None,
        ReplaceWithMarker,
        All,
    };

private:
    ext::optional<bool>             _verbose;
    ext::optional<bool>             _preserveHFSData;

private:
    std::vector<std::string>        _inputs;
    ext::optional<bool>             _ignoreMissingInputs;
    ext::optional<bool>             _resolveSrcSymlinks;
    ext::optional<std::string>      _output;
    std::vector<std::string>        _excludes;

private:
    ext::optional<bool>             _stripDebugSymbols;
    ext::optional<std::string>      _stripTool;
    ext::optional<BitcodeStripMode> _bitcodeStrip;
    ext::optional<std::string>      _bitcodeStripTool;

public:
    Options();
    ~Options();

public:
    bool verbose() const
    { return _verbose.value_or(false); }
    bool preserveHFSData() const
    { return _preserveHFSData.value_or(false); }

public:
    std::vector<std::string> const &inputs() const
    { return _inputs; }
    bool ignoreMissingInputs() const
    { return _ignoreMissingInputs.value_or(false); }
    bool resolveSrcSymlinks() const
    { return _resolveSrcSymlinks.value_or(false); }
    ext::optional<std::string> const &output() const
    { return _output; }
    std::vector<std::string> const &excludes() const
    { return _excludes; }

public:
    bool stripDebugSymbols() const
    { return _stripDebugSymbols.value_or(false); }
    ext::optional<std::string> const &stripTool() const
    { return _stripTool; }
    ext::optional<BitcodeStripMode> bitcodeStrip() const
    { return _bitcodeStrip; }
    ext::optional<std::string> const &bitcodeStripTool() const
    { return _bitcodeStripTool; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}
}

#endif // !__builtin_copy_Options_h
