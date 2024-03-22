/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
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
 * \file viewport.c
 * glViewport and glDepthRange functions.
 */


#include "context.h"
#include "enums.h"
#include "macros.h"
#include "mtypes.h"
#include "viewport.h"
#include "api_exec_decl.h"

#include "state_tracker/st_manager.h"
#include "state_tracker/st_context.h"

static void
clamp_viewport(struct gl_context *ctx, GLfloat *x, GLfloat *y,
               GLfloat *width, GLfloat *height)
{
   /* clamp width and height to the implementation dependent range */
   *width  = MIN2(*width, (GLfloat) ctx->Const.MaxViewportWidth);
   *height = MIN2(*height, (GLfloat) ctx->Const.MaxViewportHeight);

   /* The GL_ARB_viewport_array spec says:
    *
    *     "The location of the viewport's bottom-left corner, given by (x,y),
    *     are clamped to be within the implementation-dependent viewport
    *     bounds range.  The viewport bounds range [min, max] tuple may be
    *     determined by calling GetFloatv with the symbolic constant
    *     VIEWPORT_BOUNDS_RANGE (see section 6.1)."
    */
   if (_mesa_has_ARB_viewport_array(ctx) ||
       _mesa_has_OES_viewport_array(ctx)) {
      *x = CLAMP(*x,
                 ctx->Const.ViewportBounds.Min, ctx->Const.ViewportBounds.Max);
      *y = CLAMP(*y,
                 ctx->Const.ViewportBounds.Min, ctx->Const.ViewportBounds.Max);
   }
}

static void
set_viewport_no_notify(struct gl_context *ctx, unsigned idx,
                       GLfloat x, GLfloat y,
                       GLfloat width, GLfloat height)
{
   if (ctx->ViewportArray[idx].X == x &&
       ctx->ViewportArray[idx].Width == width &&
       ctx->ViewportArray[idx].Y == y &&
       ctx->ViewportArray[idx].Height == height)
      return;

   FLUSH_VERTICES(ctx, 0, GL_VIEWPORT_BIT);
   ctx->NewDriverState |= ST_NEW_VIEWPORT;

   ctx->ViewportArray[idx].X = x;
   ctx->ViewportArray[idx].Width = width;
   ctx->ViewportArray[idx].Y = y;
   ctx->ViewportArray[idx].Height = height;
}

struct gl_viewport_inputs {
   GLfloat X, Y;                /**< position */
   GLfloat Width, Height;       /**< size */
};

struct gl_depthrange_inputs {
   GLdouble Near, Far;          /**< Depth buffer range */
};

static void
viewport(struct gl_context *ctx, GLint x, GLint y, GLsizei width,
         GLsizei height)
{
   struct gl_viewport_inputs input = { x, y, width, height };

   /* Clamp the viewport to the implementation dependent values. */
   clamp_viewport(ctx, &input.X, &input.Y, &input.Width, &input.Height);

   /* The GL_ARB_viewport_array spec says:
    *
    *     "Viewport sets the parameters for all viewports to the same values
    *     and is equivalent (assuming no errors are generated) to:
    *
    *     for (uint i = 0; i < MAX_VIEWPORTS; i++)
    *         ViewportIndexedf(i, 1, (float)x, (float)y, (float)w, (float)h);"
    *
    * Set all of the viewports supported by the implementation, but only
    * signal the driver once at the end.
    */
   for (unsigned i = 0; i < ctx->Const.MaxViewports; i++)
      set_viewport_no_notify(ctx, i, input.X, input.Y, input.Width, input.Height);

   if (ctx->invalidate_on_gl_viewport)
      st_manager_invalidate_drawables(ctx);
}

/**
 * Set the viewport.
 * \sa Called via glViewport() or display list execution.
 *
 * Flushes the vertices and calls _mesa_set_viewport() with the given
 * parameters.
 */
void GLAPIENTRY
_mesa_Viewport_no_error(GLint x, GLint y, GLsizei width, GLsizei height)
{
   GET_CURRENT_CONTEXT(ctx);
   viewport(ctx, x, y, width, height);
}

void GLAPIENTRY
_mesa_Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glViewport %d %d %d %d\n", x, y, width, height);

   if (width < 0 || height < 0) {
      _mesa_error(ctx,  GL_INVALID_VALUE,
                   "glViewport(%d, %d, %d, %d)", x, y, width, height);
      return;
   }

   viewport(ctx, x, y, width, height);
}


