/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <dependency/DependencyInfo.h>

using dependency::DependencyInfo;

DependencyInfo::
DependencyInfo(std::vector<std::string> const &inputs, std::vector<std::string> const &outputs) :
    _inputs (inputs),
    _outputs(outputs)
{
}

DependencyInfo::
DependencyInfo() :
    DependencyInfo(std::vector<std::string>(), std::vector<std::string>())
{
}
