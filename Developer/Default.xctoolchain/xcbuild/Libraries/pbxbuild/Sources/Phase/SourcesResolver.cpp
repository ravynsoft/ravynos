/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/SourcesResolver.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Phase/ModuleMapResolver.h>
#include <pbxbuild/Target/Environment.h>
#include <pbxbuild/Build/Environment.h>
#include <pbxbuild/Build/Context.h>
#include <pbxbuild/Tool/ClangResolver.h>
#include <pbxbuild/Tool/HeadermapResolver.h>
#include <pbxbuild/Tool/DittoResolver.h>
#include <pbxbuild/Tool/CompilationInfo.h>
#include <pbxbuild/Tool/HeadermapInfo.h>
#include <pbxbuild/Tool/PrecompiledHeaderInfo.h>
#include <pbxbuild/Tool/SearchPaths.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

namespace Phase = pbxbuild::Phase;
namespace Target = pbxbuild::Target;
namespace Tool = pbxbuild::Tool;
using libutil::Filesystem;
using libutil::FSUtil;

Phase::SourcesResolver::
SourcesResolver(pbxproj::PBX::SourcesBuildPhase::shared_ptr const &buildPhase) :
    _buildPhase(buildPhase)
{
}

Phase::SourcesResolver::
~SourcesResolver()
{
}

static bool
CopySwiftModules(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext)
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    pbxsetting::Environment const &environment = targetEnvironment.environment();
    Tool::Context *toolContext = &phaseContext->toolContext();

    Tool::DittoResolver const *dittoResolver = phaseContext->dittoResolver(phaseEnvironment);
    if (dittoResolver == nullptr) {
        return false;
    }

    // TODO(grp): Find a better way of finding if this is building a framework.
    bool isFramework = false;
    for (pbxspec::PBX::ProductType::shared_ptr productType = targetEnvironment.productType(); productType != nullptr; productType = productType->base()) {
        if (productType->identifier() == "com.apple.product-type.framework") {
            isFramework = true;
            break;
        }
    }

    for (Tool::SwiftModuleInfo &moduleInfo : toolContext->swiftModuleInfo()) {
        moduleInfo.copiedArtifacts().clear();

        /* Output into the framework or the products directory. */
        std::string outputBase;
        if (isFramework) {
            outputBase = environment.resolve("TARGET_BUILD_DIR") + "/" + environment.resolve("CONTENTS_FOLDER_PATH") + "/" + "Modules";
        } else {
            outputBase = environment.resolve("BUILT_PRODUCTS_DIR");
        }
        outputBase += "/" + moduleInfo.moduleName() + ".swiftmodule";

        /* Each architecture has a separate module subdirectory. */
        std::string outputName = moduleInfo.architecture();
        if (outputName == "armv7") {
            /* For some reason, armv7 is special cased as "arm". */
            outputName = "arm";
        }

        /* Copy the module to let modules import it. */
        std::string outputPath = outputBase + "/" + outputName + ".swiftmodule";
        dittoResolver->resolve(toolContext, moduleInfo.modulePath(), outputPath);
        moduleInfo.copiedArtifacts().push_back(outputPath);

        /* Copy the swiftdoc. It's next to the module. */
        std::string docOutputPath = outputBase + "/" + outputName + ".swiftdoc";
        dittoResolver->resolve(toolContext, moduleInfo.docPath(), docOutputPath);
        moduleInfo.copiedArtifacts().push_back(docOutputPath);

        /* Copy the generated header, if requested. */
        if (moduleInfo.installHeader()) {
            std::string headerName = FSUtil::GetBaseName(moduleInfo.headerPath());
            std::string installedHeaderPath = environment.resolve("DERIVED_FILE_DIR") + "/" + headerName;
            dittoResolver->resolve(toolContext, moduleInfo.headerPath(), installedHeaderPath);
            moduleInfo.copiedArtifacts().push_back(installedHeaderPath);
        }
    }

    return true;
}

