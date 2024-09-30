/************************************************************
 * Copyright (c) 1996 by Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of Silicon Graphics not be
 * used in advertising or publicity pertaining to distribution
 * of the software without specific prior written permission.
 * Silicon Graphics makes no representation about the suitability
 * of this software for any purpose. It is provided "as is"
 * without any express or implied warranty.
 *
 * SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 * GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 ********************************************************/

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
#include "rules.h"
#include "include.h"
#include "scanner-utils.h"

#define MAX_INCLUDE_DEPTH 5

/* Scanner / Lexer */

/* Values returned with some tokens, like yylval. */
union lvalue {
    struct sval string;
};

enum rules_token {
    TOK_END_OF_FILE = 0,
    TOK_END_OF_LINE,
    TOK_IDENTIFIER,
    TOK_GROUP_NAME,
    TOK_BANG,
    TOK_EQUALS,
    TOK_STAR,
    TOK_INCLUDE,
    TOK_ERROR
};

static inline bool
is_ident(char ch)
{
    return is_graph(ch) && ch != '\\';
}

static enum rules_token
lex(struct scanner *s, union lvalue *val)
{
skip_more_whitespace_and_comments:
    /* Skip spaces. */
    while (scanner_chr(s, ' ') || scanner_chr(s, '\t') || scanner_chr(s, '\r'));

    /* Skip comments. */
    if (scanner_lit(s, "//")) {
        scanner_skip_to_eol(s);
    }

    /* New line. */
    if (scanner_eol(s)) {
        while (scanner_eol(s)) scanner_next(s);
        return TOK_END_OF_LINE;
    }

    /* Escaped line continuation. */
    if (scanner_chr(s, '\\')) {
        /* Optional \r. */
        scanner_chr(s, '\r');
        if (!scanner_eol(s)) {
            scanner_err(s, "illegal new line escape; must appear at end of line");
            return TOK_ERROR;
        }
        scanner_next(s);
        goto skip_more_whitespace_and_comments;
    }

    /* See if we're done. */
    if (scanner_eof(s)) return TOK_END_OF_FILE;

    /* New token. */
    s->token_line = s->line;
    s->token_column = s->column;

    /* Operators and punctuation. */
    if (scanner_chr(s, '!')) return TOK_BANG;
    if (scanner_chr(s, '=')) return TOK_EQUALS;
    if (scanner_chr(s, '*')) return TOK_STAR;

    /* Group name. */
    if (scanner_chr(s, '$')) {
        val->string.start = s->s + s->pos;
        val->string.len = 0;
        while (is_ident(scanner_peek(s))) {
            scanner_next(s);
            val->string.len++;
        }
        if (val->string.len == 0) {
            scanner_err(s, "unexpected character after \'$\'; expected name");
            return TOK_ERROR;
        }
        return TOK_GROUP_NAME;
    }

    /* Include statement. */
    if (scanner_lit(s, "include"))
        return TOK_INCLUDE;

    /* Identifier. */
    if (is_ident(scanner_peek(s))) {
        val->string.start = s->s + s->pos;
        val->string.len = 0;
        while (is_ident(scanner_peek(s))) {
            scanner_next(s);
            val->string.len++;
        }
        return TOK_IDENTIFIER;
    }

    scanner_err(s, "unrecognized token");
    return TOK_ERROR;
}

/***====================================================================***/

enum rules_mlvo {
    MLVO_MODEL,
    MLVO_LAYOUT,
    MLVO_VARIANT,
    MLVO_OPTION,
    _MLVO_NUM_ENTRIES
};

#define SVAL_LIT(literal) { literal, sizeof(literal) - 1 }

static const struct sval rules_mlvo_svals[_MLVO_NUM_ENTRIES] = {
    [MLVO_MODEL] = SVAL_LIT("model"),
    [MLVO_LAYOUT] = SVAL_LIT("layout"),
    [MLVO_VARIANT] = SVAL_LIT("variant"),
    [MLVO_OPTION] = SVAL_LIT("option"),
};

enum rules_kccgst {
    KCCGST_KEYCODES,
    KCCGST_TYPES,
    KCCGST_COMPAT,
    KCCGST_SYMBOLS,
    KCCGST_GEOMETRY,
    _KCCGST_NUM_ENTRIES
};

static const struct sval rules_kccgst_svals[_KCCGST_NUM_ENTRIES] = {
    [KCCGST_KEYCODES] = SVAL_LIT("keycodes"),
    [KCCGST_TYPES] = SVAL_LIT("types"),
    [KCCGST_COMPAT] = SVAL_LIT("compat"),
    [KCCGST_SYMBOLS] = SVAL_LIT("symbols"),
    [KCCGST_GEOMETRY] = SVAL_LIT("geometry"),
};

