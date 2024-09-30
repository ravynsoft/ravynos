/*
 * Copyright © 2014 Ran Benita <ran234@gmail.com>
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

#include "xkbcommon/xkbcommon-compose.h"

#include "test.h"

static const char *
compose_status_string(enum xkb_compose_status status)
{
    switch (status) {
    case XKB_COMPOSE_NOTHING:
        return "nothing";
    case XKB_COMPOSE_COMPOSING:
        return "composing";
    case XKB_COMPOSE_COMPOSED:
        return "composed";
    case XKB_COMPOSE_CANCELLED:
        return "cancelled";
    }

    return "<invalid-status>";
}

static const char *
feed_result_string(enum xkb_compose_feed_result result)
{
    switch (result) {
    case XKB_COMPOSE_FEED_IGNORED:
        return "ignored";
    case XKB_COMPOSE_FEED_ACCEPTED:
        return "accepted";
    }

    return "<invalid-result>";
}

/*
 * Feed a sequence of keysyms to a fresh compose state and test the outcome.
 *
 * The varargs consists of lines in the following format:
 *      <input keysym> <expected feed result> <expected status> <expected string> <expected keysym>
 * Terminated by a line consisting only of XKB_KEY_NoSymbol.
 */
static bool
test_compose_seq_va(struct xkb_compose_table *table, va_list ap)
{
    int ret;
    struct xkb_compose_state *state;
    char buffer[64];

    state = xkb_compose_state_new(table, XKB_COMPOSE_STATE_NO_FLAGS);
    assert(state);

    for (int i = 1; ; i++) {
        xkb_keysym_t input_keysym;
        enum xkb_compose_feed_result result, expected_result;
        enum xkb_compose_status status, expected_status;
        const char *expected_string;
        xkb_keysym_t keysym, expected_keysym;

        input_keysym = va_arg(ap, xkb_keysym_t);
        if (input_keysym == XKB_KEY_NoSymbol)
            break;

        expected_result = va_arg(ap, enum xkb_compose_feed_result);
        expected_status = va_arg(ap, enum xkb_compose_status);
        expected_string = va_arg(ap, const char *);
        expected_keysym = va_arg(ap, xkb_keysym_t);

        result = xkb_compose_state_feed(state, input_keysym);

        if (result != expected_result) {
            fprintf(stderr, "after feeding %d keysyms:\n", i);
            fprintf(stderr, "expected feed result: %s\n",
                    feed_result_string(expected_result));
            fprintf(stderr, "got feed result: %s\n",
                    feed_result_string(result));
            goto fail;
        }

        status = xkb_compose_state_get_status(state);
        if (status != expected_status) {
            fprintf(stderr, "after feeding %d keysyms:\n", i);
            fprintf(stderr, "expected status: %s\n",
                    compose_status_string(expected_status));
            fprintf(stderr, "got status: %s\n",
                    compose_status_string(status));
            goto fail;
        }

        ret = xkb_compose_state_get_utf8(state, buffer, sizeof(buffer));
        if (ret < 0 || (size_t) ret >= sizeof(buffer)) {
            fprintf(stderr, "after feeding %d keysyms:\n", i);
            fprintf(stderr, "expected string: %s\n", expected_string);
            fprintf(stderr, "got error: %d\n", ret);
            goto fail;
        }
        if (!streq(buffer, expected_string)) {
            fprintf(stderr, "after feeding %d keysyms:\n", i);
            fprintf(stderr, "expected string: %s\n", strempty(expected_string));
            fprintf(stderr, "got string: %s\n", buffer);
            goto fail;
        }

        keysym = xkb_compose_state_get_one_sym(state);
        if (keysym != expected_keysym) {
            fprintf(stderr, "after feeding %d keysyms:\n", i);
            xkb_keysym_get_name(expected_keysym, buffer, sizeof(buffer));
            fprintf(stderr, "expected keysym: %s\n", buffer);
            xkb_keysym_get_name(keysym, buffer, sizeof(buffer));
            fprintf(stderr, "got keysym (%#x): %s\n", keysym, buffer);
            goto fail;
        }
    }

    xkb_compose_state_unref(state);
    return true;

fail:
    xkb_compose_state_unref(state);
    return false;
}

