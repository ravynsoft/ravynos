/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_XMLParser_h
#define __plist_Format_XMLParser_h

#include <plist/Format/BaseXMLParser.h>
#include <plist/Object.h>

namespace plist {
namespace Format {

class XMLParser : public BaseXMLParser {
private:
    struct Key {
        std::string value;
        bool        valid;
        bool        active;
    };

    struct State {
        typedef std::vector <State> vector;

        Object *current;
        Key     key;
    };

private:
    Object        *_root;
    State::vector  _stack;
    State          _state;
    std::string    _cdata;

public:
    XMLParser();

public:
    Object *parse(std::vector<uint8_t> const &contents);

private:
    virtual void onBeginParse();
    virtual void onEndParse(bool success);

private:
    void onStartElement(std::string const &name, std::unordered_map<std::string, std::string> const &attrs, size_t depth);
    void onEndElement(std::string const &name, size_t depth);
    void onCharacterData(std::string const &cdata, size_t depth);

private:
    void push(Object *object);
    void pop();

private:
    inline bool inArray() const;
    inline bool inDictionary() const;
    inline bool inContainer(size_t depth) const;
    inline bool isExpectingKey() const;
    inline bool isExpectingCDATA() const;

private:
    bool beginObject(std::string const &name, size_t depth);
    bool beginArray();
    bool beginDictionary();
    bool beginString();
    bool beginInteger();
    bool beginReal();
    bool beginBoolean(bool value);
    bool beginNull();
    bool beginData();
    bool beginDate();
    bool beginKey();

private:
    bool endObject(std::string const &name);
    bool endArray();
    bool endDictionary();
    bool endString();
    bool endInteger();
    bool endReal();
    bool endBoolean();
    bool endNull();
    bool endData();
    bool endDate();
    bool endKey();
};

}
}

#endif  // !__plist_Format_XMLParser_h
