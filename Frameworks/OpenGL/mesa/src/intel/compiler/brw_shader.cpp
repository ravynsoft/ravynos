/*
 * Copyright © 2010 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "brw_cfg.h"
#include "brw_eu.h"
#include "brw_fs.h"
#include "brw_nir.h"
#include "brw_private.h"
#include "brw_vec4_tes.h"
#include "dev/intel_debug.h"
#include "util/macros.h"
#include "util/u_debug.h"

enum brw_reg_type
brw_type_for_base_type(const struct glsl_type *type)
{
   switch (type->base_type) {
   case GLSL_TYPE_FLOAT16:
      return BRW_REGISTER_TYPE_HF;
   case GLSL_TYPE_FLOAT:
      return BRW_REGISTER_TYPE_F;
   case GLSL_TYPE_INT:
   case GLSL_TYPE_BOOL:
   case GLSL_TYPE_SUBROUTINE:
      return BRW_REGISTER_TYPE_D;
   case GLSL_TYPE_INT16:
      return BRW_REGISTER_TYPE_W;
   case GLSL_TYPE_INT8:
      return BRW_REGISTER_TYPE_B;
   case GLSL_TYPE_UINT:
      return BRW_REGISTER_TYPE_UD;
   case GLSL_TYPE_UINT16:
      return BRW_REGISTER_TYPE_UW;
   case GLSL_TYPE_UINT8:
      return BRW_REGISTER_TYPE_UB;
   case GLSL_TYPE_ARRAY:
      return brw_type_for_base_type(type->fields.array);
   case GLSL_TYPE_STRUCT:
   case GLSL_TYPE_INTERFACE:
   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_TEXTURE:
   case GLSL_TYPE_ATOMIC_UINT:
      /* These should be overridden with the type of the member when
       * dereferenced into.  BRW_REGISTER_TYPE_UD seems like a likely
       * way to trip up if we don't.
       */
      return BRW_REGISTER_TYPE_UD;
   case GLSL_TYPE_IMAGE:
      return BRW_REGISTER_TYPE_UD;
   case GLSL_TYPE_DOUBLE:
      return BRW_REGISTER_TYPE_DF;
   case GLSL_TYPE_UINT64:
      return BRW_REGISTER_TYPE_UQ;
   case GLSL_TYPE_INT64:
      return BRW_REGISTER_TYPE_Q;
   case GLSL_TYPE_VOID:
   case GLSL_TYPE_ERROR:
   case GLSL_TYPE_COOPERATIVE_MATRIX:
      unreachable("not reached");
   }

   return BRW_REGISTER_TYPE_F;
}

uint32_t
brw_math_function(enum opcode op)
{
   switch (op) {
   case SHADER_OPCODE_RCP:
      return BRW_MATH_FUNCTION_INV;
   case SHADER_OPCODE_RSQ:
      return BRW_MATH_FUNCTION_RSQ;
   case SHADER_OPCODE_SQRT:
      return BRW_MATH_FUNCTION_SQRT;
   case SHADER_OPCODE_EXP2:
      return BRW_MATH_FUNCTION_EXP;
   case SHADER_OPCODE_LOG2:
      return BRW_MATH_FUNCTION_LOG;
   case SHADER_OPCODE_POW:
      return BRW_MATH_FUNCTION_POW;
   case SHADER_OPCODE_SIN:
      return BRW_MATH_FUNCTION_SIN;
   case SHADER_OPCODE_COS:
      return BRW_MATH_FUNCTION_COS;
   case SHADER_OPCODE_INT_QUOTIENT:
      return BRW_MATH_FUNCTION_INT_DIV_QUOTIENT;
   case SHADER_OPCODE_INT_REMAINDER:
      return BRW_MATH_FUNCTION_INT_DIV_REMAINDER;
   default:
      unreachable("not reached: unknown math function");
   }
}

bool
brw_texture_offset(const nir_tex_instr *tex, unsigned src,
                   uint32_t *offset_bits_out)
{
   if (!nir_src_is_const(tex->src[src].src))
      return false;

   const unsigned num_components = nir_tex_instr_src_size(tex, src);

   /* Combine all three offsets into a single unsigned dword:
    *
    *    bits 11:8 - U Offset (X component)
    *    bits  7:4 - V Offset (Y component)
    *    bits  3:0 - R Offset (Z component)
    */
   uint32_t offset_bits = 0;
   for (unsigned i = 0; i < num_components; i++) {
      int offset = nir_src_comp_as_int(tex->src[src].src, i);

      /* offset out of bounds; caller will handle it. */
      if (offset > 7 || offset < -8)
         return false;

      const unsigned shift = 4 * (2 - i);
      offset_bits |= (offset << shift) & (0xF << shift);
   }

   *offset_bits_out = offset_bits;

   return true;
}

