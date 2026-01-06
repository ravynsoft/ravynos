/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/XMLParser.h>

#include <plist/Objects.h>

using plist::Format::XMLParser;
using plist::Object;

XMLParser::XMLParser() :
    BaseXMLParser(),
    _root        (nullptr)
{
}

Object *XMLParser::
parse(std::vector<uint8_t> const &contents)
{
    if (_root != nullptr)
        return nullptr;

    if (!BaseXMLParser::parse(contents))
        return nullptr;

    return _root;
}

void XMLParser::
onBeginParse()
{
    _root             = nullptr;
    _state.current    = nullptr;
    _state.key.valid  = false;
    _state.key.active = false;
}

void XMLParser::
onEndParse(bool success)
{
    if (!success) {
        if (_state.current != nullptr && _state.current != _root) {
            _state.current->release();
        }
        for (auto const &s : _stack) {
            if (s.current != _state.current && s.current != _root) {
                s.current->release();
            }
        }
        _stack.clear();
        if (_root != nullptr) {
            _root->release();
            _root = nullptr;
        }
    }
    _state.current    = nullptr;
    _state.key.valid  = false;
    _state.key.active = false;
    _cdata.clear();
}

void XMLParser::
onStartElement(std::string const &name, std::unordered_map<std::string, std::string> const &attrs, size_t depth)
{
    if (depth == 0) {
        if (name != "plist") {
            error("expecting 'plist', found '%s'", name.c_str());
        }
        return;
    }

    //
    // If we have a root, and depth == 1 there's an extra
    // entry after the first element, bail out.
    //
    if (depth == 1 && _root != nullptr) {
        error("unexpected element '%s' after root element", name.c_str());
        return;
    }

    if (!beginObject(name, depth)) {
        return;
    }
}

void XMLParser::
onEndElement(std::string const &name, size_t)
{
    if (!endObject(name)) {
        error("unexpected end element: " + name);
    }
}

void XMLParser::
onCharacterData(std::string const &cdata, size_t)
{
    if (!isExpectingCDATA()) {
        for (size_t n = 0; n < cdata.size(); n++) {
            if (!isspace(cdata[n])) {
                error("unexpected cdata: " + cdata);
            }
        }
        return;
    }

    _cdata += cdata;
}

inline bool XMLParser::
inContainer(size_t depth) const
{
    return (depth == 1 || inDictionary() || inArray());
}

inline bool XMLParser::
inArray() const
{
    return (CastTo <Array> (_state.current) != nullptr);
}

inline bool XMLParser::
inDictionary() const
{
    return (CastTo <Dictionary> (_state.current) != nullptr);
}

inline bool XMLParser::
isExpectingKey() const
{
    return (inDictionary() && !_state.key.valid);
}

inline bool XMLParser::
isExpectingCDATA() const
{
    return (CastTo <Integer> (_state.current) != nullptr ||
            CastTo <Real> (_state.current) != nullptr ||
            CastTo <String> (_state.current) != nullptr ||
            CastTo <Data> (_state.current) != nullptr ||
            CastTo <Date> (_state.current) != nullptr ||
            (inDictionary() && _state.key.active));
}

bool XMLParser::
beginObject(std::string const &name, size_t depth)
{
    if (inDictionary()) {
        if (name == "key") {
            if (!isExpectingKey()) {
                error("unexpected 'key' when expecting value "
                        "in dictionary definition");
                return false;
            }

            return beginKey();
        } else if (isExpectingKey()) {
            error("unexpected element '%s' when a key "
                    "was expected in dictionary definition",
                    name.c_str());
            return false;
        }
    }

    if (!inContainer(depth)) {
        error("unexpected '%s' element in a non-container element.",
                name.c_str());
        return false;
    }


    if (name == "array") {
        return beginArray();
    } else if (name == "dict") {
        return beginDictionary();
    } else if (name == "string") {
        return beginString();
    } else if (name == "integer") {
        return beginInteger();
    } else if (name == "real") {
        return beginReal();
    } else if (name == "true") {
        return beginBoolean(true);
    } else if (name == "false") {
        return beginBoolean(false);
    } else if (name == "null") {
        return beginNull();
    } else if (name == "data") {
        return beginData();
    } else if (name == "date") {
        return beginDate();
    }

    error("unexpected element '%s'", name.c_str());
    return false;
}

