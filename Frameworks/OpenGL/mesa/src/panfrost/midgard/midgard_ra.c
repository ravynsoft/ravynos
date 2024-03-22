/*
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019 Collabora, Ltd.
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

#include "util/u_math.h"
#include "util/u_memory.h"
#include "compiler.h"
#include "midgard_ops.h"
#include "midgard_quirks.h"

struct phys_reg {
   /* Physical register: 0-31 */
   unsigned reg;

   /* Byte offset into the physical register: 0-15 */
   unsigned offset;

   /* log2(bytes per component) for fast mul/div */
   unsigned shift;
};

/* Shift up by reg_offset and horizontally by dst_offset. */

static void
offset_swizzle(unsigned *swizzle, unsigned reg_offset, unsigned srcshift,
               unsigned dstshift, unsigned dst_offset)
{
   unsigned out[MIR_VEC_COMPONENTS];

   signed reg_comp = reg_offset >> srcshift;
   signed dst_comp = dst_offset >> dstshift;

   unsigned max_component = (16 >> srcshift) - 1;

   assert(reg_comp << srcshift == reg_offset);
   assert(dst_comp << dstshift == dst_offset);

   for (signed c = 0; c < MIR_VEC_COMPONENTS; ++c) {
      signed comp = MAX2(c - dst_comp, 0);
      out[c] = MIN2(swizzle[comp] + reg_comp, max_component);
   }

   memcpy(swizzle, out, sizeof(out));
}

/* Helper to return the default phys_reg for a given register */

static struct phys_reg
default_phys_reg(int reg, unsigned shift)
{
   struct phys_reg r = {
      .reg = reg,
      .offset = 0,
      .shift = shift,
   };

   return r;
}

/* Determine which physical register, swizzle, and mask a virtual
 * register corresponds to */

static struct phys_reg
index_to_reg(compiler_context *ctx, struct lcra_state *l, unsigned reg,
             unsigned shift)
{
   /* Check for special cases */
   if (reg == ~0)
      return default_phys_reg(REGISTER_UNUSED, shift);
   else if (reg >= SSA_FIXED_MINIMUM)
      return default_phys_reg(SSA_REG_FROM_FIXED(reg), shift);
   else if (!l)
      return default_phys_reg(REGISTER_UNUSED, shift);

   struct phys_reg r = {
      .reg = l->solutions[reg] / 16,
      .offset = l->solutions[reg] & 0xF,
      .shift = shift,
   };

   /* Report that we actually use this register, and return it */

   if (r.reg < 16)
      ctx->info->work_reg_count = MAX2(ctx->info->work_reg_count, r.reg + 1);

   return r;
}

static void
set_class(unsigned *classes, unsigned node, unsigned class)
{
   if (node < SSA_FIXED_MINIMUM && class != classes[node]) {
      assert(classes[node] == REG_CLASS_WORK);
      classes[node] = class;
   }
}

/* Special register classes impose special constraints on who can read their
 * values, so check that */

static bool ASSERTED
check_read_class(unsigned *classes, unsigned tag, unsigned node)
{
   /* Non-nodes are implicitly ok */
   if (node >= SSA_FIXED_MINIMUM)
      return true;

   switch (classes[node]) {
   case REG_CLASS_LDST:
      return (tag == TAG_LOAD_STORE_4);
   case REG_CLASS_TEXR:
      return (tag == TAG_TEXTURE_4);
   case REG_CLASS_TEXW:
      return (tag != TAG_LOAD_STORE_4);
   case REG_CLASS_WORK:
      return IS_ALU(tag);
   default:
      unreachable("Invalid class");
   }
}

static bool ASSERTED
check_write_class(unsigned *classes, unsigned tag, unsigned node)
{
   /* Non-nodes are implicitly ok */
   if (node >= SSA_FIXED_MINIMUM)
      return true;

   switch (classes[node]) {
   case REG_CLASS_TEXR:
      return true;
   case REG_CLASS_TEXW:
      return (tag == TAG_TEXTURE_4);
   case REG_CLASS_LDST:
   case REG_CLASS_WORK:
      return IS_ALU(tag) || (tag == TAG_LOAD_STORE_4);
   default:
      unreachable("Invalid class");
   }
}

/* Prepass before RA to ensure special class restrictions are met. The idea is
 * to create a bit field of types of instructions that read a particular index.
 * Later, we'll add moves as appropriate and rewrite to specialize by type. */

static void
mark_node_class(unsigned *bitfield, unsigned node)
{
   if (node < SSA_FIXED_MINIMUM)
      BITSET_SET(bitfield, node);
}

