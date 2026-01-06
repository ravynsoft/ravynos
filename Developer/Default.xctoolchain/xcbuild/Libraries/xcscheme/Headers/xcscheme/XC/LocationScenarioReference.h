/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_LocationScenarioReference_h
#define __xcscheme_XC_LocationScenarioReference_h

#ifdef __linux__
#include <cstdint>
#endif
#include <memory>
#include <string>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class LocationScenarioReference {
public:
    typedef std::shared_ptr <LocationScenarioReference> shared_ptr;

private:
    std::string _identifier;
    uint32_t    _referenceType;

public:
    LocationScenarioReference();

public:
    inline std::string const &identifier() const
    { return _identifier; }

public:
    inline uint32_t referenceType() const
    { return _referenceType; }

public:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcscheme_XC_LocationScenarioReference_h
