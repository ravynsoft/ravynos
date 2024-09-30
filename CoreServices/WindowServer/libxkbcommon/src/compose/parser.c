/*
 * Copyright © 2013 Ran Benita <ran234@gmail.com>
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

/******************************************************************

              Copyright 1992 by Oki Technosystems Laboratory, Inc.
              Copyright 1992 by Fuji Xerox Co., Ltd.

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Oki Technosystems
Laboratory and Fuji Xerox not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.
Oki Technosystems Laboratory and Fuji Xerox make no representations
about the suitability of this software for any purpose.  It is provided
"as is" without express or implied warranty.

OKI TECHNOSYSTEMS LABORATORY AND FUJI XEROX DISCLAIM ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL OKI TECHNOSYSTEMS
LABORATORY AND FUJI XEROX BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.

  Author: Yasuhiro Kawai        Oki Technosystems Laboratory
  Author: Kazunori Nishihara    Fuji Xerox

******************************************************************/

#include "config.h"

#include <errno.h>

#include "utils.h"
#include "scanner-utils.h"
#include "table.h"
#include "paths.h"
#include "utf8.h"
#include "parser.h"

/*
 * Grammar adapted from libX11/modules/im/ximcp/imLcPrs.c.
 * See also the XCompose(5) manpage.
 *
 * FILE          ::= { [PRODUCTION] [COMMENT] "\n" | INCLUDE }
 * INCLUDE       ::= "include" '"' INCLUDE_STRING '"'
 * PRODUCTION    ::= LHS ":" RHS [ COMMENT ]
 * COMMENT       ::= "#" {<any character except null or newline>}
 * LHS           ::= EVENT { EVENT }
 * EVENT         ::= [MODIFIER_LIST] "<" keysym ">"
 * MODIFIER_LIST ::= (["!"] {MODIFIER} ) | "None"
 * MODIFIER      ::= ["~"] MODIFIER_NAME
 * MODIFIER_NAME ::= ("Ctrl"|"Lock"|"Caps"|"Shift"|"Alt"|"Meta")
 * RHS           ::= ( STRING | keysym | STRING keysym )
 * STRING        ::= '"' { CHAR } '"'
 * CHAR          ::= GRAPHIC_CHAR | ESCAPED_CHAR
 * GRAPHIC_CHAR  ::= locale (codeset) dependent code
 * ESCAPED_CHAR  ::= ('\\' | '\"' | OCTAL | HEX )
 * OCTAL         ::= '\' OCTAL_CHAR [OCTAL_CHAR [OCTAL_CHAR]]
 * OCTAL_CHAR    ::= (0|1|2|3|4|5|6|7)
 * HEX           ::= '\' (x|X) HEX_CHAR [HEX_CHAR]]
 * HEX_CHAR      ::= (0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|a|b|c|d|e|f)
 *
 * INCLUDE_STRING is a filesystem path, with the following %-expansions:
 *     %% - '%'.
 *     %H - The user's home directory (the $HOME environment variable).
 *     %L - The name of the locale specific Compose file (e.g.,
 *          "/usr/share/X11/locale/<localename>/Compose").
 *     %S - The name of the system directory for Compose files (e.g.,
 *          "/usr/share/X11/locale").
 */

enum rules_token {
    TOK_END_OF_FILE = 0,
    TOK_END_OF_LINE,
    TOK_INCLUDE,
    TOK_INCLUDE_STRING,
    TOK_LHS_KEYSYM,
    TOK_COLON,
    TOK_BANG,
    TOK_TILDE,
    TOK_STRING,
    TOK_IDENT,
    TOK_ERROR
};

/* Values returned with some tokens, like yylval. */
union lvalue {
    struct {
        /* Still \0-terminated. */
        const char *str;
        size_t len;
    } string;
};

