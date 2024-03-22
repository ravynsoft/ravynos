/* Copyright (c) 2018-2019 Alyssa Rosenzweig (alyssa@rosenzweig.io)
 * Copyright (C) 2019-2020 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __MIDGARD_OPS
#define __MIDGARD_OPS

#include "helpers.h"

/* Forward declare */

extern struct mir_op_props alu_opcode_props[256];
extern struct mir_ldst_op_props load_store_opcode_props[256];
extern struct mir_tex_op_props tex_opcode_props[16];
extern struct mir_tag_props midgard_tag_props[16];

#define OP_IS_ATOMIC(op)   (load_store_opcode_props[op].props & LDST_ATOMIC)
#define OP_USES_ATTRIB(op) (load_store_opcode_props[op].props & LDST_ATTRIB)
#define OP_IS_STORE(op)    (load_store_opcode_props[op].props & LDST_STORE)
#define OP_HAS_ADDRESS(op) (load_store_opcode_props[op].props & LDST_ADDRESS)

/* Is this opcode that of an integer (regardless of signedness)? Instruction
 * names authoritatively determine types */

static inline bool
midgard_is_integer_op(int op)
{
   return (op >= 0x40 && op <= 0x7E) || (op >= 0xA0 && op <= 0xC1);
}

static inline bool
midgard_is_unsigned_op(int op)
{
   assert(midgard_is_integer_op(op));

   switch (op) {
   case midgard_alu_op_uaddsat:
   case midgard_alu_op_usubsat:
   case midgard_alu_op_uwmul:
   case midgard_alu_op_umin:
   case midgard_alu_op_umax:
   case midgard_alu_op_uavg:
   case midgard_alu_op_uravg:
   case midgard_alu_op_ushlsat:
   case midgard_alu_op_uabsdiff:
   case midgard_alu_op_ult:
   case midgard_alu_op_ule:
   case midgard_alu_op_uball_lt:
   case midgard_alu_op_uball_lte:
   case midgard_alu_op_ubany_lt:
   case midgard_alu_op_ubany_lte:
   case midgard_alu_op_u2f_rte:
   case midgard_alu_op_u2f_rtz:
   case midgard_alu_op_u2f_rtn:
   case midgard_alu_op_u2f_rtp:
      return true;
   default:
      return false;
   }
}

/* Does this opcode *write* an integer? Same as is_integer_op, unless it's a
 * conversion between int<->float in which case we do the opposite */

static inline bool
midgard_is_integer_out_op(int op)
{
   bool is_int = midgard_is_integer_op(op);
   bool is_conversion = alu_opcode_props[op].props & OP_TYPE_CONVERT;

   return is_int ^ is_conversion;
}

/* Determines effective writemask, taking quirks and expansion into account */

static inline unsigned
effective_writemask(midgard_alu_op op, unsigned existing_mask)
{
   /* Channel count is off-by-one to fit in two-bits (0 channel makes no
    * sense) */

   unsigned channel_count = GET_CHANNEL_COUNT(alu_opcode_props[op].props);

   /* If there is a fixed channel count, construct the appropriate mask */

   if (channel_count)
      return (1 << channel_count) - 1;

   return existing_mask;
};

#endif
