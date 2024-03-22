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

#include "compiler/shader_enums.h"
#include "rogue.h"
#include "util/bitscan.h"
#include "util/macros.h"

#include <inttypes.h>
#include <stdbool.h>

/**
 * \file rogue_print.c
 *
 * \brief Contains functions to print Rogue IR types and structures.
 */

/* TODO NEXT: Go through and make types the same, i.e. decide on using ONLY
 * unsigned, uint32/64_t, etc., and then use inttypes if so */
/* TODO NEXT: Make fp the last argument. */

enum color_esc {
   ESC_RESET = 0,
   ESC_BLACK,
   ESC_RED,
   ESC_GREEN,
   ESC_YELLOW,
   ESC_BLUE,
   ESC_PURPLE,
   ESC_CYAN,
   ESC_WHITE,

   ESC_COUNT,
};

static
const char *color_esc[2][ESC_COUNT] = {
   [0] = {
      [ESC_RESET] = "",
      [ESC_BLACK] = "",
      [ESC_RED] = "",
      [ESC_GREEN] = "",
      [ESC_YELLOW] = "",
      [ESC_BLUE] = "",
      [ESC_PURPLE] = "",
      [ESC_CYAN] = "",
      [ESC_WHITE] = "",
   },
   [1] = {
      [ESC_RESET] = "\033[0m",
      [ESC_BLACK] = "\033[0;30m",
      [ESC_RED] = "\033[0;31m",
      [ESC_GREEN] = "\033[0;32m",
      [ESC_YELLOW] = "\033[0;33m",
      [ESC_BLUE] = "\033[0;34m",
      [ESC_PURPLE] = "\033[0;35m",
      [ESC_CYAN] = "\033[0;36m",
      [ESC_WHITE] = "\033[0;37m",
   },
};

static inline void RESET(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_RESET], fp);
}

static inline void BLACK(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_BLACK], fp);
}

static inline void RED(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_RED], fp);
}

static inline void GREEN(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_GREEN], fp);
}

static inline void YELLOW(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_YELLOW], fp);
}

static inline void BLUE(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_BLUE], fp);
}

static inline void PURPLE(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_PURPLE], fp);
}

static inline void CYAN(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_CYAN], fp);
}

static inline void WHITE(FILE *fp)
{
   fputs(color_esc[rogue_color][ESC_WHITE], fp);
}

static inline void rogue_print_val(FILE *fp, unsigned val)
{
   PURPLE(fp);
   fprintf(fp, "%u", val);
   RESET(fp);
}

static inline void rogue_print_reg(FILE *fp, const rogue_reg *reg)
{
   const rogue_reg_info *info = &rogue_reg_infos[reg->class];
   YELLOW(fp);
   fprintf(fp, "%s%" PRIu32, info->str, reg->index);
   RESET(fp);
}

static inline void rogue_print_regarray(FILE *fp,
                                        const rogue_regarray *regarray)
{
   const rogue_reg *reg = regarray->regs[0];
   const rogue_reg_info *info = &rogue_reg_infos[reg->class];
   YELLOW(fp);
   fprintf(fp, "%s[%" PRIu32, info->str, reg->index);
   if (regarray->size > 1) {
      RESET(fp);
      fputs("..", fp);
      YELLOW(fp);
      fprintf(fp, "%" PRIu32, regarray->size + reg->index - 1);
   }
   fputs("]", fp);
   RESET(fp);
}

static inline void rogue_print_imm(FILE *fp, const rogue_imm *imm)
{
   PURPLE(fp);
   fprintf(fp, "0x%" PRIx32, imm->imm.u32);
   RESET(fp);
}

static inline void rogue_print_io(FILE *fp, enum rogue_io io)
{
   const rogue_io_info *info = &rogue_io_infos[io];
   BLUE(fp);
   fprintf(fp, "%s", info->str);
   RESET(fp);
}

static inline void rogue_print_drc(FILE *fp, const rogue_drc *drc)
{
   RED(fp);
   fprintf(fp, "drc%u", drc->index);
   RESET(fp);
}

