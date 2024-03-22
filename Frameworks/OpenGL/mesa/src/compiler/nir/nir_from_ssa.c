/*
 * Copyright © 2014 Intel Corporation
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

#include "nir.h"
#include "nir_builder.h"
#include "nir_builder_opcodes.h"
#include "nir_vla.h"

/*
 * This file implements an out-of-SSA pass as described in "Revisiting
 * Out-of-SSA Translation for Correctness, Code Quality, and Efficiency" by
 * Boissinot et al.
 */

struct from_ssa_state {
   nir_builder builder;
   void *dead_ctx;
   struct exec_list dead_instrs;
   bool phi_webs_only;
   struct hash_table *merge_node_table;
   nir_instr *instr;
   bool progress;
};

/* Returns if def @a comes after def @b.
 *
 * The core observation that makes the Boissinot algorithm efficient
 * is that, given two properly sorted sets, we can check for
 * interference in these sets via a linear walk. This is accomplished
 * by doing single combined walk over union of the two sets in DFS
 * order. It doesn't matter what DFS we do so long as we're
 * consistent. Fortunately, the dominance algorithm we ran prior to
 * this pass did such a walk and recorded the pre- and post-indices in
 * the blocks.
 *
 * We treat SSA undefs as always coming before other instruction types.
 */
static bool
def_after(nir_def *a, nir_def *b)
{
   if (a->parent_instr->type == nir_instr_type_undef)
      return false;

   if (b->parent_instr->type == nir_instr_type_undef)
      return true;

   /* If they're in the same block, we can rely on whichever instruction
    * comes first in the block.
    */
   if (a->parent_instr->block == b->parent_instr->block)
      return a->parent_instr->index > b->parent_instr->index;

   /* Otherwise, if blocks are distinct, we sort them in DFS pre-order */
   return a->parent_instr->block->dom_pre_index >
          b->parent_instr->block->dom_pre_index;
}

/* Returns true if a dominates b */
static bool
ssa_def_dominates(nir_def *a, nir_def *b)
{
   if (a->parent_instr->type == nir_instr_type_undef) {
      /* SSA undefs always dominate */
      return true;
   }
   if (def_after(a, b)) {
      return false;
   } else if (a->parent_instr->block == b->parent_instr->block) {
      return def_after(b, a);
   } else {
      return nir_block_dominates(a->parent_instr->block,
                                 b->parent_instr->block);
   }
}

/* The following data structure, which I have named merge_set is a way of
 * representing a set registers of non-interfering registers.  This is
 * based on the concept of a "dominance forest" presented in "Fast Copy
 * Coalescing and Live-Range Identification" by Budimlic et al. but the
 * implementation concept is taken from  "Revisiting Out-of-SSA Translation
 * for Correctness, Code Quality, and Efficiency" by Boissinot et al.
 *
 * Each SSA definition is associated with a merge_node and the association
 * is represented by a combination of a hash table and the "def" parameter
 * in the merge_node structure.  The merge_set stores a linked list of
 * merge_nodes, ordered by a pre-order DFS walk of the dominance tree.  (Since
 * the liveness analysis pass indexes the SSA values in dominance order for
 * us, this is an easy thing to keep up.)  It is assumed that no pair of the
 * nodes in a given set interfere.  Merging two sets or checking for
 * interference can be done in a single linear-time merge-sort walk of the
 * two lists of nodes.
 */
struct merge_set;

typedef struct {
   struct exec_node node;
   struct merge_set *set;
   nir_def *def;
} merge_node;

typedef struct merge_set {
   struct exec_list nodes;
   unsigned size;
   bool divergent;
   nir_def *reg_decl;
} merge_set;

#if 0
static void
merge_set_dump(merge_set *set, FILE *fp)
{
   NIR_VLA(nir_def *, dom, set->size);
   int dom_idx = -1;

   foreach_list_typed(merge_node, node, node, &set->nodes) {
      while (dom_idx >= 0 && !ssa_def_dominates(dom[dom_idx], node->def))
         dom_idx--;

      for (int i = 0; i <= dom_idx; i++)
         fprintf(fp, "  ");

      fprintf(fp, "ssa_%d\n", node->def->index);

      dom[++dom_idx] = node->def;
   }
}
#endif

static merge_node *
get_merge_node(nir_def *def, struct from_ssa_state *state)
{
   struct hash_entry *entry =
      _mesa_hash_table_search(state->merge_node_table, def);
   if (entry)
      return entry->data;

   merge_set *set = rzalloc(state->dead_ctx, merge_set);
   exec_list_make_empty(&set->nodes);
   set->size = 1;
   set->divergent = def->divergent;

   merge_node *node = ralloc(state->dead_ctx, merge_node);
   node->set = set;
   node->def = def;
   exec_list_push_head(&set->nodes, &node->node);

   _mesa_hash_table_insert(state->merge_node_table, def, node);

   return node;
}

static bool
merge_nodes_interfere(merge_node *a, merge_node *b)
{
   /* There's no need to check for interference within the same set,
    * because we assume, that sets themselves are already
    * interference-free.
    */
   if (a->set == b->set)
      return false;

   return nir_defs_interfere(a->def, b->def);
}

/* Merges b into a
 *
 * This algorithm uses def_after to ensure that the sets always stay in the
 * same order as the pre-order DFS done by the liveness algorithm.
 */
