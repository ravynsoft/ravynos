/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __ninja_Value_h
#define __ninja_Value_h

#include <string>
#include <utility>
#include <vector>

namespace ninja {

/*
 * Represents a value for use in a Ninja file that has been
 * considered for escaping. It may or may not be actually escaped.
 */
struct Value {
private:
    struct Chunk {
    public:
        enum class Type {
            String,
            Expression,
        };

    private:
        Type        _type;
        std::string _value;

    public:
        Chunk(Type type, std::string const &value);

    public:
        bool operator==(Chunk const &rhs) const;
        bool operator!=(Chunk const &rhs) const;

    public:
        Type type() const
        { return _type; }
        std::string const &value() const
        { return _value; }
    };

private:
    std::vector<Chunk> _chunks;

private:
    explicit Value(std::vector<Chunk> const &chunks);

public:
    bool operator==(Value const &rhs) const;
    bool operator!=(Value const &rhs) const;
    Value operator+(Value const &rhs) const;

public:
    enum class EscapeMode {
        /*
         * Escape for a binding value.
         */
        Value,
        /*
         * Escape for a list of paths.
         */
        PathList,
        /*
         * Escape for a path list in a `build` command.
         */
        BuildPathList,
    };

public:
    /*
     * The value to put in the Ninja file.
     */
    std::string resolve(EscapeMode mode) const;

public:
    /*
     * Create an empty Ninja value.
     */
    static Value Empty();

    /*
     * Create a Ninja value by escpaing a string.
     */
    static Value String(std::string const &string);

    /*
     * Create a Ninja value from a raw, unescaped expression.
     */
    static Value Expression(std::string const &expression);
};

/*
 * A binding for a variable to write in a Ninja file.
 */
typedef std::pair<std::string, Value> Binding;

}

#endif // !__ninja_Value_h
