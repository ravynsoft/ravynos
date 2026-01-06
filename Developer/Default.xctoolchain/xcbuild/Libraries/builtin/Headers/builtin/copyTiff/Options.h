/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_copyTiff_Options_h
#define __builtin_copyTiff_Options_h

#include <libutil/Options.h>

#include <string>
#include <vector>
#include <utility>

namespace builtin {
namespace copyTiff {

class Options {
private:
    std::vector<std::string>   _inputs;
    ext::optional<std::string> _outputDirectory;

public:
    ext::optional<bool>        _validate;

public:
    ext::optional<std::string> _compressionFormat;

public:
    bool                       _separator;

public:
    Options();
    ~Options();

public:
    std::vector<std::string> const &inputs() const
    { return _inputs; }
    ext::optional<std::string> const &outputDirectory() const
    { return _outputDirectory; }

public:
    ext::optional<bool> validate() const
    { return _validate; }

public:
    ext::optional<std::string> const &compressionFormat() const
    { return _compressionFormat; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}
}

#endif // !__builtin_copyTiff_Options_h