const char *
brw_instruction_name(const struct brw_isa_info *isa, enum opcode op)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   switch (op) {
   case 0 ... NUM_BRW_OPCODES - 1:
      /* The DO instruction doesn't exist on Gfx6+, but we use it to mark the
       * start of a loop in the IR.
       */
      if (devinfo->ver >= 6 && op == BRW_OPCODE_DO)
         return "do";

      /* The following conversion opcodes doesn't exist on Gfx8+, but we use
       * then to mark that we want to do the conversion.
       */
      if (devinfo->ver > 7 && op == BRW_OPCODE_F32TO16)
         return "f32to16";

      if (devinfo->ver > 7 && op == BRW_OPCODE_F16TO32)
         return "f16to32";

      /* DPAS instructions may transiently exist on platforms that do not
       * support DPAS. They will eventually be lowered, but in the meantime it
       * must be possible to query the instruction name.
       */
      if (devinfo->verx10 < 125 && op == BRW_OPCODE_DPAS)
         return "dpas";

      assert(brw_opcode_desc(isa, op)->name);
      return brw_opcode_desc(isa, op)->name;
   case FS_OPCODE_FB_WRITE:
      return "fb_write";
   case FS_OPCODE_FB_WRITE_LOGICAL:
      return "fb_write_logical";
   case FS_OPCODE_REP_FB_WRITE:
      return "rep_fb_write";
   case FS_OPCODE_FB_READ:
      return "fb_read";
   case FS_OPCODE_FB_READ_LOGICAL:
      return "fb_read_logical";

   case SHADER_OPCODE_RCP:
      return "rcp";
   case SHADER_OPCODE_RSQ:
      return "rsq";
   case SHADER_OPCODE_SQRT:
      return "sqrt";
   case SHADER_OPCODE_EXP2:
      return "exp2";
   case SHADER_OPCODE_LOG2:
      return "log2";
   case SHADER_OPCODE_POW:
      return "pow";
   case SHADER_OPCODE_INT_QUOTIENT:
      return "int_quot";
   case SHADER_OPCODE_INT_REMAINDER:
      return "int_rem";
   case SHADER_OPCODE_SIN:
      return "sin";
   case SHADER_OPCODE_COS:
      return "cos";

   case SHADER_OPCODE_SEND:
      return "send";

   case SHADER_OPCODE_UNDEF:
      return "undef";

   case SHADER_OPCODE_TEX:
      return "tex";
   case SHADER_OPCODE_TEX_LOGICAL:
      return "tex_logical";
   case SHADER_OPCODE_TXD:
      return "txd";
   case SHADER_OPCODE_TXD_LOGICAL:
      return "txd_logical";
   case SHADER_OPCODE_TXF:
      return "txf";
   case SHADER_OPCODE_TXF_LOGICAL:
      return "txf_logical";
   case SHADER_OPCODE_TXF_LZ:
      return "txf_lz";
   case SHADER_OPCODE_TXL:
      return "txl";
   case SHADER_OPCODE_TXL_LOGICAL:
      return "txl_logical";
   case SHADER_OPCODE_TXL_LZ:
      return "txl_lz";
   case SHADER_OPCODE_TXS:
      return "txs";
   case SHADER_OPCODE_TXS_LOGICAL:
      return "txs_logical";
   case FS_OPCODE_TXB:
      return "txb";
   case FS_OPCODE_TXB_LOGICAL:
      return "txb_logical";
   case SHADER_OPCODE_TXF_CMS:
      return "txf_cms";
   case SHADER_OPCODE_TXF_CMS_LOGICAL:
      return "txf_cms_logical";
   case SHADER_OPCODE_TXF_CMS_W:
      return "txf_cms_w";
   case SHADER_OPCODE_TXF_CMS_W_LOGICAL:
      return "txf_cms_w_logical";
   case SHADER_OPCODE_TXF_CMS_W_GFX12_LOGICAL:
      return "txf_cms_w_gfx12_logical";
   case SHADER_OPCODE_TXF_UMS:
      return "txf_ums";
   case SHADER_OPCODE_TXF_UMS_LOGICAL:
      return "txf_ums_logical";
   case SHADER_OPCODE_TXF_MCS:
      return "txf_mcs";
   case SHADER_OPCODE_TXF_MCS_LOGICAL:
      return "txf_mcs_logical";
   case SHADER_OPCODE_LOD:
      return "lod";
   case SHADER_OPCODE_LOD_LOGICAL:
      return "lod_logical";
   case SHADER_OPCODE_TG4:
      return "tg4";
   case SHADER_OPCODE_TG4_LOGICAL:
      return "tg4_logical";
   case SHADER_OPCODE_TG4_OFFSET:
      return "tg4_offset";
   case SHADER_OPCODE_TG4_OFFSET_LOGICAL:
      return "tg4_offset_logical";
   case SHADER_OPCODE_SAMPLEINFO:
      return "sampleinfo";
   case SHADER_OPCODE_SAMPLEINFO_LOGICAL:
      return "sampleinfo_logical";

   case SHADER_OPCODE_IMAGE_SIZE_LOGICAL:
      return "image_size_logical";

   case VEC4_OPCODE_UNTYPED_ATOMIC:
      return "untyped_atomic";
   case SHADER_OPCODE_UNTYPED_ATOMIC_LOGICAL:
      return "untyped_atomic_logical";
   case VEC4_OPCODE_UNTYPED_SURFACE_READ:
      return "untyped_surface_read";
   case SHADER_OPCODE_UNTYPED_SURFACE_READ_LOGICAL:
      return "untyped_surface_read_logical";
   case VEC4_OPCODE_UNTYPED_SURFACE_WRITE:
      return "untyped_surface_write";
   case SHADER_OPCODE_UNTYPED_SURFACE_WRITE_LOGICAL:
      return "untyped_surface_write_logical";
   case SHADER_OPCODE_UNALIGNED_OWORD_BLOCK_READ_LOGICAL:
      return "unaligned_oword_block_read_logical";
   case SHADER_OPCODE_OWORD_BLOCK_WRITE_LOGICAL:
      return "oword_block_write_logical";
   case SHADER_OPCODE_A64_UNTYPED_READ_LOGICAL:
      return "a64_untyped_read_logical";
   case SHADER_OPCODE_A64_OWORD_BLOCK_READ_LOGICAL:
      return "a64_oword_block_read_logical";
   case SHADER_OPCODE_A64_UNALIGNED_OWORD_BLOCK_READ_LOGICAL:
      return "a64_unaligned_oword_block_read_logical";
   case SHADER_OPCODE_A64_OWORD_BLOCK_WRITE_LOGICAL:
      return "a64_oword_block_write_logical";
   case SHADER_OPCODE_A64_UNTYPED_WRITE_LOGICAL:
      return "a64_untyped_write_logical";
   case SHADER_OPCODE_A64_BYTE_SCATTERED_READ_LOGICAL:
      return "a64_byte_scattered_read_logical";
   case SHADER_OPCODE_A64_BYTE_SCATTERED_WRITE_LOGICAL:
      return "a64_byte_scattered_write_logical";
   case SHADER_OPCODE_A64_UNTYPED_ATOMIC_LOGICAL:
      return "a64_untyped_atomic_logical";
   case SHADER_OPCODE_TYPED_ATOMIC_LOGICAL:
      return "typed_atomic_logical";
   case SHADER_OPCODE_TYPED_SURFACE_READ_LOGICAL:
      return "typed_surface_read_logical";
   case SHADER_OPCODE_TYPED_SURFACE_WRITE_LOGICAL:
      return "typed_surface_write_logical";
   case SHADER_OPCODE_MEMORY_FENCE:
      return "memory_fence";
   case FS_OPCODE_SCHEDULING_FENCE:
      return "scheduling_fence";
   case SHADER_OPCODE_INTERLOCK:
      /* For an interlock we actually issue a memory fence via sendc. */
      return "interlock";

   case SHADER_OPCODE_BYTE_SCATTERED_READ_LOGICAL:
      return "byte_scattered_read_logical";
   case SHADER_OPCODE_BYTE_SCATTERED_WRITE_LOGICAL:
      return "byte_scattered_write_logical";
   case SHADER_OPCODE_DWORD_SCATTERED_READ_LOGICAL:
      return "dword_scattered_read_logical";
   case SHADER_OPCODE_DWORD_SCATTERED_WRITE_LOGICAL:
      return "dword_scattered_write_logical";

   case SHADER_OPCODE_LOAD_PAYLOAD:
      return "load_payload";
   case FS_OPCODE_PACK:
      return "pack";

   case SHADER_OPCODE_GFX4_SCRATCH_READ:
      return "gfx4_scratch_read";
   case SHADER_OPCODE_GFX4_SCRATCH_WRITE:
      return "gfx4_scratch_write";
   case SHADER_OPCODE_GFX7_SCRATCH_READ:
      return "gfx7_scratch_read";
   case SHADER_OPCODE_SCRATCH_HEADER:
      return "scratch_header";

   case SHADER_OPCODE_URB_WRITE_LOGICAL:
      return "urb_write_logical";
   case SHADER_OPCODE_URB_READ_LOGICAL:
      return "urb_read_logical";

   case SHADER_OPCODE_FIND_LIVE_CHANNEL:
      return "find_live_channel";
   case SHADER_OPCODE_FIND_LAST_LIVE_CHANNEL:
      return "find_last_live_channel";
   case FS_OPCODE_LOAD_LIVE_CHANNELS:
      return "load_live_channels";

   case SHADER_OPCODE_BROADCAST:
      return "broadcast";
   case SHADER_OPCODE_SHUFFLE:
      return "shuffle";
   case SHADER_OPCODE_SEL_EXEC:
      return "sel_exec";
   case SHADER_OPCODE_QUAD_SWIZZLE:
      return "quad_swizzle";
   case SHADER_OPCODE_CLUSTER_BROADCAST:
      return "cluster_broadcast";

   case SHADER_OPCODE_GET_BUFFER_SIZE:
      return "get_buffer_size";

   case VEC4_OPCODE_MOV_BYTES:
      return "mov_bytes";
   case VEC4_OPCODE_PACK_BYTES:
      return "pack_bytes";
   case VEC4_OPCODE_UNPACK_UNIFORM:
      return "unpack_uniform";
   case VEC4_OPCODE_DOUBLE_TO_F32:
      return "double_to_f32";
   case VEC4_OPCODE_DOUBLE_TO_D32:
      return "double_to_d32";
   case VEC4_OPCODE_DOUBLE_TO_U32:
      return "double_to_u32";
   case VEC4_OPCODE_TO_DOUBLE:
      return "single_to_double";
   case VEC4_OPCODE_PICK_LOW_32BIT:
      return "pick_low_32bit";
   case VEC4_OPCODE_PICK_HIGH_32BIT:
      return "pick_high_32bit";
   case VEC4_OPCODE_SET_LOW_32BIT:
      return "set_low_32bit";
   case VEC4_OPCODE_SET_HIGH_32BIT:
      return "set_high_32bit";
   case VEC4_OPCODE_MOV_FOR_SCRATCH:
      return "mov_for_scratch";
   case VEC4_OPCODE_ZERO_OOB_PUSH_REGS:
      return "zero_oob_push_regs";

   case FS_OPCODE_DDX_COARSE:
      return "ddx_coarse";
   case FS_OPCODE_DDX_FINE:
      return "ddx_fine";
   case FS_OPCODE_DDY_COARSE:
      return "ddy_coarse";
   case FS_OPCODE_DDY_FINE:
      return "ddy_fine";

   case FS_OPCODE_LINTERP:
      return "linterp";

   case FS_OPCODE_PIXEL_X:
      return "pixel_x";
   case FS_OPCODE_PIXEL_Y:
      return "pixel_y";

   case FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD:
      return "uniform_pull_const";
   case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD_GFX4:
      return "varying_pull_const_gfx4";
   case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD_LOGICAL:
      return "varying_pull_const_logical";

   case FS_OPCODE_SET_SAMPLE_ID:
      return "set_sample_id";

   case FS_OPCODE_PACK_HALF_2x16_SPLIT:
      return "pack_half_2x16_split";

   case SHADER_OPCODE_HALT_TARGET:
      return "halt_target";

   case FS_OPCODE_INTERPOLATE_AT_SAMPLE:
      return "interp_sample";
   case FS_OPCODE_INTERPOLATE_AT_SHARED_OFFSET:
      return "interp_shared_offset";
   case FS_OPCODE_INTERPOLATE_AT_PER_SLOT_OFFSET:
      return "interp_per_slot_offset";

   case VEC4_VS_OPCODE_URB_WRITE:
      return "vs_urb_write";
   case VS_OPCODE_PULL_CONSTANT_LOAD:
      return "pull_constant_load";
   case VS_OPCODE_PULL_CONSTANT_LOAD_GFX7:
      return "pull_constant_load_gfx7";

   case VS_OPCODE_UNPACK_FLAGS_SIMD4X2:
      return "unpack_flags_simd4x2";

   case VEC4_GS_OPCODE_URB_WRITE:
      return "gs_urb_write";
   case VEC4_GS_OPCODE_URB_WRITE_ALLOCATE:
      return "gs_urb_write_allocate";
   case GS_OPCODE_THREAD_END:
      return "gs_thread_end";
   case GS_OPCODE_SET_WRITE_OFFSET:
      return "set_write_offset";
   case GS_OPCODE_SET_VERTEX_COUNT:
      return "set_vertex_count";
   case GS_OPCODE_SET_DWORD_2:
      return "set_dword_2";
   case GS_OPCODE_PREPARE_CHANNEL_MASKS:
      return "prepare_channel_masks";
   case GS_OPCODE_SET_CHANNEL_MASKS:
      return "set_channel_masks";
   case GS_OPCODE_GET_INSTANCE_ID:
      return "get_instance_id";
   case GS_OPCODE_FF_SYNC:
      return "ff_sync";
   case GS_OPCODE_SET_PRIMITIVE_ID:
      return "set_primitive_id";
   case GS_OPCODE_SVB_WRITE:
      return "gs_svb_write";
   case GS_OPCODE_SVB_SET_DST_INDEX:
      return "gs_svb_set_dst_index";
   case GS_OPCODE_FF_SYNC_SET_PRIMITIVES:
      return "gs_ff_sync_set_primitives";
   case CS_OPCODE_CS_TERMINATE:
      return "cs_terminate";
   case SHADER_OPCODE_BARRIER:
      return "barrier";
   case SHADER_OPCODE_MULH:
      return "mulh";
   case SHADER_OPCODE_ISUB_SAT:
      return "isub_sat";
   case SHADER_OPCODE_USUB_SAT:
      return "usub_sat";
   case SHADER_OPCODE_MOV_INDIRECT:
      return "mov_indirect";
   case SHADER_OPCODE_MOV_RELOC_IMM:
      return "mov_reloc_imm";

   case VEC4_OPCODE_URB_READ:
      return "urb_read";
   case TCS_OPCODE_GET_INSTANCE_ID:
      return "tcs_get_instance_id";
   case VEC4_TCS_OPCODE_URB_WRITE:
      return "tcs_urb_write";
   case VEC4_TCS_OPCODE_SET_INPUT_URB_OFFSETS:
      return "tcs_set_input_urb_offsets";
   case VEC4_TCS_OPCODE_SET_OUTPUT_URB_OFFSETS:
      return "tcs_set_output_urb_offsets";
   case TCS_OPCODE_GET_PRIMITIVE_ID:
      return "tcs_get_primitive_id";
   case TCS_OPCODE_CREATE_BARRIER_HEADER:
      return "tcs_create_barrier_header";
   case TCS_OPCODE_SRC0_010_IS_ZERO:
      return "tcs_src0<0,1,0>_is_zero";
   case TCS_OPCODE_RELEASE_INPUT:
      return "tcs_release_input";
   case TCS_OPCODE_THREAD_END:
      return "tcs_thread_end";
   case TES_OPCODE_CREATE_INPUT_READ_HEADER:
      return "tes_create_input_read_header";
   case TES_OPCODE_ADD_INDIRECT_URB_OFFSET:
      return "tes_add_indirect_urb_offset";
   case TES_OPCODE_GET_PRIMITIVE_ID:
      return "tes_get_primitive_id";

   case RT_OPCODE_TRACE_RAY_LOGICAL:
      return "rt_trace_ray_logical";

   case SHADER_OPCODE_RND_MODE:
      return "rnd_mode";
   case SHADER_OPCODE_FLOAT_CONTROL_MODE:
      return "float_control_mode";
   case SHADER_OPCODE_BTD_SPAWN_LOGICAL:
      return "btd_spawn_logical";
   case SHADER_OPCODE_BTD_RETIRE_LOGICAL:
      return "btd_retire_logical";
   case SHADER_OPCODE_READ_SR_REG:
      return "read_sr_reg";
   }

   unreachable("not reached");
}

