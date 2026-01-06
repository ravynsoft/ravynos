/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/XMLWriter.h>
#include <plist/Objects.h>

#include <cassert>
#include <cinttypes>

using plist::Format::XMLWriter;
using plist::Object;
using plist::String;
using plist::Integer;
using plist::Real;
using plist::Boolean;
using plist::Data;
using plist::Date;
using plist::UID;
using plist::Array;
using plist::Dictionary;
using plist::CastTo;

XMLWriter::
XMLWriter(Object const *root) :
    _root   (root),
    _indent (0)
{
}

XMLWriter::
~XMLWriter()
{
}

bool XMLWriter::
write()
{
    if (!writeString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", false)) {
        return false;
    }

    if (!writeString("<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n", false)) {
        return false;
    }

    if (!writeString("<plist version=\"1.0\">\n", false)) {
        return false;
    }

    if (!handleObject(_root)) {
        return false;
    }

    if (!writeString("\n</plist>\n", false)) {
        return false;
    }

    return true;
}

/*
 * Low level functions.
 */

bool XMLWriter::
primitiveWriteString(std::string const &string)
{
    _contents.insert(_contents.end(), string.begin(), string.end());
    return true;
}

bool XMLWriter::
primitiveWriteEscapedString(std::string const &string)
{
    _contents.reserve(_contents.size() + string.size());

    for (char c : string) {
        switch (c) {
            case '<':
                if (!primitiveWriteString("&lt;")) {
                    return false;
                }
                break;
            case '>':
                if (!primitiveWriteString("&gt;")) {
                    return false;
                }
                break;
            case '&':
                if (!primitiveWriteString("&amp;")) {
                    return false;
                }
                break;
            default:
                _contents.push_back(c);
                break;
        }
    }

    return true;
}

bool XMLWriter::
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

bool XMLWriter::
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

bool XMLWriter::
handleObject(Object const *object)
{
    if (Dictionary const *dictionary = CastTo<Dictionary>(object)) {
        if (!handleDictionary(dictionary)) {
            return false;
        }
    } else if (Array const *array = CastTo<Array>(object)) {
        if (!handleArray(array)) {
            return false;
        }
    } else if (Boolean const *boolean = CastTo<Boolean>(object)) {
        if (!handleBoolean(boolean)) {
            return false;
        }
    } else if (Integer const *integer = CastTo<Integer>(object)) {
        if (!handleInteger(integer)) {
            return false;
        }
    } else if (Real const *real = CastTo<Real>(object)) {
        if (!handleReal(real)) {
            return false;
        }
    } else if (String const *string = CastTo<String>(object)) {
        if (!handleString(string)) {
            return false;
        }
    } else if (Data const *data = CastTo<Data>(object)) {
        if (!handleData(data)) {
            return false;
        }
    } else if (Date const *date = CastTo<Date>(object)) {
        if (!handleDate(date)) {
            return false;
        }
    } else if (UID const *uid = CastTo<UID>(object)) {
        if (!handleUID(uid)) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

bool XMLWriter::
handleDictionary(Dictionary const *dictionary)
{
    /* Write '<dict>'. */
    if (!writeString("<dict>", true)) {
        return false;
    }

    _indent++;

    for (size_t i = 0; i < dictionary->count(); ++i) {
        if (!writeString("\n", false)) {
            return false;
        }

        /* Write '<key>string</key>'. */
        if (!writeString("<key>", true)) {
            return false;
        }

        if (!writeEscapedString(dictionary->key(i), false)) {
            return false;
        }

        if (!writeString("</key>", false)) {
            return false;
        }

        if (!writeString("\n", false)) {
            return false;
        }

        if (!handleObject(dictionary->value(i))) {
            return false;
        }
    }

    /* Write '</dict>'. */
    if (!writeString("\n", false)) {
        return false;
    }

    _indent--;
    return writeString("</dict>", true);
}

bool XMLWriter::
handleArray(Array const *array)
{
    /* Write '<array>'. */
    if (!writeString("<array>", true)) {
        return false;
    }

    _indent++;

    for (size_t i = 0; i < array->count(); ++i) {
        if (!writeString("\n", false)) {
            return false;
        }

        if (!handleObject(array->value(i))) {
            return false;
        }
    }

    /* Write '</array>'. */
    if (!writeString("\n", false)) {
        return false;
    }

    _indent--;
    return writeString("</array>", true);
}

bool XMLWriter::
handleBoolean(Boolean const *boolean)
{
    if (!writeString(boolean->value() ? "<true />" : "<false />", true)) {
        return false;
    }

    return true;
}

bool XMLWriter::
handleString(String const *string)
{
    /* Write '<string>string</string>'. */
    if (!writeString("<string>", true)) {
        return false;
    }

    if (!writeEscapedString(string->value(), false)) {
        return false;
    }

    return writeString("</string>", false);
}

bool XMLWriter::
handleData(Data const *data)
{
    if (!writeString("<data>", true)) {
        return false;
    }

    if (!writeString(data->base64Value(), false)) {
        return false;
    }

    return writeString("</data>", false);
}

bool XMLWriter::
handleReal(Real const *real)
{
    int               rc;
    char              buf[64];

    rc = snprintf(buf, sizeof(buf), "%.17g", real->value());
    assert(rc < (int)sizeof(buf));
    (void)rc;

    /* Write '<real>number</real>'. */
    if (!writeString("<real>", true)) {
        return false;
    }

    if (!writeString(buf, false)) {
        return false;
    }

    return writeString("</real>", false);
}

bool XMLWriter::
handleInteger(Integer const *integer)
{
    int               rc;
    char              buf[32];

    rc = snprintf(buf, sizeof(buf), "%" PRId64, integer->value());
    assert(rc < (int)sizeof(buf));
    (void)rc;

    /* Write '<integer>number</integer>'. */
    if (!writeString("<integer>", true)) {
        return false;
    }

    if (!writeString(buf, false)) {
        return false;
    }

    return writeString("</integer>", false);
}

bool XMLWriter::
handleDate(Date const *date)
{
    /* Write '<date>YYYY-MM-DDTHH:MM:SSZ+ZZZZ</date>'. */
    if (!writeString("<date>", true)) {
        return false;
    }

    if (!writeString(date->stringValue(), false)) {
        return false;
    }

    return writeString("</date>", false);
}

bool XMLWriter::
handleUID(UID const *uid)
{
    /* Write a '<dict><key>CF$UID</key><integer>uid</integer></dict>'. */
    std::unique_ptr<Dictionary> dictionary = Dictionary::New();
    dictionary->set("CF$UID", Integer::New(uid->value()));
    return handleDictionary(dictionary.get());
}