static bool
test_compose_seq(struct xkb_compose_table *table, ...)
{
    va_list ap;
    bool ok;
    va_start(ap, table);
    ok = test_compose_seq_va(table, ap);
    va_end(ap);
    return ok;
}

static bool
test_compose_seq_buffer(struct xkb_context *ctx, const char *buffer, ...)
{
    va_list ap;
    bool ok;
    struct xkb_compose_table *table;
    table = xkb_compose_table_new_from_buffer(ctx, buffer, strlen(buffer), "",
                                              XKB_COMPOSE_FORMAT_TEXT_V1,
                                              XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);
    va_start(ap, buffer);
    ok = test_compose_seq_va(table, ap);
    va_end(ap);
    xkb_compose_table_unref(table);
    return ok;
}

static void
test_seqs(struct xkb_context *ctx)
{
    struct xkb_compose_table *table;
    char *path;
    FILE *file;

    path = test_get_path("locale/en_US.UTF-8/Compose");
    file = fopen(path, "rb");
    assert(file);
    free(path);

    table = xkb_compose_table_new_from_file(ctx, file, "",
                                            XKB_COMPOSE_FORMAT_TEXT_V1,
                                            XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);
    fclose(file);

    assert(test_compose_seq(table,
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_space,          XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "~",    XKB_KEY_asciitilde,
        XKB_KEY_NoSymbol));

    assert(test_compose_seq(table,
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_space,          XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "~",    XKB_KEY_asciitilde,
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_space,          XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "~",    XKB_KEY_asciitilde,
        XKB_KEY_NoSymbol));

    assert(test_compose_seq(table,
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "~",    XKB_KEY_asciitilde,
        XKB_KEY_NoSymbol));

    assert(test_compose_seq(table,
        XKB_KEY_dead_acute,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_space,          XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "'",    XKB_KEY_apostrophe,
        XKB_KEY_Caps_Lock,      XKB_COMPOSE_FEED_IGNORED,   XKB_COMPOSE_COMPOSED,   "'",    XKB_KEY_apostrophe,
        XKB_KEY_NoSymbol));

    assert(test_compose_seq(table,
        XKB_KEY_dead_acute,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_dead_acute,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "´",    XKB_KEY_acute,
        XKB_KEY_NoSymbol));

    assert(test_compose_seq(table,
        XKB_KEY_Multi_key,      XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_Shift_L,        XKB_COMPOSE_FEED_IGNORED,   XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_Caps_Lock,      XKB_COMPOSE_FEED_IGNORED,   XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_Control_L,      XKB_COMPOSE_FEED_IGNORED,   XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_T,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "@",    XKB_KEY_at,
        XKB_KEY_NoSymbol));

    assert(test_compose_seq(table,
        XKB_KEY_7,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,    "",     XKB_KEY_NoSymbol,
        XKB_KEY_a,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,    "",     XKB_KEY_NoSymbol,
        XKB_KEY_b,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,    "",     XKB_KEY_NoSymbol,
        XKB_KEY_NoSymbol));

    assert(test_compose_seq(table,
        XKB_KEY_Multi_key,      XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_apostrophe,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_7,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_CANCELLED,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_7,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,    "",     XKB_KEY_NoSymbol,
        XKB_KEY_Caps_Lock,      XKB_COMPOSE_FEED_IGNORED,   XKB_COMPOSE_NOTHING,    "",     XKB_KEY_NoSymbol,
        XKB_KEY_NoSymbol));

    xkb_compose_table_unref(table);

    /* Make sure one-keysym sequences work. */
    assert(test_compose_seq_buffer(ctx,
        "<A>          :  \"foo\"  X \n"
        "<B> <A>      :  \"baz\"  Y \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,  "foo",   XKB_KEY_X,
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,  "foo",   XKB_KEY_X,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,   "",      XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING, "",      XKB_KEY_NoSymbol,
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,  "baz",   XKB_KEY_Y,
        XKB_KEY_NoSymbol));

    /* No sequences at all. */
    assert(test_compose_seq_buffer(ctx,
        "",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,   "",      XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,   "",      XKB_KEY_NoSymbol,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,   "",      XKB_KEY_NoSymbol,
        XKB_KEY_Multi_key,      XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,   "",      XKB_KEY_NoSymbol,
        XKB_KEY_dead_acute,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,   "",      XKB_KEY_NoSymbol,
        XKB_KEY_NoSymbol));

    /* Only keysym - string derived from keysym. */
    assert(test_compose_seq_buffer(ctx,
        "<A> <B>     :  X \n"
        "<B> <A>     :  dollar \n"
        "<C>         :  dead_acute \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING, "",      XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,  "X",     XKB_KEY_X,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING, "",      XKB_KEY_NoSymbol,
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,  "$",     XKB_KEY_dollar,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,  "",      XKB_KEY_dead_acute,
        XKB_KEY_NoSymbol));

    /* Make sure a cancelling keysym doesn't start a new sequence. */
    assert(test_compose_seq_buffer(ctx,
        "<A> <B>     :  X \n"
        "<C> <D>     :  Y \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING, "",      XKB_KEY_NoSymbol,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_CANCELLED, "",      XKB_KEY_NoSymbol,
        XKB_KEY_D,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,   "",      XKB_KEY_NoSymbol,
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING, "",      XKB_KEY_NoSymbol,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_CANCELLED, "",      XKB_KEY_NoSymbol,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING, "",      XKB_KEY_NoSymbol,
        XKB_KEY_D,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,  "Y",     XKB_KEY_Y,
        XKB_KEY_NoSymbol));
}

