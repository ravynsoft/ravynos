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

#ifndef __midgard_h__
#define __midgard_h__

#include <stdbool.h>
#include <stdint.h>

#define MIDGARD_DBG_SHADERS  0x0002
#define MIDGARD_DBG_SHADERDB 0x0004
#define MIDGARD_DBG_INORDER  0x0008
#define MIDGARD_DBG_VERBOSE  0x0010
#define MIDGARD_DBG_INTERNAL 0x0020

extern int midgard_debug;

typedef enum {
   midgard_word_type_alu,
   midgard_word_type_load_store,
   midgard_word_type_texture
} midgard_word_type;

typedef enum {
   midgard_alu_vmul,
   midgard_alu_sadd,
   midgard_alu_smul,
   midgard_alu_vadd,
   midgard_alu_lut
} midgard_alu;

enum {
   TAG_INVALID = 0x0,
   TAG_BREAK = 0x1,
   TAG_TEXTURE_4_VTX = 0x2,
   TAG_TEXTURE_4 = 0x3,
   TAG_TEXTURE_4_BARRIER = 0x4,
   TAG_LOAD_STORE_4 = 0x5,
   TAG_UNKNOWN_1 = 0x6,
   TAG_UNKNOWN_2 = 0x7,
   TAG_ALU_4 = 0x8,
   TAG_ALU_8 = 0x9,
   TAG_ALU_12 = 0xA,
   TAG_ALU_16 = 0xB,
   TAG_ALU_4_WRITEOUT = 0xC,
   TAG_ALU_8_WRITEOUT = 0xD,
   TAG_ALU_12_WRITEOUT = 0xE,
   TAG_ALU_16_WRITEOUT = 0xF
};

/*
 * ALU words
 */

