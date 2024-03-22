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
 * \file clear.c
 * glClearColor, glClearIndex, glClear() functions.
 */



#include "glformats.h"
#include "util/glheader.h"
#include "context.h"
#include "enums.h"
#include "fbobject.h"
#include "get.h"
#include "macros.h"
#include "mtypes.h"
#include "state.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_clear.h"

void GLAPIENTRY
_mesa_ClearIndex( GLfloat c )
{
   GET_CURRENT_CONTEXT(ctx);

   ctx->PopAttribState |= GL_COLOR_BUFFER_BIT;
   ctx->Color.ClearIndex = (GLuint) c;
}


/**
 * Specify the clear values for the color buffers.
 *
 * \param red red color component.
 * \param green green color component.
 * \param blue blue color component.
 * \param alpha alpha component.
 *
 * \sa glClearColor().
 */
void GLAPIENTRY
_mesa_ClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
   GET_CURRENT_CONTEXT(ctx);

   ctx->PopAttribState |= GL_COLOR_BUFFER_BIT;
   ctx->Color.ClearColor.f[0] = red;
   ctx->Color.ClearColor.f[1] = green;
   ctx->Color.ClearColor.f[2] = blue;
   ctx->Color.ClearColor.f[3] = alpha;
}


/**
 * GL_EXT_texture_integer
 */
void GLAPIENTRY
_mesa_ClearColorIiEXT(GLint r, GLint g, GLint b, GLint a)
{
   GET_CURRENT_CONTEXT(ctx);

   ctx->PopAttribState |= GL_COLOR_BUFFER_BIT;
   ctx->Color.ClearColor.i[0] = r;
   ctx->Color.ClearColor.i[1] = g;
   ctx->Color.ClearColor.i[2] = b;
   ctx->Color.ClearColor.i[3] = a;
}


/**
 * GL_EXT_texture_integer
 */
void GLAPIENTRY
_mesa_ClearColorIuiEXT(GLuint r, GLuint g, GLuint b, GLuint a)
{
   GET_CURRENT_CONTEXT(ctx);

   ctx->PopAttribState |= GL_COLOR_BUFFER_BIT;
   ctx->Color.ClearColor.ui[0] = r;
   ctx->Color.ClearColor.ui[1] = g;
   ctx->Color.ClearColor.ui[2] = b;
   ctx->Color.ClearColor.ui[3] = a;
}


/**
 * Returns true if color writes are enabled for the given color attachment.
 *
 * Beyond checking ColorMask, this uses _mesa_format_has_color_component to
 * ignore components that don't actually exist in the format (such as X in
 * XRGB).
 */
static bool
color_buffer_writes_enabled(const struct gl_context *ctx, unsigned idx)
{
   struct gl_renderbuffer *rb = ctx->DrawBuffer->_ColorDrawBuffers[idx];
   GLuint c;

   if (rb) {
      for (c = 0; c < 4; c++) {
         if (GET_COLORMASK_BIT(ctx->Color.ColorMask, idx, c) &&
             _mesa_format_has_color_component(rb->Format, c)) {
            return true;
         }
      }
   }

   return false;
}


/**
 * Clear buffers.
 *
 * \param mask bit-mask indicating the buffers to be cleared.
 *
 * Flushes the vertices and verifies the parameter.
 * If __struct gl_contextRec::NewState is set then calls _mesa_update_clear_state()
 * to update gl_frame_buffer::_Xmin, etc.  If the rasterization mode is set to
 * GL_RENDER then requests the driver to clear the buffers, via the
 * dd_function_table::Clear callback.
 */