static inline void rogue_print_ref(FILE *fp, const rogue_ref *ref)
{
   switch (ref->type) {
   case ROGUE_REF_TYPE_VAL:
      rogue_print_val(fp, ref->val);
      break;

   case ROGUE_REF_TYPE_REG:
      rogue_print_reg(fp, ref->reg);
      break;

   case ROGUE_REF_TYPE_REGARRAY:
      rogue_print_regarray(fp, ref->regarray);
      break;

   case ROGUE_REF_TYPE_IMM:
      rogue_print_imm(fp, &ref->imm);
      break;

   case ROGUE_REF_TYPE_IO:
      rogue_print_io(fp, ref->io);
      break;

   case ROGUE_REF_TYPE_DRC:
      rogue_print_drc(fp, &ref->drc);
      break;

   default:
      unreachable("Unsupported ref type.");
   }
}

static inline void rogue_print_alu_dst(FILE *fp, const rogue_instr_dst *dst)
{
   rogue_print_ref(fp, &dst->ref);

   uint64_t mod = dst->mod;
   while (mod) {
      enum rogue_alu_dst_mod dst_mod = u_bit_scan64(&mod);
      assert(dst_mod < ROGUE_ALU_DST_MOD_COUNT);
      fprintf(fp, ".%s", rogue_alu_dst_mod_infos[dst_mod].str);
   }
}

static inline void rogue_print_alu_src(FILE *fp, const rogue_instr_src *src)
{
   rogue_print_ref(fp, &src->ref);

   uint64_t mod = src->mod;
   while (mod) {
      enum rogue_alu_src_mod src_mod = u_bit_scan64(&mod);
      assert(src_mod < ROGUE_ALU_SRC_MOD_COUNT);
      fprintf(fp, ".%s", rogue_alu_src_mod_infos[src_mod].str);
   }
}

static inline void rogue_print_alu_mods(FILE *fp, const rogue_alu_instr *alu)
{
   uint64_t mod = alu->mod;
   while (mod) {
      enum rogue_alu_op_mod op_mod = u_bit_scan64(&mod);
      assert(op_mod < ROGUE_ALU_OP_MOD_COUNT);
      fprintf(fp, ".%s", rogue_alu_op_mod_infos[op_mod].str);
   }
}

static inline void rogue_print_alu_instr(FILE *fp, const rogue_alu_instr *alu)
{
   const rogue_alu_op_info *info = &rogue_alu_op_infos[alu->op];

   /* TODO: Print conditional info once supported. */

   fprintf(fp, "%s", info->str);

   rogue_print_alu_mods(fp, alu);

   for (unsigned i = 0; i < info->num_dsts; ++i) {
      if (i > 0)
         fputs(",", fp);

      fputs(" ", fp);

      rogue_print_alu_dst(fp, &alu->dst[i]);
   }

   for (unsigned i = 0; i < info->num_srcs; ++i) {
      if (i == 0 && !info->num_dsts)
         fputs(" ", fp);
      else
         fputs(", ", fp);

      rogue_print_alu_src(fp, &alu->src[i]);
   }
}

static inline void rogue_print_block_label(FILE *fp, const rogue_block *block)
{
   /* For debug purposes. */
   if (block->label)
      fprintf(fp, "%s", block->label);
   else
      fprintf(fp, "block%u", block->index);
}

static inline void rogue_print_backend_dst(FILE *fp, const rogue_instr_dst *dst)
{
   rogue_print_ref(fp, &dst->ref);
}

static inline void rogue_print_backend_src(FILE *fp, const rogue_instr_src *src)
{
   rogue_print_ref(fp, &src->ref);
}

static inline void rogue_print_backend_mods(FILE *fp,
                                            const rogue_backend_instr *backend)
{
   uint64_t mod = backend->mod;
   while (mod) {
      enum rogue_backend_op_mod op_mod = u_bit_scan64(&mod);
      assert(op_mod < ROGUE_BACKEND_OP_MOD_COUNT);
      fprintf(fp, ".%s", rogue_backend_op_mod_infos[op_mod].str);
   }
}

