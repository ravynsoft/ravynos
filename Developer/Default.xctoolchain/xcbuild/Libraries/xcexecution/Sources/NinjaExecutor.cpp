/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcexecution/NinjaExecutor.h>

#include <xcexecution/Parameters.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/PhaseInvocations.h>
#include <ninja/Writer.h>
#include <ninja/Value.h>
#include <plist/Data.h>
#include <libutil/Escape.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>
#include <process/MemoryContext.h>
#include <process/Launcher.h>
#include <process/User.h>
#include <libutil/md5.h>

#include <sstream>
#include <iomanip>

#include <sys/types.h>
#include <sys/stat.h>

using xcexecution::NinjaExecutor;
using xcexecution::Parameters;
using libutil::Escape;
using libutil::Filesystem;
using libutil::FSUtil;

NinjaExecutor::
NinjaExecutor(std::shared_ptr<xcformatter::Formatter> const &formatter, bool dryRun, bool generate) :
    Executor(formatter, dryRun, generate)
{
}

NinjaExecutor::
~NinjaExecutor()
{
}

static std::string
TargetNinjaBegin(pbxproj::PBX::Target::shared_ptr const &target)
{
    return "begin-target-" + target->name();
}

static std::string
TargetNinjaWriteAuxiliaryFiles(pbxproj::PBX::Target::shared_ptr const &target)
{
    return "write-auxiliary-files-" + target->name();
}

static std::string
TargetNinjaFinish(pbxproj::PBX::Target::shared_ptr const &target)
{
    return "finish-target-" + target->name();
}

static std::string
TargetPhaseNinjaBegin(pbxproj::PBX::Target::shared_ptr const &target, int phasePriority)
{
    return "begin-target-" + target->name() + "-phase-priority-" + std::to_string(phasePriority);
}

static std::string
TargetPhaseNinjaFinish(pbxproj::PBX::Target::shared_ptr const &target, int phasePriority)
{
    return "finish-target-" + target->name() + "-phase-priority-" + std::to_string(phasePriority);
}

static std::string
TargetNinjaPath(pbxproj::PBX::Target::shared_ptr const &target, pbxbuild::Target::Environment const &targetEnvironment)
{
    /*
     * Determine where the Ninja file should go. We use the target's temp dir
     * as, being target-specific, it will allow the Ninja files to not conflict.
     */
    pbxsetting::Environment const &environment = targetEnvironment.environment();
    std::string temporaryDirectory = environment.resolve("TARGET_TEMP_DIR");
    // TODO(grp): How to handle varying configurations / actions / other build context options?

    return temporaryDirectory + "/" + "build.ninja";
}

static std::string
NinjaRuleName()
{
    return "invoke";
}

static std::string
NinjaDescription(std::string const &description)
{
    /* Limit to the first line: Ninja can only handle a single line status. */
    std::string::size_type newline = description.find('\n');
    if (newline != std::string::npos) {
        return description.substr(0, description.find('\n'));
    } else {
        return description;
    }
}

static std::string
NinjaHash(char const *data, size_t size)
{
    md5_state_t state;
    md5_init(&state);
    md5_append(&state, reinterpret_cast<const md5_byte_t *>(data), size);
    uint8_t digest[16];
    md5_finish(&state, reinterpret_cast<md5_byte_t *>(&digest));

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t c : digest) {
        ss << std::setw(2) << static_cast<int>(c);
    }

    return ss.str();
}

static ext::optional<std::string>
NinjaBuiltinExecutablePath(
    process::Context const *processContext,
    Filesystem const *filesystem,
    std::string builtinExecutable)
{
    std::vector<std::string> builtinExecutablePaths;
    ext::optional<std::string> processExecutablePath = processContext->executablePath();
    while (true) {
        if (!processExecutablePath) {
            break;
        }
        std::string builtinExecutablePathDirectory = FSUtil::GetDirectoryName(*processExecutablePath);
        builtinExecutablePaths.push_back(builtinExecutablePathDirectory);
        if (filesystem->type(*processExecutablePath) == Filesystem::Type::SymbolicLink) {
            processExecutablePath = filesystem->readSymbolicLink(*processExecutablePath);
        } else {
            break;
        }
    }
    return filesystem->findExecutable(builtinExecutable, builtinExecutablePaths);
}

