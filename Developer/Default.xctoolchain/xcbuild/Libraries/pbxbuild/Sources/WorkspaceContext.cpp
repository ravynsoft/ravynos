/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/WorkspaceContext.h>
#include <pbxsetting/Environment.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

using pbxbuild::WorkspaceContext;
using pbxbuild::DerivedDataHash;
using libutil::Filesystem;
using libutil::FSUtil;

WorkspaceContext::
WorkspaceContext(
    std::string const &basePath,
    DerivedDataHash const &derivedDataHash,
    xcworkspace::XC::Workspace::shared_ptr const &workspace,
    pbxproj::PBX::Project::shared_ptr const &project,
    std::vector<xcscheme::SchemeGroup::shared_ptr> const &schemeGroups,
    std::unordered_map<std::string, pbxproj::PBX::Project::shared_ptr> const &projects,
    std::unordered_map<pbxproj::XC::BuildConfiguration::shared_ptr, pbxsetting::XC::Config> const &configs) :
    _basePath       (basePath),
    _derivedDataHash(derivedDataHash),
    _workspace      (workspace),
    _project        (project),
    _schemeGroups   (schemeGroups),
    _projects       (projects),
    _configs        (configs)
{
}

WorkspaceContext::
~WorkspaceContext()
{
}

pbxproj::PBX::Project::shared_ptr WorkspaceContext::
project(std::string const &projectPath) const
{
    /* Normalize the path in case it has relative components. */
    std::string resolvedProjectPath = FSUtil::NormalizePath(projectPath);

    auto PI = _projects.find(resolvedProjectPath);
    if (PI != _projects.end()) {
        return PI->second;
    } else {
        return nullptr;
    }
}

xcscheme::XC::Scheme::shared_ptr WorkspaceContext::
scheme(std::string const &name) const
{
    for (xcscheme::SchemeGroup::shared_ptr const &schemeGroup : _schemeGroups) {
        if (xcscheme::XC::Scheme::shared_ptr const &scheme = schemeGroup->scheme(name)) {
            return scheme;
        }
    }

    return nullptr;
}

std::vector<std::string> WorkspaceContext::
loadedFilePaths() const
{
    std::vector<std::string> loadedFilePaths;

    /* Estimate all files; assume two schemes per scheme group. */
    loadedFilePaths.reserve(_projects.size() + _schemeGroups.size() * 2 + 1);

    /*
     * Add workspace data path.
     */
    if (_workspace != nullptr) {
        loadedFilePaths.push_back(_workspace->dataFile());
    }

    /*
     * Add all projects' data paths.
     */
    for (auto const &entry : _projects) {
        loadedFilePaths.push_back(entry.second->dataFile());
    }

    /*
     * Add all schemes' data paths.
     */
    for (xcscheme::SchemeGroup::shared_ptr const &schemeGroup : _schemeGroups) {
        for (xcscheme::XC::Scheme::shared_ptr const &scheme : schemeGroup->schemes()) {
            loadedFilePaths.push_back(scheme->path());
        }
    }

    /*
     * Add all config file paths.
     */
    for (auto const &entry : _configs) {
        loadedFilePaths.push_back(entry.second.path());
    }

    return loadedFilePaths;
}

static void
IterateWorkspaceItem(xcworkspace::XC::GroupItem::shared_ptr const &item, std::function<void(xcworkspace::XC::FileRef::shared_ptr const &)> const &cb)
{
    switch (item->type()) {
        case xcworkspace::XC::GroupItem::Type::Group: {
            xcworkspace::XC::Group::shared_ptr group = std::static_pointer_cast<xcworkspace::XC::Group>(item);
            for (xcworkspace::XC::GroupItem::shared_ptr const &child : group->items()) {
                IterateWorkspaceItem(child, cb);
            }
            break;
        }
        case xcworkspace::XC::GroupItem::Type::FileRef: {
            xcworkspace::XC::FileRef::shared_ptr fileRef = std::static_pointer_cast<xcworkspace::XC::FileRef>(item);
            cb(fileRef);
            break;
        }
    }
}