/**
 * Set new viewport parameters and update derived state.
 * Usually called from _mesa_Viewport().
 * 
 * \param ctx GL context.
 * \param idx    Index of the viewport to be updated.
 * \param x, y coordinates of the lower left corner of the viewport rectangle.
 * \param width width of the viewport rectangle.
 * \param height height of the viewport rectangle.
 */
void
_mesa_set_viewport(struct gl_context *ctx, unsigned idx, GLfloat x, GLfloat y,
                    GLfloat width, GLfloat height)
{
   clamp_viewport(ctx, &x, &y, &width, &height);
   set_viewport_no_notify(ctx, idx, x, y, width, height);

   if (ctx->invalidate_on_gl_viewport)
      st_manager_invalidate_drawables(ctx);
}

static void
viewport_array(struct gl_context *ctx, GLuint first, GLsizei count,
               struct gl_viewport_inputs *inputs)
{
   for (GLsizei i = 0; i < count; i++) {
      clamp_viewport(ctx, &inputs[i].X, &inputs[i].Y,
                     &inputs[i].Width, &inputs[i].Height);

      set_viewport_no_notify(ctx, i + first, inputs[i].X, inputs[i].Y,
                             inputs[i].Width, inputs[i].Height);
   }

   if (ctx->invalidate_on_gl_viewport)
      st_manager_invalidate_drawables(ctx);
}

void GLAPIENTRY
_mesa_ViewportArrayv_no_error(GLuint first, GLsizei count, const GLfloat *v)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_viewport_inputs *p = (struct gl_viewport_inputs *)v;
   viewport_array(ctx, first, count, p);
}

void GLAPIENTRY
_mesa_ViewportArrayv(GLuint first, GLsizei count, const GLfloat *v)
{
   int i;
   struct gl_viewport_inputs *p = (struct gl_viewport_inputs *)v;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glViewportArrayv %d %d\n", first, count);

   if ((first + count) > ctx->Const.MaxViewports) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glViewportArrayv: first (%d) + count (%d) > MaxViewports "
                  "(%d)",
                  first, count, ctx->Const.MaxViewports);
      return;
   }

   /* Verify width & height */
   for (i = 0; i < count; i++) {
      if (p[i].Width < 0 || p[i].Height < 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glViewportArrayv: index (%d) width or height < 0 "
                     "(%f, %f)",
                     i + first, p[i].Width, p[i].Height);
         return;
      }
   }

   viewport_array(ctx, first, count, p);
}

static void
viewport_indexed_err(struct gl_context *ctx, GLuint index, GLfloat x, GLfloat y,
                     GLfloat w, GLfloat h, const char *function)
{
   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "%s(%d, %f, %f, %f, %f)\n",
                  function, index, x, y, w, h);

   if (index >= ctx->Const.MaxViewports) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s: index (%d) >= MaxViewports (%d)",
                  function, index, ctx->Const.MaxViewports);
      return;
   }

   /* Verify width & height */
   if (w < 0 || h < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s: index (%d) width or height < 0 (%f, %f)",
                  function, index, w, h);
      return;
   }

   _mesa_set_viewport(ctx, index, x, y, w, h);
}

void GLAPIENTRY
_mesa_ViewportIndexedf_no_error(GLuint index, GLfloat x, GLfloat y,
                                GLfloat w, GLfloat h)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_set_viewport(ctx, index, x, y, w, h);
}

void GLAPIENTRY
_mesa_ViewportIndexedf(GLuint index, GLfloat x, GLfloat y,
                       GLfloat w, GLfloat h)
{
   GET_CURRENT_CONTEXT(ctx);
   viewport_indexed_err(ctx, index, x, y, w, h, "glViewportIndexedf");
}

void GLAPIENTRY
_mesa_ViewportIndexedfv_no_error(GLuint index, const GLfloat *v)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_set_viewport(ctx, index, v[0], v[1], v[2], v[3]);
}

void GLAPIENTRY
_mesa_ViewportIndexedfv(GLuint index, const GLfloat *v)
{
   GET_CURRENT_CONTEXT(ctx);
   viewport_indexed_err(ctx, index, v[0], v[1], v[2], v[3],
                        "glViewportIndexedfv");
}

static void
set_depth_range_no_notify(struct gl_context *ctx, unsigned idx,
                          GLclampd nearval, GLclampd farval)
{
   if (ctx->ViewportArray[idx].Near == nearval &&
       ctx->ViewportArray[idx].Far == farval)
      return;

   /* The depth range is needed by program state constants. */
   FLUSH_VERTICES(ctx, _NEW_VIEWPORT, GL_VIEWPORT_BIT);
   ctx->NewDriverState |= ST_NEW_VIEWPORT;

   ctx->ViewportArray[idx].Near = SATURATE(nearval);
   ctx->ViewportArray[idx].Far = SATURATE(farval);
}

