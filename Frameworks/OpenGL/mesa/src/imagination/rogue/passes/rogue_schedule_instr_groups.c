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
#include "rogue_builder.h"
#include "util/macros.h"

#include <stdbool.h>

/**
 * \file rogue_schedule_instr_groups.c
 *
 * \brief Contains the rogue_schedule_instr_groups pass.
 */

static inline void rogue_set_io_sel(rogue_instr_group_io_sel *map,
                                    enum rogue_alu alu,
                                    enum rogue_io io,
                                    rogue_ref *ref,
                                    bool is_dst)
{
   /* Skip unassigned I/Os. */
   if (rogue_ref_is_io_none(ref))
      return;

   /* Early skip I/Os that have already been assigned (e.g. for grouping). */
   if (rogue_ref_is_io(ref) && rogue_ref_get_io(ref) == io)
      return;

   if (alu == ROGUE_ALU_MAIN) {
      /* Hookup feedthrough outputs to W0 using IS4. */
      if (is_dst && rogue_io_is_ft(io)) {
         if (io == ROGUE_IO_FTE) {
            *(rogue_instr_group_io_sel_ref(map, ROGUE_IO_IS5)) =
               rogue_ref_io(io);
            io = ROGUE_IO_W1;
         } else {
            *(rogue_instr_group_io_sel_ref(map, ROGUE_IO_IS4)) =
               rogue_ref_io(io);
            io = ROGUE_IO_W0;
         }
      }

      /* Movc source. */
      /* TODO: hardcoded to use fte and s1 for now. */
      if (!is_dst && io == ROGUE_IO_FTE) {
         enum rogue_io src = ROGUE_IO_S1;
         *(rogue_instr_group_io_sel_ref(map, ROGUE_IO_IS0)) = rogue_ref_io(src);
         *(rogue_instr_group_io_sel_ref(map, ROGUE_IO_IS4)) = rogue_ref_io(io);
         io = src;
      }

      /* Pack source */
      if (!is_dst && io == ROGUE_IO_IS3) {
         enum rogue_io src = ROGUE_IO_S0;
         *(rogue_instr_group_io_sel_ref(map, ROGUE_IO_IS0)) = rogue_ref_io(src);
         *(rogue_instr_group_io_sel_ref(map, ROGUE_IO_IS3)) =
            rogue_ref_io(ROGUE_IO_FTE);
         io = src;
      }

      /* w0/w1 used as sources. */
      if (!is_dst && rogue_io_is_dst(io)) {
         enum rogue_io dst_ft =
            (io == ROGUE_IO_W0 ? ROGUE_IO_IS4 : ROGUE_IO_IS5);
         enum rogue_io src = ROGUE_IO_S0;
         *(rogue_instr_group_io_sel_ref(map, dst_ft)) =
            rogue_ref_io(ROGUE_IO_FTE);
         *(rogue_instr_group_io_sel_ref(map, ROGUE_IO_IS0)) = rogue_ref_io(src);
         io = src;
      }

      /* ADD64 fourth source */
      if (!is_dst && io == ROGUE_IO_IS0) {
         enum rogue_io src = ROGUE_IO_S3;
         *(rogue_instr_group_io_sel_ref(map, io)) = rogue_ref_io(src);
         io = src;
      }

      /* Test source(s). */
      /* TODO: tidy up. */
      if (!is_dst && io == ROGUE_IO_IS1) {
         enum rogue_io src = ROGUE_IO_S0;
         enum rogue_io ft = ROGUE_IO_FT0;

         /* Already set up. */
         if (io != ft)
            *(rogue_instr_group_io_sel_ref(map, io)) = rogue_ref_io(ft);

         io = src;
      }

      if (!is_dst && io == ROGUE_IO_IS2) {
         enum rogue_io src = ROGUE_IO_S3;
         enum rogue_io ft = ROGUE_IO_FTE;

         *(rogue_instr_group_io_sel_ref(map, ROGUE_IO_IS0)) =
            rogue_ref_io(ROGUE_IO_S3);

         /* Already set up. */
         if (io != ft)
            *(rogue_instr_group_io_sel_ref(map, io)) = rogue_ref_io(ft);

         io = src;
      }
   } else if (alu == ROGUE_ALU_BITWISE) {
      /* TODO: This is temporary because we just have BYP0, do it properly. */
      if (is_dst)
         io = ROGUE_IO_W0;
   }

   /* Set if not already set. */
   if (rogue_ref_is_null(rogue_instr_group_io_sel_ref(map, io)))
      *(rogue_instr_group_io_sel_ref(map, io)) = *ref;
}

