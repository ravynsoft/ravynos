/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Target/BuildRules.h>
#include <libutil/FSUtil.h>
#include <libutil/Wildcard.h>

namespace Target = pbxbuild::Target;
using libutil::FSUtil;
using libutil::Wildcard;

Target::BuildRules::BuildRule::
BuildRule(std::string const &filePatterns, pbxspec::PBX::FileType::vector const &fileTypes, pbxspec::PBX::Tool::shared_ptr const &tool, std::string const &script, std::vector<pbxsetting::Value> const &outputFiles) :
    _filePatterns(filePatterns),
    _fileTypes   (fileTypes),
    _tool        (tool),
    _script      (script),
    _outputFiles (outputFiles)
{
}

Target::BuildRules::
BuildRules(Target::BuildRules::BuildRule::vector const &buildRules) :
    _buildRules(buildRules)
{
}

Target::BuildRules::BuildRule::shared_ptr Target::BuildRules::
resolve(pbxspec::PBX::FileType::shared_ptr const &fileType, std::string const &filePath) const
{
    for (BuildRule::shared_ptr const &buildRule : _buildRules) {
        if (!buildRule->filePatterns().empty()) {
            if (Wildcard::Match(buildRule->filePatterns(), FSUtil::GetBaseName(filePath))) {
                return buildRule;
            }
        } else {
            pbxspec::PBX::FileType::vector const &fileTypes = buildRule->fileTypes();
            for (pbxspec::PBX::FileType::shared_ptr FT = fileType; FT != nullptr; FT = FT->base()) {
                if (std::find(fileTypes.begin(), fileTypes.end(), FT) != fileTypes.end()) {
                    return buildRule;
                }
            }
        }
    }

    return nullptr;
}

static Target::BuildRules::BuildRule::shared_ptr
ProjectBuildRule(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &domains, pbxproj::PBX::BuildRule::shared_ptr const &projBuildRule)
{
    pbxspec::PBX::Tool::shared_ptr tool = nullptr;
    std::string const &TS = projBuildRule->compilerSpec();
    if (TS != "com.apple.compilers.proxy.script") {
        tool = specManager->tool(TS, domains);
        if (tool == nullptr) {
            tool = std::static_pointer_cast<pbxspec::PBX::Tool>(specManager->compiler(TS, domains));
        }
        if (tool == nullptr) {
            tool = std::static_pointer_cast<pbxspec::PBX::Tool>(specManager->linker(TS, domains));
        }

        if (tool == nullptr) {
            fprintf(stderr, "warning: couldn't find tool %s specified in build rule\n", TS.c_str());
            return nullptr;
        }
    }

    pbxspec::PBX::FileType::vector fileTypes;
    std::string const &FT = projBuildRule->fileType();
    if (FT != "pattern.proxy") {
        pbxspec::PBX::FileType::shared_ptr fileType = specManager->fileType(FT, domains);
        if (fileType == nullptr) {
            fprintf(stderr, "warning: couldn't find input file type %s specified in build rule\n", FT.c_str());
            return nullptr;
        }
        fileTypes.push_back(fileType);
    }

    std::vector<pbxsetting::Value> outputFiles;
    for (std::string const &outputFile : projBuildRule->outputFiles()) {
        outputFiles.push_back(pbxsetting::Value::Parse(outputFile));
    }

    return std::make_shared<Target::BuildRules::BuildRule>(Target::BuildRules::BuildRule(
        projBuildRule->filePatterns(),
        fileTypes,
        tool,
        projBuildRule->script(),
        outputFiles
    ));
}

static Target::BuildRules::BuildRule::shared_ptr
SpecificationBuildRule(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &domains, pbxspec::PBX::BuildRule::shared_ptr const &specBuildRule)
{
    if (!specBuildRule->compilerSpec()) {
        return nullptr;
    }

    std::string const &TS = *specBuildRule->compilerSpec();
    pbxspec::PBX::Tool::shared_ptr tool = specManager->tool(TS, domains);
    if (tool == nullptr) {
        tool = std::static_pointer_cast<pbxspec::PBX::Tool>(specManager->compiler(TS, domains));
    }
    if (tool == nullptr) {
        tool = std::static_pointer_cast<pbxspec::PBX::Tool>(specManager->linker(TS, domains));
    }

    if (tool == nullptr) {
        return nullptr;
    }

    pbxspec::PBX::FileType::vector fileTypes;
    if (specBuildRule->fileTypes()) {
        for (std::string const &FT : *specBuildRule->fileTypes()) {
            pbxspec::PBX::FileType::shared_ptr fileType = specManager->fileType(FT, domains);
            if (fileType == nullptr) {
                return nullptr;
            }
            fileTypes.push_back(fileType);
        }
    }

    return std::make_shared <Target::BuildRules::BuildRule> (Target::BuildRules::BuildRule(
        std::string(),
        fileTypes,
        tool,
        std::string(),
        std::vector<pbxsetting::Value>()
    ));
}

Target::BuildRules Target::BuildRules::
Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &domains, pbxproj::PBX::Target::shared_ptr const &target)
{
    Target::BuildRules::BuildRule::vector buildRules;

    if (target->type() == pbxproj::PBX::Target::Type::Native) {
        pbxproj::PBX::NativeTarget::shared_ptr nativeTarget = std::static_pointer_cast <pbxproj::PBX::NativeTarget> (target);
        for (pbxproj::PBX::BuildRule::shared_ptr const &projBuildRule : nativeTarget->buildRules()) {
            if (Target::BuildRules::BuildRule::shared_ptr buildRule = ProjectBuildRule(specManager, domains, projBuildRule)) {
                buildRules.push_back(buildRule);
            }
        }
    }

    for (pbxspec::PBX::BuildRule::shared_ptr const &specBuildRule : specManager->buildRules()) {
        if (Target::BuildRules::BuildRule::shared_ptr buildRule = SpecificationBuildRule(specManager, domains, specBuildRule)) {
            buildRules.push_back(buildRule);
        }
    }

    for (pbxspec::PBX::BuildRule::shared_ptr const &specBuildRule : specManager->synthesizedBuildRules(domains)) {
        if (BuildRule::shared_ptr buildRule = SpecificationBuildRule(specManager, domains, specBuildRule)) {
            buildRules.push_back(buildRule);
        }
    }

    return Target::BuildRules(buildRules);
}
