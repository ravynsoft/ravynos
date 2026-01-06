/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxsetting_Value_h
#define __pbxsetting_Value_h

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace plist { class Object; }

namespace pbxsetting {

/*
 * An arbitrary build setitng value. Expressions can contain
 * literal strings and setting references. Note setting references
 * can be nested and mixed with strings to resolve to different
 * settings based on the value of a third setting:
 *
 *     $(SUFFIX_$(INDEX))_VALUE
 *
 * This class stores, not resolves, build setting value.
 */
class Value {
public:
    /*
     * A node in the AST describing the value. Can be a literal
     * string or a reference to another build setting. References are
     * themselves made up of arbitrary build setting values.
     */
    class Entry {
    public:
        enum class Type {
            String,
            Value,
        };

    private:
        Type                       _type;
        ext::optional<std::string> _string;
        std::shared_ptr<Value>     _value;

    public:
        Entry(std::string const &string);
        Entry(std::shared_ptr<Value> const &value);

    public:
        bool operator==(Entry const &rhs) const;
        bool operator!=(Entry const &rhs) const;

    public:
        Type type() const
        { return _type; }
        ext::optional<std::string> const &string() const
        { return _string; }
        std::shared_ptr<Value> const &value() const
        { return _value; }
    };

private:
    std::vector<Entry> _entries;

public:
    Value(std::vector<Entry> const &entries);
    ~Value();

public:
    bool operator==(Value const &rhs) const;
    bool operator!=(Value const &rhs) const;
    Value operator+(Value const &rhs) const;

public:
    /*
     * The top level of the AST that makes up this value.
     */
    std::vector<Entry> const &entries() const
    { return _entries; }

public:
    /*
     * The raw representation of the value. This string will be
     * normalized to always use the standard $(SETTING) syntax, and
     * not the variant ${SETTING} or $SETTING syntaxes.
     */
    std::string
    raw() const;

public:
    /*
     * A value representing an empty string.
     */
    static Value const &
    Empty(void);

    /*
     * Creates a value representing a raw string.
     */
    static Value
    String(std::string const &value);

    /*
     * Creates a value representing a setting reference. Note the
     * reference name itself is *not* parsed as a value.
     */
    static Value
    Variable(std::string const &value);

    /*
     * Parses a string into a value. By default, all content is treated
     * as a raw string. Setting references can use one of three syntaxes:
     *
     *  1. $(SETTING) <- default, recommended
     *  2. ${SETTING}
     *  3. $SETTING
     *
     * Nested setting references are parsed.
     */
    static Value
    Parse(std::string const &value);

    /*
     * Creates a value from the string representation of a property list.
     * Only certain string-convertible property lists types are supported.
     */
    static Value
    FromObject(plist::Object const *object);
};

}

#endif  // !__pbxsetting_Value_h
