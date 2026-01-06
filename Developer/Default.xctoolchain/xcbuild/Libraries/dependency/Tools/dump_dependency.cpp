/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <dependency/DependencyInfo.h>
#include <dependency/BinaryDependencyInfo.h>
#include <dependency/DirectoryDependencyInfo.h>
#include <dependency/MakefileDependencyInfo.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/Filesystem.h>
#include <process/DefaultContext.h>
#include <process/Context.h>

#include <cassert>

using libutil::DefaultFilesystem;
using libutil::Filesystem;

static void
DumpDependencyInfo(dependency::DependencyInfo const &dependencyInfo)
{
    fprintf(stdout, "input:\n");
    for (std::string const &input : dependencyInfo.inputs()) {
        fprintf(stdout, "  %s\n", input.c_str());
    }

    fprintf(stdout, "output:\n");
    for (std::string const &output : dependencyInfo.outputs()) {
        fprintf(stdout, "  %s\n", output.c_str());
    }
}

static bool
DumpDependencyInfo(Filesystem const *filesystem, std::string const &path)
{
    if (auto directoryInfo = dependency::DirectoryDependencyInfo::Deserialize(filesystem, path)) {
        fprintf(stdout, "directory dependency info\n");
        fprintf(stdout, "directory: %s\n", directoryInfo->directory().c_str());
        DumpDependencyInfo(directoryInfo->dependencyInfo());
    } else {
        std::vector<uint8_t> contents;
        if (!filesystem->read(&contents, path)) {
            fprintf(stderr, "error: failed to open %s\n", path.c_str());
            return false;
        }

        if (auto binaryInfo = dependency::BinaryDependencyInfo::Deserialize(contents)) {
            fprintf(stdout, "binary dependency info\n");
            fprintf(stdout, "version: %s\n", binaryInfo->version().c_str());

            DumpDependencyInfo(binaryInfo->dependencyInfo());

            fprintf(stdout, "missing:\n");
            for (std::string const &missing : binaryInfo->missing()) {
                fprintf(stdout, "  %s\n", missing.c_str());
            }
        } else if (auto makefileInfo = dependency::MakefileDependencyInfo::Deserialize(std::string(contents.begin(), contents.end()))) {
            fprintf(stdout, "makefile dependency info\n");
            for (dependency::DependencyInfo const &dependencyInfo : makefileInfo->dependencyInfo()) {
                DumpDependencyInfo(dependencyInfo);
            }
        } else {
            fprintf(stderr, "error: unknown dependency info type\n");
            return false;
        }
    }

    return true;
}

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();
    process::DefaultContext processContext = process::DefaultContext();

    for (std::string const &input : processContext.commandLineArguments()) {
        if (!DumpDependencyInfo(&filesystem, input)) {
            return 1;
        }
    }

    return 0;
}

