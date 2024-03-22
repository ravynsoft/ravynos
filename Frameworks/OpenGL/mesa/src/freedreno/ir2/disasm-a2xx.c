/*
 * Copyright (c) 2012 Rob Clark <robdclark@gmail.com>
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

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "disasm.h"
#include "instr-a2xx.h"

static const char *levels[] = {
   "\t",
   "\t\t",
   "\t\t\t",
   "\t\t\t\t",
   "\t\t\t\t\t",
   "\t\t\t\t\t\t",
   "\t\t\t\t\t\t\t",
   "\t\t\t\t\t\t\t\t",
   "\t\t\t\t\t\t\t\t\t",
   "x",
   "x",
   "x",
   "x",
   "x",
   "x",
};

static enum debug_t debug;

/*
 * ALU instructions:
 */

static const char chan_names[] = {
   'x',
   'y',
   'z',
   'w',
   /* these only apply to FETCH dst's: */
   '0',
   '1',
   '?',
   '_',
};

static void
print_srcreg(uint32_t num, uint32_t type, uint32_t swiz, uint32_t negate,
             uint32_t abs)
{
   if (negate)
      printf("-");
   if (abs)
      printf("|");
   printf("%c%u", type ? 'R' : 'C', num);
   if (swiz) {
      int i;
      printf(".");
      for (i = 0; i < 4; i++) {
         printf("%c", chan_names[(swiz + i) & 0x3]);
         swiz >>= 2;
      }
   }
   if (abs)
      printf("|");
}

static void
print_dstreg(uint32_t num, uint32_t mask, uint32_t dst_exp)
{
   printf("%s%u", dst_exp ? "export" : "R", num);
   if (mask != 0xf) {
      int i;
      printf(".");
      for (i = 0; i < 4; i++) {
         printf("%c", (mask & 0x1) ? chan_names[i] : '_');
         mask >>= 1;
      }
   }
}

static void
print_export_comment(uint32_t num, gl_shader_stage type)
{
   const char *name = NULL;
   switch (type) {
   case MESA_SHADER_VERTEX:
      switch (num) {
      case 62:
         name = "gl_Position";
         break;
      case 63:
         name = "gl_PointSize";
         break;
      }
      break;
   case MESA_SHADER_FRAGMENT:
      switch (num) {
      case 0:
         name = "gl_FragColor";
         break;
      }
      break;
   default:
      assert(!"not reached");
   }
   /* if we had a symbol table here, we could look
    * up the name of the varying..
    */
   if (name) {
      printf("\t; %s", name);
   }
}

