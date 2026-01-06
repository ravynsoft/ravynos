/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcexecution/SimpleExecutor.h>

#include <xcexecution/Parameters.h>
#include <builtin/Driver.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/PhaseInvocations.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>
#include <process/MemoryContext.h>
#include <process/Launcher.h>
#include <process/User.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <set>

using xcexecution::SimpleExecutor;
using xcexecution::Parameters;
using libutil::Filesystem;
using libutil::FSUtil;
using libutil::Permissions;

SimpleExecutor::
SimpleExecutor(std::shared_ptr<xcformatter::Formatter> const &formatter, bool dryRun, builtin::Registry const &builtins) :
    Executor (formatter, dryRun, false),
    _builtins(builtins)
{
}

SimpleExecutor::
~SimpleExecutor()
{
}

bool SimpleExecutor::
build(
    process::User const *user,
    process::Context const *processContext,
    process::Launcher *processLauncher,
    Filesystem *filesystem,
    pbxbuild::Build::Environment const &buildEnvironment,
    Parameters const &buildParameters)
{
    ext::optional<pbxbuild::WorkspaceContext> workspaceContext = buildParameters.loadWorkspace(filesystem, user->userName(), buildEnvironment, processContext->currentDirectory());
    if (!workspaceContext) {
        return false;
    }

    ext::optional<pbxbuild::Build::Context> buildContext = buildParameters.createBuildContext(*workspaceContext);
    if (!buildContext) {
        return false;
    }

    xcformatter::Formatter::Print(_formatter->begin(*buildContext));

    ext::optional<pbxbuild::DirectedGraph<pbxproj::PBX::Target::shared_ptr>> targetGraph = buildParameters.resolveDependencies(buildEnvironment, *buildContext);
    if (!targetGraph) {
        return false;
    }

    ext::optional<std::vector<pbxproj::PBX::Target::shared_ptr>> orderedTargets = targetGraph->ordered();
    if (!orderedTargets) {
        fprintf(stderr, "error: cycle detected in target dependencies\n");
        return false;
    }

    for (pbxproj::PBX::Target::shared_ptr const &target : *orderedTargets) {
        xcformatter::Formatter::Print(_formatter->beginTarget(*buildContext, target));

        ext::optional<pbxbuild::Target::Environment> targetEnvironment = buildContext->targetEnvironment(buildEnvironment, target);
        if (!targetEnvironment) {
            fprintf(stderr, "error: couldn't create target environment for %s\n", target->name().c_str());
            xcformatter::Formatter::Print(_formatter->finishTarget(*buildContext, target));
            return false;
        }

        xcformatter::Formatter::Print(_formatter->beginCheckDependencies(target));
        pbxbuild::Phase::Environment phaseEnvironment = pbxbuild::Phase::Environment(buildEnvironment, *buildContext, target, *targetEnvironment);
        pbxbuild::Phase::PhaseInvocations phaseInvocations = pbxbuild::Phase::PhaseInvocations::Create(phaseEnvironment, target);
        xcformatter::Formatter::Print(_formatter->finishCheckDependencies(target));

        auto result = buildTarget(processContext, processLauncher, filesystem, target, *targetEnvironment, phaseInvocations.auxiliaryFiles(), phaseInvocations.invocations());
        if (!result.first) {
            xcformatter::Formatter::Print(_formatter->finishTarget(*buildContext, target));
            xcformatter::Formatter::Print(_formatter->failure(*buildContext, result.second));
            return false;
        }

        xcformatter::Formatter::Print(_formatter->finishTarget(*buildContext, target));
    }

    xcformatter::Formatter::Print(_formatter->success(*buildContext));
    return true;
}

