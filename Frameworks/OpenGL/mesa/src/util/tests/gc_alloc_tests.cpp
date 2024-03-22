/*
 * Copyright © 2023 Valve Corporation
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
 */

#include <gtest/gtest.h>
#include "util/ralloc.h"

#if defined(__LP64__) || defined(_WIN64)
#define HEADER_ALIGN 16
#else
#define HEADER_ALIGN 8
#endif

TEST(gc_alloc, align)
{
   for (size_t size = 4; size <= 256; size += 4) {
      for (size_t align = 4; align <= HEADER_ALIGN; align *= 2) {
         gc_ctx *ctx = gc_context(NULL);

         for (unsigned i = 0; i < 16; i++) {
            uintptr_t ptr = (uintptr_t)gc_alloc_size(ctx, size, align);
            EXPECT_EQ(ptr % align, 0);
         }

         ralloc_free(ctx);
      }
   }
}