bool
brw_saturate_immediate(enum brw_reg_type type, struct brw_reg *reg)
{
   union {
      unsigned ud;
      int d;
      float f;
      double df;
   } imm, sat_imm = { 0 };

   const unsigned size = type_sz(type);

   /* We want to either do a 32-bit or 64-bit data copy, the type is otherwise
    * irrelevant, so just check the size of the type and copy from/to an
    * appropriately sized field.
    */
   if (size < 8)
      imm.ud = reg->ud;
   else
      imm.df = reg->df;

   switch (type) {
   case BRW_REGISTER_TYPE_UD:
   case BRW_REGISTER_TYPE_D:
   case BRW_REGISTER_TYPE_UW:
   case BRW_REGISTER_TYPE_W:
   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_Q:
      /* Nothing to do. */
      return false;
   case BRW_REGISTER_TYPE_F:
      sat_imm.f = SATURATE(imm.f);
      break;
   case BRW_REGISTER_TYPE_DF:
      sat_imm.df = SATURATE(imm.df);
      break;
   case BRW_REGISTER_TYPE_UB:
   case BRW_REGISTER_TYPE_B:
      unreachable("no UB/B immediates");
   case BRW_REGISTER_TYPE_V:
   case BRW_REGISTER_TYPE_UV:
   case BRW_REGISTER_TYPE_VF:
      unreachable("unimplemented: saturate vector immediate");
   case BRW_REGISTER_TYPE_HF:
      unreachable("unimplemented: saturate HF immediate");
   case BRW_REGISTER_TYPE_NF:
      unreachable("no NF immediates");
   }

   if (size < 8) {
      if (imm.ud != sat_imm.ud) {
         reg->ud = sat_imm.ud;
         return true;
      }
   } else {
      if (imm.df != sat_imm.df) {
         reg->df = sat_imm.df;
         return true;
      }
   }
   return false;
}

