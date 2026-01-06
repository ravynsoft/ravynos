/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcdriver_BuildAction_h
#define __xcdriver_BuildAction_h

namespace libutil { class Filesystem; }
namespace process { class Context; }
namespace process { class Launcher; }
namespace process { class User; }

namespace xcdriver {

class Options;

class BuildAction {
private:
    BuildAction();
    ~BuildAction();

public:
    static int
    Run(process::User const *user, process::Context const *processContext, process::Launcher *processLauncher, libutil::Filesystem *filesystem, Options const &options);
};

}

#endif // !__xcdriver_BuildAction_h
