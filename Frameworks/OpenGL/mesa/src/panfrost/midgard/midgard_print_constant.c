/*
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019-2020 Collabora, Ltd.
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

#include <inttypes.h>
#include <math.h>
#include "util/half_float.h"
#include "helpers.h"
#include "midgard.h"
#include "midgard_ops.h"

void
mir_print_constant_component(FILE *fp, const midgard_constants *consts,
                             unsigned c, midgard_reg_mode reg_mode, bool half,
                             unsigned mod, midgard_alu_op op)
{
   bool is_sint = false, is_uint = false, is_hex = false;
   const char *opname = alu_opcode_props[op].name;

   bool is_int = midgard_is_integer_op(op);

   /* Add a sentinel name to prevent crashing */
   if (!opname)
      opname = "unknown";

   if (is_int) {
      is_uint = midgard_is_unsigned_op(op);

      if (!is_uint) {
         /* Bit ops are easier to follow when the constant is printed in
          * hexadecimal. Other operations starting with a 'i' are
          * considered to operate on signed integers. That might not
          * be true for all of them, but it's good enough for traces.
          */
         if (op >= midgard_alu_op_iand && op <= midgard_alu_op_ipopcnt)
            is_hex = true;
         else
            is_sint = true;
      }
   }

   if (half)
      reg_mode--;

   switch (reg_mode) {
   case midgard_reg_mode_64:
      if (is_sint) {
         fprintf(fp, "%" PRIi64, consts->i64[c]);
      } else if (is_uint) {
         fprintf(fp, "%" PRIu64, consts->u64[c]);
      } else if (is_hex) {
         fprintf(fp, "0x%" PRIX64, consts->u64[c]);
      } else {
         double v = consts->f64[c];

         if (mod & MIDGARD_FLOAT_MOD_ABS)
            v = fabs(v);
         if (mod & MIDGARD_FLOAT_MOD_NEG)
            v = -v;

         printf("%g", v);
      }
      break;

   case midgard_reg_mode_32:
      if (is_sint) {
         int64_t v;

         if (half && mod == midgard_int_zero_extend)
            v = consts->u32[c];
         else if (half && mod == midgard_int_left_shift)
            v = (uint64_t)consts->u32[c] << 32;
         else
            v = consts->i32[c];

         fprintf(fp, "%" PRIi64, v);
      } else if (is_uint || is_hex) {
         uint64_t v;

         if (half && mod == midgard_int_left_shift)
            v = (uint64_t)consts->u32[c] << 32;
         else
            v = consts->u32[c];

         fprintf(fp, is_uint ? "%" PRIu64 : "0x%" PRIX64, v);
      } else {
         float v = consts->f32[c];

         if (mod & MIDGARD_FLOAT_MOD_ABS)
            v = fabsf(v);
         if (mod & MIDGARD_FLOAT_MOD_NEG)
            v = -v;

         fprintf(fp, "%g", v);
      }
      break;

   case midgard_reg_mode_16:
      if (is_sint) {
         int32_t v;

         if (half && mod == midgard_int_zero_extend)
            v = consts->u16[c];
         else if (half && mod == midgard_int_left_shift)
            v = (uint32_t)consts->u16[c] << 16;
         else
            v = consts->i16[c];

         fprintf(fp, "%d", v);
      } else if (is_uint || is_hex) {
         uint32_t v;

         if (half && mod == midgard_int_left_shift)
            v = (uint32_t)consts->u16[c] << 16;
         else
            v = consts->u16[c];

         fprintf(fp, is_uint ? "%u" : "0x%X", v);
      } else {
         float v = _mesa_half_to_float(consts->f16[c]);

         if (mod & MIDGARD_FLOAT_MOD_ABS)
            v = fabsf(v);
         if (mod & MIDGARD_FLOAT_MOD_NEG)
            v = -v;

         fprintf(fp, "%g", v);
      }
      break;

   case midgard_reg_mode_8:
      fprintf(fp, "0x%X", consts->u8[c]);

      if (mod)
         fprintf(fp, " /* %u */", mod);

      assert(!half); /* No 4-bit */

      break;
   }
}

static char *outmod_names_float[4] = {"", ".clamp_0_inf", ".clamp_m1_1",
                                      ".clamp_0_1"};

static char *outmod_names_int[4] = {".ssat", ".usat", ".keeplo", ".keephi"};

void
mir_print_outmod(FILE *fp, unsigned outmod, bool is_int)
{
   fprintf(fp, "%s",
           is_int ? outmod_names_int[outmod] : outmod_names_float[outmod]);
}
