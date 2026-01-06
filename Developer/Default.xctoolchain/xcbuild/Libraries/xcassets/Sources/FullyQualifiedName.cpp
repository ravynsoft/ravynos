/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/FullyQualifiedName.h>

using xcassets::FullyQualifiedName;

FullyQualifiedName::
FullyQualifiedName(std::vector<std::string> const &groups, std::string const &name) :
    _groups(groups),
    _name  (name)
{
}

std::string FullyQualifiedName::
string() const
{
    std::string string;
    for (std::string const &group : _groups) {
        string += group;
        string += "/";
    }
    string += _name;
    return string;
}

FullyQualifiedName FullyQualifiedName::
Parse(std::string const &string)
{
    /* Split on slashes. */
    std::string::size_type prev = 0;
    std::string::size_type pos = string.find('/');
    std::vector<std::string> groups;
    while (pos != std::string::npos) {
        std::string group = string.substr(prev, pos - prev);
        if (!group.empty()) {
            groups.push_back(group);
        }

        prev = pos + 1;
        pos = string.find('/', prev);
    }

    /* The name is the rest of the string. */
    std::string name = string.substr(prev, string.size() - prev);

    return FullyQualifiedName(groups, name);
}

