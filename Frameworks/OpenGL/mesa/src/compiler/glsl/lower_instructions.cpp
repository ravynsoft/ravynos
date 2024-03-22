/*
 * Copyright © 2010 Intel Corporation
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
 * \file lower_instructions.cpp
 *
 * Many GPUs lack native instructions for certain expression operations, and
 * must replace them with some other expression tree.  This pass lowers some
 * of the most common cases, allowing the lowering code to be implemented once
 * rather than in each driver backend.
 */

#include "program/prog_instruction.h" /* for swizzle */
#include "compiler/glsl_types.h"
#include "ir.h"
#include "ir_builder.h"
#include "ir_optimization.h"
#include "util/half_float.h"

#include <math.h>

/* Operations for lower_instructions() */
#define FIND_LSB_TO_FLOAT_CAST    0x20000
#define FIND_MSB_TO_FLOAT_CAST    0x40000
#define IMUL_HIGH_TO_MUL          0x80000

using namespace ir_builder;

namespace {

class lower_instructions_visitor : public ir_hierarchical_visitor {
public:
   lower_instructions_visitor(unsigned lower)
      : progress(false), lower(lower) { }

   ir_visitor_status visit_leave(ir_expression *);

   bool progress;

private:
   unsigned lower; /** Bitfield of which operations to lower */

   void double_dot_to_fma(ir_expression *);
   void double_lrp(ir_expression *);
   void find_lsb_to_float_cast(ir_expression *ir);
   void find_msb_to_float_cast(ir_expression *ir);
   void imul_high_to_mul(ir_expression *ir);

   ir_expression *_carry(operand a, operand b);

   static ir_constant *_imm_fp(void *mem_ctx,
                               const glsl_type *type,
                               double f,
                               unsigned vector_elements=1);
};

} /* anonymous namespace */

/**
 * Determine if a particular type of lowering should occur
 */
#define lowering(x) (this->lower & x)

bool
lower_instructions(exec_list *instructions,bool have_gpu_shader5)
{
   unsigned what_to_lower =
      /* Assume that if ARB_gpu_shader5 is not supported then all of the
       * extended integer functions need lowering.  It may be necessary to add
       * some caps for individual instructions.
       */
      (!have_gpu_shader5 ? FIND_LSB_TO_FLOAT_CAST |
                           FIND_MSB_TO_FLOAT_CAST |
                           IMUL_HIGH_TO_MUL : 0);

   lower_instructions_visitor v(what_to_lower);

   visit_list_elements(&v, instructions);
   return v.progress;
}

void
lower_instructions_visitor::double_dot_to_fma(ir_expression *ir)
{
   ir_variable *temp = new(ir) ir_variable(glsl_get_base_glsl_type(ir->operands[0]->type), "dot_res",
					   ir_var_temporary);
   this->base_ir->insert_before(temp);

   int nc = glsl_get_components(ir->operands[0]->type);
   for (int i = nc - 1; i >= 1; i--) {
      ir_assignment *assig;
      if (i == (nc - 1)) {
         assig = assign(temp, mul(swizzle(ir->operands[0]->clone(ir, NULL), i, 1),
                                  swizzle(ir->operands[1]->clone(ir, NULL), i, 1)));
      } else {
         assig = assign(temp, fma(swizzle(ir->operands[0]->clone(ir, NULL), i, 1),
                                  swizzle(ir->operands[1]->clone(ir, NULL), i, 1),
                                  temp));
      }
      this->base_ir->insert_before(assig);
   }

   ir->operation = ir_triop_fma;
   ir->init_num_operands();
   ir->operands[0] = swizzle(ir->operands[0], 0, 1);
   ir->operands[1] = swizzle(ir->operands[1], 0, 1);
   ir->operands[2] = new(ir) ir_dereference_variable(temp);

   this->progress = true;

}