/* TODO NEXT: Abort if anything in sel map is already set. */
/* TODO NEXT: Assert that these are register refs being set. */

static void rogue_lower_alu_io(rogue_alu_instr *alu, rogue_instr_group *group)
{
   const rogue_alu_op_info *info = &rogue_alu_op_infos[alu->op];
   enum rogue_instr_phase phase = alu->instr.index;

   for (unsigned u = 0; u < info->num_dsts; ++u) {
      if (info->phase_io[phase].dst[u] == ROGUE_IO_INVALID)
         continue;

      rogue_set_io_sel(&group->io_sel,
                       group->header.alu,
                       info->phase_io[phase].dst[u],
                       &alu->dst[u].ref,
                       true);
      alu->dst[u].ref = rogue_ref_io(info->phase_io[phase].dst[u]);
   }

   for (unsigned u = 0; u < info->num_srcs; ++u) {
      if (info->phase_io[phase].src[u] == ROGUE_IO_INVALID)
         continue;

      rogue_set_io_sel(&group->io_sel,
                       group->header.alu,
                       info->phase_io[phase].src[u],
                       &alu->src[u].ref,
                       false);
      alu->src[u].ref = rogue_ref_io(info->phase_io[phase].src[u]);
   }
}

static void rogue_lower_backend_io(rogue_backend_instr *backend,
                                   rogue_instr_group *group)
{
   const rogue_backend_op_info *info = &rogue_backend_op_infos[backend->op];

   for (unsigned u = 0; u < info->num_dsts; ++u) {
      if (info->phase_io.dst[u] == ROGUE_IO_INVALID)
         continue;

      rogue_set_io_sel(&group->io_sel,
                       group->header.alu,
                       info->phase_io.dst[u],
                       &backend->dst[u].ref,
                       true);
      backend->dst[u].ref = rogue_ref_io(info->phase_io.dst[u]);
   }

   for (unsigned u = 0; u < info->num_srcs; ++u) {
      if (info->phase_io.src[u] == ROGUE_IO_INVALID)
         continue;

      rogue_set_io_sel(&group->io_sel,
                       group->header.alu,
                       info->phase_io.src[u],
                       &backend->src[u].ref,
                       false);
      backend->src[u].ref = rogue_ref_io(info->phase_io.src[u]);
   }
}

static void rogue_lower_ctrl_io(rogue_ctrl_instr *ctrl,
                                rogue_instr_group *group)
{
   /* TODO: Support control instructions with I/O. */
}

static void rogue_lower_bitwise_io(rogue_bitwise_instr *bitwise,
                                   rogue_instr_group *group)
{
   const rogue_bitwise_op_info *info = &rogue_bitwise_op_infos[bitwise->op];
   enum rogue_instr_phase phase = bitwise->instr.index;

   for (unsigned u = 0; u < info->num_dsts; ++u) {
      if (info->phase_io[phase].dst[u] == ROGUE_IO_INVALID)
         continue;

      rogue_set_io_sel(&group->io_sel,
                       group->header.alu,
                       info->phase_io[phase].dst[u],
                       &bitwise->dst[u].ref,
                       true);
      bitwise->dst[u].ref = rogue_ref_io(info->phase_io[phase].dst[u]);
   }

   for (unsigned u = 0; u < info->num_srcs; ++u) {
      if (info->phase_io[phase].src[u] == ROGUE_IO_INVALID)
         continue;

      rogue_set_io_sel(&group->io_sel,
                       group->header.alu,
                       info->phase_io[phase].src[u],
                       &bitwise->src[u].ref,
                       false);
      bitwise->src[u].ref = rogue_ref_io(info->phase_io[phase].src[u]);
   }
}