static ALWAYS_INLINE void
clear(struct gl_context *ctx, GLbitfield mask, bool no_error)
{
   FLUSH_VERTICES(ctx, 0, 0);

   if (!no_error) {
      if (mask & ~(GL_COLOR_BUFFER_BIT |
                   GL_DEPTH_BUFFER_BIT |
                   GL_STENCIL_BUFFER_BIT |
                   GL_ACCUM_BUFFER_BIT)) {
         _mesa_error( ctx, GL_INVALID_VALUE, "glClear(0x%x)", mask);
         return;
      }

      /* Accumulation buffers were removed in core contexts, and they never
       * existed in OpenGL ES.
       */
      if ((mask & GL_ACCUM_BUFFER_BIT) != 0
          && (_mesa_is_desktop_gl_core(ctx) || _mesa_is_gles(ctx))) {
         _mesa_error( ctx, GL_INVALID_VALUE, "glClear(GL_ACCUM_BUFFER_BIT)");
         return;
      }
   }

   if (ctx->NewState) {
      _mesa_update_clear_state( ctx );	/* update _Xmin, etc */
   }

   if (!no_error && ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "glClear(incomplete framebuffer)");
      return;
   }

   if (ctx->RasterDiscard)
      return;

   if (ctx->RenderMode == GL_RENDER) {
      GLbitfield bufferMask;

      /* don't clear depth buffer if depth writing disabled */
      if (!ctx->Depth.Mask)
         mask &= ~GL_DEPTH_BUFFER_BIT;

      /* Build the bitmask to send to device driver's Clear function.
       * Note that the GL_COLOR_BUFFER_BIT flag will expand to 0, 1, 2 or 4
       * of the BUFFER_BIT_FRONT/BACK_LEFT/RIGHT flags, or one of the
       * BUFFER_BIT_COLORn flags.
       */
      bufferMask = 0;
      if (mask & GL_COLOR_BUFFER_BIT) {
         GLuint i;
         for (i = 0; i < ctx->DrawBuffer->_NumColorDrawBuffers; i++) {
            gl_buffer_index buf = ctx->DrawBuffer->_ColorDrawBufferIndexes[i];

            if (buf != BUFFER_NONE && color_buffer_writes_enabled(ctx, i)) {
               bufferMask |= 1 << buf;
            }
         }
      }

      if ((mask & GL_DEPTH_BUFFER_BIT)
          && ctx->DrawBuffer->Visual.depthBits > 0) {
         bufferMask |= BUFFER_BIT_DEPTH;
      }

      if ((mask & GL_STENCIL_BUFFER_BIT)
          && ctx->DrawBuffer->Visual.stencilBits > 0) {
         bufferMask |= BUFFER_BIT_STENCIL;
      }

      if ((mask & GL_ACCUM_BUFFER_BIT)
          && ctx->DrawBuffer->Visual.accumRedBits > 0) {
         bufferMask |= BUFFER_BIT_ACCUM;
      }

      st_Clear(ctx, bufferMask);
   }
}


void GLAPIENTRY
_mesa_Clear_no_error(GLbitfield mask)
{
   GET_CURRENT_CONTEXT(ctx);
   clear(ctx, mask, true);
}


void GLAPIENTRY
_mesa_Clear(GLbitfield mask)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glClear 0x%x\n", mask);

   clear(ctx, mask, false);
}


/** Returned by make_color_buffer_mask() for errors */
#define INVALID_MASK ~0x0U


/**
 * Convert the glClearBuffer 'drawbuffer' parameter into a bitmask of
 * BUFFER_BIT_x values.
 * Return INVALID_MASK if the drawbuffer value is invalid.
 */