void
lower_instructions_visitor::double_lrp(ir_expression *ir)
{
   int swizval;
   ir_rvalue *op0 = ir->operands[0], *op2 = ir->operands[2];
   ir_constant *one = new(ir) ir_constant(1.0, op2->type->vector_elements);

   switch (op2->type->vector_elements) {
   case 1:
      swizval = SWIZZLE_XXXX;
      break;
   default:
      assert(op0->type->vector_elements == op2->type->vector_elements);
      swizval = SWIZZLE_XYZW;
      break;
   }

   ir->operation = ir_triop_fma;
   ir->init_num_operands();
   ir->operands[0] = swizzle(op2, swizval, op0->type->vector_elements);
   ir->operands[2] = mul(sub(one, op2->clone(ir, NULL)), op0);

   this->progress = true;
}

void
lower_instructions_visitor::find_lsb_to_float_cast(ir_expression *ir)
{
   /* For more details, see:
    *
    * http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightFloatCast
    */
   const unsigned elements = ir->operands[0]->type->vector_elements;
   ir_constant *c0 = new(ir) ir_constant(unsigned(0), elements);
   ir_constant *cminus1 = new(ir) ir_constant(int(-1), elements);
   ir_constant *c23 = new(ir) ir_constant(int(23), elements);
   ir_constant *c7F = new(ir) ir_constant(int(0x7F), elements);
   ir_variable *temp =
      new(ir) ir_variable(glsl_ivec_type(elements), "temp", ir_var_temporary);
   ir_variable *lsb_only =
      new(ir) ir_variable(glsl_uvec_type(elements), "lsb_only", ir_var_temporary);
   ir_variable *as_float =
      new(ir) ir_variable(glsl_vec_type(elements), "as_float", ir_var_temporary);
   ir_variable *lsb =
      new(ir) ir_variable(glsl_ivec_type(elements), "lsb", ir_var_temporary);

   ir_instruction &i = *base_ir;

   i.insert_before(temp);

   if (ir->operands[0]->type->base_type == GLSL_TYPE_INT) {
      i.insert_before(assign(temp, ir->operands[0]));
   } else {
      assert(ir->operands[0]->type->base_type == GLSL_TYPE_UINT);
      i.insert_before(assign(temp, u2i(ir->operands[0])));
   }

   /* The int-to-float conversion is lossless because (value & -value) is
    * either a power of two or zero.  We don't use the result in the zero
    * case.  The uint() cast is necessary so that 0x80000000 does not
    * generate a negative value.
    *
    * uint lsb_only = uint(value & -value);
    * float as_float = float(lsb_only);
    */
   i.insert_before(lsb_only);
   i.insert_before(assign(lsb_only, i2u(bit_and(temp, neg(temp)))));

   i.insert_before(as_float);
   i.insert_before(assign(as_float, u2f(lsb_only)));

   /* This is basically an open-coded frexp.  Implementations that have a
    * native frexp instruction would be better served by that.  This is
    * optimized versus a full-featured open-coded implementation in two ways:
    *
    * - We don't care about a correct result from subnormal numbers (including
    *   0.0), so the raw exponent can always be safely unbiased.
    *
    * - The value cannot be negative, so it does not need to be masked off to
    *   extract the exponent.
    *
    * int lsb = (floatBitsToInt(as_float) >> 23) - 0x7f;
    */
   i.insert_before(lsb);
   i.insert_before(assign(lsb, sub(rshift(bitcast_f2i(as_float), c23), c7F)));

   /* Use lsb_only in the comparison instead of temp so that the & (far above)
    * can possibly generate the result without an explicit comparison.
    *
    * (lsb_only == 0) ? -1 : lsb;
    *
    * Since our input values are all integers, the unbiased exponent must not
    * be negative.  It will only be negative (-0x7f, in fact) if lsb_only is
    * 0.  Instead of using (lsb_only == 0), we could use (lsb >= 0).  Which is
    * better is likely GPU dependent.  Either way, the difference should be
    * small.
    */
   ir->operation = ir_triop_csel;
   ir->init_num_operands();
   ir->operands[0] = equal(lsb_only, c0);
   ir->operands[1] = cminus1;
   ir->operands[2] = new(ir) ir_dereference_variable(lsb);

   this->progress = true;
}