typedef enum {
   midgard_alu_op_fadd = 0x10, /* round to even */
   midgard_alu_op_fadd_rtz = 0x11,
   midgard_alu_op_fadd_rtn = 0x12,
   midgard_alu_op_fadd_rtp = 0x13,
   midgard_alu_op_fmul = 0x14, /* round to even */
   midgard_alu_op_fmul_rtz = 0x15,
   midgard_alu_op_fmul_rtn = 0x16,
   midgard_alu_op_fmul_rtp = 0x17,

   midgard_alu_op_fmin = 0x28, /* if an operand is NaN, propagate the other */
   midgard_alu_op_fmin_nan = 0x29,    /* if an operand is NaN, propagate it */
   midgard_alu_op_fabsmin = 0x2A,     /* min(abs(a,b)) */
   midgard_alu_op_fabsmin_nan = 0x2B, /* min_nan(abs(a,b)) */
   midgard_alu_op_fmax = 0x2C, /* if an operand is NaN, propagate the other */
   midgard_alu_op_fmax_nan = 0x2D,    /* if an operand is NaN, propagate it */
   midgard_alu_op_fabsmax = 0x2E,     /* max(abs(a,b)) */
   midgard_alu_op_fabsmax_nan = 0x2F, /* max_nan(abs(a,b)) */

   midgard_alu_op_fmov = 0x30, /* fmov_rte */
   midgard_alu_op_fmov_rtz = 0x31,
   midgard_alu_op_fmov_rtn = 0x32,
   midgard_alu_op_fmov_rtp = 0x33,
   midgard_alu_op_froundeven = 0x34,
   midgard_alu_op_ftrunc = 0x35,
   midgard_alu_op_ffloor = 0x36,
   midgard_alu_op_fceil = 0x37,
   midgard_alu_op_ffma = 0x38, /* rte */
   midgard_alu_op_ffma_rtz = 0x39,
   midgard_alu_op_ffma_rtn = 0x3A,
   midgard_alu_op_ffma_rtp = 0x3B,
   midgard_alu_op_fdot3 = 0x3C,
   midgard_alu_op_fdot3r = 0x3D,
   midgard_alu_op_fdot4 = 0x3E,
   midgard_alu_op_freduce = 0x3F,

   midgard_alu_op_iadd = 0x40,
   midgard_alu_op_ishladd = 0x41, /* (a<<1) + b */
   midgard_alu_op_isub = 0x46,
   midgard_alu_op_ishlsub = 0x47, /* (a<<1) - b */
   midgard_alu_op_iaddsat = 0x48,
   midgard_alu_op_uaddsat = 0x49,
   midgard_alu_op_isubsat = 0x4E,
   midgard_alu_op_usubsat = 0x4F,

   midgard_alu_op_imul = 0x58,
   /* Multiplies two ints and stores the result in the next larger datasize. */
   midgard_alu_op_iwmul = 0x59,  /* sint * sint = sint */
   midgard_alu_op_uwmul = 0x5A,  /* uint * uint = uint */
   midgard_alu_op_iuwmul = 0x5B, /* sint * uint = sint */

   midgard_alu_op_imin = 0x60,
   midgard_alu_op_umin = 0x61,
   midgard_alu_op_imax = 0x62,
   midgard_alu_op_umax = 0x63,
   midgard_alu_op_iavg = 0x64,
   midgard_alu_op_uavg = 0x65,
   midgard_alu_op_iravg = 0x66,
   midgard_alu_op_uravg = 0x67,
   midgard_alu_op_iasr = 0x68,
   midgard_alu_op_ilsr = 0x69,
   midgard_alu_op_ishlsat = 0x6C,
   midgard_alu_op_ushlsat = 0x6D,
   midgard_alu_op_ishl = 0x6E,

   midgard_alu_op_iand = 0x70,
   midgard_alu_op_ior = 0x71,
   midgard_alu_op_inand = 0x72,   /* ~(a & b), for inot let a = b */
   midgard_alu_op_inor = 0x73,    /* ~(a | b) */
   midgard_alu_op_iandnot = 0x74, /* (a & ~b), used for not/b2f */
   midgard_alu_op_iornot = 0x75,  /* (a | ~b) */
   midgard_alu_op_ixor = 0x76,
   midgard_alu_op_inxor = 0x77,   /* ~(a ^ b) */
   midgard_alu_op_iclz = 0x78,    /* Number of zeroes on left */
   midgard_alu_op_ipopcnt = 0x7A, /* Population count */
   midgard_alu_op_imov = 0x7B,
   midgard_alu_op_iabsdiff = 0x7C,
   midgard_alu_op_uabsdiff = 0x7D,
   midgard_alu_op_ichoose =
      0x7E, /* vector, component number - dupe for shuffle() */

   midgard_alu_op_feq = 0x80,
   midgard_alu_op_fne = 0x81,
   midgard_alu_op_flt = 0x82,
   midgard_alu_op_fle = 0x83,
   midgard_alu_op_fball_eq = 0x88,
   midgard_alu_op_fball_neq = 0x89,
   midgard_alu_op_fball_lt = 0x8A,  /* all(lessThan(.., ..)) */
   midgard_alu_op_fball_lte = 0x8B, /* all(lessThanEqual(.., ..)) */

   midgard_alu_op_fbany_eq = 0x90,
   midgard_alu_op_fbany_neq = 0x91,
   midgard_alu_op_fbany_lt = 0x92,  /* any(lessThan(.., ..)) */
   midgard_alu_op_fbany_lte = 0x93, /* any(lessThanEqual(.., ..)) */

   midgard_alu_op_f2i_rte = 0x98,
   midgard_alu_op_f2i_rtz = 0x99,
   midgard_alu_op_f2i_rtn = 0x9A,
   midgard_alu_op_f2i_rtp = 0x9B,
   midgard_alu_op_f2u_rte = 0x9C,
   midgard_alu_op_f2u_rtz = 0x9D,
   midgard_alu_op_f2u_rtn = 0x9E,
   midgard_alu_op_f2u_rtp = 0x9F,

   midgard_alu_op_ieq = 0xA0,
   midgard_alu_op_ine = 0xA1,
   midgard_alu_op_ult = 0xA2,
   midgard_alu_op_ule = 0xA3,
   midgard_alu_op_ilt = 0xA4,
   midgard_alu_op_ile = 0xA5,
   midgard_alu_op_iball_eq = 0xA8,
   midgard_alu_op_iball_neq = 0xA9,
   midgard_alu_op_uball_lt = 0xAA,
   midgard_alu_op_uball_lte = 0xAB,
   midgard_alu_op_iball_lt = 0xAC,
   midgard_alu_op_iball_lte = 0xAD,

   midgard_alu_op_ibany_eq = 0xB0,
   midgard_alu_op_ibany_neq = 0xB1,
   midgard_alu_op_ubany_lt = 0xB2,
   midgard_alu_op_ubany_lte = 0xB3,
   midgard_alu_op_ibany_lt = 0xB4,  /* any(lessThan(.., ..)) */
   midgard_alu_op_ibany_lte = 0xB5, /* any(lessThanEqual(.., ..)) */
   midgard_alu_op_i2f_rte = 0xB8,
   midgard_alu_op_i2f_rtz = 0xB9,
   midgard_alu_op_i2f_rtn = 0xBA,
   midgard_alu_op_i2f_rtp = 0xBB,
   midgard_alu_op_u2f_rte = 0xBC,
   midgard_alu_op_u2f_rtz = 0xBD,
   midgard_alu_op_u2f_rtn = 0xBE,
   midgard_alu_op_u2f_rtp = 0xBF,

   /* All csel* instructions use as a condition the output of the previous
    * vector or scalar unit, thus it must run on the second pipeline stage
    * and be scheduled to the same bundle as the opcode that it uses as a
    * condition. */
   midgard_alu_op_icsel_v = 0xC0,
   midgard_alu_op_icsel = 0xC1,
   midgard_alu_op_fcsel_v = 0xC4,
   midgard_alu_op_fcsel = 0xC5,
   midgard_alu_op_froundaway = 0xC6, /* round to nearest away */

   midgard_alu_op_fatan2_pt2 = 0xE8,
   midgard_alu_op_fpow_pt1 = 0xEC,
   midgard_alu_op_fpown_pt1 = 0xED,
   midgard_alu_op_fpowr_pt1 = 0xEE,

   midgard_alu_op_frcp = 0xF0,
   midgard_alu_op_frsqrt = 0xF2,
   midgard_alu_op_fsqrt = 0xF3,
   midgard_alu_op_fexp2 = 0xF4,
   midgard_alu_op_flog2 = 0xF5,
   midgard_alu_op_fsinpi = 0xF6, /* sin(pi * x) */
   midgard_alu_op_fcospi = 0xF7, /* cos(pi * x) */
   midgard_alu_op_fatan2_pt1 = 0xF9,
} midgard_alu_op;

