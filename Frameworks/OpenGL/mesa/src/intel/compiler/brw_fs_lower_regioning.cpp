/*
 * Copyright © 2018 Intel Corporation
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

#include "brw_fs.h"
#include "brw_cfg.h"
#include "brw_fs_builder.h"

using namespace brw;

namespace {
   /* From the SKL PRM Vol 2a, "Move":
    *
    * "A mov with the same source and destination type, no source modifier,
    *  and no saturation is a raw move. A packed byte destination region (B
    *  or UB type with HorzStride == 1 and ExecSize > 1) can only be written
    *  using raw move."
    */
   bool
   is_byte_raw_mov(const fs_inst *inst)
   {
      return type_sz(inst->dst.type) == 1 &&
             inst->opcode == BRW_OPCODE_MOV &&
             inst->src[0].type == inst->dst.type &&
             !inst->saturate &&
             !inst->src[0].negate &&
             !inst->src[0].abs;
   }

   /*
    * Return an acceptable byte stride for the destination of an instruction
    * that requires it to have some particular alignment.
    */
   unsigned
   required_dst_byte_stride(const fs_inst *inst)
   {
      if (inst->dst.is_accumulator()) {
         /* If the destination is an accumulator, insist that we leave the
          * stride alone.  We cannot "fix" accumulator destinations by writing
          * to a temporary and emitting a MOV into the original destination.
          * For multiply instructions (our one use of the accumulator), the
          * MUL writes the full 66 bits of the accumulator whereas the MOV we
          * would emit only writes 33 bits and leaves the top 33 bits
          * undefined.
          *
          * It's safe to just require the original stride here because the
          * lowering pass will detect the mismatch in has_invalid_src_region
          * and fix the sources of the multiply instead of the destination.
          */
         return inst->dst.stride * type_sz(inst->dst.type);
      } else if (type_sz(inst->dst.type) < get_exec_type_size(inst) &&
          !is_byte_raw_mov(inst)) {
         return get_exec_type_size(inst);
      } else {
         /* Calculate the maximum byte stride and the minimum/maximum type
          * size across all source and destination operands we are required to
          * lower.
          */
         unsigned max_stride = inst->dst.stride * type_sz(inst->dst.type);
         unsigned min_size = type_sz(inst->dst.type);
         unsigned max_size = type_sz(inst->dst.type);

         for (unsigned i = 0; i < inst->sources; i++) {
            if (!is_uniform(inst->src[i]) && !inst->is_control_source(i)) {
               const unsigned size = type_sz(inst->src[i].type);
               max_stride = MAX2(max_stride, inst->src[i].stride * size);
               min_size = MIN2(min_size, size);
               max_size = MAX2(max_size, size);
            }
         }

         /* All operands involved in lowering need to fit in the calculated
          * stride.
          */
         assert(max_size <= 4 * min_size);

         /* Attempt to use the largest byte stride among all present operands,
          * but never exceed a stride of 4 since that would lead to illegal
          * destination regions during lowering.
          */
         return MIN2(max_stride, 4 * min_size);
      }
   }

   /*
    * Return an acceptable byte sub-register offset for the destination of an
    * instruction that requires it to be aligned to the sub-register offset of
    * the sources.
    */
   unsigned
   required_dst_byte_offset(const intel_device_info *devinfo, const fs_inst *inst)
   {
      for (unsigned i = 0; i < inst->sources; i++) {
         if (!is_uniform(inst->src[i]) && !inst->is_control_source(i))
            if (reg_offset(inst->src[i]) % (reg_unit(devinfo) * REG_SIZE) !=
                reg_offset(inst->dst) % (reg_unit(devinfo) * REG_SIZE))
               return 0;
      }

      return reg_offset(inst->dst) % (reg_unit(devinfo) * REG_SIZE);
   }

   /*
    * Return the closest legal execution type for an instruction on
    * the specified platform.
    */
   brw_reg_type
   required_exec_type(const intel_device_info *devinfo, const fs_inst *inst)
   {
      const brw_reg_type t = get_exec_type(inst);
      const bool has_64bit = brw_reg_type_is_floating_point(t) ?
         devinfo->has_64bit_float : devinfo->has_64bit_int;

      switch (inst->opcode) {
      case SHADER_OPCODE_SHUFFLE:
         /* IVB has an issue (which we found empirically) where it reads
          * two address register components per channel for indirectly
          * addressed 64-bit sources.
          *
          * From the Cherryview PRM Vol 7. "Register Region Restrictions":
          *
          *    "When source or destination datatype is 64b or operation is
          *    integer DWord multiply, indirect addressing must not be
          *    used."
          *
          * Work around both of the above and handle platforms that
          * don't support 64-bit types at all.
          */
         if ((!devinfo->has_64bit_int ||
              devinfo->platform == INTEL_PLATFORM_CHV ||
              intel_device_info_is_9lp(devinfo)) && type_sz(t) > 4)
            return BRW_REGISTER_TYPE_UD;
         else if (has_dst_aligned_region_restriction(devinfo, inst))
            return brw_int_type(type_sz(t), false);
         else
            return t;

      case SHADER_OPCODE_SEL_EXEC:
         if ((!has_64bit || devinfo->has_64bit_float_via_math_pipe) &&
             type_sz(t) > 4)
            return BRW_REGISTER_TYPE_UD;
         else
            return t;

      case SHADER_OPCODE_QUAD_SWIZZLE:
         if (has_dst_aligned_region_restriction(devinfo, inst))
            return brw_int_type(type_sz(t), false);
         else
            return t;

      case SHADER_OPCODE_CLUSTER_BROADCAST:
         /* From the Cherryview PRM Vol 7. "Register Region Restrictions":
          *
          *    "When source or destination datatype is 64b or operation is
          *    integer DWord multiply, indirect addressing must not be
          *    used."
          *
          * For MTL (verx10 == 125), float64 is supported, but int64 is not.
          * Therefore we need to lower cluster broadcast using 32-bit int ops.
          *
          * For gfx12.5+ platforms that support int64, the register regions
          * used by cluster broadcast aren't supported by the 64-bit pipeline.
          *
          * Work around the above and handle platforms that don't
          * support 64-bit types at all.
          */
         if ((!has_64bit || devinfo->verx10 >= 125 ||
              devinfo->platform == INTEL_PLATFORM_CHV ||
              intel_device_info_is_9lp(devinfo)) && type_sz(t) > 4)
            return BRW_REGISTER_TYPE_UD;
         else
            return brw_int_type(type_sz(t), false);

      case SHADER_OPCODE_BROADCAST:
      case SHADER_OPCODE_MOV_INDIRECT:
         if (((devinfo->verx10 == 70 ||
               devinfo->platform == INTEL_PLATFORM_CHV ||
               intel_device_info_is_9lp(devinfo) ||
               devinfo->verx10 >= 125) && type_sz(inst->src[0].type) > 4) ||
             (devinfo->verx10 >= 125 &&
              brw_reg_type_is_floating_point(inst->src[0].type)))
            return brw_int_type(type_sz(t), false);
         else
            return t;

      default:
         return t;
      }
   }

   /*
    * Return the stride between channels of the specified register in
    * byte units, or ~0u if the region cannot be represented with a
    * single one-dimensional stride.
    */
   unsigned
   byte_stride(const fs_reg &reg)
   {
      switch (reg.file) {
      case BAD_FILE:
      case UNIFORM:
      case IMM:
      case VGRF:
      case MRF:
      case ATTR:
         return reg.stride * type_sz(reg.type);
      case ARF:
      case FIXED_GRF:
         if (reg.is_null()) {
            return 0;
         } else {
            const unsigned hstride = reg.hstride ? 1 << (reg.hstride - 1) : 0;
            const unsigned vstride = reg.vstride ? 1 << (reg.vstride - 1) : 0;
            const unsigned width = 1 << reg.width;

            if (width == 1) {
               return vstride * type_sz(reg.type);
            } else if (hstride * width == vstride) {
               return hstride * type_sz(reg.type);
            } else {
               return ~0u;
            }
         }
      default:
         unreachable("Invalid register file");
      }
   }

   /*
    * Return whether the instruction has an unsupported channel bit layout
    * specified for the i-th source region.
    */
   bool
   has_invalid_src_region(const intel_device_info *devinfo, const fs_inst *inst,
                          unsigned i)
   {
      if (is_send(inst) || inst->is_math() || inst->is_control_source(i) ||
          inst->opcode == BRW_OPCODE_DPAS) {
         return false;
      }

      /* Empirical testing shows that Broadwell has a bug affecting half-float
       * MAD instructions when any of its sources has a non-zero offset, such
       * as:
       *
       * mad(8) g18<1>HF -g17<4,4,1>HF g14.8<4,4,1>HF g11<4,4,1>HF { align16 1Q };
       *
       * We used to generate code like this for SIMD8 executions where we
       * used to pack components Y and W of a vector at offset 16B of a SIMD
       * register. The problem doesn't occur if the stride of the source is 0.
       */
      if (devinfo->ver == 8 &&
          inst->opcode == BRW_OPCODE_MAD &&
          inst->src[i].type == BRW_REGISTER_TYPE_HF &&
          reg_offset(inst->src[i]) % REG_SIZE > 0 &&
          inst->src[i].stride != 0) {
         return true;
      }

      const unsigned dst_byte_offset = reg_offset(inst->dst) % (reg_unit(devinfo) * REG_SIZE);
      const unsigned src_byte_offset = reg_offset(inst->src[i]) % (reg_unit(devinfo) * REG_SIZE);

      return has_dst_aligned_region_restriction(devinfo, inst) &&
             !is_uniform(inst->src[i]) &&
             (byte_stride(inst->src[i]) != byte_stride(inst->dst) ||
              src_byte_offset != dst_byte_offset);
   }

   /*
    * Return whether the instruction has an unsupported channel bit layout
    * specified for the destination region.
    */
   bool
   has_invalid_dst_region(const intel_device_info *devinfo,
                          const fs_inst *inst)
   {
      if (is_send(inst) || inst->is_math()) {
         return false;
      } else {
         const brw_reg_type exec_type = get_exec_type(inst);
         const unsigned dst_byte_offset = reg_offset(inst->dst) % (reg_unit(devinfo) * REG_SIZE);
         const bool is_narrowing_conversion = !is_byte_raw_mov(inst) &&
            type_sz(inst->dst.type) < type_sz(exec_type);

         return (has_dst_aligned_region_restriction(devinfo, inst) &&
                 (required_dst_byte_stride(inst) != byte_stride(inst->dst) ||
                  required_dst_byte_offset(devinfo, inst) != dst_byte_offset)) ||
                (is_narrowing_conversion &&
                 required_dst_byte_stride(inst) != byte_stride(inst->dst));
      }
   }

   /**
    * Return a non-zero value if the execution type of the instruction is
    * unsupported.  The destination and sources matching the returned mask
    * will be bit-cast to an integer type of appropriate size, lowering any
    * source or destination modifiers into separate MOV instructions.
    */
   unsigned
   has_invalid_exec_type(const intel_device_info *devinfo, const fs_inst *inst)
   {
      if (required_exec_type(devinfo, inst) != get_exec_type(inst)) {
         switch (inst->opcode) {
         case SHADER_OPCODE_SHUFFLE:
         case SHADER_OPCODE_QUAD_SWIZZLE:
         case SHADER_OPCODE_CLUSTER_BROADCAST:
         case SHADER_OPCODE_BROADCAST:
         case SHADER_OPCODE_MOV_INDIRECT:
            return 0x1;

         case SHADER_OPCODE_SEL_EXEC:
            return 0x3;

         default:
            unreachable("Unknown invalid execution type source mask.");
         }
      } else {
         return 0;
      }
   }

   /*
    * Return whether the instruction has unsupported source modifiers
    * specified for the i-th source region.
    */
   bool
   has_invalid_src_modifiers(const intel_device_info *devinfo,
                             const fs_inst *inst, unsigned i)
   {
      return (!inst->can_do_source_mods(devinfo) &&
              (inst->src[i].negate || inst->src[i].abs)) ||
             ((has_invalid_exec_type(devinfo, inst) & (1u << i)) &&
              (inst->src[i].negate || inst->src[i].abs ||
               inst->src[i].type != get_exec_type(inst)));
   }

   /*
    * Return whether the instruction has an unsupported type conversion
    * specified for the destination.
    */
   bool
   has_invalid_conversion(const intel_device_info *devinfo, const fs_inst *inst)
   {
      switch (inst->opcode) {
      case BRW_OPCODE_MOV:
         return false;
      case BRW_OPCODE_SEL:
         return inst->dst.type != get_exec_type(inst);
      default:
         /* FIXME: We assume the opcodes not explicitly mentioned before just
          * work fine with arbitrary conversions, unless they need to be
          * bit-cast.
          */
         return has_invalid_exec_type(devinfo, inst) &&
                inst->dst.type != get_exec_type(inst);
      }
   }

   /**
    * Return whether the instruction has unsupported destination modifiers.
    */
   bool
   has_invalid_dst_modifiers(const intel_device_info *devinfo, const fs_inst *inst)
   {
      return (has_invalid_exec_type(devinfo, inst) &&
              (inst->saturate || inst->conditional_mod)) ||
             has_invalid_conversion(devinfo, inst);
   }

   /**
    * Return whether the instruction has non-standard semantics for the
    * conditional mod which don't cause the flag register to be updated with
    * the comparison result.
    */
   bool
   has_inconsistent_cmod(const fs_inst *inst)
   {
      return inst->opcode == BRW_OPCODE_SEL ||
             inst->opcode == BRW_OPCODE_CSEL ||
             inst->opcode == BRW_OPCODE_IF ||
             inst->opcode == BRW_OPCODE_WHILE;
   }

   bool
   lower_instruction(fs_visitor *v, bblock_t *block, fs_inst *inst);
}

