/*
 * Copyright © 2019 Google, LLC
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

#include "util/u_vector.h"
#include "gtest/gtest.h"

static void test(uint32_t size_in_elements, uint32_t elements_to_walk, uint32_t start)
{
   struct u_vector vector;
   uint32_t add_counter = 0;
   uint32_t remove_counter = 0;

   ASSERT_TRUE(u_vector_init(&vector, size_in_elements, sizeof(uint64_t)));

   // Override the head and tail so we can quickly test rollover
   vector.head = vector.tail = start;

   EXPECT_EQ(sizeof(uint64_t) * size_in_elements, vector.size);
   EXPECT_EQ(0, u_vector_length(&vector));

   for (uint32_t i = 0; i < size_in_elements; i++) {
      *(uint64_t*)u_vector_add(&vector) = add_counter++;

      int length = u_vector_length(&vector);
      EXPECT_EQ(i + 1, length);

      // Check the entries
      uint32_t count = 0;
      void* element;
      u_vector_foreach(element, &vector)
      {
         EXPECT_EQ(count, *(uint64_t*)element) << "i: " << i << " count: " << count;
         count++;
      }
      EXPECT_EQ(count, length);
   }

   // Remove + add
   for (uint32_t i = 0; i < elements_to_walk; i++) {
      u_vector_remove(&vector);
      remove_counter++;
      *(uint64_t*)u_vector_add(&vector) = add_counter++;
   }

   EXPECT_EQ(sizeof(uint64_t) * size_in_elements, vector.size);

   // Grow the vector now
   *(uint64_t*)u_vector_add(&vector) = add_counter++;
   EXPECT_EQ(size_in_elements + 1, u_vector_length(&vector));

   EXPECT_EQ(sizeof(uint64_t) * size_in_elements * 2, vector.size);

   {
      uint32_t count = remove_counter;
      void* element;
      u_vector_foreach(element, &vector)
      {
         EXPECT_EQ(count++, *(uint64_t*)element) << "count: " << count;
      }
   }

   u_vector_finish(&vector);
}

TEST(Vector, Grow0) { test(4, 0, 0); }

TEST(Vector, Grow1) { test(4, 1, 0); }

TEST(Vector, Grow2) { test(4, 2, 0); }

TEST(Vector, Grow3) { test(4, 3, 0); }

TEST(Vector, Grow4) { test(4, 4, 0); }

TEST(Vector, Grow5) { test(4, 5, 0); }

TEST(Vector, Rollover)
{
   uint32_t start = (1ull << 32) - 4 * sizeof(uint64_t);
   test(8, 4, start);
}
