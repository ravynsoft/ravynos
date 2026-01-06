/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/JSONParser.h>
#include <plist/Objects.h>

#include <cstdlib>

using plist::Format::JSONParser;
using plist::Object;
using plist::String;
using plist::Boolean;
using plist::Real;
using plist::Integer;
using plist::Array;
using plist::Dictionary;

#if 0
#define JSONDebug(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#else
#define JSONDebug(...)
#endif

JSONParser::
JSONParser() :
    _root(nullptr),
    _level(0),
    _state(ValueState::Init),
    _container(nullptr),
    _key(nullptr),
    _contextState(ContextState::Parsing)
{
}

JSONParser::
~JSONParser()
{
}

bool JSONParser::
isAborted() const
{
    return _contextState == ContextState::Aborted;
}

bool JSONParser::
isDone() const
{
    return _contextState == ContextState::Done;
}

int JSONParser::
getLevel() const
{
    return _level;
}

void JSONParser::
incrementLevel()
{
    _level++;
}

void JSONParser::
decrementLevel()
{
    if (_level != 0) {
        _level--;
    }
}

void JSONParser::
abort(std::string const &error)
{
    JSONDebug("abort: %s", error.c_str());

    _error = error;
    _contextState = ContextState::Aborted;
}

bool JSONParser::
push(ValueState state, std::unique_ptr<plist::Object> container, std::unique_ptr<plist::String> key)
{
    if (isAborted()) {
        return false;
    }

    /* If valid state, push, otherwise just set the new state. */
    if (_state != ValueState::Init) {
        /* Push the old state */
        _containerStack.push(std::move(_container));
        _keyStack.push(std::move(_key));
        _stateStack.push(_state);
    }

    _state = state;

    if (container != nullptr) {
        _container = std::move(container);
    } else {
        _container = nullptr;
    }

    if (key != nullptr) {
        _key = std::move(key);
    } else {
        _key = nullptr;
    }

    return true;
}

bool JSONParser::
pop()
{
    int count = _stateStack.size();

    if (count == 0) {
        if (_state == ValueState::Init)
            return false; /* Underflow! */

        /* Reset current state. */
        _container = nullptr;
        _key = nullptr;

        _state = ValueState::Init;
        return true;
    }

    /* Pop state */
    _state = std::move(_stateStack.top());
    _stateStack.pop();

    _container = std::move(_containerStack.top());
    _containerStack.pop();

    _key = std::move(_keyStack.top());
    _keyStack.pop();

    return true;
}

/*
 * Generic container handling.
 */
bool JSONParser::
beginContainer(std::unique_ptr<plist::Object> object)
{
    ValueState state;

    if (object->type() == Array::Type()) {
        state = ValueState::Array;
    } else {
        state = ValueState::Dictionary;
    }

    if (!push(state, std::move(object), nullptr)) {
        abort("Cannot push the current state.");
        return false;
    }

    return true;
}

bool JSONParser::
storeKeyValue(std::unique_ptr<plist::String> key, std::unique_ptr<plist::Object> value)
{
    if (_container == nullptr) {
        if (_key != nullptr) {
            abort("Storing key/value pair with no container.");
            return false;
        }

        /* Make current value the root. */
        if (_root != nullptr) {
            abort("Double root.");
            return false;
        }

        _root = std::move(value);
        return true;
    }

    if (key != nullptr) {
        if (_container->type() != Dictionary::Type()) {
            abort("Storing key/value with no dictionary container.");
            return false;
        }

        plist::Dictionary *dict = static_cast<plist::Dictionary *>(_container.get());
        dict->set(key->value(), std::move(value));

        _state = ValueState::Dictionary;
    } else {
        if (_container->type() != Array::Type()) {
            abort("Storing value with no array container.");
            return false;
        }

        plist::Array *array = static_cast<plist::Array *>(_container.get());
        array->append(std::move(value));

        _state = ValueState::Array;
    }

    return true;
}

bool JSONParser::
endContainer(bool isArray)
{
    std::unique_ptr<plist::String> key;
    std::unique_ptr<plist::Object> value;
    bool success;

    /* Check state is consistant. */
    if (_state != (isArray ? ValueState::Array : ValueState::Dictionary)) {
        abort("Closing array/dictionary in wrong state.");
        return false;
    }

    if (_container == nullptr) {
        abort("Closing non-opened container.");
        return false;
    }

    value = std::move(_container);
    if (value == nullptr) {
        abort("Couldn't copy container.");
        return false;
    }

    /* Is this the right type of container? */
    if (value->type() != (isArray ? Array::Type() : Dictionary::Type())) {
        abort("Closing wrong kind of container.");
        return false;
    }

    if (!pop()) {
        abort("Parser stack underflow.");
        return false;
    }

    /* Take ownership of the saved key. */
    key = std::move(_key);
    _key = nullptr;

    success = storeKeyValue(std::move(key), std::move(value));

    return success;
}

