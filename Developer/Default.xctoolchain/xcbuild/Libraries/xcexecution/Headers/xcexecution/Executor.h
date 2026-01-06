/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcexecution_Executor_h
#define __xcexecution_Executor_h

#include <xcformatter/Formatter.h>

#include <memory>

namespace libutil { class Filesystem; }
namespace process { class Context; }
namespace process { class Launcher; }
namespace process { class User; }

namespace pbxbuild {
namespace Build { class Context; }
namespace Build { class Environment; }
namespace Target { class Environment; }
}

namespace xcexecution {

class Parameters;

/*
 * Abstract executor for builds. The executor is responsible for creating
 * environments for the target graph and actually executing the build, taking
 * into account the `formatter` and `dryRun` parameters passed in.
 */
class Executor {
protected:
    std::shared_ptr<xcformatter::Formatter> _formatter;
    bool                                    _dryRun;
    bool                                    _generate;

protected:
    Executor(std::shared_ptr<xcformatter::Formatter> const &formatter, bool dryRun, bool generate);

public:
    virtual ~Executor();

public:
    /*
     * Abstract build method. Override to implement the build.
     */
    virtual bool build(
        process::User const *user,
        process::Context const *processContext,
        process::Launcher *processLauncher,
        libutil::Filesystem *filesystem,
        pbxbuild::Build::Environment const &buildEnvironment,
        Parameters const &buildParameters) = 0;
};

}

#endif // !__xcexecution_Executor_h
