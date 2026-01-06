/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/HeadermapResolver.h>
#include <pbxbuild/Tool/HeadermapInfo.h>
#include <pbxbuild/Tool/SearchPaths.h>
#include <pbxbuild/Tool/Context.h>
#include <pbxbuild/FileTypeResolver.h>
#include <pbxbuild/HeaderMap.h>
#include <pbxsetting/Type.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

namespace Tool = pbxbuild::Tool;
using pbxbuild::HeaderMap;
using pbxbuild::FileTypeResolver;
using libutil::Filesystem;
using libutil::FSUtil;

Tool::HeadermapResolver::
HeadermapResolver(pbxspec::PBX::Tool::shared_ptr const &tool, pbxspec::PBX::Compiler::shared_ptr const &compiler, pbxspec::Manager::shared_ptr const &specManager) :
    _tool       (tool),
    _compiler   (compiler),
    _specManager(specManager)
{
}

static std::vector<std::string>
HeadermapSearchPaths(pbxspec::Manager::shared_ptr const &specManager, pbxsetting::Environment const &environment, pbxproj::PBX::Target::shared_ptr const &target, Tool::SearchPaths const &searchPaths, std::string const &workingDirectory)
{
    std::unordered_set<std::string> allHeaderSearchPaths;
    std::vector<std::string> orderedHeaderSearchPaths;

    for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : target->buildPhases()) {
        if (buildPhase->type() != pbxproj::PBX::BuildPhase::Type::Sources) {
            continue;
        }

        for (pbxproj::PBX::BuildFile::shared_ptr const &buildFile : buildPhase->files()) {
            if (buildFile->fileRef() == nullptr) {
                continue;
            }

            std::string filePath = environment.expand(buildFile->fileRef()->resolve());
            std::string fullPath = FSUtil::GetDirectoryName(filePath);
            if (allHeaderSearchPaths.insert(fullPath).second) {
                orderedHeaderSearchPaths.push_back(fullPath);
            }
        }
    }

    for (std::string const &path : searchPaths.userHeaderSearchPaths()) {
        std::string fullPath = FSUtil::ResolveRelativePath(path, workingDirectory);
        if (allHeaderSearchPaths.insert(fullPath).second) {
            orderedHeaderSearchPaths.push_back(fullPath);
        }
    }
    for (std::string const &path : searchPaths.headerSearchPaths()) {
        std::string fullPath = FSUtil::ResolveRelativePath(path, workingDirectory);
        if (allHeaderSearchPaths.insert(fullPath).second) {
            orderedHeaderSearchPaths.push_back(fullPath);
        }
    }

    return orderedHeaderSearchPaths;
}

