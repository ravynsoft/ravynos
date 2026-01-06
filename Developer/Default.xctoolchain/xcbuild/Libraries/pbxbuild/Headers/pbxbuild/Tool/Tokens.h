/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_Tokens_h
#define __pbxbuild_Tool_Tokens_h

#include <string>
#include <vector>

namespace pbxbuild {
namespace Tool {

class Environment;
class OptionsResult;

/*
 * Evaluates tokens of the form [token], as found in tools.
 */
class Tokens {
private:
    Tokens();
    ~Tokens();

public:
    /*
     * Evaluate tokens with the provided inputs.
     */
    static std::vector<std::string>
    Expand(
        std::string const &tokenized,
        std::string const &executable,
        std::vector<std::string> const &arguments,
        std::vector<std::string> const &specialArgs,
        std::vector<std::string> const &inputs,
        std::vector<std::string> const &outputs);

public:
    /*
     * Expansions from a tool environment.
     */
    class ToolExpansions {
    private:
        std::string              _executable;
        std::vector<std::string> _arguments;

    private:
        std::string              _logMessage;

    public:
        ToolExpansions(
            std::string const &executable,
            std::vector<std::string> const &arguments,
            std::string const &logMessage);
        ~ToolExpansions();

    public:
        /*
         * The first argument.
         */
        std::string const &executable() const
        { return _executable; }

        /*
         * The remaining arguments after the first.
         */
        std::vector<std::string> const &arguments() const
        { return _arguments; }

    public:
        /*
         * The command's log message.
         */
        std::string const &logMessage() const
        { return _logMessage; }
    };

    /*
     * Evaluate tokens for a tool's command line and log message.
     */
    static Tool::Tokens::ToolExpansions
    ExpandTool(
        Tool::Environment const &toolEnvironment,
        Tool::OptionsResult const &options,
        std::string const &executable = "",
        std::vector<std::string> const &specialArguments = { });
};

}
}

#endif // !__pbxbuild_Tool_Tokens_h
