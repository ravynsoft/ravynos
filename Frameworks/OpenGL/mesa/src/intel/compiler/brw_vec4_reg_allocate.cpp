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

#include "util/register_allocate.h"
#include "brw_vec4.h"
#include "brw_cfg.h"

using namespace brw;

#define REG_CLASS_COUNT 20

namespace brw {

static void
assign(unsigned int *reg_hw_locations, backend_reg *reg)
{
   if (reg->file == VGRF) {
      reg->nr = reg_hw_locations[reg->nr] + reg->offset / REG_SIZE;
      reg->offset %= REG_SIZE;
   }
}

bool
vec4_visitor::reg_allocate_trivial()
{
   unsigned int hw_reg_mapping[this->alloc.count];
   bool virtual_grf_used[this->alloc.count];
   int next;

   /* Calculate which virtual GRFs are actually in use after whatever
    * optimization passes have occurred.
    */
   for (unsigned i = 0; i < this->alloc.count; i++) {
      virtual_grf_used[i] = false;
   }

   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      if (inst->dst.file == VGRF)
         virtual_grf_used[inst->dst.nr] = true;

      for (unsigned i = 0; i < 3; i++) {
	 if (inst->src[i].file == VGRF)
            virtual_grf_used[inst->src[i].nr] = true;
      }
   }

   hw_reg_mapping[0] = this->first_non_payload_grf;
   next = hw_reg_mapping[0] + this->alloc.sizes[0];
   for (unsigned i = 1; i < this->alloc.count; i++) {
      if (virtual_grf_used[i]) {
	 hw_reg_mapping[i] = next;
	 next += this->alloc.sizes[i];
      }
   }
   prog_data->total_grf = next;

   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      assign(hw_reg_mapping, &inst->dst);
      assign(hw_reg_mapping, &inst->src[0]);
      assign(hw_reg_mapping, &inst->src[1]);
      assign(hw_reg_mapping, &inst->src[2]);
   }

   if (prog_data->total_grf > max_grf) {
      fail("Ran out of regs on trivial allocator (%d/%d)\n",
	   prog_data->total_grf, max_grf);
      return false;
   }

   return true;
}

extern "C" void
brw_vec4_alloc_reg_set(struct brw_compiler *compiler)
{
   int base_reg_count =
      compiler->devinfo->ver >= 7 ? GFX7_MRF_HACK_START : BRW_MAX_GRF;

   assert(compiler->devinfo->ver < 8);

   /* After running split_virtual_grfs(), almost all VGRFs will be of size 1.
    * SEND-from-GRF sources cannot be split, so we also need classes for each
    * potential message length.
    */
   assert(REG_CLASS_COUNT == MAX_VGRF_SIZE(compiler->devinfo));
   int class_sizes[REG_CLASS_COUNT];

   for (int i = 0; i < REG_CLASS_COUNT; i++)
      class_sizes[i] = i + 1;


   ralloc_free(compiler->vec4_reg_set.regs);
   compiler->vec4_reg_set.regs = ra_alloc_reg_set(compiler, base_reg_count, false);
   if (compiler->devinfo->ver >= 6)
      ra_set_allocate_round_robin(compiler->vec4_reg_set.regs);
   ralloc_free(compiler->vec4_reg_set.classes);
   compiler->vec4_reg_set.classes = ralloc_array(compiler, struct ra_class *, REG_CLASS_COUNT);

   /* Now, add the registers to their classes, and add the conflicts
    * between them and the base GRF registers (and also each other).
    */
   for (int i = 0; i < REG_CLASS_COUNT; i++) {
      int class_reg_count = base_reg_count - (class_sizes[i] - 1);
      compiler->vec4_reg_set.classes[i] =
         ra_alloc_contig_reg_class(compiler->vec4_reg_set.regs, class_sizes[i]);

      for (int j = 0; j < class_reg_count; j++)
         ra_class_add_reg(compiler->vec4_reg_set.classes[i], j);
   }

   ra_set_finalize(compiler->vec4_reg_set.regs, NULL);
}

void
vec4_visitor::setup_payload_interference(struct ra_graph *g,
                                         int first_payload_node,
                                         int reg_node_count)
{
   int payload_node_count = this->first_non_payload_grf;

   for (int i = 0; i < payload_node_count; i++) {
      /* Mark each payload reg node as being allocated to its physical register.
       *
       * The alternative would be to have per-physical register classes, which
       * would just be silly.
       */
      ra_set_node_reg(g, first_payload_node + i, i);

      /* For now, just mark each payload node as interfering with every other
       * node to be allocated.
       */
      for (int j = 0; j < reg_node_count; j++) {
         ra_add_node_interference(g, first_payload_node + i, j);
      }
   }
}

