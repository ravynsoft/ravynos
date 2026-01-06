/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <ninja/Value.h>

#include <sstream>

using ninja::Value;

Value::
Value(std::vector<Value::Chunk> const &chunks) :
    _chunks(chunks)
{
}

bool Value::
operator==(Value const &rhs) const
{
    return _chunks == rhs._chunks;
}

bool Value::
operator!=(Value const &rhs) const
{
    return !(*this == rhs);
}

Value Value::
operator+(Value const &rhs) const
{
    std::vector<Value::Chunk> chunks;
    chunks.insert(chunks.end(), _chunks.begin(), _chunks.end());
    chunks.insert(chunks.end(), rhs._chunks.begin(), rhs._chunks.end());
    return Value(chunks);
}

static std::string
Replace(std::string string, std::string const &search, std::string const &replace)
{
    size_t pos = 0;

    while ((pos = string.find(search, pos)) != std::string::npos) {
         string.replace(pos, search.length(), replace);
         pos += replace.length();
    }

    return string;
}

std::string Value::
resolve(Value::EscapeMode mode) const
{
    std::ostringstream result;

    for (Value::Chunk const &chunk : _chunks) {
        switch (chunk.type()) {
            case Value::Chunk::Type::String:
                switch (mode) {
                    case Value::EscapeMode::Value:
                        /* String, value: just escape variables. Spaces are allowed. */
                        result << Replace(chunk.value(), "$", "$$");
                        break;
                    case Value::EscapeMode::PathList:
                        /* String, path list: escape variables and spaces, not colons. */
                        result << Replace(Replace(chunk.value(), "$", "$$"), " ", "$ ");
                        break;
                    case Value::EscapeMode::BuildPathList:
                        /* String, build path list: escape variables, spaces, and, colons. */
                        result << Replace(Replace(Replace(chunk.value(), "$", "$$"), " ", "$ "), ":", "$:");
                        break;
                }
                break;
            case Value::Chunk::Type::Expression:
                switch (mode) {
                    case Value::EscapeMode::Value:
                        /* Expression, value: no need to escape. Allow variables. */
                        result << chunk.value();
                        break;
                    case Value::EscapeMode::PathList:
                        /* Expression, path list: just escape spaces. */
                        result << Replace(Replace(chunk.value(), "$ ", "$$ "), " ", "$ ");
                        break;
                    case Value::EscapeMode::BuildPathList:
                        /* Expression, path list: escape spaces and colons. */
                        result << Replace(Replace(Replace(Replace(chunk.value(), "$ ", "$$ "), " ", "$ "), "$:", "$$:"), ":", "$:");
                        break;
                }
                break;
        }
    }

    return result.str();
}

Value Value::
Empty()
{
    return Value({ });
}

Value Value::
String(std::string const &string)
{
    return Value({ Value::Chunk(Value::Chunk::Type::String, string) });
}

Value Value::
Expression(std::string const &expression)
{
    return Value({ Value::Chunk(Value::Chunk::Type::Expression, expression) });
}

Value::Chunk::Chunk(Type type, std::string const &value) :
    _type (type),
    _value(value)
{
}

bool Value::Chunk::
operator==(Value::Chunk const &rhs) const
{
    return (_value.empty() && rhs._value.empty()) || (_type == rhs._type && _value == rhs._value);
}

bool Value::Chunk::
operator!=(Value::Chunk const &rhs) const
{
    return !(*this == rhs);
}