static void rogue_lower_instr_group_io(rogue_instr *instr,
                                       rogue_instr_group *group)
{
   switch (instr->type) {
   case ROGUE_INSTR_TYPE_ALU:
      rogue_lower_alu_io(rogue_instr_as_alu(instr), group);
      break;

   case ROGUE_INSTR_TYPE_BACKEND:
      rogue_lower_backend_io(rogue_instr_as_backend(instr), group);
      break;

   case ROGUE_INSTR_TYPE_CTRL:
      rogue_lower_ctrl_io(rogue_instr_as_ctrl(instr), group);
      break;

   case ROGUE_INSTR_TYPE_BITWISE:
      rogue_lower_bitwise_io(rogue_instr_as_bitwise(instr), group);
      break;

   default:
      unreachable("Unsupported instruction group type.");
   }
}

/* This function uses unreachables rather than asserts because some Rogue IR
 * instructions are pseudo-instructions that need lowering on certain cores, but
 * real instructions on others, so these mistakes are more likely to happen.
 */
static inline void rogue_instr_group_put(rogue_instr *instr,
                                         rogue_instr_group *group)
{
   uint64_t supported_phases = rogue_instr_supported_phases(instr);
   if (!supported_phases)
      unreachable("Can't schedule pseudo-instructions.");
   else if (!util_is_power_of_two_or_zero64(supported_phases))
      unreachable("Multi-phase instructions unsupported.");

   enum rogue_instr_phase phase =
      rogue_get_supported_phase(supported_phases, group->header.phases);
   if (phase == ROGUE_INSTR_PHASE_INVALID)
      unreachable("Failed to schedule group instruction.");

   /* Update phases. */
   instr->group = group;
   instr->index = phase;
   group->instrs[phase] = instr;
   group->header.phases |= BITFIELD_BIT(phase);

   /* Ensure we're not mixing and matching repeats! */
   assert(group->header.repeat == 0 || group->header.repeat == instr->repeat);

   /* Update repeat count. */
   group->header.repeat = instr->repeat;

   /* Set end flag. */
   group->header.end = instr->end;
   instr->end = false;

   /* Ensure we're not mixing and matching execution conditions! */
   assert(group->header.exec_cond == ROGUE_EXEC_COND_INVALID ||
          group->header.exec_cond == instr->exec_cond);

   /* Set conditional execution flag. */
   group->header.exec_cond = instr->exec_cond;
   instr->exec_cond = ROGUE_EXEC_COND_INVALID;

   /* Lower I/O to sources/destinations/ISS. */
   rogue_lower_instr_group_io(instr, group);
}

static inline void rogue_move_instr_to_group(rogue_instr *instr,
                                             rogue_instr_group *group)
{
   /* Remove instruction from block instructions list. */
   list_del(&instr->link);

   /* ralloc_steal instr context from block to instruction group */
   ralloc_steal(group, instr);

   /* Assign instruction to instruction group. */
   rogue_instr_group_put(instr, group);
}

static void rogue_lower_regs(rogue_shader *shader)
{
   rogue_foreach_reg (reg, shader, ROGUE_REG_CLASS_INTERNAL) {
      rogue_reg_rewrite(shader,
                        reg,
                        ROGUE_REG_CLASS_SPECIAL,
                        reg->index + ROGUE_INTERNAL0_OFFSET);
   }

   rogue_foreach_reg_safe (reg, shader, ROGUE_REG_CLASS_CONST) {
      rogue_reg_rewrite(shader, reg, ROGUE_REG_CLASS_SPECIAL, reg->index);
   }

   rogue_foreach_reg_safe (reg, shader, ROGUE_REG_CLASS_PIXOUT) {
      rogue_reg_rewrite(shader,
                        reg,
                        ROGUE_REG_CLASS_SPECIAL,
                        reg->index +
                           (reg->index < ROGUE_PIXOUT_GROUP
                               ? ROGUE_PIXOUT0_OFFSET
                               : (ROGUE_PIXOUT4_OFFSET - ROGUE_PIXOUT_GROUP)));
   }
}

static unsigned rogue_reg_bank_bits(const rogue_ref *ref)
{
   const rogue_reg *reg;

   if (rogue_ref_is_reg(ref))
      reg = ref->reg;
   else if (rogue_ref_is_regarray(ref))
      reg = ref->regarray->regs[0];
   else
      unreachable("Non-register reference.");

   unsigned bits = util_last_bit(rogue_reg_bank_encoding(reg->class));
   return !bits ? 1 : bits;
}

