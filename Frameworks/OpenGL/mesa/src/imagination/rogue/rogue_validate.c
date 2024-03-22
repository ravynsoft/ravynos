/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "rogue.h"
#include "util/bitscan.h"
#include "util/macros.h"
#include "util/u_dynarray.h"

#include <stdbool.h>

/**
 * \file rogue_validate.c
 *
 * \brief Contains functions to validate Rogue IR.
 */

/* TODO: Rogue_validate should make sure that immediate (sources) don't have any
 * modifiers set... */

/* TODO NEXT: Check for emit/end/etc. as last instruction in vertex shader, and
 * nop.end, or end flag set (or just pseudo-end) otherwise. */

typedef struct rogue_validation_state {
   const rogue_shader *shader; /** The shader being validated. */
   const char *when; /** Description of the validation being done. */
   bool nonfatal; /** Don't stop at the first error.*/
   struct {
      const rogue_block *block; /** Current basic block being validated. */
      const rogue_instr *instr; /** Current instruction being validated. */
      const rogue_instr_group *group; /** Current instruction group being
                                         validated. */
      const rogue_ref *ref; /** Current reference being validated. */
      bool src; /** Current reference type (src/dst). */
      unsigned param; /** Current reference src/dst index. */
   } ctx;
   struct util_dynarray *error_msgs; /** Error message list. */
} rogue_validation_state;

/* Returns true if errors are present. */
static bool validate_print_errors(rogue_validation_state *state)
{
   if (!util_dynarray_num_elements(state->error_msgs, const char *))
      return false;

   util_dynarray_foreach (state->error_msgs, const char *, msg) {
      fprintf(stderr, "%s\n", *msg);
   }

   fputs("\n", stderr);

   rogue_print_shader(stderr, state->shader);
   fputs("\n", stderr);

   return true;
}

static void PRINTFLIKE(2, 3)
   validate_log(rogue_validation_state *state, const char *fmt, ...)
{
   char *msg = ralloc_asprintf(state->error_msgs, "Validation error");

   /* Add info about the item that was being validated. */

   if (state->ctx.block) {
      if (state->ctx.block->label)
         ralloc_asprintf_append(&msg, " block \"%s\"", state->ctx.block->label);
      else
         ralloc_asprintf_append(&msg, " block%u", state->ctx.block->index);
   }

   if (state->ctx.instr) {
      ralloc_asprintf_append(&msg, " instr %u", state->ctx.instr->index);
   }

   if (state->ctx.ref) {
      ralloc_asprintf_append(&msg,
                             " %s %u",
                             state->ctx.src ? "src" : "dst",
                             state->ctx.param);
   }

   ralloc_asprintf_append(&msg, ": ");

   va_list args;
   va_start(args, fmt);
   ralloc_vasprintf_append(&msg, fmt, args);
   util_dynarray_append(state->error_msgs, const char *, msg);
   va_end(args);

   if (!state->nonfatal) {
      validate_print_errors(state);
      abort();
   }
}

static rogue_validation_state *
create_validation_state(const rogue_shader *shader, const char *when)
{
   rogue_validation_state *state = rzalloc_size(shader, sizeof(*state));

   state->shader = shader;
   state->when = when;
   state->nonfatal = ROGUE_DEBUG(VLD_NONFATAL);

   state->error_msgs = rzalloc_size(state, sizeof(*state->error_msgs));
   util_dynarray_init(state->error_msgs, state);

   return state;
}

static void validate_regarray(rogue_validation_state *state,
                              rogue_regarray *regarray)
{
   if (!regarray->size) {
      validate_log(state, "Register array is empty.");
      return;
   }

   enum rogue_reg_class class = regarray->regs[0]->class;
   unsigned base_index = regarray->regs[0]->index;

   for (unsigned u = 0; u < regarray->size; ++u) {
      if (regarray->regs[u]->class != class)
         validate_log(state, "Register class mismatch in register array.");

      if (regarray->regs[u]->index != (base_index + u))
         validate_log(state, "Non-contiguous registers in register array.");
   }
}