static ext::optional<std::string>
NinjaExecutablePath(
    process::Context const *processContext,
    Filesystem const *filesystem,
    std::vector<std::string> const &executablePaths,
    pbxbuild::Tool::Invocation::Executable const &executable)
{
    if (ext::optional<std::string> const &builtin = executable.builtin()) {
        return NinjaBuiltinExecutablePath(processContext, filesystem, *builtin);
    } else if (ext::optional<std::string> const &external = executable.external()) {
        if (FSUtil::IsAbsolutePath(*external)) {
            return *external;
        } else {
            return filesystem->findExecutable(*external, executablePaths);
        }
    } else {
        abort();
    }
}

static std::string
NinjaInvocationPhonyOutput(pbxbuild::Tool::Invocation const &invocation)
{
    /*
     * This is a hack to support invocations that have no outputs. Ninja requires
     * all targets to have an output, but in some cases (usually with build script
     * invocations), the invocation has no outputs.
     *
     * In that case, generate a phony output name that will never exist, so the
     * target will always be out of date. This is expected, since with no outputs
     * there's no way to tell if the invocation needs to run.
     */

    // TODO(grp): Handle identical phony output invocations in a build.
    std::string key;

    if (invocation.executable()) {
        if (invocation.executable()->builtin()) {
            key += *invocation.executable()->builtin();
        } else if (invocation.executable()->external()) {
            key += *invocation.executable()->external();
        } else {
            abort();
        }
    }

    for (std::string const &arg : invocation.arguments()) {
        key += " " + arg;
    }

    return ".ninja-phony-output-" + NinjaHash(key.data(), key.size());
}

static std::vector<std::string>
NinjaInvocationOutputs(pbxbuild::Tool::Invocation const &invocation)
{
    std::vector<std::string> outputs;

    if (!invocation.outputs().empty()) {
        outputs.insert(outputs.end(), invocation.outputs().begin(), invocation.outputs().end());
    } else {
        outputs.push_back(NinjaInvocationPhonyOutput(invocation));
    }

    return outputs;
}

static void
WriteNinjaRegenerate(
    ninja::Writer *writer,
    Parameters const &buildParameters,
    std::string const &executablePath,
    std::string const &workingDirectory,
    std::string const &ninjaPath,
    std::string const &configurationHashPath,
    std::vector<std::string> const &inputPaths)
{
    /*
     * Regenerate using this executor. Force regeneration to avoid recursively
     * executing Ninja when Ninja itself calls this generate command.
     */
    std::vector<std::string> generateArguments = { "-generate", "-executor", "ninja" };

    /*
     * Add arguments necessary to recreate the same set of build parameters.
     */
    std::vector<std::string> parameterArguments = buildParameters.canonicalArguments();
    generateArguments.insert(generateArguments.end(), parameterArguments.begin(), parameterArguments.end());

    /*
     * Re-run the current executable to re-generate the Ninja file. There is an
     * implicit assumption here that this executable takes the parameters above.
     */
    std::string exec = Escape::Shell(executablePath);

    /*
     * Escape executable and input parameters for Ninja.
     */
    for (std::string const &arg : generateArguments) {
        exec += " " + Escape::Shell(arg);
    }
    std::vector<ninja::Value> inputPathValues;
    inputPathValues.push_back(ninja::Value::String(configurationHashPath));
    for (std::string const &inputPath : inputPaths) {
        inputPathValues.push_back(ninja::Value::String(inputPath));
    }

    /*
     * If the generator has changed, the ninja file needs to be regenerated as well.
     */
    inputPathValues.push_back(ninja::Value::String(executablePath));

    /*
     * Write out the Ninja rule to regenerate sources.
     */
    std::string ruleName = "regenerate";
    writer->rule(ruleName, ninja::Value::Expression("cd $dir && $exec"));
    writer->build({ ninja::Value::String(ninjaPath) }, ruleName, inputPathValues, {
        { "dir", ninja::Value::String(Escape::Shell(workingDirectory)) },
        { "exec", ninja::Value::String(exec) },
        { "description", ninja::Value::String("Regenerating Ninja files...") },

        /* This command regenerates the Ninja files. */
        { "generator", ninja::Value::String("1") },

        /* Use the console pool to pass through terminal settings. */
        { "pool", ninja::Value::String("console") },
    });
}

static bool
WriteNinja(Filesystem *filesystem, ninja::Writer const &writer, std::string const &path)
{
    if (!filesystem->createDirectory(FSUtil::GetDirectoryName(path), true)) {
        return false;
    }

    std::string contents = writer.serialize();
    std::vector<uint8_t> copy = std::vector<uint8_t>(contents.begin(), contents.end());
    if (!filesystem->write(copy, path)) {
        return false;
    }

    return true;
}