/* We use this to keep score whether an mlvo was matched or not; if not,
 * we warn the user that his preference was ignored. */
struct matched_sval {
    struct sval sval;
    bool matched;
};
typedef darray(struct matched_sval) darray_matched_sval;

/*
 * A broken-down version of xkb_rule_names (without the rules,
 * obviously).
 */
struct rule_names {
    struct matched_sval model;
    darray_matched_sval layouts;
    darray_matched_sval variants;
    darray_matched_sval options;
};

struct group {
    struct sval name;
    darray_sval elements;
};

struct mapping {
    int mlvo_at_pos[_MLVO_NUM_ENTRIES];
    unsigned int num_mlvo;
    unsigned int defined_mlvo_mask;
    xkb_layout_index_t layout_idx, variant_idx;
    int kccgst_at_pos[_KCCGST_NUM_ENTRIES];
    unsigned int num_kccgst;
    unsigned int defined_kccgst_mask;
    bool skip;
};

enum mlvo_match_type {
    MLVO_MATCH_NORMAL = 0,
    MLVO_MATCH_WILDCARD,
    MLVO_MATCH_GROUP,
};

struct rule {
    struct sval mlvo_value_at_pos[_MLVO_NUM_ENTRIES];
    enum mlvo_match_type match_type_at_pos[_MLVO_NUM_ENTRIES];
    unsigned int num_mlvo_values;
    struct sval kccgst_value_at_pos[_KCCGST_NUM_ENTRIES];
    unsigned int num_kccgst_values;
    bool skip;
};

/*
 * This is the main object used to match a given RMLVO against a rules
 * file and aggragate the results in a KcCGST. It goes through a simple
 * matching state machine, with tokens as transitions (see
 * matcher_match()).
 */
struct matcher {
    struct xkb_context *ctx;
    /* Input.*/
    struct rule_names rmlvo;
    union lvalue val;
    darray(struct group) groups;
    /* Current mapping. */
    struct mapping mapping;
    /* Current rule. */
    struct rule rule;
    /* Output. */
    darray_char kccgst[_KCCGST_NUM_ENTRIES];
};

static struct sval
strip_spaces(struct sval v)
{
    while (v.len > 0 && is_space(v.start[0])) { v.len--; v.start++; }
    while (v.len > 0 && is_space(v.start[v.len - 1])) v.len--;
    return v;
}

static darray_matched_sval
split_comma_separated_mlvo(const char *s)
{
    darray_matched_sval arr = darray_new();

    /*
     * Make sure the array returned by this function always includes at
     * least one value, e.g. "" -> { "" } and "," -> { "", "" }.
     */

    if (!s) {
        struct matched_sval val = { .sval = { NULL, 0 } };
        darray_append(arr, val);
        return arr;
    }

    while (true) {
        struct matched_sval val = { .sval = { s, 0 } };
        while (*s != '\0' && *s != ',') { s++; val.sval.len++; }
        val.sval = strip_spaces(val.sval);
        darray_append(arr, val);
        if (*s == '\0') break;
        if (*s == ',') s++;
    }

    return arr;
}

static struct matcher *
matcher_new(struct xkb_context *ctx,
            const struct xkb_rule_names *rmlvo)
{
    struct matcher *m = calloc(1, sizeof(*m));
    if (!m)
        return NULL;

    m->ctx = ctx;
    m->rmlvo.model.sval.start = rmlvo->model;
    m->rmlvo.model.sval.len = strlen_safe(rmlvo->model);
    m->rmlvo.layouts = split_comma_separated_mlvo(rmlvo->layout);
    m->rmlvo.variants = split_comma_separated_mlvo(rmlvo->variant);
    m->rmlvo.options = split_comma_separated_mlvo(rmlvo->options);

    return m;
}

static void
matcher_free(struct matcher *m)
{
    struct group *group;
    if (!m)
        return;
    darray_free(m->rmlvo.layouts);
    darray_free(m->rmlvo.variants);
    darray_free(m->rmlvo.options);
    darray_foreach(group, m->groups)
        darray_free(group->elements);
    for (int i = 0; i < _KCCGST_NUM_ENTRIES; i++)
        darray_free(m->kccgst[i]);
    darray_free(m->groups);
    free(m);
}

static void
matcher_group_start_new(struct matcher *m, struct sval name)
{
    struct group group = { .name = name, .elements = darray_new() };
    darray_append(m->groups, group);
}