static merge_set *
merge_merge_sets(merge_set *a, merge_set *b)
{
   struct exec_node *an = exec_list_get_head(&a->nodes);
   struct exec_node *bn = exec_list_get_head(&b->nodes);
   while (!exec_node_is_tail_sentinel(bn)) {
      merge_node *a_node = exec_node_data(merge_node, an, node);
      merge_node *b_node = exec_node_data(merge_node, bn, node);

      if (exec_node_is_tail_sentinel(an) ||
          def_after(a_node->def, b_node->def)) {
         struct exec_node *next = bn->next;
         exec_node_remove(bn);
         exec_node_insert_node_before(an, bn);
         exec_node_data(merge_node, bn, node)->set = a;
         bn = next;
      } else {
         an = an->next;
      }
   }

   a->size += b->size;
   b->size = 0;
   a->divergent |= b->divergent;

   return a;
}

/* Checks for any interference between two merge sets
 *
 * This is an implementation of Algorithm 2 in "Revisiting Out-of-SSA
 * Translation for Correctness, Code Quality, and Efficiency" by
 * Boissinot et al.
 */
static bool
merge_sets_interfere(merge_set *a, merge_set *b)
{
   /* List of all the nodes which dominate the current node, in dominance
    * order.
    */
   NIR_VLA(merge_node *, dom, a->size + b->size);
   int dom_idx = -1;

   struct exec_node *an = exec_list_get_head(&a->nodes);
   struct exec_node *bn = exec_list_get_head(&b->nodes);
   while (!exec_node_is_tail_sentinel(an) ||
          !exec_node_is_tail_sentinel(bn)) {

      /* We walk the union of the two sets in the same order as the pre-order
       * DFS done by liveness analysis.
       */
      merge_node *current;
      if (exec_node_is_tail_sentinel(an)) {
         current = exec_node_data(merge_node, bn, node);
         bn = bn->next;
      } else if (exec_node_is_tail_sentinel(bn)) {
         current = exec_node_data(merge_node, an, node);
         an = an->next;
      } else {
         merge_node *a_node = exec_node_data(merge_node, an, node);
         merge_node *b_node = exec_node_data(merge_node, bn, node);

         if (def_after(b_node->def, a_node->def)) {
            current = a_node;
            an = an->next;
         } else {
            current = b_node;
            bn = bn->next;
         }
      }

      /* Because our walk is a pre-order DFS, we can maintain the list of
       * dominating nodes as a simple stack, pushing every node onto the list
       * after we visit it and popping any non-dominating nodes off before we
       * visit the current node.
       */
      while (dom_idx >= 0 &&
             !ssa_def_dominates(dom[dom_idx]->def, current->def))
         dom_idx--;

      /* There are three invariants of this algorithm that are important here:
       *
       *  1. There is no interference within either set a or set b.
       *  2. None of the nodes processed up until this point interfere.
       *  3. All the dominators of `current` have been processed
       *
       * Because of these invariants, we only need to check the current node
       * against its minimal dominator.  If any other node N in the union
       * interferes with current, then N must dominate current because we are
       * in SSA form.  If N dominates current then it must also dominate our
       * minimal dominator dom[dom_idx].  Since N is live at current it must
       * also be live at the minimal dominator which means N interferes with
       * the minimal dominator dom[dom_idx] and, by invariants 2 and 3 above,
       * the algorithm would have already terminated.  Therefore, if we got
       * here, the only node that can possibly interfere with current is the
       * minimal dominator dom[dom_idx].
       *
       * This is what allows us to do a interference check of the union of the
       * two sets with a single linear-time walk.
       */
      if (dom_idx >= 0 && merge_nodes_interfere(current, dom[dom_idx]))
         return true;

      dom[++dom_idx] = current;
   }

   return false;
}

static bool
add_parallel_copy_to_end_of_block(nir_shader *shader, nir_block *block, void *dead_ctx)
{
   bool need_end_copy = false;
   if (block->successors[0]) {
      nir_instr *instr = nir_block_first_instr(block->successors[0]);
      if (instr && instr->type == nir_instr_type_phi)
         need_end_copy = true;
   }

   if (block->successors[1]) {
      nir_instr *instr = nir_block_first_instr(block->successors[1]);
      if (instr && instr->type == nir_instr_type_phi)
         need_end_copy = true;
   }

   if (need_end_copy) {
      /* If one of our successors has at least one phi node, we need to
       * create a parallel copy at the end of the block but before the jump
       * (if there is one).
       */
      nir_parallel_copy_instr *pcopy =
         nir_parallel_copy_instr_create(shader);

      nir_instr_insert(nir_after_block_before_jump(block), &pcopy->instr);
   }

   return true;
}

static nir_parallel_copy_instr *
get_parallel_copy_at_end_of_block(nir_block *block)
{
   nir_instr *last_instr = nir_block_last_instr(block);
   if (last_instr == NULL)
      return NULL;

   /* The last instruction may be a jump in which case the parallel copy is
    * right before it.
    */
   if (last_instr->type == nir_instr_type_jump)
      last_instr = nir_instr_prev(last_instr);

   if (last_instr && last_instr->type == nir_instr_type_parallel_copy)
      return nir_instr_as_parallel_copy(last_instr);
   else
      return NULL;
}

