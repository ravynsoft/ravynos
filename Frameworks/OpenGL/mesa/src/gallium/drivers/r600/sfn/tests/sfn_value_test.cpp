/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2021 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "../sfn_alu_defines.h"
#include "../sfn_debug.h"
#include "../sfn_virtualvalues.h"

#include "gtest/gtest.h"

using namespace r600;

class ValueTest : public ::testing::Test {
   void SetUp() override { init_pool(); }

   void TearDown() override { release_pool(); }
};

TEST_F(ValueTest, gpr_register_fully_pinned)
{
   Register reg(1, 2, pin_fully);

   EXPECT_EQ(reg.sel(), 1);
   EXPECT_EQ(reg.chan(), 2);
   EXPECT_EQ(reg.pin(), pin_fully);
   EXPECT_FALSE(reg.is_virtual());

   Register reg2(3, 1, pin_fully);

   EXPECT_EQ(reg2.sel(), 3);
   EXPECT_EQ(reg2.chan(), 1);
   EXPECT_EQ(reg2.pin(), pin_fully);
   EXPECT_FALSE(reg2.is_virtual());
}

#ifdef __cpp_exceptions
TEST_F(ValueTest, virtual_register_must_not_be_pinned_to_sel)
{
   EXPECT_THROW(Register(1024, 1, pin_fully), std::invalid_argument);
}
#endif

TEST_F(ValueTest, virtual_register_not_pinned)
{
   Register reg(1024, 1, pin_none);

   EXPECT_EQ(reg.sel(), 1024);
   EXPECT_EQ(reg.chan(), 1);
   EXPECT_EQ(reg.pin(), pin_none);
   EXPECT_TRUE(reg.is_virtual());

   Register reg2(1025, 2, pin_none);

   EXPECT_EQ(reg2.sel(), 1025);
   EXPECT_EQ(reg2.chan(), 2);
   EXPECT_EQ(reg2.pin(), pin_none);
   EXPECT_TRUE(reg2.is_virtual());
}

TEST_F(ValueTest, uniform_value)
{
   UniformValue reg0(512, 1);

   EXPECT_EQ(reg0.sel(), 512);
   EXPECT_EQ(reg0.chan(), 1);
   EXPECT_EQ(reg0.kcache_bank(), 0);
   EXPECT_FALSE(reg0.buf_addr());
   EXPECT_FALSE(reg0.is_virtual());

   UniformValue reg1(513, 2, 1);

   EXPECT_EQ(reg1.sel(), 513);
   EXPECT_EQ(reg1.chan(), 2);
   EXPECT_EQ(reg1.kcache_bank(), 1);
   EXPECT_FALSE(reg1.buf_addr());
   EXPECT_FALSE(reg1.is_virtual());

   auto addr = new Register(1024, 0, pin_none);
   ASSERT_TRUE(addr);

   UniformValue reg_with_buffer_addr(513, 0, addr, 0);

   EXPECT_EQ(reg_with_buffer_addr.sel(), 513);
   EXPECT_EQ(reg_with_buffer_addr.chan(), 0);
   EXPECT_EQ(reg_with_buffer_addr.pin(), pin_none);
   EXPECT_EQ(reg_with_buffer_addr.kcache_bank(), 0);
   EXPECT_FALSE(reg_with_buffer_addr.is_virtual());
   ASSERT_TRUE(reg_with_buffer_addr.buf_addr());

   auto baddr = reg_with_buffer_addr.buf_addr();
   EXPECT_EQ(baddr->sel(), 1024);
   EXPECT_EQ(baddr->chan(), 0);
   EXPECT_EQ(baddr->pin(), pin_none);
   EXPECT_TRUE(baddr->is_virtual());
}

TEST_F(ValueTest, literal_value)
{
   LiteralConstant literal(12);
   EXPECT_EQ(literal.sel(), ALU_SRC_LITERAL);
   EXPECT_EQ(literal.chan(), -1);
   EXPECT_EQ(literal.value(), 12);
   EXPECT_FALSE(literal.is_virtual());

   LiteralConstant literal2(2);
   EXPECT_EQ(literal2.sel(), ALU_SRC_LITERAL);
   EXPECT_EQ(literal2.chan(), -1);
   EXPECT_EQ(literal2.value(), 2);
   EXPECT_FALSE(literal2.is_virtual());
}

