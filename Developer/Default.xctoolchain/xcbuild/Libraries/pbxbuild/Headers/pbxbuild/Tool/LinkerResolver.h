/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_LinkerResolver_h
#define __pbxbuild_Tool_LinkerResolver_h

#include <pbxbuild/Tool/Invocation.h>
#include <pbxbuild/Tool/Input.h>

namespace pbxbuild {
namespace Tool {

class Context;

class LinkerResolver {
private:
    pbxspec::PBX::Linker::shared_ptr _linker;

public:
    explicit LinkerResolver(pbxspec::PBX::Linker::shared_ptr const &tool);
    ~LinkerResolver();

public:
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        std::vector<Tool::Input> const &inputFiles,
        std::vector<Tool::Input> const &inputLibraries,
        std::string const &output,
        std::vector<std::string> const &additionalArguments,
        std::string const &executable = ""
    );

public:
    pbxspec::PBX::Linker::shared_ptr const &linker() const
    { return _linker; }

public:
    static std::string LinkerToolIdentifier()
    { return "com.apple.pbx.linkers.ld"; }
    static std::string LibtoolToolIdentifier()
    { return "com.apple.pbx.linkers.libtool"; }
    static std::string LipoToolIdentifier()
    { return "com.apple.xcode.linkers.lipo"; }

public:
    static std::unique_ptr<LinkerResolver>
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains, std::string const &identifier);
};

}
}

#endif // !__pbxbuild_Tool_LinkerResolver_h
