/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_FORMATS_H
#define D3D12_FORMATS_H

#include "d3d12_common.h"

#include <directx/dxgiformat.h>
#include <directx/dxgicommon.h>

#include "util/format/u_formats.h"
#include "pipe/p_defines.h"
#include "pipe/p_video_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

DXGI_FORMAT
d3d12_get_format(enum pipe_format format);

DXGI_FORMAT
d3d12_get_typeless_format(enum pipe_format format);

const DXGI_FORMAT *
d3d12_get_format_cast_list(enum pipe_format format, uint32_t *num_formats);

/* These two are only used for importing external resources without a provided template */
enum pipe_format
d3d12_get_pipe_format(DXGI_FORMAT format);

enum pipe_format
d3d12_get_default_pipe_format(DXGI_FORMAT format);

DXGI_FORMAT
d3d12_get_resource_srv_format(enum pipe_format f, enum pipe_texture_target target);

DXGI_FORMAT
d3d12_get_resource_rt_format(enum pipe_format f);

unsigned
d3d12_non_opaque_plane_count(DXGI_FORMAT f);

struct d3d12_format_info {
   const enum pipe_swizzle *swizzle;
   int plane_slice;
};

struct d3d12_format_info
d3d12_get_format_info(enum pipe_format resource_format, enum pipe_format format, enum pipe_texture_target);

enum pipe_format
d3d12_emulated_vtx_format(enum pipe_format fmt);

unsigned
d3d12_get_format_start_plane(enum pipe_format fmt);

unsigned
d3d12_get_format_num_planes(enum pipe_format fmt);

DXGI_FORMAT
d3d12_convert_pipe_video_profile_to_dxgi_format(enum pipe_video_profile profile);

DXGI_COLOR_SPACE_TYPE
d3d12_convert_from_legacy_color_space(bool rgb, uint32_t bits_per_element, bool studio_rgb, bool p709, bool studio_yuv);

#ifdef __cplusplus
}
#endif

#endif