static bool
WriteAuxiliaryFiles(Filesystem *filesystem, std::map<std::string, pbxbuild::Tool::AuxiliaryFile::Chunk const *> &auxiliaryFileChunks)
{
    for (auto it : auxiliaryFileChunks) {
        if (!filesystem->createDirectory(FSUtil::GetDirectoryName(it.first), true)) {
            return false;
        }

        if (!filesystem->write(*it.second->data(), it.first)) {
            return false;
        }
    }

    return true;
}


static bool
ShouldGenerateNinja(Filesystem const *filesystem, bool generate, Parameters const &buildParameters, std::string const &ninjaPath, std::string const &configurationHashPath)
{
    /*
     * If explicitly asked to generate, definitely need to regenerate.
     */
    if (generate) {
        return true;
    }

    /*
     * If the Ninja file doesn't exist, must re-generate to create it.
     */
    if (!filesystem->exists(ninjaPath)) {
        return true;
    }

    /*
     * If the configuration hash doesn't exist, the configuration is unknown so
     * the Ninja must be regenerated to ensure it matches the configuration.
     */
    if (!filesystem->exists(configurationHashPath)) {
        return true;
    }

    /*
     * If the contents of the configration hash doesn't match, need to update for
     * the new configuration.
     */
    std::vector<uint8_t> contents;
    if (!filesystem->read(&contents, configurationHashPath)) {
        /* Can't be read, same as not existing. */
        return true;
    }
    if (std::string(contents.begin(), contents.end()) != buildParameters.canonicalHash()) {
        return true;
    }

    /*
     * Nothing changed, safe to use the cached Ninja.
     */
    return false;
}

bool NinjaExecutor::
build(
    process::User const *user,
    process::Context const *processContext,
    process::Launcher *processLauncher,
    Filesystem *filesystem,
    pbxbuild::Build::Environment const &buildEnvironment,
    Parameters const &buildParameters)
{
    /*
     * Load the derived data hash in order to output the Ninja file in the
     * right derived data directory. This does not load the workspace context
     * to avoid loading potentially very large projects for incremental builds.
     */
    std::string workspacePath = filesystem->resolvePath(buildParameters.workspace().value_or(*buildParameters.project()));
    pbxbuild::DerivedDataHash derivedDataHash = pbxbuild::DerivedDataHash::Create(workspacePath);
    pbxsetting::Level derivedDataLevel = pbxsetting::Level(derivedDataHash.overrideSettings());

    /*
     * This environment contains only settings shared for the entire build.
     */
    pbxsetting::Environment environment = pbxsetting::Environment(buildEnvironment.baseEnvironment());
    environment.insertFront(derivedDataLevel, false);

    /*
     * Determine where build-level outputs will go. Note we can't use CONFIGURATION_BUILD_DIR
     * at this point because that includes the EFFECTIVE_PLATFORM_NAME, but we don't have a platform.
     */
    std::string intermediatesDirectory = environment.resolve("OBJROOT");
    std::string ninjaPath = intermediatesDirectory + "/" + "build.ninja";
    std::string configurationHashPath = intermediatesDirectory + "/" + ".ninja-configuration";

    /*
     * Find the dependency info tool.
     */
    std::string executableRoot = FSUtil::GetDirectoryName(processContext->executablePath());
    std::string dependencyInfoToolPath = *NinjaBuiltinExecutablePath(processContext, filesystem, "dependency-info-tool");

    /*
     * If the Ninja file needs to be generated, generate it.
     */
    if (ShouldGenerateNinja(filesystem, _generate, buildParameters, ninjaPath, configurationHashPath)) {
        fprintf(stderr, "Generating Ninja files...\n");

        /*
         * Load the workspace. This can be quite slow, so only do it if it's needed to generate
         * the Ninja file. Similarly, only resolve dependencies in that case.
         */
        ext::optional<pbxbuild::WorkspaceContext> workspaceContext = buildParameters.loadWorkspace(filesystem, user->userName(), buildEnvironment, processContext->currentDirectory());
        if (!workspaceContext) {
            fprintf(stderr, "error: unable to load workspace\n");
            return false;
        }

        ext::optional<pbxbuild::Build::Context> buildContext = buildParameters.createBuildContext(*workspaceContext);
        if (!buildContext) {
            fprintf(stderr, "error: unable to create build context\n");
            return false;
        }

        ext::optional<pbxbuild::DirectedGraph<pbxproj::PBX::Target::shared_ptr>> targetGraph = buildParameters.resolveDependencies(buildEnvironment, *buildContext);
        if (!targetGraph) {
            fprintf(stderr, "error: unable to resolve dependencies\n");
            return false;
        }

        /*
         * Generate the Ninja file.
         */
        bool result = buildAction(
            processContext,
            filesystem,
            buildParameters,
            buildEnvironment,
            *buildContext,
            *targetGraph,
            dependencyInfoToolPath,
            ninjaPath,
            configurationHashPath,
            intermediatesDirectory);

        if (!result) {
            fprintf(stderr, "error: failed to generate build.ninja\n");
            return false;
        }

        /*
         * Write out the configuration hash for the parameters in the Ninja.
         */
        std::string hashContents = buildParameters.canonicalHash();
        auto contents = std::vector<uint8_t>(hashContents.begin(), hashContents.end());
        if (!filesystem->write(contents, configurationHashPath)) {
            fprintf(stderr, "error: failed to generate ninja configuration hash\n");
            return false;
        }
    }

    /*
     * Only perform a build if not passing -generate. If -generate is passed, that's because Ninja
     * is already running and asking to re-generate the project file. Re-running it would recurse.
     */
    if (!_generate) {
        /*
         * Use the Ninja file just generated.
         */
        std::vector<std::string> arguments = { "-f", ninjaPath };

        /*
         * Find the path to the Ninja executable to use.
         */
        ext::optional<std::string> executable = filesystem->findExecutable("ninja", processContext->executableSearchPaths());
        if (!executable) {
            /*
             * Couldn't find standard Ninja, try with llbuild.
             */
            executable = filesystem->findExecutable("llbuild", processContext->executableSearchPaths());

            /*
             * If neither Ninja or llbuild are available, can't start the build.
             */
            if (!executable) {
                fprintf(stderr, "error: could not find ninja or llbuild in PATH\n");
                return false;
            }

            /*
             * Use llbuild's Ninja executor, which requires extra arguments.
             */
            std::vector<std::string> llbuildArguments = { "ninja", "build" };
            arguments.insert(arguments.begin(), llbuildArguments.begin(), llbuildArguments.end());
        }

        /*
         * Pass through the dry run option.
         */
        if (_dryRun) {
            arguments.push_back("-n");
        }

        // TODO(grp): Pass number of jobs if specified.

        /*
         * Run Ninja and return if it failed. Ninja itself does the build.
         */
        process::MemoryContext ninja = process::MemoryContext(
            *executable,
            intermediatesDirectory,
            arguments,
            processContext->environmentVariables());
        ext::optional<int> exitCode = processLauncher->launch(filesystem, &ninja);
        if (!exitCode || *exitCode != 0) {
            return false;
        }
    }

    return true;
}

