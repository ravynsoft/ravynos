/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcsdk_SDK_Platform_h
#define __xcsdk_SDK_Platform_h

#include <xcsdk/SDK/Target.h>
#include <xcsdk/SDK/PlatformVersion.h>
#include <pbxsetting/Level.h>

#include <memory>
#include <string>
#include <vector>

namespace libutil { class Filesystem; };
namespace plist { class Dictionary; }

namespace xcsdk { namespace SDK {

class Platform {
public:
    typedef std::shared_ptr <Platform> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::weak_ptr<Manager>           _manager;
    PlatformVersion::shared_ptr      _platformVersion;
    std::vector<Target::shared_ptr>  _targets;

private:
    std::string                      _path;
    std::string                      _name;

private:
    ext::optional<std::string>       _identifier;
    ext::optional<std::string>       _description;
    ext::optional<std::string>       _type;
    ext::optional<std::string>       _icon;
    ext::optional<std::string>       _version;

private:
    ext::optional<std::string>       _familyIdentifier;
    ext::optional<std::string>       _familyName;

private:
    ext::optional<pbxsetting::Level> _defaultProperties;
    ext::optional<pbxsetting::Level> _overrideProperties;

private:
    ext::optional<pbxsetting::Level> _additionalInfo;
    ext::optional<std::string>       _minimumSDKVersion;

private:
    ext::optional<bool>              _isDeploymentPlatform;
    plist::Dictionary         const *_defaultDebuggerSettings;
    ext::optional<std::string>       _runtimeSystemSpecification;

public:
    Platform();
    ~Platform();

public:
    inline std::shared_ptr<Manager> manager() const
    { return _manager.lock(); }

public:
    inline PlatformVersion::shared_ptr const &platformVersion() const
    { return _platformVersion; }
    inline std::vector<Target::shared_ptr> const &targets() const
    { return _targets; }

public:
    inline std::string const &path() const
    { return _path; }
    inline std::string const &name() const
    { return _name; }

public:
    inline ext::optional<std::string> const &identifier() const
    { return _identifier; }
    inline ext::optional<std::string> const &description() const
    { return _description; }
    inline ext::optional<std::string> const &version() const
    { return _version; }
    inline ext::optional<std::string> const &icon() const
    { return _icon; }
    inline ext::optional<std::string> const &type() const
    { return _type; }

public:
    inline ext::optional<std::string> const familyIdentifier() const
    { return _familyIdentifier; }
    inline ext::optional<std::string> const familyName() const
    { return _familyName; }

public:
    inline ext::optional<pbxsetting::Level> const &defaultProperties() const
    { return _defaultProperties; }
    inline ext::optional<pbxsetting::Level> const &overrideProperties() const
    { return _overrideProperties; }

public:
    inline ext::optional<pbxsetting::Level> const &additionalInfo() const
    { return _additionalInfo; }
    inline ext::optional<std::string> const &minimumSDKVersion() const
    { return _minimumSDKVersion; }

public:
    inline bool isDeploymentPlatform() const
    { return _isDeploymentPlatform.value_or(false); }
    inline ext::optional<bool> const &isDeploymentPlatformOptional() const
    { return _isDeploymentPlatform; }
    inline plist::Dictionary const *defaultDebuggerSettings() const
    { return _defaultDebuggerSettings; }
    inline ext::optional<std::string> const &runtimeSystemSpecification() const
    { return _runtimeSystemSpecification; }

public:
    pbxsetting::Level settings() const;

public:
    std::vector<std::string> executablePaths() const;

public:
    static Platform::shared_ptr Open(libutil::Filesystem const *filesystem, std::shared_ptr<Manager> manager, std::string const &path);

private:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcsdk_SDK_Platform_h
