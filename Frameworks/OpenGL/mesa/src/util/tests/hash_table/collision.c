/*
 * Copyright © 2012 Intel Corporation
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

static void entry_free(struct hash_entry *entry)
{
   free((void *)entry->key);
}

int
main(int argc, char **argv)
{
   struct hash_table *ht;
   const char *str1 = strdup("test1");
   const char *str2 = strdup("test2");
   const char *str3 = strdup("test3");
   struct hash_entry *entry1, *entry2;
   uint32_t bad_hash = 5;
   int i;

   (void) argc;
   (void) argv;

   ht = _mesa_hash_table_create(NULL, NULL, _mesa_key_string_equal);

   /* Insert some items.  Inserting 3 items forces a rehash and the new
    * table size is big enough that we don't get rehashes later.
    */
   _mesa_hash_table_insert_pre_hashed(ht, bad_hash, str1, NULL);
   _mesa_hash_table_insert_pre_hashed(ht, bad_hash, str2, NULL);
   _mesa_hash_table_insert_pre_hashed(ht, bad_hash, str3, NULL);

   entry1 = _mesa_hash_table_search_pre_hashed(ht, bad_hash, str1);
   assert(entry1->key == str1);

   entry2 = _mesa_hash_table_search_pre_hashed(ht, bad_hash, str2);
   assert(entry2->key == str2);

   /* Check that we can still find #1 after inserting #2 */
   entry1 = _mesa_hash_table_search_pre_hashed(ht, bad_hash, str1);
   assert(entry1->key == str1);

   /* Remove the collided entry and look again. */
   _mesa_hash_table_remove(ht, entry1);
   entry2 = _mesa_hash_table_search_pre_hashed(ht, bad_hash, str2);
   assert(entry2->key == str2);

   /* Try inserting #2 again and make sure it gets overwritten */
   _mesa_hash_table_insert_pre_hashed(ht, bad_hash, str2, NULL);
   entry2 = _mesa_hash_table_search_pre_hashed(ht, bad_hash, str2);
   hash_table_foreach(ht, search_entry) {
      assert(search_entry == entry2 || search_entry->key != str2);
   }

   /* Put str1 back, then spam junk into the table to force a
    * resize and make sure we can still find them both.
    */
   _mesa_hash_table_insert_pre_hashed(ht, bad_hash, str1, NULL);
   for (i = 0; i < 100; i++) {
      char *key = malloc(10);
      sprintf(key, "spam%d", i);
      _mesa_hash_table_insert_pre_hashed(ht, _mesa_hash_string(key), key, NULL);
   }
   entry1 = _mesa_hash_table_search_pre_hashed(ht, bad_hash, str1);
   assert(entry1->key == str1);
   entry2 = _mesa_hash_table_search_pre_hashed(ht, bad_hash, str2);
   assert(entry2->key == str2);

   _mesa_hash_table_destroy(ht, entry_free);

   return 0;
}