bool
vec4_visitor::reg_allocate()
{
   unsigned int hw_reg_mapping[alloc.count];
   int payload_reg_count = this->first_non_payload_grf;

   /* Using the trivial allocator can be useful in debugging undefined
    * register access as a result of broken optimization passes.
    */
   if (0)
      return reg_allocate_trivial();

   assert(devinfo->ver < 8);

   const vec4_live_variables &live = live_analysis.require();
   int node_count = alloc.count;
   int first_payload_node = node_count;
   node_count += payload_reg_count;
   struct ra_graph *g =
      ra_alloc_interference_graph(compiler->vec4_reg_set.regs, node_count);

   for (unsigned i = 0; i < alloc.count; i++) {
      int size = this->alloc.sizes[i];
      assert(size >= 1 && size <= MAX_VGRF_SIZE(devinfo));
      ra_set_node_class(g, i, compiler->vec4_reg_set.classes[size - 1]);

      for (unsigned j = 0; j < i; j++) {
	 if (live.vgrfs_interfere(i, j)) {
	    ra_add_node_interference(g, i, j);
	 }
      }
   }

   /* Certain instructions can't safely use the same register for their
    * sources and destination.  Add interference.
    */
   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      if (inst->dst.file == VGRF && inst->has_source_and_destination_hazard()) {
         for (unsigned i = 0; i < 3; i++) {
            if (inst->src[i].file == VGRF) {
               ra_add_node_interference(g, inst->dst.nr, inst->src[i].nr);
            }
         }
      }
   }

   setup_payload_interference(g, first_payload_node, node_count);

   if (!ra_allocate(g)) {
      /* Failed to allocate registers.  Spill a reg, and the caller will
       * loop back into here to try again.
       */
      int reg = choose_spill_reg(g);
      if (this->no_spills) {
         fail("Failure to register allocate.  Reduce number of live "
              "values to avoid this.");
      } else if (reg == -1) {
         fail("no register to spill\n");
      } else {
         spill_reg(reg);
      }
      ralloc_free(g);
      return false;
   }

   /* Get the chosen virtual registers for each node, and map virtual
    * regs in the register classes back down to real hardware reg
    * numbers.
    */
   prog_data->total_grf = payload_reg_count;
   for (unsigned i = 0; i < alloc.count; i++) {
      hw_reg_mapping[i] = ra_get_node_reg(g, i);
      prog_data->total_grf = MAX2(prog_data->total_grf,
				  hw_reg_mapping[i] + alloc.sizes[i]);
   }

   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      assign(hw_reg_mapping, &inst->dst);
      assign(hw_reg_mapping, &inst->src[0]);
      assign(hw_reg_mapping, &inst->src[1]);
      assign(hw_reg_mapping, &inst->src[2]);
   }

   ralloc_free(g);

   return true;
}

/**
 * When we decide to spill a register, instead of blindly spilling every use,
 * save unspills when the spill register is used (read) in consecutive
 * instructions. This can potentially save a bunch of unspills that would
 * have very little impact in register allocation anyway.
 *
 * Notice that we need to account for this behavior when spilling a register
 * and when evaluating spilling costs. This function is designed so it can
 * be called from both places and avoid repeating the logic.
 *
 *  - When we call this function from spill_reg(), we pass in scratch_reg the
 *    actual unspill/spill register that we want to reuse in the current
 *    instruction.
 *
 *  - When we call this from evaluate_spill_costs(), we pass the register for
 *    which we are evaluating spilling costs.
 *
 * In either case, we check if the previous instructions read scratch_reg until
 * we find one that writes to it with a compatible mask or does not read/write
 * scratch_reg at all.
 */
