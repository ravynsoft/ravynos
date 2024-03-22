/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "rogue.h"

/**
 * \file rogue_info.c
 *
 * \brief Contains information and definitions for defined types and structures.
 */

/* TODO: Adjust according to core configurations. */
/* TODO: Remaining restrictions, e.g. some registers are only
 * usable by a particular instruction (vertex output) etc. */
#define S(n) BITFIELD64_BIT(ROGUE_IO_S##n - 1)
const rogue_reg_info rogue_reg_infos[ROGUE_REG_CLASS_COUNT] = {
   [ROGUE_REG_CLASS_INVALID] = { .name = "!INVALID!", .str = "!INVALID!", },
   [ROGUE_REG_CLASS_SSA] = { .name = "ssa", .str = "R", },
   [ROGUE_REG_CLASS_TEMP] = { .name = "temp", .str = "r", .num = 248, },
   [ROGUE_REG_CLASS_COEFF] = { .name = "coeff", .str = "cf", .num = 4096, .supported_io_srcs = S(0) | S(2) | S(3), },
   [ROGUE_REG_CLASS_SHARED] = { .name = "shared", .str = "sh", .num = 4096, .supported_io_srcs = S(0) | S(2) | S(3), },
   [ROGUE_REG_CLASS_SPECIAL] = { .name = "special", .str = "sr", .num = 240, }, /* TODO NEXT: Only S1, S2, S4. */
   [ROGUE_REG_CLASS_INTERNAL] = { .name = "internal", .str = "i", .num = 8, },
   [ROGUE_REG_CLASS_CONST] = { .name = "const", .str = "sc", .num = 240, },
   [ROGUE_REG_CLASS_PIXOUT] = { .name = "pixout", .str = "po", .num = 8, .supported_io_srcs = S(0) | S(2) | S(3), },
   [ROGUE_REG_CLASS_VTXIN] = { .name = "vtxin", .str = "vi", .num = 248, },
   [ROGUE_REG_CLASS_VTXOUT] = { .name = "vtxout", .str = "vo", .num = 256, },
};
#undef S

const rogue_regalloc_info regalloc_info[ROGUE_REGALLOC_CLASS_COUNT] = {
   [ROGUE_REGALLOC_CLASS_TEMP_1] = { .class = ROGUE_REG_CLASS_TEMP, .stride = 1, },
   [ROGUE_REGALLOC_CLASS_TEMP_2] = { .class = ROGUE_REG_CLASS_TEMP, .stride = 2, },
   [ROGUE_REGALLOC_CLASS_TEMP_4] = { .class = ROGUE_REG_CLASS_TEMP, .stride = 4, },
};

const rogue_reg_dst_info rogue_reg_dst_infos[ROGUE_REG_DST_VARIANTS] = {
   {
      .num_dsts = 1,
      .bank_bits = { 1 },
      .index_bits = { 6 },
      .bytes = 1,
   },
   {
      .num_dsts = 1,
      .bank_bits = { 3 },
      .index_bits = { 11 },
      .bytes = 2,
   },
   {
      .num_dsts = 2,
      .bank_bits = { 1, 1 },
      .index_bits = { 7, 6 },
      .bytes = 2,
   },
   {
      .num_dsts = 2,
      .bank_bits = { 3, 3 },
      .index_bits = { 8, 8 },
      .bytes = 3,
   },
   {
      .num_dsts = 2,
      .bank_bits = { 3, 3 },
      .index_bits = { 11, 11 },
      .bytes = 4,
   },
};

const rogue_reg_src_info rogue_reg_lower_src_infos[ROGUE_REG_SRC_VARIANTS] = {
   {
      .num_srcs = 1,
      .mux_bits = 0,
      .bank_bits = { 1 },
      .index_bits = { 6 },
      .bytes = 1,
   },
   {
      .num_srcs = 1,
      .mux_bits = 2,
      .bank_bits = { 3 },
      .index_bits = { 11 },
      .bytes = 3,
   },
   {
      .num_srcs = 2,
      .mux_bits = 0,
      .bank_bits = { 1, 1 },
      .index_bits = { 6, 5 },
      .bytes = 2,
   },
   {
      .num_srcs = 2,
      .mux_bits = 2,
      .bank_bits = { 2, 2 },
      .index_bits = { 7, 7 },
      .bytes = 3,
   },
   {
      .num_srcs = 2,
      .mux_bits = 3,
      .bank_bits = { 3, 2 },
      .index_bits = { 11, 8 },
      .bytes = 4,
   },
   {
      .num_srcs = 3,
      .mux_bits = 2,
      .bank_bits = { 2, 2, 2 },
      .index_bits = { 7, 7, 6 },
      .bytes = 4,
   },
   {
      .num_srcs = 3,
      .mux_bits = 3,
      .bank_bits = { 3, 2, 3 },
      .index_bits = { 8, 8, 8 },
      .bytes = 5,
   },
   {
      .num_srcs = 3,
      .mux_bits = 3,
      .bank_bits = { 3, 2, 3 },
      .index_bits = { 11, 8, 11 },
      .bytes = 6,
   },
};