typedef enum {
   midgard_outmod_none = 0,
   midgard_outmod_clamp_0_inf = 1, /* max(x, 0.0), NaNs become +0.0 */
   midgard_outmod_clamp_m1_1 = 2,  /* clamp(x, -1.0, 1.0), NaNs become -1.0 */
   midgard_outmod_clamp_0_1 = 3    /* clamp(x, 0.0, 1.0), NaNs become +0.0 */
} midgard_outmod_float;

/* These are applied to the resulting value that's going to be stored in the
 * dest reg. This should be set to midgard_outmod_keeplo when shrink_mode is
 * midgard_shrink_mode_none. */
typedef enum {
   midgard_outmod_ssat = 0,
   midgard_outmod_usat = 1,
   midgard_outmod_keeplo = 2, /* Keep low half */
   midgard_outmod_keephi = 3, /* Keep high half */
} midgard_outmod_int;

typedef enum {
   midgard_reg_mode_8 = 0,
   midgard_reg_mode_16 = 1,
   midgard_reg_mode_32 = 2,
   midgard_reg_mode_64 = 3
} midgard_reg_mode;

typedef enum {
   midgard_shrink_mode_lower = 0,
   midgard_shrink_mode_upper = 1,
   midgard_shrink_mode_none = 2
} midgard_shrink_mode;

/* Only used if midgard_src_expand_mode is set to one of midgard_src_expand_*. */
typedef enum {
   midgard_int_sign_extend = 0,
   midgard_int_zero_extend = 1,
   midgard_int_replicate = 2,
   midgard_int_left_shift = 3
} midgard_int_mod;

/* Unlike midgard_int_mod, fload modifiers are applied after the expansion
 * happens, so they don't depend on midgard_src_expand_mode. */
#define MIDGARD_FLOAT_MOD_ABS (1 << 0)
#define MIDGARD_FLOAT_MOD_NEG (1 << 1)

/* The expand options depend on both midgard_int_mod and midgard_reg_mode.  For
 * example, a vec4 with midgard_int_sign_extend and midgard_src_expand_low is
 * treated as a vec8 and each 16-bit element from the low 64-bits is then sign
 * extended, resulting in a vec4 where each 32-bit element corresponds to a
 * 16-bit element from the low 64-bits of the input vector. */
typedef enum {
   midgard_src_passthrough = 0,
   midgard_src_rep_low = 1,     /* replicate lower 64 bits to higher 64 bits */
   midgard_src_rep_high = 2,    /* replicate higher 64 bits to lower 64 bits */
   midgard_src_swap = 3,        /* swap lower 64 bits with higher 64 bits */
   midgard_src_expand_low = 4,  /* expand low 64 bits */
   midgard_src_expand_high = 5, /* expand high 64 bits */
   midgard_src_expand_low_swap = 6,  /* expand low 64 bits, then swap */
   midgard_src_expand_high_swap = 7, /* expand high 64 bits, then swap */
} midgard_src_expand_mode;

#define INPUT_EXPANDS(a)                                                       \
   (a >= midgard_src_expand_low && a <= midgard_src_expand_high_swap)

#define INPUT_SWAPS(a)                                                         \
   (a == midgard_src_swap || a >= midgard_src_expand_low_swap)

typedef struct __attribute__((__packed__)) {
   /* Either midgard_int_mod or from midgard_float_mod_*, depending on the
    * type of op */
   unsigned mod                        : 2;
   midgard_src_expand_mode expand_mode : 3;
   unsigned swizzle                    : 8;
} midgard_vector_alu_src;

typedef struct __attribute__((__packed__)) {
   midgard_alu_op op               : 8;
   midgard_reg_mode reg_mode       : 2;
   unsigned src1                   : 13;
   unsigned src2                   : 13;
   midgard_shrink_mode shrink_mode : 2;
   unsigned outmod                 : 2;
   unsigned mask                   : 8;
} midgard_vector_alu;

typedef struct __attribute__((__packed__)) {
   unsigned mod       : 2;
   bool full          : 1; /* 0 = 16-bit, 1 = 32-bit */
   unsigned component : 3;
} midgard_scalar_alu_src;

typedef struct __attribute__((__packed__)) {
   midgard_alu_op op : 8;
   unsigned src1     : 6;
   /* last 5 bits are used when src2 is an immediate */
   unsigned src2             : 11;
   unsigned reserved         : 1;
   unsigned outmod           : 2;
   bool output_full          : 1;
   unsigned output_component : 3;
} midgard_scalar_alu;

typedef struct __attribute__((__packed__)) {
   unsigned src1_reg : 5;
   unsigned src2_reg : 5;
   unsigned out_reg  : 5;
   bool src2_imm     : 1;
} midgard_reg_info;

/* In addition to conditional branches and jumps (unconditional branches),
 * Midgard implements a bit of fixed function functionality used in fragment
 * shaders via specially crafted branches. These have special branch opcodes,
 * which perform a fixed-function operation and/or use the results of a
 * fixed-function operation as the branch condition.  */

