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
 */

/** @file brw_fs_copy_propagation.cpp
 *
 * Support for global copy propagation in two passes: A local pass that does
 * intra-block copy (and constant) propagation, and a global pass that uses
 * dataflow analysis on the copies available at the end of each block to re-do
 * local copy propagation with more copies available.
 *
 * See Muchnick's Advanced Compiler Design and Implementation, section
 * 12.5 (p356).
 */

#include "util/bitset.h"
#include "util/u_math.h"
#include "util/rb_tree.h"
#include "brw_fs.h"
#include "brw_fs_live_variables.h"
#include "brw_cfg.h"
#include "brw_eu.h"

using namespace brw;

namespace { /* avoid conflict with opt_copy_propagation_elements */
struct acp_entry {
   struct rb_node by_dst;
   struct rb_node by_src;
   fs_reg dst;
   fs_reg src;
   unsigned global_idx;
   unsigned size_written;
   unsigned size_read;
   enum opcode opcode;
   bool is_partial_write;
   bool force_writemask_all;
};

/**
 * Compare two acp_entry::src.nr
 *
 * This is intended to be used as the comparison function for rb_tree.
 */
static int
cmp_entry_dst_entry_dst(const struct rb_node *a_node, const struct rb_node *b_node)
{
   const struct acp_entry *a_entry =
      rb_node_data(struct acp_entry, a_node, by_dst);

   const struct acp_entry *b_entry =
      rb_node_data(struct acp_entry, b_node, by_dst);

   return a_entry->dst.nr - b_entry->dst.nr;
}

static int
cmp_entry_dst_nr(const struct rb_node *a_node, const void *b_key)
{
   const struct acp_entry *a_entry =
      rb_node_data(struct acp_entry, a_node, by_dst);

   return a_entry->dst.nr - (uintptr_t) b_key;
}

static int
cmp_entry_src_entry_src(const struct rb_node *a_node, const struct rb_node *b_node)
{
   const struct acp_entry *a_entry =
      rb_node_data(struct acp_entry, a_node, by_src);

   const struct acp_entry *b_entry =
      rb_node_data(struct acp_entry, b_node, by_src);

   return a_entry->src.nr - b_entry->src.nr;
}

/**
 * Compare an acp_entry::src.nr with a raw nr.
 *
 * This is intended to be used as the comparison function for rb_tree.
 */
static int
cmp_entry_src_nr(const struct rb_node *a_node, const void *b_key)
{
   const struct acp_entry *a_entry =
      rb_node_data(struct acp_entry, a_node, by_src);

   return a_entry->src.nr - (uintptr_t) b_key;
}

class acp_forward_iterator {
public:
   acp_forward_iterator(struct rb_node *n, unsigned offset)
      : curr(n), next(nullptr), offset(offset)
   {
      next = rb_node_next_or_null(curr);
   }

   acp_forward_iterator &operator++()
   {
      curr = next;
      next = rb_node_next_or_null(curr);

      return *this;
   }

   bool operator!=(const acp_forward_iterator &other) const
   {
      return curr != other.curr;
   }

   struct acp_entry *operator*() const
   {
      /* This open-codes part of rb_node_data. */
      return curr != NULL ? (struct acp_entry *)(((char *)curr) - offset)
                          : NULL;
   }

private:
   struct rb_node *curr;
   struct rb_node *next;
   unsigned offset;
};

struct acp {
   struct rb_tree by_dst;
   struct rb_tree by_src;

   acp()
   {
      rb_tree_init(&by_dst);
      rb_tree_init(&by_src);
   }

   acp_forward_iterator begin()
   {
      return acp_forward_iterator(rb_tree_first(&by_src),
                                  rb_tree_offsetof(struct acp_entry, by_src, 0));
   }

   const acp_forward_iterator end() const
   {
      return acp_forward_iterator(nullptr, 0);
   }

   unsigned length()
   {
      unsigned l = 0;

      for (rb_node *iter = rb_tree_first(&by_src);
           iter != NULL; iter = rb_node_next(iter))
         l++;

      return l;
   }

   void add(acp_entry *entry)
   {
      rb_tree_insert(&by_dst, &entry->by_dst, cmp_entry_dst_entry_dst);
      rb_tree_insert(&by_src, &entry->by_src, cmp_entry_src_entry_src);
   }

   void remove(acp_entry *entry)
   {
      rb_tree_remove(&by_dst, &entry->by_dst);
      rb_tree_remove(&by_src, &entry->by_src);
   }

   acp_forward_iterator find_by_src(unsigned nr)
   {
      struct rb_node *rbn = rb_tree_search(&by_src,
                                           (void *)(uintptr_t) nr,
                                           cmp_entry_src_nr);

      return acp_forward_iterator(rbn, rb_tree_offsetof(struct acp_entry,
                                                        by_src, rbn));
   }

   acp_forward_iterator find_by_dst(unsigned nr)
   {
      struct rb_node *rbn = rb_tree_search(&by_dst,
                                           (void *)(uintptr_t) nr,
                                           cmp_entry_dst_nr);

      return acp_forward_iterator(rbn, rb_tree_offsetof(struct acp_entry,
                                                        by_dst, rbn));
   }
};

struct block_data {
   /**
    * Which entries in the fs_copy_prop_dataflow acp table are live at the
    * start of this block.  This is the useful output of the analysis, since
    * it lets us plug those into the local copy propagation on the second
    * pass.
    */
   BITSET_WORD *livein;

   /**
    * Which entries in the fs_copy_prop_dataflow acp table are live at the end
    * of this block.  This is done in initial setup from the per-block acps
    * returned by the first local copy prop pass.
    */
   BITSET_WORD *liveout;

   /**
    * Which entries in the fs_copy_prop_dataflow acp table are generated by
    * instructions in this block which reach the end of the block without
    * being killed.
    */
   BITSET_WORD *copy;

   /**
    * Which entries in the fs_copy_prop_dataflow acp table are killed over the
    * course of this block.
    */
   BITSET_WORD *kill;

   /**
    * Which entries in the fs_copy_prop_dataflow acp table are guaranteed to
    * have a fully uninitialized destination at the end of this block.
    */
   BITSET_WORD *undef;

   /**
    * Which entries in the fs_copy_prop_dataflow acp table can the
    * start of this block be reached from.  Note that this is a weaker
    * condition than livein.
    */
   BITSET_WORD *reachin;