static bool
can_use_scratch_for_source(const vec4_instruction *inst, unsigned i,
                           unsigned scratch_reg)
{
   assert(inst->src[i].file == VGRF);
   bool prev_inst_read_scratch_reg = false;

   /* See if any previous source in the same instructions reads scratch_reg */
   for (unsigned n = 0; n < i; n++) {
      if (inst->src[n].file == VGRF && inst->src[n].nr == scratch_reg)
         prev_inst_read_scratch_reg = true;
   }

   /* Now check if previous instructions read/write scratch_reg */
   for (vec4_instruction *prev_inst = (vec4_instruction *) inst->prev;
        !prev_inst->is_head_sentinel();
        prev_inst = (vec4_instruction *) prev_inst->prev) {

      /* If the previous instruction writes to scratch_reg then we can reuse
       * it if the write is not conditional and the channels we write are
       * compatible with our read mask
       */
      if (prev_inst->dst.file == VGRF && prev_inst->dst.nr == scratch_reg) {
         return (!prev_inst->predicate || prev_inst->opcode == BRW_OPCODE_SEL) &&
                (brw_mask_for_swizzle(inst->src[i].swizzle) &
                 ~prev_inst->dst.writemask) == 0;
      }

      /* Skip scratch read/writes so that instructions generated by spilling
       * other registers (that won't read/write scratch_reg) do not stop us from
       * reusing scratch_reg for this instruction.
       */
      if (prev_inst->opcode == SHADER_OPCODE_GFX4_SCRATCH_WRITE ||
          prev_inst->opcode == SHADER_OPCODE_GFX4_SCRATCH_READ)
         continue;

      /* If the previous instruction does not write to scratch_reg, then check
       * if it reads it
       */
      int n;
      for (n = 0; n < 3; n++) {
         if (prev_inst->src[n].file == VGRF &&
             prev_inst->src[n].nr == scratch_reg) {
            prev_inst_read_scratch_reg = true;
            break;
         }
      }
      if (n == 3) {
         /* The previous instruction does not read scratch_reg. At this point,
          * if no previous instruction has read scratch_reg it means that we
          * will need to unspill it here and we can't reuse it (so we return
          * false). Otherwise, if we found at least one consecutive instruction
          * that read scratch_reg, then we know that we got here from
          * evaluate_spill_costs (since for the spill_reg path any block of
          * consecutive instructions using scratch_reg must start with a write
          * to that register, so we would've exited the loop in the check for
          * the write that we have at the start of this loop), and in that case
          * it means that we found the point at which the scratch_reg would be
          * unspilled. Since we always unspill a full vec4, it means that we
          * have all the channels available and we can just return true to
          * signal that we can reuse the register in the current instruction
          * too.
          */
         return prev_inst_read_scratch_reg;
      }
   }

   return prev_inst_read_scratch_reg;
}

static inline float
spill_cost_for_type(enum brw_reg_type type)
{
   /* Spilling of a 64-bit register involves emitting 2 32-bit scratch
    * messages plus the 64b/32b shuffling code.
    */
   return type_sz(type) == 8 ? 2.25f : 1.0f;
}

