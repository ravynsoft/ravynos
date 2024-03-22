/* Copyright © 2011 Intel Corporation
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

#include "brw_vec4.h"
#include "brw_cfg.h"
#include "brw_eu.h"
#include "dev/intel_debug.h"
#include "util/mesa-sha1.h"

using namespace brw;

static void
generate_math1_gfx4(struct brw_codegen *p,
                    vec4_instruction *inst,
                    struct brw_reg dst,
                    struct brw_reg src)
{
   gfx4_math(p,
	     dst,
	     brw_math_function(inst->opcode),
	     inst->base_mrf,
	     src,
	     BRW_MATH_PRECISION_FULL);
}

static void
check_gfx6_math_src_arg(struct brw_reg src)
{
   /* Source swizzles are ignored. */
   assert(!src.abs);
   assert(!src.negate);
   assert(src.swizzle == BRW_SWIZZLE_XYZW);
}

static void
generate_math_gfx6(struct brw_codegen *p,
                   vec4_instruction *inst,
                   struct brw_reg dst,
                   struct brw_reg src0,
                   struct brw_reg src1)
{
   /* Can't do writemask because math can't be align16. */
   assert(dst.writemask == WRITEMASK_XYZW);
   /* Source swizzles are ignored. */
   check_gfx6_math_src_arg(src0);
   if (src1.file == BRW_GENERAL_REGISTER_FILE)
      check_gfx6_math_src_arg(src1);

   brw_set_default_access_mode(p, BRW_ALIGN_1);
   gfx6_math(p, dst, brw_math_function(inst->opcode), src0, src1);
   brw_set_default_access_mode(p, BRW_ALIGN_16);
}

static void
generate_math2_gfx4(struct brw_codegen *p,
                    vec4_instruction *inst,
                    struct brw_reg dst,
                    struct brw_reg src0,
                    struct brw_reg src1)
{
   /* From the Ironlake PRM, Volume 4, Part 1, Section 6.1.13
    * "Message Payload":
    *
    * "Operand0[7].  For the INT DIV functions, this operand is the
    *  denominator."
    *  ...
    * "Operand1[7].  For the INT DIV functions, this operand is the
    *  numerator."
    */
   bool is_int_div = inst->opcode != SHADER_OPCODE_POW;
   struct brw_reg &op0 = is_int_div ? src1 : src0;
   struct brw_reg &op1 = is_int_div ? src0 : src1;

   brw_push_insn_state(p);
   brw_set_default_saturate(p, false);
   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
   brw_set_default_flag_reg(p, 0, 0);
   brw_MOV(p, retype(brw_message_reg(inst->base_mrf + 1), op1.type), op1);
   brw_pop_insn_state(p);

   gfx4_math(p,
	     dst,
	     brw_math_function(inst->opcode),
	     inst->base_mrf,
	     op0,
	     BRW_MATH_PRECISION_FULL);
}

static void
generate_tex(struct brw_codegen *p,
             struct brw_vue_prog_data *prog_data,
             gl_shader_stage stage,
             vec4_instruction *inst,
             struct brw_reg dst,
             struct brw_reg src,
             struct brw_reg surface_index,
             struct brw_reg sampler_index)
{
   const struct intel_device_info *devinfo = p->devinfo;
   int msg_type = -1;

   if (devinfo->ver >= 5) {
      switch (inst->opcode) {
      case SHADER_OPCODE_TEX:
      case SHADER_OPCODE_TXL:
	 if (inst->shadow_compare) {
	    msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LOD_COMPARE;
	 } else {
	    msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LOD;
	 }
	 break;
      case SHADER_OPCODE_TXD:
         if (inst->shadow_compare) {
            /* Gfx7.5+.  Otherwise, lowered by brw_lower_texture_gradients(). */
            assert(devinfo->verx10 == 75);
            msg_type = HSW_SAMPLER_MESSAGE_SAMPLE_DERIV_COMPARE;
         } else {
            msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_DERIVS;
         }
	 break;
      case SHADER_OPCODE_TXF:
	 msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LD;
	 break;
      case SHADER_OPCODE_TXF_CMS:
         if (devinfo->ver >= 7)
            msg_type = GFX7_SAMPLER_MESSAGE_SAMPLE_LD2DMS;
         else
            msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LD;
         break;
      case SHADER_OPCODE_TXF_MCS:
         assert(devinfo->ver >= 7);
         msg_type = GFX7_SAMPLER_MESSAGE_SAMPLE_LD_MCS;
         break;
      case SHADER_OPCODE_TXS:
	 msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_RESINFO;
	 break;
      case SHADER_OPCODE_TG4:
         if (inst->shadow_compare) {
            msg_type = GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4_C;
         } else {
            msg_type = GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4;
         }
         break;
      case SHADER_OPCODE_TG4_OFFSET:
         if (inst->shadow_compare) {
            msg_type = GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4_PO_C;
         } else {
            msg_type = GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4_PO;
         }
         break;
      case SHADER_OPCODE_SAMPLEINFO:
         msg_type = GFX6_SAMPLER_MESSAGE_SAMPLE_SAMPLEINFO;
         break;
      default:
	 unreachable("should not get here: invalid vec4 texture opcode");
      }
   } else {
      switch (inst->opcode) {
      case SHADER_OPCODE_TEX:
      case SHADER_OPCODE_TXL:
	 if (inst->shadow_compare) {
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_LOD_COMPARE;
	    assert(inst->mlen == 3);
	 } else {
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_LOD;
	    assert(inst->mlen == 2);
	 }
	 break;
      case SHADER_OPCODE_TXD:
	 /* There is no sample_d_c message; comparisons are done manually. */
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_GRADIENTS;
	 assert(inst->mlen == 4);
	 break;
      case SHADER_OPCODE_TXF:
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD4X2_LD;
	 assert(inst->mlen == 2);
	 break;
      case SHADER_OPCODE_TXS:
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD4X2_RESINFO;
	 assert(inst->mlen == 2);
	 break;
      default:
	 unreachable("should not get here: invalid vec4 texture opcode");
      }
   }

   assert(msg_type != -1);

   assert(sampler_index.type == BRW_REGISTER_TYPE_UD);

   /* Load the message header if present.  If there's a texture offset, we need
    * to set it up explicitly and load the offset bitfield.  Otherwise, we can
    * use an implied move from g0 to the first message register.
    */
   if (inst->header_size != 0) {
      if (devinfo->ver < 6 && !inst->offset) {
         /* Set up an implied move from g0 to the MRF. */
         src = brw_vec8_grf(0, 0);
      } else {
         struct brw_reg header =
            retype(brw_message_reg(inst->base_mrf), BRW_REGISTER_TYPE_UD);
         uint32_t dw2 = 0;

         /* Explicitly set up the message header by copying g0 to the MRF. */
         brw_push_insn_state(p);
         brw_set_default_mask_control(p, BRW_MASK_DISABLE);
         brw_MOV(p, header, retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));

         brw_set_default_access_mode(p, BRW_ALIGN_1);

         if (inst->offset)
            /* Set the texel offset bits in DWord 2. */
            dw2 = inst->offset;

         /* The VS, DS, and FS stages have the g0.2 payload delivered as 0,
          * so header0.2 is 0 when g0 is copied.  The HS and GS stages do
          * not, so we must set to to 0 to avoid setting undesirable bits
          * in the message header.
          */
         if (dw2 ||
             stage == MESA_SHADER_TESS_CTRL ||
             stage == MESA_SHADER_GEOMETRY) {
            brw_MOV(p, get_element_ud(header, 2), brw_imm_ud(dw2));
         }

         brw_adjust_sampler_state_pointer(p, header, sampler_index);
         brw_pop_insn_state(p);
      }
   }

   uint32_t return_format;

   switch (dst.type) {
   case BRW_REGISTER_TYPE_D:
      return_format = BRW_SAMPLER_RETURN_FORMAT_SINT32;
      break;
   case BRW_REGISTER_TYPE_UD:
      return_format = BRW_SAMPLER_RETURN_FORMAT_UINT32;
      break;
   default:
      return_format = BRW_SAMPLER_RETURN_FORMAT_FLOAT32;
      break;
   }

   /* Stomp the resinfo output type to UINT32.  On gens 4-5, the output type
    * is set as part of the message descriptor.  On gfx4, the PRM seems to
    * allow UINT32 and FLOAT32 (i965 PRM, Vol. 4 Section 4.8.1.1), but on
    * later gens UINT32 is required.  Once you hit Sandy Bridge, the bit is
    * gone from the message descriptor entirely and you just get UINT32 all
    * the time regasrdless.  Since we can really only do non-UINT32 on gfx4,
    * just stomp it to UINT32 all the time.
    */
   if (inst->opcode == SHADER_OPCODE_TXS)
      return_format = BRW_SAMPLER_RETURN_FORMAT_UINT32;

   if (surface_index.file == BRW_IMMEDIATE_VALUE &&
       sampler_index.file == BRW_IMMEDIATE_VALUE) {
      uint32_t surface = surface_index.ud;
      uint32_t sampler = sampler_index.ud;

      brw_SAMPLE(p,
                 dst,
                 inst->base_mrf,
                 src,
                 surface,
                 sampler % 16,
                 msg_type,
                 1, /* response length */
                 inst->mlen,
                 inst->header_size != 0,
                 BRW_SAMPLER_SIMD_MODE_SIMD4X2,
                 return_format);
   } else {
      /* Non-constant sampler index. */

      struct brw_reg addr = vec1(retype(brw_address_reg(0), BRW_REGISTER_TYPE_UD));
      struct brw_reg surface_reg = vec1(retype(surface_index, BRW_REGISTER_TYPE_UD));
      struct brw_reg sampler_reg = vec1(retype(sampler_index, BRW_REGISTER_TYPE_UD));

      brw_push_insn_state(p);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_access_mode(p, BRW_ALIGN_1);

      if (brw_regs_equal(&surface_reg, &sampler_reg)) {
         brw_MUL(p, addr, sampler_reg, brw_imm_uw(0x101));
      } else {
         if (sampler_reg.file == BRW_IMMEDIATE_VALUE) {
            brw_OR(p, addr, surface_reg, brw_imm_ud(sampler_reg.ud << 8));
         } else {
            brw_SHL(p, addr, sampler_reg, brw_imm_ud(8));
            brw_OR(p, addr, addr, surface_reg);
         }
      }
      brw_AND(p, addr, addr, brw_imm_ud(0xfff));

      brw_pop_insn_state(p);

      if (inst->base_mrf != -1)
         gfx6_resolve_implied_move(p, &src, inst->base_mrf);

      /* dst = send(offset, a0.0 | <descriptor>) */
      brw_send_indirect_message(
         p, BRW_SFID_SAMPLER, dst, src, addr,
         brw_message_desc(devinfo, inst->mlen, 1, inst->header_size) |
         brw_sampler_desc(devinfo,
                          0 /* surface */,
                          0 /* sampler */,
                          msg_type,
                          BRW_SAMPLER_SIMD_MODE_SIMD4X2,
                          return_format),
         false /* EOT */);

      /* visitor knows more than we do about the surface limit required,
       * so has already done marking.
       */
   }
}

