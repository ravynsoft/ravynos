/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_SpecificationType_h
#define __pbxspec_SpecificationType_h

#include <string>
#include <ext/optional>

namespace pbxspec {

enum class SpecificationType {
    Architecture,
    BuildPhase,
    BuildSettings,
    BuildStep,
    BuildSystem,
    Compiler,
    FileType,
    Linker,
    PackageType,
    ProductType,
    Tool,
};

class SpecificationTypes {
private:
    SpecificationTypes();
    ~SpecificationTypes();

public:
    static std::string Name(SpecificationType type);
    static ext::optional<SpecificationType> Parse(std::string const &name);
};

}

#endif  // !__pbxspec_SpecificationType_h