static unsigned rogue_reg_index_bits(const rogue_ref *ref)
{
   const rogue_reg *reg;

   if (rogue_ref_is_reg(ref))
      reg = ref->reg;
   else if (rogue_ref_is_regarray(ref))
      reg = ref->regarray->regs[0];
   else
      unreachable("Non-register reference.");

   unsigned bits = util_last_bit(reg->index);
   return !bits ? 1 : bits;
}

static void rogue_calc_dsts_size(rogue_instr_group *group)
{
   const rogue_instr_group_io_sel *io_sel = &group->io_sel;

   unsigned num_dsts = (!rogue_ref_is_null(&io_sel->dsts[0]) &&
                        !rogue_ref_is_io_none(&io_sel->dsts[0])) +
                       (!rogue_ref_is_null(&io_sel->dsts[1]) &&
                        !rogue_ref_is_io_none(&io_sel->dsts[1]));
   unsigned bank_bits[ROGUE_ISA_DSTS] = { 0 };
   unsigned index_bits[ROGUE_ISA_DSTS] = { 0 };

   if (!num_dsts) {
      return;
   } else if (num_dsts == 1) {
      const rogue_ref *dst_ref = !rogue_ref_is_null(&io_sel->dsts[0])
                                    ? &io_sel->dsts[0]
                                    : &io_sel->dsts[1];
      bank_bits[0] = rogue_reg_bank_bits(dst_ref);
      index_bits[0] = rogue_reg_index_bits(dst_ref);
   } else {
      bank_bits[0] = rogue_reg_bank_bits(&io_sel->dsts[0]);
      bank_bits[1] = rogue_reg_bank_bits(&io_sel->dsts[1]);
      index_bits[0] = rogue_reg_index_bits(&io_sel->dsts[0]);
      index_bits[1] = rogue_reg_index_bits(&io_sel->dsts[1]);
   }

   for (unsigned u = 0; u < ROGUE_REG_DST_VARIANTS; ++u) {
      const rogue_reg_dst_info *info = &rogue_reg_dst_infos[u];

      if ((info->num_dsts < num_dsts) || (info->bank_bits[0] < bank_bits[0]) ||
          (info->bank_bits[1] < bank_bits[1]) ||
          (info->index_bits[0] < index_bits[0]) ||
          (info->index_bits[1] < index_bits[1]))
         continue;

      group->encode_info.dst_index = u;
      group->size.dsts = info->bytes;
      group->size.total += group->size.dsts;
      return;
   }

   unreachable("Unable to encode instruction group dsts.");
}

static void rogue_calc_iss_size(rogue_instr_group *group)
{
   group->size.iss = (group->header.alu == ROGUE_ALU_MAIN);
   group->size.total += group->size.iss;
}

