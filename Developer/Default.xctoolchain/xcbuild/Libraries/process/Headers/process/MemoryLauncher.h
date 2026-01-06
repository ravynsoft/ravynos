/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __process_MemoryLauncher_h
#define __process_MemoryLauncher_h

#include <process/Launcher.h>

#include <string>
#include <functional>
#include <unordered_map>
#include <ext/optional>

namespace process {

/*
 * In-memory simulated process launcher.
 */
class MemoryLauncher : public Launcher {
public:
    /*
     * Handler for a simulated process launch.
     */
    using Handler = std::function<ext::optional<int>(libutil::Filesystem *filesystem, Context const *context)>;

private:
    std::unordered_map<std::string, Handler> _handlers;

public:
    MemoryLauncher(std::unordered_map<std::string, Handler> const &handlers);
    ~MemoryLauncher();

public:
    virtual ext::optional<int> launch(libutil::Filesystem *filesystem, Context const *context);
};

}

#endif  // !__process_MemoryLauncher_h