static void
matcher_group_add_element(struct matcher *m, struct scanner *s,
                          struct sval element)
{
    darray_append(darray_item(m->groups, darray_size(m->groups) - 1).elements,
                  element);
}

static bool
read_rules_file(struct xkb_context *ctx,
                struct matcher *matcher,
                unsigned include_depth,
                FILE *file,
                const char *path);

static void
matcher_include(struct matcher *m, struct scanner *parent_scanner,
                unsigned include_depth,
                struct sval inc)
{
    struct scanner s; /* parses the !include value */
    FILE *file;

    scanner_init(&s, m->ctx, inc.start, inc.len,
                 parent_scanner->file_name, NULL);
    s.token_line = parent_scanner->token_line;
    s.token_column = parent_scanner->token_column;
    s.buf_pos = 0;

    if (include_depth >= MAX_INCLUDE_DEPTH) {
        scanner_err(&s, "maximum include depth (%d) exceeded; maybe there is an include loop?",
                    MAX_INCLUDE_DEPTH);
        return;
    }

    while (!scanner_eof(&s) && !scanner_eol(&s)) {
        if (scanner_chr(&s, '%')) {
            if (scanner_chr(&s, '%')) {
                scanner_buf_append(&s, '%');
            }
            else if (scanner_chr(&s, 'H')) {
                const char *home = xkb_context_getenv(m->ctx, "HOME");
                if (!home) {
                    scanner_err(&s, "%%H was used in an include statement, but the HOME environment variable is not set");
                    return;
                }
                if (!scanner_buf_appends(&s, home)) {
                    scanner_err(&s, "include path after expanding %%H is too long");
                    return;
                }
            }
            else if (scanner_chr(&s, 'S')) {
                const char *default_root = xkb_context_include_path_get_system_path(m->ctx);
                if (!scanner_buf_appends(&s, default_root) || !scanner_buf_appends(&s, "/rules")) {
                    scanner_err(&s, "include path after expanding %%S is too long");
                    return;
                }
            }
            else if (scanner_chr(&s, 'E')) {
                const char *default_root = xkb_context_include_path_get_extra_path(m->ctx);
                if (!scanner_buf_appends(&s, default_root) || !scanner_buf_appends(&s, "/rules")) {
                    scanner_err(&s, "include path after expanding %%E is too long");
                    return;
                }
            }
            else {
                scanner_err(&s, "unknown %% format (%c) in include statement", scanner_peek(&s));
                return;
            }
        }
        else {
            scanner_buf_append(&s, scanner_next(&s));
        }
    }
    if (!scanner_buf_append(&s, '\0')) {
        scanner_err(&s, "include path is too long");
        return;
    }

    file = fopen(s.buf, "rb");
    if (file) {
        bool ret = read_rules_file(m->ctx, m, include_depth + 1, file, s.buf);
        if (!ret)
            log_err(m->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "No components returned from included XKB rules \"%s\"\n",
                    s.buf);
        fclose(file);
    } else {
        log_err(m->ctx, XKB_LOG_MESSAGE_NO_ID,
                "Failed to open included XKB rules \"%s\"\n",
                s.buf);
    }
}

static void
matcher_mapping_start_new(struct matcher *m)
{
    for (unsigned i = 0; i < _MLVO_NUM_ENTRIES; i++)
        m->mapping.mlvo_at_pos[i] = -1;
    for (unsigned i = 0; i < _KCCGST_NUM_ENTRIES; i++)
        m->mapping.kccgst_at_pos[i] = -1;
    m->mapping.layout_idx = m->mapping.variant_idx = XKB_LAYOUT_INVALID;
    m->mapping.num_mlvo = m->mapping.num_kccgst = 0;
    m->mapping.defined_mlvo_mask = 0;
    m->mapping.defined_kccgst_mask = 0;
    m->mapping.skip = false;
}

static int
extract_layout_index(const char *s, size_t max_len, xkb_layout_index_t *out)
{
    /* This function is pretty stupid, but works for now. */
    *out = XKB_LAYOUT_INVALID;
    if (max_len < 3)
        return -1;
    if (s[0] != '[' || !is_digit(s[1]) || s[2] != ']')
        return -1;
    if (s[1] - '0' < 1 || s[1] - '0' > XKB_MAX_GROUPS)
        return -1;
    /* To zero-based index. */
    *out = s[1] - '0' - 1;
    return 3;
}

static void
matcher_mapping_set_mlvo(struct matcher *m, struct scanner *s,
                         struct sval ident)
{
    enum rules_mlvo mlvo;
    struct sval mlvo_sval;

