/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_CompilationInfo_h
#define __pbxbuild_Tool_CompilationInfo_h

#include <pbxbuild/Tool/PrecompiledHeaderInfo.h>

namespace pbxbuild {
namespace Tool {

class CompilationInfo {
private:
    std::unordered_map<std::string, PrecompiledHeaderInfo> _precompiledHeaderInfo;

private:
    std::string                                            _linkerDriver;
    std::vector<std::string>                               _linkerArguments;

public:
    CompilationInfo();
    ~CompilationInfo();

public:
    std::unordered_map<std::string, PrecompiledHeaderInfo> const &precompiledHeaderInfo() const
    { return _precompiledHeaderInfo; }

public:
    std::unordered_map<std::string, PrecompiledHeaderInfo> &precompiledHeaderInfo()
    { return _precompiledHeaderInfo; }

public:
    std::string const &linkerDriver() const
    { return _linkerDriver; }
    std::vector<std::string> const &linkerArguments() const
    { return _linkerArguments; }

public:
    std::string &linkerDriver()
    { return _linkerDriver; }
    std::vector<std::string> &linkerArguments()
    { return _linkerArguments; }
};

}
}

#endif // !__pbxbuild_Tool_CompilationInfo_h
