/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/ASCIIPListLexer.h>

#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/*
 * Syntax:
 *
 * Dictionaries
 *
 *  JSON:     { <key> : <value>[, <key> : <value>[, ...]] }
 *  ASCII:    { <key> = <value>; [<key> = <value>; [...]] }
 *
 * Arrays
 *
 *  JSON:     [ <value>[, <value>[, ...]]] ]
 *  ASCII:    ( <value>[, <value>[, ...]]] )
 *
 * Strings
 *
 *  JSON      "..."  quoted chars like \", unicode chars, multiple-line
 *  ASCII:    '...'
 *            "..."  quoted chars like \', multiple-line
 *
 * Data
 *
 *  JSON:     n/a
 *  ASCII:    <0123456789ABCDEF...>
 *
 * Number
 *
 *  JSON:     1.234
 *            1.2e32
 *            1234
 *            .5
 *            .5e2
 *            0100644
 *            +123
 *            -555
 *  ASCII:    same
 *
 * Boolean
 *
 *  JSON:     true / false
 *  ASCII:    n/a
 *
 * Null value
 *
 *  JSON:     null
 *  ASCII:    ""
 *
 * Character Encoding
 *
 *  JSON:     UTF8 clean.
 *  ASCII:    Not strictly ASCII 7-bit clean - can decode unicode.
 */

/** Helpers **/

static inline bool
istokenseparator(char ch, ASCIIPListLexer *lexer)
{
    /*
     * NOTE: This doesn't check for things that would constitute a
     * malformed plist, such as 123456} (ASCII).
     */
    return (isspace(ch) || ch == '\0' ||
            (lexer->style == kASCIIPListLexerStyleJSON &&
             (ch == ',' || ch == '}' || ch == ']' || ch == ':')) ||
            (lexer->style == kASCIIPListLexerStyleASCII &&
             (ch == ',' || ch == ';' || ch == ')' || ch == '=')));
}

static inline bool
isodigit(char ch)
{ return (ch >= '0' && ch <= '7'); }

static inline uint8_t
dec2bin(char ch)
{ return (ch - '0'); }

static inline uint8_t
hex2bin(char ch)
{
    if (ch >= 'a' && ch <= 'f')
        return (ch - 'a' + 10);
    else if (ch >= 'A' && ch <= 'F')
        return (ch - 'A' + 10);
    else
        return (ch - '0');
}

static int
ASCIIPListLexerReadInlineComment(ASCIIPListLexer *lexer)
{
    char const *b, *p = lexer->pointer + 2;

    lexer->tokenBegin = (p - lexer->inputBuffer);
    for (b = p; *p != '\0' && *p != '\n' && *p != '\r'; p++)
        ;
    lexer->tokenLength = p - b;
    lexer->pointer = p;

    return kASCIIPListLexerTokenInlineComment;
}

static int
ASCIIPListLexerReadLongComment(ASCIIPListLexer *lexer)
{
    char const *b, *p = lexer->pointer + 2;

    lexer->tokenBegin = (p - lexer->inputBuffer);
    for (b = p; *p != '\0'; p++) {
        if (p[0] == '\n') {
            lexer->line++;
            lexer->lineStart = p + 1;
        } else if (p[0] == '*' && p[1] == '/') {
            lexer->tokenLength = p - b;
            lexer->pointer = p + 2;
            return kASCIIPListLexerTokenLongComment;
        }
    }

    return kASCIIPListLexerUnterminatedLongComment;
}

static int
ASCIIPListLexerReadSingleQuotedString(ASCIIPListLexer *lexer)
{
    char const *b, *p = lexer->pointer + 1;

    lexer->tokenBegin = (p - lexer->inputBuffer);
    for (b = p; *p != '\'' && *p != '\0'; p++) {
        if (*p == '\n') {
            lexer->line++;
            lexer->lineStart = p + 1;
        }
    }

    if (*p != '\'') {
        return kASCIIPListLexerUnterminatedQuotedString;
    }

    lexer->tokenLength = p - b;
    lexer->pointer = p + 1;

    return kASCIIPListLexerTokenQuotedString;
}

