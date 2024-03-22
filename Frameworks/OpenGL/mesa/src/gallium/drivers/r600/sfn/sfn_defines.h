/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2018-2019 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef sfn_defines_h
#define sfn_defines_h

#include "../r600_isa.h"
#include "amd_family.h"
namespace r600 {

enum EGWSOpCode {
   cf_sema_v = 0,
   cf_sema_p = 1,
   cf_gws_barrier = 2,
   cf_gws_init = 3,
};

/* CF ALU instructions [29:26], highest bit always set. */
enum ECFAluOpCode {
   cf_alu_undefined = 0,
   cf_alu = CF_OP_ALU,
   cf_alu_push_before = CF_OP_ALU_PUSH_BEFORE,
   cf_alu_pop_after = CF_OP_ALU_POP_AFTER,
   cf_alu_pop2_after = CF_OP_ALU_POP2_AFTER,
   cf_alu_extended = CF_OP_ALU_EXT,
   cf_alu_continue = CF_OP_ALU_CONTINUE,
   cf_alu_break = CF_OP_ALU_BREAK,
   cf_alu_else_after = CF_OP_ALU_ELSE_AFTER,
};

enum ECFAluOpCodeEG {
   eg_cf_alu_undefined = 0,
   eg_cf_alu = 8,
   eg_cf_alu_push_before = 9,
   eg_cf_alu_pop_after = 10,
   eg_cf_alu_pop2_after = 11,
   eg_cf_alu_extended = 12,
   eg_cf_alu_continue = 13,
   eg_cf_alu_break = 14,
   eg_cf_alu_else_after = 15,
};

enum ECFOpCode {
   cf_nop = CF_OP_NOP,
   cf_tc = CF_OP_TEX,
   cf_vc = CF_OP_VTX,
   cf_gds = CF_OP_GDS,
   cf_loop_start = CF_OP_LOOP_START,
   cf_loop_end = CF_OP_LOOP_END,
   cf_loop_start_dx10 = CF_OP_LOOP_START_DX10,
   cf_loop_start_no_al = CF_OP_LOOP_START_NO_AL,
   cf_loop_continue = CF_OP_LOOP_CONTINUE,
   cf_loop_break = CF_OP_LOOP_BREAK,
   cf_jump = CF_OP_JUMP,
   cf_push = CF_OP_PUSH,
   cf_else = CF_OP_ELSE,
   cf_pop = CF_OP_POP,
   /* 15 - 17 reserved */
   cf_call = CF_OP_CALL,
   cf_call_fs = CF_OP_CALL_FS,
   cf_return = CF_OP_RET,
   cf_emit_vertex = CF_OP_EMIT_VERTEX,
   cf_emit_cut_vertex = CF_OP_EMIT_CUT_VERTEX,
   cf_cut_vertex = CF_OP_CUT_VERTEX,
   cf_kill = CF_OP_KILL,
   /* 25 reserved */
   cf_wait_ack = CF_OP_WAIT_ACK,
   cf_tc_ack = CF_OP_TEX_ACK,
   cf_vc_ack = CF_OP_VTX_ACK,
   cf_jump_table = CF_OP_JUMPTABLE,
   cf_global_wave_sync = CF_OP_WAVE_SYNC,
   cf_halt = CF_OP_HALT,
   /* gap 32-63*/
   cf_mem_stream0_buf0 = CF_OP_MEM_STREAM0_BUF0,
   cf_mem_stream0_buf1 = CF_OP_MEM_STREAM0_BUF1,
   cf_mem_stream0_buf2 = CF_OP_MEM_STREAM0_BUF2,
   cf_mem_stream0_buf3 = CF_OP_MEM_STREAM0_BUF3,

   cf_mem_stream1_buf0 = CF_OP_MEM_STREAM1_BUF0,
   cf_mem_stream1_buf1 = CF_OP_MEM_STREAM1_BUF1,
   cf_mem_stream1_buf2 = CF_OP_MEM_STREAM1_BUF2,
   cf_mem_stream1_buf3 = CF_OP_MEM_STREAM1_BUF3,

