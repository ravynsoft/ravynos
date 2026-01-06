/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Options.h>

using acdriver::Options;

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

    if (arg == "--version") {
        return libutil::Options::Current<bool>(&_version, arg);
    } else if (arg == "--print-contents") {
        return libutil::Options::Current<bool>(&_printContents, arg);
    } else if (arg == "--compile") {
        return libutil::Options::Next<std::string>(&_compile, args, it);
    } else if (arg == "--compile-output-filename") {
        return libutil::Options::Next<std::string>(&_compileOutputFilename, args, it);
    } else if (arg == "--output-format") {
        return libutil::Options::Next<std::string>(&_outputFormat, args, it);
    } else if (arg == "--warnings") {
        return libutil::Options::Current<bool>(&_warnings, arg);
    } else if (arg == "--errors") {
        return libutil::Options::Current<bool>(&_errors, arg);
    } else if (arg == "--notices") {
        return libutil::Options::Current<bool>(&_notices, arg);
    } else if (arg == "--product-type") {
        return libutil::Options::Next<std::string>(&_productType, args, it);
    } else if (arg == "--export-dependency-info") {
        return libutil::Options::Next<std::string>(&_exportDependencyInfo, args, it);
    } else if (arg == "--optimization") {
        return libutil::Options::Next<std::string>(&_optimization, args, it);
    } else if (arg == "--compress-pngs") {
        return libutil::Options::Current<bool>(&_compressPNGs, arg);
    } else if (arg == "--platform") {
        return libutil::Options::Next<std::string>(&_platform, args, it);
    } else if (arg == "--minimum-deployment-target") {
        return libutil::Options::Next<std::string>(&_minimumDeploymentTarget, args, it);
    } else if (arg == "--target-device") {
        return libutil::Options::AppendNext<std::string>(&_targetDevice, args, it);
    } else if (arg == "--output-partial-info-plist") {
        return libutil::Options::Next<std::string>(&_outputPartialInfoPlist, args, it);
    } else if (arg == "--app-icon") {
        return libutil::Options::Next<std::string>(&_appIcon, args, it);
    } else if (arg == "--launch-image") {
        return libutil::Options::Next<std::string>(&_launchImage, args, it);
    } else if (arg == "--flattened-app-icon-path") {
        return libutil::Options::Next<std::string>(&_flattenedAppIconPath, args, it);
    } else if (arg == "--sticker-pack-identifier-prefix") {
        return libutil::Options::Next<std::string>(&_stickerPackIdentifierPrefix, args, it);
    } else if (arg == "--sticker-pack-strings-file") {
        return libutil::Options::Next<std::string>(&_stickerPackStringsFile, args, it);
    } else if (arg == "--leaderboard-identifier-prefix") {
        return libutil::Options::Next<std::string>(&_leaderboardIdentifierPrefix, args, it);
    } else if (arg == "--leaderboard-set-identifier-prefix") {
        return libutil::Options::Next<std::string>(&_leaderboardSetIdentifierPrefix, args, it);
    } else if (arg == "--enable-on-demand-resources") {
        return libutil::Options::Next<bool>(&_enableOnDemandResources, args, it, true);
    } else if (arg == "--enable-incremental-distill") {
        return libutil::Options::Current<bool>(&_enableIncrementalDistill, arg);
    } else if (arg == "--target-name") {
        return libutil::Options::Next<std::string>(&_targetName, args, it);
    } else if (arg == "--filter-for-device-model") {
        return libutil::Options::Next<std::string>(&_filterForDeviceModel, args, it);
    } else if (arg == "--filter-for-device-os-version") {
        return libutil::Options::Next<std::string>(&_filterForDeviceOsVersion, args, it);
    } else if (arg == "--asset-pack-output-specifications") {
        return libutil::Options::Next<std::string>(&_assetPackOutputSpecifications, args, it);
    } else if (!arg.empty() && arg[0] != '-') {
        return libutil::Options::AppendCurrent<std::string>(&_inputs, arg);
    } else {
        ext::optional<std::pair<bool, std::string>> nonStandardResult = _nonStandardOptions.parseArgument(args, it);
        if (nonStandardResult) {
            return *nonStandardResult;
        }
        return std::make_pair(false, "unknown argument " + arg);
    }
}


bool Options::
isValid(Result *result) const
{
    if (!nonStandardOptions().isValid(result)) {
        return false;
    }
    return true;
}