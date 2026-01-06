/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcformatter_Formatter_h
#define __xcformatter_Formatter_h

#include <pbxproj/PBX/Target.h>

namespace pbxbuild {
namespace Build { class Context; }
namespace Tool { class Invocation; }
}

namespace xcformatter {

/*
 * Abstract formatter for build output.
 */
class Formatter {
protected:
    Formatter();

public:
    virtual ~Formatter();

public:
    virtual std::string begin(pbxbuild::Build::Context const &buildContext) = 0;
    virtual std::string success(pbxbuild::Build::Context const &buildContext) = 0;
    virtual std::string failure(pbxbuild::Build::Context const &buildContext, std::vector<pbxbuild::Tool::Invocation> const &failingInvocations) = 0;

public:
    virtual std::string beginTarget(pbxbuild::Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target) = 0;
    virtual std::string finishTarget(pbxbuild::Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target) = 0;

public:
    virtual std::string beginCheckDependencies(pbxproj::PBX::Target::shared_ptr const &target) = 0;
    virtual std::string finishCheckDependencies(pbxproj::PBX::Target::shared_ptr const &target) = 0;

public:
    virtual std::string beginWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target) = 0;
    virtual std::string createAuxiliaryDirectory(std::string const &directory) = 0;
    virtual std::string writeAuxiliaryFile(std::string const &file) = 0;
    virtual std::string setAuxiliaryExecutable(std::string const &file) = 0;
    virtual std::string finishWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target) = 0;

public:
    virtual std::string beginCreateProductStructure(pbxproj::PBX::Target::shared_ptr const &target) = 0;
    virtual std::string finishCreateProductStructure(pbxproj::PBX::Target::shared_ptr const &target) = 0;

public:
    virtual std::string beginInvocation(pbxbuild::Tool::Invocation const &invocation, std::string const &executable, bool simple) = 0;
    virtual std::string finishInvocation(pbxbuild::Tool::Invocation const &invocation, std::string const &executable, bool simple) = 0;

public:
    /*
     * Utility function to print a formatted string to standard output. This
     * is less for use by formatters than by the clients of formatters.
     */
    static void Print(std::string const &output);
};

}

#endif // !__xcformatter_Formatter_h
