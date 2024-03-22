/*
 * Copyright © 2022 Intel Corporation
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

#include "nir.h"

static nir_alu_type
nir_alu_src_type(const nir_alu_instr *instr, unsigned src)
{
   return nir_alu_type_get_base_type(nir_op_infos[instr->op].input_types[src]) |
          nir_src_bit_size(instr->src[src].src);
}

static nir_scalar
nir_alu_arg(const nir_alu_instr *alu, unsigned arg, unsigned comp)
{
   const nir_alu_src *src = &alu->src[arg];
   return nir_get_scalar(src->src.ssa, src->swizzle[comp]);
}

/* Tries to determine the value of expression "val % div", assuming that val
 * is interpreted as value of type "val_type". "div" must be a power of two.
 * Returns true if it can statically tell the value of "val % div", false if not.
 * Value of *mod is undefined if this function returned false.
 *
 * Tests are in mod_analysis_tests.cpp.
 */
bool
nir_mod_analysis(nir_scalar val, nir_alu_type val_type, unsigned div, unsigned *mod)
{
   if (div == 1) {
      *mod = 0;
      return true;
   }

   assert(util_is_power_of_two_nonzero(div));

   switch (val.def->parent_instr->type) {
   case nir_instr_type_load_const: {
      nir_load_const_instr *load =
         nir_instr_as_load_const(val.def->parent_instr);
      nir_alu_type base_type = nir_alu_type_get_base_type(val_type);

      if (base_type == nir_type_uint) {
         assert(val.comp < load->def.num_components);
         uint64_t ival = nir_const_value_as_uint(load->value[val.comp],
                                                 load->def.bit_size);
         *mod = ival % div;
         return true;
      } else if (base_type == nir_type_int) {
         assert(val.comp < load->def.num_components);
         int64_t ival = nir_const_value_as_int(load->value[val.comp],
                                               load->def.bit_size);

         /* whole analysis collapses the moment we allow negative values */
         if (ival < 0)
            return false;

         *mod = ((uint64_t)ival) % div;
         return true;
      }

      break;
   }

   case nir_instr_type_alu: {
      nir_alu_instr *alu = nir_instr_as_alu(val.def->parent_instr);

      if (alu->def.num_components != 1)
         return false;

      switch (alu->op) {
      case nir_op_ishr: {
         if (nir_src_is_const(alu->src[1].src)) {
            assert(alu->src[1].src.ssa->num_components == 1);
            uint64_t shift = nir_src_as_uint(alu->src[1].src);

            if (util_last_bit(div) + shift > 32)
               break;

            nir_alu_type type0 = nir_alu_src_type(alu, 0);
            if (!nir_mod_analysis(nir_alu_arg(alu, 0, val.comp), type0, div << shift, mod))
               return false;

            *mod >>= shift;
            return true;
         }
         break;
      }

      case nir_op_iadd: {
         unsigned mod0;
         nir_alu_type type0 = nir_alu_src_type(alu, 0);
         if (!nir_mod_analysis(nir_alu_arg(alu, 0, val.comp), type0, div, &mod0))
            return false;

         unsigned mod1;
         nir_alu_type type1 = nir_alu_src_type(alu, 1);
         if (!nir_mod_analysis(nir_alu_arg(alu, 1, val.comp), type1, div, &mod1))
            return false;

         *mod = (mod0 + mod1) % div;
         return true;
      }

      case nir_op_ishl: {
         if (nir_src_is_const(alu->src[1].src)) {
            assert(alu->src[1].src.ssa->num_components == 1);
            uint64_t shift = nir_src_as_uint(alu->src[1].src);

            if ((div >> shift) == 0) {
               *mod = 0;
               return true;
            }
            nir_alu_type type0 = nir_alu_src_type(alu, 0);
            return nir_mod_analysis(nir_alu_arg(alu, 0, val.comp), type0, div >> shift, mod);
         }
         break;
      }

      case nir_op_imul_32x16: /* multiply 32-bits with low 16-bits */
      case nir_op_imul: {
         unsigned mod0;
         nir_alu_type type0 = nir_alu_src_type(alu, 0);
         bool s1 = nir_mod_analysis(nir_alu_arg(alu, 0, val.comp), type0, div, &mod0);

         if (s1 && (mod0 == 0)) {
            *mod = 0;
            return true;
         }

         /* if divider is larger than 2nd source max (interpreted) value
          * then modulo of multiplication is unknown
          */
         if (alu->op == nir_op_imul_32x16 && div > (1u << 16))
            return false;

         unsigned mod1;
         nir_alu_type type1 = nir_alu_src_type(alu, 1);
         bool s2 = nir_mod_analysis(nir_alu_arg(alu, 1, val.comp), type1, div, &mod1);

         if (s2 && (mod1 == 0)) {
            *mod = 0;
            return true;
         }

         if (!s1 || !s2)
            return false;

         *mod = (mod0 * mod1) % div;
         return true;
      }

      default:
         break;
      }
      break;
   }

   default:
      break;
   }

   return false;
}