static void
test_conflicting(struct xkb_context *ctx)
{
    // new is prefix of old
    assert(test_compose_seq_buffer(ctx,
        "<A> <B> <C>  :  \"foo\"  A \n"
        "<A> <B>      :  \"bar\"  B \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "foo",  XKB_KEY_A,
        XKB_KEY_NoSymbol));

    // old is a prefix of new
    assert(test_compose_seq_buffer(ctx,
        "<A> <B>      :  \"bar\"  B \n"
        "<A> <B> <C>  :  \"foo\"  A \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "foo",  XKB_KEY_A,
        XKB_KEY_NoSymbol));

    // new duplicate of old
    assert(test_compose_seq_buffer(ctx,
        "<A> <B>      :  \"bar\"  B \n"
        "<A> <B>      :  \"bar\"  B \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "bar",  XKB_KEY_B,
        XKB_KEY_C,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_NOTHING,    "",     XKB_KEY_NoSymbol,
        XKB_KEY_NoSymbol));

    // new same length as old #1
    assert(test_compose_seq_buffer(ctx,
        "<A> <B>      :  \"foo\"  A \n"
        "<A> <B>      :  \"bar\"  B \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "bar",  XKB_KEY_B,
        XKB_KEY_NoSymbol));

    // new same length as old #2
    assert(test_compose_seq_buffer(ctx,
        "<A> <B>      :  \"foo\"  A \n"
        "<A> <B>      :  \"foo\"  B \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "foo",  XKB_KEY_B,
        XKB_KEY_NoSymbol));

    // new same length as old #3
    assert(test_compose_seq_buffer(ctx,
        "<A> <B>      :  \"foo\"  A \n"
        "<A> <B>      :  \"bar\"  A \n",
        XKB_KEY_A,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_B,              XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "bar",  XKB_KEY_A,
        XKB_KEY_NoSymbol));
}