struct {
   uint32_t num_srcs;
   const char *name;
} vector_instructions[0x20] = {
#define INSTR(opc, num_srcs) [opc] = {num_srcs, #opc}
      INSTR(ADDv, 2),
      INSTR(MULv, 2),
      INSTR(MAXv, 2),
      INSTR(MINv, 2),
      INSTR(SETEv, 2),
      INSTR(SETGTv, 2),
      INSTR(SETGTEv, 2),
      INSTR(SETNEv, 2),
      INSTR(FRACv, 1),
      INSTR(TRUNCv, 1),
      INSTR(FLOORv, 1),
      INSTR(MULADDv, 3),
      INSTR(CNDEv, 3),
      INSTR(CNDGTEv, 3),
      INSTR(CNDGTv, 3),
      INSTR(DOT4v, 2),
      INSTR(DOT3v, 2),
      INSTR(DOT2ADDv, 3), // ???
      INSTR(CUBEv, 2),
      INSTR(MAX4v, 1),
      INSTR(PRED_SETE_PUSHv, 2),
      INSTR(PRED_SETNE_PUSHv, 2),
      INSTR(PRED_SETGT_PUSHv, 2),
      INSTR(PRED_SETGTE_PUSHv, 2),
      INSTR(KILLEv, 2),
      INSTR(KILLGTv, 2),
      INSTR(KILLGTEv, 2),
      INSTR(KILLNEv, 2),
      INSTR(DSTv, 2),
      INSTR(MOVAv, 1),
}, scalar_instructions[0x40] = {
      INSTR(ADDs, 1),
      INSTR(ADD_PREVs, 1),
      INSTR(MULs, 1),
      INSTR(MUL_PREVs, 1),
      INSTR(MUL_PREV2s, 1),
      INSTR(MAXs, 1),
      INSTR(MINs, 1),
      INSTR(SETEs, 1),
      INSTR(SETGTs, 1),
      INSTR(SETGTEs, 1),
      INSTR(SETNEs, 1),
      INSTR(FRACs, 1),
      INSTR(TRUNCs, 1),
      INSTR(FLOORs, 1),
      INSTR(EXP_IEEE, 1),
      INSTR(LOG_CLAMP, 1),
      INSTR(LOG_IEEE, 1),
      INSTR(RECIP_CLAMP, 1),
      INSTR(RECIP_FF, 1),
      INSTR(RECIP_IEEE, 1),
      INSTR(RECIPSQ_CLAMP, 1),
      INSTR(RECIPSQ_FF, 1),
      INSTR(RECIPSQ_IEEE, 1),
      INSTR(MOVAs, 1),
      INSTR(MOVA_FLOORs, 1),
      INSTR(SUBs, 1),
      INSTR(SUB_PREVs, 1),
      INSTR(PRED_SETEs, 1),
      INSTR(PRED_SETNEs, 1),
      INSTR(PRED_SETGTs, 1),
      INSTR(PRED_SETGTEs, 1),
      INSTR(PRED_SET_INVs, 1),
      INSTR(PRED_SET_POPs, 1),
      INSTR(PRED_SET_CLRs, 1),
      INSTR(PRED_SET_RESTOREs, 1),
      INSTR(KILLEs, 1),
      INSTR(KILLGTs, 1),
      INSTR(KILLGTEs, 1),
      INSTR(KILLNEs, 1),
      INSTR(KILLONEs, 1),
      INSTR(SQRT_IEEE, 1),
      INSTR(MUL_CONST_0, 1),
      INSTR(MUL_CONST_1, 1),
      INSTR(ADD_CONST_0, 1),
      INSTR(ADD_CONST_1, 1),
      INSTR(SUB_CONST_0, 1),
      INSTR(SUB_CONST_1, 1),
      INSTR(SIN, 1),
      INSTR(COS, 1),
      INSTR(RETAIN_PREV, 1),
#undef INSTR
};

static int
disasm_alu(uint32_t *dwords, uint32_t alu_off, int level, int sync,
           gl_shader_stage type)
{
   instr_alu_t *alu = (instr_alu_t *)dwords;

   printf("%s", levels[level]);
   if (debug & PRINT_RAW) {
      printf("%02x: %08x %08x %08x\t", alu_off, dwords[0], dwords[1],
             dwords[2]);
   }

   printf("   %sALU:\t", sync ? "(S)" : "   ");

   printf("%s", vector_instructions[alu->vector_opc].name);

   if (alu->pred_select & 0x2) {
      /* seems to work similar to conditional execution in ARM instruction
       * set, so let's use a similar syntax for now:
       */
      printf((alu->pred_select & 0x1) ? "EQ" : "NE");
   }

   printf("\t");

   print_dstreg(alu->vector_dest, alu->vector_write_mask, alu->export_data);
   printf(" = ");
   if (vector_instructions[alu->vector_opc].num_srcs == 3) {
      print_srcreg(alu->src3_reg, alu->src3_sel, alu->src3_swiz,
                   alu->src3_reg_negate, alu->src3_reg_abs);
      printf(", ");
   }
   print_srcreg(alu->src1_reg, alu->src1_sel, alu->src1_swiz,
                alu->src1_reg_negate, alu->src1_reg_abs);
   if (vector_instructions[alu->vector_opc].num_srcs > 1) {
      printf(", ");
      print_srcreg(alu->src2_reg, alu->src2_sel, alu->src2_swiz,
                   alu->src2_reg_negate, alu->src2_reg_abs);
   }

   if (alu->vector_clamp)
      printf(" CLAMP");

   if (alu->export_data)
      print_export_comment(alu->vector_dest, type);

   printf("\n");

   if (alu->scalar_write_mask || !alu->vector_write_mask) {
      /* 2nd optional scalar op: */

      printf("%s", levels[level]);
      if (debug & PRINT_RAW)
         printf("                          \t");

      if (scalar_instructions[alu->scalar_opc].name) {
         printf("\t    \t%s\t", scalar_instructions[alu->scalar_opc].name);
      } else {
         printf("\t    \tOP(%u)\t", alu->scalar_opc);
      }

      print_dstreg(alu->scalar_dest, alu->scalar_write_mask, alu->export_data);
      printf(" = ");
      print_srcreg(alu->src3_reg, alu->src3_sel, alu->src3_swiz,
                   alu->src3_reg_negate, alu->src3_reg_abs);
      // TODO ADD/MUL must have another src?!?
      if (alu->scalar_clamp)
         printf(" CLAMP");
      if (alu->export_data)
         print_export_comment(alu->scalar_dest, type);
      printf("\n");
   }

   return 0;
}

