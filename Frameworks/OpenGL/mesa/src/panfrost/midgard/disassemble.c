/* Author(s):
 *   Connor Abbott
 *   Alyssa Rosenzweig
 *
 * Copyright (c) 2013 Connor Abbott (connor@abbott.cx)
 * Copyright (c) 2018 Alyssa Rosenzweig (alyssa@rosenzweig.io)
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

#include "disassemble.h"
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/bitscan.h"
#include "util/half_float.h"
#include "util/u_math.h"
#include "helpers.h"
#include "midgard.h"
#include "midgard_ops.h"
#include "midgard_quirks.h"

#define DEFINE_CASE(define, str)                                               \
   case define: {                                                              \
      fprintf(fp, str);                                                        \
      break;                                                                   \
   }

/* These are not mapped to hardware values, they just represent the possible
 * implicit arg modifiers that some midgard opcodes have, which can be decoded
 * from the opcodes via midgard_{alu,ldst,tex}_special_arg_mod() */
typedef enum {
   midgard_arg_mod_none = 0,
   midgard_arg_mod_inv,
   midgard_arg_mod_x2,
} midgard_special_arg_mod;

typedef struct {
   unsigned *midg_tags;

   /* For static analysis to ensure all registers are written at least once
    * before use along the source code path (TODO: does this break done for
    * complex CF?)
    */

   uint16_t midg_ever_written;
} disassemble_context;

/* Transform an expanded writemask (duplicated 8-bit format) into its condensed
 * form (one bit per component) */

static inline unsigned
condense_writemask(unsigned expanded_mask, unsigned bits_per_component)
{
   if (bits_per_component == 8) {
      /* Duplicate every bit to go from 8 to 16-channel wrmask */
      unsigned omask = 0;

      for (unsigned i = 0; i < 8; ++i) {
         if (expanded_mask & (1 << i))
            omask |= (3 << (2 * i));
      }

      return omask;
   }

   unsigned slots_per_component = bits_per_component / 16;
   unsigned max_comp = (16 * 8) / bits_per_component;
   unsigned condensed_mask = 0;

   for (unsigned i = 0; i < max_comp; i++) {
      if (expanded_mask & (1 << (i * slots_per_component)))
         condensed_mask |= (1 << i);
   }

   return condensed_mask;
}

static bool
print_alu_opcode(FILE *fp, midgard_alu_op op)
{
   if (alu_opcode_props[op].name)
      fprintf(fp, "%s", alu_opcode_props[op].name);
   else
      fprintf(fp, "alu_op_%02X", op);

   /* For constant analysis */
   return midgard_is_integer_op(op);
}

static void
print_ld_st_opcode(FILE *fp, midgard_load_store_op op)
{
   if (load_store_opcode_props[op].name)
      fprintf(fp, "%s", load_store_opcode_props[op].name);
   else
      fprintf(fp, "ldst_op_%02X", op);
}

static void
validate_sampler_type(enum mali_texture_op op,
                      enum mali_sampler_type sampler_type)
{
   if (op == midgard_tex_op_mov || op == midgard_tex_op_barrier)
      assert(sampler_type == 0);
   else
      assert(sampler_type > 0);
}

static void
validate_expand_mode(midgard_src_expand_mode expand_mode,
                     midgard_reg_mode reg_mode)
{
   switch (expand_mode) {
   case midgard_src_passthrough:
      break;

   case midgard_src_rep_low:
      assert(reg_mode == midgard_reg_mode_8 || reg_mode == midgard_reg_mode_16);
      break;

   case midgard_src_rep_high:
      assert(reg_mode == midgard_reg_mode_8 || reg_mode == midgard_reg_mode_16);
      break;

   case midgard_src_swap:
      assert(reg_mode == midgard_reg_mode_8 || reg_mode == midgard_reg_mode_16);
      break;

   case midgard_src_expand_low:
      assert(reg_mode != midgard_reg_mode_8);
      break;

   case midgard_src_expand_high:
      assert(reg_mode != midgard_reg_mode_8);
      break;

   case midgard_src_expand_low_swap:
      assert(reg_mode == midgard_reg_mode_16);
      break;

   case midgard_src_expand_high_swap:
      assert(reg_mode == midgard_reg_mode_16);
      break;

   default:
      unreachable("Invalid expand mode");
      break;
   }
}

static void
print_alu_reg(disassemble_context *ctx, FILE *fp, unsigned reg, bool is_write)
{
   unsigned uniform_reg = 23 - reg;
   bool is_uniform = false;

   /* For r8-r15, it could be a work or uniform. We distinguish based on
    * the fact work registers are ALWAYS written before use, but uniform
    * registers are NEVER written before use. */

   if ((reg >= 8 && reg < 16) && !(ctx->midg_ever_written & (1 << reg)))
      is_uniform = true;

   /* r16-r23 are always uniform */

   if (reg >= 16 && reg <= 23)
      is_uniform = true;

   if (reg == REGISTER_UNUSED || reg == REGISTER_UNUSED + 1)
      fprintf(fp, "TMP%u", reg - REGISTER_UNUSED);
   else if (reg == REGISTER_TEXTURE_BASE || reg == REGISTER_TEXTURE_BASE + 1)
      fprintf(fp, "%s%u", is_write ? "AT" : "TA", reg - REGISTER_TEXTURE_BASE);
   else if (reg == REGISTER_LDST_BASE || reg == REGISTER_LDST_BASE + 1)
      fprintf(fp, "AL%u", reg - REGISTER_LDST_BASE);
   else if (is_uniform)
      fprintf(fp, "U%u", uniform_reg);
   else if (reg == 31 && !is_write)
      fprintf(fp, "PC_SP");
   else
      fprintf(fp, "R%u", reg);
}

static void
print_ldst_write_reg(FILE *fp, unsigned reg)
{
   switch (reg) {
   case 26:
   case 27:
      fprintf(fp, "AL%u", reg - REGISTER_LDST_BASE);
      break;
   case 28:
   case 29:
      fprintf(fp, "AT%u", reg - REGISTER_TEXTURE_BASE);
      break;
   case 31:
      fprintf(fp, "PC_SP");
      break;
   default:
      fprintf(fp, "R%d", reg);
      break;
   }
}

static void
print_ldst_read_reg(FILE *fp, unsigned reg)
{
   switch (reg) {
   case 0:
   case 1:
      fprintf(fp, "AL%u", reg);
      break;
   case 2:
      fprintf(fp, "PC_SP");
      break;
   case 3:
      fprintf(fp, "LOCAL_STORAGE_PTR");
      break;
   case 4:
      fprintf(fp, "LOCAL_THREAD_ID");
      break;
   case 5:
      fprintf(fp, "GROUP_ID");
      break;
   case 6:
      fprintf(fp, "GLOBAL_THREAD_ID");
      break;
   case 7:
      fprintf(fp, "0");
      break;
   default:
      unreachable("Invalid load/store register read");
   }
}

static void
print_tex_reg(FILE *fp, unsigned reg, bool is_write)
{
   char *str = is_write ? "TA" : "AT";
   int select = reg & 1;

   switch (reg) {
   case 0:
   case 1:
      fprintf(fp, "R%d", select);
      break;
   case 26:
   case 27:
      fprintf(fp, "AL%d", select);
      break;
   case 28:
   case 29:
      fprintf(fp, "%s%d", str, select);
      break;
   default:
      unreachable("Invalid texture register");
   }
}

static char *srcmod_names_int[4] = {
   ".sext",
   ".zext",
   ".replicate",
   ".lshift",
};

static char *argmod_names[3] = {
   "",
   ".inv",
   ".x2",
};

static char *index_format_names[4] = {"", ".u64", ".u32", ".s32"};