static void
generate_vs_urb_write(struct brw_codegen *p, vec4_instruction *inst)
{
   brw_urb_WRITE(p,
		 brw_null_reg(), /* dest */
		 inst->base_mrf, /* starting mrf reg nr */
		 brw_vec8_grf(0, 0), /* src */
                 inst->urb_write_flags,
		 inst->mlen,
		 0,		/* response len */
		 inst->offset,	/* urb destination offset */
		 BRW_URB_SWIZZLE_INTERLEAVE);
}

static void
generate_gs_urb_write(struct brw_codegen *p, vec4_instruction *inst)
{
   struct brw_reg src = brw_message_reg(inst->base_mrf);
   brw_urb_WRITE(p,
                 brw_null_reg(), /* dest */
                 inst->base_mrf, /* starting mrf reg nr */
                 src,
                 inst->urb_write_flags,
                 inst->mlen,
                 0,             /* response len */
                 inst->offset,  /* urb destination offset */
                 BRW_URB_SWIZZLE_INTERLEAVE);
}

static void
generate_gs_urb_write_allocate(struct brw_codegen *p, vec4_instruction *inst)
{
   struct brw_reg src = brw_message_reg(inst->base_mrf);

   /* We pass the temporary passed in src0 as the writeback register */
   brw_urb_WRITE(p,
                 inst->src[0].as_brw_reg(), /* dest */
                 inst->base_mrf, /* starting mrf reg nr */
                 src,
                 BRW_URB_WRITE_ALLOCATE_COMPLETE,
                 inst->mlen,
                 1, /* response len */
                 inst->offset,  /* urb destination offset */
                 BRW_URB_SWIZZLE_INTERLEAVE);

   /* Now put allocated urb handle in dst.0 */
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, get_element_ud(inst->dst.as_brw_reg(), 0),
           get_element_ud(inst->src[0].as_brw_reg(), 0));
   brw_pop_insn_state(p);
}

static void
generate_gs_thread_end(struct brw_codegen *p, vec4_instruction *inst)
{
   struct brw_reg src = brw_message_reg(inst->base_mrf);
   brw_urb_WRITE(p,
                 brw_null_reg(), /* dest */
                 inst->base_mrf, /* starting mrf reg nr */
                 src,
                 BRW_URB_WRITE_EOT | inst->urb_write_flags,
                 inst->mlen,
                 0,              /* response len */
                 0,              /* urb destination offset */
                 BRW_URB_SWIZZLE_INTERLEAVE);
}

static void
generate_gs_set_write_offset(struct brw_codegen *p,
                             struct brw_reg dst,
                             struct brw_reg src0,
                             struct brw_reg src1)
{
   /* From p22 of volume 4 part 2 of the Ivy Bridge PRM (2.4.3.1 Message
    * Header: M0.3):
    *
    *     Slot 0 Offset. This field, after adding to the Global Offset field
    *     in the message descriptor, specifies the offset (in 256-bit units)
    *     from the start of the URB entry, as referenced by URB Handle 0, at
    *     which the data will be accessed.
    *
    * Similar text describes DWORD M0.4, which is slot 1 offset.
    *
    * Therefore, we want to multiply DWORDs 0 and 4 of src0 (the x components
    * of the register for geometry shader invocations 0 and 1) by the
    * immediate value in src1, and store the result in DWORDs 3 and 4 of dst.
    *
    * We can do this with the following EU instruction:
    *
    *     mul(2) dst.3<1>UD src0<8;2,4>UD src1<...>UW   { Align1 WE_all }
    */
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   assert(p->devinfo->ver >= 7 &&
          src1.file == BRW_IMMEDIATE_VALUE &&
          src1.type == BRW_REGISTER_TYPE_UD &&
          src1.ud <= USHRT_MAX);
   if (src0.file == BRW_IMMEDIATE_VALUE) {
      brw_MOV(p, suboffset(stride(dst, 2, 2, 1), 3),
              brw_imm_ud(src0.ud * src1.ud));
   } else {
      if (src1.file == BRW_IMMEDIATE_VALUE) {
         src1 = brw_imm_uw(src1.ud);
      }
      brw_MUL(p, suboffset(stride(dst, 2, 2, 1), 3), stride(src0, 8, 2, 4),
              retype(src1, BRW_REGISTER_TYPE_UW));
   }
   brw_pop_insn_state(p);
}

static void
generate_gs_set_vertex_count(struct brw_codegen *p,
                             struct brw_reg dst,
                             struct brw_reg src)
{
   brw_push_insn_state(p);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);

   /* If we think of the src and dst registers as composed of 8 DWORDs each,
    * we want to pick up the contents of DWORDs 0 and 4 from src, truncate
    * them to WORDs, and then pack them into DWORD 2 of dst.
    *
    * It's easier to get the EU to do this if we think of the src and dst
    * registers as composed of 16 WORDS each; then, we want to pick up the
    * contents of WORDs 0 and 8 from src, and pack them into WORDs 4 and 5
    * of dst.
    *
    * We can do that by the following EU instruction:
    *
    *     mov (2) dst.4<1>:uw src<8;1,0>:uw   { Align1, Q1, NoMask }
    */
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_MOV(p,
           suboffset(stride(retype(dst, BRW_REGISTER_TYPE_UW), 2, 2, 1), 4),
           stride(retype(src, BRW_REGISTER_TYPE_UW), 8, 1, 0));

   brw_pop_insn_state(p);
}

static void
generate_gs_svb_write(struct brw_codegen *p,
                      vec4_instruction *inst,
                      struct brw_reg dst,
                      struct brw_reg src0,
                      struct brw_reg src1)
{
   int binding = inst->sol_binding;
   bool final_write = inst->sol_final_write;

   brw_push_insn_state(p);
   brw_set_default_exec_size(p, BRW_EXECUTE_4);
   /* Copy Vertex data into M0.x */
   brw_MOV(p, stride(dst, 4, 4, 1),
           stride(retype(src0, BRW_REGISTER_TYPE_UD), 4, 4, 1));
   brw_pop_insn_state(p);

   brw_push_insn_state(p);
   /* Send SVB Write */
   brw_svb_write(p,
                 final_write ? src1 : brw_null_reg(), /* dest == src1 */
                 1, /* msg_reg_nr */
                 dst, /* src0 == previous dst */
                 BRW_GFX6_SOL_BINDING_START + binding, /* binding_table_index */
                 final_write); /* send_commit_msg */

   /* Finally, wait for the write commit to occur so that we can proceed to
    * other things safely.
    *
    * From the Sandybridge PRM, Volume 4, Part 1, Section 3.3:
    *
    *   The write commit does not modify the destination register, but
    *   merely clears the dependency associated with the destination
    *   register. Thus, a simple “mov” instruction using the register as a
    *   source is sufficient to wait for the write commit to occur.
    */
   if (final_write) {
      brw_MOV(p, src1, src1);
   }
   brw_pop_insn_state(p);
}

static void
generate_gs_svb_set_destination_index(struct brw_codegen *p,
                                      vec4_instruction *inst,
                                      struct brw_reg dst,
                                      struct brw_reg src)
{
   int vertex = inst->sol_vertex;
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, get_element_ud(dst, 5), get_element_ud(src, vertex));
   brw_pop_insn_state(p);
}

static void
generate_gs_set_dword_2(struct brw_codegen *p,
                        struct brw_reg dst,
                        struct brw_reg src)
{
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, suboffset(vec1(dst), 2), suboffset(vec1(src), 0));
   brw_pop_insn_state(p);
}

static void
generate_gs_prepare_channel_masks(struct brw_codegen *p,
                                  struct brw_reg dst)
{
   /* We want to left shift just DWORD 4 (the x component belonging to the
    * second geometry shader invocation) by 4 bits.  So generate the
    * instruction:
    *
    *     shl(1) dst.4<1>UD dst.4<0,1,0>UD 4UD { align1 WE_all }
    */
   dst = suboffset(vec1(dst), 4);
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_SHL(p, dst, dst, brw_imm_ud(4));
   brw_pop_insn_state(p);
}

static void
generate_gs_set_channel_masks(struct brw_codegen *p,
                              struct brw_reg dst,
                              struct brw_reg src)
{
   /* From p21 of volume 4 part 2 of the Ivy Bridge PRM (2.4.3.1 Message
    * Header: M0.5):
    *
    *     15 Vertex 1 DATA [3] / Vertex 0 DATA[7] Channel Mask
    *
    *        When Swizzle Control = URB_INTERLEAVED this bit controls Vertex 1
    *        DATA[3], when Swizzle Control = URB_NOSWIZZLE this bit controls
    *        Vertex 0 DATA[7].  This bit is ANDed with the corresponding
    *        channel enable to determine the final channel enable.  For the
    *        URB_READ_OWORD & URB_READ_HWORD messages, when final channel
    *        enable is 1 it indicates that Vertex 1 DATA [3] will be included
    *        in the writeback message.  For the URB_WRITE_OWORD &
    *        URB_WRITE_HWORD messages, when final channel enable is 1 it
    *        indicates that Vertex 1 DATA [3] will be written to the surface.
    *
    *        0: Vertex 1 DATA [3] / Vertex 0 DATA[7] channel not included
    *        1: Vertex DATA [3] / Vertex 0 DATA[7] channel included
    *
    *     14 Vertex 1 DATA [2] Channel Mask
    *     13 Vertex 1 DATA [1] Channel Mask
    *     12 Vertex 1 DATA [0] Channel Mask
    *     11 Vertex 0 DATA [3] Channel Mask
    *     10 Vertex 0 DATA [2] Channel Mask
    *      9 Vertex 0 DATA [1] Channel Mask
    *      8 Vertex 0 DATA [0] Channel Mask
    *
    * (This is from a section of the PRM that is agnostic to the particular
    * type of shader being executed, so "Vertex 0" and "Vertex 1" refer to
    * geometry shader invocations 0 and 1, respectively).  Since we have the
    * enable flags for geometry shader invocation 0 in bits 3:0 of DWORD 0,
    * and the enable flags for geometry shader invocation 1 in bits 7:0 of
    * DWORD 4, we just need to OR them together and store the result in bits
    * 15:8 of DWORD 5.
    *
    * It's easier to get the EU to do this if we think of the src and dst
    * registers as composed of 32 bytes each; then, we want to pick up the
    * contents of bytes 0 and 16 from src, OR them together, and store them in
    * byte 21.
    *
    * We can do that by the following EU instruction:
    *
    *     or(1) dst.21<1>UB src<0,1,0>UB src.16<0,1,0>UB { align1 WE_all }
    *
    * Note: this relies on the source register having zeros in (a) bits 7:4 of
    * DWORD 0 and (b) bits 3:0 of DWORD 4.  We can rely on (b) because the
    * source register was prepared by GS_OPCODE_PREPARE_CHANNEL_MASKS (which
    * shifts DWORD 4 left by 4 bits), and we can rely on (a) because prior to
    * the execution of GS_OPCODE_PREPARE_CHANNEL_MASKS, DWORDs 0 and 4 need to
    * contain valid channel mask values (which are in the range 0x0-0xf).
    */
   dst = retype(dst, BRW_REGISTER_TYPE_UB);
   src = retype(src, BRW_REGISTER_TYPE_UB);
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_OR(p, suboffset(vec1(dst), 21), vec1(src), suboffset(vec1(src), 16));
   brw_pop_insn_state(p);
}

