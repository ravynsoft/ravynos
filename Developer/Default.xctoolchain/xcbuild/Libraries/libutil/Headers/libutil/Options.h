/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_Options_h
#define __libutil_Options_h

#include <string>
#include <vector>
#include <ext/optional>

namespace libutil {

/*
 * Utilities for parsing command-line options.
 */
class Options {
private:
    Options();
    ~Options();

public:
    /*
     * Parse the next argument as the specified type.
     */
    template<typename T>
    static std::pair<bool, std::string>
    Next(ext::optional<T> *result, std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it, bool allowDuplicate = false);

    /*
     * Parse the next argument as the specified type, appending it to a list.
     */
    template<typename T>
    static std::pair<bool, std::string>
    AppendNext(std::vector<T> *result, std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);

public:
    /*
     * Parse the current argument as the specified type.
     */
    template<typename T>
    static std::pair<bool, std::string>
    Current(ext::optional<T> *result, std::string const &arg, bool allowDuplicate = false);

    /*
     * Parse the current argument as the specified type, appending it to a list.
     */
    template<typename T>
    static std::pair<bool, std::string>
    AppendCurrent(std::vector<T> *result, std::string const &arg);

public:
    /*
     * Parse arguments into the specified options type.
     */
    template<typename T>
    static std::pair<bool, std::string>
    Parse(T *options, std::vector<std::string> const &args)
    {
        for (auto it = args.begin(); it != args.end(); ++it) {
            std::pair<bool, std::string> result = options->parseArgument(args, &it);
            if (!result.first) {
                return result;
            }
        }

        return std::make_pair<bool, std::string>(true, std::string());
    }
};

}

#endif  // !__libutil_Options_h