/** Isolate phi nodes with parallel copies
 *
 * In order to solve the dependency problems with the sources and
 * destinations of phi nodes, we first isolate them by adding parallel
 * copies to the beginnings and ends of basic blocks.  For every block with
 * phi nodes, we add a parallel copy immediately following the last phi
 * node that copies the destinations of all of the phi nodes to new SSA
 * values.  We also add a parallel copy to the end of every block that has
 * a successor with phi nodes that, for each phi node in each successor,
 * copies the corresponding sorce of the phi node and adjust the phi to
 * used the destination of the parallel copy.
 *
 * In SSA form, each value has exactly one definition.  What this does is
 * ensure that each value used in a phi also has exactly one use.  The
 * destinations of phis are only used by the parallel copy immediately
 * following the phi nodes and.  Thanks to the parallel copy at the end of
 * the predecessor block, the sources of phi nodes are are the only use of
 * that value.  This allows us to immediately assign all the sources and
 * destinations of any given phi node to the same register without worrying
 * about interference at all.  We do coalescing to get rid of the parallel
 * copies where possible.
 *
 * Before this pass can be run, we have to iterate over the blocks with
 * add_parallel_copy_to_end_of_block to ensure that the parallel copies at
 * the ends of blocks exist.  We can create the ones at the beginnings as
 * we go, but the ones at the ends of blocks need to be created ahead of
 * time because of potential back-edges in the CFG.
 */
static bool
isolate_phi_nodes_block(nir_shader *shader, nir_block *block, void *dead_ctx)
{
   /* If we don't have any phis, then there's nothing for us to do. */
   nir_phi_instr *last_phi = nir_block_last_phi_instr(block);
   if (last_phi == NULL)
      return true;

   /* If we have phi nodes, we need to create a parallel copy at the
    * start of this block but after the phi nodes.
    */
   nir_parallel_copy_instr *block_pcopy =
      nir_parallel_copy_instr_create(shader);
   nir_instr_insert_after(&last_phi->instr, &block_pcopy->instr);

   nir_foreach_phi(phi, block) {
      nir_foreach_phi_src(src, phi) {
         if (nir_src_is_undef(src->src))
            continue;

         nir_parallel_copy_instr *pcopy =
            get_parallel_copy_at_end_of_block(src->pred);
         assert(pcopy);

         nir_parallel_copy_entry *entry = rzalloc(dead_ctx,
                                                  nir_parallel_copy_entry);

         entry->dest_is_reg = false;
         nir_def_init(&pcopy->instr, &entry->dest.def,
                      phi->def.num_components, phi->def.bit_size);
         entry->dest.def.divergent = nir_src_is_divergent(src->src);

         /* We're adding a source to a live instruction so we need to use
          * nir_instr_init_src()
          */
         entry->src_is_reg = false;
         nir_instr_init_src(&pcopy->instr, &entry->src, src->src.ssa);

         exec_list_push_tail(&pcopy->entries, &entry->node);

         nir_src_rewrite(&src->src, &entry->dest.def);
      }

      nir_parallel_copy_entry *entry = rzalloc(dead_ctx,
                                               nir_parallel_copy_entry);

      entry->dest_is_reg = false;
      nir_def_init(&block_pcopy->instr, &entry->dest.def,
                   phi->def.num_components, phi->def.bit_size);
      entry->dest.def.divergent = phi->def.divergent;

      nir_def_rewrite_uses(&phi->def, &entry->dest.def);

      /* We're adding a source to a live instruction so we need to use
       * nir_instr_init_src().
       *
       * Note that we do this after we've rewritten all uses of the phi to
       * entry->def, ensuring that entry->src will be the only remaining use
       * of the phi.
       */
      entry->src_is_reg = false;
      nir_instr_init_src(&block_pcopy->instr, &entry->src, &phi->def);

      exec_list_push_tail(&block_pcopy->entries, &entry->node);
   }

   return true;
}

static bool
coalesce_phi_nodes_block(nir_block *block, struct from_ssa_state *state)
{
   nir_foreach_phi(phi, block) {
      merge_node *dest_node = get_merge_node(&phi->def, state);

      nir_foreach_phi_src(src, phi) {
         if (nir_src_is_undef(src->src))
            continue;

         merge_node *src_node = get_merge_node(src->src.ssa, state);
         if (src_node->set != dest_node->set)
            merge_merge_sets(dest_node->set, src_node->set);
      }
   }

   return true;
}

static void
aggressive_coalesce_parallel_copy(nir_parallel_copy_instr *pcopy,
                                  struct from_ssa_state *state)
{
   nir_foreach_parallel_copy_entry(entry, pcopy) {
      assert(!entry->src_is_reg);
      assert(!entry->dest_is_reg);
      assert(entry->dest.def.num_components ==
             entry->src.ssa->num_components);

      /* Since load_const instructions are SSA only, we can't replace their
       * destinations with registers and, therefore, can't coalesce them.
       */
      if (entry->src.ssa->parent_instr->type == nir_instr_type_load_const)
         continue;

      merge_node *src_node = get_merge_node(entry->src.ssa, state);
      merge_node *dest_node = get_merge_node(&entry->dest.def, state);

      if (src_node->set == dest_node->set)
         continue;

      /* TODO: We can probably do better here but for now we should be safe if
       * we just don't coalesce things with different divergence.
       */
      if (dest_node->set->divergent != src_node->set->divergent)
         continue;

      if (!merge_sets_interfere(src_node->set, dest_node->set))
         merge_merge_sets(src_node->set, dest_node->set);
   }
}