static void
print_alu_outmod(FILE *fp, unsigned outmod, bool is_int, bool half)
{
   if (is_int && !half) {
      assert(outmod == midgard_outmod_keeplo);
      return;
   }

   if (!is_int && half)
      fprintf(fp, ".shrink");

   mir_print_outmod(fp, outmod, is_int);
}

/* arg == 0 (dest), arg == 1 (src1), arg == 2 (src2) */
static midgard_special_arg_mod
midgard_alu_special_arg_mod(midgard_alu_op op, unsigned arg)
{
   midgard_special_arg_mod mod = midgard_arg_mod_none;

   switch (op) {
   case midgard_alu_op_ishladd:
   case midgard_alu_op_ishlsub:
      if (arg == 1)
         mod = midgard_arg_mod_x2;
      break;

   default:
      break;
   }

   return mod;
}

static void
print_quad_word(FILE *fp, uint32_t *words, unsigned tabs)
{
   unsigned i;

   for (i = 0; i < 4; i++)
      fprintf(fp, "0x%08X%s ", words[i], i == 3 ? "" : ",");

   fprintf(fp, "\n");
}

static const char components[16] = "xyzwefghijklmnop";

static int
bits_for_mode(midgard_reg_mode mode)
{
   switch (mode) {
   case midgard_reg_mode_8:
      return 8;
   case midgard_reg_mode_16:
      return 16;
   case midgard_reg_mode_32:
      return 32;
   case midgard_reg_mode_64:
      return 64;
   default:
      unreachable("Invalid reg mode");
      return 0;
   }
}

static int
bits_for_mode_halved(midgard_reg_mode mode, bool half)
{
   unsigned bits = bits_for_mode(mode);

   if (half)
      bits >>= 1;

   return bits;
}

static void
print_vec_selectors_64(FILE *fp, unsigned swizzle, midgard_reg_mode reg_mode,
                       midgard_src_expand_mode expand_mode,
                       unsigned selector_offset, uint8_t mask)
{
   bool expands = INPUT_EXPANDS(expand_mode);

   unsigned comp_skip = expands ? 1 : 2;
   unsigned mask_bit = 0;
   for (unsigned i = selector_offset; i < 4; i += comp_skip, mask_bit += 4) {
      if (!(mask & (1 << mask_bit)))
         continue;

      unsigned a = (swizzle >> (i * 2)) & 3;

      if (INPUT_EXPANDS(expand_mode)) {
         if (expand_mode == midgard_src_expand_high)
            a += 2;

         fprintf(fp, "%c", components[a / 2]);
         continue;
      }

      unsigned b = (swizzle >> ((i + 1) * 2)) & 3;

      /* Normally we're adjacent, but if there's an issue,
       * don't make it ambiguous */

      if (b == a + 1)
         fprintf(fp, "%c", a >> 1 ? 'Y' : 'X');
      else
         fprintf(fp, "[%c%c]", components[a], components[b]);
   }
}

static void
print_vec_selectors(FILE *fp, unsigned swizzle, midgard_reg_mode reg_mode,
                    unsigned selector_offset, uint8_t mask,
                    unsigned *mask_offset)
{
   assert(reg_mode != midgard_reg_mode_64);

   unsigned mask_skip = MAX2(bits_for_mode(reg_mode) / 16, 1);

   bool is_vec16 = reg_mode == midgard_reg_mode_8;

   for (unsigned i = 0; i < 4; i++, *mask_offset += mask_skip) {
      if (!(mask & (1 << *mask_offset)))
         continue;

      unsigned c = (swizzle >> (i * 2)) & 3;

      /* Vec16 has two components per swizzle selector. */
      if (is_vec16)
         c *= 2;

      c += selector_offset;

      fprintf(fp, "%c", components[c]);
      if (is_vec16)
         fprintf(fp, "%c", components[c + 1]);
   }
}

static void
print_vec_swizzle(FILE *fp, unsigned swizzle, midgard_src_expand_mode expand,
                  midgard_reg_mode mode, uint8_t mask)
{
   unsigned bits = bits_for_mode_halved(mode, INPUT_EXPANDS(expand));

   /* Swizzle selectors are divided in two halves that are always
    * mirrored, the only difference is the starting component offset.
    * The number represents an offset into the components[] array. */
   unsigned first_half = 0;
   unsigned second_half = (128 / bits) / 2; /* only used for 8 and 16-bit */

   switch (expand) {
   case midgard_src_passthrough:
      if (swizzle == 0xE4)
         return; /* identity swizzle */
      break;

   case midgard_src_expand_low:
      second_half /= 2;
      break;

   case midgard_src_expand_high:
      first_half = second_half;
      second_half += second_half / 2;
      break;

      /* The rest of the cases are only used for 8 and 16-bit */

   case midgard_src_rep_low:
      second_half = 0;
      break;

   case midgard_src_rep_high:
      first_half = second_half;
      break;

   case midgard_src_swap:
      first_half = second_half;
      second_half = 0;
      break;

   case midgard_src_expand_low_swap:
      first_half = second_half / 2;
      second_half = 0;
      break;

   case midgard_src_expand_high_swap:
      first_half = second_half + second_half / 2;
      break;

   default:
      unreachable("Invalid expand mode");
      break;
   }

   fprintf(fp, ".");

   /* Vec2 are weird so we use a separate function to simplify things. */
   if (mode == midgard_reg_mode_64) {
      print_vec_selectors_64(fp, swizzle, mode, expand, first_half, mask);
      return;
   }

   unsigned mask_offs = 0;
   print_vec_selectors(fp, swizzle, mode, first_half, mask, &mask_offs);
   if (mode == midgard_reg_mode_8 || mode == midgard_reg_mode_16)
      print_vec_selectors(fp, swizzle, mode, second_half, mask, &mask_offs);
}

static void
print_scalar_constant(FILE *fp, unsigned src_binary,
                      const midgard_constants *consts, midgard_scalar_alu *alu)
{
   midgard_scalar_alu_src *src = (midgard_scalar_alu_src *)&src_binary;
   assert(consts != NULL);

   fprintf(fp, "#");
   mir_print_constant_component(
      fp, consts, src->component,
      src->full ? midgard_reg_mode_32 : midgard_reg_mode_16, false, src->mod,
      alu->op);
}

static void
print_vector_constants(FILE *fp, unsigned src_binary,
                       const midgard_constants *consts, midgard_vector_alu *alu)
{
   midgard_vector_alu_src *src = (midgard_vector_alu_src *)&src_binary;
   bool expands = INPUT_EXPANDS(src->expand_mode);
   unsigned bits = bits_for_mode_halved(alu->reg_mode, expands);
   unsigned max_comp = (sizeof(*consts) * 8) / bits;
   unsigned comp_mask, num_comp = 0;

   assert(consts);
   assert(max_comp <= 16);

   comp_mask =
      effective_writemask(alu->op, condense_writemask(alu->mask, bits));
   num_comp = util_bitcount(comp_mask);

   if (num_comp > 1)
      fprintf(fp, "<");
   else
      fprintf(fp, "#");

   bool first = true;

   for (unsigned i = 0; i < max_comp; ++i) {
      if (!(comp_mask & (1 << i)))
         continue;

      unsigned c = (src->swizzle >> (i * 2)) & 3;

      if (bits == 16 && !expands) {
         bool upper = i >= 4;

         switch (src->expand_mode) {
         case midgard_src_passthrough:
            c += upper * 4;
            break;
         case midgard_src_rep_low:
            break;
         case midgard_src_rep_high:
            c += 4;
            break;
         case midgard_src_swap:
            c += !upper * 4;
            break;
         default:
            unreachable("invalid expand mode");
            break;
         }
      } else if (bits == 32 && !expands) {
         /* Implicitly ok */
      } else if (bits == 64 && !expands) {
         /* Implicitly ok */
      } else if (bits == 8 && !expands) {
         bool upper = i >= 8;

         unsigned index = (i >> 1) & 3;
         unsigned base = (src->swizzle >> (index * 2)) & 3;
         c = base * 2;

         switch (src->expand_mode) {
         case midgard_src_passthrough:
            c += upper * 8;
            break;
         case midgard_src_rep_low:
            break;
         case midgard_src_rep_high:
            c += 8;
            break;
         case midgard_src_swap:
            c += !upper * 8;
            break;
         default:
            unreachable("invalid expand mode");
            break;
         }

         /* We work on twos, actually */
         if (i & 1)
            c++;
      }

      if (first)
         first = false;
      else
         fprintf(fp, ", ");

      mir_print_constant_component(fp, consts, c, alu->reg_mode, expands,
                                   src->mod, alu->op);
   }

   if (num_comp > 1)
      fprintf(fp, ">");
}

