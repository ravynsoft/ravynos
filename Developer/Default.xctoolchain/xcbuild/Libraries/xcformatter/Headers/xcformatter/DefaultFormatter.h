/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcformatter_DefaultFormatter_h
#define __xcformatter_DefaultFormatter_h

#include <xcformatter/Formatter.h>

namespace xcformatter {

/*
 * Formats output with the default format.
 */
class DefaultFormatter : public Formatter {
private:
    bool _color;

public:
    DefaultFormatter(bool color);
    ~DefaultFormatter();

public:
    virtual std::string begin(pbxbuild::Build::Context const &buildContext);
    virtual std::string success(pbxbuild::Build::Context const &buildContext);
    virtual std::string failure(pbxbuild::Build::Context const &buildContext, std::vector<pbxbuild::Tool::Invocation> const &failingInvocations);

public:
    virtual std::string beginTarget(pbxbuild::Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target);
    virtual std::string finishTarget(pbxbuild::Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target);

public:
    virtual std::string beginCheckDependencies(pbxproj::PBX::Target::shared_ptr const &target);
    virtual std::string finishCheckDependencies(pbxproj::PBX::Target::shared_ptr const &target);

public:
    virtual std::string beginWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target);
    virtual std::string createAuxiliaryDirectory(std::string const &directory);
    virtual std::string writeAuxiliaryFile(std::string const &file);
    virtual std::string setAuxiliaryExecutable(std::string const &file);
    virtual std::string finishWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target);

public:
    virtual std::string beginCreateProductStructure(pbxproj::PBX::Target::shared_ptr const &target);
    virtual std::string finishCreateProductStructure(pbxproj::PBX::Target::shared_ptr const &target);

public:
    virtual std::string beginInvocation(pbxbuild::Tool::Invocation const &invocation, std::string const &executable, bool simple);
    virtual std::string finishInvocation(pbxbuild::Tool::Invocation const &invocation, std::string const &executable, bool simple);

public:
    /*
     * Creates a default formatter. If color is true, terminal escapes
     * are used for bold and colored text in the formatted output.
     */
    static std::shared_ptr<DefaultFormatter>
    Create(bool color);
};

}

#endif // !__xcformatter_DefaultFormatter_h