static GLbitfield
make_color_buffer_mask(struct gl_context *ctx, GLint drawbuffer)
{
   const struct gl_renderbuffer_attachment *att = ctx->DrawBuffer->Attachment;
   GLbitfield mask = 0x0;

   /* From the GL 4.0 specification:
    *	If buffer is COLOR, a particular draw buffer DRAW_BUFFERi is
    *	specified by passing i as the parameter drawbuffer, and value
    *	points to a four-element vector specifying the R, G, B, and A
    *	color to clear that draw buffer to. If the draw buffer is one
    *	of FRONT, BACK, LEFT, RIGHT, or FRONT_AND_BACK, identifying
    *	multiple buffers, each selected buffer is cleared to the same
    *	value.
    *
    * Note that "drawbuffer" and "draw buffer" have different meaning.
    * "drawbuffer" specifies DRAW_BUFFERi, while "draw buffer" is what's
    * assigned to DRAW_BUFFERi. It could be COLOR_ATTACHMENT0, FRONT, BACK,
    * etc.
    */
   if (drawbuffer < 0 || drawbuffer >= (GLint)ctx->Const.MaxDrawBuffers) {
      return INVALID_MASK;
   }

   switch (ctx->DrawBuffer->ColorDrawBuffer[drawbuffer]) {
   case GL_FRONT:
      if (att[BUFFER_FRONT_LEFT].Renderbuffer)
         mask |= BUFFER_BIT_FRONT_LEFT;
      if (att[BUFFER_FRONT_RIGHT].Renderbuffer)
         mask |= BUFFER_BIT_FRONT_RIGHT;
      break;
   case GL_BACK:
      /* For GLES contexts with a single buffered configuration, we actually
       * only have a front renderbuffer, so any clear calls to GL_BACK should
       * affect that buffer. See draw_buffer_enum_to_bitmask for details.
       */
      if (_mesa_is_gles(ctx))
         if (!ctx->DrawBuffer->Visual.doubleBufferMode)
            if (att[BUFFER_FRONT_LEFT].Renderbuffer)
               mask |= BUFFER_BIT_FRONT_LEFT;
      if (att[BUFFER_BACK_LEFT].Renderbuffer)
         mask |= BUFFER_BIT_BACK_LEFT;
      if (att[BUFFER_BACK_RIGHT].Renderbuffer)
         mask |= BUFFER_BIT_BACK_RIGHT;
      break;
   case GL_LEFT:
      if (att[BUFFER_FRONT_LEFT].Renderbuffer)
         mask |= BUFFER_BIT_FRONT_LEFT;
      if (att[BUFFER_BACK_LEFT].Renderbuffer)
         mask |= BUFFER_BIT_BACK_LEFT;
      break;
   case GL_RIGHT:
      if (att[BUFFER_FRONT_RIGHT].Renderbuffer)
         mask |= BUFFER_BIT_FRONT_RIGHT;
      if (att[BUFFER_BACK_RIGHT].Renderbuffer)
         mask |= BUFFER_BIT_BACK_RIGHT;
      break;
   case GL_FRONT_AND_BACK:
      if (att[BUFFER_FRONT_LEFT].Renderbuffer)
         mask |= BUFFER_BIT_FRONT_LEFT;
      if (att[BUFFER_BACK_LEFT].Renderbuffer)
         mask |= BUFFER_BIT_BACK_LEFT;
      if (att[BUFFER_FRONT_RIGHT].Renderbuffer)
         mask |= BUFFER_BIT_FRONT_RIGHT;
      if (att[BUFFER_BACK_RIGHT].Renderbuffer)
         mask |= BUFFER_BIT_BACK_RIGHT;
      break;
   default:
      {
         gl_buffer_index buf =
            ctx->DrawBuffer->_ColorDrawBufferIndexes[drawbuffer];

         if (buf != BUFFER_NONE && att[buf].Renderbuffer) {
            mask |= 1 << buf;
         }
      }
   }

   return mask;
}



/**
 * New in GL 3.0
 * Clear signed integer color buffer or stencil buffer (not depth).
 */
static ALWAYS_INLINE void
clear_bufferiv(struct gl_context *ctx, GLenum buffer, GLint drawbuffer,
               const GLint *value, bool no_error)
{
   FLUSH_VERTICES(ctx, 0, 0);

   if (ctx->NewState) {
      _mesa_update_clear_state( ctx );
   }

   if (!no_error && ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "glClearBufferiv(incomplete framebuffer)");
      return;
   }

   switch (buffer) {
   case GL_STENCIL:
      /* Page 264 (page 280 of the PDF) of the OpenGL 3.0 spec says:
       *
       *     "ClearBuffer generates an INVALID VALUE error if buffer is
       *     COLOR and drawbuffer is less than zero, or greater than the
       *     value of MAX DRAW BUFFERS minus one; or if buffer is DEPTH,
       *     STENCIL, or DEPTH STENCIL and drawbuffer is not zero."
       */
      if (!no_error && drawbuffer != 0) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glClearBufferiv(drawbuffer=%d)",
                     drawbuffer);
         return;
      }
      else if (ctx->DrawBuffer->Attachment[BUFFER_STENCIL].Renderbuffer
               && !ctx->RasterDiscard) {
         /* Save current stencil clear value, set to 'value', do the
          * stencil clear and restore the clear value.
          * XXX in the future we may have a new st_ClearBuffer()
          * hook instead.
          */
         const GLuint clearSave = ctx->Stencil.Clear;
         ctx->Stencil.Clear = *value;
         st_Clear(ctx, BUFFER_BIT_STENCIL);
         ctx->Stencil.Clear = clearSave;
      }
      break;
   case GL_COLOR:
      {
         const GLbitfield mask = make_color_buffer_mask(ctx, drawbuffer);
         if (!no_error && mask == INVALID_MASK) {
            _mesa_error(ctx, GL_INVALID_VALUE, "glClearBufferiv(drawbuffer=%d)",
                        drawbuffer);
            return;
         }
         else if (mask && !ctx->RasterDiscard) {
            union gl_color_union clearSave;

            /* save color */
            clearSave = ctx->Color.ClearColor;
            /* set color */
            COPY_4V(ctx->Color.ClearColor.i, value);
            /* clear buffer(s) */
            st_Clear(ctx, mask);
            /* restore color */
            ctx->Color.ClearColor = clearSave;
         }
      }
      break;
   default:
      if (!no_error) {
         /* Page 498 of the PDF, section '17.4.3.1 Clearing Individual Buffers'
          * of the OpenGL 4.5 spec states:
          *
          *    "An INVALID_ENUM error is generated by ClearBufferiv and
          *     ClearNamedFramebufferiv if buffer is not COLOR or STENCIL."
          */
         _mesa_error(ctx, GL_INVALID_ENUM, "glClearBufferiv(buffer=%s)",
                     _mesa_enum_to_string(buffer));
      }
      return;
   }
}


