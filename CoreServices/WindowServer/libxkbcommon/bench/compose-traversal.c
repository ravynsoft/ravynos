/*
 * Copyright © 2023 Pierre Le Marre
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

#include "xkbcommon/xkbcommon-compose.h"

#include "../test/test.h"
#include "bench.h"

#define BENCHMARK_ITERATIONS 1000

int
main(void)
{
    struct xkb_context *ctx;
    char *path;
    FILE *file;
    struct xkb_compose_table *table;
    struct xkb_compose_table_iterator *iter;
    struct xkb_compose_table_entry *entry;
    struct bench bench;
    char *elapsed;

    ctx = test_get_context(CONTEXT_NO_FLAG);
    assert(ctx);

    path = test_get_path("locale/en_US.UTF-8/Compose");
    file = fopen(path, "rb");
    if (file == NULL) {
        perror(path);
        free(path);
        xkb_context_unref(ctx);
        return -1;
    }
    free(path);

    xkb_context_set_log_level(ctx, XKB_LOG_LEVEL_CRITICAL);
    xkb_context_set_log_verbosity(ctx, 0);

    table = xkb_compose_table_new_from_file(ctx, file, "",
                                            XKB_COMPOSE_FORMAT_TEXT_V1,
                                            XKB_COMPOSE_COMPILE_NO_FLAGS);
    fclose(file);
    assert(table);

    bench_start(&bench);
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        iter = xkb_compose_table_iterator_new(table);
        while ((entry = xkb_compose_table_iterator_next(iter))) {
            assert (entry);
        }
        xkb_compose_table_iterator_free(iter);
    }
    bench_stop(&bench);

    xkb_compose_table_unref(table);

    elapsed = bench_elapsed_str(&bench);
    fprintf(stderr, "traversed %d compose tables in %ss\n",
            BENCHMARK_ITERATIONS, elapsed);
    free(elapsed);

    xkb_context_unref(ctx);
    return 0;
}
