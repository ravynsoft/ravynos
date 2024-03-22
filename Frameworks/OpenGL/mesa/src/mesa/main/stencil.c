/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * \file stencil.c
 * Stencil operations.
 *
 * Note: There's some conflict between GL_EXT_stencil_two_side and
 * OpenGL 2.0's two-sided stencil feature.
 *
 * With GL_EXT_stencil_two_side, calling glStencilOp/Func/Mask() only the
 * front OR back face state (as set by glActiveStencilFaceEXT) is set.
 *
 * But with OpenGL 2.0, calling glStencilOp/Func/Mask() sets BOTH the
 * front AND back state.
 *
 * Also, note that GL_ATI_separate_stencil is different as well:
 * glStencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, ...)  vs.
 * glStencilFuncSeparate(GLenum face, GLenum func, ...).
 *
 * This problem is solved by keeping three sets of stencil state:
 *  state[0] = GL_FRONT state.
 *  state[1] = OpenGL 2.0 / GL_ATI_separate_stencil GL_BACK state.
 *  state[2] = GL_EXT_stencil_two_side GL_BACK state.
 */


#include "util/glheader.h"

#include "context.h"
#include "macros.h"
#include "stencil.h"
#include "mtypes.h"
#include "api_exec_decl.h"

#include "state_tracker/st_context.h"

static GLboolean
validate_stencil_op(struct gl_context *ctx, GLenum op)
{
   switch (op) {
   case GL_KEEP:
   case GL_ZERO:
   case GL_REPLACE:
   case GL_INCR:
   case GL_DECR:
   case GL_INVERT:
   case GL_INCR_WRAP:
   case GL_DECR_WRAP:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}


static GLboolean
validate_stencil_func(struct gl_context *ctx, GLenum func)
{
   switch (func) {
   case GL_NEVER:
   case GL_LESS:
   case GL_LEQUAL:
   case GL_GREATER:
   case GL_GEQUAL:
   case GL_EQUAL:
   case GL_NOTEQUAL:
   case GL_ALWAYS:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}


/**
 * Set the clear value for the stencil buffer.
 *
 * \param s clear value.
 *
 * \sa glClearStencil().
 *
 * Updates gl_stencil_attrib::Clear. On change
 * flushes the vertices and notifies the driver via
 * the dd_function_table::ClearStencil callback.
 */
void GLAPIENTRY
_mesa_ClearStencil( GLint s )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glClearStencil(%d)\n", s);

   ctx->PopAttribState |= GL_STENCIL_BUFFER_BIT;
   ctx->Stencil.Clear = (GLuint) s;
}


/**
 * Set the function and reference value for stencil testing.
 *
 * \param frontfunc front test function.
 * \param backfunc back test function.
 * \param ref front and back reference value.
 * \param mask front and back bitmask.
 *
 * \sa glStencilFunc().
 *
 * Verifies the parameters and updates the respective values in
 * __struct gl_contextRec::Stencil. On change flushes the vertices and notifies
 * the driver via the dd_function_table::StencilFunc callback.
 */
void GLAPIENTRY
_mesa_StencilFuncSeparateATI( GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glStencilFuncSeparateATI()\n");

   if (!validate_stencil_func(ctx, frontfunc)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glStencilFuncSeparateATI(frontfunc)");
      return;
   }
   if (!validate_stencil_func(ctx, backfunc)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glStencilFuncSeparateATI(backfunc)");
      return;
   }

   /* set both front and back state */
   if (ctx->Stencil.Function[0] == frontfunc &&
       ctx->Stencil.Function[1] == backfunc &&
       ctx->Stencil.ValueMask[0] == mask &&
       ctx->Stencil.ValueMask[1] == mask &&
       ctx->Stencil.Ref[0] == ref &&
       ctx->Stencil.Ref[1] == ref)
      return;
   FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_DSA;
   ctx->Stencil.Function[0]  = frontfunc;
   ctx->Stencil.Function[1]  = backfunc;
   ctx->Stencil.Ref[0]       = ctx->Stencil.Ref[1]       = ref;
   ctx->Stencil.ValueMask[0] = ctx->Stencil.ValueMask[1] = mask;
}


/**
 * Set the function and reference value for stencil testing.
 *
 * \param func test function.
 * \param ref reference value.
 * \param mask bitmask.
 *
 * \sa glStencilFunc().
 *
 * Verifies the parameters and updates the respective values in
 * __struct gl_contextRec::Stencil. On change flushes the vertices and notifies
 * the driver via the dd_function_table::StencilFunc callback.
 */
static void
stencil_func(struct gl_context *ctx, GLenum func, GLint ref, GLuint mask)
{
   const GLint face = ctx->Stencil.ActiveFace;

   if (face != 0) {
      if (ctx->Stencil.Function[face] == func &&
          ctx->Stencil.ValueMask[face] == mask &&
          ctx->Stencil.Ref[face] == ref)
         return;
      FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
      ctx->NewDriverState |= ST_NEW_DSA;
      ctx->Stencil.Function[face] = func;
      ctx->Stencil.Ref[face] = ref;
      ctx->Stencil.ValueMask[face] = mask;
   }
   else {
      /* set both front and back state */
      if (ctx->Stencil.Function[0] == func &&
          ctx->Stencil.Function[1] == func &&
          ctx->Stencil.ValueMask[0] == mask &&
          ctx->Stencil.ValueMask[1] == mask &&
          ctx->Stencil.Ref[0] == ref &&
          ctx->Stencil.Ref[1] == ref)
         return;
      FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
      ctx->NewDriverState |= ST_NEW_DSA;
      ctx->Stencil.Function[0]  = ctx->Stencil.Function[1]  = func;
      ctx->Stencil.Ref[0]       = ctx->Stencil.Ref[1]       = ref;
      ctx->Stencil.ValueMask[0] = ctx->Stencil.ValueMask[1] = mask;
   }
}


void GLAPIENTRY
_mesa_StencilFunc_no_error(GLenum func, GLint ref, GLuint mask)
{
   GET_CURRENT_CONTEXT(ctx);
   stencil_func(ctx, func, ref, mask);
}


void GLAPIENTRY
_mesa_StencilFunc(GLenum func, GLint ref, GLuint mask)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glStencilFunc()\n");

   if (!validate_stencil_func(ctx, func)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilFunc(func)");
      return;
   }

   stencil_func(ctx, func, ref, mask);
}