static bool
aggressive_coalesce_block(nir_block *block, struct from_ssa_state *state)
{
   nir_parallel_copy_instr *start_pcopy = NULL;
   nir_foreach_instr(instr, block) {
      /* Phi nodes only ever come at the start of a block */
      if (instr->type != nir_instr_type_phi) {
         if (instr->type != nir_instr_type_parallel_copy)
            break; /* The parallel copy must be right after the phis */

         start_pcopy = nir_instr_as_parallel_copy(instr);

         aggressive_coalesce_parallel_copy(start_pcopy, state);

         break;
      }
   }

   nir_parallel_copy_instr *end_pcopy =
      get_parallel_copy_at_end_of_block(block);

   if (end_pcopy && end_pcopy != start_pcopy)
      aggressive_coalesce_parallel_copy(end_pcopy, state);

   return true;
}

static nir_def *
decl_reg_for_ssa_def(nir_builder *b, nir_def *def)
{
   return nir_decl_reg(b, def->num_components, def->bit_size, 0);
}

static void
set_reg_divergent(nir_def *reg, bool divergent)
{
   nir_intrinsic_instr *decl = nir_reg_get_decl(reg);
   nir_intrinsic_set_divergent(decl, divergent);
}

void
nir_rewrite_uses_to_load_reg(nir_builder *b, nir_def *old,
                             nir_def *reg)
{
   nir_foreach_use_including_if_safe(use, old) {
      b->cursor = nir_before_src(use);

      /* If this is a parallel copy, it can just take the register directly */
      if (!nir_src_is_if(use) &&
          nir_src_parent_instr(use)->type == nir_instr_type_parallel_copy) {

         nir_parallel_copy_entry *copy_entry =
            list_entry(use, nir_parallel_copy_entry, src);

         assert(!copy_entry->src_is_reg);
         copy_entry->src_is_reg = true;
         nir_src_rewrite(&copy_entry->src, reg);
         continue;
      }

      /* If the immediate preceding instruction is a load_reg from the same
       * register, use it instead of creating a new load_reg. This helps when
       * a register is referenced in multiple sources in the same instruction,
       * which otherwise would turn into piles of unnecessary moves.
       */
      nir_def *load = NULL;
      if (b->cursor.option == nir_cursor_before_instr) {
         nir_instr *prev = nir_instr_prev(b->cursor.instr);

         if (prev != NULL && prev->type == nir_instr_type_intrinsic) {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(prev);
            if (intr->intrinsic == nir_intrinsic_load_reg &&
                intr->src[0].ssa == reg &&
                nir_intrinsic_base(intr) == 0)
               load = &intr->def;
         }
      }

      if (load == NULL)
         load = nir_load_reg(b, reg);

      nir_src_rewrite(use, load);
   }
}

static bool
def_replace_with_reg(nir_def *def, nir_function_impl *impl)
{
   /* These are handled elsewhere */
   assert(def->parent_instr->type != nir_instr_type_undef &&
          def->parent_instr->type != nir_instr_type_load_const);

   nir_builder b = nir_builder_create(impl);

   nir_def *reg = decl_reg_for_ssa_def(&b, def);
   nir_rewrite_uses_to_load_reg(&b, def, reg);

   if (def->parent_instr->type == nir_instr_type_phi)
      b.cursor = nir_before_block_after_phis(def->parent_instr->block);
   else
      b.cursor = nir_after_instr(def->parent_instr);

   nir_store_reg(&b, def, reg);
   return true;
}

static nir_def *
reg_for_ssa_def(nir_def *def, struct from_ssa_state *state)
{
   struct hash_entry *entry =
      _mesa_hash_table_search(state->merge_node_table, def);
   if (entry) {
      /* In this case, we're part of a phi web.  Use the web's register. */
      merge_node *node = (merge_node *)entry->data;

      /* If it doesn't have a register yet, create one.  Note that all of
       * the things in the merge set should be the same so it doesn't
       * matter which node's definition we use.
       */
      if (node->set->reg_decl == NULL) {
         node->set->reg_decl = decl_reg_for_ssa_def(&state->builder, def);
         set_reg_divergent(node->set->reg_decl, node->set->divergent);
      }

      return node->set->reg_decl;
   } else {
      assert(state->phi_webs_only);
      return NULL;
   }
}

static void
remove_no_op_phi(nir_instr *instr, struct from_ssa_state *state)
{
#ifndef NDEBUG
   nir_phi_instr *phi = nir_instr_as_phi(instr);

   struct hash_entry *entry =
      _mesa_hash_table_search(state->merge_node_table, &phi->def);
   assert(entry != NULL);
   merge_node *node = (merge_node *)entry->data;

   nir_foreach_phi_src(src, phi) {
      if (nir_src_is_undef(src->src))
         continue;

      entry = _mesa_hash_table_search(state->merge_node_table, src->src.ssa);
      assert(entry != NULL);
      merge_node *src_node = (merge_node *)entry->data;
      assert(src_node->set == node->set);
   }
#endif

   nir_instr_remove(instr);
}

