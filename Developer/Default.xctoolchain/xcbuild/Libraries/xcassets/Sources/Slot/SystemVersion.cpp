/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/SystemVersion.h>

#include <sstream>

using xcassets::Slot::SystemVersion;

SystemVersion::
SystemVersion(int major, int minor, ext::optional<int> const &patch) :
    _major(major),
    _minor(minor),
    _patch(patch)
{
}

ext::optional<SystemVersion> SystemVersion::
Parse(std::string const &value)
{
    /* Must not be empty */
    if (value.empty()) {
        fprintf(stderr, "warning: system version not valid %s\n", value.c_str());
        return ext::nullopt;
    }

    std::string::size_type dot = value.find('.');
    if (dot == std::string::npos || dot == 0 || dot == value.size() - 1) {
        /* Must contain at least one dot. */
        fprintf(stderr, "warning: system version not valid %s\n", value.c_str());
        return ext::nullopt;
    }

    /* Major is up to the first dot. */
    std::string m = value.substr(0, dot);
    std::string rest = value.substr(dot + 1, value.size() - dot - 1);

    char *mend = NULL;
    int major = std::strtol(m.c_str(), &mend, 0);
    if (mend != &m[m.size()]) {
        fprintf(stderr, "warning: major version not valid %s\n", m.c_str());
        return ext::nullopt;
    }

    dot = rest.find('.');
    if (dot == 0 || dot == rest.size() - 1) {
        /* Cannot be empty or end with a dot. */
        fprintf(stderr, "warning: system version not valid %s\n", value.c_str());
        return ext::nullopt;
    }

    /* Minor is up to the next dot. */
    std::string n = rest.substr(0, dot);

    char *nend = NULL;
    int minor = std::strtol(n.c_str(), &nend, 0);
    if (nend != &n[n.size()]) {
        fprintf(stderr, "warning: minor version not valid %s\n", n.c_str());
        return ext::nullopt;
    }

    /* Optional patch is after the second dot. */
    ext::optional<int> patch;
    if (dot != std::string::npos) {
        std::string p = rest.substr(dot + 1, rest.size() - dot - 1);

        char *pend = NULL;
        patch = std::strtol(p.c_str(), &pend, 0);
        if (pend != &p[p.size()]) {
            fprintf(stderr, "warning: patch version not valid %s\n", p.c_str());
            return ext::nullopt;
        }
    }

    return SystemVersion(major, minor, patch);
}

std::string SystemVersion::
String(SystemVersion systemVersion)
{
    std::ostringstream out;
    out << systemVersion.major();
    out << ".";
    out << systemVersion.minor();
    if (systemVersion.patch()) {
        out << ".";
        out << *systemVersion.patch();
    }
    return out.str();
}