static int
ASCIIPListLexerReadDoubleQuotedString(ASCIIPListLexer *lexer)
{
    char const *b, *p = lexer->pointer + 1;

    lexer->tokenBegin = (p - lexer->inputBuffer);
    for (b = p; *p != '\"' && *p != '\0'; p++) {
        if (*p == '\n') {
            lexer->line++;
            lexer->lineStart = p + 1;
        } else if (*p == '\\') {
            p++;
        }
    }

    if (*p != '\"') {
        return kASCIIPListLexerUnterminatedQuotedString;
    }

    lexer->tokenLength = p - b;
    lexer->pointer = p + 1;

    return kASCIIPListLexerTokenQuotedString;
}

static int
ASCIIPListLexerReadData(ASCIIPListLexer *lexer)
{
    char const *b, *p = lexer->pointer + 1;

    lexer->tokenBegin = (p - lexer->inputBuffer);
    for (b = p; *p != '>' && *p != '\0'; p++) {
        if (*p == '\n') {
            lexer->line++;
            lexer->lineStart = p + 1;
        } else if (*p == ' ') {
            /* skip over spaces */
        } else {
            uint8_t val = hex2bin(*p);
            if (val < 0 || val >= 16) {
                break;
            }
        }
    }

    if (*p != '>')
        return kASCIIPListLexerUnterminatedData;

    lexer->tokenLength = p - b;
    lexer->pointer = p + 1;

    return kASCIIPListLexerTokenData;
}

static int
ASCIIPListLexerReadString(ASCIIPListLexer *lexer)
{
    if (lexer->pointer[0] == '\'' && lexer->style != kASCIIPListLexerStyleJSON) {
        return ASCIIPListLexerReadSingleQuotedString(lexer);
    } else if (lexer->pointer[0] == '\"') {
        return ASCIIPListLexerReadDoubleQuotedString(lexer);
    } else {
        return kASCIIPListLexerInvalidToken;
    }
}

static int
ASCIIPListLexerReadJSONNumber(ASCIIPListLexer *lexer)
{
    bool integer = true;
    char const *p = lexer->pointer;
    char const *b = p;

    lexer->tokenBegin = (p - lexer->inputBuffer);

    if (*p == '-') {
        p++;
    }

    if (!isdigit(*p)) {
        return kASCIIPListLexerInvalidToken;
    }
    bool zero = (*p == '0');
    while (isdigit(*p)) {
        /* Numbers cannot start with zero. */
        if (zero && *p != '0') {
            return kASCIIPListLexerInvalidToken;
        }

        p++;
    }

    if (*p == '.') {
        integer = false;

        p++;

        if (!isdigit(*p)) {
            return kASCIIPListLexerInvalidToken;
        }
        while (isdigit(*p)) {
            p++;
        }
    }

    if (*p == 'e' || *p == 'E') {
        integer = false;

        p++;
        if (*p == '+' || *p == '-') {
            p++;
        }

        if (!isdigit(*p)) {
            return kASCIIPListLexerInvalidToken;
        }
        while (isdigit(*p)) {
            p++;
        }
    }

    if (istokenseparator(*p, lexer)) {
        lexer->tokenLength = p - b;
        lexer->pointer = p;
        return (integer ? kASCIIPListLexerTokenNumberInteger : kASCIIPListLexerTokenNumberReal);
    }

    return kASCIIPListLexerInvalidToken;
}

static int
ASCIIPListLexerReadKeyword(ASCIIPListLexer *lexer)
{
    int         rc = kASCIIPListLexerInvalidToken;
    char const *p  = lexer->pointer;
    char const *b  = p;

    lexer->tokenBegin = p - lexer->inputBuffer;

    if (lexer->style == kASCIIPListLexerStyleJSON) {
        if (*p == 't' && strncmp(p, "true", 4) == 0 &&
            istokenseparator(*(p + 4), lexer)) {
            p += 4;
            rc = kASCIIPListLexerTokenBoolTrue;
        } else if (*p == 'f' && strncmp(p, "false", 5) == 0 &&
                   istokenseparator(*(p + 5), lexer)) {
            p += 5;
            rc = kASCIIPListLexerTokenBoolFalse;
        } else if (*p == 'n' && strncmp(p, "null", 4) == 0 &&
                   istokenseparator(*(p + 4), lexer)) {
            p += 4;
            rc = kASCIIPListLexerTokenNull;
        }
    } else if (lexer->style == kASCIIPListLexerStyleASCII) {
        rc = kASCIIPListLexerTokenUnquotedString;
        /*
            * '$' is encountered in pbxproj files.
            */
        while (isalnum(*p) || *p == '_' || *p == '.' || *p == '$' ||
                              *p == '-' || *p == ':' || *p == '/') {
            if (*p & 0x80) {
                rc = kASCIIPListLexerInvalidToken;
                break;
            }
            p++;
        }
    } else {
        rc = kASCIIPListLexerInvalidToken;
    }

    lexer->tokenLength = p - b;
    lexer->pointer = p;

    return rc;
}

