/*
 * Copyright (C) 2021 Collabora, Ltd.
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

#include "compiler.h"

#include <gtest/gtest.h>

#define L(x) ((enum bi_clause_subword)(BI_CLAUSE_SUBWORD_LITERAL_0 + x))
#define U(x) ((enum bi_clause_subword)(BI_CLAUSE_SUBWORD_UPPER_0 + x))
#define T(x) ((enum bi_clause_subword)(BI_CLAUSE_SUBWORD_TUPLE_0 + x))
#define Z    BI_CLAUSE_SUBWORD_Z

TEST(Packing, PackLiteral)
{
   for (unsigned x = 0; x <= 7; ++x)
      EXPECT_EQ(bi_pack_literal(L(x)), x);
}

TEST(Packing, PackUpper)
{
   struct bi_packed_tuple tuples[] = {
      {0, 0x3 << (75 - 64)}, {0, 0x1 << (75 - 64)}, {0, 0x7 << (75 - 64)},
      {0, 0x0 << (75 - 64)}, {0, 0x2 << (75 - 64)}, {0, 0x6 << (75 - 64)},
      {0, 0x5 << (75 - 64)}, {0, 0x4 << (75 - 64)},
   };

   EXPECT_EQ(bi_pack_upper(U(0), tuples, 8), 3);
   EXPECT_EQ(bi_pack_upper(U(1), tuples, 8), 1);
   EXPECT_EQ(bi_pack_upper(U(2), tuples, 8), 7);
   EXPECT_EQ(bi_pack_upper(U(3), tuples, 8), 0);
   EXPECT_EQ(bi_pack_upper(U(4), tuples, 8), 2);
   EXPECT_EQ(bi_pack_upper(U(5), tuples, 8), 6);
   EXPECT_EQ(bi_pack_upper(U(6), tuples, 8), 5);
   EXPECT_EQ(bi_pack_upper(U(7), tuples, 8), 4);
}

TEST(Packing, PackTupleBits)
{
   struct bi_packed_tuple tuples[] = {
      {0x1234567801234567, 0x3A},
      {0x9876543299999999, 0x1B},
      {0xABCDEF0101234567, 0x7C},
   };

   EXPECT_EQ(bi_pack_tuple_bits(T(0), tuples, 8, 0, 30), 0x01234567);
   EXPECT_EQ(bi_pack_tuple_bits(T(1), tuples, 8, 10, 30), 0xca66666);
   EXPECT_EQ(bi_pack_tuple_bits(T(2), tuples, 8, 40, 15), 0x4def);
}

TEST(Packing, PackSync)
{
   struct bi_packed_tuple tuples[] = {
      {0, 0x3 << (75 - 64)}, {0, 0x5 << (75 - 64)}, {0, 0x7 << (75 - 64)},
      {0, 0x0 << (75 - 64)}, {0, 0x2 << (75 - 64)}, {0, 0x6 << (75 - 64)},
      {0, 0x5 << (75 - 64)}, {0, 0x4 << (75 - 64)},
   };

   EXPECT_EQ(bi_pack_sync(L(3), L(1), L(7), tuples, 8, false), 0xCF);
   EXPECT_EQ(bi_pack_sync(L(3), L(1), U(7), tuples, 8, false), 0xCC);
   EXPECT_EQ(bi_pack_sync(L(3), U(1), U(7), tuples, 8, false), 0xEC);
   EXPECT_EQ(bi_pack_sync(Z, U(1), U(7), tuples, 8, false), 0x2C);
   EXPECT_EQ(bi_pack_sync(Z, U(1), U(7), tuples, 8, true), 0x6C);
}