bool
brw_negate_immediate(enum brw_reg_type type, struct brw_reg *reg)
{
   switch (type) {
   case BRW_REGISTER_TYPE_D:
   case BRW_REGISTER_TYPE_UD:
      reg->d = -reg->d;
      return true;
   case BRW_REGISTER_TYPE_W:
   case BRW_REGISTER_TYPE_UW: {
      uint16_t value = -(int16_t)reg->ud;
      reg->ud = value | (uint32_t)value << 16;
      return true;
   }
   case BRW_REGISTER_TYPE_F:
      reg->f = -reg->f;
      return true;
   case BRW_REGISTER_TYPE_VF:
      reg->ud ^= 0x80808080;
      return true;
   case BRW_REGISTER_TYPE_DF:
      reg->df = -reg->df;
      return true;
   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_Q:
      reg->d64 = -reg->d64;
      return true;
   case BRW_REGISTER_TYPE_UB:
   case BRW_REGISTER_TYPE_B:
      unreachable("no UB/B immediates");
   case BRW_REGISTER_TYPE_UV:
   case BRW_REGISTER_TYPE_V:
      assert(!"unimplemented: negate UV/V immediate");
   case BRW_REGISTER_TYPE_HF:
      reg->ud ^= 0x80008000;
      return true;
   case BRW_REGISTER_TYPE_NF:
      unreachable("no NF immediates");
   }

   return false;
}