static void rogue_calc_srcs_size(rogue_instr_group *group, bool upper_srcs)
{
   const rogue_instr_group_io_sel *io_sel = &group->io_sel;
   unsigned mux_bits = 0;

   unsigned offset = upper_srcs ? 3 : 0;
   const rogue_reg_src_info *info_array =
      upper_srcs ? rogue_reg_upper_src_infos : rogue_reg_lower_src_infos;

   unsigned *src_index = upper_srcs ? &group->encode_info.upper_src_index
                                    : &group->encode_info.lower_src_index;
   unsigned *srcs = upper_srcs ? &group->size.upper_srcs
                               : &group->size.lower_srcs;

   /* Special case: some control instructions have no sources. */
   if (group->header.alu == ROGUE_ALU_CONTROL) {
      const rogue_ctrl_instr *ctrl =
         rogue_instr_as_ctrl(group->instrs[ROGUE_INSTR_PHASE_CTRL]);
      if (!rogue_ctrl_op_has_srcs(ctrl->op))
         return;
   } else if (!upper_srcs && group->header.alu == ROGUE_ALU_MAIN) {
      /* Special case, IS0 */
      if (rogue_ref_is_io(&io_sel->iss[0])) {
         switch (io_sel->iss[0].io) {
         case ROGUE_IO_S0:
            mux_bits = 0;
            break;
         case ROGUE_IO_S3:
            mux_bits = 1;
            break;
         case ROGUE_IO_S4:
            mux_bits = 2;
            break;
         case ROGUE_IO_S5:
            mux_bits = 2;
            break;
         case ROGUE_IO_S1:
            mux_bits = 3;
            break;
         case ROGUE_IO_S2:
            mux_bits = 3;
            break;

         default:
            unreachable("IS0 set to unsupported value.");
         }
      }
   }

   unsigned num_srcs = 1;
   if (!rogue_ref_is_null(&io_sel->srcs[2 + offset]))
      num_srcs = 3;
   else if (!rogue_ref_is_null(&io_sel->srcs[1 + offset]))
      num_srcs = 2;

   unsigned bank_bits[ROGUE_ISA_SRCS / 2] = { 0 };
   unsigned index_bits[ROGUE_ISA_SRCS / 2] = { 0 };

   for (unsigned u = 0; u < ARRAY_SIZE(bank_bits); ++u) {
      const rogue_ref *src = &io_sel->srcs[u + offset];
      if (rogue_ref_is_null(src))
         continue;

      bank_bits[u] = rogue_reg_bank_bits(src);
      index_bits[u] = rogue_reg_index_bits(src);
   }

   for (unsigned u = 0; u < ROGUE_REG_SRC_VARIANTS; ++u) {
      const rogue_reg_src_info *info = &info_array[u];

      if ((info->num_srcs < num_srcs) || (info->mux_bits < mux_bits) ||
          (info->bank_bits[0] < bank_bits[0]) ||
          (info->bank_bits[1] < bank_bits[1]) ||
          (info->bank_bits[2] < bank_bits[2]) ||
          (info->index_bits[0] < index_bits[0]) ||
          (info->index_bits[1] < index_bits[1]) ||
          (info->index_bits[2] < index_bits[2])) {
         continue;
      }

      *src_index = u;
      *srcs = info->bytes;
      group->size.total += *srcs;

      return;
   }

   unreachable("Unable to encode instruction group srcs.");
}

#define SM(src_mod) ROGUE_ALU_SRC_MOD_##src_mod
#define DM(dst_mod) ROGUE_ALU_DST_MOD_##dst_mod
#define OM(op_mod) ROGUE_ALU_OP_MOD_##op_mod
static void rogue_calc_alu_instrs_size(rogue_instr_group *group,
                                       rogue_alu_instr *alu,
                                       enum rogue_instr_phase phase)
{
   switch (alu->op) {
   /* TODO: All single source have 1 byte and optional extra byte w/ext,
    * commonise some of these when adding support for more single source
    * instructions.
    */
   case ROGUE_ALU_OP_MBYP:
      if (rogue_alu_src_mod_is_set(alu, 0, SM(NEG)) ||
          rogue_alu_src_mod_is_set(alu, 0, SM(ABS))) {
         group->size.instrs[phase] = 2;
      } else {
         group->size.instrs[phase] = 1;
      }
      break;

   case ROGUE_ALU_OP_FMUL:
      group->size.instrs[phase] = 1;
      break;

   case ROGUE_ALU_OP_FMAD:
      if (rogue_alu_op_mod_is_set(alu, OM(LP)) ||
          rogue_alu_src_mod_is_set(alu, 1, SM(ABS)) ||
          rogue_alu_src_mod_is_set(alu, 1, SM(NEG)) ||
          rogue_alu_src_mod_is_set(alu, 2, SM(FLR)) ||
          rogue_alu_src_mod_is_set(alu, 2, SM(ABS))) {
         group->size.instrs[phase] = 2;
      } else {
         group->size.instrs[phase] = 1;
      }
      break;

   case ROGUE_ALU_OP_TST:
      group->size.instrs[phase] = 1;

      if (rogue_alu_op_mod_is_set(alu, OM(L)) ||
          rogue_alu_op_mod_is_set(alu, OM(LE)) ||
          !rogue_alu_op_mod_is_set(alu, OM(F32)) ||
          rogue_alu_src_mod_is_set(alu, 0, SM(E1)) ||
          rogue_alu_src_mod_is_set(alu, 0, SM(E2)) ||
          rogue_alu_src_mod_is_set(alu, 0, SM(E3)) ||
          !rogue_phase_occupied(ROGUE_INSTR_PHASE_2_PCK,
                                group->header.phases)) {
         group->size.instrs[phase] = 2;
      }
      break;

   case ROGUE_ALU_OP_MOVC: {
      group->size.instrs[phase] = 1;

      bool e0 = rogue_alu_dst_mod_is_set(alu, 0, DM(E0));
      bool e1 = rogue_alu_dst_mod_is_set(alu, 0, DM(E1));
      bool e2 = rogue_alu_dst_mod_is_set(alu, 0, DM(E2));
      bool e3 = rogue_alu_dst_mod_is_set(alu, 0, DM(E3));
      bool eq = (e0 == e1) && (e0 == e2) && (e0 == e3);

      if ((!rogue_phase_occupied(ROGUE_INSTR_PHASE_2_TST,
                                 group->header.phases) &&
           !rogue_phase_occupied(ROGUE_INSTR_PHASE_2_PCK,
                                 group->header.phases)) ||
          !rogue_ref_is_io_ftt(&alu->src[0].ref) || !eq) {
         group->size.instrs[phase] = 2;
      }
      break;
   }

   case ROGUE_ALU_OP_PCK_U8888:
      group->size.instrs[phase] = 2;
      break;

   case ROGUE_ALU_OP_ADD64:
      group->size.instrs[phase] = 1;

      if (rogue_ref_is_io_p0(&alu->src[4].ref) ||
          rogue_alu_src_mod_is_set(alu, 0, SM(ABS)) ||
          rogue_alu_src_mod_is_set(alu, 0, SM(NEG)) ||
          rogue_alu_src_mod_is_set(alu, 1, SM(ABS)) ||
          rogue_alu_src_mod_is_set(alu, 1, SM(NEG)) ||
          rogue_alu_src_mod_is_set(alu, 2, SM(ABS)))
         group->size.instrs[phase] = 2;
      break;

   default:
      unreachable("Unsupported alu op.");
   }
}
#undef OM
#undef DM
#undef SM

