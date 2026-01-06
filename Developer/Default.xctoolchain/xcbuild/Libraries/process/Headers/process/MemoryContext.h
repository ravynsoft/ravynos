/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __process_MemoryContext_h
#define __process_MemoryContext_h

#include <process/Context.h>

namespace process {

/*
 * A process context with arbitrary values.
 */
class MemoryContext : public Context {
private:
    std::string _executablePath;
    std::string _currentDirectory;

private:
    std::vector<std::string> _commandLineArguments;
    std::unordered_map<std::string, std::string> _environmentVariables;

public:
    MemoryContext(
        std::string const &executablePath,
        std::string const &currentDirectory,
        std::vector<std::string> const &commandLineArguments,
        std::unordered_map<std::string, std::string> const &environmentVariables);
    explicit MemoryContext(Context const *context);
    virtual ~MemoryContext();

public:
    virtual std::string const &executablePath() const
    { return _executablePath; }
    std::string &executablePath()
    { return _executablePath; }

    virtual std::string const &currentDirectory() const
    { return _currentDirectory; }
    std::string &currentDirectory()
    { return _currentDirectory; }

public:
    virtual std::vector<std::string> const &commandLineArguments() const
    { return _commandLineArguments; }
    std::vector<std::string> &commandLineArguments()
    { return _commandLineArguments; }

    virtual std::unordered_map<std::string, std::string> const &environmentVariables() const
    { return _environmentVariables; }
    std::unordered_map<std::string, std::string> &environmentVariables()
    { return _environmentVariables; }

    virtual ext::optional<std::string> environmentVariable(std::string const &variable) const;
};

}

#endif  // !__process_MemoryContext_h
