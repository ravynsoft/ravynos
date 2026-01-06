/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/PhaseInvocations.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Phase/CopyFilesResolver.h>
#include <pbxbuild/Phase/HeadersResolver.h>
#include <pbxbuild/Phase/LegacyTargetResolver.h>
#include <pbxbuild/Phase/ModuleMapResolver.h>
#include <pbxbuild/Phase/ProductTypeResolver.h>
#include <pbxbuild/Phase/ResourcesResolver.h>
#include <pbxbuild/Phase/FrameworksResolver.h>
#include <pbxbuild/Phase/SourcesResolver.h>
#include <pbxbuild/Phase/ShellScriptResolver.h>
#include <pbxbuild/Phase/SwiftResolver.h>
#include <pbxbuild/Tool/Context.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Type.h>

namespace Phase = pbxbuild::Phase;
namespace Tool = pbxbuild::Tool;
namespace Target = pbxbuild::Target;

Phase::PhaseInvocations::
PhaseInvocations(std::vector<Tool::Invocation> const &invocations, std::vector<Tool::AuxiliaryFile> const &auxiliaryFiles) :
    _invocations   (invocations),
    _auxiliaryFiles(auxiliaryFiles)
{
}

Phase::PhaseInvocations::
~PhaseInvocations()
{
}

