/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/SpecificationType.h>

using pbxspec::SpecificationType;
using pbxspec::SpecificationTypes;

std::string SpecificationTypes::
Name(SpecificationType type)
{
    switch (type) {
        case SpecificationType::Architecture:
            return "Architecture";
        case SpecificationType::BuildPhase:
            return "BuildPhase";
        case SpecificationType::BuildSettings:
            return "BuildSettings";
        case SpecificationType::BuildStep:
            return "BuildStep";
        case SpecificationType::BuildSystem:
            return "BuildSystem";
        case SpecificationType::Compiler:
            return "Compiler";
        case SpecificationType::FileType:
            return "FileType";
        case SpecificationType::Linker:
            return "Linker";
        case SpecificationType::PackageType:
            return "PackageType";
        case SpecificationType::ProductType:
            return "ProductType";
        case SpecificationType::Tool:
            return "Tool";
    }

    abort();
}

ext::optional<SpecificationType> SpecificationTypes::
Parse(std::string const &name)
{
    if (name == "Architecture") {
        return SpecificationType::Architecture;
    } else if (name == "BuildPhase") {
        return SpecificationType::BuildPhase;
    } else if (name == "BuildSettings") {
        return SpecificationType::BuildSettings;
    } else if (name == "BuildStep") {
        return SpecificationType::BuildStep;
    } else if (name == "BuildSystem") {
        return SpecificationType::BuildSystem;
    } else if (name == "Compiler") {
        return SpecificationType::Compiler;
    } else if (name == "FileType") {
        return SpecificationType::FileType;
    } else if (name == "Linker") {
        return SpecificationType::Linker;
    } else if (name == "PackageType") {
        return SpecificationType::PackageType;
    } else if (name == "ProductType") {
        return SpecificationType::ProductType;
    } else if (name == "Tool") {
        return SpecificationType::Tool;
    }

    return ext::nullopt;
}

