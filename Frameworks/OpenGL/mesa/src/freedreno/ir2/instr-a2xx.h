/*
 * Copyright (c) 2012 Rob Clark <robdclark@gmail.com>
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

#ifndef INSTR_A2XX_H_
#define INSTR_A2XX_H_

#define PACKED __attribute__((__packed__))

#include "util/u_math.h"
#include "adreno_common.xml.h"
#include "adreno_pm4.xml.h"
#include "a2xx.xml.h"

/*
 * ALU instructions:
 */

typedef enum {
   ADDs = 0,
   ADD_PREVs = 1,
   MULs = 2,
   MUL_PREVs = 3,
   MUL_PREV2s = 4,
   MAXs = 5,
   MINs = 6,
   SETEs = 7,
   SETGTs = 8,
   SETGTEs = 9,
   SETNEs = 10,
   FRACs = 11,
   TRUNCs = 12,
   FLOORs = 13,
   EXP_IEEE = 14,
   LOG_CLAMP = 15,
   LOG_IEEE = 16,
   RECIP_CLAMP = 17,
   RECIP_FF = 18,
   RECIP_IEEE = 19,
   RECIPSQ_CLAMP = 20,
   RECIPSQ_FF = 21,
   RECIPSQ_IEEE = 22,
   MOVAs = 23,
   MOVA_FLOORs = 24,
   SUBs = 25,
   SUB_PREVs = 26,
   PRED_SETEs = 27,
   PRED_SETNEs = 28,
   PRED_SETGTs = 29,
   PRED_SETGTEs = 30,
   PRED_SET_INVs = 31,
   PRED_SET_POPs = 32,
   PRED_SET_CLRs = 33,
   PRED_SET_RESTOREs = 34,
   KILLEs = 35,
   KILLGTs = 36,
   KILLGTEs = 37,
   KILLNEs = 38,
   KILLONEs = 39,
   SQRT_IEEE = 40,
   MUL_CONST_0 = 42,
   MUL_CONST_1 = 43,
   ADD_CONST_0 = 44,
   ADD_CONST_1 = 45,
   SUB_CONST_0 = 46,
   SUB_CONST_1 = 47,
   SIN = 48,
   COS = 49,
   RETAIN_PREV = 50,
   SCALAR_NONE = 63,
} instr_scalar_opc_t;

typedef enum {
   ADDv = 0,
   MULv = 1,
   MAXv = 2,
   MINv = 3,
   SETEv = 4,
   SETGTv = 5,
   SETGTEv = 6,
   SETNEv = 7,
   FRACv = 8,
   TRUNCv = 9,
   FLOORv = 10,
   MULADDv = 11,
   CNDEv = 12,
   CNDGTEv = 13,
   CNDGTv = 14,
   DOT4v = 15,
   DOT3v = 16,
   DOT2ADDv = 17,
   CUBEv = 18,
   MAX4v = 19,
   PRED_SETE_PUSHv = 20,
   PRED_SETNE_PUSHv = 21,
   PRED_SETGT_PUSHv = 22,
   PRED_SETGTE_PUSHv = 23,
   KILLEv = 24,
   KILLGTv = 25,
   KILLGTEv = 26,
   KILLNEv = 27,
   DSTv = 28,
   MOVAv = 29,
   VECTOR_NONE = 31,
} instr_vector_opc_t;

typedef struct PACKED {
   /* dword0: */
   uint8_t vector_dest : 6;
   uint8_t vector_dest_rel : 1;
   uint8_t low_precision_16b_fp : 1;
   uint8_t scalar_dest : 6;
   uint8_t scalar_dest_rel : 1;
   uint8_t export_data : 1;
   uint8_t vector_write_mask : 4;
   uint8_t scalar_write_mask : 4;
   uint8_t vector_clamp : 1;
   uint8_t scalar_clamp : 1;
   instr_scalar_opc_t scalar_opc : 6;
   /* dword1: */
   uint8_t src3_swiz : 8;
   uint8_t src2_swiz : 8;
   uint8_t src1_swiz : 8;
   uint8_t src3_reg_negate : 1;
   uint8_t src2_reg_negate : 1;
   uint8_t src1_reg_negate : 1;
   uint8_t pred_select : 2;
   uint8_t relative_addr : 1;
   uint8_t const_1_rel_abs : 1;
   uint8_t const_0_rel_abs : 1;
   /* dword2: */
   union {
      struct {
         uint8_t src3_reg : 6;
         uint8_t src3_reg_select : 1;
         uint8_t src3_reg_abs : 1;
         uint8_t src2_reg : 6;
         uint8_t src2_reg_select : 1;
         uint8_t src2_reg_abs : 1;
         uint8_t src1_reg : 6;
         uint8_t src1_reg_select : 1;
         uint8_t src1_reg_abs : 1;
      };
      /* constants have full 8-bit index */
      struct {
         uint8_t src3_reg_byte : 8;
         uint8_t src2_reg_byte : 8;
         uint8_t src1_reg_byte : 8;
      };
   };
   instr_vector_opc_t vector_opc : 5;
   uint8_t src3_sel : 1;
   uint8_t src2_sel : 1;
   uint8_t src1_sel : 1;
} instr_alu_t;