static void
IterateWorkspaceFiles(xcworkspace::XC::Workspace::shared_ptr const &workspace, std::function<void(xcworkspace::XC::FileRef::shared_ptr const &)> const &cb)
{
    for (xcworkspace::XC::GroupItem::shared_ptr const &item : workspace->items()) {
        IterateWorkspaceItem(item, cb);
    }
}

static void
LoadWorkspaceProjects(Filesystem const *filesystem, std::vector<pbxproj::PBX::Project::shared_ptr> *projects, xcworkspace::XC::Workspace::shared_ptr const &workspace)
{
    /*
     * Load all the projects in the workspace.
     */
    IterateWorkspaceFiles(workspace, [&](xcworkspace::XC::FileRef::shared_ptr const &ref) {
        std::string path = ref->resolve(workspace);

        pbxproj::PBX::Project::shared_ptr project = pbxproj::PBX::Project::Open(filesystem, path);
        if (project != nullptr) {
            projects->push_back(project);
        }
    });
}

static void
LoadConfigurationFiles(
    Filesystem const *filesystem,
    std::unordered_map<pbxproj::XC::BuildConfiguration::shared_ptr, pbxsetting::XC::Config> *configs,
    pbxsetting::Environment const &environment,
    pbxproj::XC::ConfigurationList::shared_ptr const &configurationList)
{
    if (configurationList == nullptr) {
        return;
    }

    /*
     * Load all configuration files in the list.
     */
    for (pbxproj::XC::BuildConfiguration::shared_ptr const &buildConfiguration : configurationList->buildConfigurations()) {
        if (pbxproj::PBX::FileReference::shared_ptr const &configurationReference = buildConfiguration->baseConfigurationReference()) {
            std::string configurationPath = environment.expand(configurationReference->resolve());

            /* Load the configuration file. */
            if (ext::optional<pbxsetting::XC::Config> configuration = pbxsetting::XC::Config::Load(filesystem, environment, configurationPath)) {
                configs->insert({ buildConfiguration, *configuration });
            }
        }
    }
}

static void
LoadNestedProjects(
    Filesystem const *filesystem,
    std::vector<pbxproj::PBX::Project::shared_ptr> *projects,
    std::unordered_map<pbxproj::XC::BuildConfiguration::shared_ptr, pbxsetting::XC::Config> *configs,
    pbxsetting::Environment const &baseEnvironment,
    std::vector<pbxproj::PBX::Project::shared_ptr> const &rootProjects)
{
    std::vector<pbxproj::PBX::Project::shared_ptr> nestedProjects;

    /*
     * Load all nested projects recursively.
     */
    for (pbxproj::PBX::Project::shared_ptr const &project : rootProjects) {
        /*
         * Determine the settings environment to find the project paths. This may not be complete,
         * but it's unclear exactly what settings are available here. Notably, we don't yet know what
         * the configuration or what target to use, so just the project settings seems reasonable.
         */
        pbxsetting::Environment environment = pbxsetting::Environment(baseEnvironment);
        environment.insertFront(project->settings(), false);

        /*
         * Load project and target configurations.
         */
        LoadConfigurationFiles(filesystem, configs, environment, project->buildConfigurationList());
        for (pbxproj::PBX::Target::shared_ptr const &target : project->targets()) {
            LoadConfigurationFiles(filesystem, configs, environment, target->buildConfigurationList());
        }

        /*
         * Find the nested projects.
         */
        for (pbxproj::PBX::Project::ProjectReference const &projectReference : project->projectReferences()) {
            pbxproj::PBX::FileReference::shared_ptr const &projectFileReference = projectReference.projectReference();
            std::string projectPath = environment.expand(projectFileReference->resolve());

            /*
             * Load the project.
             */
            pbxproj::PBX::Project::shared_ptr project = pbxproj::PBX::Project::Open(filesystem, projectPath);
            if (project != nullptr) {
                nestedProjects.push_back(project);
            }
        }
    }

    /*
     * Append the nested projects. This has to be after the loop as `rootProjects` might alias `projects`.
     */
    projects->insert(projects->end(), nestedProjects.begin(), nestedProjects.end());

    if (!nestedProjects.empty()) {
        /*
         * Load nested projects of the nested projects.
         */
        LoadNestedProjects(filesystem, projects, configs, baseEnvironment, nestedProjects);
    }
}

