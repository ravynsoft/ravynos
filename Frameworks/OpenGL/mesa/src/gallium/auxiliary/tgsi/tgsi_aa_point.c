/*
 * Copyright 2014 VMware, Inc.
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
 * This utility transforms the fragment shader to support anti-aliasing points.
 */

#include "util/u_debug.h"
#include "util/u_math.h"
#include "tgsi_info.h"
#include "tgsi_aa_point.h"
#include "tgsi_transform.h"

#define INVALID_INDEX 9999

struct aa_transform_context
{
   struct tgsi_transform_context base;

   unsigned tmp;           // temp register
   unsigned color_out;     // frag color out register
   unsigned color_tmp;     // frag color temp register
   unsigned num_tmp;       // number of temp registers
   unsigned num_imm;       // number of immediates
   unsigned num_input;     // number of inputs
   unsigned aa_point_coord_index;
   bool need_texcoord_semantic;
};

static inline struct aa_transform_context *
aa_transform_context(struct tgsi_transform_context *ctx)
{
   return (struct aa_transform_context *) ctx;
}

/**
 * TGSI declaration transform callback.
 */
static void
aa_decl(struct tgsi_transform_context *ctx,
              struct tgsi_full_declaration *decl)
{
   struct aa_transform_context *ts = aa_transform_context(ctx);

   if (decl->Declaration.File == TGSI_FILE_OUTPUT &&
       decl->Semantic.Name == TGSI_SEMANTIC_COLOR &&
       decl->Semantic.Index == 0) {
         ts->color_out = decl->Range.First;
   }
   else if (decl->Declaration.File == TGSI_FILE_INPUT) {
      ts->num_input++;
   }
   else if (decl->Declaration.File == TGSI_FILE_TEMPORARY) {
      ts->num_tmp = MAX2(ts->num_tmp, (unsigned)(decl->Range.Last + 1));
   }

   ctx->emit_declaration(ctx, decl);
}

/**
 * TGSI immediate declaration transform callback.
 */
static void
aa_immediate(struct tgsi_transform_context *ctx,
                  struct tgsi_full_immediate *imm)
{
   struct aa_transform_context *ts = aa_transform_context(ctx);

   ctx->emit_immediate(ctx, imm);
   ts->num_imm++;
}

/**
 * TGSI transform prolog callback.
 */
