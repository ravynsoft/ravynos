/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxsetting_Condition_h
#define __pbxsetting_Condition_h

#include <string>
#include <unordered_map>

namespace pbxsetting {

class Condition {
public:
private:
    std::unordered_map<std::string, std::string> _values;

public:
    Condition(std::unordered_map<std::string, std::string> const &values);
    ~Condition();

public:
    friend struct std::hash<Condition>;

public:
    std::unordered_map<std::string, std::string> const &
    values() const { return _values; }

public:
    bool
    match(Condition const &condition) const;

public:
    static Condition const &
    Empty(void);
};

}

namespace std {
template<>
struct hash<pbxsetting::Condition> {
    size_t operator()(pbxsetting::Condition const &condition) const {
        size_t hash = 0;
        for (auto pair : condition._values) {
            hash ^= std::hash<std::string>()(pair.first);
            hash ^= std::hash<std::string>()(pair.second);
        }
        return hash;
    }
};
}

#endif  // !__pbxsetting_Condition_h
