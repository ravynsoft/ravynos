/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Keys/Unpack.h>

using plist::Keys::Unpack;
using plist::Object;
using plist::Dictionary;

Unpack::
Unpack(std::string const &name, Dictionary const *dict, std::unordered_set<std::string> *seen) :
    _name(name),
    _dict(dict),
    _seen(seen)
{
}

Object const *Unpack::
value(std::string const &key)
{
    _seen->insert(key);
    return _dict->value(key);
}

bool Unpack::
complete(bool check)
{
#if DEBUG_UNHANDLED_KEYS
    if (check) {
        for (size_t n = 0; n < _dict->count(); n++) {
            std::string const &key = _dict->key(n);
            if (_seen->find(key) == _seen->end()) {
                _errors.push_back("unhandled " + _name + " key " + key);
            }
        }
    }
#endif

    return _errors.empty();
}

std::string Unpack::
errorText() const
{
    std::string result;
    for (std::string const &error : _errors) {
        result += "warning: ";
        result += error;
        result += "\n";
    }
    return result;
}

