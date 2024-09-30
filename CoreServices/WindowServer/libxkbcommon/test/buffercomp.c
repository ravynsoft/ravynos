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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "test.h"

#define DATA_PATH "keymaps/stringcomp.data"

int
main(int argc, char *argv[])
{
    struct xkb_context *ctx = test_get_context(0);
    struct xkb_keymap *keymap;
    char *original, *dump;

    assert(ctx);

    /* Load in a prebuilt keymap, make sure we can compile it from memory,
     * then compare it to make sure we get the same result when dumping it
     * to a string. */
    original = test_read_file(DATA_PATH);
    assert(original);

    /* Load a prebuild keymap, once without, once with the trailing \0 */
    for (int i = 0; i <= 1; i++) {
        keymap = test_compile_buffer(ctx, original, strlen(original) + i);
        assert(keymap);

        dump = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_USE_ORIGINAL_FORMAT);
        assert(dump);

        if (!streq(original, dump)) {
            fprintf(stderr,
                    "round-trip test failed: dumped map differs from original\n");
            fprintf(stderr, "path to original file: %s\n",
                    test_get_path(DATA_PATH));
            fprintf(stderr, "length: dumped %lu, original %lu\n",
                    (unsigned long) strlen(dump),
                    (unsigned long) strlen(original));
            fprintf(stderr, "dumped map:\n");
            fprintf(stderr, "%s\n", dump);
            fflush(stderr);
            assert(0);
        }

        free(dump);
        xkb_keymap_unref(keymap);
    }

    free(original);

    /* Make sure we can't (falsely claim to) compile an empty string. */
    keymap = test_compile_buffer(ctx, "", 0);
    assert(!keymap);

    /* Make sure we can recompile our output for a normal keymap from rules. */
    keymap = test_compile_rules(ctx, NULL, NULL,
                                "ru,ca,de,us", ",multix,neo,intl", NULL);
    assert(keymap);
    dump = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_USE_ORIGINAL_FORMAT);
    assert(dump);
    xkb_keymap_unref(keymap);
    keymap = test_compile_buffer(ctx, dump, strlen(dump));
    assert(keymap);
    xkb_keymap_unref(keymap);
    free(dump);

    xkb_context_unref(ctx);

    return 0;
}