static void
generate_gs_get_instance_id(struct brw_codegen *p,
                            struct brw_reg dst)
{
   /* We want to right shift R0.0 & R0.1 by GFX7_GS_PAYLOAD_INSTANCE_ID_SHIFT
    * and store into dst.0 & dst.4. So generate the instruction:
    *
    *     shr(8) dst<1> R0<1,4,0> GFX7_GS_PAYLOAD_INSTANCE_ID_SHIFT { align1 WE_normal 1Q }
    */
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   dst = retype(dst, BRW_REGISTER_TYPE_UD);
   struct brw_reg r0(retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));
   brw_SHR(p, dst, stride(r0, 1, 4, 0),
           brw_imm_ud(GFX7_GS_PAYLOAD_INSTANCE_ID_SHIFT));
   brw_pop_insn_state(p);
}

static void
generate_gs_ff_sync_set_primitives(struct brw_codegen *p,
                                   struct brw_reg dst,
                                   struct brw_reg src0,
                                   struct brw_reg src1,
                                   struct brw_reg src2)
{
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   /* Save src0 data in 16:31 bits of dst.0 */
   brw_AND(p, suboffset(vec1(dst), 0), suboffset(vec1(src0), 0),
           brw_imm_ud(0xffffu));
   brw_SHL(p, suboffset(vec1(dst), 0), suboffset(vec1(dst), 0), brw_imm_ud(16));
   /* Save src1 data in 0:15 bits of dst.0 */
   brw_AND(p, suboffset(vec1(src2), 0), suboffset(vec1(src1), 0),
           brw_imm_ud(0xffffu));
   brw_OR(p, suboffset(vec1(dst), 0),
          suboffset(vec1(dst), 0),
          suboffset(vec1(src2), 0));
   brw_pop_insn_state(p);
}

static void
generate_gs_ff_sync(struct brw_codegen *p,
                    vec4_instruction *inst,
                    struct brw_reg dst,
                    struct brw_reg src0,
                    struct brw_reg src1)
{
   /* This opcode uses an implied MRF register for:
    *  - the header of the ff_sync message. And as such it is expected to be
    *    initialized to r0 before calling here.
    *  - the destination where we will write the allocated URB handle.
    */
   struct brw_reg header =
      retype(brw_message_reg(inst->base_mrf), BRW_REGISTER_TYPE_UD);

   /* Overwrite dword 0 of the header (SO vertices to write) and
    * dword 1 (number of primitives written).
    */
   brw_push_insn_state(p);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_MOV(p, get_element_ud(header, 0), get_element_ud(src1, 0));
   brw_MOV(p, get_element_ud(header, 1), get_element_ud(src0, 0));
   brw_pop_insn_state(p);

   /* Allocate URB handle in dst */
   brw_ff_sync(p,
               dst,
               0,
               header,
               1, /* allocate */
               1, /* response length */
               0 /* eot */);

   /* Now put allocated urb handle in header.0 */
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, get_element_ud(header, 0), get_element_ud(dst, 0));

   /* src1 is not an immediate when we use transform feedback */
   if (src1.file != BRW_IMMEDIATE_VALUE) {
      brw_set_default_exec_size(p, BRW_EXECUTE_4);
      brw_MOV(p, brw_vec4_grf(src1.nr, 0), brw_vec4_grf(dst.nr, 1));
   }

   brw_pop_insn_state(p);
}

static void
generate_gs_set_primitive_id(struct brw_codegen *p, struct brw_reg dst)
{
   /* In gfx6, PrimitiveID is delivered in R0.1 of the payload */
   struct brw_reg src = brw_vec8_grf(0, 0);
   brw_push_insn_state(p);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_MOV(p, get_element_ud(dst, 0), get_element_ud(src, 1));
   brw_pop_insn_state(p);
}

static void
generate_tcs_get_instance_id(struct brw_codegen *p, struct brw_reg dst)
{
   const struct intel_device_info *devinfo = p->devinfo;
   const bool ivb = devinfo->platform == INTEL_PLATFORM_IVB ||
                    devinfo->platform == INTEL_PLATFORM_BYT;

   /* "Instance Count" comes as part of the payload in r0.2 bits 23:17.
    *
    * Since we operate in SIMD4x2 mode, we need run half as many threads
    * as necessary.  So we assign (2i + 1, 2i) as the thread counts.  We
    * shift right by one less to accomplish the multiplication by two.
    */
   dst = retype(dst, BRW_REGISTER_TYPE_UD);
   struct brw_reg r0(retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));

   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);

   const int mask = ivb ? INTEL_MASK(22, 16) : INTEL_MASK(23, 17);
   const int shift = ivb ? 16 : 17;

   brw_AND(p, get_element_ud(dst, 0), get_element_ud(r0, 2), brw_imm_ud(mask));
   brw_SHR(p, get_element_ud(dst, 0), get_element_ud(dst, 0),
           brw_imm_ud(shift - 1));
   brw_ADD(p, get_element_ud(dst, 4), get_element_ud(dst, 0), brw_imm_ud(1));

   brw_pop_insn_state(p);
}

static void
generate_tcs_urb_write(struct brw_codegen *p,
                       vec4_instruction *inst,
                       struct brw_reg urb_header)
{
   const struct intel_device_info *devinfo = p->devinfo;

   brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
   brw_set_dest(p, send, brw_null_reg());
   brw_set_src0(p, send, urb_header);
   brw_set_desc(p, send, brw_message_desc(devinfo, inst->mlen, 0, true));

   brw_inst_set_sfid(devinfo, send, BRW_SFID_URB);
   brw_inst_set_urb_opcode(devinfo, send, BRW_URB_OPCODE_WRITE_OWORD);
   brw_inst_set_urb_global_offset(devinfo, send, inst->offset);
   if (inst->urb_write_flags & BRW_URB_WRITE_EOT) {
      brw_inst_set_eot(devinfo, send, 1);
   } else {
      brw_inst_set_urb_per_slot_offset(devinfo, send, 1);
      brw_inst_set_urb_swizzle_control(devinfo, send, BRW_URB_SWIZZLE_INTERLEAVE);
   }

   /* what happens to swizzles? */
}


static void
generate_tcs_input_urb_offsets(struct brw_codegen *p,
                               struct brw_reg dst,
                               struct brw_reg vertex,
                               struct brw_reg offset)
{
   /* Generates an URB read/write message header for HS/DS operation.
    * Inputs are a vertex index, and a byte offset from the beginning of
    * the vertex. */

   /* If `vertex` is not an immediate, we clobber a0.0 */

   assert(vertex.file == BRW_IMMEDIATE_VALUE || vertex.file == BRW_GENERAL_REGISTER_FILE);
   assert(vertex.type == BRW_REGISTER_TYPE_UD || vertex.type == BRW_REGISTER_TYPE_D);

   assert(dst.file == BRW_GENERAL_REGISTER_FILE);

   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, dst, brw_imm_ud(0));

   /* m0.5 bits 8-15 are channel enables */
   brw_MOV(p, get_element_ud(dst, 5), brw_imm_ud(0xff00));

   /* m0.0-0.1: URB handles */
   if (vertex.file == BRW_IMMEDIATE_VALUE) {
      uint32_t vertex_index = vertex.ud;
      struct brw_reg index_reg = brw_vec1_grf(
            1 + (vertex_index >> 3), vertex_index & 7);

      brw_MOV(p, vec2(get_element_ud(dst, 0)),
              retype(index_reg, BRW_REGISTER_TYPE_UD));
   } else {
      /* Use indirect addressing.  ICP Handles are DWords (single channels
       * of a register) and start at g1.0.
       *
       * In order to start our region at g1.0, we add 8 to the vertex index,
       * effectively skipping over the 8 channels in g0.0.  This gives us a
       * DWord offset to the ICP Handle.
       *
       * Indirect addressing works in terms of bytes, so we then multiply
       * the DWord offset by 4 (by shifting left by 2).
       */
      struct brw_reg addr = brw_address_reg(0);

      /* bottom half: m0.0 = g[1.0 + vertex.0]UD */
      brw_ADD(p, addr, retype(get_element_ud(vertex, 0), BRW_REGISTER_TYPE_UW),
              brw_imm_uw(0x8));
      brw_SHL(p, addr, addr, brw_imm_uw(2));
      brw_MOV(p, get_element_ud(dst, 0), deref_1ud(brw_indirect(0, 0), 0));

      /* top half: m0.1 = g[1.0 + vertex.4]UD */
      brw_ADD(p, addr, retype(get_element_ud(vertex, 4), BRW_REGISTER_TYPE_UW),
              brw_imm_uw(0x8));
      brw_SHL(p, addr, addr, brw_imm_uw(2));
      brw_MOV(p, get_element_ud(dst, 1), deref_1ud(brw_indirect(0, 0), 0));
   }

   /* m0.3-0.4: 128bit-granular offsets into the URB from the handles */
   if (offset.file != ARF)
      brw_MOV(p, vec2(get_element_ud(dst, 3)), stride(offset, 4, 1, 0));

   brw_pop_insn_state(p);
}


static void
generate_tcs_output_urb_offsets(struct brw_codegen *p,
                                struct brw_reg dst,
                                struct brw_reg write_mask,
                                struct brw_reg offset)
{
   /* Generates an URB read/write message header for HS/DS operation, for the patch URB entry. */
   assert(dst.file == BRW_GENERAL_REGISTER_FILE || dst.file == BRW_MESSAGE_REGISTER_FILE);

   assert(write_mask.file == BRW_IMMEDIATE_VALUE);
   assert(write_mask.type == BRW_REGISTER_TYPE_UD);

