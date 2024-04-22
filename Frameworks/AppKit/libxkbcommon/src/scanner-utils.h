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

#ifndef XKBCOMP_SCANNER_UTILS_H
#define XKBCOMP_SCANNER_UTILS_H

/* Point to some substring in the file; used to avoid copying. */
struct sval {
    const char *start;
    unsigned int len;
};
typedef darray(struct sval) darray_sval;

static inline bool
svaleq(struct sval s1, struct sval s2)
{
    return s1.len == s2.len && memcmp(s1.start, s2.start, s1.len) == 0;
}

static inline bool
svaleq_prefix(struct sval s1, struct sval s2)
{
    return s1.len <= s2.len && memcmp(s1.start, s2.start, s1.len) == 0;
}

struct scanner {
    const char *s;
    size_t pos;
    size_t len;
    char buf[1024];
    size_t buf_pos;
    size_t line, column;
    /* The line/column of the start of the current token. */
    size_t token_line, token_column;
    const char *file_name;
    struct xkb_context *ctx;
    void *priv;
};

#define scanner_log_with_code(scanner, level, log_msg_id, fmt, ...) \
    xkb_log_with_code((scanner)->ctx, (level), 0, log_msg_id, \
                    "%s:%zu:%zu: " fmt "\n", \
                    (scanner)->file_name, \
                    (scanner)->token_line, \
                    (scanner)->token_column, ##__VA_ARGS__)

#define scanner_log(scanner, level, fmt, ...) \
    xkb_log((scanner)->ctx, (level), 0, \
            "%s:%zu:%zu: " fmt "\n", \
            (scanner)->file_name, \
            (scanner)->token_line, (scanner)->token_column, ##__VA_ARGS__)

#define scanner_err_with_code(scanner, id, fmt, ...) \
    scanner_log_with_code(scanner, XKB_LOG_LEVEL_ERROR, id, fmt, ##__VA_ARGS__)

#define scanner_err(scanner, fmt, ...) \
    scanner_log(scanner, XKB_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

#define scanner_warn_with_code(scanner, id, fmt, ...) \
    scanner_log_with_code(scanner, XKB_LOG_LEVEL_WARNING, id, fmt, ##__VA_ARGS__)

#define scanner_warn(scanner, fmt, ...) \
    scanner_log(scanner, XKB_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)

static inline void
scanner_init(struct scanner *s, struct xkb_context *ctx,
             const char *string, size_t len, const char *file_name,
             void *priv)
{
    s->s = string;
    s->len = len;
    s->pos = 0;
    s->line = s->column = 1;
    s->token_line = s->token_column = 1;
    s->file_name = file_name;
    s->ctx = ctx;
    s->priv = priv;
}

static inline char
scanner_peek(struct scanner *s)
{
    if (unlikely(s->pos >= s->len))
        return '\0';
    return s->s[s->pos];
}

static inline bool
scanner_eof(struct scanner *s)
{
    return s->pos >= s->len;
}

static inline bool
scanner_eol(struct scanner *s)
{
    return scanner_peek(s) == '\n';
}

static inline void
scanner_skip_to_eol(struct scanner *s)
{
    const char *nl = memchr(s->s + s->pos, '\n', s->len - s->pos);
    const size_t new_pos = nl ? (size_t) (nl - s->s) : s->len;
    s->column += new_pos - s->pos;
    s->pos = new_pos;
}

static inline char
scanner_next(struct scanner *s)
{
    if (unlikely(scanner_eof(s)))
        return '\0';
    if (unlikely(scanner_eol(s))) {
        s->line++;
        s->column = 1;
    }
    else {
        s->column++;
    }
    return s->s[s->pos++];
}

static inline bool
scanner_chr(struct scanner *s, char ch)
{
    if (likely(scanner_peek(s) != ch))
        return false;
    s->pos++; s->column++;
    return true;
}

static inline bool
scanner_str(struct scanner *s, const char *string, size_t len)
{
    if (s->len - s->pos < len)
        return false;
    if (memcmp(s->s + s->pos, string, len) != 0)
        return false;
    s->pos += len; s->column += len;
    return true;
}

#define scanner_lit(s, literal) scanner_str(s, literal, sizeof(literal) - 1)

static inline bool
scanner_buf_append(struct scanner *s, char ch)
{
    if (s->buf_pos + 1 >= sizeof(s->buf))
        return false;
    s->buf[s->buf_pos++] = ch;
    return true;
}

static inline bool
scanner_buf_appends(struct scanner *s, const char *str)
{
    int ret;
    ret = snprintf(s->buf + s->buf_pos, sizeof(s->buf) - s->buf_pos, "%s", str);
    if (ret < 0 || (size_t) ret >= sizeof(s->buf) - s->buf_pos)
        return false;
    s->buf_pos += ret;
    return true;
}

static inline bool
scanner_oct(struct scanner *s, uint8_t *out)
{
    int i;
    for (i = 0, *out = 0; scanner_peek(s) >= '0' && scanner_peek(s) <= '7' && i < 3; i++)
        /* Test overflow */
        if (*out < 040) {
            *out = *out * 8 + scanner_next(s) - '0';
        } else {
            /* Consume valid digit, but mark result as invalid */
            scanner_next(s);
            return false;
        }
    return i > 0;
}

static inline bool
scanner_hex(struct scanner *s, uint8_t *out)
{
    int i;
    for (i = 0, *out = 0; is_xdigit(scanner_peek(s)) && i < 2; i++) {
        const char c = scanner_next(s);
        const char offset = (c >= '0' && c <= '9' ? '0' :
                             c >= 'a' && c <= 'f' ? 'a' - 10 : 'A' - 10);
        *out = *out * 16 + c - offset;
    }
    return i > 0;
}

#endif