static void
aa_prolog(struct tgsi_transform_context *ctx)
{
   struct aa_transform_context *ts = aa_transform_context(ctx);
   unsigned tmp0;
   unsigned texIn;
   unsigned imm;

   /* Declare two temporary registers, one for temporary and
    * one for color.
    */
   ts->tmp = ts->num_tmp++;
   ts->color_tmp = ts->num_tmp++;

   tgsi_transform_temps_decl(ctx, ts->tmp, ts->color_tmp);

   /* Declare new generic input/texcoord */
   texIn = ts->num_input++;
   if (ts->need_texcoord_semantic) {
      tgsi_transform_input_decl(ctx, texIn, TGSI_SEMANTIC_TEXCOORD,
                                ts->aa_point_coord_index,
                                TGSI_INTERPOLATE_LINEAR);
   } else {
      tgsi_transform_input_decl(ctx, texIn, TGSI_SEMANTIC_GENERIC,
                                ts->aa_point_coord_index,
                                TGSI_INTERPOLATE_LINEAR);
   }

   /* Declare extra immediates */
   imm = ts->num_imm++;
   tgsi_transform_immediate_decl(ctx, 0.5, 0.5, 0.45, 1.0);

   /*
    * Emit code to compute fragment coverage.
    * The point always has radius 0.5.  The threshold value will be a
    * value less than, but close to 0.5, such as 0.45.
    * We compute a coverage factor from the distance and threshold.
    * If the coverage is negative, the fragment is outside the circle and
    * it's discarded.
    * If the coverage is >= 1, the fragment is fully inside the threshold
    * distance.  We limit/clamp the coverage to 1.
    * Otherwise, the fragment is between the threshold value and 0.5 and we
    * compute a coverage value in [0,1].
    *
    * Input reg (texIn) usage:
    *  texIn.x = x point coord in [0,1]
    *  texIn.y = y point coord in [0,1]
    *  texIn.z = "k" the smoothing threshold distance
    *  texIn.w = unused
    *
    * Temp reg (t0) usage:
    *  t0.x = distance of fragment from center point
    *  t0.y = boolean, is t0.x > 0.5, also misc temp usage
    *  t0.z = temporary for computing 1/(0.5-k) value
    *  t0.w = final coverage value
    */

   tmp0 = ts->tmp;

   /* SUB t0.xy, texIn, (0.5, 0,5) */
   tgsi_transform_op2_inst(ctx, TGSI_OPCODE_ADD,
                           TGSI_FILE_TEMPORARY, tmp0, TGSI_WRITEMASK_XY,
                           TGSI_FILE_INPUT, texIn,
                           TGSI_FILE_IMMEDIATE, imm, true);

   /* DP2 t0.x, t0.xy, t0.xy;  # t0.x = x^2 + y^2 */
   tgsi_transform_op2_inst(ctx, TGSI_OPCODE_DP2,
                           TGSI_FILE_TEMPORARY, tmp0, TGSI_WRITEMASK_X,
                           TGSI_FILE_TEMPORARY, tmp0,
                           TGSI_FILE_TEMPORARY, tmp0, false);

   /* SQRT t0.x, t0.x */
   tgsi_transform_op1_inst(ctx, TGSI_OPCODE_SQRT,
                           TGSI_FILE_TEMPORARY, tmp0, TGSI_WRITEMASK_X,
                           TGSI_FILE_TEMPORARY, tmp0);

   /* compute coverage factor = (0.5-d)/(0.5-k) */

   /* SUB t0.w, 0.5, texIn.z;  # t0.w = 0.5-k */
   tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_ADD,
                               TGSI_FILE_TEMPORARY, tmp0, TGSI_WRITEMASK_W,
                               TGSI_FILE_IMMEDIATE, imm, TGSI_SWIZZLE_X,
                               TGSI_FILE_INPUT, texIn, TGSI_SWIZZLE_Z, true);

   /* SUB t0.y, 0.5, t0.x;  # t0.y = 0.5-d */
   tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_ADD,
                               TGSI_FILE_TEMPORARY, tmp0, TGSI_WRITEMASK_Y,
                               TGSI_FILE_IMMEDIATE, imm, TGSI_SWIZZLE_X,
                               TGSI_FILE_TEMPORARY, tmp0, TGSI_SWIZZLE_X, true);

   /* DIV t0.w, t0.y, t0.w;  # coverage = (0.5-d)/(0.5-k) */
   tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_DIV,
                               TGSI_FILE_TEMPORARY, tmp0, TGSI_WRITEMASK_W,
                               TGSI_FILE_TEMPORARY, tmp0, TGSI_SWIZZLE_Y,
                               TGSI_FILE_TEMPORARY, tmp0, TGSI_SWIZZLE_W, false);

   /* If the coverage value is negative, it means the fragment is outside
    * the point's circular boundary.  Kill it.
    */
   /* KILL_IF tmp0.w;  # if tmp0.w < 0 KILL */
   tgsi_transform_kill_inst(ctx, TGSI_FILE_TEMPORARY, tmp0,
                            TGSI_SWIZZLE_W, false);

   /* If the distance is less than the threshold, the coverage/alpha value
    * will be greater than one.  Clamp to one here.
    */
   /* MIN tmp0.w, tmp0.w, 1.0 */
   tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_MIN,
                               TGSI_FILE_TEMPORARY, tmp0, TGSI_WRITEMASK_W,
                               TGSI_FILE_TEMPORARY, tmp0, TGSI_SWIZZLE_W,
                               TGSI_FILE_IMMEDIATE, imm, TGSI_SWIZZLE_W, false);
}