bool Phase::SourcesResolver::
resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext)
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    Build::Environment const &buildEnvironment = phaseEnvironment.buildEnvironment();

    Tool::ClangResolver const *clangResolver = phaseContext->clangResolver(phaseEnvironment);
    if (clangResolver == nullptr) {
        return false;
    }

    std::unique_ptr<Tool::HeadermapResolver> headermapResolver = Tool::HeadermapResolver::Create(buildEnvironment.specManager(), targetEnvironment.specDomains(), clangResolver->compiler());
    if (headermapResolver == nullptr) {
        return false;
    }

    /* Populate the tool context with what's needed for compilation. */
    headermapResolver->resolve(&phaseContext->toolContext(), targetEnvironment.environment(), phaseEnvironment.target());

    /*
     * Module maps need to be generated.
     */
    Phase::ModuleMapResolver moduleMap = Phase::ModuleMapResolver();
    if (!moduleMap.resolve(phaseEnvironment, phaseContext)) {
        fprintf(stderr, "error: unable to resolve module map\n");
    }

    std::vector<Tool::Input> files = Phase::File::ResolveBuildFiles(Filesystem::GetDefaultUNSAFE(), phaseEnvironment, targetEnvironment.environment(), _buildPhase->files());

    /*
     * Split files based on whether their tool is architecture-neutral.
     */
    std::vector<Tool::Input> neutralFiles;
    std::vector<Tool::Input> architectureFiles;
    for (Tool::Input const &file : files) {
        if (file.buildRule() != nullptr && file.buildRule()->tool() != nullptr && file.buildRule()->tool()->isArchitectureNeutral() == false) {
            architectureFiles.push_back(file);
        } else {
            neutralFiles.push_back(file);
        }
    }

    /*
     * Resolve non-architecture-specific files. These are resolved just once.
     */
    std::vector<std::vector<Tool::Input>> neutralGroups = Phase::Context::Group(neutralFiles);
    std::string neutralOutputDirectory = targetEnvironment.environment().resolve("OBJECT_FILE_DIR");
    if (!phaseContext->resolveBuildFiles(phaseEnvironment, targetEnvironment.environment(), _buildPhase, neutralGroups, neutralOutputDirectory)) {
        return false;
    }

    /*
     * Resolve architecture-specific files.
     */
    std::vector<std::vector<Tool::Input>> architectureGroups = Phase::Context::Group(architectureFiles);
    for (std::string const &variant : targetEnvironment.variants()) {
        for (std::string const &arch : targetEnvironment.architectures()) {
            pbxsetting::Environment currentEnvironment = pbxsetting::Environment(targetEnvironment.environment());
            currentEnvironment.insertFront(Phase::Environment::VariantLevel(variant), false);
            currentEnvironment.insertFront(Phase::Environment::ArchitectureLevel(arch), false);

            std::string outputDirectory = currentEnvironment.expand(pbxsetting::Value::Parse("$(OBJECT_FILE_DIR_$(variant))/$(arch)"));

            if (!phaseContext->resolveBuildFiles(phaseEnvironment, currentEnvironment, _buildPhase, architectureGroups, outputDirectory)) {
                return false;
            }
        }
    }

    /*
     * For any built Swift modules, copy their outputs as needed.
     */
    if (!CopySwiftModules(phaseEnvironment, phaseContext)) {
        return false;
    }

    /*
     * Add dependencies to invocations marked to wait for swift artifacts.
     */
    for (auto &invocation : phaseContext->toolContext().invocations()) {
        if (invocation.waitForSwiftArtifacts()) {
            for (auto &swiftModuleInfo : phaseContext->toolContext().swiftModuleInfo()) {
                invocation.inputDependencies().insert(
                    invocation.inputDependencies().end(),
                    swiftModuleInfo.copiedArtifacts().begin(),
                    swiftModuleInfo.copiedArtifacts().end()
                );
            }
        }
    }

    return true;
}
