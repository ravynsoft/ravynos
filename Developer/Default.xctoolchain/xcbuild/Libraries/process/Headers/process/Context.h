/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __process_Context_h
#define __process_Context_h

#include <string>
#include <vector>
#include <unordered_map>
#include <ext/optional>

namespace process {

/*
 * The information passed into a launched process.
 */
class Context {
protected:
    Context();
    virtual ~Context();

public:
    /*
     * The path to the running executable.
     */
    virtual std::string const &executablePath() const = 0;

    /*
     * The current directory of the process.
     */
    virtual std::string const &currentDirectory() const = 0;

public:
    /*
     * Arguments to the process.
     */
    virtual std::vector<std::string> const &commandLineArguments() const = 0;

    /*
     * All environment variables.
     */
    virtual std::unordered_map<std::string, std::string> const &environmentVariables() const = 0;

    /*
     * Single environment variable.
     */
    virtual ext::optional<std::string> environmentVariable(std::string const &variable) const = 0;

public:
    /*
     * The default environment search paths.
     */
    std::vector<std::string> executableSearchPaths() const;
};

}

#endif  // !__process_Context_h
