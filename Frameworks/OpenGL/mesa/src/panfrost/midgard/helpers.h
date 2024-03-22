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

#ifndef __MDG_HELPERS_H
#define __MDG_HELPERS_H

#include <stdio.h>
#include <string.h>
#include "util/macros.h"
#include "midgard.h"

#define OP_IS_LOAD_VARY_F(op)                                                  \
   (op == midgard_op_ld_vary_16 || op == midgard_op_ld_vary_32)

#define OP_IS_PROJECTION(op)                                                   \
   (op == midgard_op_ldst_perspective_div_y ||                                 \
    op == midgard_op_ldst_perspective_div_z ||                                 \
    op == midgard_op_ldst_perspective_div_w)

#define OP_IS_VEC4_ONLY(op)                                                    \
   (OP_IS_PROJECTION(op) || op == midgard_op_ld_cubemap_coords)

#define OP_IS_MOVE(op)                                                         \
   ((op >= midgard_alu_op_fmov && op <= midgard_alu_op_fmov_rtp) ||            \
    op == midgard_alu_op_imov)

#define OP_IS_UBO_READ(op)                                                     \
   (op >= midgard_op_ld_ubo_u8 && op <= midgard_op_ld_ubo_128_bswap8)

#define OP_IS_CSEL_V(op)                                                       \
   (op == midgard_alu_op_icsel_v || op == midgard_alu_op_fcsel_v)

#define OP_IS_CSEL(op)                                                         \
   (OP_IS_CSEL_V(op) || op == midgard_alu_op_icsel ||                          \
    op == midgard_alu_op_fcsel)

#define OP_IS_UNSIGNED_CMP(op)                                                 \
   (op == midgard_alu_op_ult || op == midgard_alu_op_ule)

#define OP_IS_INTEGER_CMP(op)                                                  \
   (op == midgard_alu_op_ieq || op == midgard_alu_op_ine ||                    \
    op == midgard_alu_op_ilt || op == midgard_alu_op_ile ||                    \
    OP_IS_UNSIGNED_CMP(op))

#define OP_IS_COMMON_STORE(op)                                                 \
   (op >= midgard_op_st_u8 && op <= midgard_op_st_128_bswap8)

#define OP_IS_IMAGE(op)                                                        \
   ((op >= midgard_op_ld_image_32f && op <= midgard_op_ld_image_32i) ||        \
    (op >= midgard_op_st_image_32f && op <= midgard_op_st_image_32i) ||        \
    op == midgard_op_lea_image)

#define OP_IS_SPECIAL(op)                                                      \
   ((op >= midgard_op_ld_special_32f && op <= midgard_op_ld_special_32i) ||    \
    (op >= midgard_op_st_special_32f && op <= midgard_op_st_special_32i))

#define OP_IS_PACK_COLOUR(op)                                                  \
   ((op >= midgard_op_pack_colour_f32 && op <= midgard_op_pack_colour_s32))

#define OP_IS_UNPACK_COLOUR(op)                                                \
   ((op >= midgard_op_unpack_colour_f32 && op <= midgard_op_unpack_colour_s32))

/* Instructions that are on the load/store unit but don't access memory */
#define OP_IS_REG2REG_LDST(op)                                                 \
   (op >= midgard_op_unpack_colour_f32 &&                                      \
    op <= midgard_op_ldst_perspective_div_w)

/* ALU control words are single bit fields with a lot of space */

#define ALU_ENAB_VEC_MUL    (1 << 17)
#define ALU_ENAB_SCAL_ADD   (1 << 19)
#define ALU_ENAB_VEC_ADD    (1 << 21)
#define ALU_ENAB_SCAL_MUL   (1 << 23)
#define ALU_ENAB_VEC_LUT    (1 << 25)
#define ALU_ENAB_BR_COMPACT (1 << 26)
#define ALU_ENAB_BRANCH     (1 << 27)

/* Other opcode properties that don't conflict with the ALU_ENABs, non-ISA */

/* Denotes an opcode that takes a vector input with a fixed-number of
 * channels, but outputs to only a single output channel, like dot products.
 * For these, to determine the effective mask, this quirk can be set. We have
 * an intentional off-by-one, since 0-channel makes no sense but we need to fit
 * 4 channels in 2-bits. Similarly, 1-channel doesn't make sense (since then why
 * are we quirked?), so that corresponds to "no count set".
 */
#define OP_CHANNEL_COUNT(c)  ((c - 1) << 0)
#define GET_CHANNEL_COUNT(c) ((c & (0x3 << 0)) ? ((c & (0x3 << 0)) + 1) : 0)

/* For instructions that take a single argument, normally the first argument
 * slot is used for the argument and the second slot is a dummy #0 constant.
 * However, there are exceptions: instructions like fmov store their argument
 * in the _second_ slot and store a dummy r24 in the first slot, designated by
 * QUIRK_FLIPPED_R24 */

#define QUIRK_FLIPPED_R24 (1 << 2)

/* Is the op commutative? */
#define OP_COMMUTES (1 << 3)

