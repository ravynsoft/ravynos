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

/** @file brw_fs_generator.cpp
 *
 * This file supports generating code from the FS LIR to the actual
 * native instructions.
 */

#include "brw_eu.h"
#include "brw_fs.h"
#include "brw_cfg.h"
#include "dev/intel_debug.h"
#include "util/mesa-sha1.h"
#include "util/half_float.h"

static enum brw_reg_file
brw_file_from_reg(fs_reg *reg)
{
   switch (reg->file) {
   case ARF:
      return BRW_ARCHITECTURE_REGISTER_FILE;
   case FIXED_GRF:
   case VGRF:
      return BRW_GENERAL_REGISTER_FILE;
   case MRF:
      return BRW_MESSAGE_REGISTER_FILE;
   case IMM:
      return BRW_IMMEDIATE_VALUE;
   case BAD_FILE:
   case ATTR:
   case UNIFORM:
      unreachable("not reached");
   }
   return BRW_ARCHITECTURE_REGISTER_FILE;
}

static struct brw_reg
brw_reg_from_fs_reg(const struct intel_device_info *devinfo, fs_inst *inst,
                    fs_reg *reg, bool compressed)
{
   struct brw_reg brw_reg;

   switch (reg->file) {
   case MRF:
      assert((reg->nr & ~BRW_MRF_COMPR4) < BRW_MAX_MRF(devinfo->ver));
      FALLTHROUGH;
   case VGRF:
      if (reg->stride == 0) {
         brw_reg = brw_vec1_reg(brw_file_from_reg(reg), reg->nr, 0);
      } else {
         /* From the Haswell PRM:
          *
          *  "VertStride must be used to cross GRF register boundaries. This
          *   rule implies that elements within a 'Width' cannot cross GRF
          *   boundaries."
          *
          * The maximum width value that could satisfy this restriction is:
          */
         const unsigned reg_width = REG_SIZE / (reg->stride * type_sz(reg->type));

         /* Because the hardware can only split source regions at a whole
          * multiple of width during decompression (i.e. vertically), clamp
          * the value obtained above to the physical execution size of a
          * single decompressed chunk of the instruction:
          */
         const unsigned phys_width = compressed ? inst->exec_size / 2 :
                                     inst->exec_size;

         const unsigned max_hw_width = 16;

         /* XXX - The equation above is strictly speaking not correct on
          *       hardware that supports unbalanced GRF writes -- On Gfx9+
          *       each decompressed chunk of the instruction may have a
          *       different execution size when the number of components
          *       written to each destination GRF is not the same.
          */
         if (reg->stride > 4) {
            assert(reg != &inst->dst);
            assert(reg->stride * type_sz(reg->type) <= REG_SIZE);
            brw_reg = brw_vecn_reg(1, brw_file_from_reg(reg), reg->nr, 0);
            brw_reg = stride(brw_reg, reg->stride, 1, 0);
         } else {
            const unsigned width = MIN3(reg_width, phys_width, max_hw_width);
            brw_reg = brw_vecn_reg(width, brw_file_from_reg(reg), reg->nr, 0);
            brw_reg = stride(brw_reg, width * reg->stride, width, reg->stride);
         }

         if (devinfo->verx10 == 70) {
            /* From the IvyBridge PRM (EU Changes by Processor Generation, page 13):
             *  "Each DF (Double Float) operand uses an element size of 4 rather
             *   than 8 and all regioning parameters are twice what the values
             *   would be based on the true element size: ExecSize, Width,
             *   HorzStride, and VertStride. Each DF operand uses a pair of
             *   channels and all masking and swizzing should be adjusted
             *   appropriately."
             *
             * From the IvyBridge PRM (Special Requirements for Handling Double
             * Precision Data Types, page 71):
             *  "In Align1 mode, all regioning parameters like stride, execution
             *   size, and width must use the syntax of a pair of packed
             *   floats. The offsets for these data types must be 64-bit
             *   aligned. The execution size and regioning parameters are in terms
             *   of floats."
             *
             * Summarized: when handling DF-typed arguments, ExecSize,
             * VertStride, and Width must be doubled.
             *
             * It applies to BayTrail too.
             */
            if (type_sz(reg->type) == 8) {
               brw_reg.width++;
               if (brw_reg.vstride > 0)
                  brw_reg.vstride++;
               assert(brw_reg.hstride == BRW_HORIZONTAL_STRIDE_1);
            }

            /* When converting from DF->F, we set the destination stride to 2
             * because each d2f conversion implicitly writes 2 floats, being
             * the first one the converted value. IVB/BYT actually writes two
             * F components per SIMD channel, and every other component is
             * filled with garbage.
             */
            if (reg == &inst->dst && get_exec_type_size(inst) == 8 &&
                type_sz(inst->dst.type) < 8) {
               assert(brw_reg.hstride > BRW_HORIZONTAL_STRIDE_1);
               brw_reg.hstride--;
            }
         }
      }

      brw_reg = retype(brw_reg, reg->type);
      brw_reg = byte_offset(brw_reg, reg->offset);
      brw_reg.abs = reg->abs;
      brw_reg.negate = reg->negate;
      break;
   case ARF:
   case FIXED_GRF:
   case IMM:
      assert(reg->offset == 0);
      brw_reg = reg->as_brw_reg();
      break;
   case BAD_FILE:
      /* Probably unused. */
      brw_reg = brw_null_reg();
      break;
   case ATTR:
   case UNIFORM:
      unreachable("not reached");
   }

   /* On HSW+, scalar DF sources can be accessed using the normal <0,1,0>
    * region, but on IVB and BYT DF regions must be programmed in terms of
    * floats. A <0,2,1> region accomplishes this.
    */
   if (devinfo->verx10 == 70 &&
       type_sz(reg->type) == 8 &&
       brw_reg.vstride == BRW_VERTICAL_STRIDE_0 &&
       brw_reg.width == BRW_WIDTH_1 &&
       brw_reg.hstride == BRW_HORIZONTAL_STRIDE_0) {
      brw_reg.width = BRW_WIDTH_2;
      brw_reg.hstride = BRW_HORIZONTAL_STRIDE_1;
   }

   return brw_reg;
}

fs_generator::fs_generator(const struct brw_compiler *compiler,
                           const struct brw_compile_params *params,
                           struct brw_stage_prog_data *prog_data,
                           bool runtime_check_aads_emit,
                           gl_shader_stage stage)

   : compiler(compiler), params(params),
     devinfo(compiler->devinfo),
     prog_data(prog_data), dispatch_width(0),
     runtime_check_aads_emit(runtime_check_aads_emit), debug_flag(false),
     shader_name(NULL), stage(stage), mem_ctx(params->mem_ctx)
{
   p = rzalloc(mem_ctx, struct brw_codegen);
   brw_init_codegen(&compiler->isa, p, mem_ctx);

   /* In the FS code generator, we are very careful to ensure that we always
    * set the right execution size so we don't need the EU code to "help" us
    * by trying to infer it.  Sometimes, it infers the wrong thing.
    */
   p->automatic_exec_sizes = false;
}

fs_generator::~fs_generator()
{
}

class ip_record : public exec_node {
public:
   DECLARE_RALLOC_CXX_OPERATORS(ip_record)

   ip_record(int ip)
   {
      this->ip = ip;
   }

   int ip;
};

bool
fs_generator::patch_halt_jumps()
{
   if (this->discard_halt_patches.is_empty())
      return false;

   int scale = brw_jump_scale(p->devinfo);

   if (devinfo->ver >= 6) {
      /* There is a somewhat strange undocumented requirement of using
       * HALT, according to the simulator.  If some channel has HALTed to
       * a particular UIP, then by the end of the program, every channel
       * must have HALTed to that UIP.  Furthermore, the tracking is a
       * stack, so you can't do the final halt of a UIP after starting
       * halting to a new UIP.
       *
       * Symptoms of not emitting this instruction on actual hardware
       * included GPU hangs and sparkly rendering on the piglit discard
       * tests.
       */
      brw_inst *last_halt = brw_HALT(p);
      brw_inst_set_uip(p->devinfo, last_halt, 1 * scale);
      brw_inst_set_jip(p->devinfo, last_halt, 1 * scale);
   }

   int ip = p->nr_insn;

   foreach_in_list(ip_record, patch_ip, &discard_halt_patches) {
      brw_inst *patch = &p->store[patch_ip->ip];

      assert(brw_inst_opcode(p->isa, patch) == BRW_OPCODE_HALT);
      if (devinfo->ver >= 6) {
         /* HALT takes a half-instruction distance from the pre-incremented IP. */
         brw_inst_set_uip(p->devinfo, patch, (ip - patch_ip->ip) * scale);
      } else {
         brw_set_src1(p, patch, brw_imm_d((ip - patch_ip->ip) * scale));
      }
   }

   this->discard_halt_patches.make_empty();

   if (devinfo->ver < 6) {
      /* From the g965 PRM:
       *
       *    "As DMask is not automatically reloaded into AMask upon completion
       *    of this instruction, software has to manually restore AMask upon
       *    completion."
       *
       * DMask lives in the bottom 16 bits of sr0.1.
       */
      brw_inst *reset = brw_MOV(p, brw_mask_reg(BRW_AMASK),
                                   retype(brw_sr0_reg(1), BRW_REGISTER_TYPE_UW));
      brw_inst_set_exec_size(devinfo, reset, BRW_EXECUTE_1);
      brw_inst_set_mask_control(devinfo, reset, BRW_MASK_DISABLE);
      brw_inst_set_qtr_control(devinfo, reset, BRW_COMPRESSION_NONE);
      brw_inst_set_thread_control(devinfo, reset, BRW_THREAD_SWITCH);
   }

   if (devinfo->ver == 4 && devinfo->platform != INTEL_PLATFORM_G4X) {
      /* From the g965 PRM:
       *
       *    "[DevBW, DevCL] Erratum: The subfields in mask stack register are
       *    reset to zero during graphics reset, however, they are not
       *    initialized at thread dispatch. These subfields will retain the
       *    values from the previous thread. Software should make sure the
       *    mask stack is empty (reset to zero) before terminating the thread.
       *    In case that this is not practical, software may have to reset the
       *    mask stack at the beginning of each kernel, which will impact the
       *    performance."
       *
       * Luckily we can rely on:
       *
       *    "[DevBW, DevCL] This register access restriction is not
       *    applicable, hardware does ensure execution pipeline coherency,
       *    when a mask stack register is used as an explicit source and/or
       *    destination."
       */
      brw_push_insn_state(p);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);

      brw_set_default_exec_size(p, BRW_EXECUTE_2);
      brw_MOV(p, vec2(brw_mask_stack_depth_reg(0)), brw_imm_uw(0));

      brw_set_default_exec_size(p, BRW_EXECUTE_16);
      /* Reset the if stack. */
      brw_MOV(p, retype(brw_mask_stack_reg(0), BRW_REGISTER_TYPE_UW),
              brw_imm_uw(0));

      brw_pop_insn_state(p);
   }

   return true;
}

void
fs_generator::generate_send(fs_inst *inst,
                            struct brw_reg dst,
                            struct brw_reg desc,
                            struct brw_reg ex_desc,
                            struct brw_reg payload,
                            struct brw_reg payload2)
{
   const bool dst_is_null = dst.file == BRW_ARCHITECTURE_REGISTER_FILE &&
                            dst.nr == BRW_ARF_NULL;
   const unsigned rlen = dst_is_null ? 0 : inst->size_written / REG_SIZE;

   uint32_t desc_imm = inst->desc |
      brw_message_desc(devinfo, inst->mlen, rlen, inst->header_size);

   uint32_t ex_desc_imm = inst->ex_desc |
      brw_message_ex_desc(devinfo, inst->ex_mlen);

