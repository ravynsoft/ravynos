/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <stdarg.h>
#include <stdio.h>

#include "util/log.h"
#include "ir3.h"

#define PTRID(x) ((unsigned long)(x))

/* ansi escape sequences: */
#define RESET   "\x1b[0m"
#define RED     "\x1b[0;31m"
#define GREEN   "\x1b[0;32m"
#define BLUE    "\x1b[0;34m"
#define MAGENTA "\x1b[0;35m"

/* syntax coloring, mostly to make it easier to see different sorts of
 * srcs (immediate, constant, ssa, array, ...)
 */
#define SYN_REG(x)   RED x RESET
#define SYN_IMMED(x) GREEN x RESET
#define SYN_CONST(x) GREEN x RESET
#define SYN_SSA(x)   BLUE x RESET
#define SYN_ARRAY(x) MAGENTA x RESET

static const char *
type_name(type_t type)
{
   static const char *type_names[] = {
      /* clang-format off */
      [TYPE_F16] = "f16",
      [TYPE_F32] = "f32",
      [TYPE_U16] = "u16",
      [TYPE_U32] = "u32",
      [TYPE_S16] = "s16",
      [TYPE_S32] = "s32",
      [TYPE_U8]  = "u8", 
      [TYPE_S8]  = "s8",
      /* clang-format on */
   };
   return type_names[type];
}

