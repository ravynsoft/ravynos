/*
 * Copyright (c) 2017 Lima Project
 * Copyright (c) 2013 Ben Brewer (ben.brewer@codethink.co.uk)
 * Copyright (c) 2013 Connor Abbott (connor@abbott.cx)
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
 */

#ifndef LIMA_IR_PP_CODEGEN_H
#define LIMA_IR_PP_CODEGEN_H

#include <stdint.h>
#include <stdbool.h>

/* Control */

typedef union __attribute__((__packed__)) {
   struct __attribute__((__packed__)) {
      unsigned count      :  5;
      bool     stop       :  1;
      bool     sync       :  1;
      unsigned fields     : 12;
      unsigned next_count :  6;
      bool     prefetch   :  1;
      unsigned unknown    :  6;
   };
   uint32_t mask;
} ppir_codegen_ctrl;

typedef enum {
   ppir_codegen_field_shift_varying      =  0,
   ppir_codegen_field_shift_sampler      =  1,
   ppir_codegen_field_shift_uniform      =  2,
   ppir_codegen_field_shift_vec4_mul     =  3,
   ppir_codegen_field_shift_float_mul    =  4,
   ppir_codegen_field_shift_vec4_acc     =  5,
   ppir_codegen_field_shift_float_acc    =  6,
   ppir_codegen_field_shift_combine      =  7,
   ppir_codegen_field_shift_temp_write   =  8,
   ppir_codegen_field_shift_branch       =  9,
   ppir_codegen_field_shift_vec4_const_0 = 10,
   ppir_codegen_field_shift_vec4_const_1 = 11,
   ppir_codegen_field_shift_count        = 12,
} ppir_codegen_field_shift;

/* Data Inputs */

typedef enum {
   ppir_codegen_vec4_reg_frag_color =  0,
   ppir_codegen_vec4_reg_constant0  = 12,
   ppir_codegen_vec4_reg_constant1  = 13,
   ppir_codegen_vec4_reg_texture    = 14,
   ppir_codegen_vec4_reg_uniform    = 15,
   ppir_codegen_vec4_reg_discard    = 15,
} ppir_codegen_vec4_reg;

typedef union __attribute__((__packed__)) {
   struct __attribute__((__packed__)) {
      unsigned              perspective   :  2;
      unsigned              source_type   :  2;
      unsigned              unknown_0     :  1; /* = 0 */
      unsigned              alignment     :  2;
      unsigned              unknown_1     :  3; /* = 00 0 */
      unsigned              offset_vector :  4;
      unsigned              unknown_2     :  2; /* = 00 */
      unsigned              offset_scalar :  2;
      unsigned              index         :  6;
      ppir_codegen_vec4_reg dest          :  4;
      unsigned              mask          :  4;
      unsigned              unknown_3     :  2; /* = 00 */
   } imm;
   struct __attribute__((__packed__)) {
      unsigned              perspective :  2;
      unsigned              source_type :  2; /* = 01 */
      unsigned              unknown_0   :  2; /* = 00 */
      bool                  normalize   :  1;
      unsigned              unknown_1   :  3;
      ppir_codegen_vec4_reg source      :  4;
      bool                  negate      :  1;
      bool                  absolute    :  1;
      unsigned              swizzle     :  8;
      ppir_codegen_vec4_reg dest        :  4;
      unsigned              mask        :  4;
      unsigned              unknown_2   :  2; /* = 00 */
   } reg;
} ppir_codegen_field_varying;

typedef enum {
   ppir_codegen_sampler_type_generic = 0x00,
   ppir_codegen_sampler_type_cube    = 0x1F,
} ppir_codegen_sampler_type;

typedef struct __attribute__((__packed__)) {
   unsigned                  lod_bias     :  6;
   unsigned                  index_offset :  6;
   unsigned                  unknown_0    :  5; /* = 00000 */
   bool                      explicit_lod :  1;
   bool                      lod_bias_en  :  1;
   unsigned                  unknown_1    :  5; /* = 00000 */
   ppir_codegen_sampler_type type         :  5;
   bool                      offset_en    :  1;
   unsigned                  index        : 12;
   unsigned                  unknown_2    : 20; /* = 0011 1001 0000 0000 0001 */
} ppir_codegen_field_sampler;