/*
 * CF instructions:
 */

typedef enum {
   NOP = 0,
   EXEC = 1,
   EXEC_END = 2,
   COND_EXEC = 3,
   COND_EXEC_END = 4,
   COND_PRED_EXEC = 5,
   COND_PRED_EXEC_END = 6,
   LOOP_START = 7,
   LOOP_END = 8,
   COND_CALL = 9,
   RETURN = 10,
   COND_JMP = 11,
   ALLOC = 12,
   COND_EXEC_PRED_CLEAN = 13,
   COND_EXEC_PRED_CLEAN_END = 14,
   MARK_VS_FETCH_DONE = 15,
} instr_cf_opc_t;

typedef enum {
   RELATIVE_ADDR = 0,
   ABSOLUTE_ADDR = 1,
} instr_addr_mode_t;

typedef enum {
   SQ_NO_ALLOC = 0,
   SQ_POSITION = 1,
   SQ_PARAMETER_PIXEL = 2,
   SQ_MEMORY = 3,
} instr_alloc_type_t;

typedef struct PACKED {
   uint16_t address : 9;
   uint8_t reserved0 : 3;
   uint8_t count : 3;
   uint8_t yeild : 1;
   uint16_t serialize : 12;
   uint32_t vc : 6; /* vertex cache? */
   uint32_t bool_addr : 8;
   uint8_t condition : 1;
   instr_addr_mode_t address_mode : 1;
   instr_cf_opc_t opc : 4;
} instr_cf_exec_t;

typedef struct PACKED {
   uint16_t address : 10;
   uint8_t reserved0 : 6;
   uint8_t loop_id : 5;
   uint32_t reserved1 : 22;
   instr_addr_mode_t address_mode : 1;
   instr_cf_opc_t opc : 4;
} instr_cf_loop_t;

typedef struct PACKED {
   uint16_t address : 10;
   uint8_t reserved0 : 3;
   uint8_t force_call : 1;
   uint8_t predicated_jmp : 1;
   uint32_t reserved1 : 18;
   uint8_t direction : 1;
   uint32_t bool_addr : 8;
   uint8_t condition : 1;
   instr_addr_mode_t address_mode : 1;
   instr_cf_opc_t opc : 4;
} instr_cf_jmp_call_t;

typedef struct PACKED {
   uint8_t size : 4;
   uint64_t reserved0 : 36;
   uint8_t no_serial : 1;
   instr_alloc_type_t buffer_select : 2;
   uint8_t alloc_mode : 1;
   instr_cf_opc_t opc : 4;
} instr_cf_alloc_t;

typedef union PACKED {
   instr_cf_exec_t exec;
   instr_cf_loop_t loop;
   instr_cf_jmp_call_t jmp_call;
   instr_cf_alloc_t alloc;
   struct PACKED {
      uint64_t dummy : 44;
      instr_cf_opc_t opc : 4;
   };
} instr_cf_t;

/*
 * FETCH instructions:
 */

typedef enum {
   VTX_FETCH = 0,
   TEX_FETCH = 1,
   TEX_GET_BORDER_COLOR_FRAC = 16,
   TEX_GET_COMP_TEX_LOD = 17,
   TEX_GET_GRADIENTS = 18,
   TEX_GET_WEIGHTS = 19,
   TEX_SET_TEX_LOD = 24,
   TEX_SET_GRADIENTS_H = 25,
   TEX_SET_GRADIENTS_V = 26,
   TEX_RESERVED_4 = 27,
} instr_fetch_opc_t;

typedef enum {
   TEX_FILTER_POINT = 0,
   TEX_FILTER_LINEAR = 1,
   TEX_FILTER_BASEMAP = 2, /* only applicable for mip-filter */
   TEX_FILTER_USE_FETCH_CONST = 3,
} instr_tex_filter_t;