typedef enum {
   /* Regular branches */
   midgard_jmp_writeout_op_branch_uncond = 1,
   midgard_jmp_writeout_op_branch_cond = 2,

   /* In a fragment shader, execute a discard_if instruction, with the
    * corresponding condition code. Terminates the shader, so generally
    * set the branch target to out of the shader */
   midgard_jmp_writeout_op_discard = 4,

   /* Branch if the tilebuffer is not yet ready. At the beginning of a
    * fragment shader that reads from the tile buffer, for instance via
    * ARM_shader_framebuffer_fetch or EXT_pixel_local_storage, this branch
    * operation should be used as a loop. An instruction like
    * "br.tilebuffer.always -1" does the trick, corresponding to
    * "while(!is_tilebuffer_ready) */
   midgard_jmp_writeout_op_tilebuffer_pending = 6,

   /* In a fragment shader, try to write out the value pushed to r0 to the
    * tilebuffer, subject to state in r1.z and r1.w. If this
    * succeeds, the shader terminates. If it fails, it branches to the
    * specified branch target. Generally, this should be used in a loop to
    * itself, acting as "do { write(r0); } while(!write_successful);" */
   midgard_jmp_writeout_op_writeout = 7,
} midgard_jmp_writeout_op;

typedef enum {
   midgard_condition_write0 = 0,

   /* These condition codes denote a conditional branch on FALSE and on
    * TRUE respectively */
   midgard_condition_false = 1,
   midgard_condition_true = 2,

   /* This condition code always branches. For a pure branch, the
    * unconditional branch coding should be used instead, but for
    * fixed-function branch opcodes, this is still useful */
   midgard_condition_always = 3,
} midgard_condition;

enum midgard_call_mode {
   midgard_call_mode_default = 1,
   midgard_call_mode_call = 2,
   midgard_call_mode_return = 3
};

typedef struct __attribute__((__packed__)) {
   midgard_jmp_writeout_op op       : 3; /* == branch_uncond */
   unsigned dest_tag                : 4; /* tag of branch destination */
   enum midgard_call_mode call_mode : 2;
   int offset                       : 7;
} midgard_branch_uncond;

typedef struct __attribute__((__packed__)) {
   midgard_jmp_writeout_op op : 3; /* == branch_cond */
   unsigned dest_tag          : 4; /* tag of branch destination */
   int offset                 : 7;
   midgard_condition cond     : 2;
} midgard_branch_cond;

typedef struct __attribute__((__packed__)) {
   midgard_jmp_writeout_op op       : 3; /* == branch_cond */
   unsigned dest_tag                : 4; /* tag of branch destination */
   enum midgard_call_mode call_mode : 2;
   signed offset                    : 23;

   /* Extended branches permit inputting up to 4 conditions loaded into
    * r31 (two in r31.w and two in r31.x). In the most general case, we
    * specify a function f(A, B, C, D) mapping 4 1-bit conditions to a
    * single 1-bit branch criteria. Note that the domain of f has 2^(2^4)
    * elements, each mapping to 1-bit of output, so we can trivially
    * construct a Godel numbering of f as a (2^4)=16-bit integer. This
    * 16-bit integer serves as a lookup table to compute f, subject to
    * some swaps for ordering.
    *
    * Interesting, the standard 2-bit condition codes are also a LUT with
    * the same format (2^1-bit), but it's usually easier to use enums. */

   unsigned cond : 16;
} midgard_branch_extended;

typedef struct __attribute__((__packed__)) {
   midgard_jmp_writeout_op op : 3; /* == writeout */
   unsigned unknown           : 13;
} midgard_writeout;

/*
 * Load/store words
 */

