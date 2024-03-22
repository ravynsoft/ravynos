/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
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
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  *   Brian Paul
  */
 

#include "st_context.h"
#include "st_atom.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "cso_cache/cso_context.h"

#include "framebuffer.h"
#include "main/blend.h"
#include "main/glformats.h"
#include "main/macros.h"

/**
 * Convert GLenum blend tokens to pipe tokens.
 * Both blend factors and blend funcs are accepted.
 */
static GLuint
translate_blend(GLenum blend)
{
   switch (blend) {
   /* blend functions */
   case GL_FUNC_ADD:
      return PIPE_BLEND_ADD;
   case GL_FUNC_SUBTRACT:
      return PIPE_BLEND_SUBTRACT;
   case GL_FUNC_REVERSE_SUBTRACT:
      return PIPE_BLEND_REVERSE_SUBTRACT;
   case GL_MIN:
      return PIPE_BLEND_MIN;
   case GL_MAX:
      return PIPE_BLEND_MAX;

   /* blend factors */
   case GL_ONE:
      return PIPE_BLENDFACTOR_ONE;
   case GL_SRC_COLOR:
      return PIPE_BLENDFACTOR_SRC_COLOR;
   case GL_SRC_ALPHA:
      return PIPE_BLENDFACTOR_SRC_ALPHA;
   case GL_DST_ALPHA:
      return PIPE_BLENDFACTOR_DST_ALPHA;
   case GL_DST_COLOR:
      return PIPE_BLENDFACTOR_DST_COLOR;
   case GL_SRC_ALPHA_SATURATE:
      return PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE;
   case GL_CONSTANT_COLOR:
      return PIPE_BLENDFACTOR_CONST_COLOR;
   case GL_CONSTANT_ALPHA:
      return PIPE_BLENDFACTOR_CONST_ALPHA;
   case GL_SRC1_COLOR:
      return PIPE_BLENDFACTOR_SRC1_COLOR;
   case GL_SRC1_ALPHA:
      return PIPE_BLENDFACTOR_SRC1_ALPHA;
   case GL_ZERO:
      return PIPE_BLENDFACTOR_ZERO;
   case GL_ONE_MINUS_SRC_COLOR:
      return PIPE_BLENDFACTOR_INV_SRC_COLOR;
   case GL_ONE_MINUS_SRC_ALPHA:
      return PIPE_BLENDFACTOR_INV_SRC_ALPHA;
   case GL_ONE_MINUS_DST_COLOR:
      return PIPE_BLENDFACTOR_INV_DST_COLOR;
   case GL_ONE_MINUS_DST_ALPHA:
      return PIPE_BLENDFACTOR_INV_DST_ALPHA;
   case GL_ONE_MINUS_CONSTANT_COLOR:
      return PIPE_BLENDFACTOR_INV_CONST_COLOR;
   case GL_ONE_MINUS_CONSTANT_ALPHA:
      return PIPE_BLENDFACTOR_INV_CONST_ALPHA;
   case GL_ONE_MINUS_SRC1_COLOR:
      return PIPE_BLENDFACTOR_INV_SRC1_COLOR;
   case GL_ONE_MINUS_SRC1_ALPHA:
      return PIPE_BLENDFACTOR_INV_SRC1_ALPHA;
   default:
      assert("invalid GL token in translate_blend()" == NULL);
      return 0;
   }
}

/**
 * Figure out if colormasks are different per rt.
 */
static GLboolean
colormask_per_rt(const struct gl_context *ctx, unsigned num_cb)
{
   GLbitfield full_mask = _mesa_replicate_colormask(0xf, num_cb);
   GLbitfield repl_mask0 =
      _mesa_replicate_colormask(GET_COLORMASK(ctx->Color.ColorMask, 0),
                                num_cb);

   return (ctx->Color.ColorMask & full_mask) != repl_mask0;
}

/**
 * Decide whether to allow promotion of RGB colormasks (0x7) to RGBA (0xf).
 */
