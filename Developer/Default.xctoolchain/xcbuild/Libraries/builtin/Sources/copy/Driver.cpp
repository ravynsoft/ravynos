/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <builtin/copy/Driver.h>
#include <builtin/copy/Options.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>
#include <process/MemoryContext.h>
#include <process/Launcher.h>

#include <unordered_set>

using builtin::copy::Driver;
using builtin::copy::Options;
using libutil::Filesystem;
using libutil::Permissions;
using libutil::FSUtil;

Driver::
Driver()
{
}

Driver::
~Driver()
{
}

std::string Driver::
name()
{
    return "builtin-copy";
}

static bool
CopyPath(Filesystem *filesystem, std::string const &inputPath, std::string const &outputPath)
{
    /* Must be writable once copied to allow subsequent builds to overwrite. */
    Permissions permissions;
    permissions.user(Permissions::Permission::Write, true);
    Permissions::Operation operation = Permissions::Operation::Add;

    ext::optional<Filesystem::Type> type = filesystem->type(inputPath);
    if (!type) {
        /* Unknown type or doesn't exist. */
        return false;
    }

    switch (*type) {
        case Filesystem::Type::File: {
            if (!filesystem->copyFile(inputPath, outputPath)) {
                return false;
            }

            if (!filesystem->writeFilePermissions(outputPath, operation, permissions)) {
                return false;
            }

            return true;
        }
        case Filesystem::Type::SymbolicLink: {
            if (!filesystem->copySymbolicLink(inputPath, outputPath)) {
                return false;
            }

            if (!filesystem->writeSymbolicLinkPermissions(outputPath, operation, permissions)) {
                return false;
            }

            return true;
        }
        case Filesystem::Type::Directory: {
            if (!filesystem->copyDirectory(inputPath, outputPath, true)) {
                return false;
            }

            if (!filesystem->writeDirectoryPermissions(outputPath, operation, permissions, true)) {
                return false;
            }

            return true;
        }
    }

    abort();
}

static int
Run(Filesystem *filesystem, Options const &options, std::string const &workingDirectory)
{
    if (!options.output()) {
        fprintf(stderr, "error: no output path provided\n");
        return 1;
    }

    if (options.stripDebugSymbols() || options.bitcodeStrip() != Options::BitcodeStripMode::None) {
        // TODO(grp): Implement strip support when copying.
#if 0
        fprintf(stderr, "warning: strip on copy is not supported\n");
#endif
    }

    if (options.preserveHFSData()) {
        fprintf(stderr, "warning: preserve HFS data is not supported\n");
    }

    std::string const &output = FSUtil::ResolveRelativePath(*options.output(), workingDirectory);
    if (!filesystem->createDirectory(output, true)) {
        return false;
    }

#if 0
    // TODO: handle excludes paths
    auto excludes = std::unordered_set<std::string>(options.excludes().begin(), options.excludes().end());
#endif

    for (std::string input : options.inputs()) {
        input = FSUtil::ResolveRelativePath(input, workingDirectory);

        if (options.resolveSrcSymlinks()) {
            input = filesystem->resolvePath(input);
        }

        if (filesystem->type(input) != Filesystem::Type::Directory && !filesystem->isReadable(input)) {
            if (options.ignoreMissingInputs()) {
                continue;
            } else {
                fprintf(stderr, "error: missing input '%s'\n", input.c_str());
                return 1;
            }
        }

        if (options.verbose()) {
            printf("verbose: copying %s -> %s\n", input.c_str(), output.c_str());
        }

        std::string outputPath = output + "/" + FSUtil::GetBaseName(input);
        if (!CopyPath(filesystem, input, outputPath)) {
            return 1;
        }
    }

    return 0;
}

int Driver::
run(process::Context const *processContext, libutil::Filesystem *filesystem)
{
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, processContext->commandLineArguments());
    if (!result.first) {
        fprintf(stderr, "error: %s\n", result.second.c_str());
        return 1;
    }

    return Run(filesystem, options, processContext->currentDirectory());
}