static void
LoadProjectSchemes(Filesystem const *filesystem, std::string const &userName, std::vector<xcscheme::SchemeGroup::shared_ptr> *schemeGroups, std::vector<pbxproj::PBX::Project::shared_ptr> const &projects)
{
    /*
     * Load the schemes inside the projects.
     */
    for (pbxproj::PBX::Project::shared_ptr const &project : projects) {
        xcscheme::SchemeGroup::shared_ptr projectGroup = xcscheme::SchemeGroup::Open(filesystem, userName, project->basePath(), project->projectFile(), project->name());
        if (projectGroup != nullptr) {
            schemeGroups->push_back(projectGroup);
        }
    }
}

static std::unordered_map<std::string, pbxproj::PBX::Project::shared_ptr>
CreateProjectMap(std::vector<pbxproj::PBX::Project::shared_ptr> const &projects)
{
    std::unordered_map<std::string, pbxproj::PBX::Project::shared_ptr> projectsMap;

    for (pbxproj::PBX::Project::shared_ptr const &project : projects) {
        /* Normalize path so it can be found on lookup. */
        std::string normalizedPath = FSUtil::NormalizePath(project->projectFile());
        projectsMap.insert({ normalizedPath, project });
    }

    return projectsMap;
}

WorkspaceContext WorkspaceContext::
Workspace(Filesystem const *filesystem, std::string const &userName, pbxsetting::Environment const &baseEnvironment, xcworkspace::XC::Workspace::shared_ptr const &workspace)
{
    std::vector<pbxproj::PBX::Project::shared_ptr> projects;
    std::vector<xcscheme::SchemeGroup::shared_ptr> schemeGroups;
    std::unordered_map<pbxproj::XC::BuildConfiguration::shared_ptr, pbxsetting::XC::Config> configs;

    /*
     * Add the schemes from the workspace itself.
     */
    xcscheme::SchemeGroup::shared_ptr workspaceGroup = xcscheme::SchemeGroup::Open(filesystem, userName, workspace->basePath(), workspace->projectFile(), workspace->name());
    if (workspaceGroup != nullptr) {
        schemeGroups.push_back(workspaceGroup);
    }

    /*
     * Load projects within the workspace.
     */
    LoadWorkspaceProjects(filesystem, &projects, workspace);

    /*
     * Recursively load nested projects within those projects.
     */
    LoadNestedProjects(filesystem, &projects, &configs, baseEnvironment, projects);

    /*
     * Load schemes for all projects, including nested projects.
     */
    LoadProjectSchemes(filesystem, userName, &schemeGroups, projects);

    /*
     * Determine the DerivedData path for the workspace.
     */
    DerivedDataHash derivedDataHash = DerivedDataHash::Create(workspace->projectFile());

    return WorkspaceContext(workspace->basePath(), derivedDataHash, workspace, nullptr, schemeGroups, CreateProjectMap(projects), configs);
}

WorkspaceContext WorkspaceContext::
Project(Filesystem const *filesystem, std::string const &userName, pbxsetting::Environment const &baseEnvironment, pbxproj::PBX::Project::shared_ptr const &project)
{
    std::vector<pbxproj::PBX::Project::shared_ptr> projects;
    std::vector<xcscheme::SchemeGroup::shared_ptr> schemeGroups;
    std::unordered_map<pbxproj::XC::BuildConfiguration::shared_ptr, pbxsetting::XC::Config> configs;

    /*
     * The root is a project, so it should be in the projects list.
     */
    projects.push_back(project);

    /*
     * Recursively load nested projects within the project.
     */
    LoadNestedProjects(filesystem, &projects, &configs, baseEnvironment, projects);

    /*
     * Load schemes for all projects, including the root and nested projects.
     */
    LoadProjectSchemes(filesystem, userName, &schemeGroups, projects);

    /*
     * Determine the DerivedData path for the root project.
     */
    DerivedDataHash derivedDataHash = DerivedDataHash::Create(project->projectFile());

    return WorkspaceContext(project->basePath(), derivedDataHash, nullptr, project, schemeGroups, CreateProjectMap(projects), configs);
}