void
_mesa_set_depth_range(struct gl_context *ctx, unsigned idx,
                      GLclampd nearval, GLclampd farval)
{
   set_depth_range_no_notify(ctx, idx, nearval, farval);
}

/**
 * Called by glDepthRange
 *
 * \param nearval  specifies the Z buffer value which should correspond to
 *                 the near clip plane
 * \param farval  specifies the Z buffer value which should correspond to
 *                the far clip plane
 */
void GLAPIENTRY
_mesa_DepthRange(GLclampd nearval, GLclampd farval)
{
   unsigned i;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE&VERBOSE_API)
      _mesa_debug(ctx, "glDepthRange %f %f\n", nearval, farval);

   /* The GL_ARB_viewport_array spec says:
    *
    *     "DepthRange sets the depth range for all viewports to the same
    *     values and is equivalent (assuming no errors are generated) to:
    *
    *     for (uint i = 0; i < MAX_VIEWPORTS; i++)
    *         DepthRangeIndexed(i, n, f);"
    *
    * Set the depth range for all of the viewports supported by the
    * implementation, but only signal the driver once at the end.
    */
   for (i = 0; i < ctx->Const.MaxViewports; i++)
      set_depth_range_no_notify(ctx, i, nearval, farval);
}

void GLAPIENTRY
_mesa_DepthRangef(GLclampf nearval, GLclampf farval)
{
   _mesa_DepthRange(nearval, farval);
}

/**
 * Update a range DepthRange values
 *
 * \param first   starting array index
 * \param count   count of DepthRange items to update
 * \param v       pointer to memory containing
 *                GLclampd near and far clip-plane values
 */
static ALWAYS_INLINE void
depth_range_arrayv(struct gl_context *ctx, GLuint first, GLsizei count,
                   const struct gl_depthrange_inputs *const inputs)
{
   for (GLsizei i = 0; i < count; i++)
      set_depth_range_no_notify(ctx, i + first, inputs[i].Near, inputs[i].Far);
}

void GLAPIENTRY
_mesa_DepthRangeArrayv_no_error(GLuint first, GLsizei count, const GLclampd *v)
{
   GET_CURRENT_CONTEXT(ctx);

   const struct gl_depthrange_inputs *const p =
      (struct gl_depthrange_inputs *)v;
   depth_range_arrayv(ctx, first, count, p);
}

void GLAPIENTRY
_mesa_DepthRangeArrayv(GLuint first, GLsizei count, const GLclampd *v)
{
   const struct gl_depthrange_inputs *const p =
      (struct gl_depthrange_inputs *) v;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glDepthRangeArrayv %d %d\n", first, count);

   if ((first + count) > ctx->Const.MaxViewports) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glDepthRangev: first (%d) + count (%d) >= MaxViewports (%d)",
                  first, count, ctx->Const.MaxViewports);
      return;
   }

   depth_range_arrayv(ctx, first, count, p);
}

void GLAPIENTRY
_mesa_DepthRangeArrayfvOES(GLuint first, GLsizei count, const GLfloat *v)
{
   int i;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glDepthRangeArrayfv %d %d\n", first, count);

   if ((first + count) > ctx->Const.MaxViewports) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glDepthRangeArrayfv: first (%d) + count (%d) >= MaxViewports (%d)",
                  first, count, ctx->Const.MaxViewports);
      return;
   }

   for (i = 0; i < count; i++)
      set_depth_range_no_notify(ctx, i + first, v[i * 2], v[i * 2 + 1]);
}

/**
 * Update a single DepthRange
 *
 * \param index    array index to update
 * \param nearval  specifies the Z buffer value which should correspond to
 *                 the near clip plane
 * \param farval   specifies the Z buffer value which should correspond to
 *                 the far clip plane
 */
void GLAPIENTRY
_mesa_DepthRangeIndexed_no_error(GLuint index, GLclampd nearval,
                                 GLclampd farval)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_set_depth_range(ctx, index, nearval, farval);
}


void GLAPIENTRY
_mesa_DepthRangeIndexed(GLuint index, GLclampd nearval, GLclampd farval)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glDepthRangeIndexed(%d, %f, %f)\n",
                  index, nearval, farval);

   if (index >= ctx->Const.MaxViewports) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glDepthRangeIndexed: index (%d) >= MaxViewports (%d)",
                  index, ctx->Const.MaxViewports);
      return;
   }

   _mesa_set_depth_range(ctx, index, nearval, farval);
}