static void validate_dst(rogue_validation_state *state,
                         const rogue_instr_dst *dst,
                         uint64_t supported_dst_types,
                         unsigned i,
                         unsigned stride,
                         unsigned repeat,
                         uint64_t repeat_mask)
{
   state->ctx.ref = &dst->ref;
   state->ctx.src = false;
   state->ctx.param = i;

   if (rogue_ref_is_null(&dst->ref))
      validate_log(state, "Destination has not been set.");

   if (!rogue_ref_type_supported(dst->ref.type, supported_dst_types))
      validate_log(state, "Unsupported destination type.");

   if (rogue_ref_is_reg_or_regarray(&dst->ref) && stride != ~0U) {
      unsigned dst_size = stride + 1;
      if (repeat_mask & (1 << i))
         dst_size *= repeat;

      if (rogue_ref_is_regarray(&dst->ref)) {
         if (rogue_ref_get_regarray_size(&dst->ref) != dst_size) {
            validate_log(state,
                         "Expected regarray size %u, got %u.",
                         dst_size,
                         rogue_ref_get_regarray_size(&dst->ref));
         }
      } else if (dst_size > 1) {
         validate_log(state, "Expected regarray type for destination.");
      }
   }

   state->ctx.ref = NULL;
}

static void validate_src(rogue_validation_state *state,
                         const rogue_instr_src *src,
                         uint64_t supported_src_types,
                         unsigned i,
                         unsigned stride,
                         unsigned repeat,
                         uint64_t repeat_mask)
{
   state->ctx.ref = &src->ref;
   state->ctx.src = true;
   state->ctx.param = i;

   if (rogue_ref_is_null(&src->ref))
      validate_log(state, "Source has not been set.");

   if (!rogue_ref_type_supported(src->ref.type, supported_src_types))
      validate_log(state, "Unsupported source type.");

   if (rogue_ref_is_reg_or_regarray(&src->ref) && stride != ~0U) {
      unsigned src_size = stride + 1;
      if (repeat_mask & (1 << i))
         src_size *= repeat;

      if (rogue_ref_is_regarray(&src->ref)) {
         if (rogue_ref_get_regarray_size(&src->ref) != src_size) {
            validate_log(state,
                         "Expected regarray size %u, got %u.",
                         src_size,
                         rogue_ref_get_regarray_size(&src->ref));
         }
      } else if (src_size > 1) {
         validate_log(state, "Expected regarray type for source.");
      }
   }

   state->ctx.ref = NULL;
}

static bool validate_alu_op_mod_combo(uint64_t mods)
{
   rogue_foreach_mod_in_set (mod, mods) {
      const rogue_alu_op_mod_info *info = &rogue_alu_op_mod_infos[mod];

      /* Check if any excluded op mods have been included. */
      if (info->exclude & mods)
         return false;

      /* Check if any required op mods have been missed. */
      if (info->require && !(info->require & mods))
         return false;
   }

   return true;
}

static void validate_alu_instr(rogue_validation_state *state,
                               const rogue_alu_instr *alu)
{
   if (alu->op == ROGUE_ALU_OP_INVALID || alu->op >= ROGUE_ALU_OP_COUNT)
      validate_log(state, "Unknown ALU op 0x%x encountered.", alu->op);

   const rogue_alu_op_info *info = &rogue_alu_op_infos[alu->op];

   /* Check if instruction modifiers are valid. */
   if (!rogue_mods_supported(alu->mod, info->supported_op_mods))
      validate_log(state, "Unsupported ALU op modifiers.");

   if (!validate_alu_op_mod_combo(alu->mod))
      validate_log(state, "Unsupported ALU op modifier combination.");

   /* Instruction repeat checks. */
   if (alu->instr.repeat > 1 && !info->dst_repeat_mask &&
       !info->src_repeat_mask) {
      validate_log(state, "Repeat set for ALU op without repeat support.");
   }

   /* Validate destinations and sources for ungrouped shaders. */
   if (!state->shader->is_grouped) {
      for (unsigned i = 0; i < info->num_dsts; ++i) {
         validate_dst(state,
                      &alu->dst[i],
                      info->supported_dst_types[i],
                      i,
                      info->dst_stride[i],
                      alu->instr.repeat,
                      info->dst_repeat_mask);
      }

      for (unsigned i = 0; i < info->num_srcs; ++i) {
         validate_src(state,
                      &alu->src[i],
                      info->supported_src_types[i],
                      i,
                      info->src_stride[i],
                      alu->instr.repeat,
                      info->src_repeat_mask);
      }
   }
}