static void
print_srcmod(FILE *fp, bool is_int, bool expands, unsigned mod, bool scalar)
{
   /* Modifiers change meaning depending on the op's context */

   if (is_int) {
      if (expands)
         fprintf(fp, "%s", srcmod_names_int[mod]);
   } else {
      if (mod & MIDGARD_FLOAT_MOD_ABS)
         fprintf(fp, ".abs");
      if (mod & MIDGARD_FLOAT_MOD_NEG)
         fprintf(fp, ".neg");
      if (expands)
         fprintf(fp, ".widen");
   }
}

static void
print_vector_src(disassemble_context *ctx, FILE *fp, unsigned src_binary,
                 midgard_reg_mode mode, unsigned reg,
                 midgard_shrink_mode shrink_mode, uint8_t src_mask, bool is_int,
                 midgard_special_arg_mod arg_mod)
{
   midgard_vector_alu_src *src = (midgard_vector_alu_src *)&src_binary;

   validate_expand_mode(src->expand_mode, mode);

   print_alu_reg(ctx, fp, reg, false);

   print_vec_swizzle(fp, src->swizzle, src->expand_mode, mode, src_mask);

   fprintf(fp, "%s", argmod_names[arg_mod]);

   print_srcmod(fp, is_int, INPUT_EXPANDS(src->expand_mode), src->mod, false);
}

static uint16_t
decode_vector_imm(unsigned src2_reg, unsigned imm)
{
   uint16_t ret;
   ret = src2_reg << 11;
   ret |= (imm & 0x7) << 8;
   ret |= (imm >> 3) & 0xFF;
   return ret;
}

static void
print_immediate(FILE *fp, uint16_t imm, bool is_instruction_int)
{
   if (is_instruction_int)
      fprintf(fp, "#%u", imm);
   else
      fprintf(fp, "#%g", _mesa_half_to_float(imm));
}

static void
update_dest(disassemble_context *ctx, unsigned reg)
{
   /* We should record writes as marking this as a work register. Store
    * the max register in work_count; we'll add one at the end */

   if (reg < 16)
      ctx->midg_ever_written |= (1 << reg);
}

static void
print_dest(disassemble_context *ctx, FILE *fp, unsigned reg)
{
   update_dest(ctx, reg);
   print_alu_reg(ctx, fp, reg, true);
}

/* For 16-bit+ masks, we read off from the 8-bit mask field. For 16-bit (vec8),
 * it's just one bit per channel, easy peasy. For 32-bit (vec4), it's one bit
 * per channel with one duplicate bit in the middle. For 64-bit (vec2), it's
 * one-bit per channel with _3_ duplicate bits in the middle. Basically, just
 * subdividing the 128-bit word in 16-bit increments. For 64-bit, we uppercase
 * the mask to make it obvious what happened */

static void
print_alu_mask(FILE *fp, uint8_t mask, unsigned bits,
               midgard_shrink_mode shrink_mode)
{
   /* Skip 'complete' masks */

   if (shrink_mode == midgard_shrink_mode_none && mask == 0xFF)
      return;

   fprintf(fp, ".");

   unsigned skip = MAX2(bits / 16, 1);
   bool tripped = false;

   /* To apply an upper destination shrink_mode, we "shift" the alphabet.
    * E.g. with an upper shrink_mode on 32-bit, instead of xyzw, print efgh.
    * For upper 16-bit, instead of xyzwefgh, print ijklmnop */

   const char *alphabet = components;

   if (shrink_mode == midgard_shrink_mode_upper) {
      assert(bits != 8);
      alphabet += (128 / bits);
   }

   for (unsigned i = 0; i < 8; i += skip) {
      bool a = (mask & (1 << i)) != 0;

      for (unsigned j = 1; j < skip; ++j) {
         bool dupe = (mask & (1 << (i + j))) != 0;
         tripped |= (dupe != a);
      }

      if (a) {
         /* TODO: handle shrinking from 16-bit */
         unsigned comp_idx = bits == 8 ? i * 2 : i;
         char c = alphabet[comp_idx / skip];

         fprintf(fp, "%c", c);
         if (bits == 8)
            fprintf(fp, "%c", alphabet[comp_idx + 1]);
      }
   }

   if (tripped)
      fprintf(fp, " /* %X */", mask);
}

/* TODO: 16-bit mode */
static void
print_ldst_mask(FILE *fp, unsigned mask, unsigned swizzle)
{
   fprintf(fp, ".");

   for (unsigned i = 0; i < 4; ++i) {
      bool write = (mask & (1 << i)) != 0;
      unsigned c = (swizzle >> (i * 2)) & 3;
      /* We can't omit the swizzle here since many ldst ops have a
       * combined swizzle/writemask, and it would be ambiguous to not
       * print the masked-out components. */
      fprintf(fp, "%c", write ? components[c] : '~');
   }
}

static void
print_tex_mask(FILE *fp, unsigned mask, bool upper)
{
   if (mask == 0xF) {
      if (upper)
         fprintf(fp, "'");

      return;
   }

   fprintf(fp, ".");

   for (unsigned i = 0; i < 4; ++i) {
      bool a = (mask & (1 << i)) != 0;
      if (a)
         fprintf(fp, "%c", components[i + (upper ? 4 : 0)]);
   }
}