static void
test_state(struct xkb_context *ctx)
{
    struct xkb_compose_table *table;
    struct xkb_compose_state *state;
    char *path;
    FILE *file;

    path = test_get_path("locale/en_US.UTF-8/Compose");
    file = fopen(path, "rb");
    assert(file);
    free(path);

    table = xkb_compose_table_new_from_file(ctx, file, "",
                                            XKB_COMPOSE_FORMAT_TEXT_V1,
                                            XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);
    fclose(file);

    state = xkb_compose_state_new(table, XKB_COMPOSE_STATE_NO_FLAGS);
    assert(state);

    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_NOTHING);
    xkb_compose_state_reset(state);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_NOTHING);
    xkb_compose_state_feed(state, XKB_KEY_NoSymbol);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_NOTHING);
    xkb_compose_state_feed(state, XKB_KEY_Multi_key);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_COMPOSING);
    xkb_compose_state_reset(state);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_NOTHING);
    xkb_compose_state_feed(state, XKB_KEY_Multi_key);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_COMPOSING);
    xkb_compose_state_feed(state, XKB_KEY_Multi_key);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_CANCELLED);
    xkb_compose_state_feed(state, XKB_KEY_Multi_key);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_COMPOSING);
    xkb_compose_state_feed(state, XKB_KEY_Multi_key);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_CANCELLED);
    xkb_compose_state_reset(state);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_NOTHING);
    xkb_compose_state_feed(state, XKB_KEY_dead_acute);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_COMPOSING);
    xkb_compose_state_feed(state, XKB_KEY_A);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_COMPOSED);
    xkb_compose_state_reset(state);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_NOTHING);
    xkb_compose_state_feed(state, XKB_KEY_dead_acute);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_COMPOSING);
    xkb_compose_state_feed(state, XKB_KEY_A);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_COMPOSED);
    xkb_compose_state_reset(state);
    xkb_compose_state_feed(state, XKB_KEY_NoSymbol);
    assert(xkb_compose_state_get_status(state) == XKB_COMPOSE_NOTHING);

    xkb_compose_state_unref(state);
    xkb_compose_table_unref(table);
}

static void
test_XCOMPOSEFILE(struct xkb_context *ctx)
{
    struct xkb_compose_table *table;
    char *path;

    path = test_get_path("locale/en_US.UTF-8/Compose");
    setenv("XCOMPOSEFILE", path, 1);
    free(path);

    table = xkb_compose_table_new_from_locale(ctx, "blabla",
                                              XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);

    unsetenv("XCOMPOSEFILE");

    assert(test_compose_seq(table,
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_space,          XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "~",    XKB_KEY_asciitilde,
        XKB_KEY_NoSymbol));

    xkb_compose_table_unref(table);
}