   if (ex_desc.file != BRW_IMMEDIATE_VALUE || ex_desc.ud || ex_desc_imm ||
       inst->send_ex_desc_scratch) {
      /* If we have any sort of extended descriptor, then we need SENDS.  This
       * also covers the dual-payload case because ex_mlen goes in ex_desc.
       */
      brw_send_indirect_split_message(p, inst->sfid, dst, payload, payload2,
                                      desc, desc_imm, ex_desc, ex_desc_imm,
                                      inst->send_ex_desc_scratch,
                                      inst->send_ex_bso, inst->eot);
      if (inst->check_tdr)
         brw_inst_set_opcode(p->isa, brw_last_inst,
                             devinfo->ver >= 12 ? BRW_OPCODE_SENDC : BRW_OPCODE_SENDSC);
   } else {
      brw_send_indirect_message(p, inst->sfid, dst, payload, desc, desc_imm,
                                   inst->eot);
      if (inst->check_tdr)
         brw_inst_set_opcode(p->isa, brw_last_inst, BRW_OPCODE_SENDC);
   }
}

void
fs_generator::fire_fb_write(fs_inst *inst,
                            struct brw_reg payload,
                            struct brw_reg implied_header,
                            GLuint nr)
{
   struct brw_wm_prog_data *prog_data = brw_wm_prog_data(this->prog_data);

   if (devinfo->ver < 6) {
      brw_push_insn_state(p);
      brw_set_default_exec_size(p, BRW_EXECUTE_8);
      brw_set_default_mask_control(p, BRW_MASK_DISABLE);
      brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
      brw_set_default_flag_reg(p, 0, 0);
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
      brw_MOV(p, offset(retype(payload, BRW_REGISTER_TYPE_UD), 1),
              offset(retype(implied_header, BRW_REGISTER_TYPE_UD), 1));
      brw_pop_insn_state(p);
   }

   uint32_t msg_control = brw_fb_write_msg_control(inst, prog_data);

   /* We assume render targets start at 0, because headerless FB write
    * messages set "Render Target Index" to 0.  Using a different binding
    * table index would make it impossible to use headerless messages.
    */
   const uint32_t surf_index = inst->target;

   brw_inst *insn = brw_fb_WRITE(p,
                                 payload,
                                 retype(implied_header, BRW_REGISTER_TYPE_UW),
                                 msg_control,
                                 surf_index,
                                 nr,
                                 0,
                                 inst->eot,
                                 inst->last_rt,
                                 inst->header_size != 0);

   if (devinfo->ver >= 6)
      brw_inst_set_rt_slot_group(devinfo, insn, inst->group / 16);
}

void
fs_generator::generate_fb_write(fs_inst *inst, struct brw_reg payload)
{
   assert(devinfo->ver < 7);

   brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
   brw_set_default_flag_reg(p, 0, 0);

   const struct brw_reg implied_header =
      devinfo->ver < 6 ? payload : brw_null_reg();

   if (inst->base_mrf >= 0)
      payload = brw_message_reg(inst->base_mrf);

   if (!runtime_check_aads_emit) {
      fire_fb_write(inst, payload, implied_header, inst->mlen);
   } else {
      /* This can only happen in gen < 6 */
      assert(devinfo->ver < 6);

      struct brw_reg v1_null_ud = vec1(retype(brw_null_reg(), BRW_REGISTER_TYPE_UD));

      /* Check runtime bit to detect if we have to send AA data or not */
      brw_push_insn_state(p);
      brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
      brw_set_default_exec_size(p, BRW_EXECUTE_1);
      brw_AND(p,
              v1_null_ud,
              retype(brw_vec1_grf(1, 6), BRW_REGISTER_TYPE_UD),
              brw_imm_ud(1<<26));
      brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);

      int jmp = brw_JMPI(p, brw_imm_ud(0), BRW_PREDICATE_NORMAL) - p->store;
      brw_pop_insn_state(p);
      {
         /* Don't send AA data */
         fire_fb_write(inst, offset(payload, 1), implied_header, inst->mlen-1);
      }
      brw_land_fwd_jump(p, jmp);
      fire_fb_write(inst, payload, implied_header, inst->mlen);
   }
}

void
fs_generator::generate_fb_read(fs_inst *inst, struct brw_reg dst,
                               struct brw_reg payload)
{
   assert(inst->size_written % REG_SIZE == 0);
   struct brw_wm_prog_data *prog_data = brw_wm_prog_data(this->prog_data);
   /* We assume that render targets start at binding table index 0. */
   const unsigned surf_index = inst->target;

   gfx9_fb_READ(p, dst, payload, surf_index,
                inst->header_size, inst->size_written / REG_SIZE,
                prog_data->persample_dispatch);
}

void
fs_generator::generate_mov_indirect(fs_inst *inst,
                                    struct brw_reg dst,
                                    struct brw_reg reg,
                                    struct brw_reg indirect_byte_offset)
{
   assert(indirect_byte_offset.type == BRW_REGISTER_TYPE_UD);
   assert(indirect_byte_offset.file == BRW_GENERAL_REGISTER_FILE);
   assert(!reg.abs && !reg.negate);

   /* Gen12.5 adds the following region restriction:
    *
    *    "Vx1 and VxH indirect addressing for Float, Half-Float, Double-Float
    *    and Quad-Word data must not be used."
    *
    * We require the source and destination types to match so stomp to an
    * unsigned integer type.
    */
   assert(reg.type == dst.type);
   reg.type = dst.type = brw_reg_type_from_bit_size(type_sz(reg.type) * 8,
                                                    BRW_REGISTER_TYPE_UD);

   unsigned imm_byte_offset = reg.nr * REG_SIZE + reg.subnr;

   if (indirect_byte_offset.file == BRW_IMMEDIATE_VALUE) {
      imm_byte_offset += indirect_byte_offset.ud;

      reg.nr = imm_byte_offset / REG_SIZE;
      reg.subnr = imm_byte_offset % REG_SIZE;
      if (type_sz(reg.type) > 4 && !devinfo->has_64bit_float) {
         brw_MOV(p, subscript(dst, BRW_REGISTER_TYPE_D, 0),
                    subscript(reg, BRW_REGISTER_TYPE_D, 0));
         brw_set_default_swsb(p, tgl_swsb_null());
         brw_MOV(p, subscript(dst, BRW_REGISTER_TYPE_D, 1),
                    subscript(reg, BRW_REGISTER_TYPE_D, 1));
      } else {
         brw_MOV(p, dst, reg);
      }
   } else {
      /* Prior to Broadwell, there are only 8 address registers. */
      assert(inst->exec_size <= 8 || devinfo->ver >= 8);

      /* We use VxH indirect addressing, clobbering a0.0 through a0.7. */
      struct brw_reg addr = vec8(brw_address_reg(0));

      /* Whether we can use destination dependency control without running the
       * risk of a hang if an instruction gets shot down.
       */
      const bool use_dep_ctrl = !inst->predicate &&
                                inst->exec_size == dispatch_width;
      brw_inst *insn;

      /* The destination stride of an instruction (in bytes) must be greater
       * than or equal to the size of the rest of the instruction.  Since the
       * address register is of type UW, we can't use a D-type instruction.
       * In order to get around this, re retype to UW and use a stride.
       */
      indirect_byte_offset =
         retype(spread(indirect_byte_offset, 2), BRW_REGISTER_TYPE_UW);

      /* There are a number of reasons why we don't use the base offset here.
       * One reason is that the field is only 9 bits which means we can only
       * use it to access the first 16 GRFs.  Also, from the Haswell PRM
       * section "Register Region Restrictions":
       *
       *    "The lower bits of the AddressImmediate must not overflow to
       *    change the register address.  The lower 5 bits of Address
       *    Immediate when added to lower 5 bits of address register gives
       *    the sub-register offset. The upper bits of Address Immediate
       *    when added to upper bits of address register gives the register
       *    address. Any overflow from sub-register offset is dropped."
       *
       * Since the indirect may cause us to cross a register boundary, this
       * makes the base offset almost useless.  We could try and do something
       * clever where we use a actual base offset if base_offset % 32 == 0 but
       * that would mean we were generating different code depending on the
       * base offset.  Instead, for the sake of consistency, we'll just do the
       * add ourselves.  This restriction is only listed in the Haswell PRM
       * but empirical testing indicates that it applies on all older
       * generations and is lifted on Broadwell.
       *
       * In the end, while base_offset is nice to look at in the generated
       * code, using it saves us 0 instructions and would require quite a bit
       * of case-by-case work.  It's just not worth it.
       *
       * Due to a hardware bug some platforms (particularly Gfx11+) seem to
       * require the address components of all channels to be valid whether or
       * not they're active, which causes issues if we use VxH addressing
       * under non-uniform control-flow.  We can easily work around that by
       * initializing the whole address register with a pipelined NoMask MOV
       * instruction.
       */
      if (devinfo->ver >= 7) {
         insn = brw_MOV(p, addr, brw_imm_uw(imm_byte_offset));
         brw_inst_set_mask_control(devinfo, insn, BRW_MASK_DISABLE);
         brw_inst_set_pred_control(devinfo, insn, BRW_PREDICATE_NONE);
         if (devinfo->ver >= 12)
            brw_set_default_swsb(p, tgl_swsb_null());
         else
            brw_inst_set_no_dd_clear(devinfo, insn, use_dep_ctrl);
      }

      insn = brw_ADD(p, addr, indirect_byte_offset, brw_imm_uw(imm_byte_offset));
      if (devinfo->ver >= 12)
         brw_set_default_swsb(p, tgl_swsb_regdist(1));
      else if (devinfo->ver >= 7)
         brw_inst_set_no_dd_check(devinfo, insn, use_dep_ctrl);

      if (type_sz(reg.type) > 4 &&
          ((devinfo->verx10 == 70) ||
           devinfo->platform == INTEL_PLATFORM_CHV || intel_device_info_is_9lp(devinfo) ||
           !devinfo->has_64bit_float || devinfo->verx10 >= 125)) {
         /* IVB has an issue (which we found empirically) where it reads two
          * address register components per channel for indirectly addressed
          * 64-bit sources.
          *
          * From the Cherryview PRM Vol 7. "Register Region Restrictions":
          *
          *    "When source or destination datatype is 64b or operation is
          *    integer DWord multiply, indirect addressing must not be used."
          *
          * To work around both of these, we do two integer MOVs insead of one
          * 64-bit MOV.  Because no double value should ever cross a register
          * boundary, it's safe to use the immediate offset in the indirect
          * here to handle adding 4 bytes to the offset and avoid the extra
          * ADD to the register file.
          */
         brw_MOV(p, subscript(dst, BRW_REGISTER_TYPE_D, 0),
                    retype(brw_VxH_indirect(0, 0), BRW_REGISTER_TYPE_D));
         brw_set_default_swsb(p, tgl_swsb_null());
         brw_MOV(p, subscript(dst, BRW_REGISTER_TYPE_D, 1),
                    retype(brw_VxH_indirect(0, 4), BRW_REGISTER_TYPE_D));
      } else {
         struct brw_reg ind_src = brw_VxH_indirect(0, 0);

         brw_inst *mov = brw_MOV(p, dst, retype(ind_src, reg.type));

         if (devinfo->ver == 6 && dst.file == BRW_MESSAGE_REGISTER_FILE &&
             !inst->get_next()->is_tail_sentinel() &&
             ((fs_inst *)inst->get_next())->mlen > 0) {
            /* From the Sandybridge PRM:
             *
             *    "[Errata: DevSNB(SNB)] If MRF register is updated by any
             *    instruction that “indexed/indirect” source AND is followed
             *    by a send, the instruction requires a “Switch”. This is to
             *    avoid race condition where send may dispatch before MRF is
             *    updated."
             */
            brw_inst_set_thread_control(devinfo, mov, BRW_THREAD_SWITCH);
         }
      }
   }
}