void
mir_lower_special_reads(compiler_context *ctx)
{
   mir_compute_temp_count(ctx);
   size_t sz = BITSET_WORDS(ctx->temp_count) * sizeof(BITSET_WORD);

   /* Bitfields for the various types of registers we could have. aluw can
    * be written by either ALU or load/store */

   unsigned *alur = calloc(sz, 1);
   unsigned *aluw = calloc(sz, 1);
   unsigned *brar = calloc(sz, 1);
   unsigned *ldst = calloc(sz, 1);
   unsigned *texr = calloc(sz, 1);
   unsigned *texw = calloc(sz, 1);

   /* Pass #1 is analysis, a linear scan to fill out the bitfields */

   mir_foreach_instr_global(ctx, ins) {
      switch (ins->type) {
      case TAG_ALU_4:
         mark_node_class(aluw, ins->dest);
         mark_node_class(alur, ins->src[0]);
         mark_node_class(alur, ins->src[1]);
         mark_node_class(alur, ins->src[2]);

         if (ins->compact_branch && ins->writeout)
            mark_node_class(brar, ins->src[0]);

         break;

      case TAG_LOAD_STORE_4:
         mark_node_class(aluw, ins->dest);
         mark_node_class(ldst, ins->src[0]);
         mark_node_class(ldst, ins->src[1]);
         mark_node_class(ldst, ins->src[2]);
         mark_node_class(ldst, ins->src[3]);
         break;

      case TAG_TEXTURE_4:
         mark_node_class(texr, ins->src[0]);
         mark_node_class(texr, ins->src[1]);
         mark_node_class(texr, ins->src[2]);
         mark_node_class(texw, ins->dest);
         break;

      default:
         break;
      }
   }

   /* Pass #2 is lowering now that we've analyzed all the classes.
    * Conceptually, if an index is only marked for a single type of use,
    * there is nothing to lower. If it is marked for different uses, we
    * split up based on the number of types of uses. To do so, we divide
    * into N distinct classes of use (where N>1 by definition), emit N-1
    * moves from the index to copies of the index, and finally rewrite N-1
    * of the types of uses to use the corresponding move */

   unsigned spill_idx = ctx->temp_count;

   for (unsigned i = 0; i < ctx->temp_count; ++i) {
      bool is_alur = BITSET_TEST(alur, i);
      bool is_aluw = BITSET_TEST(aluw, i);
      bool is_brar = BITSET_TEST(brar, i);
      bool is_ldst = BITSET_TEST(ldst, i);
      bool is_texr = BITSET_TEST(texr, i);
      bool is_texw = BITSET_TEST(texw, i);

      /* Analyse to check how many distinct uses there are. ALU ops
       * (alur) can read the results of the texture pipeline (texw)
       * but not ldst or texr. Load/store ops (ldst) cannot read
       * anything but load/store inputs. Texture pipeline cannot read
       * anything but texture inputs. TODO: Simplify.  */

      bool collision = (is_alur && (is_ldst || is_texr)) ||
                       (is_ldst && (is_alur || is_texr || is_texw)) ||
                       (is_texr && (is_alur || is_ldst || is_texw)) ||
                       (is_texw && (is_aluw || is_ldst || is_texr)) ||
                       (is_brar && is_texw);

      if (!collision)
         continue;

      /* Use the index as-is as the work copy. Emit copies for
       * special uses */

      unsigned classes[] = {TAG_LOAD_STORE_4, TAG_TEXTURE_4, TAG_TEXTURE_4,
                            TAG_ALU_4};
      bool collisions[] = {is_ldst, is_texr, is_texw && is_aluw, is_brar};

      for (unsigned j = 0; j < ARRAY_SIZE(collisions); ++j) {
         if (!collisions[j])
            continue;

         /* When the hazard is from reading, we move and rewrite
          * sources (typical case). When it's from writing, we
          * flip the move and rewrite destinations (obscure,
          * only from control flow -- impossible in SSA) */

         bool hazard_write = (j == 2);

         unsigned idx = spill_idx++;

         /* Insert move before each read/write, depending on the
          * hazard we're trying to account for */

         mir_foreach_block(ctx, block_) {
            midgard_block *block = (midgard_block *)block_;
            midgard_instruction *mov = NULL;

            mir_foreach_instr_in_block_safe(block, pre_use) {
               if (pre_use->type != classes[j])
                  continue;

               if (hazard_write) {
                  if (pre_use->dest != i)
                     continue;

                  midgard_instruction m = v_mov(idx, i);
                  m.dest_type = pre_use->dest_type;
                  m.src_types[1] = m.dest_type;
                  m.mask = pre_use->mask;

                  midgard_instruction *use = mir_next_op(pre_use);
                  assert(use);
                  mir_insert_instruction_before(ctx, use, m);
                  mir_rewrite_index_dst_single(pre_use, i, idx);
               } else {
                  if (!mir_has_arg(pre_use, i))
                     continue;

                  unsigned mask = mir_from_bytemask(
                     mir_round_bytemask_up(
                        mir_bytemask_of_read_components(pre_use, i), 32),
                     32);

                  if (mov == NULL || !mir_is_ssa(i)) {
                     midgard_instruction m = v_mov(i, spill_idx++);
                     m.mask = mask;
                     mov = mir_insert_instruction_before(ctx, pre_use, m);
                  } else {
                     mov->mask |= mask;
                  }

                  mir_rewrite_index_src_single(pre_use, i, mov->dest);
               }
            }
         }
      }
   }

   free(alur);
   free(aluw);
   free(brar);
   free(ldst);
   free(texr);
   free(texw);
}