const rogue_reg_src_info rogue_reg_upper_src_infos[ROGUE_REG_SRC_VARIANTS] = {
   {
      .num_srcs = 1,
      .bank_bits = { 1 },
      .index_bits = { 6 },
      .bytes = 1,
   },
   {
      .num_srcs = 1,
      .bank_bits = { 3 },
      .index_bits = { 11 },
      .bytes = 3,
   },
   {
      .num_srcs = 2,
      .bank_bits = { 1, 1 },
      .index_bits = { 6, 5 },
      .bytes = 2,
   },
   {
      .num_srcs = 2,
      .bank_bits = { 2, 2 },
      .index_bits = { 7, 7 },
      .bytes = 3,
   },
   {
      .num_srcs = 2,
      .bank_bits = { 3, 2 },
      .index_bits = { 11, 8 },
      .bytes = 4,
   },
   {
      .num_srcs = 3,
      .bank_bits = { 2, 2, 2 },
      .index_bits = { 7, 7, 6 },
      .bytes = 4,
   },
   {
      .num_srcs = 3,
      .bank_bits = { 3, 2, 2 },
      .index_bits = { 8, 8, 8 },
      .bytes = 5,
   },
   {
      .num_srcs = 3,
      .bank_bits = { 3, 2, 2 },
      .index_bits = { 11, 8, 8 },
      .bytes = 6,
   },
};

#define OM(op_mod) BITFIELD64_BIT(ROGUE_ALU_OP_MOD_##op_mod)
const rogue_alu_op_mod_info rogue_alu_op_mod_infos[ROGUE_ALU_OP_MOD_COUNT] = {
   [ROGUE_ALU_OP_MOD_LP] = { .str = "lp", },
   [ROGUE_ALU_OP_MOD_SAT] = { .str = "sat", },
   [ROGUE_ALU_OP_MOD_SCALE] = { .str = "scale", },
   [ROGUE_ALU_OP_MOD_ROUNDZERO] = { .str = "roundzero", },

   [ROGUE_ALU_OP_MOD_Z] = { .str = "z", .exclude = OM(GZ) | OM(GEZ) | OM(C) | OM(E) | OM(G) | OM(GE) | OM(NE) | OM(L) | OM(LE) },
   [ROGUE_ALU_OP_MOD_GZ] = { .str = "gz", .exclude = OM(Z) | OM(GEZ) | OM(C) | OM(E) | OM(G) | OM(GE) | OM(NE) | OM(L) | OM(LE) },
   [ROGUE_ALU_OP_MOD_GEZ] = { .str = "gez", .exclude = OM(Z) | OM(GZ) | OM(C) | OM(E) | OM(G) | OM(GE) | OM(NE) | OM(L) | OM(LE) },
   [ROGUE_ALU_OP_MOD_C] = { .str = "c", .exclude = OM(Z) | OM(GZ) | OM(GEZ) | OM(E) | OM(G) | OM(GE) | OM(NE) | OM(L) | OM(LE) },
   [ROGUE_ALU_OP_MOD_E] = { .str = "e", .exclude = OM(Z) | OM(GZ) | OM(GEZ) | OM(C) | OM(G) | OM(GE) | OM(NE) | OM(L) | OM(LE) },
   [ROGUE_ALU_OP_MOD_G] = { .str = "g", .exclude = OM(Z) | OM(GZ) | OM(GEZ) | OM(C) | OM(E) | OM(GE) | OM(NE) | OM(L) | OM(LE) },
   [ROGUE_ALU_OP_MOD_GE] = { .str = "ge", .exclude = OM(Z) | OM(GZ) | OM(GEZ) | OM(C) | OM(E) | OM(G) | OM(NE) | OM(L) | OM(LE) },
   [ROGUE_ALU_OP_MOD_NE] = { .str = "ne", .exclude = OM(Z) | OM(GZ) | OM(GEZ) | OM(C) | OM(E) | OM(G) | OM(GE) | OM(L) | OM(LE) },
   [ROGUE_ALU_OP_MOD_L] = { .str = "l", .exclude = OM(Z) | OM(GZ) | OM(GEZ) | OM(C) | OM(E) | OM(G) | OM(GE) | OM(NE) | OM(LE) },
   [ROGUE_ALU_OP_MOD_LE] = { .str = "le", .exclude = OM(Z) | OM(GZ) | OM(GEZ) | OM(C) | OM(E) | OM(G) | OM(GE) | OM(NE) | OM(L) },

   [ROGUE_ALU_OP_MOD_F32] = { .str = "f32", .exclude = OM(U16) | OM(S16) | OM(U8) | OM(S8) | OM(U32) | OM(S32) },
   [ROGUE_ALU_OP_MOD_U16] = { .str = "u16", .exclude = OM(F32) | OM(S16) | OM(U8) | OM(S8) | OM(U32) | OM(S32) },
   [ROGUE_ALU_OP_MOD_S16] = { .str = "s16", .exclude = OM(F32) | OM(U16) | OM(U8) | OM(S8) | OM(U32) | OM(S32) },
   [ROGUE_ALU_OP_MOD_U8] = { .str = "u8", .exclude = OM(F32) | OM(U16) | OM(S16) | OM(S8) | OM(U32) | OM(S32) },
   [ROGUE_ALU_OP_MOD_S8] = { .str = "s8", .exclude = OM(F32) | OM(U16) | OM(S16) | OM(U8) | OM(U32) | OM(S32) },
   [ROGUE_ALU_OP_MOD_U32] = { .str = "u32", .exclude = OM(F32) | OM(U16) | OM(S16) | OM(U8) | OM(S8) | OM(S32) },
   [ROGUE_ALU_OP_MOD_S32] = { .str = "s32", .exclude = OM(F32) | OM(U16) | OM(S16) | OM(U8) | OM(S8) | OM(U32) },
};
#undef OM