static bool validate_backend_op_mod_combo(uint64_t mods)
{
   rogue_foreach_mod_in_set (mod, mods) {
      const rogue_backend_op_mod_info *info = &rogue_backend_op_mod_infos[mod];

      /* Check if any excluded op mods have been included. */
      if (info->exclude & mods)
         return false;

      /* Check if any required op mods have been missed. */
      if (info->require && !(info->require & mods))
         return false;
   }

   return true;
}

static void validate_backend_instr(rogue_validation_state *state,
                                   const rogue_backend_instr *backend)
{
   if (backend->op == ROGUE_BACKEND_OP_INVALID ||
       backend->op >= ROGUE_BACKEND_OP_COUNT)
      validate_log(state, "Unknown backend op 0x%x encountered.", backend->op);

   const rogue_backend_op_info *info = &rogue_backend_op_infos[backend->op];

   /* Check if instruction modifiers are valid. */
   if (!rogue_mods_supported(backend->mod, info->supported_op_mods))
      validate_log(state, "Unsupported backend op modifiers.");

   if (!validate_backend_op_mod_combo(backend->mod))
      validate_log(state, "Unsupported backend op modifier combination.");

   /* Instruction repeat checks. */
   if (backend->instr.repeat > 1 && !info->dst_repeat_mask &&
       !info->src_repeat_mask) {
      validate_log(state, "Repeat set for backend op without repeat support.");
   }

   /* Validate destinations and sources for ungrouped shaders. */
   if (!state->shader->is_grouped) {
      for (unsigned i = 0; i < info->num_dsts; ++i) {
         validate_dst(state,
                      &backend->dst[i],
                      info->supported_dst_types[i],
                      i,
                      info->dst_stride[i],
                      backend->instr.repeat,
                      info->dst_repeat_mask);
      }

      for (unsigned i = 0; i < info->num_srcs; ++i) {
         validate_src(state,
                      &backend->src[i],
                      info->supported_src_types[i],
                      i,
                      info->src_stride[i],
                      backend->instr.repeat,
                      info->src_repeat_mask);
      }
   }
}

static bool validate_ctrl_op_mod_combo(uint64_t mods)
{
   rogue_foreach_mod_in_set (mod, mods) {
      const rogue_ctrl_op_mod_info *info = &rogue_ctrl_op_mod_infos[mod];

      /* Check if any excluded op mods have been included. */
      if (info->exclude & mods)
         return false;

      /* Check if any required op mods have been missed. */
      if (info->require && !(info->require & mods))
         return false;
   }

   return true;
}

