/*
 * Copyright © 2018 Intel Corporation
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
 */

#include <gtest/gtest.h>
#include "util/hash_table.h"
#include "util/set.h"

TEST(set, basic)
{
   struct set *s = _mesa_set_create(NULL, _mesa_hash_pointer,
                                    _mesa_key_pointer_equal);
   struct set_entry *entry;

   const void *a = (const void *)10;
   const void *b = (const void *)20;

   _mesa_set_add(s, a);
   _mesa_set_add(s, b);
   EXPECT_EQ(s->entries, 2);

   _mesa_set_add(s, a);
   EXPECT_EQ(s->entries, 2);

   entry = _mesa_set_search(s, a);
   EXPECT_TRUE(entry);
   EXPECT_EQ(entry->key, a);

   _mesa_set_remove(s, entry);
   EXPECT_EQ(s->entries, 1);

   entry = _mesa_set_search(s, a);
   EXPECT_FALSE(entry);

   _mesa_set_clear(s, NULL);
   EXPECT_EQ(s->entries, 0);
   EXPECT_EQ(s->deleted_entries, 0);
   set_foreach(s, he) {
      GTEST_FAIL();
   }

   _mesa_set_add(s, a);
   _mesa_set_add(s, b);
   EXPECT_EQ(s->entries, 2);
   unsigned count = s->entries;
   set_foreach_remove(s, he) {
      EXPECT_TRUE(he->key == a || he->key == b);
      EXPECT_EQ(s->entries, count--);
      EXPECT_EQ(s->deleted_entries, 0);
   }
   EXPECT_EQ(s->entries, 0);
   set_foreach(s, he) {
      GTEST_FAIL();
   }

   _mesa_set_destroy(s, NULL);
}

TEST(set, clone)
{
   struct set *s = _mesa_set_create(NULL, _mesa_hash_pointer,
                                    _mesa_key_pointer_equal);
   struct set_entry *entry;

   const void *a = (const void *)10;
   const void *b = (const void *)20;
   const void *c = (const void *)30;

   _mesa_set_add(s, a);
   _mesa_set_add(s, b);
   _mesa_set_add(s, c);

   entry = _mesa_set_search(s, c);
   EXPECT_TRUE(entry);
   EXPECT_EQ(entry->key, c);

   _mesa_set_remove(s, entry);
   EXPECT_EQ(s->entries, 2);

   struct set *clone = _mesa_set_clone(s, NULL);
   EXPECT_EQ(clone->entries, 2);

   EXPECT_TRUE(_mesa_set_search(clone, a));
   EXPECT_TRUE(_mesa_set_search(clone, b));
   EXPECT_FALSE(_mesa_set_search(clone, c));

   _mesa_set_destroy(s, NULL);
   _mesa_set_destroy(clone, NULL);
}

TEST(set, remove_key)
{
   struct set *s = _mesa_set_create(NULL, _mesa_hash_pointer,
                                    _mesa_key_pointer_equal);

   const void *a = (const void *)10;
   const void *b = (const void *)20;
   const void *c = (const void *)30;

   _mesa_set_add(s, a);
   _mesa_set_add(s, b);
   EXPECT_EQ(s->entries, 2);

   /* Remove existing key. */
   _mesa_set_remove_key(s, a);
   EXPECT_EQ(s->entries, 1);
   EXPECT_FALSE(_mesa_set_search(s, a));
   EXPECT_TRUE(_mesa_set_search(s, b));

   /* Remove non-existing key. */
   _mesa_set_remove_key(s, c);
   EXPECT_EQ(s->entries, 1);
   EXPECT_FALSE(_mesa_set_search(s, a));
   EXPECT_TRUE(_mesa_set_search(s, b));

   _mesa_set_destroy(s, NULL);
}

static uint32_t hash_int(const void *p)
{
   int i = *(const int *)p;
   return i;
}

static bool cmp_int(const void *p1, const void *p2)
{
   int i1 = *(const int *)p1, i2 = *(const int *)p2;
   return i1 == i2;
}

TEST(set, search_or_add)
{
   struct set *s = _mesa_set_create(NULL, hash_int, cmp_int);

   int a = 10, b = 20, c = 20, d = 30;

   _mesa_set_add(s, &a);
   _mesa_set_add(s, &b);
   EXPECT_EQ(s->entries, 2);

   bool found = false;
   struct set_entry *entry = _mesa_set_search_or_add(s, &c, &found);
   EXPECT_EQ(entry->key, (void *)&b);
   EXPECT_EQ(found, true);
   EXPECT_EQ(s->entries, 2);

   found = false;
   struct set_entry *entry3 = _mesa_set_search_or_add(s, &d, &found);
   EXPECT_EQ(entry3->key, &d);
   EXPECT_EQ(found, false);
   EXPECT_EQ(s->entries, 3);

   _mesa_set_destroy(s, NULL);
}