bool NinjaExecutor::
buildAction(
    process::Context const *processContext,
    Filesystem *filesystem,
    Parameters const &buildParameters,
    pbxbuild::Build::Environment const &buildEnvironment,
    pbxbuild::Build::Context const &buildContext,
    pbxbuild::DirectedGraph<pbxproj::PBX::Target::shared_ptr> const &targetGraph,
    std::string const &dependencyInfoToolPath,
    std::string const &ninjaPath,
    std::string const &configurationHashPath,
    std::string const &intermediatesDirectory)
{
    /*
     * Write out a Ninja file for the build as a whole. Note each target will have a separate
     * file, this is to coordinate the build between targets.
     */
    ninja::Writer writer;
    writer.comment("xcbuild ninja");
    writer.comment("Action: " + buildContext.action());
    if (buildContext.workspaceContext().workspace() != nullptr) {
        writer.comment("Workspace: " + buildContext.workspaceContext().workspace()->projectFile());
    } else if (buildContext.workspaceContext().project() != nullptr) {
        writer.comment("Project: " + buildContext.workspaceContext().project()->projectFile());
    }
    if (buildContext.scheme() != nullptr) {
        writer.comment("Scheme: " + buildContext.scheme()->name());
    }
    writer.comment("Configuation: " + buildContext.configuration());
    writer.newline();

    /*
     * Ninja's intermediate outputs should also go in the temp dir.
     */
    writer.binding({ "builddir", { ninja::Value::String(intermediatesDirectory) } });
    writer.newline();

    /*
     * Since invocations are already resolved at this point, we can't use more specific
     * rules at the Ninja level. Instead, add a single rule that just passes through from
     * the build command that calls it.
     */
    writer.rule(NinjaRuleName(), ninja::Value::Expression("cd $dir && env $env $exec && $depexec"));

    /*
     * Go over each target and write out Ninja targets for the start and end of each.
     * Don't bother topologically sorting the targets now, since Ninja will do that for us.
     */
    for (pbxproj::PBX::Target::shared_ptr const &target : targetGraph.nodes()) {

        /*
         * Beginning target depends on finishing the targets before that. This is implemented
         * in three parts:
         *
         *  1. Each target has a "target begin" Ninja target depending on completing the build
         *     of any dependent targets.
         *  2. Each invocation's Ninja target depends on the "target begin" target to order
         *     them necessarily after the target started building.
         *  3. Each target also has a "target finish" Ninja target, which depends on all of
         *     the invocations created for the target.
         *
         * The end result is that targets build in the right order. Note this does not preclude
         * cross-target parallelization; if the target dependency graph doesn't have an edge,
         * then they will be parallelized. Linear builds have edges from each target to all
         * previous targets.
         */

        /*
         * Resolve this target and generate its invocations.
         */
        ext::optional<pbxbuild::Target::Environment> targetEnvironment = buildContext.targetEnvironment(buildEnvironment, target);
        if (!targetEnvironment) {
            fprintf(stderr, "error: couldn't create target environment for %s\n", target->name().c_str());
            continue;
        }

        pbxbuild::Phase::Environment phaseEnvironment = pbxbuild::Phase::Environment(buildEnvironment, buildContext, target, *targetEnvironment);
        pbxbuild::Phase::PhaseInvocations phaseInvocations = pbxbuild::Phase::PhaseInvocations::Create(phaseEnvironment, target);

        /*
         * As described above, the target's begin depends on all of the target dependencies.
         */
        std::vector<ninja::Value> dependenciesFinished;
        for (pbxproj::PBX::Target::shared_ptr const &dependency : targetGraph.adjacent(target)) {
            std::string targetFinished = TargetNinjaFinish(dependency);
            dependenciesFinished.push_back(ninja::Value::String(targetFinished));
        }

        /*
         * Add the phony target for beginning this target's build.
         */
        std::string targetBegin = TargetNinjaBegin(target);
        writer.build({ ninja::Value::String(targetBegin) }, "phony", dependenciesFinished);

        /*
         * Add the phony target for the checkpoint after writing auxiliary files.
         */
        std::string targetWriteAuxiliaryFiles = TargetNinjaWriteAuxiliaryFiles(target);
        std::vector<ninja::Value> auxiliaryFileOutputs = { ninja::Value::String(targetBegin) };
        for (pbxbuild::Tool::AuxiliaryFile const &auxiliaryFile : phaseInvocations.auxiliaryFiles()) {
            auxiliaryFileOutputs.push_back(ninja::Value::String(auxiliaryFile.path()));
        }
        writer.build({ ninja::Value::String(targetWriteAuxiliaryFiles) }, "phony", auxiliaryFileOutputs);

        /*
         * Write out the Ninja file to build this target.
         */
        if (!buildTargetInvocations(processContext, filesystem, dependencyInfoToolPath, target, *targetEnvironment, phaseInvocations.auxiliaryFiles(), phaseInvocations.invocations())) {
            fprintf(stderr, "error: failed to build target ninja\n");
            return false;
        }

        /*
         * Load the Ninja file generated for this target.
         */
        std::string targetPath = TargetNinjaPath(target, *targetEnvironment);
        writer.subninja(ninja::Value::String(targetPath));

        /*
         * As described above, the target's finish depends on all of the invocation outputs.
         */
        std::unordered_set<std::string> invocationOutputs;
        for (pbxbuild::Tool::Invocation const &invocation : phaseInvocations.invocations()) {
            if (!invocation.executable()) {
                /* No outputs. */
                continue;
            }

            std::vector<std::string> outputs = NinjaInvocationOutputs(invocation);
            invocationOutputs.insert(outputs.begin(), outputs.end());
        }

        /*
         * Add phony rules for input dependencies that we don't know if they exist.
         * This can come up, for example, for user-specified custom script inputs.
         * However, avoid adding the phony invocation if a real output *does* include
         * the phony input, to avoid Ninja complaining about duplicate rules.
         */
        for (pbxbuild::Tool::Invocation const &invocation : phaseInvocations.invocations()) {
            for (std::string const &phonyInput : invocation.phonyInputs()) {
                if (invocationOutputs.find(phonyInput) == invocationOutputs.end()) {
                    writer.build({ ninja::Value::String(phonyInput) }, "phony", { });
                }
            }
        }

        /*
         * Add the phony target for ending this target's build.
         */
        uint32_t maxInvocationPriority = 0;
        for (pbxbuild::Tool::Invocation const &invocation : phaseInvocations.invocations()) {
            maxInvocationPriority = std::max(maxInvocationPriority, invocation.priority());
        }
        std::string targetFinish = TargetNinjaFinish(target);
        writer.build({ ninja::Value::String(targetFinish) }, "phony", { ninja::Value::String(TargetPhaseNinjaFinish(target, maxInvocationPriority)) });
    }

    /*
     * Build up a list of all of the inputs to the build, so Ninja can regenerate as necessary.
     */
    std::vector<std::string> inputPaths = buildContext.workspaceContext().loadedFilePaths();

    /*
     * Add a Ninja rule to regenerate the build.ninja file itself.
     */
    WriteNinjaRegenerate(
        &writer,
        buildParameters,
        processContext->executablePath(),
        processContext->currentDirectory(),
        ninjaPath,
        configurationHashPath,
        inputPaths);

    /*
     * Serialize the Ninja file into the build root.
     */
    if (!WriteNinja(filesystem, writer, ninjaPath)) {
        fprintf(stderr, "error: failed to write Ninja to %s\n", ninjaPath.c_str());
        return false;
    }

    /*
     * Note where the Ninja file is written.
     */
    fprintf(stderr, "Wrote Ninja: %s\n", ninjaPath.c_str());

    return true;
}

