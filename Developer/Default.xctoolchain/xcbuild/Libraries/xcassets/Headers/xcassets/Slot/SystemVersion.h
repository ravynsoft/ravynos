/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Slot_SystemVersion_h
#define __xcassets_Slot_SystemVersion_h

#include <ext/optional>
#include <string>

/*
 * for legacy reasons, the GNU C Library might define major & minor,
 * breaking the compilation unless we undefine them.
 */
#ifdef major
# undef major
#endif
#ifdef minor
# undef minor
#endif

namespace xcassets {
namespace Slot {

/*
 * The size of an image.
 */
class SystemVersion {
private:
    int                _major;
    int                _minor;
    ext::optional<int> _patch;

private:
    SystemVersion(int major, int minor, ext::optional<int> const &patch);

public:
    /*
     * The system major version.
     */
    int major() const
    { return _major; }

    /*
     * The system minor version.
     */
    int minor() const
    { return _minor; }

    /*
     * The system patch version.
     */
    ext::optional<int> patch() const
    { return _patch; }

public:
    /*
     * Parse a matching system version from a string, if valid.
     */
    static ext::optional<SystemVersion> Parse(std::string const &value);

    /*
     * Convert an system version to a string.
     */
    static std::string String(SystemVersion systemVersion);
};

}
}

#endif // !__xcassets_Slot_SystemVersion_h
