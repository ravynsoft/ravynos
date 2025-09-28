/***********************************************************
 * Copyright 1987, 1998  The Open Group
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of The Open Group shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from The Open Group.
 *
 *
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                      All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 ******************************************************************/

/************************************************************
 * Copyright 1994 by Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of Silicon Graphics not be
 * used in advertising or publicity pertaining to distribution
 * of the software without specific prior written permission.
 * Silicon Graphics makes no representation about the suitability
 * of this software for any purpose. It is provided "as is"
 * without any express or implied warranty.
 *
 * SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 * GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 ********************************************************/

#include "config.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "atom.h"
#include "darray.h"
#include "utils.h"

/* FNV-1a (http://www.isthe.com/chongo/tech/comp/fnv/). */
static inline uint32_t
hash_buf(const char *string, size_t len)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < (len + 1) / 2; i++) {
        hash ^= (uint8_t) string[i];
        hash *= 0x01000193;
        hash ^= (uint8_t) string[len - 1 - i];
        hash *= 0x01000193;
    }
    return hash;
}

/*
 * The atom table is an insert-only linear probing hash table
 * mapping strings to atoms. Another array maps the atoms to
 * strings. The atom value is the position in the strings array.
 */
struct atom_table {
    xkb_atom_t *index;
    size_t index_size;
    darray(char *) strings;
};

struct atom_table *
atom_table_new(void)
{
    struct atom_table *table = calloc(1, sizeof(*table));
    if (!table)
        return NULL;

    darray_init(table->strings);
    darray_append(table->strings, NULL);
    table->index_size = 4;
    table->index = calloc(table->index_size, sizeof(*table->index));

    return table;
}

void
atom_table_free(struct atom_table *table)
{
    if (!table)
        return;

    char **string;
    darray_foreach(string, table->strings)
        free(*string);
    darray_free(table->strings);
    free(table->index);
    free(table);
}

const char *
atom_text(struct atom_table *table, xkb_atom_t atom)
{
    assert(atom < darray_size(table->strings));
    return darray_item(table->strings, atom);
}

xkb_atom_t
atom_intern(struct atom_table *table, const char *string, size_t len, bool add)
{
    if (darray_size(table->strings) > 0.80 * table->index_size) {
        table->index_size *= 2;
        table->index = realloc(table->index, table->index_size * sizeof(*table->index));
        memset(table->index, 0, table->index_size * sizeof(*table->index));
        for (size_t j = 1; j < darray_size(table->strings); j++) {
            const char *s = darray_item(table->strings, j);
            uint32_t hash = hash_buf(s, strlen(s));
            for (size_t i = 0; i < table->index_size; i++) {
                size_t index_pos = (hash + i) & (table->index_size - 1);
                if (index_pos == 0)
                    continue;

                xkb_atom_t atom = table->index[index_pos];
                if (atom == XKB_ATOM_NONE) {
                    table->index[index_pos] = j;
                    break;
                }
            }
        }
    }

    uint32_t hash = hash_buf(string, len);
    for (size_t i = 0; i < table->index_size; i++) {
        size_t index_pos = (hash + i) & (table->index_size - 1);
        if (index_pos == 0)
            continue;

        xkb_atom_t existing_atom = table->index[index_pos];
        if (existing_atom == XKB_ATOM_NONE) {
            if (add) {
                xkb_atom_t new_atom = darray_size(table->strings);
                darray_append(table->strings, strndup(string, len));
                table->index[index_pos] = new_atom;
                return new_atom;
            } else {
                return XKB_ATOM_NONE;
            }
        }

        const char *existing_value = darray_item(table->strings, existing_atom);
        if (strncmp(existing_value, string, len) == 0 && existing_value[len] == '\0')
            return existing_atom;
    }

    assert(!"couldn't find an empty slot during probing");
}