void GLAPIENTRY
_mesa_ClearBufferiv_no_error(GLenum buffer, GLint drawbuffer, const GLint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   clear_bufferiv(ctx, buffer, drawbuffer, value, true);
}


void GLAPIENTRY
_mesa_ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   clear_bufferiv(ctx, buffer, drawbuffer, value, false);
}


/**
 * The ClearBuffer framework is so complicated and so riddled with the
 * assumption that the framebuffer is bound that, for now, we will just fake
 * direct state access clearing for the user.
 */
void GLAPIENTRY
_mesa_ClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer,
                              GLint drawbuffer, const GLint *value)
{
   GLint oldfb;

   _mesa_GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldfb);
   _mesa_BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
   _mesa_ClearBufferiv(buffer, drawbuffer, value);
   _mesa_BindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint) oldfb);
}


/**
 * New in GL 3.0
 * Clear unsigned integer color buffer (not depth, not stencil).
 */
static ALWAYS_INLINE void
clear_bufferuiv(struct gl_context *ctx, GLenum buffer, GLint drawbuffer,
                const GLuint *value, bool no_error)
{
   FLUSH_VERTICES(ctx, 0, 0);

   if (ctx->NewState) {
      _mesa_update_clear_state( ctx );
   }

   if (!no_error && ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION,
                  "glClearBufferuiv(incomplete framebuffer)");
      return;
   }

   switch (buffer) {
   case GL_COLOR:
      {
         const GLbitfield mask = make_color_buffer_mask(ctx, drawbuffer);
         if (!no_error && mask == INVALID_MASK) {
            _mesa_error(ctx, GL_INVALID_VALUE, "glClearBufferuiv(drawbuffer=%d)",
                        drawbuffer);
            return;
         }
         else if (mask && !ctx->RasterDiscard) {
            union gl_color_union clearSave;

            /* save color */
            clearSave = ctx->Color.ClearColor;
            /* set color */
            COPY_4V(ctx->Color.ClearColor.ui, value);
            /* clear buffer(s) */
            st_Clear(ctx, mask);
            /* restore color */
            ctx->Color.ClearColor = clearSave;
         }
      }
      break;
   default:
      if (!no_error) {
         /* Page 498 of the PDF, section '17.4.3.1 Clearing Individual Buffers'
          * of the OpenGL 4.5 spec states:
          *
          *    "An INVALID_ENUM error is generated by ClearBufferuiv and
          *     ClearNamedFramebufferuiv if buffer is not COLOR."
          */
         _mesa_error(ctx, GL_INVALID_ENUM, "glClearBufferuiv(buffer=%s)",
                     _mesa_enum_to_string(buffer));
      }
      return;
   }
}


void GLAPIENTRY
_mesa_ClearBufferuiv_no_error(GLenum buffer, GLint drawbuffer,
                              const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   clear_bufferuiv(ctx, buffer, drawbuffer, value, true);
}


void GLAPIENTRY
_mesa_ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   clear_bufferuiv(ctx, buffer, drawbuffer, value, false);
}


/**
 * The ClearBuffer framework is so complicated and so riddled with the
 * assumption that the framebuffer is bound that, for now, we will just fake
 * direct state access clearing for the user.
 */
