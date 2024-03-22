/*
 * Copyright © 2023 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include "util/ralloc.h"

TEST(LinearAlloc, Basic)
{
   void *ctx = ralloc_context(NULL);
   linear_ctx *lin_ctx = linear_context(ctx);

   for (unsigned i = 0; i < 1024; i++) {
      linear_alloc_child(lin_ctx, i * 4);
   }

   ralloc_free(ctx);
}

TEST(LinearAlloc, RallocParent)
{
   void *ctx = ralloc_context(NULL);
   linear_ctx *lin_ctx = linear_context(ctx);
   EXPECT_EQ(ralloc_parent_of_linear_context(lin_ctx), ctx);
   ralloc_free(ctx);
}

TEST(LinearAlloc, StrCat)
{
   void *ctx = ralloc_context(NULL);
   linear_ctx *lin_ctx = linear_context(ctx);

   char *s = linear_strdup(lin_ctx, "hello,");
   bool ok = linear_strcat(lin_ctx, &s, " triangle");
   EXPECT_TRUE(ok);
   EXPECT_STREQ(s, "hello, triangle");

   ralloc_free(ctx);
}

TEST(LinearAlloc, RewriteTail)
{
   void *ctx = ralloc_context(NULL);
   linear_ctx *lin_ctx = linear_context(ctx);

   char *s = linear_strdup(lin_ctx, "hello, world");
   size_t start = 7;
   bool ok = linear_asprintf_rewrite_tail(lin_ctx, &s, &start, "%s", "triangle");
   EXPECT_TRUE(ok);
   EXPECT_STREQ(s, "hello, triangle");
   EXPECT_EQ(start, 7 + 8);

   ralloc_free(ctx);
}

TEST(LinearAlloc, AvoidWasteAfterLargeAlloc)
{
   void *ctx = ralloc_context(NULL);
   linear_ctx *lin_ctx = linear_context(ctx);

   char *first = (char *) linear_alloc_child(lin_ctx, 32);

   /* Large allocation that would force a larger buffer. */
   linear_alloc_child(lin_ctx, 1024 * 16);

   char *second = (char *) linear_alloc_child(lin_ctx, 32);

   EXPECT_EQ(second - first, 32);

   ralloc_free(ctx);
}

TEST(LinearAlloc, Options)
{
   void *ctx = ralloc_context(NULL);

   linear_opts opts = {};
   opts.min_buffer_size = 8192;

   linear_ctx *lin_ctx = linear_context_with_opts(ctx, &opts);

   /* Assert allocations spanning the first 8192 bytes are contiguous. */
   char *first = (char *)linear_alloc_child(lin_ctx, 1024);
   for (int i = 1; i < 8; i++) {
      char *ptr = (char *)linear_alloc_child(lin_ctx, 1024);
      EXPECT_EQ(ptr - first, 1024 * i);
   }

   ralloc_free(ctx);
}
