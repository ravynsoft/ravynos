/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_Registry_h
#define __builtin_Registry_h

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace builtin {

class Driver;

class Registry {
private:
    std::unordered_map<std::string, std::shared_ptr<Driver>> _drivers;

public:
    Registry(std::unordered_map<std::string, std::shared_ptr<Driver>> const &drivers);
    ~Registry();

public:
    std::shared_ptr<Driver>
    driver(std::string const &name);

public:
    static Registry
    Create(std::vector<std::shared_ptr<Driver>> const &drivers);
    static Registry
    Default();
};

}

#endif // !__builtin_Registry_h