static void
mir_compute_interference(compiler_context *ctx, struct lcra_state *l)
{
   /* First, we need liveness information to be computed per block */
   mir_compute_liveness(ctx);

   /* We need to force r1.w live throughout a blend shader */

   if (ctx->inputs->is_blend) {
      unsigned r1w = ~0;

      mir_foreach_block(ctx, _block) {
         midgard_block *block = (midgard_block *)_block;
         mir_foreach_instr_in_block_rev(block, ins) {
            if (ins->writeout)
               r1w = ins->dest;
         }

         if (r1w != ~0)
            break;
      }

      mir_foreach_instr_global(ctx, ins) {
         if (ins->dest < ctx->temp_count)
            lcra_add_node_interference(l, ins->dest, mir_bytemask(ins), r1w,
                                       0xF);
      }
   }

   /* Now that every block has live_in/live_out computed, we can determine
    * interference by walking each block linearly. Take live_out at the
    * end of each block and walk the block backwards. */

   mir_foreach_block(ctx, _blk) {
      midgard_block *blk = (midgard_block *)_blk;

      /* The scalar and vector units run in parallel. We need to make
       * sure they don't write to same portion of the register file
       * otherwise the result is undefined. Add interferences to
       * avoid this situation.
       */
      util_dynarray_foreach(&blk->bundles, midgard_bundle, bundle) {
         midgard_instruction *instrs[2][4];
         unsigned instr_count[2] = {0, 0};

         for (unsigned i = 0; i < bundle->instruction_count; i++) {
            if (bundle->instructions[i]->unit == UNIT_VMUL ||
                bundle->instructions[i]->unit == UNIT_SADD)
               instrs[0][instr_count[0]++] = bundle->instructions[i];
            else
               instrs[1][instr_count[1]++] = bundle->instructions[i];
         }

         for (unsigned i = 0; i < ARRAY_SIZE(instr_count); i++) {
            for (unsigned j = 0; j < instr_count[i]; j++) {
               midgard_instruction *ins_a = instrs[i][j];

               if (ins_a->dest >= ctx->temp_count)
                  continue;

               for (unsigned k = j + 1; k < instr_count[i]; k++) {
                  midgard_instruction *ins_b = instrs[i][k];

                  if (ins_b->dest >= ctx->temp_count)
                     continue;

                  lcra_add_node_interference(l, ins_b->dest,
                                             mir_bytemask(ins_b), ins_a->dest,
                                             mir_bytemask(ins_a));
               }
            }
         }
      }

      uint16_t *live =
         mem_dup(_blk->live_out, ctx->temp_count * sizeof(uint16_t));

      mir_foreach_instr_in_block_rev(blk, ins) {
         /* Mark all registers live after the instruction as
          * interfering with the destination */

         unsigned dest = ins->dest;

         if (dest < ctx->temp_count) {
            for (unsigned i = 0; i < ctx->temp_count; ++i) {
               if (live[i]) {
                  unsigned mask = mir_bytemask(ins);
                  lcra_add_node_interference(l, dest, mask, i, live[i]);
               }
            }
         }

         /* Add blend shader interference: blend shaders might
          * clobber r0-r3. */
         if (ins->compact_branch && ins->writeout) {
            for (unsigned i = 0; i < ctx->temp_count; ++i) {
               if (!live[i])
                  continue;

               for (unsigned j = 0; j < 4; j++) {
                  lcra_add_node_interference(l, ctx->temp_count + j, 0xFFFF, i,
                                             live[i]);
               }
            }
         }

         /* Update live_in */
         mir_liveness_ins_update(live, ins, ctx->temp_count);
      }

      free(live);
   }
}

static bool
mir_is_64(midgard_instruction *ins)
{
   if (nir_alu_type_get_type_size(ins->dest_type) == 64)
      return true;

   mir_foreach_src(ins, v) {
      if (nir_alu_type_get_type_size(ins->src_types[v]) == 64)
         return true;
   }

   return false;
}

/*
 * Determine if a shader needs a contiguous workgroup. This impacts register
 * allocation. TODO: Optimize if barriers and local memory are unused.
 */
static bool
needs_contiguous_workgroup(compiler_context *ctx)
{
   return gl_shader_stage_uses_workgroup(ctx->stage);
}

/*
 * Determine an upper-bound on the number of threads in a workgroup. The GL
 * driver reports 128 for the maximum number of threads (the minimum-maximum in
 * OpenGL ES 3.1), so we pessimistically assume 128 threads for variable
 * workgroups.
 */
static unsigned
max_threads_per_workgroup(compiler_context *ctx)
{
   if (ctx->nir->info.workgroup_size_variable) {
      return 128;
   } else {
      return ctx->nir->info.workgroup_size[0] *
             ctx->nir->info.workgroup_size[1] *
             ctx->nir->info.workgroup_size[2];
   }
}

/*
 * Calculate the maximum number of work registers available to the shader.
 * Architecturally, Midgard shaders may address up to 16 work registers, but
 * various features impose other limits:
 *
 * 1. Blend shaders are limited to 8 registers by ABI.
 * 2. If there are more than 8 register-mapped uniforms, then additional
 *    register-mapped uniforms use space that otherwise would be used for work
 *    registers.
 * 3. If more than 4 registers are used, at most 128 threads may be spawned. If
 *    more than 8 registers are used, at most 64 threads may be spawned. These
 *    limits are architecturally visible in compute kernels that require an
 *    entire workgroup to be spawned at once (for barriers or local memory to
 *    work properly).
 */
static unsigned
max_work_registers(compiler_context *ctx)
{
   if (ctx->inputs->is_blend)
      return 8;

   unsigned rmu_vec4 = ctx->info->push.count / 4;
   unsigned max_work_registers = (rmu_vec4 >= 8) ? (24 - rmu_vec4) : 16;

   if (needs_contiguous_workgroup(ctx)) {
      unsigned threads = max_threads_per_workgroup(ctx);
      assert(threads <= 128 && "maximum threads in ABI exceeded");

      if (threads > 64)
         max_work_registers = MIN2(max_work_registers, 8);
   }

   return max_work_registers;
}

