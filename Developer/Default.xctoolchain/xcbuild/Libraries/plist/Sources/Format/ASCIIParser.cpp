/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/ASCIIParser.h>
#include <plist/Objects.h>

#include <cstdlib>

using plist::Format::ASCIIParser;
using plist::Object;
using plist::String;
using plist::Data;
using plist::Array;
using plist::Dictionary;

ASCIIParser::
ASCIIParser() :
    _root(nullptr),
    _level(0),
    _state(ValueState::Init),
    _container(nullptr),
    _key(nullptr),
    _contextState(ContextState::Parsing)
{
}

ASCIIParser::
~ASCIIParser()
{
}

bool ASCIIParser::
isAborted() const
{
    return _contextState == ContextState::Aborted;
}

bool ASCIIParser::
isDone() const
{
    return _contextState == ContextState::Done;
}

int ASCIIParser::
getLevel() const
{
    return _level;
}

void ASCIIParser::
incrementLevel()
{
    _level++;
}

void ASCIIParser::
decrementLevel()
{
    if (_level != 0) {
        _level--;
    }
}

void ASCIIParser::
abort(std::string const &error, int line)
{
    _error = error;
    if (line != -1) {
        _error = "[line " + std::to_string(line) + "] " + _error;
    }
    _contextState = ContextState::Aborted;
}

bool ASCIIParser::
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

bool ASCIIParser::
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
bool ASCIIParser::
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

bool ASCIIParser::
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

bool ASCIIParser::
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

bool ASCIIParser::
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

bool ASCIIParser::
endArray()
{
    return endContainer(true);
}

bool ASCIIParser::
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

bool ASCIIParser::
endDictionary()
{
    return endContainer(false);
}

/*
 * Store the key of the current dictionary.
 */
bool ASCIIParser::
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

bool ASCIIParser::
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

bool ASCIIParser::
isDictionary() const
{
    return _state == ValueState::Dictionary;
}

bool ASCIIParser::
isArray() const
{
    return _state == ValueState::Array;
}

bool ASCIIParser::
finish()
{
    if (isDone()) {
        return true;
    }

    _contextState = ContextState::Done;
    return true;
}

#if 0
#define ASCIIDebug(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#else
#define ASCIIDebug(...)
#endif

static inline uint8_t
hex_to_bin_digit(char ch)
{
    if (ch >= 'a' && ch <= 'f')
        return (ch - 'a') + 10;
    else if (ch >= 'A' && ch <= 'F')
        return (ch - 'A') + 10;
    else if (ch >= '0' && ch <= '9')
        return (ch - '0');
    else
        return 0;
}

static inline uint8_t
hex_to_bin(char const *ascii)
{
    return (hex_to_bin_digit(ascii[0]) << 4) | hex_to_bin_digit(ascii[1]);
}