Phase::PhaseInvocations Phase::PhaseInvocations::
Create(Phase::Environment const &phaseEnvironment, pbxproj::PBX::Target::shared_ptr const &target)
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    pbxsetting::Environment const &environment = targetEnvironment.environment();

    /* Create the tool context for building. */
    Tool::SearchPaths searchPaths = Tool::SearchPaths::Create(
        targetEnvironment.environment(),
        targetEnvironment.workingDirectory());
    Tool::Context toolContext = Tool::Context(
        targetEnvironment.sdk(),
        targetEnvironment.toolchains(),
        targetEnvironment.workingDirectory(),
        searchPaths);

    Phase::Context phaseContext(toolContext);

    /* Filter build phases to ones appropriate for this target. */
    bool deploymentPostprocessing = pbxsetting::Type::ParseBoolean(environment.resolve("DEPLOYMENT_POSTPROCESSING"));
    std::vector<pbxproj::PBX::BuildPhase::shared_ptr> buildPhases;
    for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : target->buildPhases()) {
        // TODO(grp): Check buildActionMask against buildContext.action.

        if (buildPhase->runOnlyForDeploymentPostprocessing() && !deploymentPostprocessing) {
            /* Requires deployment postprocessing, but not doing that. */
            continue;
        }

        buildPhases.push_back(buildPhase);
    }

    /*
     * Find sources phase, and synthesize framework phase for linking, if it does not exist
     */
    auto it_sources = std::find_if(buildPhases.begin(), buildPhases.end(), [](pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase) -> bool {
        return (buildPhase->type() == pbxproj::PBX::BuildPhase::Type::Sources);
    });
    if (it_sources != buildPhases.end()) {
        auto it_frameworks = std::find_if(buildPhases.begin(), buildPhases.end(), [](pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase) -> bool {
            return (buildPhase->type() == pbxproj::PBX::BuildPhase::Type::Frameworks);
        });
        if (it_frameworks == buildPhases.end()) {
            pbxproj::PBX::FrameworksBuildPhase::shared_ptr frameworksPhase = std::make_shared<pbxproj::PBX::FrameworksBuildPhase> ();
            buildPhases.insert(std::next(it_sources), frameworksPhase);
        }
    }

    /*
     * Resolve all build phases.
     */
    phaseContext.toolContext().currentPhaseInvocationPriority() = PHASE_INVOCATION_PRIORITY_BASE;
    uint32_t targetLevelPhaseInsertionPoint = phaseContext.toolContext().currentPhaseInvocationPriority();
    for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : buildPhases) {
        bool updateTargetLevelPhaseInsertionPoint = false;
        switch (buildPhase->type()) {
            case pbxproj::PBX::BuildPhase::Type::Sources: {
                auto BP = std::static_pointer_cast <pbxproj::PBX::SourcesBuildPhase> (buildPhase);

                Phase::SourcesResolver sources = Phase::SourcesResolver(BP);
                if (!sources.resolve(phaseEnvironment, &phaseContext)) {
                    fprintf(stderr, "error: unable to resolve sources\n");
                }

                updateTargetLevelPhaseInsertionPoint = true;
                break;
            }
            case pbxproj::PBX::BuildPhase::Type::Frameworks: {
                auto BP = std::static_pointer_cast <pbxproj::PBX::FrameworksBuildPhase> (buildPhase);

                Phase::FrameworksResolver frameworks = Phase::FrameworksResolver(BP);
                if (!frameworks.resolve(phaseEnvironment, &phaseContext)) {
                    fprintf(stderr, "error: unable to resolve linking\n");
                }

                updateTargetLevelPhaseInsertionPoint = true;
                break;
            }
            case pbxproj::PBX::BuildPhase::Type::ShellScript: {
                auto BP = std::static_pointer_cast <pbxproj::PBX::ShellScriptBuildPhase> (buildPhase);

                Phase::ShellScriptResolver shellScript = Phase::ShellScriptResolver(BP);
                if (!shellScript.resolve(phaseEnvironment, &phaseContext)) {
                    fprintf(stderr, "error: unable to resolve shell script\n");
                }
                break;
            }
            case pbxproj::PBX::BuildPhase::Type::CopyFiles: {
                auto BP = std::static_pointer_cast <pbxproj::PBX::CopyFilesBuildPhase> (buildPhase);

                Phase::CopyFilesResolver copyFiles = Phase::CopyFilesResolver(BP);
                if (!copyFiles.resolve(phaseEnvironment, &phaseContext)) {
                    fprintf(stderr, "error: unable to resolve copy files\n");
                }

                updateTargetLevelPhaseInsertionPoint = true;
                break;
            }
            case pbxproj::PBX::BuildPhase::Type::Headers: {
                auto BP = std::static_pointer_cast <pbxproj::PBX::HeadersBuildPhase> (buildPhase);

                Phase::HeadersResolver headers = Phase::HeadersResolver(BP);
                if (!headers.resolve(phaseEnvironment, &phaseContext)) {
                    fprintf(stderr, "error: unable to resolve headers\n");
                }

                updateTargetLevelPhaseInsertionPoint = true;
                break;
            }
            case pbxproj::PBX::BuildPhase::Type::Resources: {
                auto BP = std::static_pointer_cast <pbxproj::PBX::ResourcesBuildPhase> (buildPhase);

                Phase::ResourcesResolver resources = Phase::ResourcesResolver(BP);
                if (!resources.resolve(phaseEnvironment, &phaseContext)) {
                    fprintf(stderr, "error: unable to resolve resources\n");
                }

                updateTargetLevelPhaseInsertionPoint = true;
                break;
            }
            case pbxproj::PBX::BuildPhase::Type::AppleScript: {
                // TODO: Compile AppleScript
                auto BP = std::static_pointer_cast <pbxproj::PBX::AppleScriptBuildPhase> (buildPhase);
                break;
            }
            case pbxproj::PBX::BuildPhase::Type::Rez: {
                // TODO: Compile Rez
                auto BP = std::static_pointer_cast <pbxproj::PBX::RezBuildPhase> (buildPhase);
                break;
            }
        }

        phaseContext.toolContext().currentPhaseInvocationPriority() += PHASE_INVOCATION_PRIORITY_INCREMENT;

        /*
         * At the end of iteration, every invocation with priority equal to or greater than
         * targetLevelPhaseInsertionPoint will be bumped up to next priority level to accomodate
         * room for target level phase invocations.
         */
        if (updateTargetLevelPhaseInsertionPoint) {
            targetLevelPhaseInsertionPoint = std::max(
                targetLevelPhaseInsertionPoint,
                phaseContext.toolContext().currentPhaseInvocationPriority()
            );
        }
    }

    for (auto invocation : phaseContext.toolContext().invocations()) {
        if (invocation.priority() >= targetLevelPhaseInsertionPoint) {
            invocation.priority() += PHASE_INVOCATION_PRIORITY_INCREMENT;
        }
    }

    /* Temporarily override phase invocation priority to add target level invocations */
    uint32_t currentPhaseInvocationPriorityCache = phaseContext.toolContext().currentPhaseInvocationPriority();
    phaseContext.toolContext().currentPhaseInvocationPriority() = targetLevelPhaseInsertionPoint;

    /*
     * Add target-level invocations. Note these must come last as they use the context values set
     * by tools added above for the various build phases. For example, the final 'touch' must depend
     * on all previous invocations that created the output bundle.
     */
    switch (target->type()) {
        case pbxproj::PBX::Target::Type::Native: {
            /*
             * Various product types have special (post-)processing. For example, bundles have
             * have info plist processing and applications additionally have a validation step.
             */
            if (pbxspec::PBX::ProductType::shared_ptr const &PT = phaseEnvironment.targetEnvironment().productType()) {
                Phase::ProductTypeResolver productType = Phase::ProductTypeResolver(PT);
                if (!productType.resolve(phaseEnvironment, &phaseContext)) {
                    fprintf(stderr, "error: unable to resolve product type\n");
                }
            }

            /*
             * Swift requires the standard library be copied into the product.
             */
            Phase::SwiftResolver swift = Phase::SwiftResolver();
            if (!swift.resolve(phaseEnvironment, &phaseContext)) {
                fprintf(stderr, "error: unable to resolve swift\n");
            }
            break;
        }
        case pbxproj::PBX::Target::Type::Legacy: {
            /*
             * Run the script to build the legacy target.
             */
            pbxproj::PBX::LegacyTarget::shared_ptr LT = std::static_pointer_cast<pbxproj::PBX::LegacyTarget>(target);

            Phase::LegacyTargetResolver legacyScript = Phase::LegacyTargetResolver(LT);
            if (!legacyScript.resolve(phaseEnvironment, &phaseContext)) {
                fprintf(stderr, "error: unable to resolve legacy script\n");
            }

            break;
        }
        case pbxproj::PBX::Target::Type::Aggregate:
            break;
    }

    /* Restore current phase invocation priority level */
    phaseContext.toolContext().currentPhaseInvocationPriority() = currentPhaseInvocationPriorityCache;

    return Phase::PhaseInvocations(phaseContext.toolContext().invocations(), phaseContext.toolContext().auxiliaryFiles());
}