static ext::optional<std::vector<pbxbuild::Tool::Invocation>>
SortInvocations(std::vector<pbxbuild::Tool::Invocation> const &invocations)
{
    std::unordered_map<std::string, pbxbuild::Tool::Invocation const *> outputToInvocation;
    std::set<uint32_t, std::less<uint32_t>> orderedPhasePriorities;
    for (pbxbuild::Tool::Invocation const &invocation : invocations) {
        for (std::string const &output : invocation.outputs()) {
            outputToInvocation.insert({ output, &invocation });
        }
        orderedPhasePriorities.insert(invocation.priority());
    }

    pbxbuild::DirectedGraph<pbxbuild::Tool::Invocation const *> graph;
    for (pbxbuild::Tool::Invocation const &invocation : invocations) {
        std::unordered_set<pbxbuild::Tool::Invocation const *> emptySet;
        graph.insert(&invocation, emptySet);

        for (std::string const &input : invocation.inputs()) {
            auto it = outputToInvocation.find(input);
            if (it != outputToInvocation.end()) {
                graph.insert(&invocation, { it->second });
            }
        }
        for (std::string const &phonyInputs : invocation.phonyInputs()) {
            auto it = outputToInvocation.find(phonyInputs);
            if (it != outputToInvocation.end()) {
                graph.insert(&invocation, { it->second });
            }
        }
        for (std::string const &inputDependency : invocation.inputDependencies()) {
            auto it = outputToInvocation.find(inputDependency);
            if (it != outputToInvocation.end()) {
                graph.insert(&invocation, { it->second });
            }
        }

        auto it = orderedPhasePriorities.find(invocation.priority());
        if (it != orderedPhasePriorities.end() && std::next(it) != orderedPhasePriorities.end()) {
            for (pbxbuild::Tool::Invocation const &otherInvocation : invocations) {
                if (otherInvocation.priority() == *std::next(it)) {
                    graph.insert(&otherInvocation, { &invocation });
                }
            }
        }
    }

    std::vector<pbxbuild::Tool::Invocation> result;

    ext::optional<std::vector<pbxbuild::Tool::Invocation const *>> orderedInvocations = graph.ordered();
    if (!orderedInvocations) {
        return ext::nullopt;
    }

    for (pbxbuild::Tool::Invocation const *invocation : *orderedInvocations) {
        result.push_back(*invocation);
    }
    return result;
}

bool SimpleExecutor::
writeAuxiliaryFiles(
    Filesystem *filesystem,
    std::vector<pbxbuild::Tool::AuxiliaryFile> const &auxiliaryFiles)
{
    for (pbxbuild::Tool::AuxiliaryFile const &auxiliaryFile : auxiliaryFiles) {
        std::string directory = FSUtil::GetDirectoryName(auxiliaryFile.path());
        if (filesystem->type(directory) != Filesystem::Type::Directory) {
            xcformatter::Formatter::Print(_formatter->createAuxiliaryDirectory(directory));

            if (!_dryRun) {
                if (!filesystem->createDirectory(directory, true)) {
                    return false;
                }
            }
        }

        xcformatter::Formatter::Print(_formatter->writeAuxiliaryFile(auxiliaryFile.path()));

        if (!_dryRun) {
            std::vector<uint8_t> data;

            for (pbxbuild::Tool::AuxiliaryFile::Chunk const &chunk : auxiliaryFile.chunks()) {
                switch (chunk.type()) {
                    case pbxbuild::Tool::AuxiliaryFile::Chunk::Type::Data: {
                        data.insert(data.end(), chunk.data()->begin(), chunk.data()->end());
                        break;
                    }
                    case pbxbuild::Tool::AuxiliaryFile::Chunk::Type::File: {
                        std::vector<uint8_t> contents;
                        if (!filesystem->read(&contents, *chunk.file())) {
                            return false;
                        }
                        data.insert(data.end(), contents.begin(), contents.end());
                        break;
                    }
                    default: abort();
                }
            }

            if (!filesystem->write(data, auxiliaryFile.path())) {
                return false;
            }
        }

        if (auxiliaryFile.executable() && !filesystem->isExecutable(auxiliaryFile.path())) {
            xcformatter::Formatter::Print(_formatter->setAuxiliaryExecutable(auxiliaryFile.path()));

            if (!_dryRun) {
                Permissions permissions = Permissions(
                    { Permissions::Permission::Read, Permissions::Permission::Write, Permissions::Permission::Execute },
                    { Permissions::Permission::Read, Permissions::Permission::Execute },
                    { Permissions::Permission::Read, Permissions::Permission::Execute });
                if (!filesystem->writeFilePermissions(auxiliaryFile.path(), Permissions::Operation::Set, permissions)) {
                    return false;
                }
            }
        }
    }

    return true;
}