   brw_push_insn_state(p);

   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, dst, brw_imm_ud(0));

   unsigned mask = write_mask.ud;

   /* m0.5 bits 15:12 and 11:8 are channel enables */
   brw_MOV(p, get_element_ud(dst, 5), brw_imm_ud((mask << 8) | (mask << 12)));

   /* HS patch URB handle is delivered in r0.0 */
   struct brw_reg urb_handle = brw_vec1_grf(0, 0);

   /* m0.0-0.1: URB handles */
   brw_MOV(p, vec2(get_element_ud(dst, 0)),
           retype(urb_handle, BRW_REGISTER_TYPE_UD));

   /* m0.3-0.4: 128bit-granular offsets into the URB from the handles */
   if (offset.file != ARF)
      brw_MOV(p, vec2(get_element_ud(dst, 3)), stride(offset, 4, 1, 0));

   brw_pop_insn_state(p);
}

static void
generate_tes_create_input_read_header(struct brw_codegen *p,
                                      struct brw_reg dst)
{
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);

   /* Initialize the register to 0 */
   brw_MOV(p, dst, brw_imm_ud(0));

   /* Enable all the channels in m0.5 bits 15:8 */
   brw_MOV(p, get_element_ud(dst, 5), brw_imm_ud(0xff00));

   /* Copy g1.3 (the patch URB handle) to m0.0 and m0.1.  For safety,
    * mask out irrelevant "Reserved" bits, as they're not marked MBZ.
    */
   brw_AND(p, vec2(get_element_ud(dst, 0)),
           retype(brw_vec1_grf(1, 3), BRW_REGISTER_TYPE_UD),
           brw_imm_ud(0x1fff));
   brw_pop_insn_state(p);
}

static void
generate_tes_add_indirect_urb_offset(struct brw_codegen *p,
                                     struct brw_reg dst,
                                     struct brw_reg header,
                                     struct brw_reg offset)
{
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);

   brw_MOV(p, dst, header);

   /* Uniforms will have a stride <0;4,1>, and we need to convert to <0;1,0>.
    * Other values get <4;1,0>.
    */
   struct brw_reg restrided_offset;
   if (offset.vstride == BRW_VERTICAL_STRIDE_0 &&
       offset.width == BRW_WIDTH_4 &&
       offset.hstride == BRW_HORIZONTAL_STRIDE_1) {
      restrided_offset = stride(offset, 0, 1, 0);
   } else {
      restrided_offset = stride(offset, 4, 1, 0);
   }

   /* m0.3-0.4: 128-bit-granular offsets into the URB from the handles */
   brw_MOV(p, vec2(get_element_ud(dst, 3)), restrided_offset);

   brw_pop_insn_state(p);
}

static void
generate_vec4_urb_read(struct brw_codegen *p,
                       vec4_instruction *inst,
                       struct brw_reg dst,
                       struct brw_reg header)
{
   const struct intel_device_info *devinfo = p->devinfo;

   assert(header.file == BRW_GENERAL_REGISTER_FILE);
   assert(header.type == BRW_REGISTER_TYPE_UD);

   brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
   brw_set_dest(p, send, dst);
   brw_set_src0(p, send, header);

   brw_set_desc(p, send, brw_message_desc(devinfo, 1, 1, true));

   brw_inst_set_sfid(devinfo, send, BRW_SFID_URB);
   brw_inst_set_urb_opcode(devinfo, send, BRW_URB_OPCODE_READ_OWORD);
   brw_inst_set_urb_swizzle_control(devinfo, send, BRW_URB_SWIZZLE_INTERLEAVE);
   brw_inst_set_urb_per_slot_offset(devinfo, send, 1);

   brw_inst_set_urb_global_offset(devinfo, send, inst->offset);
}

static void
generate_tcs_release_input(struct brw_codegen *p,
                           struct brw_reg header,
                           struct brw_reg vertex,
                           struct brw_reg is_unpaired)
{
   const struct intel_device_info *devinfo = p->devinfo;

   assert(vertex.file == BRW_IMMEDIATE_VALUE);
   assert(vertex.type == BRW_REGISTER_TYPE_UD);

   /* m0.0-0.1: URB handles */
   struct brw_reg urb_handles =
      retype(brw_vec2_grf(1 + (vertex.ud >> 3), vertex.ud & 7),
             BRW_REGISTER_TYPE_UD);

   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, header, brw_imm_ud(0));
   brw_MOV(p, vec2(get_element_ud(header, 0)), urb_handles);
   brw_pop_insn_state(p);

   brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
   brw_set_dest(p, send, brw_null_reg());
   brw_set_src0(p, send, header);
   brw_set_desc(p, send, brw_message_desc(devinfo, 1, 0, true));

   brw_inst_set_sfid(devinfo, send, BRW_SFID_URB);
   brw_inst_set_urb_opcode(devinfo, send, BRW_URB_OPCODE_READ_OWORD);
   brw_inst_set_urb_complete(devinfo, send, 1);
   brw_inst_set_urb_swizzle_control(devinfo, send, is_unpaired.ud ?
                                    BRW_URB_SWIZZLE_NONE :
                                    BRW_URB_SWIZZLE_INTERLEAVE);
}

static void
generate_tcs_thread_end(struct brw_codegen *p, vec4_instruction *inst)
{
   struct brw_reg header = brw_message_reg(inst->base_mrf);

   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, header, brw_imm_ud(0));
   brw_MOV(p, get_element_ud(header, 5), brw_imm_ud(WRITEMASK_X << 8));
   brw_MOV(p, get_element_ud(header, 0),
           retype(brw_vec1_grf(0, 0), BRW_REGISTER_TYPE_UD));
   brw_MOV(p, brw_message_reg(inst->base_mrf + 1), brw_imm_ud(0u));
   brw_pop_insn_state(p);

   brw_urb_WRITE(p,
                 brw_null_reg(), /* dest */
                 inst->base_mrf, /* starting mrf reg nr */
                 header,
                 BRW_URB_WRITE_EOT | BRW_URB_WRITE_OWORD |
                 BRW_URB_WRITE_USE_CHANNEL_MASKS,
                 inst->mlen,
                 0,              /* response len */
                 0,              /* urb destination offset */
                 0);
}

static void
generate_tes_get_primitive_id(struct brw_codegen *p, struct brw_reg dst)
{
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_MOV(p, dst, retype(brw_vec1_grf(1, 7), BRW_REGISTER_TYPE_D));
   brw_pop_insn_state(p);
}

static void
generate_tcs_get_primitive_id(struct brw_codegen *p, struct brw_reg dst)
{
   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_MOV(p, dst, retype(brw_vec1_grf(0, 1), BRW_REGISTER_TYPE_UD));
   brw_pop_insn_state(p);
}

static void
generate_tcs_create_barrier_header(struct brw_codegen *p,
                                   struct brw_vue_prog_data *prog_data,
                                   struct brw_reg dst)
{
   const struct intel_device_info *devinfo = p->devinfo;
   const bool ivb = devinfo->platform == INTEL_PLATFORM_IVB ||
                    devinfo->platform == INTEL_PLATFORM_BYT;
   struct brw_reg m0_2 = get_element_ud(dst, 2);
   unsigned instances = ((struct brw_tcs_prog_data *) prog_data)->instances;

   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);

   /* Zero the message header */
   brw_MOV(p, retype(dst, BRW_REGISTER_TYPE_UD), brw_imm_ud(0u));

   /* Copy "Barrier ID" from r0.2, bits 16:13 (Gfx7.5+) or 15:12 (Gfx7) */
   brw_AND(p, m0_2,
           retype(brw_vec1_grf(0, 2), BRW_REGISTER_TYPE_UD),
           brw_imm_ud(ivb ? INTEL_MASK(15, 12) : INTEL_MASK(16, 13)));

   /* Shift it up to bits 27:24. */
   brw_SHL(p, m0_2, get_element_ud(dst, 2), brw_imm_ud(ivb ? 12 : 11));

   /* Set the Barrier Count and the enable bit */
   brw_OR(p, m0_2, m0_2, brw_imm_ud(instances << 9 | (1 << 15)));

   brw_pop_insn_state(p);
}

static void
generate_oword_dual_block_offsets(struct brw_codegen *p,
                                  struct brw_reg m1,
                                  struct brw_reg index)
{
   int second_vertex_offset;

   if (p->devinfo->ver >= 6)
      second_vertex_offset = 1;
   else
      second_vertex_offset = 16;

   m1 = retype(m1, BRW_REGISTER_TYPE_D);

   /* Set up M1 (message payload).  Only the block offsets in M1.0 and
    * M1.4 are used, and the rest are ignored.
    */
   struct brw_reg m1_0 = suboffset(vec1(m1), 0);
   struct brw_reg m1_4 = suboffset(vec1(m1), 4);
   struct brw_reg index_0 = suboffset(vec1(index), 0);
   struct brw_reg index_4 = suboffset(vec1(index), 4);

   brw_push_insn_state(p);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_set_default_access_mode(p, BRW_ALIGN_1);

   brw_MOV(p, m1_0, index_0);

   if (index.file == BRW_IMMEDIATE_VALUE) {
      index_4.ud += second_vertex_offset;
      brw_MOV(p, m1_4, index_4);
   } else {
      brw_ADD(p, m1_4, index_4, brw_imm_d(second_vertex_offset));
   }

   brw_pop_insn_state(p);
}

static void
generate_unpack_flags(struct brw_codegen *p,
                      struct brw_reg dst)
{
   brw_push_insn_state(p);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);
   brw_set_default_access_mode(p, BRW_ALIGN_1);

   struct brw_reg flags = brw_flag_reg(0, 0);
   struct brw_reg dst_0 = suboffset(vec1(dst), 0);
   struct brw_reg dst_4 = suboffset(vec1(dst), 4);

   brw_AND(p, dst_0, flags, brw_imm_ud(0x0f));
   brw_AND(p, dst_4, flags, brw_imm_ud(0xf0));
   brw_SHR(p, dst_4, dst_4, brw_imm_ud(4));

   brw_pop_insn_state(p);
}

static void
generate_scratch_read(struct brw_codegen *p,
                      vec4_instruction *inst,
                      struct brw_reg dst,
                      struct brw_reg index)
{
   const struct intel_device_info *devinfo = p->devinfo;
   struct brw_reg header = brw_vec8_grf(0, 0);

   gfx6_resolve_implied_move(p, &header, inst->base_mrf);

   generate_oword_dual_block_offsets(p, brw_message_reg(inst->base_mrf + 1),
				     index);

   uint32_t msg_type;

   if (devinfo->ver >= 6)
      msg_type = GFX6_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ;
   else if (devinfo->verx10 >= 45)
      msg_type = G45_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ;
   else
      msg_type = BRW_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ;

   const unsigned target_cache =
      devinfo->ver >= 7 ? GFX7_SFID_DATAPORT_DATA_CACHE :
      devinfo->ver >= 6 ? GFX6_SFID_DATAPORT_RENDER_CACHE :
      BRW_SFID_DATAPORT_READ;

