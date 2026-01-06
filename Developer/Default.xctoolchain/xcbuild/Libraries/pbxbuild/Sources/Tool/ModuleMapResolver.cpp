/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/ModuleMapResolver.h>
#include <pbxbuild/Tool/Context.h>
#include <pbxbuild/Tool/CopyResolver.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Type.h>
#include <libutil/FSUtil.h>

#include <iterator>

namespace Tool = pbxbuild::Tool;
using libutil::FSUtil;

Tool::ModuleMapResolver::
ModuleMapResolver()
{
}

Tool::ModuleMapResolver::
~ModuleMapResolver()
{
}

static ext::optional<std::string>
FindUmbrellaHeaderName(pbxsetting::Environment const &environment, std::string const &moduleName, pbxproj::PBX::Target::shared_ptr const &target)
{
    std::string lowerModuleName;
    std::transform(moduleName.begin(), moduleName.end(), std::back_inserter(lowerModuleName), ::tolower);

    ext::optional<std::string> umbrellaHeader;
    for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : target->buildPhases()) {
        if (buildPhase->type() != pbxproj::PBX::BuildPhase::Type::Headers) {
            continue;
        }

        for (pbxproj::PBX::BuildFile::shared_ptr const &buildFile : buildPhase->files()) {
            if (buildFile->fileRef() == nullptr) {
                continue;
            }

            std::string filePath = environment.expand(buildFile->fileRef()->resolve());
            if (FSUtil::GetFileExtension(filePath) == "h") {
                std::string baseName = FSUtil::GetBaseNameWithoutExtension(filePath);

                std::string lowerBaseName;
                std::transform(baseName.begin(), baseName.end(), std::back_inserter(lowerBaseName), ::tolower);

                if (lowerBaseName == lowerModuleName) {
                    return FSUtil::GetBaseName(filePath);
                }
            }
        }
    }

    return ext::nullopt;
}

static ext::optional<Tool::AuxiliaryFile::Chunk>
Contents(pbxsetting::Environment const &environment, pbxproj::PBX::Target::shared_ptr const &target, std::string const &workingDirectory)
{
    std::string inputModuleMapPath = environment.resolve("MODULEMAP_FILE");

    if (!inputModuleMapPath.empty()) {
        /*
         * Copy in the input module map as an auxiliary file.
         */
        std::string path = FSUtil::ResolveRelativePath(inputModuleMapPath, workingDirectory);
        return Tool::AuxiliaryFile::Chunk::File(path);
    } else {
        /*
         * Find umbrella header: a header with the same name as the target's module.
         */
        std::string moduleName = environment.resolve("PRODUCT_MODULE_NAME");
        ext::optional<std::string> umbrellaHeaderName = FindUmbrellaHeaderName(environment, moduleName, target);

        if (umbrellaHeaderName) {
            /*
             * Generate a module map as an auxiliary file.
             */
            auto contents = std::string();
            contents += "framework module " + moduleName + " {\n";
            contents += "  umbrella header \"" + *umbrellaHeaderName + "\"\n";
            contents += "  \n";
            contents += "  export *\n";
            contents += "  module * { export * }\n";
            contents += "}\n";

            auto data = std::vector<uint8_t>(contents.begin(), contents.end());
            return Tool::AuxiliaryFile::Chunk::Data(data);
        } else {
            return ext::nullopt;
        }
    }
}

void Tool::ModuleMapResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &environment,
    pbxproj::PBX::Target::shared_ptr const &target) const
{
    std::string const &workingDirectory = toolContext->workingDirectory();

    std::string intermediateModuleMapRoot = environment.resolve("TARGET_TEMP_DIR");
    // TODO: what about non-framework product types?
    std::string finalModuleMapRoot = environment.resolve("TARGET_BUILD_DIR") + "/" + environment.resolve("CONTENTS_FOLDER_PATH") + "/" + "Modules";

    /*
     * Handle the standard module map, either copied or generated.
     */
    ext::optional<Tool::AuxiliaryFile::Chunk> contents = Contents(environment, target, workingDirectory);
    if (contents) {
        std::string intermediateModuleMapPath = intermediateModuleMapRoot + "/" + "module.modulemap";
        auto moduleMapAuxiliaryFile = Tool::AuxiliaryFile(intermediateModuleMapPath, { *contents });

        std::string finalModuleMapPath = finalModuleMapRoot + "/" + "module.modulemap";
        auto entry = Tool::ModuleMapInfo::Entry(*contents, intermediateModuleMapPath, finalModuleMapPath);
        toolContext->moduleMapInfo().moduleMap() = entry;
    }

    /*
     * Handle the private module map. This module map can only be specified; it is never auto-generated.
     */
    std::string inputPrivateModuleMapPath = environment.resolve("MODULEMAP_PRIVATE_FILE");
    if (!inputPrivateModuleMapPath.empty()) {
        /*
         * Copy in the private module map.
         */
        auto privateContents = Tool::AuxiliaryFile::Chunk::File(
            FSUtil::ResolveRelativePath(inputPrivateModuleMapPath, workingDirectory));

        std::string intermediatePrivateModuleMapPath = intermediateModuleMapRoot + "/" + "module.private.modulemap";
        auto privateModuleMapAuxiliaryFile = Tool::AuxiliaryFile(intermediatePrivateModuleMapPath, { privateContents });

        std::string finalPrivateModuleMapPath = finalModuleMapRoot + "/" + "module.private.modulemap";
        auto entry = Tool::ModuleMapInfo::Entry(privateContents, intermediatePrivateModuleMapPath, finalPrivateModuleMapPath);
        toolContext->moduleMapInfo().privateModuleMap() = entry;
    }
}

std::unique_ptr<Tool::ModuleMapResolver> Tool::ModuleMapResolver::
Create()
{
    return std::unique_ptr<Tool::ModuleMapResolver>(new Tool::ModuleMapResolver());
}

