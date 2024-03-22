/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef I915_FPC_H
#define I915_FPC_H

#include "i915_context.h"
#include "i915_reg.h"

#include "pipe/p_shader_tokens.h"

#include "tgsi/tgsi_parse.h"

struct nir_shader;

#define I915_PROGRAM_SIZE 192

/**
 * Program translation state
 */
struct i915_fp_compile {
   struct i915_fragment_shader *shader; /* the shader we're compiling */

   bool used_constants[I915_MAX_CONSTANT];

   /** maps TGSI immediate index to constant slot */
   uint32_t num_immediates;
   uint32_t immediates_map[I915_MAX_CONSTANT];
   float immediates[I915_MAX_CONSTANT][4];

   bool first_instruction;

   uint32_t declarations[I915_PROGRAM_SIZE];
   uint32_t program[I915_PROGRAM_SIZE];

   uint32_t *csr; /**< Cursor, points into program. */

   uint32_t *decl; /**< Cursor, points into declarations. */

   uint32_t decl_s; /**< flags for which s regs need to be decl'd */
   uint32_t decl_t; /**< flags for which t regs need to be decl'd */

   uint32_t temp_flag;  /**< Tracks temporary regs which are in use */
   uint32_t utemp_flag; /**< Tracks TYPE_U temporary regs which are in use */

   uint32_t register_phases[I915_MAX_TEMPORARY];
   uint32_t nr_tex_indirect;
   uint32_t nr_tex_insn;
   uint32_t nr_alu_insn;
   uint32_t nr_decl_insn;

   bool log_program_errors;
   char *error;
   uint32_t NumNativeInstructions;
   uint32_t NumNativeAluInstructions;
   uint32_t NumNativeTexInstructions;
   uint32_t NumNativeTexIndirections;
};

/* Having zero and one in here makes the definition of swizzle a lot
 * easier.
 */
#define UREG_TYPE_SHIFT              29
#define UREG_NR_SHIFT                24
#define UREG_CHANNEL_X_NEGATE_SHIFT  23
#define UREG_CHANNEL_X_SHIFT         20
#define UREG_CHANNEL_Y_NEGATE_SHIFT  19
#define UREG_CHANNEL_Y_SHIFT         16
#define UREG_CHANNEL_Z_NEGATE_SHIFT  15
#define UREG_CHANNEL_Z_SHIFT         12
#define UREG_CHANNEL_W_NEGATE_SHIFT  11
#define UREG_CHANNEL_W_SHIFT         8
#define UREG_CHANNEL_ZERO_NEGATE_MBZ 5
#define UREG_CHANNEL_ZERO_SHIFT      4
#define UREG_CHANNEL_ONE_NEGATE_MBZ  1
#define UREG_CHANNEL_ONE_SHIFT       0

#define UREG_BAD 0xffffffff /* not a valid ureg */

#define X    SRC_X
#define Y    SRC_Y
#define Z    SRC_Z
#define W    SRC_W
#define ZERO SRC_ZERO
#define ONE  SRC_ONE

/* Construct a ureg:
 */
#define UREG(type, nr)                                                         \
   (((type) << UREG_TYPE_SHIFT) | ((nr) << UREG_NR_SHIFT) |                    \
    (X << UREG_CHANNEL_X_SHIFT) | (Y << UREG_CHANNEL_Y_SHIFT) |                \
    (Z << UREG_CHANNEL_Z_SHIFT) | (W << UREG_CHANNEL_W_SHIFT) |                \
    (ZERO << UREG_CHANNEL_ZERO_SHIFT) | (ONE << UREG_CHANNEL_ONE_SHIFT))

#define GET_CHANNEL_SRC(reg, channel) ((reg << (channel * 4)) & (0xf << 20))
#define CHANNEL_SRC(src, channel)     (src >> (channel * 4))

#define GET_UREG_TYPE(reg) (((reg) >> UREG_TYPE_SHIFT) & REG_TYPE_MASK)
#define GET_UREG_NR(reg)   (((reg) >> UREG_NR_SHIFT) & REG_NR_MASK)