void
fs_generator::generate_shuffle(fs_inst *inst,
                               struct brw_reg dst,
                               struct brw_reg src,
                               struct brw_reg idx)
{
   assert(src.file == BRW_GENERAL_REGISTER_FILE);
   assert(!src.abs && !src.negate);

   /* Ivy bridge has some strange behavior that makes this a real pain to
    * implement for 64-bit values so we just don't bother.
    */
   assert((devinfo->verx10 >= 75 && devinfo->has_64bit_float) ||
          type_sz(src.type) <= 4);

   /* Gen12.5 adds the following region restriction:
    *
    *    "Vx1 and VxH indirect addressing for Float, Half-Float, Double-Float
    *    and Quad-Word data must not be used."
    *
    * We require the source and destination types to match so stomp to an
    * unsigned integer type.
    */
   assert(src.type == dst.type);
   src.type = dst.type = brw_reg_type_from_bit_size(type_sz(src.type) * 8,
                                                    BRW_REGISTER_TYPE_UD);

   /* Because we're using the address register, we're limited to 8-wide
    * execution on gfx7.  On gfx8, we're limited to 16-wide by the address
    * register file and 8-wide for 64-bit types.  We could try and make this
    * instruction splittable higher up in the compiler but that gets weird
    * because it reads all of the channels regardless of execution size.  It's
    * easier just to split it here.
    */
   const unsigned lower_width =
      devinfo->ver <= 7 || element_sz(src) > 4 || element_sz(dst) > 4 ? 8 :
      MIN2(16, inst->exec_size);

   brw_set_default_exec_size(p, cvt(lower_width) - 1);
   for (unsigned group = 0; group < inst->exec_size; group += lower_width) {
      brw_set_default_group(p, group);

      if ((src.vstride == 0 && src.hstride == 0) ||
          idx.file == BRW_IMMEDIATE_VALUE) {
         /* Trivial, the source is already uniform or the index is a constant.
          * We will typically not get here if the optimizer is doing its job,
          * but asserting would be mean.
          */
         const unsigned i = idx.file == BRW_IMMEDIATE_VALUE ? idx.ud : 0;
         struct brw_reg group_src = stride(suboffset(src, i), 0, 1, 0);
         struct brw_reg group_dst = suboffset(dst, group << (dst.hstride - 1));
         brw_MOV(p, group_dst, group_src);
      } else {
         /* We use VxH indirect addressing, clobbering a0.0 through a0.7. */
         struct brw_reg addr = vec8(brw_address_reg(0));

         struct brw_reg group_idx = suboffset(idx, group);

         if (lower_width == 8 && group_idx.width == BRW_WIDTH_16) {
            /* Things get grumpy if the register is too wide. */
            group_idx.width--;
            group_idx.vstride--;
         }

         assert(type_sz(group_idx.type) <= 4);
         if (type_sz(group_idx.type) == 4) {
            /* The destination stride of an instruction (in bytes) must be
             * greater than or equal to the size of the rest of the
             * instruction.  Since the address register is of type UW, we
             * can't use a D-type instruction.  In order to get around this,
             * re retype to UW and use a stride.
             */
            group_idx = retype(spread(group_idx, 2), BRW_REGISTER_TYPE_W);
         }

         uint32_t src_start_offset = src.nr * REG_SIZE + src.subnr;

         /* From the Haswell PRM:
          *
          *    "When a sequence of NoDDChk and NoDDClr are used, the last
          *    instruction that completes the scoreboard clear must have a
          *    non-zero execution mask. This means, if any kind of predication
          *    can change the execution mask or channel enable of the last
          *    instruction, the optimization must be avoided.  This is to
          *    avoid instructions being shot down the pipeline when no writes
          *    are required."
          *
          * Whenever predication is enabled or the instructions being emitted
          * aren't the full width, it's possible that it will be run with zero
          * channels enabled so we can't use dependency control without
          * running the risk of a hang if an instruction gets shot down.
          */
         const bool use_dep_ctrl = !inst->predicate &&
                                   lower_width == dispatch_width;
         brw_inst *insn;

         /* Due to a hardware bug some platforms (particularly Gfx11+) seem
          * to require the address components of all channels to be valid
          * whether or not they're active, which causes issues if we use VxH
          * addressing under non-uniform control-flow.  We can easily work
          * around that by initializing the whole address register with a
          * pipelined NoMask MOV instruction.
          */
         insn = brw_MOV(p, addr, brw_imm_uw(src_start_offset));
         brw_inst_set_mask_control(devinfo, insn, BRW_MASK_DISABLE);
         brw_inst_set_pred_control(devinfo, insn, BRW_PREDICATE_NONE);
         if (devinfo->ver >= 12)
            brw_set_default_swsb(p, tgl_swsb_null());
         else
            brw_inst_set_no_dd_clear(devinfo, insn, use_dep_ctrl);

         /* Take into account the component size and horizontal stride. */
         assert(src.vstride == src.hstride + src.width);
         insn = brw_SHL(p, addr, group_idx,
                        brw_imm_uw(util_logbase2(type_sz(src.type)) +
                                   src.hstride - 1));
         if (devinfo->ver >= 12)
            brw_set_default_swsb(p, tgl_swsb_regdist(1));
         else
            brw_inst_set_no_dd_check(devinfo, insn, use_dep_ctrl);

         /* Add on the register start offset */
         brw_ADD(p, addr, addr, brw_imm_uw(src_start_offset));
         brw_MOV(p, suboffset(dst, group << (dst.hstride - 1)),
                 retype(brw_VxH_indirect(0, 0), src.type));
      }

      brw_set_default_swsb(p, tgl_swsb_null());
   }
}

void
fs_generator::generate_quad_swizzle(const fs_inst *inst,
                                    struct brw_reg dst, struct brw_reg src,
                                    unsigned swiz)
{
   /* Requires a quad. */
   assert(inst->exec_size >= 4);

   if (src.file == BRW_IMMEDIATE_VALUE ||
       has_scalar_region(src)) {
      /* The value is uniform across all channels */
      brw_MOV(p, dst, src);

   } else if (devinfo->ver < 11 && type_sz(src.type) == 4) {
      /* This only works on 8-wide 32-bit values */
      assert(inst->exec_size == 8);
      assert(src.hstride == BRW_HORIZONTAL_STRIDE_1);
      assert(src.vstride == src.width + 1);
      brw_set_default_access_mode(p, BRW_ALIGN_16);
      struct brw_reg swiz_src = stride(src, 4, 4, 1);
      swiz_src.swizzle = swiz;
      brw_MOV(p, dst, swiz_src);

   } else {
      assert(src.hstride == BRW_HORIZONTAL_STRIDE_1);
      assert(src.vstride == src.width + 1);
      const struct brw_reg src_0 = suboffset(src, BRW_GET_SWZ(swiz, 0));

      switch (swiz) {
      case BRW_SWIZZLE_XXXX:
      case BRW_SWIZZLE_YYYY:
      case BRW_SWIZZLE_ZZZZ:
      case BRW_SWIZZLE_WWWW:
         brw_MOV(p, dst, stride(src_0, 4, 4, 0));
         break;

      case BRW_SWIZZLE_XXZZ:
      case BRW_SWIZZLE_YYWW:
         brw_MOV(p, dst, stride(src_0, 2, 2, 0));
         break;

      case BRW_SWIZZLE_XYXY:
      case BRW_SWIZZLE_ZWZW:
         assert(inst->exec_size == 4);
         brw_MOV(p, dst, stride(src_0, 0, 2, 1));
         break;

      default:
         assert(inst->force_writemask_all);
         brw_set_default_exec_size(p, cvt(inst->exec_size / 4) - 1);

         for (unsigned c = 0; c < 4; c++) {
            brw_inst *insn = brw_MOV(
               p, stride(suboffset(dst, c),
                         4 * inst->dst.stride, 1, 4 * inst->dst.stride),
               stride(suboffset(src, BRW_GET_SWZ(swiz, c)), 4, 1, 0));

            if (devinfo->ver < 12) {
               brw_inst_set_no_dd_clear(devinfo, insn, c < 3);
               brw_inst_set_no_dd_check(devinfo, insn, c > 0);
            }

            brw_set_default_swsb(p, tgl_swsb_null());
         }

         break;
      }
   }
}

void
fs_generator::generate_cs_terminate(fs_inst *inst, struct brw_reg payload)
{
   struct brw_inst *insn;

   insn = brw_next_insn(p, BRW_OPCODE_SEND);

   brw_set_dest(p, insn, retype(brw_null_reg(), BRW_REGISTER_TYPE_UW));
   brw_set_src0(p, insn, retype(payload, BRW_REGISTER_TYPE_UW));
   if (devinfo->ver < 12)
      brw_set_src1(p, insn, brw_imm_ud(0u));

   /* For XeHP and newer send a message to the message gateway to terminate a
    * compute shader. For older devices, a message is sent to the thread
    * spawner.
    */
   if (devinfo->verx10 >= 125)
      brw_inst_set_sfid(devinfo, insn, BRW_SFID_MESSAGE_GATEWAY);
   else
      brw_inst_set_sfid(devinfo, insn, BRW_SFID_THREAD_SPAWNER);
   brw_inst_set_mlen(devinfo, insn, 1);
   brw_inst_set_rlen(devinfo, insn, 0);
   brw_inst_set_eot(devinfo, insn, inst->eot);
   brw_inst_set_header_present(devinfo, insn, false);

   brw_inst_set_ts_opcode(devinfo, insn, 0); /* Dereference resource */

   if (devinfo->ver < 11) {
      brw_inst_set_ts_request_type(devinfo, insn, 0); /* Root thread */

      /* Note that even though the thread has a URB resource associated with it,
       * we set the "do not dereference URB" bit, because the URB resource is
       * managed by the fixed-function unit, so it will free it automatically.
       */
      brw_inst_set_ts_resource_select(devinfo, insn, 1); /* Do not dereference URB */
   }

   brw_inst_set_mask_control(devinfo, insn, BRW_MASK_DISABLE);
}

void
fs_generator::generate_barrier(fs_inst *, struct brw_reg src)
{
   brw_barrier(p, src);
   if (devinfo->ver >= 12) {
      brw_set_default_swsb(p, tgl_swsb_null());
      brw_SYNC(p, TGL_SYNC_BAR);
   } else {
      brw_WAIT(p);
   }
}

bool
fs_generator::generate_linterp(fs_inst *inst,
                               struct brw_reg dst, struct brw_reg *src)
{
   /* PLN reads:
    *                      /   in SIMD16   \
    *    -----------------------------------
    *   | src1+0 | src1+1 | src1+2 | src1+3 |
    *   |-----------------------------------|
    *   |(x0, x1)|(y0, y1)|(x2, x3)|(y2, y3)|
    *    -----------------------------------
    *
    * but for the LINE/MAC pair, the LINE reads Xs and the MAC reads Ys:
    *
    *    -----------------------------------
    *   | src1+0 | src1+1 | src1+2 | src1+3 |
    *   |-----------------------------------|
    *   |(x0, x1)|(y0, y1)|        |        | in SIMD8
    *   |-----------------------------------|
    *   |(x0, x1)|(x2, x3)|(y0, y1)|(y2, y3)| in SIMD16
    *    -----------------------------------
    *
    * See also: emit_interpolation_setup_gfx4().
    */
   struct brw_reg delta_x = src[0];
   struct brw_reg delta_y = offset(src[0], inst->exec_size / 8);
   struct brw_reg interp = src[1];
   brw_inst *i[2];

