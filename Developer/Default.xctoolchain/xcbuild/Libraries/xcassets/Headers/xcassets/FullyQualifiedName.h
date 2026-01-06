/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_FullyQualifiedName_h
#define __xcassets_FullyQualifiedName_h

#include <string>
#include <vector>

namespace xcassets {

/*
 * The unique identifier of an asset in a catalog.
 */
class FullyQualifiedName {
private:
    std::vector<std::string> _groups;
    std::string              _name;

public:
    FullyQualifiedName(std::vector<std::string> const &groups, std::string const &name);

public:
    /*
     * The list of groups containing this asset.
     */
    std::vector<std::string> const &groups() const
    { return _groups; }

    /*
     * The name of the asset.
     */
    std::string const &name() const
    { return _name; }

public:
    /*
     * The string representation of the asset.
     */
    std::string string() const;

    /*
     * Parse a fully qualified name from a string.
     */
    static FullyQualifiedName Parse(std::string const &string);
};

}

#endif // !__xcassets_FullyQualifiedName_h