   /**
    * Which entries in the fs_copy_prop_dataflow acp table are
    * overwritten by an instruction with channel masks inconsistent
    * with the copy instruction (e.g. due to force_writemask_all).
    * Such an overwrite can cause the copy entry to become invalid
    * even if the copy instruction is subsequently re-executed for any
    * given channel i, since the execution of the overwrite for
    * channel i may corrupt other channels j!=i inactive for the
    * subsequent copy.
    */
   BITSET_WORD *exec_mismatch;
};

class fs_copy_prop_dataflow
{
public:
   fs_copy_prop_dataflow(linear_ctx *lin_ctx, cfg_t *cfg,
                         const fs_live_variables &live,
                         struct acp *out_acp);

   void setup_initial_values();
   void run();

   void dump_block_data() const UNUSED;

   cfg_t *cfg;
   const fs_live_variables &live;

   acp_entry **acp;
   int num_acp;
   int bitset_words;

  struct block_data *bd;
};
} /* anonymous namespace */

fs_copy_prop_dataflow::fs_copy_prop_dataflow(linear_ctx *lin_ctx, cfg_t *cfg,
                                             const fs_live_variables &live,
                                             struct acp *out_acp)
   : cfg(cfg), live(live)
{
   bd = linear_zalloc_array(lin_ctx, struct block_data, cfg->num_blocks);

   num_acp = 0;
   foreach_block (block, cfg)
      num_acp += out_acp[block->num].length();

   bitset_words = BITSET_WORDS(num_acp);

   foreach_block (block, cfg) {
      bd[block->num].livein = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      bd[block->num].liveout = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      bd[block->num].copy = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      bd[block->num].kill = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      bd[block->num].undef = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      bd[block->num].reachin = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
      bd[block->num].exec_mismatch = linear_zalloc_array(lin_ctx, BITSET_WORD, bitset_words);
   }

   acp = linear_zalloc_array(lin_ctx, struct acp_entry *, num_acp);

   int next_acp = 0;
   foreach_block (block, cfg) {
      for (auto iter = out_acp[block->num].begin();
           iter != out_acp[block->num].end(); ++iter) {
         acp[next_acp] = *iter;

         (*iter)->global_idx = next_acp;

         /* opt_copy_propagation_local populates out_acp with copies created
          * in a block which are still live at the end of the block.  This
          * is exactly what we want in the COPY set.
          */
         BITSET_SET(bd[block->num].copy, next_acp);

         next_acp++;
      }
   }

   assert(next_acp == num_acp);

   setup_initial_values();
   run();
}

/**
 * Like reg_offset, but register must be VGRF or FIXED_GRF.
 */
static inline unsigned
grf_reg_offset(const fs_reg &r)
{
   return (r.file == VGRF ? 0 : r.nr) * REG_SIZE +
          r.offset +
          (r.file == FIXED_GRF ? r.subnr : 0);
}

/**
 * Like regions_overlap, but register must be VGRF or FIXED_GRF.
 */
static inline bool
grf_regions_overlap(const fs_reg &r, unsigned dr, const fs_reg &s, unsigned ds)
{
   return reg_space(r) == reg_space(s) &&
          !(grf_reg_offset(r) + dr <= grf_reg_offset(s) ||
            grf_reg_offset(s) + ds <= grf_reg_offset(r));
}

/**
 * Set up initial values for each of the data flow sets, prior to running
 * the fixed-point algorithm.
 */
void
fs_copy_prop_dataflow::setup_initial_values()
{
   /* Initialize the COPY and KILL sets. */
   {
      struct acp acp_table;

      /* First, get all the KILLs for instructions which overwrite ACP
       * destinations.
       */
      for (int i = 0; i < num_acp; i++)
         acp_table.add(acp[i]);

      foreach_block (block, cfg) {
         foreach_inst_in_block(fs_inst, inst, block) {
            if (inst->dst.file != VGRF &&
                inst->dst.file != FIXED_GRF)
               continue;

            for (auto iter = acp_table.find_by_src(inst->dst.nr);
              iter != acp_table.end() && (*iter)->src.nr == inst->dst.nr;
              ++iter) {
               if (grf_regions_overlap(inst->dst, inst->size_written,
                                       (*iter)->src, (*iter)->size_read)) {
                  BITSET_SET(bd[block->num].kill, (*iter)->global_idx);
                  if (inst->force_writemask_all && !(*iter)->force_writemask_all)
                     BITSET_SET(bd[block->num].exec_mismatch, (*iter)->global_idx);
               }
            }

            if (inst->dst.file != VGRF)
               continue;

            for (auto iter = acp_table.find_by_dst(inst->dst.nr);
              iter != acp_table.end() && (*iter)->dst.nr == inst->dst.nr;
              ++iter) {
               if (grf_regions_overlap(inst->dst, inst->size_written,
                                       (*iter)->dst, (*iter)->size_written)) {
                  BITSET_SET(bd[block->num].kill, (*iter)->global_idx);
                  if (inst->force_writemask_all && !(*iter)->force_writemask_all)
                     BITSET_SET(bd[block->num].exec_mismatch, (*iter)->global_idx);
               }
            }
         }
      }
   }

   /* Populate the initial values for the livein and liveout sets.  For the
    * block at the start of the program, livein = 0 and liveout = copy.
    * For the others, set liveout and livein to ~0 (the universal set).
    */
   foreach_block (block, cfg) {
      if (block->parents.is_empty()) {
         for (int i = 0; i < bitset_words; i++) {
            bd[block->num].livein[i] = 0u;
            bd[block->num].liveout[i] = bd[block->num].copy[i];
         }
      } else {
         for (int i = 0; i < bitset_words; i++) {
            bd[block->num].liveout[i] = ~0u;
            bd[block->num].livein[i] = ~0u;
         }
      }
   }

   /* Initialize the undef set. */
   foreach_block (block, cfg) {
      for (int i = 0; i < num_acp; i++) {
         BITSET_SET(bd[block->num].undef, i);
         for (unsigned off = 0; off < acp[i]->size_written; off += REG_SIZE) {
            if (BITSET_TEST(live.block_data[block->num].defout,
                            live.var_from_reg(byte_offset(acp[i]->dst, off))))
               BITSET_CLEAR(bd[block->num].undef, i);
         }
      }
   }
}

