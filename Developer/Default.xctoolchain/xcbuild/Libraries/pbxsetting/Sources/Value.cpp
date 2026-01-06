/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxsetting/Value.h>
#include <pbxsetting/Type.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Integer.h>
#include <plist/Real.h>
#include <plist/String.h>

#include <cassert>

using pbxsetting::Value;

Value::Entry::
Entry(std::string const &string) :
    _type  (Type::String),
    _string(string)
{
}

Value::Entry::
Entry(std::shared_ptr<Value> const &value) :
    _type (Type::Value),
    _value(value)
{
}

bool Value::Entry::
operator==(Entry const &rhs) const
{
    if (_type != rhs._type) {
        return false;
    } else if (_type == Entry::Type::String) {
        return *_string == *rhs._string;
    } else if (_type == Entry::Type::Value) {
        return *_value == *rhs._value;
    } else {
        assert(false);
        return false;
    }
}

bool Value::Entry::
operator!=(Entry const &entry) const
{
    return !(*this == entry);
}

Value::
Value(std::vector<Entry> const &entries) :
    _entries(entries)
{
}

Value::
~Value()
{
}

std::string Value::
raw() const
{
    std::string out;
    for (Value::Entry const &entry : _entries) {
        switch (entry.type()) {
            case Value::Entry::Type::String: {
                out += *entry.string();
                break;
            }
            case Value::Entry::Type::Value: {
                out += "$(" + entry.value()->raw() + ")";
                break;
            }
        }
    }
    return out;
}

bool Value::
operator==(Value const &rhs) const
{
    return _entries == rhs._entries;
}

bool Value::
operator!=(Value const &rhs) const
{
    return !(*this == rhs);
}

Value Value::
operator+(Value const &rhs) const
{
    std::vector<Value::Entry> entries;
    entries.insert(entries.end(), _entries.begin(), _entries.end());

    auto it = rhs.entries().begin();
    if (!entries.empty() && !rhs.entries().empty()) {
        if (entries.back().type() == Value::Entry::Type::String && it->type() == Value::Entry::Type::String) {
            entries.back() = Entry(*entries.back().string() + *it->string());
            ++it;
        }
    }

    entries.insert(entries.end(), it, rhs.entries().end());
    return Value(entries);
}

struct ParseResult {
    bool found;
    size_t end;
    Value value;
};

enum ValueDelimiter {
    kDelimiterNone,
    kDelimiterParentheses,
    kDelimiterBraces,
    kDelimiterIdentifier,
};

static ParseResult
ParseValue(std::string const &value, size_t from, ValueDelimiter end = kDelimiterNone)
{
    std::vector<Value::Entry> entries;

    size_t length;
    size_t search_offset = from;
    size_t append_offset = from;

    do {
        size_t to = 0;
        switch (end) {
            case kDelimiterNone: {
                to = value.size();
                break;
            }
            case kDelimiterParentheses: {
                to = value.find(')', search_offset);
                break;
            }
            case kDelimiterBraces: {
                to = value.find('}', search_offset);
                break;
            }
            case kDelimiterIdentifier: {
                const std::string identifier = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
                to = value.find_first_not_of(identifier, search_offset);
                if (to == std::string::npos) {
                    to = value.size();
                }
                if (to - search_offset == 0) {
                    to = std::string::npos;
                }
                break;
            }
        }
        if (to == std::string::npos) {
            return { false, from, Value(entries) };
        }

        size_t pno = value.find("$(", search_offset);
        size_t cbo = value.find("${", search_offset);
        size_t ido = value.find("$", search_offset);

        size_t openlen = 0;
        size_t open = std::string::npos;
        ValueDelimiter start = kDelimiterNone;
        size_t closelen = 0;
        if (pno != std::string::npos && (pno <= ido || ido == std::string::npos) && (pno < cbo || cbo == std::string::npos)) {
            open = pno;
            openlen = 2;
            start = kDelimiterParentheses;
            closelen = 1;
        } else if (cbo != std::string::npos && (cbo <= ido || ido == std::string::npos) && (cbo < pno || pno == std::string::npos)) {
            open = cbo;
            openlen = 2;
            start = kDelimiterBraces;
            closelen = 1;
        } else if (ido != std::string::npos && (ido < pno || pno == std::string::npos) && (ido < cbo || cbo == std::string::npos)) {
            open = ido;
            openlen = 1;
            start = kDelimiterIdentifier;
            closelen = 0;
        }

        if (open == std::string::npos || start == kDelimiterNone || open >= to) {
            length = to - append_offset;
            if (length > 0) {
                entries.push_back(Value::Entry(value.substr(append_offset, length)));
            }

            return { true, to, Value(entries) };
        }

        ParseResult result = ParseValue(value, open + openlen, start);
        if (result.found) {
            length = open - append_offset;
            if (length > 0) {
                entries.push_back(Value::Entry(value.substr(append_offset, length)));
            }

            entries.push_back(Value::Entry(std::make_shared<Value>(result.value)));

            append_offset = result.end + closelen;
            search_offset = result.end + closelen;
        } else {
            search_offset += openlen;
        }
    } while (true);
}

Value Value::
Parse(std::string const &value)
{
    return ParseValue(value, 0).value;
}

Value Value::
String(std::string const &value)
{
    if (value.empty()) {
        return Empty();
    }

    return Value({
        Entry(value),
    });
}

Value Value::
Variable(std::string const &value)
{
    return Value({
        Entry(std::make_shared<Value>(Value({
            Entry(value),
        }))),
    });
}

Value Value::
FromObject(plist::Object const *object)
{
    if (object == nullptr) {
        return pbxsetting::Value::Empty();
    } else if (plist::String const *stringValue = plist::CastTo <plist::String> (object)) {
        return pbxsetting::Value::Parse(stringValue->value());
    } else if (plist::Boolean const *booleanValue = plist::CastTo <plist::Boolean> (object)) {
        return pbxsetting::Value::String(Type::FormatBoolean(booleanValue->value()));
    } else if (plist::Integer const *integerValue = plist::CastTo <plist::Integer> (object)) {
        return pbxsetting::Value::String(Type::FormatInteger(integerValue->value()));
    } else if (plist::Real const *realValue = plist::CastTo <plist::Real> (object)) {
        return pbxsetting::Value::String(Type::FormatReal(realValue->value()));
    } else if (plist::Array const *arrayValue = plist::CastTo <plist::Array> (object)) {
        std::vector<std::string> values;
        for (size_t n = 0; n < arrayValue->count(); n++) {
            if (auto arg = arrayValue->value <plist::String> (n)) {
                std::vector<std::string> parsed = Type::ParseList(arg->value());
                values.insert(values.end(), parsed.begin(), parsed.end());
            }
        }
        return pbxsetting::Value::Parse(Type::FormatList(values));
    } else {
        // TODO(grp): Handle additional types?
        fprintf(stderr, "Warning: Unknown value type for object.\n");
        return pbxsetting::Value::Empty();
    }
}

Value const &Value::
Empty(void)
{
    static Value *value = new Value({ });
    return *value;
}

