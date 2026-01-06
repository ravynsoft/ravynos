/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcsdk_SDK_PlatformVersion_h
#define __xcsdk_SDK_PlatformVersion_h

#include <memory>
#include <string>
#include <ext/optional>

namespace libutil { class Filesystem; };
namespace plist { class Dictionary; }

namespace xcsdk { namespace SDK {

class PlatformVersion {
public:
    typedef std::shared_ptr <PlatformVersion> shared_ptr;

private:
    ext::optional<std::string> _projectName;
    ext::optional<std::string> _productBuildVersion;
    ext::optional<std::string> _buildVersion;
    ext::optional<std::string> _sourceVersion;
    ext::optional<std::string> _bundleShortVersionString;
    ext::optional<std::string> _bundleVersion;

public:
    PlatformVersion();

public:
    inline ext::optional<std::string> const &projectName() const
    { return _projectName; }
    inline ext::optional<std::string> const &projectBuildVersion() const
    { return _productBuildVersion; }
    inline ext::optional<std::string> const &buildVersion() const
    { return _buildVersion; }
    inline ext::optional<std::string> const &sourceVersion() const
    { return _sourceVersion; }
    inline ext::optional<std::string> const &bundleShortVersionString() const
    { return _bundleShortVersionString; }
    inline ext::optional<std::string> const &bundleVersion() const
    { return _bundleVersion; }

public:
    static PlatformVersion::shared_ptr Open(libutil::Filesystem const *filesystem, std::string const &path);

private:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcsdk_SDK_PlatformVersion_h