bool XMLParser::
endObject(std::string const &name)
{
    if (name == "plist") {
        return true;
    } if (name == "key") {
        return endKey();
    } else if (name == "array") {
        return endArray();
    } else if (name == "dict") {
        return endDictionary();
    } else if (name == "string") {
        return endString();
    } else if (name == "integer") {
        return endInteger();
    } else if (name == "real") {
        return endReal();
    } else if (name == "true" || name == "false") {
        return endBoolean();
    } else if (name == "null") {
        return endNull();
    } else if (name == "data") {
        return endData();
    } else if (name == "date") {
        return endDate();
    }

    error("unexpected element '%s'", name.c_str());
    return false;
}

void XMLParser::
push(Object *object)
{
    if (_state.current != nullptr) {
        _stack.push_back(_state);
    }
    _state.current    = object;
    _state.key.valid  = false;
    _state.key.active = false;
    if (_root == nullptr) {
        _root = _state.current;
    }
}

void XMLParser::
pop()
{
    if (_stack.empty() && _state.current == nullptr) {
        error("stack underflow");
        return;
    }

    if (_state.current != _root) {
        State old = _state;
        _state = _stack.back();
        _stack.pop_back();

        if (auto array = CastTo <Array> (_state.current)) {
            array->append(std::unique_ptr<Object>(old.current));
        } else if (auto dict = CastTo <Dictionary> (_state.current)) {
            if (!isExpectingKey()) {
                dict->set(_state.key.value, std::unique_ptr<Object>(old.current));
                _state.key.valid  = false;
                _state.key.active = false;
            }
        }
    }

    _cdata.clear();
}

bool XMLParser::
beginArray()
{
    push(Array::New().release());
    return true;
}

bool XMLParser::
endArray()
{
    pop();
    return true;
}

bool XMLParser::
beginDictionary()
{
    push(Dictionary::New().release());
    _state.key.valid  = false;
    _state.key.active = false;
    return true;
}

bool XMLParser::
endDictionary()
{
    if (auto dict = CastTo<Dictionary>(_state.current)) {
        /* Convert CF$UID dictionaries into UID objects. */
        if (dict->count() == 1) {
            std::string const &key = dict->key(0);
            Object const *value = dict->value(0);
            if (key == "CF$UID" && value->type() == Integer::Type()) {
                Integer const *integer = CastTo<Integer>(value);

                uint32_t value = static_cast<uint32_t>(integer->value());
                _state.current->release();
                _state.current = UID::New(value).release();
            }
        }
    }

    pop();
    return true;
}

bool XMLParser::
beginString()
{
    push(String::New().release());
    _cdata.clear();
    return true;
}

bool XMLParser::
endString()
{
    CastTo <String> (_state.current)->setValue(_cdata);
    pop();
    return true;
}

bool XMLParser::
beginInteger()
{
    push(Integer::New().release());
    _cdata.clear();
    return true;
}

bool XMLParser::
endInteger()
{
    char *end = NULL;
    long long integer = ::strtoll(_cdata.c_str(), &end, 0);
    if (end != _cdata.c_str()) {
        CastTo <Integer> (_state.current)->setValue(integer);
        pop();
        return true;
    } else {
        pop();
        return false;
    }
}

bool XMLParser::
beginReal()
{
    push(Real::New().release());
    _cdata.clear();
    return true;
}

bool XMLParser::
endReal()
{
    char *end = NULL;
    double real = ::strtod(_cdata.c_str(), &end);
    if (end != _cdata.c_str()) {
        CastTo <Real> (_state.current)->setValue(real);
        pop();
        return true;
    } else {
        pop();
        return false;
    }
}

bool XMLParser::
beginNull()
{
    push(Null::New().release());
    return true;
}

bool XMLParser::
endNull()
{
    pop();
    return true;
}

bool XMLParser::
beginBoolean(bool value)
{
    push(Boolean::New(value).release());
    return true;
}

bool XMLParser::
endBoolean()
{
    pop();
    return true;
}

bool XMLParser::
beginData()
{
    push(Data::New().release());
    _cdata.clear();
    return true;
}

bool XMLParser::
endData()
{
    CastTo <Data> (_state.current)->setBase64Value(_cdata);
    pop();
    return true;
}

bool XMLParser::
beginDate()
{
    push(Date::New().release());
    _cdata.clear();
    return true;
}

bool XMLParser::
endDate()
{
    CastTo <Date> (_state.current)->setStringValue(_cdata);
    pop();
    return true;
}

bool XMLParser::
beginKey()
{
    _state.key.valid = false;
    _state.key.active = true;
    _cdata.clear();
    return true;
}

bool XMLParser::
endKey()
{
    _state.key.active = false;
    _state.key.valid = true;
    _state.key.value = _cdata;
    _cdata.clear();
    return true;
}
