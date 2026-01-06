/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __process_DefaultContext_h
#define __process_DefaultContext_h

#include <process/Context.h>

namespace process {

/*
 * A process context for the current process. Generally, this should only
 * be created in `main()` and passed down to anywhere else that needs it.
 */
class DefaultContext : public Context {
public:
    explicit DefaultContext();
    virtual ~DefaultContext();

public:
    virtual std::string const &executablePath() const;
    virtual std::string const &currentDirectory() const;

public:
    virtual std::vector<std::string> const &commandLineArguments() const;
    virtual std::unordered_map<std::string, std::string> const &environmentVariables() const;
    virtual ext::optional<std::string> environmentVariable(std::string const &variable) const;
};

}

#endif  // !__process_DefaultContext_h
