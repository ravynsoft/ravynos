/*
 * Copyright © 2014 Intel Corporation
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
#include <math.h>
#include "brw_reg.h"

class vf_float_conversion_test : public ::testing::Test {
   virtual void SetUp();

public:
   float vf_to_float[128];
};

void vf_float_conversion_test::SetUp() {
   /* 0 is special cased. */
   vf_to_float[0] = 0.0;

   for (int vf = 1; vf < 128; vf++) {
      int ebits = (vf >> 4) & 0x7;
      int mbits = vf & 0xf;

      float x = 1.0f + mbits / 16.0f;
      int exp = ebits - 3;

      vf_to_float[vf] = ldexpf(x, exp);
   }
}

union fu {
   float f;
   unsigned u;
};

static unsigned
f2u(float f)
{
   union fu fu;
   fu.f = f;
   return fu.u;
}

TEST_F(vf_float_conversion_test, test_vf_to_float)
{
   for (int vf = 0; vf < 256; vf++) {
      float expected = vf_to_float[vf % 128];
      if (vf > 127)
         expected = -expected;

      EXPECT_EQ(f2u(expected), f2u(brw_vf_to_float(vf)));
   }
}

TEST_F(vf_float_conversion_test, test_float_to_vf)
{
   for (int vf = 0; vf < 256; vf++) {
      float f = vf_to_float[vf % 128];
      if (vf > 127)
         f = -f;

      EXPECT_EQ(vf, brw_float_to_vf(f));
   }
}

TEST_F(vf_float_conversion_test, test_special_case_0)
{
   /* ±0.0f are special cased to the VFs that would otherwise correspond
    * to ±0.125f. Make sure we can't convert these values to VF.
    */
   EXPECT_EQ(brw_float_to_vf(+0.125f), -1);
   EXPECT_EQ(brw_float_to_vf(-0.125f), -1);

   EXPECT_EQ(f2u(brw_vf_to_float(brw_float_to_vf(+0.0f))), f2u(+0.0f));
   EXPECT_EQ(f2u(brw_vf_to_float(brw_float_to_vf(-0.0f))), f2u(-0.0f));
}

TEST_F(vf_float_conversion_test, test_nonrepresentable_float_input)
{
   EXPECT_EQ(brw_float_to_vf(+32.0f), -1);
   EXPECT_EQ(brw_float_to_vf(-32.0f), -1);

   EXPECT_EQ(brw_float_to_vf(+16.5f), -1);
   EXPECT_EQ(brw_float_to_vf(-16.5f), -1);

   EXPECT_EQ(brw_float_to_vf(+8.25f), -1);
   EXPECT_EQ(brw_float_to_vf(-8.25f), -1);

   EXPECT_EQ(brw_float_to_vf(+4.125f), -1);
   EXPECT_EQ(brw_float_to_vf(-4.125f), -1);
}