#define OM(op_mod) BITFIELD64_BIT(ROGUE_BACKEND_OP_MOD_##op_mod)
static bool rogue_backend_cachemode_is_set(const rogue_backend_instr *backend)
{
   return !!(backend->mod & (OM(BYPASS) | OM(FORCELINEFILL) | OM(WRITETHROUGH) |
                             OM(WRITEBACK) | OM(LAZYWRITEBACK)));
}

static bool
rogue_backend_slccachemode_is_set(const rogue_backend_instr *backend)
{
   return !!(backend->mod & (OM(SLCBYPASS) | OM(SLCWRITEBACK) |
                             OM(SLCWRITETHROUGH) | OM(SLCNOALLOC)));
}
#undef OM

#define OM(op_mod) ROGUE_BACKEND_OP_MOD_##op_mod
static void rogue_calc_backend_instrs_size(rogue_instr_group *group,
                                           rogue_backend_instr *backend,
                                           enum rogue_instr_phase phase)
{
   switch (backend->op) {
   case ROGUE_BACKEND_OP_FITR_PIXEL:
   case ROGUE_BACKEND_OP_FITRP_PIXEL:
      group->size.instrs[phase] = 2;
      break;

   case ROGUE_BACKEND_OP_UVSW_WRITETHENEMITTHENENDTASK:
   case ROGUE_BACKEND_OP_UVSW_WRITE:
      group->size.instrs[phase] = 2;
      break;

   case ROGUE_BACKEND_OP_UVSW_EMIT:
   case ROGUE_BACKEND_OP_UVSW_ENDTASK:
   case ROGUE_BACKEND_OP_UVSW_EMITTHENENDTASK:
      group->size.instrs[phase] = 1;
      break;

   case ROGUE_BACKEND_OP_LD:
      group->size.instrs[phase] = 2;

      if (rogue_ref_is_val(&backend->src[1].ref) ||
          rogue_backend_slccachemode_is_set(backend)) {
         group->size.instrs[phase] = 3;
      }
      break;

   case ROGUE_BACKEND_OP_ST:
      group->size.instrs[phase] = 3;

      if (rogue_backend_op_mod_is_set(backend, OM(TILED)) ||
          rogue_backend_slccachemode_is_set(backend) ||
          !rogue_ref_is_io_none(&backend->src[5].ref)) {
         group->size.instrs[phase] = 4;
      }
      break;

   case ROGUE_BACKEND_OP_SMP1D:
   case ROGUE_BACKEND_OP_SMP2D:
   case ROGUE_BACKEND_OP_SMP3D:
      group->size.instrs[phase] = 2;

      if (rogue_backend_op_mod_is_set(backend, OM(ARRAY))) {
         group->size.instrs[phase] = 5;
      } else if (rogue_backend_op_mod_is_set(backend, OM(WRT)) ||
                 rogue_backend_op_mod_is_set(backend, OM(SCHEDSWAP)) ||
                 rogue_backend_op_mod_is_set(backend, OM(F16)) ||
                 rogue_backend_cachemode_is_set(backend) ||
                 rogue_backend_slccachemode_is_set(backend)) {
         group->size.instrs[phase] = 4;
      } else if (rogue_backend_op_mod_is_set(backend, OM(TAO)) ||
                 rogue_backend_op_mod_is_set(backend, OM(SOO)) ||
                 rogue_backend_op_mod_is_set(backend, OM(SNO)) ||
                 rogue_backend_op_mod_is_set(backend, OM(NNCOORDS)) ||
                 rogue_backend_op_mod_is_set(backend, OM(DATA)) ||
                 rogue_backend_op_mod_is_set(backend, OM(INFO)) ||
                 rogue_backend_op_mod_is_set(backend, OM(BOTH)) ||
                 rogue_backend_op_mod_is_set(backend, OM(PROJ)) ||
                 rogue_backend_op_mod_is_set(backend, OM(PPLOD))) {
         group->size.instrs[phase] = 3;
      }
      break;

   case ROGUE_BACKEND_OP_IDF:
      group->size.instrs[phase] = 2;
      break;

   case ROGUE_BACKEND_OP_EMITPIX:
      group->size.instrs[phase] = 1;
      break;

   default:
      unreachable("Unsupported backend op.");
   }
}
#undef OM