void GLAPIENTRY
_mesa_DepthRangeIndexedfOES(GLuint index, GLfloat nearval, GLfloat farval)
{
   _mesa_DepthRangeIndexed(index, nearval, farval);
}

/** 
 * Initialize the context viewport attribute group.
 * \param ctx  the GL context.
 */
void _mesa_init_viewport(struct gl_context *ctx)
{
   unsigned i;

   ctx->Transform.ClipOrigin = GL_LOWER_LEFT;
   ctx->Transform.ClipDepthMode = GL_NEGATIVE_ONE_TO_ONE;

   /* Note: ctx->Const.MaxViewports may not have been set by the driver yet,
    * so just initialize all of them.
    */
   for (i = 0; i < MAX_VIEWPORTS; i++) {
      /* Viewport group */
      ctx->ViewportArray[i].X = 0;
      ctx->ViewportArray[i].Y = 0;
      ctx->ViewportArray[i].Width = 0;
      ctx->ViewportArray[i].Height = 0;
      ctx->ViewportArray[i].Near = 0.0;
      ctx->ViewportArray[i].Far = 1.0;
      ctx->ViewportArray[i].SwizzleX = GL_VIEWPORT_SWIZZLE_POSITIVE_X_NV;
      ctx->ViewportArray[i].SwizzleY = GL_VIEWPORT_SWIZZLE_POSITIVE_Y_NV;
      ctx->ViewportArray[i].SwizzleZ = GL_VIEWPORT_SWIZZLE_POSITIVE_Z_NV;
      ctx->ViewportArray[i].SwizzleW = GL_VIEWPORT_SWIZZLE_POSITIVE_W_NV;
   }

   ctx->SubpixelPrecisionBias[0] = 0;
   ctx->SubpixelPrecisionBias[1] = 0;
}


static ALWAYS_INLINE void
clip_control(struct gl_context *ctx, GLenum origin, GLenum depth, bool no_error)
{
   if (ctx->Transform.ClipOrigin == origin &&
       ctx->Transform.ClipDepthMode == depth)
      return;

   if (!no_error &&
       origin != GL_LOWER_LEFT && origin != GL_UPPER_LEFT) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glClipControl");
      return;
   }

   if (!no_error &&
       depth != GL_NEGATIVE_ONE_TO_ONE && depth != GL_ZERO_TO_ONE) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glClipControl");
      return;
   }

   /* Affects transform state and the viewport transform */
   FLUSH_VERTICES(ctx, 0, GL_TRANSFORM_BIT);
   ctx->NewDriverState |= ST_NEW_VIEWPORT | ST_NEW_RASTERIZER;

   if (ctx->Transform.ClipOrigin != origin) {
      ctx->Transform.ClipOrigin = origin;

      /* Affects the winding order of the front face. */
      ctx->NewDriverState |= ST_NEW_RASTERIZER;
   }

   if (ctx->Transform.ClipDepthMode != depth) {
      ctx->Transform.ClipDepthMode = depth;
   }
}


void GLAPIENTRY
_mesa_ClipControl_no_error(GLenum origin, GLenum depth)
{
   GET_CURRENT_CONTEXT(ctx);
   clip_control(ctx, origin, depth, true);
}


void GLAPIENTRY
_mesa_ClipControl(GLenum origin, GLenum depth)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glClipControl(%s, %s)\n",
	          _mesa_enum_to_string(origin),
                  _mesa_enum_to_string(depth));

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (!ctx->Extensions.ARB_clip_control) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glClipControl");
      return;
   }

   clip_control(ctx, origin, depth, false);
}

/**
 * Computes the scaling and the translation part of the
 * viewport transform matrix of the \param i-th viewport
 * and writes that into \param scale and \param translate.
 */
void
_mesa_get_viewport_xform(struct gl_context *ctx, unsigned i,
                         float scale[3], float translate[3])
{
   float x = ctx->ViewportArray[i].X;
   float y = ctx->ViewportArray[i].Y;
   float half_width = 0.5f * ctx->ViewportArray[i].Width;
   float half_height = 0.5f * ctx->ViewportArray[i].Height;
   double n = ctx->ViewportArray[i].Near;
   double f = ctx->ViewportArray[i].Far;

   scale[0] = half_width;
   translate[0] = half_width + x;
   if (ctx->Transform.ClipOrigin == GL_UPPER_LEFT) {
      scale[1] = -half_height;
   } else {
      scale[1] = half_height;
   }
   translate[1] = half_height + y;

   if (ctx->Transform.ClipDepthMode == GL_NEGATIVE_ONE_TO_ONE) {
      scale[2] = 0.5 * (f - n);
      translate[2] = 0.5 * (n + f);
   } else {
      scale[2] = f - n;
      translate[2] = n;
   }
}


