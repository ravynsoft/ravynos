/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __acdriver_Output_h
#define __acdriver_Output_h

#ifdef __linux__
#include <cstdint>
#endif
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <ext/optional>

namespace plist { class Dictionary; }
namespace plist { class Object; }

namespace acdriver {

/*
 * Represents the output of a command.
 */
class Output {
public:
    /*
     * Output formats supported.
     */
    enum class Format {
        XML,
        Binary,
        Text,
    };

private:
    std::unique_ptr<plist::Dictionary>           _values;
    std::unordered_map<std::string, std::string> _texts;

public:
    Output();
    ~Output();

public:
    /*
     * Add an output entry.
     */
    void add(
        std::string const &key,
        std::unique_ptr<plist::Object> value,
        std::string const &text);

    /*
     * Output encoded as the specified format.
     */
    ext::optional<std::vector<uint8_t>> serialize(Format format) const;
};

}

#endif // !__acdriver_Output_h