int
ASCIIPListLexerReadToken(ASCIIPListLexer *lexer)
{
    char const *p = lexer->pointer;
    while (p < lexer->endBuffer) {
        switch (*p) {
            case '/': /* Comments */
                if (p[1] == '/') {
                    lexer->pointer = p;
                    return ASCIIPListLexerReadInlineComment(lexer);
                } else if (p[1] == '*') {
                    lexer->pointer = p;
                    return ASCIIPListLexerReadLongComment(lexer);
                } else {
                    lexer->pointer = p;
                    return ASCIIPListLexerReadKeyword(lexer);
                }
                break;

            case '{': case '}': /* Dictionaries */
                lexer->tokenBegin = p - lexer->inputBuffer;
                lexer->tokenLength = 1;
                lexer->pointer = p + 1;
                return (*p == '{' ? kASCIIPListLexerTokenDictionaryStart :
                        kASCIIPListLexerTokenDictionaryEnd);

            case '[': case ']': /* Arrays (JSON) */
                if (lexer->style == kASCIIPListLexerStyleJSON) {
                    lexer->tokenBegin = p - lexer->inputBuffer;
                    lexer->tokenLength = 1;
                    lexer->pointer = p + 1;
                    return (*p == '[' ? kASCIIPListLexerTokenArrayStart :
                            kASCIIPListLexerTokenArrayEnd);
                }
                return kASCIIPListLexerInvalidToken;

            case '(': case ')': /* Arrays (ASCII) */
                if (lexer->style == kASCIIPListLexerStyleASCII) {
                    lexer->tokenBegin = p - lexer->inputBuffer;
                    lexer->tokenLength = 1;
                    lexer->pointer = p + 1;
                    return (*p == '(' ? kASCIIPListLexerTokenArrayStart :
                            kASCIIPListLexerTokenArrayEnd);
                }
                return kASCIIPListLexerInvalidToken;

            case ':': /* Key : Value (JSON) */
                if (lexer->style == kASCIIPListLexerStyleJSON) {
                    lexer->tokenBegin = p - lexer->inputBuffer;
                    lexer->tokenLength = 1;
                    lexer->pointer = p + 1;
                    return kASCIIPListLexerTokenDictionaryKeyValSeparator;
                }
                return kASCIIPListLexerInvalidToken;

            case '=': /* Key = Value (ASCII) */
                if (lexer->style == kASCIIPListLexerStyleASCII) {
                    lexer->tokenBegin = p - lexer->inputBuffer;
                    lexer->tokenLength = 1;
                    lexer->pointer = p + 1;
                    return kASCIIPListLexerTokenDictionaryKeyValSeparator;
                }
                return kASCIIPListLexerInvalidToken;

            case ',': /* Array/Dictionary (JSON), Array (ASCII) Entries */
                if (lexer->style == kASCIIPListLexerStyleJSON ||
                    lexer->style == kASCIIPListLexerStyleASCII) {
                    lexer->tokenBegin = p - lexer->inputBuffer;
                    lexer->tokenLength = 1;
                    lexer->pointer = p + 1;
                    return *p;
                }
                return kASCIIPListLexerInvalidToken;

            case ';': /* Dictionary Entries (ASCII) */
                if (lexer->style == kASCIIPListLexerStyleASCII) {
                    lexer->tokenBegin = p - lexer->inputBuffer;
                    lexer->tokenLength = 1;
                    lexer->pointer = p + 1;
                    return *p;
                }
                return kASCIIPListLexerInvalidToken;

            case '\'':
            case '\"':
                lexer->pointer = p;
                return ASCIIPListLexerReadString(lexer);

            case '<':
                if (lexer->style == kASCIIPListLexerStyleASCII) {
                    lexer->pointer = p;
                    return ASCIIPListLexerReadData(lexer);
                }
                return kASCIIPListLexerInvalidToken;

            case '+': case '-': case '.':
                if (lexer->style == kASCIIPListLexerStyleJSON) {
                    lexer->pointer = p;
                    lexer->tokenBegin = p - lexer->inputBuffer;
                    lexer->tokenLength = 1;
                    int rc = ASCIIPListLexerReadJSONNumber(lexer);
                    if (rc != kASCIIPListLexerInvalidToken) {
                        return rc;
                    }
                }
                lexer->pointer = p;
                return ASCIIPListLexerReadKeyword(lexer);

            case ' ': case '\f': case '\t': case '\r':
                 p++;
                 break;

            case '\n':
                 p++, lexer->line++; lexer->lineStart = p;
                 break;

            default:
                 if (isdigit(*p)) {
                     lexer->pointer = p;
                     if (lexer->style == kASCIIPListLexerStyleJSON) {
                         int rc = ASCIIPListLexerReadJSONNumber(lexer);
                         if (rc != kASCIIPListLexerInvalidToken) {
                             return rc;
                         }
                     }
                     return ASCIIPListLexerReadKeyword(lexer);
                 } else {
                     lexer->pointer = p;
                     return ASCIIPListLexerReadKeyword(lexer);
                 }
                 break;
        }
    }
    return kASCIIPListLexerEndOfFile;
}