typedef enum {
   midgard_op_ld_st_noop = 0x03,

   /* Unpacks a colour from a native format to <format> */
   midgard_op_unpack_colour_f32 = 0x04,
   midgard_op_unpack_colour_f16 = 0x05,
   midgard_op_unpack_colour_u32 = 0x06,
   midgard_op_unpack_colour_s32 = 0x07,

   /* Packs a colour from <format> to a native format */
   midgard_op_pack_colour_f32 = 0x08,
   midgard_op_pack_colour_f16 = 0x09,
   midgard_op_pack_colour_u32 = 0x0A,
   midgard_op_pack_colour_s32 = 0x0B,

   /* Computes the effective address of a mem address expression */
   midgard_op_lea = 0x0C,

   /* Converts image coordinates into mem address */
   midgard_op_lea_image = 0x0D,

   /* Unclear why this is on the L/S unit, but moves fp32 cube map
    * coordinates in r27 to its cube map texture coordinate destination
    * (e.g r29). */

   midgard_op_ld_cubemap_coords = 0x0E,

   /* A mov between registers that the ldst pipeline can access */
   midgard_op_ldst_mov = 0x10,

   /* The L/S unit can do perspective division a clock faster than the ALU
    * if you're lucky. Put the vec4 in r27, and call with 0x24 as the
    * unknown state; the output will be <x/w, y/w, z/w, 1>. Replace w with
    * z for the z version */
   midgard_op_ldst_perspective_div_y = 0x11,
   midgard_op_ldst_perspective_div_z = 0x12,
   midgard_op_ldst_perspective_div_w = 0x13,

   /* val in r27.y, address embedded, outputs result to argument. Invert val for
      sub. Let val = +-1 for inc/dec. */
   midgard_op_atomic_add = 0x40,
   midgard_op_atomic_add64 = 0x41,
   midgard_op_atomic_add_be = 0x42,
   midgard_op_atomic_add64_be = 0x43,

   midgard_op_atomic_and = 0x44,
   midgard_op_atomic_and64 = 0x45,
   midgard_op_atomic_and_be = 0x46,
   midgard_op_atomic_and64_be = 0x47,
   midgard_op_atomic_or = 0x48,
   midgard_op_atomic_or64 = 0x49,
   midgard_op_atomic_or_be = 0x4A,
   midgard_op_atomic_or64_be = 0x4B,
   midgard_op_atomic_xor = 0x4C,
   midgard_op_atomic_xor64 = 0x4D,
   midgard_op_atomic_xor_be = 0x4E,
   midgard_op_atomic_xor64_be = 0x4F,

   midgard_op_atomic_imin = 0x50,
   midgard_op_atomic_imin64 = 0x51,
   midgard_op_atomic_imin_be = 0x52,
   midgard_op_atomic_imin64_be = 0x53,
   midgard_op_atomic_umin = 0x54,
   midgard_op_atomic_umin64 = 0x55,
   midgard_op_atomic_umin_be = 0x56,
   midgard_op_atomic_umin64_be = 0x57,
   midgard_op_atomic_imax = 0x58,
   midgard_op_atomic_imax64 = 0x59,
   midgard_op_atomic_imax_be = 0x5A,
   midgard_op_atomic_imax64_be = 0x5B,
   midgard_op_atomic_umax = 0x5C,
   midgard_op_atomic_umax64 = 0x5D,
   midgard_op_atomic_umax_be = 0x5E,
   midgard_op_atomic_umax64_be = 0x5F,

   midgard_op_atomic_xchg = 0x60,
   midgard_op_atomic_xchg64 = 0x61,
   midgard_op_atomic_xchg_be = 0x62,
   midgard_op_atomic_xchg64_be = 0x63,

   midgard_op_atomic_cmpxchg = 0x64,
   midgard_op_atomic_cmpxchg64 = 0x65,
   midgard_op_atomic_cmpxchg_be = 0x66,
   midgard_op_atomic_cmpxchg64_be = 0x67,

   /* Used for compute shader's __global arguments, __local
    * variables (or for register spilling) */

   midgard_op_ld_u8 = 0x80,         /* zero extends */
   midgard_op_ld_i8 = 0x81,         /* sign extends */
   midgard_op_ld_u16 = 0x84,        /* zero extends */
   midgard_op_ld_i16 = 0x85,        /* sign extends */
   midgard_op_ld_u16_be = 0x86,     /* zero extends, big endian */
   midgard_op_ld_i16_be = 0x87,     /* sign extends, big endian */
   midgard_op_ld_32 = 0x88,         /* short2, int, float */
   midgard_op_ld_32_bswap2 = 0x89,  /* 16-bit big endian vector */
   midgard_op_ld_32_bswap4 = 0x8A,  /* 32-bit big endian scalar */
   midgard_op_ld_64 = 0x8C,         /* int2, float2, long */
   midgard_op_ld_64_bswap2 = 0x8D,  /* 16-bit big endian vector */
   midgard_op_ld_64_bswap4 = 0x8E,  /* 32-bit big endian vector */
   midgard_op_ld_64_bswap8 = 0x8F,  /* 64-bit big endian scalar */
   midgard_op_ld_128 = 0x90,        /* float4, long2 */
   midgard_op_ld_128_bswap2 = 0x91, /* 16-bit big endian vector */
   midgard_op_ld_128_bswap4 = 0x92, /* 32-bit big endian vector */
   midgard_op_ld_128_bswap8 = 0x93, /* 64-bit big endian vector */

   midgard_op_ld_attr_32 = 0x94,
   midgard_op_ld_attr_16 = 0x95,
   midgard_op_ld_attr_32u = 0x96,
   midgard_op_ld_attr_32i = 0x97,
   midgard_op_ld_vary_32 = 0x98,
   midgard_op_ld_vary_16 = 0x99,
   midgard_op_ld_vary_32u = 0x9A,
   midgard_op_ld_vary_32i = 0x9B,

   /* This instruction behaves differently depending if the gpu is a v4
    * or a newer gpu. The main difference hinges on which values of the
    * second argument are valid for each gpu.
    * TODO: properly document and decode each possible value for the
    * second argument. */
   midgard_op_ld_special_32f = 0x9C,
   midgard_op_ld_special_16f = 0x9D,
   midgard_op_ld_special_32u = 0x9E,
   midgard_op_ld_special_32i = 0x9F,

   /* The distinction between these ops is the alignment
    * requirement / accompanying shift. Thus, the offset to
    * ld_ubo_128 is in 16-byte units and can load 128-bit. The
    * offset to ld_ubo_64 is in 8-byte units; ld_ubo_32 in 4-byte
    * units. */
   midgard_op_ld_ubo_u8 = 0xA0,     /* theoretical */
   midgard_op_ld_ubo_i8 = 0xA1,     /* theoretical */
   midgard_op_ld_ubo_u16 = 0xA4,    /* theoretical */
   midgard_op_ld_ubo_i16 = 0xA5,    /* theoretical */
   midgard_op_ld_ubo_u16_be = 0xA6, /* theoretical */
   midgard_op_ld_ubo_i16_be = 0xA7, /* theoretical */
   midgard_op_ld_ubo_32 = 0xA8,
   midgard_op_ld_ubo_32_bswap2 = 0xA9,
   midgard_op_ld_ubo_32_bswap4 = 0xAA,
   midgard_op_ld_ubo_64 = 0xAC,
   midgard_op_ld_ubo_64_bswap2 = 0xAD,
   midgard_op_ld_ubo_64_bswap4 = 0xAE,
   midgard_op_ld_ubo_64_bswap8 = 0xAF,
   midgard_op_ld_ubo_128 = 0xB0,
   midgard_op_ld_ubo_128_bswap2 = 0xB1,
   midgard_op_ld_ubo_128_bswap4 = 0xB2,
   midgard_op_ld_ubo_128_bswap8 = 0xB3,

   midgard_op_ld_image_32f = 0xB4,
   midgard_op_ld_image_16f = 0xB5,
   midgard_op_ld_image_32u = 0xB6,
   midgard_op_ld_image_32i = 0xB7,

   /* Only works on v5 or newer.
    * Older cards must use ld_special with tilebuffer selectors. */
   midgard_op_ld_tilebuffer_32f = 0xB8,
   midgard_op_ld_tilebuffer_16f = 0xB9,
   midgard_op_ld_tilebuffer_raw = 0xBA,

   midgard_op_st_u8 = 0xC0,         /* zero extends */
   midgard_op_st_i8 = 0xC1,         /* sign extends */
   midgard_op_st_u16 = 0xC4,        /* zero extends */
   midgard_op_st_i16 = 0xC5,        /* sign extends */
   midgard_op_st_u16_be = 0xC6,     /* zero extends, big endian */
   midgard_op_st_i16_be = 0xC7,     /* sign extends, big endian */
   midgard_op_st_32 = 0xC8,         /* short2, int, float */
   midgard_op_st_32_bswap2 = 0xC9,  /* 16-bit big endian vector */
   midgard_op_st_32_bswap4 = 0xCA,  /* 32-bit big endian scalar */
   midgard_op_st_64 = 0xCC,         /* int2, float2, long */
   midgard_op_st_64_bswap2 = 0xCD,  /* 16-bit big endian vector */
   midgard_op_st_64_bswap4 = 0xCE,  /* 32-bit big endian vector */
   midgard_op_st_64_bswap8 = 0xCF,  /* 64-bit big endian scalar */
   midgard_op_st_128 = 0xD0,        /* float4, long2 */
   midgard_op_st_128_bswap2 = 0xD1, /* 16-bit big endian vector */
   midgard_op_st_128_bswap4 = 0xD2, /* 32-bit big endian vector */
   midgard_op_st_128_bswap8 = 0xD3, /* 64-bit big endian vector */

   midgard_op_st_vary_32 = 0xD4,
   midgard_op_st_vary_16 = 0xD5,
   midgard_op_st_vary_32u = 0xD6,
   midgard_op_st_vary_32i = 0xD7,

   /* Value to st in r27, location r26.w as short2 */
   midgard_op_st_image_32f = 0xD8,
   midgard_op_st_image_16f = 0xD9,
   midgard_op_st_image_32u = 0xDA,
   midgard_op_st_image_32i = 0xDB,

   midgard_op_st_special_32f = 0xDC,
   midgard_op_st_special_16f = 0xDD,
   midgard_op_st_special_32u = 0xDE,
   midgard_op_st_special_32i = 0xDF,

   /* Only works on v5 or newer.
    * Older cards must use ld_special with tilebuffer selectors. */
   midgard_op_st_tilebuffer_32f = 0xE8,
   midgard_op_st_tilebuffer_16f = 0xE9,
   midgard_op_st_tilebuffer_raw = 0xEA,
   midgard_op_trap = 0xFC,
} midgard_load_store_op;

