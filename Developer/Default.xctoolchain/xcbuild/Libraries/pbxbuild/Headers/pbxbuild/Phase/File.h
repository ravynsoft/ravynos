/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Phase_File_h
#define __pbxbuild_Phase_File_h

#include <pbxbuild/Base.h>
#include <pbxbuild/Tool/Input.h>

namespace libutil { class Filesystem; }
namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Phase {

class Environment;

/*
 * Represents a resolved file to build as part of a phase.
 */
class File {
private:
    File();
    ~File();

public:
    /*
     * Resolves build files into `Tool::Input`s, using the phase environment. Note that due to
     * certain types of build files being more complex than a single `Tool::Input`, the number
     * of `Tool::Input`s returned may be different than the number of build files passed in.
     */
    static std::vector<Tool::Input>
    ResolveBuildFiles(libutil::Filesystem const *filesystem, Phase::Environment const &phaseEnvironment, pbxsetting::Environment const &environment, std::vector<pbxproj::PBX::BuildFile::shared_ptr> const &buildFiles);
};

}
}

#endif // !__pbxbuild_Phase_File_h