    for (mlvo = 0; mlvo < _MLVO_NUM_ENTRIES; mlvo++) {
        mlvo_sval = rules_mlvo_svals[mlvo];

        if (svaleq_prefix(mlvo_sval, ident))
            break;
    }

    /* Not found. */
    if (mlvo >= _MLVO_NUM_ENTRIES) {
        scanner_err(s, "invalid mapping: %.*s is not a valid value here; ignoring rule set",
                    ident.len, ident.start);
        m->mapping.skip = true;
        return;
    }

    if (m->mapping.defined_mlvo_mask & (1u << mlvo)) {
        scanner_err(s, "invalid mapping: %.*s appears twice on the same line; ignoring rule set",
                    mlvo_sval.len, mlvo_sval.start);
        m->mapping.skip = true;
        return;
    }

    /* If there are leftovers still, it must be an index. */
    if (mlvo_sval.len < ident.len) {
        xkb_layout_index_t idx;
        int consumed = extract_layout_index(ident.start + mlvo_sval.len,
                                            ident.len - mlvo_sval.len, &idx);
        if ((int) (ident.len - mlvo_sval.len) != consumed) {
            scanner_err(s, "invalid mapping: \"%.*s\" may only be followed by a valid group index; ignoring rule set",
                        mlvo_sval.len, mlvo_sval.start);
            m->mapping.skip = true;
            return;
        }

        if (mlvo == MLVO_LAYOUT) {
            m->mapping.layout_idx = idx;
        }
        else if (mlvo == MLVO_VARIANT) {
            m->mapping.variant_idx = idx;
        }
        else {
            scanner_err(s, "invalid mapping: \"%.*s\" cannot be followed by a group index; ignoring rule set",
                        mlvo_sval.len, mlvo_sval.start);
            m->mapping.skip = true;
            return;
        }
    }

    m->mapping.mlvo_at_pos[m->mapping.num_mlvo] = mlvo;
    m->mapping.defined_mlvo_mask |= 1u << mlvo;
    m->mapping.num_mlvo++;
}

static void
matcher_mapping_set_kccgst(struct matcher *m, struct scanner *s, struct sval ident)
{
    enum rules_kccgst kccgst;
    struct sval kccgst_sval;

    for (kccgst = 0; kccgst < _KCCGST_NUM_ENTRIES; kccgst++) {
        kccgst_sval = rules_kccgst_svals[kccgst];

        if (svaleq(rules_kccgst_svals[kccgst], ident))
            break;
    }

    /* Not found. */
    if (kccgst >= _KCCGST_NUM_ENTRIES) {
        scanner_err(s, "invalid mapping: %.*s is not a valid value here; ignoring rule set",
                    ident.len, ident.start);
        m->mapping.skip = true;
        return;
    }

    if (m->mapping.defined_kccgst_mask & (1u << kccgst)) {
        scanner_err(s, "invalid mapping: %.*s appears twice on the same line; ignoring rule set",
                    kccgst_sval.len, kccgst_sval.start);
        m->mapping.skip = true;
        return;
    }

    m->mapping.kccgst_at_pos[m->mapping.num_kccgst] = kccgst;
    m->mapping.defined_kccgst_mask |= 1u << kccgst;
    m->mapping.num_kccgst++;
}

static void
matcher_mapping_verify(struct matcher *m, struct scanner *s)
{
    if (m->mapping.num_mlvo == 0) {
        scanner_err(s, "invalid mapping: must have at least one value on the left hand side; ignoring rule set");
        goto skip;
    }

    if (m->mapping.num_kccgst == 0) {
        scanner_err(s, "invalid mapping: must have at least one value on the right hand side; ignoring rule set");
        goto skip;
    }

    /*
     * This following is very stupid, but this is how it works.
     * See the "Notes" section in the overview above.
     */

    if (m->mapping.defined_mlvo_mask & (1u << MLVO_LAYOUT)) {
        if (m->mapping.layout_idx == XKB_LAYOUT_INVALID) {
            if (darray_size(m->rmlvo.layouts) > 1)
                goto skip;
        }
        else {
            if (darray_size(m->rmlvo.layouts) == 1 ||
                m->mapping.layout_idx >= darray_size(m->rmlvo.layouts))
                goto skip;
        }
    }

    if (m->mapping.defined_mlvo_mask & (1u << MLVO_VARIANT)) {
        if (m->mapping.variant_idx == XKB_LAYOUT_INVALID) {
            if (darray_size(m->rmlvo.variants) > 1)
                goto skip;
        }
        else {
            if (darray_size(m->rmlvo.variants) == 1 ||
                m->mapping.variant_idx >= darray_size(m->rmlvo.variants))
                goto skip;
        }
    }

    return;

skip:
    m->mapping.skip = true;
}