namespace brw {
   /**
    * Remove any modifiers from the \p i-th source region of the instruction,
    * including negate, abs and any implicit type conversion to the execution
    * type.  Instead any source modifiers will be implemented as a separate
    * MOV instruction prior to the original instruction.
    */
   bool
   lower_src_modifiers(fs_visitor *v, bblock_t *block, fs_inst *inst, unsigned i)
   {
      assert(inst->components_read(i) == 1);
      assert(v->devinfo->has_integer_dword_mul ||
             inst->opcode != BRW_OPCODE_MUL ||
             brw_reg_type_is_floating_point(get_exec_type(inst)) ||
             MIN2(type_sz(inst->src[0].type), type_sz(inst->src[1].type)) >= 4 ||
             type_sz(inst->src[i].type) == get_exec_type_size(inst));

      const fs_builder ibld(v, block, inst);
      const fs_reg tmp = ibld.vgrf(get_exec_type(inst));

      lower_instruction(v, block, ibld.MOV(tmp, inst->src[i]));
      inst->src[i] = tmp;

      return true;
   }
}

namespace {
   /**
    * Remove any modifiers from the destination region of the instruction,
    * including saturate, conditional mod and any implicit type conversion
    * from the execution type.  Instead any destination modifiers will be
    * implemented as a separate MOV instruction after the original
    * instruction.
    */
   bool
   lower_dst_modifiers(fs_visitor *v, bblock_t *block, fs_inst *inst)
   {
      const fs_builder ibld(v, block, inst);
      const brw_reg_type type = get_exec_type(inst);
      /* Not strictly necessary, but if possible use a temporary with the same
       * channel alignment as the current destination in order to avoid
       * violating the restrictions enforced later on by lower_src_region()
       * and lower_dst_region(), which would introduce additional copy
       * instructions into the program unnecessarily.
       */
      const unsigned stride =
         type_sz(inst->dst.type) * inst->dst.stride <= type_sz(type) ? 1 :
         type_sz(inst->dst.type) * inst->dst.stride / type_sz(type);
      fs_reg tmp = ibld.vgrf(type, stride);
      ibld.UNDEF(tmp);
      tmp = horiz_stride(tmp, stride);

      /* Emit a MOV taking care of all the destination modifiers. */
      fs_inst *mov = ibld.at(block, inst->next).MOV(inst->dst, tmp);
      mov->saturate = inst->saturate;
      if (!has_inconsistent_cmod(inst))
         mov->conditional_mod = inst->conditional_mod;
      if (inst->opcode != BRW_OPCODE_SEL) {
         mov->predicate = inst->predicate;
         mov->predicate_inverse = inst->predicate_inverse;
      }
      mov->flag_subreg = inst->flag_subreg;
      lower_instruction(v, block, mov);

      /* Point the original instruction at the temporary, and clean up any
       * destination modifiers.
       */
      assert(inst->size_written == inst->dst.component_size(inst->exec_size));
      inst->dst = tmp;
      inst->size_written = inst->dst.component_size(inst->exec_size);
      inst->saturate = false;
      if (!has_inconsistent_cmod(inst))
         inst->conditional_mod = BRW_CONDITIONAL_NONE;

      assert(!inst->flags_written(v->devinfo) || !mov->predicate);
      return true;
   }