static inline void rogue_print_backend_instr(FILE *fp,
                                             const rogue_backend_instr *backend)
{
   const rogue_backend_op_info *info = &rogue_backend_op_infos[backend->op];

   fprintf(fp, "%s", info->str);

   rogue_print_backend_mods(fp, backend);

   for (unsigned i = 0; i < info->num_dsts; ++i) {
      if (i > 0)
         fputs(",", fp);

      fputs(" ", fp);

      rogue_print_backend_dst(fp, &backend->dst[i]);
   }

   for (unsigned i = 0; i < info->num_srcs; ++i) {
      if (i == 0 && !info->num_dsts)
         fputs(" ", fp);
      else
         fputs(", ", fp);

      rogue_print_backend_src(fp, &backend->src[i]);
   }
}

static inline void rogue_print_ctrl_mods(FILE *fp, const rogue_ctrl_instr *ctrl)
{
   uint64_t mod = ctrl->mod;
   while (mod) {
      enum rogue_ctrl_op_mod op_mod = u_bit_scan64(&mod);
      assert(op_mod < ROGUE_CTRL_OP_MOD_COUNT);
      fprintf(fp, ".%s", rogue_ctrl_op_mod_infos[op_mod].str);
   }
}

static inline void rogue_print_ctrl_src(FILE *fp, const rogue_instr_src *src)
{
   rogue_print_ref(fp, &src->ref);
}

static inline void rogue_print_ctrl_instr(FILE *fp,
                                          const rogue_ctrl_instr *ctrl)
{
   const rogue_ctrl_op_info *info = &rogue_ctrl_op_infos[ctrl->op];

   /* TODO: Print conditional info once supported. */

   fprintf(fp, "%s", info->str);

   rogue_print_ctrl_mods(fp, ctrl);

   if (ctrl->target_block) {
      fputs(" ", fp);
      rogue_print_block_label(fp, ctrl->target_block);
   }

   /* TODO NEXT: Dests. */
   /* TODO: Special case for the conditional ctrl instructions as they're
    * printed as source 0, then dest, then rest of the sources. */

   for (unsigned i = 0; i < info->num_srcs; ++i) {
      if (i == 0 && !info->num_dsts)
         fputs(" ", fp);
      else
         fputs(", ", fp);

      rogue_print_ctrl_src(fp, &ctrl->src[i]);
   }
}

static inline void rogue_print_bitwise_dst(FILE *fp, const rogue_instr_dst *dst)
{
   rogue_print_ref(fp, &dst->ref);
}

static inline void rogue_print_bitwise_src(FILE *fp, const rogue_instr_src *src)
{
   rogue_print_ref(fp, &src->ref);
}

static inline void rogue_print_bitwise_instr(FILE *fp,
                                             const rogue_bitwise_instr *bitwise)
{
   const rogue_bitwise_op_info *info = &rogue_bitwise_op_infos[bitwise->op];

   fprintf(fp, "%s", info->str);

   /* rogue_print_bitwise_mods(fp, bitwise); */

   for (unsigned i = 0; i < info->num_dsts; ++i) {
      if (i > 0)
         fputs(",", fp);

      fputs(" ", fp);

      rogue_print_bitwise_dst(fp, &bitwise->dst[i]);
   }

   for (unsigned i = 0; i < info->num_srcs; ++i) {
      if (i == 0 && !info->num_dsts)
         fputs(" ", fp);
      else
         fputs(", ", fp);

      rogue_print_bitwise_src(fp, &bitwise->src[i]);
   }
}

PUBLIC
void rogue_print_instr(FILE *fp, const rogue_instr *instr)
{
   if (instr->exec_cond > ROGUE_EXEC_COND_PE_TRUE)
      fprintf(fp, "%s ", rogue_exec_cond_str[instr->exec_cond]);

   if (instr->repeat > 1)
      fprintf(fp, "(rpt%u) ", instr->repeat);

   GREEN(fp);
   switch (instr->type) {
   case ROGUE_INSTR_TYPE_ALU:
      rogue_print_alu_instr(fp, rogue_instr_as_alu(instr));
      break;

   case ROGUE_INSTR_TYPE_BACKEND:
      rogue_print_backend_instr(fp, rogue_instr_as_backend(instr));
      break;

   case ROGUE_INSTR_TYPE_CTRL:
      rogue_print_ctrl_instr(fp, rogue_instr_as_ctrl(instr));
      break;

   case ROGUE_INSTR_TYPE_BITWISE:
      rogue_print_bitwise_instr(fp, rogue_instr_as_bitwise(instr));
      break;

   default:
      unreachable("Unsupported instruction type.");
   }
   RESET(fp);

   if (instr->end)
      fputs(" {end}", fp);

   /* For debug purposes. */
   fputs(";", fp);

   if (instr->comment)
      fprintf(fp, " /* %s */", instr->comment);
}