bool JSONParser::
beginArray()
{
    std::unique_ptr<plist::Array> array = Array::New();
    if (array == nullptr) {
        abort("Cannot create an array.");
        return false;
    }

    bool success = beginContainer(std::move(array));
    return success;
}

bool JSONParser::
endArray()
{
    return endContainer(true);
}

bool JSONParser::
beginDictionary()
{
    std::unique_ptr<plist::Dictionary> dict = Dictionary::New();
    if (dict == nullptr) {
        abort("Cannot create a dictionary.");
        return false;
    }

    bool success = beginContainer(std::move(dict));
    return success;
}

bool JSONParser::
endDictionary()
{
    return endContainer(false);
}

/*
 * Store the key of the current dictionary.
 */
bool JSONParser::
storeKey(std::unique_ptr<plist::String> key)
{
    if (key == nullptr) {
        key = String::New();

        if (key == nullptr) {
            abort("Cannot create empty key.");
            return false;
        }
    }

    if (_state != ValueState::Dictionary) {
        abort("Storing key in wrong state.");
        return false;
    }

    _key = std::move(key);

    _state = ValueState::DictionaryValue;
    return true;
}

bool JSONParser::
storeValue(std::unique_ptr<plist::Object> value)
{
    bool success;

    success = storeKeyValue(std::move(_key), std::move(value));
    _key = nullptr;

    if (_state == ValueState::DictionaryValue) {
        _state = ValueState::Dictionary;
    }

    return success;
}

bool JSONParser::
isDictionary() const
{
    return _state == ValueState::Dictionary;
}

bool JSONParser::
isArray() const
{
    return _state == ValueState::Array;
}

bool JSONParser::
finish()
{
    if (isDone()) {
        return true;
    }

    _contextState = ContextState::Done;
    return true;
}