/**
 * Set the stencil writing mask.
 *
 * \param mask bit-mask to enable/disable writing of individual bits in the
 * stencil planes.
 *
 * \sa glStencilMask().
 *
 * Updates gl_stencil_attrib::WriteMask. On change flushes the vertices and
 * notifies the driver via the dd_function_table::StencilMask callback.
 */
void GLAPIENTRY
_mesa_StencilMask( GLuint mask )
{
   GET_CURRENT_CONTEXT(ctx);
   const GLint face = ctx->Stencil.ActiveFace;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glStencilMask()\n");

   if (face != 0) {
      /* Only modify the EXT_stencil_two_side back-face state.
       */
      if (ctx->Stencil.WriteMask[face] == mask)
         return;
      FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
      ctx->NewDriverState |= ST_NEW_DSA;
      ctx->Stencil.WriteMask[face] = mask;
   }
   else {
      /* set both front and back state */
      if (ctx->Stencil.WriteMask[0] == mask &&
          ctx->Stencil.WriteMask[1] == mask)
         return;
      FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
      ctx->NewDriverState |= ST_NEW_DSA;
      ctx->Stencil.WriteMask[0] = ctx->Stencil.WriteMask[1] = mask;
   }
}


/**
 * Set the stencil test actions.
 *
 * \param fail action to take when stencil test fails.
 * \param zfail action to take when stencil test passes, but depth test fails.
 * \param zpass action to take when stencil test passes and the depth test
 * passes (or depth testing is not enabled).
 *
 * \sa glStencilOp().
 *
 * Verifies the parameters and updates the respective fields in
 * __struct gl_contextRec::Stencil. On change flushes the vertices and notifies
 * the driver via the dd_function_table::StencilOp callback.
 */
static void
stencil_op(struct gl_context *ctx, GLenum fail, GLenum zfail, GLenum zpass)
{
   const GLint face = ctx->Stencil.ActiveFace;

   if (face != 0) {
      /* only set active face state */
      if (ctx->Stencil.ZFailFunc[face] == zfail &&
          ctx->Stencil.ZPassFunc[face] == zpass &&
          ctx->Stencil.FailFunc[face] == fail)
         return;
      FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
      ctx->NewDriverState |= ST_NEW_DSA;
      ctx->Stencil.ZFailFunc[face] = zfail;
      ctx->Stencil.ZPassFunc[face] = zpass;
      ctx->Stencil.FailFunc[face] = fail;
   }
   else {
      /* set both front and back state */
      if (ctx->Stencil.ZFailFunc[0] == zfail &&
          ctx->Stencil.ZFailFunc[1] == zfail &&
          ctx->Stencil.ZPassFunc[0] == zpass &&
          ctx->Stencil.ZPassFunc[1] == zpass &&
          ctx->Stencil.FailFunc[0] == fail &&
          ctx->Stencil.FailFunc[1] == fail)
         return;
      FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
      ctx->NewDriverState |= ST_NEW_DSA;
      ctx->Stencil.ZFailFunc[0] = ctx->Stencil.ZFailFunc[1] = zfail;
      ctx->Stencil.ZPassFunc[0] = ctx->Stencil.ZPassFunc[1] = zpass;
      ctx->Stencil.FailFunc[0]  = ctx->Stencil.FailFunc[1]  = fail;
   }
}


