/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/Usage.h>

#include <sstream>

using xcdriver::Usage;

std::string Usage::
Text(std::string const &name) {
    std::ostringstream result;

    result << "Usage: " << name << " [-project <projectname>] "
        "[[-target <targetname>]...|-alltargets] "
        "[-configuration <configurationname>] "
        "[-arch <architecture>]... "
        "[-sdk [<sdkname>|<sdkpath>]] "
        "[-showBuildSettings] [<buildsetting>=<value>]... "
        "[-formatter [default]] "
        "[-executor [simple|ninja]] "
        "[-generate] "
        "[<buildaction>]..." << std::endl;

    result << "       " << name << " "
        "[-project <projectname>] -scheme <schemeName> "
        "[-destination <destinationspecifier>]... "
        "[-configuration <configurationname>] "
        "[-arch <architecture>]... "
        "[-sdk [<sdkname>|<sdkpath>]] "
        "[-showBuildSettings] "
        "[<buildsetting>=<value>]... "
        "[-formatter [default]] "
        "[-executor [simple|ninja]] "
        "[-generate] "
        "[<buildaction>]..." << std::endl;

    result << "       " << name << " "
        "-workspace <workspacename> -scheme <schemeName> "
        "[-destination <destinationspecifier>]... "
        "[-configuration <configurationname>] "
        "[-arch <architecture>]... "
        "[-sdk [<sdkname>|<sdkpath>]] "
        "[-showBuildSettings] "
        "[<buildsetting>=<value>]... "
        "[-formatter [default]] "
        "[-executor [simple|ninja]] "
        "[-generate] "
        "[<buildaction>]..." << std::endl;

    result << "       " << name << " -version "
        "[-sdk [<sdkfullpath>|<sdkname>] "
        "[<infoitem>] ]" << std::endl;

    result << "       " << name << " -list "
        "[[-project <projectname>]|[-workspace <workspacename>]]" << std::endl;

    result << "       " << name << " -showsdks" << std::endl;

    result << "       " << name << " -exportArchive "
        "-archivePath <xcarchivepath> "
        "-exportPath <destinationpath> "
        "-exportOptionsPlist <plistpath>" << std::endl;

    result << "       " << name << " -exportLocalizations "
        "-localizationPath <path> "
        "-project <projectname> "
        "[-exportLanguage <targetlanguage>...]" << std::endl;

    result << "       " << name << " -importLocalizations "
        "-localizationPath <path> "
        "-project <projectname>" << std::endl;

    result << "       " << name << " -help" << std::endl;

    result << std::endl;

    return result.str();
}