   /* nir_lower_interpolation() will do the lowering to MAD instructions for
    * us on gfx11+
    */
   assert(devinfo->ver < 11);

   if (devinfo->has_pln) {
      if (devinfo->ver <= 6 && (delta_x.nr & 1) != 0) {
         /* From the Sandy Bridge PRM Vol. 4, Pt. 2, Section 8.3.53, "Plane":
          *
          *    "[DevSNB]:<src1> must be even register aligned.
          *
          * This restriction is lifted on Ivy Bridge.
          *
          * This means that we need to split PLN into LINE+MAC on-the-fly.
          * Unfortunately, the inputs are laid out for PLN and not LINE+MAC so
          * we have to split into SIMD8 pieces.  For gfx4 (!has_pln), the
          * coordinate registers are laid out differently so we leave it as a
          * SIMD16 instruction.
          */
         assert(inst->exec_size == 8 || inst->exec_size == 16);
         assert(inst->group % 16 == 0);

         brw_push_insn_state(p);
         brw_set_default_exec_size(p, BRW_EXECUTE_8);

         /* Thanks to two accumulators, we can emit all the LINEs and then all
          * the MACs.  This improves parallelism a bit.
          */
         for (unsigned g = 0; g < inst->exec_size / 8; g++) {
            brw_inst *line = brw_LINE(p, brw_null_reg(), interp,
                                      offset(delta_x, g * 2));
            brw_inst_set_group(devinfo, line, inst->group + g * 8);

            /* LINE writes the accumulator automatically on gfx4-5.  On Sandy
             * Bridge and later, we have to explicitly enable it.
             */
            if (devinfo->ver >= 6)
               brw_inst_set_acc_wr_control(p->devinfo, line, true);

            /* brw_set_default_saturate() is called before emitting
             * instructions, so the saturate bit is set in each instruction,
             * so we need to unset it on the LINE instructions.
             */
            brw_inst_set_saturate(p->devinfo, line, false);
         }

         for (unsigned g = 0; g < inst->exec_size / 8; g++) {
            brw_inst *mac = brw_MAC(p, offset(dst, g), suboffset(interp, 1),
                                    offset(delta_x, g * 2 + 1));
            brw_inst_set_group(devinfo, mac, inst->group + g * 8);
            brw_inst_set_cond_modifier(p->devinfo, mac, inst->conditional_mod);
         }

         brw_pop_insn_state(p);

         return true;
      } else {
         brw_PLN(p, dst, interp, delta_x);

         return false;
      }
   } else {
      i[0] = brw_LINE(p, brw_null_reg(), interp, delta_x);
      i[1] = brw_MAC(p, dst, suboffset(interp, 1), delta_y);

      brw_inst_set_cond_modifier(p->devinfo, i[1], inst->conditional_mod);

      /* brw_set_default_saturate() is called before emitting instructions, so
       * the saturate bit is set in each instruction, so we need to unset it on
       * the first instruction.
       */
      brw_inst_set_saturate(p->devinfo, i[0], false);

      return true;
   }
}

void
fs_generator::generate_tex(fs_inst *inst, struct brw_reg dst,
                           struct brw_reg surface_index,
                           struct brw_reg sampler_index)
{
   assert(devinfo->ver < 7);
   assert(inst->size_written % REG_SIZE == 0);
   int msg_type = -1;
   uint32_t simd_mode;
   uint32_t return_format;

   /* Sampler EOT message of less than the dispatch width would kill the
    * thread prematurely.
    */
   assert(!inst->eot || inst->exec_size == dispatch_width);

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

   switch (inst->exec_size) {
   case 8:
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD8;
      break;
   case 16:
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
      break;
   default:
      unreachable("Invalid width for texture instruction");
   }

   if (devinfo->ver >= 5) {
      switch (inst->opcode) {
      case SHADER_OPCODE_TEX:
	 if (inst->shadow_compare) {
	    msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_COMPARE;
	 } else {
	    msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE;
	 }
	 break;
      case FS_OPCODE_TXB:
	 if (inst->shadow_compare) {
	    msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_BIAS_COMPARE;
	 } else {
	    msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_BIAS;
	 }
	 break;
      case SHADER_OPCODE_TXL:
	 if (inst->shadow_compare) {
	    msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LOD_COMPARE;
	 } else {
	    msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LOD;
	 }
	 break;
      case SHADER_OPCODE_TXS:
	 msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_RESINFO;
	 break;
      case SHADER_OPCODE_TXD:
         assert(!inst->shadow_compare);
         msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_DERIVS;
	 break;
      case SHADER_OPCODE_TXF:
	 msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LD;
	 break;
      case SHADER_OPCODE_TXF_CMS:
         msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LD;
         break;
      case SHADER_OPCODE_LOD:
         msg_type = GFX5_SAMPLER_MESSAGE_LOD;
         break;
      case SHADER_OPCODE_TG4:
         assert(devinfo->ver == 6);
         assert(!inst->shadow_compare);
         msg_type = GFX7_SAMPLER_MESSAGE_SAMPLE_GATHER4;
         break;
      case SHADER_OPCODE_SAMPLEINFO:
         msg_type = GFX6_SAMPLER_MESSAGE_SAMPLE_SAMPLEINFO;
         break;
      default:
	 unreachable("not reached");
      }
   } else {
      switch (inst->opcode) {
      case SHADER_OPCODE_TEX:
	 /* Note that G45 and older determines shadow compare and dispatch width
	  * from message length for most messages.
	  */
         if (inst->exec_size == 8) {
            msg_type = BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE;
            if (inst->shadow_compare) {
               assert(inst->mlen == 6);
            } else {
               assert(inst->mlen <= 4);
            }
         } else {
            if (inst->shadow_compare) {
               msg_type = BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_COMPARE;
               assert(inst->mlen == 9);
            } else {
               msg_type = BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE;
               assert(inst->mlen <= 7 && inst->mlen % 2 == 1);
            }
         }
	 break;
      case FS_OPCODE_TXB:
	 if (inst->shadow_compare) {
            assert(inst->exec_size == 8);
	    assert(inst->mlen == 6);
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_BIAS_COMPARE;
	 } else {
	    assert(inst->mlen == 9);
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_BIAS;
	    simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
	 }
	 break;
      case SHADER_OPCODE_TXL:
	 if (inst->shadow_compare) {
            assert(inst->exec_size == 8);
	    assert(inst->mlen == 6);
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_LOD_COMPARE;
	 } else {
	    assert(inst->mlen == 9);
	    msg_type = BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_LOD;
	    simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
	 }
	 break;
      case SHADER_OPCODE_TXD:
	 /* There is no sample_d_c message; comparisons are done manually */
         assert(inst->exec_size == 8);
	 assert(inst->mlen == 7 || inst->mlen == 10);
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_GRADIENTS;
	 break;
      case SHADER_OPCODE_TXF:
         assert(inst->mlen <= 9 && inst->mlen % 2 == 1);
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD16_LD;
	 simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
	 break;
      case SHADER_OPCODE_TXS:
	 assert(inst->mlen == 3);
	 msg_type = BRW_SAMPLER_MESSAGE_SIMD16_RESINFO;
	 simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
	 break;
      default:
	 unreachable("not reached");
      }
   }
   assert(msg_type != -1);

   if (simd_mode == BRW_SAMPLER_SIMD_MODE_SIMD16) {
      dst = vec16(dst);
   }

   assert(sampler_index.type == BRW_REGISTER_TYPE_UD);

   /* Load the message header if present.  If there's a texture offset,
    * we need to set it up explicitly and load the offset bitfield.
    * Otherwise, we can use an implied move from g0 to the first message reg.
    */
   struct brw_reg src = brw_null_reg();
   if (inst->header_size != 0) {
      if (devinfo->ver < 6 && !inst->offset) {
         /* Set up an implied move from g0 to the MRF. */
         src = retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UW);
      } else {
         const tgl_swsb swsb = brw_get_default_swsb(p);
         assert(inst->base_mrf != -1);
         struct brw_reg header_reg = brw_message_reg(inst->base_mrf);

         brw_push_insn_state(p);
         brw_set_default_swsb(p, tgl_swsb_src_dep(swsb));
         brw_set_default_exec_size(p, BRW_EXECUTE_8);
         brw_set_default_mask_control(p, BRW_MASK_DISABLE);
         brw_set_default_compression_control(p, BRW_COMPRESSION_NONE);
         /* Explicitly set up the message header by copying g0 to the MRF. */
         brw_MOV(p, header_reg, brw_vec8_grf(0, 0));
         brw_set_default_swsb(p, tgl_swsb_regdist(1));

         brw_set_default_exec_size(p, BRW_EXECUTE_1);
         if (inst->offset) {
            /* Set the offset bits in DWord 2. */
            brw_MOV(p, get_element_ud(header_reg, 2),
                       brw_imm_ud(inst->offset));
         }

         brw_pop_insn_state(p);
         brw_set_default_swsb(p, tgl_swsb_dst_dep(swsb, 1));
      }
   }

   assert(surface_index.file == BRW_IMMEDIATE_VALUE);
   assert(sampler_index.file == BRW_IMMEDIATE_VALUE);

   brw_SAMPLE(p,
              retype(dst, BRW_REGISTER_TYPE_UW),
              inst->base_mrf,
              src,
              surface_index.ud,
              sampler_index.ud % 16,
              msg_type,
              inst->size_written / REG_SIZE,
              inst->mlen,
              inst->header_size != 0,
              simd_mode,
              return_format);
}


/* For OPCODE_DDX and OPCODE_DDY, per channel of output we've got input
 * looking like:
 *
 * arg0: ss0.tl ss0.tr ss0.bl ss0.br ss1.tl ss1.tr ss1.bl ss1.br
 *
 * Ideally, we want to produce:
 *
 *           DDX                     DDY
 * dst: (ss0.tr - ss0.tl)     (ss0.tl - ss0.bl)
 *      (ss0.tr - ss0.tl)     (ss0.tr - ss0.br)
 *      (ss0.br - ss0.bl)     (ss0.tl - ss0.bl)
 *      (ss0.br - ss0.bl)     (ss0.tr - ss0.br)
 *      (ss1.tr - ss1.tl)     (ss1.tl - ss1.bl)
 *      (ss1.tr - ss1.tl)     (ss1.tr - ss1.br)
 *      (ss1.br - ss1.bl)     (ss1.tl - ss1.bl)
 *      (ss1.br - ss1.bl)     (ss1.tr - ss1.br)
 *
 * and add another set of two more subspans if in 16-pixel dispatch mode.
 *
 * For DDX, it ends up being easy: width = 2, horiz=0 gets us the same result
 * for each pair, and vertstride = 2 jumps us 2 elements after processing a
 * pair.  But the ideal approximation may impose a huge performance cost on
 * sample_d.  On at least Haswell, sample_d instruction does some
 * optimizations if the same LOD is used for all pixels in the subspan.
 *
 * For DDY, we need to use ALIGN16 mode since it's capable of doing the
 * appropriate swizzling.
 */
void
fs_generator::generate_ddx(const fs_inst *inst,
                           struct brw_reg dst, struct brw_reg src)
{
   unsigned vstride, width;

