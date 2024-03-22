/**
 * \file polygon.c
 * Polygon operations.
 */

/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
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


#include "util/glheader.h"

#include "context.h"
#include "draw_validate.h"
#include "image.h"
#include "enums.h"
#include "pack.h"
#include "pbo.h"
#include "polygon.h"
#include "mtypes.h"
#include "api_exec_decl.h"
#include "varray.h"

#include "state_tracker/st_context.h"

/**
 * Specify whether to cull front- or back-facing facets.
 *
 * \param mode culling mode.
 *
 * \sa glCullFace().
 *
 * Verifies the parameter and updates gl_polygon_attrib::CullFaceMode. On
 * change, flushes the vertices and notifies the driver via
 * the dd_function_table::CullFace callback.
 */
static ALWAYS_INLINE void
cull_face(struct gl_context *ctx, GLenum mode, bool no_error)
{
   if (ctx->Polygon.CullFaceMode == mode)
      return;

   if (!no_error &&
       mode != GL_FRONT && mode != GL_BACK && mode != GL_FRONT_AND_BACK) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glCullFace");
      return;
   }

   FLUSH_VERTICES(ctx, 0,
                  GL_POLYGON_BIT);
   ctx->NewDriverState |= ST_NEW_RASTERIZER;
   ctx->Polygon.CullFaceMode = mode;
}


void GLAPIENTRY
_mesa_CullFace_no_error(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   cull_face(ctx, mode, true);
}


void GLAPIENTRY
_mesa_CullFace(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glCullFace %s\n", _mesa_enum_to_string(mode));

   cull_face(ctx, mode, false);
}


/**
 * Define front- and back-facing
 *
 * \param mode orientation of front-facing polygons.
 *
 * \sa glFrontFace().
 *
 * Verifies the parameter and updates gl_polygon_attrib::FrontFace. On change
 * flushes the vertices and notifies the driver via
 * the dd_function_table::FrontFace callback.
 */
static ALWAYS_INLINE void
front_face(struct gl_context *ctx, GLenum mode, bool no_error)
{
   if (ctx->Polygon.FrontFace == mode)
      return;

   if (!no_error && mode != GL_CW && mode != GL_CCW) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glFrontFace");
      return;
   }

   FLUSH_VERTICES(ctx, 0,
                  GL_POLYGON_BIT);
   ctx->NewDriverState |= ST_NEW_RASTERIZER;
   ctx->Polygon.FrontFace = mode;
}


void GLAPIENTRY
_mesa_FrontFace_no_error(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   front_face(ctx, mode, true);
}


void GLAPIENTRY
_mesa_FrontFace(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glFrontFace %s\n", _mesa_enum_to_string(mode));

   front_face(ctx, mode, false);
}


/**
 * Set the polygon rasterization mode.
 *
 * \param face the polygons which \p mode applies to.
 * \param mode how polygons should be rasterized.
 *
 * \sa glPolygonMode().
 *
 * Verifies the parameters and updates gl_polygon_attrib::FrontMode and
 * gl_polygon_attrib::BackMode. On change flushes the vertices and notifies the
 * driver via the dd_function_table::PolygonMode callback.
 */
static ALWAYS_INLINE void
polygon_mode(struct gl_context *ctx, GLenum face, GLenum mode, bool no_error)
{
   bool old_mode_has_fill_rectangle =
      ctx->Polygon.FrontMode == GL_FILL_RECTANGLE_NV ||
      ctx->Polygon.BackMode == GL_FILL_RECTANGLE_NV;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glPolygonMode %s %s\n",
                  _mesa_enum_to_string(face),
                  _mesa_enum_to_string(mode));

   if (!no_error) {
      switch (mode) {
      case GL_POINT:
      case GL_LINE:
      case GL_FILL:
         break;
      case GL_FILL_RECTANGLE_NV:
         if (ctx->Extensions.NV_fill_rectangle)
            break;
         FALLTHROUGH;
      default:
         _mesa_error(ctx, GL_INVALID_ENUM, "glPolygonMode(mode)");
         return;
      }
   }

   switch (face) {
   case GL_FRONT:
      if (!no_error && _mesa_is_desktop_gl_core(ctx)) {
         _mesa_error( ctx, GL_INVALID_ENUM, "glPolygonMode(face)" );
         return;
      }
      if (ctx->Polygon.FrontMode == mode)
         return;
      FLUSH_VERTICES(ctx, 0,
                     GL_POLYGON_BIT);
      ctx->NewDriverState |= ST_NEW_RASTERIZER;
      ctx->Polygon.FrontMode = mode;
      _mesa_update_edgeflag_state_vao(ctx);
      break;
   case GL_FRONT_AND_BACK:
      if (ctx->Polygon.FrontMode == mode && ctx->Polygon.BackMode == mode)
         return;
      FLUSH_VERTICES(ctx, 0,
                     GL_POLYGON_BIT);
      ctx->NewDriverState |= ST_NEW_RASTERIZER;
      ctx->Polygon.FrontMode = mode;
      ctx->Polygon.BackMode = mode;
      _mesa_update_edgeflag_state_vao(ctx);
      break;
   case GL_BACK:
      if (!no_error && _mesa_is_desktop_gl_core(ctx)) {
         _mesa_error( ctx, GL_INVALID_ENUM, "glPolygonMode(face)" );
         return;
      }
      if (ctx->Polygon.BackMode == mode)
         return;
      FLUSH_VERTICES(ctx, 0,
                     GL_POLYGON_BIT);
      ctx->NewDriverState |= ST_NEW_RASTERIZER;
      ctx->Polygon.BackMode = mode;
      _mesa_update_edgeflag_state_vao(ctx);
      break;
   default:
      if (!no_error)
         _mesa_error( ctx, GL_INVALID_ENUM, "glPolygonMode(face)" );
      return;
   }

   if (ctx->Extensions.INTEL_conservative_rasterization ||
       (mode == GL_FILL_RECTANGLE_NV || old_mode_has_fill_rectangle))
      _mesa_update_valid_to_render_state(ctx);
}


