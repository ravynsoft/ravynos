/*
 * Copyright © 2020 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <gtest/gtest.h>

#include "util/macros.h"
#include "util/u_debug_stack.h"

static void ATTRIBUTE_NOINLINE
func_a(void)
{
   struct debug_stack_frame backtrace[16];

   fprintf(stderr, "--- backtrace from func_a:\n");
   debug_backtrace_capture(backtrace, 0, 16);
   debug_backtrace_dump(backtrace, 16);
}

static void ATTRIBUTE_NOINLINE
func_b(void)
{
   struct debug_stack_frame backtrace[16];

   func_a();

   fprintf(stderr, "--- backtrace from func_b:\n");
   debug_backtrace_capture(backtrace, 0, 16);
   debug_backtrace_dump(backtrace, 16);
}

static void ATTRIBUTE_NOINLINE
func_c(struct debug_stack_frame *frames)
{
   debug_backtrace_capture(frames, 0, 16);
}

TEST(u_debug_stack_test, basics)
{
   struct debug_stack_frame backtrace[16];
   struct debug_stack_frame stored_backtrace[16];

   func_c(stored_backtrace);

   fprintf(stderr, "--- backtrace from main to stderr:\n");
   debug_backtrace_capture(backtrace, 0, 16);
   debug_backtrace_print(stderr, backtrace, 16);

   fprintf(stderr, "--- backtrace from main again to debug_printf:\n");
   debug_backtrace_capture(backtrace, 0, 16);
   debug_backtrace_dump(backtrace, 16);

   func_a();

   func_b();

   fprintf(stderr, "--- stored backtrace from start of main:\n");
   debug_backtrace_dump(stored_backtrace, 16);
}

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)

TEST(u_debug_stack_test, capture_not_overwritten)
{
   struct debug_stack_frame backtrace1[16], backtrace2[16];

   FILE *fp;
   char *bt1, *bt2;
   size_t size;

   /* Old android implementation uses one global capture per thread. Test that
    * we can store multiple captures and that they decode to different
    * backtraces.
    */

   func_c(backtrace1);
   debug_backtrace_capture(backtrace2, 0, 16);

   fp = open_memstream(&bt1, &size);
   debug_backtrace_print(fp, backtrace1, 16);
   fclose(fp);

   fp = open_memstream(&bt2, &size);
   debug_backtrace_print(fp, backtrace2, 16);
   fclose(fp);

   if (size > 0) {
      EXPECT_STRNE(bt1, bt2);
   }

   free(bt1);
   free(bt2);
}

#endif