const rogue_alu_dst_mod_info rogue_alu_dst_mod_infos[ROGUE_ALU_DST_MOD_COUNT] = {
   [ROGUE_ALU_DST_MOD_E0] = { .str = "e0", },
   [ROGUE_ALU_DST_MOD_E1] = { .str = "e1", },
   [ROGUE_ALU_DST_MOD_E2] = { .str = "e2", },
   [ROGUE_ALU_DST_MOD_E3] = { .str = "e3", },
};

const rogue_alu_src_mod_info rogue_alu_src_mod_infos[ROGUE_ALU_SRC_MOD_COUNT] = {
   [ROGUE_ALU_SRC_MOD_FLR] = { .str = "flr", },
   [ROGUE_ALU_SRC_MOD_ABS] = { .str = "abs", },
   [ROGUE_ALU_SRC_MOD_NEG] = { .str = "neg", },
   [ROGUE_ALU_SRC_MOD_E0] = { .str = "e0", },
   [ROGUE_ALU_SRC_MOD_E1] = { .str = "e1", },
   [ROGUE_ALU_SRC_MOD_E2] = { .str = "e2", },
   [ROGUE_ALU_SRC_MOD_E3] = { .str = "e3", },
};

#define OM(op_mod) BITFIELD64_BIT(ROGUE_CTRL_OP_MOD_##op_mod)
const rogue_ctrl_op_mod_info rogue_ctrl_op_mod_infos[ROGUE_CTRL_OP_MOD_COUNT] = {
   [ROGUE_CTRL_OP_MOD_LINK] = { .str = "link", },
   [ROGUE_CTRL_OP_MOD_ALLINST] = { .str = "allinst", .exclude = OM(ANYINST) },
   [ROGUE_CTRL_OP_MOD_ANYINST] = { .str = "anyinst", .exclude = OM(ALLINST) },
   [ROGUE_CTRL_OP_MOD_END] = { .str = "end", },
};
#undef OM

#define OM(op_mod) BITFIELD64_BIT(ROGUE_CTRL_OP_MOD_##op_mod)
#define T(type) BITFIELD64_BIT(ROGUE_REF_TYPE_##type - 1)
const rogue_ctrl_op_info rogue_ctrl_op_infos[ROGUE_CTRL_OP_COUNT] = {
	[ROGUE_CTRL_OP_INVALID] = { .str = "!INVALID!", },
	[ROGUE_CTRL_OP_END] = { .str = "end", .ends_block = true, },
	[ROGUE_CTRL_OP_NOP] = { .str = "nop",
		.supported_op_mods = OM(END),
	},
	[ROGUE_CTRL_OP_WOP] = { .str = "wop", },
	[ROGUE_CTRL_OP_BR] = { .str = "br", .has_target = true, .ends_block = true,
		.supported_op_mods = OM(LINK) | OM(ALLINST) | OM(ANYINST),
   },
	[ROGUE_CTRL_OP_BA] = { .str = "ba", .ends_block = true, .num_srcs = 1,
		.supported_op_mods = OM(LINK) | OM(ALLINST) | OM(ANYINST),
      .supported_src_types = { [0] = T(VAL), },
   },
	[ROGUE_CTRL_OP_WDF] = { .str = "wdf", .num_srcs = 1,
      .supported_src_types = { [0] = T(DRC), },
   },
};
#undef T
#undef OM

