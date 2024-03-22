/*
 * Copyright © 2012 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include "brw_fs.h"
#include "brw_fs_live_variables.h"

using namespace brw;

#define MAX_INSTRUCTION (1 << 30)

/** @file brw_fs_live_variables.cpp
 *
 * Support for calculating liveness information about virtual GRFs.
 *
 * This produces a live interval for each whole virtual GRF.  We could
 * choose to expose per-component live intervals for VGRFs of size > 1,
 * but we currently do not.  It is easier for the consumers of this
 * information to work with whole VGRFs.
 *
 * However, we internally track use/def information at the per-GRF level for
 * greater accuracy.  Large VGRFs may be accessed piecemeal over many
 * (possibly non-adjacent) instructions.  In this case, examining a single
 * instruction is insufficient to decide whether a whole VGRF is ultimately
 * used or defined.  Tracking individual components allows us to easily
 * assemble this information.
 *
 * See Muchnick's Advanced Compiler Design and Implementation, section
 * 14.1 (p444).
 */

void
fs_live_variables::setup_one_read(struct block_data *bd,
                                  int ip, const fs_reg &reg)
{
   int var = var_from_reg(reg);
   assert(var < num_vars);

   start[var] = MIN2(start[var], ip);
   end[var] = MAX2(end[var], ip);

   /* The use[] bitset marks when the block makes use of a variable (VGRF
    * channel) without having completely defined that variable within the
    * block.
    */
   if (!BITSET_TEST(bd->def, var))
      BITSET_SET(bd->use, var);
}

void
fs_live_variables::setup_one_write(struct block_data *bd, fs_inst *inst,
                                   int ip, const fs_reg &reg)
{
   int var = var_from_reg(reg);
   assert(var < num_vars);

   start[var] = MIN2(start[var], ip);
   end[var] = MAX2(end[var], ip);

   /* The def[] bitset marks when an initialization in a block completely
    * screens off previous updates of that variable (VGRF channel).
    */
   if (inst->dst.file == VGRF) {
      if (!inst->is_partial_write() && !BITSET_TEST(bd->use, var))
         BITSET_SET(bd->def, var);

      BITSET_SET(bd->defout, var);
   }
}

/**
 * Sets up the use[] and def[] bitsets.
 *
 * The basic-block-level live variable analysis needs to know which
 * variables get used before they're completely defined, and which
 * variables are completely defined before they're used.
 *
 * These are tracked at the per-component level, rather than whole VGRFs.
 */
void
fs_live_variables::setup_def_use()
{
   int ip = 0;

   foreach_block (block, cfg) {
      assert(ip == block->start_ip);
      if (block->num > 0)
         assert(cfg->blocks[block->num - 1]->end_ip == ip - 1);

      struct block_data *bd = &block_data[block->num];

      foreach_inst_in_block(fs_inst, inst, block) {
         /* Set use[] for this instruction */
         for (unsigned int i = 0; i < inst->sources; i++) {
            fs_reg reg = inst->src[i];

            if (reg.file != VGRF)
               continue;

            for (unsigned j = 0; j < regs_read(inst, i); j++) {
               setup_one_read(bd, ip, reg);
               reg.offset += REG_SIZE;
            }
         }

         bd->flag_use[0] |= inst->flags_read(devinfo) & ~bd->flag_def[0];

         /* Set def[] for this instruction */
         if (inst->dst.file == VGRF) {
            fs_reg reg = inst->dst;
            for (unsigned j = 0; j < regs_written(inst); j++) {
               setup_one_write(bd, inst, ip, reg);
               reg.offset += REG_SIZE;
            }
         }

         if (!inst->predicate && inst->exec_size >= 8)
            bd->flag_def[0] |= inst->flags_written(devinfo) & ~bd->flag_use[0];

         ip++;
      }
   }
}

/**
 * The algorithm incrementally sets bits in liveout and livein,
 * propagating it through control flow.  It will eventually terminate
 * because it only ever adds bits, and stops when no bits are added in
 * a pass.
 */
void
fs_live_variables::compute_live_variables()
{
   bool cont = true;

   /* Propagate defin and defout down the CFG to calculate the union of live
    * variables potentially defined along any possible control flow path.
    */
   do {
      cont = false;

      foreach_block (block, cfg) {
         const struct block_data *bd = &block_data[block->num];

         foreach_list_typed(bblock_link, child_link, link, &block->children) {
            struct block_data *child_bd = &block_data[child_link->block->num];

            for (int i = 0; i < bitset_words; i++) {
               const BITSET_WORD new_def = bd->defout[i] & ~child_bd->defin[i];
               child_bd->defin[i] |= new_def;
               child_bd->defout[i] |= new_def;
               cont |= new_def;
            }
         }
      }
   } while (cont);

   do {
      cont = false;

      foreach_block_reverse (block, cfg) {
         struct block_data *bd = &block_data[block->num];

         /* Update liveout */
         foreach_list_typed(bblock_link, child_link, link, &block->children) {
            struct block_data *child_bd = &block_data[child_link->block->num];

            for (int i = 0; i < bitset_words; i++) {
               BITSET_WORD new_liveout = (child_bd->livein[i] &
                                          ~bd->liveout[i]);
               new_liveout &= bd->defout[i]; /* Screen off uses with no reaching def */
               if (new_liveout)
                  bd->liveout[i] |= new_liveout;
            }
            BITSET_WORD new_liveout = (child_bd->flag_livein[0] &
                                       ~bd->flag_liveout[0]);
            if (new_liveout)
               bd->flag_liveout[0] |= new_liveout;
         }

         /* Update livein */
         for (int i = 0; i < bitset_words; i++) {
            BITSET_WORD new_livein = (bd->use[i] |
                                      (bd->liveout[i] &
                                       ~bd->def[i]));
            new_livein &= bd->defin[i]; /* Screen off uses with no reaching def */
            if (new_livein & ~bd->livein[i]) {
               bd->livein[i] |= new_livein;
               cont = true;
            }
         }
         BITSET_WORD new_livein = (bd->flag_use[0] |
                                   (bd->flag_liveout[0] &
                                    ~bd->flag_def[0]));
         if (new_livein & ~bd->flag_livein[0]) {
            bd->flag_livein[0] |= new_livein;
            cont = true;
         }
      }
   } while (cont);
}