static void
print_vector_field(disassemble_context *ctx, FILE *fp, const char *name,
                   uint16_t *words, uint16_t reg_word,
                   const midgard_constants *consts, unsigned tabs, bool verbose)
{
   midgard_reg_info *reg_info = (midgard_reg_info *)&reg_word;
   midgard_vector_alu *alu_field = (midgard_vector_alu *)words;
   midgard_reg_mode mode = alu_field->reg_mode;
   midgard_alu_op op = alu_field->op;
   unsigned shrink_mode = alu_field->shrink_mode;
   bool is_int = midgard_is_integer_op(op);
   bool is_int_out = midgard_is_integer_out_op(op);

   if (verbose)
      fprintf(fp, "%s.", name);

   bool is_instruction_int = print_alu_opcode(fp, alu_field->op);

   /* Print lane width */
   fprintf(fp, ".%c%d", is_int_out ? 'i' : 'f', bits_for_mode(mode));

   fprintf(fp, " ");

   /* Mask denoting status of 8-lanes */
   uint8_t mask = alu_field->mask;

   /* First, print the destination */
   print_dest(ctx, fp, reg_info->out_reg);

   if (shrink_mode != midgard_shrink_mode_none) {
      bool shrinkable = (mode != midgard_reg_mode_8);
      bool known = shrink_mode != 0x3; /* Unused value */

      if (!(shrinkable && known))
         fprintf(fp, "/* do%u */ ", shrink_mode);
   }

   /* Instructions like fdot4 do *not* replicate, ensure the
    * mask is of only a single component */

   unsigned rep = GET_CHANNEL_COUNT(alu_opcode_props[op].props);

   if (rep) {
      unsigned comp_mask = condense_writemask(mask, bits_for_mode(mode));
      unsigned num_comp = util_bitcount(comp_mask);
      if (num_comp != 1)
         fprintf(fp, "/* err too many components */");
   }
   print_alu_mask(fp, mask, bits_for_mode(mode), shrink_mode);

   /* Print output modifiers */

   print_alu_outmod(fp, alu_field->outmod, is_int_out,
                    shrink_mode != midgard_shrink_mode_none);

   /* Mask out unused components based on the writemask, but don't mask out
    * components that are used for interlane instructions like fdot3. */
   uint8_t src_mask =
      rep ? expand_writemask(mask_of(rep),
                             util_logbase2(128 / bits_for_mode(mode)))
          : mask;

   fprintf(fp, ", ");

   if (reg_info->src1_reg == REGISTER_CONSTANT)
      print_vector_constants(fp, alu_field->src1, consts, alu_field);
   else {
      midgard_special_arg_mod argmod = midgard_alu_special_arg_mod(op, 1);
      print_vector_src(ctx, fp, alu_field->src1, mode, reg_info->src1_reg,
                       shrink_mode, src_mask, is_int, argmod);
   }

   fprintf(fp, ", ");

   if (reg_info->src2_imm) {
      uint16_t imm =
         decode_vector_imm(reg_info->src2_reg, alu_field->src2 >> 2);
      print_immediate(fp, imm, is_instruction_int);
   } else if (reg_info->src2_reg == REGISTER_CONSTANT) {
      print_vector_constants(fp, alu_field->src2, consts, alu_field);
   } else {
      midgard_special_arg_mod argmod = midgard_alu_special_arg_mod(op, 2);
      print_vector_src(ctx, fp, alu_field->src2, mode, reg_info->src2_reg,
                       shrink_mode, src_mask, is_int, argmod);
   }

   fprintf(fp, "\n");
}

static void
print_scalar_src(disassemble_context *ctx, FILE *fp, bool is_int,
                 unsigned src_binary, unsigned reg)
{
   midgard_scalar_alu_src *src = (midgard_scalar_alu_src *)&src_binary;

   print_alu_reg(ctx, fp, reg, false);

   unsigned c = src->component;

   if (src->full) {
      assert((c & 1) == 0);
      c >>= 1;
   }

   fprintf(fp, ".%c", components[c]);

   print_srcmod(fp, is_int, !src->full, src->mod, true);
}

static uint16_t
decode_scalar_imm(unsigned src2_reg, unsigned imm)
{
   uint16_t ret;
   ret = src2_reg << 11;
   ret |= (imm & 3) << 9;
   ret |= (imm & 4) << 6;
   ret |= (imm & 0x38) << 2;
   ret |= imm >> 6;
   return ret;
}

static void
print_scalar_field(disassemble_context *ctx, FILE *fp, const char *name,
                   uint16_t *words, uint16_t reg_word,
                   const midgard_constants *consts, unsigned tabs, bool verbose)
{
   midgard_reg_info *reg_info = (midgard_reg_info *)&reg_word;
   midgard_scalar_alu *alu_field = (midgard_scalar_alu *)words;
   bool is_int = midgard_is_integer_op(alu_field->op);
   bool is_int_out = midgard_is_integer_out_op(alu_field->op);
   bool full = alu_field->output_full;

   if (alu_field->reserved)
      fprintf(fp, "scalar ALU reserved bit set\n");

   if (verbose)
      fprintf(fp, "%s.", name);

   bool is_instruction_int = print_alu_opcode(fp, alu_field->op);

   /* Print lane width, in this case the lane width is always 32-bit, but
    * we print it anyway to make it consistent with the other instructions. */
   fprintf(fp, ".%c32", is_int_out ? 'i' : 'f');

   fprintf(fp, " ");

   print_dest(ctx, fp, reg_info->out_reg);
   unsigned c = alu_field->output_component;

   if (full) {
      assert((c & 1) == 0);
      c >>= 1;
   }

   fprintf(fp, ".%c", components[c]);

   print_alu_outmod(fp, alu_field->outmod, is_int_out, !full);

   fprintf(fp, ", ");

   if (reg_info->src1_reg == REGISTER_CONSTANT)
      print_scalar_constant(fp, alu_field->src1, consts, alu_field);
   else
      print_scalar_src(ctx, fp, is_int, alu_field->src1, reg_info->src1_reg);

   fprintf(fp, ", ");

   if (reg_info->src2_imm) {
      uint16_t imm = decode_scalar_imm(reg_info->src2_reg, alu_field->src2);
      print_immediate(fp, imm, is_instruction_int);
   } else if (reg_info->src2_reg == REGISTER_CONSTANT) {
      print_scalar_constant(fp, alu_field->src2, consts, alu_field);
   } else
      print_scalar_src(ctx, fp, is_int, alu_field->src2, reg_info->src2_reg);

   fprintf(fp, "\n");
}

static void
print_branch_op(FILE *fp, unsigned op)
{
   switch (op) {
   case midgard_jmp_writeout_op_branch_uncond:
      fprintf(fp, "uncond.");
      break;

   case midgard_jmp_writeout_op_branch_cond:
      fprintf(fp, "cond.");
      break;

   case midgard_jmp_writeout_op_writeout:
      fprintf(fp, "write.");
      break;

   case midgard_jmp_writeout_op_tilebuffer_pending:
      fprintf(fp, "tilebuffer.");
      break;

   case midgard_jmp_writeout_op_discard:
      fprintf(fp, "discard.");
      break;

   default:
      fprintf(fp, "unk%u.", op);
      break;
   }
}

static void
print_branch_cond(FILE *fp, int cond)
{
   switch (cond) {
   case midgard_condition_write0:
      fprintf(fp, "write0");
      break;

   case midgard_condition_false:
      fprintf(fp, "false");
      break;

   case midgard_condition_true:
      fprintf(fp, "true");
      break;

   case midgard_condition_always:
      fprintf(fp, "always");
      break;

   default:
      fprintf(fp, "unk%X", cond);
      break;
   }
}

static const char *
function_call_mode(enum midgard_call_mode mode)
{
   switch (mode) {
   case midgard_call_mode_default:
      return "";
   case midgard_call_mode_call:
      return ".call";
   case midgard_call_mode_return:
      return ".return";
   default:
      return ".reserved";
   }
}

static bool
print_compact_branch_writeout_field(disassemble_context *ctx, FILE *fp,
                                    uint16_t word)
{
   midgard_jmp_writeout_op op = word & 0x7;

   switch (op) {
   case midgard_jmp_writeout_op_branch_uncond: {
      midgard_branch_uncond br_uncond;
      memcpy((char *)&br_uncond, (char *)&word, sizeof(br_uncond));
      fprintf(fp, "br.uncond%s ", function_call_mode(br_uncond.call_mode));

      if (br_uncond.offset >= 0)
         fprintf(fp, "+");

      fprintf(fp, "%d -> %s", br_uncond.offset,
              midgard_tag_props[br_uncond.dest_tag].name);
      fprintf(fp, "\n");

      return br_uncond.offset >= 0;
   }

   case midgard_jmp_writeout_op_branch_cond:
   case midgard_jmp_writeout_op_writeout:
   case midgard_jmp_writeout_op_discard:
   default: {
      midgard_branch_cond br_cond;
      memcpy((char *)&br_cond, (char *)&word, sizeof(br_cond));

      fprintf(fp, "br.");

      print_branch_op(fp, br_cond.op);
      print_branch_cond(fp, br_cond.cond);

      fprintf(fp, " ");

      if (br_cond.offset >= 0)
         fprintf(fp, "+");

      fprintf(fp, "%d -> %s", br_cond.offset,
              midgard_tag_props[br_cond.dest_tag].name);
      fprintf(fp, "\n");

      return br_cond.offset >= 0;
   }
   }

   return false;
}