   /**
    * Remove any non-trivial shuffling of data from the \p i-th source region
    * of the instruction.  Instead implement the region as a series of integer
    * copies into a temporary with the same channel layout as the destination.
    */
   bool
   lower_src_region(fs_visitor *v, bblock_t *block, fs_inst *inst, unsigned i)
   {
      assert(inst->components_read(i) == 1);
      const fs_builder ibld(v, block, inst);
      const unsigned stride = type_sz(inst->dst.type) * inst->dst.stride /
                              type_sz(inst->src[i].type);
      assert(stride > 0);
      fs_reg tmp = ibld.vgrf(inst->src[i].type, stride);
      ibld.UNDEF(tmp);
      tmp = horiz_stride(tmp, stride);

      /* Emit a series of 32-bit integer copies with any source modifiers
       * cleaned up (because their semantics are dependent on the type).
       */
      const brw_reg_type raw_type = brw_int_type(MIN2(type_sz(tmp.type), 4),
                                                 false);
      const unsigned n = type_sz(tmp.type) / type_sz(raw_type);
      fs_reg raw_src = inst->src[i];
      raw_src.negate = false;
      raw_src.abs = false;

      for (unsigned j = 0; j < n; j++)
         ibld.MOV(subscript(tmp, raw_type, j), subscript(raw_src, raw_type, j));

      /* Point the original instruction at the temporary, making sure to keep
       * any source modifiers in the instruction.
       */
      fs_reg lower_src = tmp;
      lower_src.negate = inst->src[i].negate;
      lower_src.abs = inst->src[i].abs;
      inst->src[i] = lower_src;

      return true;
   }

