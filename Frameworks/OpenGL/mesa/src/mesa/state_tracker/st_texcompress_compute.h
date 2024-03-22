/**************************************************************************
 *
 * Copyright © 2022 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef ST_TEXCOMPRESS_COMPUTE_H
#define ST_TEXCOMPRESS_COMPUTE_H

bool
st_init_texcompress_compute(struct st_context *st);

void
st_destroy_texcompress_compute(struct st_context *st);

/**
 * When this function returns true, the destination image will contain the
 * contents of astc_data but transcoded to DXT5/BC3.
 *
 * Note that this function will internally create compute programs by using
 * glCreateShaderProgramv with the application's GL context.
 */
bool
st_compute_transcode_astc_to_dxt5(struct st_context *st,
                                  uint8_t *astc_data,
                                  unsigned astc_stride,
                                  mesa_format astc_format,
                                  struct pipe_resource *dxt5_tex,
                                  unsigned dxt5_level,
                                  unsigned dxt5_layer);

#endif /* ST_TEXCOMPRESS_COMPUTE_H */
