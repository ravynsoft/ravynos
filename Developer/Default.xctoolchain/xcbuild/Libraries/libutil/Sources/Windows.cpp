/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/Windows.h>

#include <cstring>
#include <ext/optional>

namespace Path = libutil::Path;

char Path::Windows::
Separator = '\\';

bool Path::Windows::
IsSeparator(char c)
{
    return (c == Path::Windows::Separator || c == '/');
}

enum class WindowsPathType {
    Relative,
    DriveAbsolute,
    DriveRelative,
    DriveRooted,
    UNC,
    DOSDevice,
    NTWin32,
    NTNative,
    DOSSpecialDevice,
};

static WindowsPathType
DetermineWindowsPathType(std::string const &path, std::string::size_type *start = nullptr)
{
    std::string::size_type _start;
    start = (start != nullptr ? start : &_start);

    if (path.size() >= 3 && ::isalpha(path[0]) && path[1] == ':' && Path::Windows::IsSeparator(path[2])) {
        *start = 3;
        return WindowsPathType::DriveAbsolute;
    } else if (path.size() >= 2 && ::isalpha(path[0]) && path[1] == ':') {
        *start = 2;
        return WindowsPathType::DriveRelative;
    } else if (path.size() >= 4 && path[0] == '\\' && path[1] == '?' && path[2] == '?' && path[3] == '\\') {
        *start = 4;
        return WindowsPathType::NTNative;
    } else if (path.size() >= 4 && path[0] == '\\' && path[1] == '\\' && path[2] == '?' && path[3] == '\\') {
        *start = 4;
        return WindowsPathType::NTWin32;
    } else if (path.size() >= 4 && Path::Windows::IsSeparator(path[0]) && Path::Windows::IsSeparator(path[1]) && path[2] == '.'  && Path::Windows::IsSeparator(path[3])) {
        *start = 4;
        return WindowsPathType::DOSDevice;
    } else if (path.size() >= 2 && Path::Windows::IsSeparator(path[0]) && Path::Windows::IsSeparator(path[1]) && (path.size() < 3 || (path[2] != '.' && path[2] != '?'))) {
        *start = 2;
        return WindowsPathType::UNC;
    } else if (path.size() >= 1 && Path::Windows::IsSeparator(path[0])) {
        *start = 1;
        return WindowsPathType::DriveRooted;
    } else if (
        (path == "PRN" || path == "AUX" || path == "NUL" || path == "CON") ||
        (path.size() == 4 && path[0] == 'L' && path[1] == 'P' && path[2] == 'T' && ::isdigit(path[3]) && path[3] != '0') ||
        (path.size() == 4 && path[0] == 'C' && path[1] == 'O' && path[2] == 'M' && ::isdigit(path[3]) && path[3] != '0') ||
        (path == "CONIN$" || path == "CONOUT$")) {
        *start = 0;
        return WindowsPathType::DOSSpecialDevice;
    } else {
        *start = 0;
        return WindowsPathType::Relative;
    }
}

bool Path::Windows::
IsAbsolute(std::string const &path, size_t *start)
{
    switch (DetermineWindowsPathType(path, start)) {
        case WindowsPathType::Relative:
            return false;
        case WindowsPathType::DriveAbsolute:
            return true;
        case WindowsPathType::DriveRelative:
            return false;
        case WindowsPathType::DriveRooted:
            return false;
        case WindowsPathType::UNC:
            return true;
        case WindowsPathType::DOSDevice:
            return true;
        case WindowsPathType::NTWin32:
            return true;
        case WindowsPathType::NTNative:
            return true;
        case WindowsPathType::DOSSpecialDevice:
            return true;
    }

    abort();
}

static ext::optional<char>
DriveLetter(std::string const &path, std::string::size_type *end)
{
    switch (DetermineWindowsPathType(path, end)) {
        case WindowsPathType::DriveAbsolute:
        case WindowsPathType::DriveRelative:
            /* Is on a drive. */
            return path[0];
        case WindowsPathType::DOSDevice:
        case WindowsPathType::NTWin32:
        case WindowsPathType::NTNative:
            if (path.size() > *end + 3 && ::isalpha(path[*end + 0]) && path[*end + 1] == ':' && Path::Windows::IsSeparator(path[*end + 2])) {
                /* Has a drive letter after the root. */
                *end += 3;
                return path[*end + 0];
            } else {
                /* Some other kind of path. */
                return ext::nullopt;
            }
        case WindowsPathType::Relative:
        case WindowsPathType::DriveRooted:
        case WindowsPathType::UNC:
        case WindowsPathType::DOSSpecialDevice:
            /* Not a drive path. */
            return ext::nullopt;
    }

    abort();
}

bool Path::Windows::
Resolve(
    std::string const &path,
    std::string const &against,
    std::string *base,
    std::string *relative)
{
    std::string::size_type start;
    switch (DetermineWindowsPathType(path, &start)) {
        case WindowsPathType::Relative: {
            *base = against;
            *relative = path;
            return true;
        }
        case WindowsPathType::DriveRelative: {
            std::string::size_type end;
            ext::optional<char> drive = DriveLetter(against, &end);
            if (!drive) {
                /* No drive to resolve against. Unsupported. */
                return false;
            }

            if (path[0] != drive) {
                /* Drive-relative path for another drive. Unsupported. */
                return false;
            }

            *base = against.substr(0, end);
            *relative = path.substr(start);
            return true;
        }
        case WindowsPathType::DriveRooted: {
            std::string::size_type end;
            ext::optional<char> drive = DriveLetter(against, &end);
            if (!drive) {
                /* No drive to resolve against. Unsupported. */
                return false;
            }

            *base = against.substr(0, end);
            *relative = path.substr(start);
            return true;
        }
        case WindowsPathType::DriveAbsolute:
        case WindowsPathType::DOSDevice:
        case WindowsPathType::NTWin32:
        case WindowsPathType::NTNative:
        case WindowsPathType::UNC:
        case WindowsPathType::DOSSpecialDevice:
            /* Absolute path, should not get here. */
            abort();
    }

    abort();
}

