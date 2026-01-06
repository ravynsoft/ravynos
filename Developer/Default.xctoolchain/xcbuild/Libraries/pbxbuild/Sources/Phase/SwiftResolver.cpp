/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/SwiftResolver.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Target/Environment.h>
#include <pbxbuild/Tool/SwiftStandardLibraryResolver.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Type.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

namespace Target = pbxbuild::Target;
namespace Phase = pbxbuild::Phase;
namespace Tool = pbxbuild::Tool;
using libutil::Filesystem;
using libutil::FSUtil;

Phase::SwiftResolver::
SwiftResolver()
{
}

static bool
ShouldBundleSwiftRuntime(Tool::Context const *toolContext, pbxsetting::Environment const &environment, pbxspec::PBX::ProductType::shared_ptr const &productType)
{
    if (!pbxsetting::Type::ParseBoolean(environment.resolve("EMBEDDED_CONTENT_CONTAINS_SWIFT"))) {
        /* Only check if there isn't embedded content: always need a runtime for embedded content. */

        // TODO(grp): Find a better way of finding if this is building a application.
        bool isApplication = false;
        for (pbxspec::PBX::ProductType::shared_ptr PT = productType; PT != nullptr; PT = PT->base()) {
            if (PT->identifier() == "com.apple.product-type.application") {
                isApplication = true;
                break;
            }
        }

        /*
         * Only applications should bundle Swift libraries; frameworks or plugins will
         * get their copies of the libraries from the application containing them.
         */
        if (!isApplication) {
            return false;
        }

        /*
         * Check if this application contains Swift source code. If not, since no embedded
         * content uses Swift, no need to copy Swift libraries. The app uses no Swift.
         */
        if (toolContext->swiftModuleInfo().empty()) {
            return false;
        }
    }

    /*
     * Need a place to put the runtime libraries. If the product is not a wrapper, then
     * there isn't a frameworks directory to include Swift in.
     */
    if (!productType->isWrapper()) {
        return false;
    }

    return true;
}

static std::vector<std::string>
CollectScanDirectories(
    Phase::Environment const &phaseEnvironment,
    pbxsetting::Environment const &environment,
    xcsdk::SDK::Target::shared_ptr const &sdk,
    pbxproj::PBX::Target::shared_ptr const &target)
{
    std::string targetDirectory = environment.resolve("TARGET_BUILD_DIR");

    std::vector<std::string> directories;

    /*
     * Frameworks using Swift are output in the frameworks path.
     */
    std::string frameworksDirectory = environment.resolve("FRAMEWORKS_FOLDER_PATH");
    if (!frameworksDirectory.empty()) {
        directories.push_back(targetDirectory + "/" + frameworksDirectory);
    }

    /*
     * Plugins using Swift are output in the frameworks path.
     */
    std::string pluginsDirectory = environment.resolve("PLUGINS_FOLDER_PATH");
    if (!pluginsDirectory.empty()) {
        directories.push_back(targetDirectory + "/" + pluginsDirectory);
    }

    /*
     * Include all linked, non-system frameworks. These will all contain
     * binaries that the libraries link and that might contain Swift.
     */
    for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : target->buildPhases()) {
        if (buildPhase->type() != pbxproj::PBX::BuildPhase::Type::Frameworks) {
            continue;
        }

        std::vector<Tool::Input> files = Phase::File::ResolveBuildFiles(Filesystem::GetDefaultUNSAFE(), phaseEnvironment, environment, buildPhase->files());
        for (Tool::Input const &file : files) {
            if (file.fileType() != nullptr && file.fileType()->isFrameworkWrapper()) {
                directories.push_back(file.path());
            }
        }
    }

    return directories;
}

bool Phase::SwiftResolver::
resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext) const
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    Build::Environment const &buildEnvironment = phaseEnvironment.buildEnvironment();
    pbxsetting::Environment const &environment = targetEnvironment.environment();

    /*
     * Check if the product needs the Swift runtime. If not, return success since there is
     * nothing for this phase to do.
     */
    if (!ShouldBundleSwiftRuntime(&phaseContext->toolContext(), environment, targetEnvironment.productType())) {
        return true;
    }

    /*
     * Get the tool for copying the standard library.
     */
    std::unique_ptr<Tool::SwiftStandardLibraryResolver> swiftStandardLibraryResolver = Tool::SwiftStandardLibraryResolver::Create(buildEnvironment.specManager(), targetEnvironment.specDomains());
    if (swiftStandardLibraryResolver == nullptr) {
        return false;
    }

    /*
     * Find the inputs to the standard library tool.
     */
    std::string executablePath = environment.resolve("TARGET_BUILD_DIR") + "/" + environment.resolve("EXECUTABLE_PATH");
    Tool::Input executableInput = Tool::Input(executablePath, nullptr);
    std::vector<std::string> directories = CollectScanDirectories(phaseEnvironment, environment, targetEnvironment.sdk(), phaseEnvironment.target());

    /*
     * Copy the standard library.
     */
    swiftStandardLibraryResolver->resolve(&phaseContext->toolContext(), environment, executableInput, directories);

    return true;
}
