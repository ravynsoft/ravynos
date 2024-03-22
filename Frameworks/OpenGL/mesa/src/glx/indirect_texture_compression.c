/*
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glx_texture_compression.c
 * Contains the routines required to implement GLX protocol for
 * ARB_texture_compression and related extensions.
 *
 * \sa http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_compression.txt
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include "packrender.h"
#include "packsingle.h"
#include "indirect.h"

#include <assert.h>


void
__indirect_glGetCompressedTexImage(GLenum target, GLint level,
                                      GLvoid * img)
{
   __GLX_SINGLE_DECLARE_VARIABLES();
   xGLXGetTexImageReply reply;
   size_t image_bytes;

   __GLX_SINGLE_LOAD_VARIABLES();
   __GLX_SINGLE_BEGIN(X_GLsop_GetCompressedTexImage, 8);
   __GLX_SINGLE_PUT_LONG(0, target);
   __GLX_SINGLE_PUT_LONG(4, level);
   __GLX_SINGLE_READ_XREPLY();

   image_bytes = reply.width;
   assert(image_bytes <= ((4 * reply.length) - 0));
   assert(image_bytes >= ((4 * reply.length) - 3));

   if (image_bytes != 0) {
      _XRead(dpy, (char *) img, image_bytes);
      if (image_bytes < (4 * reply.length)) {
         _XEatData(dpy, (4 * reply.length) - image_bytes);
      }
   }

   __GLX_SINGLE_END();
}


/**
 * Internal function used for \c glCompressedTexImage1D and
 * \c glCompressedTexImage2D.
 */
static void
CompressedTexImage1D2D(GLenum target, GLint level,
                       GLenum internal_format,
                       GLsizei width, GLsizei height,
                       GLint border, GLsizei image_size,
                       const GLvoid * data, CARD32 rop)
{
   __GLX_DECLARE_VARIABLES();

   __GLX_LOAD_VARIABLES();
   if (gc->currentDpy == NULL) {
      return;
   }

   if ((target == GL_PROXY_TEXTURE_1D)
       || (target == GL_PROXY_TEXTURE_2D)
       || (target == GL_PROXY_TEXTURE_CUBE_MAP)) {
      compsize = 0;
   }
   else {
      compsize = image_size;
   }

   cmdlen = __GLX_PAD(__GLX_COMPRESSED_TEXIMAGE_CMD_HDR_SIZE + compsize);
   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      __GLX_BEGIN_VARIABLE(rop, cmdlen);
      __GLX_PUT_LONG(4, target);
      __GLX_PUT_LONG(8, level);
      __GLX_PUT_LONG(12, internal_format);
      __GLX_PUT_LONG(16, width);
      __GLX_PUT_LONG(20, height);
      __GLX_PUT_LONG(24, border);
      __GLX_PUT_LONG(28, image_size);
      if (compsize != 0) {
         __GLX_PUT_CHAR_ARRAY(__GLX_COMPRESSED_TEXIMAGE_CMD_HDR_SIZE,
                              data, image_size);
      }
      __GLX_END(cmdlen);
   }
   else {
      assert(compsize != 0);

      __GLX_BEGIN_VARIABLE_LARGE(rop, cmdlen + 4);
      __GLX_PUT_LONG(8, target);
      __GLX_PUT_LONG(12, level);
      __GLX_PUT_LONG(16, internal_format);
      __GLX_PUT_LONG(20, width);
      __GLX_PUT_LONG(24, height);
      __GLX_PUT_LONG(28, border);
      __GLX_PUT_LONG(32, image_size);
      __glXSendLargeCommand(gc, gc->pc,
                            __GLX_COMPRESSED_TEXIMAGE_CMD_HDR_SIZE + 4,
                            data, image_size);
   }
}


/**
 * Internal function used for \c glCompressedTexSubImage1D and
 * \c glCompressedTexSubImage2D.
 */