typedef enum {
   ppir_codegen_uniform_src_uniform   = 0,
   ppir_codegen_uniform_src_temporary = 3,
} ppir_codegen_uniform_src;

typedef struct __attribute__((__packed__)) {
   ppir_codegen_uniform_src source     :  2;
   unsigned                 unknown_0  :  8; /* = 00 0000 00 */
   unsigned                 alignment  :  2; /* 00: float, 01: vec2, 10: vec4 */
   unsigned                 unknown_1  :  6; /* = 00 0000 */
   unsigned                 offset_reg :  6;
   bool                     offset_en  :  1;
   unsigned                 index      : 16;
} ppir_codegen_field_uniform;

/* Vector Pipe */

typedef enum {
   ppir_codegen_vec4_mul_op_not = 0x08, /* Logical Not */
   ppir_codegen_vec4_mul_op_and = 0x09, /* Logical AND */
   ppir_codegen_vec4_mul_op_or  = 0x0A, /* Logical OR */
   ppir_codegen_vec4_mul_op_xor = 0x0B, /* Logical XOR */
   ppir_codegen_vec4_mul_op_ne  = 0x0C, /* Not Equal */
   ppir_codegen_vec4_mul_op_gt  = 0x0D, /* Great Than */
   ppir_codegen_vec4_mul_op_ge  = 0x0E, /* Great than or Equal */
   ppir_codegen_vec4_mul_op_eq  = 0x0F, /* Equal */
   ppir_codegen_vec4_mul_op_min = 0x10, /* Minimum */
   ppir_codegen_vec4_mul_op_max = 0x11, /* Maximum */
   ppir_codegen_vec4_mul_op_mov = 0x1F, /* Passthrough, result = arg1 */
} ppir_codegen_vec4_mul_op;

typedef enum {
   ppir_codegen_outmod_none           = 0,
   ppir_codegen_outmod_clamp_fraction = 1,
   ppir_codegen_outmod_clamp_positive = 2,
   ppir_codegen_outmod_round          = 3,
} ppir_codegen_outmod;

typedef struct __attribute__((__packed__)) {
   ppir_codegen_vec4_reg    arg0_source   : 4;
   unsigned                 arg0_swizzle  : 8;
   bool                     arg0_absolute : 1;
   bool                     arg0_negate   : 1;
   ppir_codegen_vec4_reg    arg1_source   : 4;
   unsigned                 arg1_swizzle  : 8;
   bool                     arg1_absolute : 1;
   bool                     arg1_negate   : 1;
   unsigned                 dest          : 4;
   unsigned                 mask          : 4;
   ppir_codegen_outmod      dest_modifier : 2;
   ppir_codegen_vec4_mul_op op            : 5;
} ppir_codegen_field_vec4_mul;

typedef enum {
   ppir_codegen_vec4_acc_op_add   = 0x00,
   ppir_codegen_vec4_acc_op_fract = 0x04, /* Fract? */
   ppir_codegen_vec4_acc_op_ne    = 0x08, /* Not Equal */
   ppir_codegen_vec4_acc_op_gt    = 0x09, /* Great-Than */
   ppir_codegen_vec4_acc_op_ge    = 0x0A, /* Great-than or Equal */
   ppir_codegen_vec4_acc_op_eq    = 0x0B, /* Equal */
   ppir_codegen_vec4_acc_op_floor = 0x0C,
   ppir_codegen_vec4_acc_op_ceil  = 0x0D,
   ppir_codegen_vec4_acc_op_min   = 0x0E,
   ppir_codegen_vec4_acc_op_max   = 0x0F,
   ppir_codegen_vec4_acc_op_sum3  = 0x10, /* dest.xyzw = (arg0.x + arg0.y + arg0.z) */
   ppir_codegen_vec4_acc_op_sum4  = 0x11, /* dest.xyzw = (arg0.x + arg0.y + arg0.z + arg0.w) */
   ppir_codegen_vec4_acc_op_dFdx  = 0x14,
   ppir_codegen_vec4_acc_op_dFdy  = 0x15,
   ppir_codegen_vec4_acc_op_sel   = 0x17, /* result = (^fmul ? arg0 : arg1) */
   ppir_codegen_vec4_acc_op_mov   = 0x1F, /* Passthrough, result = arg0 */
} ppir_codegen_vec4_acc_op;

