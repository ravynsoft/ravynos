/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/InterfaceBuilderCommon.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Type.h>

namespace Tool = pbxbuild::Tool;

std::vector<std::string> Tool::InterfaceBuilderCommon::
TargetedDeviceNames(std::string const &platformName, std::string const &deviceFamily)
{
    if (platformName == "macosx") {
        return { "mac" };
    } else {
        std::vector<std::string> targetedDeviceNames;

        std::string::size_type off = 0;
        do {
            std::string::size_type noff = deviceFamily.find(',', off);
            std::string entry = (noff == std::string::npos ? deviceFamily.substr(off) : deviceFamily.substr(off, noff));

            if (!entry.empty()) {
                if (entry == "1") {
                    targetedDeviceNames.push_back("iphone");
                } else if (entry == "2") {
                    targetedDeviceNames.push_back("ipad");
                } else if (entry == "3") {
                    targetedDeviceNames.push_back("tv");
                } else if (entry == "4") {
                    targetedDeviceNames.push_back("watch");
                }
            }

            off = noff;
        } while ((off != std::string::npos) && (off++ < deviceFamily.size()));

        return targetedDeviceNames;
    }
}

pbxsetting::Setting Tool::InterfaceBuilderCommon::
TargetedDeviceSetting(pbxsetting::Environment const &environment)
{
    /*
     * Determine the target devices from the environment.
     */
    std::vector<std::string> targetDeviceNames = InterfaceBuilderCommon::TargetedDeviceNames(
        environment.resolve("PLATFORM_NAME"),
        environment.resolve("TARGETED_DEVICE_FAMILY"));

    return pbxsetting::Setting::Create("RESOURCES_TARGETED_DEVICE_FAMILY", pbxsetting::Type::FormatList(targetDeviceNames));
}

std::vector<std::string> Tool::InterfaceBuilderCommon::
DeploymentTargetArguments(pbxsetting::Environment const &environment)
{
    std::string deploymentTarget = environment.resolve(environment.resolve("DEPLOYMENT_TARGET_SETTING_NAME"));
    if (deploymentTarget.empty()) {
        return std::vector<std::string>();
    } else {
        return { "--minimum-deployment-target", deploymentTarget };
    }
}