/* This routine performs the actual register allocation. It should be succeeded
 * by install_registers */

static struct lcra_state *
allocate_registers(compiler_context *ctx, bool *spilled)
{
   int work_count = max_work_registers(ctx);

   /* No register allocation to do with no SSA */
   mir_compute_temp_count(ctx);
   if (!ctx->temp_count)
      return NULL;

   /* Initialize LCRA. Allocate extra node at the end for r1-r3 for
    * interference */

   struct lcra_state *l = lcra_alloc_equations(ctx->temp_count + 4, 5);
   unsigned node_r1 = ctx->temp_count + 1;

   /* Starts of classes, in bytes */
   l->class_start[REG_CLASS_WORK] = 16 * 0;
   l->class_start[REG_CLASS_LDST] = 16 * 26;
   l->class_start[REG_CLASS_TEXR] = 16 * 28;
   l->class_start[REG_CLASS_TEXW] = 16 * 28;

   l->class_size[REG_CLASS_WORK] = 16 * work_count;
   l->class_size[REG_CLASS_LDST] = 16 * 2;
   l->class_size[REG_CLASS_TEXR] = 16 * 2;
   l->class_size[REG_CLASS_TEXW] = 16 * 2;

   lcra_set_disjoint_class(l, REG_CLASS_TEXR, REG_CLASS_TEXW);

   /* To save space on T*20, we don't have real texture registers.
    * Instead, tex inputs reuse the load/store pipeline registers, and
    * tex outputs use work r0/r1. Note we still use TEXR/TEXW classes,
    * noting that this handles interferences and sizes correctly. */

   if (ctx->quirks & MIDGARD_INTERPIPE_REG_ALIASING) {
      l->class_start[REG_CLASS_TEXR] = l->class_start[REG_CLASS_LDST];
      l->class_start[REG_CLASS_TEXW] = l->class_start[REG_CLASS_WORK];
   }

   unsigned *found_class = calloc(sizeof(unsigned), ctx->temp_count);
   unsigned *min_alignment = calloc(sizeof(unsigned), ctx->temp_count);
   unsigned *min_bound = calloc(sizeof(unsigned), ctx->temp_count);

   mir_foreach_instr_global(ctx, ins) {
      /* Swizzles of 32-bit sources on 64-bit instructions need to be
       * aligned to either bottom (xy) or top (zw). More general
       * swizzle lowering should happen prior to scheduling (TODO),
       * but once we get RA we shouldn't disrupt this further. Align
       * sources of 64-bit instructions. */

      if (ins->type == TAG_ALU_4 && mir_is_64(ins)) {
         mir_foreach_src(ins, v) {
            unsigned s = ins->src[v];

            if (s < ctx->temp_count)
               min_alignment[s] = MAX2(3, min_alignment[s]);
         }
      }

      if (ins->type == TAG_LOAD_STORE_4 && OP_HAS_ADDRESS(ins->op)) {
         mir_foreach_src(ins, v) {
            unsigned s = ins->src[v];
            unsigned size = nir_alu_type_get_type_size(ins->src_types[v]);

            if (s < ctx->temp_count)
               min_alignment[s] = MAX2((size == 64) ? 3 : 2, min_alignment[s]);
         }
      }

      /* Anything read as 16-bit needs proper alignment to ensure the
       * resulting code can be packed.
       */
      mir_foreach_src(ins, s) {
         unsigned src_size = nir_alu_type_get_type_size(ins->src_types[s]);
         if (src_size == 16 && ins->src[s] < SSA_FIXED_MINIMUM)
            min_bound[ins->src[s]] = MAX2(min_bound[ins->src[s]], 8);
      }

      /* Everything after this concerns only the destination, not the
       * sources.
       */
      if (ins->dest >= SSA_FIXED_MINIMUM)
         continue;

      unsigned size = nir_alu_type_get_type_size(ins->dest_type);

      if (ins->is_pack)
         size = 32;

      /* 0 for x, 1 for xy, 2 for xyz, 3 for xyzw */
      int comps1 = util_logbase2(ins->mask);

      int bytes = (comps1 + 1) * (size / 8);

      /* Use the largest class if there's ambiguity, this
       * handles partial writes */

      int dest = ins->dest;
      found_class[dest] = MAX2(found_class[dest], bytes);

      min_alignment[dest] =
         MAX2(min_alignment[dest], (size == 16) ? 1 : /* (1 << 1) = 2-byte */
                                      (size == 32) ? 2
                                                   : /* (1 << 2) = 4-byte */
                                      (size == 64) ? 3
                                                   : /* (1 << 3) = 8-byte */
                                      3);            /* 8-bit todo */

      /* We can't cross xy/zw boundaries. TODO: vec8 can */
      if (size == 16 && min_alignment[dest] != 4)
         min_bound[dest] = 8;

      /* We don't have a swizzle for the conditional and we don't
       * want to muck with the conditional itself, so just force
       * alignment for now */

      if (ins->type == TAG_ALU_4 && OP_IS_CSEL_V(ins->op)) {
         min_alignment[dest] = 4; /* 1 << 4= 16-byte = vec4 */

         /* LCRA assumes bound >= alignment */
         min_bound[dest] = 16;
      }

      /* Since ld/st swizzles and masks are 32-bit only, we need them
       * aligned to enable final packing */
      if (ins->type == TAG_LOAD_STORE_4)
         min_alignment[dest] = MAX2(min_alignment[dest], 2);
   }

   for (unsigned i = 0; i < ctx->temp_count; ++i) {
      lcra_set_alignment(l, i, min_alignment[i] ? min_alignment[i] : 2,
                         min_bound[i] ? min_bound[i] : 16);
      lcra_restrict_range(l, i, found_class[i]);
   }

   free(found_class);
   free(min_alignment);
   free(min_bound);

   /* Next, we'll determine semantic class. We default to zero (work).
    * But, if we're used with a special operation, that will force us to a
    * particular class. Each node must be assigned to exactly one class; a
    * prepass before RA should have lowered what-would-have-been
    * multiclass nodes into a series of moves to break it up into multiple
    * nodes (TODO) */

   mir_foreach_instr_global(ctx, ins) {
      /* Check if this operation imposes any classes */

      if (ins->type == TAG_LOAD_STORE_4) {
         set_class(l->class, ins->src[0], REG_CLASS_LDST);
         set_class(l->class, ins->src[1], REG_CLASS_LDST);
         set_class(l->class, ins->src[2], REG_CLASS_LDST);
         set_class(l->class, ins->src[3], REG_CLASS_LDST);

         if (OP_IS_VEC4_ONLY(ins->op)) {
            lcra_restrict_range(l, ins->dest, 16);
            lcra_restrict_range(l, ins->src[0], 16);
            lcra_restrict_range(l, ins->src[1], 16);
            lcra_restrict_range(l, ins->src[2], 16);
            lcra_restrict_range(l, ins->src[3], 16);
         }
      } else if (ins->type == TAG_TEXTURE_4) {
         set_class(l->class, ins->dest, REG_CLASS_TEXW);
         set_class(l->class, ins->src[0], REG_CLASS_TEXR);
         set_class(l->class, ins->src[1], REG_CLASS_TEXR);
         set_class(l->class, ins->src[2], REG_CLASS_TEXR);
         set_class(l->class, ins->src[3], REG_CLASS_TEXR);
      }
   }

   /* Check that the semantics of the class are respected */
   mir_foreach_instr_global(ctx, ins) {
      assert(check_write_class(l->class, ins->type, ins->dest));
      assert(check_read_class(l->class, ins->type, ins->src[0]));
      assert(check_read_class(l->class, ins->type, ins->src[1]));
      assert(check_read_class(l->class, ins->type, ins->src[2]));
      assert(check_read_class(l->class, ins->type, ins->src[3]));
   }

   /* Mark writeout to r0, depth to r1.x, stencil to r1.y,
    * render target to r1.z, unknown to r1.w */
   mir_foreach_instr_global(ctx, ins) {
      if (!(ins->compact_branch && ins->writeout))
         continue;

      if (ins->src[0] < ctx->temp_count)
         l->solutions[ins->src[0]] = 0;

      if (ins->src[2] < ctx->temp_count)
         l->solutions[ins->src[2]] = (16 * 1) + COMPONENT_X * 4;

      if (ins->src[3] < ctx->temp_count)
         l->solutions[ins->src[3]] = (16 * 1) + COMPONENT_Y * 4;

      if (ins->src[1] < ctx->temp_count)
         l->solutions[ins->src[1]] = (16 * 1) + COMPONENT_Z * 4;

      if (ins->dest < ctx->temp_count)
         l->solutions[ins->dest] = (16 * 1) + COMPONENT_W * 4;
   }

   /* Destinations of instructions in a writeout block cannot be assigned
    * to r1 unless they are actually used as r1 from the writeout itself,
    * since the writes to r1 are special. A code sequence like:
    *
    *      sadd.fmov r1.x, [...]
    *      vadd.fadd r0, r1, r2
    *      [writeout branch]
    *
    * will misbehave since the r1.x write will be interpreted as a
    * gl_FragDepth write so it won't show up correctly when r1 is read in
    * the following segment. We model this as interference.
    */

   for (unsigned i = 0; i < 4; ++i)
      l->solutions[ctx->temp_count + i] = (16 * i);

   mir_foreach_block(ctx, _blk) {
      midgard_block *blk = (midgard_block *)_blk;

      mir_foreach_bundle_in_block(blk, v) {
         /* We need at least a writeout and nonwriteout instruction */
         if (v->instruction_count < 2)
            continue;

         /* Branches always come at the end */
         midgard_instruction *br = v->instructions[v->instruction_count - 1];

         if (!br->writeout)
            continue;

         for (signed i = v->instruction_count - 2; i >= 0; --i) {
            midgard_instruction *ins = v->instructions[i];

            if (ins->dest >= ctx->temp_count)
               continue;

            bool used_as_r1 = (br->dest == ins->dest);

            mir_foreach_src(br, s)
               used_as_r1 |= (s > 0) && (br->src[s] == ins->dest);

            if (!used_as_r1)
               lcra_add_node_interference(l, ins->dest, mir_bytemask(ins),
                                          node_r1, 0xFFFF);
         }
      }
   }

   /* Precolour blend input to r0. Note writeout is necessarily at the end
    * and blend shaders are single-RT only so there is only a single
    * writeout block, so this cannot conflict with the writeout r0 (there
    * is no need to have an intermediate move) */

   if (ctx->blend_input != ~0) {
      assert(ctx->blend_input < ctx->temp_count);
      l->solutions[ctx->blend_input] = 0;
   }

   /* Same for the dual-source blend input/output, except here we use r2,
    * which is also set in the fragment shader. */

   if (ctx->blend_src1 != ~0) {
      assert(ctx->blend_src1 < ctx->temp_count);
      l->solutions[ctx->blend_src1] = (16 * 2);
      ctx->info->work_reg_count = MAX2(ctx->info->work_reg_count, 3);
   }

   mir_compute_interference(ctx, l);

   *spilled = !lcra_solve(l);
   return l;
}