static void
print_instr_name(struct log_stream *stream, struct ir3_instruction *instr,
                 bool flags)
{
   if (!instr)
      return;
#ifdef DEBUG
   mesa_log_stream_printf(stream, "%04u:", instr->serialno);
#endif
   mesa_log_stream_printf(stream, "%04u:", instr->ip);
   if (instr->flags & IR3_INSTR_UNUSED) {
      mesa_log_stream_printf(stream, "XXX: ");
   } else {
      mesa_log_stream_printf(stream, "%03u: ", instr->use_count);
   }

   if (flags) {
      mesa_log_stream_printf(stream, "\t");
      if (instr->flags & IR3_INSTR_SY)
         mesa_log_stream_printf(stream, "(sy)");
      if (instr->flags & IR3_INSTR_SS)
         mesa_log_stream_printf(stream, "(ss)");
      if (instr->flags & IR3_INSTR_JP)
         mesa_log_stream_printf(stream, "(jp)");
      if (instr->repeat)
         mesa_log_stream_printf(stream, "(rpt%d)", instr->repeat);
      if (instr->nop)
         mesa_log_stream_printf(stream, "(nop%d)", instr->nop);
      if (instr->flags & IR3_INSTR_UL)
         mesa_log_stream_printf(stream, "(ul)");
   } else {
      mesa_log_stream_printf(stream, " ");
   }

   if (is_meta(instr)) {
      switch (instr->opc) {
      case OPC_META_INPUT:
         mesa_log_stream_printf(stream, "_meta:in");
         break;
      case OPC_META_SPLIT:
         mesa_log_stream_printf(stream, "_meta:split");
         break;
      case OPC_META_COLLECT:
         mesa_log_stream_printf(stream, "_meta:collect");
         break;
      case OPC_META_TEX_PREFETCH:
         mesa_log_stream_printf(stream, "_meta:tex_prefetch");
         break;
      case OPC_META_PARALLEL_COPY:
         mesa_log_stream_printf(stream, "_meta:parallel_copy");
         break;
      case OPC_META_PHI:
         mesa_log_stream_printf(stream, "_meta:phi");
         break;

      /* shouldn't hit here.. just for debugging: */
      default:
         mesa_log_stream_printf(stream, "_meta:%d", instr->opc);
         break;
      }
   } else if (opc_cat(instr->opc) == 1) {
      if (instr->opc == OPC_MOV) {
         if (instr->cat1.src_type == instr->cat1.dst_type)
            mesa_log_stream_printf(stream, "mov");
         else
            mesa_log_stream_printf(stream, "cov");
      } else {
         mesa_log_stream_printf(stream, "%s",
                                disasm_a3xx_instr_name(instr->opc));
      }

      if (instr->opc == OPC_SCAN_MACRO) {
         switch (instr->cat1.reduce_op) {
         case REDUCE_OP_ADD_U:
            mesa_log_stream_printf(stream, ".add.u");
            break;
         case REDUCE_OP_ADD_F:
            mesa_log_stream_printf(stream, ".add.f");
            break;
         case REDUCE_OP_MUL_U:
            mesa_log_stream_printf(stream, ".mul.u");
            break;
         case REDUCE_OP_MUL_F:
            mesa_log_stream_printf(stream, ".mul.f");
            break;
         case REDUCE_OP_MIN_U:
            mesa_log_stream_printf(stream, ".min.u");
            break;
         case REDUCE_OP_MIN_S:
            mesa_log_stream_printf(stream, ".min.s");
            break;
         case REDUCE_OP_MIN_F:
            mesa_log_stream_printf(stream, ".min.f");
            break;
         case REDUCE_OP_MAX_U:
            mesa_log_stream_printf(stream, ".max.u");
            break;
         case REDUCE_OP_MAX_S:
            mesa_log_stream_printf(stream, ".max.s");
            break;
         case REDUCE_OP_MAX_F:
            mesa_log_stream_printf(stream, ".max.f");
            break;
         case REDUCE_OP_AND_B:
            mesa_log_stream_printf(stream, ".and.b");
            break;
         case REDUCE_OP_OR_B:
            mesa_log_stream_printf(stream, ".or.b");
            break;
         case REDUCE_OP_XOR_B:
            mesa_log_stream_printf(stream, ".xor.b");
            break;
         }
      }

      if (instr->opc != OPC_MOVMSK && instr->opc != OPC_SCAN_MACRO &&
          instr->opc != OPC_PUSH_CONSTS_LOAD_MACRO) {
         mesa_log_stream_printf(stream, ".%s%s",
                                type_name(instr->cat1.src_type),
                                type_name(instr->cat1.dst_type));
      }
   } else if (instr->opc == OPC_B) {
      const char *name[8] = {
         /* clang-format off */
         [BRANCH_PLAIN] = "br",
         [BRANCH_OR]    = "brao",
         [BRANCH_AND]   = "braa",
         [BRANCH_CONST] = "brac",
         [BRANCH_ANY]   = "bany",
         [BRANCH_ALL]   = "ball",
         [BRANCH_X]     = "brax",
         /* clang-format on */
      };
      mesa_log_stream_printf(stream, "%s", name[instr->cat0.brtype]);
   } else {
      mesa_log_stream_printf(stream, "%s", disasm_a3xx_instr_name(instr->opc));
      if (instr->flags & IR3_INSTR_3D)
         mesa_log_stream_printf(stream, ".3d");
      if (instr->flags & IR3_INSTR_A)
         mesa_log_stream_printf(stream, ".a");
      if (instr->flags & IR3_INSTR_O)
         mesa_log_stream_printf(stream, ".o");
      if (instr->flags & IR3_INSTR_P)
         mesa_log_stream_printf(stream, ".p");
      if (instr->flags & IR3_INSTR_S)
         mesa_log_stream_printf(stream, ".s");
      if (instr->flags & IR3_INSTR_A1EN)
         mesa_log_stream_printf(stream, ".a1en");
      if (instr->opc == OPC_LDC)
         mesa_log_stream_printf(stream, ".offset%d", instr->cat6.d);
      if (instr->opc == OPC_LDC_K)
         mesa_log_stream_printf(stream, ".%d", instr->cat6.iim_val);
      if (instr->flags & IR3_INSTR_B) {
         mesa_log_stream_printf(
            stream, ".base%d",
            is_tex(instr) ? instr->cat5.tex_base : instr->cat6.base);
      }
      if (instr->flags & IR3_INSTR_S2EN)
         mesa_log_stream_printf(stream, ".s2en");

      static const char *cond[0x7] = {
         "lt", "le", "gt", "ge", "eq", "ne",
      };

      switch (instr->opc) {
      case OPC_CMPS_F:
      case OPC_CMPS_U:
      case OPC_CMPS_S:
      case OPC_CMPV_F:
      case OPC_CMPV_U:
      case OPC_CMPV_S:
         mesa_log_stream_printf(stream, ".%s",
                                cond[instr->cat2.condition & 0x7]);
         break;
      default:
         break;
      }
   }
}

static void
print_ssa_def_name(struct log_stream *stream, struct ir3_register *reg)
{
   mesa_log_stream_printf(stream, SYN_SSA("ssa_%u"), reg->instr->serialno);
   if (reg->name != 0)
      mesa_log_stream_printf(stream, ":%u", reg->name);
}