void
lower_instructions_visitor::find_msb_to_float_cast(ir_expression *ir)
{
   /* For more details, see:
    *
    * http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightFloatCast
    */
   const unsigned elements = ir->operands[0]->type->vector_elements;
   ir_constant *c0 = new(ir) ir_constant(int(0), elements);
   ir_constant *cminus1 = new(ir) ir_constant(int(-1), elements);
   ir_constant *c23 = new(ir) ir_constant(int(23), elements);
   ir_constant *c7F = new(ir) ir_constant(int(0x7F), elements);
   ir_constant *c000000FF = new(ir) ir_constant(0x000000FFu, elements);
   ir_constant *cFFFFFF00 = new(ir) ir_constant(0xFFFFFF00u, elements);
   ir_variable *temp =
      new(ir) ir_variable(glsl_uvec_type(elements), "temp", ir_var_temporary);
   ir_variable *as_float =
      new(ir) ir_variable(glsl_vec_type(elements), "as_float", ir_var_temporary);
   ir_variable *msb =
      new(ir) ir_variable(glsl_ivec_type(elements), "msb", ir_var_temporary);

   ir_instruction &i = *base_ir;

   i.insert_before(temp);

   if (ir->operands[0]->type->base_type == GLSL_TYPE_UINT) {
      i.insert_before(assign(temp, ir->operands[0]));
   } else {
      assert(ir->operands[0]->type->base_type == GLSL_TYPE_INT);

      /* findMSB(uint(abs(some_int))) almost always does the right thing.
       * There are two problem values:
       *
       * * 0x80000000.  Since abs(0x80000000) == 0x80000000, findMSB returns
       *   31.  However, findMSB(int(0x80000000)) == 30.
       *
       * * 0xffffffff.  Since abs(0xffffffff) == 1, findMSB returns
       *   31.  Section 8.8 (Integer Functions) of the GLSL 4.50 spec says:
       *
       *    For a value of zero or negative one, -1 will be returned.
       *
       * For all negative number cases, including 0x80000000 and 0xffffffff,
       * the correct value is obtained from findMSB if instead of negating the
       * (already negative) value the logical-not is used.  A conditonal
       * logical-not can be achieved in two instructions.
       */
      ir_variable *as_int =
         new(ir) ir_variable(glsl_ivec_type(elements), "as_int", ir_var_temporary);
      ir_constant *c31 = new(ir) ir_constant(int(31), elements);

      i.insert_before(as_int);
      i.insert_before(assign(as_int, ir->operands[0]));
      i.insert_before(assign(temp, i2u(expr(ir_binop_bit_xor,
                                            as_int,
                                            rshift(as_int, c31)))));
   }

   /* The int-to-float conversion is lossless because bits are conditionally
    * masked off the bottom of temp to ensure the value has at most 24 bits of
    * data or is zero.  We don't use the result in the zero case.  The uint()
    * cast is necessary so that 0x80000000 does not generate a negative value.
    *
    * float as_float = float(temp > 255 ? temp & ~255 : temp);
    */
   i.insert_before(as_float);
   i.insert_before(assign(as_float, u2f(csel(greater(temp, c000000FF),
                                             bit_and(temp, cFFFFFF00),
                                             temp))));

   /* This is basically an open-coded frexp.  Implementations that have a
    * native frexp instruction would be better served by that.  This is
    * optimized versus a full-featured open-coded implementation in two ways:
    *
    * - We don't care about a correct result from subnormal numbers (including
    *   0.0), so the raw exponent can always be safely unbiased.
    *
    * - The value cannot be negative, so it does not need to be masked off to
    *   extract the exponent.
    *
    * int msb = (floatBitsToInt(as_float) >> 23) - 0x7f;
    */
   i.insert_before(msb);
   i.insert_before(assign(msb, sub(rshift(bitcast_f2i(as_float), c23), c7F)));

   /* Use msb in the comparison instead of temp so that the subtract can
    * possibly generate the result without an explicit comparison.
    *
    * (msb < 0) ? -1 : msb;
    *
    * Since our input values are all integers, the unbiased exponent must not
    * be negative.  It will only be negative (-0x7f, in fact) if temp is 0.
    */
   ir->operation = ir_triop_csel;
   ir->init_num_operands();
   ir->operands[0] = less(msb, c0);
   ir->operands[1] = cminus1;
   ir->operands[2] = new(ir) ir_dereference_variable(msb);

   this->progress = true;
}

