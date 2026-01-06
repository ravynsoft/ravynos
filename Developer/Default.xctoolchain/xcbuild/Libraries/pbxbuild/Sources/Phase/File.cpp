/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/File.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Target/Environment.h>
#include <pbxbuild/Build/Context.h>
#include <pbxbuild/Build/Environment.h>
#include <pbxbuild/FileTypeResolver.h>
#include <libutil/Filesystem.h>

namespace Phase = pbxbuild::Phase;
namespace Target = pbxbuild::Target;
namespace Tool = pbxbuild::Tool;
namespace Build = pbxbuild::Build;
using pbxbuild::FileTypeResolver;
using libutil::Filesystem;

std::vector<Tool::Input> Phase::File::
ResolveBuildFiles(Filesystem const *filesystem, Phase::Environment const &phaseEnvironment, pbxsetting::Environment const &environment, std::vector<pbxproj::PBX::BuildFile::shared_ptr> const &buildFiles)
{
    Target::Environment const &targetEnvironment = phaseEnvironment.targetEnvironment();
    Target::BuildRules const &buildRules = targetEnvironment.buildRules();
    Build::Environment const &buildEnvironment = phaseEnvironment.buildEnvironment();
    Build::Context const &buildContext = phaseEnvironment.buildContext();

    std::vector<Tool::Input> result;

    for (pbxproj::PBX::BuildFile::shared_ptr const &buildFile : buildFiles) {
        if (buildFile->fileRef() == nullptr) {
            fprintf(stderr, "warning: build phase input does not reference a file\n");
            continue;
        }

        ext::optional<std::string> fileNameDisambiguator;
        auto it = targetEnvironment.buildFileDisambiguation().find(buildFile);
        if (it != targetEnvironment.buildFileDisambiguation().end()) {
            fileNameDisambiguator = it->second;
        }

        switch (buildFile->fileRef()->type()) {
            case pbxproj::PBX::GroupItem::Type::FileReference: {
                pbxproj::PBX::FileReference::shared_ptr const &fileReference = std::static_pointer_cast <pbxproj::PBX::FileReference> (buildFile->fileRef());

                std::string path = environment.expand(fileReference->resolve());
                pbxspec::PBX::FileType::shared_ptr fileType = FileTypeResolver::Resolve(filesystem, buildEnvironment.specManager(), { pbxspec::Manager::AnyDomain() }, fileReference, path);

                Target::BuildRules::BuildRule::shared_ptr buildRule = buildRules.resolve(fileType, path);
                Tool::Input file = Tool::Input(path, fileType, buildRule, fileNameDisambiguator, ext::nullopt, ext::nullopt, buildFile->attributes(), buildFile->compilerFlags());
                result.push_back(file);
                break;
            }
            case pbxproj::PBX::GroupItem::Type::ReferenceProxy: {
                pbxproj::PBX::ReferenceProxy::shared_ptr const &referenceProxy = std::static_pointer_cast <pbxproj::PBX::ReferenceProxy> (buildFile->fileRef());

                pbxproj::PBX::ContainerItemProxy::shared_ptr const &proxy = referenceProxy->remoteRef();
                pbxproj::PBX::FileReference::shared_ptr const &containerReference = proxy->containerPortal();
                std::string containerPath = environment.expand(containerReference->resolve());

                auto remote = buildContext.resolveProductIdentifier(buildContext.workspaceContext().project(containerPath), proxy->remoteGlobalIDString());
                if (!remote) {
                    fprintf(stderr, "error: unable to find remote target product from proxied reference\n");
                    continue;
                }

                ext::optional<Target::Environment> remoteEnvironment = buildContext.targetEnvironment(buildEnvironment, remote->first);
                if (!remoteEnvironment) {
                    fprintf(stderr, "error: unable to create target environment for remote target\n");
                    continue;
                }

                pbxproj::PBX::FileReference::shared_ptr const &fileReference = remote->second;
                std::string path = remoteEnvironment->environment().expand(fileReference->resolve());
                pbxspec::PBX::FileType::shared_ptr fileType = FileTypeResolver::Resolve(filesystem, buildEnvironment.specManager(), { pbxspec::Manager::AnyDomain() }, fileReference, path);

                Target::BuildRules::BuildRule::shared_ptr buildRule = buildRules.resolve(fileType, path);
                Tool::Input file = Tool::Input(path, fileType, buildRule, ext::nullopt, ext::nullopt, ext::nullopt, buildFile->attributes(), buildFile->compilerFlags());
                result.push_back(file);
                break;
            }
            case pbxproj::PBX::GroupItem::Type::VariantGroup: {
                pbxproj::PBX::VariantGroup::shared_ptr const &variantGroup = std::static_pointer_cast <pbxproj::PBX::VariantGroup> (buildFile->fileRef());
                for (pbxproj::PBX::GroupItem::shared_ptr const &child : variantGroup->children()) {
                    if (child->type() != pbxproj::PBX::GroupItem::Type::FileReference) {
                        continue;
                    }

                    pbxproj::PBX::FileReference::shared_ptr const &fileReference = std::static_pointer_cast <pbxproj::PBX::FileReference> (child);
                    std::string const &localization = fileReference->name();

                    std::string path = environment.expand(fileReference->resolve());
                    pbxspec::PBX::FileType::shared_ptr fileType = FileTypeResolver::Resolve(filesystem, buildEnvironment.specManager(), { pbxspec::Manager::AnyDomain() }, fileReference, path);

                    Target::BuildRules::BuildRule::shared_ptr buildRule = buildRules.resolve(fileType, path);
                    Tool::Input file = Tool::Input(path, fileType, buildRule, fileNameDisambiguator, localization, buildFile->blueprintIdentifier(), buildFile->attributes(), buildFile->compilerFlags());
                    result.push_back(file);
                }
                break;
            }
            case pbxproj::PBX::GroupItem::Type::VersionGroup: {
                pbxproj::XC::VersionGroup::shared_ptr const &versionGroup = std::static_pointer_cast <pbxproj::XC::VersionGroup> (buildFile->fileRef());

                std::string path = environment.expand(versionGroup->resolve());
                pbxspec::PBX::FileType::shared_ptr fileType = FileTypeResolver::Resolve(filesystem, buildEnvironment.specManager(), { pbxspec::Manager::AnyDomain() }, versionGroup, path);

                Target::BuildRules::BuildRule::shared_ptr buildRule = buildRules.resolve(fileType, path);
                Tool::Input file = Tool::Input(path, fileType, buildRule, fileNameDisambiguator, ext::nullopt, ext::nullopt, buildFile->attributes(), buildFile->compilerFlags());
                result.push_back(file);
                break;

            }
            case pbxproj::PBX::GroupItem::Type::Group: {
                fprintf(stderr, "warning: unhandled group item type %lu\n", static_cast<unsigned long>(buildFile->fileRef()->type()));
                break;
            }
        }
    }

    return result;
}