bool
brw_abs_immediate(enum brw_reg_type type, struct brw_reg *reg)
{
   switch (type) {
   case BRW_REGISTER_TYPE_D:
      reg->d = abs(reg->d);
      return true;
   case BRW_REGISTER_TYPE_W: {
      uint16_t value = abs((int16_t)reg->ud);
      reg->ud = value | (uint32_t)value << 16;
      return true;
   }
   case BRW_REGISTER_TYPE_F:
      reg->f = fabsf(reg->f);
      return true;
   case BRW_REGISTER_TYPE_DF:
      reg->df = fabs(reg->df);
      return true;
   case BRW_REGISTER_TYPE_VF:
      reg->ud &= ~0x80808080;
      return true;
   case BRW_REGISTER_TYPE_Q:
      reg->d64 = imaxabs(reg->d64);
      return true;
   case BRW_REGISTER_TYPE_UB:
   case BRW_REGISTER_TYPE_B:
      unreachable("no UB/B immediates");
   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_UD:
   case BRW_REGISTER_TYPE_UW:
   case BRW_REGISTER_TYPE_UV:
      /* Presumably the absolute value modifier on an unsigned source is a
       * nop, but it would be nice to confirm.
       */
      assert(!"unimplemented: abs unsigned immediate");
   case BRW_REGISTER_TYPE_V:
      assert(!"unimplemented: abs V immediate");
   case BRW_REGISTER_TYPE_HF:
      reg->ud &= ~0x80008000;
      return true;
   case BRW_REGISTER_TYPE_NF:
      unreachable("no NF immediates");
   }

   return false;
}

backend_shader::backend_shader(const struct brw_compiler *compiler,
                               const struct brw_compile_params *params,
                               const nir_shader *shader,
                               struct brw_stage_prog_data *stage_prog_data,
                               bool debug_enabled)
   : compiler(compiler),
     log_data(params->log_data),
     devinfo(compiler->devinfo),
     nir(shader),
     stage_prog_data(stage_prog_data),
     mem_ctx(params->mem_ctx),
     cfg(NULL), idom_analysis(this),
     stage(shader->info.stage),
     debug_enabled(debug_enabled)
{
}

backend_shader::~backend_shader()
{
}

bool
backend_reg::equals(const backend_reg &r) const
{
   return brw_regs_equal(this, &r) && offset == r.offset;
}

bool
backend_reg::negative_equals(const backend_reg &r) const
{
   return brw_regs_negative_equal(this, &r) && offset == r.offset;
}

bool
backend_reg::is_zero() const
{
   if (file != IMM)
      return false;

   assert(type_sz(type) > 1);

   switch (type) {
   case BRW_REGISTER_TYPE_HF:
      assert((d & 0xffff) == ((d >> 16) & 0xffff));
      return (d & 0xffff) == 0 || (d & 0xffff) == 0x8000;
   case BRW_REGISTER_TYPE_F:
      return f == 0;
   case BRW_REGISTER_TYPE_DF:
      return df == 0;
   case BRW_REGISTER_TYPE_W:
   case BRW_REGISTER_TYPE_UW:
      assert((d & 0xffff) == ((d >> 16) & 0xffff));
      return (d & 0xffff) == 0;
   case BRW_REGISTER_TYPE_D:
   case BRW_REGISTER_TYPE_UD:
      return d == 0;
   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_Q:
      return u64 == 0;
   default:
      return false;
   }
}

bool
backend_reg::is_one() const
{
   if (file != IMM)
      return false;

   assert(type_sz(type) > 1);

   switch (type) {
   case BRW_REGISTER_TYPE_HF:
      assert((d & 0xffff) == ((d >> 16) & 0xffff));
      return (d & 0xffff) == 0x3c00;
   case BRW_REGISTER_TYPE_F:
      return f == 1.0f;
   case BRW_REGISTER_TYPE_DF:
      return df == 1.0;
   case BRW_REGISTER_TYPE_W:
   case BRW_REGISTER_TYPE_UW:
      assert((d & 0xffff) == ((d >> 16) & 0xffff));
      return (d & 0xffff) == 1;
   case BRW_REGISTER_TYPE_D:
   case BRW_REGISTER_TYPE_UD:
      return d == 1;
   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_Q:
      return u64 == 1;
   default:
      return false;
   }
}

bool
backend_reg::is_negative_one() const
{
   if (file != IMM)
      return false;

   assert(type_sz(type) > 1);

   switch (type) {
   case BRW_REGISTER_TYPE_HF:
      assert((d & 0xffff) == ((d >> 16) & 0xffff));
      return (d & 0xffff) == 0xbc00;
   case BRW_REGISTER_TYPE_F:
      return f == -1.0;
   case BRW_REGISTER_TYPE_DF:
      return df == -1.0;
   case BRW_REGISTER_TYPE_W:
      assert((d & 0xffff) == ((d >> 16) & 0xffff));
      return (d & 0xffff) == 0xffff;
   case BRW_REGISTER_TYPE_D:
      return d == -1;
   case BRW_REGISTER_TYPE_Q:
      return d64 == -1;
   default:
      return false;
   }
}

bool
backend_reg::is_null() const
{
   return file == ARF && nr == BRW_ARF_NULL;
}


bool
backend_reg::is_accumulator() const
{
   return file == ARF && nr == BRW_ARF_ACCUMULATOR;
}

bool
backend_instruction::is_commutative() const
{
   switch (opcode) {
   case BRW_OPCODE_AND:
   case BRW_OPCODE_OR:
   case BRW_OPCODE_XOR:
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_ADD3:
   case BRW_OPCODE_MUL:
   case SHADER_OPCODE_MULH:
      return true;
   case BRW_OPCODE_SEL:
      /* MIN and MAX are commutative. */
      if (conditional_mod == BRW_CONDITIONAL_GE ||
          conditional_mod == BRW_CONDITIONAL_L) {
         return true;
      }
      FALLTHROUGH;
   default:
      return false;
   }
}

bool
backend_instruction::is_3src(const struct brw_compiler *compiler) const
{
   return ::is_3src(&compiler->isa, opcode);
}

bool
backend_instruction::is_math() const
{
   return (opcode == SHADER_OPCODE_RCP ||
           opcode == SHADER_OPCODE_RSQ ||
           opcode == SHADER_OPCODE_SQRT ||
           opcode == SHADER_OPCODE_EXP2 ||
           opcode == SHADER_OPCODE_LOG2 ||
           opcode == SHADER_OPCODE_SIN ||
           opcode == SHADER_OPCODE_COS ||
           opcode == SHADER_OPCODE_INT_QUOTIENT ||
           opcode == SHADER_OPCODE_INT_REMAINDER ||
           opcode == SHADER_OPCODE_POW);
}

