/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/HelpAction.h>
#include <xcdriver/Usage.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>

#include <cstdio>

using xcdriver::HelpAction;
using xcdriver::Usage;
using libutil::FSUtil;

HelpAction::
HelpAction()
{
}

HelpAction::
~HelpAction()
{
}

int HelpAction::
Run(process::Context const *processContext)
{
    std::string path = processContext->executablePath();
    std::string text = Usage::Text(FSUtil::GetBaseName(path));
    fprintf(stdout, "%s", text.c_str());

    fprintf(stdout, "Options:\n");
    fprintf(
        stdout,
        "    -usage                                      "
        "print brief usage instructions\n");
    fprintf(
        stdout,
        "    -help                                       "
        "print complete usage instructions\n");
    fprintf(
        stdout,
        "    -verbose                                    "
        "print diagnostic messages and other detailed information when "
        "performing an action\n");
    fprintf(
        stdout,
        "    -license                                    "
        "print license\n");
    fprintf(
        stdout,
        "    -checkFirstLaunchStatus                     "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -formatter NAME                             "
        "use the output formatter NAME. currently only 'default' is "
        "supported\n");
    fprintf(
        stdout,
        "    -executor NAME                              "
        "use the execution engine NAME. currently 'ninja' and 'simple' are "
        "supported\n");
    fprintf(
        stdout,
        "    -generate                                   "
        "specify that an execution engine based on generating another build "
        "language should regenerate\n");
    fprintf(
        stdout,
        "    -project NAME                               "
        "build the project NAME\n");
    fprintf(
        stdout,
        "    -target NAME                                "
        "build the target NAME\n");
    fprintf(
        stdout,
        "    -alltargets                                 "
        "build all targets\n");
    fprintf(
        stdout,
        "    -workspace NAME                             "
        "build the workspace NAME\n");
    fprintf(
        stdout,
        "    -scheme NAME                                "
        "build the scheme NAME\n");
    fprintf(
        stdout,
        "    -configuration NAME                         "
        "use the build configuration NAME for building each target\n");
    fprintf(
        stdout,
        "    -xcconfig PATH                              "
        "apply the build settings defined in the file at PATH, overriding "
        "any settings already specified elsewhere, such as the project file\n");
    fprintf(
        stdout,
        "    -arch ARCH                                  "
        "instead of building the architectures defined in the project, build "
        "each target for the architecture ARCH\n");
    fprintf(
        stdout,
        "    -sdk NAME|PATH                              "
        "build using the SDK NAME, or using the SDK at the given PATH\n");
    fprintf(
        stdout,
        "    -toolchain NAME                             "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -destination DESTINATIONSPECIFIER           "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -destination-timeout TIMEOUT                "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -parallelizeTargets                         "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -jobs NUMBER                                "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -dry-run                                    "
        "print build output without producing any build products\n");
    fprintf(
        stdout,
        "    -hideShellScriptEnvironment                 "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -showsdks                                   "
        "print a list of the installed SDKs\n");
    fprintf(
        stdout,
        "    -showBuildSettings                          "
        "print the build settings and their values for the given target\n");
    fprintf(
        stdout,
        "    -list                                       "
        "print the targets and configurations for a given project, or the "
        "schemes for a given workspace\n");
    fprintf(
        stdout,
        "    -find-executable NAME                       "
        "print the path to executable NAME in the provided SDK and toolchain\n");
    fprintf(
        stdout,
        "    -find-library NAME                          "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -version                                    "
        "print version, or the version of any SDKs specified using the "
        "-sdk option\n");
    fprintf(
        stdout,
        "    -enableAddressSanitizer YES|NO              "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -resultBundlePath PATH                      "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -derivedDataPath PATH                       "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -archivePath PATH                           "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -exportArchive                              "
        "export an archive after building\n");
    fprintf(
        stdout,
        "    -exportOptionsPlist PATH                    "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -enableCodeCoverage YES|NO                  "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -exportPath PATH                            "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -skipUnavailableActions                     "
        "specifies that scheme actions that cannot be performed should be "
        "skipped, instead of causing a failure\n");
    fprintf(
        stdout,
        "    -exportLocalizations                        "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -importLocalizations                        "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -localizationPath                           "
        "not yet implemented\n");
    fprintf(
        stdout,
        "    -exportLanguage                             "
        "not yet implemented\n");

    fprintf(stdout, "\n");

    return 0;
}

