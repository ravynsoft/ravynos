/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_Test_h
#define __xcscheme_XC_Test_h

#include <memory>
#include <string>
#include <vector>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class Test {
public:
    typedef std::shared_ptr <Test> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string _identifier;

public:
    Test();

public:
    inline std::string const &identifier() const
    { return _identifier; }

public:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcscheme_XC_Test_h
