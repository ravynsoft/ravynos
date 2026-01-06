/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/FrameworksResolver.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Phase/File.h>
#include <pbxbuild/Target/Environment.h>
#include <pbxbuild/Build/Environment.h>
#include <pbxbuild/Build/Context.h>
#include <pbxbuild/Tool/ToolResolver.h>
#include <pbxbuild/Tool/LinkerResolver.h>
#include <pbxbuild/Tool/CompilationInfo.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

namespace Phase = pbxbuild::Phase;
namespace Build = pbxbuild::Build;
namespace Target = pbxbuild::Target;
namespace Tool = pbxbuild::Tool;
using libutil::Filesystem;
using libutil::FSUtil;

Phase::FrameworksResolver::
FrameworksResolver(pbxproj::PBX::FrameworksBuildPhase::shared_ptr const &buildPhase) :
    _buildPhase(buildPhase)
{
}

Phase::FrameworksResolver::
~FrameworksResolver()
{
}

bool Phase::FrameworksResolver::
resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext)
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    Build::Environment const &buildEnvironment = phaseEnvironment.buildEnvironment();

    Tool::CompilationInfo const &compilationInfo = phaseContext->toolContext().compilationInfo();

    std::unique_ptr<Tool::LinkerResolver> ldResolver = Tool::LinkerResolver::Create(buildEnvironment.specManager(), targetEnvironment.specDomains(), Tool::LinkerResolver::LinkerToolIdentifier());
    std::unique_ptr<Tool::LinkerResolver> libtoolResolver = Tool::LinkerResolver::Create(buildEnvironment.specManager(), targetEnvironment.specDomains(), Tool::LinkerResolver::LibtoolToolIdentifier());
    std::unique_ptr<Tool::LinkerResolver> lipoResolver = Tool::LinkerResolver::Create(buildEnvironment.specManager(), targetEnvironment.specDomains(), Tool::LinkerResolver::LipoToolIdentifier());
    std::unique_ptr<Tool::ToolResolver> dsymutilResolver = Tool::ToolResolver::Create(buildEnvironment.specManager(), targetEnvironment.specDomains(), "com.apple.tools.dsymutil");
    if (ldResolver == nullptr || libtoolResolver == nullptr || lipoResolver == nullptr || dsymutilResolver == nullptr) {
        fprintf(stderr, "error: couldn't get linker tools\n");
        return false;
    }

    std::string binaryType = targetEnvironment.environment().resolve("MACH_O_TYPE");

    Tool::LinkerResolver *linkerResolver = nullptr;
    std::string linkerExecutable;
    std::vector<std::string> linkerArguments;

    if (binaryType == "staticlib") {
        linkerResolver = libtoolResolver.get();
    } else {
        linkerResolver = ldResolver.get();
        linkerExecutable = compilationInfo.linkerDriver();
        linkerArguments.insert(linkerArguments.end(), compilationInfo.linkerArguments().begin(), compilationInfo.linkerArguments().end());
    }

    std::string workingDirectory = targetEnvironment.workingDirectory();
    std::string productsDirectory = targetEnvironment.environment().resolve("BUILT_PRODUCTS_DIR");

    std::vector<Tool::Input> files = Phase::File::ResolveBuildFiles(Filesystem::GetDefaultUNSAFE(), phaseEnvironment, targetEnvironment.environment(), _buildPhase->files());

    for (std::string const &variant : targetEnvironment.variants()) {
        pbxsetting::Environment variantEnvironment = pbxsetting::Environment(targetEnvironment.environment());
        variantEnvironment.insertFront(Phase::Environment::VariantLevel(variant), false);

        std::string variantIntermediatesName = variantEnvironment.resolve("EXECUTABLE_NAME") + variantEnvironment.resolve("EXECUTABLE_VARIANT_SUFFIX");
        std::string variantIntermediatesDirectory = variantEnvironment.resolve("OBJECT_FILE_DIR_" + variant);

        std::string variantProductsPath = variantEnvironment.resolve("EXECUTABLE_PATH") + variantEnvironment.resolve("EXECUTABLE_VARIANT_SUFFIX");
        std::string variantProductsOutput = productsDirectory + "/" + variantProductsPath;

        bool createUniversalBinary = targetEnvironment.architectures().size() > 1;
        std::vector<Tool::Input> universalBinaryInputs;

        for (std::string const &arch : targetEnvironment.architectures()) {
            pbxsetting::Environment archEnvironment = pbxsetting::Environment(variantEnvironment);
            archEnvironment.insertFront(Phase::Environment::ArchitectureLevel(arch), false);

            std::vector<Tool::Input> sourceOutputs;
            auto it = phaseContext->toolContext().variantArchitectureInvocations().find(std::make_pair(variant, arch));
            if (it != phaseContext->toolContext().variantArchitectureInvocations().end()) {
                std::vector<Tool::Invocation> const &sourceInvocations = it->second;
                for (Tool::Invocation const &invocation : sourceInvocations) {
                    for (std::string const &output : invocation.outputs()) {
                        // TODO(grp): Is this the right set of source outputs to link?
                        // TODO(grp): Use the object file file type and include in input.
                        if (FSUtil::GetFileExtension(output) == "o") {
                            Tool::Input outputInput = Tool::Input(output, nullptr);
                            sourceOutputs.push_back(outputInput);
                        }
                    }
                }
            }

            if (createUniversalBinary) {
                std::string architectureIntermediatesDirectory = variantIntermediatesDirectory + "/" + arch;
                std::string architectureIntermediatesOutput = architectureIntermediatesDirectory + "/" + variantIntermediatesName;

                linkerResolver->resolve(&phaseContext->toolContext(), archEnvironment, sourceOutputs, files, architectureIntermediatesOutput, linkerArguments, linkerExecutable);
                universalBinaryInputs.push_back(Tool::Input(architectureIntermediatesOutput, nullptr));
            } else {
                linkerResolver->resolve(&phaseContext->toolContext(), archEnvironment, sourceOutputs, files, variantProductsOutput, linkerArguments, linkerExecutable);
            }
        }

        if (createUniversalBinary) {
            lipoResolver->resolve(&phaseContext->toolContext(), variantEnvironment, universalBinaryInputs, { }, variantProductsOutput, { });
        }

        if (variantEnvironment.resolve("DEBUG_INFORMATION_FORMAT") == "dwarf-with-dsym" && (binaryType != "staticlib" && binaryType != "mh_object")) {
            Tool::Input outputInput = Tool::Input(variantProductsOutput, nullptr);
            std::string dsymfile = variantEnvironment.resolve("DWARF_DSYM_FOLDER_PATH") + "/" + variantEnvironment.resolve("DWARF_DSYM_FILE_NAME");
            dsymutilResolver->resolve(&phaseContext->toolContext(), variantEnvironment, { outputInput }, std::vector<std::string>({dsymfile}));
        }
    }

    return true;
}