void Tool::HeadermapResolver::
resolve(
    Tool::Context *toolContext,
    pbxsetting::Environment const &environment,
    pbxproj::PBX::Target::shared_ptr const &target
) const
{
    /* Add the compiler default environment, which contains the headermap setting defaults. */
    pbxsetting::Environment compilerEnvironment = pbxsetting::Environment(environment);
    compilerEnvironment.insertFront(_compiler->defaultSettings(), true);

    if (!pbxsetting::Type::ParseBoolean(compilerEnvironment.resolve("USE_HEADERMAP"))) {
        return;
    }

    if (pbxsetting::Type::ParseBoolean(compilerEnvironment.resolve("HEADERMAP_USES_VFS"))) {
        // TODO(grp): Support VFS-based header maps.
    }

    HeaderMap targetName;
    HeaderMap ownTargetHeaders;
    HeaderMap projectHeaders;
    HeaderMap allTargetHeaders;
    HeaderMap allNonFrameworkTargetHeaders;

    bool includeFlatEntriesForTargetBeingBuilt     = pbxsetting::Type::ParseBoolean(compilerEnvironment.resolve("HEADERMAP_INCLUDES_FLAT_ENTRIES_FOR_TARGET_BEING_BUILT"));
    bool includeFrameworkEntriesForAllProductTypes = pbxsetting::Type::ParseBoolean(compilerEnvironment.resolve("HEADERMAP_INCLUDES_FRAMEWORK_ENTRIES_FOR_ALL_PRODUCT_TYPES"));
    bool includeProjectHeaders                     = pbxsetting::Type::ParseBoolean(compilerEnvironment.resolve("HEADERMAP_INCLUDES_PROJECT_HEADERS"));

    // TODO(grp): Populate generated headers.
    HeaderMap generatedFiles;

    pbxproj::PBX::Project::shared_ptr project = target->project();

    std::vector<std::string> headermapSearchPaths = HeadermapSearchPaths(_specManager, compilerEnvironment, target, toolContext->searchPaths(), toolContext->workingDirectory());
    for (std::string const &path : headermapSearchPaths) {
        Filesystem::GetDefaultUNSAFE()->readDirectory(path, false, [&](std::string const &fileName) -> bool {
            // TODO(grp): Use FileTypeResolver when reliable.
            std::string extension = FSUtil::GetFileExtension(fileName);
            if (extension != "h" && extension != "hpp") {
                return true;
            }

            targetName.add(fileName, path + "/", fileName);
            return true;
        });
    }

    for (pbxproj::PBX::FileReference::shared_ptr const &fileReference : project->fileReferences()) {
        std::string filePath = compilerEnvironment.expand(fileReference->resolve());
        pbxspec::PBX::FileType::shared_ptr fileType = FileTypeResolver::Resolve(Filesystem::GetDefaultUNSAFE(), _specManager, { pbxspec::Manager::AnyDomain() }, fileReference, filePath);
        if (fileType == nullptr || (fileType->identifier() != "sourcecode.c.h" && fileType->identifier() != "sourcecode.cpp.h")) {
            continue;
        }

        std::string fileName = FSUtil::GetBaseName(filePath);
        std::string fileDirectory = FSUtil::GetDirectoryName(filePath) + "/";

        projectHeaders.add(fileName, fileDirectory, fileName);
        if (includeProjectHeaders) {
            targetName.add(fileName, fileDirectory, fileName);
        }
    }

    for (pbxproj::PBX::Target::shared_ptr const &projectTarget : project->targets()) {
       for (pbxproj::PBX::BuildPhase::shared_ptr const &buildPhase : projectTarget->buildPhases()) {
            if (buildPhase->type() != pbxproj::PBX::BuildPhase::Type::Headers) {
                continue;
            }

            for (pbxproj::PBX::BuildFile::shared_ptr const &buildFile : buildPhase->files()) {
                if (buildFile->fileRef() == nullptr || buildFile->fileRef()->type() != pbxproj::PBX::GroupItem::Type::FileReference) {
                    continue;
                }

                pbxproj::PBX::FileReference::shared_ptr const &fileReference = std::static_pointer_cast <pbxproj::PBX::FileReference> (buildFile->fileRef());
                std::string filePath = compilerEnvironment.expand(fileReference->resolve());
                pbxspec::PBX::FileType::shared_ptr fileType = FileTypeResolver::Resolve(Filesystem::GetDefaultUNSAFE(), _specManager, { pbxspec::Manager::AnyDomain() }, fileReference, filePath);
                if (fileType == nullptr || (fileType->identifier() != "sourcecode.c.h" && fileType->identifier() != "sourcecode.cpp.h")) {
                    continue;
                }

                std::string fileName = FSUtil::GetBaseName(filePath);
                std::string fileDirectory = FSUtil::GetDirectoryName(filePath) + "/";
                std::string frameworkName = projectTarget->productName() + "/" + fileName;

                std::vector<std::string> const &attributes = buildFile->attributes();
                bool isPublic  = std::find(attributes.begin(), attributes.end(), "Public") != attributes.end();
                bool isPrivate = std::find(attributes.begin(), attributes.end(), "Private") != attributes.end();

                if (projectTarget == target) {
                    ownTargetHeaders.add(fileName, fileDirectory, fileName);

                    if (!isPublic && !isPrivate) {
                        ownTargetHeaders.add(frameworkName, fileDirectory, fileName);
                        if (includeFlatEntriesForTargetBeingBuilt) {
                            targetName.add(frameworkName, fileDirectory, fileName);
                        }
                    }
                }

                if (isPublic || isPrivate) {
                    allTargetHeaders.add(frameworkName, fileDirectory, fileName);
                    if (includeFrameworkEntriesForAllProductTypes) {
                        targetName.add(frameworkName, fileDirectory, fileName);
                    }

                    // TODO(grp): This is a little messy. Maybe check the product type specification, or the product reference's file type?
                    if (projectTarget->type() == pbxproj::PBX::Target::Type::Native && std::static_pointer_cast<pbxproj::PBX::NativeTarget>(projectTarget)->productType().find("framework") == std::string::npos) {
                        allNonFrameworkTargetHeaders.add(frameworkName, fileDirectory, fileName);
                        if (!includeFrameworkEntriesForAllProductTypes) {
                            targetName.add(frameworkName, fileDirectory, fileName);
                        }
                    }
                }
            }
        }
    }

    std::string headermapFile                                = compilerEnvironment.resolve("CPP_HEADERMAP_FILE");
    std::string headermapFileForOwnTargetHeaders             = compilerEnvironment.resolve("CPP_HEADERMAP_FILE_FOR_OWN_TARGET_HEADERS");
    std::string headermapFileForAllTargetHeaders             = compilerEnvironment.resolve("CPP_HEADERMAP_FILE_FOR_ALL_TARGET_HEADERS");
    std::string headermapFileForAllNonFrameworkTargetHeaders = compilerEnvironment.resolve("CPP_HEADERMAP_FILE_FOR_ALL_NON_FRAMEWORK_TARGET_HEADERS");
    std::string headermapFileForGeneratedFiles               = compilerEnvironment.resolve("CPP_HEADERMAP_FILE_FOR_GENERATED_FILES");
    std::string headermapFileForProjectFiles                 = compilerEnvironment.resolve("CPP_HEADERMAP_FILE_FOR_PROJECT_FILES");

    std::vector<Tool::AuxiliaryFile> auxiliaryFiles = {
        Tool::AuxiliaryFile::Data(headermapFile, targetName.write()),
        Tool::AuxiliaryFile::Data(headermapFileForOwnTargetHeaders, ownTargetHeaders.write()),
        Tool::AuxiliaryFile::Data(headermapFileForAllTargetHeaders, allTargetHeaders.write()),
        Tool::AuxiliaryFile::Data(headermapFileForAllNonFrameworkTargetHeaders, allNonFrameworkTargetHeaders.write()),
        Tool::AuxiliaryFile::Data(headermapFileForGeneratedFiles, generatedFiles.write()),
        Tool::AuxiliaryFile::Data(headermapFileForProjectFiles, projectHeaders.write()),
    };

    toolContext->auxiliaryFiles().insert(toolContext->auxiliaryFiles().end(), auxiliaryFiles.begin(), auxiliaryFiles.end());

    std::vector<std::string> systemHeadermapFiles;
    std::vector<std::string> userHeadermapFiles;

    if (pbxsetting::Type::ParseBoolean(compilerEnvironment.resolve("ALWAYS_SEARCH_USER_PATHS")) && !pbxsetting::Type::ParseBoolean(compilerEnvironment.resolve("ALWAYS_USE_SEPARATE_HEADERMAPS"))) {
        systemHeadermapFiles.push_back(headermapFile);
    } else {
        if (includeFlatEntriesForTargetBeingBuilt) {
            systemHeadermapFiles.push_back(headermapFileForOwnTargetHeaders);
        }
        if (includeFrameworkEntriesForAllProductTypes) {
            systemHeadermapFiles.push_back(headermapFileForAllTargetHeaders);
        } else {
            systemHeadermapFiles.push_back(headermapFileForAllNonFrameworkTargetHeaders);
        }

        userHeadermapFiles.push_back(headermapFileForGeneratedFiles);
        if (includeProjectHeaders) {
            userHeadermapFiles.push_back(headermapFileForProjectFiles);
        }
    }

    Tool::HeadermapInfo *headermapInfo = &toolContext->headermapInfo();
    headermapInfo->systemHeadermapFiles().insert(headermapInfo->systemHeadermapFiles().end(), systemHeadermapFiles.begin(), systemHeadermapFiles.end());
    headermapInfo->userHeadermapFiles().insert(headermapInfo->userHeadermapFiles().end(), userHeadermapFiles.begin(), userHeadermapFiles.end());
}

std::unique_ptr<Tool::HeadermapResolver> Tool::HeadermapResolver::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains, pbxspec::PBX::Compiler::shared_ptr const &compiler)
{
    pbxspec::PBX::Tool::shared_ptr headermapTool = specManager->tool(Tool::HeadermapResolver::ToolIdentifier(), specDomains);
    if (headermapTool == nullptr) {
        fprintf(stderr, "warning: could not find headermap tool\n");
        return nullptr;
    }

    return std::unique_ptr<Tool::HeadermapResolver>(new Tool::HeadermapResolver(headermapTool, compiler, specManager));
}