   cf_mem_stream2_buf0 = CF_OP_MEM_STREAM2_BUF0,
   cf_mem_stream2_buf1 = CF_OP_MEM_STREAM2_BUF1,
   cf_mem_stream2_buf2 = CF_OP_MEM_STREAM2_BUF2,
   cf_mem_stream2_buf3 = CF_OP_MEM_STREAM2_BUF3,

   cf_mem_stream3_buf0 = CF_OP_MEM_STREAM3_BUF0,
   cf_mem_stream3_buf1 = CF_OP_MEM_STREAM3_BUF1,
   cf_mem_stream3_buf2 = CF_OP_MEM_STREAM3_BUF2,
   cf_mem_stream3_buf3 = CF_OP_MEM_STREAM3_BUF3,

   cf_mem_write_scratch = CF_OP_MEM_SCRATCH,
   /* reserved 81 */
   cf_mem_ring = CF_OP_MEM_RING,
   cf_export = CF_OP_EXPORT,
   cf_export_done = CF_OP_EXPORT_DONE,
   cf_mem_export = CF_OP_MEM_EXPORT,
   cf_mem_rat = CF_OP_MEM_RAT,
   cf_mem_rat_cacheless = CF_OP_MEM_RAT_NOCACHE,

   cf_mem_ring1 = CF_OP_MEM_RING1,
   cf_mem_ring2 = CF_OP_MEM_RING2,
   cf_mem_ring3 = CF_OP_MEM_RING3,
   cf_mem_export_combined = CF_OP_MEM_MEM_COMBINED,
   cf_mem_rat_combined_cacheless = CF_OP_MEM_RAT_COMBINED_NOCACHE
};

enum ECFOpCodeEG {
   eg_cf_nop = 0,
   eg_cf_tc = 1,
   eg_cf_vc = 2,
   eg_cf_gds = 3,
   eg_cf_loop_start = 4,
   eg_cf_loop_end = 5,
   eg_cf_loop_start_dx10 = 6,
   eg_cf_loop_start_no_al = 7,
   eg_cf_loop_continue = 8,
   eg_cf_loop_break = 9,
   eg_cf_jump = 10,
   eg_cf_push = 11,
   eg_cf_else = 13,
   eg_cf_pop = 14,
   /* 15 - 17 reserved */
   eg_cf_call = 18,
   eg_cf_call_fs,
   eg_cf_return,
   eg_cf_emit_vertex,
   eg_cf_emit_cut_vertex,
   eg_cf_cut_vertex,
   eg_cf_kill,
   /* 25 reserved */
   eg_cf_wait_ack = 26,
   eg_cf_tc_ack,
   eg_cf_vc_ack,
   eg_cf_jump_table,
   eg_cf_global_wave_sync,
   eg_cf_halt,
   /* gap 32-63*/
   eg_cf_mem_stream0_buf0 = 64,
   eg_cf_mem_stream0_buf1,
   eg_cf_mem_stream0_buf2,
   eg_cf_mem_stream0_buf3,

   eg_cf_mem_stream1_buf0,
   eg_cf_mem_stream1_buf1,
   eg_cf_mem_stream1_buf2,
   eg_cf_mem_stream1_buf3,

   eg_cf_mem_stream2_buf0,
   eg_cf_mem_stream2_buf1,
   eg_cf_mem_stream2_buf2,
   eg_cf_mem_stream2_buf3,

   eg_cf_mem_stream3_buf0,
   eg_cf_mem_stream3_buf1,
   eg_cf_mem_stream3_buf2,
   eg_cf_mem_stream3_buf3,

   eg_cf_mem_write_scratch,
   /* reserved 81 */
   eg_cf_mem_ring = 82,
   eg_cf_export,
   eg_cf_export_done,
   eg_cf_mem_export,
   eg_cf_mem_rat,
   eg_cf_mem_rat_cacheless,

