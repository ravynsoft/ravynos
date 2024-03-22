/*
 * Copyright (c) 2019 Zodiac Inflight Innovations
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Jonathan Marek <jonathan@marek.ca>
 */

#include "etnaviv_compiler_nir.h"
#include "util/register_allocate.h"

/* use "r63.z" for depth reg, it will wrap around to r0.z by reg_get_base
 * (fs registers are offset by 1 to avoid reserving r0)
 */
#define REG_FRAG_DEPTH ((ETNA_MAX_TEMPS - 1) * NUM_REG_TYPES + REG_TYPE_VIRT_SCALAR_Z)

/* precomputed by register_allocate  */
static unsigned int *q_values[] = {
   (unsigned int[]) {1, 2, 3, 4, 2, 2, 3, },
   (unsigned int[]) {3, 5, 6, 6, 5, 5, 6, },
   (unsigned int[]) {3, 4, 4, 4, 4, 4, 4, },
   (unsigned int[]) {1, 1, 1, 1, 1, 1, 1, },
   (unsigned int[]) {1, 2, 2, 2, 1, 2, 2, },
   (unsigned int[]) {2, 3, 3, 3, 2, 3, 3, },
   (unsigned int[]) {2, 2, 2, 2, 2, 2, 2, },
};

static inline int reg_get_class(int virt_reg)
{
   switch (reg_get_type(virt_reg)) {
   case REG_TYPE_VEC4:
      return REG_CLASS_VEC4;
   case REG_TYPE_VIRT_VEC3_XYZ:
   case REG_TYPE_VIRT_VEC3_XYW:
   case REG_TYPE_VIRT_VEC3_XZW:
   case REG_TYPE_VIRT_VEC3_YZW:
      return REG_CLASS_VIRT_VEC3;
   case REG_TYPE_VIRT_VEC2_XY:
   case REG_TYPE_VIRT_VEC2_XZ:
   case REG_TYPE_VIRT_VEC2_XW:
   case REG_TYPE_VIRT_VEC2_YZ:
   case REG_TYPE_VIRT_VEC2_YW:
   case REG_TYPE_VIRT_VEC2_ZW:
      return REG_CLASS_VIRT_VEC2;
   case REG_TYPE_VIRT_SCALAR_X:
   case REG_TYPE_VIRT_SCALAR_Y:
   case REG_TYPE_VIRT_SCALAR_Z:
   case REG_TYPE_VIRT_SCALAR_W:
      return REG_CLASS_VIRT_SCALAR;
   case REG_TYPE_VIRT_VEC2T_XY:
   case REG_TYPE_VIRT_VEC2T_ZW:
      return REG_CLASS_VIRT_VEC2T;
   case REG_TYPE_VIRT_VEC2C_XY:
   case REG_TYPE_VIRT_VEC2C_YZ:
   case REG_TYPE_VIRT_VEC2C_ZW:
      return REG_CLASS_VIRT_VEC2C;
   case REG_TYPE_VIRT_VEC3C_XYZ:
   case REG_TYPE_VIRT_VEC3C_YZW:
      return REG_CLASS_VIRT_VEC3C;
   }

   assert(false);
   return 0;
}

struct ra_regs *
etna_ra_setup(void *mem_ctx)
{
   struct ra_regs *regs = ra_alloc_reg_set(mem_ctx, ETNA_MAX_TEMPS *
                  NUM_REG_TYPES, false);

   /* classes always be created from index 0, so equal to the class enum
    * which represents a register with (c+1) components
    */
   struct ra_class *classes[NUM_REG_CLASSES];
   for (int c = 0; c < NUM_REG_CLASSES; c++)
      classes[c] = ra_alloc_reg_class(regs);
   /* add each register of each class */
   for (int r = 0; r < NUM_REG_TYPES * ETNA_MAX_TEMPS; r++)
      ra_class_add_reg(classes[reg_get_class(r)], r);
   /* set conflicts */
   for (int r = 0; r < ETNA_MAX_TEMPS; r++) {
      for (int i = 0; i < NUM_REG_TYPES; i++) {
         for (int j = 0; j < i; j++) {
            if (reg_writemask[i] & reg_writemask[j]) {
               ra_add_reg_conflict(regs, NUM_REG_TYPES * r + i,
                                         NUM_REG_TYPES * r + j);
            }
         }
      }
   }
   ra_set_finalize(regs, q_values);

   return regs;
}

