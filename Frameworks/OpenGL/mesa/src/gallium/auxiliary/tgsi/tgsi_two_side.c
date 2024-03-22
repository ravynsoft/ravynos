/*
 * Copyright 2013 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * This utility transforms fragment shaders to facilitate two-sided lighting.
 *
 * Basically, if the FS has any color inputs (TGSI_SEMANTIC_COLOR) we'll:
 * 1. create corresponding back-color inputs (TGSI_SEMANTIC_BCOLOR)
 * 2. use the FACE register to choose between front/back colors and put the
 *    selected color in new temp regs.
 * 3. replace reads of the original color inputs with the new temp regs.
 *
 * Then, the driver just needs to link the VS front/back output colors to
 * the FS front/back input colors.
 */

#include "util/u_debug.h"
#include "util/u_math.h"
#include "tgsi_info.h"
#include "tgsi_two_side.h"
#include "tgsi_transform.h"


#define INVALID_INDEX 9999


struct two_side_transform_context
{
   struct tgsi_transform_context base;
   unsigned num_temps;
   unsigned num_inputs;
   unsigned face_input;           /**< index of the FACE input */
   unsigned front_color_input[2]; /**< INPUT regs */
   enum tgsi_interpolate_mode front_color_interp[2];/**< TGSI_INTERPOLATE_x */
   unsigned back_color_input[2];  /**< INPUT regs */
   unsigned new_colors[2];        /**< TEMP regs */
};


static inline struct two_side_transform_context *
two_side_transform_context(struct tgsi_transform_context *ctx)
{
   return (struct two_side_transform_context *) ctx;
}


static void
xform_decl(struct tgsi_transform_context *ctx,
           struct tgsi_full_declaration *decl)
{
   struct two_side_transform_context *ts = two_side_transform_context(ctx);
   unsigned range_end = decl->Range.Last + 1;

   if (decl->Declaration.File == TGSI_FILE_INPUT) {
      if (decl->Semantic.Name == TGSI_SEMANTIC_COLOR) {
         /* found a front color */
         assert(decl->Semantic.Index < 2);
         ts->front_color_input[decl->Semantic.Index] = decl->Range.First;
         ts->front_color_interp[decl->Semantic.Index] = decl->Interp.Interpolate;
      }
      else if (decl->Semantic.Name == TGSI_SEMANTIC_FACE) {
         ts->face_input = decl->Range.First;
      }
      ts->num_inputs = MAX2(ts->num_inputs, range_end);
   }
   else if (decl->Declaration.File == TGSI_FILE_TEMPORARY) {
      ts->num_temps = MAX2(ts->num_temps, range_end);
   }

   ctx->emit_declaration(ctx, decl);
}