typedef enum {
   midgard_interp_sample = 0,
   midgard_interp_centroid = 1,
   midgard_interp_default = 2
} midgard_interpolation;

typedef enum {
   midgard_varying_mod_none = 0,

   /* Take the would-be result and divide all components by its y/z/w
    * (perspective division baked in with the load)  */
   midgard_varying_mod_perspective_y = 1,
   midgard_varying_mod_perspective_z = 2,
   midgard_varying_mod_perspective_w = 3,

   /* The result is a 64-bit cubemap descriptor to use with
    * midgard_tex_op_normal or midgard_tex_op_gradient */
   midgard_varying_mod_cubemap = 4,
} midgard_varying_modifier;

typedef struct __attribute__((__packed__)) {
   midgard_varying_modifier modifier : 3;

   bool flat_shading : 1;

   /* These are ignored if flat_shading is enabled. */
   bool perspective_correction : 1;
   bool centroid_mapping       : 1;

   /* This is ignored if the shader only runs once per pixel. */
   bool interpolate_sample : 1;

   bool zero0 : 1; /* Always zero */

   unsigned direct_sample_pos_x : 4;
   unsigned direct_sample_pos_y : 4;
} midgard_varying_params;

/* 8-bit register/etc selector for load/store ops */
typedef struct __attribute__((__packed__)) {
   /* Indexes into the register */
   unsigned component : 2;

   /* Register select between r26/r27 */
   unsigned select : 1;

   unsigned unknown : 2;

   /* Like any good Arm instruction set, load/store arguments can be
    * implicitly left-shifted... but only the second argument. Zero for no
    * shifting, up to <<7 possible though. This is useful for indexing.
    *
    * For the first argument, it's unknown what these bits mean */
   unsigned shift : 3;
} midgard_ldst_register_select;