/**
 * Walk the set of instructions in the block, marking which entries in the acp
 * are killed by the block.
 */
void
fs_copy_prop_dataflow::run()
{
   bool progress;

   do {
      progress = false;

      foreach_block (block, cfg) {
         if (block->parents.is_empty())
            continue;

         for (int i = 0; i < bitset_words; i++) {
            const BITSET_WORD old_liveout = bd[block->num].liveout[i];
            const BITSET_WORD old_reachin = bd[block->num].reachin[i];
            BITSET_WORD livein_from_any_block = 0;

            /* Update livein for this block.  If a copy is live out of all
             * parent blocks, it's live coming in to this block.
             */
            bd[block->num].livein[i] = ~0u;
            foreach_list_typed(bblock_link, parent_link, link, &block->parents) {
               bblock_t *parent = parent_link->block;
               /* Consider ACP entries with a known-undefined destination to
                * be available from the parent.  This is valid because we're
                * free to set the undefined variable equal to the source of
                * the ACP entry without breaking the application's
                * expectations, since the variable is undefined.
                */
               bd[block->num].livein[i] &= (bd[parent->num].liveout[i] |
                                            bd[parent->num].undef[i]);
               livein_from_any_block |= bd[parent->num].liveout[i];

               /* Update reachin for this block.  If the end of any
                * parent block is reachable from the copy, the start
                * of this block is reachable from it as well.
                */
               bd[block->num].reachin[i] |= (bd[parent->num].reachin[i] |
                                             bd[parent->num].copy[i]);
            }

            /* Limit to the set of ACP entries that can possibly be available
             * at the start of the block, since propagating from a variable
             * which is guaranteed to be undefined (rather than potentially
             * undefined for some dynamic control-flow paths) doesn't seem
             * particularly useful.
             */
            bd[block->num].livein[i] &= livein_from_any_block;

            /* Update liveout for this block. */
            bd[block->num].liveout[i] =
               bd[block->num].copy[i] | (bd[block->num].livein[i] &
                                         ~bd[block->num].kill[i]);

            if (old_liveout != bd[block->num].liveout[i] ||
                old_reachin != bd[block->num].reachin[i])
               progress = true;
         }
      }
   } while (progress);

   /* Perform a second fixed-point pass in order to propagate the
    * exec_mismatch bitsets.  Note that this requires an accurate
    * value of the reachin bitsets as input, which isn't available
    * until the end of the first propagation pass, so this loop cannot
    * be folded into the previous one.
    */
   do {
      progress = false;

      foreach_block (block, cfg) {
         for (int i = 0; i < bitset_words; i++) {
            const BITSET_WORD old_exec_mismatch = bd[block->num].exec_mismatch[i];

            /* Update exec_mismatch for this block.  If the end of a
             * parent block is reachable by an overwrite with
             * inconsistent execution masking, the start of this block
             * is reachable by such an overwrite as well.
             */
            foreach_list_typed(bblock_link, parent_link, link, &block->parents) {
               bblock_t *parent = parent_link->block;
               bd[block->num].exec_mismatch[i] |= (bd[parent->num].exec_mismatch[i] &
                                                   bd[parent->num].reachin[i]);
            }

            /* Only consider overwrites with inconsistent execution
             * masking if they are reachable from the copy, since
             * overwrites unreachable from a copy are harmless to that
             * copy.
             */
            bd[block->num].exec_mismatch[i] &= bd[block->num].reachin[i];
            if (old_exec_mismatch != bd[block->num].exec_mismatch[i])
               progress = true;
         }
      }
   } while (progress);
}

void
fs_copy_prop_dataflow::dump_block_data() const
{
   foreach_block (block, cfg) {
      fprintf(stderr, "Block %d [%d, %d] (parents ", block->num,
             block->start_ip, block->end_ip);
      foreach_list_typed(bblock_link, link, link, &block->parents) {
         bblock_t *parent = link->block;
         fprintf(stderr, "%d ", parent->num);
      }
      fprintf(stderr, "):\n");
      fprintf(stderr, "       livein = 0x");
      for (int i = 0; i < bitset_words; i++)
         fprintf(stderr, "%08x", bd[block->num].livein[i]);
      fprintf(stderr, ", liveout = 0x");
      for (int i = 0; i < bitset_words; i++)
         fprintf(stderr, "%08x", bd[block->num].liveout[i]);
      fprintf(stderr, ",\n       copy   = 0x");
      for (int i = 0; i < bitset_words; i++)
         fprintf(stderr, "%08x", bd[block->num].copy[i]);
      fprintf(stderr, ", kill    = 0x");
      for (int i = 0; i < bitset_words; i++)
         fprintf(stderr, "%08x", bd[block->num].kill[i]);
      fprintf(stderr, "\n");
   }
}

static bool
is_logic_op(enum opcode opcode)
{
   return (opcode == BRW_OPCODE_AND ||
           opcode == BRW_OPCODE_OR  ||
           opcode == BRW_OPCODE_XOR ||
           opcode == BRW_OPCODE_NOT);
}

static bool
can_take_stride(fs_inst *inst, brw_reg_type dst_type,
                unsigned arg, unsigned stride,
                const struct brw_compiler *compiler)
{
   const struct intel_device_info *devinfo = compiler->devinfo;

   if (stride > 4)
      return false;

   /* Bail if the channels of the source need to be aligned to the byte offset
    * of the corresponding channel of the destination, and the provided stride
    * would break this restriction.
    */
   if (has_dst_aligned_region_restriction(devinfo, inst, dst_type) &&
       !(type_sz(inst->src[arg].type) * stride ==
           type_sz(dst_type) * inst->dst.stride ||
         stride == 0))
      return false;

   /* 3-source instructions can only be Align16, which restricts what strides
    * they can take. They can only take a stride of 1 (the usual case), or 0
    * with a special "repctrl" bit. But the repctrl bit doesn't work for
    * 64-bit datatypes, so if the source type is 64-bit then only a stride of
    * 1 is allowed. From the Broadwell PRM, Volume 7 "3D Media GPGPU", page
    * 944:
    *
    *    This is applicable to 32b datatypes and 16b datatype. 64b datatypes
    *    cannot use the replicate control.
    */
   if (inst->is_3src(compiler)) {
      if (type_sz(inst->src[arg].type) > 4)
         return stride == 1;
      else
         return stride == 1 || stride == 0;
   }

   /* From the Broadwell PRM, Volume 2a "Command Reference - Instructions",
    * page 391 ("Extended Math Function"):
    *
    *     The following restrictions apply for align1 mode: Scalar source is
    *     supported. Source and destination horizontal stride must be the
    *     same.
    *
    * From the Haswell PRM Volume 2b "Command Reference - Instructions", page
    * 134 ("Extended Math Function"):
    *
    *    Scalar source is supported. Source and destination horizontal stride
    *    must be 1.
    *
    * and similar language exists for IVB and SNB. Pre-SNB, math instructions
    * are sends, so the sources are moved to MRF's and there are no
    * restrictions.
    */
   if (inst->is_math()) {
      if (devinfo->ver == 6 || devinfo->ver == 7) {
         assert(inst->dst.stride == 1);
         return stride == 1 || stride == 0;
      } else if (devinfo->ver >= 8) {
         return stride == inst->dst.stride || stride == 0;
      }
   }

   return true;
}