#define UREG_XYZW_CHANNEL_MASK 0x00ffff00

/* One neat thing about the UREG representation:
 */
static inline int
swizzle(int reg, uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
   assert(x <= SRC_ONE);
   assert(y <= SRC_ONE);
   assert(z <= SRC_ONE);
   assert(w <= SRC_ONE);
   return ((reg & ~UREG_XYZW_CHANNEL_MASK) |
           CHANNEL_SRC(GET_CHANNEL_SRC(reg, x), 0) |
           CHANNEL_SRC(GET_CHANNEL_SRC(reg, y), 1) |
           CHANNEL_SRC(GET_CHANNEL_SRC(reg, z), 2) |
           CHANNEL_SRC(GET_CHANNEL_SRC(reg, w), 3));
}

#define A0_DEST(reg) (((reg)&UREG_TYPE_NR_MASK) >> UREG_A0_DEST_SHIFT_LEFT)
#define D0_DEST(reg) (((reg)&UREG_TYPE_NR_MASK) >> UREG_A0_DEST_SHIFT_LEFT)
#define T0_DEST(reg) (((reg)&UREG_TYPE_NR_MASK) >> UREG_A0_DEST_SHIFT_LEFT)
#define A0_SRC0(reg) (((reg)&UREG_MASK) >> UREG_A0_SRC0_SHIFT_LEFT)
#define A1_SRC0(reg) (((reg)&UREG_MASK) << UREG_A1_SRC0_SHIFT_RIGHT)
#define A1_SRC1(reg) (((reg)&UREG_MASK) >> UREG_A1_SRC1_SHIFT_LEFT)
#define A2_SRC1(reg) (((reg)&UREG_MASK) << UREG_A2_SRC1_SHIFT_RIGHT)
#define A2_SRC2(reg) (((reg)&UREG_MASK) >> UREG_A2_SRC2_SHIFT_LEFT)

/* These are special, and don't have swizzle/negate bits.
 */
#define T0_SAMPLER(reg) (GET_UREG_NR(reg) << T0_SAMPLER_NR_SHIFT)
#define T1_ADDRESS_REG(reg)                                                    \
   ((GET_UREG_NR(reg) << T1_ADDRESS_REG_NR_SHIFT) |                            \
    (GET_UREG_TYPE(reg) << T1_ADDRESS_REG_TYPE_SHIFT))

/* Macros for translating UREG's into the various register fields used
 * by the I915 programmable unit.
 */
#define UREG_A0_DEST_SHIFT_LEFT (UREG_TYPE_SHIFT - A0_DEST_TYPE_SHIFT)
#define UREG_A0_SRC0_SHIFT_LEFT (UREG_TYPE_SHIFT - A0_SRC0_TYPE_SHIFT)
#define UREG_A1_SRC0_SHIFT_RIGHT                                               \
   (A1_SRC0_CHANNEL_W_SHIFT - UREG_CHANNEL_W_SHIFT)
#define UREG_A1_SRC1_SHIFT_LEFT (UREG_TYPE_SHIFT - A1_SRC1_TYPE_SHIFT)
#define UREG_A2_SRC1_SHIFT_RIGHT                                               \
   (A2_SRC1_CHANNEL_W_SHIFT - UREG_CHANNEL_W_SHIFT)
#define UREG_A2_SRC2_SHIFT_LEFT (UREG_TYPE_SHIFT - A2_SRC2_TYPE_SHIFT)

#define UREG_MASK 0xffffff00
#define UREG_TYPE_NR_MASK                                                      \
   ((REG_TYPE_MASK << UREG_TYPE_SHIFT) | (REG_NR_MASK << UREG_NR_SHIFT))

/***********************************************************************
 * Public interface for the compiler
 */
extern void i915_translate_fragment_program(struct i915_context *i915,
                                            struct i915_fragment_shader *fs);

extern uint32_t i915_get_temp(struct i915_fp_compile *p);
extern uint32_t i915_get_utemp(struct i915_fp_compile *p);
extern void i915_release_utemps(struct i915_fp_compile *p);