bool JSONParser::
parse(ASCIIPListLexer *lexer)
{
    enum class JSONParseState {
        Parse,
        KeyValueSeparator,
        EntrySeparator,
    };

    int token;
    JSONParseState state = JSONParseState::Parse;

    for (;;) {
        token = ASCIIPListLexerReadToken(lexer);
        if (token < 0) {
            if (token == kASCIIPListLexerEndOfFile && isDone()) {
                /* success */
                return true;
            } else if (token == kASCIIPListLexerEndOfFile) {
                abort("Encountered premature EOF");
                return false;
            } else if (token == kASCIIPListLexerInvalidToken) {
                abort("Encountered invalid token");
                return false;
            } else if (token == kASCIIPListLexerUnterminatedQuotedString) {
                abort("Encountered unterminated quoted string");
                return false;
            } else {
                abort("Encountered unrecognized token error code");
                return false;
            }
        }

        switch (state) {
            case JSONParseState::Parse:
                if (isDone()) {
                    abort("Encountered token when finished.");
                    return false;
                }

                if (token == kASCIIPListLexerTokenDictionaryStart) {
                    JSONDebug("Starting dictionary");
                    if (!beginDictionary()) {
                        return false;
                    }
                    incrementLevel();
                    state = JSONParseState::Parse;
                } else if (token == kASCIIPListLexerTokenArrayStart) {
                    JSONDebug("Starting array");
                    if (!beginArray()) {
                        return false;
                    }
                    incrementLevel();
                    state = JSONParseState::Parse;
                } else if (token == kASCIIPListLexerTokenDictionaryEnd) {
                    JSONDebug("Ending dictionary");
                    if (!endDictionary()) {
                        return false;
                    }
                    decrementLevel();
                    if (getLevel()) {
                        state = JSONParseState::EntrySeparator;
                    } else {
                        if (!finish()) {
                            return false;
                        }
                        state = JSONParseState::Parse;
                    }
                } else if (token == kASCIIPListLexerTokenArrayEnd) {
                    JSONDebug("Ending array");
                    if (!endArray()) {
                        return false;
                    }
                    decrementLevel();
                    if (getLevel()) {
                        state = JSONParseState::EntrySeparator;
                    } else {
                        if (!finish()) {
                            return false;
                        }
                        state = JSONParseState::Parse;
                    }
                } else {
                    /* Read all non-container tokens */
                    bool topLevel     = getLevel() == 0;
                    bool isDictionary = this->isDictionary();

                    if (token == kASCIIPListLexerTokenBoolTrue ||
                        token == kASCIIPListLexerTokenBoolFalse) {
                        if (isDictionary) {
                            abort("Boolean cannot be dictionary key");
                            return false;
                        }

                        bool value = (token == kASCIIPListLexerTokenBoolTrue);
                        std::unique_ptr<Boolean> boolean = Boolean::New(value);

                        JSONDebug("Storing boolean");
                        if (!storeValue(std::move(boolean))) {
                            return false;
                        }
                    } else if (token == kASCIIPListLexerTokenNull) {
                        if (isDictionary) {
                            abort("Null cannot be dictionary key");
                            return false;
                        }

                        std::unique_ptr<Null> null = Null::New();

                        JSONDebug("Storing null");
                        if (!storeValue(std::move(null))) {
                            return false;
                        }
                    } else if (token == kASCIIPListLexerTokenNumberInteger) {
                        if (isDictionary) {
                            abort("Integer cannot be dictionary key");
                            return false;
                        }

                        char *end = NULL;
                        char *contents = ASCIIPListCopyUnquotedString(lexer, '?');
                        long long value = ::strtoll(contents, &end, 0);
                        bool success = (end != contents);
                        free(contents);

                        if (success) {
                            std::unique_ptr<Integer> integer = Integer::New(value);

                            JSONDebug("Storing integer");
                            if (!storeValue(std::move(integer))) {
                                return false;
                            }
                        } else {
                            abort("Unable to parse integer");
                            return false;
                        }
                    } else if (token == kASCIIPListLexerTokenNumberReal) {
                        if (isDictionary) {
                            abort("Real cannot be dictionary key");
                            return false;
                        }

                        char *end = NULL;
                        char *contents = ASCIIPListCopyUnquotedString(lexer, '?');
                        double value = ::strtod(contents, &end);
                        bool success = (end != contents);
                        free(contents);

                        if (success) {
                            std::unique_ptr<Real> real = Real::New(value);

                            JSONDebug("Storing real");
                            if (!storeValue(std::move(real))) {
                                return false;
                            }
                        } else {
                            abort("Unable to parse real");
                            return false;
                        }
                    } else if (token == kASCIIPListLexerTokenQuotedString) {
                        char *contents = ASCIIPListCopyUnquotedString(lexer, '?');
                        std::unique_ptr<String> string = String::New(std::string(contents));
                        free(contents);

                        if (string == NULL) {
                            abort("OOM when copying string");
                            return false;
                        }

                        /* Container context */
                        if (isDictionary) {
                            JSONDebug("Storing string %s as key", string->value().c_str());
                            if (!storeKey(std::move(string))) {
                                return false;
                            }
                        } else {
                            JSONDebug("Storing string %s", string->value().c_str());
                            if (!storeValue(std::move(string))) {
                                return false;
                            }
                        }
                    } else {
                        abort("Unknown token found: " + std::to_string(token));
                        return false;
                    }

                    if (topLevel) {
                        if (!finish()) {
                            return false;
                        }
                        state = JSONParseState::Parse;
                    } else {
                        if (isDictionary) {
                            state = JSONParseState::KeyValueSeparator;
                        } else {
                            state = JSONParseState::EntrySeparator;
                        }
                    }
                }
                break;

            case JSONParseState::KeyValueSeparator:
                if (token != kASCIIPListLexerTokenDictionaryKeyValSeparator) {
                    abort("Expected key-value separator; found something else");
                    return false;
                }
                JSONDebug("Found keyval separator");
                state = JSONParseState::Parse;
                break;

            case JSONParseState::EntrySeparator:
                if (token == kASCIIPListLexerTokenDictionaryEnd) {
                    JSONDebug("Ending dictionary");
                    if (!endDictionary()) {
                        return false;
                    }
                    decrementLevel();
                    if (getLevel()) {
                        state = JSONParseState::EntrySeparator;
                    } else {
                        if (!finish()) {
                            return false;
                        }
                        state = JSONParseState::Parse;
                    }
                } else if (token == kASCIIPListLexerTokenArrayEnd) {
                    JSONDebug("Ending array");
                    if (!endArray()) {
                        return false;
                    }
                    decrementLevel();
                    if (getLevel()) {
                        state = JSONParseState::EntrySeparator;
                    } else {
                        if (!finish()) {
                            return false;
                        }
                        state = JSONParseState::Parse;
                    }
                } else if (token == ',') {
                    state = JSONParseState::Parse;
                } else {
                    abort("Unexpected token");
                    return false;
                }
                break;
            default:
                abort("Unexpected state");
                return false;
        }
    }
}

