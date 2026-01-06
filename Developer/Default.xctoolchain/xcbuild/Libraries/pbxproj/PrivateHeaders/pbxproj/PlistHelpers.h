/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_JSHelpers_h
#define __pbxproj_JSHelpers_h

#include <string>

namespace plist { class Dictionary; }
namespace plist { namespace Keys { class Unpack; } }

namespace pbxproj {

plist::Dictionary const *
PlistDictionaryGetPBXObject(plist::Dictionary const *dict,
                            std::string const &key,
                            std::string const &isa);

plist::Dictionary const *
PlistDictionaryGetIndirectPBXObject(plist::Dictionary const *objects,
                                    plist::Keys::Unpack *unpack,
                                    std::string const &key,
                                    std::string const &isa,
                                    std::string *id);

}

#endif  // !__pbxproj_JSHelpers_h