bool NinjaExecutor::
buildTargetInvocations(
    process::Context const *processContext,
    Filesystem *filesystem,
    std::string const &dependencyInfoToolPath,
    pbxproj::PBX::Target::shared_ptr const &target,
    pbxbuild::Target::Environment const &targetEnvironment,
    std::vector<pbxbuild::Tool::AuxiliaryFile> const &auxiliaryFiles,
    std::vector<pbxbuild::Tool::Invocation> const &invocations)
{
    /*
     * Start building the Ninja file for this target.
     */
    ninja::Writer writer;
    writer.comment("xcbuild ninja");
    writer.comment("Target: " + target->name());
    writer.newline();

    std::string targetBegin = TargetNinjaBegin(target);
    std::string targetWriteAuxiliaryFiles = TargetNinjaWriteAuxiliaryFiles(target);

    pbxsetting::Environment const &environment = targetEnvironment.environment();
    std::string temporaryDirectory = environment.resolve("TARGET_TEMP_DIR");

    /*
     * Write auxiliary files to run first.
     */
    std::map<std::string, pbxbuild::Tool::AuxiliaryFile::Chunk const *> auxiliaryFileChunks;
    for (pbxbuild::Tool::AuxiliaryFile const &auxiliaryFile : auxiliaryFiles) {
        if (!buildAuxiliaryFile(&writer, auxiliaryFile, targetBegin, temporaryDirectory, auxiliaryFileChunks)) {
            return false;
        }
    }

    /*
     * Group every invocation in target by its phase priority.
     */
    std::map<int, std::unordered_set<std::string>, std::less<uint32_t>> priorityToOutputs;
    for (pbxbuild::Tool::Invocation const &invocation : invocations) {
        std::vector<std::string> invocationOutputs = NinjaInvocationOutputs(invocation);
        if (!invocationOutputs.empty()) {
            priorityToOutputs[invocation.priority()].insert(invocationOutputs.begin(), invocationOutputs.end());
        }
    }

    /*
     * Write phony targets for start and end of each phase priorities.
     */
    ninja::Value previousPhase = ninja::Value::String(targetWriteAuxiliaryFiles);
    for (auto const &priorityMappedOutputs : priorityToOutputs) {
        // begin phase phony target.
        ninja::Value targetPhaseBegin = ninja::Value::String(TargetPhaseNinjaBegin(target, priorityMappedOutputs.first));
        writer.build({ targetPhaseBegin }, "phony", { previousPhase });

        // finish phase phony target.
        ninja::Value targetPhaseFinish = ninja::Value::String(TargetPhaseNinjaFinish(target, priorityMappedOutputs.first));
        std::vector<ninja::Value> phaseFinishDependency;

        // make sure phase orders are kept by having phase begin as dependency of phase finish.
        phaseFinishDependency.push_back(targetPhaseBegin);

        for (std::string const &output: priorityMappedOutputs.second) {
            phaseFinishDependency.push_back(ninja::Value::String(output));
        }
        writer.build({ targetPhaseFinish }, "phony", phaseFinishDependency);

        // update previous phase so that next phase can depend upon it.
        previousPhase = targetPhaseFinish;
    }

    /*
     * Add the build command for each invocation.
     */
    for (pbxbuild::Tool::Invocation const &invocation : invocations) {
        // TODO(grp): This should perhaps be a separate flag for a 'phony' invocation.
        if (invocation.executable()) {
            /* Find invocation executable. */
            ext::optional<std::string> executablePath = NinjaExecutablePath(processContext, filesystem, targetEnvironment.executablePaths(), *invocation.executable());
            if (!executablePath) {
                fprintf(stderr, "unable to find executable: %s\n", invocation.executable()->builtin().value_or(invocation.executable()->external().value_or("<NONE>")).c_str());

                return false;
            }

            /* Write invocations to run after auxiliary files. */
            if (!buildInvocation(&writer, invocation, *executablePath, dependencyInfoToolPath, temporaryDirectory, TargetPhaseNinjaBegin(target, invocation.priority()))) {
                return false;
            }
        }
    }

    /*
     * Serialize the Ninja file into the build root.
     */
    std::string path = TargetNinjaPath(target, targetEnvironment);
    if (!WriteNinja(filesystem, writer, path)) {
        fprintf(stderr, "error: unable to write target ninja: %s\n", path.c_str());
        return false;
    }
    if (!WriteAuxiliaryFiles(filesystem, auxiliaryFileChunks)) {
        fprintf(stderr, "error: unable to write auxiliary files\n");
        return false;
    }

    return true;
}