   if (devinfo->ver >= 8) {
      if (inst->opcode == FS_OPCODE_DDX_FINE) {
         /* produce accurate derivatives */
         vstride = BRW_VERTICAL_STRIDE_2;
         width = BRW_WIDTH_2;
      } else {
         /* replicate the derivative at the top-left pixel to other pixels */
         vstride = BRW_VERTICAL_STRIDE_4;
         width = BRW_WIDTH_4;
      }

      struct brw_reg src0 = byte_offset(src, type_sz(src.type));;
      struct brw_reg src1 = src;

      src0.vstride = vstride;
      src0.width   = width;
      src0.hstride = BRW_HORIZONTAL_STRIDE_0;
      src1.vstride = vstride;
      src1.width   = width;
      src1.hstride = BRW_HORIZONTAL_STRIDE_0;

      brw_ADD(p, dst, src0, negate(src1));
   } else {
      /* On Haswell and earlier, the region used above appears to not work
       * correctly for compressed instructions.  At least on Haswell and
       * Iron Lake, compressed ALIGN16 instructions do work.  Since we
       * would have to split to SIMD8 no matter which method we choose, we
       * may as well use ALIGN16 on all platforms gfx7 and earlier.
       */
      struct brw_reg src0 = stride(src, 4, 4, 1);
      struct brw_reg src1 = stride(src, 4, 4, 1);
      if (inst->opcode == FS_OPCODE_DDX_FINE) {
         src0.swizzle = BRW_SWIZZLE_XXZZ;
         src1.swizzle = BRW_SWIZZLE_YYWW;
      } else {
         src0.swizzle = BRW_SWIZZLE_XXXX;
         src1.swizzle = BRW_SWIZZLE_YYYY;
      }

      brw_push_insn_state(p);
      brw_set_default_access_mode(p, BRW_ALIGN_16);
      brw_ADD(p, dst, negate(src0), src1);
      brw_pop_insn_state(p);
   }
}

/* The negate_value boolean is used to negate the derivative computation for
 * FBOs, since they place the origin at the upper left instead of the lower
 * left.
 */
void
fs_generator::generate_ddy(const fs_inst *inst,
                           struct brw_reg dst, struct brw_reg src)
{
   const uint32_t type_size = type_sz(src.type);

   if (inst->opcode == FS_OPCODE_DDY_FINE) {
      /* produce accurate derivatives.
       *
       * From the Broadwell PRM, Volume 7 (3D-Media-GPGPU)
       * "Register Region Restrictions", Section "1. Special Restrictions":
       *
       *    "In Align16 mode, the channel selects and channel enables apply to
       *     a pair of half-floats, because these parameters are defined for
       *     DWord elements ONLY. This is applicable when both source and
       *     destination are half-floats."
       *
       * So for half-float operations we use the Gfx11+ Align1 path. CHV
       * inherits its FP16 hardware from SKL, so it is not affected.
       */
      if (devinfo->ver >= 11 ||
          (devinfo->platform == INTEL_PLATFORM_BDW && src.type == BRW_REGISTER_TYPE_HF)) {
         src = stride(src, 0, 2, 1);

         brw_push_insn_state(p);
         brw_set_default_exec_size(p, BRW_EXECUTE_4);
         for (uint32_t g = 0; g < inst->exec_size; g += 4) {
            brw_set_default_group(p, inst->group + g);
            brw_ADD(p, byte_offset(dst, g * type_size),
                       negate(byte_offset(src,  g * type_size)),
                       byte_offset(src, (g + 2) * type_size));
            brw_set_default_swsb(p, tgl_swsb_null());
         }
         brw_pop_insn_state(p);
      } else {
         struct brw_reg src0 = stride(src, 4, 4, 1);
         struct brw_reg src1 = stride(src, 4, 4, 1);
         src0.swizzle = BRW_SWIZZLE_XYXY;
         src1.swizzle = BRW_SWIZZLE_ZWZW;

         brw_push_insn_state(p);
         brw_set_default_access_mode(p, BRW_ALIGN_16);
         brw_ADD(p, dst, negate(src0), src1);
         brw_pop_insn_state(p);
      }
   } else {
      /* replicate the derivative at the top-left pixel to other pixels */
      if (devinfo->ver >= 8) {
         struct brw_reg src0 = byte_offset(stride(src, 4, 4, 0), 0 * type_size);
         struct brw_reg src1 = byte_offset(stride(src, 4, 4, 0), 2 * type_size);

         brw_ADD(p, dst, negate(src0), src1);
      } else {
         /* On Haswell and earlier, the region used above appears to not work
          * correctly for compressed instructions.  At least on Haswell and
          * Iron Lake, compressed ALIGN16 instructions do work.  Since we
          * would have to split to SIMD8 no matter which method we choose, we
          * may as well use ALIGN16 on all platforms gfx7 and earlier.
          */
         struct brw_reg src0 = stride(src, 4, 4, 1);
         struct brw_reg src1 = stride(src, 4, 4, 1);
         src0.swizzle = BRW_SWIZZLE_XXXX;
         src1.swizzle = BRW_SWIZZLE_ZZZZ;

         brw_push_insn_state(p);
         brw_set_default_access_mode(p, BRW_ALIGN_16);
         brw_ADD(p, dst, negate(src0), src1);
         brw_pop_insn_state(p);
      }
   }
}

void
fs_generator::generate_halt(fs_inst *)
{
   /* This HALT will be patched up at FB write time to point UIP at the end of
    * the program, and at brw_uip_jip() JIP will be set to the end of the
    * current block (or the program).
    */
   this->discard_halt_patches.push_tail(new(mem_ctx) ip_record(p->nr_insn));
   brw_HALT(p);
}

void
fs_generator::generate_scratch_write(fs_inst *inst, struct brw_reg src)
{
   /* The 32-wide messages only respect the first 16-wide half of the channel
    * enable signals which are replicated identically for the second group of
    * 16 channels, so we cannot use them unless the write is marked
    * force_writemask_all.
    */
   const unsigned lower_size = inst->force_writemask_all ? inst->exec_size :
                               MIN2(16, inst->exec_size);
   const unsigned block_size = 4 * lower_size / REG_SIZE;
   const tgl_swsb swsb = brw_get_default_swsb(p);
   assert(inst->mlen != 0);

   brw_push_insn_state(p);
   brw_set_default_exec_size(p, cvt(lower_size) - 1);
   brw_set_default_compression(p, lower_size > 8);

   for (unsigned i = 0; i < inst->exec_size / lower_size; i++) {
      brw_set_default_group(p, inst->group + lower_size * i);

      if (i > 0) {
         assert(swsb.mode & TGL_SBID_SET);
         brw_set_default_swsb(p, tgl_swsb_sbid(TGL_SBID_SRC, swsb.sbid));
      } else {
         brw_set_default_swsb(p, tgl_swsb_src_dep(swsb));
      }

      brw_MOV(p, brw_uvec_mrf(lower_size, inst->base_mrf + 1, 0),
              retype(offset(src, block_size * i), BRW_REGISTER_TYPE_UD));

      brw_set_default_swsb(p, tgl_swsb_dst_dep(swsb, 1));
      brw_oword_block_write_scratch(p, brw_message_reg(inst->base_mrf),
                                    block_size,
                                    inst->offset + block_size * REG_SIZE * i);
   }

   brw_pop_insn_state(p);
}

void
fs_generator::generate_scratch_read(fs_inst *inst, struct brw_reg dst)
{
   assert(inst->exec_size <= 16 || inst->force_writemask_all);
   assert(inst->mlen != 0);

   brw_oword_block_read_scratch(p, dst, brw_message_reg(inst->base_mrf),
                                inst->exec_size / 8, inst->offset);
}

void
fs_generator::generate_scratch_read_gfx7(fs_inst *inst, struct brw_reg dst)
{
   assert(inst->exec_size <= 16 || inst->force_writemask_all);

   gfx7_block_read_scratch(p, dst, inst->exec_size / 8, inst->offset);
}

/* The A32 messages take a buffer base address in header.5:[31:0] (See
 * MH1_A32_PSM for typed messages or MH_A32_GO for byte/dword scattered
 * and OWord block messages in the SKL PRM Vol. 2d for more details.)
 * Unfortunately, there are a number of subtle differences:
 *
 * For the block read/write messages:
 *
 *   - We always stomp header.2 to fill in the actual scratch address (in
 *     units of OWORDs) so we don't care what's in there.
 *
 *   - They rely on per-thread scratch space value in header.3[3:0] to do
 *     bounds checking so that needs to be valid.  The upper bits of
 *     header.3 are ignored, though, so we can copy all of g0.3.
 *
 *   - They ignore header.5[9:0] and assumes the address is 1KB aligned.
 *
 *
 * For the byte/dword scattered read/write messages:
 *
 *   - We want header.2 to be zero because that gets added to the per-channel
 *     offset in the non-header portion of the message.
 *
 *   - Contrary to what the docs claim, they don't do any bounds checking so
 *     the value of header.3[3:0] doesn't matter.
 *
 *   - They consider all of header.5 for the base address and header.5[9:0]
 *     are not ignored.  This means that we can't copy g0.5 verbatim because
 *     g0.5[9:0] contains the FFTID on most platforms.  Instead, we have to
 *     use an AND to mask off the bottom 10 bits.
 *
 *
 * For block messages, just copying g0 gives a valid header because all the
 * garbage gets ignored except for header.2 which we stomp as part of message
 * setup.  For byte/dword scattered messages, we can just zero out the header
 * and copy over the bits we need from g0.5.  This opcode, however, tries to
 * satisfy the requirements of both by starting with 0 and filling out the
 * information required by either set of opcodes.
 */
void
fs_generator::generate_scratch_header(fs_inst *inst, struct brw_reg dst)
{
   assert(inst->exec_size == 8 && inst->force_writemask_all);
   assert(dst.file == BRW_GENERAL_REGISTER_FILE);

   dst.type = BRW_REGISTER_TYPE_UD;

   brw_inst *insn = brw_MOV(p, dst, brw_imm_ud(0));
   if (devinfo->ver >= 12)
      brw_set_default_swsb(p, tgl_swsb_null());
   else
      brw_inst_set_no_dd_clear(p->devinfo, insn, true);

   /* Copy the per-thread scratch space size from g0.3[3:0] */
   brw_set_default_exec_size(p, BRW_EXECUTE_1);
   insn = brw_AND(p, suboffset(dst, 3),
                     retype(brw_vec1_grf(0, 3), BRW_REGISTER_TYPE_UD),
                     brw_imm_ud(INTEL_MASK(3, 0)));
   if (devinfo->ver < 12) {
      brw_inst_set_no_dd_clear(p->devinfo, insn, true);
      brw_inst_set_no_dd_check(p->devinfo, insn, true);
   }

   /* Copy the scratch base address from g0.5[31:10] */
   insn = brw_AND(p, suboffset(dst, 5),
                     retype(brw_vec1_grf(0, 5), BRW_REGISTER_TYPE_UD),
                     brw_imm_ud(INTEL_MASK(31, 10)));
   if (devinfo->ver < 12)
      brw_inst_set_no_dd_check(p->devinfo, insn, true);
}

void
fs_generator::generate_uniform_pull_constant_load(fs_inst *inst,
                                                  struct brw_reg dst,
                                                  struct brw_reg index,
                                                  struct brw_reg offset)
{
   assert(type_sz(dst.type) == 4);
   assert(inst->mlen != 0);

   assert(index.file == BRW_IMMEDIATE_VALUE &&
	  index.type == BRW_REGISTER_TYPE_UD);
   uint32_t surf_index = index.ud;

   assert(offset.file == BRW_IMMEDIATE_VALUE &&
	  offset.type == BRW_REGISTER_TYPE_UD);
   uint32_t read_offset = offset.ud;

   brw_oword_block_read(p, dst, brw_message_reg(inst->base_mrf),
			read_offset, surf_index);
}

void
fs_generator::generate_varying_pull_constant_load_gfx4(fs_inst *inst,
                                                       struct brw_reg dst,
                                                       struct brw_reg index)
{
   assert(devinfo->ver < 7); /* Should use the gfx7 variant. */
   assert(inst->header_size != 0);
   assert(inst->mlen);