   /* Each of the 8 channel enables is considered for whether each
    * dword is written.
    */
   brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
   brw_inst_set_sfid(devinfo, send, target_cache);
   brw_set_dest(p, send, dst);
   brw_set_src0(p, send, header);
   if (devinfo->ver < 6)
      brw_inst_set_cond_modifier(devinfo, send, inst->base_mrf);
   brw_set_desc(p, send,
                brw_message_desc(devinfo, 2, 1, true) |
                brw_dp_read_desc(devinfo,
                                 brw_scratch_surface_idx(p),
                                 BRW_DATAPORT_OWORD_DUAL_BLOCK_1OWORD,
                                 msg_type, BRW_DATAPORT_READ_TARGET_RENDER_CACHE));
}

static void
generate_scratch_write(struct brw_codegen *p,
                       vec4_instruction *inst,
                       struct brw_reg dst,
                       struct brw_reg src,
                       struct brw_reg index)
{
   const struct intel_device_info *devinfo = p->devinfo;
   const unsigned target_cache =
      (devinfo->ver >= 7 ? GFX7_SFID_DATAPORT_DATA_CACHE :
       devinfo->ver >= 6 ? GFX6_SFID_DATAPORT_RENDER_CACHE :
       BRW_SFID_DATAPORT_WRITE);
   struct brw_reg header = brw_vec8_grf(0, 0);
   bool write_commit;

   /* If the instruction is predicated, we'll predicate the send, not
    * the header setup.
    */
   brw_push_insn_state(p);
   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
   brw_set_default_flag_reg(p, 0, 0);

   gfx6_resolve_implied_move(p, &header, inst->base_mrf);

   generate_oword_dual_block_offsets(p, brw_message_reg(inst->base_mrf + 1),
				     index);

   brw_MOV(p,
	   retype(brw_message_reg(inst->base_mrf + 2), BRW_REGISTER_TYPE_D),
	   retype(src, BRW_REGISTER_TYPE_D));

   brw_pop_insn_state(p);

   uint32_t msg_type;

   if (devinfo->ver >= 7)
      msg_type = GFX7_DATAPORT_DC_OWORD_DUAL_BLOCK_WRITE;
   else if (devinfo->ver == 6)
      msg_type = GFX6_DATAPORT_WRITE_MESSAGE_OWORD_DUAL_BLOCK_WRITE;
   else
      msg_type = BRW_DATAPORT_WRITE_MESSAGE_OWORD_DUAL_BLOCK_WRITE;

   brw_set_default_predicate_control(p, inst->predicate);

   /* Pre-gfx6, we have to specify write commits to ensure ordering
    * between reads and writes within a thread.  Afterwards, that's
    * guaranteed and write commits only matter for inter-thread
    * synchronization.
    */
   if (devinfo->ver >= 6) {
      write_commit = false;
   } else {
      /* The visitor set up our destination register to be g0.  This
       * means that when the next read comes along, we will end up
       * reading from g0 and causing a block on the write commit.  For
       * write-after-read, we are relying on the value of the previous
       * read being used (and thus blocking on completion) before our
       * write is executed.  This means we have to be careful in
       * instruction scheduling to not violate this assumption.
       */
      write_commit = true;
   }

   /* Each of the 8 channel enables is considered for whether each
    * dword is written.
    */
   brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
   brw_inst_set_sfid(p->devinfo, send, target_cache);
   brw_set_dest(p, send, dst);
   brw_set_src0(p, send, header);
   if (devinfo->ver < 6)
      brw_inst_set_cond_modifier(p->devinfo, send, inst->base_mrf);
   brw_set_desc(p, send,
                brw_message_desc(devinfo, 3, write_commit, true) |
                brw_dp_write_desc(devinfo,
                                  brw_scratch_surface_idx(p),
                                  BRW_DATAPORT_OWORD_DUAL_BLOCK_1OWORD,
                                  msg_type,
                                  write_commit));
}

static void
generate_pull_constant_load(struct brw_codegen *p,
                            vec4_instruction *inst,
                            struct brw_reg dst,
                            struct brw_reg index,
                            struct brw_reg offset)
{
   const struct intel_device_info *devinfo = p->devinfo;
   const unsigned target_cache =
      (devinfo->ver >= 6 ? GFX6_SFID_DATAPORT_SAMPLER_CACHE :
       BRW_SFID_DATAPORT_READ);
   assert(index.file == BRW_IMMEDIATE_VALUE &&
	  index.type == BRW_REGISTER_TYPE_UD);
   uint32_t surf_index = index.ud;

   struct brw_reg header = brw_vec8_grf(0, 0);

   gfx6_resolve_implied_move(p, &header, inst->base_mrf);

   if (devinfo->ver >= 6) {
      if (offset.file == BRW_IMMEDIATE_VALUE) {
         brw_MOV(p, retype(brw_message_reg(inst->base_mrf + 1),
                           BRW_REGISTER_TYPE_D),
                 brw_imm_d(offset.ud >> 4));
      } else {
         brw_SHR(p, retype(brw_message_reg(inst->base_mrf + 1),
                           BRW_REGISTER_TYPE_D),
                 offset, brw_imm_d(4));
      }
   } else {
      brw_MOV(p, retype(brw_message_reg(inst->base_mrf + 1),
                        BRW_REGISTER_TYPE_D),
              offset);
   }

   uint32_t msg_type;

   if (devinfo->ver >= 6)
      msg_type = GFX6_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ;
   else if (devinfo->verx10 >= 45)
      msg_type = G45_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ;
   else
      msg_type = BRW_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ;

   /* Each of the 8 channel enables is considered for whether each
    * dword is written.
    */
   brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
   brw_inst_set_sfid(devinfo, send, target_cache);
   brw_set_dest(p, send, dst);
   brw_set_src0(p, send, header);
   if (devinfo->ver < 6)
      brw_inst_set_cond_modifier(p->devinfo, send, inst->base_mrf);
   brw_set_desc(p, send,
                brw_message_desc(devinfo, 2, 1, true) |
                brw_dp_read_desc(devinfo, surf_index,
                                 BRW_DATAPORT_OWORD_DUAL_BLOCK_1OWORD,
                                 msg_type,
                                 BRW_DATAPORT_READ_TARGET_DATA_CACHE));
}

static void
generate_get_buffer_size(struct brw_codegen *p,
                         vec4_instruction *inst,
                         struct brw_reg dst,
                         struct brw_reg src,
                         struct brw_reg surf_index)
{
   assert(p->devinfo->ver >= 7);
   assert(surf_index.type == BRW_REGISTER_TYPE_UD &&
          surf_index.file == BRW_IMMEDIATE_VALUE);

   brw_SAMPLE(p,
              dst,
              inst->base_mrf,
              src,
              surf_index.ud,
              0,
              GFX5_SAMPLER_MESSAGE_SAMPLE_RESINFO,
              1, /* response length */
              inst->mlen,
              inst->header_size > 0,
              BRW_SAMPLER_SIMD_MODE_SIMD4X2,
              BRW_SAMPLER_RETURN_FORMAT_SINT32);
}

static void
generate_pull_constant_load_gfx7(struct brw_codegen *p,
                                 vec4_instruction *inst,
                                 struct brw_reg dst,
                                 struct brw_reg surf_index,
                                 struct brw_reg offset)
{
   const struct intel_device_info *devinfo = p->devinfo;
   assert(surf_index.type == BRW_REGISTER_TYPE_UD);

   if (surf_index.file == BRW_IMMEDIATE_VALUE) {

      brw_inst *insn = brw_next_insn(p, BRW_OPCODE_SEND);
      brw_inst_set_sfid(devinfo, insn, BRW_SFID_SAMPLER);
      brw_set_dest(p, insn, dst);
      brw_set_src0(p, insn, offset);
      brw_set_desc(p, insn,
                   brw_message_desc(devinfo, inst->mlen, 1, inst->header_size) |
                   brw_sampler_desc(devinfo, surf_index.ud,
                                    0, /* LD message ignores sampler unit */
                                    GFX5_SAMPLER_MESSAGE_SAMPLE_LD,
                                    BRW_SAMPLER_SIMD_MODE_SIMD4X2, 0));
   } else {

      struct brw_reg addr = vec1(retype(brw_address_reg(0), BRW_REGISTER_TYPE_UD));

      brw_push_insn_state(p);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_access_mode(p, BRW_ALIGN_1);

      /* a0.0 = surf_index & 0xff */
      brw_inst *insn_and = brw_next_insn(p, BRW_OPCODE_AND);
      brw_inst_set_exec_size(devinfo, insn_and, BRW_EXECUTE_1);
      brw_set_dest(p, insn_and, addr);
      brw_set_src0(p, insn_and, vec1(retype(surf_index, BRW_REGISTER_TYPE_UD)));
      brw_set_src1(p, insn_and, brw_imm_ud(0x0ff));

      brw_pop_insn_state(p);

      /* dst = send(offset, a0.0 | <descriptor>) */
      brw_send_indirect_message(
         p, BRW_SFID_SAMPLER, dst, offset, addr,
         brw_message_desc(devinfo, inst->mlen, 1, inst->header_size) |
         brw_sampler_desc(devinfo,
                          0 /* surface */,
                          0 /* sampler */,
                          GFX5_SAMPLER_MESSAGE_SAMPLE_LD,
                          BRW_SAMPLER_SIMD_MODE_SIMD4X2,
                          0),
         false /* EOT */);
   }
}

static void
generate_mov_indirect(struct brw_codegen *p,
                      vec4_instruction *,
                      struct brw_reg dst, struct brw_reg reg,
                      struct brw_reg indirect)
{
   assert(indirect.type == BRW_REGISTER_TYPE_UD);
   assert(p->devinfo->ver >= 6);

   unsigned imm_byte_offset = reg.nr * REG_SIZE + reg.subnr * (REG_SIZE / 2);

   /* This instruction acts in align1 mode */
   assert(dst.writemask == WRITEMASK_XYZW);

   if (indirect.file == BRW_IMMEDIATE_VALUE) {
      imm_byte_offset += indirect.ud;

      reg.nr = imm_byte_offset / REG_SIZE;
      reg.subnr = (imm_byte_offset / (REG_SIZE / 2)) % 2;
      unsigned shift = (imm_byte_offset / 4) % 4;
      reg.swizzle += BRW_SWIZZLE4(shift, shift, shift, shift);

      brw_MOV(p, dst, reg);
   } else {
      brw_push_insn_state(p);
      brw_set_default_access_mode(p, BRW_ALIGN_1);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);

      struct brw_reg addr = vec8(brw_address_reg(0));

      /* We need to move the indirect value into the address register.  In
       * order to make things make some sense, we want to respect at least the
       * X component of the swizzle.  In order to do that, we need to convert
       * the subnr (probably 0) to an align1 subnr and add in the swizzle.
       */
      assert(brw_is_single_value_swizzle(indirect.swizzle));
      indirect.subnr = (indirect.subnr * 4 + BRW_GET_SWZ(indirect.swizzle, 0));