static bool
print_extended_branch_writeout_field(disassemble_context *ctx, FILE *fp,
                                     uint8_t *words, unsigned next)
{
   midgard_branch_extended br;
   memcpy((char *)&br, (char *)words, sizeof(br));

   fprintf(fp, "brx%s.", function_call_mode(br.call_mode));

   print_branch_op(fp, br.op);

   /* Condition codes are a LUT in the general case, but simply repeated 8 times
    * for single-channel conditions.. Check this. */

   bool single_channel = true;

   for (unsigned i = 0; i < 16; i += 2) {
      single_channel &= (((br.cond >> i) & 0x3) == (br.cond & 0x3));
   }

   if (single_channel)
      print_branch_cond(fp, br.cond & 0x3);
   else
      fprintf(fp, "lut%X", br.cond);

   fprintf(fp, " ");

   if (br.offset >= 0)
      fprintf(fp, "+");

   fprintf(fp, "%d -> %s\n", br.offset, midgard_tag_props[br.dest_tag].name);

   unsigned I = next + br.offset * 4;

   if (ctx->midg_tags[I] && ctx->midg_tags[I] != br.dest_tag) {
      fprintf(fp, "\t/* XXX TAG ERROR: jumping to %s but tagged %s \n",
              midgard_tag_props[br.dest_tag].name,
              midgard_tag_props[ctx->midg_tags[I]].name);
   }

   ctx->midg_tags[I] = br.dest_tag;

   return br.offset >= 0;
}

static unsigned
num_alu_fields_enabled(uint32_t control_word)
{
   unsigned ret = 0;

   if ((control_word >> 17) & 1)
      ret++;

   if ((control_word >> 19) & 1)
      ret++;

   if ((control_word >> 21) & 1)
      ret++;

   if ((control_word >> 23) & 1)
      ret++;

   if ((control_word >> 25) & 1)
      ret++;

   return ret;
}

static bool
print_alu_word(disassemble_context *ctx, FILE *fp, uint32_t *words,
               unsigned num_quad_words, unsigned tabs, unsigned next,
               bool verbose)
{
   uint32_t control_word = words[0];
   uint16_t *beginning_ptr = (uint16_t *)(words + 1);
   unsigned num_fields = num_alu_fields_enabled(control_word);
   uint16_t *word_ptr = beginning_ptr + num_fields;
   unsigned num_words = 2 + num_fields;
   const midgard_constants *consts = NULL;
   bool branch_forward = false;

   if ((control_word >> 17) & 1)
      num_words += 3;

   if ((control_word >> 19) & 1)
      num_words += 2;

   if ((control_word >> 21) & 1)
      num_words += 3;

   if ((control_word >> 23) & 1)
      num_words += 2;

   if ((control_word >> 25) & 1)
      num_words += 3;

   if ((control_word >> 26) & 1)
      num_words += 1;

   if ((control_word >> 27) & 1)
      num_words += 3;

   if (num_quad_words > (num_words + 7) / 8) {
      assert(num_quad_words == (num_words + 15) / 8);
      // Assume that the extra quadword is constants
      consts = (midgard_constants *)(words + (4 * num_quad_words - 4));
   }

   if ((control_word >> 16) & 1)
      fprintf(fp, "unknown bit 16 enabled\n");

   if ((control_word >> 17) & 1) {
      print_vector_field(ctx, fp, "vmul", word_ptr, *beginning_ptr, consts,
                         tabs, verbose);
      beginning_ptr += 1;
      word_ptr += 3;
   }

   if ((control_word >> 18) & 1)
      fprintf(fp, "unknown bit 18 enabled\n");

   if ((control_word >> 19) & 1) {
      print_scalar_field(ctx, fp, "sadd", word_ptr, *beginning_ptr, consts,
                         tabs, verbose);
      beginning_ptr += 1;
      word_ptr += 2;
   }

   if ((control_word >> 20) & 1)
      fprintf(fp, "unknown bit 20 enabled\n");

   if ((control_word >> 21) & 1) {
      print_vector_field(ctx, fp, "vadd", word_ptr, *beginning_ptr, consts,
                         tabs, verbose);
      beginning_ptr += 1;
      word_ptr += 3;
   }

   if ((control_word >> 22) & 1)
      fprintf(fp, "unknown bit 22 enabled\n");

   if ((control_word >> 23) & 1) {
      print_scalar_field(ctx, fp, "smul", word_ptr, *beginning_ptr, consts,
                         tabs, verbose);
      beginning_ptr += 1;
      word_ptr += 2;
   }

   if ((control_word >> 24) & 1)
      fprintf(fp, "unknown bit 24 enabled\n");

   if ((control_word >> 25) & 1) {
      print_vector_field(ctx, fp, "lut", word_ptr, *beginning_ptr, consts, tabs,
                         verbose);
      word_ptr += 3;
   }

   if ((control_word >> 26) & 1) {
      branch_forward |= print_compact_branch_writeout_field(ctx, fp, *word_ptr);
      word_ptr += 1;
   }

   if ((control_word >> 27) & 1) {
      branch_forward |= print_extended_branch_writeout_field(
         ctx, fp, (uint8_t *)word_ptr, next);
      word_ptr += 3;
   }

   if (consts)
      fprintf(fp, "uconstants 0x%X, 0x%X, 0x%X, 0x%X\n", consts->u32[0],
              consts->u32[1], consts->u32[2], consts->u32[3]);

   return branch_forward;
}

/* TODO: how can we use this now that we know that these params can't be known
 * before run time in every single case? Maybe just use it in the cases we can? */
UNUSED static void
print_varying_parameters(FILE *fp, midgard_load_store_word *word)
{
   midgard_varying_params p = midgard_unpack_varying_params(*word);

   /* If a varying, there are qualifiers */
   if (p.flat_shading)
      fprintf(fp, ".flat");

   if (p.perspective_correction)
      fprintf(fp, ".correction");

   if (p.centroid_mapping)
      fprintf(fp, ".centroid");

   if (p.interpolate_sample)
      fprintf(fp, ".sample");

   switch (p.modifier) {
   case midgard_varying_mod_perspective_y:
      fprintf(fp, ".perspectivey");
      break;
   case midgard_varying_mod_perspective_z:
      fprintf(fp, ".perspectivez");
      break;
   case midgard_varying_mod_perspective_w:
      fprintf(fp, ".perspectivew");
      break;
   default:
      unreachable("invalid varying modifier");
      break;
   }
}

/* Helper to print integer well-formatted, but only when non-zero. */
static void
midgard_print_sint(FILE *fp, int n)
{
   if (n > 0)
      fprintf(fp, " + 0x%X", n);
   else if (n < 0)
      fprintf(fp, " - 0x%X", -n);
}