   assert(index.file == BRW_IMMEDIATE_VALUE &&
	  index.type == BRW_REGISTER_TYPE_UD);
   uint32_t surf_index = index.ud;

   uint32_t simd_mode, rlen, msg_type;
   if (inst->exec_size == 16) {
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
      rlen = 8;
   } else {
      assert(inst->exec_size == 8);
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD8;
      rlen = 4;
   }

   if (devinfo->ver >= 5)
      msg_type = GFX5_SAMPLER_MESSAGE_SAMPLE_LD;
   else {
      /* We always use the SIMD16 message so that we only have to load U, and
       * not V or R.
       */
      msg_type = BRW_SAMPLER_MESSAGE_SIMD16_LD;
      assert(inst->mlen == 3);
      assert(inst->size_written == 8 * REG_SIZE);
      rlen = 8;
      simd_mode = BRW_SAMPLER_SIMD_MODE_SIMD16;
   }

   struct brw_reg header = brw_vec8_grf(0, 0);
   gfx6_resolve_implied_move(p, &header, inst->base_mrf);

   brw_inst *send = brw_next_insn(p, BRW_OPCODE_SEND);
   brw_inst_set_compression(devinfo, send, false);
   brw_inst_set_sfid(devinfo, send, BRW_SFID_SAMPLER);
   brw_set_dest(p, send, retype(dst, BRW_REGISTER_TYPE_UW));
   brw_set_src0(p, send, header);
   if (devinfo->ver < 6)
      brw_inst_set_base_mrf(p->devinfo, send, inst->base_mrf);

   /* Our surface is set up as floats, regardless of what actual data is
    * stored in it.
    */
   uint32_t return_format = BRW_SAMPLER_RETURN_FORMAT_FLOAT32;
   brw_set_desc(p, send,
                brw_message_desc(devinfo, inst->mlen, rlen, inst->header_size) |
                brw_sampler_desc(devinfo, surf_index,
                                 0, /* sampler (unused) */
                                 msg_type, simd_mode, return_format));
}

/* Sets vstride=1, width=4, hstride=0 of register src1 during
 * the ADD instruction.
 */
void
fs_generator::generate_set_sample_id(fs_inst *inst,
                                     struct brw_reg dst,
                                     struct brw_reg src0,
                                     struct brw_reg src1)
{
   assert(dst.type == BRW_REGISTER_TYPE_D ||
          dst.type == BRW_REGISTER_TYPE_UD);
   assert(src0.type == BRW_REGISTER_TYPE_D ||
          src0.type == BRW_REGISTER_TYPE_UD);

   const struct brw_reg reg = stride(src1, 1, 4, 0);
   const unsigned lower_size = MIN2(inst->exec_size,
                                    devinfo->ver >= 8 ? 16 : 8);

   for (unsigned i = 0; i < inst->exec_size / lower_size; i++) {
      brw_inst *insn = brw_ADD(p, offset(dst, i * lower_size / 8),
                               offset(src0, (src0.vstride == 0 ? 0 : (1 << (src0.vstride - 1)) *
                                             (i * lower_size / (1 << src0.width))) *
                                            type_sz(src0.type) / REG_SIZE),
                               suboffset(reg, i * lower_size / 4));
      brw_inst_set_exec_size(devinfo, insn, cvt(lower_size) - 1);
      brw_inst_set_group(devinfo, insn, inst->group + lower_size * i);
      brw_inst_set_compression(devinfo, insn, lower_size > 8);
      brw_set_default_swsb(p, tgl_swsb_null());
   }
}

void
fs_generator::enable_debug(const char *shader_name)
{
   debug_flag = true;
   this->shader_name = shader_name;
}

static gfx12_systolic_depth
translate_systolic_depth(unsigned d)
{
   /* Could also return (ffs(d) - 1) & 3. */
   switch (d) {
   case 2:  return BRW_SYSTOLIC_DEPTH_2;
   case 4:  return BRW_SYSTOLIC_DEPTH_4;
   case 8:  return BRW_SYSTOLIC_DEPTH_8;
   case 16: return BRW_SYSTOLIC_DEPTH_16;
   default: unreachable("Invalid systolic depth.");
   }
}

