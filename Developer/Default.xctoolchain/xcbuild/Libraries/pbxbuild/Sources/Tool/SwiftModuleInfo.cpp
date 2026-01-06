/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/SwiftModuleInfo.h>

namespace Tool = pbxbuild::Tool;

Tool::SwiftModuleInfo::
SwiftModuleInfo(
    std::string const &architecture,
    std::string const &moduleName,
    std::string const &modulePath,
    std::string const &docPath,
    std::string const &headerPath,
    bool installHeader) :
    _architecture (architecture),
    _moduleName   (moduleName),
    _modulePath   (modulePath),
    _docPath      (docPath),
    _headerPath   (headerPath),
    _installHeader(installHeader)
{
}