static void
print_load_store_instr(disassemble_context *ctx, FILE *fp, uint64_t data,
                       bool verbose)
{
   midgard_load_store_word *word = (midgard_load_store_word *)&data;

   print_ld_st_opcode(fp, word->op);

   if (word->op == midgard_op_trap) {
      fprintf(fp, " 0x%X\n", word->signed_offset);
      return;
   }

   /* Print opcode modifiers */

   if (OP_USES_ATTRIB(word->op)) {
      /* Print non-default attribute tables */
      bool default_secondary = (word->op == midgard_op_st_vary_32) ||
                               (word->op == midgard_op_st_vary_16) ||
                               (word->op == midgard_op_st_vary_32u) ||
                               (word->op == midgard_op_st_vary_32i) ||
                               (word->op == midgard_op_ld_vary_32) ||
                               (word->op == midgard_op_ld_vary_16) ||
                               (word->op == midgard_op_ld_vary_32u) ||
                               (word->op == midgard_op_ld_vary_32i);

      bool default_primary = (word->op == midgard_op_ld_attr_32) ||
                             (word->op == midgard_op_ld_attr_16) ||
                             (word->op == midgard_op_ld_attr_32u) ||
                             (word->op == midgard_op_ld_attr_32i);

      bool has_default = (default_secondary || default_primary);
      bool auto32 = (word->index_format >> 0) & 1;
      bool is_secondary = (word->index_format >> 1) & 1;

      if (auto32)
         fprintf(fp, ".a32");

      if (has_default && (is_secondary != default_secondary))
         fprintf(fp, ".%s", is_secondary ? "secondary" : "primary");
   } else if (word->op == midgard_op_ld_cubemap_coords ||
              OP_IS_PROJECTION(word->op))
      fprintf(fp, ".%s", word->bitsize_toggle ? "f32" : "f16");

   fprintf(fp, " ");

   /* src/dest register */

   if (!OP_IS_STORE(word->op)) {
      print_ldst_write_reg(fp, word->reg);

      /* Some opcodes don't have a swizzable src register, and
       * instead the swizzle is applied before the result is written
       * to the dest reg. For these ops, we combine the writemask
       * with the swizzle to display them in the disasm compactly. */
      unsigned swizzle = word->swizzle;
      if ((OP_IS_REG2REG_LDST(word->op) && word->op != midgard_op_lea &&
           word->op != midgard_op_lea_image) ||
          OP_IS_ATOMIC(word->op))
         swizzle = 0xE4;
      print_ldst_mask(fp, word->mask, swizzle);
   } else {
      uint8_t mask = (word->mask & 0x1) | ((word->mask & 0x2) << 1) |
                     ((word->mask & 0x4) << 2) | ((word->mask & 0x8) << 3);
      mask |= mask << 1;
      print_ldst_read_reg(fp, word->reg);
      print_vec_swizzle(fp, word->swizzle, midgard_src_passthrough,
                        midgard_reg_mode_32, mask);
   }

   /* ld_ubo args */
   if (OP_IS_UBO_READ(word->op)) {
      if (word->signed_offset & 1) { /* buffer index imm */
         unsigned imm = midgard_unpack_ubo_index_imm(*word);
         fprintf(fp, ", %u", imm);
      } else { /* buffer index from reg */
         fprintf(fp, ", ");
         print_ldst_read_reg(fp, word->arg_reg);
         fprintf(fp, ".%c", components[word->arg_comp]);
      }

      fprintf(fp, ", ");
      print_ldst_read_reg(fp, word->index_reg);
      fprintf(fp, ".%c", components[word->index_comp]);
      if (word->index_shift)
         fprintf(fp, " << %u", word->index_shift);
      midgard_print_sint(fp, UNPACK_LDST_UBO_OFS(word->signed_offset));
   }

   /* mem addr expression */
   if (OP_HAS_ADDRESS(word->op)) {
      fprintf(fp, ", ");
      bool first = true;

      /* Skip printing zero */
      if (word->arg_reg != 7 || verbose) {
         print_ldst_read_reg(fp, word->arg_reg);
         fprintf(fp, ".u%d.%c", word->bitsize_toggle ? 64 : 32,
                 components[word->arg_comp]);
         first = false;
      }

      if ((word->op < midgard_op_atomic_cmpxchg ||
           word->op > midgard_op_atomic_cmpxchg64_be) &&
          word->index_reg != 0x7) {
         if (!first)
            fprintf(fp, " + ");

         print_ldst_read_reg(fp, word->index_reg);
         fprintf(fp, "%s.%c", index_format_names[word->index_format],
                 components[word->index_comp]);
         if (word->index_shift)
            fprintf(fp, " << %u", word->index_shift);
      }

      midgard_print_sint(fp, word->signed_offset);
   }

   /* src reg for reg2reg ldst opcodes */
   if (OP_IS_REG2REG_LDST(word->op)) {
      fprintf(fp, ", ");
      print_ldst_read_reg(fp, word->arg_reg);
      print_vec_swizzle(fp, word->swizzle, midgard_src_passthrough,
                        midgard_reg_mode_32, 0xFF);
   }

   /* atomic ops encode the source arg where the ldst swizzle would be. */
   if (OP_IS_ATOMIC(word->op)) {
      unsigned src = (word->swizzle >> 2) & 0x7;
      unsigned src_comp = word->swizzle & 0x3;
      fprintf(fp, ", ");
      print_ldst_read_reg(fp, src);
      fprintf(fp, ".%c", components[src_comp]);
   }

   /* CMPXCHG encodes the extra comparison arg where the index reg would be. */
   if (word->op >= midgard_op_atomic_cmpxchg &&
       word->op <= midgard_op_atomic_cmpxchg64_be) {
      fprintf(fp, ", ");
      print_ldst_read_reg(fp, word->index_reg);
      fprintf(fp, ".%c", components[word->index_comp]);
   }

   /* index reg for attr/vary/images, selector for ld/st_special */
   if (OP_IS_SPECIAL(word->op) || OP_USES_ATTRIB(word->op)) {
      fprintf(fp, ", ");
      print_ldst_read_reg(fp, word->index_reg);
      fprintf(fp, ".%c", components[word->index_comp]);
      if (word->index_shift)
         fprintf(fp, " << %u", word->index_shift);
      midgard_print_sint(fp, UNPACK_LDST_ATTRIB_OFS(word->signed_offset));
   }

   /* vertex reg for attrib/varying ops, coord reg for image ops */
   if (OP_USES_ATTRIB(word->op)) {
      fprintf(fp, ", ");
      print_ldst_read_reg(fp, word->arg_reg);

      if (OP_IS_IMAGE(word->op))
         fprintf(fp, ".u%d", word->bitsize_toggle ? 64 : 32);

      fprintf(fp, ".%c", components[word->arg_comp]);

      if (word->bitsize_toggle && !OP_IS_IMAGE(word->op))
         midgard_print_sint(fp, UNPACK_LDST_VERTEX_OFS(word->signed_offset));
   }

   /* TODO: properly decode format specifier for PACK/UNPACK ops */
   if (OP_IS_PACK_COLOUR(word->op) || OP_IS_UNPACK_COLOUR(word->op)) {
      fprintf(fp, ", ");
      unsigned format_specifier =
         (word->signed_offset << 4) | word->index_shift;
      fprintf(fp, "0x%X", format_specifier);
   }

   fprintf(fp, "\n");

   /* Debugging stuff */

   if (!OP_IS_STORE(word->op))
      update_dest(ctx, word->reg);
}

static void
print_load_store_word(disassemble_context *ctx, FILE *fp, uint32_t *word,
                      bool verbose)
{
   midgard_load_store *load_store = (midgard_load_store *)word;

   if (load_store->word1 != 3) {
      print_load_store_instr(ctx, fp, load_store->word1, verbose);
   }

   if (load_store->word2 != 3) {
      print_load_store_instr(ctx, fp, load_store->word2, verbose);
   }
}

static void
print_texture_reg_select(FILE *fp, uint8_t u, unsigned base)
{
   midgard_tex_register_select sel;
   memcpy(&sel, &u, sizeof(u));

   print_tex_reg(fp, base + sel.select, false);

   unsigned component = sel.component;

   /* Use the upper half in half-reg mode */
   if (sel.upper) {
      assert(!sel.full);
      component += 4;
   }

   fprintf(fp, ".%c.%d", components[component], sel.full ? 32 : 16);

   assert(sel.zero == 0);
}

