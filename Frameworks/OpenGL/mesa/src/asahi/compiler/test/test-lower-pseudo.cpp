/*
 * Copyright 2021 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "agx_test.h"

#include <gtest/gtest.h>

#define CASE(instr, expected)                                                  \
   INSTRUCTION_CASE(instr, expected, agx_lower_pseudo)
#define NEGCASE(instr) CASE(instr, instr)

class LowerPseudo : public testing::Test {
 protected:
   LowerPseudo()
   {
      mem_ctx = ralloc_context(NULL);

      wx = agx_register(0, AGX_SIZE_32);
      wy = agx_register(2, AGX_SIZE_32);
      wz = agx_register(4, AGX_SIZE_32);
   }

   ~LowerPseudo()
   {
      ralloc_free(mem_ctx);
   }

   void *mem_ctx;
   agx_index wx, wy, wz;
};

TEST_F(LowerPseudo, Move)
{
   CASE(agx_mov_to(b, wx, wy), agx_bitop_to(b, wx, wy, agx_zero(), 0xA));
}

TEST_F(LowerPseudo, Not)
{
   CASE(agx_not_to(b, wx, wy), agx_bitop_to(b, wx, wy, agx_zero(), 0x5));
}

TEST_F(LowerPseudo, BinaryBitwise)
{
   CASE(agx_and_to(b, wx, wy, wz), agx_bitop_to(b, wx, wy, wz, 0x8));
   CASE(agx_xor_to(b, wx, wy, wz), agx_bitop_to(b, wx, wy, wz, 0x6));
   CASE(agx_or_to(b, wx, wy, wz), agx_bitop_to(b, wx, wy, wz, 0xE));
}