void
ASCIIPListLexerInit(ASCIIPListLexer *lexer, char const *buffer, int length,
        int style)
{
    memset(lexer, 0, sizeof(*lexer));
    lexer->inputBuffer = buffer;
    lexer->pointer = lexer->inputBuffer;
    lexer->lineStart = lexer->inputBuffer;
    lexer->endBuffer = lexer->inputBuffer + length;
    lexer->style = style;
    lexer->line = 1;
}

/* Convert sequence \xXX */
static inline bool
dehexify(char *p, int avail, int *eat)
{
    uint8_t  byte = 0;
    char    *rep  = p;

    p += 2, (*eat)++, avail -= 2; /* Remove '\x' */
    if (avail >= 1) {
        if (!isxdigit(*p))
            return false;
        byte = hex2bin(*p++);
        (*eat)++, avail--;
    }
    if (avail >= 1 && isxdigit(*p)) {
        byte <<= 4;
        byte |= hex2bin(*p++);
        (*eat)++, avail--;
    }
    *rep = byte;
    return true;
}

/* Convert sequence \0XXXX */
static inline bool
deoctify(char *p, int avail, int *eat)
{
    uint8_t  byte;
    char    *rep = p;

    p += 2, (*eat)++, avail -= 2; /* Remove '\0' */

    if (avail >= 1) {
        if (!isodigit(*p))
            return false;
        byte = dec2bin(*p++);
        (*eat)++, avail--;
    } else {
        return false;
    }
    if (avail >= 1 && isodigit(*p)) {
        byte <<= 3;
        byte |= dec2bin(*p++);
        (*eat)++, avail--;
    }
    if (avail >= 1 && isodigit(*p)) {
        byte <<= 3;
        byte |= dec2bin(*p++);
        (*eat)++, avail--;
    }
    if (avail >= 1 && isodigit(*p)) {
        byte <<= 3;
        byte |= dec2bin(*p++);
        (*eat)++, avail--;
    }
    *rep = byte;
    return true;
}