std::pair<bool, std::vector<pbxbuild::Tool::Invocation>> SimpleExecutor::
performInvocations(
    process::Context const *processContext,
    process::Launcher *processLauncher,
    Filesystem *filesystem,
    std::vector<std::string> const &executablePaths,
    std::vector<pbxbuild::Tool::Invocation> const &orderedInvocations,
    bool createProductStructure)
{
    for (pbxbuild::Tool::Invocation const &invocation : orderedInvocations) {
        // TODO(grp): This should perhaps be a separate flag for a 'phony' invocation.
        if (!invocation.executable()) {
            continue;
        }
        pbxbuild::Tool::Invocation::Executable const &executable = *invocation.executable();

        if (invocation.createsProductStructure() != createProductStructure) {
            continue;
        }

        if (!_dryRun) {
            bool success = true;

            for (std::string const &output : invocation.outputs()) {
                std::string directory = FSUtil::GetDirectoryName(output);

                if (!filesystem->createDirectory(directory, true)) {
                    return std::make_pair(false, std::vector<pbxbuild::Tool::Invocation>({ invocation }));
                }
            }

            if (ext::optional<std::string> const &builtin = executable.builtin()) {
                /* Builtin tool, find and run in-process. */
                if (std::shared_ptr<builtin::Driver> driver = _builtins.driver(*builtin)) {
                    xcformatter::Formatter::Print(_formatter->beginInvocation(invocation, *builtin, createProductStructure));

                    process::MemoryContext context = process::MemoryContext(
                        *builtin,
                        invocation.workingDirectory(),
                        invocation.arguments(),
                        invocation.environment());
                    int exitCode = driver->run(&context, filesystem);
                    success = (exitCode == 0);

                    xcformatter::Formatter::Print(_formatter->finishInvocation(invocation, *builtin, createProductStructure));
                } else {
                    /* Failed to find builtin tool. */
                    return std::make_pair(false, std::vector<pbxbuild::Tool::Invocation>({ invocation }));
                }
            } else if (ext::optional<std::string> const &external = executable.external()) {
                /* External tool, find on the filesystem. */
                ext::optional<std::string> path;
                if (FSUtil::IsAbsolutePath(*external)) {
                    if (filesystem->isExecutable(*external)) {
                        path = external;
                    }
                } else {
                    path = filesystem->findExecutable(*external, executablePaths);
                }

                if (path) {
                    xcformatter::Formatter::Print(_formatter->beginInvocation(invocation, *path, createProductStructure));

                    /* Create the execution environment from the process and invocation environments, preferring the invocation. */
                    std::unordered_map<std::string, std::string> environment = invocation.environment();
                    environment.insert(processContext->environmentVariables().begin(), processContext->environmentVariables().end());

                    process::MemoryContext context = process::MemoryContext(
                        *path,
                        invocation.workingDirectory(),
                        invocation.arguments(),
                        environment);
                    ext::optional<int> exitCode = processLauncher->launch(filesystem, &context);
                    success = (exitCode && *exitCode == 0);

                    xcformatter::Formatter::Print(_formatter->finishInvocation(invocation, *path, createProductStructure));
                } else {
                    /* Failed to find executable. */
                    return std::make_pair(false, std::vector<pbxbuild::Tool::Invocation>({ invocation }));
                }
            } else {
                abort();
            }

            if (!success) {
                return std::make_pair(false, std::vector<pbxbuild::Tool::Invocation>({ invocation }));
            }
        }
    }

    return std::make_pair(true, std::vector<pbxbuild::Tool::Invocation>());
}

std::pair<bool, std::vector<pbxbuild::Tool::Invocation>> SimpleExecutor::
buildTarget(
    process::Context const *processContext,
    process::Launcher *processLauncher,
    Filesystem *filesystem,
    pbxproj::PBX::Target::shared_ptr const &target,
    pbxbuild::Target::Environment const &targetEnvironment,
    std::vector<pbxbuild::Tool::AuxiliaryFile> const &auxiliaryFiles,
    std::vector<pbxbuild::Tool::Invocation> const &invocations)
{
    xcformatter::Formatter::Print(_formatter->beginWriteAuxiliaryFiles(target));
    bool auxiliaryFilesSuccess = this->writeAuxiliaryFiles(filesystem, auxiliaryFiles);
    xcformatter::Formatter::Print(_formatter->finishWriteAuxiliaryFiles(target));
    if (!auxiliaryFilesSuccess) {
        return std::make_pair(false, std::vector<pbxbuild::Tool::Invocation>());
    }

    ext::optional<std::vector<pbxbuild::Tool::Invocation>> orderedInvocations = SortInvocations(invocations);
    if (!orderedInvocations) {
        fprintf(stderr, "error: cycle detected building invocation graph\n");
        return std::make_pair(false, std::vector<pbxbuild::Tool::Invocation>());
    }

    xcformatter::Formatter::Print(_formatter->beginCreateProductStructure(target));
    std::pair<bool, std::vector<pbxbuild::Tool::Invocation>> structureResult = performInvocations(processContext, processLauncher, filesystem, targetEnvironment.executablePaths(), *orderedInvocations, true);
    xcformatter::Formatter::Print(_formatter->finishCreateProductStructure(target));
    if (!structureResult.first) {
        return structureResult;
    }

    std::pair<bool, std::vector<pbxbuild::Tool::Invocation>> invocationsResult = performInvocations(processContext, processLauncher, filesystem, targetEnvironment.executablePaths(), *orderedInvocations, false);
    if (!invocationsResult.first) {
        return invocationsResult;
    }

    return std::make_pair(true, std::vector<pbxbuild::Tool::Invocation>());
}

std::unique_ptr<SimpleExecutor> SimpleExecutor::
Create(std::shared_ptr<xcformatter::Formatter> const &formatter, bool dryRun, builtin::Registry const &builtins)
{
    return std::unique_ptr<SimpleExecutor>(new SimpleExecutor(
        formatter,
        dryRun,
        builtins
    ));
}