void GLAPIENTRY
_mesa_StencilOp_no_error(GLenum fail, GLenum zfail, GLenum zpass)
{
   GET_CURRENT_CONTEXT(ctx);
   stencil_op(ctx, fail, zfail, zpass);
}


void GLAPIENTRY
_mesa_StencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glStencilOp()\n");

   if (!validate_stencil_op(ctx, fail)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilOp(sfail)");
      return;
   }

   if (!validate_stencil_op(ctx, zfail)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilOp(zfail)");
      return;
   }

   if (!validate_stencil_op(ctx, zpass)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilOp(zpass)");
      return;
   }

   stencil_op(ctx, fail, zfail, zpass);
}


/* GL_EXT_stencil_two_side */
void GLAPIENTRY
_mesa_ActiveStencilFaceEXT(GLenum face)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glActiveStencilFaceEXT()\n");

   if (!ctx->Extensions.EXT_stencil_two_side) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glActiveStencilFaceEXT");
      return;
   }

   if (face == GL_FRONT || face == GL_BACK) {
      ctx->Stencil.ActiveFace = (face == GL_FRONT) ? 0 : 2;
   }
   else {
      _mesa_error(ctx, GL_INVALID_ENUM, "glActiveStencilFaceEXT(face)");
   }
}


static void
stencil_op_separate(struct gl_context *ctx, GLenum face, GLenum sfail,
                    GLenum zfail, GLenum zpass)
{
   if (face != GL_BACK) {
      /* set front */
      if (ctx->Stencil.ZFailFunc[0] != zfail ||
          ctx->Stencil.ZPassFunc[0] != zpass ||
          ctx->Stencil.FailFunc[0] != sfail){
         FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
         ctx->NewDriverState |= ST_NEW_DSA;
         ctx->Stencil.ZFailFunc[0] = zfail;
         ctx->Stencil.ZPassFunc[0] = zpass;
         ctx->Stencil.FailFunc[0] = sfail;
      }
   }

   if (face != GL_FRONT) {
      /* set back */
      if (ctx->Stencil.ZFailFunc[1] != zfail ||
          ctx->Stencil.ZPassFunc[1] != zpass ||
          ctx->Stencil.FailFunc[1] != sfail) {
         FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
         ctx->NewDriverState |= ST_NEW_DSA;
         ctx->Stencil.ZFailFunc[1] = zfail;
         ctx->Stencil.ZPassFunc[1] = zpass;
         ctx->Stencil.FailFunc[1] = sfail;
      }
   }
}


void GLAPIENTRY
_mesa_StencilOpSeparate_no_error(GLenum face, GLenum sfail, GLenum zfail,
                                 GLenum zpass)
{
   GET_CURRENT_CONTEXT(ctx);
   stencil_op_separate(ctx, face, sfail, zfail, zpass);
}


void GLAPIENTRY
_mesa_StencilOpSeparate(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glStencilOpSeparate()\n");

   if (!validate_stencil_op(ctx, sfail)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilOpSeparate(sfail)");
      return;
   }

   if (!validate_stencil_op(ctx, zfail)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilOpSeparate(zfail)");
      return;
   }

   if (!validate_stencil_op(ctx, zpass)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilOpSeparate(zpass)");
      return;
   }

   if (face != GL_FRONT && face != GL_BACK && face != GL_FRONT_AND_BACK) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilOpSeparate(face)");
      return;
   }

   stencil_op_separate(ctx, face, sfail, zfail, zpass);
}


static void
stencil_func_separate(struct gl_context *ctx, GLenum face, GLenum func,
                      GLint ref, GLuint mask)
{
   FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_DSA;

   if (face != GL_BACK) {
      /* set front */
      ctx->Stencil.Function[0] = func;
      ctx->Stencil.Ref[0] = ref;
      ctx->Stencil.ValueMask[0] = mask;
   }

   if (face != GL_FRONT) {
      /* set back */
      ctx->Stencil.Function[1] = func;
      ctx->Stencil.Ref[1] = ref;
      ctx->Stencil.ValueMask[1] = mask;
   }
}


