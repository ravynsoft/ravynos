/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __process_Launcher_h
#define __process_Launcher_h

#include <ext/optional>

namespace libutil { class Filesystem; }

namespace process {

class Context;

/*
 * Abstract process launcher.
 */
class Launcher {
protected:
    Launcher();
    ~Launcher();

public:
    /*
     * Launch and wait for a process. The filesystem is symbolic, to note
     * that launching a process could arbitrarily affect the filesystem.
     */
    virtual ext::optional<int> launch(libutil::Filesystem *filesystem, Context const *context) = 0;
};

}

#endif  // !__process_Launcher_h