   eg_cf_mem_ring1,
   eg_cf_mem_ring2,
   eg_cf_mem_ring3,
   eg_cf_mem_export_combined,
   eg_cf_mem_rat_combined_cacheless
};

enum EVFetchInstr {
   vc_fetch = FETCH_OP_VFETCH,
   vc_semantic = FETCH_OP_SEMFETCH,
   vc_get_buf_resinfo = FETCH_OP_GET_BUFFER_RESINFO,
   vc_read_scratch = FETCH_OP_READ_SCRATCH,
   vc_unknown
};

enum EVFetchType {
   vertex_data = 0,
   instance_data = 1,
   no_index_offset = 2
};

enum EVTXDataFormat {
   fmt_invalid = 0,
   fmt_8 = 1,
   fmt_4_4 = 2,
   fmt_3_3_2 = 3,
   fmt_reserved_4 = 4,
   fmt_16 = 5,
   fmt_16_float = 6,
   fmt_8_8 = 7,
   fmt_5_6_5 = 8,
   fmt_6_5_5 = 9,
   fmt_1_5_5_5 = 10,
   fmt_4_4_4_4 = 11,
   fmt_5_5_5_1 = 12,
   fmt_32 = 13,
   fmt_32_float = 14,
   fmt_16_16 = 15,
   fmt_16_16_float = 16,
   fmt_8_24 = 17,
   fmt_8_24_float = 18,
   fmt_24_8 = 19,
   fmt_24_8_float = 20,
   fmt_10_11_11 = 21,
   fmt_10_11_11_float = 22,
   fmt_11_11_10 = 23,
   fmt_11_11_10_float = 24,
   fmt_2_10_10_10 = 25,
   fmt_8_8_8_8 = 26,
   fmt_10_10_10_2 = 27,
   fmt_x24_8_32_float = 28,
   fmt_32_32 = 29,
   fmt_32_32_float = 30,
   fmt_16_16_16_16 = 31,
   fmt_16_16_16_16_float = 32,
   fmt_reserved_33 = 33,
   fmt_32_32_32_32 = 34,
   fmt_32_32_32_32_float = 35,
   fmt_reserved_36 = 36,
   fmt_1 = 37,
   fmt_1_reversed = 38,
   fmt_gb_gr = 39,
   fmt_bg_rg = 40,
   fmt_32_as_8 = 41,
   fmt_32_as_8_8 = 42,
   fmt_5_9_9_9_sharedexp = 43,
   fmt_8_8_8 = 44,
   fmt_16_16_16 = 45,
   fmt_16_16_16_float = 46,
   fmt_32_32_32 = 47,
   fmt_32_32_32_float = 48,
   fmt_bc1 = 49,
   fmt_bc2 = 50,
   fmt_bc3 = 51,
   fmt_bc4 = 52,
   fmt_bc5 = 53,
   fmt_apc0 = 54,
   fmt_apc1 = 55,
   fmt_apc2 = 56,
   fmt_apc3 = 57,
   fmt_apc4 = 58,
   fmt_apc5 = 59,
   fmt_apc6 = 60,
   fmt_apc7 = 61,
   fmt_ctx1 = 62,
   fmt_reserved_63 = 63
};

enum EVFetchNumFormat {
   vtx_nf_norm = 0,
   vtx_nf_int = 1,
   vtx_nf_scaled = 2
};

enum EVFetchEndianSwap {
   vtx_es_none = 0,
   vtx_es_8in16 = 1,
   vtx_es_8in32 = 2
};

enum EVFetchFlagShift {
   vtx_fetch_whole_quad,
   vtx_use_const_field,
   vtx_format_comp_signed,
   vtx_srf_mode,
   vtx_buf_no_stride,
   vtx_alt_const,
   vtx_use_tc,
   vtx_vpm,
   vtx_is_mega_fetch,
   vtx_uncached,
   vtx_indexed,
   vtx_unknown
};

enum EBufferIndexMode {
   bim_none,
   bim_zero,
   bim_one,
   bim_invalid
};

} // namespace r600

#endif // DEFINES_H
