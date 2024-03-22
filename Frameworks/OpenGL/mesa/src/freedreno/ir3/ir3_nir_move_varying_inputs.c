/*
 * Copyright © 2019 Red Hat
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

#include "compiler/nir/nir_builder.h"
#include "ir3_nir.h"

/**
 * This pass moves varying fetches (and the instructions they depend on
 * into the start block.
 *
 * We need to set the (ei) "end input" flag on the last varying fetch.
 * And we want to ensure that all threads execute the instruction that
 * sets (ei).  The easiest way to ensure this is to move all varying
 * fetches into the start block.  Which is something we used to get for
 * free by using lower_all_io_to_temps=true.
 *
 * This may come at the cost of additional register usage.  OTOH setting
 * the (ei) flag earlier probably frees up more VS to run.
 *
 * Not all varying fetches could be pulled into the start block.
 * If there are fetches we couldn't pull, like load_interpolated_input
 * with offset which depends on a non-reorderable ssbo load or on a
 * phi node, this pass is skipped since it would be hard to find a place
 * to set (ei) flag (beside at the very end).
 * a5xx and a6xx do automatically release varying storage at the end.
 */

typedef struct {
   nir_block *start_block;
   bool precondition_failed;
} precond_state;

typedef struct {
   nir_shader *shader;
   nir_block *start_block;
} state;

static void check_precondition_instr(precond_state *state, nir_instr *instr);
static void move_instruction_to_start_block(state *state, nir_instr *instr);

static bool
check_precondition_src(nir_src *src, void *state)
{
   check_precondition_instr(state, src->ssa->parent_instr);
   return true;
}

/* Recursively check if there is even a single dependency which
 * cannot be moved.
 */
static void
check_precondition_instr(precond_state *state, nir_instr *instr)
{
   if (instr->block == state->start_block)
      return;

   switch (instr->type) {
   case nir_instr_type_alu:
   case nir_instr_type_deref:
   case nir_instr_type_load_const:
   case nir_instr_type_undef:
      /* These could be safely moved around */
      break;
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
      if (!nir_intrinsic_can_reorder(intr)) {
         state->precondition_failed = true;
         return;
      }
      break;
   }
   default:
      state->precondition_failed = true;
      return;
   }

   nir_foreach_src(instr, check_precondition_src, state);
}

static void
check_precondition_block(precond_state *state, nir_block *block)
{
   nir_foreach_instr_safe (instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

      switch (intr->intrinsic) {
      case nir_intrinsic_load_interpolated_input:
      case nir_intrinsic_load_input:
         break;
      default:
         continue;
      }

      check_precondition_instr(state, instr);

      if (state->precondition_failed)
         return;
   }
}

static bool
move_src(nir_src *src, void *state)
{
   move_instruction_to_start_block(state, src->ssa->parent_instr);
   return true;
}

static void
move_instruction_to_start_block(state *state, nir_instr *instr)
{
   /* nothing to do if the instruction is already in the start block */
   if (instr->block == state->start_block)
      return;

   /* first move (recursively) all src's to ensure they appear before
    * load*_input that we are trying to move:
    */
   nir_foreach_src(instr, move_src, state);

   /* and then move the instruction itself:
    */
   exec_node_remove(&instr->node);
   exec_list_push_tail(&state->start_block->instr_list, &instr->node);
   instr->block = state->start_block;
}

static bool
move_varying_inputs_block(state *state, nir_block *block)
{
   bool progress = false;

   nir_foreach_instr_safe (instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

      switch (intr->intrinsic) {
      case nir_intrinsic_load_interpolated_input:
      case nir_intrinsic_load_input:
         /* TODO any others to handle? */
         break;
      default:
         continue;
      }

      move_instruction_to_start_block(state, instr);

      progress = true;
   }

   return progress;
}

bool
ir3_nir_move_varying_inputs(nir_shader *shader)
{
   bool progress = false;

   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   nir_foreach_function (function, shader) {
      precond_state state;

      if (!function->impl)
         continue;

      state.precondition_failed = false;
      state.start_block = nir_start_block(function->impl);

      nir_foreach_block (block, function->impl) {
         if (block == state.start_block)
            continue;

         check_precondition_block(&state, block);

         if (state.precondition_failed)
            return false;
      }
   }

   nir_foreach_function (function, shader) {
      state state;

      if (!function->impl)
         continue;

      state.shader = shader;
      state.start_block = nir_start_block(function->impl);

      bool progress = false;
      nir_foreach_block (block, function->impl) {
         /* don't need to move anything that is already in the first block */
         if (block == state.start_block)
            continue;
         progress |= move_varying_inputs_block(&state, block);
      }

      if (progress) {
         nir_metadata_preserve(
            function->impl, nir_metadata_block_index | nir_metadata_dominance);
      }
   }

   return progress;
}
