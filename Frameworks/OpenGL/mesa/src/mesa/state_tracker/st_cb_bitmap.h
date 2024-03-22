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


#ifndef ST_CB_BITMAP_H
#define ST_CB_BITMAP_H


#include <stdbool.h>

struct dd_function_table;
struct st_context;
struct gl_program;
struct st_program;
struct gl_context;
struct gl_pixelstore_attrib;

extern void
st_destroy_bitmap(struct st_context *st);

extern void
st_flush_bitmap_cache(struct st_context *st);

struct pipe_resource *
st_make_bitmap_texture(struct gl_context *ctx, GLsizei width, GLsizei height,
                       const struct gl_pixelstore_attrib *unpack,
                       const GLubyte *bitmap);

void st_Bitmap(struct gl_context *ctx, GLint x, GLint y,
               GLsizei width, GLsizei height,
               const struct gl_pixelstore_attrib *unpack, const GLubyte *bitmap,
               struct pipe_resource *tex);

#endif /* ST_CB_BITMAP_H */