/* Once registers have been decided via register allocation
 * (allocate_registers), we need to rewrite the MIR to use registers instead of
 * indices */

static void
install_registers_instr(compiler_context *ctx, struct lcra_state *l,
                        midgard_instruction *ins)
{
   unsigned src_shift[MIR_SRC_COUNT];

   for (unsigned i = 0; i < MIR_SRC_COUNT; ++i) {
      src_shift[i] =
         util_logbase2(nir_alu_type_get_type_size(ins->src_types[i]) / 8);
   }

   unsigned dest_shift =
      util_logbase2(nir_alu_type_get_type_size(ins->dest_type) / 8);

   switch (ins->type) {
   case TAG_ALU_4:
   case TAG_ALU_8:
   case TAG_ALU_12:
   case TAG_ALU_16: {
      if (ins->compact_branch)
         return;

      struct phys_reg src1 = index_to_reg(ctx, l, ins->src[0], src_shift[0]);
      struct phys_reg src2 = index_to_reg(ctx, l, ins->src[1], src_shift[1]);
      struct phys_reg dest = index_to_reg(ctx, l, ins->dest, dest_shift);

      mir_set_bytemask(ins, mir_bytemask(ins) << dest.offset);

      unsigned dest_offset =
         GET_CHANNEL_COUNT(alu_opcode_props[ins->op].props) ? 0 : dest.offset;

      offset_swizzle(ins->swizzle[0], src1.offset, src1.shift, dest.shift,
                     dest_offset);
      if (!ins->has_inline_constant)
         offset_swizzle(ins->swizzle[1], src2.offset, src2.shift, dest.shift,
                        dest_offset);
      if (ins->src[0] != ~0)
         ins->src[0] = SSA_FIXED_REGISTER(src1.reg);
      if (ins->src[1] != ~0)
         ins->src[1] = SSA_FIXED_REGISTER(src2.reg);
      if (ins->dest != ~0)
         ins->dest = SSA_FIXED_REGISTER(dest.reg);
      break;
   }

   case TAG_LOAD_STORE_4: {
      /* Which physical register we read off depends on
       * whether we are loading or storing -- think about the
       * logical dataflow */

      bool encodes_src = OP_IS_STORE(ins->op);

      if (encodes_src) {
         struct phys_reg src = index_to_reg(ctx, l, ins->src[0], src_shift[0]);
         assert(src.reg == 26 || src.reg == 27);

         ins->src[0] = SSA_FIXED_REGISTER(src.reg);
         offset_swizzle(ins->swizzle[0], src.offset, src.shift, 0, 0);
      } else {
         struct phys_reg dst = index_to_reg(ctx, l, ins->dest, dest_shift);

         ins->dest = SSA_FIXED_REGISTER(dst.reg);
         offset_swizzle(ins->swizzle[0], 0, 2, dest_shift, dst.offset);
         mir_set_bytemask(ins, mir_bytemask(ins) << dst.offset);
      }

      /* We also follow up by actual arguments */

      for (int i = 1; i <= 3; i++) {
         unsigned src_index = ins->src[i];
         if (src_index != ~0) {
            struct phys_reg src = index_to_reg(ctx, l, src_index, src_shift[i]);
            unsigned component = src.offset >> src.shift;
            assert(component << src.shift == src.offset);
            ins->src[i] = SSA_FIXED_REGISTER(src.reg);
            ins->swizzle[i][0] += component;
         }
      }

      break;
   }

   case TAG_TEXTURE_4: {
      if (ins->op == midgard_tex_op_barrier)
         break;

      /* Grab RA results */
      struct phys_reg dest = index_to_reg(ctx, l, ins->dest, dest_shift);
      struct phys_reg coord = index_to_reg(ctx, l, ins->src[1], src_shift[1]);
      struct phys_reg lod = index_to_reg(ctx, l, ins->src[2], src_shift[2]);
      struct phys_reg offset = index_to_reg(ctx, l, ins->src[3], src_shift[3]);

      /* First, install the texture coordinate */
      if (ins->src[1] != ~0)
         ins->src[1] = SSA_FIXED_REGISTER(coord.reg);
      offset_swizzle(ins->swizzle[1], coord.offset, coord.shift, dest.shift, 0);

      /* Next, install the destination */
      if (ins->dest != ~0)
         ins->dest = SSA_FIXED_REGISTER(dest.reg);
      offset_swizzle(ins->swizzle[0], 0, 2, dest.shift,
                     dest_shift == 1 ? dest.offset % 8 : dest.offset);
      mir_set_bytemask(ins, mir_bytemask(ins) << dest.offset);

      /* If there is a register LOD/bias, use it */
      if (ins->src[2] != ~0) {
         assert(!(lod.offset & 3));
         ins->src[2] = SSA_FIXED_REGISTER(lod.reg);
         ins->swizzle[2][0] = lod.offset / 4;
      }

      /* If there is an offset register, install it */
      if (ins->src[3] != ~0) {
         ins->src[3] = SSA_FIXED_REGISTER(offset.reg);
         ins->swizzle[3][0] = offset.offset / 4;
      }

      break;
   }

   default:
      break;
   }
}