typedef enum {
   /* 0 is reserved */
   midgard_index_address_u64 = 1,
   midgard_index_address_u32 = 2,
   midgard_index_address_s32 = 3,
} midgard_index_address_format;

typedef struct __attribute__((__packed__)) {
   midgard_load_store_op op : 8;

   /* Source/dest reg */
   unsigned reg : 5;

   /* Generally is a writemask.
    * For ST_ATTR and ST_TEX, unused.
    * For other stores, each bit masks 1/4th of the output. */
   unsigned mask : 4;

   /* Swizzle for stores, but for atomics it encodes also the source
    * register. This fits because atomics dont need a swizzle since they
    * are not vectorized instructions. */
   unsigned swizzle : 8;

   /* Arg reg, meaning changes according to each opcode */
   unsigned arg_comp : 2;
   unsigned arg_reg  : 3;

   /* 64-bit address enable
    * 32-bit data type enable for CUBEMAP and perspective div.
    * Explicit indexing enable for LD_ATTR.
    * 64-bit coordinate enable for LD_IMAGE. */
   bool bitsize_toggle : 1;

   /* These are mainly used for opcodes that have addresses.
    * For cmpxchg, index_reg is used for the comparison value.
    * For ops that access the attrib table, bit 1 encodes which table.
    * For LD_VAR and LD/ST_ATTR, bit 0 enables dest/src type inferral. */
   midgard_index_address_format index_format : 2;
   unsigned index_comp                       : 2;
   unsigned index_reg                        : 3;
   unsigned index_shift                      : 4;

   /* Generaly is a signed offset, but has different bitsize and starts at
    * different bits depending on the opcode, LDST_*_DISPLACEMENT helpers
    * are recommended when packing/unpacking this attribute.
    * For LD_UBO, bit 0 enables ubo index immediate.
    * For LD_TILEBUFFER_RAW, bit 0 disables sample index immediate. */
   int signed_offset : 18;
} midgard_load_store_word;

typedef struct __attribute__((__packed__)) {
   unsigned type      : 4;
   unsigned next_type : 4;
   uint64_t word1     : 60;
   uint64_t word2     : 60;
} midgard_load_store;

/* 8-bit register selector used in texture ops to select a bias/LOD/gradient
 * register, shoved into the `bias` field */

typedef struct __attribute__((__packed__)) {
   /* 32-bit register, clear for half-register */
   unsigned full : 1;

   /* Register select between r28/r29 */
   unsigned select : 1;

   /* For a half-register, selects the upper half */
   unsigned upper : 1;

   /* Indexes into the register */
   unsigned component : 2;

   /* Padding to make this 8-bit */
   unsigned zero : 3;
} midgard_tex_register_select;

/* Texture pipeline results are in r28-r29 */
#define REG_TEX_BASE 28

enum mali_texture_op {
   /* [texture + LOD bias]
    * If the texture is mipmapped, barriers must be enabled in the
    * instruction word in order for this opcode to compute the output
    * correctly. */
   midgard_tex_op_normal = 1,

   /* [texture + gradient for LOD and anisotropy]
    * Unlike midgard_tex_op_normal, this opcode does not require barriers
    * to compute the output correctly. */
   midgard_tex_op_gradient = 2,

   /* [unfiltered texturing]
    * Unlike midgard_tex_op_normal, this opcode does not require barriers
    * to compute the output correctly. */
   midgard_tex_op_fetch = 4,

   /* [gradient from derivative] */
   midgard_tex_op_grad_from_derivative = 9,

   /* [mov] */
   midgard_tex_op_mov = 10,

   /* [noop]
    * Mostly used for barriers. */
   midgard_tex_op_barrier = 11,

   /* [gradient from coords] */
   midgard_tex_op_grad_from_coords = 12,

   /* [derivative]
    * Computes derivatives in 2x2 fragment blocks. */
   midgard_tex_op_derivative = 13
};

enum mali_sampler_type {
   /* 0 is reserved */
   MALI_SAMPLER_FLOAT = 0x1,    /* sampler */
   MALI_SAMPLER_UNSIGNED = 0x2, /* usampler */
   MALI_SAMPLER_SIGNED = 0x3,   /* isampler */
};

/* Texture modes */
enum mali_texture_mode {
   TEXTURE_NORMAL = 1,
   TEXTURE_SHADOW = 5,
   TEXTURE_GATHER_SHADOW = 6,
   TEXTURE_GATHER_X = 8,
   TEXTURE_GATHER_Y = 9,
   TEXTURE_GATHER_Z = 10,
   TEXTURE_GATHER_W = 11,
};