void
vec4_visitor::evaluate_spill_costs(float *spill_costs, bool *no_spill)
{
   float loop_scale = 1.0;

   unsigned *reg_type_size = (unsigned *)
      ralloc_size(NULL, this->alloc.count * sizeof(unsigned));

   for (unsigned i = 0; i < this->alloc.count; i++) {
      spill_costs[i] = 0.0;
      no_spill[i] = alloc.sizes[i] != 1 && alloc.sizes[i] != 2;
      reg_type_size[i] = 0;
   }

   /* Calculate costs for spilling nodes.  Call it a cost of 1 per
    * spill/unspill we'll have to do, and guess that the insides of
    * loops run 10 times.
    */
   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      for (unsigned int i = 0; i < 3; i++) {
         if (inst->src[i].file == VGRF && !no_spill[inst->src[i].nr]) {
            /* We will only unspill src[i] it it wasn't unspilled for the
             * previous instruction, in which case we'll just reuse the scratch
             * reg for this instruction.
             */
            if (!can_use_scratch_for_source(inst, i, inst->src[i].nr)) {
               spill_costs[inst->src[i].nr] +=
                  loop_scale * spill_cost_for_type(inst->src[i].type);
               if (inst->src[i].reladdr ||
                   inst->src[i].offset >= REG_SIZE)
                  no_spill[inst->src[i].nr] = true;

               /* We don't support unspills of partial DF reads.
                *
                * Our 64-bit unspills are implemented with two 32-bit scratch
                * messages, each one reading that for both SIMD4x2 threads that
                * we need to shuffle into correct 64-bit data. Ensure that we
                * are reading data for both threads.
                */
               if (type_sz(inst->src[i].type) == 8 && inst->exec_size != 8)
                  no_spill[inst->src[i].nr] = true;
            }

            /* We can't spill registers that mix 32-bit and 64-bit access (that
             * contain 64-bit data that is operated on via 32-bit instructions)
             */
            unsigned type_size = type_sz(inst->src[i].type);
            if (reg_type_size[inst->src[i].nr] == 0)
               reg_type_size[inst->src[i].nr] = type_size;
            else if (reg_type_size[inst->src[i].nr] != type_size)
               no_spill[inst->src[i].nr] = true;
         }
      }

      if (inst->dst.file == VGRF && !no_spill[inst->dst.nr]) {
         spill_costs[inst->dst.nr] +=
            loop_scale * spill_cost_for_type(inst->dst.type);
         if (inst->dst.reladdr || inst->dst.offset >= REG_SIZE)
            no_spill[inst->dst.nr] = true;

         /* We don't support spills of partial DF writes.
          *
          * Our 64-bit spills are implemented with two 32-bit scratch messages,
          * each one writing that for both SIMD4x2 threads. Ensure that we
          * are writing data for both threads.
          */
         if (type_sz(inst->dst.type) == 8 && inst->exec_size != 8)
            no_spill[inst->dst.nr] = true;

         /* We can't spill registers that mix 32-bit and 64-bit access (that
          * contain 64-bit data that is operated on via 32-bit instructions)
          */
         unsigned type_size = type_sz(inst->dst.type);
         if (reg_type_size[inst->dst.nr] == 0)
            reg_type_size[inst->dst.nr] = type_size;
         else if (reg_type_size[inst->dst.nr] != type_size)
            no_spill[inst->dst.nr] = true;
      }

      switch (inst->opcode) {

      case BRW_OPCODE_DO:
         loop_scale *= 10;
         break;

      case BRW_OPCODE_WHILE:
         loop_scale /= 10;
         break;

      case SHADER_OPCODE_GFX4_SCRATCH_READ:
      case SHADER_OPCODE_GFX4_SCRATCH_WRITE:
      case VEC4_OPCODE_MOV_FOR_SCRATCH:
         for (int i = 0; i < 3; i++) {
            if (inst->src[i].file == VGRF)
               no_spill[inst->src[i].nr] = true;
         }
         if (inst->dst.file == VGRF)
            no_spill[inst->dst.nr] = true;
         break;

      default:
         break;
      }
   }

   ralloc_free(reg_type_size);
}

int
vec4_visitor::choose_spill_reg(struct ra_graph *g)
{
   float spill_costs[this->alloc.count];
   bool no_spill[this->alloc.count];

   evaluate_spill_costs(spill_costs, no_spill);

   for (unsigned i = 0; i < this->alloc.count; i++) {
      if (!no_spill[i])
         ra_set_node_spill_cost(g, i, spill_costs[i]);
   }

   return ra_get_best_spill_node(g);
}

void
vec4_visitor::spill_reg(unsigned spill_reg_nr)
{
   assert(alloc.sizes[spill_reg_nr] == 1 || alloc.sizes[spill_reg_nr] == 2);
   unsigned spill_offset = last_scratch;
   last_scratch += alloc.sizes[spill_reg_nr];

   /* Generate spill/unspill instructions for the objects being spilled. */
   unsigned scratch_reg = ~0u;
   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      for (unsigned i = 0; i < 3; i++) {
         if (inst->src[i].file == VGRF && inst->src[i].nr == spill_reg_nr) {
            if (scratch_reg == ~0u ||
                !can_use_scratch_for_source(inst, i, scratch_reg)) {
               /* We need to unspill anyway so make sure we read the full vec4
                * in any case. This way, the cached register can be reused
                * for consecutive instructions that read different channels of
                * the same vec4.
                */
               scratch_reg = alloc.allocate(alloc.sizes[spill_reg_nr]);
               src_reg temp = inst->src[i];
               temp.nr = scratch_reg;
               temp.offset = 0;
               temp.swizzle = BRW_SWIZZLE_XYZW;
               emit_scratch_read(block, inst,
                                 dst_reg(temp), inst->src[i], spill_offset);
               temp.offset = inst->src[i].offset;
            }
            assert(scratch_reg != ~0u);
            inst->src[i].nr = scratch_reg;
         }
      }

      if (inst->dst.file == VGRF && inst->dst.nr == spill_reg_nr) {
         emit_scratch_write(block, inst, spill_offset);
         scratch_reg = inst->dst.nr;
      }
   }

   invalidate_analysis(DEPENDENCY_INSTRUCTIONS | DEPENDENCY_VARIABLES);
}

} /* namespace brw */
