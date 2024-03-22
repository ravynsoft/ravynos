/*
 * Copyright 2016 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ST_PBO_H
#define ST_PBO_H

struct gl_pixelstore_attrib;

struct st_context;

struct st_pbo_addresses {
   int xoffset;
   int yoffset;
   unsigned width;
   unsigned height;
   unsigned depth;

   unsigned bytes_per_pixel;

   /* Everything below is filled in by st_pbo_from_pixelstore */
   unsigned pixels_per_row;
   unsigned image_height;

   /* Everything below is filled in by st_pbo_setup_buffer */

   /* Buffer and view. */
   struct pipe_resource *buffer; /* non-owning pointer */
   unsigned first_element;
   unsigned last_element;

   /* Constant buffer for the fragment shader. */
   struct {
      int32_t xoffset;
      int32_t yoffset;
      int32_t stride;
      int32_t image_size;
      int32_t layer_offset;
   } constants;
};

/* Conversion to apply in the fragment shader. */
enum st_pbo_conversion {
   ST_PBO_CONVERT_FLOAT = 0,
   ST_PBO_CONVERT_UINT,
   ST_PBO_CONVERT_SINT,
   ST_PBO_CONVERT_UINT_TO_SINT,
   ST_PBO_CONVERT_SINT_TO_UINT,

   ST_NUM_PBO_CONVERSIONS
};

const struct glsl_type *
st_pbo_sampler_type_for_target(enum pipe_texture_target target,
                               enum st_pbo_conversion conv);
bool
st_pbo_addresses_setup(struct st_context *st,
                       struct pipe_resource *buf, intptr_t buf_offset,
                       struct st_pbo_addresses *addr);

bool
st_pbo_addresses_pixelstore(struct st_context *st,
                            GLenum gl_target, bool skip_images,
                            const struct gl_pixelstore_attrib *store,
                            const void *pixels,
                            struct st_pbo_addresses *addr);

void
st_pbo_addresses_invert_y(struct st_pbo_addresses *addr,
                          unsigned viewport_height);

bool
st_pbo_draw(struct st_context *st, const struct st_pbo_addresses *addr,
            unsigned surface_width, unsigned surface_height);

void *
st_pbo_create_vs(struct st_context *st);

void *
st_pbo_create_gs(struct st_context *st);

void *
st_pbo_get_upload_fs(struct st_context *st,
                     enum pipe_format src_format,
                     enum pipe_format dst_format,
                     bool need_layer);

void *
st_pbo_get_download_fs(struct st_context *st, enum pipe_texture_target target,
                       enum pipe_format src_format,
                       enum pipe_format dst_format,
                       bool need_layer);

bool
st_GetTexSubImage_shader(struct gl_context * ctx,
                         GLint xoffset, GLint yoffset, GLint zoffset,
                         GLsizei width, GLsizei height, GLint depth,
                         GLenum format, GLenum type, void * pixels,
                         struct gl_texture_image *texImage);

enum pipe_format
st_pbo_get_dst_format(struct gl_context *ctx, enum pipe_texture_target target,
                      enum pipe_format src_format, bool is_compressed,
                      GLenum format, GLenum type, unsigned bind);
enum pipe_format
st_pbo_get_src_format(struct pipe_screen *screen, enum pipe_format src_format, struct pipe_resource *src);

extern void
st_init_pbo_helpers(struct st_context *st);

extern void
st_destroy_pbo_helpers(struct st_context *st);
void
st_pbo_compute_deinit(struct st_context *st);

#endif /* ST_PBO_H */
