/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcsdk_SDK_Target_h
#define __xcsdk_SDK_Target_h

#include <xcsdk/SDK/Product.h>
#include <xcsdk/SDK/Toolchain.h>
#include <pbxsetting/Level.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace libutil { class Filesystem; };
namespace plist { class Dictionary; }

namespace xcsdk { namespace SDK {

class Manager;
class Platform;

class Target {
public:
    typedef std::shared_ptr <Target> shared_ptr;

private:
    std::weak_ptr<Manager>                  _manager;
    std::weak_ptr<Platform>                 _platform;

private:
    Product::shared_ptr                     _product;
    std::vector<Toolchain::shared_ptr>      _toolchains;

private:
    std::string                             _path;
    std::string                             _bundleName;

private:
    ext::optional<std::string>              _canonicalName;
    ext::optional<std::string>              _displayName;
    ext::optional<std::string>              _minimalDisplayName;
    ext::optional<std::string>              _version;

private:
    ext::optional<bool>                     _isBaseSDK;
    ext::optional<std::string>              _defaultDeploymentTarget;
    ext::optional<std::string>              _maximumDeploymentTarget;
    ext::optional<pbxsetting::Level>        _defaultProperties;
    ext::optional<pbxsetting::Level>        _customProperties;
    ext::optional<std::vector<std::string>> _propertyConditionFallbackNames;

private:
    ext::optional<std::string>              _docSetFeedName;
    ext::optional<std::string>              _docSetFeedURL;

private:
    ext::optional<std::string>              _minimumSupportedToolsVersion;
    ext::optional<std::vector<std::string>> _supportedBuildToolComponents;

public:
    Target();
    ~Target();

public:
    inline std::shared_ptr<Manager> manager() const
    { return _manager.lock(); }
    inline std::shared_ptr<Platform> platform() const
    { return _platform.lock(); }

public:
    inline Product::shared_ptr const &product() const
    { return _product; }
    inline std::vector<Toolchain::shared_ptr> const &toolchains() const
    { return _toolchains; }

public:
    inline std::string const &path() const
    { return _path; }
    inline std::string const &bundleName() const
    { return _bundleName; }

public:
    inline ext::optional<std::string> const &canonicalName() const
    { return _canonicalName; }
    inline ext::optional<std::string> const &displayName() const
    { return _displayName; }
    inline ext::optional<std::string> const &minimalDisplayName() const
    { return _minimalDisplayName; }
    inline ext::optional<std::string> const &version() const
    { return _version; }

public:
    inline bool isBaseSDK() const
    { return _isBaseSDK.value_or(false); }
    inline ext::optional<bool> isBaseSDKOptional() const
    { return _isBaseSDK; }
    inline ext::optional<std::string> const &defaultDeploymentTarget() const
    { return _defaultDeploymentTarget; }
    inline ext::optional<std::string> const &maximumDeploymentTarget() const
    { return _maximumDeploymentTarget; }
    inline ext::optional<pbxsetting::Level> const &defaultProperties() const
    { return _defaultProperties; }
    inline ext::optional<pbxsetting::Level> const &customProperties() const
    { return _customProperties; }
    inline ext::optional<std::vector<std::string>> const &propertyConditionFallbackNames()
    { return _propertyConditionFallbackNames; }

public:
    inline ext::optional<std::string> const &docSetFeedName()
    { return _docSetFeedName; }
    inline ext::optional<std::string> const &docSetFeedURL()
    { return _docSetFeedURL; }

public:
    inline ext::optional<std::string> const &minimumSupportedToolsVersion()
    { return _minimumSupportedToolsVersion; }
    inline ext::optional<std::vector<std::string>> &supportedBuildToolComponents()
    { return _supportedBuildToolComponents; }

public:
    pbxsetting::Level settings(void) const;

public:
    std::vector<std::string> executablePaths() const;

public:
    static Target::shared_ptr Open(libutil::Filesystem const *filesystem, std::shared_ptr<Manager> manager, std::shared_ptr<Platform>, std::string const &path);

private:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcsdk_SDK_Target_h