   /**
    * Remove any non-trivial shuffling of data from the destination region of
    * the instruction.  Instead implement the region as a series of integer
    * copies from a temporary with a channel layout compatible with the
    * sources.
    */
   bool
   lower_dst_region(fs_visitor *v, bblock_t *block, fs_inst *inst)
   {
      /* We cannot replace the result of an integer multiply which writes the
       * accumulator because MUL+MACH pairs act on the accumulator as a 66-bit
       * value whereas the MOV will act on only 32 or 33 bits of the
       * accumulator.
       */
      assert(inst->opcode != BRW_OPCODE_MUL || !inst->dst.is_accumulator() ||
             brw_reg_type_is_floating_point(inst->dst.type));

      const fs_builder ibld(v, block, inst);
      const unsigned stride = required_dst_byte_stride(inst) /
                              type_sz(inst->dst.type);
      assert(stride > 0);
      fs_reg tmp = ibld.vgrf(inst->dst.type, stride);
      ibld.UNDEF(tmp);
      tmp = horiz_stride(tmp, stride);

      /* Emit a series of 32-bit integer copies from the temporary into the
       * original destination.
       */
      const brw_reg_type raw_type = brw_int_type(MIN2(type_sz(tmp.type), 4),
                                                 false);
      const unsigned n = type_sz(tmp.type) / type_sz(raw_type);

      if (inst->predicate && inst->opcode != BRW_OPCODE_SEL) {
         /* Note that in general we cannot simply predicate the copies on the
          * same flag register as the original instruction, since it may have
          * been overwritten by the instruction itself.  Instead initialize
          * the temporary with the previous contents of the destination
          * register.
          */
         for (unsigned j = 0; j < n; j++)
            ibld.MOV(subscript(tmp, raw_type, j),
                     subscript(inst->dst, raw_type, j));
      }

      for (unsigned j = 0; j < n; j++)
         ibld.at(block, inst->next).MOV(subscript(inst->dst, raw_type, j),
                                        subscript(tmp, raw_type, j));

      /* Point the original instruction at the temporary, making sure to keep
       * any destination modifiers in the instruction.
       */
      assert(inst->size_written == inst->dst.component_size(inst->exec_size));
      inst->dst = tmp;
      inst->size_written = inst->dst.component_size(inst->exec_size);

      return true;
   }