/* TODO NEXT: Split this up into separate functions for printing lower srcs,
 * upper srcs, etc. since we'd want to print them in-between instructions. */
/* TODO NEXT: Commonise with printing the ref io stuff. */
static inline void
rogue_print_instr_group_io_sel(FILE *fp, const rogue_instr_group_io_sel *io_sel)
{
   bool present = false;

   fputs(" ", fp);

   /* TODO NEXT: Commonise this code!! */
   /* Print upper and lower sources. */
   for (unsigned i = 0; i < ARRAY_SIZE(io_sel->srcs); ++i) {
      if (rogue_ref_is_null(&io_sel->srcs[i]))
         continue;

      if (present && i > 0)
         fputs(", ", fp);

      present = true;

      rogue_print_io(fp, ROGUE_IO_S0 + i);
      fputs("=", fp);

      if (rogue_ref_is_reg(&io_sel->srcs[i]))
         rogue_print_reg(fp, io_sel->srcs[i].reg);
      else if (rogue_ref_is_regarray(&io_sel->srcs[i]))
         rogue_print_regarray(fp, io_sel->srcs[i].regarray);
      else if (rogue_ref_is_io(&io_sel->srcs[i]))
         rogue_print_io(fp, io_sel->srcs[i].io);
      else
         unreachable("Unsupported src map.");
   }
   if (present)
      fputs(" ", fp);

   /* Print internal sources. */
   present = false;
   for (unsigned i = 0; i < ARRAY_SIZE(io_sel->iss); ++i) {
      if (rogue_ref_is_null(&io_sel->iss[i]))
         continue;

      if (present && i > 0)
         fputs(", ", fp);

      present = true;

      rogue_print_io(fp, ROGUE_IO_IS0 + i);
      fputs("=", fp);

      if (rogue_ref_is_reg(&io_sel->iss[i]))
         rogue_print_reg(fp, io_sel->iss[i].reg);
      else if (rogue_ref_is_regarray(&io_sel->iss[i]))
         rogue_print_regarray(fp, io_sel->iss[i].regarray);
      else if (rogue_ref_is_io(&io_sel->iss[i]))
         rogue_print_io(fp, io_sel->iss[i].io);
      else
         unreachable("Unsupported iss map.");
   }
   if (present)
      fputs(" ", fp);

   /* Print destinations. */
   present = false;
   for (unsigned i = 0; i < ARRAY_SIZE(io_sel->dsts); ++i) {
      if (rogue_ref_is_null(&io_sel->dsts[i]))
         continue;

      if (present && i > 0)
         fputs(", ", fp);

      present = true;

      rogue_print_io(fp, ROGUE_IO_W0 + i);
      fputs("=", fp);

      if (rogue_ref_is_reg(&io_sel->dsts[i]))
         rogue_print_reg(fp, io_sel->dsts[i].reg);
      else if (rogue_ref_is_regarray(&io_sel->dsts[i]))
         rogue_print_regarray(fp, io_sel->dsts[i].regarray);
      else if (rogue_ref_is_io(&io_sel->dsts[i]))
         rogue_print_io(fp, io_sel->dsts[i].io);
      else
         unreachable("Unsupported dst map.");
   }
   if (present)
      fputs(" ", fp);
}

static inline void rogue_print_instr_phase(FILE *fp,
                                           enum rogue_alu alu,
                                           enum rogue_instr_phase phase)
{
   const char *phase_str = rogue_instr_phase_str[alu][phase];
   assert(phase_str);
   fputs(phase_str, fp);
}

static inline void
rogue_print_instr_group_header(FILE *fp, const rogue_instr_group *group)
{
   /* ALU specific */
   switch (group->header.alu) {
   case ROGUE_ALU_MAIN:
      break;

   case ROGUE_ALU_BITWISE:
      break;

   case ROGUE_ALU_CONTROL:
      break;

   default:
      unreachable("Unsupported instruction group ALU.");
   }

   if (group->header.end)
      fputs(".end", fp);
}

