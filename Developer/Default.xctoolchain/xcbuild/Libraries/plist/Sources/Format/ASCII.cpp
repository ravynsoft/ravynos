/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/ASCII.h>
#include <plist/Format/ASCIIParser.h>
#include <plist/Format/ASCIIWriter.h>
#include <plist/Objects.h>

using plist::Format::Type;
using plist::Format::Encoding;
using plist::Format::Format;
using plist::Format::ASCII;
using plist::Object;

ASCII::
ASCII(bool strings, Encoding encoding) :
    _strings (strings),
    _encoding(encoding)
{
}

Type ASCII::
FormatType()
{
    return Type::ASCII;
}

namespace plist { namespace Format {

template<>
std::unique_ptr<ASCII> Format<ASCII>::
Identify(std::vector<uint8_t> const &contents)
{
    Encoding encoding = Encodings::Detect(contents);

    /*
     * Identification of ASCII is as follows:
     *
     * If BOM is present, it's unicode encoded, otherwise MacRoman.
     * Any C / * ... * / and C++ // style comments are skipped.
     * The start of any data (disambiguated from XML), dictionary, array,
     * quoted string, or unquoted string marks the start (checks for no '=' after
     * the string to make sure this isn't strings format).
     *
     * TODO: Encoding may also be specified in the leading inline comment. Handle this?
     */

    enum State {
        kStateBegin,
        kStateComment,
        kStateInlineComment,
        kStateIdentifier,
        kStateQuoting,
        kStateEqual,
    };

    uint8_t last = '\0';
    enum State state = kStateBegin, pstate = state;
    bool identifier = false;

    for (auto bp = contents.begin(); bp != contents.end();) {
        /* Conceal zeroes for UTF-16/32 encodings. */
        if (*bp == 0 || (state != kStateComment &&
                         state != kStateInlineComment &&
                         state != kStateIdentifier &&
                         isspace(*bp))) {
            bp++;
            continue;
        }

        if (state == kStateComment) {
            switch (*bp) {
                case '/':
                    if (last == '*') {
                        state = pstate;
                    }
                    last = 0;
                    break;
                case '*':
                    last = *bp;
                    break;
                default:
                    last = 0;
                    break;
            }
            bp++;
            continue;
        } else if (state == kStateInlineComment) {
            if (*bp == '\n') {
                state = pstate;
                last = 0;
            }

            bp++;
            continue;
        } else {
            switch (*bp) {
                case '/':
                    if (last == '/') {
                        pstate = state;
                        state = kStateInlineComment;
                        last = 0;
                        bp++;
                    } else if (last != 0) {
                        return nullptr;
                    } else {
                        last = *bp++;
                    }
                    continue;

                case '*':
                    if (last == '/') {
                        pstate = state;
                        state = kStateComment;
                    }
                    last = 0;
                    bp++;
                    continue;

                default:
                    break;
            }
        }

        if (state == kStateBegin) {
            switch (*bp) {
                    /*
                     * We conceal some BOM chars for UTF encodings
                     * in the first four bytes.
                     */
                case 0xfe: /* UTF-16/32 */
                case 0xff:
                case 0xef: /* UTF-8 */
                case 0xbb:
                case 0xbf:
                    if (bp - contents.begin() < 4) {
                        bp++;
                        continue;
                    } else {
                        return nullptr;
                    }
                case '/': /* Comments */
                case '*':
                    break;
                case '(': {
                    return std::unique_ptr<ASCII>(new ASCII(ASCII::Create(false, encoding)));
                }
                case '{':
                    state = kStateIdentifier;
                    bp++;
                    break;
                case '<':
                    if (bp[1] != '?' && bp[1] != '!') {
                        return std::unique_ptr<ASCII>(new ASCII(ASCII::Create(false, encoding)));
                    }
                    return nullptr;
                case '\"':
                    identifier = true;
                    state = kStateIdentifier;
                    break;
                default:
                    if (isalnum(*bp) || *bp == '_' || *bp == '.' || *bp == '$') {
                        identifier = true;
                        state = kStateIdentifier;
                        break;
                    }
                    return nullptr;
            }
        } else if (state == kStateIdentifier) {
            if (isspace(*bp) && last == 0) {
                /* Haven't started the identifier yet */
            } else if (*bp == '"') {
                state = kStateQuoting;
            } else if (isspace(*bp) && last) {
                state = kStateEqual;
                last = 0;
            } else {
                last = *bp;
            }

            bp++;
        } else if (state == kStateQuoting) {
            if (*bp == '"' && last != '\\') {
                state = kStateEqual;
                last = 0;
            } else {
                last = *bp;
            }
            bp++;
        } else if (state == kStateEqual) {
            if (*bp != '=') {
                /* Probably JSON */
                return nullptr;
            }

            return std::unique_ptr<ASCII>(new ASCII(ASCII::Create(identifier, encoding)));
        } else {
            return nullptr;
        }
    }

    if (state == kStateEqual && identifier) {
        /* ASCII standalone string. */
        return std::unique_ptr<ASCII>(new ASCII(ASCII::Create(false, encoding)));
    }

    if (state == kStateBegin) {
        /* Empty document. Assume strings format. */
        return std::unique_ptr<ASCII>(new ASCII(ASCII::Create(true, encoding)));
    }

    return nullptr;
}

template<>
std::pair<std::unique_ptr<Object>, std::string> Format<ASCII>::
Deserialize(std::vector<uint8_t> const &contents, ASCII const &format)
{
    std::unique_ptr<Object> root = nullptr;
    std::string             error;

    std::vector<uint8_t> const data = Encodings::Convert(contents, format.encoding(), Encoding::UTF8);

    /* Create lexer. */
    ASCIIPListLexer lexer;
    ASCIIPListLexerInit(&lexer, reinterpret_cast<char const *>(data.data()), data.size(), kASCIIPListLexerStyleASCII);

    /* Parse contents. */
    ASCIIParser parser;
    if (parser.parse(&lexer, format.strings())) {
        root = std::move(parser.root());
    } else {
        error = parser.error();
    }

    return std::make_pair(std::move(root), error);
}

template<>
std::pair<std::unique_ptr<std::vector<uint8_t>>, std::string> Format<ASCII>::
Serialize(Object const *object, ASCII const &format)
{
    if (object == nullptr) {
        return std::make_pair(nullptr, "object was null");
    }

    ASCIIWriter writer = ASCIIWriter(object, format.strings());
    if (!writer.write()) {
        return std::make_pair(nullptr, "serialization failed");
    }

    std::vector<uint8_t> const data = Encodings::Convert(writer.contents(), Encoding::UTF8, format.encoding());

    return std::make_pair(std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(data.begin(), data.end())), std::string());
}

} }

ASCII ASCII::
Create(bool strings, Encoding encoding)
{
    return ASCII(strings, encoding);
}
