/**********************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef SVGA_FORMAT_H_
#define SVGA_FORMAT_H_


#include "util/format/u_formats.h"
#include "svga_context.h"
#include "svga_types.h"
#include "svga_reg.h"
#include "svga3d_reg.h"


struct svga_screen;


/**
 * Vertex format flags.  These are used to specify that some vertex formats
 * need extra processing/conversion in the vertex shader.  For example,
 * setting the W component to 1, or swapping R/B, or converting packed uint
 * types to signed int/snorm.
 */
#define VF_ADJUST_RANGE     (1 << 0)
#define VF_W_TO_1           (1 << 1)
#define VF_U_TO_F_CAST      (1 << 2)  /* convert uint to float */
#define VF_I_TO_F_CAST      (1 << 3)  /* convert sint to float */
#define VF_BGRA             (1 << 4)  /* swap R/B */
#define VF_PUINT_TO_SNORM   (1 << 5)  /* 10_10_10_2 to snorm */
#define VF_PUINT_TO_USCALED (1 << 6)  /* 10_10_10_2 to uscaled */
#define VF_PUINT_TO_SSCALED (1 << 7)  /* 10_10_10_2 to sscaled */

/**
 * Texture format flags.
 */
#define TF_GEN_MIPS         (1 << 8)  /* supports hw generate mipmap */
#define TF_000X             (1 << 9)  /* swizzle <0, 0, 0, X> */
#define TF_XXXX             (1 << 10) /* swizzle <X, X, X, X> */
#define TF_XXX1             (1 << 11) /* swizzle <X, X, X, 1> */
#define TF_XXXY             (1 << 12) /* swizzle <X, X, X, Y> */
#define TF_UAV              (1 << 13) /* supports uav */
#define TF_SM5              (1 << 14) /* supported in SM5 */

void
svga_translate_vertex_format_vgpu10(enum pipe_format format,
                                    SVGA3dSurfaceFormat *svga_format,
                                    unsigned *vf_flags);

void
svga_translate_texture_buffer_view_format(enum pipe_format format,
                                          SVGA3dSurfaceFormat *svga_format,
                                          unsigned *tf_flags);

enum SVGA3dSurfaceFormat
svga_translate_format(const struct svga_screen *ss,
                      enum pipe_format format,
                      unsigned bind);

void
svga_get_format_cap(struct svga_screen *ss,
                    SVGA3dSurfaceFormat format,
                    SVGA3dSurfaceFormatCaps *caps);

void
svga_format_size(SVGA3dSurfaceFormat format,
                 unsigned *block_width,
                 unsigned *block_height,
                 unsigned *bytes_per_block);

const char *
svga_format_name(SVGA3dSurfaceFormat format);

bool
svga_format_is_integer(SVGA3dSurfaceFormat format);

bool
svga_format_support_gen_mips(enum pipe_format format);

enum tgsi_return_type
svga_get_texture_datatype(enum pipe_format format);


// XXX: Move this to svga_context?
bool
svga_has_any_integer_cbufs(const struct svga_context *svga);


SVGA3dSurfaceFormat
svga_typeless_format(SVGA3dSurfaceFormat format);


SVGA3dSurfaceFormat
svga_sampler_format(SVGA3dSurfaceFormat format);


bool
svga_format_is_uncompressed_snorm(SVGA3dSurfaceFormat format);


bool
svga_format_is_typeless(SVGA3dSurfaceFormat format);

bool
svga_format_is_shareable(const struct svga_screen *ss,
                         enum pipe_format pformat,
                         SVGA3dSurfaceFormat sformat,
                         unsigned bind,
                         bool verbose);

SVGA3dSurfaceFormat
svga_linear_to_srgb(SVGA3dSurfaceFormat format);


bool
svga_is_format_supported(struct pipe_screen *screen,
                         enum pipe_format format,
                         enum pipe_texture_target target,
                         unsigned sample_count,
                         unsigned storage_sample_count,
                         unsigned bindings);


bool
svga_is_dx_format_supported(struct pipe_screen *screen,
                            enum pipe_format format,
                            enum pipe_texture_target target,
                            unsigned sample_count,
                            unsigned storage_sample_count,
                            unsigned bindings);

#endif /* SVGA_FORMAT_H_ */
