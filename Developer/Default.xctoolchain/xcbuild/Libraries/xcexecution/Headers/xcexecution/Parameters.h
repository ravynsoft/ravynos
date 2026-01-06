/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcexecution_Parameters_h
#define __xcexecution_Parameters_h

#include <pbxsetting/Level.h>
#include <pbxbuild/Build/Context.h>
#include <pbxbuild/DirectedGraph.h>
#include <pbxbuild/WorkspaceContext.h>

#include <string>
#include <vector>
#include <ext/optional>

namespace libutil { class Filesystem; }

namespace xcexecution {

/*
 * All of the inputs for a build. Can load the workspace on-demand and create
 * a build context for a build action. This data is kept at this level to avoid
 * loading the workspace when the parameters have not changed to an executor.
 */
class Parameters {
private:
    ext::optional<std::string>     _workspace;
    ext::optional<std::string>     _project;
    ext::optional<std::string>     _scheme;
    ext::optional<std::vector<std::string>> _target;
    bool                           _allTargets;
    std::vector<std::string>       _actions;
    ext::optional<std::string>     _configuration;
    std::vector<pbxsetting::Level> _overrideLevels;

public:
    Parameters(
        ext::optional<std::string> const &workspace,
        ext::optional<std::string> const &project,
        ext::optional<std::string> const &scheme,
        ext::optional<std::vector<std::string>> const &target,
        bool allTargets,
        std::vector<std::string> const &actions,
        ext::optional<std::string> const &configuration,
        std::vector<pbxsetting::Level> const &overrideLevels);

public:
    /*
     * The workspace to build.
     */
    ext::optional<std::string> const &workspace() const
    { return _workspace; }

    /*
     * The project to build.
     */
    ext::optional<std::string> const &project() const
    { return _project; }

    /*
     * The scheme to build.
     */
    ext::optional<std::string> const &scheme() const
    { return _scheme; }

    /*
     * The target to build.
     */
    ext::optional<std::vector<std::string>> const &target() const
    { return _target; }

    /*
     * Build all targets.
     */
    bool allTargets() const
    { return _allTargets; }

    /*
     * The specified actions to build.
     */
    std::vector<std::string> const &actions() const
    { return _actions; }

    /*
     * The configuration to build.
     */
    ext::optional<std::string> const &configuration() const
    { return _configuration; }

    /*
     * Build setting overrides.
     */
    std::vector<pbxsetting::Level> const &overrideLevels() const
    { return _overrideLevels; }

public:
    /*
     * The canonical set of arguments to reproduce these parameters.
     */
    std::vector<std::string> canonicalArguments() const;

    /*
     * A stable hash of the parameters. Useful as a cache key.
     */
    std::string canonicalHash() const;

public:
    /*
     * Loads the workspace from the build parameters.
     */
    ext::optional<pbxbuild::WorkspaceContext> loadWorkspace(
        libutil::Filesystem const *filesystem,
        std::string const &userName,
        pbxbuild::Build::Environment const &buildEnvironment,
        std::string const &workingDirectory) const;

    /*
     * Creates the build context for a specific action.
     */
    ext::optional<pbxbuild::Build::Context> createBuildContext(
        pbxbuild::WorkspaceContext const &workspaceContext) const;

    /*
     * Resolve inter-target dependencies.
     */
    ext::optional<pbxbuild::DirectedGraph<pbxproj::PBX::Target::shared_ptr>>
    resolveDependencies(
        pbxbuild::Build::Environment const &buildEnvironment,
        pbxbuild::Build::Context const &buildContext) const;
};

}

#endif // !__xcexecution_Parameters_h