int
fs_generator::generate_code(const cfg_t *cfg, int dispatch_width,
                            struct shader_stats shader_stats,
                            const brw::performance &perf,
                            struct brw_compile_stats *stats,
                            unsigned max_polygons)
{
   /* align to 64 byte boundary. */
   brw_realign(p, 64);

   this->dispatch_width = dispatch_width;

   int start_offset = p->next_insn_offset;

   int loop_count = 0, send_count = 0, nop_count = 0;
   bool is_accum_used = false;

   struct disasm_info *disasm_info = disasm_initialize(p->isa, cfg);

   foreach_block_and_inst (block, fs_inst, inst, cfg) {
      if (inst->opcode == SHADER_OPCODE_UNDEF)
         continue;

      struct brw_reg src[4], dst;
      unsigned int last_insn_offset = p->next_insn_offset;
      bool multiple_instructions_emitted = false;
      tgl_swsb swsb = inst->sched;

      /* From the Broadwell PRM, Volume 7, "3D-Media-GPGPU", in the
       * "Register Region Restrictions" section: for BDW, SKL:
       *
       *    "A POW/FDIV operation must not be followed by an instruction
       *     that requires two destination registers."
       *
       * The documentation is often lacking annotations for Atom parts,
       * and empirically this affects CHV as well.
       */
      if (devinfo->ver >= 8 &&
          devinfo->ver <= 9 &&
          p->nr_insn > 1 &&
          brw_inst_opcode(p->isa, brw_last_inst) == BRW_OPCODE_MATH &&
          brw_inst_math_function(devinfo, brw_last_inst) == BRW_MATH_FUNCTION_POW &&
          inst->dst.component_size(inst->exec_size) > REG_SIZE) {
         brw_NOP(p);
         last_insn_offset = p->next_insn_offset;

         /* In order to avoid spurious instruction count differences when the
          * instruction schedule changes, keep track of the number of inserted
          * NOPs.
          */
         nop_count++;
      }

      /* Wa_14010017096:
       *
       * Clear accumulator register before end of thread.
       */
      if (inst->eot && is_accum_used &&
          intel_needs_workaround(devinfo, 14010017096)) {
         brw_set_default_exec_size(p, BRW_EXECUTE_16);
         brw_set_default_group(p, 0);
         brw_set_default_mask_control(p, BRW_MASK_DISABLE);
         brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
         brw_set_default_flag_reg(p, 0, 0);
         brw_set_default_swsb(p, tgl_swsb_src_dep(swsb));
         brw_MOV(p, brw_acc_reg(8), brw_imm_f(0.0f));
         last_insn_offset = p->next_insn_offset;
         swsb = tgl_swsb_dst_dep(swsb, 1);
      }

      if (!is_accum_used && !inst->eot) {
         is_accum_used = inst->writes_accumulator_implicitly(devinfo) ||
                         inst->dst.is_accumulator();
      }

      /* Wa_14013672992:
       *
       * Always use @1 SWSB for EOT.
       */
      if (inst->eot && intel_needs_workaround(devinfo, 14013672992)) {
         if (tgl_swsb_src_dep(swsb).mode) {
            brw_set_default_exec_size(p, BRW_EXECUTE_1);
            brw_set_default_mask_control(p, BRW_MASK_DISABLE);
            brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
            brw_set_default_flag_reg(p, 0, 0);
            brw_set_default_swsb(p, tgl_swsb_src_dep(swsb));
            brw_SYNC(p, TGL_SYNC_NOP);
            last_insn_offset = p->next_insn_offset;
         }

         swsb = tgl_swsb_dst_dep(swsb, 1);
      }

      if (unlikely(debug_flag))
         disasm_annotate(disasm_info, inst, p->next_insn_offset);

      /* If the instruction writes to more than one register, it needs to be
       * explicitly marked as compressed on Gen <= 5.  On Gen >= 6 the
       * hardware figures out by itself what the right compression mode is,
       * but we still need to know whether the instruction is compressed to
       * set up the source register regions appropriately.
       *
       * XXX - This is wrong for instructions that write a single register but
       *       read more than one which should strictly speaking be treated as
       *       compressed.  For instructions that don't write any registers it
       *       relies on the destination being a null register of the correct
       *       type and regioning so the instruction is considered compressed
       *       or not accordingly.
       */
      const bool compressed =
           inst->dst.component_size(inst->exec_size) > REG_SIZE;
      brw_set_default_compression(p, compressed);
      brw_set_default_group(p, inst->group);

      for (unsigned int i = 0; i < inst->sources; i++) {
         src[i] = brw_reg_from_fs_reg(devinfo, inst,
                                      &inst->src[i], compressed);
	 /* The accumulator result appears to get used for the
	  * conditional modifier generation.  When negating a UD
	  * value, there is a 33rd bit generated for the sign in the
	  * accumulator value, so now you can't check, for example,
	  * equality with a 32-bit value.  See piglit fs-op-neg-uvec4.
	  */
	 assert(!inst->conditional_mod ||
		inst->src[i].type != BRW_REGISTER_TYPE_UD ||
		!inst->src[i].negate);
      }
      dst = brw_reg_from_fs_reg(devinfo, inst,
                                &inst->dst, compressed);

      brw_set_default_access_mode(p, BRW_ALIGN_1);
      brw_set_default_predicate_control(p, inst->predicate);
      brw_set_default_predicate_inverse(p, inst->predicate_inverse);
      /* On gfx7 and above, hardware automatically adds the group onto the
       * flag subregister number.  On Sandy Bridge and older, we have to do it
       * ourselves.
       */
      const unsigned flag_subreg = inst->flag_subreg +
         (devinfo->ver >= 7 ? 0 : inst->group / 16);
      brw_set_default_flag_reg(p, flag_subreg / 2, flag_subreg % 2);
      brw_set_default_saturate(p, inst->saturate);
      brw_set_default_mask_control(p, inst->force_writemask_all);
      brw_set_default_acc_write_control(p, inst->writes_accumulator);
      brw_set_default_swsb(p, swsb);

      unsigned exec_size = inst->exec_size;
      if (devinfo->verx10 == 70 &&
          (get_exec_type_size(inst) == 8 || type_sz(inst->dst.type) == 8)) {
         exec_size *= 2;
      }

      brw_set_default_exec_size(p, cvt(exec_size) - 1);

      assert(inst->force_writemask_all || inst->exec_size >= 4);
      assert(inst->force_writemask_all || inst->group % inst->exec_size == 0);
      assert(inst->base_mrf + inst->mlen <= BRW_MAX_MRF(devinfo->ver));
      assert(inst->mlen <= BRW_MAX_MSG_LENGTH * reg_unit(devinfo));

      switch (inst->opcode) {
      case BRW_OPCODE_SYNC:
         assert(src[0].file == BRW_IMMEDIATE_VALUE);
         brw_SYNC(p, tgl_sync_function(src[0].ud));
         break;
      case BRW_OPCODE_MOV:
	 brw_MOV(p, dst, src[0]);
	 break;
      case BRW_OPCODE_ADD:
	 brw_ADD(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_MUL:
	 brw_MUL(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_AVG:
	 brw_AVG(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_MACH:
	 brw_MACH(p, dst, src[0], src[1]);
	 break;

      case BRW_OPCODE_DP4A:
         assert(devinfo->ver >= 12);
         brw_DP4A(p, dst, src[0], src[1], src[2]);
         break;

      case BRW_OPCODE_LINE:
         brw_LINE(p, dst, src[0], src[1]);
         break;

      case BRW_OPCODE_DPAS:
         assert(devinfo->verx10 >= 125);
         brw_DPAS(p, translate_systolic_depth(inst->sdepth), inst->rcount,
                  dst, src[0], src[1], src[2]);
         break;

      case BRW_OPCODE_MAD:
         assert(devinfo->ver >= 6);
         if (devinfo->ver < 10)
            brw_set_default_access_mode(p, BRW_ALIGN_16);
         brw_MAD(p, dst, src[0], src[1], src[2]);
	 break;

      case BRW_OPCODE_LRP:
         assert(devinfo->ver >= 6 && devinfo->ver <= 10);
         if (devinfo->ver < 10)
            brw_set_default_access_mode(p, BRW_ALIGN_16);
         brw_LRP(p, dst, src[0], src[1], src[2]);
	 break;

      case BRW_OPCODE_ADD3:
         assert(devinfo->verx10 >= 125);
         brw_ADD3(p, dst, src[0], src[1], src[2]);
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
      case BRW_OPCODE_ROL:
	 assert(devinfo->ver >= 11);
	 assert(src[0].type == dst.type);
	 brw_ROL(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_ROR:
	 assert(devinfo->ver >= 11);
	 assert(src[0].type == dst.type);
	 brw_ROR(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_F32TO16:
         brw_F32TO16(p, dst, src[0]);
         break;
      case BRW_OPCODE_F16TO32:
         brw_F16TO32(p, dst, src[0]);
         break;
      case BRW_OPCODE_CMP:
         if (inst->exec_size >= 16 && devinfo->verx10 == 70 &&
             dst.file == BRW_ARCHITECTURE_REGISTER_FILE) {
            /* For unknown reasons the WaCMPInstFlagDepClearedEarly workaround
             * implemented in the compiler is not sufficient. Overriding the
             * type when the destination is the null register is necessary but
             * not sufficient by itself.
             */
            dst.type = BRW_REGISTER_TYPE_D;
         }
         brw_CMP(p, dst, inst->conditional_mod, src[0], src[1]);
	 break;
      case BRW_OPCODE_CMPN:
         if (inst->exec_size >= 16 && devinfo->verx10 == 70 &&
             dst.file == BRW_ARCHITECTURE_REGISTER_FILE) {
            /* For unknown reasons the WaCMPInstFlagDepClearedEarly workaround
             * implemented in the compiler is not sufficient. Overriding the
             * type when the destination is the null register is necessary but
             * not sufficient by itself.
             */
            dst.type = BRW_REGISTER_TYPE_D;
         }
         brw_CMPN(p, dst, inst->conditional_mod, src[0], src[1]);
         break;
      case BRW_OPCODE_SEL:
	 brw_SEL(p, dst, src[0], src[1]);
	 break;
      case BRW_OPCODE_CSEL:
         assert(devinfo->ver >= 8);
         if (devinfo->ver < 10)
            brw_set_default_access_mode(p, BRW_ALIGN_16);
         brw_CSEL(p, dst, src[0], src[1], src[2]);
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
         if (devinfo->ver < 10)
            brw_set_default_access_mode(p, BRW_ALIGN_16);
         brw_BFE(p, dst, src[0], src[1], src[2]);
         break;

      case BRW_OPCODE_BFI1:
         assert(devinfo->ver >= 7);
         brw_BFI1(p, dst, src[0], src[1]);
         break;
      case BRW_OPCODE_BFI2:
         assert(devinfo->ver >= 7);
         if (devinfo->ver < 10)
            brw_set_default_access_mode(p, BRW_ALIGN_16);
         brw_BFI2(p, dst, src[0], src[1], src[2]);
         break;

      case BRW_OPCODE_IF:
	 if (inst->src[0].file != BAD_FILE) {
	    /* The instruction has an embedded compare (only allowed on gfx6) */
	    assert(devinfo->ver == 6);
	    gfx6_IF(p, inst->conditional_mod, src[0], src[1]);
	 } else {
	    brw_IF(p, brw_get_default_exec_size(p));
	 }
	 break;

      case BRW_OPCODE_ELSE:
	 brw_ELSE(p);
	 break;
      case BRW_OPCODE_ENDIF:
	 brw_ENDIF(p);
	 break;

      case BRW_OPCODE_DO:
	 brw_DO(p, brw_get_default_exec_size(p));
	 break;

      case BRW_OPCODE_BREAK:
	 brw_BREAK(p);
	 break;
      case BRW_OPCODE_CONTINUE:
         brw_CONT(p);
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
	 if (devinfo->ver >= 6) {
            assert(inst->mlen == 0);
            assert(devinfo->ver >= 7 || inst->exec_size == 8);
            gfx6_math(p, dst, brw_math_function(inst->opcode),
                      src[0], brw_null_reg());
	 } else {
            assert(inst->mlen >= 1);
            assert(devinfo->ver == 5 || devinfo->platform == INTEL_PLATFORM_G4X || inst->exec_size == 8);
            gfx4_math(p, dst,
                      brw_math_function(inst->opcode),
                      inst->base_mrf, src[0],
                      BRW_MATH_PRECISION_FULL);
            send_count++;
	 }
	 break;
      case SHADER_OPCODE_INT_QUOTIENT:
      case SHADER_OPCODE_INT_REMAINDER:
      case SHADER_OPCODE_POW:
         assert(devinfo->verx10 < 125);
         assert(inst->conditional_mod == BRW_CONDITIONAL_NONE);
         if (devinfo->ver >= 6) {
            assert(inst->mlen == 0);
            assert((devinfo->ver >= 7 && inst->opcode == SHADER_OPCODE_POW) ||
                   inst->exec_size == 8);
            gfx6_math(p, dst, brw_math_function(inst->opcode), src[0], src[1]);
         } else {
            assert(inst->mlen >= 1);
            assert(inst->exec_size == 8);
            gfx4_math(p, dst, brw_math_function(inst->opcode),
                      inst->base_mrf, src[0],
                      BRW_MATH_PRECISION_FULL);
            send_count++;
	 }
	 break;
      case FS_OPCODE_LINTERP:
	 multiple_instructions_emitted = generate_linterp(inst, dst, src);
	 break;
      case FS_OPCODE_PIXEL_X:
         assert(src[0].type == BRW_REGISTER_TYPE_UW);
         assert(src[1].type == BRW_REGISTER_TYPE_UW);
         src[0].subnr = 0 * type_sz(src[0].type);
         if (src[1].file == BRW_IMMEDIATE_VALUE) {
            assert(src[1].ud == 0);
            brw_MOV(p, dst, stride(src[0], 8, 4, 1));
         } else {
            /* Coarse pixel case */
            brw_ADD(p, dst, stride(src[0], 8, 4, 1), src[1]);
         }
         break;
      case FS_OPCODE_PIXEL_Y:
         assert(src[0].type == BRW_REGISTER_TYPE_UW);
         assert(src[1].type == BRW_REGISTER_TYPE_UW);
         src[0].subnr = 4 * type_sz(src[0].type);
         if (src[1].file == BRW_IMMEDIATE_VALUE) {
            assert(src[1].ud == 0);
            brw_MOV(p, dst, stride(src[0], 8, 4, 1));
         } else {
            /* Coarse pixel case */
            brw_ADD(p, dst, stride(src[0], 8, 4, 1), src[1]);
         }
         break;

      case SHADER_OPCODE_SEND:
         generate_send(inst, dst, src[0], src[1], src[2],
                       inst->ex_mlen > 0 ? src[3] : brw_null_reg());
         send_count++;
         break;

      case SHADER_OPCODE_TEX:
      case FS_OPCODE_TXB:
      case SHADER_OPCODE_TXD:
      case SHADER_OPCODE_TXF:
      case SHADER_OPCODE_TXF_CMS:
      case SHADER_OPCODE_TXL:
      case SHADER_OPCODE_TXS:
      case SHADER_OPCODE_LOD:
      case SHADER_OPCODE_TG4:
      case SHADER_OPCODE_SAMPLEINFO:
         assert(inst->src[0].file == BAD_FILE);
         generate_tex(inst, dst, src[1], src[2]);
         send_count++;
         break;

      case FS_OPCODE_DDX_COARSE:
      case FS_OPCODE_DDX_FINE:
         generate_ddx(inst, dst, src[0]);
         break;
      case FS_OPCODE_DDY_COARSE:
      case FS_OPCODE_DDY_FINE:
         generate_ddy(inst, dst, src[0]);
	 break;

      case SHADER_OPCODE_GFX4_SCRATCH_WRITE:
	 generate_scratch_write(inst, src[0]);
         send_count++;
	 break;

      case SHADER_OPCODE_GFX4_SCRATCH_READ:
	 generate_scratch_read(inst, dst);
         send_count++;
	 break;

      case SHADER_OPCODE_GFX7_SCRATCH_READ:
	 generate_scratch_read_gfx7(inst, dst);
         send_count++;
	 break;

      case SHADER_OPCODE_SCRATCH_HEADER:
         generate_scratch_header(inst, dst);
         break;

      case SHADER_OPCODE_MOV_INDIRECT:
         generate_mov_indirect(inst, dst, src[0], src[1]);
         break;

      case SHADER_OPCODE_MOV_RELOC_IMM:
         assert(src[0].file == BRW_IMMEDIATE_VALUE);
         brw_MOV_reloc_imm(p, dst, dst.type, src[0].ud);
         break;

      case FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD:
         assert(inst->force_writemask_all);
	 generate_uniform_pull_constant_load(inst, dst,
                                             src[PULL_UNIFORM_CONSTANT_SRC_SURFACE],
                                             src[PULL_UNIFORM_CONSTANT_SRC_OFFSET]);
         send_count++;
	 break;

      case FS_OPCODE_VARYING_PULL_CONSTANT_LOAD_GFX4:
	 generate_varying_pull_constant_load_gfx4(inst, dst, src[0]);
         send_count++;
	 break;

      case FS_OPCODE_REP_FB_WRITE:
      case FS_OPCODE_FB_WRITE:
	 generate_fb_write(inst, src[0]);
         send_count++;
	 break;

      case FS_OPCODE_FB_READ:
         generate_fb_read(inst, dst, src[0]);
         send_count++;
         break;

      case BRW_OPCODE_HALT:
         generate_halt(inst);
         break;

      case SHADER_OPCODE_INTERLOCK:
      case SHADER_OPCODE_MEMORY_FENCE: {
         assert(src[1].file == BRW_IMMEDIATE_VALUE);
         assert(src[2].file == BRW_IMMEDIATE_VALUE);

         const enum opcode send_op = inst->opcode == SHADER_OPCODE_INTERLOCK ?
            BRW_OPCODE_SENDC : BRW_OPCODE_SEND;

         brw_memory_fence(p, dst, src[0], send_op,
                          brw_message_target(inst->sfid),
                          inst->desc,
                          /* commit_enable */ src[1].ud,
                          /* bti */ src[2].ud);
         send_count++;
         break;
      }

      case FS_OPCODE_SCHEDULING_FENCE:
         if (inst->sources == 0 && swsb.regdist == 0 &&
                                   swsb.mode == TGL_SBID_NULL) {
            if (unlikely(debug_flag))
               disasm_info->use_tail = true;
            break;
         }

         if (devinfo->ver >= 12) {
            /* Use the available SWSB information to stall.  A single SYNC is
             * sufficient since if there were multiple dependencies, the
             * scoreboard algorithm already injected other SYNCs before this
             * instruction.
             */
            brw_SYNC(p, TGL_SYNC_NOP);
         } else {
            for (unsigned i = 0; i < inst->sources; i++) {
               /* Emit a MOV to force a stall until the instruction producing the
                * registers finishes.
                */
               brw_MOV(p, retype(brw_null_reg(), BRW_REGISTER_TYPE_UW),
                       retype(src[i], BRW_REGISTER_TYPE_UW));
            }

            if (inst->sources > 1)
               multiple_instructions_emitted = true;
         }

         break;

      case SHADER_OPCODE_FIND_LIVE_CHANNEL:
         brw_find_live_channel(p, dst, false);
         break;
      case SHADER_OPCODE_FIND_LAST_LIVE_CHANNEL:
         brw_find_live_channel(p, dst, true);
         break;

      case FS_OPCODE_LOAD_LIVE_CHANNELS: {
         assert(devinfo->ver >= 8);
         assert(inst->force_writemask_all && inst->group == 0);
         assert(inst->dst.file == BAD_FILE);
         brw_set_default_exec_size(p, BRW_EXECUTE_1);
         brw_MOV(p, retype(brw_flag_subreg(inst->flag_subreg),
                           BRW_REGISTER_TYPE_UD),
                 retype(brw_mask_reg(0), BRW_REGISTER_TYPE_UD));
         break;
      }
      case SHADER_OPCODE_BROADCAST:
         assert(inst->force_writemask_all);
         brw_broadcast(p, dst, src[0], src[1]);
         break;

      case SHADER_OPCODE_SHUFFLE:
         generate_shuffle(inst, dst, src[0], src[1]);
         break;

      case SHADER_OPCODE_SEL_EXEC:
         assert(inst->force_writemask_all);
         assert(devinfo->has_64bit_float || type_sz(dst.type) <= 4);
         brw_set_default_mask_control(p, BRW_MASK_DISABLE);
         brw_MOV(p, dst, src[1]);
         brw_set_default_mask_control(p, BRW_MASK_ENABLE);
         brw_set_default_swsb(p, tgl_swsb_null());
         brw_MOV(p, dst, src[0]);
         break;

      case SHADER_OPCODE_QUAD_SWIZZLE:
         assert(src[1].file == BRW_IMMEDIATE_VALUE);
         assert(src[1].type == BRW_REGISTER_TYPE_UD);
         generate_quad_swizzle(inst, dst, src[0], src[1].ud);
         break;

      case SHADER_OPCODE_CLUSTER_BROADCAST: {
         assert((devinfo->platform != INTEL_PLATFORM_CHV &&
                 !intel_device_info_is_9lp(devinfo) &&
                 devinfo->has_64bit_float) || type_sz(src[0].type) <= 4);
         assert(!src[0].negate && !src[0].abs);
         assert(src[1].file == BRW_IMMEDIATE_VALUE);
         assert(src[1].type == BRW_REGISTER_TYPE_UD);
         assert(src[2].file == BRW_IMMEDIATE_VALUE);
         assert(src[2].type == BRW_REGISTER_TYPE_UD);
         const unsigned component = src[1].ud;
         const unsigned cluster_size = src[2].ud;
         assert(inst->src[0].file != ARF && inst->src[0].file != FIXED_GRF);
         const unsigned s = inst->src[0].stride;
         unsigned vstride = cluster_size * s;
         unsigned width = cluster_size;

         /* The maximum exec_size is 32, but the maximum width is only 16. */
         if (inst->exec_size == width) {
            vstride = 0;
            width = 1;
         }

         struct brw_reg strided = stride(suboffset(src[0], component * s),
                                         vstride, width, 0);
         brw_MOV(p, dst, strided);
         break;
      }

      case FS_OPCODE_SET_SAMPLE_ID:
         generate_set_sample_id(inst, dst, src[0], src[1]);
         break;

      case SHADER_OPCODE_HALT_TARGET:
         /* This is the place where the final HALT needs to be inserted if
          * we've emitted any discards.  If not, this will emit no code.
          */
         if (!patch_halt_jumps()) {
            if (unlikely(debug_flag)) {
               disasm_info->use_tail = true;
            }
         }
         break;

      case CS_OPCODE_CS_TERMINATE:
         generate_cs_terminate(inst, src[0]);
         send_count++;
         break;

      case SHADER_OPCODE_BARRIER:
	 generate_barrier(inst, src[0]);
         send_count++;
	 break;

      case BRW_OPCODE_DIM:
         assert(devinfo->platform == INTEL_PLATFORM_HSW);
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

      case SHADER_OPCODE_FLOAT_CONTROL_MODE:
         assert(src[0].file == BRW_IMMEDIATE_VALUE);
         assert(src[1].file == BRW_IMMEDIATE_VALUE);
         brw_float_controls_mode(p, src[0].d, src[1].d);
         break;

      case SHADER_OPCODE_READ_SR_REG:
         if (devinfo->ver >= 12) {
            /* There is a SWSB restriction that requires that any time sr0 is
             * accessed both the instruction doing the access and the next one
             * have SWSB set to RegDist(1).
             */
            if (brw_get_default_swsb(p).mode != TGL_SBID_NULL)
               brw_SYNC(p, TGL_SYNC_NOP);
            assert(src[0].file == BRW_IMMEDIATE_VALUE);
            brw_set_default_swsb(p, tgl_swsb_regdist(1));
            brw_MOV(p, dst, brw_sr0_reg(src[0].ud));
            brw_set_default_swsb(p, tgl_swsb_regdist(1));
            brw_AND(p, dst, dst, brw_imm_ud(0xffffffff));
         } else {
            brw_MOV(p, dst, brw_sr0_reg(src[0].ud));
         }
         break;

      default:
         unreachable("Unsupported opcode");

      case SHADER_OPCODE_LOAD_PAYLOAD:
         unreachable("Should be lowered by lower_load_payload()");
      }

      if (multiple_instructions_emitted)
         continue;

      if (inst->no_dd_clear || inst->no_dd_check || inst->conditional_mod) {
         assert(p->next_insn_offset == last_insn_offset + 16 ||
                !"conditional_mod, no_dd_check, or no_dd_clear set for IR "
                 "emitting more than 1 instruction");

         brw_inst *last = &p->store[last_insn_offset / 16];

         if (inst->conditional_mod)
            brw_inst_set_cond_modifier(p->devinfo, last, inst->conditional_mod);
         if (devinfo->ver < 12) {
            brw_inst_set_no_dd_clear(p->devinfo, last, inst->no_dd_clear);
            brw_inst_set_no_dd_check(p->devinfo, last, inst->no_dd_check);
         }
      }

      /* When enabled, insert sync NOP after every instruction and make sure
       * that current instruction depends on the previous instruction.
       */
      if (INTEL_DEBUG(DEBUG_SWSB_STALL) && devinfo->ver >= 12) {
         brw_set_default_swsb(p, tgl_swsb_regdist(1));
         brw_SYNC(p, TGL_SYNC_NOP);
      }
   }

   brw_set_uip_jip(p, start_offset);

   /* end of program sentinel */
   disasm_new_inst_group(disasm_info, p->next_insn_offset);

   /* `send_count` explicitly does not include spills or fills, as we'd
    * like to use it as a metric for intentional memory access or other
    * shared function use.  Otherwise, subtle changes to scheduling or
    * register allocation could cause it to fluctuate wildly - and that
    * effect is already counted in spill/fill counts.
    */
   send_count -= shader_stats.spill_count;
   send_count -= shader_stats.fill_count;

#ifndef NDEBUG
   bool validated =
#else
   if (unlikely(debug_flag))
#endif
      brw_validate_instructions(&compiler->isa, p->store,
                                start_offset,
                                p->next_insn_offset,
                                disasm_info);

   int before_size = p->next_insn_offset - start_offset;
   brw_compact_instructions(p, start_offset, disasm_info);
   int after_size = p->next_insn_offset - start_offset;

   bool dump_shader_bin = brw_should_dump_shader_bin();
   unsigned char sha1[21];
   char sha1buf[41];

   if (unlikely(debug_flag || dump_shader_bin)) {
      _mesa_sha1_compute(p->store + start_offset / sizeof(brw_inst),
                         after_size, sha1);
      _mesa_sha1_format(sha1buf, sha1);
   }

   if (unlikely(dump_shader_bin))
      brw_dump_shader_bin(p->store, start_offset, p->next_insn_offset,
                          sha1buf);

   if (unlikely(debug_flag)) {
      fprintf(stderr, "Native code for %s (src_hash 0x%08x) (sha1 %s)\n"
              "SIMD%d shader: %d instructions. %d loops. %u cycles. "
              "%d:%d spills:fills, %u sends, "
              "scheduled with mode %s. "
              "Promoted %u constants. "
              "Compacted %d to %d bytes (%.0f%%)\n",
              shader_name, params->source_hash, sha1buf,
              dispatch_width, before_size / 16,
              loop_count, perf.latency,
              shader_stats.spill_count,
              shader_stats.fill_count,
              send_count,
              shader_stats.scheduler_mode,
              shader_stats.promoted_constants,
              before_size, after_size,
              100.0f * (before_size - after_size) / before_size);

      /* overriding the shader makes disasm_info invalid */
      if (!brw_try_override_assembly(p, start_offset, sha1buf)) {
         dump_assembly(p->store, start_offset, p->next_insn_offset,
                       disasm_info, perf.block_latency);
      } else {
         fprintf(stderr, "Successfully overrode shader with sha1 %s\n\n", sha1buf);
      }
   }
   ralloc_free(disasm_info);
#ifndef NDEBUG
   if (!validated && !debug_flag) {
      fprintf(stderr,
            "Validation failed. Rerun with INTEL_DEBUG=shaders to get more information.\n");
   }
#endif
   assert(validated);

   brw_shader_debug_log(compiler, params->log_data,
                        "%s SIMD%d shader: %d inst, %d loops, %u cycles, "
                        "%d:%d spills:fills, %u sends, "
                        "scheduled with mode %s, "
                        "Promoted %u constants, "
                        "compacted %d to %d bytes.\n",
                        _mesa_shader_stage_to_abbrev(stage),
                        dispatch_width, before_size / 16 - nop_count,
                        loop_count, perf.latency,
                        shader_stats.spill_count,
                        shader_stats.fill_count,
                        send_count,
                        shader_stats.scheduler_mode,
                        shader_stats.promoted_constants,
                        before_size, after_size);
   if (stats) {
      stats->dispatch_width = dispatch_width;
      stats->max_polygons = max_polygons;
      stats->max_dispatch_width = dispatch_width;
      stats->instructions = before_size / 16 - nop_count;
      stats->sends = send_count;
      stats->loops = loop_count;
      stats->cycles = perf.latency;
      stats->spills = shader_stats.spill_count;
      stats->fills = shader_stats.fill_count;
      stats->max_live_registers = shader_stats.max_register_pressure;
   }

   return start_offset;
}

void
fs_generator::add_const_data(void *data, unsigned size)
{
   assert(prog_data->const_data_size == 0);
   if (size > 0) {
      prog_data->const_data_size = size;
      prog_data->const_data_offset = brw_append_data(p, data, size, 32);
   }
}

void
fs_generator::add_resume_sbt(unsigned num_resume_shaders, uint64_t *sbt)
{
   assert(brw_shader_stage_is_bindless(stage));
   struct brw_bs_prog_data *bs_prog_data = brw_bs_prog_data(prog_data);
   if (num_resume_shaders > 0) {
      bs_prog_data->resume_sbt_offset =
         brw_append_data(p, sbt, num_resume_shaders * sizeof(uint64_t), 32);
      for (unsigned i = 0; i < num_resume_shaders; i++) {
         size_t offset = bs_prog_data->resume_sbt_offset + i * sizeof(*sbt);
         assert(offset <= UINT32_MAX);
         brw_add_reloc(p, BRW_SHADER_RELOC_SHADER_START_OFFSET,
                       BRW_SHADER_RELOC_TYPE_U32,
                       (uint32_t)offset, (uint32_t)sbt[i]);
      }
   }
}

const unsigned *
fs_generator::get_assembly()
{
   prog_data->relocs = brw_get_shader_relocs(p, &prog_data->num_relocs);

   return brw_get_program(p, &prog_data->program_size);
}