/* Convert sequence \uXXXX */
static inline bool
deunicodify(char **pp, int avail, int *eat)
{
    uint32_t codepoint;
    bool     utf32;
    char    *p = *pp;
    char    *rep = p;

    p++; /* Remove '\' */
    utf32 = (*p == 'U');
    p++, (*eat)++, avail--; /* Remove 'u' */
    if (avail >= 1) {
        if (!isxdigit(*p))
            return false;
        codepoint = hex2bin(*p++);
        (*eat)++, avail--;
    } else {
        return false;
    }
    if (avail >= 1 && isxdigit(*p)) {
        codepoint <<= 4;
        codepoint |= hex2bin(*p++);
        (*eat)++, avail--;
    }
    if (avail >= 1 && isxdigit(*p)) {
        codepoint <<= 4;
        codepoint |= hex2bin(*p++);
        (*eat)++, avail--;
    }
    if (avail >= 1 && isxdigit(*p)) {
        codepoint <<= 4;
        codepoint |= hex2bin(*p++);
        (*eat)++, avail--;
    }
    if (utf32) {
        if (avail >= 1 && isxdigit(*p)) {
            codepoint <<= 4;
            codepoint |= hex2bin(*p++);
            (*eat)++, avail--;
        }
        if (avail >= 1 && isxdigit(*p)) {
            codepoint <<= 4;
            codepoint |= hex2bin(*p++);
            (*eat)++, avail--;
        }
        if (avail >= 1 && isxdigit(*p)) {
            codepoint <<= 4;
            codepoint |= hex2bin(*p++);
            (*eat)++, avail--;
        }
        if (avail >= 1 && isxdigit(*p)) {
            codepoint <<= 4;
            codepoint |= hex2bin(*p++);
            (*eat)++, avail--;
        }
    }

    if (codepoint >= 0x110000)
        codepoint = 0xfffe;

    if (codepoint <= 0x7f)
        *rep = codepoint;
    else if (codepoint <= 0x7ff) {
        *rep++ = 0xc0 | (codepoint >> 6);
        *rep++ = 0x80 | (codepoint & 0x3f);
        (*eat) -= 2;
    } else if (codepoint <= 0xffff) {
        *rep++ = 0xe0 | (codepoint >> 12);
        *rep++ = 0x80 | ((codepoint >> 6) & 0x3f);
        *rep++ = 0x80 | (codepoint & 0x3f);
        (*eat) -= 3;
    } else if (codepoint <= 0x10ffff) {
        *rep++ = 0xf0 | (codepoint >> 18);
        *rep++ = 0x80 | ((codepoint >> 12) & 0x3f);
        *rep++ = 0x80 | ((codepoint >> 6) & 0x3f);
        *rep++ = 0x80 | (codepoint & 0x3f);
        (*eat) -= 4;
    }

    *pp  = rep;
    return true;
}

/* Unquoted string copy */

char *
ASCIIPListCopyUnquotedString(ASCIIPListLexer const *lexer, int lossByte)
{
    char *copy, *end, *p;

    copy = (char *)malloc(lexer->tokenLength + 1);
    if (copy == NULL)
        return NULL;

    memcpy(copy, lexer->inputBuffer + lexer->tokenBegin, lexer->tokenLength);
    copy[lexer->tokenLength] = '\0';

    p = copy;
    end = copy + lexer->tokenLength;

    while ((p = strchr(p, '\\')) != NULL) {
        int eat = 1;

        /* Keep quoting char if it's at the end of the string. */
        if ((p - end) == 0)
            break;

        switch (p[1]) {
            case 'a':  *p++ = '\a'; break;
            case 'b':  *p++ = '\b'; break;
            case 'f':  *p++ = '\f'; break;
            case 'n':  *p++ = '\n'; break;
            case 'r':  *p++ = '\r'; break;
            case 't':  *p++ = '\t'; break;
            case '\\': ++p; break;
            case 'x':
                if (!dehexify(p, p - end, &eat)) {
                    if (lossByte <= 0)
                        goto fail;
                    else
                        *p++ = lossByte;
                }
                break;
            case '0':
                if ((p - end) >= 2 && isdigit(p[2])) {
                    if (!deoctify(p, p - end, &eat)) {
                        if (lossByte <= 0)
                            goto fail;
                        else
                            *p++ = lossByte;
                    }
                } else {
                    *p = '\0';
                }
                break;

            case 'u':
            case 'U':
                if ((end - p) >= 2 && isxdigit(p[2])) {
                    if (!deunicodify(&p, end - p, &eat)) {
                        if (lossByte <= 0)
                            goto fail;
                        else
                            *p++ = lossByte;
                    }
                }
                break;
            case '\r':
            case '\n':
                /* Line continuation */
                eat = 2;
                break;
        }

        if (eat != 0) {
            size_t newlen = end - (p + eat);
            memmove(p, p + eat, newlen);
            end -= eat;
            *end = '\0';
        }
    }

    *end = '\0';

    return copy;

fail:
    free(copy);
    return NULL;
}

/* Data copy */

char *
ASCIIPListCopyData(ASCIIPListLexer const *lexer)
{
    char *copy, *end, *p;

    copy = (char *)malloc(lexer->tokenLength + 1);
    if (copy == NULL)
        return NULL;

    memcpy(copy, lexer->inputBuffer + lexer->tokenBegin, lexer->tokenLength);
    copy[lexer->tokenLength] = '\0';

    end = copy + lexer->tokenLength;

    while ((p = strchr(copy, ' ')) != NULL) {
        size_t newlen = end - (p + 1);
        memmove(p, p + 1, newlen);
        end -= 1;
    }

    *end = '\0';

    return copy;
}

