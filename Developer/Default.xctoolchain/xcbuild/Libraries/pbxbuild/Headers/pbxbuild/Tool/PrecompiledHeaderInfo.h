/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_PrecompiledHeaderInfo_h
#define __pbxbuild_Tool_PrecompiledHeaderInfo_h

#include <pbxspec/PBX/Compiler.h>
#include <pbxspec/PBX/FileType.h>
#include <pbxsetting/Value.h>

#include <string>
#include <vector>

namespace pbxbuild {
namespace Tool {

class PrecompiledHeaderInfo {
private:
    std::string                        _prefixHeader;
    pbxspec::PBX::FileType::shared_ptr _fileType;
    std::vector<std::string>           _arguments;
    std::vector<std::string>           _relevantArguments;

public:
    PrecompiledHeaderInfo(std::string const &prefixHeader, pbxspec::PBX::FileType::shared_ptr const &fileType, std::vector<std::string> const &arguments, std::vector<std::string> const &relevantArguments);
    ~PrecompiledHeaderInfo();

public:
    std::string const &prefixHeader() const
    { return _prefixHeader; }
    pbxspec::PBX::FileType::shared_ptr const &fileType() const
    { return _fileType; }
    std::vector<std::string> const &arguments() const
    { return _arguments; }
    std::vector<std::string> const &relevantArguments() const
    { return _relevantArguments; }

public:
    std::string hash() const;
    std::vector<uint8_t> serialize() const;

public:
    pbxsetting::Value logicalOutputPath() const;
    pbxsetting::Value compileOutputPath() const;
    pbxsetting::Value serializedOutputPath() const;

public:
    static PrecompiledHeaderInfo
    Create(pbxspec::PBX::Compiler::shared_ptr const &compiler, std::string const &prefixHeader, pbxspec::PBX::FileType::shared_ptr const &fileType, std::vector<std::string> const &arguments);
};

}
}

#endif // !__pbxbuild_Tool_PrecompiledHeaderInfo_h
