/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_AdditionalOption_h
#define __xcscheme_XC_AdditionalOption_h

#include <memory>
#include <string>
#include <vector>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class AdditionalOption {
public:
    typedef std::shared_ptr <AdditionalOption> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    bool        _isEnabled;
    std::string _key;
    std::string _value;

public:
    AdditionalOption();

public:
    inline bool isEnabled() const
    { return _isEnabled; }

public:
    inline std::string const &key() const
    { return _key; }
    inline std::string const &value() const
    { return _value; }

public:
    inline std::pair <std::string, std::string> pair() const
    { return std::make_pair(key(), value()); }

public:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcscheme_XC_AdditionalOption_h