typedef struct __attribute__((__packed__)) {
   ppir_codegen_vec4_reg    arg0_source   : 4;
   unsigned                 arg0_swizzle  : 8;
   bool                     arg0_absolute : 1;
   bool                     arg0_negate   : 1;
   ppir_codegen_vec4_reg    arg1_source   : 4;
   unsigned                 arg1_swizzle  : 8;
   bool                     arg1_absolute : 1;
   bool                     arg1_negate   : 1;
   unsigned                 dest          : 4;
   unsigned                 mask          : 4;
   ppir_codegen_outmod      dest_modifier : 2;
   ppir_codegen_vec4_acc_op op            : 5;
   bool                     mul_in        : 1; /* whether to get arg0 from multiply unit below */
} ppir_codegen_field_vec4_acc;

/* Float (Scalar) Pipe */

typedef enum {
   ppir_codegen_float_mul_op_not = 0x08, /* Logical Not */
   ppir_codegen_float_mul_op_and = 0x09, /* Logical AND */
   ppir_codegen_float_mul_op_or  = 0x0A, /* Logical OR */
   ppir_codegen_float_mul_op_xor = 0x0B, /* Logical XOR */
   ppir_codegen_float_mul_op_ne  = 0x0C, /* Not Equal */
   ppir_codegen_float_mul_op_gt  = 0x0D, /* Great Than */
   ppir_codegen_float_mul_op_ge  = 0x0E, /* great than or Equal */
   ppir_codegen_float_mul_op_eq  = 0x0F, /* Equal */
   ppir_codegen_float_mul_op_min = 0x10, /* Minimum */
   ppir_codegen_float_mul_op_max = 0x11, /* Maximum */
   ppir_codegen_float_mul_op_mov = 0x1F, /* Passthrough, result = arg1 */
} ppir_codegen_float_mul_op;

typedef struct __attribute__((__packed__)) {
   unsigned                  arg0_source   : 6;
   bool                      arg0_absolute : 1;
   bool                      arg0_negate   : 1;
   unsigned                  arg1_source   : 6;
   bool                      arg1_absolute : 1;
   bool                      arg1_negate   : 1;
   unsigned                  dest          : 6;
   bool                      output_en     : 1; /* Set to 0 when outputting directly to float_acc below. */
   ppir_codegen_outmod       dest_modifier : 2;
   ppir_codegen_float_mul_op op            : 5;
} ppir_codegen_field_float_mul;

typedef enum {
   ppir_codegen_float_acc_op_add   = 0x00,
   ppir_codegen_float_acc_op_fract = 0x04,
   ppir_codegen_float_acc_op_ne    = 0x08, /* Not Equal */
   ppir_codegen_float_acc_op_gt    = 0x09, /* Great-Than */
   ppir_codegen_float_acc_op_ge    = 0x0A, /* Great-than or Equal */
   ppir_codegen_float_acc_op_eq    = 0x0B, /* Equal */
   ppir_codegen_float_acc_op_floor = 0x0C,
   ppir_codegen_float_acc_op_ceil  = 0x0D,
   ppir_codegen_float_acc_op_min   = 0x0E,
   ppir_codegen_float_acc_op_max   = 0x0F,
   ppir_codegen_float_acc_op_dFdx  = 0x14,
   ppir_codegen_float_acc_op_dFdy  = 0x15,
   ppir_codegen_float_acc_op_sel   = 0x17, /* result = (^fmul ? arg0 : arg1) */
   ppir_codegen_float_acc_op_mov   = 0x1F, /* Passthrough, result = arg1 */
} ppir_codegen_float_acc_op;

typedef struct __attribute__((__packed__)) {
   unsigned                  arg0_source   : 6;
   bool                      arg0_absolute : 1;
   bool                      arg0_negate   : 1;
   unsigned                  arg1_source   : 6;
   bool                      arg1_absolute : 1;
   bool                      arg1_negate   : 1;
   unsigned                  dest          : 6;
   bool                      output_en     : 1; /* Always true */
   ppir_codegen_outmod       dest_modifier : 2;
   ppir_codegen_float_acc_op op            : 5;
   bool                      mul_in        : 1; /* Get arg1 from float_mul above. */
} ppir_codegen_field_float_acc;

