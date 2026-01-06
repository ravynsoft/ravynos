/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Build_Context_h
#define __pbxbuild_Build_Context_h

#include <pbxbuild/Base.h>
#include <pbxbuild/WorkspaceContext.h>
#include <pbxbuild/Build/Environment.h>
#include <pbxbuild/Target/Environment.h>

#include <ext/optional>

namespace pbxbuild {
namespace Build {

/*
 * Represents the options and inputs for a specific build. It holds the
 * workspace, scheme, action, and configuration for the build, in addition
 * to custom options that can be passed into certain builds.
 */
class Context {
private:
    WorkspaceContext                  _workspaceContext;
    xcscheme::XC::Scheme::shared_ptr  _scheme;
    xcscheme::SchemeGroup::shared_ptr _schemeGroup;
    std::string                       _action;
    std::string                       _configuration;
    bool                              _defaultConfiguration;
    std::vector<pbxsetting::Level>    _overrideLevels;

private:
    std::shared_ptr<std::unordered_map<pbxproj::PBX::Target::shared_ptr, Target::Environment>> _targetEnvironments;

public:
    Context(
        WorkspaceContext const &workspaceContext,
        xcscheme::XC::Scheme::shared_ptr const &scheme,
        xcscheme::SchemeGroup::shared_ptr const &schemeGroup,
        std::string const &action,
        std::string const &configuration,
        bool defaultConfiguration,
        std::vector<pbxsetting::Level> const &overrideLevels);

public:
    /*
     * The workspace this build will take place in.
     */
    WorkspaceContext const &workspaceContext() const
    { return _workspaceContext; }

    /*
     * The scheme the build will use.
     */
    xcscheme::XC::Scheme::shared_ptr const &scheme() const
    { return _scheme; }

    /*
     * The group containing the active scheme.
     */
    xcscheme::SchemeGroup::shared_ptr const &schemeGroup() const
    { return _schemeGroup; }

public:
    /*
     * The action the build is for.
     */
    std::string const &action() const
    { return _action; }

    /*
     * The configuration being built.
     */
    std::string const &configuration() const
    { return _configuration; }

    /*
     * If the specified configuration is a default configuration, or was
     * directly specified.
     */
    bool defaultConfiguration() const
    { return _defaultConfiguration; }

public:
    /*
     * Build setting levels to use as overrides for all targets in the build.
     */
    std::vector<pbxsetting::Level> const &overrideLevels() const
    { return _overrideLevels; }

public:
    /*
     * The base set of build settings from the full context.
     */
    pbxsetting::Level baseSettings(void) const;

    /*
     * The build settings implied by the specified action.
     */
    pbxsetting::Level actionSettings(void) const;

public:
    /*
     * Create or fetch a target's computed environment.
     */
    ext::optional<Target::Environment>
    targetEnvironment(Build::Environment const &buildEnvironment, pbxproj::PBX::Target::shared_ptr const &target) const;

public:
    /*
     * Finds a target by identifier within a project.
     */
    pbxproj::PBX::Target::shared_ptr
    resolveTargetIdentifier(pbxproj::PBX::Project::shared_ptr const &project, std::string const &identifier) const;

    /*
     * Finds a target by the identifier of its product within a project.
     */
    ext::optional<std::pair<pbxproj::PBX::Target::shared_ptr, pbxproj::PBX::FileReference::shared_ptr>>
    resolveProductIdentifier(pbxproj::PBX::Project::shared_ptr const &project, std::string const &identifier) const;
};

}
}

#endif // !__pbxbuild_Build_Context_h