static void
install_registers(compiler_context *ctx, struct lcra_state *l)
{
   mir_foreach_instr_global(ctx, ins)
      install_registers_instr(ctx, l, ins);
}

/* If register allocation fails, find the best spill node */

static signed
mir_choose_spill_node(compiler_context *ctx, struct lcra_state *l)
{
   /* We can't spill a previously spilled value or an unspill */

   mir_foreach_instr_global(ctx, ins) {
      if (ins->no_spill & (1 << l->spill_class)) {
         lcra_set_node_spill_cost(l, ins->dest, -1);

         if (l->spill_class != REG_CLASS_WORK) {
            mir_foreach_src(ins, s)
               lcra_set_node_spill_cost(l, ins->src[s], -1);
         }
      }
   }

   return lcra_get_best_spill_node(l);
}

/* Once we've chosen a spill node, spill it */

static void
mir_spill_register(compiler_context *ctx, unsigned spill_node,
                   unsigned spill_class, unsigned *spill_count)
{
   if (spill_class == REG_CLASS_WORK && ctx->inputs->is_blend)
      unreachable("Blend shader spilling is currently unimplemented");

   unsigned spill_index = ctx->temp_count;

   /* We have a spill node, so check the class. Work registers
    * legitimately spill to TLS, but special registers just spill to work
    * registers */

   bool is_special = spill_class != REG_CLASS_WORK;
   bool is_special_w = spill_class == REG_CLASS_TEXW;

   /* Allocate TLS slot (maybe) */
   unsigned spill_slot = !is_special ? (*spill_count)++ : 0;

   /* For special reads, figure out how many bytes we need */
   unsigned read_bytemask = 0;

   /* If multiple instructions write to this destination, we'll have to
    * fill from TLS before writing */
   unsigned write_count = 0;

   mir_foreach_instr_global_safe(ctx, ins) {
      read_bytemask |= mir_bytemask_of_read_components(ins, spill_node);
      if (ins->dest == spill_node)
         ++write_count;
   }

   /* For TLS, replace all stores to the spilled node. For
    * special reads, just keep as-is; the class will be demoted
    * implicitly. For special writes, spill to a work register */

   if (!is_special || is_special_w) {
      if (is_special_w)
         spill_slot = spill_index++;

      unsigned last_id = ~0;
      unsigned last_fill = ~0;
      unsigned last_spill_index = ~0;
      midgard_instruction *last_spill = NULL;

      mir_foreach_block(ctx, _block) {
         midgard_block *block = (midgard_block *)_block;
         mir_foreach_instr_in_block_safe(block, ins) {
            if (ins->dest != spill_node)
               continue;

            /* Note: it's important to match the mask of the spill
             * with the mask of the instruction whose destination
             * we're spilling, or otherwise we'll read invalid
             * components and can fail RA in a subsequent iteration
             */

            if (is_special_w) {
               midgard_instruction st = v_mov(spill_node, spill_slot);
               st.no_spill |= (1 << spill_class);
               st.mask = ins->mask;
               st.dest_type = st.src_types[1] = ins->dest_type;

               /* Hint: don't rewrite this node */
               st.hint = true;

               mir_insert_instruction_after_scheduled(ctx, block, ins, st);
            } else {
               unsigned bundle = ins->bundle_id;
               unsigned dest =
                  (bundle == last_id) ? last_spill_index : spill_index++;

               unsigned bytemask = mir_bytemask(ins);
               unsigned write_mask =
                  mir_from_bytemask(mir_round_bytemask_up(bytemask, 32), 32);

               if (write_count > 1 && bytemask != 0xFFFF &&
                   bundle != last_fill) {
                  midgard_instruction read =
                     v_load_store_scratch(dest, spill_slot, false, 0xF);
                  mir_insert_instruction_before_scheduled(ctx, block, ins,
                                                          read);
                  write_mask = 0xF;
                  last_fill = bundle;
               }

               ins->dest = dest;
               ins->no_spill |= (1 << spill_class);

               bool move = false;

               /* In the same bundle, reads of the destination
                * of the spilt instruction need to be direct */
               midgard_instruction *it = ins;
               while ((it = list_first_entry(&it->link, midgard_instruction,
                                             link)) &&
                      (it->bundle_id == bundle)) {

                  if (!mir_has_arg(it, spill_node))
                     continue;

                  mir_rewrite_index_src_single(it, spill_node, dest);

                  /* The spilt instruction will write to
                   * a work register for `it` to read but
                   * the spill needs an LD/ST register */
                  move = true;
               }

               if (move)
                  dest = spill_index++;

               if (last_id == bundle) {
                  last_spill->mask |= write_mask;
                  u_foreach_bit(c, write_mask)
                     last_spill->swizzle[0][c] = c;
               } else {
                  midgard_instruction st =
                     v_load_store_scratch(dest, spill_slot, true, write_mask);
                  last_spill = mir_insert_instruction_after_scheduled(
                     ctx, block, ins, st);
               }

               if (move) {
                  midgard_instruction mv = v_mov(ins->dest, dest);
                  mv.no_spill |= (1 << spill_class);

                  mir_insert_instruction_after_scheduled(ctx, block, ins, mv);
               }

               last_id = bundle;
               last_spill_index = ins->dest;
            }

            if (!is_special)
               ctx->spills++;
         }
      }
   }

   /* Insert a load from TLS before the first consecutive
    * use of the node, rewriting to use spilled indices to
    * break up the live range. Or, for special, insert a
    * move. Ironically the latter *increases* register
    * pressure, but the two uses of the spilling mechanism
    * are somewhat orthogonal. (special spilling is to use
    * work registers to back special registers; TLS
    * spilling is to use memory to back work registers) */

   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      mir_foreach_instr_in_block(block, ins) {
         /* We can't rewrite the moves used to spill in the
          * first place. These moves are hinted. */
         if (ins->hint)
            continue;

         /* If we don't use the spilled value, nothing to do */
         if (!mir_has_arg(ins, spill_node))
            continue;

         unsigned index = 0;

         if (!is_special_w) {
            index = ++spill_index;

            midgard_instruction *before = ins;
            midgard_instruction st;

            if (is_special) {
               /* Move */
               st = v_mov(spill_node, index);
               st.no_spill |= (1 << spill_class);
            } else {
               /* TLS load */
               st = v_load_store_scratch(index, spill_slot, false, 0xF);
            }

            /* Mask the load based on the component count
             * actually needed to prevent RA loops */

            st.mask =
               mir_from_bytemask(mir_round_bytemask_up(read_bytemask, 32), 32);

            mir_insert_instruction_before_scheduled(ctx, block, before, st);
         } else {
            /* Special writes already have their move spilled in */
            index = spill_slot;
         }

         /* Rewrite to use */
         mir_rewrite_index_src_single(ins, spill_node, index);

         if (!is_special)
            ctx->fills++;
      }
   }

   /* Reset hints */

   mir_foreach_instr_global(ctx, ins) {
      ins->hint = false;
   }
}