#define IO(io) ROGUE_IO_##io
#define OM(op_mod) BITFIELD64_BIT(ROGUE_BACKEND_OP_MOD_##op_mod)
#define T(type) BITFIELD64_BIT(ROGUE_REF_TYPE_##type - 1)
#define B(n) BITFIELD64_BIT(n)
const rogue_backend_op_info rogue_backend_op_infos[ROGUE_BACKEND_OP_COUNT] = {
	[ROGUE_BACKEND_OP_INVALID] = { .str = "!INVALID!", },
   [ROGUE_BACKEND_OP_UVSW_WRITE] = { .str = "uvsw.write", .num_dsts = 1, .num_srcs = 1,
      .phase_io = { .src[0] = IO(W0), },
      .supported_dst_types = { [0] = T(REG), },
      .supported_src_types = { [0] = T(REG), },
   },
   [ROGUE_BACKEND_OP_UVSW_EMIT] = { .str = "uvsw.emit", },
   [ROGUE_BACKEND_OP_UVSW_ENDTASK] = { .str = "uvsw.endtask", },

   [ROGUE_BACKEND_OP_UVSW_EMITTHENENDTASK] = { .str = "uvsw.emitthenendtask", },
   [ROGUE_BACKEND_OP_UVSW_WRITETHENEMITTHENENDTASK] = { .str = "uvsw.writethenemitthenendtask", .num_dsts = 1, .num_srcs = 1,
      .phase_io = { .src[0] = IO(W0), },
      .supported_dst_types = { [0] = T(REG), },
      .supported_src_types = { [0] = T(REG), },
   },
   [ROGUE_BACKEND_OP_IDF] = { .str = "idf", .num_srcs = 2,
      .phase_io = { .src[1] = IO(S0), },
      .supported_src_types = { [0] = T(DRC), [1] = T(REGARRAY), },
      .src_stride = {
         [1] = 1,
      },
   },

   [ROGUE_BACKEND_OP_EMITPIX] = { .str = "emitpix", .num_srcs = 2,
      .phase_io = { .src[0] = IO(S0), .src[1] = IO(S2), },
      .supported_op_mods = OM(FREEP),
      .supported_src_types = { [0] = T(REG), [1] = T(REG), },
   },
   /* .src[1] and .src[2] can actually be S0-5. */
   [ROGUE_BACKEND_OP_LD] = { .str = "ld", .num_dsts = 1, .num_srcs = 3,
      .phase_io = { .dst[0] = IO(S3), .src[2] = IO(S0), },
      .supported_op_mods = OM(BYPASS) | OM(FORCELINEFILL) | OM(SLCBYPASS) | OM(SLCNOALLOC),
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY), },
      .supported_src_types = {
         [0] = T(DRC),
         [1] = T(VAL),
         [2] = T(REGARRAY),
      },
      .dst_stride = {
         [0] = ~0U,
      },
      .src_stride = {
         [2] = 1,
      },
   },
   /* .src[0] and .src[4] can actually be S0-5. */
   [ROGUE_BACKEND_OP_ST] = { .str = "st", .num_srcs = 6,
      .phase_io = { .src[0] = IO(S3), .src[4] = IO(S0), },
      .supported_op_mods = OM(TILED) | OM(WRITETHROUGH) | OM(WRITEBACK) | OM(LAZYWRITEBACK) |
         OM(SLCBYPASS) | OM(SLCWRITEBACK) | OM(SLCWRITETHROUGH) | OM(SLCNOALLOC),
      .supported_src_types = {
         [0] = T(REG) | T(REGARRAY),
         [1] = T(VAL),
         [2] = T(DRC),
         [3] = T(VAL),
         [4] = T(REGARRAY),
         [5] = T(IO),
      },
      .src_stride = {
         [4] = 1,
      },
   },
	[ROGUE_BACKEND_OP_FITR_PIXEL] = { .str = "fitr.pixel", .num_dsts = 1, .num_srcs = 3,
      .phase_io = { .dst[0] = IO(S3), .src[1] = IO(S0), },
      .supported_op_mods = OM(SAT),
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY), },
      .supported_src_types = {
         [0] = T(DRC),
         [1] = T(REGARRAY),
         [2] = T(VAL),
      },
      .dst_stride = {
         [0] = ~0U,
      },
      .src_stride = {
         [1] = ~0U,
      },
   },
	[ROGUE_BACKEND_OP_FITRP_PIXEL] = { .str = "fitrp.pixel", .num_dsts = 1, .num_srcs = 4,
      .phase_io = { .dst[0] = IO(S3), .src[1] = IO(S0), .src[2] = IO(S2), },
      .supported_op_mods = OM(SAT),
      .supported_dst_types = { [0] = T(REG), },
      .supported_src_types = {
         [0] = T(DRC),
         [1] = T(REGARRAY),
         [2] = T(REGARRAY),
         [3] = T(VAL),
      },
      .src_stride = {
         [1] = 3,
         [2] = ~0U,
      },
   },
	[ROGUE_BACKEND_OP_SMP1D] = { .str = "smp1d", .num_dsts = 1, .num_srcs = 6,
      .phase_io = { .dst[0] = IO(S4), .src[1] = IO(S0), .src[2] = IO(S1), .src[3] = IO(S2), },
      .supported_op_mods = OM(PROJ) | OM(FCNORM) | OM(NNCOORDS) | OM(BIAS) | OM(REPLACE) |
         OM(GRADIENT) | OM(PPLOD) | OM(TAO) | OM(SOO) | OM(SNO) | OM(WRT) | OM(DATA) |
         OM(INFO) | OM(BOTH) | OM(BYPASS) | OM(FORCELINEFILL) | OM(WRITETHROUGH) |
         OM(WRITEBACK) | OM(LAZYWRITEBACK) | OM(SLCBYPASS) | OM(SLCWRITEBACK) |
         OM(SLCWRITETHROUGH) | OM(SLCNOALLOC) | OM(ARRAY) | OM(INTEGER) | OM(SCHEDSWAP) |
         OM(F16),
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY), },
      .supported_src_types = {
         [0] = T(DRC),
         [1] = T(REGARRAY),
         [2] = T(REG) | T(REGARRAY),
         [3] = T(REGARRAY),
         [4] = T(REGARRAY) | T(IO),
         [5] = T(VAL),
      },
      /* TODO: This may depend on the other options set. */
      .src_stride = {
         [1] = 3,
         [2] = ~0U,
         [3] = 3,
         [4] = 1,
      },
      .dst_stride = {
         [0] = ~0U,
      },
   },
	[ROGUE_BACKEND_OP_SMP2D] = { .str = "smp2d", .num_dsts = 1, .num_srcs = 6,
      .phase_io = { .dst[0] = IO(S4), .src[1] = IO(S0), .src[2] = IO(S1), .src[3] = IO(S2), },
      .supported_op_mods = OM(PROJ) | OM(FCNORM) | OM(NNCOORDS) | OM(BIAS) | OM(REPLACE) |
         OM(GRADIENT) | OM(PPLOD) | OM(TAO) | OM(SOO) | OM(SNO) | OM(WRT) | OM(DATA) |
         OM(INFO) | OM(BOTH) | OM(BYPASS) | OM(FORCELINEFILL) | OM(WRITETHROUGH) |
         OM(WRITEBACK) | OM(LAZYWRITEBACK) | OM(SLCBYPASS) | OM(SLCWRITEBACK) |
         OM(SLCWRITETHROUGH) | OM(SLCNOALLOC) | OM(ARRAY) | OM(INTEGER) | OM(SCHEDSWAP) |
         OM(F16),
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY), },
      .supported_src_types = {
         [0] = T(DRC),
         [1] = T(REGARRAY),
         [2] = T(REG) | T(REGARRAY),
         [3] = T(REGARRAY),
         [4] = T(REGARRAY) | T(IO),
         [5] = T(VAL),
      },
      /* TODO: This may depend on the other options set. */
      .src_stride = {
         [1] = 3,
         [2] = ~0U,
         [3] = 3,
         [4] = 1,
      },
      .dst_stride = {
         [0] = ~0U,
      },
   },
	[ROGUE_BACKEND_OP_SMP3D] = { .str = "smp3d", .num_dsts = 1, .num_srcs = 6,
      .phase_io = { .dst[0] = IO(S4), .src[1] = IO(S0), .src[2] = IO(S1), .src[3] = IO(S2), },
      .supported_op_mods = OM(PROJ) | OM(FCNORM) | OM(NNCOORDS) | OM(BIAS) | OM(REPLACE) |
         OM(GRADIENT) | OM(PPLOD) | OM(TAO) | OM(SOO) | OM(SNO) | OM(WRT) | OM(DATA) |
         OM(INFO) | OM(BOTH) | OM(BYPASS) | OM(FORCELINEFILL) | OM(WRITETHROUGH) |
         OM(WRITEBACK) | OM(LAZYWRITEBACK) | OM(SLCBYPASS) | OM(SLCWRITEBACK) |
         OM(SLCWRITETHROUGH) | OM(SLCNOALLOC) | OM(ARRAY) | OM(INTEGER) | OM(SCHEDSWAP) |
         OM(F16),
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY), },
      .supported_src_types = {
         [0] = T(DRC),
         [1] = T(REGARRAY),
         [2] = T(REG) | T(REGARRAY),
         [3] = T(REGARRAY),
         [4] = T(REGARRAY) | T(IO),
         [5] = T(VAL),
      },
      /* TODO: This may depend on the other options set. */
      .src_stride = {
         [1] = 3,
         [2] = ~0U,
         [3] = 3,
         [4] = 1,
      },
      .dst_stride = {
         [0] = ~0U,
      },
   },
};
#undef B
#undef T
#undef OM
#undef IO

