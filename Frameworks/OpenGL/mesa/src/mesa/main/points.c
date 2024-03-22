/**
 * \file points.c
 * Point operations.
 */

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


#include "util/glheader.h"
#include "context.h"
#include "macros.h"
#include "points.h"
#include "mtypes.h"
#include "api_exec_decl.h"


static void
update_point_size_set(struct gl_context *ctx)
{
   float size = CLAMP(ctx->Point.Size, ctx->Point.MinSize, ctx->Point.MaxSize);
   ctx->PointSizeIsSet = (size == 1.0 && ctx->Point.Size == 1.0) || ctx->Point._Attenuated;
}

/**
 * Set current point size.
 * \param size  point diameter in pixels
 * \sa glPointSize().
 */
static ALWAYS_INLINE void
point_size(struct gl_context *ctx, GLfloat size, bool no_error)
{
   if (ctx->Point.Size == size)
      return;

   if (!no_error && size <= 0.0F) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glPointSize");
      return;
   }

   FLUSH_VERTICES(ctx, _NEW_POINT, GL_POINT_BIT);
   ctx->Point.Size = size;
   update_point_size_set(ctx);
}


void GLAPIENTRY
_mesa_PointSize_no_error(GLfloat size)
{
   GET_CURRENT_CONTEXT(ctx);
   point_size(ctx, size, true);
}


void GLAPIENTRY
_mesa_PointSize( GLfloat size )
{
   GET_CURRENT_CONTEXT(ctx);
   point_size(ctx, size, false);
}


void GLAPIENTRY
_mesa_PointParameteri( GLenum pname, GLint param )
{
   GLfloat p[3];
   p[0] = (GLfloat) param;
   p[1] = p[2] = 0.0F;
   _mesa_PointParameterfv(pname, p);
}


void GLAPIENTRY
_mesa_PointParameteriv( GLenum pname, const GLint *params )
{
   GLfloat p[3];
   p[0] = (GLfloat) params[0];
   if (pname == GL_DISTANCE_ATTENUATION_EXT) {
      p[1] = (GLfloat) params[1];
      p[2] = (GLfloat) params[2];
   }
   _mesa_PointParameterfv(pname, p);
}


void GLAPIENTRY
_mesa_PointParameterf( GLenum pname, GLfloat param)
{
   GLfloat p[3];
   p[0] = param;
   p[1] = p[2] = 0.0F;
   _mesa_PointParameterfv(pname, p);
}


void GLAPIENTRY
_mesa_PointParameterfv( GLenum pname, const GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);

   switch (pname) {
      case GL_DISTANCE_ATTENUATION_EXT:
         if (TEST_EQ_3V(ctx->Point.Params, params))
            return;
         FLUSH_VERTICES(ctx, _NEW_POINT | _NEW_FF_VERT_PROGRAM |
                        _NEW_TNL_SPACES, GL_POINT_BIT);
         COPY_3V(ctx->Point.Params, params);
         ctx->Point._Attenuated = (ctx->Point.Params[0] != 1.0F ||
                                   ctx->Point.Params[1] != 0.0F ||
                                   ctx->Point.Params[2] != 0.0F);
         update_point_size_set(ctx);
         break;
      case GL_POINT_SIZE_MIN_EXT:
         if (params[0] < 0.0F) {
            _mesa_error( ctx, GL_INVALID_VALUE,
                         "glPointParameterf[v]{EXT,ARB}(param)" );
            return;
         }
         if (ctx->Point.MinSize == params[0])
            return;
         FLUSH_VERTICES(ctx, _NEW_POINT, GL_POINT_BIT);
         ctx->Point.MinSize = params[0];
         break;
      case GL_POINT_SIZE_MAX_EXT:
         if (params[0] < 0.0F) {
            _mesa_error( ctx, GL_INVALID_VALUE,
                         "glPointParameterf[v]{EXT,ARB}(param)" );
            return;
         }
         if (ctx->Point.MaxSize == params[0])
            return;
         FLUSH_VERTICES(ctx, _NEW_POINT, GL_POINT_BIT);
         ctx->Point.MaxSize = params[0];
         break;
      case GL_POINT_FADE_THRESHOLD_SIZE_EXT:
         if (params[0] < 0.0F) {
            _mesa_error( ctx, GL_INVALID_VALUE,
                         "glPointParameterf[v]{EXT,ARB}(param)" );
            return;
         }
         if (ctx->Point.Threshold == params[0])
            return;
         FLUSH_VERTICES(ctx, _NEW_POINT, GL_POINT_BIT);
         ctx->Point.Threshold = params[0];
         break;
      case GL_POINT_SPRITE_COORD_ORIGIN:
	 /* GL_POINT_SPRITE_COORD_ORIGIN was added to point sprites when the
	  * extension was merged into OpenGL 2.0.
	  */
         if ((_mesa_is_desktop_gl_compat(ctx) && ctx->Version >= 20)
             || _mesa_is_desktop_gl_core(ctx)) {
            GLenum value = (GLenum) params[0];
            if (value != GL_LOWER_LEFT && value != GL_UPPER_LEFT) {
               _mesa_error(ctx, GL_INVALID_VALUE,
                           "glPointParameterf[v]{EXT,ARB}(param)");
               return;
            }
            if (ctx->Point.SpriteOrigin == value)
               return;
            FLUSH_VERTICES(ctx, _NEW_POINT, GL_POINT_BIT);
            ctx->Point.SpriteOrigin = value;
         }
         else {
            _mesa_error(ctx, GL_INVALID_ENUM,
                        "glPointParameterf[v]{EXT,ARB}(pname)");
            return;
         }
         break;
      default:
         _mesa_error( ctx, GL_INVALID_ENUM,
                      "glPointParameterf[v]{EXT,ARB}(pname)" );
         return;
   }
}



/**
 * Initialize the context point state.
 *
 * \param ctx GL context.
 *
 * Initializes __struct gl_contextRec::Point and point related constants in
 * __struct gl_contextRec::Const.
 */
void
_mesa_init_point(struct gl_context *ctx)
{
   ctx->Point.SmoothFlag = GL_FALSE;
   ctx->Point.Size = 1.0;
   ctx->Point.Params[0] = 1.0;
   ctx->Point.Params[1] = 0.0;
   ctx->Point.Params[2] = 0.0;
   ctx->Point._Attenuated = GL_FALSE;
   ctx->Point.MinSize = 0.0;
   ctx->Point.MaxSize
      = MAX2(ctx->Const.MaxPointSize, ctx->Const.MaxPointSizeAA);
   ctx->Point.Threshold = 1.0;

   /* Page 403 (page 423 of the PDF) of the OpenGL 3.0 spec says:
    *
    *     "Non-sprite points (section 3.4) - Enable/Disable targets
    *     POINT_SMOOTH and POINT_SPRITE, and all associated state. Point
    *     rasterization is always performed as though POINT_SPRITE were
    *     enabled."
    *
    * In a core context, the state will default to true, and the setters and
    * getters are disabled.
    */
   ctx->Point.PointSprite = (_mesa_is_desktop_gl_core(ctx) ||
                             _mesa_is_gles2(ctx));

   ctx->Point.SpriteOrigin = GL_UPPER_LEFT; /* GL_ARB_point_sprite */
   ctx->Point.CoordReplace = 0; /* GL_ARB_point_sprite */
}