static void
matcher_rule_start_new(struct matcher *m)
{
    memset(&m->rule, 0, sizeof(m->rule));
    m->rule.skip = m->mapping.skip;
}

static void
matcher_rule_set_mlvo_common(struct matcher *m, struct scanner *s,
                             struct sval ident,
                             enum mlvo_match_type match_type)
{
    if (m->rule.num_mlvo_values + 1 > m->mapping.num_mlvo) {
        scanner_err(s, "invalid rule: has more values than the mapping line; ignoring rule");
        m->rule.skip = true;
        return;
    }
    m->rule.match_type_at_pos[m->rule.num_mlvo_values] = match_type;
    m->rule.mlvo_value_at_pos[m->rule.num_mlvo_values] = ident;
    m->rule.num_mlvo_values++;
}

static void
matcher_rule_set_mlvo_wildcard(struct matcher *m, struct scanner *s)
{
    struct sval dummy = { NULL, 0 };
    matcher_rule_set_mlvo_common(m, s, dummy, MLVO_MATCH_WILDCARD);
}

static void
matcher_rule_set_mlvo_group(struct matcher *m, struct scanner *s,
                            struct sval ident)
{
    matcher_rule_set_mlvo_common(m, s, ident, MLVO_MATCH_GROUP);
}

static void
matcher_rule_set_mlvo(struct matcher *m, struct scanner *s,
                      struct sval ident)
{
    matcher_rule_set_mlvo_common(m, s, ident, MLVO_MATCH_NORMAL);
}

static void
matcher_rule_set_kccgst(struct matcher *m, struct scanner *s,
                        struct sval ident)
{
    if (m->rule.num_kccgst_values + 1 > m->mapping.num_kccgst) {
        scanner_err(s, "invalid rule: has more values than the mapping line; ignoring rule");
        m->rule.skip = true;
        return;
    }
    m->rule.kccgst_value_at_pos[m->rule.num_kccgst_values] = ident;
    m->rule.num_kccgst_values++;
}

static bool
match_group(struct matcher *m, struct sval group_name, struct sval to)
{
    struct group *group;
    struct sval *element;
    bool found = false;

    darray_foreach(group, m->groups) {
        if (svaleq(group->name, group_name)) {
            found = true;
            break;
        }
    }

    if (!found) {
        /*
         * rules/evdev intentionally uses some undeclared group names
         * in rules (e.g. commented group definitions which may be
         * uncommented if needed). So we continue silently.
         */
        return false;
    }

    darray_foreach(element, group->elements)
        if (svaleq(to, *element))
            return true;

    return false;
}

static bool
match_value(struct matcher *m, struct sval val, struct sval to,
            enum mlvo_match_type match_type)
{
    if (match_type == MLVO_MATCH_WILDCARD)
        return true;
    if (match_type == MLVO_MATCH_GROUP)
        return match_group(m, val, to);
    return svaleq(val, to);
}

static bool
match_value_and_mark(struct matcher *m, struct sval val,
                     struct matched_sval *to, enum mlvo_match_type match_type)
{
    bool matched = match_value(m, val, to->sval, match_type);
    if (matched)
        to->matched = true;
    return matched;
}

/*
 * This function performs %-expansion on @value (see overview above),
 * and appends the result to @to.
 */