enum mali_derivative_mode {
   TEXTURE_DFDX = 0,
   TEXTURE_DFDY = 1,
};

enum midgard_partial_execution {
   MIDGARD_PARTIAL_EXECUTION_SKIP = 1,
   MIDGARD_PARTIAL_EXECUTION_KILL = 2,
   MIDGARD_PARTIAL_EXECUTION_NONE = 3
};

typedef struct __attribute__((__packed__)) {
   unsigned type      : 4;
   unsigned next_type : 4;

   enum mali_texture_op op             : 4;
   unsigned mode                       : 4;
   enum midgard_partial_execution exec : 2;

   unsigned format : 2;

   /* Are sampler_handle/texture_handler respectively set by registers? If
    * true, the lower 8-bits of the respective field is a register word.
    * If false, they are an immediate */

   unsigned sampler_register : 1;
   unsigned texture_register : 1;

   /* Is a register used to specify the
    * LOD/bias/offset? If set, use the `bias` field as
    * a register index. If clear, use the `bias` field
    * as an immediate. */
   unsigned lod_register : 1;

   /* Is a register used to specify an offset? If set, use the
    * offset_reg_* fields to encode this, duplicated for each of the
    * components. If clear, there is implcitly always an immediate offst
    * specificed in offset_imm_* */
   unsigned offset_register : 1;

   unsigned in_reg_full    : 1;
   unsigned in_reg_select  : 1;
   unsigned in_reg_upper   : 1;
   unsigned in_reg_swizzle : 8;

   unsigned unknown8 : 2;

   unsigned out_full : 1;

   enum mali_sampler_type sampler_type : 2;

   unsigned out_reg_select : 1;
   unsigned out_upper      : 1;

   unsigned mask : 4;

   /* Intriguingly, textures can take an outmod just like alu ops. Int
    * outmods are not supported as far as I can tell, so this is only
    * meaningful for float samplers */
   midgard_outmod_float outmod : 2;

   unsigned swizzle : 8;

   /* These indicate how many bundles after this texture op may be
    * executed in parallel with this op. We may execute only ALU and
    * ld/st in parallel (not other textures), and obviously there cannot
    * be any dependency (the blob appears to forbid even accessing other
    * channels of a given texture register). */

   unsigned out_of_order : 4;
   unsigned unknown4     : 8;

   /* In immediate mode, each offset field is an immediate range [0, 7].
    *
    * In register mode, offset_x becomes a register (full, select, upper)
    * triplet followed by a vec3 swizzle is splattered across
    * offset_y/offset_z in a genuinely bizarre way.
    *
    * For texel fetches in immediate mode, the range is the full [-8, 7],
    * but for normal texturing the top bit must be zero and a register
    * used instead. It's not clear where this limitation is from.
    *
    * union {
    *      struct {
    *              signed offset_x  : 4;
    *              signed offset_y  : 4;
    *              signed offset_z  : 4;
    *      } immediate;
    *      struct {
    *              bool full        : 1;
    *              bool select      : 1;
    *              bool upper       : 1;
    *              unsigned swizzle : 8;
    *              unsigned zero    : 1;
    *      } register;
    * }
    */

   unsigned offset : 12;

   /* In immediate bias mode, for a normal texture op, this is
    * texture bias, computed as int(2^8 * frac(biasf)), with
    * bias_int = floor(bias). For a textureLod, it's that, but
    * s/bias/lod. For a texel fetch, this is the LOD as-is.
    *
    * In register mode, this is a midgard_tex_register_select
    * structure and bias_int is zero */

   unsigned bias   : 8;
   signed bias_int : 8;

   /* If sampler/texture_register is set, the bottom 8-bits are
    * midgard_tex_register_select and the top 8-bits are zero. If they are
    * clear, they are immediate texture indices */

   unsigned sampler_handle : 16;
   unsigned texture_handle : 16;
} midgard_texture_word;

/* Technically barriers are texture instructions but it's less work to add them
 * as an explicitly zeroed special case, since most fields are forced to go to
 * zero */

typedef struct __attribute__((__packed__)) {
   unsigned type      : 4;
   unsigned next_type : 4;

   /* op = TEXTURE_OP_BARRIER */
   unsigned op    : 6;
   unsigned zero1 : 2;

   /* Since helper invocations don't make any sense, these are forced to one */
   unsigned cont  : 1;
   unsigned last  : 1;
   unsigned zero2 : 14;

   unsigned zero3        : 24;
   unsigned out_of_order : 4;
   unsigned zero4        : 4;

   uint64_t zero5;
} midgard_texture_barrier_word;

typedef union midgard_constants {
   double f64[2];
   uint64_t u64[2];
   int64_t i64[2];
   float f32[4];
   uint32_t u32[4];
   int32_t i32[4];
   uint16_t f16[8];
   uint16_t u16[8];
   int16_t i16[8];
   uint8_t u8[16];
   int8_t i8[16];
} midgard_constants;

enum midgard_roundmode {
   MIDGARD_RTE = 0x0, /* round to even */
   MIDGARD_RTZ = 0x1, /* round to zero */
   MIDGARD_RTN = 0x2, /* round to negative */
   MIDGARD_RTP = 0x3, /* round to positive */
};

#endif
