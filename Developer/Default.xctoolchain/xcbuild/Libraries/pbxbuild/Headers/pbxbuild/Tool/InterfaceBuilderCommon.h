/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_InterfaceBuilderCommon_h
#define __pbxbuild_Tool_InterfaceBuilderCommon_h

#include <pbxsetting/Setting.h>

#include <string>
#include <vector>

namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Tool {

class InterfaceBuilderCommon {
private:
    InterfaceBuilderCommon();
    ~InterfaceBuilderCommon();

public:
    /*
     * List device names for the given platform and device family.
     */
    static std::vector<std::string>
    TargetedDeviceNames(std::string const &platformName, std::string const &deviceFamily);

    /*
     * The standard setting for the targeted devices.
     */
    static pbxsetting::Setting
    TargetedDeviceSetting(pbxsetting::Environment const &environment);

public:
    /*
     * The arguments needed to add a deployment target.
     */
    static std::vector<std::string>
    DeploymentTargetArguments(pbxsetting::Environment const &environment);
};

}
}

#endif // !__pbxbuild_Tool_InterfaceBuilderCommon_h