      /* We then use a region of <8,4,0>:uw to pick off the first 2 bytes of
       * the indirect and splat it out to all four channels of the given half
       * of a0.
       */
      indirect.subnr *= 2;
      indirect = stride(retype(indirect, BRW_REGISTER_TYPE_UW), 8, 4, 0);
      brw_ADD(p, addr, indirect, brw_imm_uw(imm_byte_offset));

      /* Now we need to incorporate the swizzle from the source register */
      if (reg.swizzle != BRW_SWIZZLE_XXXX) {
         uint32_t uv_swiz = BRW_GET_SWZ(reg.swizzle, 0) << 2 |
                            BRW_GET_SWZ(reg.swizzle, 1) << 6 |
                            BRW_GET_SWZ(reg.swizzle, 2) << 10 |
                            BRW_GET_SWZ(reg.swizzle, 3) << 14;
         uv_swiz |= uv_swiz << 16;

         brw_ADD(p, addr, addr, brw_imm_uv(uv_swiz));
      }

      brw_MOV(p, dst, retype(brw_VxH_indirect(0, 0), reg.type));

      brw_pop_insn_state(p);
   }
}

static void
generate_zero_oob_push_regs(struct brw_codegen *p,
                            struct brw_stage_prog_data *prog_data,
                            struct brw_reg scratch,
                            struct brw_reg bit_mask_in)
{
   const uint64_t want_zero = prog_data->zero_push_reg;
   assert(want_zero);

   assert(bit_mask_in.file == BRW_GENERAL_REGISTER_FILE);
   assert(BRW_GET_SWZ(bit_mask_in.swizzle, 1) ==
          BRW_GET_SWZ(bit_mask_in.swizzle, 0) + 1);
   bit_mask_in.subnr += BRW_GET_SWZ(bit_mask_in.swizzle, 0) * 4;
   bit_mask_in.type = BRW_REGISTER_TYPE_W;

   /* Scratch should be 3 registers in the GRF */
   assert(scratch.file == BRW_GENERAL_REGISTER_FILE);
   scratch = vec8(scratch);
   struct brw_reg mask_w16 = retype(scratch, BRW_REGISTER_TYPE_W);
   struct brw_reg mask_d16 = retype(byte_offset(scratch, REG_SIZE),
                                    BRW_REGISTER_TYPE_D);

   brw_push_insn_state(p);
   brw_set_default_access_mode(p, BRW_ALIGN_1);
   brw_set_default_mask_control(p, BRW_MASK_DISABLE);

   for (unsigned i = 0; i < 64; i++) {
      if (i % 16 == 0 && (want_zero & BITFIELD64_RANGE(i, 16))) {
         brw_set_default_exec_size(p, BRW_EXECUTE_8);
         brw_SHL(p, suboffset(mask_w16, 8),
                    vec1(byte_offset(bit_mask_in, i / 8)),
                    brw_imm_v(0x01234567));
         brw_SHL(p, mask_w16, suboffset(mask_w16, 8), brw_imm_w(8));

         brw_set_default_exec_size(p, BRW_EXECUTE_16);
         brw_ASR(p, mask_d16, mask_w16, brw_imm_w(15));
      }

      if (want_zero & BITFIELD64_BIT(i)) {
         unsigned push_start = prog_data->dispatch_grf_start_reg;
         struct brw_reg push_reg =
            retype(brw_vec8_grf(push_start + i, 0), BRW_REGISTER_TYPE_D);

         brw_set_default_exec_size(p, BRW_EXECUTE_8);
         brw_AND(p, push_reg, push_reg, vec1(suboffset(mask_d16, i)));
      }
   }

   brw_pop_insn_state(p);
}

