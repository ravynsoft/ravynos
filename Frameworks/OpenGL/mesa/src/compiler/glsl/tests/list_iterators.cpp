/*
 * Copyright © 2020 Intel Corporation
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

#include "list.h"

class test_node_inherite : public exec_node  {
public:
   uint32_t value;

   virtual ~test_node_inherite() = default;
};

class list_iterators_node_inherite : public ::testing::TestWithParam<size_t> {
public:
   virtual void SetUp();
   virtual void TearDown();

   void *mem_ctx;

   exec_list node_list;
};

void
list_iterators_node_inherite::SetUp()
{
   mem_ctx = ralloc_context(NULL);

   exec_list_make_empty(&node_list);

   for (size_t i = 0; i < GetParam(); i++) {
      test_node_inherite *node = new(mem_ctx) test_node_inherite();
      node->value = i;
      exec_list_push_tail(&node_list, node);
   }
}

void
list_iterators_node_inherite::TearDown()
{
   exec_list_make_empty(&node_list);

   ralloc_free(mem_ctx);
   mem_ctx = NULL;
}

INSTANTIATE_TEST_SUITE_P(
   list_iterators_node_inherite,
   list_iterators_node_inherite,
   ::testing::Values(0, 1, 10)
);

TEST_P(list_iterators_node_inherite, foreach_in_list)
{
   size_t i = 0;
   foreach_in_list(test_node_inherite, n, &node_list) {
      EXPECT_EQ(n->value, i);
      i++;
   }
}

TEST_P(list_iterators_node_inherite, foreach_in_list_reverse)
{
   size_t i = GetParam() - 1;
   foreach_in_list_reverse(test_node_inherite, n, &node_list) {
      EXPECT_EQ(n->value, i);
      i--;
   }
}

TEST_P(list_iterators_node_inherite, foreach_in_list_safe)
{
   size_t i = 0;
   foreach_in_list_safe(test_node_inherite, n, &node_list) {
      EXPECT_EQ(n->value, i);

      if (i % 2 == 0) {
         n->remove();
      }

      i++;
   }

   exec_list_validate(&node_list);
}

TEST_P(list_iterators_node_inherite, foreach_in_list_reverse_safe)
{
   size_t i = GetParam() - 1;
   foreach_in_list_reverse_safe(test_node_inherite, n, &node_list) {
      EXPECT_EQ(n->value, i);

      if (i % 2 == 0) {
         n->remove();
      }

      i--;
   }

   exec_list_validate(&node_list);
}

TEST_P(list_iterators_node_inherite, foreach_in_list_use_after)
{
   size_t i = 0;
   foreach_in_list_use_after(test_node_inherite, n, &node_list) {
      EXPECT_EQ(n->value, i);

      if (i == GetParam() / 2) {
         break;
      }

      i++;
   }

   if (GetParam() > 0) {
      EXPECT_EQ(n->value, GetParam() / 2);
   }
}

class test_node_embed {
   DECLARE_RZALLOC_CXX_OPERATORS(test_node_embed)
public:

   uint32_t value_header;
   exec_node node;
   uint32_t value_footer;

   virtual ~test_node_embed() = default;
};

class list_iterators_node_embed : public ::testing::TestWithParam<size_t> {
public:
   virtual void SetUp();
   virtual void TearDown();

   void *mem_ctx;

   exec_list node_list;
};

void
list_iterators_node_embed::SetUp()
{
   mem_ctx = ralloc_context(NULL);

   exec_list_make_empty(&node_list);

   for (size_t i = 0; i < GetParam(); i++) {
      test_node_embed *node = new(mem_ctx) test_node_embed();
      node->value_header = i;
      node->value_footer = i;
      exec_list_push_tail(&node_list, &node->node);
   }
}

void
list_iterators_node_embed::TearDown()
{
   exec_list_make_empty(&node_list);

   ralloc_free(mem_ctx);
   mem_ctx = NULL;
}

INSTANTIATE_TEST_SUITE_P(
   list_iterators_node_embed,
   list_iterators_node_embed,
   ::testing::Values(0, 1, 10)
);

TEST_P(list_iterators_node_embed, foreach_list_typed)
{
   size_t i = 0;
   foreach_list_typed(test_node_embed, n, node, &node_list) {
      EXPECT_EQ(n->value_header, i);
      EXPECT_EQ(n->value_footer, i);
      i++;
   }
}

TEST_P(list_iterators_node_embed, foreach_list_typed_from)
{
   if (GetParam() == 0) {
      return;
   }

   exec_node *start_node = node_list.get_head();

   size_t i = 0;
   for (; i < GetParam() / 2; i++) {
      start_node = start_node->get_next();
   }

   foreach_list_typed_from(test_node_embed, n, node, &node_list, start_node) {
      EXPECT_EQ(n->value_header, i);
      EXPECT_EQ(n->value_footer, i);
      i++;
   }
}

TEST_P(list_iterators_node_embed, foreach_list_typed_reverse)
{
   size_t i = GetParam() - 1;
   foreach_list_typed_reverse(test_node_embed, n, node, &node_list) {
      EXPECT_EQ(n->value_header, i);
      EXPECT_EQ(n->value_footer, i);
      i--;
   }
}

TEST_P(list_iterators_node_embed, foreach_list_typed_safe)
{
   size_t i = 0;
   foreach_list_typed_safe(test_node_embed, n, node, &node_list) {
      EXPECT_EQ(n->value_header, i);
      EXPECT_EQ(n->value_footer, i);

      if (i % 2 == 0) {
         exec_node_remove(&n->node);
      }

      i++;
   }

   exec_list_validate(&node_list);
}

TEST_P(list_iterators_node_embed, foreach_list_typed_reverse_safe)
{
   size_t i = GetParam() - 1;
   foreach_list_typed_reverse_safe(test_node_embed, n, node, &node_list) {
      EXPECT_EQ(n->value_header, i);
      EXPECT_EQ(n->value_footer, i);

      if (i % 2 == 0) {
         exec_node_remove(&n->node);
      }

      i--;
   }

   exec_list_validate(&node_list);
}