static bool
rewrite_ssa_def(nir_def *def, void *void_state)
{
   struct from_ssa_state *state = void_state;

   nir_def *reg = reg_for_ssa_def(def, state);
   if (reg == NULL)
      return true;

   assert(nir_def_is_unused(def));

   /* At this point we know a priori that this SSA def is part of a
    * nir_dest.  We can use exec_node_data to get the dest pointer.
    */
   assert(def->parent_instr->type != nir_instr_type_load_const);
   nir_store_reg(&state->builder, def, reg);

   state->progress = true;
   return true;
}

static bool
rewrite_src(nir_src *src, void *void_state)
{
   struct from_ssa_state *state = void_state;

   nir_def *reg = reg_for_ssa_def(src->ssa, state);
   if (reg == NULL)
      return true;

   nir_src_rewrite(src, nir_load_reg(&state->builder, reg));

   state->progress = true;
   return true;
}

/* Resolves ssa definitions to registers.  While we're at it, we also
 * remove phi nodes.
 */
static void
resolve_registers_impl(nir_function_impl *impl, struct from_ssa_state *state)
{
   nir_foreach_block_reverse(block, impl) {
      /* Remove successor phis in case there's a back edge. */
      for (unsigned i = 0; i < 2; i++) {
         nir_block *succ = block->successors[i];
         if (succ == NULL)
            continue;

         nir_foreach_instr_safe(instr, succ) {
            if (instr->type != nir_instr_type_phi)
               break;

            remove_no_op_phi(instr, state);
         }
      }

      /* The following if is right after the block, handle its condition as the
       * last source "in" the block.
       */
      nir_if *nif = nir_block_get_following_if(block);
      if (nif) {
         state->builder.cursor = nir_before_src(&nif->condition);
         rewrite_src(&nif->condition, state);
      }

      nir_foreach_instr_reverse_safe(instr, block) {
         switch (instr->type) {
         case nir_instr_type_phi:
            remove_no_op_phi(instr, state);
            break;

         case nir_instr_type_parallel_copy: {
            nir_parallel_copy_instr *pcopy = nir_instr_as_parallel_copy(instr);

            nir_foreach_parallel_copy_entry(entry, pcopy) {
               assert(!entry->dest_is_reg);

               /* Parallel copy destinations will always be registers */
               nir_def *reg = reg_for_ssa_def(&entry->dest.def, state);
               assert(reg != NULL);

               /* We're switching from the nir_def to the nir_src in the dest
                * union so we need to use nir_instr_init_src() here.
                */
               assert(nir_def_is_unused(&entry->dest.def));
               entry->dest_is_reg = true;
               nir_instr_init_src(&pcopy->instr, &entry->dest.reg, reg);
            }

            nir_foreach_parallel_copy_entry(entry, pcopy) {
               assert(!entry->src_is_reg);
               nir_def *reg = reg_for_ssa_def(entry->src.ssa, state);
               if (reg == NULL)
                  continue;

               entry->src_is_reg = true;
               nir_src_rewrite(&entry->src, reg);
            }
            break;
         }

         default:
            state->builder.cursor = nir_after_instr(instr);
            nir_foreach_def(instr, rewrite_ssa_def, state);
            state->builder.cursor = nir_before_instr(instr);
            nir_foreach_src(instr, rewrite_src, state);
         }
      }
   }
}

/* Resolves a single parallel copy operation into a sequence of movs
 *
 * This is based on Algorithm 1 from "Revisiting Out-of-SSA Translation for
 * Correctness, Code Quality, and Efficiency" by Boissinot et al.
 * However, I never got the algorithm to work as written, so this version
 * is slightly modified.
 *
 * The algorithm works by playing this little shell game with the values.
 * We start by recording where every source value is and which source value
 * each destination value should receive.  We then grab any copy whose
 * destination is "empty", i.e. not used as a source, and do the following:
 *  - Find where its source value currently lives
 *  - Emit the move instruction
 *  - Set the location of the source value to the destination
 *  - Mark the location containing the source value
 *  - Mark the destination as no longer needing to be copied
 *
 * When we run out of "empty" destinations, we have a cycle and so we
 * create a temporary register, copy to that register, and mark the value
 * we copied as living in that temporary.  Now, the cycle is broken, so we
 * can continue with the above steps.
 */
struct copy_value {
   bool is_reg;
   nir_def *ssa;
};

static bool
copy_values_equal(struct copy_value a, struct copy_value b)
{
   return a.is_reg == b.is_reg && a.ssa == b.ssa;
}

static bool
copy_value_is_divergent(struct copy_value v)
{
   if (!v.is_reg)
      return v.ssa->divergent;

   nir_intrinsic_instr *decl = nir_reg_get_decl(v.ssa);
   return nir_intrinsic_divergent(decl);
}

static void
copy_values(nir_builder *b, struct copy_value dest, struct copy_value src)
{
   nir_def *val = src.is_reg ? nir_load_reg(b, src.ssa) : src.ssa;

   assert(!copy_value_is_divergent(src) || copy_value_is_divergent(dest));

   assert(dest.is_reg);
   nir_store_reg(b, val, dest.ssa);
}