static void
test_from_locale(struct xkb_context *ctx)
{
    struct xkb_compose_table *table;
    char *path;

    path = test_get_path("locale");
    setenv("XLOCALEDIR", path, 1);
    free(path);

    /* Direct directory name match. */
    table = xkb_compose_table_new_from_locale(ctx, "en_US.UTF-8",
                                              XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);
    xkb_compose_table_unref(table);

    /* Direct locale name match. */
    table = xkb_compose_table_new_from_locale(ctx, "C.UTF-8",
                                              XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);
    xkb_compose_table_unref(table);

    /* Alias. */
    table = xkb_compose_table_new_from_locale(ctx, "univ.utf8",
                                              XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);
    xkb_compose_table_unref(table);

    /* Special case - C. */
    table = xkb_compose_table_new_from_locale(ctx, "C",
                                              XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);
    xkb_compose_table_unref(table);

    /* Bogus - not found. */
    table = xkb_compose_table_new_from_locale(ctx, "blabla",
                                              XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(!table);

    unsetenv("XLOCALEDIR");
}


static void
test_modifier_syntax(struct xkb_context *ctx)
{
    const char *table_string;

    /* We don't do anything with the modifiers, but make sure we can parse
     * them. */

    assert(test_compose_seq_buffer(ctx,
        "None <A>          : X \n"
        "Shift <B>         : Y \n"
        "Ctrl <C>          : Y \n"
        "Alt <D>           : Y \n"
        "Caps <E>          : Y \n"
        "Lock <F>          : Y \n"
        "Shift Ctrl <G>    : Y \n"
        "~Shift <H>        : Y \n"
        "~Shift Ctrl <I>   : Y \n"
        "Shift ~Ctrl <J>   : Y \n"
        "Shift ~Ctrl ~Alt <K> : Y \n"
        "! Shift <B>       : Y \n"
        "! Ctrl <C>        : Y \n"
        "! Alt <D>         : Y \n"
        "! Caps <E>        : Y \n"
        "! Lock <F>        : Y \n"
        "! Shift Ctrl <G>  : Y \n"
        "! ~Shift <H>      : Y \n"
        "! ~Shift Ctrl <I> : Y \n"
        "! Shift ~Ctrl <J> : Y \n"
        "! Shift ~Ctrl ~Alt <K> : Y \n"
        "<L> ! Shift <M>   : Y \n"
        "None <N> ! Shift <O> : Y \n"
        "None <P> ! Shift <Q> : Y \n",
        XKB_KEY_NoSymbol));

    fprintf(stderr, "<START bad input string>\n");
    table_string =
        "! None <A>        : X \n"
        "! Foo <B>         : X \n"
        "None ! Shift <C>  : X \n"
        "! ! <D>           : X \n"
        "! ~ <E>           : X \n"
        "! ! <F>           : X \n"
        "! Ctrl ! Ctrl <G> : X \n"
        "<H> !             : X \n"
        "<I> None          : X \n"
        "None None <J>     : X \n"
        "<K>               : !Shift X \n";
    assert(!xkb_compose_table_new_from_buffer(ctx, table_string,
                                              strlen(table_string), "C",
                                              XKB_COMPOSE_FORMAT_TEXT_V1,
                                              XKB_COMPOSE_COMPILE_NO_FLAGS));
    fprintf(stderr, "<END bad input string>\n");
}

static void
test_include(struct xkb_context *ctx)
{
    char *path, *table_string;

    path = test_get_path("locale/en_US.UTF-8/Compose");
    assert(path);

    /* We don't have a mechanism to change the include paths like we
     * have for keymaps. So we must include the full path. */
    table_string = asprintf_safe("<dead_tilde> <space>   : \"foo\" X\n"
                                 "include \"%s\"\n"
                                 "<dead_tilde> <dead_tilde> : \"bar\" Y\n", path);
    assert(table_string);

    assert(test_compose_seq_buffer(ctx, table_string,
        /* No conflict. */
        XKB_KEY_dead_acute,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_dead_acute,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "´",    XKB_KEY_acute,

        /* Comes before - doesn't override. */
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_space,          XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "~",    XKB_KEY_asciitilde,

        /* Comes after - does override. */
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_dead_tilde,     XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "bar",  XKB_KEY_Y,

        XKB_KEY_NoSymbol));

    free(path);
    free(table_string);
}

static void
test_override(struct xkb_context *ctx)
{
    const char *table_string = "<dead_circumflex> <dead_circumflex> : \"foo\" X\n"
                               "<dead_circumflex> <e> : \"bar\" Y\n"
                               "<dead_circumflex> <dead_circumflex> <e> : \"baz\" Z\n";

    assert(test_compose_seq_buffer(ctx, table_string,
        /* Comes after - does override. */
        XKB_KEY_dead_circumflex, XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_dead_circumflex, XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_e,               XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "baz",  XKB_KEY_Z,

        /* Override does not affect sibling nodes */
        XKB_KEY_dead_circumflex, XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_e,               XKB_COMPOSE_FEED_ACCEPTED,  XKB_COMPOSE_COMPOSED,   "bar",  XKB_KEY_Y,

        XKB_KEY_NoSymbol));
}

static bool
test_eq_entry_va(struct xkb_compose_table_entry *entry, xkb_keysym_t keysym_ref, const char *utf8_ref, va_list ap)
{
    assert (entry != NULL);

    assert (xkb_compose_table_entry_keysym(entry) == keysym_ref);

    const char *utf8 = xkb_compose_table_entry_utf8(entry);
    assert (utf8 && utf8_ref && strcmp(utf8, utf8_ref) == 0);

    size_t nsyms;
    const xkb_keysym_t *sequence = xkb_compose_table_entry_sequence(entry, &nsyms);

    xkb_keysym_t keysym;
    for (unsigned k = 0; ; k++) {
        keysym = va_arg(ap, xkb_keysym_t);
        if (keysym == XKB_KEY_NoSymbol) {
            return (k == nsyms - 1);
        }
        assert (k < nsyms);
        assert (keysym == sequence[k]);
    }
}

static bool
test_eq_entry(struct xkb_compose_table_entry *entry, xkb_keysym_t keysym, const char *utf8, ...)
{
    va_list ap;
    bool ok;
    va_start(ap, utf8);
    ok = test_eq_entry_va(entry, keysym, utf8, ap);
    va_end(ap);
    return ok;
}

static void
test_traverse(struct xkb_context *ctx)
{
    struct xkb_compose_table *table;

    const char *buffer = "<dead_circumflex> <dead_circumflex> : \"foo\" X\n"
                         "<Ahook> <x> : \"foobar\"\n"
                         "<Multi_key> <o> <e> : oe\n"
                         "<dead_circumflex> <e> : \"bar\" Y\n"
                         "<Multi_key> <a> <e> : \"æ\" ae\n"
                         "<dead_circumflex> <a> : \"baz\" Z\n"
                         "<dead_acute> <e> : \"é\" eacute\n"
                         "<Multi_key> <a> <a> <c>: \"aac\"\n"
                         "<Multi_key> <a> <a> <b>: \"aab\"\n"
                         "<Multi_key> <a> <a> <a>: \"aaa\"\n";

    table = xkb_compose_table_new_from_buffer(ctx, buffer, strlen(buffer), "",
                                              XKB_COMPOSE_FORMAT_TEXT_V1,
                                              XKB_COMPOSE_COMPILE_NO_FLAGS);
    assert(table);

    struct xkb_compose_table_iterator *iter = xkb_compose_table_iterator_new(table);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_eacute, "é",
                  XKB_KEY_dead_acute, XKB_KEY_e, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_Z, "baz",
                  XKB_KEY_dead_circumflex, XKB_KEY_a, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_Y, "bar",
                  XKB_KEY_dead_circumflex, XKB_KEY_e, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_X, "foo",
                  XKB_KEY_dead_circumflex, XKB_KEY_dead_circumflex, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_NoSymbol, "aaa",
                  XKB_KEY_Multi_key, XKB_KEY_a, XKB_KEY_a, XKB_KEY_a, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_NoSymbol, "aab",
                  XKB_KEY_Multi_key, XKB_KEY_a, XKB_KEY_a, XKB_KEY_b, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_NoSymbol, "aac",
                  XKB_KEY_Multi_key, XKB_KEY_a, XKB_KEY_a, XKB_KEY_c, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_ae, "æ",
                  XKB_KEY_Multi_key, XKB_KEY_a, XKB_KEY_e, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_oe, "",
                  XKB_KEY_Multi_key, XKB_KEY_o, XKB_KEY_e, XKB_KEY_NoSymbol);

    test_eq_entry(xkb_compose_table_iterator_next(iter),
                  XKB_KEY_NoSymbol, "foobar",
                  XKB_KEY_Ahook, XKB_KEY_x, XKB_KEY_NoSymbol);

    assert (xkb_compose_table_iterator_next(iter) == NULL);

    xkb_compose_table_iterator_free(iter);
    xkb_compose_table_unref(table);
}

