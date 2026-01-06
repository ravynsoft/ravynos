/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/CopyFilesResolver.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/File.h>
#include <pbxbuild/Tool/CopyResolver.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Type.h>
#include <pbxsetting/Value.h>
#include <libutil/Filesystem.h>

namespace Phase = pbxbuild::Phase;
namespace Tool = pbxbuild::Tool;
using libutil::Filesystem;

Phase::CopyFilesResolver::
CopyFilesResolver(pbxproj::PBX::CopyFilesBuildPhase::shared_ptr const &buildPhase) :
    _buildPhase(buildPhase)
{
}

Phase::CopyFilesResolver::
~CopyFilesResolver()
{
}

static pbxsetting::Value
DestinationOutputPath(pbxproj::PBX::CopyFilesBuildPhase::Destination destination)
{
    pbxsetting::Value products = pbxsetting::Value::Variable("BUILT_PRODUCTS_DIR") + pbxsetting::Value::String("/");

    switch (destination) {
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationAbsolute:
            return pbxsetting::Value::Empty();
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationWrapper:
            return products + pbxsetting::Value::Variable("CONTENTS_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationExecutables:
            return products + pbxsetting::Value::Variable("EXECUTABLES_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationResources:
            return products + pbxsetting::Value::Variable("UNLOCALIZED_RESOURCES_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationPublicHeaders:
            return products + pbxsetting::Value::Variable("PUBLIC_HEADERS_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationPrivateHeaders:
            return products + pbxsetting::Value::Variable("PRIVATE_HEADERS_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationFrameworks:
            return products + pbxsetting::Value::Variable("FRAMEWORKS_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationSharedFrameworks:
            return products + pbxsetting::Value::Variable("SHARED_FRAMEWORKS_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationSharedSupport:
            return products + pbxsetting::Value::Variable("SHARED_SUPPORT_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationPlugIns:
            return products + pbxsetting::Value::Variable("PLUGINS_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationScripts:
            return products + pbxsetting::Value::Variable("SCRIPTS_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationJavaResources:
            return products + pbxsetting::Value::Variable("JAVA_FOLDER_PATH");
        case pbxproj::PBX::CopyFilesBuildPhase::kDestinationProducts:
            return products;
        default:
            fprintf(stderr, "error: unknown destination %d\n", (int)destination);
            return pbxsetting::Value::Empty();
    }
}

bool Phase::CopyFilesResolver::
resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext)
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    pbxsetting::Environment const &environment = targetEnvironment.environment();

    Tool::CopyResolver const *copyResolver = phaseContext->copyResolver(phaseEnvironment);
    if (copyResolver == nullptr) {
        return false;
    }

    std::string root = environment.expand(DestinationOutputPath(_buildPhase->dstSubfolderSpec()));
    std::string path = environment.expand(_buildPhase->dstPath());
    std::string outputDirectory = root + "/" + path;

    std::vector<Tool::Input> files = Phase::File::ResolveBuildFiles(Filesystem::GetDefaultUNSAFE(), phaseEnvironment, environment, _buildPhase->files());
    std::vector<std::vector<Tool::Input>> groups = Phase::Context::Group(files);

    if (pbxsetting::Type::ParseBoolean(environment.resolve("APPLY_RULES_IN_COPY_FILES"))) {
        if (!phaseContext->resolveBuildFiles(phaseEnvironment, environment, _buildPhase, groups, outputDirectory, Tool::CopyResolver::ToolIdentifier())) {
            return false;
        }
    } else {
        for (Tool::Input const &file : files) {
            copyResolver->resolve(&phaseContext->toolContext(), environment, { file }, outputDirectory, "PBXCp");
        }
    }

    return true;
}
