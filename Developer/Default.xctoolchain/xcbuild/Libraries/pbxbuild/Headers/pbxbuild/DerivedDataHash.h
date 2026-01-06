/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_DerivedDataHash_h
#define __pbxbuild_DerivedDataHash_h

#ifdef __linux__
#include <cstdint>
#endif
#include <pbxsetting/Setting.h>

#include <string>
#include <vector>

namespace pbxbuild {

/*
 * The path within derived data that a workspace is stored. Creatable without loading
 * a workspace or project as the only input is the path to the workspace or project.
 */
class DerivedDataHash {
private:
    std::string _name;
    std::string _hash;

public:
    DerivedDataHash(
        std::string const &name,
        std::string const &hash);

public:
    /*
     * The base name of the input path.
     */
    std::string const &name() const
    { return _name; }

    /*
     * The computed hash of the input path.
     */
    std::string const &hash() const
    { return _hash; }

public:
    /*
     * The derived data path.
     */
    std::string derivedDataHash() const;

    /*
     * Build setting overrides to use the derived data path.
     */
    std::vector<pbxsetting::Setting> overrideSettings() const;

public:
    /*
     * Create a derived data hash. The path is the path to the loaded file, either
     * a workspace or a project file. The absolute path and base name are used.
     */
    static DerivedDataHash
    Create(std::string const &path);
};

}

#endif  // !__pbxbuild_DerivedDataHash_h
