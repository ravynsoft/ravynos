/*
 * Copyright (c) 2008-2016 VMware, Inc.
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
 */


#ifndef U_DEBUG_IMAGE_H
#define U_DEBUG_IMAGE_H


#include "util/compiler.h"
#include "util/format/u_formats.h"


#ifdef DEBUG
struct pipe_context;
struct pipe_surface;
struct pipe_transfer;
struct pipe_resource;

void debug_dump_image(const char *prefix,
                      enum pipe_format format, unsigned cpp,
                      unsigned width, unsigned height,
                      unsigned stride,
                      const void *data);
void debug_dump_surface(struct pipe_context *pipe,
			const char *prefix,
                        struct pipe_surface *surface);
void debug_dump_texture(struct pipe_context *pipe,
			const char *prefix,
                        struct pipe_resource *texture);
void debug_dump_surface_bmp(struct pipe_context *pipe,
                            const char *filename,
                            struct pipe_surface *surface);
void debug_dump_transfer_bmp(struct pipe_context *pipe,
                             const char *filename,
                             struct pipe_transfer *transfer, void *ptr);
void debug_dump_float_rgba_bmp(const char *filename,
                               unsigned width, unsigned height,
                               float *rgba, unsigned stride);
void debug_dump_ubyte_rgba_bmp(const char *filename,
                               unsigned width, unsigned height,
                               const uint8_t *rgba, unsigned stride);
#else
#define debug_dump_image(prefix, format, cpp, width, height, stride, data) ((void)0)
#define debug_dump_surface(pipe, prefix, surface) ((void)0)
#define debug_dump_surface_bmp(pipe, filename, surface) ((void)0)
#define debug_dump_transfer_bmp(filename, transfer, ptr) ((void)0)
#define debug_dump_float_rgba_bmp(filename, width, height, rgba, stride) ((void)0)
#define debug_dump_ubyte_rgba_bmp(filename, width, height, rgba, stride) ((void)0)
#endif


#endif
