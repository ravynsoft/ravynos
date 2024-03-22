/*
 * Copyright (c) 2012-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#ifndef H_ETNAVIV_ASM
#define H_ETNAVIV_ASM

#include <stdint.h>
#include <stdbool.h>
#include "util/u_math.h"
#include "hw/isa.xml.h"

/* Size of an instruction in 32-bit words */
#define ETNA_INST_SIZE (4)
/* Number of source operands per instruction */
#define ETNA_NUM_SRC (3)

/* Broadcast swizzle to all four components */
#define INST_SWIZ_BROADCAST(x) \
        (INST_SWIZ_X(x) | INST_SWIZ_Y(x) | INST_SWIZ_Z(x) | INST_SWIZ_W(x))
/* Identity (NOP) swizzle */
#define INST_SWIZ_IDENTITY \
        (INST_SWIZ_X(0) | INST_SWIZ_Y(1) | INST_SWIZ_Z(2) | INST_SWIZ_W(3))
/* Fully specified swizzle */
#define INST_SWIZ(x,y,z,w) \
        (INST_SWIZ_X(x) | INST_SWIZ_Y(y) | INST_SWIZ_Z(z) | INST_SWIZ_W(w))
#define SWIZZLE(c0,c1,c2,c3) \
        INST_SWIZ(INST_SWIZ_COMP_##c0, \
                  INST_SWIZ_COMP_##c1, \
                  INST_SWIZ_COMP_##c2, \
                  INST_SWIZ_COMP_##c3)

/*** operands ***/

/* destination operand */
struct etna_inst_dst {
   unsigned use:1; /* 0: not in use, 1: in use */
   unsigned amode:3; /* INST_AMODE_* */
   unsigned reg:7; /* register number 0..127 */
   unsigned write_mask:4; /* INST_COMPS_* */
};

/* texture operand */
struct etna_inst_tex {
   unsigned id:5; /* sampler id */
   unsigned amode:3; /* INST_AMODE_* */
   unsigned swiz:8; /* INST_SWIZ */
};

/* source operand */
struct etna_inst_src {
   unsigned use:1; /* 0: not in use, 1: in use */
   unsigned rgroup:3; /* INST_RGROUP_* */
   union {
      struct __attribute__((__packed__)) {
         unsigned reg:9; /* register or uniform number 0..511 */
         unsigned swiz:8;   /* INST_SWIZ */
         unsigned neg:1;    /* negate (flip sign) if set */
         unsigned abs:1;    /* absolute (remove sign) if set */
         unsigned amode:3;  /* INST_AMODE_* */
      };
      struct __attribute__((__packed__)) {
         unsigned imm_val : 20;
         unsigned imm_type : 2;
      };
   };
};

/*** instruction ***/
struct etna_inst {
   uint8_t opcode; /* INST_OPCODE_* */
   uint8_t type; /* INST_TYPE_* */
   unsigned cond:5; /* INST_CONDITION_* */
   unsigned sat:1; /* saturate result between 0..1 */
   unsigned sel_bit0:1; /* select low half mediump */
   unsigned sel_bit1:1; /* select high half mediump */
   unsigned dst_full:1; /* write to highp register */
   unsigned no_oneconst_limit:1; /* allow multiple different uniform sources */
   struct etna_inst_dst dst; /* destination operand */
   struct etna_inst_tex tex; /* texture operand */
   struct etna_inst_src src[ETNA_NUM_SRC]; /* source operand */
   unsigned imm;  /* takes place of src[2] for BRANCH/CALL */
};

/* Compose two swizzles (computes swz1.swz2) */
static inline uint32_t inst_swiz_compose(uint32_t swz1, uint32_t swz2)
{
   return INST_SWIZ_X((swz1 >> (((swz2 >> 0)&3)*2))&3) |
          INST_SWIZ_Y((swz1 >> (((swz2 >> 2)&3)*2))&3) |
          INST_SWIZ_Z((swz1 >> (((swz2 >> 4)&3)*2))&3) |
          INST_SWIZ_W((swz1 >> (((swz2 >> 6)&3)*2))&3);
};

/* Compose two write_masks (computes wm1.wm2) */
static inline uint32_t inst_write_mask_compose(uint32_t wm1, uint32_t wm2)
{
   unsigned wm = 0;
   for (unsigned i = 0, j = 0; i < 4; i++) {
      if (wm2 & (1 << i)) {
         if (wm1 & (1 << j))
            wm |= (1 << i);
         j++;
      }
   }
   return wm;
};

/* Return whether the rgroup is one of the uniforms */
static inline int
etna_rgroup_is_uniform(unsigned rgroup)
{
   return rgroup == INST_RGROUP_UNIFORM_0 ||
          rgroup == INST_RGROUP_UNIFORM_1;
}

static inline struct etna_inst_src
etna_immediate_src(unsigned type, uint32_t bits)
{
   return (struct etna_inst_src) {
      .use = 1,
      .rgroup = INST_RGROUP_IMMEDIATE,
      .imm_val = bits,
      .imm_type = type
   };
}

static inline struct etna_inst_src
etna_immediate_float(float x)
{
	uint32_t bits = fui(x);
	assert((bits & 0xfff) == 0); /* 12 lsb cut off */
	return etna_immediate_src(0, bits >> 12);
}

static inline struct etna_inst_src
etna_immediate_int(int x)
{
    assert(x >= -0x80000 && x < 0x80000); /* 20-bit signed int */
	return etna_immediate_src(1, x);
}

/**
 * Build vivante instruction from structure with
 *  opcode, cond, sat, dst_use, dst_amode,
 *  dst_reg, dst_comps, tex_id, tex_amode, tex_swiz,
 *  src[0-2]_reg, use, swiz, neg, abs, amode, rgroup,
 *  imm
 *
 * Return 0 if successful, and a non-zero
 * value otherwise.
 */
int
etna_assemble(uint32_t *out, const struct etna_inst *inst);

/**
 * Set field imm of already-assembled instruction.
 * This is used for filling in jump destinations in a separate pass.
 */
static inline void
etna_assemble_set_imm(uint32_t *out, uint32_t imm)
{
    out[3] |= VIV_ISA_WORD_3_SRC2_IMM(imm);
}

#endif