static bool
instruction_requires_packed_data(fs_inst *inst)
{
   switch (inst->opcode) {
   case FS_OPCODE_DDX_FINE:
   case FS_OPCODE_DDX_COARSE:
   case FS_OPCODE_DDY_FINE:
   case FS_OPCODE_DDY_COARSE:
   case SHADER_OPCODE_QUAD_SWIZZLE:
      return true;
   default:
      return false;
   }
}

static bool
try_copy_propagate(const brw_compiler *compiler, fs_inst *inst,
                   acp_entry *entry, int arg,
                   const brw::simple_allocator &alloc,
                   uint8_t max_polygons)
{
   if (inst->src[arg].file != VGRF)
      return false;

   const struct intel_device_info *devinfo = compiler->devinfo;

   assert(entry->src.file == VGRF || entry->src.file == UNIFORM ||
          entry->src.file == ATTR || entry->src.file == FIXED_GRF);

   /* Avoid propagating a LOAD_PAYLOAD instruction into another if there is a
    * good chance that we'll be able to eliminate the latter through register
    * coalescing.  If only part of the sources of the second LOAD_PAYLOAD can
    * be simplified through copy propagation we would be making register
    * coalescing impossible, ending up with unnecessary copies in the program.
    * This is also the case for is_multi_copy_payload() copies that can only
    * be coalesced when the instruction is lowered into a sequence of MOVs.
    *
    * Worse -- In cases where the ACP entry was the result of CSE combining
    * multiple LOAD_PAYLOAD subexpressions, propagating the first LOAD_PAYLOAD
    * into the second would undo the work of CSE, leading to an infinite
    * optimization loop.  Avoid this by detecting LOAD_PAYLOAD copies from CSE
    * temporaries which should match is_coalescing_payload().
    */
   if (entry->opcode == SHADER_OPCODE_LOAD_PAYLOAD &&
       (is_coalescing_payload(alloc, inst) || is_multi_copy_payload(inst)))
      return false;

   assert(entry->dst.file == VGRF);
   if (inst->src[arg].nr != entry->dst.nr)
      return false;

   /* Bail if inst is reading a range that isn't contained in the range
    * that entry is writing.
    */
   if (!region_contained_in(inst->src[arg], inst->size_read(arg),
                            entry->dst, entry->size_written))
      return false;

   /* Send messages with EOT set are restricted to use g112-g127 (and we
    * sometimes need g127 for other purposes), so avoid copy propagating
    * anything that would make it impossible to satisfy that restriction.
    */
   if (inst->eot) {
      /* Avoid propagating a FIXED_GRF register, as that's already pinned. */
      if (entry->src.file == FIXED_GRF)
         return false;

      /* We might be propagating from a large register, while the SEND only
       * is reading a portion of it (say the .A channel in an RGBA value).
       * We need to pin both split SEND sources in g112-g126/127, so only
       * allow this if the registers aren't too large.
       */
      if (inst->opcode == SHADER_OPCODE_SEND && entry->src.file == VGRF) {
         int other_src = arg == 2 ? 3 : 2;
         unsigned other_size = inst->src[other_src].file == VGRF ?
                               alloc.sizes[inst->src[other_src].nr] :
                               inst->size_read(other_src);
         unsigned prop_src_size = alloc.sizes[entry->src.nr];
         if (other_size + prop_src_size > 15)
            return false;
      }
   }

   /* Avoid propagating odd-numbered FIXED_GRF registers into the first source
    * of a LINTERP instruction on platforms where the PLN instruction has
    * register alignment restrictions.
    */
   if (devinfo->has_pln && devinfo->ver <= 6 &&
       entry->src.file == FIXED_GRF && (entry->src.nr & 1) &&
       inst->opcode == FS_OPCODE_LINTERP && arg == 0)
      return false;

   /* we can't generally copy-propagate UD negations because we
    * can end up accessing the resulting values as signed integers
    * instead. See also resolve_ud_negate() and comment in
    * fs_generator::generate_code.
    */
   if (entry->src.type == BRW_REGISTER_TYPE_UD &&
       entry->src.negate)
      return false;

   bool has_source_modifiers = entry->src.abs || entry->src.negate;

   if (has_source_modifiers && !inst->can_do_source_mods(devinfo))
      return false;

   /* Reject cases that would violate register regioning restrictions. */
   if ((entry->src.file == UNIFORM || !entry->src.is_contiguous()) &&
       ((devinfo->ver == 6 && inst->is_math()) ||
        inst->is_send_from_grf() ||
        inst->uses_indirect_addressing())) {
      return false;
   }

   if (has_source_modifiers &&
       inst->opcode == SHADER_OPCODE_GFX4_SCRATCH_WRITE)
      return false;

   /* Some instructions implemented in the generator backend, such as
    * derivatives, assume that their operands are packed so we can't
    * generally propagate strided regions to them.
    */
   const unsigned entry_stride = (entry->src.file == FIXED_GRF ? 1 :
                                  entry->src.stride);
   if (instruction_requires_packed_data(inst) && entry_stride != 1)
      return false;

   const brw_reg_type dst_type = (has_source_modifiers &&
                                  entry->dst.type != inst->src[arg].type) ?
      entry->dst.type : inst->dst.type;

   /* Bail if the result of composing both strides would exceed the
    * hardware limit.
    */
   if (!can_take_stride(inst, dst_type, arg,
                        entry_stride * inst->src[arg].stride,
                        compiler))
      return false;

   /* From the Cherry Trail/Braswell PRMs, Volume 7: 3D Media GPGPU:
    *    EU Overview
    *       Register Region Restrictions
    *          Special Requirements for Handling Double Precision Data Types :
    *
    *   "When source or destination datatype is 64b or operation is integer
    *    DWord multiply, regioning in Align1 must follow these rules:
    *
    *      1. Source and Destination horizontal stride must be aligned to the
    *         same qword.
    *      2. Regioning must ensure Src.Vstride = Src.Width * Src.Hstride.
    *      3. Source and Destination offset must be the same, except the case
    *         of scalar source."
    *
    * Most of this is already checked in can_take_stride(), we're only left
    * with checking 3.
    */
   if (has_dst_aligned_region_restriction(devinfo, inst, dst_type) &&
       entry_stride != 0 &&
       (reg_offset(inst->dst) % REG_SIZE) != (reg_offset(entry->src) % REG_SIZE))
      return false;

   /* The <8;8,0> regions used for FS attributes in multipolygon
    * dispatch mode could violate regioning restrictions, don't copy
    * propagate them in such cases.
    */
   if (entry->src.file == ATTR && max_polygons > 1 &&
       (has_dst_aligned_region_restriction(devinfo, inst, dst_type) ||
	instruction_requires_packed_data(inst) ||
	(inst->is_3src(compiler) && arg == 2) ||
	entry->dst.type != inst->src[arg].type))
      return false;

   /* Bail if the source FIXED_GRF region of the copy cannot be trivially
    * composed with the source region of the instruction -- E.g. because the
    * copy uses some extended stride greater than 4 not supported natively by
    * the hardware as a horizontal stride, or because instruction compression
    * could require us to use a vertical stride shorter than a GRF.
    */
   if (entry->src.file == FIXED_GRF &&
       (inst->src[arg].stride > 4 ||
        inst->dst.component_size(inst->exec_size) >
        inst->src[arg].component_size(inst->exec_size)))
      return false;

   /* Bail if the instruction type is larger than the execution type of the
    * copy, what implies that each channel is reading multiple channels of the
    * destination of the copy, and simply replacing the sources would give a
    * program with different semantics.
    */
   if ((type_sz(entry->dst.type) < type_sz(inst->src[arg].type) ||
        entry->is_partial_write) &&
       inst->opcode != BRW_OPCODE_MOV) {
      return false;
   }

   /* Bail if the result of composing both strides cannot be expressed
    * as another stride. This avoids, for example, trying to transform
    * this:
    *
    *     MOV (8) rX<1>UD rY<0;1,0>UD
    *     FOO (8) ...     rX<8;8,1>UW
    *
    * into this:
    *
    *     FOO (8) ...     rY<0;1,0>UW
    *
    * Which would have different semantics.
    */
   if (entry_stride != 1 &&
       (inst->src[arg].stride *
        type_sz(inst->src[arg].type)) % type_sz(entry->src.type) != 0)
      return false;

   /* Since semantics of source modifiers are type-dependent we need to
    * ensure that the meaning of the instruction remains the same if we
    * change the type. If the sizes of the types are different the new
    * instruction will read a different amount of data than the original
    * and the semantics will always be different.
    */
   if (has_source_modifiers &&
       entry->dst.type != inst->src[arg].type &&
       (!inst->can_change_types() ||
        type_sz(entry->dst.type) != type_sz(inst->src[arg].type)))
      return false;

   if (devinfo->ver >= 8 && (entry->src.negate || entry->src.abs) &&
       is_logic_op(inst->opcode)) {
      return false;
   }

   /* Save the offset of inst->src[arg] relative to entry->dst for it to be
    * applied later.
    */
   const unsigned rel_offset = inst->src[arg].offset - entry->dst.offset;

   /* Fold the copy into the instruction consuming it. */
   inst->src[arg].file = entry->src.file;
   inst->src[arg].nr = entry->src.nr;
   inst->src[arg].subnr = entry->src.subnr;
   inst->src[arg].offset = entry->src.offset;

   /* Compose the strides of both regions. */
   if (entry->src.file == FIXED_GRF) {
      if (inst->src[arg].stride) {
         const unsigned orig_width = 1 << entry->src.width;
         const unsigned reg_width = REG_SIZE / (type_sz(inst->src[arg].type) *
                                                inst->src[arg].stride);
         inst->src[arg].width = cvt(MIN2(orig_width, reg_width)) - 1;
         inst->src[arg].hstride = cvt(inst->src[arg].stride);
         inst->src[arg].vstride = inst->src[arg].hstride + inst->src[arg].width;
      } else {
         inst->src[arg].vstride = inst->src[arg].hstride =
            inst->src[arg].width = 0;
      }

      inst->src[arg].stride = 1;

      /* Hopefully no Align16 around here... */
      assert(entry->src.swizzle == BRW_SWIZZLE_XYZW);
      inst->src[arg].swizzle = entry->src.swizzle;
   } else {
      inst->src[arg].stride *= entry->src.stride;
   }

   /* Compute the first component of the copy that the instruction is
    * reading, and the base byte offset within that component.
    */
   assert((entry->dst.offset % REG_SIZE == 0 || inst->opcode == BRW_OPCODE_MOV) &&
           entry->dst.stride == 1);
   const unsigned component = rel_offset / type_sz(entry->dst.type);
   const unsigned suboffset = rel_offset % type_sz(entry->dst.type);

   /* Calculate the byte offset at the origin of the copy of the given
    * component and suboffset.
    */
   inst->src[arg] = byte_offset(inst->src[arg],
      component * entry_stride * type_sz(entry->src.type) + suboffset);

   if (has_source_modifiers) {
      if (entry->dst.type != inst->src[arg].type) {
         /* We are propagating source modifiers from a MOV with a different
          * type.  If we got here, then we can just change the source and
          * destination types of the instruction and keep going.
          */
         for (int i = 0; i < inst->sources; i++) {
            inst->src[i].type = entry->dst.type;
         }
         inst->dst.type = entry->dst.type;
      }

      if (!inst->src[arg].abs) {
         inst->src[arg].abs = entry->src.abs;
         inst->src[arg].negate ^= entry->src.negate;
      }
   }

   return true;
}