void GLAPIENTRY
_mesa_ClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer,
                               GLint drawbuffer, const GLuint *value)
{
   GLint oldfb;

   _mesa_GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldfb);
   _mesa_BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
   _mesa_ClearBufferuiv(buffer, drawbuffer, value);
   _mesa_BindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint) oldfb);
}


/**
 * New in GL 3.0
 * Clear fixed-pt or float color buffer or depth buffer (not stencil).
 */
static ALWAYS_INLINE void
clear_bufferfv(struct gl_context *ctx, GLenum buffer, GLint drawbuffer,
               const GLfloat *value, bool no_error)
{
   FLUSH_VERTICES(ctx, 0, 0);

   if (ctx->NewState) {
      _mesa_update_clear_state( ctx );
   }

   if (!no_error && ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION,
                  "glClearBufferfv(incomplete framebuffer)");
      return;
   }

   switch (buffer) {
   case GL_DEPTH:
      /* Page 264 (page 280 of the PDF) of the OpenGL 3.0 spec says:
       *
       *     "ClearBuffer generates an INVALID VALUE error if buffer is
       *     COLOR and drawbuffer is less than zero, or greater than the
       *     value of MAX DRAW BUFFERS minus one; or if buffer is DEPTH,
       *     STENCIL, or DEPTH STENCIL and drawbuffer is not zero."
       */
      if (!no_error && drawbuffer != 0) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glClearBufferfv(drawbuffer=%d)",
                     drawbuffer);
         return;
      }
      else if (ctx->DrawBuffer->Attachment[BUFFER_DEPTH].Renderbuffer
               && !ctx->RasterDiscard) {
         /* Save current depth clear value, set to 'value', do the
          * depth clear and restore the clear value.
          * XXX in the future we may have a new st_ClearBuffer()
          * hook instead.
          */
         const GLclampd clearSave = ctx->Depth.Clear;

         /* Page 263 (page 279 of the PDF) of the OpenGL 3.0 spec says:
          *
          *     "If buffer is DEPTH, drawbuffer must be zero, and value points
          *     to the single depth value to clear the depth buffer to.
          *     Clamping and type conversion for fixed-point depth buffers are
          *     performed in the same fashion as for ClearDepth."
          */
         const struct gl_renderbuffer *rb =
            ctx->DrawBuffer->Attachment[BUFFER_DEPTH].Renderbuffer;
         const bool is_float_depth =
            _mesa_has_depth_float_channel(rb->InternalFormat);
         ctx->Depth.Clear = is_float_depth ? *value : SATURATE(*value);

         st_Clear(ctx, BUFFER_BIT_DEPTH);
         ctx->Depth.Clear = clearSave;
      }
      /* clear depth buffer to value */
      break;
   case GL_COLOR:
      {
         const GLbitfield mask = make_color_buffer_mask(ctx, drawbuffer);
         if (!no_error && mask == INVALID_MASK) {
            _mesa_error(ctx, GL_INVALID_VALUE, "glClearBufferfv(drawbuffer=%d)",
                        drawbuffer);
            return;
         }
         else if (mask && !ctx->RasterDiscard) {
            union gl_color_union clearSave;

            /* save color */
            clearSave = ctx->Color.ClearColor;
            /* set color */
            COPY_4V(ctx->Color.ClearColor.f, value);
            /* clear buffer(s) */
            st_Clear(ctx, mask);
            /* restore color */
            ctx->Color.ClearColor = clearSave;
         }
      }
      break;
   default:
      if (!no_error) {
         /* Page 498 of the PDF, section '17.4.3.1 Clearing Individual Buffers'
          * of the OpenGL 4.5 spec states:
          *
          *    "An INVALID_ENUM error is generated by ClearBufferfv and
          *     ClearNamedFramebufferfv if buffer is not COLOR or DEPTH."
          */
         _mesa_error(ctx, GL_INVALID_ENUM, "glClearBufferfv(buffer=%s)",
                     _mesa_enum_to_string(buffer));
      }
      return;
   }
}


void GLAPIENTRY
_mesa_ClearBufferfv_no_error(GLenum buffer, GLint drawbuffer,
                             const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   clear_bufferfv(ctx, buffer, drawbuffer, value, true);
}


void GLAPIENTRY
_mesa_ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   clear_bufferfv(ctx, buffer, drawbuffer, value, false);
}


/**
 * The ClearBuffer framework is so complicated and so riddled with the
 * assumption that the framebuffer is bound that, for now, we will just fake
 * direct state access clearing for the user.
 */