static void
resolve_parallel_copy(nir_parallel_copy_instr *pcopy,
                      struct from_ssa_state *state)
{
   unsigned num_copies = 0;
   nir_foreach_parallel_copy_entry(entry, pcopy) {
      /* Sources may be SSA but destinations are always registers */
      assert(entry->dest_is_reg);
      if (entry->src_is_reg && entry->src.ssa == entry->dest.reg.ssa)
         continue;

      num_copies++;
   }

   if (num_copies == 0) {
      /* Hooray, we don't need any copies! */
      nir_instr_remove(&pcopy->instr);
      exec_list_push_tail(&state->dead_instrs, &pcopy->instr.node);
      return;
   }

   /* The register/source corresponding to the given index */
   NIR_VLA_ZERO(struct copy_value, values, num_copies * 2);

   /* The current location of a given piece of data.  We will use -1 for "null" */
   NIR_VLA_FILL(int, loc, num_copies * 2, -1);

   /* The piece of data that the given piece of data is to be copied from.  We will use -1 for "null" */
   NIR_VLA_FILL(int, pred, num_copies * 2, -1);

   /* The destinations we have yet to properly fill */
   NIR_VLA(int, to_do, num_copies * 2);
   int to_do_idx = -1;

   state->builder.cursor = nir_before_instr(&pcopy->instr);

   /* Now we set everything up:
    *  - All values get assigned a temporary index
    *  - Current locations are set from sources
    *  - Predecessors are recorded from sources and destinations
    */
   int num_vals = 0;
   nir_foreach_parallel_copy_entry(entry, pcopy) {
      /* Sources may be SSA but destinations are always registers */
      if (entry->src_is_reg && entry->src.ssa == entry->dest.reg.ssa)
         continue;

      struct copy_value src_value = {
         .is_reg = entry->src_is_reg,
         .ssa = entry->src.ssa,
      };

      int src_idx = -1;
      for (int i = 0; i < num_vals; ++i) {
         if (copy_values_equal(values[i], src_value))
            src_idx = i;
      }
      if (src_idx < 0) {
         src_idx = num_vals++;
         values[src_idx] = src_value;
      }

      assert(entry->dest_is_reg);
      struct copy_value dest_value = {
         .is_reg = true,
         .ssa = entry->dest.reg.ssa,
      };

      int dest_idx = -1;
      for (int i = 0; i < num_vals; ++i) {
         if (copy_values_equal(values[i], dest_value)) {
            /* Each destination of a parallel copy instruction should be
             * unique.  A destination may get used as a source, so we still
             * have to walk the list.  However, the predecessor should not,
             * at this point, be set yet, so we should have -1 here.
             */
            assert(pred[i] == -1);
            dest_idx = i;
         }
      }
      if (dest_idx < 0) {
         dest_idx = num_vals++;
         values[dest_idx] = dest_value;
      }

      loc[src_idx] = src_idx;
      pred[dest_idx] = src_idx;

      to_do[++to_do_idx] = dest_idx;
   }

   /* Currently empty destinations we can go ahead and fill */
   NIR_VLA(int, ready, num_copies * 2);
   int ready_idx = -1;

   /* Mark the ones that are ready for copying.  We know an index is a
    * destination if it has a predecessor and it's ready for copying if
    * it's not marked as containing data.
    */
   for (int i = 0; i < num_vals; i++) {
      if (pred[i] != -1 && loc[i] == -1)
         ready[++ready_idx] = i;
   }

   while (1) {
      while (ready_idx >= 0) {
         int b = ready[ready_idx--];
         int a = pred[b];
         copy_values(&state->builder, values[b], values[loc[a]]);

         /* b has been filled, mark it as not needing to be copied */
         pred[b] = -1;

         /* The next bit only applies if the source and destination have the
          * same divergence.  If they differ (it must be convergent ->
          * divergent), then we can't guarantee we won't need the convergent
          * version of it again.
          */
         if (copy_value_is_divergent(values[a]) ==
             copy_value_is_divergent(values[b])) {
            /* If a needs to be filled... */
            if (pred[a] != -1) {
               /* If any other copies want a they can find it at b */
               loc[a] = b;

               /* It's ready for copying now */
               ready[++ready_idx] = a;
            }
         }
      }

      assert(ready_idx < 0);
      if (to_do_idx < 0)
         break;

      int b = to_do[to_do_idx--];
      if (pred[b] == -1)
         continue;

      /* If we got here, then we don't have any more trivial copies that we
       * can do.  We have to break a cycle, so we create a new temporary
       * register for that purpose.  Normally, if going out of SSA after
       * register allocation, you would want to avoid creating temporary
       * registers.  However, we are going out of SSA before register
       * allocation, so we would rather not create extra register
       * dependencies for the backend to deal with.  If it wants, the
       * backend can coalesce the (possibly multiple) temporaries.
       *
       * We can also get here in the case where there is no cycle but our
       * source value is convergent, is also used as a destination by another
       * element of the parallel copy, and all the destinations of the
       * parallel copy which copy from it are divergent. In this case, the
       * above loop cannot detect that the value has moved due to all the
       * divergent destinations and we'll end up emitting a copy to a
       * temporary which never gets used. We can avoid this with additional
       * tracking or we can just trust the back-end to dead-code the unused
       * temporary (which is trivial).
       */
      assert(num_vals < num_copies * 2);
      nir_def *reg;
      if (values[b].is_reg) {
         nir_intrinsic_instr *decl = nir_reg_get_decl(values[b].ssa);
         uint8_t num_components = nir_intrinsic_num_components(decl);
         uint8_t bit_size = nir_intrinsic_bit_size(decl);
         reg = nir_decl_reg(&state->builder, num_components, bit_size, 0);
      } else {
         reg = decl_reg_for_ssa_def(&state->builder, values[b].ssa);
      }
      set_reg_divergent(reg, copy_value_is_divergent(values[b]));

      values[num_vals] = (struct copy_value){
         .is_reg = true,
         .ssa = reg,
      };
      copy_values(&state->builder, values[num_vals], values[b]);
      loc[b] = num_vals;
      ready[++ready_idx] = b;
      num_vals++;
   }

