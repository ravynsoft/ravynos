/*
 * Copyright © 2009 Dan Nicholson
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

static int
test_file(struct xkb_context *ctx, const char *path_rel)
{
    struct xkb_keymap *keymap = test_compile_file(ctx, path_rel);

    if (!keymap)
        return 0;

    xkb_keymap_unref(keymap);
    return 1;
}

int
main(void)
{
    struct xkb_context *ctx = test_get_context(0);

    assert(test_file(ctx, "keymaps/basic.xkb"));
    assert(test_file(ctx, "keymaps/comprehensive-plus-geom.xkb"));
    assert(test_file(ctx, "keymaps/no-types.xkb"));
    assert(test_file(ctx, "keymaps/quartz.xkb"));
    assert(test_file(ctx, "keymaps/no-aliases.xkb"));
    assert(test_file(ctx, "keymaps/modmap-none.xkb"));
    assert(test_file(ctx, "keymaps/invalid-escape-sequence.xkb"));

    assert(!test_file(ctx, "keymaps/divide-by-zero.xkb"));
    assert(!test_file(ctx, "keymaps/bad.xkb"));
    assert(!test_file(ctx, "keymaps/syntax-error.xkb"));
    assert(!test_file(ctx, "keymaps/syntax-error2.xkb"));
    assert(!test_file(ctx, "does not exist"));

    /* Test response to invalid flags and formats. */
    fclose(stdin);
    assert(!xkb_keymap_new_from_file(ctx, NULL, XKB_KEYMAP_FORMAT_TEXT_V1, 0));
    assert(!xkb_keymap_new_from_file(ctx, stdin, 0, 0));
    assert(!xkb_keymap_new_from_file(ctx, stdin, XKB_KEYMAP_USE_ORIGINAL_FORMAT, 0));
    assert(!xkb_keymap_new_from_file(ctx, stdin, 1234, 0));
    assert(!xkb_keymap_new_from_file(ctx, stdin, XKB_KEYMAP_FORMAT_TEXT_V1, -1));
    assert(!xkb_keymap_new_from_file(ctx, stdin, XKB_KEYMAP_FORMAT_TEXT_V1, 1234));

    xkb_context_unref(ctx);

    return 0;
}
