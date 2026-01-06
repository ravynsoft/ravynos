/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcexecution_NinjaExecutor_h
#define __xcexecution_NinjaExecutor_h

#include <xcexecution/Executor.h>
#include <pbxbuild/Tool/AuxiliaryFile.h>
#include <pbxbuild/Tool/Invocation.h>
#include <pbxbuild/DirectedGraph.h>

namespace ninja { class Writer; }

namespace xcexecution {

/*
 * Concrete executor that generates Ninja files.
 */
class NinjaExecutor : public Executor {
public:
    NinjaExecutor(std::shared_ptr<xcformatter::Formatter> const &formatter, bool dryRun, bool generate);
    ~NinjaExecutor();

public:
    virtual bool build(
        process::User const *user,
        process::Context const *processContext,
        process::Launcher *processLauncher,
        libutil::Filesystem *filesystem,
        pbxbuild::Build::Environment const &buildEnvironment,
        Parameters const &buildParameters);

private:
    bool buildAction(
        process::Context const *processContext,
        libutil::Filesystem *filesystem,
        Parameters const &buildParameters,
        pbxbuild::Build::Environment const &buildEnvironment,
        pbxbuild::Build::Context const &buildContext,
        pbxbuild::DirectedGraph<pbxproj::PBX::Target::shared_ptr> const &targetGraph,
        std::string const &dependencyInfoToolPath,
        std::string const &ninjaPath,
        std::string const &configurationHashPath,
        std::string const &intermediatesDirectory);
    bool buildOutputDirectories(
        ninja::Writer *writer,
        std::vector<pbxbuild::Tool::Invocation> const &invocations,
        std::unordered_set<std::string> *seenDirectories);
    bool buildTargetInvocations(
        process::Context const *processContext,
        libutil::Filesystem *filesystem,
        std::string const &dependencyInfoToolPath,
        pbxproj::PBX::Target::shared_ptr const &target,
        pbxbuild::Target::Environment const &targetEnvironment,
        std::vector<pbxbuild::Tool::AuxiliaryFile> const &auxiliaryFiles,
        std::vector<pbxbuild::Tool::Invocation> const &invocations);

private:
    bool buildAuxiliaryFile(
        ninja::Writer *writer,
        pbxbuild::Tool::AuxiliaryFile const &auxiliaryFile,
        std::string const &after,
        std::string const &temporaryDirectory,
        std::map<std::string, pbxbuild::Tool::AuxiliaryFile::Chunk const *> &auxiliaryFileChunks);
    bool buildInvocation(
        ninja::Writer *writer,
        pbxbuild::Tool::Invocation const &invocation,
        std::string const &executablePath,
        std::string const &dependencyInfoToolPath,
        std::string const &temporaryDirectory,
        std::string const &after);

public:
    static std::unique_ptr<NinjaExecutor>
    Create(std::shared_ptr<xcformatter::Formatter> const &formatter, bool dryRun, bool generate);
};

}

#endif // !__xcexecution_NinjaExecutor_h