void
etna_ra_assign(struct etna_compile *c, nir_shader *shader)
{
   struct etna_compiler *compiler = c->variant->shader->compiler;
   struct ra_regs *regs = compiler->regs;

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);

   /* liveness and interference */

   nir_index_blocks(impl);
   nir_index_ssa_defs(impl);

   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            instr->pass_flags &= ~PASS_FLAGS_IS_DEAD_MASK;
         }
      }
   }

   /* this gives an approximation/upper limit on how many nodes are needed
    * (some ssa values do not represent an allocated register)
    */
   unsigned max_nodes = impl->ssa_alloc;
   unsigned *live_map = ralloc_array(NULL, unsigned, max_nodes);
   memset(live_map, 0xff, sizeof(unsigned) * max_nodes);
   struct live_def *defs = rzalloc_array(NULL, struct live_def, max_nodes);

   unsigned num_nodes = etna_live_defs(impl, defs, live_map);
   struct ra_graph *g = ra_alloc_interference_graph(regs, num_nodes);

   /* set classes from num_components */
   for (unsigned i = 0; i < num_nodes; i++) {
      nir_instr *instr = defs[i].instr;
      nir_def *def = defs[i].def;
      unsigned comp = def->num_components - 1;

      if (instr->type == nir_instr_type_alu &&
          c->specs->has_new_transcendentals) {
         switch (nir_instr_as_alu(instr)->op) {
         case nir_op_fdiv:
         case nir_op_flog2:
         case nir_op_fsin:
         case nir_op_fcos:
            comp = REG_CLASS_VIRT_VEC2T;
            break;
         default:
            break;
         }
      }

      if (instr->type == nir_instr_type_intrinsic) {
         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         /* can't have dst swizzle or sparse writemask on UBO loads */
         if (intr->intrinsic == nir_intrinsic_load_ubo) {
            assert(def == &intr->def);
            if (def->num_components == 2)
               comp = REG_CLASS_VIRT_VEC2C;
            if (def->num_components == 3)
               comp = REG_CLASS_VIRT_VEC3C;
         }
      }

      ra_set_node_class(g, i, ra_get_class_from_index(regs, comp));
   }

   nir_foreach_block(block, impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_def *def = def_for_instr(instr);
         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         unsigned reg;

         switch (intr->intrinsic) {
         case nir_intrinsic_store_deref: {
            /* don't want outputs to be swizzled
             * TODO: better would be to set the type to X/XY/XYZ/XYZW
             * TODO: what if fragcoord.z is read after writing fragdepth?
             */
            nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
            unsigned index = live_map[src_index(impl, &intr->src[1])];

            if (shader->info.stage == MESA_SHADER_FRAGMENT &&
                deref->var->data.location == FRAG_RESULT_DEPTH) {
               ra_set_node_reg(g, index, REG_FRAG_DEPTH);
            } else {
               ra_set_node_class(g, index, ra_get_class_from_index(regs, REG_CLASS_VEC4));
            }
         } continue;
         case nir_intrinsic_load_input:
            reg = nir_intrinsic_base(intr) * NUM_REG_TYPES + (unsigned[]) {
               REG_TYPE_VIRT_SCALAR_X,
               REG_TYPE_VIRT_VEC2_XY,
               REG_TYPE_VIRT_VEC3_XYZ,
               REG_TYPE_VEC4,
            }[def->num_components - 1];
            break;
         case nir_intrinsic_load_instance_id:
            reg = c->variant->infile.num_reg * NUM_REG_TYPES + REG_TYPE_VIRT_SCALAR_Y;
            break;
         default:
            continue;
         }

         ra_set_node_reg(g, live_map[def_index(impl, def)], reg);
      }
   }

   /* add interference for intersecting live ranges */
   for (unsigned i = 0; i < num_nodes; i++) {
      assert(defs[i].live_start < defs[i].live_end);
      for (unsigned j = 0; j < i; j++) {
         if (defs[i].live_start >= defs[j].live_end || defs[j].live_start >= defs[i].live_end)
            continue;
         ra_add_node_interference(g, i, j);
      }
   }

   ralloc_free(defs);

   /* Allocate registers */
   ASSERTED bool ok = ra_allocate(g);
   assert(ok);

   c->g = g;
   c->live_map = live_map;
   c->num_nodes = num_nodes;
}

unsigned
etna_ra_finish(struct etna_compile *c)
{
   /* TODO: better way to get number of registers used? */
   unsigned j = 0;
   for (unsigned i = 0; i < c->num_nodes; i++) {
      j = MAX2(j, reg_get_base(c, ra_get_node_reg(c->g, i)) + 1);
   }

   ralloc_free(c->g);
   ralloc_free(c->live_map);

   return j;
}