/* Does the op convert types between int- and float- space (i2f/f2u/etc) */
#define OP_TYPE_CONVERT (1 << 4)

/* Is this opcode the first in a f2x (rte, rtz, rtn, rtp) sequence? If so,
 * takes a roundmode argument in the IR. This has the semantic of rounding the
 * source (it's all fused in), which is why it doesn't necessarily make sense
 * for i2f (though folding there might be necessary for OpenCL reasons). Comes
 * up in format conversion, i.e. f2u_rte */
#define MIDGARD_ROUNDS (1 << 5)

/* Vector-independant shorthands for the above; these numbers are arbitrary and
 * not from the ISA. Convert to the above with unit_enum_to_midgard */

#define UNIT_MUL 0
#define UNIT_ADD 1
#define UNIT_LUT 2

#define IS_ALU(tag) (tag >= TAG_ALU_4)

/* Special register aliases */

#define MAX_WORK_REGISTERS 16

/* Uniforms are begin at (REGISTER_UNIFORMS - uniform_count) */
#define REGISTER_UNIFORMS 24

/* r24 and r25 are special registers that only exist during the pipeline,
 * by using them when we don't care about the register we skip a roundtrip
 * to the register file. */
#define REGISTER_UNUSED       24
#define REGISTER_CONSTANT     26
#define REGISTER_LDST_BASE    26
#define REGISTER_TEXTURE_BASE 28
#define REGISTER_SELECT       31

/* The following registers are read-only */

/* XY is Program Counter, ZW is Stack Pointer */
#define REGISTER_LDST_PC_SP 2

/* XY is Thread Local Storage pointer, ZW is Workgroup Local Storage pointer */
#define REGISTER_LDST_LOCAL_STORAGE_PTR 3

#define REGISTER_LDST_LOCAL_THREAD_ID  4
#define REGISTER_LDST_GROUP_ID         5
#define REGISTER_LDST_GLOBAL_THREAD_ID 6

/* This register is always zeroed when read. */
#define REGISTER_LDST_ZERO 7

/* SSA helper aliases to mimic the registers. */

#define SSA_FIXED_SHIFT         24
#define SSA_FIXED_REGISTER(reg) (((1 + (reg)) << SSA_FIXED_SHIFT) | 1)
#define SSA_REG_FROM_FIXED(reg) ((((reg) & ~1) >> SSA_FIXED_SHIFT) - 1)
#define SSA_FIXED_MINIMUM       SSA_FIXED_REGISTER(0)

#define COMPONENT_X 0x0
#define COMPONENT_Y 0x1
#define COMPONENT_Z 0x2
#define COMPONENT_W 0x3

#define SWIZZLE_IDENTITY                                                       \
   {                                                                           \
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},                  \
         {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},               \
         {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},               \
      {                                                                        \
         0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15                  \
      }                                                                        \
   }

