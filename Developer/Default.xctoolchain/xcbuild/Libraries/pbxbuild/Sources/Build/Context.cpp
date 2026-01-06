/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Build/Context.h>

namespace Build = pbxbuild::Build;
namespace Target = pbxbuild::Target;
using pbxbuild::WorkspaceContext;

Build::Context::
Context(
    WorkspaceContext const &workspaceContext,
    xcscheme::XC::Scheme::shared_ptr const &scheme,
    xcscheme::SchemeGroup::shared_ptr const &schemeGroup,
    std::string const &action,
    std::string const &configuration,
    bool defaultConfiguration,
    std::vector<pbxsetting::Level> const &overrideLevels
) :
    _workspaceContext    (workspaceContext),
    _scheme              (scheme),
    _schemeGroup         (schemeGroup),
    _action              (action),
    _configuration       (configuration),
    _defaultConfiguration(defaultConfiguration),
    _overrideLevels      (overrideLevels),
    _targetEnvironments  (std::make_shared<std::unordered_map<pbxproj::PBX::Target::shared_ptr, Target::Environment>>())
{
}

ext::optional<pbxbuild::Target::Environment> Build::Context::
targetEnvironment(Build::Environment const &buildEnvironment, pbxproj::PBX::Target::shared_ptr const &target) const
{
    auto TEI = _targetEnvironments->find(target);
    if (TEI != _targetEnvironments->end()) {
        return TEI->second;
    } else {
        ext::optional<Target::Environment> targetEnvironment = Target::Environment::Create(buildEnvironment, *this, target);
        if (targetEnvironment) {
            _targetEnvironments->insert(std::make_pair(target, *targetEnvironment));
        }
        return targetEnvironment;
    }
}

pbxproj::PBX::Target::shared_ptr Build::Context::
resolveTargetIdentifier(pbxproj::PBX::Project::shared_ptr const &project, std::string const &identifier) const
{
    if (project == nullptr) {
        return nullptr;
    }

    pbxproj::PBX::Target::shared_ptr foundTarget = nullptr;
    for (pbxproj::PBX::Target::shared_ptr const &target : project->targets()) {
        if (target->blueprintIdentifier() == identifier) {
            return target;
        }
    }

    return nullptr;
}

ext::optional<std::pair<pbxproj::PBX::Target::shared_ptr, pbxproj::PBX::FileReference::shared_ptr>> Build::Context::
resolveProductIdentifier(pbxproj::PBX::Project::shared_ptr const &project, std::string const &identifier) const
{
    if (project == nullptr) {
        return ext::nullopt;
    }

    pbxproj::PBX::Target::shared_ptr foundTarget = nullptr;
    for (pbxproj::PBX::Target::shared_ptr const &target : project->targets()) {
        if (target->type() == pbxproj::PBX::Target::Type::Native) {
            pbxproj::PBX::NativeTarget::shared_ptr nativeTarget = std::static_pointer_cast<pbxproj::PBX::NativeTarget>(target);
            if (nativeTarget->productReference() != nullptr && nativeTarget->productReference()->blueprintIdentifier() == identifier) {
                return std::make_pair(target, nativeTarget->productReference());
            }
        }
    }

    return ext::nullopt;
}

pbxsetting::Level Build::Context::
actionSettings(void) const
{
    std::vector<pbxsetting::Setting> settings = {
        pbxsetting::Setting::Create("ACTION", _action),
        pbxsetting::Setting::Create("BUILD_COMPONENTS", "headers build"), // TODO(grp): Should depend on action.
        pbxsetting::Setting::Create("CONFIGURATION", _configuration),
    };

    DerivedDataHash const &derivedDataHash = _workspaceContext.derivedDataHash();
    std::vector<pbxsetting::Setting> derivedDataSettings = derivedDataHash.overrideSettings();
    settings.insert(settings.end(), derivedDataSettings.begin(), derivedDataSettings.end());

    return pbxsetting::Level(settings);
}

pbxsetting::Level Build::Context::
baseSettings(void) const
{
    return pbxsetting::Level({
        pbxsetting::Setting::Parse("CONFIGURATION_BUILD_DIR", "$(BUILD_DIR)/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)"),
        pbxsetting::Setting::Parse("CONFIGURATION_TEMP_DIR", "$(PROJECT_TEMP_DIR)/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)"),
    });
}

