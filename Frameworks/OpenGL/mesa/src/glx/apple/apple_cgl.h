/*
 Copyright (c) 2008, 2009 Apple Inc.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 
 Except as contained in this notice, the name(s) of the above
 copyright holders shall not be used in advertising or otherwise to
 promote the sale, use or other dealings in this Software without
 prior written authorization.
*/

#ifndef APPLE_CGL_H
#define APPLE_CGL_H

#include <stdio.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>

/* For GLint and GLsizei on Tiger */
#include <OpenGL/gl.h>

struct apple_cgl_api
{
     GLint version_major, version_minor;
     void (*get_version) (GLint * version_major, GLint * version_minor);

     CGLError(*choose_pixel_format) (const CGLPixelFormatAttribute * attribs,
                                     CGLPixelFormatObj * pix, GLint * npix);
     CGLError(*destroy_pixel_format) (CGLPixelFormatObj pix);

     CGLError(*clear_drawable) (CGLContextObj ctx);
     CGLError(*flush_drawable) (CGLContextObj ctx);

     CGLError(*create_context) (CGLPixelFormatObj pix, CGLContextObj share,
                                CGLContextObj * ctx);
     CGLError(*destroy_context) (CGLContextObj pix);

     CGLError(*set_current_context) (CGLContextObj ctx);
     CGLContextObj(*get_current_context) (void);
   const char *(*error_string) (CGLError error);

     CGLError(*set_off_screen) (CGLContextObj ctx,
                                GLsizei width, GLsizei height, GLint rowbytes,
                                void *baseaddr);

     CGLError(*copy_context) (CGLContextObj src, CGLContextObj dst,
                              GLbitfield mask);

     CGLError(*create_pbuffer) (GLsizei width,
                                GLsizei height,
                                GLenum target,
                                GLenum internalFormat,
                                GLint max_level, CGLPBufferObj * pbuffer);

     CGLError(*destroy_pbuffer) (CGLPBufferObj pbuffer);

     CGLError(*set_pbuffer) (CGLContextObj ctx,
                             CGLPBufferObj pbuffer,
                             GLenum face, GLint level, GLint screen);
};

extern struct apple_cgl_api apple_cgl;

extern void apple_cgl_init(void);

extern void *apple_cgl_get_dl_handle(void);

#endif