static void
mir_demote_uniforms(compiler_context *ctx, unsigned new_cutoff)
{
   unsigned uniforms = ctx->info->push.count / 4;
   unsigned old_work_count = 16 - MAX2(uniforms - 8, 0);
   unsigned work_count = 16 - MAX2((new_cutoff - 8), 0);

   unsigned min_demote = SSA_FIXED_REGISTER(old_work_count);
   unsigned max_demote = SSA_FIXED_REGISTER(work_count);

   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      mir_foreach_instr_in_block(block, ins) {
         mir_foreach_src(ins, i) {
            if (ins->src[i] < min_demote || ins->src[i] >= max_demote)
               continue;

            midgard_instruction *before = ins;

            unsigned temp = make_compiler_temp(ctx);
            unsigned idx = (23 - SSA_REG_FROM_FIXED(ins->src[i])) * 4;
            assert(idx < ctx->info->push.count);

            ctx->ubo_mask |= BITSET_BIT(ctx->info->push.words[idx].ubo);

            midgard_instruction ld = {
               .type = TAG_LOAD_STORE_4,
               .mask = 0xF,
               .dest = temp,
               .dest_type = ins->src_types[i],
               .src = {~0, ~0, ~0, ~0},
               .swizzle = SWIZZLE_IDENTITY_4,
               .op = midgard_op_ld_ubo_128,
               .load_store =
                  {
                     .index_reg = REGISTER_LDST_ZERO,
                  },
               .constants.u32[0] = ctx->info->push.words[idx].offset,
            };

            midgard_pack_ubo_index_imm(&ld.load_store,
                                       ctx->info->push.words[idx].ubo);

            mir_insert_instruction_before_scheduled(ctx, block, before, ld);

            mir_rewrite_index_src_single(ins, ins->src[i], temp);
         }
      }
   }

   ctx->info->push.count = MIN2(ctx->info->push.count, new_cutoff * 4);
}