ir_expression *
lower_instructions_visitor::_carry(operand a, operand b)
{
   return i2u(b2i(less(add(a, b),
                       a.val->clone(ralloc_parent(a.val), NULL))));
}

void
lower_instructions_visitor::imul_high_to_mul(ir_expression *ir)
{
   /*   ABCD
    * * EFGH
    * ======
    * (GH * CD) + (GH * AB) << 16 + (EF * CD) << 16 + (EF * AB) << 32
    *
    * In GLSL, (a * b) becomes
    *
    * uint m1 = (a & 0x0000ffffu) * (b & 0x0000ffffu);
    * uint m2 = (a & 0x0000ffffu) * (b >> 16);
    * uint m3 = (a >> 16)         * (b & 0x0000ffffu);
    * uint m4 = (a >> 16)         * (b >> 16);
    *
    * uint c1;
    * uint c2;
    * uint lo_result;
    * uint hi_result;
    *
    * lo_result = uaddCarry(m1, m2 << 16, c1);
    * hi_result = m4 + c1;
    * lo_result = uaddCarry(lo_result, m3 << 16, c2);
    * hi_result = hi_result + c2;
    * hi_result = hi_result + (m2 >> 16) + (m3 >> 16);
    */
   const unsigned elements = ir->operands[0]->type->vector_elements;
   ir_variable *src1 =
      new(ir) ir_variable(glsl_uvec_type(elements), "src1", ir_var_temporary);
   ir_variable *src1h =
      new(ir) ir_variable(glsl_uvec_type(elements), "src1h", ir_var_temporary);
   ir_variable *src1l =
      new(ir) ir_variable(glsl_uvec_type(elements), "src1l", ir_var_temporary);
   ir_variable *src2 =
      new(ir) ir_variable(glsl_uvec_type(elements), "src2", ir_var_temporary);
   ir_variable *src2h =
      new(ir) ir_variable(glsl_uvec_type(elements), "src2h", ir_var_temporary);
   ir_variable *src2l =
      new(ir) ir_variable(glsl_uvec_type(elements), "src2l", ir_var_temporary);
   ir_variable *t1 =
      new(ir) ir_variable(glsl_uvec_type(elements), "t1", ir_var_temporary);
   ir_variable *t2 =
      new(ir) ir_variable(glsl_uvec_type(elements), "t2", ir_var_temporary);
   ir_variable *lo =
      new(ir) ir_variable(glsl_uvec_type(elements), "lo", ir_var_temporary);
   ir_variable *hi =
      new(ir) ir_variable(glsl_uvec_type(elements), "hi", ir_var_temporary);
   ir_variable *different_signs = NULL;
   ir_constant *c0000FFFF = new(ir) ir_constant(0x0000FFFFu, elements);
   ir_constant *c16 = new(ir) ir_constant(16u, elements);

   ir_instruction &i = *base_ir;

   i.insert_before(src1);
   i.insert_before(src2);
   i.insert_before(src1h);
   i.insert_before(src2h);
   i.insert_before(src1l);
   i.insert_before(src2l);

   if (ir->operands[0]->type->base_type == GLSL_TYPE_UINT) {
      i.insert_before(assign(src1, ir->operands[0]));
      i.insert_before(assign(src2, ir->operands[1]));
   } else {
      assert(ir->operands[0]->type->base_type == GLSL_TYPE_INT);

      ir_variable *itmp1 =
         new(ir) ir_variable(glsl_ivec_type(elements), "itmp1", ir_var_temporary);
      ir_variable *itmp2 =
         new(ir) ir_variable(glsl_ivec_type(elements), "itmp2", ir_var_temporary);
      ir_constant *c0 = new(ir) ir_constant(int(0), elements);

      i.insert_before(itmp1);
      i.insert_before(itmp2);
      i.insert_before(assign(itmp1, ir->operands[0]));
      i.insert_before(assign(itmp2, ir->operands[1]));

      different_signs =
         new(ir) ir_variable(glsl_bvec_type(elements), "different_signs",
                             ir_var_temporary);

      i.insert_before(different_signs);
      i.insert_before(assign(different_signs, expr(ir_binop_logic_xor,
                                                   less(itmp1, c0),
                                                   less(itmp2, c0->clone(ir, NULL)))));

      i.insert_before(assign(src1, i2u(abs(itmp1))));
      i.insert_before(assign(src2, i2u(abs(itmp2))));
   }

   i.insert_before(assign(src1l, bit_and(src1, c0000FFFF)));
   i.insert_before(assign(src2l, bit_and(src2, c0000FFFF->clone(ir, NULL))));
   i.insert_before(assign(src1h, rshift(src1, c16)));
   i.insert_before(assign(src2h, rshift(src2, c16->clone(ir, NULL))));

   i.insert_before(lo);
   i.insert_before(hi);
   i.insert_before(t1);
   i.insert_before(t2);

   i.insert_before(assign(lo, mul(src1l, src2l)));
   i.insert_before(assign(t1, mul(src1l, src2h)));
   i.insert_before(assign(t2, mul(src1h, src2l)));
   i.insert_before(assign(hi, mul(src1h, src2h)));

   i.insert_before(assign(hi, add(hi, _carry(lo, lshift(t1, c16->clone(ir, NULL))))));
   i.insert_before(assign(lo,            add(lo, lshift(t1, c16->clone(ir, NULL)))));

   i.insert_before(assign(hi, add(hi, _carry(lo, lshift(t2, c16->clone(ir, NULL))))));
   i.insert_before(assign(lo,            add(lo, lshift(t2, c16->clone(ir, NULL)))));

   if (different_signs == NULL) {
      assert(ir->operands[0]->type->base_type == GLSL_TYPE_UINT);

      ir->operation = ir_binop_add;
      ir->init_num_operands();
      ir->operands[0] = add(hi, rshift(t1, c16->clone(ir, NULL)));
      ir->operands[1] = rshift(t2, c16->clone(ir, NULL));
   } else {
      assert(ir->operands[0]->type->base_type == GLSL_TYPE_INT);

      i.insert_before(assign(hi, add(add(hi, rshift(t1, c16->clone(ir, NULL))),
                                     rshift(t2, c16->clone(ir, NULL)))));

      /* For channels where different_signs is set we have to perform a 64-bit
       * negation.  This is *not* the same as just negating the high 32-bits.
       * Consider -3 * 2.  The high 32-bits is 0, but the desired result is
       * -1, not -0!  Recall -x == ~x + 1.
       */
      ir_variable *neg_hi =
         new(ir) ir_variable(glsl_ivec_type(elements), "neg_hi", ir_var_temporary);
      ir_constant *c1 = new(ir) ir_constant(1u, elements);

      i.insert_before(neg_hi);
      i.insert_before(assign(neg_hi, add(bit_not(u2i(hi)),
                                         u2i(_carry(bit_not(lo), c1)))));

      ir->operation = ir_triop_csel;
      ir->init_num_operands();
      ir->operands[0] = new(ir) ir_dereference_variable(different_signs);
      ir->operands[1] = new(ir) ir_dereference_variable(neg_hi);
      ir->operands[2] = u2i(hi);
   }
}

ir_visitor_status
lower_instructions_visitor::visit_leave(ir_expression *ir)
{
   switch (ir->operation) {
   case ir_binop_dot:
      if (glsl_type_is_double(ir->operands[0]->type))
         double_dot_to_fma(ir);
      break;
   case ir_triop_lrp:
      if (glsl_type_is_double(ir->operands[0]->type))
         double_lrp(ir);
      break;

   case ir_unop_find_lsb:
      if (lowering(FIND_LSB_TO_FLOAT_CAST))
         find_lsb_to_float_cast(ir);
      break;

   case ir_unop_find_msb:
      if (lowering(FIND_MSB_TO_FLOAT_CAST))
         find_msb_to_float_cast(ir);
      break;

   case ir_binop_imul_high:
      if (lowering(IMUL_HIGH_TO_MUL))
         imul_high_to_mul(ir);
      break;

   default:
      return visit_continue;
   }

   return visit_continue;
}
