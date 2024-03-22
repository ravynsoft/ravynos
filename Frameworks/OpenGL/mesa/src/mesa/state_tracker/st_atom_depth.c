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
  *   Zack  Rusin
  */
 

#include <assert.h>

#include "st_context.h"
#include "st_atom.h"
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "cso_cache/cso_context.h"
#include "main/samplerobj.h"
#include "main/stencil.h"


/**
 * Convert GLenum stencil op tokens to pipe tokens.
 */
static GLuint
gl_stencil_op_to_pipe(GLenum func)
{
   switch (func) {
   case GL_KEEP:
      return PIPE_STENCIL_OP_KEEP;
   case GL_ZERO:
      return PIPE_STENCIL_OP_ZERO;
   case GL_REPLACE:
      return PIPE_STENCIL_OP_REPLACE;
   case GL_INCR:
      return PIPE_STENCIL_OP_INCR;
   case GL_DECR:
      return PIPE_STENCIL_OP_DECR;
   case GL_INCR_WRAP:
      return PIPE_STENCIL_OP_INCR_WRAP;
   case GL_DECR_WRAP:
      return PIPE_STENCIL_OP_DECR_WRAP;
   case GL_INVERT:
      return PIPE_STENCIL_OP_INVERT;
   default:
      assert("invalid GL token in gl_stencil_op_to_pipe()" == NULL);
      return 0;
   }
}

void
st_update_depth_stencil_alpha(struct st_context *st)
{
   struct pipe_depth_stencil_alpha_state *dsa = &st->state.depth_stencil;
   struct pipe_stencil_ref sr;
   struct gl_context *ctx = st->ctx;

   memset(dsa, 0, sizeof(*dsa));
   memset(&sr, 0, sizeof(sr));

   if (ctx->DrawBuffer->Visual.depthBits > 0) {
      if (ctx->Depth.Test) {
         dsa->depth_enabled = 1;
         dsa->depth_func = func_to_gallium(ctx->Depth.Func);
         if (dsa->depth_func != PIPE_FUNC_EQUAL)
            dsa->depth_writemask = ctx->Depth.Mask;
      }
      if (ctx->Depth.BoundsTest) {
         dsa->depth_bounds_test = 1;
         dsa->depth_bounds_min = ctx->Depth.BoundsMin;
         dsa->depth_bounds_max = ctx->Depth.BoundsMax;
      }
   }

   if (ctx->Stencil.Enabled && ctx->DrawBuffer->Visual.stencilBits > 0) {
      dsa->stencil[0].enabled = 1;
      dsa->stencil[0].func = func_to_gallium(ctx->Stencil.Function[0]);
      dsa->stencil[0].fail_op = gl_stencil_op_to_pipe(ctx->Stencil.FailFunc[0]);
      dsa->stencil[0].zfail_op = gl_stencil_op_to_pipe(ctx->Stencil.ZFailFunc[0]);
      dsa->stencil[0].zpass_op = gl_stencil_op_to_pipe(ctx->Stencil.ZPassFunc[0]);
      dsa->stencil[0].valuemask = ctx->Stencil.ValueMask[0] & 0xff;
      dsa->stencil[0].writemask = ctx->Stencil.WriteMask[0] & 0xff;
      sr.ref_value[0] = _mesa_get_stencil_ref(ctx, 0);

      if (_mesa_stencil_is_two_sided(ctx)) {
         const GLuint back = ctx->Stencil._BackFace;
         dsa->stencil[1].enabled = 1;
         dsa->stencil[1].func = func_to_gallium(ctx->Stencil.Function[back]);
         dsa->stencil[1].fail_op = gl_stencil_op_to_pipe(ctx->Stencil.FailFunc[back]);
         dsa->stencil[1].zfail_op = gl_stencil_op_to_pipe(ctx->Stencil.ZFailFunc[back]);
         dsa->stencil[1].zpass_op = gl_stencil_op_to_pipe(ctx->Stencil.ZPassFunc[back]);
         dsa->stencil[1].valuemask = ctx->Stencil.ValueMask[back] & 0xff;
         dsa->stencil[1].writemask = ctx->Stencil.WriteMask[back] & 0xff;
         sr.ref_value[1] = _mesa_get_stencil_ref(ctx, back);
      }
      else {
         /* This should be unnecessary. Drivers must not expect this to
          * contain valid data, except the enabled bit
          */
         dsa->stencil[1] = dsa->stencil[0];
         dsa->stencil[1].enabled = 0;
         sr.ref_value[1] = sr.ref_value[0];
      }
   }

   if (ctx->Color.AlphaEnabled && !st->lower_alpha_test &&
       !(ctx->DrawBuffer->_IntegerBuffers & 0x1)) {
      dsa->alpha_enabled = 1;
      dsa->alpha_func = func_to_gallium(ctx->Color.AlphaFunc);
      dsa->alpha_ref_value = ctx->Color.AlphaRefUnclamped;
   }

   cso_set_depth_stencil_alpha(st->cso_context, dsa);
   cso_set_stencil_ref(st->cso_context, sr);
}
