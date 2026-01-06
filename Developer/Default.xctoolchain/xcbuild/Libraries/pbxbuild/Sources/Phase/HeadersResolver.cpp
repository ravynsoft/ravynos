/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/HeadersResolver.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Phase/File.h>
#include <pbxbuild/Tool/CopyResolver.h>
#include <libutil/Filesystem.h>

namespace Phase = pbxbuild::Phase;
namespace Tool = pbxbuild::Tool;
using libutil::Filesystem;

Phase::HeadersResolver::
HeadersResolver(pbxproj::PBX::HeadersBuildPhase::shared_ptr const &buildPhase) :
    _buildPhase(buildPhase)
{
}

Phase::HeadersResolver::
~HeadersResolver()
{
}

bool Phase::HeadersResolver::
resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext)
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    pbxsetting::Environment const &environment = targetEnvironment.environment();

    Tool::CopyResolver const *copyResolver = phaseContext->copyResolver(phaseEnvironment);
    if (copyResolver == nullptr) {
        return false;
    }

    std::string targetBuildDirectory = environment.resolve("TARGET_BUILD_DIR");
    std::string publicOutputDirectory = targetBuildDirectory + "/" + environment.resolve("PUBLIC_HEADERS_FOLDER_PATH");
    std::string privateOutputDirectory = targetBuildDirectory + "/" + environment.resolve("PRIVATE_HEADERS_FOLDER_PATH");

    std::vector<Tool::Input> files = Phase::File::ResolveBuildFiles(Filesystem::GetDefaultUNSAFE(), phaseEnvironment, environment, _buildPhase->files());

    for (Tool::Input const &file : files) {
        std::vector<std::string> const &attributes = file.attributes().value_or(std::vector<std::string>());
        bool isPublic  = std::find(attributes.begin(), attributes.end(), "Public") != attributes.end();
        bool isPrivate = std::find(attributes.begin(), attributes.end(), "Private") != attributes.end();

        std::string const &outputDirectory = (isPublic ? publicOutputDirectory : privateOutputDirectory);

        if (isPublic || isPrivate) {
            copyResolver->resolve(&phaseContext->toolContext(), environment, { file }, outputDirectory, "CpHeader");
        }
    }

    return true;
}