static void
print_ssa_name(struct log_stream *stream, struct ir3_register *reg, bool dst)
{
   if (!dst) {
      if (!reg->def)
         mesa_log_stream_printf(stream, SYN_SSA("undef"));
      else
         print_ssa_def_name(stream, reg->def);
   } else {
      print_ssa_def_name(stream, reg);
   }

   if (reg->num != INVALID_REG && !(reg->flags & IR3_REG_ARRAY))
      mesa_log_stream_printf(stream, "(" SYN_REG("r%u.%c") ")", reg_num(reg),
                             "xyzw"[reg_comp(reg)]);
}

static void
print_reg_name(struct log_stream *stream, struct ir3_instruction *instr,
               struct ir3_register *reg, bool dest)
{
   if ((reg->flags & (IR3_REG_FABS | IR3_REG_SABS)) &&
       (reg->flags & (IR3_REG_FNEG | IR3_REG_SNEG | IR3_REG_BNOT)))
      mesa_log_stream_printf(stream, "(absneg)");
   else if (reg->flags & (IR3_REG_FNEG | IR3_REG_SNEG | IR3_REG_BNOT))
      mesa_log_stream_printf(stream, "(neg)");
   else if (reg->flags & (IR3_REG_FABS | IR3_REG_SABS))
      mesa_log_stream_printf(stream, "(abs)");

   if (reg->flags & IR3_REG_FIRST_KILL)
      mesa_log_stream_printf(stream, "(kill)");
   if (reg->flags & IR3_REG_UNUSED)
      mesa_log_stream_printf(stream, "(unused)");

   if (reg->flags & IR3_REG_R)
      mesa_log_stream_printf(stream, "(r)");

   if (reg->flags & IR3_REG_EARLY_CLOBBER)
      mesa_log_stream_printf(stream, "(early_clobber)");

   /* Right now all instructions that use tied registers only have one
    * destination register, so we can just print (tied) as if it's a flag,
    * although it's more convenient for RA if it's a pointer.
    */
   if (reg->tied)
      mesa_log_stream_printf(stream, "(tied)");

   if (reg->flags & IR3_REG_SHARED)
      mesa_log_stream_printf(stream, "s");
   if (reg->flags & IR3_REG_HALF)
      mesa_log_stream_printf(stream, "h");

   if (reg->flags & IR3_REG_IMMED) {
      mesa_log_stream_printf(stream, SYN_IMMED("imm[%f,%d,0x%x]"), reg->fim_val,
                             reg->iim_val, reg->iim_val);
   } else if (reg->flags & IR3_REG_ARRAY) {
      if (reg->flags & IR3_REG_SSA) {
         print_ssa_name(stream, reg, dest);
         mesa_log_stream_printf(stream, ":");
      }
      mesa_log_stream_printf(stream,
                             SYN_ARRAY("arr[id=%u, offset=%d, size=%u]"),
                             reg->array.id, reg->array.offset, reg->size);
      if (reg->array.base != INVALID_REG)
         mesa_log_stream_printf(stream, "(" SYN_REG("r%u.%c") ")",
                                reg->array.base >> 2,
                                "xyzw"[reg->array.base & 0x3]);
   } else if (reg->flags & IR3_REG_SSA) {
      print_ssa_name(stream, reg, dest);
   } else if (reg->flags & IR3_REG_RELATIV) {
      if (reg->flags & IR3_REG_CONST)
         mesa_log_stream_printf(stream, SYN_CONST("c<a0.x + %d>"),
                                reg->array.offset);
      else
         mesa_log_stream_printf(stream, SYN_REG("r<a0.x + %d>") " (%u)",
                                reg->array.offset, reg->size);
   } else {
      if (reg->flags & IR3_REG_CONST)
         mesa_log_stream_printf(stream, SYN_CONST("c%u.%c"), reg_num(reg),
                                "xyzw"[reg_comp(reg)]);
      else
         mesa_log_stream_printf(stream, SYN_REG("r%u.%c"), reg_num(reg),
                                "xyzw"[reg_comp(reg)]);
   }

   if (reg->wrmask > 0x1)
      mesa_log_stream_printf(stream, " (wrmask=0x%x)", reg->wrmask);
}

static void
tab(struct log_stream *stream, int lvl)
{
   for (int i = 0; i < lvl; i++)
      mesa_log_stream_printf(stream, "\t");
}

