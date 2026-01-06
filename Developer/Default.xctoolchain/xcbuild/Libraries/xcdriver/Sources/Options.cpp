/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/Options.h>

using xcdriver::Options;

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

    if (arg == "-usage") {
        return libutil::Options::Current<bool>(&_usage, arg);
    } else if (arg == "-help") {
        return libutil::Options::Current<bool>(&_help, arg);
    } else if (arg == "-license") {
        return libutil::Options::Current<bool>(&_license, arg);
    } else if (arg == "-checkFirstLaunchStatus") {
        return libutil::Options::Current<bool>(&_checkFirstLaunchStatus, arg);
    } else if (arg == "-verbose") {
        return libutil::Options::Current<bool>(&_verbose, arg);
    } else if (arg == "-quiet") {
        return libutil::Options::Current<bool>(&_quiet, arg);
    } else if (arg == "-json") {
        return libutil::Options::Current<bool>(&_json, arg);
    } else if (arg == "-project") {
        return libutil::Options::Next<std::string>(&_project, args, it);
    } else if (arg == "-target") {
        return libutil::Options::AppendNext<std::string>(&_target, args, it);
    } else if (arg == "-alltargets") {
        return libutil::Options::Current<bool>(&_allTargets, arg);
    } else if (arg == "-workspace") {
        return libutil::Options::Next<std::string>(&_workspace, args, it);
    } else if (arg == "-scheme") {
        return libutil::Options::Next<std::string>(&_scheme, args, it);
    } else if (arg == "-configuration") {
        return libutil::Options::Next<std::string>(&_configuration, args, it);
    } else if (arg == "-xcconfig") {
        return libutil::Options::Next<std::string>(&_xcconfig, args, it);
    } else if (arg == "-arch") {
        return libutil::Options::Next<std::string>(&_arch, args, it);
    } else if (arg == "-sdk") {
        return libutil::Options::Next<std::string>(&_sdk, args, it);
    } else if (arg == "-toolchain") {
        return libutil::Options::Next<std::string>(&_toolchain, args, it);
    } else if (arg == "-destination") {
        return libutil::Options::Next<std::string>(&_destination, args, it);
    } else if (arg == "-destination-timeout") {
        return libutil::Options::Next<std::string>(&_destinationTimeout, args, it);
    } else if (arg == "-parallelizeTargets") {
        return libutil::Options::Current<bool>(&_parallelizeTargets, arg);
    } else if (arg == "-jobs") {
        return libutil::Options::Next<int>(&_jobs, args, it);
    } else if (arg == "-dryrun" || arg == "-n") {
        return libutil::Options::Current<bool>(&_dryRun, arg);
    } else if (arg == "-hideShellScriptEnvironment") {
        return libutil::Options::Current<bool>(&_hideShellScriptEnvironment, arg);
    } else if (arg == "-showsdks") {
        return libutil::Options::Current<bool>(&_showSDKs, arg);
    } else if (arg == "-showBuildSettings") {
        return libutil::Options::Current<bool>(&_showBuildSettings, arg);
    } else if (arg == "-list") {
        return libutil::Options::Current<bool>(&_list, arg);
    } else if (arg == "-find" || arg == "-find-executable") {
        return libutil::Options::Next<std::string>(&_findExecutable, args, it);
    } else if (arg == "-find-library") {
        return libutil::Options::Next<std::string>(&_findLibrary, args, it);
    } else if (arg == "-version") {
        return libutil::Options::Current<bool>(&_version, arg);
    } else if (arg == "-enableAddressSanitizer") {
        return libutil::Options::Next<bool>(&_enableAddressSanitizer, args, it);
    } else if (arg == "-enableThreadSanitizer") {
        return libutil::Options::Next<bool>(&_enableThreadSanitizer, args, it);
    } else if (arg == "-resultBundlePath") {
        return libutil::Options::Next<std::string>(&_resultBundlePath, args, it);
    } else if (arg == "-derivedDataPath") {
        return libutil::Options::Next<std::string>(&_derivedDataPath, args, it);
    } else if (arg == "-archivePath") {
        return libutil::Options::Next<std::string>(&_archivePath, args, it);
    } else if (arg == "-exportArchive") {
        return libutil::Options::Current<bool>(&_exportArchive, arg);
    } else if (arg == "-exportOptionsPlist") {
        return libutil::Options::Next<std::string>(&_exportOptionsPlist, args, it);
    } else if (arg == "-enableCodeCoverage") {
        return libutil::Options::Next<bool>(&_enableCodeCoverage, args, it);
    } else if (arg == "-xctestrun") {
        return libutil::Options::Next<std::string>(&_xctestrun, args, it);
    } else if (arg.find("-only-testing:") == 0) {
        std::string value = arg.substr(arg.find(':') + 1);
        _onlyTesting.push_back(value);
        return std::make_pair(true, std::string());
    } else if (arg.find("-skip-testing:") == 0) {
        std::string value = arg.substr(arg.find(':') + 1);
        _skipTesting.push_back(value);
        return std::make_pair(true, std::string());
    } else if (arg == "-exportPath") {
        return libutil::Options::Next<std::string>(&_exportPath, args, it);
    } else if (arg == "-skipUnavailableActions") {
        return libutil::Options::Current<bool>(&_skipUnavailableActions, arg);
    } else if (arg == "-exportLocalizations") {
        return libutil::Options::Current<bool>(&_exportLocalizations, arg);
    } else if (arg == "-importLocalizations") {
        return libutil::Options::Current<bool>(&_importLocalizations, arg);
    } else if (arg == "-localizationPath") {
        return libutil::Options::Next<std::string>(&_localizationPath, args, it);
    } else if (arg == "-exportLanguage") {
        return libutil::Options::Next<std::string>(&_exportLanguage, args, it);
    } else if (arg == "-forceImport") {
        return libutil::Options::Current<bool>(&_forceImport, arg);
    } else if (arg == "-executor") {
        return libutil::Options::Next<std::string>(&_executor, args, it);
    } else if (arg == "-formatter") {
        return libutil::Options::Next<std::string>(&_formatter, args, it);
    } else if (arg == "-generate") {
        return libutil::Options::Current<bool>(&_generate, arg);
    } else if (!arg.empty() && arg[0] != '-') {
        if (arg.find('=') != std::string::npos) {
            if (ext::optional<pbxsetting::Setting> setting = pbxsetting::Setting::Parse(arg)) {
                _settings.push_back(*setting);
                return std::make_pair(true, std::string());
            } else {
                return std::make_pair(false, "unknown argument " + arg);
            }
        } else {
            _actions.push_back(arg);
            return std::make_pair(true, std::string());
        }
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