   nir_instr_remove(&pcopy->instr);
   exec_list_push_tail(&state->dead_instrs, &pcopy->instr.node);
}

/* Resolves the parallel copies in a block.  Each block can have at most
 * two:  One at the beginning, right after all the phi noces, and one at
 * the end (or right before the final jump if it exists).
 */
static bool
resolve_parallel_copies_block(nir_block *block, struct from_ssa_state *state)
{
   /* At this point, we have removed all of the phi nodes.  If a parallel
    * copy existed right after the phi nodes in this block, it is now the
    * first instruction.
    */
   nir_instr *first_instr = nir_block_first_instr(block);
   if (first_instr == NULL)
      return true; /* Empty, nothing to do. */

   /* There can be load_reg in the way of the copies... don't be clever. */
   nir_foreach_instr_safe(instr, block) {
      if (instr->type == nir_instr_type_parallel_copy) {
         nir_parallel_copy_instr *pcopy = nir_instr_as_parallel_copy(instr);

         resolve_parallel_copy(pcopy, state);
      }
   }

   return true;
}

static bool
nir_convert_from_ssa_impl(nir_function_impl *impl,
                          bool phi_webs_only)
{
   nir_shader *shader = impl->function->shader;

   struct from_ssa_state state;

   state.builder = nir_builder_create(impl);
   state.dead_ctx = ralloc_context(NULL);
   state.phi_webs_only = phi_webs_only;
   state.merge_node_table = _mesa_pointer_hash_table_create(NULL);
   state.progress = false;
   exec_list_make_empty(&state.dead_instrs);

   nir_foreach_block(block, impl) {
      add_parallel_copy_to_end_of_block(shader, block, state.dead_ctx);
   }

   nir_foreach_block(block, impl) {
      isolate_phi_nodes_block(shader, block, state.dead_ctx);
   }

   /* Mark metadata as dirty before we ask for liveness analysis */
   nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);

   nir_metadata_require(impl, nir_metadata_instr_index |
                                 nir_metadata_live_defs |
                                 nir_metadata_dominance);

   nir_foreach_block(block, impl) {
      coalesce_phi_nodes_block(block, &state);
   }

   nir_foreach_block(block, impl) {
      aggressive_coalesce_block(block, &state);
   }

   resolve_registers_impl(impl, &state);

   nir_foreach_block(block, impl) {
      resolve_parallel_copies_block(block, &state);
   }

   nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);

   /* Clean up dead instructions and the hash tables */
   nir_instr_free_list(&state.dead_instrs);
   _mesa_hash_table_destroy(state.merge_node_table, NULL);
   ralloc_free(state.dead_ctx);
   return state.progress;
}

bool
nir_convert_from_ssa(nir_shader *shader,
                     bool phi_webs_only)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      progress |= nir_convert_from_ssa_impl(impl, phi_webs_only);
   }

   return progress;
}

static void
place_phi_read(nir_builder *b, nir_def *reg,
               nir_def *def, nir_block *block, struct set *visited_blocks)
{
   /* Search already visited blocks to avoid back edges in tree */
   if (_mesa_set_search(visited_blocks, block) == NULL) {
      /* Try to go up the single-successor tree */
      bool all_single_successors = true;
      set_foreach(block->predecessors, entry) {
         nir_block *pred = (nir_block *)entry->key;
         if (pred->successors[0] && pred->successors[1]) {
            all_single_successors = false;
            break;
         }
      }

      if (all_single_successors) {
         /* All predecessors of this block have exactly one successor and it
          * is this block so they must eventually lead here without
          * intersecting each other.  Place the reads in the predecessors
          * instead of this block.
          */
         _mesa_set_add(visited_blocks, block);

         set_foreach(block->predecessors, entry) {
            place_phi_read(b, reg, def, (nir_block *)entry->key, visited_blocks);
         }
         return;
      }
   }

   b->cursor = nir_after_block_before_jump(block);
   nir_store_reg(b, def, reg);
}

