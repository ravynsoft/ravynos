/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_FileTypeResolver_h
#define __pbxbuild_FileTypeResolver_h

#include <pbxbuild/Base.h>
#include <pbxbuild/Build/Environment.h>

namespace libutil { class Filesystem; }

namespace pbxbuild {

/*
 * Determines the file type of a file.
 */
class FileTypeResolver {
private:
    FileTypeResolver();
    ~FileTypeResolver();

public:
    /*
     * Determine the file type of a file path.
     */
    static pbxspec::PBX::FileType::shared_ptr
    Resolve(libutil::Filesystem const *filesystem, pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &domains, std::string const &filePath);

    /*
     * Determine the file type of a file reference. If a file reference is available, use
     * this method instead of the one with just the file type, as the reference can override
     * the automatically determined file type from the file path.
     */
    static pbxspec::PBX::FileType::shared_ptr
    Resolve(libutil::Filesystem const *filesystem, pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &domains, pbxproj::PBX::FileReference::shared_ptr const &fileReference, std::string const &filePath);

    /*
     * Determine the file type of a version group. Uses the explicit file type or falls back
     * to autodetecting the file type from the path provided.
     */
    static pbxspec::PBX::FileType::shared_ptr
    Resolve(libutil::Filesystem const *filesystem, pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &domains, pbxproj::XC::VersionGroup::shared_ptr const &versionGroup, std::string const &filePath);
};

}

#endif // !__pbxbuild_FileTypeResolver_h