static bool
try_constant_propagate(const brw_compiler *compiler, fs_inst *inst,
                       acp_entry *entry, int arg)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   bool progress = false;

   if (type_sz(entry->src.type) > 4)
      return false;

   if (inst->src[arg].file != VGRF)
      return false;

   assert(entry->dst.file == VGRF);
   if (inst->src[arg].nr != entry->dst.nr)
      return false;

   /* Bail if inst is reading a range that isn't contained in the range
    * that entry is writing.
    */
   if (!region_contained_in(inst->src[arg], inst->size_read(arg),
                            entry->dst, entry->size_written))
      return false;

   /* If the size of the use type is larger than the size of the entry
    * type, the entry doesn't contain all of the data that the user is
    * trying to use.
    */
   if (type_sz(inst->src[arg].type) > type_sz(entry->dst.type))
      return false;

   fs_reg val = entry->src;

   /* If the size of the use type is smaller than the size of the entry,
    * clamp the value to the range of the use type.  This enables constant
    * copy propagation in cases like
    *
    *
    *    mov(8)          g12<1>UD        0x0000000cUD
    *    ...
    *    mul(8)          g47<1>D         g86<8,8,1>D     g12<16,8,2>W
    */
   if (type_sz(inst->src[arg].type) < type_sz(entry->dst.type)) {
      if (type_sz(inst->src[arg].type) != 2 || type_sz(entry->dst.type) != 4)
         return false;

      assert(inst->src[arg].subnr == 0 || inst->src[arg].subnr == 2);

      /* When subnr is 0, we want the lower 16-bits, and when it's 2, we
       * want the upper 16-bits. No other values of subnr are valid for a
       * UD source.
       */
      const uint16_t v = inst->src[arg].subnr == 2 ? val.ud >> 16 : val.ud;

      val.ud = v | (uint32_t(v) << 16);
   }

   val.type = inst->src[arg].type;

   if (inst->src[arg].abs) {
      if ((devinfo->ver >= 8 && is_logic_op(inst->opcode)) ||
          !brw_abs_immediate(val.type, &val.as_brw_reg())) {
         return false;
      }
   }

   if (inst->src[arg].negate) {
      if ((devinfo->ver >= 8 && is_logic_op(inst->opcode)) ||
          !brw_negate_immediate(val.type, &val.as_brw_reg())) {
         return false;
      }
   }

   switch (inst->opcode) {
   case BRW_OPCODE_MOV:
   case SHADER_OPCODE_LOAD_PAYLOAD:
   case FS_OPCODE_PACK:
      inst->src[arg] = val;
      progress = true;
      break;

   case SHADER_OPCODE_POW:
      /* Allow constant propagation into src1 (except on Gen 6 which
       * doesn't support scalar source math), and let constant combining
       * promote the constant on Gen < 8.
       */
      if (devinfo->ver == 6)
         break;

      if (arg == 1) {
         inst->src[arg] = val;
         progress = true;
      }
      break;

   case BRW_OPCODE_SUBB:
      if (arg == 1) {
         inst->src[arg] = val;
         progress = true;
      }
      break;

   case BRW_OPCODE_MACH:
   case BRW_OPCODE_MUL:
   case SHADER_OPCODE_MULH:
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_XOR:
   case BRW_OPCODE_ADDC:
      if (arg == 1) {
         inst->src[arg] = val;
         progress = true;
      } else if (arg == 0 && inst->src[1].file != IMM) {
         /* Don't copy propagate the constant in situations like
          *
          *    mov(8)          g8<1>D          0x7fffffffD
          *    mul(8)          g16<1>D         g8<8,8,1>D      g15<16,8,2>W
          *
          * On platforms that only have a 32x16 multiplier, this will
          * result in lowering the multiply to
          *
          *    mul(8)          g15<1>D         g14<8,8,1>D     0xffffUW
          *    mul(8)          g16<1>D         g14<8,8,1>D     0x7fffUW
          *    add(8)          g15.1<2>UW      g15.1<16,8,2>UW g16<16,8,2>UW
          *
          * On Gfx8 and Gfx9, which have the full 32x32 multiplier, it
          * results in
          *
          *    mul(8)          g16<1>D         g15<16,8,2>W    0x7fffffffD
          *
          * Volume 2a of the Skylake PRM says:
          *
          *    When multiplying a DW and any lower precision integer, the
          *    DW operand must on src0.
          */
         if (inst->opcode == BRW_OPCODE_MUL &&
             type_sz(inst->src[1].type) < 4 &&
             type_sz(val.type) == 4)
            break;

         /* Fit this constant in by commuting the operands.
          * Exception: we can't do this for 32-bit integer MUL/MACH
          * because it's asymmetric.
          *
          * The BSpec says for Broadwell that
          *
          *    "When multiplying DW x DW, the dst cannot be accumulator."
          *
          * Integer MUL with a non-accumulator destination will be lowered
          * by lower_integer_multiplication(), so don't restrict it.
          */
         if (((inst->opcode == BRW_OPCODE_MUL &&
               inst->dst.is_accumulator()) ||
              inst->opcode == BRW_OPCODE_MACH) &&
             (inst->src[1].type == BRW_REGISTER_TYPE_D ||
              inst->src[1].type == BRW_REGISTER_TYPE_UD))
            break;
         inst->src[0] = inst->src[1];
         inst->src[1] = val;
         progress = true;
      }
      break;

   case BRW_OPCODE_ADD3:
      /* add3 can have a single imm16 source. Proceed if the source type is
       * already W or UW or the value can be coerced to one of those types.
       */
      if (val.type == BRW_REGISTER_TYPE_W || val.type == BRW_REGISTER_TYPE_UW)
         ; /* Nothing to do. */
      else if (val.ud <= 0xffff)
         val = brw_imm_uw(val.ud);
      else if (val.d >= -0x8000 && val.d <= 0x7fff)
         val = brw_imm_w(val.d);
      else
         break;

      if (arg == 2) {
         inst->src[arg] = val;
         progress = true;
      } else if (inst->src[2].file != IMM) {
         inst->src[arg] = inst->src[2];
         inst->src[2] = val;
         progress = true;
      }

      break;

   case BRW_OPCODE_CMP:
   case BRW_OPCODE_IF:
      if (arg == 1) {
         inst->src[arg] = val;
         progress = true;
      } else if (arg == 0 && inst->src[1].file != IMM) {
         enum brw_conditional_mod new_cmod;

         new_cmod = brw_swap_cmod(inst->conditional_mod);
         if (new_cmod != BRW_CONDITIONAL_NONE) {
            /* Fit this constant in by swapping the operands and
             * flipping the test
             */
            inst->src[0] = inst->src[1];
            inst->src[1] = val;
            inst->conditional_mod = new_cmod;
            progress = true;
         }
      }
      break;

   case BRW_OPCODE_SEL:
      if (arg == 1) {
         inst->src[arg] = val;
         progress = true;
      } else if (arg == 0) {
         if (inst->src[1].file != IMM &&
             (inst->conditional_mod == BRW_CONDITIONAL_NONE ||
              /* Only GE and L are commutative. */
              inst->conditional_mod == BRW_CONDITIONAL_GE ||
              inst->conditional_mod == BRW_CONDITIONAL_L)) {
            inst->src[0] = inst->src[1];
            inst->src[1] = val;

            /* If this was predicated, flipping operands means
             * we also need to flip the predicate.
             */
            if (inst->conditional_mod == BRW_CONDITIONAL_NONE) {
               inst->predicate_inverse =
                  !inst->predicate_inverse;
            }
         } else {
            inst->src[0] = val;
         }

         progress = true;
      }
      break;

   case FS_OPCODE_FB_WRITE_LOGICAL:
      /* The stencil and omask sources of FS_OPCODE_FB_WRITE_LOGICAL are
       * bit-cast using a strided region so they cannot be immediates.
       */
      if (arg != FB_WRITE_LOGICAL_SRC_SRC_STENCIL &&
          arg != FB_WRITE_LOGICAL_SRC_OMASK) {
         inst->src[arg] = val;
         progress = true;
      }
      break;

   case SHADER_OPCODE_INT_QUOTIENT:
   case SHADER_OPCODE_INT_REMAINDER:
      /* Allow constant propagation into either source (except on Gen 6
       * which doesn't support scalar source math). Constant combining
       * promote the src1 constant on Gen < 8, and it will promote the src0
       * constant on all platforms.
       */
      if (devinfo->ver == 6)
         break;

      FALLTHROUGH;
   case BRW_OPCODE_AND:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_BFE:
   case BRW_OPCODE_BFI1:
   case BRW_OPCODE_BFI2:
   case BRW_OPCODE_ROL:
   case BRW_OPCODE_ROR:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_SHR:
   case BRW_OPCODE_OR:
   case SHADER_OPCODE_TEX_LOGICAL:
   case SHADER_OPCODE_TXD_LOGICAL:
   case SHADER_OPCODE_TXF_LOGICAL:
   case SHADER_OPCODE_TXL_LOGICAL:
   case SHADER_OPCODE_TXS_LOGICAL:
   case FS_OPCODE_TXB_LOGICAL:
   case SHADER_OPCODE_TXF_CMS_LOGICAL:
   case SHADER_OPCODE_TXF_CMS_W_LOGICAL:
   case SHADER_OPCODE_TXF_CMS_W_GFX12_LOGICAL:
   case SHADER_OPCODE_TXF_UMS_LOGICAL:
   case SHADER_OPCODE_TXF_MCS_LOGICAL:
   case SHADER_OPCODE_LOD_LOGICAL:
   case SHADER_OPCODE_TG4_LOGICAL:
   case SHADER_OPCODE_TG4_OFFSET_LOGICAL:
   case SHADER_OPCODE_SAMPLEINFO_LOGICAL:
   case SHADER_OPCODE_IMAGE_SIZE_LOGICAL:
   case SHADER_OPCODE_UNTYPED_ATOMIC_LOGICAL:
   case SHADER_OPCODE_UNTYPED_SURFACE_READ_LOGICAL:
   case SHADER_OPCODE_UNTYPED_SURFACE_WRITE_LOGICAL:
   case SHADER_OPCODE_TYPED_ATOMIC_LOGICAL:
   case SHADER_OPCODE_TYPED_SURFACE_READ_LOGICAL:
   case SHADER_OPCODE_TYPED_SURFACE_WRITE_LOGICAL:
   case SHADER_OPCODE_BYTE_SCATTERED_WRITE_LOGICAL:
   case SHADER_OPCODE_BYTE_SCATTERED_READ_LOGICAL:
   case FS_OPCODE_UNIFORM_PULL_CONSTANT_LOAD:
   case SHADER_OPCODE_BROADCAST:
   case BRW_OPCODE_MAD:
   case BRW_OPCODE_LRP:
   case FS_OPCODE_PACK_HALF_2x16_SPLIT:
   case SHADER_OPCODE_SHUFFLE:
      inst->src[arg] = val;
      progress = true;
      break;

   default:
      break;
   }

   return progress;
}

