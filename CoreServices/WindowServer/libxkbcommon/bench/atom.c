/*
 * Copyright © 2021 Ran Benita <ran@unusedvar.com>
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "atom.h"
#include "bench.h"
#include "darray.h"

#define BENCHMARK_ITERATIONS 100

int
main(void)
{
    FILE *file;
    char wordbuf[1024];
    darray(char *) words;
    char **worditer;
    struct atom_table *table;
    xkb_atom_t atom;
    const char *text;
    struct bench bench;
    char *elapsed;

    darray_init(words);
    file = fopen("/usr/share/dict/words", "rb");
    if (file == NULL) {
        perror("/usr/share/dict/words");
        return -1;
    }
    while (fgets(wordbuf, sizeof(wordbuf), file)) {
        size_t len = strlen(wordbuf);
        if (len > 0 && wordbuf[len - 1] == '\n')
            wordbuf[len - 1] = '\0';
        darray_append(words, strdup(wordbuf));
    }
    fclose(file);

    bench_start(&bench);
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        table = atom_table_new();
        assert(table);

        darray_foreach(worditer, words) {
            atom = atom_intern(table, *worditer, strlen(*worditer) - 1, true);
            assert(atom != XKB_ATOM_NONE);

            text = atom_text(table, atom);
            assert(text != NULL);
        }

        atom_table_free(table);
    }
    bench_stop(&bench);

    elapsed = bench_elapsed_str(&bench);
    fprintf(stderr, "%d iterations in %ss\n",
            BENCHMARK_ITERATIONS, elapsed);
    free(elapsed);

    darray_foreach(worditer, words) {
        free(*worditer);
    }
    darray_free(words);

    return 0;
}