/*
 * FETCH instructions:
 */

struct {
   const char *name;
} fetch_types[0xff] = {
#define TYPE(id) [id] = {#id}
   TYPE(FMT_1_REVERSE),
   TYPE(FMT_32_FLOAT),
   TYPE(FMT_32_32_FLOAT),
   TYPE(FMT_32_32_32_FLOAT),
   TYPE(FMT_32_32_32_32_FLOAT),
   TYPE(FMT_16),
   TYPE(FMT_16_16),
   TYPE(FMT_16_16_16_16),
   TYPE(FMT_8),
   TYPE(FMT_8_8),
   TYPE(FMT_8_8_8_8),
   TYPE(FMT_32),
   TYPE(FMT_32_32),
   TYPE(FMT_32_32_32_32),
#undef TYPE
};

static void
print_fetch_dst(uint32_t dst_reg, uint32_t dst_swiz)
{
   int i;
   printf("\tR%u.", dst_reg);
   for (i = 0; i < 4; i++) {
      printf("%c", chan_names[dst_swiz & 0x7]);
      dst_swiz >>= 3;
   }
}

static void
print_fetch_vtx(instr_fetch_t *fetch)
{
   instr_fetch_vtx_t *vtx = &fetch->vtx;

   if (vtx->pred_select) {
      /* seems to work similar to conditional execution in ARM instruction
       * set, so let's use a similar syntax for now:
       */
      printf(vtx->pred_condition ? "EQ" : "NE");
   }

   print_fetch_dst(vtx->dst_reg, vtx->dst_swiz);
   printf(" = R%u.", vtx->src_reg);
   printf("%c", chan_names[vtx->src_swiz & 0x3]);
   if (fetch_types[vtx->format].name) {
      printf(" %s", fetch_types[vtx->format].name);
   } else {
      printf(" TYPE(0x%x)", vtx->format);
   }
   printf(" %s", vtx->format_comp_all ? "SIGNED" : "UNSIGNED");
   if (!vtx->num_format_all)
      printf(" NORMALIZED");
   printf(" STRIDE(%u)", vtx->stride);
   if (vtx->offset)
      printf(" OFFSET(%u)", vtx->offset);
   printf(" CONST(%u, %u)", vtx->const_index, vtx->const_index_sel);
   if (0) {
      // XXX
      printf(" src_reg_am=%u", vtx->src_reg_am);
      printf(" dst_reg_am=%u", vtx->dst_reg_am);
      printf(" num_format_all=%u", vtx->num_format_all);
      printf(" signed_rf_mode_all=%u", vtx->signed_rf_mode_all);
      printf(" exp_adjust_all=%u", vtx->exp_adjust_all);
   }
}

