/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/ListAction.h>
#include <xcdriver/Action.h>
#include <xcdriver/Options.h>
#include <libutil/Filesystem.h>
#include <libutil/Strings.h>
#include <process/Context.h>
#include <process/User.h>

using xcdriver::ListAction;
using xcdriver::Options;
using libutil::Filesystem;

ListAction::
ListAction()
{
}

ListAction::
~ListAction()
{
}

int ListAction::
Run(process::User const *user, process::Context const *processContext, Filesystem const *filesystem, Options const &options)
{
    ext::optional<pbxbuild::Build::Environment> buildEnvironment = pbxbuild::Build::Environment::Default(user, processContext, filesystem);
    if (!buildEnvironment) {
        fprintf(stderr, "error: couldn't create build environment\n");
        return -1;
    }

    std::vector<pbxsetting::Level> overrideLevels = Action::CreateOverrideLevels(processContext, filesystem, buildEnvironment->baseEnvironment(), options, processContext->currentDirectory());
    xcexecution::Parameters parameters = Action::CreateParameters(options, overrideLevels);

    ext::optional<pbxbuild::WorkspaceContext> context = parameters.loadWorkspace(filesystem, user->userName(), *buildEnvironment, processContext->currentDirectory());
    if (!context) {
        return -1;
    }

    /* Collect all schemes in the workspace. */
    std::vector<xcscheme::XC::Scheme::shared_ptr> schemes;
    for (xcscheme::SchemeGroup::shared_ptr const &schemeGroup : context->schemeGroups()) {
        schemes.insert(schemes.end(), schemeGroup->schemes().begin(), schemeGroup->schemes().end());
    }

    std::sort(schemes.begin(), schemes.end(), [](xcscheme::XC::Scheme::shared_ptr const &a, xcscheme::XC::Scheme::shared_ptr const &b) -> bool {
        return libutil::strcasecmp(a->name().c_str(), b->name().c_str()) < 0;
    });

    auto I = std::unique(schemes.begin(), schemes.end(), [](xcscheme::XC::Scheme::shared_ptr const &a, xcscheme::XC::Scheme::shared_ptr const &b) -> bool {
        return (a->path() == b->path());
    });
    schemes.resize(std::distance(schemes.begin(), I));

    if (context->workspace() != nullptr) {
        xcworkspace::XC::Workspace::shared_ptr const &workspace = context->workspace();

        printf("Information about workspace \"%s\":\n", workspace->name().c_str());

        if (schemes.empty()) {
            printf("\n%4sThis workspace contains no scheme.\n", "");
        } else {
            printf("%4sSchemes:\n", "");
            for (auto const &scheme : schemes) {
                printf("%8s%s\n", "", scheme->name().c_str());
            }
            printf("\n");
        }
    } else if (context->project() != nullptr) {
        pbxproj::PBX::Project::shared_ptr const &project = context->project();

        printf("Information about project \"%s\":\n", project->name().c_str());

        if (!project->targets().empty()) {
            printf("%4sTargets:\n", "");
            for (auto const &target : project->targets()) {
                printf("%8s%s\n", "", target->name().c_str());
            }
        } else {
            printf("%4sThis project contains no targets.\n", "");
        }
        printf("\n");

        if (project->buildConfigurationList()) {
            printf("%4sBuild Configurations:\n", "");
            for (auto const &config : project->buildConfigurationList()->buildConfigurations()) {
                printf("%8s%s\n", "", config->name().c_str());
            }
            printf("\n%4sIf no build configuration is specified and -scheme is not passed then \"%s\" is used.\n", "", project->buildConfigurationList()->defaultConfigurationName().c_str());
        } else {
            printf("%4sThis project contains no build configurations.\n", "");
        }
        printf("\n");

        if (schemes.empty()) {
            printf("\n%4sThis project contains no scheme.\n", "");
        } else {
            printf("%4sSchemes:\n", "");
            for (auto const &scheme : schemes) {
                printf("%8s%s\n", "", scheme->name().c_str());
            }
            printf("\n");
        }
    }

    return 0;
}
