/*
 * Copyright © 2020 Advanced Micro Devices, Inc.
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

#include "main/glthread_marshal.h"
#include "main/dispatch.h"

uint32_t
_mesa_unmarshal_GetIntegerv(struct gl_context *ctx,
                            const struct marshal_cmd_GetIntegerv *restrict cmd)
{
   unreachable("never executed");
   return 0;
}

void GLAPIENTRY
_mesa_marshal_GetIntegerv(GLenum pname, GLint *p)
{
   GET_CURRENT_CONTEXT(ctx);

   /* This will generate GL_INVALID_OPERATION, as it should. */
   if (ctx->GLThread.inside_begin_end)
      goto sync;

   /* TODO: Use get_hash_params.py to return values for items containing:
    * - CONST(
    * - CONTEXT_[A-Z]*(Const
    */

   switch (pname) {
   case GL_ACTIVE_TEXTURE:
      *p = GL_TEXTURE0 + ctx->GLThread.ActiveTexture;
      return;
   case GL_ARRAY_BUFFER_BINDING:
      *p = ctx->GLThread.CurrentArrayBufferName;
      return;
   case GL_ATTRIB_STACK_DEPTH:
      *p = ctx->GLThread.AttribStackDepth;
      return;
   case GL_CLIENT_ACTIVE_TEXTURE:
      *p = GL_TEXTURE0 + ctx->GLThread.ClientActiveTexture;
      return;
   case GL_CLIENT_ATTRIB_STACK_DEPTH:
      *p = ctx->GLThread.ClientAttribStackTop;
      return;
   case GL_CURRENT_PROGRAM:
      *p = ctx->GLThread.CurrentProgram;
      return;
   case GL_DRAW_INDIRECT_BUFFER_BINDING:
      *p = ctx->GLThread.CurrentDrawIndirectBufferName;
      return;
   case GL_DRAW_FRAMEBUFFER_BINDING:
      *p = ctx->GLThread.CurrentDrawFramebuffer;
      return;
   case GL_READ_FRAMEBUFFER_BINDING:
      *p = ctx->GLThread.CurrentReadFramebuffer;
      return;
   case GL_PIXEL_PACK_BUFFER_BINDING:
      *p = ctx->GLThread.CurrentPixelPackBufferName;
      return;
   case GL_PIXEL_UNPACK_BUFFER_BINDING:
      *p = ctx->GLThread.CurrentPixelUnpackBufferName;
      return;
   case GL_QUERY_BUFFER_BINDING:
      *p = ctx->GLThread.CurrentQueryBufferName;
      return;

   case GL_MATRIX_MODE:
      *p = ctx->GLThread.MatrixMode;
      return;
   case GL_CURRENT_MATRIX_STACK_DEPTH_ARB:
      *p = ctx->GLThread.MatrixStackDepth[ctx->GLThread.MatrixIndex] + 1;
      return;
   case GL_MODELVIEW_STACK_DEPTH:
      *p = ctx->GLThread.MatrixStackDepth[M_MODELVIEW] + 1;
      return;
   case GL_PROJECTION_STACK_DEPTH:
      *p = ctx->GLThread.MatrixStackDepth[M_PROJECTION] + 1;
      return;
   case GL_TEXTURE_STACK_DEPTH:
      *p = ctx->GLThread.MatrixStackDepth[M_TEXTURE0 + ctx->GLThread.ActiveTexture] + 1;
      return;

   case GL_VERTEX_ARRAY:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled & (1 << VERT_ATTRIB_POS)) != 0;
      return;
   case GL_NORMAL_ARRAY:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled & (1 << VERT_ATTRIB_NORMAL)) != 0;
      return;
   case GL_COLOR_ARRAY:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled & (1 << VERT_ATTRIB_COLOR0)) != 0;
      return;
   case GL_SECONDARY_COLOR_ARRAY:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled & (1 << VERT_ATTRIB_COLOR1)) != 0;
      return;
   case GL_FOG_COORD_ARRAY:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled & (1 << VERT_ATTRIB_FOG)) != 0;
      return;
   case GL_INDEX_ARRAY:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled & (1 << VERT_ATTRIB_COLOR_INDEX)) != 0;
      return;
   case GL_EDGE_FLAG_ARRAY:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled & (1 << VERT_ATTRIB_EDGEFLAG)) != 0;
      return;
   case GL_TEXTURE_COORD_ARRAY:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled &
            (1 << (VERT_ATTRIB_TEX0 + ctx->GLThread.ClientActiveTexture))) != 0;
      return;
   case GL_POINT_SIZE_ARRAY_OES:
      *p = (ctx->GLThread.CurrentVAO->UserEnabled & (1 << VERT_ATTRIB_POINT_SIZE)) != 0;
      return;
   }

sync:
   _mesa_glthread_finish_before(ctx, "GetIntegerv");
   CALL_GetIntegerv(ctx->Dispatch.Current, (pname, p));
}

/* TODO: Implement glGetBooleanv, glGetFloatv, etc. if needed */