static bool
allow_rgb_colormask_promotion(const struct st_context *st,
                              unsigned num_cb,
                              bool *need_independent_blend)
{
   const struct gl_context *ctx = st->ctx;

   if (num_cb == 1)
      return true;

   GLbitfield rgb_mask = _mesa_replicate_colormask(0x7, num_cb);
   GLbitfield full_mask = _mesa_replicate_colormask(0xf, num_cb);

   /* True if all colormasks should be promoted.  If so, we can do so
    * without needing independent blending.  (If none should be promoted,
    * we can just skip this optimization as it doesn't do anything.)
    */
   bool same = ctx->DrawBuffer->_IsRGB == u_bit_consecutive(0, num_cb) &&
               (ctx->Color.ColorMask & full_mask) == rgb_mask;

   /* We can support different per-RT promotion decisions if we driver
    * supports independent blending (but we must actually enable it).
    */
   if (st->has_indep_blend_enable && !same) {
      *need_independent_blend = true;
      return true;
   }

   return same;
}

/**
 * Figure out if blend enables/state are different per rt.
 */
static GLboolean
blend_per_rt(const struct st_context *st, unsigned num_cb)
{
   const struct gl_context *ctx = st->ctx;
   GLbitfield cb_mask = u_bit_consecutive(0, num_cb);
   GLbitfield blend_enabled = ctx->Color.BlendEnabled & cb_mask;

   if (blend_enabled && blend_enabled != cb_mask) {
      /* This can only happen if GL_EXT_draw_buffers2 is enabled */
      return GL_TRUE;
   }
   if (ctx->Color._BlendFuncPerBuffer || ctx->Color._BlendEquationPerBuffer) {
      /* this can only happen if GL_ARB_draw_buffers_blend is enabled */
      return GL_TRUE;
   }
   if (ctx->DrawBuffer->_IntegerBuffers &&
       (ctx->DrawBuffer->_IntegerBuffers != cb_mask)) {
      /* If there is a mix of integer/non-integer buffers then blending
       * must be handled on a per buffer basis. */
      return GL_TRUE;
   }

   if (ctx->DrawBuffer->_BlendForceAlphaToOne) {
      /* Overriding requires independent blend functions (not just enables),
       * requiring drivers to expose PIPE_CAP_INDEP_BLEND_FUNC.
       */
      assert(st->has_indep_blend_func);

      /* If some of the buffers are RGB or emulated L/I, we may need to override blend
       * factors that reference destination-alpha to constants.  We may
       * need different blend factor overrides per buffer (say one uses
       * a DST_ALPHA factor and another uses INV_DST_ALPHA), so we flip
       * on independent blending.  This may not be required in all cases,
       * but burning the CPU to figure it out is probably not worthwhile.
       */
      return GL_TRUE;
   }

   return GL_FALSE;
}

/**
 * Modify blend function to force destination alpha to 1.0
 *
 * If \c function specifies a blend function that uses destination alpha,
 * replace it with a function that hard-wires destination alpha to 1.0.
 * This is useful when emulating a GL RGB format with an RGBA pipe_format.
 */
static enum pipe_blendfactor
fix_xrgb_alpha(enum pipe_blendfactor factor)
{
   switch (factor) {
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return PIPE_BLENDFACTOR_ONE;

   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return PIPE_BLENDFACTOR_ZERO;
   default:
      return factor;
   }
}