/* Returns true if instruction can end block. */
static bool validate_ctrl_instr(rogue_validation_state *state,
                                const rogue_ctrl_instr *ctrl)
{
   if (ctrl->op == ROGUE_CTRL_OP_INVALID || ctrl->op >= ROGUE_CTRL_OP_COUNT)
      validate_log(state, "Unknown ctrl op 0x%x encountered.", ctrl->op);

   /* TODO: Validate rest, check blocks, etc. */
   const rogue_ctrl_op_info *info = &rogue_ctrl_op_infos[ctrl->op];

   if (info->has_target && !ctrl->target_block)
      validate_log(state, "Ctrl op expected target block, but none provided.");
   else if (!info->has_target && ctrl->target_block)
      validate_log(state,
                   "Ctrl op did not expect target block, but one provided.");

   /* Check if instruction modifiers are valid. */
   if (!rogue_mods_supported(ctrl->mod, info->supported_op_mods))
      validate_log(state, "Unsupported CTRL op modifiers.");

   if (!validate_ctrl_op_mod_combo(ctrl->mod))
      validate_log(state, "Unsupported CTRL op modifier combination.");

   /* Instruction repeat checks. */
   if (ctrl->instr.repeat > 1 && !info->dst_repeat_mask &&
       !info->src_repeat_mask) {
      validate_log(state, "Repeat set for CTRL op without repeat support.");
   }

   /* Validate destinations and sources for ungrouped shaders. */
   if (!state->shader->is_grouped) {
      for (unsigned i = 0; i < info->num_dsts; ++i) {
         validate_dst(state,
                      &ctrl->dst[i],
                      info->supported_dst_types[i],
                      i,
                      info->dst_stride[i],
                      ctrl->instr.repeat,
                      info->dst_repeat_mask);
      }

      for (unsigned i = 0; i < info->num_srcs; ++i) {
         validate_src(state,
                      &ctrl->src[i],
                      info->supported_src_types[i],
                      i,
                      info->src_stride[i],
                      ctrl->instr.repeat,
                      info->src_repeat_mask);
      }
   }

   /* nop.end counts as a end-of-block instruction. */
   if (rogue_instr_is_nop_end(&ctrl->instr))
      return true;

   /* Control instructions have no end flag to set. */
   if (ctrl->instr.end)
      validate_log(state, "CTRL ops have no end flag.");

   return info->ends_block;
}

static bool validate_bitwise_op_mod_combo(uint64_t mods)
{
   rogue_foreach_mod_in_set (mod, mods) {
      const rogue_bitwise_op_mod_info *info = &rogue_bitwise_op_mod_infos[mod];

      /* Check if any excluded op mods have been included. */
      if (info->exclude & mods)
         return false;

      /* Check if any required op mods have been missed. */
      if (info->require && !(info->require & mods))
         return false;
   }

   return true;
}

static void validate_bitwise_instr(rogue_validation_state *state,
                                   const rogue_bitwise_instr *bitwise)
{
   if (bitwise->op == ROGUE_BITWISE_OP_INVALID ||
       bitwise->op >= ROGUE_BITWISE_OP_COUNT)
      validate_log(state, "Unknown bitwise op 0x%x encountered.", bitwise->op);

   const rogue_bitwise_op_info *info = &rogue_bitwise_op_infos[bitwise->op];

   /* Check if instruction modifiers are valid. */
   if (!rogue_mods_supported(bitwise->mod, info->supported_op_mods))
      validate_log(state, "Unsupported bitwise op modifiers.");

   if (!validate_bitwise_op_mod_combo(bitwise->mod))
      validate_log(state, "Unsupported bitwise op modifier combination.");

   /* Instruction repeat checks. */
   if (bitwise->instr.repeat > 1 && !info->dst_repeat_mask &&
       !info->src_repeat_mask) {
      validate_log(state, "Repeat set for bitwise op without repeat support.");
   }

   /* Validate destinations and sources for ungrouped shaders. */
   if (!state->shader->is_grouped) {
      for (unsigned i = 0; i < info->num_dsts; ++i) {
         validate_dst(state,
                      &bitwise->dst[i],
                      info->supported_dst_types[i],
                      i,
                      info->dst_stride[i],
                      bitwise->instr.repeat,
                      info->dst_repeat_mask);
      }

      for (unsigned i = 0; i < info->num_srcs; ++i) {
         validate_src(state,
                      &bitwise->src[i],
                      info->supported_src_types[i],
                      i,
                      info->src_stride[i],
                      bitwise->instr.repeat,
                      info->src_repeat_mask);
      }
   }
}

