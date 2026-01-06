/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/Environment.h>
#include <libutil/Base.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>
#include <process/User.h>

using xcsdk::Environment;
using libutil::Filesystem;
using libutil::FSUtil;

static std::string
UserDeveloperRootLink(std::string const &userHomeDirectory)
{
    return userHomeDirectory + "/.xcsdk/xcode_select_link";
}

static std::string
PrimaryDeveloperRootLink()
{
    return "/var/db/xcode_select_link";
}

static std::string
SecondaryDeveloperRootLink()
{
    return "/usr/share/xcode-select/xcode_dir_path";
}

static std::string
ResolveDeveloperRoot(Filesystem const *filesystem, std::string const &path)
{
    /*
     * Support finding the developer directory inside an application directory.
     */
    std::string application = path + "/Contents/Developer";
    if (filesystem->type(application) == Filesystem::Type::Directory) {
        return application;
    }

    return path;
}

ext::optional<std::string> Environment::
DeveloperRoot(process::User const *user, process::Context const *processContext, Filesystem const *filesystem)
{
    if (auto path = processContext->environmentVariable("DEVELOPER_DIR")) {
        return ResolveDeveloperRoot(filesystem, *path);
    }

    if (ext::optional<std::string> userHomeDirectory = user->userHomeDirectory()) {
        if (auto path = filesystem->readSymbolicLink(UserDeveloperRootLink(*userHomeDirectory))) {
            return path;
        }
    }

    if (auto path = filesystem->readSymbolicLink(PrimaryDeveloperRootLink())) {
        return path;
    }

    if (auto path = filesystem->readSymbolicLink(SecondaryDeveloperRootLink())) {
        return path;
    }

    /*
     * Fall back to a set of known directories.
     */
    std::vector<std::string> defaults = {
        "/Applications/Xcode.app/Contents/Developer",
        "/Developer",
    };
    for (std::string const &path : defaults) {
        if (filesystem->type(path) == Filesystem::Type::Directory) {
            return path;
        }
    }

    return ext::nullopt;
}

bool Environment::
WriteDeveloperRoot(libutil::Filesystem *filesystem, ext::optional<std::string> const &path)
{
    /*
     * Remove any existing link.
     */
    if (filesystem->exists(PrimaryDeveloperRootLink())) {
        if (!filesystem->removeFile(PrimaryDeveloperRootLink())) {
            return false;
        }
    }

    if (path) {
        /*
         * Write the new link.
         */
        std::string normalized = FSUtil::NormalizePath(*path);
        std::string resolved = ResolveDeveloperRoot(filesystem, normalized);
        if (!filesystem->writeSymbolicLink(normalized, PrimaryDeveloperRootLink(), true)) {
            return false;
        }
    }

    return true;
}