static void
CompressedTexSubImage1D2D(GLenum target, GLint level,
                          GLsizei xoffset, GLsizei yoffset,
                          GLsizei width, GLsizei height,
                          GLenum format, GLsizei image_size,
                          const GLvoid * data, CARD32 rop)
{
   __GLX_DECLARE_VARIABLES();

   __GLX_LOAD_VARIABLES();
   if (gc->currentDpy == NULL) {
      return;
   }

   if (target == GL_PROXY_TEXTURE_3D) {
      compsize = 0;
   }
   else {
      compsize = image_size;
   }

   cmdlen = __GLX_PAD(__GLX_COMPRESSED_TEXSUBIMAGE_CMD_HDR_SIZE + compsize);
   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      __GLX_BEGIN_VARIABLE(rop, cmdlen);
      __GLX_PUT_LONG(4, target);
      __GLX_PUT_LONG(8, level);
      __GLX_PUT_LONG(12, xoffset);
      __GLX_PUT_LONG(16, yoffset);
      __GLX_PUT_LONG(20, width);
      __GLX_PUT_LONG(24, height);
      __GLX_PUT_LONG(28, format);
      __GLX_PUT_LONG(32, image_size);
      if (compsize != 0) {
         __GLX_PUT_CHAR_ARRAY(__GLX_COMPRESSED_TEXSUBIMAGE_CMD_HDR_SIZE,
                              data, image_size);
      }
      __GLX_END(cmdlen);
   }
   else {
      assert(compsize != 0);

      __GLX_BEGIN_VARIABLE_LARGE(rop, cmdlen + 4);
      __GLX_PUT_LONG(8, target);
      __GLX_PUT_LONG(12, level);
      __GLX_PUT_LONG(16, xoffset);
      __GLX_PUT_LONG(20, yoffset);
      __GLX_PUT_LONG(24, width);
      __GLX_PUT_LONG(28, height);
      __GLX_PUT_LONG(32, format);
      __GLX_PUT_LONG(36, image_size);
      __glXSendLargeCommand(gc, gc->pc,
                            __GLX_COMPRESSED_TEXSUBIMAGE_CMD_HDR_SIZE + 4,
                            data, image_size);
   }
}


void
__indirect_glCompressedTexImage1D(GLenum target, GLint level,
                                     GLenum internal_format, GLsizei width,
                                     GLint border, GLsizei image_size,
                                     const GLvoid * data)
{
   CompressedTexImage1D2D(target, level, internal_format, width, 0,
                          border, image_size, data,
                          X_GLrop_CompressedTexImage1D);
}


void
__indirect_glCompressedTexImage2D(GLenum target, GLint level,
                                     GLenum internal_format,
                                     GLsizei width, GLsizei height,
                                     GLint border, GLsizei image_size,
                                     const GLvoid * data)
{
   CompressedTexImage1D2D(target, level, internal_format, width, height,
                          border, image_size, data,
                          X_GLrop_CompressedTexImage2D);
}


void
__indirect_glCompressedTexImage3D(GLenum target, GLint level,
                                     GLenum internal_format,
                                     GLsizei width, GLsizei height,
                                     GLsizei depth, GLint border,
                                     GLsizei image_size, const GLvoid * data)
{
   __GLX_DECLARE_VARIABLES();

   __GLX_LOAD_VARIABLES();
   if (gc->currentDpy == NULL) {
      return;
   }

   cmdlen = __GLX_PAD(__GLX_COMPRESSED_TEXIMAGE_3D_CMD_HDR_SIZE + image_size);
   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      __GLX_BEGIN_VARIABLE(X_GLrop_CompressedTexImage3D, cmdlen);
      __GLX_PUT_LONG(4, target);
      __GLX_PUT_LONG(8, level);
      __GLX_PUT_LONG(12, internal_format);
      __GLX_PUT_LONG(16, width);
      __GLX_PUT_LONG(20, height);
      __GLX_PUT_LONG(24, depth);
      __GLX_PUT_LONG(28, border);
      __GLX_PUT_LONG(32, image_size);
      if (image_size != 0) {
         __GLX_PUT_CHAR_ARRAY(__GLX_COMPRESSED_TEXIMAGE_3D_CMD_HDR_SIZE,
                              data, image_size);
      }
      __GLX_END(cmdlen);
   }
   else {
      __GLX_BEGIN_VARIABLE_LARGE(X_GLrop_CompressedTexImage3D, cmdlen + 4);
      __GLX_PUT_LONG(8, target);
      __GLX_PUT_LONG(12, level);
      __GLX_PUT_LONG(16, internal_format);
      __GLX_PUT_LONG(20, width);
      __GLX_PUT_LONG(24, height);
      __GLX_PUT_LONG(28, depth);
      __GLX_PUT_LONG(32, border);
      __GLX_PUT_LONG(36, image_size);
      __glXSendLargeCommand(gc, gc->pc,
                            __GLX_COMPRESSED_TEXIMAGE_3D_CMD_HDR_SIZE + 4,
                            data, image_size);
   }
}