typedef enum {
   ANISO_FILTER_DISABLED = 0,
   ANISO_FILTER_MAX_1_1 = 1,
   ANISO_FILTER_MAX_2_1 = 2,
   ANISO_FILTER_MAX_4_1 = 3,
   ANISO_FILTER_MAX_8_1 = 4,
   ANISO_FILTER_MAX_16_1 = 5,
   ANISO_FILTER_USE_FETCH_CONST = 7,
} instr_aniso_filter_t;

typedef enum {
   ARBITRARY_FILTER_2X4_SYM = 0,
   ARBITRARY_FILTER_2X4_ASYM = 1,
   ARBITRARY_FILTER_4X2_SYM = 2,
   ARBITRARY_FILTER_4X2_ASYM = 3,
   ARBITRARY_FILTER_4X4_SYM = 4,
   ARBITRARY_FILTER_4X4_ASYM = 5,
   ARBITRARY_FILTER_USE_FETCH_CONST = 7,
} instr_arbitrary_filter_t;

typedef enum {
   SAMPLE_CENTROID = 0,
   SAMPLE_CENTER = 1,
} instr_sample_loc_t;

typedef enum a2xx_sq_surfaceformat instr_surf_fmt_t;

typedef struct PACKED {
   /* dword0: */
   instr_fetch_opc_t opc : 5;
   uint32_t src_reg : 6;
   uint8_t src_reg_am : 1;
   uint32_t dst_reg : 6;
   uint8_t dst_reg_am : 1;
   uint8_t fetch_valid_only : 1;
   uint32_t const_idx : 5;
   uint8_t tx_coord_denorm : 1;
   uint8_t src_swiz : 6;
   /* dword1: */
   uint16_t dst_swiz : 12;
   instr_tex_filter_t mag_filter : 2;
   instr_tex_filter_t min_filter : 2;
   instr_tex_filter_t mip_filter : 2;
   instr_aniso_filter_t aniso_filter : 3;
   instr_arbitrary_filter_t arbitrary_filter : 3;
   instr_tex_filter_t vol_mag_filter : 2;
   instr_tex_filter_t vol_min_filter : 2;
   uint8_t use_comp_lod : 1;
   uint8_t use_reg_lod : 2; /* 0 for cube, 1 for 2d */
   uint8_t pred_select : 1;
   /* dword2: */
   uint8_t use_reg_gradients : 1;
   instr_sample_loc_t sample_location : 1;
   uint32_t lod_bias : 7;
   uint8_t unused : 7;
   uint8_t offset_x : 5;
   uint32_t offset_y : 5;
   uint8_t offset_z : 5;
   uint8_t pred_condition : 1;
} instr_fetch_tex_t;

typedef struct PACKED {
   /* dword0: */
   instr_fetch_opc_t opc : 5;
   uint32_t src_reg : 6;
   uint8_t src_reg_am : 1;
   uint32_t dst_reg : 6;
   uint8_t dst_reg_am : 1;
   uint8_t must_be_one : 1;
   uint32_t const_index : 5;
   uint8_t const_index_sel : 2;
   uint8_t reserved0 : 3;
   uint8_t src_swiz : 2;
   /* dword1: */
   uint16_t dst_swiz : 12;
   uint8_t format_comp_all : 1; /* '1' for signed, '0' for unsigned? */
   uint8_t num_format_all : 1;  /* '0' for normalized, '1' for unnormalized */
   uint8_t signed_rf_mode_all : 1;
   uint8_t reserved1 : 1;
   instr_surf_fmt_t format : 6;
   uint8_t reserved2 : 2;
   uint8_t exp_adjust_all : 6;
   uint8_t reserved3 : 1;
   uint8_t pred_select : 1;
   /* dword2: */
   uint8_t stride : 8;
   uint32_t offset : 22;
   uint8_t reserved4 : 1;
   uint8_t pred_condition : 1;
} instr_fetch_vtx_t;

typedef union PACKED {
   instr_fetch_tex_t tex;
   instr_fetch_vtx_t vtx;
   struct PACKED {
      /* dword0: */
      instr_fetch_opc_t opc : 5;
      uint32_t dummy0 : 27;
      /* dword1: */
      uint32_t dummy1 : 31;
      uint8_t pred_select : 1;
      /* dword2: */
      uint32_t dummy2 : 31;
      uint8_t pred_condition : 1;
   };
} instr_fetch_t;

typedef union PACKED {
   instr_alu_t alu;
   instr_fetch_t fetch;
} instr_t;

#endif /* INSTR_H_ */