TEST_F(ValueTest, inline_constant)
{
   InlineConstant c0(ALU_SRC_1);

   EXPECT_EQ(c0.sel(), ALU_SRC_1);
   EXPECT_EQ(c0.chan(), 0);
   EXPECT_FALSE(c0.is_virtual());

   InlineConstant c1(ALU_SRC_M_1_INT);
   EXPECT_EQ(c1.sel(), ALU_SRC_M_1_INT);
   EXPECT_EQ(c1.chan(), 0);
   EXPECT_FALSE(c1.is_virtual());

   InlineConstant c2(ALU_SRC_PV, 1);
   EXPECT_EQ(c2.sel(), ALU_SRC_PV);
   EXPECT_EQ(c2.chan(), 1);
   EXPECT_FALSE(c2.is_virtual());
}

TEST_F(ValueTest, array)
{
   LocalArray array(1024, 2, 12);

   EXPECT_EQ(array.size(), 12);
   EXPECT_EQ(array.nchannels(), 2);

   auto elm0 = array.element(0, nullptr, 0);
   ASSERT_TRUE(elm0);

   EXPECT_EQ(elm0->sel(), 1024);
   EXPECT_EQ(elm0->chan(), 0);
   EXPECT_EQ(elm0->pin(), pin_array);

   EXPECT_FALSE(elm0->get_addr());

   auto elm1 = array.element(8, nullptr, 1);
   ASSERT_TRUE(elm1);

   EXPECT_EQ(elm1->sel(), 1024 + 8);
   EXPECT_EQ(elm1->chan(), 1);
   EXPECT_EQ(elm1->pin(), pin_array);
   EXPECT_FALSE(elm1->get_addr());

   auto addr = new Register(2000, 0, pin_none);
   ASSERT_TRUE(addr);

   auto elm_indirect = array.element(0, addr, 1);
   ASSERT_TRUE(elm_indirect);

   auto elm_addr = elm_indirect->get_addr();
   ASSERT_TRUE(elm_addr);

   EXPECT_EQ(elm_indirect->sel(), 1024);
   EXPECT_EQ(elm_indirect->chan(), 1);
   EXPECT_EQ(elm_indirect->pin(), pin_array);

   EXPECT_EQ(elm_addr->sel(), 2000);
   EXPECT_EQ(elm_addr->chan(), 0);
   EXPECT_EQ(elm_addr->pin(), pin_none);

   // A constant addr should resolve directly
   auto addr2 = new LiteralConstant(3);
   ASSERT_TRUE(addr2);

   auto elm_direct = array.element(0, addr2, 0);
   auto elm_direct_addr = elm_direct->get_addr();
   EXPECT_FALSE(elm_direct_addr);

   EXPECT_EQ(elm_direct->sel(), 1027);
   EXPECT_EQ(elm_direct->chan(), 0);
   EXPECT_EQ(elm_direct->pin(), pin_array);

#ifdef __cpp_exceptions
   EXPECT_THROW(array.element(12, nullptr, 0), std::invalid_argument);
   EXPECT_THROW(array.element(3, nullptr, 2), std::invalid_argument);

   auto addr3 = new LiteralConstant(12);
   ASSERT_TRUE(addr3);
   EXPECT_THROW(array.element(0, addr3, 0), std::invalid_argument);
#endif
}

TEST_F(ValueTest, reg_from_string)
{
   Register reg(1000, 0, pin_none);
   auto fs = Register::from_string("R1000.x");
   EXPECT_EQ(*fs, reg);

   EXPECT_EQ(*Register::from_string("R1001.y"), Register(1001, 1, pin_none));

   auto reg2 = Register(1, 2, pin_fully);
   reg2.set_flag(Register::pin_start);
   EXPECT_EQ(*Register::from_string("R1.z@fully"), reg2);
   EXPECT_EQ(*Register::from_string("R1000.y@chan"), Register(1000, 1, pin_chan));
   EXPECT_EQ(*Register::from_string("R1000.y@free"), Register(1000, 1, pin_free));

   EXPECT_EQ(*VirtualValue::from_string("L[0x1]"), LiteralConstant(1));
   EXPECT_EQ(*VirtualValue::from_string("L[0x2]"), LiteralConstant(2));
   EXPECT_EQ(*VirtualValue::from_string("L[0xA]"), LiteralConstant(10));

   EXPECT_EQ(*VirtualValue::from_string("I[0]"), InlineConstant(ALU_SRC_0));
   EXPECT_EQ(*VirtualValue::from_string("I[HW_WAVE_ID]"),
             InlineConstant(ALU_SRC_HW_WAVE_ID));
}
