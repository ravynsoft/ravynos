/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __acdriver_Options_h
#define __acdriver_Options_h

#include <acdriver/NonStandard.h>
#include <acdriver/Result.h>
#include <libutil/Options.h>

#include <string>
#include <vector>

namespace acdriver {

/*
 * Options for actool.
 */
class Options {
private:
    ext::optional<bool>        _version;
    ext::optional<bool>        _printContents;
    std::vector<std::string>   _inputs;
    ext::optional<std::string> _productType;

private:
    ext::optional<std::string> _outputFormat;
    ext::optional<bool>        _warnings;
    ext::optional<bool>        _errors;
    ext::optional<bool>        _notices;

private:
    ext::optional<std::string> _compile;

    /*
     * extension: allows changing the name of the output file, including the file extension.
     * "Assets.car" will be used if unspecified.
     */
    ext::optional<std::string> _compileOutputFilename;

    ext::optional<std::string> _exportDependencyInfo;

private:
    ext::optional<std::string> _optimization;
    ext::optional<bool>        _compressPNGs;

private:
    ext::optional<std::string> _platform;
    ext::optional<std::string> _minimumDeploymentTarget;
    std::vector<std::string>   _targetDevice;

private:
    ext::optional<std::string> _outputPartialInfoPlist;
    ext::optional<std::string> _appIcon;
    ext::optional<std::string> _launchImage;
    ext::optional<std::string> _flattenedAppIconPath;

private:
    ext::optional<std::string> _stickerPackIdentifierPrefix;
    ext::optional<std::string> _stickerPackStringsFile;
    ext::optional<std::string> _leaderboardIdentifierPrefix;
    ext::optional<std::string> _leaderboardSetIdentifierPrefix;

private:
    ext::optional<bool>        _enableOnDemandResources;
    ext::optional<bool>        _enableIncrementalDistill;
    ext::optional<std::string> _targetName;
    ext::optional<std::string> _filterForDeviceModel;
    ext::optional<std::string> _filterForDeviceOsVersion;
    ext::optional<std::string> _assetPackOutputSpecifications;

private:
    // Options not compatible with Apple's actool
    NonStandard::ActoolOptions  _nonStandardOptions;

public:
    Options();
    ~Options();

public:
    bool version() const
    { return _version.value_or(false); }
    bool printContents() const
    { return _printContents.value_or(false); }
    ext::optional<std::string> const &compile() const
    { return _compile; }
    ext::optional<std::string> const &compileOutputFilename() const
    { return _compileOutputFilename; }
public:
    ext::optional<std::string> const &outputFormat() const
    { return _outputFormat; }
    bool warnings() const
    { return _warnings.value_or(false); }
    bool errors() const
    { return _errors.value_or(false); }
    bool notices() const
    { return _notices.value_or(false); }

public:
    ext::optional<std::string> const &exportDependencyInfo() const
    { return _exportDependencyInfo; }
    std::vector<std::string> const &inputs() const
    { return _inputs; }
    ext::optional<std::string> const &productType() const
    { return _productType; }

public:
    ext::optional<std::string> const &optimization() const
    { return _optimization; }
    bool compressPNGs() const
    { return _compressPNGs.value_or(false); }

public:
    ext::optional<std::string> const &platform() const
    { return _platform; }
    ext::optional<std::string> const &minimumDeploymentTarget() const
    { return _minimumDeploymentTarget; }
    std::vector<std::string> const &targetDevice() const
    { return _targetDevice; }

public:
    ext::optional<std::string> const &outputPartialInfoPlist() const
    { return _outputPartialInfoPlist; }
    ext::optional<std::string> const &appIcon() const
    { return _appIcon; }
    ext::optional<std::string> const &launchImage() const
    { return _launchImage; }
    ext::optional<std::string> const &flattenedAppIconPath() const
    { return _flattenedAppIconPath; }

public:
    ext::optional<std::string> const &stickerPackIdentifierPrefix() const
    { return _stickerPackIdentifierPrefix; }
    ext::optional<std::string> const &stickerPackStringsFile() const
    { return _stickerPackStringsFile; }
    ext::optional<std::string> const &leaderboardIdentifierPrefix() const
    { return _leaderboardIdentifierPrefix; }
    ext::optional<std::string> const &leaderboardSetIdentifierPrefix() const
    { return _leaderboardSetIdentifierPrefix; }

public:
    bool enableOnDemandResources() const
    { return _enableOnDemandResources.value_or(false); }
    bool enableIncrementalDistill() const
    { return _enableIncrementalDistill.value_or(false); }
    ext::optional<std::string> const &targetName() const
    { return _targetName; }
    ext::optional<std::string> const &filterForDeviceModel() const
    { return _filterForDeviceModel; }
    ext::optional<std::string> const &filterForDeviceOsVersion() const
    { return _filterForDeviceOsVersion; }
    ext::optional<std::string> const &assetPackOutputSpecifications() const
    { return _assetPackOutputSpecifications; }

public:
    NonStandard::ActoolOptions const &nonStandardOptions() const
    { return _nonStandardOptions; }

public:

    bool
    isValid(Result *result) const;

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

}

#endif // !__acdriver_Options_h
