/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcsdk_SDK_Toolchain_h
#define __xcsdk_SDK_Toolchain_h

#include <pbxsetting/Level.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace libutil { class Filesystem; };
namespace plist { class Dictionary; }

namespace xcsdk { namespace SDK {

class Manager;

class Toolchain {
public:
    typedef std::shared_ptr <Toolchain> shared_ptr;

private:
    std::string                             _path;
    std::string                             _name;

private:
    ext::optional<std::string>              _identifier;
    ext::optional<std::vector<std::string>> _aliases;

public:
    ext::optional<std::string>              _displayName;
    ext::optional<std::string>              _shortDisplayName;
    ext::optional<std::string>              _createdDate;
    ext::optional<std::string>              _reportProblemURL;

public:
    ext::optional<std::string>              _bundleIdentifier;
    ext::optional<std::string>              _version;
    ext::optional<int>                      _compatibilityVersion;
    ext::optional<std::string>              _compatibilityVersionDisplayString;

public:
    ext::optional<bool>                     _providesSwiftVersion;
    ext::optional<pbxsetting::Level>        _overrideBuildSettings;

public:
    Toolchain();
    ~Toolchain();

public:
    inline std::string const &path() const
    { return _path; }
    inline std::string const &name() const
    { return _name; }

public:
    inline ext::optional<std::string> const &identifier() const
    { return _identifier; }
    inline ext::optional<std::vector<std::string>> const &aliases() const
    { return _aliases; }

public:
    inline ext::optional<std::string> const &displayName() const
    { return _displayName; }
    inline ext::optional<std::string> const &shortDisplayName() const
    { return _shortDisplayName; }
    inline ext::optional<std::string> const &createdDate() const
    { return _createdDate; }
    inline ext::optional<std::string> const &reportProblemURL() const
    { return _reportProblemURL; }

public:
    inline ext::optional<std::string> const &bundleIdentifier() const
    { return _bundleIdentifier; }
    inline ext::optional<std::string> const &version() const
    { return _version; }
    inline ext::optional<int> const &compatibilityVersion() const
    { return _compatibilityVersion; }
    inline ext::optional<std::string> const &compatibilityVersionDisplayString() const
    { return _compatibilityVersionDisplayString; }

public:
    inline bool providesSwiftVersion() const
    { return _providesSwiftVersion.value_or(false); }
    inline ext::optional<bool> const &providesSwiftVersionOptional() const
    { return _providesSwiftVersion; }
    inline ext::optional<pbxsetting::Level> const &overrideBuildSettings() const
    { return _overrideBuildSettings; }

public:
    std::vector<std::string> executablePaths() const;

public:
    static Toolchain::shared_ptr Open(libutil::Filesystem const *filesystem, std::string const &path);

public:
    static std::string DefaultIdentifier(void);

private:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcsdk_SDK_Toolchain_h