static void
subpixel_precision_bias(struct gl_context *ctx, GLuint xbits, GLuint ybits)
{
   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glSubpixelPrecisionBiasNV(%u, %u)\n", xbits, ybits);

   FLUSH_VERTICES(ctx, 0, GL_VIEWPORT_BIT);

   ctx->SubpixelPrecisionBias[0] = xbits;
   ctx->SubpixelPrecisionBias[1] = ybits;

   ctx->NewDriverState |= ST_NEW_RASTERIZER;
}

void GLAPIENTRY
_mesa_SubpixelPrecisionBiasNV_no_error(GLuint xbits, GLuint ybits)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glSubpixelPrecisionBiasNV(%u, %u)\n", xbits, ybits);

   subpixel_precision_bias(ctx, xbits, ybits);
}

void GLAPIENTRY
_mesa_SubpixelPrecisionBiasNV(GLuint xbits, GLuint ybits)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glSubpixelPrecisionBiasNV(%u, %u)\n", xbits, ybits);

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (!ctx->Extensions.NV_conservative_raster) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glSubpixelPrecisionBiasNV not supported");
      return;
   }

   if (xbits > ctx->Const.MaxSubpixelPrecisionBiasBits) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glSubpixelPrecisionBiasNV");
      return;
   }

   if (ybits > ctx->Const.MaxSubpixelPrecisionBiasBits) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glSubpixelPrecisionBiasNV");
      return;
   }

   subpixel_precision_bias(ctx, xbits, ybits);
}

static void
set_viewport_swizzle(struct gl_context *ctx, GLuint index,
                     GLenum swizzlex, GLenum swizzley,
                     GLenum swizzlez, GLenum swizzlew)
{
   struct gl_viewport_attrib *viewport = &ctx->ViewportArray[index];
   if (viewport->SwizzleX == swizzlex &&
       viewport->SwizzleY == swizzley &&
       viewport->SwizzleZ == swizzlez &&
       viewport->SwizzleW == swizzlew)
      return;

   FLUSH_VERTICES(ctx, _NEW_VIEWPORT, GL_VIEWPORT_BIT);
   ctx->NewDriverState |= ST_NEW_VIEWPORT;

   viewport->SwizzleX = swizzlex;
   viewport->SwizzleY = swizzley;
   viewport->SwizzleZ = swizzlez;
   viewport->SwizzleW = swizzlew;
}

void GLAPIENTRY
_mesa_ViewportSwizzleNV_no_error(GLuint index,
                                 GLenum swizzlex, GLenum swizzley,
                                 GLenum swizzlez, GLenum swizzlew)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glViewportSwizzleNV(%x, %x, %x, %x)\n",
                  swizzlex, swizzley, swizzlez, swizzlew);

   set_viewport_swizzle(ctx, index, swizzlex, swizzley, swizzlez, swizzlew);
}

static bool
verify_viewport_swizzle(GLenum swizzle)
{
   return swizzle >= GL_VIEWPORT_SWIZZLE_POSITIVE_X_NV &&
      swizzle <= GL_VIEWPORT_SWIZZLE_NEGATIVE_W_NV;
}

void GLAPIENTRY
_mesa_ViewportSwizzleNV(GLuint index,
                        GLenum swizzlex, GLenum swizzley,
                        GLenum swizzlez, GLenum swizzlew)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glViewportSwizzleNV(%x, %x, %x, %x)\n",
                  swizzlex, swizzley, swizzlez, swizzlew);

   if (!ctx->Extensions.NV_viewport_swizzle) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glViewportSwizzleNV not supported");
      return;
   }

   if (index >= ctx->Const.MaxViewports) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glViewportSwizzleNV: index (%d) >= MaxViewports (%d)",
                  index, ctx->Const.MaxViewports);
      return;
   }

   if (!verify_viewport_swizzle(swizzlex)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glViewportSwizzleNV(swizzlex=%x)", swizzlex);
      return;
   }

   if (!verify_viewport_swizzle(swizzley)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glViewportSwizzleNV(swizzley=%x)", swizzley);
      return;
   }

   if (!verify_viewport_swizzle(swizzlez)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glViewportSwizzleNV(swizzlez=%x)", swizzlez);
      return;
   }

   if (!verify_viewport_swizzle(swizzlew)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glViewportSwizzleNV(swizzlew=%x)", swizzlew);
      return;
   }

   set_viewport_swizzle(ctx, index, swizzlex, swizzley, swizzlez, swizzlew);
}
