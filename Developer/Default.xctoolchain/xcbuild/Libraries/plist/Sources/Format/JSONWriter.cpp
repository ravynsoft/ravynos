/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/JSONWriter.h>
#include <plist/Objects.h>

#include <cassert>
#include <cinttypes>

using plist::Format::JSONWriter;
using plist::Object;
using plist::String;
using plist::Integer;
using plist::Real;
using plist::Boolean;
using plist::Data;
using plist::Date;
using plist::Array;
using plist::UID;
using plist::Dictionary;
using plist::CastTo;

JSONWriter::
JSONWriter(Object const *root) :
    _root   (root),
    _indent (0),
    _lastKey(false)
{
}

JSONWriter::
~JSONWriter()
{
}

bool JSONWriter::
write()
{
    if (!handleObject(_root, true)) {
        return false;
    }

    return true;
}

/*
 * Low level functions.
 */

bool JSONWriter::
primitiveWriteString(std::string const &string)
{
    _contents.insert(_contents.end(), string.begin(), string.end());
    return true;
}

bool JSONWriter::
primitiveWriteEscapedString(std::string const &string)
{
    _contents.reserve(_contents.size() + string.size());

    if (!primitiveWriteString("\"")) {
        return false;
    }

    for (char c : string) {
        if (c < 0x20) {
            char buf[64];
            int rc = snprintf(buf, sizeof(buf), "\\%04x", c);
            assert(rc < (int)sizeof(buf));
            (void)rc;

            if (!primitiveWriteString(buf)) {
                return false;
            }
        } else {
            switch (c) {
                case '"':  if (!primitiveWriteString("\\\"")) { return false; } break;
                case '\\': if (!primitiveWriteString("\\"))   { return false; } break;
                default: _contents.push_back(c); break;
            }
        }
    }

    if (!primitiveWriteString("\"")) {
        return false;
    }

    return true;
}

bool JSONWriter::
writeString(std::string const &string, bool final)
{
    if (final) {
        for (int n = 0; n < _indent; n++) {
            if (!primitiveWriteString("\t")) {
                return false;
            }
        }
    }

    return primitiveWriteString(string);
}

bool JSONWriter::
writeEscapedString(std::string const &string, bool final)
{
    if (final) {
        for (int n = 0; n < _indent; n++) {
            if (!primitiveWriteString("\t")) {
                return false;
            }
        }
    }

    return primitiveWriteEscapedString(string);
}

/*
 * Higher level functions.
 */

bool JSONWriter::
handleObject(Object const *object, bool root)
{
    if (Dictionary const *dictionary = CastTo<Dictionary>(object)) {
        if (!handleDictionary(dictionary, root)) {
            return false;
        }
    } else if (Array const *array = CastTo<Array>(object)) {
        if (!handleArray(array, root)) {
            return false;
        }
    } else if (Boolean const *boolean = CastTo<Boolean>(object)) {
        if (!handleBoolean(boolean, root)) {
            return false;
        }
    } else if (Integer const *integer = CastTo<Integer>(object)) {
        if (!handleInteger(integer, root)) {
            return false;
        }
    } else if (Real const *real = CastTo<Real>(object)) {
        if (!handleReal(real, root)) {
            return false;
        }
    } else if (String const *string = CastTo<String>(object)) {
        if (!handleString(string, root)) {
            return false;
        }
    } else if (Data const *data = CastTo<Data>(object)) {
        if (!handleData(data, root)) {
            return false;
        }
    } else if (Date const *date = CastTo<Date>(object)) {
        if (!handleDate(date, root)) {
            return false;
        }
    } else if (UID const *uid = CastTo<UID>(object)) {
        if (!handleUID(uid, root)) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

bool JSONWriter::
handleDictionary(Dictionary const *dictionary, bool root)
{
    /* Write '{'. */
    if (!writeString("{\n", !_lastKey)) {
        return false;
    }

    _indent++;

    _lastKey = false;

    for (size_t i = 0; i < dictionary->count(); ++i) {
        /* Write ',' if not first entry. */
        if (i != 0) {
            if (!writeString(",\n", false)) {
                return false;
            }
        }

        _lastKey = false;

        if (!writeEscapedString(dictionary->key(i), !_lastKey)) {
            return false;
        }

        if (!writeString(": ", false)) {
            return false;
        }

        _lastKey = true;

        if (!handleObject(dictionary->value(i), false)) {
            return false;
        }
    }

    /* Write '}'. */
    if (!writeString("\n", false)) {
        return false;
    }

    _indent--;
    if (!writeString("}", true)) {
        return false;
    }

    return true;
}

bool JSONWriter::
handleArray(Array const *array, bool root)
{
    /* Write '['. */
    if (!writeString("[\n", !_lastKey)) {
        return false;
    }

    _lastKey = false;

    _indent++;

    for (size_t i = 0; i < array->count(); ++i) {
        /* Write ',' if not first entry. */
        if (i != 0) {
            if (!writeString(",\n", false)) {
                return false;
            }
        }

        if (!handleObject(array->value(i), false)) {
            return false;
        }
    }

    /* Write ']'. */
    if (!writeString("\n", false)) {
        return false;
    }

    _indent--;
    return writeString("]", true);
}

bool JSONWriter::
handleBoolean(Boolean const *boolean, bool root)
{
    if (!writeString(boolean->value() ? "true" : "false", !_lastKey)) {
        return false;
    }

    _lastKey = false;
    return true;
}

bool JSONWriter::
handleString(String const *string, bool root)
{
    if (!writeEscapedString(string->value(), !_lastKey)) {
        return false;
    }

    _lastKey = false;
    return true;
}

bool JSONWriter::
handleData(Data const *data, bool root)
{
    if (!writeString("\"", !_lastKey)) {
        return false;
    }

    _lastKey = false;

    std::vector<uint8_t> const &value = data->value();
    for (auto it : value) {
        char buf[3];
        int rc = snprintf(buf, sizeof(buf), "%02x", it);
        assert(rc < (int)sizeof(buf));
        (void)rc;

        if (!writeString(buf, false)) {
            return false;
        }
    }

    return writeString("\"", false);
}

bool JSONWriter::
handleReal(Real const *real, bool root)
{
    char buf[64];
    int rc = snprintf(buf, sizeof(buf), "%g", real->value());
    assert(rc < (int)sizeof(buf));
    (void)rc;

    if (!writeString(buf, !_lastKey)) {
        return false;
    }

    _lastKey = false;

    return true;
}

bool JSONWriter::
handleInteger(Integer const *integer, bool root)
{
    int               rc;
    char              buf[32];

    rc = snprintf(buf, sizeof(buf), "%" PRId64, integer->value());
    assert(rc < (int)sizeof(buf));
    (void)rc;

    if (!writeString(buf, !_lastKey)) {
        return false;
    }

    _lastKey = false;

    return true;
}

bool JSONWriter::
handleDate(Date const *date, bool root)
{
    if (!writeEscapedString(date->stringValue(), !_lastKey)) {
        return false;
    }

    _lastKey = false;
    return true;
}

bool JSONWriter::
handleUID(UID const *uid, bool root)
{
    /* Write a CF$UID dictionary. */
    std::unique_ptr<Dictionary> dictionary = Dictionary::New();
    dictionary->set("CF$UID", Integer::New(uid->value()));
    return handleDictionary(dictionary.get(), root);
}