static bool
can_propagate_from(fs_inst *inst)
{
   return (inst->opcode == BRW_OPCODE_MOV &&
           inst->dst.file == VGRF &&
           ((inst->src[0].file == VGRF &&
             !grf_regions_overlap(inst->dst, inst->size_written,
                                  inst->src[0], inst->size_read(0))) ||
            inst->src[0].file == ATTR ||
            inst->src[0].file == UNIFORM ||
            inst->src[0].file == IMM ||
            (inst->src[0].file == FIXED_GRF &&
             inst->src[0].is_contiguous())) &&
           inst->src[0].type == inst->dst.type &&
           !inst->saturate &&
           /* Subset of !is_partial_write() conditions. */
           !inst->predicate && inst->dst.is_contiguous()) ||
          is_identity_payload(FIXED_GRF, inst);
}

/* Walks a basic block and does copy propagation on it using the acp
 * list.
 */
static bool
opt_copy_propagation_local(const brw_compiler *compiler, linear_ctx *lin_ctx,
                           bblock_t *block, struct acp &acp,
                           const brw::simple_allocator &alloc,
                           uint8_t max_polygons)
{
   bool progress = false;

   foreach_inst_in_block(fs_inst, inst, block) {
      /* Try propagating into this instruction. */
      bool instruction_progress = false;
      for (int i = inst->sources - 1; i >= 0; i--) {
         if (inst->src[i].file != VGRF)
            continue;

         for (auto iter = acp.find_by_dst(inst->src[i].nr);
              iter != acp.end() && (*iter)->dst.nr == inst->src[i].nr;
              ++iter) {
            if ((*iter)->src.file == IMM) {
               if (try_constant_propagate(compiler, inst, *iter, i)) {
                  instruction_progress = true;
                  break;
               }
            } else {
               if (try_copy_propagate(compiler, inst, *iter, i, alloc,
                                      max_polygons)) {
                  instruction_progress = true;
                  break;
               }
            }
         }
      }

      if (instruction_progress) {
         progress = true;

         /* ADD3 can only have the immediate as src0. */
         if (inst->opcode == BRW_OPCODE_ADD3) {
            if (inst->src[2].file == IMM) {
               const auto src0 = inst->src[0];
               inst->src[0] = inst->src[2];
               inst->src[2] = src0;
            }
         }

         /* If only one of the sources of a 2-source, commutative instruction (e.g.,
          * AND) is immediate, it must be src1. If both are immediate, opt_algebraic
          * should fold it away.
          */
         if (inst->sources == 2 && inst->is_commutative() &&
             inst->src[0].file == IMM && inst->src[1].file != IMM) {
            const auto src1 = inst->src[1];
            inst->src[1] = inst->src[0];
            inst->src[0] = src1;
         }
      }

      /* kill the destination from the ACP */
      if (inst->dst.file == VGRF || inst->dst.file == FIXED_GRF) {
         for (auto iter = acp.find_by_dst(inst->dst.nr);
              iter != acp.end() && (*iter)->dst.nr == inst->dst.nr;
              ++iter) {
            if (grf_regions_overlap((*iter)->dst, (*iter)->size_written,
                                    inst->dst, inst->size_written))
               acp.remove(*iter);
         }

         for (auto iter = acp.find_by_src(inst->dst.nr);
              iter != acp.end() && (*iter)->src.nr == inst->dst.nr;
              ++iter) {
            /* Make sure we kill the entry if this instruction overwrites
             * _any_ of the registers that it reads
             */
            if (grf_regions_overlap((*iter)->src, (*iter)->size_read,
                                    inst->dst, inst->size_written))
               acp.remove(*iter);
         }
      }

      /* If this instruction's source could potentially be folded into the
       * operand of another instruction, add it to the ACP.
       */
      if (can_propagate_from(inst)) {
         acp_entry *entry = linear_zalloc(lin_ctx, acp_entry);
         entry->dst = inst->dst;
         entry->src = inst->src[0];
         entry->size_written = inst->size_written;
         for (unsigned i = 0; i < inst->sources; i++)
            entry->size_read += inst->size_read(i);
         entry->opcode = inst->opcode;
         entry->is_partial_write = inst->is_partial_write();
         entry->force_writemask_all = inst->force_writemask_all;
         acp.add(entry);
      } else if (inst->opcode == SHADER_OPCODE_LOAD_PAYLOAD &&
                 inst->dst.file == VGRF) {
         int offset = 0;
         for (int i = 0; i < inst->sources; i++) {
            int effective_width = i < inst->header_size ? 8 : inst->exec_size;
            const unsigned size_written = effective_width *
                                          type_sz(inst->src[i].type);
            if (inst->src[i].file == VGRF ||
                (inst->src[i].file == FIXED_GRF &&
                 inst->src[i].is_contiguous())) {
               const brw_reg_type t = i < inst->header_size ?
                  BRW_REGISTER_TYPE_UD : inst->src[i].type;
               fs_reg dst = byte_offset(retype(inst->dst, t), offset);
               if (!dst.equals(inst->src[i])) {
                  acp_entry *entry = linear_zalloc(lin_ctx, acp_entry);
                  entry->dst = dst;
                  entry->src = retype(inst->src[i], t);
                  entry->size_written = size_written;
                  entry->size_read = inst->size_read(i);
                  entry->opcode = inst->opcode;
                  entry->force_writemask_all = inst->force_writemask_all;
                  acp.add(entry);
               }
            }
            offset += size_written;
         }
      }
   }

   return progress;
}