bool NinjaExecutor::
buildAuxiliaryFile(
    ninja::Writer *writer,
    pbxbuild::Tool::AuxiliaryFile const &auxiliaryFile,
    std::string const &after,
    std::string const &temporaryDirectory,
    std::map<std::string, pbxbuild::Tool::AuxiliaryFile::Chunk const *> &auxiliaryFileChunks)
{
    std::vector<ninja::Value> inputs;
    std::vector<ninja::Value> outputs = { ninja::Value::String(auxiliaryFile.path()) };
    std::vector<ninja::Value> orderDependencies = { ninja::Value::String(after) };

    /*
     * Build up the command to create the auxiliary file.
     */
    std::string escapedPath = Escape::Shell(auxiliaryFile.path());
    std::string exec = "echo -n > " + escapedPath;
    for (pbxbuild::Tool::AuxiliaryFile::Chunk const &chunk : auxiliaryFile.chunks()) {
        exec += " && ";

        switch (chunk.type()) {
            case pbxbuild::Tool::AuxiliaryFile::Chunk::Type::Data: {
                // Cache auxiliary file path and data, so that we can write them in build directory later.
                const std::string auxiliaryFileChunkPath = temporaryDirectory + "/" + ".ninja-auxiliary-file-" + NinjaHash(reinterpret_cast<const char *>(chunk.data()->data()), chunk.data()->size()) + ".chunk";
                auxiliaryFileChunks.insert(std::make_pair(auxiliaryFileChunkPath, &chunk));
                exec += "cat " + Escape::Shell(auxiliaryFileChunkPath);
                inputs.push_back(ninja::Value::String(auxiliaryFileChunkPath));
                break;
            }
            case pbxbuild::Tool::AuxiliaryFile::Chunk::Type::File: {
                exec += "cat " + Escape::Shell(*chunk.file());
                inputs.push_back(ninja::Value::String(*chunk.file()));
                break;
            }
            default: abort();
        }

        exec += " >> ";
        exec += escapedPath;
    }

    /* Mark the file as executable if necessary. */
    if (auxiliaryFile.executable()) {
        exec += " && ";
        exec += "chmod 0755 " + escapedPath;
    }

    /*
     * Write out the auxiliary file.
     */
    std::string description = NinjaDescription(_formatter->writeAuxiliaryFile(auxiliaryFile.path()));
    std::vector<ninja::Binding> bindings = {
        { "description", ninja::Value::String(description) },
        { "dir", ninja::Value::String("/") },
        { "exec", ninja::Value::String(exec) },
        { "depexec", ninja::Value::String("true") },
    };
    writer->build(outputs, NinjaRuleName(), inputs, bindings, { }, orderDependencies);

    return true;
}

