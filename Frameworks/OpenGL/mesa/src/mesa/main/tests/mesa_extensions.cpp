/*
 * Copyright © 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \name mesa_extensions.cpp
 *
 * Verify that the extensions table is sorted.
 */

#include <gtest/gtest.h>
#include "util/macros.h"

/**
 * Debug/test: verify the extension table is alphabetically sorted.
 */
TEST(MesaExtensionsTest, AlphabeticallySorted)
{
   const char *ext_names[] = {
   #define EXT(name_str, ...) #name_str,
   #include "main/extensions_table.h"
   #undef EXT
   };

   for (unsigned i = 0; i < ARRAY_SIZE(ext_names) - 1; ++i) {
      const char *current_str = ext_names[i];
      const char *next_str = ext_names[i+1];

      /* We expect the extension table to be alphabetically sorted */
      ASSERT_LT(strcmp(current_str, next_str), 0);
   }
}