extern uint32_t i915_emit_texld(struct i915_fp_compile *p, uint32_t dest,
                                uint32_t destmask, uint32_t sampler,
                                uint32_t coord, uint32_t op,
                                uint32_t coord_mask);

extern uint32_t i915_emit_arith(struct i915_fp_compile *p, uint32_t op,
                                uint32_t dest, uint32_t mask, uint32_t saturate,
                                uint32_t src0, uint32_t src1, uint32_t src2);

extern uint32_t i915_emit_decl(struct i915_fp_compile *p, uint32_t type,
                               uint32_t nr, uint32_t d0_flags);

extern uint32_t i915_emit_const1f(struct i915_fp_compile *p, float c0);

extern uint32_t i915_emit_const2f(struct i915_fp_compile *p, float c0,
                                  float c1);

extern uint32_t i915_emit_const4fv(struct i915_fp_compile *p, const float *c);

extern uint32_t i915_emit_const4f(struct i915_fp_compile *p, float c0, float c1,
                                  float c2, float c3);

/*======================================================================
 * i915_fpc_translate.c
 */

extern void i915_program_error(struct i915_fp_compile *p, const char *msg, ...);

/*======================================================================
 * i915_fpc_optimize.c
 */

struct i915_src_register {
   unsigned File : 4;      /* TGSI_FILE_ */
   unsigned Indirect : 1;  /* BOOL */
   unsigned Dimension : 1; /* BOOL */
   int Index : 16;         /* SINT */
   unsigned SwizzleX : 3;  /* TGSI_SWIZZLE_ */
   unsigned SwizzleY : 3;  /* TGSI_SWIZZLE_ */
   unsigned SwizzleZ : 3;  /* TGSI_SWIZZLE_ */
   unsigned SwizzleW : 3;  /* TGSI_SWIZZLE_ */
   unsigned Absolute : 1;  /* BOOL */
   unsigned Negate : 1;    /* BOOL */
};

/* Additional swizzle supported in i915 */
#define TGSI_SWIZZLE_ZERO 4
#define TGSI_SWIZZLE_ONE  5

struct i915_dst_register {
   unsigned File : 4;      /* TGSI_FILE_ */
   unsigned WriteMask : 4; /* TGSI_WRITEMASK_ */
   unsigned Indirect : 1;  /* BOOL */
   unsigned Dimension : 1; /* BOOL */
   int Index : 16;         /* SINT */
   unsigned Padding : 6;
};

struct i915_full_dst_register {
   struct i915_dst_register Register;
   /*
      struct tgsi_ind_register               Indirect;
      struct tgsi_dimension                  Dimension;
      struct tgsi_ind_register               DimIndirect;
   */
};

struct i915_full_src_register {
   struct i915_src_register Register;
   /*
      struct tgsi_ind_register         Indirect;
      struct tgsi_dimension            Dimension;
      struct tgsi_ind_register         DimIndirect;
   */
};

struct i915_full_instruction {
   struct tgsi_instruction Instruction;
   /*
      struct tgsi_instruction_label       Label;
   */
   struct tgsi_instruction_texture Texture;
   struct i915_full_dst_register Dst[1];
   struct i915_full_src_register Src[3];
};

union i915_full_token {
   struct tgsi_token Token;
   struct tgsi_full_declaration FullDeclaration;
   struct tgsi_full_immediate FullImmediate;
   struct i915_full_instruction FullInstruction;
   struct tgsi_full_property FullProperty;
};

struct i915_token_list {
   union i915_full_token *Tokens;
   unsigned NumTokens;
};

char *i915_test_fragment_shader_compile(struct pipe_screen *screen,
                                        struct nir_shader *s);

extern struct i915_token_list *i915_optimize(const struct tgsi_token *tokens);

extern void i915_optimize_free(struct i915_token_list *tokens);

extern uint32_t i915_coord_mask(enum tgsi_opcode opcode,
                                enum tgsi_texture_type tex);

#endif
