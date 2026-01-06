/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcsdk_Environment_h
#define __xcsdk_Environment_h

#include <string>
#include <ext/optional>

namespace libutil { class Filesystem; };
namespace process { class Context; };
namespace process { class User; };

namespace xcsdk {

class Environment {
private:
    Environment();
    ~Environment();

public:
    static ext::optional<std::string> DeveloperRoot(
        process::User const *user,
        process::Context const *processContext,
        libutil::Filesystem const *filesystem);

public:
    /*
     * Set a new developer root. Returns success;
     */
    static bool WriteDeveloperRoot(
        libutil::Filesystem *filesystem,
        ext::optional<std::string> const &path);
};

}

#endif // !__xcsdk_Environment_h