/* Returns true if instruction can end block. */
static bool validate_instr(rogue_validation_state *state,
                           const rogue_instr *instr)
{
   state->ctx.instr = instr;

   bool ends_block = false;

   switch (instr->type) {
   case ROGUE_INSTR_TYPE_ALU:
      validate_alu_instr(state, rogue_instr_as_alu(instr));
      break;

   case ROGUE_INSTR_TYPE_BACKEND:
      validate_backend_instr(state, rogue_instr_as_backend(instr));
      break;

   case ROGUE_INSTR_TYPE_CTRL:
      ends_block = validate_ctrl_instr(state, rogue_instr_as_ctrl(instr));
      break;

   case ROGUE_INSTR_TYPE_BITWISE:
      validate_bitwise_instr(state, rogue_instr_as_bitwise(instr));
      break;

   default:
      validate_log(state,
                   "Unknown instruction type 0x%x encountered.",
                   instr->type);
   }

   /* If the last instruction isn't control flow but has the end flag set, it
    * can end a block. */
   if (!ends_block)
      ends_block = instr->end;

   state->ctx.instr = NULL;

   return ends_block;
}

/* Returns true if instruction can end block. */
static bool validate_instr_group(rogue_validation_state *state,
                                 const rogue_instr_group *group)
{
   state->ctx.group = group;
   /* TODO: Validate group properties. */
   /* TODO: Check for pseudo-instructions. */

   bool ends_block = false;

   /* Validate instructions in group. */
   /* TODO: Check util_last_bit group_phases < bla bla */
   rogue_foreach_phase_in_set (p, group->header.phases) {
      const rogue_instr *instr = group->instrs[p];

      if (!instr)
         validate_log(state, "Missing instruction where phase was set.");

      /* TODO NEXT: Groups that have control instructions should only have a
       * single instruction. */
      ends_block = validate_instr(state, instr);
   }

   state->ctx.group = NULL;

   if (group->header.alu != ROGUE_ALU_CONTROL)
      return group->header.end;

   return ends_block;
}

static void validate_block(rogue_validation_state *state,
                           const rogue_block *block)
{
   /* TODO: Validate block properties. */
   state->ctx.block = block;

   if (list_is_empty(&block->instrs)) {
      validate_log(state, "Block is empty.");
      state->ctx.block = NULL;
      return;
   }

   unsigned block_ends = 0;
   struct list_head *block_end = NULL;
   struct list_head *last = block->instrs.prev;

   /* Validate instructions/groups in block. */
   if (!block->shader->is_grouped) {
      rogue_foreach_instr_in_block (instr, block) {
         bool ends_block = validate_instr(state, instr);
         block_ends += ends_block;
         block_end = ends_block ? &instr->link : block_end;
      }
   } else {
      rogue_foreach_instr_group_in_block (group, block) {
         bool ends_block = validate_instr_group(state, group);
         block_ends += ends_block;
         block_end = ends_block ? &group->link : block_end;
      }
   }

   if (!block_ends || block_ends > 1)
      validate_log(state,
                   "Block must end with a single control flow instruction.");
   else if (block_end != last)
      validate_log(
         state,
         "Control flow instruction is present prior to the end of the block.");

   state->ctx.block = NULL;
}

static void validate_reg_use(rogue_validation_state *state,
                             const rogue_reg_use *use,
                             uint64_t supported_io_srcs)
{
   /* No restrictions. */
   if (!supported_io_srcs)
      return;

   const rogue_instr *instr = use->instr;

   rogue_foreach_phase_in_set (p, rogue_instr_supported_phases(instr)) {
      enum rogue_io io_src = rogue_instr_src_io_src(instr, p, use->src_index);
      if (io_src == ROGUE_IO_INVALID)
         validate_log(state, "Register used where no source is present.");

      if (!rogue_io_supported(io_src, supported_io_srcs))
         validate_log(state,
                      "Register class unsupported in S%u.",
                      io_src - ROGUE_IO_S0); /* TODO: Either add info here to
                                                get register class and print as
                                                string, or add info to
                                                rogue_validation_state. */
   }
}

