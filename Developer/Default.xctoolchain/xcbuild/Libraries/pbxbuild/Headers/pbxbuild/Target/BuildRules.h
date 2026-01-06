/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Target_BuildRules_h
#define __pbxbuild_Target_BuildRules_h

#include <pbxbuild/Base.h>

namespace pbxbuild {
namespace Target {

class BuildRules {
public:
    class BuildRule {
    public:
        typedef std::shared_ptr<BuildRule> shared_ptr;
        typedef std::vector<shared_ptr> vector;

    private:
        friend class pbxbuild::Target::BuildRules;

    private:
        std::string                    _filePatterns;
        pbxspec::PBX::FileType::vector _fileTypes;
        pbxspec::PBX::Tool::shared_ptr _tool;
        std::string                    _script;
        std::vector<pbxsetting::Value> _outputFiles;

    public:
        BuildRule(std::string const &filePatterns, pbxspec::PBX::FileType::vector const &fileTypes, pbxspec::PBX::Tool::shared_ptr const &tool, std::string const &script, std::vector<pbxsetting::Value> const &outputFiles);

    public:
        inline std::string const &filePatterns() const
        { return _filePatterns; }
        inline pbxspec::PBX::FileType::vector const &fileTypes() const
        { return _fileTypes; }

    public:
        inline pbxspec::PBX::Tool::shared_ptr const &tool() const
        { return _tool; }
        inline std::string const &script() const
        { return _script; }

    public:
        inline std::vector<pbxsetting::Value> const &outputFiles() const
        { return _outputFiles; }
    };

private:
    BuildRule::vector _buildRules;

private:
    BuildRules(BuildRule::vector const &buildRules);

public:
    BuildRule::shared_ptr
    resolve(pbxspec::PBX::FileType::shared_ptr const &fileType, std::string const &filePath) const;

public:
    static BuildRules
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &domains, pbxproj::PBX::Target::shared_ptr const &target);
};

}
}

#endif // !__pbxbuild_Target_BuildRules_h
