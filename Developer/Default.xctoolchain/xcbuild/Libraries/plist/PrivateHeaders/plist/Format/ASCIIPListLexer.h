/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_ASCIIPListLexer_h
#define __plist_ASCIIPListLexer_h

enum {
    kASCIIPListLexerStyleASCII,
    kASCIIPListLexerStyleJSON,
};

typedef struct ASCIIPListLexer {
    char const *inputBuffer;
    char const *endBuffer;
    char const *pointer;
    char const *lineStart;
    int         style;
    int         line;
    int         tokenBegin;
    int         tokenLength;
} ASCIIPListLexer;

enum {
    kASCIIPListLexerTokenInlineComment = 1000,
    kASCIIPListLexerTokenLongComment,
    kASCIIPListLexerTokenUnquotedString,
    kASCIIPListLexerTokenQuotedString,
    kASCIIPListLexerTokenNumberInteger,
    kASCIIPListLexerTokenNumberReal,
    kASCIIPListLexerTokenBoolFalse,
    kASCIIPListLexerTokenBoolTrue,
    kASCIIPListLexerTokenNull,
    kASCIIPListLexerTokenDictionaryStart,
    kASCIIPListLexerTokenDictionaryKeyValSeparator,
    kASCIIPListLexerTokenDictionaryEnd,
    kASCIIPListLexerTokenArrayStart,
    kASCIIPListLexerTokenArrayEnd,
    kASCIIPListLexerTokenData,

    kASCIIPListLexerInvalidToken = -1,
    kASCIIPListLexerEndOfFile = -2,
    kASCIIPListLexerUnterminatedLongComment = -3,
    kASCIIPListLexerUnterminatedUnquotedString = -4,
    kASCIIPListLexerUnterminatedQuotedString = -5,
    kASCIIPListLexerUnterminatedData = -6,
};

#ifdef __cplusplus
extern "C" {
#endif

void ASCIIPListLexerInit(ASCIIPListLexer *lexer, char const *buffer,
        int length, int style);

int ASCIIPListLexerReadToken(ASCIIPListLexer *lexer);
char *ASCIIPListCopyUnquotedString(ASCIIPListLexer const *lexer, int lossByte);
char *ASCIIPListCopyData(ASCIIPListLexer const *lexer);

#ifdef __cplusplus
}
#endif

#endif  /* !__plist_ASCIIPListLexer_h */
