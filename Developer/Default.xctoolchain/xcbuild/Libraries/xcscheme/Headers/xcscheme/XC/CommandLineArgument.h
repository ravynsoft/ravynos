/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_CommandLineArgument_h
#define __xcscheme_XC_CommandLineArgument_h

#include <memory>
#include <string>
#include <vector>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class CommandLineArgument {
public:
    typedef std::shared_ptr <CommandLineArgument> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    bool        _isEnabled;
    std::string _argument;

public:
    CommandLineArgument();

public:
    inline bool isEnabled() const
    { return _isEnabled; }

public:
    inline std::string const &argument() const
    { return _argument; }

public:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcscheme_XC_CommandLineArgument_h