static inline void rogue_print_instr_group(FILE *fp,
                                           const rogue_instr_group *group)
{
   /* For debug purposes. */
   fprintf(fp, "%u", group->index);
   fputs(": ", fp);

   if (group->header.exec_cond > ROGUE_EXEC_COND_PE_TRUE)
      fprintf(fp, "%s ", rogue_exec_cond_str[group->header.exec_cond]);

   if (group->header.repeat > 1)
      fprintf(fp, "(rpt%u) ", group->header.repeat);

   fputs("{ ", fp);

   CYAN(fp);
   fprintf(fp, "%s", rogue_alu_str[group->header.alu]);
   RESET(fp);

   /* Print each instruction. */
   rogue_foreach_phase_in_set (p, group->header.phases) {
      const rogue_instr *instr = group->instrs[p];
      assert(instr);

      fputs(" ", fp);
      rogue_print_instr_phase(fp, group->header.alu, p);
      fputs(": ", fp);
      rogue_print_instr(fp, instr);
   }

   /* Print source/dest mappings (if present). */
   rogue_print_instr_group_io_sel(fp, &group->io_sel);

   fputs("}", fp);

   /* Print group header info. */
   rogue_print_instr_group_header(fp, group);
}

static inline void rogue_print_block(FILE *fp, const rogue_block *block)
{
   rogue_print_block_label(fp, block);
   fputs(":\n", fp);

   if (!block->shader->is_grouped) {
      rogue_foreach_instr_in_block (instr, block) {
         fputs("\t", fp);
         fprintf(fp, "%u", instr->index);
         fputs(": ", fp);
         fprintf(fp, "%s: ", rogue_instr_type_str[instr->type]);
         rogue_print_instr(fp, instr);
         fputs("\n", fp);
      }
   } else {
      rogue_foreach_instr_group_in_block (group, block) {
         fputs("\t", fp);
         rogue_print_instr_group(fp, group);
         fputs("\n", fp);
      }
   }
}

PUBLIC
void rogue_print_shader(FILE *fp, const rogue_shader *shader)
{
   fputs("/*", fp);

   if (shader->stage == MESA_SHADER_NONE)
      fputs(" USC program", fp);
   else
      fprintf(fp, " %s shader", _mesa_shader_stage_to_string(shader->stage));

   if (shader->name)
      fprintf(fp, " - %s", shader->name);

   fputs(" */\n", fp);

   rogue_foreach_block (block, shader)
      rogue_print_block(fp, block);
}

static void rogue_print_instr_ref(FILE *fp,
                                  const rogue_instr *instr,
                                  bool dst,
                                  unsigned index,
                                  bool is_grouped)
{
   if (is_grouped) {
      fprintf(fp, "%u", instr->group->index);
      fputs(": { ", fp);
      rogue_print_instr_phase(fp, instr->group->header.alu, instr->index);
   } else {
      fprintf(fp, "%u", instr->index);
      if (index != ~0)
         fputs(": ", fp);
   }

   if (index != ~0) {
      BLUE(fp);
      fprintf(fp, "[%s%u]", dst ? "dst" : "src", index);
      RESET(fp);
   }

   if (is_grouped)
      fputs(" }", fp);
}

PUBLIC
void rogue_print_reg_writes(FILE *fp, const rogue_shader *shader)
{
   fputs("/* register writes */\n", fp);
   for (enum rogue_reg_class class = 0; class < ROGUE_REG_CLASS_COUNT;
        ++class) {
      rogue_foreach_reg (reg, shader, class) {
         bool unused = true;

         rogue_print_reg(fp, reg);
         fputs(":", fp);

         rogue_foreach_reg_write (write, reg) {
            assert(write->instr);
            unused = false;

            fputs(" ", fp);
            rogue_print_instr_ref(fp,
                                  write->instr,
                                  true,
                                  write->dst_index,
                                  shader->is_grouped);
         }

         if (reg->regarray) {
            rogue_foreach_regarray_write (write, reg->regarray) {
               assert(write->instr);
               unused = false;

               fputs(" ", fp);
               rogue_print_instr_ref(fp,
                                     write->instr,
                                     false,
                                     write->dst_index,
                                     shader->is_grouped);
            }

            rogue_foreach_subarray (subarray, reg->regarray) {
               unsigned subarray_start = subarray->regs[0]->index;
               unsigned subarray_end = subarray_start + subarray->size - 1;
               if (reg->index < subarray_start || reg->index > subarray_end)
                  continue;

               rogue_foreach_regarray_write (write, subarray) {
                  assert(write->instr);
                  unused = false;

                  fputs(" ", fp);
                  rogue_print_instr_ref(fp,
                                        write->instr,
                                        false,
                                        write->dst_index,
                                        shader->is_grouped);
               }
            }
         }

         if (unused) {
            fputs(" <none>\n", fp);
            continue;
         }

         fputs("\n", fp);
      }
   }
}