/* Run register allocation in a loop, spilling until we succeed */

void
mir_ra(compiler_context *ctx)
{
   struct lcra_state *l = NULL;
   bool spilled = false;
   int iter_count = 1000; /* max iterations */

   /* Number of 128-bit slots in memory we've spilled into */
   unsigned spill_count = DIV_ROUND_UP(ctx->info->tls_size, 16);

   mir_create_pipeline_registers(ctx);

   do {
      if (spilled) {
         signed spill_node = mir_choose_spill_node(ctx, l);
         unsigned uniforms = ctx->info->push.count / 4;

         /* It's a lot cheaper to demote uniforms to get more
          * work registers than to spill to TLS. */
         if (l->spill_class == REG_CLASS_WORK && uniforms > 8) {
            mir_demote_uniforms(ctx, MAX2(uniforms - 4, 8));
         } else if (spill_node == -1) {
            fprintf(stderr, "ERROR: Failed to choose spill node\n");
            lcra_free(l);
            return;
         } else {
            mir_spill_register(ctx, spill_node, l->spill_class, &spill_count);
         }
      }

      mir_squeeze_index(ctx);
      mir_invalidate_liveness(ctx);

      if (l) {
         lcra_free(l);
         l = NULL;
      }

      l = allocate_registers(ctx, &spilled);
   } while (spilled && ((iter_count--) > 0));

   if (iter_count <= 0) {
      fprintf(
         stderr,
         "panfrost: Gave up allocating registers, rendering will be incomplete\n");
      assert(0);
   }

   /* Report spilling information. spill_count is in 128-bit slots (vec4 x
    * fp32), but tls_size is in bytes, so multiply by 16 */

   ctx->info->tls_size = spill_count * 16;

   install_registers(ctx, l);

   lcra_free(l);
}