/* Temporary Write / Framebuffer Read */

typedef union __attribute__((__packed__)) {
   struct __attribute__((__packed__)) {
      unsigned dest       :  2; /* = 11 */
      unsigned unknown_0  :  2; /* = 00 */
      unsigned source     :  6;
      unsigned alignment  :  2; /* 0: float, 1:vec2, 2: vec4 */
      unsigned unknown_1  :  6; /* = 00 0000 */
      unsigned offset_reg :  6;
      bool     offset_en  :  1;
      unsigned index      : 16;
   } temp_write;
   struct __attribute__((__packed__)) {
      bool     source    :  1; /* 0 = fb_depth, 1 = fb_color */
      unsigned unknown_0 :  5; /* = 00 111 */
      unsigned dest      :  4;
      unsigned unknown_1 : 31; /* = 0 0000 ... 10 */
   } fb_read;
} ppir_codegen_field_temp_write;

/* Result combiner */

typedef enum {
   ppir_codegen_combine_scalar_op_rcp   = 0, /* Reciprocal */
   ppir_codegen_combine_scalar_op_mov   = 1, /* No Operation */
   ppir_codegen_combine_scalar_op_sqrt  = 2, /* Square-Root */
   ppir_codegen_combine_scalar_op_rsqrt = 3, /* Inverse Square-Root */
   ppir_codegen_combine_scalar_op_exp2  = 4, /* Binary Exponent */
   ppir_codegen_combine_scalar_op_log2  = 5, /* Binary Logarithm */
   ppir_codegen_combine_scalar_op_sin   = 6, /* Sine   (Scaled LUT) */
   ppir_codegen_combine_scalar_op_cos   = 7, /* Cosine (Scaled LUT) */
   ppir_codegen_combine_scalar_op_atan  = 8, /* Arc Tangent Part 1 */
   ppir_codegen_combine_scalar_op_atan2 = 9, /* Arc Tangent 2 Part 1 */
} ppir_codegen_combine_scalar_op;

typedef union __attribute__((__packed__)) {
   struct __attribute__((__packed__)) {
      bool                           dest_vec      : 1;
      bool                           arg1_en       : 1;
      ppir_codegen_combine_scalar_op op            : 4;
      bool                           arg1_absolute : 1;
      bool                           arg1_negate   : 1;
      unsigned                       arg1_src      : 6;
      bool                           arg0_absolute : 1;
      bool                           arg0_negate   : 1;
      unsigned                       arg0_src      : 6;
      ppir_codegen_outmod            dest_modifier : 2;
      unsigned                       dest          : 6;
   } scalar;
   struct __attribute__((__packed__)) {
      bool     dest_vec     : 1;
      bool     arg1_en      : 1;
      unsigned arg1_swizzle : 8;
      unsigned arg1_source  : 4;
      unsigned padding_0    : 8;
      unsigned mask         : 4;
      unsigned dest         : 4;
   } vector;
} ppir_codegen_field_combine;

/* Branch/Control Flow */

#define PPIR_CODEGEN_DISCARD_WORD0 0x007F0003
#define PPIR_CODEGEN_DISCARD_WORD1 0x00000000
#define PPIR_CODEGEN_DISCARD_WORD2 0x000

typedef union __attribute__((__packed__)) {
   struct __attribute__((__packed__)) {
      unsigned unknown_0   :  4; /* = 0000 */
      unsigned arg1_source :  6;
      unsigned arg0_source :  6;
      bool     cond_gt     :  1;
      bool     cond_eq     :  1;
      bool     cond_lt     :  1;
      unsigned unknown_1   : 22; /* = 0 0000 0000 0000 0000 0000 0 */
      signed   target      : 27;
      unsigned next_count  :  5;
   } branch;
   struct __attribute__((__packed__)) {
      unsigned word0 : 32;
      unsigned word1 : 32;
      unsigned word2 : 9;
   } discard;
} ppir_codegen_field_branch;

void ppir_disassemble_instr(uint32_t *instr, unsigned offset, FILE *fp);

#endif