bool NinjaExecutor::
buildInvocation(
    ninja::Writer *writer,
    pbxbuild::Tool::Invocation const &invocation,
    std::string const &executablePath,
    std::string const &dependencyInfoToolPath,
    std::string const &temporaryDirectory,
    std::string const &after)
{
    /*
     * Build the invocation arguments. Must escape for shell arguments as Ninja passes
     * the command string directly to the shell, which would interpret spaces, etc as meaningful.
     */
    std::string exec = Escape::Shell(executablePath);
    for (std::string const &arg : invocation.arguments()) {
        exec += " " + Escape::Shell(arg);
    }

    /*
     * Build the invocation environment. To set the environment, we use standard shell tools:
     * `env` to avoid Bash-specific limitations on environment variables (some versions of Bash
     * don't allow setting "UID"). Intentionally add to, not replace, the process environment.
     */
    std::string environment;
    for (auto it = invocation.environment().begin(); it != invocation.environment().end(); ++it) {
        if (it != invocation.environment().begin()) {
            environment += " ";
        }
        environment += it->first + "=" + Escape::Shell(it->second);
    }

    /*
     * Determine the status message for Ninja to print for this invocation.
     */
    std::string executableDisplayName = invocation.executable()->builtin().value_or(executablePath);
    std::string description = NinjaDescription(_formatter->beginInvocation(invocation, executableDisplayName, false));

    /*
     * Add the dependency info converter & file.
     */
    std::string dependencyInfoFile;
    std::string dependencyInfoExec;

    if (!invocation.dependencyInfo().empty()) {
        /* Determine the first output; Ninja expects that as the Makefile rule. */
        std::string output = NinjaInvocationOutputs(invocation).front();

        /* Find where the generated dependency info should go. */
        dependencyInfoFile = temporaryDirectory + "/" + ".ninja-dependency-info-" + NinjaHash(output.data(), output.size()) + ".d";

        /* Build the dependency info rewriter arguments. */
        std::vector<std::string> dependencyInfoArguments = {
            "--name", output,
            "--output", dependencyInfoFile,
        };

        /* Add the input for each dependency info. */
        for (pbxbuild::Tool::Invocation::DependencyInfo const &dependencyInfo : invocation.dependencyInfo()) {
            std::string formatName;
            if (!dependency::DependencyInfoFormats::Name(dependencyInfo.format(), &formatName)) {
                return false;
            }

            dependencyInfoArguments.push_back(formatName + ":" + dependencyInfo.path());
        }

        /* Create the command for converting the dependency info. */
        dependencyInfoExec = Escape::Shell(dependencyInfoToolPath);
        for (std::string const &arg : dependencyInfoArguments) {
            dependencyInfoExec += " " + Escape::Shell(arg);
        }
    } else {
        // TODO(grp): Avoid the need for an empty dependency info command if not used.
        dependencyInfoExec = "true";
    }

    /*
     * Build up the bindings for the invocation.
     */
    std::vector<ninja::Binding> bindings = {
        { "description", ninja::Value::String(description) },
        { "dir", ninja::Value::String(Escape::Shell(invocation.workingDirectory())) },
        { "exec", ninja::Value::String(exec) },
    };
    if (!environment.empty()) {
        bindings.push_back({ "env", ninja::Value::String(environment) });
    }
    if (!dependencyInfoExec.empty()) {
        bindings.push_back({ "depexec", ninja::Value::String(dependencyInfoExec) });
    }
    if (!dependencyInfoFile.empty()) {
        bindings.push_back({ "depfile", ninja::Value::String(dependencyInfoFile) });
    }

    /*
     * Build up outputs as literal Ninja values.
     */
    std::vector<ninja::Value> outputs;
    for (std::string const &output : NinjaInvocationOutputs(invocation)) {
        outputs.push_back(ninja::Value::String(output));
    }

    /*
     * Build up inputs as literal Ninja values.
     */
    std::vector<ninja::Value> inputs;
    for (std::string const &input : invocation.inputs()) {
        inputs.push_back(ninja::Value::String(input));
    }

    /*
     * Build up input dependencies as literal Ninja values.
     */
    std::vector<ninja::Value> inputDependencies;
    for (std::string const &inputDependency : invocation.inputDependencies()) {
        inputDependencies.push_back(ninja::Value::String(inputDependency));
    }

    /*
     * Build up order dependencies as literal Ninja values.
     */
    std::vector<ninja::Value> orderDependencies;
    for (std::string const &orderDependency : invocation.orderDependencies()) {
        orderDependencies.push_back(ninja::Value::String(orderDependency));
    }

    /*
     * All invocations depend on the target containing them beginning.
     */
    orderDependencies.push_back(ninja::Value::String(after));

    /*
     * Add the rule to build this invocation.
     */
    writer->build(outputs, NinjaRuleName(), inputs, bindings, inputDependencies, orderDependencies);

    return true;
}

std::unique_ptr<NinjaExecutor> NinjaExecutor::
Create(std::shared_ptr<xcformatter::Formatter> const &formatter, bool dryRun, bool generate)
{
    return std::unique_ptr<NinjaExecutor>(new NinjaExecutor(
        formatter,
        dryRun,
        generate
    ));
}
