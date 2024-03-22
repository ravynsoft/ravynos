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

int
main(int argc, char **argv)
{
   struct hash_table *ht;
   const char *str1 = "test1";
   const char *str2 = "test2";
   struct hash_entry *entry;

   (void) argc;
   (void) argv;

   ht = _mesa_hash_table_create(NULL, _mesa_hash_string,
                                _mesa_key_string_equal);

   _mesa_hash_table_insert(ht, str1, NULL);
   _mesa_hash_table_insert(ht, str2, NULL);

   entry = _mesa_hash_table_search(ht, str1);
   assert(strcmp(entry->key, str1) == 0);

   entry = _mesa_hash_table_search(ht, str2);
   assert(strcmp(entry->key, str2) == 0);

   _mesa_hash_table_destroy(ht, NULL);

   return 0;
}