void GLAPIENTRY
_mesa_PolygonMode_no_error(GLenum face, GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   polygon_mode(ctx, face, mode, true);
}


void GLAPIENTRY
_mesa_PolygonMode(GLenum face, GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   polygon_mode(ctx, face, mode, false);
}


/**
 * Called by glPolygonStipple.
 */
void GLAPIENTRY
_mesa_PolygonStipple(const GLubyte *pattern)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glPolygonStipple\n");

   FLUSH_VERTICES(ctx, 0, GL_POLYGON_STIPPLE_BIT);
   ctx->NewDriverState |= ST_NEW_POLY_STIPPLE;

   pattern = _mesa_map_validate_pbo_source(ctx, 2,
                                           &ctx->Unpack, 32, 32, 1,
                                           GL_COLOR_INDEX, GL_BITMAP,
                                           INT_MAX, pattern,
                                           "glPolygonStipple");
   if (!pattern)
      return;

   _mesa_unpack_polygon_stipple(pattern, ctx->PolygonStipple, &ctx->Unpack);

   _mesa_unmap_pbo_source(ctx, &ctx->Unpack);
}


/**
 * Called by glPolygonStipple.
 */
void GLAPIENTRY
_mesa_GetnPolygonStippleARB( GLsizei bufSize, GLubyte *dest )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE&VERBOSE_API)
      _mesa_debug(ctx, "glGetPolygonStipple\n");

   if (ctx->Pack.BufferObj)
      ctx->Pack.BufferObj->UsageHistory |= USAGE_PIXEL_PACK_BUFFER;

   dest = _mesa_map_validate_pbo_dest(ctx, 2,
                                      &ctx->Pack, 32, 32, 1,
                                      GL_COLOR_INDEX, GL_BITMAP,
                                      bufSize, dest, "glGetPolygonStipple");
   if (!dest)
      return;

   _mesa_pack_polygon_stipple(ctx->PolygonStipple, dest, &ctx->Pack);

   _mesa_unmap_pbo_dest(ctx, &ctx->Pack);
}


void GLAPIENTRY
_mesa_GetPolygonStipple( GLubyte *dest )
{
   _mesa_GetnPolygonStippleARB(INT_MAX, dest);
}

void
_mesa_polygon_offset_clamp(struct gl_context *ctx,
                           GLfloat factor, GLfloat units, GLfloat clamp)
{
   if (ctx->Polygon.OffsetFactor == factor &&
       ctx->Polygon.OffsetUnits == units &&
       ctx->Polygon.OffsetClamp == clamp)
      return;

   FLUSH_VERTICES(ctx, 0,
                  GL_POLYGON_BIT);
   ctx->NewDriverState |= ST_NEW_RASTERIZER;
   ctx->Polygon.OffsetFactor = factor;
   ctx->Polygon.OffsetUnits = units;
   ctx->Polygon.OffsetClamp = clamp;
}

void GLAPIENTRY
_mesa_PolygonOffset( GLfloat factor, GLfloat units )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE&VERBOSE_API)
      _mesa_debug(ctx, "glPolygonOffset %f %f\n", factor, units);

   _mesa_polygon_offset_clamp(ctx, factor, units, 0.0);
}

void GLAPIENTRY
_mesa_PolygonOffsetClampEXT( GLfloat factor, GLfloat units, GLfloat clamp )
{
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_polygon_offset_clamp) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "unsupported function (%s) called", "glPolygonOffsetClamp");
      return;
   }

   if (MESA_VERBOSE&VERBOSE_API)
      _mesa_debug(ctx, "glPolygonOffsetClamp %f %f %f\n", factor, units, clamp);

   _mesa_polygon_offset_clamp(ctx, factor, units, clamp);
}

/**********************************************************************/
/** \name Initialization */
/*@{*/

/**
 * Initialize the context polygon state.
 *
 * \param ctx GL context.
 *
 * Initializes __struct gl_contextRec::Polygon and __struct gl_contextRec::PolygonStipple
 * attribute groups.
 */
void _mesa_init_polygon( struct gl_context * ctx )
{
   /* Polygon group */
   ctx->Polygon.CullFlag = GL_FALSE;
   ctx->Polygon.CullFaceMode = GL_BACK;
   ctx->Polygon.FrontFace = GL_CCW;
   ctx->Polygon.FrontMode = GL_FILL;
   ctx->Polygon.BackMode = GL_FILL;
   ctx->Polygon.SmoothFlag = GL_FALSE;
   ctx->Polygon.StippleFlag = GL_FALSE;
   ctx->Polygon.OffsetFactor = 0.0F;
   ctx->Polygon.OffsetUnits = 0.0F;
   ctx->Polygon.OffsetClamp = 0.0F;
   ctx->Polygon.OffsetPoint = GL_FALSE;
   ctx->Polygon.OffsetLine = GL_FALSE;
   ctx->Polygon.OffsetFill = GL_FALSE;


   /* Polygon Stipple group */
   memset( ctx->PolygonStipple, 0xff, 32*sizeof(GLuint) );
}

/*@}*/