static void
generate_code(struct brw_codegen *p,
              const struct brw_compiler *compiler,
              const struct brw_compile_params *params,
              const nir_shader *nir,
              struct brw_vue_prog_data *prog_data,
              const struct cfg_t *cfg,
              const performance &perf,
              struct brw_compile_stats *stats,
              bool debug_enabled)
{
   const struct intel_device_info *devinfo = p->devinfo;
   const char *stage_abbrev = _mesa_shader_stage_to_abbrev(nir->info.stage);
   struct disasm_info *disasm_info = disasm_initialize(p->isa, cfg);

   /* `send_count` explicitly does not include spills or fills, as we'd
    * like to use it as a metric for intentional memory access or other
    * shared function use.  Otherwise, subtle changes to scheduling or
    * register allocation could cause it to fluctuate wildly - and that
    * effect is already counted in spill/fill counts.
    */
   int spill_count = 0, fill_count = 0;
   int loop_count = 0, send_count = 0;

   foreach_block_and_inst (block, vec4_instruction, inst, cfg) {
      struct brw_reg src[3], dst;

      if (unlikely(debug_enabled))
         disasm_annotate(disasm_info, inst, p->next_insn_offset);

      for (unsigned int i = 0; i < 3; i++) {
         src[i] = inst->src[i].as_brw_reg();
      }
      dst = inst->dst.as_brw_reg();

      brw_set_default_predicate_control(p, inst->predicate);
      brw_set_default_predicate_inverse(p, inst->predicate_inverse);
      brw_set_default_flag_reg(p, inst->flag_subreg / 2, inst->flag_subreg % 2);
      brw_set_default_saturate(p, inst->saturate);
      brw_set_default_mask_control(p, inst->force_writemask_all);
      brw_set_default_acc_write_control(p, inst->writes_accumulator);

      assert(inst->group % inst->exec_size == 0);
      assert(inst->group % 4 == 0);

      /* There are some instructions where the destination is 64-bit
       * but we retype it to a smaller type. In that case, we cannot
       * double the exec_size.
       */
      const bool is_df = (get_exec_type_size(inst) == 8 ||
                          inst->dst.type == BRW_REGISTER_TYPE_DF) &&
                         inst->opcode != VEC4_OPCODE_PICK_LOW_32BIT &&
                         inst->opcode != VEC4_OPCODE_PICK_HIGH_32BIT &&
                         inst->opcode != VEC4_OPCODE_SET_LOW_32BIT &&
                         inst->opcode != VEC4_OPCODE_SET_HIGH_32BIT;

      unsigned exec_size = inst->exec_size;
      if (devinfo->verx10 == 70 && is_df)
         exec_size *= 2;

      brw_set_default_exec_size(p, cvt(exec_size) - 1);

      if (!inst->force_writemask_all)
         brw_set_default_group(p, inst->group);

      assert(inst->base_mrf + inst->mlen <= BRW_MAX_MRF(devinfo->ver));
      assert(inst->mlen <= BRW_MAX_MSG_LENGTH);

      unsigned pre_emit_nr_insn = p->nr_insn;

      switch (inst->opcode) {
      case VEC4_OPCODE_UNPACK_UNIFORM:
      case BRW_OPCODE_MOV:
      case VEC4_OPCODE_MOV_FOR_SCRATCH:
         brw_MOV(p, dst, src[0]);
         break;
      case BRW_OPCODE_ADD:
         brw_ADD(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_MUL:
         brw_MUL(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_MACH:
         brw_MACH(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_MAD:
         assert(devinfo->ver >= 6);
         brw_MAD(p, dst, src[0], src[1], src[2]);
         break;

      case BRW_OPCODE_FRC:
         brw_FRC(p, dst, src[0]);
         break;
      case BRW_OPCODE_RNDD:
         brw_RNDD(p, dst, src[0]);
         break;
      case BRW_OPCODE_RNDE:
         brw_RNDE(p, dst, src[0]);
         break;
      case BRW_OPCODE_RNDZ:
         brw_RNDZ(p, dst, src[0]);
         break;

      case BRW_OPCODE_AND:
         brw_AND(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_OR:
         brw_OR(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_XOR:
         brw_XOR(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_NOT:
         brw_NOT(p, dst, src[0]);
         break;
      case BRW_OPCODE_ASR:
         brw_ASR(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_SHR:
         brw_SHR(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_SHL:
         brw_SHL(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_CMP:
         brw_CMP(p, dst, inst->conditional_mod, src[0], src[1]);
         break;
      case BRW_OPCODE_CMPN:
         brw_CMPN(p, dst, inst->conditional_mod, src[0], src[1]);
         break;
      case BRW_OPCODE_SEL:
         brw_SEL(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_DPH:
         brw_DPH(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_DP4:
         brw_DP4(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_DP3:
         brw_DP3(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_DP2:
         brw_DP2(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_F32TO16:
         assert(devinfo->ver >= 7);
         brw_F32TO16(p, dst, src[0]);
         break;

      case BRW_OPCODE_F16TO32:
         assert(devinfo->ver >= 7);
         brw_F16TO32(p, dst, src[0]);
         break;

      case BRW_OPCODE_LRP:
         assert(devinfo->ver >= 6);
         brw_LRP(p, dst, src[0], src[1], src[2]);
         break;

      case BRW_OPCODE_BFREV:
         assert(devinfo->ver >= 7);
         brw_BFREV(p, retype(dst, BRW_REGISTER_TYPE_UD),
                   retype(src[0], BRW_REGISTER_TYPE_UD));
         break;
      case BRW_OPCODE_FBH:
         assert(devinfo->ver >= 7);
         brw_FBH(p, retype(dst, src[0].type), src[0]);
         break;
      case BRW_OPCODE_FBL:
         assert(devinfo->ver >= 7);
         brw_FBL(p, retype(dst, BRW_REGISTER_TYPE_UD),
                 retype(src[0], BRW_REGISTER_TYPE_UD));
         break;
      case BRW_OPCODE_LZD:
         brw_LZD(p, dst, src[0]);
         break;
      case BRW_OPCODE_CBIT:
         assert(devinfo->ver >= 7);
         brw_CBIT(p, retype(dst, BRW_REGISTER_TYPE_UD),
                  retype(src[0], BRW_REGISTER_TYPE_UD));
         break;
      case BRW_OPCODE_ADDC:
         assert(devinfo->ver >= 7);
         brw_ADDC(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_SUBB:
         assert(devinfo->ver >= 7);
         brw_SUBB(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_MAC:
         brw_MAC(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_BFE:
         assert(devinfo->ver >= 7);
         brw_BFE(p, dst, src[0], src[1], src[2]);
         break;

      case BRW_OPCODE_BFI1:
         assert(devinfo->ver >= 7);
         brw_BFI1(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_BFI2:
         assert(devinfo->ver >= 7);
         brw_BFI2(p, dst, src[0], src[1], src[2]);
         break;

      case BRW_OPCODE_IF:
         if (!inst->src[0].is_null()) {
            /* The instruction has an embedded compare (only allowed on gfx6) */
            assert(devinfo->ver == 6);
            gfx6_IF(p, inst->conditional_mod, src[0], src[1]);
         } else {
            brw_inst *if_inst = brw_IF(p, BRW_EXECUTE_8);
            brw_inst_set_pred_control(p->devinfo, if_inst, inst->predicate);
         }
         break;

      case BRW_OPCODE_ELSE:
         brw_ELSE(p);
         break;
      case BRW_OPCODE_ENDIF:
         brw_ENDIF(p);
         break;

      case BRW_OPCODE_DO:
         brw_DO(p, BRW_EXECUTE_8);
         break;

      case BRW_OPCODE_BREAK:
         brw_BREAK(p);
         brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
         break;
      case BRW_OPCODE_CONTINUE:
         brw_CONT(p);
         brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
         break;

      case BRW_OPCODE_WHILE:
         brw_WHILE(p);
         loop_count++;
         break;

      case SHADER_OPCODE_RCP:
      case SHADER_OPCODE_RSQ:
      case SHADER_OPCODE_SQRT:
      case SHADER_OPCODE_EXP2:
      case SHADER_OPCODE_LOG2:
      case SHADER_OPCODE_SIN:
      case SHADER_OPCODE_COS:
         assert(inst->conditional_mod == BRW_CONDITIONAL_NONE);
         if (devinfo->ver >= 7) {
            gfx6_math(p, dst, brw_math_function(inst->opcode), src[0],
                      brw_null_reg());
         } else if (devinfo->ver == 6) {
            generate_math_gfx6(p, inst, dst, src[0], brw_null_reg());
         } else {
            generate_math1_gfx4(p, inst, dst, src[0]);
            send_count++;
         }
         break;

      case SHADER_OPCODE_POW:
      case SHADER_OPCODE_INT_QUOTIENT:
      case SHADER_OPCODE_INT_REMAINDER:
         assert(inst->conditional_mod == BRW_CONDITIONAL_NONE);
         if (devinfo->ver >= 7) {
            gfx6_math(p, dst, brw_math_function(inst->opcode), src[0], src[1]);
         } else if (devinfo->ver == 6) {
            generate_math_gfx6(p, inst, dst, src[0], src[1]);
         } else {
            generate_math2_gfx4(p, inst, dst, src[0], src[1]);
            send_count++;
         }
         break;

      case SHADER_OPCODE_TEX:
      case SHADER_OPCODE_TXD:
      case SHADER_OPCODE_TXF:
      case SHADER_OPCODE_TXF_CMS:
      case SHADER_OPCODE_TXF_CMS_W:
      case SHADER_OPCODE_TXF_MCS:
      case SHADER_OPCODE_TXL:
      case SHADER_OPCODE_TXS:
      case SHADER_OPCODE_TG4:
      case SHADER_OPCODE_TG4_OFFSET:
      case SHADER_OPCODE_SAMPLEINFO:
         generate_tex(p, prog_data, nir->info.stage,
                      inst, dst, src[0], src[1], src[2]);
         send_count++;
         break;

      case SHADER_OPCODE_GET_BUFFER_SIZE:
         generate_get_buffer_size(p, inst, dst, src[0], src[1]);
         send_count++;
         break;

      case VEC4_VS_OPCODE_URB_WRITE:
         generate_vs_urb_write(p, inst);
         send_count++;
         break;

      case SHADER_OPCODE_GFX4_SCRATCH_READ:
         generate_scratch_read(p, inst, dst, src[0]);
         fill_count++;
         break;

      case SHADER_OPCODE_GFX4_SCRATCH_WRITE:
         generate_scratch_write(p, inst, dst, src[0], src[1]);
         spill_count++;
         break;

      case VS_OPCODE_PULL_CONSTANT_LOAD:
         generate_pull_constant_load(p, inst, dst, src[0], src[1]);
         send_count++;
         break;

      case VS_OPCODE_PULL_CONSTANT_LOAD_GFX7:
         generate_pull_constant_load_gfx7(p, inst, dst, src[0], src[1]);
         send_count++;
         break;

      case VEC4_GS_OPCODE_URB_WRITE:
         generate_gs_urb_write(p, inst);
         send_count++;
         break;

      case VEC4_GS_OPCODE_URB_WRITE_ALLOCATE:
         generate_gs_urb_write_allocate(p, inst);
         send_count++;
         break;

      case GS_OPCODE_SVB_WRITE:
         generate_gs_svb_write(p, inst, dst, src[0], src[1]);
         send_count++;
         break;

      case GS_OPCODE_SVB_SET_DST_INDEX:
         generate_gs_svb_set_destination_index(p, inst, dst, src[0]);
         break;

      case GS_OPCODE_THREAD_END:
         generate_gs_thread_end(p, inst);
         send_count++;
         break;

      case GS_OPCODE_SET_WRITE_OFFSET:
         generate_gs_set_write_offset(p, dst, src[0], src[1]);
         break;

      case GS_OPCODE_SET_VERTEX_COUNT:
         generate_gs_set_vertex_count(p, dst, src[0]);
         break;

      case GS_OPCODE_FF_SYNC:
         generate_gs_ff_sync(p, inst, dst, src[0], src[1]);
         send_count++;
         break;

      case GS_OPCODE_FF_SYNC_SET_PRIMITIVES:
         generate_gs_ff_sync_set_primitives(p, dst, src[0], src[1], src[2]);
         break;

      case GS_OPCODE_SET_PRIMITIVE_ID:
         generate_gs_set_primitive_id(p, dst);
         break;

      case GS_OPCODE_SET_DWORD_2:
         generate_gs_set_dword_2(p, dst, src[0]);
         break;

      case GS_OPCODE_PREPARE_CHANNEL_MASKS:
         generate_gs_prepare_channel_masks(p, dst);
         break;

      case GS_OPCODE_SET_CHANNEL_MASKS:
         generate_gs_set_channel_masks(p, dst, src[0]);
         break;

      case GS_OPCODE_GET_INSTANCE_ID:
         generate_gs_get_instance_id(p, dst);
         break;

      case VEC4_OPCODE_UNTYPED_ATOMIC:
         assert(src[2].file == BRW_IMMEDIATE_VALUE);
         brw_untyped_atomic(p, dst, src[0], src[1], src[2].ud, inst->mlen,
                            !inst->dst.is_null(), inst->header_size);
         send_count++;
         break;

      case VEC4_OPCODE_UNTYPED_SURFACE_READ:
         assert(!inst->header_size);
         assert(src[2].file == BRW_IMMEDIATE_VALUE);
         brw_untyped_surface_read(p, dst, src[0], src[1], inst->mlen,
                                  src[2].ud);
         send_count++;
         break;

      case VEC4_OPCODE_UNTYPED_SURFACE_WRITE:
         assert(src[2].file == BRW_IMMEDIATE_VALUE);
         brw_untyped_surface_write(p, src[0], src[1], inst->mlen,
                                   src[2].ud, inst->header_size);
         send_count++;
         break;

      case SHADER_OPCODE_MEMORY_FENCE:
         brw_memory_fence(p, dst, src[0], BRW_OPCODE_SEND,
                          brw_message_target(inst->sfid),
                          inst->desc,
                          /* commit_enable */ false,
                          /* bti */ 0);
         send_count++;
         break;

      case SHADER_OPCODE_FIND_LIVE_CHANNEL:
         brw_find_live_channel(p, dst, false);
         break;

      case SHADER_OPCODE_BROADCAST:
         assert(inst->force_writemask_all);
         brw_broadcast(p, dst, src[0], src[1]);
         break;

      case VS_OPCODE_UNPACK_FLAGS_SIMD4X2:
         generate_unpack_flags(p, dst);
         break;

      case VEC4_OPCODE_MOV_BYTES: {
         /* Moves the low byte from each channel, using an Align1 access mode
          * and a <4,1,0> source region.
          */
         assert(src[0].type == BRW_REGISTER_TYPE_UB ||
                src[0].type == BRW_REGISTER_TYPE_B);

         brw_set_default_access_mode(p, BRW_ALIGN_1);
         src[0].vstride = BRW_VERTICAL_STRIDE_4;
         src[0].width = BRW_WIDTH_1;
         src[0].hstride = BRW_HORIZONTAL_STRIDE_0;
         brw_MOV(p, dst, src[0]);
         brw_set_default_access_mode(p, BRW_ALIGN_16);
         break;
      }

      case VEC4_OPCODE_DOUBLE_TO_F32:
      case VEC4_OPCODE_DOUBLE_TO_D32:
      case VEC4_OPCODE_DOUBLE_TO_U32: {
         assert(type_sz(src[0].type) == 8);
         assert(type_sz(dst.type) == 8);

         brw_reg_type dst_type;

         switch (inst->opcode) {
         case VEC4_OPCODE_DOUBLE_TO_F32:
            dst_type = BRW_REGISTER_TYPE_F;
            break;
         case VEC4_OPCODE_DOUBLE_TO_D32:
            dst_type = BRW_REGISTER_TYPE_D;
            break;
         case VEC4_OPCODE_DOUBLE_TO_U32:
            dst_type = BRW_REGISTER_TYPE_UD;
            break;
         default:
            unreachable("Not supported conversion");
         }
         dst = retype(dst, dst_type);

         brw_set_default_access_mode(p, BRW_ALIGN_1);

         /* When converting from DF->F, we set destination's stride as 2 as an
          * alignment requirement. But in IVB/BYT, each DF implicitly writes
          * two floats, being the first one the converted value. So we don't
          * need to explicitly set stride 2, but 1.
          */
         struct brw_reg spread_dst;
         if (devinfo->verx10 == 70)
            spread_dst = stride(dst, 8, 4, 1);
         else
            spread_dst = stride(dst, 8, 4, 2);

         brw_MOV(p, spread_dst, src[0]);

         brw_set_default_access_mode(p, BRW_ALIGN_16);
         break;
      }

      case VEC4_OPCODE_TO_DOUBLE: {
         assert(type_sz(src[0].type) == 4);
         assert(type_sz(dst.type) == 8);

         brw_set_default_access_mode(p, BRW_ALIGN_1);

         brw_MOV(p, dst, src[0]);

         brw_set_default_access_mode(p, BRW_ALIGN_16);
         break;
      }

      case VEC4_OPCODE_PICK_LOW_32BIT:
      case VEC4_OPCODE_PICK_HIGH_32BIT: {
         /* Stores the low/high 32-bit of each 64-bit element in src[0] into
          * dst using ALIGN1 mode and a <8,4,2>:UD region on the source.
          */
         assert(type_sz(src[0].type) == 8);
         assert(type_sz(dst.type) == 4);

         brw_set_default_access_mode(p, BRW_ALIGN_1);

         dst = retype(dst, BRW_REGISTER_TYPE_UD);
         dst.hstride = BRW_HORIZONTAL_STRIDE_1;

         src[0] = retype(src[0], BRW_REGISTER_TYPE_UD);
         if (inst->opcode == VEC4_OPCODE_PICK_HIGH_32BIT)
            src[0] = suboffset(src[0], 1);
         src[0] = spread(src[0], 2);
         brw_MOV(p, dst, src[0]);

         brw_set_default_access_mode(p, BRW_ALIGN_16);
         break;
      }

      case VEC4_OPCODE_SET_LOW_32BIT:
      case VEC4_OPCODE_SET_HIGH_32BIT: {
         /* Reads consecutive 32-bit elements from src[0] and writes
          * them to the low/high 32-bit of each 64-bit element in dst.
          */
         assert(type_sz(src[0].type) == 4);
         assert(type_sz(dst.type) == 8);

         brw_set_default_access_mode(p, BRW_ALIGN_1);

         dst = retype(dst, BRW_REGISTER_TYPE_UD);
         if (inst->opcode == VEC4_OPCODE_SET_HIGH_32BIT)
            dst = suboffset(dst, 1);
         dst.hstride = BRW_HORIZONTAL_STRIDE_2;

         src[0] = retype(src[0], BRW_REGISTER_TYPE_UD);
         brw_MOV(p, dst, src[0]);

         brw_set_default_access_mode(p, BRW_ALIGN_16);
         break;
      }

      case VEC4_OPCODE_PACK_BYTES: {
         /* Is effectively:
          *
          *   mov(8) dst<16,4,1>:UB src<4,1,0>:UB
          *
          * but destinations' only regioning is horizontal stride, so instead we
          * have to use two instructions:
          *
          *   mov(4) dst<1>:UB     src<4,1,0>:UB
          *   mov(4) dst.16<1>:UB  src.16<4,1,0>:UB
          *
          * where they pack the four bytes from the low and high four DW.
          */
         assert(util_is_power_of_two_nonzero(dst.writemask));
         unsigned offset = __builtin_ctz(dst.writemask);

         dst.type = BRW_REGISTER_TYPE_UB;

         brw_set_default_access_mode(p, BRW_ALIGN_1);

         src[0].type = BRW_REGISTER_TYPE_UB;
         src[0].vstride = BRW_VERTICAL_STRIDE_4;
         src[0].width = BRW_WIDTH_1;
         src[0].hstride = BRW_HORIZONTAL_STRIDE_0;
         dst.subnr = offset * 4;
         struct brw_inst *insn = brw_MOV(p, dst, src[0]);
         brw_inst_set_exec_size(p->devinfo, insn, BRW_EXECUTE_4);
         brw_inst_set_no_dd_clear(p->devinfo, insn, true);
         brw_inst_set_no_dd_check(p->devinfo, insn, inst->no_dd_check);

         src[0].subnr = 16;
         dst.subnr = 16 + offset * 4;
         insn = brw_MOV(p, dst, src[0]);
         brw_inst_set_exec_size(p->devinfo, insn, BRW_EXECUTE_4);
         brw_inst_set_no_dd_clear(p->devinfo, insn, inst->no_dd_clear);
         brw_inst_set_no_dd_check(p->devinfo, insn, true);

         brw_set_default_access_mode(p, BRW_ALIGN_16);
         break;
      }

      case VEC4_OPCODE_ZERO_OOB_PUSH_REGS:
         generate_zero_oob_push_regs(p, &prog_data->base, dst, src[0]);
         break;

      case VEC4_TCS_OPCODE_URB_WRITE:
         generate_tcs_urb_write(p, inst, src[0]);
         send_count++;
         break;

      case VEC4_OPCODE_URB_READ:
         generate_vec4_urb_read(p, inst, dst, src[0]);
         send_count++;
         break;

      case VEC4_TCS_OPCODE_SET_INPUT_URB_OFFSETS:
         generate_tcs_input_urb_offsets(p, dst, src[0], src[1]);
         break;

      case VEC4_TCS_OPCODE_SET_OUTPUT_URB_OFFSETS:
         generate_tcs_output_urb_offsets(p, dst, src[0], src[1]);
         break;

      case TCS_OPCODE_GET_INSTANCE_ID:
         generate_tcs_get_instance_id(p, dst);
         break;

      case TCS_OPCODE_GET_PRIMITIVE_ID:
         generate_tcs_get_primitive_id(p, dst);
         break;

      case TCS_OPCODE_CREATE_BARRIER_HEADER:
         generate_tcs_create_barrier_header(p, prog_data, dst);
         break;

      case TES_OPCODE_CREATE_INPUT_READ_HEADER:
         generate_tes_create_input_read_header(p, dst);
         break;

      case TES_OPCODE_ADD_INDIRECT_URB_OFFSET:
         generate_tes_add_indirect_urb_offset(p, dst, src[0], src[1]);
         break;

      case TES_OPCODE_GET_PRIMITIVE_ID:
         generate_tes_get_primitive_id(p, dst);
         break;

      case TCS_OPCODE_SRC0_010_IS_ZERO:
         /* If src_reg had stride like fs_reg, we wouldn't need this. */
         brw_MOV(p, brw_null_reg(), stride(src[0], 0, 1, 0));
         break;

      case TCS_OPCODE_RELEASE_INPUT:
         generate_tcs_release_input(p, dst, src[0], src[1]);
         send_count++;
         break;

      case TCS_OPCODE_THREAD_END:
         generate_tcs_thread_end(p, inst);
         send_count++;
         break;

      case SHADER_OPCODE_BARRIER:
         brw_barrier(p, src[0]);
         brw_WAIT(p);
         send_count++;
         break;

      case SHADER_OPCODE_MOV_INDIRECT:
         generate_mov_indirect(p, inst, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_DIM:
         assert(devinfo->verx10 == 75);
         assert(src[0].type == BRW_REGISTER_TYPE_DF);
         assert(dst.type == BRW_REGISTER_TYPE_DF);
         brw_DIM(p, dst, retype(src[0], BRW_REGISTER_TYPE_F));
         break;

      case SHADER_OPCODE_RND_MODE: {
         assert(src[0].file == BRW_IMMEDIATE_VALUE);
         /*
          * Changes the floating point rounding mode updating the control
          * register field defined at cr0.0[5-6] bits.
          */
         enum brw_rnd_mode mode =
            (enum brw_rnd_mode) (src[0].d << BRW_CR0_RND_MODE_SHIFT);
         brw_float_controls_mode(p, mode, BRW_CR0_RND_MODE_MASK);
      }
         break;

      default:
         unreachable("Unsupported opcode");
      }

      if (inst->opcode == VEC4_OPCODE_PACK_BYTES) {
         /* Handled dependency hints in the generator. */

         assert(!inst->conditional_mod);
      } else if (inst->no_dd_clear || inst->no_dd_check || inst->conditional_mod) {
         assert(p->nr_insn == pre_emit_nr_insn + 1 ||
                !"conditional_mod, no_dd_check, or no_dd_clear set for IR "
                 "emitting more than 1 instruction");

         brw_inst *last = &p->store[pre_emit_nr_insn];

         if (inst->conditional_mod)
            brw_inst_set_cond_modifier(p->devinfo, last, inst->conditional_mod);
         brw_inst_set_no_dd_clear(p->devinfo, last, inst->no_dd_clear);
         brw_inst_set_no_dd_check(p->devinfo, last, inst->no_dd_check);
      }
   }

   brw_set_uip_jip(p, 0);

   /* end of program sentinel */
   disasm_new_inst_group(disasm_info, p->next_insn_offset);

#ifndef NDEBUG
   bool validated =
#else
   if (unlikely(debug_enabled))
#endif
      brw_validate_instructions(&compiler->isa, p->store,
                                0, p->next_insn_offset,
                                disasm_info);

   int before_size = p->next_insn_offset;
   brw_compact_instructions(p, 0, disasm_info);
   int after_size = p->next_insn_offset;

   bool dump_shader_bin = brw_should_dump_shader_bin();
   unsigned char sha1[21];
   char sha1buf[41];

   if (unlikely(debug_enabled || dump_shader_bin)) {
      _mesa_sha1_compute(p->store, p->next_insn_offset, sha1);
      _mesa_sha1_format(sha1buf, sha1);
   }

   if (unlikely(dump_shader_bin))
      brw_dump_shader_bin(p->store, 0, p->next_insn_offset, sha1buf);

   if (unlikely(debug_enabled)) {
      fprintf(stderr, "Native code for %s %s shader %s (src_hash 0x%08x) (sha1 %s):\n",
            nir->info.label ? nir->info.label : "unnamed",
            _mesa_shader_stage_to_string(nir->info.stage), nir->info.name,
            params->source_hash, sha1buf);

      fprintf(stderr, "%s vec4 shader: %d instructions. %d loops. %u cycles. %d:%d "
                     "spills:fills, %u sends. Compacted %d to %d bytes (%.0f%%)\n",
            stage_abbrev, before_size / 16, loop_count, perf.latency,
            spill_count, fill_count, send_count, before_size, after_size,
            100.0f * (before_size - after_size) / before_size);

      /* overriding the shader makes disasm_info invalid */
      if (!brw_try_override_assembly(p, 0, sha1buf)) {
         dump_assembly(p->store, 0, p->next_insn_offset,
                       disasm_info, perf.block_latency);
      } else {
         fprintf(stderr, "Successfully overrode shader with sha1 %s\n\n", sha1buf);
      }
   }
   ralloc_free(disasm_info);
   assert(validated);

   brw_shader_debug_log(compiler, params->log_data,
                        "%s vec4 shader: %d inst, %d loops, %u cycles, "
                        "%d:%d spills:fills, %u sends, "
                        "compacted %d to %d bytes.\n",
                        stage_abbrev, before_size / 16,
                        loop_count, perf.latency, spill_count,
                        fill_count, send_count, before_size, after_size);
   if (stats) {
      stats->dispatch_width = 0;
      stats->max_dispatch_width = 0;
      stats->instructions = before_size / 16;
      stats->sends = send_count;
      stats->loops = loop_count;
      stats->cycles = perf.latency;
      stats->spills = spill_count;
      stats->fills = fill_count;
   }
}

extern "C" const unsigned *
brw_vec4_generate_assembly(const struct brw_compiler *compiler,
                           const struct brw_compile_params *params,
                           const nir_shader *nir,
                           struct brw_vue_prog_data *prog_data,
                           const struct cfg_t *cfg,
                           const performance &perf,
                           bool debug_enabled)
{
   struct brw_codegen *p = rzalloc(params->mem_ctx, struct brw_codegen);
   brw_init_codegen(&compiler->isa, p, params->mem_ctx);
   brw_set_default_access_mode(p, BRW_ALIGN_16);

   generate_code(p, compiler, params,
                 nir, prog_data, cfg, perf,
                 params->stats, debug_enabled);

   assert(prog_data->base.const_data_size == 0);
   if (nir->constant_data_size > 0) {
      prog_data->base.const_data_size = nir->constant_data_size;
      prog_data->base.const_data_offset =
         brw_append_data(p, nir->constant_data, nir->constant_data_size, 32);
   }

   return brw_get_program(p, &prog_data->base.program_size);
}