static void
print_fetch_tex(instr_fetch_t *fetch)
{
   static const char *filter[] = {
      [TEX_FILTER_POINT] = "POINT",
      [TEX_FILTER_LINEAR] = "LINEAR",
      [TEX_FILTER_BASEMAP] = "BASEMAP",
   };
   static const char *aniso_filter[] = {
      [ANISO_FILTER_DISABLED] = "DISABLED",
      [ANISO_FILTER_MAX_1_1] = "MAX_1_1",
      [ANISO_FILTER_MAX_2_1] = "MAX_2_1",
      [ANISO_FILTER_MAX_4_1] = "MAX_4_1",
      [ANISO_FILTER_MAX_8_1] = "MAX_8_1",
      [ANISO_FILTER_MAX_16_1] = "MAX_16_1",
   };
   static const char *arbitrary_filter[] = {
      [ARBITRARY_FILTER_2X4_SYM] = "2x4_SYM",
      [ARBITRARY_FILTER_2X4_ASYM] = "2x4_ASYM",
      [ARBITRARY_FILTER_4X2_SYM] = "4x2_SYM",
      [ARBITRARY_FILTER_4X2_ASYM] = "4x2_ASYM",
      [ARBITRARY_FILTER_4X4_SYM] = "4x4_SYM",
      [ARBITRARY_FILTER_4X4_ASYM] = "4x4_ASYM",
   };
   static const char *sample_loc[] = {
      [SAMPLE_CENTROID] = "CENTROID",
      [SAMPLE_CENTER] = "CENTER",
   };
   instr_fetch_tex_t *tex = &fetch->tex;
   uint32_t src_swiz = tex->src_swiz;
   int i;

   if (tex->pred_select) {
      /* seems to work similar to conditional execution in ARM instruction
       * set, so let's use a similar syntax for now:
       */
      printf(tex->pred_condition ? "EQ" : "NE");
   }

   print_fetch_dst(tex->dst_reg, tex->dst_swiz);
   printf(" = R%u.", tex->src_reg);
   for (i = 0; i < 3; i++) {
      printf("%c", chan_names[src_swiz & 0x3]);
      src_swiz >>= 2;
   }
   printf(" CONST(%u)", tex->const_idx);
   if (tex->fetch_valid_only)
      printf(" VALID_ONLY");
   if (tex->tx_coord_denorm)
      printf(" DENORM");
   if (tex->mag_filter != TEX_FILTER_USE_FETCH_CONST)
      printf(" MAG(%s)", filter[tex->mag_filter]);
   if (tex->min_filter != TEX_FILTER_USE_FETCH_CONST)
      printf(" MIN(%s)", filter[tex->min_filter]);
   if (tex->mip_filter != TEX_FILTER_USE_FETCH_CONST)
      printf(" MIP(%s)", filter[tex->mip_filter]);
   if (tex->aniso_filter != ANISO_FILTER_USE_FETCH_CONST)
      printf(" ANISO(%s)", aniso_filter[tex->aniso_filter]);
   if (tex->arbitrary_filter != ARBITRARY_FILTER_USE_FETCH_CONST)
      printf(" ARBITRARY(%s)", arbitrary_filter[tex->arbitrary_filter]);
   if (tex->vol_mag_filter != TEX_FILTER_USE_FETCH_CONST)
      printf(" VOL_MAG(%s)", filter[tex->vol_mag_filter]);
   if (tex->vol_min_filter != TEX_FILTER_USE_FETCH_CONST)
      printf(" VOL_MIN(%s)", filter[tex->vol_min_filter]);
   if (!tex->use_comp_lod) {
      printf(" LOD(%u)", tex->use_comp_lod);
      printf(" LOD_BIAS(%u)", tex->lod_bias);
   }
   if (tex->use_reg_lod) {
      printf(" REG_LOD(%u)", tex->use_reg_lod);
   }
   if (tex->use_reg_gradients)
      printf(" USE_REG_GRADIENTS");
   printf(" LOCATION(%s)", sample_loc[tex->sample_location]);
   if (tex->offset_x || tex->offset_y || tex->offset_z)
      printf(" OFFSET(%u,%u,%u)", tex->offset_x, tex->offset_y, tex->offset_z);
}

