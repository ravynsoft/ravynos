/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_JSONParser_h
#define __plist_Format_JSONParser_h

#include <plist/Format/ASCIIPListLexer.h>
#include <plist/Object.h>
#include <plist/String.h>

#include <stack>
#include <string>

namespace plist {
namespace Format {

struct JSONParser {
private:
    enum class ContextState {
        Parsing = 0,
        Done,
        Aborted,
    };

private:
    enum class ValueState {
        Init = 0,
        Dictionary,
        DictionaryValue,
        Array,
    };

private:
    std::unique_ptr<plist::Object> _root;
    int                            _level;

private:
    ValueState                     _state;
    std::stack<ValueState>         _stateStack;

private:
    std::unique_ptr<plist::Object> _container;
    std::stack<std::unique_ptr<plist::Object>> _containerStack;

private:
    std::unique_ptr<plist::String> _key;
    std::stack<std::unique_ptr<plist::String>> _keyStack;

private:
    ContextState                _contextState;
    std::string                 _error;

public:
    JSONParser();
    ~JSONParser();

public:
    bool parse(ASCIIPListLexer *lexer);

public:
    std::unique_ptr<plist::Object> &root()
    { return _root; }
    std::string error() const
    { return _error; }

private:
    bool isAborted() const;
    void abort(std::string const &error);

private:
    bool isDone() const;
    bool finish();

private:
    int getLevel() const;
    void incrementLevel();
    void decrementLevel();

private:
    bool push(ValueState state, std::unique_ptr<plist::Object> container, std::unique_ptr<plist::String> key);
    bool pop();

private:
    bool beginContainer(std::unique_ptr<plist::Object> container);
    bool storeKeyValue(std::unique_ptr<plist::String> key, std::unique_ptr<plist::Object> value);
    bool endContainer(bool isArray);

private:
    bool isArray() const;
    bool beginArray();
    bool endArray();

private:
    bool isDictionary() const;
    bool beginDictionary();
    bool endDictionary();

private:
    bool storeKey(std::unique_ptr<plist::String> key);
    bool storeValue(std::unique_ptr<plist::Object> value);
};

}
}

#endif  // !__plist_Format_JSONParser_h
