/*
 * Copyright © 2012 Ran Benita <ran234@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "xkbcomp-priv.h"
#include "parser-priv.h"
#include "scanner-utils.h"

static bool
number(struct scanner *s, int64_t *out, int *out_tok)
{
    bool is_float = false, is_hex = false;
    const char *start = s->s + s->pos;
    char *end;

    if (scanner_lit(s, "0x")) {
        while (is_xdigit(scanner_peek(s))) scanner_next(s);
        is_hex = true;
    }
    else {
        while (is_digit(scanner_peek(s))) scanner_next(s);
        is_float = scanner_chr(s, '.');
        while (is_digit(scanner_peek(s))) scanner_next(s);
    }
    if (s->s + s->pos == start)
        return false;

    errno = 0;
    if (is_hex)
        *out = strtoul(start, &end, 16);
    else if (is_float)
        /* The parser currently just ignores floats, so the cast is
         * fine - the value doesn't matter. */
        *out = strtod(start, &end);
    else
        *out = strtoul(start, &end, 10);
    if (errno != 0 || s->s + s->pos != end)
        *out_tok = ERROR_TOK;
    else
        *out_tok = (is_float ? FLOAT : INTEGER);
    return true;
}

int
_xkbcommon_lex(YYSTYPE *yylval, struct scanner *s)
{
    int tok;

skip_more_whitespace_and_comments:
    /* Skip spaces. */
    while (is_space(scanner_peek(s))) scanner_next(s);

    /* Skip comments. */
    if (scanner_lit(s, "//") || scanner_chr(s, '#')) {
        scanner_skip_to_eol(s);
        goto skip_more_whitespace_and_comments;
    }

    /* See if we're done. */
    if (scanner_eof(s)) return END_OF_FILE;

    /* New token. */
    s->token_line = s->line;
    s->token_column = s->column;
    s->buf_pos = 0;

    /* String literal. */
    if (scanner_chr(s, '\"')) {
        while (!scanner_eof(s) && !scanner_eol(s) && scanner_peek(s) != '\"') {
            if (scanner_chr(s, '\\')) {
                uint8_t o;
                size_t start_pos = s->pos;
                if      (scanner_chr(s, '\\')) scanner_buf_append(s, '\\');
                else if (scanner_chr(s, 'n'))  scanner_buf_append(s, '\n');
                else if (scanner_chr(s, 't'))  scanner_buf_append(s, '\t');
                else if (scanner_chr(s, 'r'))  scanner_buf_append(s, '\r');
                else if (scanner_chr(s, 'b'))  scanner_buf_append(s, '\b');
                else if (scanner_chr(s, 'f'))  scanner_buf_append(s, '\f');
                else if (scanner_chr(s, 'v'))  scanner_buf_append(s, '\v');
                else if (scanner_chr(s, 'e'))  scanner_buf_append(s, '\033');
                else if (scanner_oct(s, &o) && is_valid_char((char) o))
                    scanner_buf_append(s, (char) o);
                else if (s->pos > start_pos)
                    scanner_warn_with_code(s,
                        XKB_WARNING_INVALID_ESCAPE_SEQUENCE,
                        "invalid octal escape sequence (%.*s) in string literal",
                        (int) (s->pos - start_pos + 1), &s->s[start_pos - 1]);
                    /* Ignore. */
                else {
                    scanner_warn_with_code(s,
                        XKB_WARNING_UNKNOWN_CHAR_ESCAPE_SEQUENCE,
                        "unknown escape sequence (\\%c) in string literal",
                        scanner_peek(s));
                    /* Ignore. */
                }
            } else {
                scanner_buf_append(s, scanner_next(s));
            }
        }
        if (!scanner_buf_append(s, '\0') || !scanner_chr(s, '\"')) {
            scanner_err(s, "unterminated string literal");
            return ERROR_TOK;
        }
        yylval->str = strdup(s->buf);
        if (!yylval->str)
            return ERROR_TOK;
        return STRING;
    }

    /* Key name literal. */
    if (scanner_chr(s, '<')) {
        while (is_graph(scanner_peek(s)) && scanner_peek(s) != '>')
            scanner_buf_append(s, scanner_next(s));
        if (!scanner_buf_append(s, '\0') || !scanner_chr(s, '>')) {
            scanner_err(s, "unterminated key name literal");
            return ERROR_TOK;
        }
        /* Empty key name literals are allowed. */
        yylval->atom = xkb_atom_intern(s->ctx, s->buf, s->buf_pos - 1);
        return KEYNAME;
    }

    /* Operators and punctuation. */
    if (scanner_chr(s, ';')) return SEMI;
    if (scanner_chr(s, '{')) return OBRACE;
    if (scanner_chr(s, '}')) return CBRACE;
    if (scanner_chr(s, '=')) return EQUALS;
    if (scanner_chr(s, '[')) return OBRACKET;
    if (scanner_chr(s, ']')) return CBRACKET;
    if (scanner_chr(s, '(')) return OPAREN;
    if (scanner_chr(s, ')')) return CPAREN;
    if (scanner_chr(s, '.')) return DOT;
    if (scanner_chr(s, ',')) return COMMA;
    if (scanner_chr(s, '+')) return PLUS;
    if (scanner_chr(s, '-')) return MINUS;
    if (scanner_chr(s, '*')) return TIMES;
    if (scanner_chr(s, '/')) return DIVIDE;
    if (scanner_chr(s, '!')) return EXCLAM;
    if (scanner_chr(s, '~')) return INVERT;

    /* Identifier. */
    if (is_alpha(scanner_peek(s)) || scanner_peek(s) == '_') {
        s->buf_pos = 0;
        while (is_alnum(scanner_peek(s)) || scanner_peek(s) == '_')
            scanner_buf_append(s, scanner_next(s));
        if (!scanner_buf_append(s, '\0')) {
            scanner_err(s, "identifier too long");
            return ERROR_TOK;
        }

        /* Keyword. */
        tok = keyword_to_token(s->buf, s->buf_pos - 1);
        if (tok != -1) return tok;

        yylval->str = strdup(s->buf);
        if (!yylval->str)
            return ERROR_TOK;
        return IDENT;
    }

    /* Number literal (hexadecimal / decimal / float). */
    if (number(s, &yylval->num, &tok)) {
        if (tok == ERROR_TOK) {
            scanner_err_with_code(s, XKB_ERROR_MALFORMED_NUMBER_LITERAL,
                                  "malformed number literal");
            return ERROR_TOK;
        }
        return tok;
    }

    scanner_err(s, "unrecognized token");
    return ERROR_TOK;
}

XkbFile *
XkbParseString(struct xkb_context *ctx, const char *string, size_t len,
               const char *file_name, const char *map)
{
    struct scanner scanner;
    scanner_init(&scanner, ctx, string, len, file_name, NULL);
    return parse(ctx, &scanner, map);
}

XkbFile *
XkbParseFile(struct xkb_context *ctx, FILE *file,
             const char *file_name, const char *map)
{
    bool ok;
    XkbFile *xkb_file;
    char *string;
    size_t size;

    ok = map_file(file, &string, &size);
    if (!ok) {
        log_err(ctx, XKB_LOG_MESSAGE_NO_ID,
                "Couldn't read XKB file %s: %s\n",
                file_name, strerror(errno));
        return NULL;
    }

    xkb_file = XkbParseString(ctx, string, size, file_name, map);
    unmap_file(string, size);
    return xkb_file;
}
