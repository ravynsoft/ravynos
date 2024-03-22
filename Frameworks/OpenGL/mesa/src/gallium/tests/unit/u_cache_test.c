/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


/*
 * Test case for u_cache.
 */


#include <assert.h>
#include <stdio.h>

#include "util/u_cache.h"
#include "util/crc32.h"


typedef uint32_t cache_test_key;
typedef uint32_t cache_test_value;


static uint32_t
cache_test_hash(const void *key)
{
   return util_hash_crc32(key, sizeof(cache_test_key));
}


static void
cache_test_destroy(void *key, void *value)
{
   free(key);
   free(value);
}


static int
cache_test_compare(const void *key1, const void *key2) {
   return !(key1 == key2);
}


int main() {
   unsigned cache_size;
   unsigned cache_count;

   for (cache_size = 2; cache_size < (1 << 15); cache_size *= 2) {
      for (cache_count = (cache_size << 5); cache_count < (cache_size << 10); cache_count *= 2) {
         struct util_cache * cache;
         cache_test_key *key;
         cache_test_value *value_in;
         cache_test_value *value_out;
         int i;

         printf("Testing cache size of %d with %d values.\n", cache_size, cache_count);

         cache = util_cache_create(cache_test_hash,
                                   cache_test_compare,
                                   cache_test_destroy,
                                   cache_size);

         /*
          * Retrieve a value from an empty cache.
          */
         key = malloc(sizeof(cache_test_key));
         *key = 0xdeadbeef;
         value_out = (cache_test_value *) util_cache_get(cache, key);
         assert(value_out == NULL);
         (void)value_out;
         free(key);


         /*
          * Repeatedly insert into and retrieve values from the cache.
          */
         for (i = 0; i < cache_count; i++) {
            key = malloc(sizeof(cache_test_key));
            value_in = malloc(sizeof(cache_test_value));

            *key = rand();
            *value_in = rand();
            util_cache_set(cache, key, value_in);

            value_out = util_cache_get(cache, key);
            assert(value_out != NULL);
            assert(value_in == value_out);
            assert(*value_in == *value_out);
         }

         /*
          * In debug builds, this will trigger a self-check by the cache of
          * the distribution of hits in its internal cache entries.
          */
         util_cache_destroy(cache);
      }
   }

   return 0;
}