static void rogue_calc_ctrl_instrs_size(rogue_instr_group *group,
                                        rogue_ctrl_instr *ctrl,
                                        enum rogue_instr_phase phase)
{
   switch (ctrl->op) {
   case ROGUE_CTRL_OP_NOP:
      group->size.instrs[phase] = 1;
      break;

   case ROGUE_CTRL_OP_WOP:
      group->size.instrs[phase] = 0;
      break;

   case ROGUE_CTRL_OP_BR:
   case ROGUE_CTRL_OP_BA:
      group->size.instrs[phase] = 5;
      break;

   case ROGUE_CTRL_OP_WDF:
      group->size.instrs[phase] = 0;
      break;

   default:
      unreachable("Unsupported ctrl op.");
   }
}

static void rogue_calc_bitwise_instrs_size(rogue_instr_group *group,
                                           rogue_bitwise_instr *bitwise,
                                           enum rogue_instr_phase phase)
{
   switch (bitwise->op) {
   case ROGUE_BITWISE_OP_BYP0:
      group->size.instrs[phase] = 1;

      if (rogue_ref_is_val(&bitwise->src[1].ref)) {
         group->size.instrs[phase] = 3;

         /* If upper 16 bits aren't zero. */
         if (rogue_ref_get_val(&bitwise->src[1].ref) & 0xffff0000)
            group->size.instrs[phase] = 5;
      }
      break;

   default:
      unreachable("Invalid bitwise op.");
   }
}

static void rogue_calc_instrs_size(rogue_instr_group *group)
{
   rogue_foreach_phase_in_set (p, group->header.phases) {
      const rogue_instr *instr = group->instrs[p];

      switch (instr->type) {
      case ROGUE_INSTR_TYPE_ALU:
         rogue_calc_alu_instrs_size(group, rogue_instr_as_alu(instr), p);
         break;

      case ROGUE_INSTR_TYPE_BACKEND:
         rogue_calc_backend_instrs_size(group,
                                        rogue_instr_as_backend(instr),
                                        p);
         break;

      case ROGUE_INSTR_TYPE_CTRL:
         rogue_calc_ctrl_instrs_size(group, rogue_instr_as_ctrl(instr), p);
         break;

      case ROGUE_INSTR_TYPE_BITWISE:
         rogue_calc_bitwise_instrs_size(group,
                                        rogue_instr_as_bitwise(instr),
                                        p);
         break;

      default:
         unreachable("Unsupported instruction type.");
      }

      group->size.total += group->size.instrs[p];
   }
}

