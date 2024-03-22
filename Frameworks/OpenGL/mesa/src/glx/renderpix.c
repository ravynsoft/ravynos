/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: SGI-B-2.0
 */

/*
 * (C) Copyright IBM Corporation 2005
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM,
 * AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "packrender.h"
#include "indirect.h"

/**
 * Send a large image to the server.  If necessary, a buffer is allocated
 * to hold the unpacked data that is copied from the clients memory.
 * 
 * \param gc        Current GLX context
 * \param compsize  Size, in bytes, of the image portion
 * \param dim       Number of dimensions of the image
 * \param width     Width of the image
 * \param height    Height of the image, must be 1 for 1D images
 * \param depth     Depth of the image, must be 1 for 1D or 2D images
 * \param format    Format of the image
 * \param type      Data type of the image
 * \param src       Pointer to the image data
 * \param pc        Pointer to end of the command header
 * \param modes     Pointer to the pixel unpack data
 *
 * \todo
 * Modify this function so that \c NULL images are sent using
 * \c __glXSendLargeChunk instead of __glXSendLargeCommand.  Doing this
 * will eliminate the need to allocate a buffer for that case.
 */
void
__glXSendLargeImage(struct glx_context * gc, GLint compsize, GLint dim,
                    GLint width, GLint height, GLint depth,
                    GLenum format, GLenum type, const GLvoid * src,
                    GLubyte * pc, GLubyte * modes)
{
    /* Allocate a temporary holding buffer */
    GLubyte *buf = malloc(compsize);
    if (!buf) {
	__glXSetError(gc, GL_OUT_OF_MEMORY);
	return;
    }

    /* Apply pixel store unpack modes to copy data into buf */
    if (src != NULL) {
	__glFillImage(gc, dim, width, height, depth, format, type,
                      src, buf, modes);
    }
    else {
	if (dim < 3) {
	    (void) memcpy(modes, __glXDefaultPixelStore + 4, 20);
	}
	else {
	    (void) memcpy(modes, __glXDefaultPixelStore + 0, 36);
	}
    }

    /* Send large command */
    __glXSendLargeCommand(gc, gc->pc, pc - gc->pc, buf, compsize);

    /* Free buffer */
    free((char *) buf);
}

/************************************************************************/

/**
 * Implement GLX protocol for \c glSeparableFilter2D.
 */
void
__indirect_glSeparableFilter2D(GLenum target, GLenum internalformat,
                               GLsizei width, GLsizei height, GLenum format,
                               GLenum type, const GLvoid * row,
                               const GLvoid * column)
{
   __GLX_DECLARE_VARIABLES();
   GLuint compsize2, hdrlen, totalhdrlen, image1len, image2len;

   __GLX_LOAD_VARIABLES();
   compsize = __glImageSize(width, 1, 1, format, type, 0);
   compsize2 = __glImageSize(height, 1, 1, format, type, 0);
   totalhdrlen = __GLX_PAD(__GLX_CONV_FILT_CMD_HDR_SIZE);
   hdrlen = __GLX_PAD(__GLX_CONV_FILT_HDR_SIZE);
   image1len = __GLX_PAD(compsize);
   image2len = __GLX_PAD(compsize2);
   cmdlen = totalhdrlen + image1len + image2len;
   if (!gc->currentDpy)
      return;

   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      /* Use GLXRender protocol to send small command */
      __GLX_BEGIN_VARIABLE_WITH_PIXEL(X_GLrop_SeparableFilter2D, cmdlen);
      __GLX_PUT_LONG(0, target);
      __GLX_PUT_LONG(4, internalformat);
      __GLX_PUT_LONG(8, width);
      __GLX_PUT_LONG(12, height);
      __GLX_PUT_LONG(16, format);
      __GLX_PUT_LONG(20, type);
      pc += hdrlen;
      if (compsize > 0) {
         __glFillImage(gc, 1, width, 1, 1, format, type, row, pc,
                       pixelHeaderPC);
         pc += image1len;
      }
      if (compsize2 > 0) {
         __glFillImage(gc, 1, height, 1, 1, format, type, column, pc, NULL);
         pc += image2len;
      }
      if ((compsize == 0) && (compsize2 == 0)) {
         /* Setup default store modes */
         (void) memcpy(pixelHeaderPC, __glXDefaultPixelStore + 4, 20);
      }
      __GLX_END(0);
   }
   else {
      GLubyte *buf;
      const GLint bufsize = image1len + image2len;

      /* Use GLXRenderLarge protocol to send command */
      __GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL(X_GLrop_SeparableFilter2D,
                                            cmdlen + 4);
      __GLX_PUT_LONG(0, target);
      __GLX_PUT_LONG(4, internalformat);
      __GLX_PUT_LONG(8, width);
      __GLX_PUT_LONG(12, height);
      __GLX_PUT_LONG(16, format);
      __GLX_PUT_LONG(20, type);
      pc += hdrlen;

      /* Allocate a temporary holding buffer */
      buf = malloc(bufsize);
      if (!buf) {
         __glXSetError(gc, GL_OUT_OF_MEMORY);
         return;
      }
      __glFillImage(gc, 1, width, 1, 1, format, type, row, buf,
                    pixelHeaderPC);

      __glFillImage(gc, 1, height, 1, 1, format, type, column,
                    buf + image1len, pixelHeaderPC);

      /* Send large command */
      __glXSendLargeCommand(gc, gc->pc, (GLint) (pc - gc->pc), buf,
                            bufsize);
      /* Free buffer */
      free((char *) buf);
   }
}
