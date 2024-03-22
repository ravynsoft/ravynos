/*
 * Copyright (C) 2016 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#undef NDEBUG

#include "util/hash_table.h"

#define SIZE 1000

static void *make_key(uint32_t i)
{
      return (void *)(uintptr_t)(1 + i);
}

static uint32_t key_id(const void *key)
{
   return (uintptr_t)key - 1;
}

static uint32_t key_hash(const void *key)
{
   return (uintptr_t)key;
}

static bool key_equal(const void *a, const void *b)
{
   return a == b;
}

static void delete_function(struct hash_entry *entry)
{
   bool *deleted = (bool *)entry->data;
   assert(!*deleted);
   *deleted = true;
}

int main()
{
   struct hash_table *ht;
   bool flags[SIZE];
   uint32_t i;

   ht = _mesa_hash_table_create(NULL, key_hash, key_equal);

   for (i = 0; i < SIZE; ++i) {
      flags[i] = false;
      _mesa_hash_table_insert(ht, make_key(i), &flags[i]);
   }

   _mesa_hash_table_clear(ht, delete_function);
   assert(_mesa_hash_table_next_entry(ht, NULL) == NULL);

   /* Check that delete_function was called and that repopulating the table
    * works. */
   for (i = 0; i < SIZE; ++i) {
      assert(flags[i]);
      flags[i] = false;
      _mesa_hash_table_insert(ht, make_key(i), &flags[i]);
   }

   /* Check that exactly the right set of entries is in the table. */
   for (i = 0; i < SIZE; ++i) {
      assert(_mesa_hash_table_search(ht, make_key(i)));
   }

   hash_table_foreach(ht, entry) {
      assert(key_id(entry->key) < SIZE);
   }
   _mesa_hash_table_clear(ht, NULL);
   assert(!ht->entries);
   assert(!ht->deleted_entries);
   hash_table_foreach(ht, entry) {
      assert(0);
   }

   for (i = 0; i < SIZE; ++i) {
      flags[i] = false;
      _mesa_hash_table_insert(ht, make_key(i), &flags[i]);
   }
   hash_table_foreach_remove(ht, entry) {
      assert(key_id(entry->key) < SIZE);
   }
   assert(!ht->entries);
   assert(!ht->deleted_entries);

   _mesa_hash_table_destroy(ht, NULL);

   return 0;
}