   /**
    * Change sources and destination of the instruction to an
    * appropriate legal type, splitting the instruction into multiple
    * ones of smaller execution type if necessary, to be used in cases
    * where the execution type of an instruction is unsupported.
    */
   bool
   lower_exec_type(fs_visitor *v, bblock_t *block, fs_inst *inst)
   {
      assert(inst->dst.type == get_exec_type(inst));
      const unsigned mask = has_invalid_exec_type(v->devinfo, inst);
      const brw_reg_type raw_type = required_exec_type(v->devinfo, inst);
      const unsigned n = get_exec_type_size(inst) / type_sz(raw_type);
      const fs_builder ibld(v, block, inst);

      fs_reg tmp = ibld.vgrf(inst->dst.type, inst->dst.stride);
      ibld.UNDEF(tmp);
      tmp = horiz_stride(tmp, inst->dst.stride);

      for (unsigned j = 0; j < n; j++) {
         fs_inst sub_inst = *inst;

         for (unsigned i = 0; i < inst->sources; i++) {
            if (mask & (1u << i)) {
               assert(inst->src[i].type == inst->dst.type);
               sub_inst.src[i] = subscript(inst->src[i], raw_type, j);
            }
         }

         sub_inst.dst = subscript(tmp, raw_type, j);

         assert(sub_inst.size_written == sub_inst.dst.component_size(sub_inst.exec_size));
         assert(!sub_inst.flags_written(v->devinfo) && !sub_inst.saturate);
         ibld.emit(sub_inst);

         fs_inst *mov = ibld.MOV(subscript(inst->dst, raw_type, j),
                                 subscript(tmp, raw_type, j));
         if (inst->opcode != BRW_OPCODE_SEL) {
            mov->predicate = inst->predicate;
            mov->predicate_inverse = inst->predicate_inverse;
         }
         lower_instruction(v, block, mov);
      }

      inst->remove(block);

      return true;
   }