bool
backend_instruction::is_control_flow_begin() const
{
   switch (opcode) {
   case BRW_OPCODE_DO:
   case BRW_OPCODE_IF:
   case BRW_OPCODE_ELSE:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::is_control_flow_end() const
{
   switch (opcode) {
   case BRW_OPCODE_ELSE:
   case BRW_OPCODE_WHILE:
   case BRW_OPCODE_ENDIF:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::is_control_flow() const
{
   switch (opcode) {
   case BRW_OPCODE_DO:
   case BRW_OPCODE_WHILE:
   case BRW_OPCODE_IF:
   case BRW_OPCODE_ELSE:
   case BRW_OPCODE_ENDIF:
   case BRW_OPCODE_BREAK:
   case BRW_OPCODE_CONTINUE:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::uses_indirect_addressing() const
{
   switch (opcode) {
   case SHADER_OPCODE_BROADCAST:
   case SHADER_OPCODE_CLUSTER_BROADCAST:
   case SHADER_OPCODE_MOV_INDIRECT:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::can_do_source_mods() const
{
   switch (opcode) {
   case BRW_OPCODE_ADDC:
   case BRW_OPCODE_BFE:
   case BRW_OPCODE_BFI1:
   case BRW_OPCODE_BFI2:
   case BRW_OPCODE_BFREV:
   case BRW_OPCODE_CBIT:
   case BRW_OPCODE_FBH:
   case BRW_OPCODE_FBL:
   case BRW_OPCODE_ROL:
   case BRW_OPCODE_ROR:
   case BRW_OPCODE_SUBB:
   case BRW_OPCODE_DP4A:
   case BRW_OPCODE_DPAS:
   case SHADER_OPCODE_BROADCAST:
   case SHADER_OPCODE_CLUSTER_BROADCAST:
   case SHADER_OPCODE_MOV_INDIRECT:
   case SHADER_OPCODE_SHUFFLE:
   case SHADER_OPCODE_INT_QUOTIENT:
   case SHADER_OPCODE_INT_REMAINDER:
      return false;
   default:
      return true;
   }
}

bool
backend_instruction::can_do_saturate() const
{
   switch (opcode) {
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_ADD3:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_AVG:
   case BRW_OPCODE_CSEL:
   case BRW_OPCODE_DP2:
   case BRW_OPCODE_DP3:
   case BRW_OPCODE_DP4:
   case BRW_OPCODE_DPH:
   case BRW_OPCODE_DP4A:
   case BRW_OPCODE_F16TO32:
   case BRW_OPCODE_F32TO16:
   case BRW_OPCODE_LINE:
   case BRW_OPCODE_LRP:
   case BRW_OPCODE_MAC:
   case BRW_OPCODE_MAD:
   case BRW_OPCODE_MATH:
   case BRW_OPCODE_MOV:
   case BRW_OPCODE_MUL:
   case SHADER_OPCODE_MULH:
   case BRW_OPCODE_PLN:
   case BRW_OPCODE_RNDD:
   case BRW_OPCODE_RNDE:
   case BRW_OPCODE_RNDU:
   case BRW_OPCODE_RNDZ:
   case BRW_OPCODE_SEL:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_SHR:
   case FS_OPCODE_LINTERP:
   case SHADER_OPCODE_COS:
   case SHADER_OPCODE_EXP2:
   case SHADER_OPCODE_LOG2:
   case SHADER_OPCODE_POW:
   case SHADER_OPCODE_RCP:
   case SHADER_OPCODE_RSQ:
   case SHADER_OPCODE_SIN:
   case SHADER_OPCODE_SQRT:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::can_do_cmod() const
{
   switch (opcode) {
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_ADD3:
   case BRW_OPCODE_ADDC:
   case BRW_OPCODE_AND:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_AVG:
   case BRW_OPCODE_CMP:
   case BRW_OPCODE_CMPN:
   case BRW_OPCODE_DP2:
   case BRW_OPCODE_DP3:
   case BRW_OPCODE_DP4:
   case BRW_OPCODE_DPH:
   case BRW_OPCODE_F16TO32:
   case BRW_OPCODE_F32TO16:
   case BRW_OPCODE_FRC:
   case BRW_OPCODE_LINE:
   case BRW_OPCODE_LRP:
   case BRW_OPCODE_LZD:
   case BRW_OPCODE_MAC:
   case BRW_OPCODE_MACH:
   case BRW_OPCODE_MAD:
   case BRW_OPCODE_MOV:
   case BRW_OPCODE_MUL:
   case BRW_OPCODE_NOT:
   case BRW_OPCODE_OR:
   case BRW_OPCODE_PLN:
   case BRW_OPCODE_RNDD:
   case BRW_OPCODE_RNDE:
   case BRW_OPCODE_RNDU:
   case BRW_OPCODE_RNDZ:
   case BRW_OPCODE_SAD2:
   case BRW_OPCODE_SADA2:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_SHR:
   case BRW_OPCODE_SUBB:
   case BRW_OPCODE_XOR:
   case FS_OPCODE_LINTERP:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::reads_accumulator_implicitly() const
{
   switch (opcode) {
   case BRW_OPCODE_MAC:
   case BRW_OPCODE_MACH:
   case BRW_OPCODE_SADA2:
      return true;
   default:
      return false;
   }
}

bool
backend_instruction::writes_accumulator_implicitly(const struct intel_device_info *devinfo) const
{
   return writes_accumulator ||
          (devinfo->ver < 6 &&
           ((opcode >= BRW_OPCODE_ADD && opcode < BRW_OPCODE_NOP) ||
            (opcode >= FS_OPCODE_DDX_COARSE && opcode <= FS_OPCODE_LINTERP))) ||
          (opcode == FS_OPCODE_LINTERP &&
           (!devinfo->has_pln || devinfo->ver <= 6)) ||
          (eot && intel_needs_workaround(devinfo, 14010017096));
}

bool
backend_instruction::has_side_effects() const
{
   switch (opcode) {
   case SHADER_OPCODE_SEND:
      return send_has_side_effects;

   case BRW_OPCODE_SYNC:
   case VEC4_OPCODE_UNTYPED_ATOMIC:
   case SHADER_OPCODE_UNTYPED_ATOMIC_LOGICAL:
   case SHADER_OPCODE_GFX4_SCRATCH_WRITE:
   case VEC4_OPCODE_UNTYPED_SURFACE_WRITE:
   case SHADER_OPCODE_UNTYPED_SURFACE_WRITE_LOGICAL:
   case SHADER_OPCODE_A64_UNTYPED_WRITE_LOGICAL:
   case SHADER_OPCODE_A64_BYTE_SCATTERED_WRITE_LOGICAL:
   case SHADER_OPCODE_A64_UNTYPED_ATOMIC_LOGICAL:
   case SHADER_OPCODE_BYTE_SCATTERED_WRITE_LOGICAL:
   case SHADER_OPCODE_DWORD_SCATTERED_WRITE_LOGICAL:
   case SHADER_OPCODE_TYPED_ATOMIC_LOGICAL:
   case SHADER_OPCODE_TYPED_SURFACE_WRITE_LOGICAL:
   case SHADER_OPCODE_MEMORY_FENCE:
   case SHADER_OPCODE_INTERLOCK:
   case SHADER_OPCODE_URB_WRITE_LOGICAL:
   case FS_OPCODE_FB_WRITE:
   case FS_OPCODE_FB_WRITE_LOGICAL:
   case FS_OPCODE_REP_FB_WRITE:
   case SHADER_OPCODE_BARRIER:
   case VEC4_TCS_OPCODE_URB_WRITE:
   case TCS_OPCODE_RELEASE_INPUT:
   case SHADER_OPCODE_RND_MODE:
   case SHADER_OPCODE_FLOAT_CONTROL_MODE:
   case FS_OPCODE_SCHEDULING_FENCE:
   case SHADER_OPCODE_OWORD_BLOCK_WRITE_LOGICAL:
   case SHADER_OPCODE_A64_OWORD_BLOCK_WRITE_LOGICAL:
   case SHADER_OPCODE_BTD_SPAWN_LOGICAL:
   case SHADER_OPCODE_BTD_RETIRE_LOGICAL:
   case RT_OPCODE_TRACE_RAY_LOGICAL:
   case VEC4_OPCODE_ZERO_OOB_PUSH_REGS:
      return true;
   default:
      return eot;
   }
}

bool
backend_instruction::is_volatile() const
{
   switch (opcode) {
   case SHADER_OPCODE_SEND:
      return send_is_volatile;

   case VEC4_OPCODE_UNTYPED_SURFACE_READ:
   case SHADER_OPCODE_UNTYPED_SURFACE_READ_LOGICAL:
   case SHADER_OPCODE_TYPED_SURFACE_READ_LOGICAL:
   case SHADER_OPCODE_BYTE_SCATTERED_READ_LOGICAL:
   case SHADER_OPCODE_DWORD_SCATTERED_READ_LOGICAL:
   case SHADER_OPCODE_A64_UNTYPED_READ_LOGICAL:
   case SHADER_OPCODE_A64_BYTE_SCATTERED_READ_LOGICAL:
   case VEC4_OPCODE_URB_READ:
      return true;
   default:
      return false;
   }
}

#ifndef NDEBUG
static bool
inst_is_in_block(const bblock_t *block, const backend_instruction *inst)
{
   const exec_node *n = inst;

   /* Find the tail sentinel. If the tail sentinel is the sentinel from the
    * list header in the bblock_t, then this instruction is in that basic
    * block.
    */
   while (!n->is_tail_sentinel())
      n = n->get_next();

   return n == &block->instructions.tail_sentinel;
}
#endif

static void
adjust_later_block_ips(bblock_t *start_block, int ip_adjustment)
{
   for (bblock_t *block_iter = start_block->next();
        block_iter;
        block_iter = block_iter->next()) {
      block_iter->start_ip += ip_adjustment;
      block_iter->end_ip += ip_adjustment;
   }
}

void
backend_instruction::insert_after(bblock_t *block, backend_instruction *inst)
{
   assert(this != inst);
   assert(block->end_ip_delta == 0);

   if (!this->is_head_sentinel())
      assert(inst_is_in_block(block, this) || !"Instruction not in block");

   block->end_ip++;

   adjust_later_block_ips(block, 1);

   exec_node::insert_after(inst);
}

void
backend_instruction::insert_before(bblock_t *block, backend_instruction *inst)
{
   assert(this != inst);
   assert(block->end_ip_delta == 0);

   if (!this->is_tail_sentinel())
      assert(inst_is_in_block(block, this) || !"Instruction not in block");

   block->end_ip++;

   adjust_later_block_ips(block, 1);

   exec_node::insert_before(inst);
}

void
backend_instruction::remove(bblock_t *block, bool defer_later_block_ip_updates)
{
   assert(inst_is_in_block(block, this) || !"Instruction not in block");

   if (defer_later_block_ip_updates) {
      block->end_ip_delta--;
   } else {
      assert(block->end_ip_delta == 0);
      adjust_later_block_ips(block, -1);
   }

   if (block->start_ip == block->end_ip) {
      if (block->end_ip_delta != 0) {
         adjust_later_block_ips(block, block->end_ip_delta);
         block->end_ip_delta = 0;
      }

      block->cfg->remove_block(block);
   } else {
      block->end_ip--;
   }

   exec_node::remove();
}

void
backend_shader::dump_instructions(const char *name) const
{
   FILE *file = stderr;
   if (name && __normal_user()) {
      file = fopen(name, "w");
      if (!file)
         file = stderr;
   }

   dump_instructions_to_file(file);

   if (file != stderr) {
      fclose(file);
   }
}

void
backend_shader::dump_instructions_to_file(FILE *file) const
{
   if (cfg) {
      int ip = 0;
      foreach_block_and_inst(block, backend_instruction, inst, cfg) {
         if (!INTEL_DEBUG(DEBUG_OPTIMIZER))
            fprintf(file, "%4d: ", ip++);
         dump_instruction(inst, file);
      }
   } else {
      int ip = 0;
      foreach_in_list(backend_instruction, inst, &instructions) {
         if (!INTEL_DEBUG(DEBUG_OPTIMIZER))
            fprintf(file, "%4d: ", ip++);
         dump_instruction(inst, file);
      }
   }
}

void
backend_shader::calculate_cfg()
{
   if (this->cfg)
      return;
   cfg = new(mem_ctx) cfg_t(this, &this->instructions);
}

void
backend_shader::invalidate_analysis(brw::analysis_dependency_class c)
{
   idom_analysis.invalidate(c);
}

extern "C" const unsigned *
brw_compile_tes(const struct brw_compiler *compiler,
                brw_compile_tes_params *params)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   nir_shader *nir = params->base.nir;
   const struct brw_tes_prog_key *key = params->key;
   const struct brw_vue_map *input_vue_map = params->input_vue_map;
   struct brw_tes_prog_data *prog_data = params->prog_data;

   const bool is_scalar = compiler->scalar_stage[MESA_SHADER_TESS_EVAL];
   const bool debug_enabled = brw_should_print_shader(nir, DEBUG_TES);
   const unsigned *assembly;

   prog_data->base.base.stage = MESA_SHADER_TESS_EVAL;
   prog_data->base.base.ray_queries = nir->info.ray_queries;

   nir->info.inputs_read = key->inputs_read;
   nir->info.patch_inputs_read = key->patch_inputs_read;

   brw_nir_apply_key(nir, compiler, &key->base, 8);
   brw_nir_lower_tes_inputs(nir, input_vue_map);
   brw_nir_lower_vue_outputs(nir);
   brw_postprocess_nir(nir, compiler, debug_enabled,
                       key->base.robust_flags);

   brw_compute_vue_map(devinfo, &prog_data->base.vue_map,
                       nir->info.outputs_written,
                       nir->info.separate_shader, 1);

   unsigned output_size_bytes = prog_data->base.vue_map.num_slots * 4 * 4;

   assert(output_size_bytes >= 1);
   if (output_size_bytes > GFX7_MAX_DS_URB_ENTRY_SIZE_BYTES) {
      params->base.error_str = ralloc_strdup(params->base.mem_ctx,
                                             "DS outputs exceed maximum size");
      return NULL;
   }

   prog_data->base.clip_distance_mask =
      ((1 << nir->info.clip_distance_array_size) - 1);
   prog_data->base.cull_distance_mask =
      ((1 << nir->info.cull_distance_array_size) - 1) <<
      nir->info.clip_distance_array_size;

   prog_data->include_primitive_id =
      BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_PRIMITIVE_ID);

   /* URB entry sizes are stored as a multiple of 64 bytes. */
   prog_data->base.urb_entry_size = ALIGN(output_size_bytes, 64) / 64;

   prog_data->base.urb_read_length = 0;

   STATIC_ASSERT(BRW_TESS_PARTITIONING_INTEGER == TESS_SPACING_EQUAL - 1);
   STATIC_ASSERT(BRW_TESS_PARTITIONING_ODD_FRACTIONAL ==
                 TESS_SPACING_FRACTIONAL_ODD - 1);
   STATIC_ASSERT(BRW_TESS_PARTITIONING_EVEN_FRACTIONAL ==
                 TESS_SPACING_FRACTIONAL_EVEN - 1);

   prog_data->partitioning =
      (enum brw_tess_partitioning) (nir->info.tess.spacing - 1);

   switch (nir->info.tess._primitive_mode) {
   case TESS_PRIMITIVE_QUADS:
      prog_data->domain = BRW_TESS_DOMAIN_QUAD;
      break;
   case TESS_PRIMITIVE_TRIANGLES:
      prog_data->domain = BRW_TESS_DOMAIN_TRI;
      break;
   case TESS_PRIMITIVE_ISOLINES:
      prog_data->domain = BRW_TESS_DOMAIN_ISOLINE;
      break;
   default:
      unreachable("invalid domain shader primitive mode");
   }

   if (nir->info.tess.point_mode) {
      prog_data->output_topology = BRW_TESS_OUTPUT_TOPOLOGY_POINT;
   } else if (nir->info.tess._primitive_mode == TESS_PRIMITIVE_ISOLINES) {
      prog_data->output_topology = BRW_TESS_OUTPUT_TOPOLOGY_LINE;
   } else {
      /* Hardware winding order is backwards from OpenGL */
      prog_data->output_topology =
         nir->info.tess.ccw ? BRW_TESS_OUTPUT_TOPOLOGY_TRI_CW
                             : BRW_TESS_OUTPUT_TOPOLOGY_TRI_CCW;
   }

   if (unlikely(debug_enabled)) {
      fprintf(stderr, "TES Input ");
      brw_print_vue_map(stderr, input_vue_map, MESA_SHADER_TESS_EVAL);
      fprintf(stderr, "TES Output ");
      brw_print_vue_map(stderr, &prog_data->base.vue_map,
                        MESA_SHADER_TESS_EVAL);
   }

   if (is_scalar) {
      const unsigned dispatch_width = devinfo->ver >= 20 ? 16 : 8;
      fs_visitor v(compiler, &params->base, &key->base,
                   &prog_data->base.base, nir, dispatch_width,
                   params->base.stats != NULL, debug_enabled);
      if (!v.run_tes()) {
         params->base.error_str =
            ralloc_strdup(params->base.mem_ctx, v.fail_msg);
         return NULL;
      }

      assert(v.payload().num_regs % reg_unit(devinfo) == 0);
      prog_data->base.base.dispatch_grf_start_reg = v.payload().num_regs / reg_unit(devinfo);

      prog_data->base.dispatch_mode = DISPATCH_MODE_SIMD8;

      fs_generator g(compiler, &params->base,
                     &prog_data->base.base, false, MESA_SHADER_TESS_EVAL);
      if (unlikely(debug_enabled)) {
         g.enable_debug(ralloc_asprintf(params->base.mem_ctx,
                                        "%s tessellation evaluation shader %s",
                                        nir->info.label ? nir->info.label
                                                        : "unnamed",
                                        nir->info.name));
      }

      g.generate_code(v.cfg, dispatch_width, v.shader_stats,
                      v.performance_analysis.require(), params->base.stats);

      g.add_const_data(nir->constant_data, nir->constant_data_size);

      assembly = g.get_assembly();
   } else {
      brw::vec4_tes_visitor v(compiler, &params->base, key, prog_data,
                              nir, debug_enabled);
      if (!v.run()) {
         params->base.error_str =
            ralloc_strdup(params->base.mem_ctx, v.fail_msg);
	 return NULL;
      }

      if (unlikely(debug_enabled))
	 v.dump_instructions();

      assembly = brw_vec4_generate_assembly(compiler, &params->base, nir,
                                            &prog_data->base, v.cfg,
                                            v.performance_analysis.require(),
                                            debug_enabled);
   }

   return assembly;
}