bool ASCIIParser::
parse(ASCIIPListLexer *lexer, bool strings)
{
    typedef enum _ASCIIParseState {
        kASCIIParsePList = 0,
        kASCIIParseKeyValSeparator,
        kASCIIParseEntrySeparator,

        kASCIIInvalid = -1
    } ASCIIParseState;

    if (strings) {
        /* Begin the root dictionary. */
        if (!beginDictionary()) {
            return false;
        }
        incrementLevel();
    }

    int token;
    ASCIIParseState state = kASCIIParsePList;

    for (;;) {
        token = ASCIIPListLexerReadToken(lexer);
        if (token < 0) {
            if (token == kASCIIPListLexerEndOfFile && isDone()) {
                /* success */
                return true;
            } else if (token == kASCIIPListLexerEndOfFile && state == kASCIIParsePList && strings) {
                if (!endDictionary()) {
                    return false;
                }
                decrementLevel();

                if (getLevel()) {
                    abort("Encountered premature EOF", lexer->line);
                    return false;
                } else {
                    if (!finish()) {
                        return false;
                    }

                    /* success */
                    return true;
                }
            } else if (token == kASCIIPListLexerEndOfFile) {
                abort("Encountered premature EOF", lexer->line);
                return false;
            } else if (token == kASCIIPListLexerInvalidToken) {
                abort("Encountered invalid token", lexer->line);
                return false;
            } else if (token == kASCIIPListLexerUnterminatedLongComment) {
                abort("Encountered unterminated long comment", lexer->line);
                return false;
            } else if (token == kASCIIPListLexerUnterminatedUnquotedString) {
                abort("Encountered unterminated unquoted string", lexer->line);
                return false;
            } else if (token == kASCIIPListLexerUnterminatedQuotedString) {
                abort("Encountered unterminated quoted string", lexer->line);
                return false;
            } else if (token == kASCIIPListLexerUnterminatedData) {
                abort("Encountered unterminated data", lexer->line);
                return false;
            } else {
                abort("Encountered unrecognized token error code", lexer->line);
                return false;
            }
        }

        /* Ignore comments */
        if (token == kASCIIPListLexerTokenInlineComment ||
            token == kASCIIPListLexerTokenLongComment)
            continue;

        switch (state) {
            case kASCIIParsePList:
                if (isDone()) {
                    abort("Encountered token when finished.", lexer->line);
                    return false;
                }

                if (token == kASCIIPListLexerTokenDictionaryStart) {
                    ASCIIDebug("Starting dictionary");
                    if (!beginDictionary()) {
                        return false;
                    }
                    incrementLevel();
                    state = kASCIIParsePList;
                } else if (token == kASCIIPListLexerTokenArrayStart) {
                    ASCIIDebug("Starting array");
                    if (!beginArray()) {
                        return false;
                    }
                    incrementLevel();
                    state = kASCIIParsePList;
                } else if (token == kASCIIPListLexerTokenDictionaryEnd) {
                    ASCIIDebug("Ending dictionary");
                    if (!endDictionary()) {
                        return false;
                    }
                    decrementLevel();
                    if (getLevel()) {
                        state = kASCIIParseEntrySeparator;
                    } else {
                        if (!finish()) {
                            return false;
                        }
                        state = kASCIIParsePList;
                    }
                } else if (token == kASCIIPListLexerTokenArrayEnd) {
                    ASCIIDebug("Ending array");
                    if (!endArray()) {
                        return false;
                    }
                    decrementLevel();
                    if (getLevel()) {
                        state = kASCIIParseEntrySeparator;
                    } else {
                        if (!finish()) {
                            return false;
                        }
                        state = kASCIIParsePList;
                    }
                } else {
                    /* Read all non-container tokens */
                    bool topLevel     = getLevel() == 0;
                    bool isDictionary = this->isDictionary();

                    if (token == kASCIIPListLexerTokenUnquotedString ||
                        token == kASCIIPListLexerTokenQuotedString) {
                        char *contents = ASCIIPListCopyUnquotedString(lexer, '?');
                        std::unique_ptr<String> string = String::New(std::string(contents));
                        free(contents);

                        if (string == NULL) {
                            abort("OOM when copying string", lexer->line);
                            return false;
                        }

                        /* Container context */
                        if (isDictionary) {
                            ASCIIDebug("Storing string %s as key", string->value().c_str());
                            if (!storeKey(std::move(string))) {
                                return false;
                            }
                        } else {
                            ASCIIDebug("Storing string %s", string->value().c_str());
                            if (!storeValue(std::move(string))) {
                                return false;
                            }
                        }
                    } else if (token == kASCIIPListLexerTokenData) {
                        if (isDictionary) {
                            abort("Data cannot be dictionary key", lexer->line);
                            return false;
                        }

                        char   *contents = ASCIIPListCopyData(lexer);
                        size_t  alength  = strlen(contents);
                        size_t  length   = alength / 2;
                        auto    bytes    = std::vector<uint8_t>(length, 0);

                        for (size_t n = 0; n < alength; n += 2) {
                            bytes[n >> 1] = hex_to_bin(contents + n);
                        }

                        std::unique_ptr<Data> data = Data::New(bytes);
                        free(contents);

                        if (data == NULL) {
                            abort("OOM when copying data", lexer->line);
                            return false;
                        }

                        ASCIIDebug("Storing string as data");
                        if (!storeValue(std::move(data))) {
                            return false;
                        }
                    } else {
                        abort("Encountered unexpected token code", lexer->line);
                        return false;
                    }

                    if (topLevel) {
                        if (!finish()) {
                            return false;
                        }
                        state = kASCIIParsePList;
                    } else {
                        if (isDictionary) {
                            state = kASCIIParseKeyValSeparator;
                        } else {
                            state = kASCIIParseEntrySeparator;
                        }
                    }
                }
                break;

            case kASCIIParseKeyValSeparator:
                if (token != kASCIIPListLexerTokenDictionaryKeyValSeparator) {
                    abort("Expected key-value separator; found something else", lexer->line);
                    return false;
                }
                ASCIIDebug("Found keyval separator");
                state = kASCIIParsePList;
                break;

            case kASCIIParseEntrySeparator:
                if (token != ';' && token != ',' &&
                    /*
                     * Arrays do not require a final separator. Dictionaries do.
                     */
                    token != kASCIIPListLexerTokenArrayEnd) {
                    abort("Expected entry separator or array end; found something else", lexer->line);
                    return false;
                }

                if (isDictionary() && token != ';') {
                    abort("Expected ';'", lexer->line);
                    return false;
                }

                if (isArray() &&
                    token != ',' &&
                    token != kASCIIPListLexerTokenArrayEnd) {
                    abort("Expected ',' or ')'", lexer->line);
                    return false;
                }

                if (token == kASCIIPListLexerTokenArrayEnd) {
                    if (isDictionary()) {
                        abort(NULL, lexer->line);
                        return false;
                    }
                    ASCIIDebug("Found array end");
                    if (!endArray()) {
                        return false;
                    }

                    decrementLevel();
                    if (getLevel()) {
                        state = kASCIIParseEntrySeparator;
                    } else {
                        if (!finish()) {
                            return false;
                        }
                        state = kASCIIParsePList;
                    }
                } else {
                    ASCIIDebug("Found entry separator");
                    state = kASCIIParsePList;
                }
                break;
            default:
                abort("Unexpected state", lexer->line);
                return false;
        }
    }
}