static void validate_reg_state(rogue_validation_state *state,
                               rogue_shader *shader)
{
   BITSET_WORD *regs_used = NULL;

   for (enum rogue_reg_class class = 0; class < ROGUE_REG_CLASS_COUNT;
        ++class) {
      const rogue_reg_info *info = &rogue_reg_infos[class];
      if (info->num)
         regs_used =
            rzalloc_size(state, sizeof(*regs_used) * BITSET_WORDS(info->num));

      rogue_foreach_reg (reg, shader, class) {
         /* Ensure that the range restrictions are satisfied. */
         if (info->num && reg->index >= info->num)
            validate_log(state, "%s register index out of range.", info->name);

         /* Ensure that only registers of this class are in the regs list. */
         if (reg->class != class)
            validate_log(state,
                         "%s register found in %s register list.",
                         rogue_reg_infos[reg->class].name,
                         info->name);

         /* Track the registers used in the class. */
         if (info->num)
            BITSET_SET(regs_used, reg->index);

         /* Check register cache entry. */
         rogue_reg **reg_cached =
            util_sparse_array_get(&shader->reg_cache[class], reg->index);
         if (!reg_cached || !*reg_cached)
            validate_log(state,
                         "Missing %s register %u cache entry.",
                         info->name,
                         reg->index);
         else if (*reg_cached != reg || (*reg_cached)->index != reg->index ||
                  (*reg_cached)->class != reg->class)
            validate_log(state,
                         "Mismatching %s register %u cache entry.",
                         info->name,
                         reg->index);
         else if (reg_cached != reg->cached)
            validate_log(state,
                         "Mismatching %s register %u cache entry pointer.",
                         info->name,
                         reg->index);

         /* Validate register uses. */
         const rogue_reg_info *reg_info = &rogue_reg_infos[class];
         rogue_foreach_reg_use (use, reg)
            validate_reg_use(state, use, reg_info->supported_io_srcs);
      }

      /* Check that the registers used matches the usage list. */
      if (info->num && memcmp(shader->regs_used[class],
                              regs_used,
                              sizeof(*regs_used) * BITSET_WORDS(info->num)))
         validate_log(state, "Incorrect %s register usage list.", info->name);

      ralloc_free(regs_used);
   }

   /* Check that SSA registers aren't being written to more than once. */
   rogue_foreach_reg (reg, shader, ROGUE_REG_CLASS_SSA)
      if (list_length(&reg->writes) > 1)
         validate_log(state,
                      "SSA register %u is written to more than once.",
                      reg->index);

   rogue_foreach_regarray (regarray, shader) {
      /* Validate regarray contents. */
      validate_regarray(state, regarray);

      /* Check regarray cache entry. */
      uint64_t key = rogue_regarray_cache_key(regarray->size,
                                              regarray->regs[0]->class,
                                              regarray->regs[0]->index,
                                              false,
                                              0);
      rogue_regarray **regarray_cached =
         util_sparse_array_get(&shader->regarray_cache, key);
      if (!regarray_cached || !*regarray_cached)
         validate_log(state, "Missing regarray cache entry.");
      else if (*regarray_cached != regarray ||
               (*regarray_cached)->size != regarray->size ||
               (*regarray_cached)->parent != regarray->parent ||
               (*regarray_cached)->regs != regarray->regs)
         validate_log(state, "Mismatching regarray cache entry.");
      else if (regarray_cached != regarray->cached)
         validate_log(state, "Mismatching regarray cache entry pointer.");

      if (regarray->parent && (regarray->parent->size <= regarray->size ||
                               regarray->parent->parent))
         validate_log(state, "Invalid sub-regarray.");
   }
}

PUBLIC
bool rogue_validate_shader(rogue_shader *shader, const char *when)
{
   if (ROGUE_DEBUG(VLD_SKIP))
      return true;

   bool errors_present;

   rogue_validation_state *state = create_validation_state(shader, when);

   validate_reg_state(state, shader);

   /* TODO: Ensure there is at least one block (with at least an end
    * instruction!) */
   rogue_foreach_block (block, shader)
      validate_block(state, block);

   errors_present = validate_print_errors(state);

   ralloc_free(state);

   return !errors_present;
}
