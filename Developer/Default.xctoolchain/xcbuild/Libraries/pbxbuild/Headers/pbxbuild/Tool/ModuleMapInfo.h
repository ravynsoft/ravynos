/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_ModuleMapInfo_h
#define __pbxbuild_Tool_ModuleMapInfo_h

#include <pbxbuild/Tool/AuxiliaryFile.h>

#include <string>
#include <ext/optional>

namespace pbxbuild {
namespace Tool {

/*
 * Information about generated module maps.
 */
class ModuleMapInfo {
public:
    /*
     * Information about a specific generated module map.
     */
    class Entry {
    private:
        Tool::AuxiliaryFile::Chunk _contents;

    private:
        std::string                _intermediatePath;
        std::string                _finalPath;

    public:
        Entry(
            Tool::AuxiliaryFile::Chunk const &contents,
            std::string const &intermediatePath,
            std::string const &finalPath);

    public:
        /*
         * The contents of the module map. May reference a file.
         */
        Tool::AuxiliaryFile::Chunk const &contents() const
        { return _contents; }

    public:
        /*
         * The temporary path the module map is written to.
         */
        std::string const &intermediatePath() const
        { return _intermediatePath; }

        /*
         * The product path the module map is copied to.
         */
        std::string const &finalPath() const
        { return _finalPath; }
    };

private:
    ext::optional<Entry> _moduleMap;
    ext::optional<Entry> _privateModuleMap;

public:
    ModuleMapInfo();

public:
    /*
     * The public module map.
     */
    ext::optional<Entry> const &moduleMap() const
    { return _moduleMap; }
    ext::optional<Entry> &moduleMap()
    { return _moduleMap; }

    /*
     * The private module map.
     */
    ext::optional<Entry> const &privateModuleMap() const
    { return _privateModuleMap; }
    ext::optional<Entry> &privateModuleMap()
    { return _privateModuleMap; }
};

}
}

#endif // !__pbxbuild_Tool_ModuleMapInfo_h