static bool
append_expanded_kccgst_value(struct matcher *m, struct scanner *s,
                             darray_char *to, struct sval value)
{
    const char *str = value.start;
    darray_char expanded = darray_new();
    char ch;
    bool expanded_plus, to_plus;

    /*
     * Some ugly hand-lexing here, but going through the scanner is more
     * trouble than it's worth, and the format is ugly on its own merit.
     */
    for (unsigned i = 0; i < value.len; ) {
        enum rules_mlvo mlv;
        xkb_layout_index_t idx;
        char pfx, sfx;
        struct matched_sval *expanded_value;

        /* Check if that's a start of an expansion. */
        if (str[i] != '%') {
            /* Just a normal character. */
            darray_appends_nullterminate(expanded, &str[i++], 1);
            continue;
        }
        if (++i >= value.len) goto error;

        pfx = sfx = 0;

        /* Check for prefix. */
        if (str[i] == '(' || str[i] == '+' || str[i] == '|' ||
            str[i] == '_' || str[i] == '-') {
            pfx = str[i];
            if (str[i] == '(') sfx = ')';
            if (++i >= value.len) goto error;
        }

        /* Mandatory model/layout/variant specifier. */
        switch (str[i++]) {
        case 'm': mlv = MLVO_MODEL; break;
        case 'l': mlv = MLVO_LAYOUT; break;
        case 'v': mlv = MLVO_VARIANT; break;
        default: goto error;
        }

        /* Check for index. */
        idx = XKB_LAYOUT_INVALID;
        if (i < value.len && str[i] == '[') {
            int consumed;

            if (mlv != MLVO_LAYOUT && mlv != MLVO_VARIANT) {
                scanner_err(s, "invalid index in %%-expansion; may only index layout or variant");
                goto error;
            }

            consumed = extract_layout_index(str + i, value.len - i, &idx);
            if (consumed == -1) goto error;
            i += consumed;
        }

        /* Check for suffix, if there supposed to be one. */
        if (sfx != 0) {
            if (i >= value.len) goto error;
            if (str[i++] != sfx) goto error;
        }

        /* Get the expanded value. */
        expanded_value = NULL;

        if (mlv == MLVO_LAYOUT) {
            if (idx != XKB_LAYOUT_INVALID &&
                idx < darray_size(m->rmlvo.layouts) &&
                darray_size(m->rmlvo.layouts) > 1)
                expanded_value = &darray_item(m->rmlvo.layouts, idx);
            else if (idx == XKB_LAYOUT_INVALID &&
                     darray_size(m->rmlvo.layouts) == 1)
                expanded_value = &darray_item(m->rmlvo.layouts, 0);
        }
        else if (mlv == MLVO_VARIANT) {
            if (idx != XKB_LAYOUT_INVALID &&
                idx < darray_size(m->rmlvo.variants) &&
                darray_size(m->rmlvo.variants) > 1)
                expanded_value = &darray_item(m->rmlvo.variants, idx);
            else if (idx == XKB_LAYOUT_INVALID &&
                     darray_size(m->rmlvo.variants) == 1)
                expanded_value = &darray_item(m->rmlvo.variants, 0);
        }
        else if (mlv == MLVO_MODEL) {
            expanded_value = &m->rmlvo.model;
        }

        /* If we didn't get one, skip silently. */
        if (!expanded_value || expanded_value->sval.len == 0)
            continue;

        if (pfx != 0)
            darray_appends_nullterminate(expanded, &pfx, 1);
        darray_appends_nullterminate(expanded,
                                     expanded_value->sval.start,
                                     expanded_value->sval.len);
        if (sfx != 0)
            darray_appends_nullterminate(expanded, &sfx, 1);
        expanded_value->matched = true;
    }

    /*
     * Appending  bar to  foo ->  foo (not an error if this happens)
     * Appending +bar to  foo ->  foo+bar
     * Appending  bar to +foo ->  bar+foo
     * Appending +bar to +foo -> +foo+bar
     */

    ch = (darray_empty(expanded) ? '\0' : darray_item(expanded, 0));
    expanded_plus = (ch == '+' || ch == '|');
    ch = (darray_empty(*to) ? '\0' : darray_item(*to, 0));
    to_plus = (ch == '+' || ch == '|');

    if (expanded_plus || darray_empty(*to))
        darray_appends_nullterminate(*to, expanded.item, expanded.size);
    else if (to_plus)
        darray_prepends_nullterminate(*to, expanded.item, expanded.size);

    darray_free(expanded);
    return true;

error:
    darray_free(expanded);
    scanner_err(s, "invalid %%-expansion in value; not used");
    return false;
}

static void
matcher_rule_verify(struct matcher *m, struct scanner *s)
{
    if (m->rule.num_mlvo_values != m->mapping.num_mlvo ||
        m->rule.num_kccgst_values != m->mapping.num_kccgst) {
        scanner_err(s, "invalid rule: must have same number of values as mapping line; ignoring rule");
        m->rule.skip = true;
    }
}