static void
print_instr(struct log_stream *stream, struct ir3_instruction *instr, int lvl)
{
   tab(stream, lvl);

   print_instr_name(stream, instr, true);

   if (is_tex(instr)) {
      mesa_log_stream_printf(stream, " (%s)(", type_name(instr->cat5.type));
      for (unsigned i = 0; i < 4; i++)
         if (instr->dsts[0]->wrmask & (1 << i))
            mesa_log_stream_printf(stream, "%c", "xyzw"[i]);
      mesa_log_stream_printf(stream, ")");
   } else if ((instr->srcs_count > 0 || instr->dsts_count > 0) &&
              (instr->opc != OPC_B)) {
      /* NOTE the b(ranch) instruction has a suffix, which is
       * handled below
       */
      mesa_log_stream_printf(stream, " ");
   }

   if (!is_flow(instr) || instr->opc == OPC_END || instr->opc == OPC_CHMASK) {
      bool first = true;
      foreach_dst (reg, instr) {
         if (reg->wrmask == 0)
            continue;
         if (!first)
            mesa_log_stream_printf(stream, ", ");
         print_reg_name(stream, instr, reg, true);
         first = false;
      }
      foreach_src_n (reg, n, instr) {
         if (!first)
            mesa_log_stream_printf(stream, ", ");
         print_reg_name(stream, instr, reg, false);
         if (instr->opc == OPC_END || instr->opc == OPC_CHMASK)
            mesa_log_stream_printf(stream, " (%u)", instr->end.outidxs[n]);
         first = false;
      }
   }

   if (is_tex(instr) && !(instr->flags & IR3_INSTR_S2EN)) {
      if (!!(instr->flags & IR3_INSTR_B) && !!(instr->flags & IR3_INSTR_A1EN)) {
         mesa_log_stream_printf(stream, ", s#%d", instr->cat5.samp);
      } else {
         mesa_log_stream_printf(stream, ", s#%d, t#%d", instr->cat5.samp,
                                instr->cat5.tex);
      }
   }

   if (instr->opc == OPC_META_SPLIT) {
      mesa_log_stream_printf(stream, ", off=%d", instr->split.off);
   } else if (instr->opc == OPC_META_TEX_PREFETCH) {
      mesa_log_stream_printf(stream, ", tex=%d, samp=%d, input_offset=%d",
                             instr->prefetch.tex, instr->prefetch.samp,
                             instr->prefetch.input_offset);
   } else if (instr->opc == OPC_PUSH_CONSTS_LOAD_MACRO) {
      mesa_log_stream_printf(
         stream, " dst_offset=%d, src_offset = %d, src_size = %d",
         instr->push_consts.dst_base, instr->push_consts.src_base,
         instr->push_consts.src_size);
   }

   if (is_flow(instr) && instr->cat0.target) {
      /* the predicate register src is implied: */
      if (instr->opc == OPC_B) {
         static const struct {
            int nsrc;
            bool idx;
         } brinfo[7] = {
            /* clang-format off */
            [BRANCH_PLAIN] = {1, false},
            [BRANCH_OR]    = {2, false},
            [BRANCH_AND]   = {2, false},
            [BRANCH_CONST] = {0, true},
            [BRANCH_ANY]   = {1, false},
            [BRANCH_ALL]   = {1, false},
            [BRANCH_X]     = {0, false},
            /* clang-format on */
         };

         if (brinfo[instr->cat0.brtype].idx) {
            mesa_log_stream_printf(stream, ".%u", instr->cat0.idx);
         }
         if (brinfo[instr->cat0.brtype].nsrc >= 1) {
            mesa_log_stream_printf(stream, " %sp0.%c (",
                                   instr->cat0.inv1 ? "!" : "",
                                   "xyzw"[instr->cat0.comp1 & 0x3]);
            print_reg_name(stream, instr, instr->srcs[0], false);
            mesa_log_stream_printf(stream, "), ");
         }
         if (brinfo[instr->cat0.brtype].nsrc >= 2) {
            mesa_log_stream_printf(stream, " %sp0.%c (",
                                   instr->cat0.inv2 ? "!" : "",
                                   "xyzw"[instr->cat0.comp2 & 0x3]);
            print_reg_name(stream, instr, instr->srcs[1], false);
            mesa_log_stream_printf(stream, "), ");
         }
      }
      mesa_log_stream_printf(stream, " target=block%u",
                             block_id(instr->cat0.target));
   }

   if (instr->deps_count) {
      mesa_log_stream_printf(stream, ", false-deps:");
      unsigned n = 0;
      for (unsigned i = 0; i < instr->deps_count; i++) {
         if (!instr->deps[i])
            continue;
         if (n++ > 0)
            mesa_log_stream_printf(stream, ", ");
         mesa_log_stream_printf(stream, SYN_SSA("ssa_%u"),
                                instr->deps[i]->serialno);
      }
   }

   mesa_log_stream_printf(stream, "\n");
}

