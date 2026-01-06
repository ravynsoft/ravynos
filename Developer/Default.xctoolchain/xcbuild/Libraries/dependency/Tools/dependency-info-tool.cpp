/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/Options.h>
#include <libutil/Escape.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/DefaultContext.h>
#include <process/Context.h>

#include <dependency/DependencyInfo.h>
#include <dependency/BinaryDependencyInfo.h>
#include <dependency/DirectoryDependencyInfo.h>
#include <dependency/MakefileDependencyInfo.h>

#include <cassert>
#include <cstdlib>

using libutil::Escape;
using libutil::DefaultFilesystem;
using libutil::Filesystem;
using libutil::FSUtil;

class Options {
private:
    ext::optional<bool>        _help;
    ext::optional<bool>        _version;

private:
    std::vector<std::pair<dependency::DependencyInfoFormat, std::string>> _inputs;
    ext::optional<std::string> _output;
    ext::optional<std::string> _name;

public:
    Options();
    ~Options();

public:
    bool help() const
    { return _help.value_or(false); }
    bool version() const
    { return _version.value_or(false); }

public:
    std::vector<std::pair<dependency::DependencyInfoFormat, std::string>> const &inputs() const
    { return _inputs; }
    ext::optional<std::string> const &output() const
    { return _output; }
    ext::optional<std::string> const &name() const
    { return _name; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

Options::
Options()
{
}

Options::
~Options()
{
}

std::pair<bool, std::string> Options::
parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
    std::string const &arg = **it;

    if (arg == "-h" || arg == "--help") {
        return libutil::Options::Current<bool>(&_help, arg);
    } else if (arg == "-v" || arg == "--version") {
        return libutil::Options::Current<bool>(&_version, arg);
    } else if (arg == "-o" || arg == "--output") {
        return libutil::Options::Next<std::string>(&_output, args, it);
    } else if (arg == "-n" || arg == "--name") {
        return libutil::Options::Next<std::string>(&_name, args, it);
    } else if (!arg.empty() && arg[0] != '-') {
        std::string::size_type offset = arg.find(':');
        if (offset != std::string::npos && offset != 0 && offset != arg.size() - 1) {
            std::string name = arg.substr(0, offset);
            std::string path = arg.substr(offset + 1);

            dependency::DependencyInfoFormat format;
            if (!dependency::DependencyInfoFormats::Parse(name, &format)) {
                return std::make_pair(false, "unknown format "+ name);
            }

            _inputs.push_back({ format, path });
            return std::make_pair(true, std::string());
        } else {
            return std::make_pair(false, "unknown input "+ arg + " (use format:/path/to/input)");
        }
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

static int
Help(std::string const &error = std::string())
{
    if (!error.empty()) {
        fprintf(stderr, "error: %s\n", error.c_str());
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "Usage: dependency-info-tool [options] [inputs]\n\n");
    fprintf(stderr, "Converts dependency info to Ninja format.\n\n");

#define INDENT "  "
    fprintf(stderr, "Information:\n");
    fprintf(stderr, INDENT "-h, --help\n");
    fprintf(stderr, INDENT "-v, --version\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "Conversion Options:\n");
    fprintf(stderr, INDENT "-o, --output\n");
    fprintf(stderr, INDENT "-n, --name\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "Inputs:\n");
    fprintf(stderr, INDENT "<format>:<path>\n");
    fprintf(stderr, INDENT "format: makefile, binary, directory\n");
    fprintf(stderr, "\n");
#undef INDENT

    return (error.empty() ? EXIT_SUCCESS : EXIT_FAILURE);
}

static int
Version()
{
    printf("dependency-info-tool version 1\n");
    return EXIT_SUCCESS;
}

static bool
LoadDependencyInfo(Filesystem const *filesystem, std::string const &path, dependency::DependencyInfoFormat format, std::vector<dependency::DependencyInfo> *dependencyInfo)
{
    if (format == dependency::DependencyInfoFormat::Binary) {
        std::vector<uint8_t> contents;
        if (!filesystem->read(&contents, path)) {
            fprintf(stderr, "error: failed to open %s\n", path.c_str());
            return false;
        }

        auto binaryInfo = dependency::BinaryDependencyInfo::Deserialize(contents);
        if (!binaryInfo) {
            fprintf(stderr, "error: invalid binary dependency info\n");
            return false;
        }

        dependencyInfo->push_back(binaryInfo->dependencyInfo());
        return true;
    } else if (format == dependency::DependencyInfoFormat::Directory) {
        if (filesystem->type(path) != Filesystem::Type::Directory) {
            fprintf(stderr, "warning: ignoring non-directory %s\n", path.c_str());
            return true;
        }

        auto directoryInfo = dependency::DirectoryDependencyInfo::Deserialize(filesystem, path);
        if (!directoryInfo) {
            fprintf(stderr, "error: invalid directory\n");
            return false;
        }

        dependencyInfo->push_back(directoryInfo->dependencyInfo());
        return true;
    } else if (format == dependency::DependencyInfoFormat::Makefile) {
        std::vector<uint8_t> contents;
        if (!filesystem->read(&contents, path)) {
            fprintf(stderr, "error: failed to open %s\n", path.c_str());
            return false;
        }

        std::string makefileContents = std::string(contents.begin(), contents.end());
        auto makefileInfo = dependency::MakefileDependencyInfo::Deserialize(makefileContents);
        if (!makefileInfo) {
            fprintf(stderr, "error: invalid makefile dependency info\n");
            return false;
        }

        dependencyInfo->insert(dependencyInfo->end(), makefileInfo->dependencyInfo().begin(), makefileInfo->dependencyInfo().end());
        return true;
    } else {
        assert(false);
        return false;
    }
}

static std::string
SerializeMakefileDependencyInfo(std::string const &currentDirectory, std::string const &output, std::vector<std::string> const &inputs)
{
    dependency::DependencyInfo dependencyInfo;
    dependencyInfo.outputs() = { output };

    /* Normalize path as Ninja requires matching paths. */
    for (std::string const &input : inputs) {
        std::string path = FSUtil::ResolveRelativePath(input, currentDirectory);
        dependencyInfo.inputs().push_back(path);
    }

    /* Serialize dependency info. */
    dependency::MakefileDependencyInfo makefileInfo;
    makefileInfo.dependencyInfo() = { dependencyInfo };
    return makefileInfo.serialize();
}

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();
    process::DefaultContext processContext = process::DefaultContext();

    /*
     * Parse out the options, or print help & exit.
     */
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, processContext.commandLineArguments());
    if (!result.first) {
        return Help(result.second);
    }

    /*
     * Handle the basic options.
     */
    if (options.help()) {
        return Help();
    } else if (options.version()) {
        return Version();
    }

    /*
     * Diagnose missing options.
     */
    if (options.inputs().empty() || !options.output() || !options.name()) {
        return Help("missing option(s)");
    }

    std::vector<std::string> inputs;
    for (std::pair<dependency::DependencyInfoFormat, std::string> const &input : options.inputs()) {
        /*
         * Load the dependency info.
         */
        std::vector<dependency::DependencyInfo> info;
        if (!LoadDependencyInfo(&filesystem, input.second, input.first, &info)) {
            return EXIT_FAILURE;
        }

        for (dependency::DependencyInfo const &dependencyInfo : info) {
            inputs.insert(inputs.end(), dependencyInfo.inputs().begin(), dependencyInfo.inputs().end());
        }
    }

    /*
     * Serialize the output.
     */
    std::string contents = SerializeMakefileDependencyInfo(processContext.currentDirectory(), *options.name(), inputs);

    /*
     * Write out the output.
     */
    std::vector<uint8_t> makefileContents = std::vector<uint8_t>(contents.begin(), contents.end());
    if (!filesystem.write(makefileContents, *options.output())) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
