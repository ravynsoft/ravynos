/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_PBX_BuildPhaseInjection_h
#define __pbxspec_PBX_BuildPhaseInjection_h

#include <plist/Dictionary.h>
#include <pbxsetting/Value.h>

#include <string>
#include <ext/optional>

namespace pbxspec { namespace PBX {

class BuildPhaseInjection {
protected:
    ext::optional<std::string>       _buildPhase;
    ext::optional<std::string>       _name;
    ext::optional<bool>              _runOnlyForDeploymentPostprocessing;
    ext::optional<bool>              _needsRunpathSearchPathForFrameworks;
    ext::optional<int>               _dstSubfolderSpec;
    ext::optional<pbxsetting::Value> _dstPath;

protected:
    friend class FileType;
    friend class ProductType;
    BuildPhaseInjection();

public:
    inline ext::optional<std::string> const &buildPhase() const
    { return _buildPhase; }
    inline ext::optional<std::string> const &name() const
    { return _name; }

public:
    inline bool runOnlyForDeploymentPostprocessing() const
    { return _runOnlyForDeploymentPostprocessing.value_or(false); }
    inline ext::optional<bool> runOnlyForDeploymentPostprocessingOptional() const
    { return _runOnlyForDeploymentPostprocessing; }
    inline bool needsRunpathSearchPathForFrameworks() const
    { return _needsRunpathSearchPathForFrameworks.value_or(false); }
    inline ext::optional<bool> needsRunpathSearchPathForFrameworksOptional() const
    { return _needsRunpathSearchPathForFrameworks; }

public:
    inline ext::optional<int> dstSubfolderSpec() const
    { return _dstSubfolderSpec; }
    inline ext::optional<pbxsetting::Value> const &dstPath() const
    { return _dstPath; }

protected:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__pbxspec_PBX_BuildPhaseInjection_h