#define OM(op_mod) BITFIELD64_BIT(ROGUE_BACKEND_OP_MOD_##op_mod)
const rogue_backend_op_mod_info rogue_backend_op_mod_infos[ROGUE_BACKEND_OP_MOD_COUNT] = {
   [ROGUE_BACKEND_OP_MOD_PROJ]  = { .str = "proj", },
   [ROGUE_BACKEND_OP_MOD_FCNORM]  = { .str = "fcnorm", },
   [ROGUE_BACKEND_OP_MOD_NNCOORDS]  = { .str = "nncoords", },
   [ROGUE_BACKEND_OP_MOD_BIAS]  = { .str = "bias", .exclude = OM(REPLACE) | OM(GRADIENT) },
   [ROGUE_BACKEND_OP_MOD_REPLACE]  = { .str = "replace", .exclude = OM(BIAS) | OM(GRADIENT) },
   [ROGUE_BACKEND_OP_MOD_GRADIENT]  = { .str = "gradient", .exclude = OM(BIAS) | OM(REPLACE) },
   [ROGUE_BACKEND_OP_MOD_PPLOD]  = { .str = "pplod", .require = OM(BIAS) | OM(REPLACE) },
   [ROGUE_BACKEND_OP_MOD_TAO]  = { .str = "tao", },
   [ROGUE_BACKEND_OP_MOD_SOO]  = { .str = "soo", },
   [ROGUE_BACKEND_OP_MOD_SNO]  = { .str = "sno", },
   [ROGUE_BACKEND_OP_MOD_WRT]  = { .str = "wrt", },
   [ROGUE_BACKEND_OP_MOD_DATA]  = { .str = "data", .exclude = OM(INFO) | OM(BOTH) },
   [ROGUE_BACKEND_OP_MOD_INFO]  = { .str = "info", .exclude = OM(DATA) | OM(BOTH) },
   [ROGUE_BACKEND_OP_MOD_BOTH]  = { .str = "both", .exclude = OM(DATA) | OM(INFO) },
   [ROGUE_BACKEND_OP_MOD_TILED] = { .str = "tiled", },
   [ROGUE_BACKEND_OP_MOD_BYPASS]  = { .str = "bypass", .exclude = OM(FORCELINEFILL) | OM(WRITETHROUGH) | OM(WRITEBACK) | OM(LAZYWRITEBACK) },
   [ROGUE_BACKEND_OP_MOD_FORCELINEFILL]  = { .str = "forcelinefill", .exclude = OM(BYPASS) | OM(WRITETHROUGH) | OM(WRITEBACK) | OM(LAZYWRITEBACK) },
   [ROGUE_BACKEND_OP_MOD_WRITETHROUGH]  = { .str = "writethrough", .exclude = OM(BYPASS) | OM(FORCELINEFILL) | OM(WRITEBACK) | OM(LAZYWRITEBACK) },
   [ROGUE_BACKEND_OP_MOD_WRITEBACK]  = { .str = "writeback", .exclude = OM(BYPASS) | OM(FORCELINEFILL) | OM(WRITETHROUGH) | OM(LAZYWRITEBACK) },
   [ROGUE_BACKEND_OP_MOD_LAZYWRITEBACK]  = { .str = "lazywriteback", .exclude = OM(BYPASS) | OM(FORCELINEFILL) | OM(WRITETHROUGH) | OM(WRITEBACK) },
   [ROGUE_BACKEND_OP_MOD_SLCBYPASS]  = { .str = "slcbypass", .exclude = OM(SLCWRITEBACK) | OM(SLCWRITETHROUGH) | OM(SLCNOALLOC) },
   [ROGUE_BACKEND_OP_MOD_SLCWRITEBACK]  = { .str = "slcwriteback", .exclude = OM(SLCBYPASS) | OM(SLCWRITETHROUGH) | OM(SLCNOALLOC) },
   [ROGUE_BACKEND_OP_MOD_SLCWRITETHROUGH]  = { .str = "slcwritethrough", .exclude = OM(SLCBYPASS) | OM(SLCWRITEBACK) | OM(SLCNOALLOC) },
   [ROGUE_BACKEND_OP_MOD_SLCNOALLOC]  = { .str = "slcnoalloc", .exclude = OM(SLCBYPASS) | OM(SLCWRITEBACK) | OM(SLCWRITETHROUGH) },
   [ROGUE_BACKEND_OP_MOD_ARRAY]  = { .str = "array", },
   [ROGUE_BACKEND_OP_MOD_INTEGER]  = { .str = "integer", },
   [ROGUE_BACKEND_OP_MOD_SCHEDSWAP]  = { .str = "schedswap", },
   [ROGUE_BACKEND_OP_MOD_F16]  = { .str = "f16", },
   [ROGUE_BACKEND_OP_MOD_SAT]  = { .str = "sat", },
   [ROGUE_BACKEND_OP_MOD_FREEP] = { .str = "freep", },
};
#undef OM