/**
 * Extend the start/end ranges for each variable to account for the
 * new information calculated from control flow.
 */
void
fs_live_variables::compute_start_end()
{
   foreach_block (block, cfg) {
      struct block_data *bd = &block_data[block->num];
      unsigned i;

      BITSET_FOREACH_SET(i, bd->livein, (unsigned)num_vars) {
         start[i] = MIN2(start[i], block->start_ip);
         end[i] = MAX2(end[i], block->start_ip);
      }

      BITSET_FOREACH_SET(i, bd->liveout, (unsigned)num_vars) {
         start[i] = MIN2(start[i], block->end_ip);
         end[i] = MAX2(end[i], block->end_ip);
      }
   }
}

fs_live_variables::fs_live_variables(const backend_shader *s)
   : devinfo(s->devinfo), cfg(s->cfg)
{
   mem_ctx = ralloc_context(NULL);
   linear_ctx *lin_ctx = linear_context(mem_ctx);

   num_vgrfs = s->alloc.count;
   num_vars = 0;
   var_from_vgrf = linear_zalloc_array(lin_ctx, int, num_vgrfs);
   for (int i = 0; i < num_vgrfs; i++) {
      var_from_vgrf[i] = num_vars;
      num_vars += s->alloc.sizes[i];
   }

   vgrf_from_var = linear_zalloc_array(lin_ctx, int, num_vars);
   for (int i = 0; i < num_vgrfs; i++) {
      for (unsigned j = 0; j < s->alloc.sizes[i]; j++) {
         vgrf_from_var[var_from_vgrf[i] + j] = i;
      }
   }

   start = ralloc_array(mem_ctx, int, num_vars);
   end = linear_zalloc_array(lin_ctx, int, num_vars);
   for (int i = 0; i < num_vars; i++) {
      start[i] = MAX_INSTRUCTION;
      end[i] = -1;
   }

   vgrf_start = ralloc_array(mem_ctx, int, num_vgrfs);
   vgrf_end = ralloc_array(mem_ctx, int, num_vgrfs);
   for (int i = 0; i < num_vgrfs; i++) {
      vgrf_start[i] = MAX_INSTRUCTION;
      vgrf_end[i] = -1;
   }

   block_data = linear_zalloc_array(lin_ctx, struct block_data, cfg->num_blocks);

   bitset_words = BITSET_WORDS(num_vars);
   for (int i = 0; i < cfg->num_blocks; i++) {
      block_data[i].def = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      block_data[i].use = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      block_data[i].livein = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      block_data[i].liveout = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      block_data[i].defin = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      block_data[i].defout = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);

      block_data[i].flag_def[0] = 0;
      block_data[i].flag_use[0] = 0;
      block_data[i].flag_livein[0] = 0;
      block_data[i].flag_liveout[0] = 0;
   }

   setup_def_use();
   compute_live_variables();
   compute_start_end();

   /* Merge the per-component live ranges to whole VGRF live ranges. */
   for (int i = 0; i < num_vars; i++) {
      const unsigned vgrf = vgrf_from_var[i];
      vgrf_start[vgrf] = MIN2(vgrf_start[vgrf], start[i]);
      vgrf_end[vgrf] = MAX2(vgrf_end[vgrf], end[i]);
   }
}

fs_live_variables::~fs_live_variables()
{
   ralloc_free(mem_ctx);
}

static bool
check_register_live_range(const fs_live_variables *live, int ip,
                          const fs_reg &reg, unsigned n)
{
   const unsigned var = live->var_from_reg(reg);

   if (var + n > unsigned(live->num_vars) ||
       live->vgrf_start[reg.nr] > ip || live->vgrf_end[reg.nr] < ip)
      return false;

   for (unsigned j = 0; j < n; j++) {
      if (live->start[var + j] > ip || live->end[var + j] < ip)
         return false;
   }

   return true;
}

bool
fs_live_variables::validate(const backend_shader *s) const
{
   int ip = 0;

   foreach_block_and_inst(block, fs_inst, inst, s->cfg) {
      for (unsigned i = 0; i < inst->sources; i++) {
         if (inst->src[i].file == VGRF &&
             !check_register_live_range(this, ip,
                                        inst->src[i], regs_read(inst, i)))
            return false;
      }

      if (inst->dst.file == VGRF &&
          !check_register_live_range(this, ip, inst->dst, regs_written(inst)))
         return false;

      ip++;
   }

   return true;
}

bool
fs_live_variables::vars_interfere(int a, int b) const
{
   return !(end[b] <= start[a] ||
            end[a] <= start[b]);
}

bool
fs_live_variables::vgrfs_interfere(int a, int b) const
{
   return !(vgrf_end[a] <= vgrf_start[b] ||
            vgrf_end[b] <= vgrf_start[a]);
}
