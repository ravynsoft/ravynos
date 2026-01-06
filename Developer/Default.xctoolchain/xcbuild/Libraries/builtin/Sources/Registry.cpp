/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/Registry.h>
#include <builtin/copy/Driver.h>
#include <builtin/copyPlist/Driver.h>
#include <builtin/copyStrings/Driver.h>
#include <builtin/copyTiff/Driver.h>
#include <builtin/infoPlistUtility/Driver.h>
#include <builtin/lsRegisterURL/Driver.h>
#include <builtin/productPackagingUtility/Driver.h>
#include <builtin/validationUtility/Driver.h>
#include <builtin/embeddedBinaryValidationUtility/Driver.h>

using builtin::Registry;
using builtin::Driver;

Registry::
Registry(std::unordered_map<std::string, std::shared_ptr<Driver>> const &drivers) :
    _drivers(drivers)
{
}

Registry::
~Registry()
{
}

std::shared_ptr<Driver> Registry::
driver(std::string const &name)
{
    auto const &it = _drivers.find(name);
    if (it != _drivers.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

Registry Registry::
Create(std::vector<std::shared_ptr<Driver>> const &drivers)
{
    std::unordered_map<std::string, std::shared_ptr<Driver>> driverMap;
    for (std::shared_ptr<Driver> const &driver : drivers) {
        driverMap.insert({ driver->name(), driver });
    }
    return Registry(driverMap);
}

Registry Registry::
Default()
{
    return Create({
        std::make_shared<builtin::copy::Driver>(),
        std::make_shared<builtin::copyPlist::Driver>(),
        std::make_shared<builtin::copyStrings::Driver>(),
        std::make_shared<builtin::copyTiff::Driver>(),
        std::make_shared<builtin::infoPlistUtility::Driver>(),
        std::make_shared<builtin::lsRegisterURL::Driver>(),
        std::make_shared<builtin::productPackagingUtility::Driver>(),
        std::make_shared<builtin::validationUtility::Driver>(),
        std::make_shared<builtin::embeddedBinaryValidationUtility::Driver>(),
    });
}
