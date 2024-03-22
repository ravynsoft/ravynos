/*
 * Copyright © 2009 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 */

#undef NDEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "util/hash_table.h"

#define SIZE 10000

static uint32_t
key_value(const void *key)
{
   return *(const uint32_t *)key;
}

static bool
uint32_t_key_equals(const void *a, const void *b)
{
   return key_value(a) == key_value(b);
}

int
main(int argc, char **argv)
{
   struct hash_table *ht;
   struct hash_entry *entry;
   uint32_t keys[SIZE];
   uint32_t i;

   (void) argc;
   (void) argv;

   ht = _mesa_hash_table_create(NULL, key_value, uint32_t_key_equals);

   for (i = 0; i < SIZE; i++) {
      keys[i] = i;

      _mesa_hash_table_insert(ht, keys + i, NULL);
   }

   for (i = 0; i < SIZE; i++) {
      entry = _mesa_hash_table_search(ht, keys + i);
      assert(entry);
      assert(key_value(entry->key) == i);
   }
   assert(ht->entries == SIZE);

   _mesa_hash_table_destroy(ht, NULL);

   return 0;
}
