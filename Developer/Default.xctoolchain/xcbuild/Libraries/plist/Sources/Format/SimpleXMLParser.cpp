/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/SimpleXMLParser.h>

#include <plist/Objects.h>

using plist::Format::SimpleXMLParser;
using plist::Dictionary;

SimpleXMLParser::SimpleXMLParser() :
    BaseXMLParser(),
    _root          (nullptr),
    _current       (nullptr)
{
}

Dictionary *SimpleXMLParser::
parse(std::vector<uint8_t> const &contents)
{
    if (_root != nullptr)
        return nullptr;

    if (!BaseXMLParser::parse(contents))
        return nullptr;

    return _root;
}

void SimpleXMLParser::
onBeginParse()
{
    _root = Dictionary::New().release();
    _current = _root;
}

void SimpleXMLParser::
onEndParse(bool success)
{
    if (!success) {
        _root->release();
        _root = nullptr;
    }
    _current = nullptr;
}

void SimpleXMLParser::
onStartElement(std::string const &name, std::unordered_map<std::string, std::string> const &attrs, size_t)
{
    Dictionary *dict = Dictionary::New().release();

    for (auto const &I : attrs) {
        if (I.second == "YES") {
            dict->set(I.first, Boolean::New(true));
        } else if (I.second == "NO") {
            dict->set(I.first, Boolean::New(false));
        } else {
            dict->set(I.first, String::New(I.second));
        }
    }

    Object *old = const_cast <Object *> (_current->value(name));
    if (old != nullptr) {
        Array *array = CastTo <Array> (old);
        if (array == nullptr) {
            array = Array::New().release();
            array->append(old->copy());

            _current->set(name, std::unique_ptr<Object>(array));
        }
        array->append(std::unique_ptr<Object>(dict));
    } else {
        _current->set(name, std::unique_ptr<Object>(dict));
    }

    _stack.push_back(_current);
    _current = dict;
}

void SimpleXMLParser::
onEndElement(std::string const &name, size_t)
{
    _current = _stack.back();
    _stack.pop_back();
}