void
__indirect_glCompressedTexSubImage1D(GLenum target, GLint level,
                                        GLint xoffset,
                                        GLsizei width,
                                        GLenum format, GLsizei image_size,
                                        const GLvoid * data)
{
   CompressedTexSubImage1D2D(target, level, xoffset, 0, width, 0,
                             format, image_size, data,
                             X_GLrop_CompressedTexSubImage1D);
}


void
__indirect_glCompressedTexSubImage2D(GLenum target, GLint level,
                                        GLint xoffset, GLint yoffset,
                                        GLsizei width, GLsizei height,
                                        GLenum format, GLsizei image_size,
                                        const GLvoid * data)
{
   CompressedTexSubImage1D2D(target, level, xoffset, yoffset, width, height,
                             format, image_size, data,
                             X_GLrop_CompressedTexSubImage2D);
}


void
__indirect_glCompressedTexSubImage3D(GLenum target, GLint level,
                                        GLint xoffset, GLint yoffset,
                                        GLint zoffset, GLsizei width,
                                        GLsizei height, GLsizei depth,
                                        GLenum format, GLsizei image_size,
                                        const GLvoid * data)
{
   __GLX_DECLARE_VARIABLES();

   __GLX_LOAD_VARIABLES();
   if (gc->currentDpy == NULL) {
      return;
   }

   cmdlen = __GLX_PAD(__GLX_COMPRESSED_TEXSUBIMAGE_3D_CMD_HDR_SIZE
                      + image_size);
   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      __GLX_BEGIN_VARIABLE(X_GLrop_CompressedTexSubImage3D, cmdlen);
      __GLX_PUT_LONG(4, target);
      __GLX_PUT_LONG(8, level);
      __GLX_PUT_LONG(12, xoffset);
      __GLX_PUT_LONG(16, yoffset);
      __GLX_PUT_LONG(20, zoffset);
      __GLX_PUT_LONG(24, width);
      __GLX_PUT_LONG(28, height);
      __GLX_PUT_LONG(32, depth);
      __GLX_PUT_LONG(36, format);
      __GLX_PUT_LONG(40, image_size);
      if (image_size != 0) {
         __GLX_PUT_CHAR_ARRAY(__GLX_COMPRESSED_TEXSUBIMAGE_3D_CMD_HDR_SIZE,
                              data, image_size);
      }
      __GLX_END(cmdlen);
   }
   else {
      __GLX_BEGIN_VARIABLE_LARGE(X_GLrop_CompressedTexSubImage3D, cmdlen + 4);
      __GLX_PUT_LONG(8, target);
      __GLX_PUT_LONG(12, level);
      __GLX_PUT_LONG(16, xoffset);
      __GLX_PUT_LONG(20, yoffset);
      __GLX_PUT_LONG(24, zoffset);
      __GLX_PUT_LONG(28, width);
      __GLX_PUT_LONG(32, height);
      __GLX_PUT_LONG(36, depth);
      __GLX_PUT_LONG(40, format);
      __GLX_PUT_LONG(44, image_size);
      __glXSendLargeCommand(gc, gc->pc,
                            __GLX_COMPRESSED_TEXSUBIMAGE_3D_CMD_HDR_SIZE + 4,
                            data, image_size);
   }
}