static void
emit_prolog(struct tgsi_transform_context *ctx)
{
   struct two_side_transform_context *ts = two_side_transform_context(ctx);
   struct tgsi_full_declaration decl;
   struct tgsi_full_instruction inst;
   unsigned num_colors = 0;
   unsigned i;

   /* Declare 0, 1 or 2 new BCOLOR inputs */
   for (i = 0; i < 2; i++) {
      if (ts->front_color_input[i] != INVALID_INDEX) {
         decl = tgsi_default_full_declaration();
         decl.Declaration.File = TGSI_FILE_INPUT;
         decl.Declaration.Interpolate = 1;
         decl.Declaration.Semantic = 1;
         decl.Semantic.Name = TGSI_SEMANTIC_BCOLOR;
         decl.Semantic.Index = i;
         decl.Range.First = decl.Range.Last = ts->num_inputs++;
         decl.Interp.Interpolate = ts->front_color_interp[i];
         ctx->emit_declaration(ctx, &decl);
         ts->back_color_input[i] = decl.Range.First;
         num_colors++;
      }
   }

   if (num_colors > 0) {
      /* Declare 1 or 2 temp registers */
      decl = tgsi_default_full_declaration();
      decl.Declaration.File = TGSI_FILE_TEMPORARY;
      decl.Range.First = ts->num_temps;
      decl.Range.Last = ts->num_temps + num_colors - 1;
      ctx->emit_declaration(ctx, &decl);
      ts->new_colors[0] = ts->num_temps;
      ts->new_colors[1] = ts->num_temps + 1;

      if (ts->face_input == INVALID_INDEX) {
         /* declare FACE INPUT register */
         decl = tgsi_default_full_declaration();
         decl.Declaration.File = TGSI_FILE_INPUT;
         decl.Declaration.Semantic = 1;
         decl.Semantic.Name = TGSI_SEMANTIC_FACE;
         decl.Semantic.Index = 0;
         decl.Range.First = decl.Range.Last = ts->num_inputs++;
         ctx->emit_declaration(ctx, &decl);
         ts->face_input = decl.Range.First;
      }

      /* CMP temp[c0], face, bcolor[c0], fcolor[c0]
       * temp[c0] = face < 0.0 ? bcolor[c0] : fcolor[c0]
       */
      for (i = 0; i < 2; i++) {
         if (ts->front_color_input[i] != INVALID_INDEX) {
            inst = tgsi_default_full_instruction();
            inst.Instruction.Opcode = TGSI_OPCODE_CMP;
            inst.Instruction.NumDstRegs = 1;
            inst.Dst[0].Register.File = TGSI_FILE_TEMPORARY;
            inst.Dst[0].Register.Index = ts->new_colors[i];
            inst.Instruction.NumSrcRegs = 3;
            inst.Src[0].Register.File = TGSI_FILE_INPUT;
            inst.Src[0].Register.Index = ts->face_input;
            inst.Src[1].Register.File = TGSI_FILE_INPUT;
            inst.Src[1].Register.Index = ts->back_color_input[i];
            inst.Src[2].Register.File = TGSI_FILE_INPUT;
            inst.Src[2].Register.Index = ts->front_color_input[i];

            ctx->emit_instruction(ctx, &inst);
         }
      }
   }
}


static void
xform_inst(struct tgsi_transform_context *ctx,
           struct tgsi_full_instruction *inst)
{
   struct two_side_transform_context *ts = two_side_transform_context(ctx);
   const struct tgsi_opcode_info *info =
      tgsi_get_opcode_info(inst->Instruction.Opcode);
   unsigned i, j;

   /* Look for src regs which reference the input color and replace
    * them with the temp color.
    */
   for (i = 0; i < info->num_src; i++) {
      if (inst->Src[i].Register.File == TGSI_FILE_INPUT) {
         for (j = 0; j < 2; j++) {
	    if (inst->Src[i].Register.Index == (int)ts->front_color_input[j]) {
               /* replace color input with temp reg */
               inst->Src[i].Register.File = TGSI_FILE_TEMPORARY;
               inst->Src[i].Register.Index = ts->new_colors[j];
               break;
            }
         }
      }
   }

   ctx->emit_instruction(ctx, inst);
}


struct tgsi_token *
tgsi_add_two_side(const struct tgsi_token *tokens_in)
{
   struct two_side_transform_context transform;
   const unsigned num_new_tokens = 100; /* should be enough */
   const unsigned new_len = tgsi_num_tokens(tokens_in) + num_new_tokens;

   /* setup transformation context */
   memset(&transform, 0, sizeof(transform));
   transform.base.transform_declaration = xform_decl;
   transform.base.transform_instruction = xform_inst;
   transform.base.prolog = emit_prolog;
   transform.face_input = INVALID_INDEX;
   transform.front_color_input[0] = INVALID_INDEX;
   transform.front_color_input[1] = INVALID_INDEX;
   transform.front_color_interp[0] = TGSI_INTERPOLATE_COLOR;
   transform.front_color_interp[1] = TGSI_INTERPOLATE_COLOR;
   transform.back_color_input[0] = INVALID_INDEX;
   transform.back_color_input[1] = INVALID_INDEX;

   return tgsi_transform_shader(tokens_in, new_len, &transform.base);
}