void
ir3_print_instr_stream(struct log_stream *stream, struct ir3_instruction *instr)
{
   print_instr(stream, instr, 0);
}

void
ir3_print_instr(struct ir3_instruction *instr)
{
   struct log_stream *stream = mesa_log_streami();
   print_instr(stream, instr, 0);
   mesa_log_stream_destroy(stream);
}

static void
print_block(struct ir3_block *block, int lvl)
{
   struct log_stream *stream = mesa_log_streami();

   tab(stream, lvl);
   mesa_log_stream_printf(stream, "block%u {\n", block_id(block));

   if (block->predecessors_count > 0) {
      tab(stream, lvl + 1);
      mesa_log_stream_printf(stream, "pred: ");
      for (unsigned i = 0; i < block->predecessors_count; i++) {
         struct ir3_block *pred = block->predecessors[i];
         if (i != 0)
            mesa_log_stream_printf(stream, ", ");
         mesa_log_stream_printf(stream, "block%u", block_id(pred));
      }
      mesa_log_stream_printf(stream, "\n");
   }

   if (block->physical_predecessors_count > 0) {
      tab(stream, lvl + 1);
      mesa_log_stream_printf(stream, "physical pred: ");
      for (unsigned i = 0; i < block->physical_predecessors_count; i++) {
         struct ir3_block *pred = block->physical_predecessors[i];
         if (i != 0)
            mesa_log_stream_printf(stream, ", ");
         mesa_log_stream_printf(stream, "block%u", block_id(pred));
      }
      mesa_log_stream_printf(stream, "\n");
   }

   foreach_instr (instr, &block->instr_list) {
      print_instr(stream, instr, lvl + 1);
   }

   tab(stream, lvl + 1);
   mesa_log_stream_printf(stream, "/* keeps:\n");
   for (unsigned i = 0; i < block->keeps_count; i++) {
      print_instr(stream, block->keeps[i], lvl + 2);
   }
   tab(stream, lvl + 1);
   mesa_log_stream_printf(stream, " */\n");

   if (block->successors[1]) {
      /* leading into if/else: */
      tab(stream, lvl + 1);
      mesa_log_stream_printf(stream, "/* succs: if ");
      switch (block->brtype) {
      case IR3_BRANCH_COND:
         break;
      case IR3_BRANCH_ANY:
         mesa_log_stream_printf(stream, "any ");
         break;
      case IR3_BRANCH_ALL:
         mesa_log_stream_printf(stream, "all ");
         break;
      case IR3_BRANCH_GETONE:
         mesa_log_stream_printf(stream, "getone ");
         break;
      case IR3_BRANCH_SHPS:
         mesa_log_stream_printf(stream, "shps ");
         break;
      }
      if (block->condition)
         mesa_log_stream_printf(stream, SYN_SSA("ssa_%u") " ",
                                block->condition->serialno);
      mesa_log_stream_printf(stream, "block%u; else block%u; */\n",
                             block_id(block->successors[0]),
                             block_id(block->successors[1]));
   } else if (block->successors[0]) {
      tab(stream, lvl + 1);
      mesa_log_stream_printf(stream, "/* succs: block%u; */\n",
                             block_id(block->successors[0]));
   }
   if (block->physical_successors[0]) {
      tab(stream, lvl + 1);
      mesa_log_stream_printf(stream, "/* physical succs: block%u",
                             block_id(block->physical_successors[0]));
      if (block->physical_successors[1]) {
         mesa_log_stream_printf(stream, ", block%u",
                                block_id(block->physical_successors[1]));
      }
      mesa_log_stream_printf(stream, " */\n");
   }
   tab(stream, lvl);
   mesa_log_stream_printf(stream, "}\n");
}

void
ir3_print(struct ir3 *ir)
{
   foreach_block (block, &ir->block_list)
      print_block(block, 0);
}
