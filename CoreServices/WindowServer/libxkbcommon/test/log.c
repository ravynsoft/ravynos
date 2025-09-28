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

#include "test.h"
#include "context.h"
#include "messages-codes.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wmissing-format-attribute"
#endif

static const char *
log_level_to_string(enum xkb_log_level level)
{
    switch (level) {
    case XKB_LOG_LEVEL_CRITICAL:
        return "critical";
    case XKB_LOG_LEVEL_ERROR:
        return "error";
    case XKB_LOG_LEVEL_WARNING:
        return "warning";
    case XKB_LOG_LEVEL_INFO:
        return "info";
    case XKB_LOG_LEVEL_DEBUG:
        return "debug";
    }

    return "unknown";
}

ATTR_PRINTF(3, 0) static void
log_fn(struct xkb_context *ctx, enum xkb_log_level level,
       const char *fmt, va_list args)
{
    char *s;
    int size;
    darray_char *ls = xkb_context_get_user_data(ctx);
    assert(ls);

    size = vasprintf(&s, fmt, args);
    assert(size != -1);

    darray_append_string(*ls, log_level_to_string(level));
    darray_append_lit(*ls, ": ");
    darray_append_string(*ls, s);
    free(s);
}

int
main(void)
{
    darray_char log_string;
    struct xkb_context *ctx;
    int ret;

    ret = setenv("XKB_LOG_LEVEL", "warn", 1);
    assert(ret == 0);
    ret = setenv("XKB_LOG_VERBOSITY", "5", 1);
    assert(ret == 0);
    ctx = test_get_context(0);
    assert(ctx);

    darray_init(log_string);
    xkb_context_set_user_data(ctx, &log_string);
    xkb_context_set_log_fn(ctx, log_fn);

    log_warn(ctx, XKB_LOG_MESSAGE_NO_ID, "first warning: %d\n", 87);
    log_info(ctx, XKB_LOG_MESSAGE_NO_ID, "first info\n");
    log_dbg(ctx, XKB_LOG_MESSAGE_NO_ID, "first debug: %s\n", "hello");
    log_err(ctx, XKB_LOG_MESSAGE_NO_ID, "first error: %lu\n", 115415UL);
    log_vrb(ctx, 5, XKB_LOG_MESSAGE_NO_ID, "first verbose 5\n");

    xkb_context_set_log_level(ctx, XKB_LOG_LEVEL_DEBUG);
    log_warn(ctx, XKB_LOG_MESSAGE_NO_ID, "second warning: %d\n", 87);
    log_dbg(ctx, XKB_LOG_MESSAGE_NO_ID, "second debug: %s %s\n", "hello", "world");
    log_info(ctx, XKB_LOG_MESSAGE_NO_ID, "second info\n");
    log_err(ctx, XKB_ERROR_MALFORMED_NUMBER_LITERAL, "second error: %lu\n", 115415UL);
    log_vrb(ctx, 6, XKB_LOG_MESSAGE_NO_ID, "second verbose 6\n");

    xkb_context_set_log_verbosity(ctx, 0);
    xkb_context_set_log_level(ctx, XKB_LOG_LEVEL_CRITICAL);
    log_warn(ctx, XKB_LOG_MESSAGE_NO_ID, "third warning: %d\n", 87);
    log_dbg(ctx, XKB_LOG_MESSAGE_NO_ID, "third debug: %s %s\n", "hello", "world");
    log_info(ctx, XKB_LOG_MESSAGE_NO_ID, "third info\n");
    log_err(ctx, XKB_LOG_MESSAGE_NO_ID, "third error: %lu\n", 115415UL);
    log_vrb(ctx, 0, XKB_LOG_MESSAGE_NO_ID, "third verbose 0\n");

    printf("%s", log_string.item);

    assert(streq(log_string.item,
                 "warning: first warning: 87\n"
                 "error: first error: 115415\n"
                 "warning: first verbose 5\n"
                 "warning: second warning: 87\n"
                 "debug: second debug: hello world\n"
                 "info: second info\n"
                 "error: [XKB-034] second error: 115415\n"));

    xkb_context_unref(ctx);
    darray_free(log_string);
    return 0;
}