static void rogue_calc_header_size(rogue_instr_group *group)
{
   group->size.header = 2;
   if (group->header.alu != ROGUE_ALU_MAIN ||
       (group->header.end || group->header.repeat > 1 ||
        group->header.exec_cond > ROGUE_EXEC_COND_P0_TRUE)) {
      group->size.header = 3;
   }

   group->size.total += group->size.header;
}

static void rogue_calc_padding_size(rogue_instr_group *group)
{
   group->size.word_padding = (group->size.total % 2);
   group->size.total += group->size.word_padding;
}

static void rogue_finalise_instr_group(rogue_instr_group *group)
{
   rogue_calc_dsts_size(group);
   rogue_calc_iss_size(group);
   rogue_calc_srcs_size(group, true);
   rogue_calc_srcs_size(group, false);
   rogue_calc_instrs_size(group);
   rogue_calc_header_size(group);
   rogue_calc_padding_size(group);
}

static void rogue_finalise_shader_offsets(rogue_shader *shader)
{
   rogue_instr_group *penultimate_group = NULL;
   rogue_instr_group *last_group = NULL;

   /* Set instruction group offsets. */
   unsigned offset = 0;
   rogue_foreach_instr_group_in_shader (group, shader) {
      group->size.offset = offset;
      offset += group->size.total;

      penultimate_group = last_group;
      last_group = group;
   }

   /* Ensure the final instruction group has a total size and offset that are a
    * multiple of the icache alignment. */
   unsigned total_align = last_group->size.total % ROGUE_ISA_ICACHE_ALIGN;
   unsigned offset_align = last_group->size.offset % ROGUE_ISA_ICACHE_ALIGN;

   if (total_align) {
      unsigned padding = ROGUE_ISA_ICACHE_ALIGN - total_align;
      /* Pad the size of the last instruction. */
      last_group->size.align_padding += padding;
      last_group->size.total += padding;
   }

   if (offset_align) {
      unsigned padding = ROGUE_ISA_ICACHE_ALIGN - offset_align;
      /* Pad the size of the penultimate instruction. */
      penultimate_group->size.align_padding += padding;
      penultimate_group->size.total += padding;
      /* Update the offset of the last instruction. */
      last_group->size.offset += padding;
   }
}

/* TODO: This just puts single instructions into groups for now. Later we need
 * to:
 * - create rules for what instructions can be co-issued/groups.
 * - schedule/shuffle instructions to get them ready for grouping (also need to
 * implement ways to stop certain instructions being rearranged, etc. first!)
 */
PUBLIC
bool rogue_schedule_instr_groups(rogue_shader *shader, bool multi_instr_groups)
{
   if (shader->is_grouped)
      return false;

   if (multi_instr_groups) {
      unreachable("Multi instruction groups are unsupported.");
      return false;
   }

   rogue_lower_regs(shader);

   rogue_instr_group *group;
   bool grouping = false;
   unsigned g = 0;
   rogue_foreach_block (block, shader) {
      struct list_head instr_groups;
      list_inithead(&instr_groups);

      rogue_foreach_instr_in_block_safe (instr, block) {
         enum rogue_alu group_alu = ROGUE_ALU_INVALID;
         switch (instr->type) {
         case ROGUE_INSTR_TYPE_ALU:
         case ROGUE_INSTR_TYPE_BACKEND:
            group_alu = ROGUE_ALU_MAIN;
            break;

         case ROGUE_INSTR_TYPE_CTRL:
            group_alu = ROGUE_ALU_CONTROL;
            break;

         case ROGUE_INSTR_TYPE_BITWISE:
            group_alu = ROGUE_ALU_BITWISE;
            break;

         default:
            unreachable("Unsupported instruction type.");
         }

         if (!grouping) {
            group = rogue_instr_group_create(block, group_alu);
            group->index = g++;
         }

         assert(group_alu == group->header.alu);
         rogue_move_instr_to_group(instr, group);

         grouping = instr->group_next;

         if (!grouping) {
            rogue_finalise_instr_group(group);
            list_addtail(&group->link, &instr_groups);
         }
      }

      list_replace(&instr_groups, &block->instrs);
   }

   shader->next_instr = g;
   shader->is_grouped = true;

   rogue_finalise_shader_offsets(shader);

   return true;
}