static void
print_texture_format(FILE *fp, int format)
{
   /* Act like a modifier */
   fprintf(fp, ".");

   switch (format) {
      DEFINE_CASE(1, "1d");
      DEFINE_CASE(2, "2d");
      DEFINE_CASE(3, "3d");
      DEFINE_CASE(0, "cube");

   default:
      unreachable("Bad format");
   }
}

static void
print_texture_op(FILE *fp, unsigned op)
{
   if (tex_opcode_props[op].name)
      fprintf(fp, "%s", tex_opcode_props[op].name);
   else
      fprintf(fp, "tex_op_%02X", op);
}

static bool
texture_op_takes_bias(unsigned op)
{
   return op == midgard_tex_op_normal;
}

static char
sampler_type_name(enum mali_sampler_type t)
{
   switch (t) {
   case MALI_SAMPLER_FLOAT:
      return 'f';
   case MALI_SAMPLER_UNSIGNED:
      return 'u';
   case MALI_SAMPLER_SIGNED:
      return 'i';
   default:
      return '?';
   }
}

static void
print_texture_barrier(FILE *fp, uint32_t *word)
{
   midgard_texture_barrier_word *barrier = (midgard_texture_barrier_word *)word;

   if (barrier->type != TAG_TEXTURE_4_BARRIER)
      fprintf(fp, "/* barrier tag %X != tex/bar */ ", barrier->type);

   if (!barrier->cont)
      fprintf(fp, "/* cont missing? */");

   if (!barrier->last)
      fprintf(fp, "/* last missing? */");

   if (barrier->zero1)
      fprintf(fp, "/* zero1 = 0x%X */ ", barrier->zero1);

   if (barrier->zero2)
      fprintf(fp, "/* zero2 = 0x%X */ ", barrier->zero2);

   if (barrier->zero3)
      fprintf(fp, "/* zero3 = 0x%X */ ", barrier->zero3);

   if (barrier->zero4)
      fprintf(fp, "/* zero4 = 0x%X */ ", barrier->zero4);

   if (barrier->zero5)
      fprintf(fp, "/* zero4 = 0x%" PRIx64 " */ ", barrier->zero5);

   if (barrier->out_of_order)
      fprintf(fp, ".ooo%u", barrier->out_of_order);

   fprintf(fp, "\n");
}

#undef DEFINE_CASE

static const char *
texture_mode(enum mali_texture_mode mode)
{
   switch (mode) {
   case TEXTURE_NORMAL:
      return "";
   case TEXTURE_SHADOW:
      return ".shadow";
   case TEXTURE_GATHER_SHADOW:
      return ".gather.shadow";
   case TEXTURE_GATHER_X:
      return ".gatherX";
   case TEXTURE_GATHER_Y:
      return ".gatherY";
   case TEXTURE_GATHER_Z:
      return ".gatherZ";
   case TEXTURE_GATHER_W:
      return ".gatherW";
   default:
      return "unk";
   }
}

static const char *
derivative_mode(enum mali_derivative_mode mode)
{
   switch (mode) {
   case TEXTURE_DFDX:
      return ".x";
   case TEXTURE_DFDY:
      return ".y";
   default:
      return "unk";
   }
}

static const char *
partial_exection_mode(enum midgard_partial_execution mode)
{
   switch (mode) {
   case MIDGARD_PARTIAL_EXECUTION_NONE:
      return "";
   case MIDGARD_PARTIAL_EXECUTION_SKIP:
      return ".skip";
   case MIDGARD_PARTIAL_EXECUTION_KILL:
      return ".kill";
   default:
      return ".reserved";
   }
}

static void
print_texture_word(disassemble_context *ctx, FILE *fp, uint32_t *word,
                   unsigned tabs, unsigned in_reg_base, unsigned out_reg_base)
{
   midgard_texture_word *texture = (midgard_texture_word *)word;
   validate_sampler_type(texture->op, texture->sampler_type);

   /* Broad category of texture operation in question */
   print_texture_op(fp, texture->op);

   /* Barriers use a dramatically different code path */
   if (texture->op == midgard_tex_op_barrier) {
      print_texture_barrier(fp, word);
      return;
   } else if (texture->type == TAG_TEXTURE_4_BARRIER)
      fprintf(fp, "/* nonbarrier had tex/bar tag */ ");
   else if (texture->type == TAG_TEXTURE_4_VTX)
      fprintf(fp, ".vtx");

   if (texture->op == midgard_tex_op_derivative)
      fprintf(fp, "%s", derivative_mode(texture->mode));
   else
      fprintf(fp, "%s", texture_mode(texture->mode));

   /* Specific format in question */
   print_texture_format(fp, texture->format);

   /* Instruction "modifiers" parallel the ALU instructions. */
   fputs(partial_exection_mode(texture->exec), fp);

   if (texture->out_of_order)
      fprintf(fp, ".ooo%u", texture->out_of_order);

   fprintf(fp, " ");
   print_tex_reg(fp, out_reg_base + texture->out_reg_select, true);
   print_tex_mask(fp, texture->mask, texture->out_upper);
   fprintf(fp, ".%c%d", texture->sampler_type == MALI_SAMPLER_FLOAT ? 'f' : 'i',
           texture->out_full ? 32 : 16);
   assert(!(texture->out_full && texture->out_upper));

   /* Output modifiers are only valid for float texture operations */
   if (texture->sampler_type == MALI_SAMPLER_FLOAT)
      mir_print_outmod(fp, texture->outmod, false);

   fprintf(fp, ", ");

   /* Depending on whether we read from textures directly or indirectly,
    * we may be able to update our analysis */

   if (texture->texture_register) {
      fprintf(fp, "texture[");
      print_texture_reg_select(fp, texture->texture_handle, in_reg_base);
      fprintf(fp, "], ");
   } else {
      fprintf(fp, "texture%u, ", texture->texture_handle);
   }

   /* Print the type, GL style */
   fprintf(fp, "%csampler", sampler_type_name(texture->sampler_type));

   if (texture->sampler_register) {
      fprintf(fp, "[");
      print_texture_reg_select(fp, texture->sampler_handle, in_reg_base);
      fprintf(fp, "]");
   } else {
      fprintf(fp, "%u", texture->sampler_handle);
   }

   print_vec_swizzle(fp, texture->swizzle, midgard_src_passthrough,
                     midgard_reg_mode_32, 0xFF);

   fprintf(fp, ", ");

   midgard_src_expand_mode exp =
      texture->in_reg_upper ? midgard_src_expand_high : midgard_src_passthrough;
   print_tex_reg(fp, in_reg_base + texture->in_reg_select, false);
   print_vec_swizzle(fp, texture->in_reg_swizzle, exp, midgard_reg_mode_32,
                     0xFF);
   fprintf(fp, ".%d", texture->in_reg_full ? 32 : 16);
   assert(!(texture->in_reg_full && texture->in_reg_upper));

   /* There is *always* an offset attached. Of
    * course, that offset is just immediate #0 for a
    * GLES call that doesn't take an offset. If there
    * is a non-negative non-zero offset, this is
    * specified in immediate offset mode, with the
    * values in the offset_* fields as immediates. If
    * this is a negative offset, we instead switch to
    * a register offset mode, where the offset_*
    * fields become register triplets */

   if (texture->offset_register) {
      fprintf(fp, " + ");

      bool full = texture->offset & 1;
      bool select = texture->offset & 2;
      bool upper = texture->offset & 4;
      unsigned swizzle = texture->offset >> 3;
      midgard_src_expand_mode exp =
         upper ? midgard_src_expand_high : midgard_src_passthrough;

      print_tex_reg(fp, in_reg_base + select, false);
      print_vec_swizzle(fp, swizzle, exp, midgard_reg_mode_32, 0xFF);
      fprintf(fp, ".%d", full ? 32 : 16);
      assert(!(texture->out_full && texture->out_upper));

      fprintf(fp, ", ");
   } else if (texture->offset) {
      /* Only select ops allow negative immediate offsets, verify */

      signed offset_x = (texture->offset & 0xF);
      signed offset_y = ((texture->offset >> 4) & 0xF);
      signed offset_z = ((texture->offset >> 8) & 0xF);

      bool neg_x = offset_x < 0;
      bool neg_y = offset_y < 0;
      bool neg_z = offset_z < 0;
      bool any_neg = neg_x || neg_y || neg_z;

      if (any_neg && texture->op != midgard_tex_op_fetch)
         fprintf(fp, "/* invalid negative */ ");

      /* Regardless, just print the immediate offset */

      fprintf(fp, " + <%d, %d, %d>, ", offset_x, offset_y, offset_z);
   } else {
      fprintf(fp, ", ");
   }

   char lod_operand = texture_op_takes_bias(texture->op) ? '+' : '=';

   if (texture->lod_register) {
      fprintf(fp, "lod %c ", lod_operand);
      print_texture_reg_select(fp, texture->bias, in_reg_base);
      fprintf(fp, ", ");

      if (texture->bias_int)
         fprintf(fp, " /* bias_int = 0x%X */", texture->bias_int);
   } else if (texture->op == midgard_tex_op_fetch) {
      /* For texel fetch, the int LOD is in the fractional place and
       * there is no fraction. We *always* have an explicit LOD, even
       * if it's zero. */

      if (texture->bias_int)
         fprintf(fp, " /* bias_int = 0x%X */ ", texture->bias_int);

      fprintf(fp, "lod = %u, ", texture->bias);
   } else if (texture->bias || texture->bias_int) {
      signed bias_int = texture->bias_int;
      float bias_frac = texture->bias / 256.0f;
      float bias = bias_int + bias_frac;

      bool is_bias = texture_op_takes_bias(texture->op);
      char sign = (bias >= 0.0) ? '+' : '-';
      char operand = is_bias ? sign : '=';

      fprintf(fp, "lod %c %f, ", operand, fabsf(bias));
   }

   fprintf(fp, "\n");

   /* While not zero in general, for these simple instructions the
    * following unknowns are zero, so we don't include them */

   if (texture->unknown4 || texture->unknown8) {
      fprintf(fp, "// unknown4 = 0x%x\n", texture->unknown4);
      fprintf(fp, "// unknown8 = 0x%x\n", texture->unknown8);
   }
}