static void
matcher_rule_apply_if_matches(struct matcher *m, struct scanner *s)
{
    for (unsigned i = 0; i < m->mapping.num_mlvo; i++) {
        enum rules_mlvo mlvo = m->mapping.mlvo_at_pos[i];
        struct sval value = m->rule.mlvo_value_at_pos[i];
        enum mlvo_match_type match_type = m->rule.match_type_at_pos[i];
        struct matched_sval *to;
        bool matched = false;

        if (mlvo == MLVO_MODEL) {
            to = &m->rmlvo.model;
            matched = match_value_and_mark(m, value, to, match_type);
        }
        else if (mlvo == MLVO_LAYOUT) {
            xkb_layout_index_t idx = m->mapping.layout_idx;
            idx = (idx == XKB_LAYOUT_INVALID ? 0 : idx);
            to = &darray_item(m->rmlvo.layouts, idx);
            matched = match_value_and_mark(m, value, to, match_type);
        }
        else if (mlvo == MLVO_VARIANT) {
            xkb_layout_index_t idx = m->mapping.layout_idx;
            idx = (idx == XKB_LAYOUT_INVALID ? 0 : idx);
            to = &darray_item(m->rmlvo.variants, idx);
            matched = match_value_and_mark(m, value, to, match_type);
        }
        else if (mlvo == MLVO_OPTION) {
            darray_foreach(to, m->rmlvo.options) {
                matched = match_value_and_mark(m, value, to, match_type);
                if (matched)
                    break;
            }
        }

        if (!matched)
            return;
    }

    for (unsigned i = 0; i < m->mapping.num_kccgst; i++) {
        enum rules_kccgst kccgst = m->mapping.kccgst_at_pos[i];
        struct sval value = m->rule.kccgst_value_at_pos[i];
        append_expanded_kccgst_value(m, s, &m->kccgst[kccgst], value);
    }

    /*
     * If a rule matches in a rule set, the rest of the set should be
     * skipped. However, rule sets matching against options may contain
     * several legitimate rules, so they are processed entirely.
     */
    if (!(m->mapping.defined_mlvo_mask & (1 << MLVO_OPTION)))
        m->mapping.skip = true;
}

static enum rules_token
gettok(struct matcher *m, struct scanner *s)
{
    return lex(s, &m->val);
}

static bool
matcher_match(struct matcher *m, struct scanner *s,
              unsigned include_depth,
              const char *string, size_t len,
              const char *file_name)
{
    enum rules_token tok;

    if (!m)
        return false;

initial:
    switch (tok = gettok(m, s)) {
    case TOK_BANG:
        goto bang;
    case TOK_END_OF_LINE:
        goto initial;
    case TOK_END_OF_FILE:
        goto finish;
    default:
        goto unexpected;
    }

bang:
    switch (tok = gettok(m, s)) {
    case TOK_GROUP_NAME:
        matcher_group_start_new(m, m->val.string);
        goto group_name;
    case TOK_INCLUDE:
        goto include_statement;
    case TOK_IDENTIFIER:
        matcher_mapping_start_new(m);
        matcher_mapping_set_mlvo(m, s, m->val.string);
        goto mapping_mlvo;
    default:
        goto unexpected;
    }

group_name:
    switch (tok = gettok(m, s)) {
    case TOK_EQUALS:
        goto group_element;
    default:
        goto unexpected;
    }

group_element:
    switch (tok = gettok(m, s)) {
    case TOK_IDENTIFIER:
        matcher_group_add_element(m, s, m->val.string);
        goto group_element;
    case TOK_END_OF_LINE:
        goto initial;
    default:
        goto unexpected;
    }

include_statement:
    switch (tok = gettok(m, s)) {
    case TOK_IDENTIFIER:
        matcher_include(m, s, include_depth, m->val.string);
        goto initial;
    default:
        goto unexpected;
    }

mapping_mlvo:
    switch (tok = gettok(m, s)) {
    case TOK_IDENTIFIER:
        if (!m->mapping.skip)
            matcher_mapping_set_mlvo(m, s, m->val.string);
        goto mapping_mlvo;
    case TOK_EQUALS:
        goto mapping_kccgst;
    default:
        goto unexpected;
    }

mapping_kccgst:
    switch (tok = gettok(m, s)) {
    case TOK_IDENTIFIER:
        if (!m->mapping.skip)
            matcher_mapping_set_kccgst(m, s, m->val.string);
        goto mapping_kccgst;
    case TOK_END_OF_LINE:
        if (!m->mapping.skip)
            matcher_mapping_verify(m, s);
        goto rule_mlvo_first;
    default:
        goto unexpected;
    }

rule_mlvo_first:
    switch (tok = gettok(m, s)) {
    case TOK_BANG:
        goto bang;
    case TOK_END_OF_LINE:
        goto rule_mlvo_first;
    case TOK_END_OF_FILE:
        goto finish;
    default:
        matcher_rule_start_new(m);
        goto rule_mlvo_no_tok;
    }

rule_mlvo:
    tok = gettok(m, s);
rule_mlvo_no_tok:
    switch (tok) {
    case TOK_IDENTIFIER:
        if (!m->rule.skip)
            matcher_rule_set_mlvo(m, s, m->val.string);
        goto rule_mlvo;
    case TOK_STAR:
        if (!m->rule.skip)
            matcher_rule_set_mlvo_wildcard(m, s);
        goto rule_mlvo;
    case TOK_GROUP_NAME:
        if (!m->rule.skip)
            matcher_rule_set_mlvo_group(m, s, m->val.string);
        goto rule_mlvo;
    case TOK_EQUALS:
        goto rule_kccgst;
    default:
        goto unexpected;
    }

rule_kccgst:
    switch (tok = gettok(m, s)) {
    case TOK_IDENTIFIER:
        if (!m->rule.skip)
            matcher_rule_set_kccgst(m, s, m->val.string);
        goto rule_kccgst;
    case TOK_END_OF_LINE:
        if (!m->rule.skip)
            matcher_rule_verify(m, s);
        if (!m->rule.skip)
            matcher_rule_apply_if_matches(m, s);
        goto rule_mlvo_first;
    default:
        goto unexpected;
    }

unexpected:
    switch (tok) {
    case TOK_ERROR:
        goto error;
    default:
        goto state_error;
    }

finish:
    return true;

state_error:
    scanner_err(s, "unexpected token");
error:
    return false;
}