#define OM(op_mod) BITFIELD64_BIT(ROGUE_BITWISE_OP_MOD_##op_mod)
const rogue_bitwise_op_mod_info
   rogue_bitwise_op_mod_infos[ROGUE_BITWISE_OP_MOD_COUNT] = {
      [ROGUE_BITWISE_OP_MOD_TWB] = { .str = "twb",
                                     .exclude = OM(PWB) | OM(MTB) | OM(FTB) },
      [ROGUE_BITWISE_OP_MOD_PWB] = { .str = "pwb",
                                     .exclude = OM(TWB) | OM(MTB) | OM(FTB) },
      [ROGUE_BITWISE_OP_MOD_MTB] = { .str = "mtb",
                                     .exclude = OM(TWB) | OM(PWB) | OM(FTB) },
      [ROGUE_BITWISE_OP_MOD_FTB] = { .str = "ftb",
                                     .exclude = OM(TWB) | OM(PWB) | OM(MTB) },
   };
#undef OM

#define P(type) BITFIELD64_BIT(ROGUE_INSTR_PHASE_##type)
#define PH(type) ROGUE_INSTR_PHASE_##type
#define IO(io) ROGUE_IO_##io
#define T(type) BITFIELD64_BIT(ROGUE_REF_TYPE_##type - 1)
const rogue_bitwise_op_info rogue_bitwise_op_infos[ROGUE_BITWISE_OP_COUNT] = {
   [ROGUE_BITWISE_OP_INVALID] = { .str = "", },
   [ROGUE_BITWISE_OP_BYP0] = { .str = "byp", .num_dsts = 2, .num_srcs = 2,
      .supported_phases = P(0_BITMASK),
      .phase_io[PH(0_BITMASK)] = { .dst[1] = IO(FT1), },
      .supported_dst_types = {
         [0] = T(REG) | T(REGARRAY) | T(IO),
         [1] = T(REG) | T(REGARRAY) | T(IO),
      },
      .supported_src_types = {
         [0] = T(REG) | T(REGARRAY) | T(IO),
         [1] = T(REG) | T(REGARRAY) | T(IO) | T(VAL),
      },
   },
};
#undef T
#undef IO
#undef PH
#undef P

const rogue_io_info rogue_io_infos[ROGUE_IO_COUNT] = {
   [ROGUE_IO_INVALID] = { .str = "!INVALID!", },
   [ROGUE_IO_S0] = { .str = "s0", },
   [ROGUE_IO_S1] = { .str = "s1", },
   [ROGUE_IO_S2] = { .str = "s2", },
   [ROGUE_IO_S3] = { .str = "s3", },
   [ROGUE_IO_S4] = { .str = "s4", },
   [ROGUE_IO_S5] = { .str = "s5", },
   [ROGUE_IO_W0] = { .str = "w0", },
   [ROGUE_IO_W1] = { .str = "w1", },
   [ROGUE_IO_IS0] = { .str = "is0", },
   [ROGUE_IO_IS1] = { .str = "is1", },
   [ROGUE_IO_IS2] = { .str = "is2", },
   [ROGUE_IO_IS3] = { .str = "is3", },
   [ROGUE_IO_IS4] = { .str = "is4/w0", },
   [ROGUE_IO_IS5] = { .str = "is5/w1", },
   [ROGUE_IO_FT0] = { .str = "ft0", },
   [ROGUE_IO_FT1] = { .str = "ft1", },
   [ROGUE_IO_FT2] = { .str = "ft2", },
   [ROGUE_IO_FTE] = { .str = "fte", },
   [ROGUE_IO_FT3] = { .str = "ft3", },
   [ROGUE_IO_FT4] = { .str = "ft4", },
   [ROGUE_IO_FT5] = { .str = "ft5", },
   [ROGUE_IO_FTT] = { .str = "ftt", },
   [ROGUE_IO_P0] = { .str = "p0", },
   [ROGUE_IO_NONE] = { .str = "_", },
};