static enum rules_token
lex(struct scanner *s, union lvalue *val)
{
skip_more_whitespace_and_comments:
    /* Skip spaces. */
    while (is_space(scanner_peek(s)))
        if (scanner_next(s) == '\n')
            return TOK_END_OF_LINE;

    /* Skip comments. */
    if (scanner_chr(s, '#')) {
        scanner_skip_to_eol(s);
        goto skip_more_whitespace_and_comments;
    }

    /* See if we're done. */
    if (scanner_eof(s)) return TOK_END_OF_FILE;

    /* New token. */
    s->token_line = s->line;
    s->token_column = s->column;
    s->buf_pos = 0;

    /* LHS Keysym. */
    if (scanner_chr(s, '<')) {
        while (scanner_peek(s) != '>' && !scanner_eol(s) && !scanner_eof(s))
            scanner_buf_append(s, scanner_next(s));
        if (!scanner_chr(s, '>')) {
            scanner_err(s, "unterminated keysym literal");
            return TOK_ERROR;
        }
        if (!scanner_buf_append(s, '\0')) {
            scanner_err(s, "keysym literal is too long");
            return TOK_ERROR;
        }
        val->string.str = s->buf;
        val->string.len = s->buf_pos;
        return TOK_LHS_KEYSYM;
    }

    /* Colon. */
    if (scanner_chr(s, ':'))
        return TOK_COLON;
    if (scanner_chr(s, '!'))
        return TOK_BANG;
    if (scanner_chr(s, '~'))
        return TOK_TILDE;

    /* String literal. */
    if (scanner_chr(s, '\"')) {
        while (!scanner_eof(s) && !scanner_eol(s) && scanner_peek(s) != '\"') {
            if (scanner_chr(s, '\\')) {
                uint8_t o;
                size_t start_pos = s->pos;
                if (scanner_chr(s, '\\')) {
                    scanner_buf_append(s, '\\');
                }
                else if (scanner_chr(s, '"')) {
                    scanner_buf_append(s, '"');
                }
                else if (scanner_chr(s, 'x') || scanner_chr(s, 'X')) {
                    if (scanner_hex(s, &o) && is_valid_char((char) o)) {
                        scanner_buf_append(s, (char) o);
                    } else {
                        scanner_warn_with_code(s,
                            XKB_WARNING_INVALID_ESCAPE_SEQUENCE,
                            "illegal hexadecimal escape sequence (%.*s) in string literal",
                            (int) (s->pos - start_pos + 1), &s->s[start_pos - 1]);
                    }
                }
                else if (scanner_oct(s, &o) && is_valid_char((char) o)) {
                    scanner_buf_append(s, (char) o);
                }
                else if (s->pos > start_pos) {
                    scanner_warn_with_code(s,
                        XKB_WARNING_INVALID_ESCAPE_SEQUENCE,
                        "illegal octal escape sequence (%.*s) in string literal",
                        (int) (s->pos - start_pos + 1), &s->s[start_pos - 1]);
                    /* Ignore. */
                }
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
        if (!scanner_chr(s, '\"')) {
            scanner_err(s, "unterminated string literal");
            return TOK_ERROR;
        }
        if (!scanner_buf_append(s, '\0')) {
            scanner_err(s, "string literal is too long");
            return TOK_ERROR;
        }
        if (!is_valid_utf8(s->buf, s->buf_pos - 1)) {
            scanner_err(s, "string literal is not a valid UTF-8 string");
            return TOK_ERROR;
        }
        val->string.str = s->buf;
        val->string.len = s->buf_pos;
        return TOK_STRING;
    }

    /* Identifier or include. */
    if (is_alpha(scanner_peek(s)) || scanner_peek(s) == '_') {
        s->buf_pos = 0;
        while (is_alnum(scanner_peek(s)) || scanner_peek(s) == '_')
            scanner_buf_append(s, scanner_next(s));
        if (!scanner_buf_append(s, '\0')) {
            scanner_err(s, "identifier is too long");
            return TOK_ERROR;
        }

        if (streq(s->buf, "include"))
            return TOK_INCLUDE;

        val->string.str = s->buf;
        val->string.len = s->buf_pos;
        return TOK_IDENT;
    }

    /* Discard rest of line. */
    scanner_skip_to_eol(s);

    scanner_err(s, "unrecognized token");
    return TOK_ERROR;
}

static enum rules_token
lex_include_string(struct scanner *s, struct xkb_compose_table *table,
                   union lvalue *val_out)
{
    while (is_space(scanner_peek(s)))
        if (scanner_next(s) == '\n')
            return TOK_END_OF_LINE;

    s->token_line = s->line;
    s->token_column = s->column;
    s->buf_pos = 0;

    if (!scanner_chr(s, '\"')) {
        scanner_err(s, "include statement must be followed by a path");
        return TOK_ERROR;
    }

    while (!scanner_eof(s) && !scanner_eol(s) && scanner_peek(s) != '\"') {
        if (scanner_chr(s, '%')) {
            if (scanner_chr(s, '%')) {
                scanner_buf_append(s, '%');
            }
            else if (scanner_chr(s, 'H')) {
                const char *home = xkb_context_getenv(table->ctx, "HOME");
                if (!home) {
                    scanner_err(s, "%%H was used in an include statement, but the HOME environment variable is not set");
                    return TOK_ERROR;
                }
                if (!scanner_buf_appends(s, home)) {
                    scanner_err(s, "include path after expanding %%H is too long");
                    return TOK_ERROR;
                }
            }
            else if (scanner_chr(s, 'L')) {
                char *path = get_locale_compose_file_path(table->ctx, table->locale);
                if (!path) {
                    scanner_err(s, "failed to expand %%L to the locale Compose file");
                    return TOK_ERROR;
                }
                if (!scanner_buf_appends(s, path)) {
                    free(path);
                    scanner_err(s, "include path after expanding %%L is too long");
                    return TOK_ERROR;
                }
                free(path);
            }
            else if (scanner_chr(s, 'S')) {
                const char *xlocaledir = get_xlocaledir_path(table->ctx);
                if (!scanner_buf_appends(s, xlocaledir)) {
                    scanner_err(s, "include path after expanding %%S is too long");
                    return TOK_ERROR;
                }
            }
            else {
                scanner_err(s, "unknown %% format (%c) in include statement", scanner_peek(s));
                return TOK_ERROR;
            }
        } else {
            scanner_buf_append(s, scanner_next(s));
        }
    }
    if (!scanner_chr(s, '\"')) {
        scanner_err(s, "unterminated include statement");
        return TOK_ERROR;
    }
    if (!scanner_buf_append(s, '\0')) {
        scanner_err(s, "include path is too long");
        return TOK_ERROR;
    }
    val_out->string.str = s->buf;
    val_out->string.len = s->buf_pos;
    return TOK_INCLUDE_STRING;
}

struct production {
    xkb_keysym_t lhs[MAX_LHS_LEN];
    unsigned int len;
    xkb_keysym_t keysym;
    char string[256];
    /* At least one of these is true. */
    bool has_keysym;
    bool has_string;

    /* The matching is as follows: (active_mods & modmask) == mods. */
    xkb_mod_mask_t modmask;
    xkb_mod_mask_t mods;
};

static void
add_production(struct xkb_compose_table *table, struct scanner *s,
               const struct production *production)
{
    unsigned lhs_pos = 0;
    uint32_t curr = darray_size(table->nodes) == 1 ? 0 : 1;
    uint32_t *pptr = NULL;
    struct compose_node *node = NULL;

    /* Warn before potentially going over the limit, discard silently after. */
    if (darray_size(table->nodes) + production->len + MAX_LHS_LEN > MAX_COMPOSE_NODES)
        scanner_warn(s, "too many sequences for one Compose file; will ignore further lines");
    if (darray_size(table->nodes) + production->len >= MAX_COMPOSE_NODES)
        return;

    /*
     * Insert the sequence to the ternary search tree, creating new nodes as
     * needed.
     *
     * TODO: We insert in the order given, this means some inputs can create
     * long O(n) chains, which results in total O(n^2) parsing time. We should
     * ensure the tree is reasonably balanced somehow.
     */
    while (true) {
        const xkb_keysym_t keysym = production->lhs[lhs_pos];
        const bool last = lhs_pos + 1 == production->len;

        if (curr == 0) {
            /*
             * Create a new node and update the parent pointer to it.
             * Update the pointer first because the append invalidates it.
             */
            struct compose_node new = {
                .keysym = keysym,
                .lokid = 0,
                .hikid = 0,
                .internal = {
                    .eqkid = 0,
                    .is_leaf = false,
                },
            };
            curr = darray_size(table->nodes);
            if (pptr != NULL) {
                *pptr = curr;
                pptr = NULL;
            }
            darray_append(table->nodes, new);
        }

        node = &darray_item(table->nodes, curr);

        if (keysym < node->keysym) {
            pptr = &node->lokid;
            curr = node->lokid;
        } else if (keysym > node->keysym) {
            pptr = &node->hikid;
            curr = node->hikid;
        } else if (!last) {
            if (node->is_leaf) {
                scanner_warn(s, "a sequence already exists which is a prefix of this sequence; overriding");
                node->internal.eqkid = 0;
                node->internal.is_leaf = false;
            }
            lhs_pos++;
            pptr = &node->internal.eqkid;
            curr = node->internal.eqkid;
        } else {
            if (node->is_leaf) {
                bool same_string =
                    (node->leaf.utf8 == 0 && !production->has_string) ||
                    (
                        node->leaf.utf8 != 0 && production->has_string &&
                        streq(&darray_item(table->utf8, node->leaf.utf8),
                              production->string)
                    );
                bool same_keysym =
                    (node->leaf.keysym == XKB_KEY_NoSymbol && !production->has_keysym) ||
                    (
                        node->leaf.keysym != XKB_KEY_NoSymbol && production->has_keysym &&
                        node->leaf.keysym == production->keysym
                    );
                if (same_string && same_keysym) {
                    scanner_warn(s, "this compose sequence is a duplicate of another; skipping line");
                    return;
                } else {
                    scanner_warn(s, "this compose sequence already exists; overriding");
                }
            } else if (node->internal.eqkid != 0) {
                scanner_warn(s, "this compose sequence is a prefix of another; skipping line");
                return;
            }
            node->is_leaf = true;
            if (production->has_string) {
                node->leaf.utf8 = darray_size(table->utf8);
                darray_append_items(table->utf8, production->string,
                                    strlen(production->string) + 1);
            }
            if (production->has_keysym) {
                node->leaf.keysym = production->keysym;
            }
            return;
        }
    }
}

/* Should match resolve_modifier(). */
#define ALL_MODS_MASK ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3))

static xkb_mod_index_t
resolve_modifier(const char *name)
{
    static const struct {
        const char *name;
        xkb_mod_index_t mod;
    } mods[] = {
        { "Shift", 0 },
        { "Ctrl", 2 },
        { "Alt", 3 },
        { "Meta", 3 },
        { "Lock", 1 },
        { "Caps", 1 },
    };

    for (unsigned i = 0; i < ARRAY_SIZE(mods); i++)
        if (streq(name, mods[i].name))
            return mods[i].mod;

    return XKB_MOD_INVALID;
}

static bool
parse(struct xkb_compose_table *table, struct scanner *s,
      unsigned include_depth);

static bool
do_include(struct xkb_compose_table *table, struct scanner *s,
           const char *path, unsigned include_depth)
{
    FILE *file;
    bool ok;
    char *string;
    size_t size;
    struct scanner new_s;

    if (include_depth >= MAX_INCLUDE_DEPTH) {
        scanner_err(s, "maximum include depth (%d) exceeded; maybe there is an include loop?",
                    MAX_INCLUDE_DEPTH);
        return false;
    }

    file = fopen(path, "rb");
    if (!file) {
        scanner_err(s, "failed to open included Compose file \"%s\": %s",
                    path, strerror(errno));
        return false;
    }

    ok = map_file(file, &string, &size);
    if (!ok) {
        scanner_err(s, "failed to read included Compose file \"%s\": %s",
                    path, strerror(errno));
        goto err_file;
    }

    scanner_init(&new_s, table->ctx, string, size, path, s->priv);

    ok = parse(table, &new_s, include_depth + 1);
    if (!ok)
        goto err_unmap;

err_unmap:
    unmap_file(string, size);
err_file:
    fclose(file);
    return ok;
}

static bool
parse(struct xkb_compose_table *table, struct scanner *s,
      unsigned include_depth)
{
    enum rules_token tok;
    union lvalue val;
    xkb_keysym_t keysym;
    struct production production;
    enum { MAX_ERRORS = 10 };
    int num_errors = 0;

initial:
    production.len = 0;
    production.has_keysym = false;
    production.has_string = false;
    production.mods = 0;
    production.modmask = 0;

    /* fallthrough */

initial_eol:
    switch (tok = lex(s, &val)) {
    case TOK_END_OF_LINE:
        goto initial_eol;
    case TOK_END_OF_FILE:
        goto finished;
    case TOK_INCLUDE:
        goto include;
    default:
        goto lhs_tok;
    }

include:
    switch (tok = lex_include_string(s, table, &val)) {
    case TOK_INCLUDE_STRING:
        goto include_eol;
    default:
        goto unexpected;
    }

include_eol:
    switch (tok = lex(s, &val)) {
    case TOK_END_OF_LINE:
        if (!do_include(table, s, val.string.str, include_depth))
            goto fail;
        goto initial;
    default:
        goto unexpected;
    }

lhs:
    tok = lex(s, &val);
lhs_tok:
    switch (tok) {
    case TOK_COLON:
        if (production.len <= 0) {
            scanner_warn(s, "expected at least one keysym on left-hand side; skipping line");
            goto skip;
        }
        goto rhs;
    case TOK_IDENT:
        if (streq(val.string.str, "None")) {
            production.mods = 0;
            production.modmask = ALL_MODS_MASK;
            goto lhs_keysym;
        }
        goto lhs_mod_list_tok;
    case TOK_TILDE:
        goto lhs_mod_list_tok;
    case TOK_BANG:
        production.modmask = ALL_MODS_MASK;
        goto lhs_mod_list;
    default:
        goto lhs_keysym_tok;
    }

lhs_keysym:
    tok = lex(s, &val);
lhs_keysym_tok:
    switch (tok) {
    case TOK_LHS_KEYSYM:
        keysym = xkb_keysym_from_name(val.string.str, XKB_KEYSYM_NO_FLAGS);
        if (keysym == XKB_KEY_NoSymbol) {
            scanner_err(s, "unrecognized keysym \"%s\" on left-hand side",
                        val.string.str);
            goto error;
        }
        if (production.len + 1 > MAX_LHS_LEN) {
            scanner_warn(s, "too many keysyms (%d) on left-hand side; skipping line",
                         MAX_LHS_LEN + 1);
            goto skip;
        }
        production.lhs[production.len++] = keysym;
        production.mods = 0;
        production.modmask = 0;
        goto lhs;
    default:
        goto unexpected;
    }

lhs_mod_list:
    tok = lex(s, &val);
lhs_mod_list_tok: {
        bool tilde = false;
        xkb_mod_index_t mod;

        if (tok != TOK_TILDE && tok != TOK_IDENT)
            goto lhs_keysym_tok;

        if (tok == TOK_TILDE) {
            tilde = true;
            tok = lex(s, &val);
        }

        if (tok != TOK_IDENT)
            goto unexpected;

        mod = resolve_modifier(val.string.str);
        if (mod == XKB_MOD_INVALID) {
            scanner_err(s, "unrecognized modifier \"%s\"",
                        val.string.str);
            goto error;
        }

        production.modmask |= 1 << mod;
        if (tilde)
            production.mods &= ~(1 << mod);
        else
            production.mods |= 1 << mod;

        goto lhs_mod_list;
    }

rhs:
    switch (tok = lex(s, &val)) {
    case TOK_STRING:
        if (production.has_string) {
            scanner_warn(s, "right-hand side can have at most one string; skipping line");
            goto skip;
        }
        if (val.string.len <= 0) {
            scanner_warn(s, "right-hand side string must not be empty; skipping line");
            goto skip;
        }
        if (val.string.len >= sizeof(production.string)) {
            scanner_warn(s, "right-hand side string is too long; skipping line");
            goto skip;
        }
        strcpy(production.string, val.string.str);
        production.has_string = true;
        goto rhs;
    case TOK_IDENT:
        keysym = xkb_keysym_from_name(val.string.str, XKB_KEYSYM_NO_FLAGS);
        if (keysym == XKB_KEY_NoSymbol) {
            scanner_err(s, "unrecognized keysym \"%s\" on right-hand side",
                        val.string.str);
            goto error;
        }
        if (production.has_keysym) {
            scanner_warn(s, "right-hand side can have at most one keysym; skipping line");
            goto skip;
        }
        production.keysym = keysym;
        production.has_keysym = true;
        /* fallthrough */
    case TOK_END_OF_LINE:
        if (!production.has_string && !production.has_keysym) {
            scanner_warn(s, "right-hand side must have at least one of string or keysym; skipping line");
            goto skip;
        }
        add_production(table, s, &production);
        goto initial;
    default:
        goto unexpected;
    }

unexpected:
    if (tok != TOK_ERROR)
        scanner_err(s, "unexpected token");
error:
    num_errors++;
    if (num_errors <= MAX_ERRORS)
        goto skip;

    scanner_err(s, "too many errors");
    goto fail;

fail:
    scanner_err(s, "failed to parse file");
    return false;

skip:
    while (tok != TOK_END_OF_LINE && tok != TOK_END_OF_FILE)
        tok = lex(s, &val);
    goto initial;

finished:
    return true;
}

bool
parse_string(struct xkb_compose_table *table, const char *string, size_t len,
             const char *file_name)
{
    struct scanner s;
    scanner_init(&s, table->ctx, string, len, file_name, NULL);
    if (!parse(table, &s, 0))
        return false;
    /* Maybe the allocator can use the excess space. */
    darray_shrink(table->nodes);
    darray_shrink(table->utf8);
    return true;
}

bool
parse_file(struct xkb_compose_table *table, FILE *file, const char *file_name)
{
    bool ok;
    char *string;
    size_t size;

    ok = map_file(file, &string, &size);
    if (!ok) {
        log_err(table->ctx,
                XKB_LOG_MESSAGE_NO_ID,
                "Couldn't read Compose file %s: %s\n",
                file_name, strerror(errno));
        return false;
    }

    ok = parse_string(table, string, size, file_name);
    unmap_file(string, size);
    return ok;
}
