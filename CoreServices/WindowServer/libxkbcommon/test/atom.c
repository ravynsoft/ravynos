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

#include <time.h>

#include "test.h"
#include "atom.h"

#define INTERN_LITERAL(table, literal) \
    atom_intern(table, literal, sizeof(literal) - 1, true)

#define LOOKUP_LITERAL(table, literal) \
    atom_intern(table, literal, sizeof(literal) - 1, false)

static void
random_string(char **str_out, size_t *len_out)
{
    /* Keep this small, so collisions might happen. */
    static const char random_chars[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g'
    };

    size_t len;
    char *str;

    len = rand() % 15;
    str = malloc(len + 1);
    assert(str);

    for (size_t i = 0; i < len; i++)
        str[i] = random_chars[rand() % ARRAY_SIZE(random_chars)];
    /* Don't always terminate it; should work without. */
    if (rand() % 2 == 0)
        str[len] = '\0';

    *str_out = str;
    *len_out = len;
}

static void
test_random_strings(void)
{
    struct atom_string {
        xkb_atom_t atom;
        char *string;
        size_t len;
    };

    struct atom_table *table;
    struct atom_string *arr;
    int N;
    xkb_atom_t atom;
    const char *string;

    table = atom_table_new();
    assert(table);

    unsigned seed = (unsigned) clock();
    srand(seed);

    N = 1 + rand() % 100000;
    arr = calloc(N, sizeof(*arr));
    assert(arr);

    for (int i = 0; i < N; i++) {
        random_string(&arr[i].string, &arr[i].len);

        atom = atom_intern(table, arr[i].string, arr[i].len, false);
        if (atom != XKB_ATOM_NONE) {
            string = atom_text(table, atom);
            assert(string);

            if (arr[i].len != strlen(string) ||
                strncmp(string, arr[i].string, arr[i].len) != 0) {
                fprintf(stderr, "got a collision, but strings don't match!\n");
                fprintf(stderr, "existing length %zu, string %s\n",
                        strlen(string), string);
                fprintf(stderr, "new length %zu, string %.*s\n",
                        arr[i].len, (int) arr[i].len, arr[i].string);
                fprintf(stderr, "seed: %u\n", seed);
                assert(false);
            }

            /* OK, got a real collision. */
            free(arr[i].string);
            i--;
            continue;
        }

        arr[i].atom = atom_intern(table, arr[i].string, arr[i].len, true);
        if (arr[i].atom == XKB_ATOM_NONE) {
            fprintf(stderr, "failed to intern! len: %zu, string: %.*s\n",
                    arr[i].len, (int) arr[i].len, arr[i].string);
            fprintf(stderr, "seed: %u\n", seed);
            assert(false);
        }
    }

    for (int i = 0; i < N; i++) {
        string = atom_text(table, arr[i].atom);
        assert(string);

        if (arr[i].len != strlen(string) ||
            strncmp(string, arr[i].string, arr[i].len) != 0) {
            fprintf(stderr, "looked-up string doesn't match!\n");
            fprintf(stderr, "found length %zu, string %s\n",
                    strlen(string), string);
            fprintf(stderr, "expected length %zu, string %.*s\n",
                    arr[i].len, (int) arr[i].len, arr[i].string);

            /* Since this is random, we need to dump the failing data,
             * so we might have some chance to reproduce. */
            fprintf(stderr, "START dump of arr, N=%d\n", N);
            for (int j = 0; j < N; j++) {
                fprintf(stderr, "%u\t\t%zu\t\t%.*s\n", arr[i].atom,
                        arr[i].len, (int) arr[i].len, arr[i].string);
            }
            fprintf(stderr, "END\n");

            fprintf(stderr, "seed: %u\n", seed);
            assert(false);
        }
    }

    for (int i = 0; i < N; i++)
        free(arr[i].string);
    free(arr);
    atom_table_free(table);
}

int
main(void)
{
    struct atom_table *table;
    xkb_atom_t atom1, atom2, atom3;

    table = atom_table_new();
    assert(table);

    assert(atom_text(table, XKB_ATOM_NONE) == NULL);
    assert(atom_intern(table, NULL, 0, false) == XKB_ATOM_NONE);

    atom1 = INTERN_LITERAL(table, "hello");
    assert(atom1 != XKB_ATOM_NONE);
    assert(atom1 == LOOKUP_LITERAL(table, "hello"));
    assert(streq(atom_text(table, atom1), "hello"));

    atom2 = atom_intern(table, "hello", 3, true);
    assert(atom2 != XKB_ATOM_NONE);
    assert(atom1 != atom2);
    assert(streq(atom_text(table, atom2), "hel"));
    assert(LOOKUP_LITERAL(table, "hel") == atom2);
    assert(LOOKUP_LITERAL(table, "hell") == XKB_ATOM_NONE);
    assert(LOOKUP_LITERAL(table, "hello") == atom1);

    atom3 = atom_intern(table, "", 0, true);
    assert(atom3 != XKB_ATOM_NONE);
    assert(LOOKUP_LITERAL(table, "") == atom3);

    atom_table_free(table);

    test_random_strings();

    return 0;
}