#define SWIZZLE_IDENTITY_4                                                     \
   {                                                                           \
      {0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                        \
         {0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                     \
         {0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                     \
         {0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                     \
   }

static inline unsigned
mask_of(unsigned nr_comp)
{
   return (1 << nr_comp) - 1;
}

/* See ISA notes */

#define LDST_NOP (3)

/* There are five ALU units: VMUL, VADD, SMUL, SADD, LUT. A given opcode is
 * implemented on some subset of these units (or occassionally all of them).
 * This table encodes a bit mask of valid units for each opcode, so the
 * scheduler can figure where to plonk the instruction. */

/* Shorthands for each unit */
#define UNIT_VMUL ALU_ENAB_VEC_MUL
#define UNIT_SADD ALU_ENAB_SCAL_ADD
#define UNIT_VADD ALU_ENAB_VEC_ADD
#define UNIT_SMUL ALU_ENAB_SCAL_MUL
#define UNIT_VLUT ALU_ENAB_VEC_LUT

/* Shorthands for usual combinations of units */

#define UNITS_MUL        (UNIT_VMUL | UNIT_SMUL)
#define UNITS_ADD        (UNIT_VADD | UNIT_SADD)
#define UNITS_MOST       (UNITS_MUL | UNITS_ADD)
#define UNITS_ALL        (UNITS_MOST | UNIT_VLUT)
#define UNITS_SCALAR     (UNIT_SADD | UNIT_SMUL)
#define UNITS_VECTOR     (UNIT_VMUL | UNIT_VADD)
#define UNITS_ANY_VECTOR (UNITS_VECTOR | UNIT_VLUT)

struct mir_op_props {
   const char *name;
   unsigned props;
};

/* For load/store */

struct mir_ldst_op_props {
   const char *name;
   unsigned props;
};

struct mir_tex_op_props {
   const char *name;
   unsigned props;
};

struct mir_tag_props {
   const char *name;
   unsigned size;
};

/* Lower 2-bits are a midgard_reg_mode */
#define GET_LDST_SIZE(c) (c & 3)

/* Store (so the primary register is a source, not a destination */
#define LDST_STORE (1 << 2)

/* Mask has special meaning and should not be manipulated directly */
#define LDST_SPECIAL_MASK (1 << 3)

/* Non-store operation has side effects and should not be eliminated even if
 * its mask is 0 */
#define LDST_SIDE_FX (1 << 4)

/* Computes an address according to indirects/zext/shift/etc */
#define LDST_ADDRESS (1 << 5)

/* Some fields such swizzle and address have special meanings */
#define LDST_ATOMIC (1 << 6)

/* Operates on attributes/varyings (including images) */
#define LDST_ATTRIB (1 << 7)

/* This file is common, so don't define the tables themselves. #include
 * midgard_op.h if you need that, or edit midgard_ops.c directly */

/* Duplicate bits to convert a per-component to duplicated 8-bit format,
 * which is used for vector units */

static inline unsigned
expand_writemask(unsigned mask, unsigned log2_channels)
{
   unsigned o = 0;
   unsigned factor = 8 >> log2_channels;
   unsigned expanded = (1 << factor) - 1;

   for (unsigned i = 0; i < (1 << log2_channels); ++i)
      if (mask & (1 << i))
         o |= (expanded << (factor * i));

   return o;
}

/* Coerce structs to integer */

static inline unsigned
vector_alu_srco_unsigned(midgard_vector_alu_src src)
{
   unsigned u;
   memcpy(&u, &src, sizeof(src));
   return u;
}

static inline midgard_vector_alu_src
vector_alu_from_unsigned(unsigned u)
{
   midgard_vector_alu_src s;
   memcpy(&s, &u, sizeof(s));
   return s;
}

static inline void
mir_compose_swizzle(unsigned *left, unsigned *right, unsigned *final_out)
{
   unsigned out[16];

   for (unsigned c = 0; c < 16; ++c)
      out[c] = right[left[c]];

   memcpy(final_out, out, sizeof(out));
}

/* Checks for an xyzw.. swizzle, given a mask */

static inline bool
mir_is_simple_swizzle(unsigned *swizzle, unsigned mask)
{
   for (unsigned i = 0; i < 16; ++i) {
      if (!(mask & (1 << i)))
         continue;

      if (swizzle[i] != i)
         return false;
   }

   return true;
}

/* Packs a load/store argument */

static inline uint8_t
midgard_ldst_comp(unsigned reg, unsigned component, unsigned size)
{
   assert((reg & ~1) == 0);
   assert(size == 16 || size == 32 || size == 64);

   /* Shift so everything is in terms of 32-bit units */
   if (size == 64) {
      assert(component < 2);
      component <<= 1;
   } else if (size == 16) {
      assert((component & 1) == 0);
      component >>= 1;
   }

   return component;
}

/* Packs/unpacks a ubo index immediate. The unpack must be defined here so it
 * can be used with the disassembler, which need not be linked with the main
 * compiler.
 */

void midgard_pack_ubo_index_imm(midgard_load_store_word *word, unsigned index);

static inline unsigned
midgard_unpack_ubo_index_imm(midgard_load_store_word word)
{
   unsigned ubo = word.arg_comp | (word.arg_reg << 2) |
                  (word.bitsize_toggle << 5) | (word.index_format << 6);

   return ubo;
}

/* Packs/unpacks varying parameters.
 * FIXME: IMPORTANT: We currently handle varying mode weirdly, by passing all
 * parameters via an offset and using REGISTER_LDST_ZERO as base. This works
 * for most parameters, but does not allow us to encode/decode direct sample
 * position. */
void midgard_pack_varying_params(midgard_load_store_word *word,
                                 midgard_varying_params p);
midgard_varying_params
midgard_unpack_varying_params(midgard_load_store_word word);

/* Load/store ops' displacement helpers.
 * This is useful because different types of load/store ops have different
 * displacement bitsize. */

#define UNPACK_LDST_ATTRIB_OFS(a)   ((a) >> 9)
#define UNPACK_LDST_VERTEX_OFS(a)   util_sign_extend((a)&0x1FF, 9)
#define UNPACK_LDST_SELECTOR_OFS(a) ((a) >> 9)
#define UNPACK_LDST_UBO_OFS(a)      ((a) >> 2)
#define UNPACK_LDST_MEM_OFS(a)      ((a))

#define PACK_LDST_ATTRIB_OFS(a)   ((a) << 9)
#define PACK_LDST_VERTEX_OFS(a)   ((a)&0x1FF)
#define PACK_LDST_SELECTOR_OFS(a) ((a) << 9)
#define PACK_LDST_UBO_OFS(a)      ((a) << 2)
#define PACK_LDST_MEM_OFS(a)      ((a))

static inline bool
midgard_is_branch_unit(unsigned unit)
{
   return (unit == ALU_ENAB_BRANCH) || (unit == ALU_ENAB_BR_COMPACT);
}

/* Packs ALU mod argument */
struct midgard_instruction;
unsigned mir_pack_mod(struct midgard_instruction *ins, unsigned i, bool scalar);

void mir_print_constant_component(FILE *fp, const midgard_constants *consts,
                                  unsigned c, midgard_reg_mode reg_mode,
                                  bool half, unsigned mod, midgard_alu_op op);

void mir_print_outmod(FILE *fp, unsigned outmod, bool is_int);

#endif
