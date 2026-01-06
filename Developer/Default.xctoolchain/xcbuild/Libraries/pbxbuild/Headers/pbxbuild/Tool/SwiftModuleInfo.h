/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_SwiftModuleInfo_h
#define __pbxbuild_Tool_SwiftModuleInfo_h

#include <pbxbuild/Tool/PrecompiledHeaderInfo.h>

namespace pbxbuild {
namespace Tool {

/*
 * Represents the information about a Swift module that was built.
 * The properties are useful for installing the Swift module and
 * header a location for use by dependent modules.
 */
class SwiftModuleInfo {
private:
    std::string _architecture;
    std::string _moduleName;

private:
    std::string _modulePath;
    std::string _docPath;
    std::string _headerPath;

private:
    bool        _installHeader;

private:
    std::vector<std::string> _copiedArtifacts;

public:
    SwiftModuleInfo(
        std::string const &architecture,
        std::string const &moduleName,
        std::string const &modulePath,
        std::string const &docPath,
        std::string const &headerPath,
        bool installHeader);

public:
    /*
     * The architecture being built. Note that Swift modules have custom
     * names for certain architectures, this does *not* use that scheme.
     */
    std::string const &architecture() const
    { return _architecture; }

    /*
     * The name of the Swift module.
     */
    std::string const &moduleName() const
    { return _moduleName; }

public:
    /*
     * The path to the Swift module.
     */
    std::string const &modulePath() const
    { return _modulePath; }

    /*
     * The path to the Swift documentation output.
     */
    std::string const &docPath() const
    { return _docPath; }

    /*
     * The path to the Objective-C header output by Swift.
     */
    std::string const &headerPath() const
    { return _headerPath; }

public:
    /*
     * Whether the header should be installed in the product.
     */
    bool installHeader() const
    { return _installHeader; }

public:
    /*
     * List of artifacts copied at time of CopySwiftModules call.
     */
    std::vector<std::string> &copiedArtifacts()
    { return _copiedArtifacts; }
};

}
}

#endif // !__pbxbuild_Tool_SwiftModuleInfo_h
