/*
 * Copyright © 2017 Intel Corporation
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

#include "mesa-sha1.h"

#include <gtest/gtest.h>

#define SHA1_LENGTH 40

struct Params {
   const char *string;
   const char *expected_sha1;
};

static const Params test_data[] = {
   {"Mesa Rocks! 273", "7fb99737373d65a73f049cdabc01e73aa6bc60f3"},
   {"Mesa Rocks! 300", "b2180263e37d3bed6a4be0afe41b1a82ebbcf4c3"},
   {"Mesa Rocks! 583", "7fb9734108a62503e8a149c1051facd7fb112d05"},
};

class MesaSHA1TestFixture : public testing::TestWithParam<Params> {};
INSTANTIATE_TEST_SUITE_P(
   MesaSHA1Test,
   MesaSHA1TestFixture,
   testing::ValuesIn(test_data)
);

TEST_P(MesaSHA1TestFixture, Match)
{
   Params p = GetParam();

   unsigned char sha1[20];
   _mesa_sha1_compute(p.string, strlen(p.string), sha1);

   char buf[41];
   _mesa_sha1_format(buf, sha1);

   ASSERT_TRUE(memcmp(buf, p.expected_sha1, SHA1_LENGTH) == 0)
      << "For string \"" << p.string << "\", length " << strlen(p.string) << ":\n"
      << "\t  Actual: " << buf << "\n"
      << "\tExpected: " << p.expected_sha1 << "\n";
}
