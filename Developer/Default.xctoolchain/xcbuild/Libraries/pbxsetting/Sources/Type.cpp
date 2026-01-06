/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxsetting/Type.h>
#include <libutil/Strings.h>
#include <libutil/Wildcard.h>

#include <iomanip>
#include <sstream>

using pbxsetting::Type;
using libutil::Wildcard;

Type::
Type()
{
}

Type::
~Type()
{
}

bool Type::
ParseBoolean(std::string const &value)
{
    return libutil::strcasecmp(value.c_str(), "yes") == 0 || libutil::strcasecmp(value.c_str(), "true") == 0;
}

int64_t Type::
ParseInteger(std::string const &value)
{
    return std::strtoll(value.c_str(), NULL, 0);
}

std::vector<std::string> Type::
ParseList(std::string const &value)
{
    std::vector<std::string> entries;

    std::string str;
    char quote = '\0';
    bool escaped = false;

    for (char c : value) {
        if (!escaped) {
            if (c == '\'' || c == '"') {
                if (quote != '\0') {
                    if (c == quote) {
                        quote = '\0';
                        continue;
                    }
                } else {
                    quote = c;
                    continue;
                }
            } else if (c == '\\') {
                escaped = true;
                continue;
            } else if (quote == '\0' && isspace(c)) {
                if (!str.empty()) {
                    entries.push_back(str);
                    str = std::string();
                }
                continue;
            }
        }

        str += c;
        escaped = false;
    }

    if (!str.empty()) {
        entries.push_back(str);
        str = std::string();
    }

    return entries;
}

std::string Type::
FormatBoolean(bool value)
{
    return (value ? "YES" : "NO");
}

std::string Type::
FormatInteger(int64_t value)
{
    return std::to_string(value);
}

std::string Type::
FormatReal(double value)
{
    std::ostringstream result;
    //result << std::setprecision(1) << std::fixed << value;
    result << value;
    return result.str();
}

std::string Type::
FormatList(std::vector<std::string> const &value)
{
    std::ostringstream result;

    for (std::string const &entry : value) {
        if (&entry != &value.front()) {
            result << " ";
        }

        for (char c : entry) {
            if (c == '\\' || c == '\'' || c == '"' || isspace(c)) {
                result << '\\' << c;
            } else {
                result << c;
            }
        }
    }

    return result.str();
}
