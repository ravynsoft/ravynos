/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcdriver_Action_h
#define __xcdriver_Action_h

#include <pbxsetting/Environment.h>
#include <pbxsetting/Level.h>
#include <xcexecution/Parameters.h>

#include <string>
#include <vector>
#include <ext/optional>

namespace libutil { class Filesystem; }
namespace process { class Context; }

namespace xcdriver {

class Options;

class Action {
private:
    Action();
    ~Action();

public:
    /*
     * The possible basic actions that the driver can take.
     */
    enum Type {
        Build,
        ShowBuildSettings,
        List,
        Version,
        Usage,
        Help,
        License,
        CheckFirstLaunch,
        ShowSDKs,
        Find,
        ExportArchive,
        Localizations,
    };

public:
    /*
     * Determine the action from a raw set of options. Picks the right
     * action even if multiple are specified or conflicting options are.
     */
    static Type
    Determine(Options const &options);

public:
    /*
     * Verifies that the passed in build actions are valid.
     */
    static bool
    VerifyBuildActions(std::vector<std::string> const &actions);

public:
    static std::vector<pbxsetting::Level>
    CreateOverrideLevels(process::Context const *processContext, libutil::Filesystem const *filesystem, pbxsetting::Environment const &environment, Options const &options, std::string const &workingDirectory);

public:
    static xcexecution::Parameters
    CreateParameters(Options const &options, std::vector<pbxsetting::Level> const &overrideLevels);
};

}

#endif // !__xcdriver_Action_h