static void
test_escape_sequences(struct xkb_context *ctx)
{
    /* The following escape sequences should be ignored:
     * • \401 overflows
     * • \0 and \x0 produce NULL
     */
    const char *table_string = "<o> <e> : \"\\401f\\x0o\\0o\" X\n";

    assert(test_compose_seq_buffer(ctx, table_string,
        XKB_KEY_o, XKB_COMPOSE_FEED_ACCEPTED, XKB_COMPOSE_COMPOSING,  "",     XKB_KEY_NoSymbol,
        XKB_KEY_e, XKB_COMPOSE_FEED_ACCEPTED, XKB_COMPOSE_COMPOSED,   "foo",  XKB_KEY_X,
        XKB_KEY_NoSymbol));
}

int
main(int argc, char *argv[])
{
    struct xkb_context *ctx;

    ctx = test_get_context(CONTEXT_NO_FLAG);
    assert(ctx);

    /*
     * Ensure no environment variables but “top_srcdir” is set. This ensures
     * that user Compose file paths are unset before the tests and set
     * explicitely when necessary.
     */
#ifdef __linux__
    const char *srcdir = getenv("top_srcdir");
    clearenv();
    setenv("top_srcdir", srcdir, 1);
#else
    unsetenv("XCOMPOSEFILE");
    unsetenv("XDG_CONFIG_HOME");
    unsetenv("HOME");
    unsetenv("XLOCALEDIR");
#endif

    test_seqs(ctx);
    test_conflicting(ctx);
    test_XCOMPOSEFILE(ctx);
    test_from_locale(ctx);
    test_state(ctx);
    test_modifier_syntax(ctx);
    test_include(ctx);
    test_override(ctx);
    test_traverse(ctx);
    test_escape_sequences(ctx);

    xkb_context_unref(ctx);
    return 0;
}
