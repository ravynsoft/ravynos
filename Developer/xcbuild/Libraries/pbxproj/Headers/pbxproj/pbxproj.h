/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_pbxproj_h
#define __pbxproj_pbxproj_h

#ifdef __linux__
#include <cstdint>
#include <cstdlib>
#endif

#include <pbxproj/PBX/AggregateTarget.h>
#include <pbxproj/PBX/BuildFile.h>
#include <pbxproj/PBX/BuildPhase.h>
#include <pbxproj/PBX/BuildPhases.h>
#include <pbxproj/PBX/BuildRule.h>
#include <pbxproj/PBX/ContainerItemProxy.h>
#include <pbxproj/PBX/FileReference.h>
#include <pbxproj/PBX/ReferenceProxy.h>
#include <pbxproj/PBX/Group.h>
#include <pbxproj/PBX/LegacyTarget.h>
#include <pbxproj/PBX/NativeTarget.h>
#include <pbxproj/PBX/Project.h>
#include <pbxproj/PBX/TargetDependency.h>
#include <pbxproj/PBX/VariantGroup.h>

#include <pbxproj/XC/BuildConfiguration.h>
#include <pbxproj/XC/ConfigurationList.h>
#include <pbxproj/XC/VersionGroup.h>

#endif  // !__pbxproj_pbxproj_h
