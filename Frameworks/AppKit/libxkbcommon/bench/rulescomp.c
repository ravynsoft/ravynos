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

#include <time.h>

#include "../test/test.h"
#include "bench.h"

#define BENCHMARK_ITERATIONS 1000

int
main(int argc, char *argv[])
{
    struct xkb_context *ctx;
    struct xkb_keymap *keymap;
    struct bench bench;
    char *elapsed;
    int i;

    ctx = test_get_context(0);
    assert(ctx);

    xkb_context_set_log_level(ctx, XKB_LOG_LEVEL_CRITICAL);
    xkb_context_set_log_verbosity(ctx, 0);

    bench_start(&bench);
    for (i = 0; i < BENCHMARK_ITERATIONS; i++) {
        keymap = test_compile_rules(ctx, "evdev", "evdev", "us", "", "");
        assert(keymap);
        xkb_keymap_unref(keymap);
    }
    bench_stop(&bench);

    elapsed = bench_elapsed_str(&bench);
    fprintf(stderr, "compiled %d keymaps in %ss\n",
            BENCHMARK_ITERATIONS, elapsed);
    free(elapsed);

    xkb_context_unref(ctx);
    return 0;
}