/* OpenGL 2.0 */
void GLAPIENTRY
_mesa_StencilFuncSeparate_no_error(GLenum face, GLenum func, GLint ref,
                                   GLuint mask)
{
   GET_CURRENT_CONTEXT(ctx);
   stencil_func_separate(ctx, face, func, ref, mask);
}


void GLAPIENTRY
_mesa_StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glStencilFuncSeparate()\n");

   if (face != GL_FRONT && face != GL_BACK && face != GL_FRONT_AND_BACK) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilFuncSeparate(face)");
      return;
   }

   if (!validate_stencil_func(ctx, func)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilFuncSeparate(func)");
      return;
   }

   stencil_func_separate(ctx, face, func, ref, mask);
}


static void
stencil_mask_separate(struct gl_context *ctx, GLenum face, GLuint mask)
{
   FLUSH_VERTICES(ctx, 0, GL_STENCIL_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_DSA;

   if (face != GL_BACK) {
      ctx->Stencil.WriteMask[0] = mask;
   }

   if (face != GL_FRONT) {
      ctx->Stencil.WriteMask[1] = mask;
   }
}


/* OpenGL 2.0 */
void GLAPIENTRY
_mesa_StencilMaskSeparate_no_error(GLenum face, GLuint mask)
{
   GET_CURRENT_CONTEXT(ctx);
   stencil_mask_separate(ctx, face, mask);
}


void GLAPIENTRY
_mesa_StencilMaskSeparate(GLenum face, GLuint mask)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glStencilMaskSeparate()\n");

   if (face != GL_FRONT && face != GL_BACK && face != GL_FRONT_AND_BACK) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glStencilaMaskSeparate(face)");
      return;
   }

   stencil_mask_separate(ctx, face, mask);
}


/**
 * Initialize the context stipple state.
 *
 * \param ctx GL context.
 *
 * Initializes __struct gl_contextRec::Stencil attribute group.
 */
void
_mesa_init_stencil(struct gl_context *ctx)
{
   ctx->Stencil.Enabled = GL_FALSE;
   ctx->Stencil.TestTwoSide = GL_FALSE;
   ctx->Stencil.ActiveFace = 0;  /* 0 = GL_FRONT, 2 = GL_BACK */
   ctx->Stencil.Function[0] = GL_ALWAYS;
   ctx->Stencil.Function[1] = GL_ALWAYS;
   ctx->Stencil.Function[2] = GL_ALWAYS;
   ctx->Stencil.FailFunc[0] = GL_KEEP;
   ctx->Stencil.FailFunc[1] = GL_KEEP;
   ctx->Stencil.FailFunc[2] = GL_KEEP;
   ctx->Stencil.ZPassFunc[0] = GL_KEEP;
   ctx->Stencil.ZPassFunc[1] = GL_KEEP;
   ctx->Stencil.ZPassFunc[2] = GL_KEEP;
   ctx->Stencil.ZFailFunc[0] = GL_KEEP;
   ctx->Stencil.ZFailFunc[1] = GL_KEEP;
   ctx->Stencil.ZFailFunc[2] = GL_KEEP;
   ctx->Stencil.Ref[0] = 0;
   ctx->Stencil.Ref[1] = 0;
   ctx->Stencil.Ref[2] = 0;

   /* 4.1.4 Stencil Test section of the GL-ES 3.0 specification says:
    *
    *     "In the initial state, [...] the front and back stencil mask are both
    *     set to the value 2^s − 1, where s is greater than or equal to the
    *     number of bits in the deepest stencil buffer* supported by the GL
    *     implementation."
    *
    * Since the maximum supported precision for stencil buffers is 8 bits,
    * mask values should be initialized to 2^8 - 1 = 0xFF.
    */
   ctx->Stencil.ValueMask[0] = 0xFF;
   ctx->Stencil.ValueMask[1] = 0xFF;
   ctx->Stencil.ValueMask[2] = 0xFF;
   ctx->Stencil.WriteMask[0] = 0xFF;
   ctx->Stencil.WriteMask[1] = 0xFF;
   ctx->Stencil.WriteMask[2] = 0xFF;

   ctx->Stencil.Clear = 0;
   ctx->Stencil._BackFace = 1;
}