void GLAPIENTRY
_mesa_ClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer,
                              GLint drawbuffer, const GLfloat *value)
{
   GLint oldfb;

   _mesa_GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldfb);
   _mesa_BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
   _mesa_ClearBufferfv(buffer, drawbuffer, value);
   _mesa_BindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint) oldfb);
}


/**
 * New in GL 3.0
 * Clear depth/stencil buffer only.
 */
static ALWAYS_INLINE void
clear_bufferfi(struct gl_context *ctx, GLenum buffer, GLint drawbuffer,
               GLfloat depth, GLint stencil, bool no_error)
{
   GLbitfield mask = 0;

   FLUSH_VERTICES(ctx, 0, 0);

   if (!no_error) {
      if (buffer != GL_DEPTH_STENCIL) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glClearBufferfi(buffer=%s)",
                     _mesa_enum_to_string(buffer));
         return;
      }

      /* Page 264 (page 280 of the PDF) of the OpenGL 3.0 spec says:
       *
       *     "ClearBuffer generates an INVALID VALUE error if buffer is
       *     COLOR and drawbuffer is less than zero, or greater than the
       *     value of MAX DRAW BUFFERS minus one; or if buffer is DEPTH,
       *     STENCIL, or DEPTH STENCIL and drawbuffer is not zero."
       */
      if (drawbuffer != 0) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glClearBufferfi(drawbuffer=%d)",
                     drawbuffer);
         return;
      }
   }

   if (ctx->RasterDiscard)
      return;

   if (ctx->NewState) {
      _mesa_update_clear_state( ctx );
   }

   if (!no_error && ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "glClearBufferfi(incomplete framebuffer)");
      return;
   }

   if (ctx->DrawBuffer->Attachment[BUFFER_DEPTH].Renderbuffer)
      mask |= BUFFER_BIT_DEPTH;
   if (ctx->DrawBuffer->Attachment[BUFFER_STENCIL].Renderbuffer)
      mask |= BUFFER_BIT_STENCIL;

   if (mask) {
      /* save current clear values */
      const GLclampd clearDepthSave = ctx->Depth.Clear;
      const GLuint clearStencilSave = ctx->Stencil.Clear;

      /* set new clear values
       *
       * Page 263 (page 279 of the PDF) of the OpenGL 3.0 spec says:
       *
       *     "depth and stencil are the values to clear the depth and stencil
       *     buffers to, respectively. Clamping and type conversion for
       *     fixed-point depth buffers are performed in the same fashion as
       *     for ClearDepth."
       */
      const struct gl_renderbuffer *rb =
         ctx->DrawBuffer->Attachment[BUFFER_DEPTH].Renderbuffer;
      const bool has_float_depth = rb &&
         _mesa_has_depth_float_channel(rb->InternalFormat);
      ctx->Depth.Clear = has_float_depth ? depth : SATURATE(depth);
      ctx->Stencil.Clear = stencil;

      /* clear buffers */
      st_Clear(ctx, mask);

      /* restore */
      ctx->Depth.Clear = clearDepthSave;
      ctx->Stencil.Clear = clearStencilSave;
   }
}


void GLAPIENTRY
_mesa_ClearBufferfi_no_error(GLenum buffer, GLint drawbuffer,
                             GLfloat depth, GLint stencil)
{
   GET_CURRENT_CONTEXT(ctx);
   clear_bufferfi(ctx, buffer, drawbuffer, depth, stencil, true);
}


void GLAPIENTRY
_mesa_ClearBufferfi(GLenum buffer, GLint drawbuffer,
                    GLfloat depth, GLint stencil)
{
   GET_CURRENT_CONTEXT(ctx);
   clear_bufferfi(ctx, buffer, drawbuffer, depth, stencil, false);
}


/**
 * The ClearBuffer framework is so complicated and so riddled with the
 * assumption that the framebuffer is bound that, for now, we will just fake
 * direct state access clearing for the user.
 */
void GLAPIENTRY
_mesa_ClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer,
                              GLint drawbuffer, GLfloat depth, GLint stencil)
{
   GLint oldfb;

   _mesa_GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldfb);
   _mesa_BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
   _mesa_ClearBufferfi(buffer, drawbuffer, depth, stencil);
   _mesa_BindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint) oldfb);
}