#define SM(src_mod) BITFIELD64_BIT(ROGUE_ALU_SRC_MOD_##src_mod)
#define DM(dst_mod) BITFIELD64_BIT(ROGUE_ALU_DST_MOD_##dst_mod)
#define OM(op_mod) BITFIELD64_BIT(ROGUE_ALU_OP_MOD_##op_mod)
#define P(type) BITFIELD64_BIT(ROGUE_INSTR_PHASE_##type)
#define PH(type) ROGUE_INSTR_PHASE_##type
#define IO(io) ROGUE_IO_##io
#define T(type) BITFIELD64_BIT(ROGUE_REF_TYPE_##type - 1)
#define B(n) BITFIELD64_BIT(n)
const rogue_alu_op_info rogue_alu_op_infos[ROGUE_ALU_OP_COUNT] = {
   [ROGUE_ALU_OP_INVALID] = { .str = "!INVALID!", },
   [ROGUE_ALU_OP_MBYP] = { .str = "mbyp", .num_dsts = 1, .num_srcs = 1,
      .supported_phases = P(0),
      .phase_io[PH(0)] = { .dst[0] = IO(FT0), .src[0] = IO(S0), },
      .supported_src_mods = {
         [0] = SM(ABS) | SM(NEG),
      },
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY) | T(IO), },
      .supported_src_types = {
         [0] = T(REG) | T(REGARRAY),
      },
   },
   [ROGUE_ALU_OP_FADD] = { .str = "fadd", .num_dsts = 1, .num_srcs = 2,
      .supported_phases = P(0),
      .phase_io[PH(0)] = { .dst[0] = IO(FT0), .src[0] = IO(S0), .src[1] = IO(S1), },
      .supported_op_mods = OM(LP) | OM(SAT),
      .supported_src_mods = {
         [0] = SM(FLR) | SM(ABS) | SM(NEG),
         [1] = SM(ABS),
      },
   },
   [ROGUE_ALU_OP_FMUL] = { .str = "fmul", .num_dsts = 1, .num_srcs = 2,
      .supported_phases = P(0),
      .phase_io[PH(0)] = { .dst[0] = IO(FT0), .src[0] = IO(S0), .src[1] = IO(S1), },
      .supported_op_mods = OM(LP) | OM(SAT),
      .supported_src_mods = {
         [0] = SM(FLR) | SM(ABS) | SM(NEG),
         [1] = SM(ABS),
      },
      .supported_dst_types = { [0] = T(REG), },
      .supported_src_types = {
         [0] = T(REG),
         [1] = T(REG),
      },
   },
   [ROGUE_ALU_OP_FMAD] = { .str = "fmad", .num_dsts = 1, .num_srcs = 3,
      .supported_phases = P(0),
      .phase_io[PH(0)] = { .dst[0] = IO(FT0), .src[0] = IO(S0), .src[1] = IO(S1), .src[2] = IO(S2), },
      .supported_op_mods = OM(LP) | OM(SAT),
      .supported_src_mods = {
         [0] = SM(ABS) | SM(NEG),
         [1] = SM(ABS) | SM(NEG),
         [2] = SM(FLR) | SM(ABS) | SM(NEG),
      },
      .supported_dst_types = { [0] = T(REG), },
      .supported_src_types = {
         [0] = T(REG),
         [1] = T(REG),
         [2] = T(REG),
      },
   },
   /* TODO NEXT!: Validate - can/must only select element if non-32-bit type, element has to be same for both args if both args present, 16-bit must be 0 or 1, 32-bit must be 0-3 (can't have no element set)
    * Also validate number of sources provided/nulled out based on test op */
   [ROGUE_ALU_OP_TST] = { .str = "tst", .num_dsts = 2, .num_srcs = 2,
      .supported_phases = P(2_TST),
      .phase_io[PH(2_TST)] = { .src[0] = IO(IS1), .src[1] = IO(IS2), },
      .supported_op_mods = OM(Z) | OM(GZ) | OM(GEZ) | OM(C) | OM(E) | OM(G) | OM(GE) | OM(NE) | OM(L) | OM(LE) |
         OM(F32) | OM(U16) | OM(S16) | OM(U8) | OM(S8) | OM(U32) | OM(S32),
      .supported_src_mods = {
         [0] = SM(E0) | SM(E1) | SM(E2) | SM(E3),
         [1] = SM(E0) | SM(E1) | SM(E2) | SM(E3),
      },
      .supported_dst_types = { [0] = T(IO), [1] = T(IO), }, /* FTT and either P0 or NONE */
      .supported_src_types = {
         [0] = T(REG) | T(IO),
         [1] = T(REG) | T(IO),
      },
   },
   /* TODO: Support fully. */
   [ROGUE_ALU_OP_MOVC] = { .str = "movc", .num_dsts = 2, .num_srcs = 3,
      .supported_phases = P(2_MOV),
      .phase_io[PH(2_MOV)] = { .dst[0] = IO(W0), .src[1] = IO(FTE), },
      .supported_dst_mods = {
         [0] = DM(E0) | DM(E1) | DM(E2) | DM(E3),
      },
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY), [1] = T(REG) | T(REGARRAY) | T(IO), },
      .supported_src_types = {
         [0] = T(IO),
         [1] = T(REG) | T(REGARRAY) | T(IO),
         [2] = T(REG) | T(REGARRAY) | T(IO),
      },
   },
   [ROGUE_ALU_OP_ADD64] = { .str = "add64", .num_dsts = 3, .num_srcs = 5,
      .supported_phases = P(0),
      .phase_io[PH(0)] = { .dst[0] = IO(FT0), .dst[1] = IO(FTE), .src[0] = IO(S0), .src[1] = IO(S1), .src[2] = IO(S2), .src[3] = IO(IS0), },
      .supported_src_mods = {
         [0] = SM(ABS) | SM(NEG),
         [1] = SM(ABS) | SM(NEG),
         [2] = SM(ABS) | SM(NEG),
         [3] = SM(ABS) | SM(NEG),
      },
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY), [1] = T(REG) | T(REGARRAY) | T(IO), [2] = T(IO) },
      .supported_src_types = {
         [0] = T(REG) | T(REGARRAY),
         [1] = T(REG) | T(REGARRAY),
         [2] = T(REG) | T(REGARRAY) | T(IMM),
         [3] = T(REG) | T(REGARRAY)| T(IO) | T(IMM),
         [4] = T(IO),
      },
   },
   [ROGUE_ALU_OP_PCK_U8888] = { .str = "pck.u8888", .num_dsts = 1, .num_srcs = 1,
      .supported_phases = P(2_PCK),
      .phase_io[PH(2_PCK)] = { .dst[0] = IO(FT2), .src[0] = IO(IS3), },
      .supported_op_mods = OM(SCALE) | OM(ROUNDZERO),
      .supported_dst_types = { [0] = T(REG), },
      .supported_src_types = {
         [0] = T(REGARRAY),
      },
      .src_repeat_mask = B(0),
   },
   [ROGUE_ALU_OP_MOV] = { .str = "mov", .num_dsts = 1, .num_srcs = 1,
      .supported_dst_types = { [0] = T(REG) | T(REGARRAY), },
      .supported_src_types = {
         [0] = T(REG) | T(REGARRAY) | T(IMM),
      },
   },
   [ROGUE_ALU_OP_CMOV] = { .str = "cmov", .num_dsts = 1, .num_srcs = 3,
      .supported_dst_types = { [0] = T(REG), },
      .supported_src_types = {
         [0] = T(IO),
         [1] = T(REG),
         [2] = T(REG),
      },
   },
   [ROGUE_ALU_OP_FABS] = { .str = "fabs", .num_dsts = 1, .num_srcs = 1, },
   [ROGUE_ALU_OP_FNEG] = { .str = "fneg", .num_dsts = 1, .num_srcs = 1, },
   [ROGUE_ALU_OP_FNABS] = { .str = "fnabs", .num_dsts = 1, .num_srcs = 1, },

   [ROGUE_ALU_OP_FMAX] = { .str = "fmax", .num_dsts = 1, .num_srcs = 2, }, /* TODO */
   [ROGUE_ALU_OP_FMIN] = { .str = "fmin", .num_dsts = 1, .num_srcs = 2, }, /* TODO */
};
#undef B
#undef T
#undef IO
#undef PH
#undef P
#undef OM
#undef DM
#undef SM

