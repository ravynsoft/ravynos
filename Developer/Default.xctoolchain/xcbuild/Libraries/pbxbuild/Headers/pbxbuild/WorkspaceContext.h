/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_WorkspaceContext_h
#define __pbxbuild_WorkspaceContext_h

#include <pbxbuild/Base.h>
#include <pbxbuild/DerivedDataHash.h>
#include <pbxsetting/XC/Config.h>

namespace pbxsetting { class Environment; }
namespace libutil { class Filesystem; }

namespace pbxbuild {

/*
 * Abstracts a loaded workspace. Workspaces can be an actual workspace file,
 * or in legacy builds, from just a project file. Workspaces define:
 *
 *  - The available schemes (from inside a .xcworkspace or a .pbxproj).
 *  - All of the available projects (including nested projects).
 *  - The build root, where workspace-relative paths are based.
 *  - Where outputs go in derived data (i.e. OBJROOT).
 */
class WorkspaceContext {
private:
    std::string                                    _basePath;
    DerivedDataHash                                _derivedDataHash;
    xcworkspace::XC::Workspace::shared_ptr         _workspace;
    pbxproj::PBX::Project::shared_ptr              _project;
    std::vector<xcscheme::SchemeGroup::shared_ptr> _schemeGroups;
    std::unordered_map<std::string, pbxproj::PBX::Project::shared_ptr> _projects;
    std::unordered_map<pbxproj::XC::BuildConfiguration::shared_ptr, pbxsetting::XC::Config> _configs;

public:
    WorkspaceContext(
        std::string const &basePath,
        DerivedDataHash const &derivedDataHash,
        xcworkspace::XC::Workspace::shared_ptr const &workspace,
        pbxproj::PBX::Project::shared_ptr const &project,
        std::vector<xcscheme::SchemeGroup::shared_ptr> const &schemeGroups,
        std::unordered_map<std::string, pbxproj::PBX::Project::shared_ptr> const &projects,
        std::unordered_map<pbxproj::XC::BuildConfiguration::shared_ptr, pbxsetting::XC::Config> const &configs);
    ~WorkspaceContext();

public:
    /*
     * The base path of either the .xcworkspace or the legacy .xcodeproj.
     */
    std::string const &basePath() const
    { return _basePath; }

    /*
     * The path inside DerivedData to use when building this workspace.
     */
    DerivedDataHash const &derivedDataHash() const
    { return _derivedDataHash; }

public:
    /*
     * The workspace. May be nullptr if this is a legacy project-only workspace.
     */
    xcworkspace::XC::Workspace::shared_ptr const &workspace() const
    { return _workspace; }

    /*
     * The root project. May be nullptr if this is a real workspace.
     */
    pbxproj::PBX::Project::shared_ptr const &project() const
    { return _project; }

public:
    /*
     * All scheme groups for the workspace itself and any projects in the workspace.
     */
    std::vector<xcscheme::SchemeGroup::shared_ptr> const &schemeGroups() const
    { return _schemeGroups; }

    /*
     * All projects, including the root project, workspace projects, and nested projects.
     */
    std::unordered_map<std::string, pbxproj::PBX::Project::shared_ptr> const &projects() const
    { return _projects; }

    /*
     * All configuration files, including for all loaded projects and targets.
     */
    std::unordered_map<pbxproj::XC::BuildConfiguration::shared_ptr, pbxsetting::XC::Config> const &configs() const
    { return _configs; }

public:
    /*
     * Find a project in the workspace. Could be the root project (for a legacy build), a project
     * referenced by the workspace (with a real workspace), or a nested project in any of those.
     * project referenced by the workspace.
     */
    pbxproj::PBX::Project::shared_ptr
    project(std::string const &projectPath) const;

    /*
     * Find a scheme from inside the workspace. For real workspaces, schemes
     * are searched in the workspace itself as well as projects in the workspace.
     */
    xcscheme::XC::Scheme::shared_ptr
    scheme(std::string const &name) const;

public:
    /*
     * All loaded files in the workspace. Includes files loaded for the workspace itself, any
     * projects inside the workspace, and all schemes in the workspace or projects inside it.
     */
    std::vector<std::string> loadedFilePaths() const;

public:
    /*
     * Creates a workspace context from a real workspace.
     */
    static WorkspaceContext
    Workspace(libutil::Filesystem const *filesystem, std::string const &userName, pbxsetting::Environment const &baseEnvironment, xcworkspace::XC::Workspace::shared_ptr const &workspace);

    /*
     * Creates a workspace context for a legacy project-only build.
     */
    static WorkspaceContext
    Project(libutil::Filesystem const *filesystem, std::string const &userName, pbxsetting::Environment const &baseEnvironment, pbxproj::PBX::Project::shared_ptr const &project);
};

}

#endif // !__pbxbuild_WorkspaceContext_h