/** Lower all of the phi nodes in a block to movs to and from a register
 *
 * This provides a very quick-and-dirty out-of-SSA pass that you can run on a
 * single block to convert all of its phis to a register and some movs.
 * The code that is generated, while not optimal for actual codegen in a
 * back-end, is easy to generate, correct, and will turn into the same set of
 * phis after you call regs_to_ssa and do some copy propagation.  For each phi
 * node we do the following:
 *
 *  1. For each phi instruction in the block, create a new nir_register
 *
 *  2. Insert movs at the top of the destination block for each phi and
 *     rewrite all uses of the phi to use the mov.
 *
 *  3. For each phi source, insert movs in the predecessor block from the phi
 *     source to the register associated with the phi.
 *
 * Correctness is guaranteed by the fact that we create a new register for
 * each phi and emit movs on both sides of the control-flow edge.  Because all
 * the phis have SSA destinations (we assert this) and there is a separate
 * temporary for each phi, all movs inserted in any particular block have
 * unique destinations so the order of operations does not matter.
 *
 * The one intelligent thing this pass does is that it places the moves from
 * the phi sources as high up the predecessor tree as possible instead of in
 * the exact predecessor.  This means that, in particular, it will crawl into
 * the deepest nesting of any if-ladders.  In order to ensure that doing so is
 * safe, it stops as soon as one of the predecessors has multiple successors.
 */
bool
nir_lower_phis_to_regs_block(nir_block *block)
{
   nir_builder b = nir_builder_create(nir_cf_node_get_function(&block->cf_node));
   struct set *visited_blocks = _mesa_set_create(NULL, _mesa_hash_pointer,
                                                 _mesa_key_pointer_equal);

   bool progress = false;
   nir_foreach_phi_safe(phi, block) {
      nir_def *reg = decl_reg_for_ssa_def(&b, &phi->def);

      b.cursor = nir_after_instr(&phi->instr);
      nir_def_rewrite_uses(&phi->def, nir_load_reg(&b, reg));

      nir_foreach_phi_src(src, phi) {

         _mesa_set_add(visited_blocks, src->src.ssa->parent_instr->block);
         place_phi_read(&b, reg, src->src.ssa, src->pred, visited_blocks);
         _mesa_set_clear(visited_blocks, NULL);
      }

      nir_instr_remove(&phi->instr);

      progress = true;
   }

   _mesa_set_destroy(visited_blocks, NULL);

   return progress;
}

struct ssa_def_to_reg_state {
   nir_function_impl *impl;
   bool progress;
};

static bool
def_replace_with_reg_state(nir_def *def, void *void_state)
{
   struct ssa_def_to_reg_state *state = void_state;
   state->progress |= def_replace_with_reg(def, state->impl);
   return true;
}

static bool
ssa_def_is_local_to_block(nir_def *def, UNUSED void *state)
{
   nir_block *block = def->parent_instr->block;
   nir_foreach_use_including_if(use_src, def) {
      if (nir_src_is_if(use_src) ||
          nir_src_parent_instr(use_src)->block != block ||
          nir_src_parent_instr(use_src)->type == nir_instr_type_phi) {
         return false;
      }
   }

   return true;
}

static bool
instr_is_load_new_reg(nir_instr *instr, unsigned old_num_ssa)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *load = nir_instr_as_intrinsic(instr);
   if (load->intrinsic != nir_intrinsic_load_reg)
      return false;

   nir_def *reg = load->src[0].ssa;

   return reg->index >= old_num_ssa;
}

/** Lower all of the SSA defs in a block to registers
 *
 * This performs the very simple operation of blindly replacing all of the SSA
 * defs in the given block with registers.  If not used carefully, this may
 * result in phi nodes with register sources which is technically invalid.
 * Fortunately, the register-based into-SSA pass handles them anyway.
 */
bool
nir_lower_ssa_defs_to_regs_block(nir_block *block)
{
   nir_function_impl *impl = nir_cf_node_get_function(&block->cf_node);
   nir_builder b = nir_builder_create(impl);

   struct ssa_def_to_reg_state state = {
      .impl = impl,
      .progress = false,
   };

   /* Save off the current number of SSA defs so we can detect which regs
    * we've added vs. regs that were already there.
    */
   const unsigned num_ssa = impl->ssa_alloc;

   nir_foreach_instr_safe(instr, block) {
      if (instr->type == nir_instr_type_undef) {
         /* Undefs are just a read of something never written. */
         nir_undef_instr *undef = nir_instr_as_undef(instr);
         nir_def *reg = decl_reg_for_ssa_def(&b, &undef->def);
         nir_rewrite_uses_to_load_reg(&b, &undef->def, reg);
      } else if (instr->type == nir_instr_type_load_const) {
         nir_load_const_instr *load = nir_instr_as_load_const(instr);
         nir_def *reg = decl_reg_for_ssa_def(&b, &load->def);
         nir_rewrite_uses_to_load_reg(&b, &load->def, reg);

         b.cursor = nir_after_instr(instr);
         nir_store_reg(&b, &load->def, reg);
      } else if (instr_is_load_new_reg(instr, num_ssa)) {
         /* Calls to nir_rewrite_uses_to_load_reg() may place new load_reg
          * intrinsics in this block with new SSA destinations.  To avoid
          * infinite recursion, we don't want to lower any newly placed
          * load_reg instructions to yet anoter load/store_reg.
          */
      } else if (nir_foreach_def(instr, ssa_def_is_local_to_block, NULL)) {
         /* If the SSA def produced by this instruction is only in the block
          * in which it is defined and is not used by ifs or phis, then we
          * don't have a reason to convert it to a register.
          */
      } else {
         nir_foreach_def(instr, def_replace_with_reg_state, &state);
      }
   }

   return state.progress;
}
