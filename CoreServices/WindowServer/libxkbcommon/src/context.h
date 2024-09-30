/*
 * Copyright © 2012 Intel Corporation
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
 *
 * Author: Daniel Stone <daniel@fooishbar.org>
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "atom.h"
#include "messages-codes.h"

struct xkb_context {
    int refcnt;

    ATTR_PRINTF(3, 0) void (*log_fn)(struct xkb_context *ctx,
                                     enum xkb_log_level level,
                                     const char *fmt, va_list args);
    enum xkb_log_level log_level;
    int log_verbosity;
    void *user_data;

    struct xkb_rule_names names_dflt;

    darray(char *) includes;
    darray(char *) failed_includes;

    struct atom_table *atom_table;

    /* Used and allocated by xkbcommon-x11, free()d with the context. */
    void *x11_atom_cache;

    /* Buffer for the *Text() functions. */
    char text_buffer[2048];
    size_t text_next;

    unsigned int use_environment_names : 1;
    unsigned int use_secure_getenv : 1;
};

char *
xkb_context_getenv(struct xkb_context *ctx, const char *name);

unsigned int
xkb_context_num_failed_include_paths(struct xkb_context *ctx);

const char *
xkb_context_failed_include_path_get(struct xkb_context *ctx,
                                    unsigned int idx);

const char *
xkb_context_include_path_get_extra_path(struct xkb_context *ctx);

const char *
xkb_context_include_path_get_system_path(struct xkb_context *ctx);

/*
 * Returns XKB_ATOM_NONE if @string was not previously interned,
 * otherwise returns the atom.
 */
xkb_atom_t
xkb_atom_lookup(struct xkb_context *ctx, const char *string);

xkb_atom_t
xkb_atom_intern(struct xkb_context *ctx, const char *string, size_t len);

#define xkb_atom_intern_literal(ctx, literal) \
    xkb_atom_intern((ctx), (literal), sizeof(literal) - 1)

/**
 * If @string is dynamically allocated, NUL-terminated, free'd immediately
 * after being interned, and not used afterwards, use this function
 * instead of xkb_atom_intern to avoid some unnecessary allocations.
 * The caller should not use or free the passed in string afterwards.
 */
xkb_atom_t
xkb_atom_steal(struct xkb_context *ctx, char *string);

const char *
xkb_atom_text(struct xkb_context *ctx, xkb_atom_t atom);

char *
xkb_context_get_buffer(struct xkb_context *ctx, size_t size);

ATTR_PRINTF(4, 5) void
xkb_log(struct xkb_context *ctx, enum xkb_log_level level, int verbosity,
        const char *fmt, ...);

void
xkb_context_sanitize_rule_names(struct xkb_context *ctx,
                                struct xkb_rule_names *rmlvo);

/*
 * Macro sorcery: PREPEND_MESSAGE_ID enables the log functions to format messages
 * with the message ID only if the ID is not 0 (XKB_LOG_MESSAGE_NO_ID).
 * This avoid checking the ID value at run time.
 *
 * The trick resides in CHECK_ID:
 * • CHECK_ID(0) expands to:
 *   ‣ SECOND(MATCH0, WITH_ID, unused)
 *   ‣ SECOND(unused,WITHOUT_ID, WITH_ID, unused)
 *   ‣ WITHOUT_ID
 * • CHECK_ID(123) expands to:
 *   ‣ SECOND(MATCH123, WITH_ID, unused)
 *   ‣ WITH_ID
*/
#define EXPAND(...)              __VA_ARGS__ /* needed for MSVC compatibility */

#define JOIN_EXPAND(a, b)        a##b
#define JOIN(a, b)               JOIN_EXPAND(a, b)

#define SECOND_EXPAND(a, b, ...) b
#define SECOND(...)              EXPAND(SECOND_EXPAND(__VA_ARGS__))

#define MATCH0                   unused,WITHOUT_ID
#define CHECK_ID(value)          SECOND(JOIN(MATCH, value), WITH_ID, unused)

#define FORMAT_MESSAGE_WITHOUT_ID(id, fmt) fmt
#define FORMAT_MESSAGE_WITH_ID(id, fmt)    "[XKB-%03d] " fmt, id
#define PREPEND_MESSAGE_ID(id, fmt) JOIN(FORMAT_MESSAGE_, CHECK_ID(id))(id, fmt)

/*
 * The format is not part of the argument list in order to avoid the
 * "ISO C99 requires rest arguments to be used" warning when only the
 * format is supplied without arguments. Not supplying it would still
 * result in an error, though.
 */
#define xkb_log_with_code(ctx, level, verbosity, msg_id, fmt, ...) \
    xkb_log(ctx, level, verbosity, PREPEND_MESSAGE_ID(msg_id, fmt), ##__VA_ARGS__)
#define log_dbg(ctx, id, ...) \
    xkb_log_with_code((ctx), XKB_LOG_LEVEL_DEBUG, 0, id, __VA_ARGS__)
#define log_info(ctx, id, ...) \
    xkb_log_with_code((ctx), XKB_LOG_LEVEL_INFO, 0, id, __VA_ARGS__)
#define log_warn(ctx, id, ...) \
    xkb_log_with_code((ctx), XKB_LOG_LEVEL_WARNING, 0, id, __VA_ARGS__)
#define log_err(ctx, id, ...) \
    xkb_log_with_code((ctx), XKB_LOG_LEVEL_ERROR, 0, id, __VA_ARGS__)
#define log_wsgo(ctx, id, ...) \
    xkb_log_with_code((ctx), XKB_LOG_LEVEL_CRITICAL, 0, id, __VA_ARGS__)
#define log_vrb(ctx, vrb, id, ...) \
    xkb_log_with_code((ctx), XKB_LOG_LEVEL_WARNING, (vrb), id, __VA_ARGS__)

/*
 * Variants which are prefixed by the name of the function they're
 * called from.
 * Here we must have the silly 1 variant.
 */
#define log_err_func(ctx, fmt, ...) \
    log_err(ctx, XKB_LOG_MESSAGE_NO_ID, "%s: " fmt, __func__, __VA_ARGS__)
#define log_err_func1(ctx, fmt) \
    log_err(ctx, XKB_LOG_MESSAGE_NO_ID, "%s: " fmt, __func__)

#endif