bool
fs_visitor::opt_copy_propagation()
{
   bool progress = false;
   void *copy_prop_ctx = ralloc_context(NULL);
   linear_ctx *lin_ctx = linear_context(copy_prop_ctx);
   struct acp out_acp[cfg->num_blocks];

   const fs_live_variables &live = live_analysis.require();

   /* First, walk through each block doing local copy propagation and getting
    * the set of copies available at the end of the block.
    */
   foreach_block (block, cfg) {
      progress = opt_copy_propagation_local(compiler, lin_ctx, block,
                                            out_acp[block->num], alloc,
                                            max_polygons) || progress;

      /* If the destination of an ACP entry exists only within this block,
       * then there's no need to keep it for dataflow analysis.  We can delete
       * it from the out_acp table and avoid growing the bitsets any bigger
       * than we absolutely have to.
       *
       * Because nothing in opt_copy_propagation_local touches the block
       * start/end IPs and opt_copy_propagation_local is incapable of
       * extending the live range of an ACP destination beyond the block,
       * it's safe to use the liveness information in this way.
       */
      for (auto iter = out_acp[block->num].begin();
           iter != out_acp[block->num].end(); ++iter) {
         assert((*iter)->dst.file == VGRF);
         if (block->start_ip <= live.vgrf_start[(*iter)->dst.nr] &&
             live.vgrf_end[(*iter)->dst.nr] <= block->end_ip) {
            out_acp[block->num].remove(*iter);
         }
      }
   }

   /* Do dataflow analysis for those available copies. */
   fs_copy_prop_dataflow dataflow(lin_ctx, cfg, live, out_acp);

   /* Next, re-run local copy propagation, this time with the set of copies
    * provided by the dataflow analysis available at the start of a block.
    */
   foreach_block (block, cfg) {
      struct acp in_acp;

      for (int i = 0; i < dataflow.num_acp; i++) {
         if (BITSET_TEST(dataflow.bd[block->num].livein, i) &&
             !BITSET_TEST(dataflow.bd[block->num].exec_mismatch, i)) {
            struct acp_entry *entry = dataflow.acp[i];
            in_acp.add(entry);
         }
      }

      progress = opt_copy_propagation_local(compiler, lin_ctx, block,
                                            in_acp, alloc, max_polygons) ||
                 progress;
   }

   ralloc_free(copy_prop_ctx);

   if (progress)
      invalidate_analysis(DEPENDENCY_INSTRUCTION_DATA_FLOW |
                          DEPENDENCY_INSTRUCTION_DETAIL);

   return progress;
}