/**
 * TGSI instruction transform callback.
 */
static void
aa_inst(struct tgsi_transform_context *ctx,
        struct tgsi_full_instruction *inst)
{
   struct aa_transform_context *ts = aa_transform_context(ctx);
   unsigned i;

   /* Look for writes to color output reg and replace it with
    * color temp reg.
    */
   for (i = 0; i < inst->Instruction.NumDstRegs; i++) {
      struct tgsi_full_dst_register *dst = &inst->Dst[i];
      if (dst->Register.File == TGSI_FILE_OUTPUT &&
	  dst->Register.Index == (int)ts->color_out) {
         dst->Register.File = TGSI_FILE_TEMPORARY;
         dst->Register.Index = ts->color_tmp;
      }
   }

   ctx->emit_instruction(ctx, inst);
}

/**
 * TGSI transform epilog callback.
 */
static void
aa_epilog(struct tgsi_transform_context *ctx)
{
   struct aa_transform_context *ts = aa_transform_context(ctx);

   /* add alpha modulation code at tail of program */
   assert(ts->color_out != INVALID_INDEX);
   assert(ts->color_tmp != INVALID_INDEX);

   /* MOV output.color.xyz colorTmp */
   tgsi_transform_op1_inst(ctx, TGSI_OPCODE_MOV,
                           TGSI_FILE_OUTPUT, ts->color_out,
                           TGSI_WRITEMASK_XYZ,
                           TGSI_FILE_TEMPORARY, ts->color_tmp);

   /* MUL output.color.w colorTmp.w tmp0.w */
   tgsi_transform_op2_inst(ctx, TGSI_OPCODE_MUL,
                           TGSI_FILE_OUTPUT, ts->color_out,
                           TGSI_WRITEMASK_W,
                           TGSI_FILE_TEMPORARY, ts->color_tmp,
                           TGSI_FILE_TEMPORARY, ts->tmp, false);
}

/**
 * TGSI utility to transform a fragment shader to support antialiasing point.
 *
 * This utility accepts two inputs:
 *\param tokens_in  -- the original token string of the shader
 *\param aa_point_coord_index -- the semantic index of the generic register
 *                            that contains the point sprite texture coord
 *
 * For each fragment in the point, we compute the distance of the fragment
 * from the point center using the point sprite texture coordinates.
 * If the distance is greater than 0.5, we'll discard the fragment.
 * Otherwise, we'll compute a coverage value which approximates how much
 * of the fragment is inside the bounding circle of the point. If the distance
 * is less than 'k', the coverage is 1. Else, the coverage is between 0 and 1.
 * The final fragment color's alpha channel is then modulated by the coverage
 * value.
 */
struct tgsi_token *
tgsi_add_aa_point(const struct tgsi_token *tokens_in,
                  const int aa_point_coord_index,
                  const bool need_texcoord_semantic)
{
   struct aa_transform_context transform;
   const unsigned num_new_tokens = 200; /* should be enough */
   const unsigned new_len = tgsi_num_tokens(tokens_in) + num_new_tokens;

   /* setup transformation context */
   memset(&transform, 0, sizeof(transform));
   transform.base.transform_declaration = aa_decl;
   transform.base.transform_instruction = aa_inst;
   transform.base.transform_immediate = aa_immediate;
   transform.base.prolog = aa_prolog;
   transform.base.epilog = aa_epilog;

   transform.tmp = INVALID_INDEX;
   transform.color_out = INVALID_INDEX;
   transform.color_tmp = INVALID_INDEX;

   assert(aa_point_coord_index != -1);
   transform.aa_point_coord_index = (unsigned)aa_point_coord_index;
   transform.need_texcoord_semantic = need_texcoord_semantic;

   transform.num_tmp = 0;
   transform.num_imm = 0;
   transform.num_input = 0;

   return tgsi_transform_shader(tokens_in, new_len, &transform.base);
}
