/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_HeadermapInfo_h
#define __pbxbuild_Tool_HeadermapInfo_h

#include <string>
#include <vector>

namespace pbxbuild {
namespace Tool {

class HeadermapInfo {
private:
    std::vector<std::string> _systemHeadermapFiles;
    std::vector<std::string> _userHeadermapFiles;

public:
    HeadermapInfo();
    ~HeadermapInfo();

public:
    std::vector<std::string> const &systemHeadermapFiles() const
    { return _systemHeadermapFiles; }
    std::vector<std::string> const &userHeadermapFiles() const
    { return _userHeadermapFiles; }

public:
    std::vector<std::string> &systemHeadermapFiles()
    { return _systemHeadermapFiles; }
    std::vector<std::string> &userHeadermapFiles()
    { return _userHeadermapFiles; }
};

}
}

#endif // !__pbxbuild_Tool_HeadermapInfo_h