void
disassemble_midgard(FILE *fp, uint8_t *code, size_t size, unsigned gpu_id,
                    bool verbose)
{
   uint32_t *words = (uint32_t *)code;
   unsigned num_words = size / 4;
   int tabs = 0;

   bool branch_forward = false;

   int last_next_tag = -1;

   unsigned i = 0;

   disassemble_context ctx = {
      .midg_tags = calloc(sizeof(ctx.midg_tags[0]), num_words),
      .midg_ever_written = 0,
   };

   while (i < num_words) {
      unsigned tag = words[i] & 0xF;
      unsigned next_tag = (words[i] >> 4) & 0xF;
      unsigned num_quad_words = midgard_tag_props[tag].size;

      if (ctx.midg_tags[i] && ctx.midg_tags[i] != tag) {
         fprintf(fp, "\t/* XXX: TAG ERROR branch, got %s expected %s */\n",
                 midgard_tag_props[tag].name,
                 midgard_tag_props[ctx.midg_tags[i]].name);
      }

      ctx.midg_tags[i] = tag;

      /* Check the tag. The idea is to ensure that next_tag is
       * *always* recoverable from the disassembly, such that we may
       * safely omit printing next_tag. To show this, we first
       * consider that next tags are semantically off-byone -- we end
       * up parsing tag n during step n+1. So, we ensure after we're
       * done disassembling the next tag of the final bundle is BREAK
       * and warn otherwise. We also ensure that the next tag is
       * never INVALID. Beyond that, since the last tag is checked
       * outside the loop, we can check one tag prior. If equal to
       * the current tag (which is unique), we're done. Otherwise, we
       * print if that tag was > TAG_BREAK, which implies the tag was
       * not TAG_BREAK or TAG_INVALID. But we already checked for
       * TAG_INVALID, so it's just if the last tag was TAG_BREAK that
       * we're silent. So we throw in a print for break-next on at
       * the end of the bundle (if it's not the final bundle, which
       * we already check for above), disambiguating this case as
       * well.  Hence in all cases we are unambiguous, QED. */

      if (next_tag == TAG_INVALID)
         fprintf(fp, "\t/* XXX: invalid next tag */\n");

      if (last_next_tag > TAG_BREAK && last_next_tag != tag) {
         fprintf(fp, "\t/* XXX: TAG ERROR sequence, got %s expexted %s */\n",
                 midgard_tag_props[tag].name,
                 midgard_tag_props[last_next_tag].name);
      }

      last_next_tag = next_tag;

      /* Tags are unique in the following way:
       *
       * INVALID, BREAK, UNKNOWN_*: verbosely printed
       * TEXTURE_4_BARRIER: verified by barrier/!barrier op
       * TEXTURE_4_VTX: .vtx tag printed
       * TEXTURE_4: tetxure lack of barriers or .vtx
       * TAG_LOAD_STORE_4: only load/store
       * TAG_ALU_4/8/12/16: by number of instructions/constants
       * TAG_ALU_4_8/12/16_WRITEOUT: ^^ with .writeout tag
       */

      switch (tag) {
      case TAG_TEXTURE_4_VTX ... TAG_TEXTURE_4_BARRIER: {
         bool interpipe_aliasing =
            midgard_get_quirks(gpu_id) & MIDGARD_INTERPIPE_REG_ALIASING;

         print_texture_word(
            &ctx, fp, &words[i], tabs, interpipe_aliasing ? 0 : REG_TEX_BASE,
            interpipe_aliasing ? REGISTER_LDST_BASE : REG_TEX_BASE);
         break;
      }

      case TAG_LOAD_STORE_4:
         print_load_store_word(&ctx, fp, &words[i], verbose);
         break;

      case TAG_ALU_4 ... TAG_ALU_16_WRITEOUT:
         branch_forward = print_alu_word(&ctx, fp, &words[i], num_quad_words,
                                         tabs, i + 4 * num_quad_words, verbose);

         /* TODO: infer/verify me */
         if (tag >= TAG_ALU_4_WRITEOUT)
            fprintf(fp, "writeout\n");

         break;

      default:
         fprintf(fp, "Unknown word type %u:\n", words[i] & 0xF);
         num_quad_words = 1;
         print_quad_word(fp, &words[i], tabs);
         fprintf(fp, "\n");
         break;
      }

      /* Include a synthetic "break" instruction at the end of the
       * bundle to signify that if, absent a branch, the shader
       * execution will stop here. Stop disassembly at such a break
       * based on a heuristic */

      if (next_tag == TAG_BREAK) {
         if (branch_forward) {
            fprintf(fp, "break\n");
         } else {
            fprintf(fp, "\n");
            break;
         }
      }

      fprintf(fp, "\n");

      i += 4 * num_quad_words;
   }

   if (last_next_tag != TAG_BREAK) {
      fprintf(fp, "/* XXX: shader ended with tag %s */\n",
              midgard_tag_props[last_next_tag].name);
   }

   free(ctx.midg_tags);
}