struct {
   const char *name;
   void (*fxn)(instr_fetch_t *cf);
} fetch_instructions[] = {
#define INSTR(opc, name, fxn) [opc] = {name, fxn}
   INSTR(VTX_FETCH, "VERTEX", print_fetch_vtx),
   INSTR(TEX_FETCH, "SAMPLE", print_fetch_tex),
   INSTR(TEX_GET_BORDER_COLOR_FRAC, "?", print_fetch_tex),
   INSTR(TEX_GET_COMP_TEX_LOD, "?", print_fetch_tex),
   INSTR(TEX_GET_GRADIENTS, "?", print_fetch_tex),
   INSTR(TEX_GET_WEIGHTS, "?", print_fetch_tex),
   INSTR(TEX_SET_TEX_LOD, "SET_TEX_LOD", print_fetch_tex),
   INSTR(TEX_SET_GRADIENTS_H, "?", print_fetch_tex),
   INSTR(TEX_SET_GRADIENTS_V, "?", print_fetch_tex),
   INSTR(TEX_RESERVED_4, "?", print_fetch_tex),
#undef INSTR
};

static int
disasm_fetch(uint32_t *dwords, uint32_t alu_off, int level, int sync)
{
   instr_fetch_t *fetch = (instr_fetch_t *)dwords;

   printf("%s", levels[level]);
   if (debug & PRINT_RAW) {
      printf("%02x: %08x %08x %08x\t", alu_off, dwords[0], dwords[1],
             dwords[2]);
   }

   printf("   %sFETCH:\t", sync ? "(S)" : "   ");
   printf("%s", fetch_instructions[fetch->opc].name);
   fetch_instructions[fetch->opc].fxn(fetch);
   printf("\n");

   return 0;
}

/*
 * CF instructions:
 */

static int
cf_exec(instr_cf_t *cf)
{
   return (cf->opc == EXEC) || (cf->opc == EXEC_END) ||
          (cf->opc == COND_EXEC) || (cf->opc == COND_EXEC_END) ||
          (cf->opc == COND_PRED_EXEC) || (cf->opc == COND_PRED_EXEC_END) ||
          (cf->opc == COND_EXEC_PRED_CLEAN) ||
          (cf->opc == COND_EXEC_PRED_CLEAN_END);
}

static int
cf_cond_exec(instr_cf_t *cf)
{
   return (cf->opc == COND_EXEC) || (cf->opc == COND_EXEC_END) ||
          (cf->opc == COND_PRED_EXEC) || (cf->opc == COND_PRED_EXEC_END) ||
          (cf->opc == COND_EXEC_PRED_CLEAN) ||
          (cf->opc == COND_EXEC_PRED_CLEAN_END);
}

static void
print_cf_nop(instr_cf_t *cf)
{
}

static void
print_cf_exec(instr_cf_t *cf)
{
   printf(" ADDR(0x%x) CNT(0x%x)", cf->exec.address, cf->exec.count);
   if (cf->exec.yeild)
      printf(" YIELD");
   if (cf->exec.vc)
      printf(" VC(0x%x)", cf->exec.vc);
   if (cf->exec.bool_addr)
      printf(" BOOL_ADDR(0x%x)", cf->exec.bool_addr);
   if (cf->exec.address_mode == ABSOLUTE_ADDR)
      printf(" ABSOLUTE_ADDR");
   if (cf_cond_exec(cf))
      printf(" COND(%d)", cf->exec.condition);
}

static void
print_cf_loop(instr_cf_t *cf)
{
   printf(" ADDR(0x%x) LOOP_ID(%d)", cf->loop.address, cf->loop.loop_id);
   if (cf->loop.address_mode == ABSOLUTE_ADDR)
      printf(" ABSOLUTE_ADDR");
}

static void
print_cf_jmp_call(instr_cf_t *cf)
{
   printf(" ADDR(0x%x) DIR(%d)", cf->jmp_call.address, cf->jmp_call.direction);
   if (cf->jmp_call.force_call)
      printf(" FORCE_CALL");
   if (cf->jmp_call.predicated_jmp)
      printf(" COND(%d)", cf->jmp_call.condition);
   if (cf->jmp_call.bool_addr)
      printf(" BOOL_ADDR(0x%x)", cf->jmp_call.bool_addr);
   if (cf->jmp_call.address_mode == ABSOLUTE_ADDR)
      printf(" ABSOLUTE_ADDR");
}