PUBLIC
void rogue_print_reg_uses(FILE *fp, const rogue_shader *shader)
{
   fputs("/* register uses */\n", fp);
   for (enum rogue_reg_class class = 0; class < ROGUE_REG_CLASS_COUNT;
        ++class) {
      rogue_foreach_reg (reg, shader, class) {
         bool unused = true;

         rogue_print_reg(fp, reg);
         fputs(":", fp);

         rogue_foreach_reg_use (use, reg) {
            assert(use->instr);
            unused = false;

            fputs(" ", fp);
            rogue_print_instr_ref(fp,
                                  use->instr,
                                  false,
                                  use->src_index,
                                  shader->is_grouped);
         }

         if (reg->regarray) {
            rogue_foreach_regarray_use (use, reg->regarray) {
               assert(use->instr);
               unused = false;

               fputs(" ", fp);
               rogue_print_instr_ref(fp,
                                     use->instr,
                                     false,
                                     use->src_index,
                                     shader->is_grouped);
            }

            rogue_foreach_subarray (subarray, reg->regarray) {
               unsigned subarray_start = subarray->regs[0]->index;
               unsigned subarray_end = subarray_start + subarray->size - 1;
               if (reg->index < subarray_start || reg->index > subarray_end)
                  continue;

               rogue_foreach_regarray_use (use, subarray) {
                  assert(use->instr);
                  unused = false;

                  fputs(" ", fp);
                  rogue_print_instr_ref(fp,
                                        use->instr,
                                        false,
                                        use->src_index,
                                        shader->is_grouped);
               }
            }
         }

         if (unused) {
            fputs(" <none>\n", fp);
            continue;
         }

         fputs("\n", fp);
      }
   }
}

PUBLIC
void rogue_print_block_uses(FILE *fp, const rogue_shader *shader)
{
   fputs("/* block uses */\n", fp);
   rogue_foreach_block (block, shader) {
      rogue_print_block_label(fp, block);
      fputs(":", fp);

      if (list_is_empty(&block->uses)) {
         if (list_first_entry(&shader->blocks, rogue_block, link) == block)
            fputs(" <entry>\n", fp);
         else
            fputs(" <none>\n", fp);

         continue;
      }

      rogue_foreach_block_use (use, block) {
         assert(use->instr);

         fputs(" ", fp);
         rogue_print_instr_ref(fp, use->instr, false, ~0, shader->is_grouped);
      }

      fputs("\n", fp);
   }
}

static void rogue_print_drc_trxn(FILE *fp,
                                 const rogue_shader *shader,
                                 const rogue_drc_trxn *drc_trxn,
                                 unsigned index)
{
   fprintf(fp, "drc%u: ack: ", index);

   rogue_print_instr_ref(fp, drc_trxn->acquire, false, ~0, shader->is_grouped);

   fputs(", rel: ", fp);

   if (drc_trxn->release) {
      rogue_print_instr_ref(fp,
                            drc_trxn->release,
                            false,
                            ~0,
                            shader->is_grouped);
   } else {
      fputs("<none>", fp);
   }

   fputs("\n", fp);
}

PUBLIC
void rogue_print_drc_trxns(FILE *fp, const rogue_shader *shader)
{
   fputs("/* DRC transactions */\n", fp);

   rogue_foreach_drc_trxn (drc_trxn, shader, 0) {
      rogue_print_drc_trxn(fp, shader, drc_trxn, 0);
   }

   rogue_foreach_drc_trxn (drc_trxn, shader, 1) {
      rogue_print_drc_trxn(fp, shader, drc_trxn, 1);
   }
}
