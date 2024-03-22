/*
 * Copyright © 2011 Intel Corporation
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

/**
 * @file brw_vec4_copy_propagation.cpp
 *
 * Implements tracking of values copied between registers, and
 * optimizations based on that: copy propagation and constant
 * propagation.
 */

#include "brw_vec4.h"
#include "brw_cfg.h"
#include "brw_eu.h"

namespace brw {

struct copy_entry {
   src_reg *value[4];
   int saturatemask;
};

static bool
is_direct_copy(vec4_instruction *inst)
{
   return (inst->opcode == BRW_OPCODE_MOV &&
	   !inst->predicate &&
	   inst->dst.file == VGRF &&
	   inst->dst.offset % REG_SIZE == 0 &&
	   !inst->dst.reladdr &&
	   !inst->src[0].reladdr &&
	   (inst->dst.type == inst->src[0].type ||
            (inst->dst.type == BRW_REGISTER_TYPE_F &&
             inst->src[0].type == BRW_REGISTER_TYPE_VF)));
}

static bool
is_dominated_by_previous_instruction(vec4_instruction *inst)
{
   return (inst->opcode != BRW_OPCODE_DO &&
	   inst->opcode != BRW_OPCODE_WHILE &&
	   inst->opcode != BRW_OPCODE_ELSE &&
	   inst->opcode != BRW_OPCODE_ENDIF);
}

static bool
is_channel_updated(vec4_instruction *inst, src_reg *values[4], int ch)
{
   const src_reg *src = values[ch];

   /* consider GRF only */
   assert(inst->dst.file == VGRF);
   if (!src || src->file != VGRF)
      return false;

   return regions_overlap(*src, REG_SIZE, inst->dst, inst->size_written) &&
          (inst->dst.offset != src->offset ||
           inst->dst.writemask & (1 << BRW_GET_SWZ(src->swizzle, ch)));
}

/**
 * Get the origin of a copy as a single register if all components present in
 * the given readmask originate from the same register and have compatible
 * regions, otherwise return a BAD_FILE register.
 */
static src_reg
get_copy_value(const copy_entry &entry, unsigned readmask)
{
   unsigned swz[4] = {};
   src_reg value;

   for (unsigned i = 0; i < 4; i++) {
      if (readmask & (1 << i)) {
         if (entry.value[i]) {
            src_reg src = *entry.value[i];

            if (src.file == IMM) {
               swz[i] = i;
            } else {
               swz[i] = BRW_GET_SWZ(src.swizzle, i);
               /* Overwrite the original swizzle so the src_reg::equals call
                * below doesn't care about it, the correct swizzle will be
                * calculated once the swizzles of all components are known.
                */
               src.swizzle = BRW_SWIZZLE_XYZW;
            }

            if (value.file == BAD_FILE) {
               value = src;
            } else if (!value.equals(src)) {
               return src_reg();
            }
         } else {
            return src_reg();
         }
      }
   }

   return swizzle(value,
                  brw_compose_swizzle(brw_swizzle_for_mask(readmask),
                                      BRW_SWIZZLE4(swz[0], swz[1],
                                                   swz[2], swz[3])));
}

static bool
try_constant_propagate(vec4_instruction *inst,
                       int arg, const copy_entry *entry)
{
   /* For constant propagation, we only handle the same constant
    * across all 4 channels.  Some day, we should handle the 8-bit
    * float vector format, which would let us constant propagate
    * vectors better.
    * We could be more aggressive here -- some channels might not get used
    * based on the destination writemask.
    */
   src_reg value =
      get_copy_value(*entry,
                     brw_apply_inv_swizzle_to_mask(inst->src[arg].swizzle,
                                                   WRITEMASK_XYZW));

   if (value.file != IMM)
      return false;

   /* 64-bit types can't be used except for one-source instructions, which
    * higher levels should have constant folded away, so there's no point in
    * propagating immediates here.
    */
   if (type_sz(value.type) == 8 || type_sz(inst->src[arg].type) == 8)
      return false;

   if (value.type == BRW_REGISTER_TYPE_VF) {
      /* The result of bit-casting the component values of a vector float
       * cannot in general be represented as an immediate.
       */
      if (inst->src[arg].type != BRW_REGISTER_TYPE_F)
         return false;
   } else {
      value.type = inst->src[arg].type;
   }

   if (inst->src[arg].abs) {
      if (!brw_abs_immediate(value.type, &value.as_brw_reg()))
         return false;
   }

   if (inst->src[arg].negate) {
      if (!brw_negate_immediate(value.type, &value.as_brw_reg()))
         return false;
   }

   value = swizzle(value, inst->src[arg].swizzle);

   switch (inst->opcode) {
   case BRW_OPCODE_MOV:
   case SHADER_OPCODE_BROADCAST:
      inst->src[arg] = value;
      return true;

   case VEC4_OPCODE_UNTYPED_ATOMIC:
      if (arg == 1) {
         inst->src[arg] = value;
         return true;
      }
      break;

   case SHADER_OPCODE_POW:
   case SHADER_OPCODE_INT_QUOTIENT:
   case SHADER_OPCODE_INT_REMAINDER:
         break;
   case BRW_OPCODE_DP2:
   case BRW_OPCODE_DP3:
   case BRW_OPCODE_DP4:
   case BRW_OPCODE_DPH:
   case BRW_OPCODE_BFI1:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_SHR:
   case BRW_OPCODE_SUBB:
      if (arg == 1) {
         inst->src[arg] = value;
         return true;
      }
      break;

   case BRW_OPCODE_MACH:
   case BRW_OPCODE_MUL:
   case SHADER_OPCODE_MULH:
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_OR:
   case BRW_OPCODE_AND:
   case BRW_OPCODE_XOR:
   case BRW_OPCODE_ADDC:
      if (arg == 1) {
	 inst->src[arg] = value;
	 return true;
      } else if (arg == 0 && inst->src[1].file != IMM) {
	 /* Fit this constant in by commuting the operands.  Exception: we
	  * can't do this for 32-bit integer MUL/MACH because it's asymmetric.
	  */
	 if ((inst->opcode == BRW_OPCODE_MUL ||
              inst->opcode == BRW_OPCODE_MACH) &&
	     (inst->src[1].type == BRW_REGISTER_TYPE_D ||
	      inst->src[1].type == BRW_REGISTER_TYPE_UD))
	    break;
	 inst->src[0] = inst->src[1];
	 inst->src[1] = value;
	 return true;
      }
      break;
   case GS_OPCODE_SET_WRITE_OFFSET:
      /* This is just a multiply by a constant with special strides.
       * The generator will handle immediates in both arguments (generating
       * a single MOV of the product).  So feel free to propagate in src0.
       */
      inst->src[arg] = value;
      return true;

   case BRW_OPCODE_CMP:
      if (arg == 1) {
	 inst->src[arg] = value;
	 return true;
      } else if (arg == 0 && inst->src[1].file != IMM) {
	 enum brw_conditional_mod new_cmod;

	 new_cmod = brw_swap_cmod(inst->conditional_mod);
	 if (new_cmod != BRW_CONDITIONAL_NONE) {
	    /* Fit this constant in by swapping the operands and
	     * flipping the test.
	     */
	    inst->src[0] = inst->src[1];
	    inst->src[1] = value;
	    inst->conditional_mod = new_cmod;
	    return true;
	 }
      }
      break;

   case BRW_OPCODE_SEL:
      if (arg == 1) {
	 inst->src[arg] = value;
	 return true;
      } else if (arg == 0 && inst->src[1].file != IMM) {
	 inst->src[0] = inst->src[1];
	 inst->src[1] = value;

	 /* If this was predicated, flipping operands means
	  * we also need to flip the predicate.
	  */
	 if (inst->conditional_mod == BRW_CONDITIONAL_NONE) {
	    inst->predicate_inverse = !inst->predicate_inverse;
	 }
	 return true;
      }
      break;

   default:
      break;
   }

   return false;
}

static bool
is_align1_opcode(unsigned opcode)
{
   switch (opcode) {
   case VEC4_OPCODE_DOUBLE_TO_F32:
   case VEC4_OPCODE_DOUBLE_TO_D32:
   case VEC4_OPCODE_DOUBLE_TO_U32:
   case VEC4_OPCODE_TO_DOUBLE:
   case VEC4_OPCODE_PICK_LOW_32BIT:
   case VEC4_OPCODE_PICK_HIGH_32BIT:
   case VEC4_OPCODE_SET_LOW_32BIT:
   case VEC4_OPCODE_SET_HIGH_32BIT:
      return true;
   default:
      return false;
   }
}

static bool
try_copy_propagate(const struct brw_compiler *compiler,
                   vec4_instruction *inst, int arg,
                   const copy_entry *entry, int attributes_per_reg)
{
   const struct intel_device_info *devinfo = compiler->devinfo;

   /* Build up the value we are propagating as if it were the source of a
    * single MOV
    */
   src_reg value =
      get_copy_value(*entry,
                     brw_apply_inv_swizzle_to_mask(inst->src[arg].swizzle,
                                                   WRITEMASK_XYZW));

   /* Check that we can propagate that value */
   if (value.file != UNIFORM &&
       value.file != VGRF &&
       value.file != ATTR)
      return false;

   /* Instructions that write 2 registers also need to read 2 registers. Make
    * sure we don't break that restriction by copy propagating from a uniform.
    */
   if (inst->size_written > REG_SIZE && is_uniform(value))
      return false;

   /* There is a regioning restriction such that if execsize == width
    * and hstride != 0 then the vstride can't be 0. When we split instrutions
    * that take a single-precision source (like F->DF conversions) we end up
    * with a 4-wide source on an instruction with an execution size of 4.
    * If we then copy-propagate the source from a uniform we also end up with a
    * vstride of 0 and we violate the restriction.
    */
   if (inst->exec_size == 4 && value.file == UNIFORM &&
       type_sz(value.type) == 4)
      return false;

   /* If the type of the copy value is different from the type of the
    * instruction then the swizzles and writemasks involved don't have the same
    * meaning and simply replacing the source would produce different semantics.
    */
   if (type_sz(value.type) != type_sz(inst->src[arg].type))
      return false;

   if (inst->src[arg].offset % REG_SIZE || value.offset % REG_SIZE)
      return false;

   bool has_source_modifiers = value.negate || value.abs;

   /* gfx6 math and gfx7+ SENDs from GRFs ignore source modifiers on
    * instructions.
    */
   if (has_source_modifiers && !inst->can_do_source_mods(devinfo))
      return false;

   /* Reject cases that would violate register regioning restrictions. */
   if ((value.file == UNIFORM || value.swizzle != BRW_SWIZZLE_XYZW) &&
       ((devinfo->ver == 6 && inst->is_math()) ||
        inst->is_send_from_grf() ||
        inst->uses_indirect_addressing())) {
      return false;
   }

   if (has_source_modifiers &&
       value.type != inst->src[arg].type &&
       !inst->can_change_types())
      return false;

   if (has_source_modifiers &&
       (inst->opcode == SHADER_OPCODE_GFX4_SCRATCH_WRITE ||
        inst->opcode == VEC4_OPCODE_PICK_HIGH_32BIT))
      return false;

   unsigned composed_swizzle = brw_compose_swizzle(inst->src[arg].swizzle,
                                                   value.swizzle);

   /* Instructions that operate on vectors in ALIGN1 mode will ignore swizzles
    * so copy-propagation won't be safe if the composed swizzle is anything
    * other than the identity.
    */
   if (is_align1_opcode(inst->opcode) && composed_swizzle != BRW_SWIZZLE_XYZW)
      return false;

   if (inst->is_3src(compiler) &&
       (value.file == UNIFORM ||
        (value.file == ATTR && attributes_per_reg != 1)) &&
       !brw_is_single_value_swizzle(composed_swizzle))
      return false;

   if (inst->is_send_from_grf())
      return false;

   /* we can't generally copy-propagate UD negations because we
    * end up accessing the resulting values as signed integers
    * instead. See also resolve_ud_negate().
    */
   if (value.negate &&
       value.type == BRW_REGISTER_TYPE_UD)
      return false;

   /* Don't report progress if this is a noop. */
   if (value.equals(inst->src[arg]))
      return false;

   const unsigned dst_saturate_mask = inst->dst.writemask &
      brw_apply_swizzle_to_mask(inst->src[arg].swizzle, entry->saturatemask);

   if (dst_saturate_mask) {
      /* We either saturate all or nothing. */
      if (dst_saturate_mask != inst->dst.writemask)
         return false;

      /* Limit saturate propagation only to SEL with src1 bounded within 0.0
       * and 1.0, otherwise skip copy propagate altogether.
       */
      switch(inst->opcode) {
      case BRW_OPCODE_SEL:
         if (arg != 0 ||
             inst->src[0].type != BRW_REGISTER_TYPE_F ||
             inst->src[1].file != IMM ||
             inst->src[1].type != BRW_REGISTER_TYPE_F ||
             inst->src[1].f < 0.0 ||
             inst->src[1].f > 1.0) {
            return false;
         }
         if (!inst->saturate)
            inst->saturate = true;
         break;
      default:
         return false;
      }
   }

   /* Build the final value */
   if (inst->src[arg].abs) {
      value.negate = false;
      value.abs = true;
   }
   if (inst->src[arg].negate)
      value.negate = !value.negate;

   value.swizzle = composed_swizzle;
   if (has_source_modifiers &&
       value.type != inst->src[arg].type) {
      assert(inst->can_change_types());
      for (int i = 0; i < 3; i++) {
         inst->src[i].type = value.type;
      }
      inst->dst.type = value.type;
   } else {
      value.type = inst->src[arg].type;
   }

   inst->src[arg] = value;
   return true;
}

bool
vec4_visitor::opt_copy_propagation(bool do_constant_prop)
{
   /* If we are in dual instanced or single mode, then attributes are going
    * to be interleaved, so one register contains two attribute slots.
    */
   const int attributes_per_reg =
      prog_data->dispatch_mode == DISPATCH_MODE_4X2_DUAL_OBJECT ? 1 : 2;
   bool progress = false;
   struct copy_entry entries[alloc.total_size];

   memset(&entries, 0, sizeof(entries));

   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      /* This pass only works on basic blocks.  If there's flow
       * control, throw out all our information and start from
       * scratch.
       *
       * This should really be fixed by using a structure like in
       * src/glsl/opt_copy_propagation.cpp to track available copies.
       */
      if (!is_dominated_by_previous_instruction(inst)) {
	 memset(&entries, 0, sizeof(entries));
	 continue;
      }

      /* For each source arg, see if each component comes from a copy
       * from the same type file (IMM, VGRF, UNIFORM), and try
       * optimizing out access to the copy result
       */
      for (int i = 2; i >= 0; i--) {
	 /* Copied values end up in GRFs, and we don't track reladdr
	  * accesses.
	  */
	 if (inst->src[i].file != VGRF ||
	     inst->src[i].reladdr)
	    continue;

         /* We only handle register-aligned single GRF copies. */
         if (inst->size_read(i) != REG_SIZE ||
             inst->src[i].offset % REG_SIZE)
            continue;

         const unsigned reg = (alloc.offsets[inst->src[i].nr] +
                               inst->src[i].offset / REG_SIZE);
         const copy_entry &entry = entries[reg];

         if (do_constant_prop && try_constant_propagate(inst, i, &entry))
            progress = true;
         else if (try_copy_propagate(compiler, inst, i, &entry, attributes_per_reg))
	    progress = true;
      }

      /* Track available source registers. */
      if (inst->dst.file == VGRF) {
	 const int reg =
            alloc.offsets[inst->dst.nr] + inst->dst.offset / REG_SIZE;

	 /* Update our destination's current channel values.  For a direct copy,
	  * the value is the newly propagated source.  Otherwise, we don't know
	  * the new value, so clear it.
	  */
	 bool direct_copy = is_direct_copy(inst);
         entries[reg].saturatemask &= ~inst->dst.writemask;
	 for (int i = 0; i < 4; i++) {
	    if (inst->dst.writemask & (1 << i)) {
               entries[reg].value[i] = direct_copy ? &inst->src[0] : NULL;
               entries[reg].saturatemask |=
                  inst->saturate && direct_copy ? 1 << i : 0;
            }
	 }

	 /* Clear the records for any registers whose current value came from
	  * our destination's updated channels, as the two are no longer equal.
	  */
	 if (inst->dst.reladdr)
	    memset(&entries, 0, sizeof(entries));
	 else {
	    for (unsigned i = 0; i < alloc.total_size; i++) {
	       for (int j = 0; j < 4; j++) {
		  if (is_channel_updated(inst, entries[i].value, j)) {
		     entries[i].value[j] = NULL;
		     entries[i].saturatemask &= ~(1 << j);
                  }
	       }
	    }
	 }
      }
   }

   if (progress)
      invalidate_analysis(DEPENDENCY_INSTRUCTION_DATA_FLOW |
                          DEPENDENCY_INSTRUCTION_DETAIL);

   return progress;
}

} /* namespace brw */
