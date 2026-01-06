/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcexecution/Parameters.h>

#include <pbxbuild/Build/DependencyResolver.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <libutil/md5.h>

#include <sstream>
#include <iomanip>

using xcexecution::Parameters;
using libutil::Filesystem;
using libutil::FSUtil;

Parameters::
Parameters(
    ext::optional<std::string> const &workspace,
    ext::optional<std::string> const &project,
    ext::optional<std::string> const &scheme,
    ext::optional<std::vector<std::string>> const &target,
    bool allTargets,
    std::vector<std::string> const &actions,
    ext::optional<std::string> const &configuration,
    std::vector<pbxsetting::Level> const &overrideLevels) :
    _workspace     (workspace),
    _project       (project),
    _scheme        (scheme),
    _target        (target),
    _allTargets    (allTargets),
    _actions       (actions),
    _configuration (configuration),
    _overrideLevels(overrideLevels)
{
}

std::vector<std::string> Parameters::
canonicalArguments() const
{
    std::vector<std::string> arguments;

    if (_workspace) {
        arguments.push_back("-workspace");
        arguments.push_back(*_workspace);
    }

    if (_project) {
        arguments.push_back("-project");
        arguments.push_back(*_project);
    }

    if (_scheme) {
        arguments.push_back("-scheme");
        arguments.push_back(*_scheme);
    }

    if (_target) {
        for (std::string const &target : *_target) {
            arguments.push_back("-target");
            arguments.push_back(target);
        }
    }

    if (_allTargets) {
        arguments.push_back("-allTargets");
    }

    if (_configuration) {
        arguments.push_back("-configuration");
        arguments.push_back(*_configuration);
    }

    for (std::string const &action : _actions) {
        arguments.push_back(action);
    }

    for (pbxsetting::Level const &overrideLevel : _overrideLevels) {
        for (pbxsetting::Setting const &setting : overrideLevel.settings()) {
            arguments.push_back(setting.name() + "=" + setting.value().raw());
        }
    }

    return arguments;
}

std::string Parameters::
canonicalHash() const
{
    md5_state_t state;
    md5_init(&state);

    std::vector<std::string> arguments = canonicalArguments();
    for (std::string const &argument : arguments) {
        /* Inlucde trailing NUL terminator to separate arguments. */
        md5_append(&state, reinterpret_cast<const md5_byte_t *>(argument.data()), argument.size() + 1);
    }

    uint8_t digest[16];
    md5_finish(&state, reinterpret_cast<md5_byte_t *>(&digest));

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t c : digest) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
}

static pbxproj::PBX::Project::shared_ptr
OpenProject(Filesystem const *filesystem, ext::optional<std::string> const &projectPath, std::string const &directory)
{
    if (projectPath) {
        return pbxproj::PBX::Project::Open(filesystem, *projectPath);
    } else {
        bool multiple = false;
        std::string projectName;

        filesystem->readDirectory(directory, false, [&](std::string const &filename) -> void {
            if (FSUtil::GetFileExtension(filename) != "xcodeproj") {
                return;
            }

            if (!projectName.empty()) {
                multiple = true;
            }

            projectName = filename;
        });

        if (multiple) {
            fprintf(stderr, "error: multiple projects in directory\n");
            return nullptr;
        } else if (projectName.empty()) {
            fprintf(stderr, "error: no project found\n");
            return nullptr;
        } else {
            pbxproj::PBX::Project::shared_ptr project = pbxproj::PBX::Project::Open(filesystem, directory + "/" + projectName);
            if (project == nullptr) {
                fprintf(stderr, "error: unable to open project '%s'\n", projectName.c_str());
            }
            return project;
        }
    }
}

ext::optional<pbxbuild::WorkspaceContext> Parameters::
loadWorkspace(Filesystem const *filesystem, std::string const &userName, pbxbuild::Build::Environment const &buildEnvironment, std::string const &workingDirectory) const
{
    if (_workspace) {
        xcworkspace::XC::Workspace::shared_ptr workspace = xcworkspace::XC::Workspace::Open(filesystem, *_workspace);
        if (workspace == nullptr) {
            fprintf(stderr, "error: unable to open workspace '%s'\n", _workspace->c_str());
            return ext::nullopt;
        }

        return pbxbuild::WorkspaceContext::Workspace(filesystem, userName, buildEnvironment.baseEnvironment(), workspace);
    } else {
        pbxproj::PBX::Project::shared_ptr project = OpenProject(filesystem, _project, workingDirectory);
        if (project == nullptr) {
            return ext::nullopt;
        }

        return pbxbuild::WorkspaceContext::Project(filesystem, userName, buildEnvironment.baseEnvironment(), project);
    }
}

ext::optional<pbxbuild::Build::Context> Parameters::
createBuildContext(pbxbuild::WorkspaceContext const &workspaceContext) const
{
    std::vector<std::string> actions = (!_actions.empty() ? _actions : std::vector<std::string>({ "build" }));
    std::string action = actions.front(); // TODO(grp): Support multiple actions and skipUnavailableOptions.
    if (action != "build") {
        fprintf(stderr, "error: action '%s' is not implemented\n", action.c_str());
        return ext::nullopt;
    }

    /* Find the scheme from the options. */
    xcscheme::XC::Scheme::shared_ptr scheme = nullptr;
    xcscheme::SchemeGroup::shared_ptr schemeGroup = nullptr;
    if (_scheme) {
        for (xcscheme::SchemeGroup::shared_ptr const &schemeGroup_ : workspaceContext.schemeGroups()) {
            if (xcscheme::XC::Scheme::shared_ptr scheme_ = schemeGroup_->scheme(*_scheme)) {
                scheme = scheme_;
                schemeGroup = schemeGroup_;
            }
        }

        if (scheme == nullptr || schemeGroup == nullptr) {
            fprintf(stderr, "error: unable to find scheme '%s'\n", _scheme->c_str());
            return ext::nullopt;
        }
    }

    std::string configuration;
    bool defaultConfiguration;

    if (_configuration) {
        defaultConfiguration = false;
        configuration = *_configuration;
    } else {
        defaultConfiguration = true;

        if (scheme != nullptr) {
            configuration = scheme->buildAction()->buildConfiguration();
            if (configuration.empty()) {
                configuration = "Debug";
            }
        } else if (workspaceContext.project() != nullptr) {
            defaultConfiguration = true;
            configuration = workspaceContext.project()->buildConfigurationList()->defaultConfigurationName();
        } else {
            fprintf(stderr, "error: a scheme is required to build a workspace\n");
            return ext::nullopt;
        }

        if (configuration.empty()) {
            fprintf(stderr, "error: unable to determine build configuration\n");
            return ext::nullopt;
        }
    }

    return pbxbuild::Build::Context(
        workspaceContext,
        scheme,
        schemeGroup,
        action,
        configuration,
        defaultConfiguration,
        _overrideLevels);
}

ext::optional<pbxbuild::DirectedGraph<pbxproj::PBX::Target::shared_ptr>> Parameters::
resolveDependencies(pbxbuild::Build::Environment const &buildEnvironment, pbxbuild::Build::Context const &buildContext) const
{
    pbxbuild::Build::DependencyResolver resolver = pbxbuild::Build::DependencyResolver(buildEnvironment);

    if (buildContext.scheme() != nullptr) {
        return resolver.resolveSchemeDependencies(buildContext);
    } else if (buildContext.workspaceContext().project() != nullptr) {
        return resolver.resolveLegacyDependencies(buildContext, _allTargets, _target);
    } else {
        fprintf(stderr, "error: scheme is required for workspace\n");
        return ext::nullopt;
    }
}

