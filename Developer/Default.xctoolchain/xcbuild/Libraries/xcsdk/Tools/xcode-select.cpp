/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/Environment.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/Filesystem.h>
#include <process/DefaultContext.h>
#include <process/Context.h>
#include <process/DefaultUser.h>
#include <process/User.h>
#include <libutil/Options.h>
#include <ext/optional>

using libutil::DefaultFilesystem;
using libutil::Filesystem;

class Options {
private:
    ext::optional<bool>        _help;
    ext::optional<bool>        _version;

private:
    ext::optional<bool>        _printPath;
    ext::optional<bool>        _resetPath;
    ext::optional<std::string> _switchPath;

private:
    ext::optional<bool>        _install;

public:
    Options();
    ~Options();

public:
    bool help() const
    { return _help.value_or(false); }
    bool version() const
    { return _version.value_or(false); }

public:
    bool printPath() const
    { return _printPath.value_or(false); }
    bool resetPath() const
    { return _resetPath.value_or(false); }
    ext::optional<std::string> const &switchPath() const
    { return _switchPath; }

public:
    bool install() const
    { return _install.value_or(false); }

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
    } else if (arg == "-v" || arg == "--version" || arg == "-version") {
        return libutil::Options::Current<bool>(&_version, arg);
    } else if (arg == "-p" || arg == "--print-path" || arg == "-print-path") {
        return libutil::Options::Current<bool>(&_printPath, arg);
    } else if (arg == "-r" || arg == "--reset") {
        return libutil::Options::Current<bool>(&_resetPath, arg);
    } else if (arg == "-s" || arg == "--switch" || arg == "-switch") {
        return libutil::Options::Next<std::string>(&_switchPath, args, it);
    } else if (arg == "--install") {
        return libutil::Options::Current<bool>(&_install, arg);
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

static int
Help(ext::optional<std::string> const &error = ext::nullopt)
{
    if (error) {
        fprintf(stderr, "error: %s\n", error->c_str());
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "Usage: xcode-select [action]\n\n");
    fprintf(stderr, "Manipulate default developer directory.\n\n");

#define INDENT "  "
    fprintf(stderr, "Actions:\n");
    fprintf(stderr, INDENT "-p, --print-path\n");
    fprintf(stderr, INDENT "-r, --reset\n");
    fprintf(stderr, INDENT "-s <path>, --switch <path>\n");
    fprintf(stderr, INDENT "--install\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "More information:\n");
    fprintf(stderr, INDENT "-h, --help (this message)\n");
    fprintf(stderr, INDENT "-v, --version\n");
    fprintf(stderr, "\n");
#undef INDENT

    return (!error ? 0 : -1);
}

static int
Version()
{
    printf("xcode-select version 1 (xcbuild)\n");
    return 0;
}

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();
    process::DefaultContext processContext = process::DefaultContext();
    process::DefaultUser user = process::DefaultUser();

    /*
     * Parse out the options, or print help & exit.
     */
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, processContext.commandLineArguments());
    if (!result.first) {
        return Help(result.second);
    }

    /*
     * Handle the various actions.
     */
    if (options.help()) {
        return Help();
    } else if (options.version()) {
        return Version();
    } else if (options.printPath()) {
        ext::optional<std::string> developer = xcsdk::Environment::DeveloperRoot(&user, &processContext, &filesystem);
        if (!developer) {
            fprintf(stderr, "error: no developer directory found\n");
            return 1;
        }

        fprintf(stdout, "%s\n", developer->c_str());
        return 0;
    } else if (options.resetPath()) {
        if (!xcsdk::Environment::WriteDeveloperRoot(&filesystem, ext::nullopt)) {
            fprintf(stderr, "error: failed to reset developer root. are you root?\n");
            return 1;
        }

        return 0;
    } else if (options.switchPath()) {
        if (!xcsdk::Environment::WriteDeveloperRoot(&filesystem, *options.switchPath())) {
            fprintf(stderr, "error: failed to set developer root. are you root?\n");
            return 1;
        }

        return 0;
    } else if (options.install()) {
        fprintf(stderr, "error: install not implemented\n");
        return 1;
    } else {
        return Help(std::string("no actions provided"));
    }
}