   /**
    * Legalize the source and destination regioning controls of the specified
    * instruction.
    */
   bool
   lower_instruction(fs_visitor *v, bblock_t *block, fs_inst *inst)
   {
      const intel_device_info *devinfo = v->devinfo;
      bool progress = false;

      if (has_invalid_dst_modifiers(devinfo, inst))
         progress |= lower_dst_modifiers(v, block, inst);

      if (has_invalid_dst_region(devinfo, inst))
         progress |= lower_dst_region(v, block, inst);

      for (unsigned i = 0; i < inst->sources; i++) {
         if (has_invalid_src_modifiers(devinfo, inst, i))
            progress |= lower_src_modifiers(v, block, inst, i);

         if (has_invalid_src_region(devinfo, inst, i))
            progress |= lower_src_region(v, block, inst, i);
      }

      if (has_invalid_exec_type(devinfo, inst))
         progress |= lower_exec_type(v, block, inst);

      return progress;
   }
}

bool
fs_visitor::lower_regioning()
{
   bool progress = false;

   foreach_block_and_inst_safe(block, fs_inst, inst, cfg)
      progress |= lower_instruction(this, block, inst);

   if (progress)
      invalidate_analysis(DEPENDENCY_INSTRUCTIONS | DEPENDENCY_VARIABLES);

   return progress;
}