void
st_update_blend( struct st_context *st )
{
   struct pipe_blend_state *blend = &st->state.blend;
   const struct gl_context *ctx = st->ctx;
   unsigned num_cb = st->state.fb_num_cb;
   unsigned num_state = 1;
   unsigned i, j;

   memset(blend, 0, sizeof(*blend));

   blend->max_rt = MAX2(1, num_cb) - 1;

   bool need_independent_blend = num_cb > 1 &&
      (blend_per_rt(st, num_cb) || colormask_per_rt(ctx, num_cb));

   bool promote_rgb_colormasks =
      allow_rgb_colormask_promotion(st, num_cb, &need_independent_blend);

   if (need_independent_blend) {
      num_state = num_cb;
      blend->independent_blend_enable = 1;
   }

   for (i = 0; i < num_state; i++) {
      unsigned colormask = GET_COLORMASK(ctx->Color.ColorMask, i);

      /* When faking RGB as RGBA and writing every real channel, also enable
       * writes to the A channel as well.  Some GPUs are able to render more
       * efficiently if they know whole pixels are being overwritten, whereas
       * partial writes may require preserving/combining new and old data.
       */
      if (promote_rgb_colormasks &&
          colormask == 0x7 && (ctx->DrawBuffer->_IsRGB & (1 << i)))
         colormask = 0xf;

      blend->rt[i].colormask = colormask;
   }

   if (ctx->Color._AdvancedBlendMode != BLEND_NONE) {
      blend->blend_coherent = ctx->Color.BlendCoherent;
   }

   if (ctx->Color.ColorLogicOpEnabled) {
      /* logicop enabled */
      blend->logicop_enable = 1;
      blend->logicop_func = ctx->Color._LogicOp;
   }
   else if (ctx->Color.BlendEnabled &&
            ctx->Color._AdvancedBlendMode != BLEND_NONE) {
      blend->advanced_blend_func = ctx->Color._AdvancedBlendMode;
   }
   else if (ctx->Color.BlendEnabled &&
            ctx->Color._AdvancedBlendMode == BLEND_NONE) {
      /* blending enabled */
      for (i = 0, j = 0; i < num_state; i++) {
         if (!(ctx->Color.BlendEnabled & (1 << i)) ||
             (ctx->DrawBuffer->_IntegerBuffers & (1 << i)) ||
             !blend->rt[i].colormask)
            continue;

         if (ctx->Extensions.ARB_draw_buffers_blend)
            j = i;

         blend->rt[i].blend_enable = 1;
         blend->rt[i].rgb_func =
            translate_blend(ctx->Color.Blend[j].EquationRGB);

         if (ctx->Color.Blend[i].EquationRGB == GL_MIN ||
             ctx->Color.Blend[i].EquationRGB == GL_MAX) {
            /* Min/max are special */
            blend->rt[i].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
            blend->rt[i].rgb_dst_factor = PIPE_BLENDFACTOR_ONE;
         }
         else {
            blend->rt[i].rgb_src_factor =
               translate_blend(ctx->Color.Blend[j].SrcRGB);
            blend->rt[i].rgb_dst_factor =
               translate_blend(ctx->Color.Blend[j].DstRGB);
         }

         blend->rt[i].alpha_func =
            translate_blend(ctx->Color.Blend[j].EquationA);

         if (ctx->Color.Blend[i].EquationA == GL_MIN ||
             ctx->Color.Blend[i].EquationA == GL_MAX) {
            /* Min/max are special */
            blend->rt[i].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
            blend->rt[i].alpha_dst_factor = PIPE_BLENDFACTOR_ONE;
         }
         else {
            blend->rt[i].alpha_src_factor =
               translate_blend(ctx->Color.Blend[j].SrcA);
            blend->rt[i].alpha_dst_factor =
               translate_blend(ctx->Color.Blend[j].DstA);
         }

         const struct gl_renderbuffer *rb =
            ctx->DrawBuffer->_ColorDrawBuffers[i];

         if (rb && (ctx->DrawBuffer->_BlendForceAlphaToOne & (1 << i))) {
            struct pipe_rt_blend_state *rt = &blend->rt[i];
            rt->rgb_src_factor = fix_xrgb_alpha(rt->rgb_src_factor);
            rt->rgb_dst_factor = fix_xrgb_alpha(rt->rgb_dst_factor);
            rt->alpha_src_factor = fix_xrgb_alpha(rt->alpha_src_factor);
            rt->alpha_dst_factor = fix_xrgb_alpha(rt->alpha_dst_factor);
         }
      }
   }
   else {
      /* no blending / logicop */
   }

   if (st->can_dither)
      blend->dither = ctx->Color.DitherFlag;

   if (_mesa_is_multisample_enabled(ctx) &&
       !(ctx->DrawBuffer->_IntegerBuffers & 0x1)) {
      /* Unlike in gallium/d3d10 these operations are only performed
       * if both msaa is enabled and we have a multisample buffer.
       */
      blend->alpha_to_coverage = ctx->Multisample.SampleAlphaToCoverage;
      blend->alpha_to_one = ctx->Multisample.SampleAlphaToOne;
      blend->alpha_to_coverage_dither =
         ctx->Multisample.SampleAlphaToCoverageDitherControl !=
         GL_ALPHA_TO_COVERAGE_DITHER_DISABLE_NV;
   }

   cso_set_blend(st->cso_context, blend);
}

void
st_update_blend_color(struct st_context *st)
{
   struct pipe_context *pipe = st->pipe;
   struct pipe_blend_color *bc =
      (struct pipe_blend_color *)st->ctx->Color.BlendColorUnclamped;

   pipe->set_blend_color(pipe, bc);
}