static bool
read_rules_file(struct xkb_context *ctx,
                struct matcher *matcher,
                unsigned include_depth,
                FILE *file,
                const char *path)
{
    bool ret = false;
    char *string;
    size_t size;
    struct scanner scanner;

    ret = map_file(file, &string, &size);
    if (!ret) {
        log_err(ctx, XKB_LOG_MESSAGE_NO_ID,
                "Couldn't read rules file \"%s\": %s\n",
                path, strerror(errno));
        goto out;
    }

    scanner_init(&scanner, matcher->ctx, string, size, path, NULL);

    ret = matcher_match(matcher, &scanner, include_depth, string, size, path);

    unmap_file(string, size);
out:
    return ret;
}

bool
xkb_components_from_rules(struct xkb_context *ctx,
                          const struct xkb_rule_names *rmlvo,
                          struct xkb_component_names *out)
{
    bool ret = false;
    FILE *file;
    char *path = NULL;
    struct matcher *matcher = NULL;
    struct matched_sval *mval;
    unsigned int offset = 0;

    file = FindFileInXkbPath(ctx, rmlvo->rules, FILE_TYPE_RULES, &path, &offset);
    if (!file)
        goto err_out;

    matcher = matcher_new(ctx, rmlvo);

    ret = read_rules_file(ctx, matcher, 0, file, path);
    if (!ret ||
        darray_empty(matcher->kccgst[KCCGST_KEYCODES]) ||
        darray_empty(matcher->kccgst[KCCGST_TYPES]) ||
        darray_empty(matcher->kccgst[KCCGST_COMPAT]) ||
        /* darray_empty(matcher->kccgst[KCCGST_GEOMETRY]) || */
        darray_empty(matcher->kccgst[KCCGST_SYMBOLS])) {
        log_err(ctx, XKB_LOG_MESSAGE_NO_ID,
                "No components returned from XKB rules \"%s\"\n", path);
        ret = false;
        goto err_out;
    }

    darray_steal(matcher->kccgst[KCCGST_KEYCODES], &out->keycodes, NULL);
    darray_steal(matcher->kccgst[KCCGST_TYPES], &out->types, NULL);
    darray_steal(matcher->kccgst[KCCGST_COMPAT], &out->compat, NULL);
    darray_steal(matcher->kccgst[KCCGST_SYMBOLS], &out->symbols, NULL);
    darray_free(matcher->kccgst[KCCGST_GEOMETRY]);

    mval = &matcher->rmlvo.model;
    if (!mval->matched && mval->sval.len > 0)
        log_err(matcher->ctx, XKB_LOG_MESSAGE_NO_ID,
                "Unrecognized RMLVO model \"%.*s\" was ignored\n",
                mval->sval.len, mval->sval.start);
    darray_foreach(mval, matcher->rmlvo.layouts)
        if (!mval->matched && mval->sval.len > 0)
            log_err(matcher->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Unrecognized RMLVO layout \"%.*s\" was ignored\n",
                    mval->sval.len, mval->sval.start);
    darray_foreach(mval, matcher->rmlvo.variants)
        if (!mval->matched && mval->sval.len > 0)
            log_err(matcher->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Unrecognized RMLVO variant \"%.*s\" was ignored\n",
                    mval->sval.len, mval->sval.start);
    darray_foreach(mval, matcher->rmlvo.options)
        if (!mval->matched && mval->sval.len > 0)
            log_err(matcher->ctx, XKB_LOG_MESSAGE_NO_ID,
                    "Unrecognized RMLVO option \"%.*s\" was ignored\n",
                    mval->sval.len, mval->sval.start);

err_out:
    if (file)
        fclose(file);
    matcher_free(matcher);
    free(path);
    return ret;
}
