/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
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
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 * Authors:
 *      Christian König <christian.koenig@amd.com>
 *
 */

#ifndef ST_VDPAU_H
#define ST_VDPAU_H

#ifdef HAVE_ST_VDPAU

void
st_vdpau_map_surface(struct gl_context *ctx, GLenum target, GLenum access,
                     GLboolean output, struct gl_texture_object *texObj,
                     struct gl_texture_image *texImage,
                     const void *vdpSurface, GLuint index);
void
st_vdpau_unmap_surface(struct gl_context *ctx, GLenum target, GLenum access,
                       GLboolean output, struct gl_texture_object *texObj,
                       struct gl_texture_image *texImage,
                       const void *vdpSurface, GLuint index);
#else

static inline void
st_vdpau_map_surface(struct gl_context *ctx, GLenum target, GLenum access,
                     GLboolean output, struct gl_texture_object *texObj,
                     struct gl_texture_image *texImage,
                     const void *vdpSurface, GLuint index) {}
static inline void
st_vdpau_unmap_surface(struct gl_context *ctx, GLenum target, GLenum access,
                       GLboolean output, struct gl_texture_object *texObj,
                       struct gl_texture_image *texImage,
                       const void *vdpSurface, GLuint index) {}
#endif

#endif /* ST_VDPAU_H */