const char *rogue_exec_cond_str[ROGUE_EXEC_COND_COUNT] = {
   [ROGUE_EXEC_COND_INVALID] = "!INVALID!",
   [ROGUE_EXEC_COND_PE_TRUE] = "if(pe)",
   [ROGUE_EXEC_COND_P0_TRUE] = "if(p0)",
   [ROGUE_EXEC_COND_PE_ANY] = "any(pe)",
   [ROGUE_EXEC_COND_P0_FALSE] = "if(!p0)",
};

const char *rogue_instr_type_str[ROGUE_INSTR_TYPE_COUNT] = {
   [ROGUE_INSTR_TYPE_INVALID] = "!INVALID!",

   [ROGUE_INSTR_TYPE_ALU] = "alu",
   /* [ROGUE_INSTR_TYPE_CMPLX] = "cmplx", */
   [ROGUE_INSTR_TYPE_BACKEND] = "backend",
   [ROGUE_INSTR_TYPE_CTRL] = "ctrl",
   [ROGUE_INSTR_TYPE_BITWISE] = "bitwise",
   /* [ROGUE_INSTR_TYPE_F16SOP] = "f16sop", */
};

const char *const rogue_alu_str[ROGUE_ALU_COUNT] = {
   [ROGUE_ALU_INVALID] = "!INVALID!",
   [ROGUE_ALU_MAIN] = "main",
   [ROGUE_ALU_BITWISE] = "bitwise",
   [ROGUE_ALU_CONTROL] = "control",
};

const char *const rogue_instr_phase_str[ROGUE_ALU_COUNT][ROGUE_INSTR_PHASE_COUNT] = {
   /** Main/ALU (and backend) instructions. */
   [ROGUE_ALU_MAIN] = {
      [ROGUE_INSTR_PHASE_0] = "p0",
      [ROGUE_INSTR_PHASE_1] = "p1",
      [ROGUE_INSTR_PHASE_2_PCK] = "p2pck",
      [ROGUE_INSTR_PHASE_2_TST] = "p2tst",
      [ROGUE_INSTR_PHASE_2_MOV] = "p2mov",
      [ROGUE_INSTR_PHASE_BACKEND] = "backend",
   },

   /** Bitwise instructions. */
   [ROGUE_ALU_BITWISE] = {
      [ROGUE_INSTR_PHASE_0_BITMASK] = "p0bm",
      [ROGUE_INSTR_PHASE_0_SHIFT1] = "p0shf1",
      [ROGUE_INSTR_PHASE_0_COUNT] = "p0cnt",
      [ROGUE_INSTR_PHASE_1_LOGICAL] = "p1log",
      [ROGUE_INSTR_PHASE_2_SHIFT2] = "p2shf2",
      [ROGUE_INSTR_PHASE_2_TEST] = "p2tst",
   },

   /** Control instructions (no co-issuing). */
   [ROGUE_ALU_CONTROL] = {
      [ROGUE_INSTR_PHASE_CTRL] = "ctrl",
   },
};