static void
print_cf_alloc(instr_cf_t *cf)
{
   static const char *bufname[] = {
      [SQ_NO_ALLOC] = "NO ALLOC",
      [SQ_POSITION] = "POSITION",
      [SQ_PARAMETER_PIXEL] = "PARAM/PIXEL",
      [SQ_MEMORY] = "MEMORY",
   };
   printf(" %s SIZE(0x%x)", bufname[cf->alloc.buffer_select], cf->alloc.size);
   if (cf->alloc.no_serial)
      printf(" NO_SERIAL");
   if (cf->alloc.alloc_mode) // ???
      printf(" ALLOC_MODE");
}

struct {
   const char *name;
   void (*fxn)(instr_cf_t *cf);
} cf_instructions[] = {
#define INSTR(opc, fxn) [opc] = {#opc, fxn}
   INSTR(NOP, print_cf_nop),
   INSTR(EXEC, print_cf_exec),
   INSTR(EXEC_END, print_cf_exec),
   INSTR(COND_EXEC, print_cf_exec),
   INSTR(COND_EXEC_END, print_cf_exec),
   INSTR(COND_PRED_EXEC, print_cf_exec),
   INSTR(COND_PRED_EXEC_END, print_cf_exec),
   INSTR(LOOP_START, print_cf_loop),
   INSTR(LOOP_END, print_cf_loop),
   INSTR(COND_CALL, print_cf_jmp_call),
   INSTR(RETURN, print_cf_jmp_call),
   INSTR(COND_JMP, print_cf_jmp_call),
   INSTR(ALLOC, print_cf_alloc),
   INSTR(COND_EXEC_PRED_CLEAN, print_cf_exec),
   INSTR(COND_EXEC_PRED_CLEAN_END, print_cf_exec),
   INSTR(MARK_VS_FETCH_DONE, print_cf_nop), // ??
#undef INSTR
};

static void
print_cf(instr_cf_t *cf, int level)
{
   printf("%s", levels[level]);
   if (debug & PRINT_RAW) {
      uint16_t words[3];
      memcpy(&words, cf, sizeof(words));
      printf("    %04x %04x %04x            \t", words[0], words[1], words[2]);
   }
   printf("%s", cf_instructions[cf->opc].name);
   cf_instructions[cf->opc].fxn(cf);
   printf("\n");
}

/*
 * The adreno shader microcode consists of two parts:
 *   1) A CF (control-flow) program, at the header of the compiled shader,
 *      which refers to ALU/FETCH instructions that follow it by address.
 *   2) ALU and FETCH instructions
 */

int
disasm_a2xx(uint32_t *dwords, int sizedwords, int level, gl_shader_stage type)
{
   instr_cf_t *cfs = (instr_cf_t *)dwords;
   int idx, max_idx;

   for (idx = 0;; idx++) {
      instr_cf_t *cf = &cfs[idx];
      if (cf_exec(cf)) {
         max_idx = 2 * cf->exec.address;
         break;
      }
   }

   for (idx = 0; idx < max_idx; idx++) {
      instr_cf_t *cf = &cfs[idx];

      print_cf(cf, level);

      if (cf_exec(cf)) {
         uint32_t sequence = cf->exec.serialize;
         uint32_t i;
         for (i = 0; i < cf->exec.count; i++) {
            uint32_t alu_off = (cf->exec.address + i);
            if (sequence & 0x1) {
               disasm_fetch(dwords + alu_off * 3, alu_off, level,
                            sequence & 0x2);
            } else {
               disasm_alu(dwords + alu_off * 3, alu_off, level, sequence & 0x2,
                          type);
            }
            sequence >>= 2;
         }
      }
   }

   return 0;
}

void
disasm_a2xx_set_debug(enum debug_t d)
{
   debug = d;
}
